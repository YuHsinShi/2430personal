#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "ctrlboard.h"
#include "wifiMgr.h"
#include "sys/ioctl.h"

#ifdef CFG_NCP18_ENABLE
    #include "ncp18_sensor.h"

    #define NCP18_REFRESH_TIME    1000
#endif

static ITUProgressBar* settinLightProgressBar;
static ITUTrackBar* settingLightTrackBar;

static ITUProgressBar* settingVoiceProgressBar;
static ITUTrackBar* settingVoiceTrackBar;

static ITUProgressBar* settingScreenProtectTimeProgressBar;
static ITUTrackBar* settingScreenProtectTimeTrackBar;

static ITURadioBox* settingScreenSaverTypeRadioBox[2];
static ITURadioBox*  settingOpenWiFiRadioBox;
static ITURadioBox* settingCloseWiFiRadioBox;
static ITUContainer* settingWiFiSsidContainer;
static ITUScrollListBox *settingWiFiSsidNameScrollListBox;
static ITUScrollListBox *settingWiFiSsidStatusScrollListBox;
static ITUScrollListBox *settingWiFiSsidSignalScrollListBox;
static ITULayer         *settingWiFiPasswordLayer;
static ITULayer         *settingWiFiOthersLayer;
static ITULayer         *settingWiFiNetworkLayer;
static ITURadioBox *settingWiFiSsidOpenRadioBox;
static ITURadioBox *settingWiFiSsidCloseRadioBox;
static bool              settingWiFiReturnLayer = false;
//static ITUBackground* topBackground;


static ITUText* settingIPText;
static ITUText* settingMACAddressText;
static ITUText* Text_OTAAddress;

ITUSprite* topWiFiSprite;

//for temperature
static ITUText* settingCurrentTmpText;
static ITUTextBox* settingCalibrationTempTextBox;
static ITUTextBox* settingCalibrationOffsetTextBox;

static double current_tmp = 0, pre_tmp = 0;
static int current_tmp_point = 0;
static char temperature[32] = {0};
static double calibration_temp = 0.0;
static double calibration_offset = 0.0;

#ifdef CFG_NCP18_ENABLE
    static float current_tmp_float = 0;
    static uint32_t tick = 0, lasttick = 0, refresh_time = NCP18_REFRESH_TIME;
    static bool tick_first = true;
    static float offset_before_updating = 0;
    static float offset_update = 0;
#endif

static int is_set_calibration_temp = -1; //-1: user did not set any value, 0: user sets temp offset, 1: user sets calibration tmp

static WIFI_MGR_SCANAP_LIST pList[64];
static int                  gnApCount = 0;

extern void ScreenSetBrightness(int value);
#if defined(CFG_NET_WIFI_SDIO_NGPL)
extern uint8_t* LwIP_GetIP(struct netif *pnetif);
extern uint8_t* LwIP_GetMAC(struct netif *pnetif);
extern struct netif xnetif[NET_IF_NUM];
#endif

#define MAX_SSID_COUNT 15 // (sizeof(settingWiFiSsidNameArray) / sizeof(settingWiFiSsidNameArray[0]))

static int getMaxSsidCount()
{
	if (gnApCount <= 0)
	{
		return MAX_SSID_COUNT;
	}
	else
	{
		return gnApCount;
	}
}

