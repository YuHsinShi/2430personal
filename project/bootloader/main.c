#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "ite/ite_usbex.h"
#include "bootloader.h"
#include "config.h"
#include "ite/itp.h"
#if defined(CFG_ENABLE_UART_CLI) || defined(CFG_OPENRTOS_CLI)
#include "ite/cli.h"
#endif

static pthread_t loadImageTask;
static bool upgradeCheckFail = false;
static bool blLcdOn = false;
static bool blLcdConsoleOn = false;
static bool stop_led = false;

#if defined(CFG_ENABLE_UART_CLI)
extern bool cliQuit;
//char tftppara[128] = "tftp://192.168.191.102/ITEPKG03.PKG";
#endif //end of #if defined (CFG_ENABLE_UART_CLI)

#if defined(CFG_UPGRADE_FROM_UART_BOOT_TIME)
#if defined(CFG_UPGRADE_UART0)
#define UPGRADE_UART_PORT       ITP_DEVICE_UART0
#define UPGRADE_UART_BAUDRATE   CFG_UART0_BAUDRATE
#define UPGRADE_UART_DEVICE     itpDeviceUart0
#elif defined(CFG_UPGRADE_UART1)
#define UPGRADE_UART_PORT       ITP_DEVICE_UART1
#define UPGRADE_UART_BAUDRATE   CFG_UART1_BAUDRATE
#define UPGRADE_UART_DEVICE     itpDeviceUart1
#else
#define UPGRADE_UART_PORT       ITP_DEVICE_UART0
#define UPGRADE_UART_BAUDRATE   CFG_UART0_BAUDRATE
#define UPGRADE_UART_DEVICE     itpDeviceUart0
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

#if defined(CFG_MSC_ENABLE) || defined(CFG_USBH_CD_MST)
static bool usbInited = false;
#endif

#ifdef CFG_UPGRADE_FILE_FOR_NAND_PROGRAMMER
extern unsigned char gDoReMapFlow;
#endif

static void BeginLoadImage(void)
{
    if (!upgradeCheckFail)
    {
        pthread_create(&loadImageTask, NULL, LoadImage, NULL);
    }
}

static void EndLoadImage(bool doBoot)
{
    if (!upgradeCheckFail)
    {
        pthread_join(loadImageTask, NULL);
        if (!doBoot)
            ReleaseImage();
    }
}

#ifdef CFG_UPGRADE_USB_DEVICE

static bool DetectUsbDeviceMode(void)
{
    bool ret = false;
    LOG_INFO "Detect USB device mode...\r\n" LOG_END

    // init card device
#if  !defined(_WIN32) && (defined(CFG_SD0_ENABLE) || defined(CFG_SD1_ENABLE) || defined(CFG_USBH_CD_MST) || defined(CFG_MSC_ENABLE) || defined(CFG_RAMDISK_ENABLE))
    itpRegisterDevice(ITP_DEVICE_CARD, &itpDeviceCard);
    ioctl(ITP_DEVICE_CARD, ITP_IOCTL_INIT, NULL);
#endif

    // init usb
#if defined(CFG_USB0_ENABLE) || defined(CFG_USB1_ENABLE) || defined(CFG_USBHCC)
    itpRegisterDevice(ITP_DEVICE_USB, &itpDeviceUsb);
    if (ioctl(ITP_DEVICE_USB, ITP_IOCTL_INIT, NULL) != -1)
        usbInited = true;
#endif

#ifdef CFG_USBD_ACM
    if (usbInited)
    {
        int timeout = CFG_UPGRADE_USB_DETECT_TIMEOUT;
        ITPUsbInfo usbInfo = {0};
        usbInfo.host = false;

        itpRegisterDevice(ITP_DEVICE_USBDACM, &itpDeviceUsbdAcm);
        ioctl(ITP_DEVICE_USBDACM, ITP_IOCTL_INIT, NULL);

        itpRegisterDevice(ITP_DEVICE_USBD, &itpDeviceUsbd);
        ioctl(ITP_DEVICE_USBD, ITP_IOCTL_INIT, NULL);

        while (!ret)
        {
            ioctl(ITP_DEVICE_USB, ITP_IOCTL_GET_INFO, (void*)&usbInfo);
            if (usbInfo.b_device)
            {
                ret = true;
                break;
            }

            timeout -= 10;
            if (timeout <= 0)
            {
                LOG_INFO "USB ACM device not connected.\n" LOG_END
                break;
            }
            usleep(10000);
        }
    }
#endif // CFG_USBD_ACM
    return ret;
}
#endif // CFG_UPGRADE_USB_DEVICE

