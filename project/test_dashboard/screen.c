#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include "SDL/SDL.h"
#include "ite/itp.h"
#include "ite/itu.h"
#include "ctrlboard.h"
#include "wifiMgr.h"
static bool screenOff,doubleclick;

static float screenSaverCountDown,screenSaverCountDownAgain;
static uint32_t screenSaverLastTick;

void ScreenInit(void)
{
    //ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_SET_BRIGHTNESS, (void*)theConfig.brightness);
    screenSaverLastTick = SDL_GetTicks();
    screenSaverCountDown = theConfig.screensaver_time * 60.0f;
	screenSaverCountDownAgain = theConfig.screensaver_time * 0.5f;
    screenOff = false;
	doubleclick = false;
}

void ScreenSetDoubleClick(void)
{
    doubleclick = true;
}

bool ScreenIsOff(void)
{
    return screenOff;
}

void ScreenSetBrightness(int value)
{
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_SET_BRIGHTNESS, (void*)value);
}

int ScreenGetMaxBrightness(void)
{
    return ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_GET_MAX_LEVEL, NULL);
}

void ScreenOffContinue(void)
{
#if defined(CFG_POWER_SLEEP)
    ioctl(ITP_DEVICE_POWER, ITP_IOCTL_SLEEP_CONTINUE, NULL);  
    screenOff = true;
#endif
}

void ScreenOff(void)
{
    int nRet;
    int nWiFiConnState = 0, nWiFiConnEcode = 0;
    puts("Screen Off!");

#ifdef CFG_POWER_STANDBY
    ioctl(ITP_DEVICE_POWER, ITP_IOCTL_STANDBY, NULL);
#elif defined(CFG_POWER_SLEEP)
#ifdef CFG_NET_WIFI
    //while (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_SLEEP_STATUS, NULL) != default_no_sleep_or_wakeup){
    while (WifiMgr_Is_Wpa_Wifi_Terminating()){        
        printf("[ScreenOff] wait wifi close process\n");
        usleep(100*1000);
		if (!ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_DEVICE_READY, NULL))
			break;
    }

    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_SLEEP, (void *)wakeup_to_sleep);
    usleep(400*1000);
    if(theConfig.wifi_on_off){
        do
        {
            nRet = wifiMgr_get_connect_state(&nWiFiConnState, &nWiFiConnEcode);
            usleep(1000*1000);
            printf("waiting for connection done\n");
            wifiMgr_CancelConnect();
        }  while(nWiFiConnState != WIFIMGR_CONNSTATE_STOP);
    }
    printf("connection is done\n");


    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_STOP_DHCP, NULL);
    wifiMgr_clientMode_sleep_disconnect();
    wifiMgr_terminate();


#endif
    ioctl(ITP_DEVICE_POWER, ITP_IOCTL_SLEEP, NULL);
#else
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_OFF, NULL);
#endif
    screenOff = true;
}

int gScreenOnCounter = 0;
void ScreenOn(void)
{
    printf("----------------------------------------Screen On!: %d-----------------------------------\n", ++gScreenOnCounter);

#ifdef CFG_POWER_STANDBY
    ioctl(ITP_DEVICE_POWER, ITP_IOCTL_RESUME, NULL);
#elif defined(CFG_POWER_SLEEP)
    ioctl(ITP_DEVICE_POWER, ITP_IOCTL_RESUME, NULL);
#ifdef CFG_NET_WIFI
	usleep(100*1000);
	ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_SLEEP, (void *)sleep_to_wakeup);
	//usleep(100*1000);
	/*while (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_SLEEP_STATUS, NULL) != default_no_sleep_or_wakeup){
		//printf("[ScreenOn] wait wifi resume process\n");
		usleep(100*1000);
	}*/
#endif
#else
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_ON, NULL);
#endif
	screenOff = false;
	doubleclick =false;
}