bool SettingOnEnter(ITUWidget* widget, char* param)
{
	int i = 0;
    int nRet;
    int nWiFiConnState = 0, nWiFiConnEcode = 0;
    char buf[32];
#ifdef CFG_NET_WIFI_SDIO_NGPL
    unsigned char *mac = LwIP_GetMAC(&xnetif[0]);
    unsigned char *ip  = LwIP_GetIP(&xnetif[0]);
#endif
#ifdef CFG_NET_WIFI
    nRet = wifiMgr_get_connect_state(&nWiFiConnState, &nWiFiConnEcode);

#ifndef CFG_NET_WIFI_SDIO_NGPL
    /* For USB type WIFI(wifimgr) */
    if (nWiFiConnState == WIFIMGR_CONNSTATE_CONNECTING)
    {
        printf("SettingOnEnter connecting ,wait ....................\n");
        usleep(500000);
    }
#endif

    if (theConfig.wifi_on_off == WIFIMGR_SWITCH_ON)
    {
#ifdef CFG_NET_WIFI_SDIO_NGPL
        //Check wifimgr ecode,  (-1) means something happened that cause connection fail
        if (nWiFiConnEcode  == WIFIMGR_ECODE_OK
            || nWiFiConnEcode  == WIFIMGR_ECODE_SET_DISCONNECT
            || nWiFiConnEcode  == -1){
            //Check WIFI connection state from wifimgr
            if(nWiFiConnState == WIFIMGR_CONNSTATE_STOP){
                printf("SettingOnEnter press setting button, wait ....................\n");
                settingWiFiReturnLayer = false;
                //gnApCount = wifiMgr_get_scan_ap_info(pList);
            }else if (nWiFiConnState == WIFIMGR_CONNSTATE_CONNECTING){
                printf("SettingOnEnter connecting ,wait ....................\n");
                if (strcmp(pList[0].ssidName, "") == 0)
                    gnApCount = wifiMgr_get_scan_ap_info(pList); //If device can not get DHCP before set power-off, get new list again while power-on

                settingWiFiReturnLayer = true;
                usleep(500000);
            }else if (nWiFiConnState == WIFIMGR_CONNSTATE_SCANNING){
                printf("SettingOnEnter scanning ,wait ....................\n");
                settingWiFiReturnLayer = true;
                usleep(500000);
            }
        }else{
            printf("SettingOnEnter wifimgr is doing something(%d) ,wait ....................\n", nWiFiConnEcode);
            if (nWiFiConnEcode == WIFIMGR_ECODE_DHCP_ERROR)
                settingWiFiReturnLayer = true;

            usleep(500000);
        }
#else
        gnApCount = wifiMgr_get_scan_ap_info(pList);
#endif
    }
    else
    {
        printf("Wifi is closed, no SSID list...\n");
    }
#endif

	if (!settinLightProgressBar)
	{
		char tmp[32];

		// add for remove top layer wifi icon		
		topWiFiSprite = ituSceneFindWidget(&theScene, "topWiFiSprite");
		assert(topWiFiSprite);


		settinLightProgressBar = ituSceneFindWidget(&theScene, "settinLightProgressBar");
		assert(settinLightProgressBar);

		settingLightTrackBar = ituSceneFindWidget(&theScene, "settingLightTrackBar");
		assert(settingLightTrackBar);

		settingVoiceProgressBar = ituSceneFindWidget(&theScene, "settingVoiceProgressBar");
		assert(settingVoiceProgressBar);

		settingVoiceTrackBar = ituSceneFindWidget(&theScene, "settingVoiceTrackBar");
		assert(settingVoiceTrackBar);

		settingScreenProtectTimeProgressBar = ituSceneFindWidget(&theScene, "settingScreenProtectTimeProgressBar");
		assert(settingScreenProtectTimeProgressBar);

		settingScreenProtectTimeTrackBar = ituSceneFindWidget(&theScene, "settingScreenProtectTimeTrackBar");
		assert(settingScreenProtectTimeTrackBar);

		settingOpenWiFiRadioBox = ituSceneFindWidget(&theScene, "settingOpenWiFiRadioBox");
		assert(settingOpenWiFiRadioBox);

		settingCloseWiFiRadioBox = ituSceneFindWidget(&theScene, "settingCloseWiFiRadioBox");
		assert(settingCloseWiFiRadioBox);

		settingWiFiSsidContainer = ituSceneFindWidget(&theScene, "settingWiFiSsidContainer");
		assert(settingWiFiSsidContainer);

		settingWiFiSsidNameScrollListBox = ituSceneFindWidget(&theScene, "settingWiFiSsidNameScrollListBox");
		assert(settingWiFiSsidNameScrollListBox);

		settingWiFiSsidStatusScrollListBox = ituSceneFindWidget(&theScene, "settingWiFiSsidStatusScrollListBox");
		assert(settingWiFiSsidStatusScrollListBox);

		settingWiFiSsidSignalScrollListBox = ituSceneFindWidget(&theScene, "settingWiFiSsidSignalScrollListBox");
		assert(settingWiFiSsidSignalScrollListBox);

		settingWiFiPasswordLayer = ituSceneFindWidget(&theScene, "settingWiFiPasswordLayer");
		assert(settingWiFiPasswordLayer);

//		topBackground = ituSceneFindWidget(&theScene, "topBackground");
//		assert(topBackground);
		
		settingIPText = ituSceneFindWidget(&theScene, "settingIPText");
		assert(settingIPText);
		
		settingMACAddressText = ituSceneFindWidget(&theScene, "settingMACAddressText");
		assert(settingMACAddressText);
		

		Text_OTAAddress = ituSceneFindWidget(&theScene, "Text_OTAAddress");
		assert(Text_OTAAddress);

		ituTextSetString(Text_OTAAddress, CFG_UPGRADE_FTP_URL);
		
		//for temperature
		settingCurrentTmpText = ituSceneFindWidget(&theScene, "settingCurrentTmpText");
		assert(settingCurrentTmpText);

		settingCalibrationTempTextBox = ituSceneFindWidget(&theScene, "settingCalibrationTempTextBox");
		assert(settingCalibrationTempTextBox);

		settingCalibrationOffsetTextBox = ituSceneFindWidget(&theScene, "settingCalibrationOffsetTextBox");
		assert(settingCalibrationOffsetTextBox);


		for (i = 0; i < 2; i++)
		{
			sprintf(tmp, "settingScreenSaverTypeRadioBox%d", i);
			settingScreenSaverTypeRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(settingScreenSaverTypeRadioBox[i]);
		}
	}

	ituProgressBarSetValue(settinLightProgressBar, theConfig.brightness);
	ituTrackBarSetValue(settingLightTrackBar, theConfig.brightness);

	ituProgressBarSetValue(settingVoiceProgressBar, theConfig.keylevel);
	ituTrackBarSetValue(settingVoiceTrackBar, theConfig.keylevel);

	ituProgressBarSetValue(settingScreenProtectTimeProgressBar, theConfig.screensaver_time);
	ituTrackBarSetValue(settingScreenProtectTimeTrackBar, theConfig.screensaver_time);

	if (theConfig.screensaver_type == SCREENSAVER_BLANK)
		ituRadioBoxSetChecked(settingScreenSaverTypeRadioBox[0], true);
	else
		ituRadioBoxSetChecked(settingScreenSaverTypeRadioBox[1], true);

    /* Auto reconnection after system reboot, check IP/MAC first time */
    if (theConfig.wifi_on_off == WIFIMGR_SWITCH_ON) {
		ituRadioBoxSetChecked(settingCloseWiFiRadioBox, false);
		ituRadioBoxSetChecked(settingOpenWiFiRadioBox, true);
		ituWidgetSetVisible(settingWiFiSsidContainer, true);
#ifdef CFG_NET_WIFI
        if (strcmp(pList[0].ssidName, "") == 0)
            gnApCount = wifiMgr_get_scan_ap_info(pList);

#ifdef CFG_NET_WIFI_SDIO_NGPL
        if (ioctl(ITP_DEVICE_WIFI_NGPL, ITP_IOCTL_IS_AVAIL, NULL)){
            sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
                (uint8_t)mac[0],
                (uint8_t)mac[1],
                (uint8_t)mac[2],
                (uint8_t)mac[3],
                (uint8_t)mac[4],
                (uint8_t)mac[5]);

            ituTextSetString(settingMACAddressText, buf);
            ituTextSetString(settingIPText, ipaddr_ntoa(&xnetif[0].ip_addr));
        } else {
            ituTextSetString(settingIPText, "0.0.0.0");
        }
#endif
#endif
    } else {
		ituRadioBoxSetChecked(settingOpenWiFiRadioBox, false);
		ituRadioBoxSetChecked(settingCloseWiFiRadioBox, true);
		ituWidgetSetVisible(settingWiFiSsidContainer, false);
        ituTextSetString(settingIPText, "0.0.0.0");
    }

	if (ituRadioBoxIsChecked(settingOpenWiFiRadioBox))
	{
		//open WiFi
		//get wifi AP data....

#ifdef CFG_NET_WIFI_SDIO_NGPL
        sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
        (uint8_t)mac[0],
        (uint8_t)mac[1],
        (uint8_t)mac[2],
        (uint8_t)mac[3],
        (uint8_t)mac[4],
        (uint8_t)mac[5]);
        
		ituTextSetString(settingIPText, ipaddr_ntoa(&xnetif[0].ip_addr)); //set ip address string
		ituTextSetString(settingMACAddressText, buf); //set mac address string