static void InitFileSystem(void)
{
    // init card device
#if  !defined(CFG_UPGRADE_USB_DEVICE) && !defined(_WIN32) && (defined(CFG_SD0_ENABLE) || defined(CFG_SD1_ENABLE) || defined(CFG_MSC_ENABLE) || defined(CFG_USBH_CD_MST) || defined(CFG_RAMDISK_ENABLE))
	ithPrintf( "=%s %d=\n",__func__,__LINE__);
    itpRegisterDevice(ITP_DEVICE_CARD, &itpDeviceCard);
    ioctl(ITP_DEVICE_CARD, ITP_IOCTL_INIT, NULL);
#endif
	ithPrintf( "=%s %d=\n",__func__,__LINE__);

    // init usb
#if defined(CFG_MSC_ENABLE) || defined(CFG_USBH_CD_MST)
    if (!usbInited)
    {
        itpRegisterDevice(ITP_DEVICE_USB, &itpDeviceUsb);
        if (ioctl(ITP_DEVICE_USB, ITP_IOCTL_INIT, NULL) != -1)
            usbInited = true;
    }
#endif
#ifdef CFG_SD0_STATIC
		itpRegisterDevice(ITP_DEVICE_SD0, &itpDeviceSd0);
		ioctl(ITP_DEVICE_SD0, ITP_IOCTL_INIT, NULL);
#endif



	ithPrintf( "=%s %d=\n",__func__,__LINE__);

    // init fat
#ifdef CFG_FS_FAT
    itpRegisterDevice(ITP_DEVICE_FAT, &itpFSDeviceFat.dev);
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_INIT, NULL);
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_ENABLE, NULL);
#endif

    // init drive table
	itpRegisterDevice(ITP_DEVICE_DRIVE, &itpDeviceDrive);
	ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_INIT, 	 NULL);
	ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_MOUNT,	 NULL);
	ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_ENABLE,	 NULL);
#ifdef CFG_TASK_DRIVE_PROBE
	ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_INIT_TASK, NULL);
#endif


}

#if !defined(CFG_LCD_ENABLE) && (CFG_CHIP_FAMILY == 970)
static void* UgLedTask(void* arg)
{
    int gpio_pin = 15;
    ithGpioSetOut(16);
    ithGpioSetMode(16,ITH_GPIO_MODE0);
    ithGpioSetOut(15);
    ithGpioSetMode(15,ITH_GPIO_MODE0);
    stop_led = false;

    for(;;)
    {
        if(stop_led == true)
        {
            ithGpioSet(15);
            ithGpioSet(16);
            while(1)
                usleep(500000);
        }
        ithGpioClear(gpio_pin);
        if(gpio_pin==16)
            gpio_pin = 15;
        else
            gpio_pin = 16;
        ithGpioSet(gpio_pin);
        usleep(500000);
    }
}
#endif



static void InitLcdConsole(void)
{
    // output messages to LCD console
#if defined(CFG_LCD_ENABLE) && defined(CFG_BL_LCD_CONSOLE)
    if (!blLcdOn)
    {
    #if !defined(CFG_BL_SHOW_LOGO)
        extern uint32_t __lcd_base_a;
        extern uint32_t __lcd_base_b;
        extern uint32_t __lcd_base_c;

        itpRegisterDevice(ITP_DEVICE_SCREEN, &itpDeviceScreen);
        ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_RESET, NULL);
        ithLcdSetBaseAddrA((uint32_t) &__lcd_base_a);
        ithLcdSetBaseAddrB((uint32_t) &__lcd_base_b);

    #ifdef CFG_BACKLIGHT_ENABLE
        itpRegisterDevice(ITP_DEVICE_BACKLIGHT, &itpDeviceBacklight);
        ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_INIT, NULL);
    #endif // CFG_BACKLIGHT_ENABLE

    #endif // !defined(CFG_BL_SHOW_LOGO)

        ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
        ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_RESET, NULL);
        blLcdOn = true;
    }
    if (!blLcdConsoleOn)
    {
        itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceLcdConsole);
        itpRegisterDevice(ITP_DEVICE_LCDCONSOLE, &itpDeviceLcdConsole);
        ioctl(ITP_DEVICE_LCDCONSOLE, ITP_IOCTL_INIT, NULL);
        ioctl(ITP_DEVICE_LCDCONSOLE, ITP_IOCTL_CLEAR, NULL);
        blLcdConsoleOn = true;
    }
