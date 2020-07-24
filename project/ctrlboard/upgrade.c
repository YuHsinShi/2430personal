#include <sys/ioctl.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "curl/curl.h"
#include "ite/ite_sd.h"
#include "ite/itu.h"
#include "ite/ug.h"
#include "SDL/SDL.h"
#include "ctrlboard.h"

#define URL_LEN 256

static bool upgradeIsReady;
static char upgradeUrl[URL_LEN];
static bool upgradeIsFinished;
static int upgradeResult;
static ITCArrayStream arrayStream;
static ITCFileStream fileStream;
static ITCStream* upgradeStream;
static char pkgFilePath[PATH_MAX];

#if defined(CFG_UPGRADE_FROM_UART_RUN_TIME)

#if defined(CFG_UPGRADE_UART0)
#define UPGRADE_UART_PORT       ITP_DEVICE_UART0
#elif defined(CFG_UPGRADE_UART1)
#define UPGRADE_UART_PORT       ITP_DEVICE_UART1
#endif

#define UPGRADE_PATTERN		0x1A
#define ACK20				0x14
#define ACK50				0x32
#define ACK100				0x64
#define ACK150				0x96
#define ACK200				0xC8
#define ACK210				0xD2
#define ACK211				0xD3
#define ACK220				0xDC
#define ACK221				0xDD

//the total check times is CHECK_NUM or CHECK_NUM+1
#define CHECK_NUM			4
#define RETRY_SIZE			5
#define RETRY_CHECKSUM		1
#define RETRY_DATA			1

#define CHECK_TIME			10

typedef enum UPGRADE_UART_STATE_TAG
{
    WAIT_SIZE,
	WAIT_SIZE_ACK,
	READY,
	WAIT_CHECKSUM,
	WAIT_CHECK_ACK,
	FINISH,
	FAIL,
} UPGRADE_UART_STATE;

static ITCArrayStream UartStream;
static UPGRADE_UART_STATE gState = WAIT_SIZE;
static UPGRADE_UART_STATE gTimeState[CHECK_TIME];
static int gSec = 0;
static unsigned int gTotalSize = 0;
static unsigned int gSecSize[CHECK_TIME];
#endif

struct MemoryStruct
{
  char *memory;
  size_t size;
};

#ifdef CFG_UPGRADE_GUI

static const uint8_t upgradeItu[] __attribute__ ((aligned)) =
{
#include "upgrade.inc"
};

static void* UpgradeGuiTask(void* arg)
{
    ITUScene scene;
    ITUProgressBar* bar;
    SDL_Window *window = SDL_CreateWindow("Display Control Board", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ithLcdGetWidth(), ithLcdGetHeight(), 0);

#ifdef CFG_M2D_ENABLE
    ituM2dInit();
#else
    ituSWInit();
#endif

    ituSceneInit(&scene, NULL);
    ituSceneLoadArray(&scene, upgradeItu, sizeof(upgradeItu));

    bar = (ITUProgressBar*)ituSceneFindWidget(&scene, "upgradeProgressBar");
    assert(bar);

    for (;;)
    {
        ituProgressBarSetValue(bar, ugGetProrgessPercentage());

        if (ituSceneUpdate(&scene, ITU_EVENT_TIMER, 0, 0, 0))
        {
            ituSceneDraw(&scene, ituGetDisplaySurface());
            ituFlip(ituGetDisplaySurface());
        }
        usleep(33000);
    }
}

#endif // CFG_UPGRADE_GUI

int UpgradeInit(void)
{
    int ret = 0;

#ifdef CFG_CHECK_FILES_CRC_ON_BOOTING

    ret = ugCheckFilesCrc(CFG_PUBLIC_DRIVE ":", CFG_PUBLIC_DRIVE ":/ite_crc.dat");
    if (ret)
    {
        printf("check public file crc result: %d\n", ret);
        ret = QUIT_RESET_FACTORY;
        goto end;
    }
end:
#endif // CFG_CHECK_FILES_CRC_ON_BOOTING

    upgradeStream = NULL;
    upgradeIsReady = false;
    upgradeIsFinished = false;
    upgradeResult = 0;
    return ret;
}

static void UpgradeLcdConsoleEnable(void)
{
#ifdef CFG_UPGRADE_GUI
    pthread_t task;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&task, &attr, UpgradeGuiTask, NULL);
