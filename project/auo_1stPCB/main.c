﻿#include <sys/ioctl.h>
#include <unistd.h>
#include "libxml/parser.h"
#include "SDL/SDL.h"
#include "ite/itp.h"
#include "ctrlboard.h"
#include "scene.h"
#include "network_config.h"

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

int SDL_main(int argc, char *argv[])
{
    int ret = 0;
    int restryCount = 0;

	ithGpioSetOut(60);
	ithGpioSetMode(60, ITH_GPIO_MODE0);
	ithGpioSet(60);

    
#ifdef CFG_LCD_MULTIPLE
    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_RESET, (void*)0);
#endif

#if !defined(CFG_LCD_INIT_ON_BOOTING) && !defined(CFG_BL_SHOW_LOGO)
    ScreenClear();
#endif

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


//	NetworkInit_wifi_on();

#ifdef CFG_NET_ENABLE
    NetworkInit();
    #ifdef CFG_NET_WIFI
    if (Ethernet_Wifi_DualMAC == 1)
        WebServerInit();
    #else 
        WebServerInit();
    #endif
#endif // CFG_NET_ENABLE

    ScreenInit();
    ExternalInit();
#if defined(CFG_UPGRADE_FROM_UART_RUN_TIME)
	UpgradeUartInit();
#endif
    StorageInit();
    AudioInit();
    PhotoInit();
#ifndef WIN32
	Can_init();
#endif
    SceneInit();
    SceneLoad();
    ret = SceneRun();
	ithPrintf("(DBG OTA): %d\n", __LINE__);

    SceneExit();
	ithPrintf("(DBG OTA): %d\n", __LINE__);

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_EXIT, NULL);
	ithPrintf("(DBG OTA): %d\n", __LINE__);

    PhotoExit();
    AudioExit();
	
#if defined(CFG_UPGRADE_FROM_UART_RUN_TIME)
	UpgradeUartExit();
#endif
	ithPrintf("(DBG OTA): %d\n", __LINE__);

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
	printf("DBG: %d\n", __LINE__);

    ret = UpgradeProcess(ret);
    itp_codec_standby();
    exit(ret);
    return ret;
}