#endif // defined(CFG_LCD_ENABLE) && defined(BL_LCD_CONSOLE)
}

static void DoUpgrade(void)
{
    ITCStream* upgradeFile;

    LOG_INFO "Do Upgrade...\r\n" LOG_END

    upgradeFile = OpenUpgradePackage();
    if (upgradeFile)
    {
        int ret = 0;


        //InitLcdConsole();
        EndLoadImage(false);

        if (ugCheckCrc(upgradeFile, NULL))
        {
            LOG_ERR "Upgrade failed.\n" LOG_END
            //ShowUpgradeFail();
            while (1)
                sleep(10);
        }
        else
            ret = ugUpgradePackage(upgradeFile);

    #ifdef CFG_UPGRADE_DELETE_PKGFILE_AFTER_FINISH
        DeleteUpgradePackage();
    #endif

    #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
        LOG_INFO "Flushing NOR cache...\n" LOG_END
        ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
    #endif



        if (ret)
        {
            LOG_INFO "Upgrade failed.\n" LOG_END
            ShowUpgradeFail();
            while (1)
                sleep(10);
        }
        else
        {
            stop_led = true;
            LOG_INFO "Upgrade finished.\n" LOG_END
        }
    #if defined(CFG_UPGRADE_DELAY_AFTER_FINISH) && CFG_UPGRADE_DELAY_AFTER_FINISH > 0
        sleep(CFG_UPGRADE_DELAY_AFTER_FINISH);
    #endif

        exit(ret);
        while (1);
    }
}

#ifdef CFG_BOOT_TESTBIN_ENABLE
static void DoBootTestBin(void)
{
    ITCStream* testBinFile;

    LOG_INFO "Do Test Bin Booting...\r\n" LOG_END

    testBinFile = OpenTestBin();
    if (testBinFile)
    {
        EndLoadImage(false);

        BootTestBin(testBinFile);
    }
}
#endif

#ifdef CFG_BACKLIGHT_ENABLE
static void* backlightResetTask(void* arg)
{
    usleep(100000);
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_RESET, NULL);
    return NULL;
}
#endif

#if defined(CFG_UPGRADE_FROM_UART_BOOT_TIME)
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

	//Init UartTimeout_handler
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
							exit(ret);
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

		LOG_INFO "FAIL state, Uart Connection Error\n" LOG_END
		if(pkgData)
			free(pkgData);

		exit(-1);
	}
	else {
		//Download Finish
		ACK[0] = ACK210;
		write(UPGRADE_UART_PORT, ACK, 1);

		LOG_INFO "Finish state\n" LOG_END
	}

#if 0
	for(i = 0; i < 512; i++) {
		printf("%02X ", pkgData[i]);
		if(((i+1) % 16) == 0) {
			printf("\n");
		}
	}
	printf("\n");
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
		LOG_INFO "fwStream is NULL\n" LOG_END
		free(pkgData);
		ret = -1;
		goto UPGRADE_ERROR;
	}

  InitLcdConsole();

	ret = ugCheckCrc(fwStream, NULL);
	if (ret)
	{
		LOG_INFO "check crc fail\n" LOG_END
		goto UPGRADE_ERROR;
	}

	ret = ugUpgradePackage(fwStream);
	if (ret)
	{
		LOG_INFO "Upgrade failed.\n" LOG_END
		goto UPGRADE_ERROR;
	}

#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
    LOG_INFO "Flushing NOR cache...\n" LOG_END
    ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
#endif

#ifdef CFG_NAND_ENABLE
	ioctl(ITP_DEVICE_NAND, ITP_IOCTL_FLUSH, (void*)ITP_NAND_FTL_MODE);
#endif]

	LOG_INFO "upgrade success!\n" LOG_END

#if defined(CFG_UPGRADE_DELAY_AFTER_FINISH) && CFG_UPGRADE_DELAY_AFTER_FINISH > 0
	sleep(CFG_UPGRADE_DELAY_AFTER_FINISH);