#endif
		ituListBoxReload((ITUListBox*)settingWiFiSsidNameScrollListBox);
		ituListBoxReload((ITUListBox*)settingWiFiSsidStatusScrollListBox);
		ituListBoxReload((ITUListBox*)settingWiFiSsidSignalScrollListBox);
	}
	else if (ituRadioBoxIsChecked(settingCloseWiFiRadioBox))
	{
		//close WiFi

		//ituTextSetString(settingIPText, ""); //set ip address string
		//ituTextSetString(settingMACAddressText, "FF:FF:FF:FF:FF:FF"); //set mac address string
		
		memset(theConfig.ssid, 0, sizeof(theConfig.ssid));
		memset(theConfig.password, 0, sizeof(theConfig.password));
	}

#ifdef CFG_NET_WIFI_SDIO_NGPL
    if(ioctl(ITP_DEVICE_WIFI_NGPL, ITP_IOCTL_IS_AVAIL, NULL)){
		if (ip[0] != 0){
			ituSpriteGoto(topWiFiSprite, 2);
			ithPrintf("=========WiFi IP %s==========\n",ip);}
		else{
			ithPrintf("=========WiFi IP NULL======\n");
			ituSpriteGoto(topWiFiSprite, 1);}
    }
    else
#endif
    {
        ituSpriteGoto(topWiFiSprite, 2);
	   	//ithPrintf("=========WiFi not available======\n");
    }

