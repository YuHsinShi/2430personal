#include <sys/ioctl.h>
#include <unistd.h>
#include "libxml/parser.h"
#include "SDL/SDL.h"
#include "ite/itp.h"
#include "project.h"
#include "scene.h"

#ifdef _WIN32
    #include <crtdbg.h>
#else
    #include "openrtos/FreeRTOS.h"
    #include "openrtos/task.h"
#endif

extern void BackupInit(void);
extern void BackupRestore(void);
extern void BackupSyncFile(void);
extern void BackupDestroy(void);







#ifndef WIN32
void test_spi_master_write()
{
	int fd = 0;
	ITPSpiInfo SpiInfo = {0};
	uint8_t command[5] = {0};
	uint8_t id2[3]	= {0};
	
	//itpRegisterDevice(ITP_DEVICE_SPI, &itpDeviceSpi0);
	//ioctl(ITP_DEVICE_SPI, ITP_IOCTL_INIT, NULL);
	
	printf("test_spi_master_write test!\n");
	
	fd = open(":spi0", O_RDONLY);
	if (!fd)
		printf("--- open device spi0 fail ---\n");
	else
		printf("fd = %d\n", fd);
	
	command[0] = 0x9F;
	
	SpiInfo.readWriteFunc = ITP_SPI_PIO_WRITE; //決定讀寫行為
	SpiInfo.cmdBuffer = &command;
	SpiInfo.cmdBufferSize = 1;
	SpiInfo.dataBuffer = &id2;
	SpiInfo.dataBufferSize = 3;
	
	while (1)
	{
		write(fd, &SpiInfo, 1);
		printf("SpiInfo 0x%x, 0x%x, 0x%x\n", id2[0],id2[1], id2[2]);
		sleep(1);
	}
	
	close(fd);
}


void test_spi_master_read()
{
	int fd = 0;
	ITPSpiInfo SpiInfo = {0};
	uint8_t command[5] = {0};
	uint8_t id2[3]	= {0};
	
	//itpRegisterDevice(ITP_DEVICE_SPI, &itpDeviceSpi0);
	//ioctl(ITP_DEVICE_SPI, ITP_IOCTL_INIT, NULL);
	
	printf("Start SPI test!\n");
	
	fd = open(":spi0", O_RDONLY);
	if (!fd)
	printf("--- open device spi0 fail ---\n");
	else
	printf("fd = %d\n", fd);
	
	command[0] = 0x9F;
	
	SpiInfo.readWriteFunc = ITP_SPI_PIO_READ; //決定讀寫行為
	SpiInfo.cmdBuffer = &command;
	SpiInfo.cmdBufferSize = 1;
	SpiInfo.dataBuffer = &id2;
	SpiInfo.dataBufferSize = 3;
	
	while (1)
	{
		read(fd, &SpiInfo, 1);
		printf("SpiInfo 0x%x, 0x%x, 0x%x\n", id2[0],id2[1], id2[2]);
	}
	
	close(fd);
}
#endif
int SDL_main(int argc, char *argv[])
{
    int ret = 0;
    int restryCount = 0;
    
#ifdef CFG_LCD_MULTIPLE
    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_RESET, (void*)0);
#endif



//test_spi_master();
//printf("HOLDING \n");

//while(1);



#ifdef CFG_CHECK_FILES_CRC_ON_BOOTING
    BackupInit();
retry_backup:
    ret = UpgradeInit();
    if (ret)
    {
        if (++restryCount > 2)
        {
            printf("cannot recover from backup....\n");
            goto end;
        }
        BackupRestore();
        goto retry_backup;
    }
    BackupSyncFile();
#else
    ret = UpgradeInit();
    if (ret)
        goto end;
#endif

#ifdef CFG_LCD_MULTIPLE
    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
    usleep(100000);
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_RESET, NULL);
#endif // CFG_LCD_MULTIPLE

#ifdef	CFG_DYNAMIC_LOAD_TP_MODULE
	//This function must be in front of SDL_Init().
	DynamicLoadTpModule();
#endif

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());

    ConfigInit();
	SettingInit();

#ifdef CFG_NET_ENABLE
    NetworkInit();
    WebServerInit();
#endif // CFG_NET_ENABLE

    ExternalInit();
#if defined(CFG_UPGRADE_FROM_UART)
	UpgradeUartInit();
#endif
    StorageInit();
    AudioInit();

    SceneInit();
    SceneLoad();


	
    ret = SceneRun();

    SceneExit();

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_EXIT, NULL);

    AudioExit();
#if defined(CFG_UPGRADE_FROM_UART)
	UpgradeUartExit();
#endif
    ExternalExit();

#ifdef CFG_NET_ENABLE
    if (ret != QUIT_UPGRADE_WEB)
        WebServerExit();

    xmlCleanupParser();
#endif // CFG_NET_ENABLE

    ConfigExit();
    SDL_Quit();

#ifdef CFG_CHECK_FILES_CRC_ON_BOOTING
    BackupDestroy();
#endif

#ifdef _WIN32
    _CrtDumpMemoryLeaks();
#else
    if (0)
    {
    #if (configUSE_TRACE_FACILITY == 1)
        static signed char buf[2048];
        vTaskList(buf);
        puts(buf);
    #endif
        malloc_stats();

    #ifdef CFG_DBG_RMALLOC
        Rmalloc_stat(__FILE__);
    #endif
    }
#endif // _WIN32

end:
    ret = UpgradeProcess(ret);
    itp_codec_standby();
    exit(ret);
    return ret;
}