#endif

	//Delete UgPercentage_handler Timer
	timer_delete(timer2);

	//Upgrade OK
	ACK[0] = ACK220;
	write(UPGRADE_UART_PORT, ACK, 1);

	exit(0);
	while(1);

UPGRADE_ERROR:
	//Delete UgPercentage_handler Timer
	timer_delete(timer2);

	//Upgrade Fail
	ACK[0] = ACK221;
	write(UPGRADE_UART_PORT, ACK, 1);

	exit(ret);
	while(1);

	return 0;
}

static void DetectUartPattern(void)
{
	unsigned char tmp[1024] = {0};
	unsigned char Ack[1] = {0};
	int readLen = 0, i = 0;
	unsigned int detect_count = 10000000;
	bool upgradeFlag = false;

	printf("DetectUartPattern start\n");

	itpRegisterDevice(UPGRADE_UART_PORT, &UPGRADE_UART_DEVICE);
	ioctl(UPGRADE_UART_PORT, ITP_IOCTL_RESET, NULL);

	while (detect_count)
	{
		readLen = read(UPGRADE_UART_PORT, tmp, 1024);
		if (readLen > 0 && !upgradeFlag)
		{
			detect_count = 10000000;
			for (i = 0; i < readLen; i++)
			{
				if (tmp[i] == UPGRADE_PATTERN)
				{
					upgradeFlag = true;
					break;
				}
			}
			if (upgradeFlag)
			{
				Ack[0] = ACK20;
				write(UPGRADE_UART_PORT, Ack, 1);
				break;
			}
		}
		else
			detect_count--;
	}

	if(upgradeFlag == true) {
		LOG_INFO "Upgrade FW by Uart\n" LOG_END
#if (UPGRADE_UART_PORT == ITP_DEVICE_UART0)
		ithUartSetRxTriggerLevel(ITH_UART0_BASE, ITH_UART_TRGL0);
#elif (UPGRADE_UART_PORT == ITP_DEVICE_UART1)
		ithUartSetRxTriggerLevel(ITH_UART1_BASE, ITH_UART_TRGL0);
#endif
		UpgradeUartPackage();
	}

	printf("DetectUartPattern end, detect_count=%d\n", detect_count);

}
#endif

void* BootloaderMain(void* arg)
{
    int ret = 0;
	ithPrintf( "BootloaderMain\n");

#if defined(CFG_UPGRADE_PRESSKEY) || defined(CFG_UPGRADE_RESET_FACTORY) || defined(CFG_UPGRADE_RECOVERY)
    ITPKeypadEvent ev;
#endif

#ifdef CFG_WATCHDOG_ENABLE
    ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_DISABLE, NULL);
#endif

    ithMemDbgDisable(ITH_MEMDBG0);
    ithMemDbgDisable(ITH_MEMDBG1);

    if (ugUpgradeCheck())
        upgradeCheckFail = true;

    BeginLoadImage();
	ithPrintf( "=%s %d=\n",__func__,__LINE__);


#ifdef CFG_UPGRADE_USB_DEVICE
    if (DetectUsbDeviceMode())
    {
        LOG_INFO "Do USB Device Commands\n" LOG_END
        EndLoadImage(false);
        InitFileSystem();
        DoUsbDeviceCommands();
    }
#endif // CFG_UPGRADE_USB_DEVICE
	ithPrintf( "=%s %d=\n",__func__,__LINE__);