#else
#ifdef CFG_LCD_ENABLE
    // switch to lcd console
    itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceLcdConsole);
    itpRegisterDevice(ITP_DEVICE_LCDCONSOLE, &itpDeviceLcdConsole);
    ioctl(ITP_DEVICE_LCDCONSOLE, ITP_IOCTL_INIT, NULL);
    ioctl(ITP_DEVICE_LCDCONSOLE, ITP_IOCTL_CLEAR, NULL);
#endif
#endif // CFG_UPGRADE_GUI
}

static int UpgradeResetFactory(void)
{
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*)ITP_DISK_NOR);
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORCE_MOUNT, (void*)ITP_DISK_NOR);
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORMAT, (void*)1);
    ugResetFactory();
    return 0;
}

static ITCStream* OpenUsbPackage(char* path)
{
    ITPDriveStatus* driveStatusTable;
    ITPDriveStatus* driveStatus = NULL;
    int i;

    // try to find the package drive
    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    for (i = ITP_MAX_DRIVE - 1; i >= 0; i--)
    {
        driveStatus = &driveStatusTable[i];
        if (driveStatus->avail && driveStatus->disk >= ITP_DISK_MSC00 && driveStatus->disk <= ITP_DISK_MSC17)
        {
            char buf[PATH_MAX], *ptr, *saveptr;

            // get file path from list
            strcpy(buf, path);
            ptr = strtok_r(buf, " ", &saveptr);
            do
            {
                strcpy(pkgFilePath, driveStatus->name);
                strcat(pkgFilePath, ptr);

                if (itcFileStreamOpen(&fileStream, pkgFilePath, false) == 0)
                {
                    printf("found package file %s\n", pkgFilePath);
                    return &fileStream.stream;
                }
                else
                {
                    printf("try to fopen(%s) fail:0x%X\n", pkgFilePath, errno);
                }
            }
            while ((ptr = strtok_r(NULL, " ", &saveptr)) != NULL);
        }
    }
    printf("cannot find package file.\n");
    return NULL;
}

#ifdef CFG_NET_ENABLE

static size_t throw_away(void *ptr, size_t size, size_t nmemb, void *data)
{
    (void)ptr;
    (void)data;
    /* we are not interested in the headers itself,
     so we only return the size we would have saved ... */
    return (size_t)(size * nmemb);
}

static int GetPackageSize(char* ftpurl)
{
    CURL* curl = NULL;
    CURLcode res = CURLE_OK;
    double filesize = 0.0;

    curl = curl_easy_init();
    if (!curl)
    {
        printf("curl_easy_init() fail.\n");
        goto end;
    }

    curl_easy_setopt(curl, CURLOPT_URL, ftpurl);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, throw_away);
    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

    /* some servers don't like requests that are made without a user-agent
     field, so we provide one */ 
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    curl_easy_setopt(curl, CURLOPT_FTP_RESPONSE_TIMEOUT, 15L);

#ifndef NDEBUG
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

    res = curl_easy_perform(curl);
    if (CURLE_OK != res)
    {
        printf("curl_easy_perform() fail: %d\n", res);
        goto end;
    }

    res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &filesize);
    if ((CURLE_OK == res) && (filesize > 0.0))
    {
        printf("filesize: %0.0f bytes\n", filesize);
    }
    else
    {
        printf("curl_easy_getinfo(CURLINFO_CONTENT_LENGTH_DOWNLOAD) fail: %d, filesize: %0.0f bytes\n", res, filesize);
        filesize = 0.0;
        goto end;
    }

end:
    if (curl)
        curl_easy_cleanup(curl);

    return (int)filesize;
}

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    assert(mem->memory);

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;

    return realsize;
}

static ITCStream* DownloadPackage(char* ftpurl, int filesize)
{
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(filesize);    /* will be grown as needed by the realloc above */ 
    chunk.size = 0;                     /* no data at this point */ 

    /* init the curl session */ 
    curl = curl_easy_init();
    if (!curl)
    {
        printf("curl_easy_init() fail.\n");
        goto error;
    }

    /* specify URL to get */ 
    curl_easy_setopt(curl, CURLOPT_URL, ftpurl);

    /* send all data to this function  */ 
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    /* we pass our 'chunk' struct to the callback function */ 
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    /* some servers don't like requests that are made without a user-agent
     field, so we provide one */ 
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    curl_easy_setopt(curl, CURLOPT_FTP_RESPONSE_TIMEOUT, 15L);

#ifndef NDEBUG
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

    /* get it! */
    res = curl_easy_perform(curl);
    if (CURLE_OK != res)
    {
        printf("curl_easy_perform() fail: %d\n", res);
        goto error;
    }
    else
    {
        printf("%lu bytes retrieved\n", (long)chunk.size);
    }

    curl_easy_cleanup(curl);

    itcArrayStreamOpen(&arrayStream, chunk.memory, chunk.size);

    return &arrayStream.stream;

error:
    if (curl)
        curl_easy_cleanup(curl);

    free(chunk.memory);

    return NULL;
}