#ifdef CFG_NCP18_ENABLE
    if (tick_first)
    {
        tick = refresh_time;
        tick_first = false;
    }
    else
    {
        tick = SDL_GetTicks();
    }

    if (tick - lasttick >= refresh_time)
    {
        current_tmp_float = NCP18_Detect();
        if (current_tmp_float > 0)
        {
            current_tmp = (double)((int)(current_tmp_float * 10 + 0.5) / 10.0);
        }
        else
        {
            current_tmp = (double)((int)(-current_tmp_float * 10 + 0.5) / 10.0);
            current_tmp = -current_tmp;
        }

        pre_tmp = current_tmp;
        lasttick = tick;

        if (refresh_time == 0) refresh_time = NCP18_REFRESH_TIME;
    }

    current_tmp_point = (int)(current_tmp * 10.0) - (int)current_tmp * 10;
    memset(temperature, 0x0, sizeof(temperature));

    if (theConfig.ncp18_offset >= 0)
        sprintf(temperature, "%d.%d (+%.2f)", (int)current_tmp, current_tmp_point, theConfig.ncp18_offset);
    else
        sprintf(temperature, "%d.%d (%.2f)", (int)current_tmp, current_tmp_point, theConfig.ncp18_offset);

    ituTextSetString(settingCurrentTmpText, temperature);
#endif
	
    return true;
}

bool SettingOnLeave(ITUWidget* widget, char* param)
{
	ConfigSave();
    return true;
}

bool SettingScreenSaverType(ITUWidget* widget, char* param)
{
	theConfig.screensaver_type = atoi(param);
    return true;
}

bool SettingProtectTimeTrackBarOnChanged(ITUWidget* widget, char* param)
{
	theConfig.screensaver_time = settingScreenProtectTimeTrackBar->value;
    return true;
}

bool SettingProtectTimeUpOnMouseUp(ITUWidget* widget, char* param)
{
	int value = settingScreenProtectTimeTrackBar->value;
	value = value + 5;
	if (value <= 30)
	{
		theConfig.screensaver_time = value;
		ituProgressBarSetValue(settingScreenProtectTimeProgressBar, theConfig.screensaver_time);
		ituTrackBarSetValue(settingScreenProtectTimeTrackBar, theConfig.screensaver_time);
	}

    return true;
}

bool SettingProtectTimeDownOnMouseUp(ITUWidget* widget, char* param)
{
	int value = settingScreenProtectTimeTrackBar->value;
	value = value - 5;
	if (value >= 5)
	{
		theConfig.screensaver_time = value;
		ituProgressBarSetValue(settingScreenProtectTimeProgressBar, theConfig.screensaver_time);
		ituTrackBarSetValue(settingScreenProtectTimeTrackBar, theConfig.screensaver_time);
	}

    return true;
}

bool SettingVoiceTrackBarOnChanged(ITUWidget* widget, char* param)
{
	theConfig.keylevel = settingVoiceTrackBar->value;
    AudioSetKeyLevel(theConfig.keylevel);
    return true;
}

bool SettingVoiceUpOnMouseUp(ITUWidget* widget, char* param)
{
	int value = settingVoiceTrackBar->value;
	value++;
	if (value <= 20)
	{
		theConfig.keylevel = value;
		ituProgressBarSetValue(settingVoiceProgressBar, theConfig.keylevel);
		ituTrackBarSetValue(settingVoiceTrackBar, theConfig.keylevel);
        AudioSetKeyLevel(theConfig.keylevel);
	}

    return true;
}

bool SettingVoiceDownOnMouseUp(ITUWidget* widget, char* param)
{
	int value = settingVoiceTrackBar->value;
	value--;
	if (value >= 0)
	{
		theConfig.keylevel = value;
		ituProgressBarSetValue(settingVoiceProgressBar, theConfig.keylevel);
		ituTrackBarSetValue(settingVoiceTrackBar, theConfig.keylevel);
        AudioSetKeyLevel(theConfig.keylevel);
	}
    return true;
}