#if defined(CFG_UPGRADE_PRESSKEY) || defined(CFG_UPGRADE_RESET_FACTORY) || defined(CFG_UPGRADE_RECOVERY)
    ioctl(ITP_DEVICE_KEYPAD, ITP_IOCTL_PROBE, NULL);
    if (read(ITP_DEVICE_KEYPAD, &ev, sizeof (ITPKeypadEvent)) == sizeof (ITPKeypadEvent))
    {
        int key = ev.code, delay = 0;

#endif // defined(CFG_UPGRADE_PRESSKEY) || defined(CFG_UPGRADE_RESET_FACTORY) || defined(CFG_UPGRADE_RECOVERY)

    #ifdef CFG_UPGRADE_RECOVERY
        if (key == CFG_UPGRADE_RECOVERY_PRESSKEY_NUM)
        {
            struct timeval time = ev.time;

            // detect key pressed time
            for (;;)
            {
                if (ev.flags & ITP_KEYPAD_UP)
                    break;

                ioctl(ITP_DEVICE_KEYPAD, ITP_IOCTL_PROBE, NULL);
                if (read(ITP_DEVICE_KEYPAD, &ev, sizeof (ITPKeypadEvent)) == 0)
                    continue;

                delay += itpTimevalDiff(&time, &ev.time);
                time = ev.time;

                LOG_DBG "recovery key: time=%ld.%ld,code=%d,down=%d,up=%d,repeat=%d,delay=%d\r\n",
                    ev.time.tv_sec,
                    ev.time.tv_usec / 1000,
                    ev.code,
                    (ev.flags & ITP_KEYPAD_DOWN) ? 1 : 0,
                    (ev.flags & ITP_KEYPAD_UP) ? 1 : 0,
                    (ev.flags & ITP_KEYPAD_REPEAT) ? 1 : 0,
                    delay
                LOG_END

                if (delay >= CFG_UPGRADE_RECOVERY_PRESSKEY_DELAY)
                    break;
            };

            if (delay >= CFG_UPGRADE_RECOVERY_PRESSKEY_DELAY)
            {
                ITCStream* upgradeFile;
            #ifdef CFG_UPGRADE_RECOVERY_LED
                int fd = open(":led:" CFG_UPGRADE_RECOVERY_LED_NUM, O_RDONLY);
                ioctl(fd, ITP_IOCTL_FLICKER, (void*)500);
            #endif

                InitLcdConsole();

                LOG_INFO "Do Recovery...\r\n" LOG_END

                InitFileSystem();

                upgradeFile = OpenRecoveryPackage();
                if (upgradeFile)
                {
                    EndLoadImage(false);

                    if (ugCheckCrc(upgradeFile, NULL))
                        LOG_ERR "Recovery failed.\n" LOG_END
                    else
                        ret = ugUpgradePackage(upgradeFile);

                    #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
                        LOG_INFO "Flushing NOR cache...\n" LOG_END
                        ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
                    #endif

				    #if 0 //def CFG_NAND_ENABLE
				        ioctl(ITP_DEVICE_NAND, ITP_IOCTL_FLUSH, (void*)ITP_NAND_FTL_MODE);
				    #endif

                    if (ret)
                        LOG_INFO "Recovery failed.\n" LOG_END
                    else
                        LOG_INFO "Recovery finished.\n" LOG_END

                    exit(ret);
                }
                else
                {
                #ifdef CFG_UPGRADE_RECOVERY_LED
                    ioctl(fd, ITP_IOCTL_OFF, NULL);
                #endif
                }
                while (1);
            }
        }
    #endif // CFG_UPGRADE_RECOVERY

    #ifdef CFG_UPGRADE_RESET_FACTORY
        if (key == CFG_UPGRADE_RESET_FACTORY_PRESSKEY_NUM)
        {
            struct timeval time = ev.time;

            // detect key pressed time
            for (;;)
            {
                if (ev.flags & ITP_KEYPAD_UP)
                    break;

                ioctl(ITP_DEVICE_KEYPAD, ITP_IOCTL_PROBE, NULL);
                if (read(ITP_DEVICE_KEYPAD, &ev, sizeof (ITPKeypadEvent)) == 0)
                    continue;

                delay += itpTimevalDiff(&time, &ev.time);
                time = ev.time;

                LOG_DBG "reset key: time=%ld.%ld,code=%d,down=%d,up=%d,repeat=%d,delay=%d\r\n",
                    ev.time.tv_sec,
                    ev.time.tv_usec / 1000,
                    ev.code,
                    (ev.flags & ITP_KEYPAD_DOWN) ? 1 : 0,
                    (ev.flags & ITP_KEYPAD_UP) ? 1 : 0,
                    (ev.flags & ITP_KEYPAD_REPEAT) ? 1 : 0,
                    delay
                LOG_END

                if (delay >= CFG_UPGRADE_RESET_FACTORY_PRESSKEY_DELAY)
                    break;
            };

            if (delay >= CFG_UPGRADE_RESET_FACTORY_PRESSKEY_DELAY)
            {
                InitLcdConsole();

                LOG_INFO "Do Reset...\r\n" LOG_END

                EndLoadImage(false);

                InitFileSystem();
                ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_MOUNT, NULL);

                ret = ugResetFactory();

            #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
                LOG_INFO "Flushing NOR cache...\n" LOG_END
                ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
            #endif

			#ifdef CFG_NAND_ENABLE
				ioctl(ITP_DEVICE_NAND, ITP_IOCTL_FLUSH, (void*)ITP_NAND_FTL_MODE);
			#endif

                if (ret)
                    LOG_INFO "Reset failed.\n" LOG_END
                else
                    LOG_INFO "Reset finished.\n" LOG_END

                exit(ret);
                while (1);
            }
        }
    #endif // CFG_UPGRADE_RESET_FACTORY

    #ifdef CFG_UPGRADE_PRESSKEY
        if (key == CFG_UPGRADE_PRESSKEY_NUM)
        {
            struct timeval time = ev.time;

            // detect key pressed time
            for (;;)
            {
                if (ev.flags & ITP_KEYPAD_UP)
                    break;

                ioctl(ITP_DEVICE_KEYPAD, ITP_IOCTL_PROBE, NULL);
                if (read(ITP_DEVICE_KEYPAD, &ev, sizeof (ITPKeypadEvent)) == 0)
                    continue;

                delay += itpTimevalDiff(&time, &ev.time);
                time = ev.time;

                LOG_DBG "upgrade key: time=%ld.%ld,code=%d,down=%d,up=%d,repeat=%d,delay=%d\r\n",
                    ev.time.tv_sec,
                    ev.time.tv_usec / 1000,
                    ev.code,
                    (ev.flags & ITP_KEYPAD_DOWN) ? 1 : 0,
                    (ev.flags & ITP_KEYPAD_UP) ? 1 : 0,
                    (ev.flags & ITP_KEYPAD_REPEAT) ? 1 : 0,
                    delay
                LOG_END

                if (delay >= CFG_UPGRADE_PRESSKEY_DELAY)
                    break;
            };

            if (delay >= CFG_UPGRADE_PRESSKEY_DELAY)
            {
                InitFileSystem();
                DoUpgrade();
            }
        }
    #endif // CFG_UPGRADE_PRESSKEY