static ITCStream* OpenFtpPackage(char* path)
{
    int filesize;
    ITCStream* fwStream = NULL;
    int retry = 10;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    printf("ftp url: %s\n", path);

    // get file size first
    while (retry-- >= 0)
    {
        filesize = GetPackageSize(path);
        if (filesize > 0)
            break;
    }

    // download firmware
    while (retry-- >= 0)
    {
        fwStream = DownloadPackage(path, filesize);
        if (fwStream)
            break;
    };

    curl_global_cleanup();
    return fwStream;
}
#endif // CFG_NET_ENABLE

static int UpgradePackage(void)
{
    int ret = 0;
    ITCStream* fwStream = NULL;

    if (upgradeUrl[0] == '\0')
    {
       // open from USB drive
       fwStream = OpenUsbPackage(CFG_UPGRADE_FILENAME_LIST);
       if (!fwStream)
       {
           ret = -1;
           printf("packages unavailable: %s\n", CFG_UPGRADE_FILENAME_LIST);
           return ret;
       }
    }
#ifdef CFG_NET_ENABLE
    else
    {
        // download from ftp server
        fwStream = OpenFtpPackage(upgradeUrl);
        if (!fwStream)
        {
            ret = -1;
            printf("remote package unavailable: %s\n", upgradeUrl);
            return ret;
        }
    }
#endif // CFG_NET_ENABLE

    ret = ugCheckCrc(fwStream, NULL);
    if (ret)
    {
        printf("check crc fail: %d.\n", ret);
        return ret;
    }

    ret = ugUpgradePackage(fwStream);
    if (ret)
    {
        printf("upgrade fail: %d.\n", ret);
        return ret;
    }

#ifdef CFG_UPGRADE_DELETE_PKGFILE_AFTER_FINISH
    if (upgradeUrl[0] == '\0')
    {
        remove(pkgFilePath);
    }
#endif

    printf("upgrade success!\n");
    
#if defined(CFG_UPGRADE_DELAY_AFTER_FINISH) && CFG_UPGRADE_DELAY_AFTER_FINISH > 0
    sleep(CFG_UPGRADE_DELAY_AFTER_FINISH);
#endif    
        
    return 0;
}

#if defined(CFG_UPGRADE_FROM_UART_RUN_TIME)
static void UartTimeout_handler(timer_t timerid, int arg)
{
	int i;
	int same_count = 0;
	
	gTimeState[gSec] = gState;
	gSecSize[gSec] = gTotalSize;
	gSec++;
	
	if(gSec >= CHECK_TIME) {  //10 sec
		for(i = 0; i < CHECK_TIME-1; i++) {
			if(gTimeState[i] == gTimeState[i+1]) {
				same_count++;
			}
		}	

		if(same_count == CHECK_TIME-1) {
			if(gState == READY) {
				same_count = 0;
				
				for(i = 0; i < CHECK_TIME-1; i++) {
					if(gSecSize[i] == gSecSize[i+1]) {
						same_count++;
					}
				}

				if(same_count == CHECK_TIME-1) {
					printf("Timeout, gTotalSize = %d\n", gTotalSize);
#if 0
					for(i = 0; i < CHECK_TIME; i++) {
						printf("gSecSize[%d]=%d\n", i, gSecSize[i]);
					}
#endif
					gState = FAIL;
				}
			}
			else if(gState != READY){
				printf("Timeout, state=%d\n", gState);
				gState = FAIL;
			}
		}
		else {
			//reset
			for(i = 0; i < CHECK_TIME; i++) {
				gTimeState[i] = WAIT_SIZE;
			}
		}

		gSec = 0;
	}	
}

static void UgPercentage_handler(timer_t timerid, int arg)
{
	unsigned char ProgressBar[1] = {0};

	//Notice: int(4 bytes) convert to unsigned char(1 byte)
	ProgressBar[0] = ugGetProrgessPercentage();
	
	write(UPGRADE_UART_PORT, ProgressBar, 1);
}