bool SettingLightTrackBarOnChanged(ITUWidget* widget, char* param)
{
	theConfig.brightness = settingLightTrackBar->value;
    ScreenSetBrightness(theConfig.brightness - 1);
    return true;
}

bool SettingLightUpOnMouseUp(ITUWidget* widget, char* param)
{
	int value = settingLightTrackBar->value;
	value++;
	if (value <= 10)
	{
		theConfig.brightness = value;
		ituProgressBarSetValue(settinLightProgressBar, theConfig.brightness);
		ituTrackBarSetValue(settingLightTrackBar, theConfig.brightness);
        ScreenSetBrightness(theConfig.brightness - 1);
	}

    return true;
}

bool SettingLightDownOnMouseUp(ITUWidget* widget, char* param)
{
	int value = settingLightTrackBar->value;
	value--;
	if (value >= 1)
	{
		theConfig.brightness = value;
		ituProgressBarSetValue(settinLightProgressBar, theConfig.brightness);
		ituTrackBarSetValue(settingLightTrackBar, theConfig.brightness);
        ScreenSetBrightness(theConfig.brightness - 1);
	}

    return true;
}

bool SettingSetDefaultOKBtOnMouseUp(ITUWidget* widget, char* param)
{
    // Restore System Default
    SceneQuit(QUIT_RESET_FACTORY);
    
	return true;
}

bool SettingRebootBtOnMouseUp(ITUWidget* widget, char* param)
{
    //Reboot
	ConfigSave();
    SceneQuit(QUIT_DEFAULT);
	return true;
}

bool SettingWiFiSsidNameScrollListBoxOnLoad(ITUWidget* widget, char* param)
{
	ITUListBox          *listbox = (ITUListBox *)widget;
	ITUScrollListBox    *slistbox = (ITUScrollListBox *)listbox;
	ITCTree             *node;
	int                 i, j, count;

	assert(listbox);

	count = ituScrollListBoxGetItemCount(slistbox);
	node = ituScrollListBoxGetLastPageItem(slistbox);


	listbox->pageCount = getMaxSsidCount() ? (getMaxSsidCount() + count - 1) / count : 1;

	printf("SettingWiFiSsidNameScrollListBoxOnLoad,      SSID count per page = %d,   Need %d pages to show all counts \n", count, listbox->pageCount);

	if (listbox->pageIndex == 0)
	{
		// initialize
		listbox->pageIndex = 1;
		listbox->focusIndex = -1;
	}

	if (listbox->pageIndex <= 1)
	{
		for (i = 0; i < count; i++)
		{
			ITUScrollText *scrolltext = (ITUScrollText *)node;
			ituScrollTextSetString(scrolltext, "");

			node = node->sibling;
		}
	}

	i = 0;
	j = count * (listbox->pageIndex - 2);
	if (j < 0)
		j = 0;

	for (; j < getMaxSsidCount(); j++)
	{
		ITUScrollText   *scrolltext = (ITUScrollText *)node;
		char            buf[32];
		const char      *entry;//= settingWiFiSsidNameArray[j];

		if (theConfig.wifi_on_off == WIFIMGR_SWITCH_ON)
		{
			sprintf(buf, "%s", pList[j].ssidName);
		}
		else if (theConfig.wifi_on_off == WIFIMGR_SWITCH_OFF)
		{
			/*CANCEL PANEL DISPALY*/
			memset(pList[j].ssidName, 0, 32);
			pList[j].rfQualityQuant = 0;

			sprintf(buf, "%s", pList[j].ssidName);
			sprintf(buf, "%s", pList[j].rfQualityQuant);
		}

		ituScrollTextSetString(scrolltext, (char *)buf);
		ituWidgetSetCustomData(scrolltext, j);

		i++;

		node = node->sibling;

		if (node == NULL)
			break;
	}

	for (; node; node = node->sibling)
	{
		ITUScrollText *scrolltext = (ITUScrollText *)node;
		ituScrollTextSetString(scrolltext, "");
	}

	if (listbox->pageIndex == listbox->pageCount)
	{
		listbox->itemCount = i % count;
		if (listbox->itemCount == 0)
			listbox->itemCount = count;
	}
	else
		listbox->itemCount = count;

	return true;
}