void ScreenSaverRefresh(void)
{
    screenSaverLastTick = SDL_GetTicks();
    screenSaverCountDown = theConfig.screensaver_time * 60.0f;
	screenSaverCountDownAgain = theConfig.screensaver_time * 0.5f;

#if defined(CFG_POWER_WAKEUP_TOUCH_DOUBLE_CLICK)
	if (doubleclick && screenOff && theConfig.screensaver_type == SCREENSAVER_BLANK)
#else
    if (screenOff && theConfig.screensaver_type == SCREENSAVER_BLANK)
#endif
    {
        ScreenOn();
    }
}

int ScreenSaverCheckForDoubleClick(void)
{
    uint32_t diff, tick = SDL_GetTicks();

    if (tick >= screenSaverLastTick)
    {
        diff = tick - screenSaverLastTick;
    }
    else
    {
        diff = 0xFFFFFFFF - screenSaverLastTick + tick;
    }

    //printf("ScreenSaverCheck: tick: %d diff: %d countdown: %d\n", tick, diff, (int)screenSaverCountDown);
    if (diff >= 1000)
    {
        screenSaverCountDownAgain -= (float)diff / 1000.0f;
        screenSaverLastTick = tick;

        if (screenSaverCountDownAgain <= 0.0f)
            return -1;
    }
    return 0;
}

int ScreenSaverCheck(void)
{
    uint32_t diff, tick = SDL_GetTicks();

    if (tick >= screenSaverLastTick)
    {
        diff = tick - screenSaverLastTick;
    }
    else
    {
        diff = 0xFFFFFFFF - screenSaverLastTick + tick;
    }

    //printf("ScreenSaverCheck: tick: %d diff: %d countdown: %d\n", tick, diff, (int)screenSaverCountDown);

    if (diff >= 1000)
    {
        screenSaverCountDown -= (float)diff / 1000.0f;
        screenSaverLastTick = tick;

        if (screenSaverCountDown <= 0.0f)
            return -1;
    }
    return 0;
}
bool ScreenSaverIsScreenSaving(void)
{
    return screenOff || (screenSaverCountDown <= 0.0f);
}

void ScreenClear(void)
{

}
#ifdef CFG_SCREENSHOT_ENABLE
void Screenshot(void* lcdSurf)
{
    static int screenshotCount = 0;
    static bool overwrite = false;
    char filePath[PATH_MAX];

#ifdef _WIN32

    // get file path
    if (overwrite)
    {
        sprintf(filePath, CFG_TEMP_DRIVE ":/screenshot%04d.ppm", screenshotCount++);
    }
    else
    {
        do
        {
            sprintf(filePath, CFG_TEMP_DRIVE ":/screenshot%04d.ppm", screenshotCount++);
        } while (screenshotCount < 10000 && access(filePath, R_OK) == 0);
    }

    if (screenshotCount >= 10000)
    {
        screenshotCount = 0;
        overwrite = true;
    }

#else
    ITPDriveStatus* driveStatusTable;
    int i;

    // try to find the usb drive
    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    for (i = ITP_MAX_DRIVE - 1; i >= 0; i--)
    {
        ITPDriveStatus* driveStatus = &driveStatusTable[i];
        if (driveStatus->avail && driveStatus->disk >= ITP_DISK_MSC00 && driveStatus->disk <= ITP_DISK_MSC17)
        {
            // get file path
            if (overwrite)
            {
                sprintf(filePath, "%sscreenshot%04d.ppm", driveStatus->name, screenshotCount++);
            }
            else
            {
                do
                {
                    sprintf(filePath, "%sscreenshot%04d.ppm", driveStatus->name, screenshotCount++);
                } while (screenshotCount < 10000 && access(filePath, R_OK) == 0);
            }

            if (screenshotCount >= 10000)
            {
                screenshotCount = 0;
                overwrite = true;
            }
            break;
        }
    }
    if (i < 0)
    {
        printf("cannot find USB drive.\n");
        return;
    }
#endif // _WIN32
    ituScreenshot(lcdSurf, filePath);
}
#endif // CFG_SCREENSHOT_ENABLE