static int UpgradeUartPackage(void)
{
	int ret = 0, i = 0;
	ITCStream* fwStream = NULL;
	unsigned char tmp[1024] = {0};
	unsigned char *pkgData = NULL;
	unsigned char *checksumData = NULL;
	unsigned char SizeAck[4] = {0}, CSAck[4] = {0}, ACK[1] = {0};
	unsigned int tmpTotalSize = 0;
	unsigned int checkSum = 0;
	unsigned int PkgSize = 0, readLen = 0;
	int retrySize = 1, retryChecksum = 1, retryData = 1;
	int checkSize = 0, remainSize = 0;
	int sliceNum = 1, sliceSize = 0;
	timer_t timer, timer2;
    struct itimerspec value, value2;
	//FILE *fptr = NULL;
	
	printf("UpgradeUartPackage start\n");

	//Init TimeChecker
	for(i = 0; i < 10; i++) {
		gTimeState[i] = WAIT_SIZE;
	}
    timer_create(CLOCK_REALTIME, NULL, &timer);
    timer_connect(timer, (VOIDFUNCPTR)UartTimeout_handler, 0);
    value.it_value.tv_sec = value.it_interval.tv_sec = 1;
    value.it_value.tv_nsec = value.it_interval.tv_nsec = 0;
    timer_settime(timer, 0, &value, NULL);

	while(1) {
		switch(gState) {
			case WAIT_SIZE:
				readLen = read(UPGRADE_UART_PORT, tmp+tmpTotalSize, 1024);
				tmpTotalSize+=readLen;
				if(tmpTotalSize >= 3) {					
					//Get Pkg_size
					PkgSize = tmp[0] | tmp[1] << 8 | tmp[2] << 16;
					printf("PkgSize=0x%X\n", PkgSize);

					//Send Ack=0x64 + PkgSize
					SizeAck[0] = ACK50;
					SizeAck[1] = tmp[0];
					SizeAck[2] = tmp[1];
					SizeAck[3] = tmp[2];
					write(UPGRADE_UART_PORT, SizeAck, 4);

					//Change to WAIT_SIZE_ACK state
					gState = WAIT_SIZE_ACK;

					//reset data
					tmpTotalSize = 0;
					memset(tmp, 0, 1024);					
				}
				break;
			case WAIT_SIZE_ACK:
				readLen = read(UPGRADE_UART_PORT, tmp+tmpTotalSize, 1024);
				tmpTotalSize+=readLen;
				if(tmpTotalSize >= 1) {
					if(tmp[0] == ACK50 + 0x1) {
						//allocate memory for pkgData
						pkgData = (unsigned char *)malloc(PkgSize);
						if(pkgData == NULL) {
							printf("pkgData cannot be allocated\n");
							ret = -1;
							return ret;
						}
						memset(pkgData, 0, PkgSize);

						checkSize = PkgSize / CHECK_NUM;
						remainSize = PkgSize % CHECK_NUM; 
						printf("checkSize = %d remainSize = %d\n", checkSize, remainSize);

						//send Ack100
						ACK[0] = ACK100;
						write(UPGRADE_UART_PORT, ACK, 1);
						
						gState = READY;
					}
					else {
						printf("WAIT_SIZE_ACK FAIL:Retry %d\n", retrySize);
						//ACK50+0x28
						if(retrySize >= RETRY_SIZE) {
							gState = FAIL;
						}
						else {
							gState = WAIT_SIZE;
							retrySize++;
						}
					}

					//reset data
					tmpTotalSize = 0;
					memset(tmp, 0, 1024);
				}
				break;
			case READY:
				readLen = read(UPGRADE_UART_PORT, pkgData+gTotalSize, 1024);
				gTotalSize+=readLen;
				sliceSize+=readLen;

				if(sliceSize >= checkSize && sliceNum <= CHECK_NUM) {
					//Send Ack150
					ACK[0] = ACK150;
					write(UPGRADE_UART_PORT, ACK, 1);
				
					gState = WAIT_CHECKSUM;
				}
				else if(sliceSize >= remainSize && sliceNum > CHECK_NUM) {
					//the last slice, send Ack150
					ACK[0] = ACK150;
					write(UPGRADE_UART_PORT, ACK, 1);
				
					gState = WAIT_CHECKSUM;
				}
				break;
			case WAIT_CHECKSUM:
				readLen = read(UPGRADE_UART_PORT, tmp+tmpTotalSize, 1024);
				tmpTotalSize+=readLen;
				if(tmpTotalSize >= 4) {
					//printf("tmpTotalSize=%d TotalSize=%d sliceSize=%d\n", tmpTotalSize, TotalSize, sliceSize);
					//Calculate CheckSum(only new data need to do)
					if(retryChecksum == 1 || retryData > 1) {
						checksumData = pkgData + (gTotalSize - sliceSize);
						for(i = 0; i < sliceSize; i++) {
							checkSum += checksumData[i];
						}

						CSAck[0] = checkSum & 0xFF;
						CSAck[1] = (checkSum >> 8) & 0xFF;
						CSAck[2] = (checkSum >> 16) & 0xFF;
						CSAck[3] = (checkSum >> 24) & 0xFF;
					}
					
					printf("Checksum(PC): %x %x %x %x\n", tmp[0], tmp[1], tmp[2], tmp[3]);
					printf("Checksum(ITE): %x %x %x %x\n", CSAck[0], CSAck[1], CSAck[2], CSAck[3]);
					
					if((tmp[0] == CSAck[0]) && (tmp[1] == CSAck[1]) && (tmp[2] == CSAck[2]) && (tmp[3] == CSAck[3])) {
						//OK
						ACK[0] = ACK200;						
					}
					else {
						//FAIL
						ACK[0] = ACK200+0x28;
					}
					//Send Ack+OK/FAIL
					write(UPGRADE_UART_PORT, ACK, 1);
					gState = WAIT_CHECK_ACK;

					//reset data
					tmpTotalSize = 0;
					memset(tmp, 0, 1024);
				}
				break;
			case WAIT_CHECK_ACK:
				readLen = read(UPGRADE_UART_PORT, tmp+tmpTotalSize, 1024);
				tmpTotalSize+=readLen;
				if(tmpTotalSize >= 1) {
					if(tmp[0] == ACK200+1) {
						if(remainSize == 0 && sliceNum == CHECK_NUM) {
							gState = FINISH;
						}
						else if (remainSize != 0 && sliceNum > CHECK_NUM) {
							gState = FINISH;
						}
						else {
							gState = READY;
							
							//reset data
							tmpTotalSize = 0;
							memset(tmp, 0, 1024);
							sliceSize = 0;
							sliceNum++;
							retryChecksum = 1;
							retryData = 1;
							checkSum = 0;
						}						
					}
					else {						
						//ACK200+0x1+0x28
						if(retryChecksum <= RETRY_CHECKSUM) {
							printf("WAIT_CHECK_ACK FAIL, Retry Checksum\n");
							//retry checksum
							gState = WAIT_CHECKSUM;
							retryChecksum++;

							//reset data
							tmpTotalSize = 0;
							memset(tmp, 0, 1024);							
						}
						else if(retryData <= RETRY_DATA) {
							printf("WAIT_CHECK_ACK FAIL, Retry Data\n");
							//retry data
							gState = READY;
							retryData++;

							//reset data
							tmpTotalSize = 0;
							memset(tmp, 0, 1024);
							gTotalSize -= sliceSize;
							memset(pkgData+gTotalSize, 0, sliceSize);
							sliceSize = 0;	
							checkSum = 0;
						}
						else {
							//retry checksum and date done, but it still fail.
							gState = FAIL;
						}
					}
				}
				break;
			case FINISH:
				printf("FINISH state\n");
				break;
			case FAIL:
				break;
			default:
				break;
		}

		if(gState == FAIL || gState == FINISH) {
			printf("State=%d\n", gState);
			timer_delete(timer);
			break;
		}
	}

	if(gState == FAIL) {
		//Download Fail
		ACK[0] = ACK211;
		write(UPGRADE_UART_PORT, ACK, 1);
	
		printf("FAIL state, Uart Connection Error\n");
		if(pkgData)
			free(pkgData);

		return -1;
	}
	else {
		//Download Finish
		ACK[0] = ACK210;
		write(UPGRADE_UART_PORT, ACK, 1);

		printf("Finish state");
	}

#if 0
	fptr = fopen("E:/Record.txt", "wb");
	if(fptr == NULL) {
		printf("file is not exist\n");
		return NULL;
	}

	fwrite(pkgData, 1, PkgSize, fptr);
	fclose(fptr);
#endif

	//Init UgPercentage_handler Timer
	timer_create(CLOCK_REALTIME, NULL, &timer2);
	timer_connect(timer2, (VOIDFUNCPTR)UgPercentage_handler, 0);
	value2.it_value.tv_sec = value2.it_interval.tv_sec = 1;
	value2.it_value.tv_nsec = value2.it_interval.tv_nsec = 0;
	timer_settime(timer2, 0, &value2, NULL);

	//Upgrade FW start
	printf("Start to upgrade FW\n");
	itcArrayStreamOpen(&UartStream, pkgData, PkgSize);
	fwStream = &UartStream.stream;
	if (!fwStream) {
		printf("fwStream is NULL\n");
		free(pkgData);
		ret = -1;
		goto UPGRADE_ERROR;
	}

	ret = ugCheckCrc(fwStream, NULL);
    if (ret)
    {
        printf("check crc fail: %d.\n", ret);
        goto UPGRADE_ERROR;
    }

    ret = ugUpgradePackage(fwStream);
    if (ret)
    {
        printf("upgrade fail: %d.\n", ret);
        goto UPGRADE_ERROR;
    }

    printf("upgrade success!\n");
    
#if defined(CFG_UPGRADE_DELAY_AFTER_FINISH) && CFG_UPGRADE_DELAY_AFTER_FINISH > 0
    sleep(CFG_UPGRADE_DELAY_AFTER_FINISH);
#endif

	printf("UpgradeUartPackage end\n");

	//Delete UgPercentage_handler Timer
	timer_delete(timer2);

	//Upgrade OK
	ACK[0] = ACK220;
	write(UPGRADE_UART_PORT, ACK, 1);

	return 0;

UPGRADE_ERROR:
	//Delete UgPercentage_handler Timer
	timer_delete(timer2);
	
	//Upgrade Fail
	ACK[0] = ACK221;
	write(UPGRADE_UART_PORT, ACK, 1);
	
	return ret;
}
#endif