#ifdef CFG_NET_WIFI_SDIO_NGPL
extern int SettingWiFiPasswordSetData(char *ssid, unsigned long securityMode);
#else
extern int SettingWiFiPasswordSetData(char *ssid, int securityMode);
#endif
bool SettingWiFiSsidScrollListBoxOnSelect(ITUWidget *widget, char *param)
{
	ITUListBox              *listbox = (ITUListBox *)widget;
	ITUScrollIconListBox    *silistbox = (ITUScrollIconListBox *)listbox;
	ITUScrollText           *scrolltext = (ITUScrollText *)ituListBoxGetFocusItem(listbox);
	int                     nIndex;

	if (scrolltext)
	{
		nIndex = (int)ituWidgetGetCustomData(scrolltext);
		if (!strncmp(theConfig.ssid, pList[nIndex].ssidName, strlen(pList[nIndex].ssidName)))//same do nothing
		{
			printf("SettingWiFiSsidScrollListBoxOnSelect \n");
		}
		else
		{
			SettingWiFiPasswordSetData(pList[nIndex].ssidName, pList[nIndex].securityMode);
			//if(have to enter password)
			//ituWidgetDisable(topBackground);
			ituLayerGoto(settingWiFiPasswordLayer);
			//else if  (no need to enter password)
			/*{
			connet to AP
			ituListBoxReload((ITUListBox*)settingWiFiSsidNameScrollListBox);
			ituListBoxReload((ITUListBox*)settingWiFiSsidStatusScrollListBox);
			ituListBoxReload((ITUListBox*)settingWiFiSsidSignalScrollListBox);
			}*/
		}

	}
	return true;
}

bool SettingWiFiSsidStatusScrollListBoxOnLoad(ITUWidget* widget, char* param)
{
	ITUListBox          *listbox = (ITUListBox *)widget;
	ITUScrollListBox    *slistbox = (ITUScrollListBox *)listbox;
	ITCTree             *node;
	int                 i, j, count;
	int                 nRet;
	int                 nConnect = 0;
	int                 bIsAvail = 0;

	assert(listbox);

	count = ituScrollListBoxGetItemCount(slistbox);
	node = ituScrollListBoxGetLastPageItem(slistbox);

	listbox->pageCount = getMaxSsidCount() ? (getMaxSsidCount() + count - 1) / count : 1;

	if (listbox->pageIndex == 0)
	{
		// initialize
		listbox->pageIndex = 1;
		listbox->focusIndex = -1;
	}

	if (listbox->pageIndex <= 1)
	{
		for (i = 0; i < count; i++)
		{
			ITUScrollText *scrolltext = (ITUScrollText *)node;
			ituScrollTextSetString(scrolltext, "");

			node = node->sibling;
		}
	}

	i = 0;
	j = count * (listbox->pageIndex - 2);
	if (j < 0)
		j = 0;

#ifdef CFG_NET_WIFI
	nRet = wifiMgr_is_wifi_available(&bIsAvail);
#endif
	printf("SettingWiFiSsidStatusScrollListBoxOnLoad,    IsAvail = %d,   Max Ssid Count = %d,   switch status = %d  \n", bIsAvail, getMaxSsidCount(), theConfig.wifi_on_off);

	for (; j < getMaxSsidCount(); j++)
	{
		ITUScrollText *scrolltext = (ITUScrollText *)node;

		if (!strncmp(theConfig.ssid, pList[j].ssidName, strlen(pList[j].ssidName)))
		{
			if (bIsAvail && nConnect == 0)
			{
#ifdef CFG_NET_WIFI_SDIO_NGPL
				if (settingWiFiReturnLayer){
					ituScrollTextSetString(scrolltext, "");
					return true;
				}
#endif
				ituScrollTextSetString(scrolltext, (char *)StringGetWiFiConnected());
				printf("SsidStatus %s \n", theConfig.ssid);
				nConnect++;
			}
			else
			{
				ituScrollTextSetString(scrolltext, "");
			}
		}
		else
		{
			ituScrollTextSetString(scrolltext, "");
		}

		ituWidgetSetCustomData(scrolltext, j);

		i++;

		node = node->sibling;

		if (node == NULL)
			break;
	}

	for (; node; node = node->sibling)
	{
		ITUScrollText *scrolltext = (ITUScrollText *)node;
		ituScrollTextSetString(scrolltext, "");
	}

	if (listbox->pageIndex == listbox->pageCount)
	{
		listbox->itemCount = i % count;
		if (listbox->itemCount == 0)
			listbox->itemCount = count;
	}
	else
		listbox->itemCount = count;

	return true;
}
bool SettingWiFiSsidSignalScrollListBoxOnLoad(ITUWidget* widget, char* param)
{
	ITUListBox          *listbox = (ITUListBox *)widget;
	ITUScrollListBox    *slistbox = (ITUScrollListBox *)listbox;
	ITCTree             *node;
	int                 i, j, count;
	assert(listbox);

	count = ituScrollListBoxGetItemCount(slistbox);
	node = ituScrollListBoxGetLastPageItem(slistbox);

	listbox->pageCount = getMaxSsidCount() ? (getMaxSsidCount() + count - 1) / count : 1;

	if (listbox->pageIndex == 0)
	{
		// initialize
		listbox->pageIndex = 1;
		listbox->focusIndex = -1;
	}

	if (listbox->pageIndex <= 1)
	{
		for (i = 0; i < count; i++)
		{
			ITUScrollText *scrolltext = (ITUScrollText *)node;
			ituScrollTextSetString(scrolltext, "");

			node = node->sibling;
		}
	}

	i = 0;
	j = count * (listbox->pageIndex - 2);
	if (j < 0)
		j = 0;

	for (; j < getMaxSsidCount(); j++)
	{
		ITUScrollText   *scrolltext = (ITUScrollText *)node;
		char            buf[8];

		sprintf(buf, "%d%%", pList[j].rfQualityQuant);

		ituScrollTextSetString(scrolltext, buf);

		ituWidgetSetCustomData(scrolltext, j);

		i++;

		node = node->sibling;

		if (node == NULL)
			break;
	}

	for (; node; node = node->sibling)
	{
		ITUScrollText *scrolltext = (ITUScrollText *)node;
		ituScrollTextSetString(scrolltext, "");
	}

	if (listbox->pageIndex == listbox->pageCount)
	{
		listbox->itemCount = i % count;
		if (listbox->itemCount == 0)
			listbox->itemCount = count;
	}
	else
		listbox->itemCount = count;

	return true;
}