#if defined(CFG_UPGRADE_PRESSKEY) || defined(CFG_UPGRADE_RESET_FACTORY) || defined(CFG_UPGRADE_RECOVERY)
    }
#endif


	ithPrintf( "=%s %d=\n",__func__,__LINE__);

#if !defined(CFG_UPGRADE_PRESSKEY) && defined(CFG_UPGRADE_OPEN_FILE)

    InitFileSystem();
	ithPrintf( "=%s %d=\n",__func__,__LINE__);

    DoUpgrade();
#endif

	ithPrintf( "=%s %d=\n",__func__,__LINE__);
#ifdef CFG_ENABLE_UART_CLI
	cliInit();
	while (!cliQuit)
		sleep(1);
#endif

#if defined(CFG_UPGRADE_FROM_UART_BOOT_TIME)
	//Upgrade FW by Uart
	DetectUartPattern();
#endif

    if (upgradeCheckFail)
    {
        InitLcdConsole();

#ifdef CFG_UPGRADE_BACKUP_PACKAGE

        LOG_INFO "Last upgrade failed, try to restore from internal package...\r\n" LOG_END

    #ifndef CFG_UPGRADE_USB_DEVICE

        // init fat
    #ifdef CFG_FS_FAT
        itpRegisterDevice(ITP_DEVICE_FAT, &itpFSDeviceFat.dev);
        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_INIT, NULL);
        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_ENABLE, NULL);
    #endif // CFG_FS_FAT

        // mount disks on booting
        ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_MOUNT, NULL);

    #endif // !CFG_UPGRADE_USB_DEVICE

        RestorePackage();

#else
    ShowLastUpgradeFail();
    sleep(10);

#endif // CFG_UPGRADE_BACKUP_PACKAGE
    }

#ifdef CFG_BOOT_TESTBIN_ENABLE
    #if !defined(CFG_UPGRADE_OPEN_FILE)
    InitFileSystem();
    #endif
    DoBootTestBin();
#endif
    LOG_INFO "Do Booting...\r\n" LOG_END
while(1);// LAW to lock
	ithPrintf( "=%s %d=\n",__func__,__LINE__);

    EndLoadImage(true);
    BootImage();

    return NULL;
}
/*
#if (CFG_CHIP_PKG_IT9079)
void
ithCodecPrintfWrite(
    char* string,
    int length)
{
    // this is a dummy function for linking with library itp_boot. (itp_swuart_codec.c)
}

#endif
*/