void UpgradeSetFileCrc(char* filepath)
{
    if (strncmp(filepath, CFG_PUBLIC_DRIVE, 1) == 0)
        ugSetFileCrc(&filepath[2], CFG_PUBLIC_DRIVE ":", CFG_PUBLIC_DRIVE ":/ite_crc.dat");
}

static void UpgradeFilesCrc(void)
{
    ugUpgradeFilesCrc(CFG_PUBLIC_DRIVE ":", CFG_PUBLIC_DRIVE ":/ite_crc.dat");
}

void UpgradeSetUrl(char* url)
{
    if (url)
    {
        strncpy(upgradeUrl, url, URL_LEN - 1);
        upgradeUrl[URL_LEN - 1] = '\0';
    }
    else
        upgradeUrl[0] = '\0';
}

void UpgradeSetStream(void* stream)
{
    upgradeIsFinished = false;
    upgradeResult = 0;
    upgradeStream = stream;
}
int UpgradeProcess(int code)
{
    int ret = 0;

    if (code == QUIT_RESET_FACTORY)
    {
        UpgradeLcdConsoleEnable();
        ret = UpgradeResetFactory();
    }
    else if (code == QUIT_UPGRADE_FIRMWARE)
    {
        UpgradeLcdConsoleEnable();
        ret = UpgradePackage();
    }
    else if (code == QUIT_UPGRADE_WEB)
    {
        upgradeStream = NULL;
        upgradeIsReady = true;

        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*)ITP_DISK_MSC00);
        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*)ITP_DISK_SD0);

        while (!upgradeStream)
            sleep(1);

        UpgradeLcdConsoleEnable();

    #ifdef CFG_WATCHDOG_ENABLE
        ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_DISABLE, NULL);
    #endif
        upgradeResult = ugUpgradePackage(upgradeStream);
        upgradeIsFinished = true;

    #ifdef CFG_WATCHDOG_ENABLE
        ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_ENABLE, NULL);
    #endif

        for (;;)
            sleep(UINT_MAX);
    }
#if defined(CFG_UPGRADE_FROM_UART_RUN_TIME)
	else if(code == QUIT_UPGRADE_UART)
	{
		printf("QUIT_UPGRADE_UART\n");
		UpgradeLcdConsoleEnable();
		ret = UpgradeUartPackage();
	}
#endif

    if (code == QUIT_RESET_FACTORY || code == QUIT_UPGRADE_FIRMWARE || code == QUIT_RESET_NETWORK || code == QUIT_DEFAULT || code == QUIT_UPGRADE_UART)
    {
    #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
        ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
    #endif
    }
    return ret;
}

bool UpgradeIsReady(void)
{
    return upgradeIsReady;
}

bool UpgradeIsFinished(void)
{
    return upgradeIsFinished;
}

int UpgradeGetResult(void)
{
    return upgradeResult;
}