bool SettingWiFiSsidOthersBtOnMouseUp(ITUWidget* widget, char* param)
{
	return true;
}

bool SettingOpenWiFiRadioBoxOnPress(ITUWidget* widget, char* param)
{
	//open WiFi
	//get wifi AP data....
	/*Eason*/
    theConfig.wifi_mode = WIFIMGR_MODE_MAX;
	theConfig.wifi_on_off = WIFIMGR_SWITCH_ON;
	#ifdef CFG_NET_WIFI
	WifiMgr_clientMode_switch(theConfig.wifi_on_off);
	gnApCount = wifiMgr_get_scan_ap_info(pList);
	#endif
	/***************************************************/

	ituListBoxReload((ITUListBox*)settingWiFiSsidNameScrollListBox);
	ituListBoxReload((ITUListBox*)settingWiFiSsidStatusScrollListBox);
	ituListBoxReload((ITUListBox*)settingWiFiSsidSignalScrollListBox);
	return true;
}

bool SettingWiFiSsidCloseRadioBoxOnPress(ITUWidget* widget, char* param)
{
	//close WiFi
	theConfig.wifi_on_off = WIFIMGR_SWITCH_OFF;
	ituSpriteGoto(topWiFiSprite, 0);
#ifdef CFG_NET_WIFI
    wifiMgr_clientMode_disconnect();
    WifiMgr_clientMode_switch(theConfig.wifi_on_off);
#ifdef CFG_NET_WIFI_SDIO_NGPL
	ip_addr_set_zero(&xnetif[0].ip_addr);
#endif
#endif

	memset(theConfig.ssid, 0, sizeof(theConfig.ssid));
	memset(theConfig.password, 0, sizeof(theConfig.password));
    ituTextSetString(settingIPText,         "0.0.0.0");
    ituTextSetString(settingMACAddressText, "FF:FF:FF:FF:FF:FF");
	return true;
}

bool SettingWiFiSsidOnLeave(ITUWidget *widget, char *param)
{
    return true;
}

bool SettingCoverFlowOnChanged(ITUWidget* widget, char* param)
{
	ITUCoverFlow* coverflow = (ITUCoverFlow*)widget;
	//printf("coverflow->focus %d \n", coverflow->focusIndex);
//	ituSpriteGoto(settingPageSprite, coverflow->focusIndex);
	return true;
}

//for temperature
bool SettingCalibrationTempBtOnMouseUp(ITUWidget* widget, char* param)
{
    #ifdef CFG_NCP18_ENABLE

    if (NULL != ituTextBoxGetString(settingCalibrationTempTextBox) && NULL == ituTextBoxGetString(settingCalibrationOffsetTextBox))
    {
        offset_before_updating = theConfig.ncp18_offset;

        calibration_temp = atof(ituTextBoxGetString(settingCalibrationTempTextBox));
        printf("calibration_temp: %f\n", calibration_temp);

        calibration_offset = calibration_temp - (current_tmp - offset_before_updating);
        offset_update = (float)calibration_offset;
        printf("calibration_offset: %f\n", offset_update);

        if (offset_update <= 0)
        {
            if ((offset_update * -1) >= NCP_18_TOLERANCE)
            {
                printf("invalid calibration_offset(must be within the range of %d to %d)\n", -1 * NCP_18_TOLERANCE, NCP_18_TOLERANCE);
                return true;
            }
        }
        else
        {
            if (offset_update >= NCP_18_TOLERANCE)
            {
                printf("invalid calibration_offset(must be within the range of %d to %d)\n", -1 * NCP_18_TOLERANCE, NCP_18_TOLERANCE);
                return true;
            }
        }

        if (offset_before_updating != offset_update)
        {
            theConfig.ncp18_offset = offset_update;
            ConfigSave();
            refresh_time = 0;
        }

        is_set_calibration_temp = 1; //-1: user did not set any value, 0: user sets temp offset, 1: user sets calibration tmp
    }

    else if (NULL != ituTextBoxGetString(settingCalibrationOffsetTextBox) && NULL == ituTextBoxGetString(settingCalibrationTempTextBox))
    {
        offset_before_updating = theConfig.ncp18_offset;

        calibration_offset = atof(ituTextBoxGetString(settingCalibrationOffsetTextBox));
        offset_update = (float)calibration_offset;
        printf("calibration_offset: %f\n", offset_update);

        if (offset_update <= 0)
        {
            if ((offset_update * -1) >= NCP_18_TOLERANCE)
            {
                printf("invalid calibration_offset(must be within the range of %d to %d)\n", -1 * NCP_18_TOLERANCE, NCP_18_TOLERANCE);
                return true;
            }
        }
        else
        {
            if (offset_update >= NCP_18_TOLERANCE)
            {
                printf("invalid calibration_offset(must be within the range of %d to %d)\n", -1 * NCP_18_TOLERANCE, NCP_18_TOLERANCE);
                return true;
            }
        }

        if (offset_before_updating != offset_update)
        {
            theConfig.ncp18_offset = offset_update;
            ConfigSave();
            refresh_time = 0;
        }

        is_set_calibration_temp = 0; //-1: user did not set any value, 0: user sets temp offset, 1: user sets calibration tmp
	}
	else if (NULL == ituTextBoxGetString(settingCalibrationOffsetTextBox) && NULL == ituTextBoxGetString(settingCalibrationTempTextBox))
	{
		is_set_calibration_temp = -1;
	}
	#endif

	return true;
}

bool SettingOnTimer(ITUWidget* widget, char* param)
{
    bool ret = false;

#ifdef CFG_NCP18_ENABLE
    if (tick_first)
    {
        tick = refresh_time;
        tick_first = false;
    }
    else
    {
        tick = SDL_GetTicks();
    }

    if (tick - lasttick >= refresh_time)
    {
        current_tmp_float = NCP18_Detect();
        if (current_tmp_float > 0)
        {
            current_tmp = (double)((int)(current_tmp_float * 10 + 0.5) / 10.0);
        }
        else
        {
            current_tmp = (double)((int)(-current_tmp_float * 10 + 0.5) / 10.0);
            current_tmp = -current_tmp;
        }

        lasttick = tick;

        if (refresh_time == 0) refresh_time = NCP18_REFRESH_TIME;
    }

    if (pre_tmp != current_tmp)
    {
        current_tmp_point = (int)(current_tmp * 10.0) - (int)current_tmp * 10;
        memset(temperature, 0x0, sizeof(temperature));

        if (theConfig.ncp18_offset >= 0)
            sprintf(temperature, "%d.%d (+%.2f)", (int)current_tmp, current_tmp_point, theConfig.ncp18_offset);
        else
            sprintf(temperature, "%d.%d (%.2f)", (int)current_tmp, current_tmp_point, theConfig.ncp18_offset);

        ituTextSetString(settingCurrentTmpText, temperature);
        pre_tmp = current_tmp;

        ret = true;
    }
#endif

    return ret;
}



bool OTA_http(ITUWidget* widget, char* param)
{
	bool network_is_ready = false;

#ifdef CFG_NET_ENABLE

#ifdef CFG_NET_ETHERNET
	network_is_ready = NetworkIsReady();
#elif CFG_NET_WIFI
	network_is_ready = (bool)ioctl(ITP_DEVICE_WIFI_NGPL, ITP_IOCTL_IS_AVAIL, NULL);
#endif

	if (network_is_ready)
	{
		UpgradeSetUrl(CFG_UPGRADE_FTP_URL);
		SceneQuit(QUIT_UPGRADE_FIRMWARE);
	}
#else
	UpgradeSetUrl(NULL);
#endif
	return true;
}