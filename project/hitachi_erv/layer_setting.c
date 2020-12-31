#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"
#include "wifiMgr.h"
#include "math.h"

#define STOPANYWHERE_H 496


static ITUStopAnywhere* settingStopAnywhere = 0;
static ITUCheckBox* settingWarningLightCheckBox = 0;

static ITUText* settingWiFiSsidNameText = 0;
static ITUStopAnywhere* settingWiFiStopAnywhere = 0;
static ITUBackground* settingWiFiAllBackground = 0;
//static ITUBackground* settingWiFiSubBackground0 = 0;
//static ITUButton* settingWiFiSubButton0 = 0;
//static ITUText* settingWiFiSsidNameSubText0 = 0;
//static ITUSprite* settingWiFiConnectSprite0 = 0;
//static ITUSprite* settingWiFiSignalSprite0 = 0;
static ITUCheckBox* settingWiFiOpenCheckBox = 0;

static ITUBackground* settingWiFiTmpBackground = 0;

static ITUWheel* settingTimeYearWheel = 0;
static ITUWheel* settingTimeMonthWheel = 0;
static ITUWheel* settingTimeDayWheel = 0;
static ITUWheel* settingTimeHrWheel = 0;
static ITUWheel* settingTimeMinWheel = 0;
static ITUText* settingTimeText = 0;
static ITUCheckBox* settingTimeAutoCheckBox = 0;
static ITUContainer* settingTimeManualContainer = 0;
static ITUContainer* settingTimeSyncContainer = 0;
static ITUText* settingTimeSyncYearText = 0;
static ITUText* settingTimeSyncMonthText = 0;
static ITUText* settingTimeSyncDayText = 0;
static ITUText* settingTimeSyncHourText = 0;
static ITUText* settingTimeSyncMinText = 0;

//static ITUCheckBox* settingLightAutoCheckBox = 0;

//static ITUProgressBar* settingScreenLightProgressBar = 0;
//static ITUTrackBar* settingScreenLightTrackBar = 0;
//static ITUProgressBar* settingIndLightProgressBar = 0;
//static ITUTrackBar* settingIndLightTrackBar = 0;
//static ITUIcon* settingLightTopBarBtnIcon[2] = { 0 };

static ITUSprite* settingScreenLockTimeSprite = 0;
static ITURadioBox* settingScreenLockRadioBox[3] = { 0 };

static ITUBackground* settingExamineBackground = 0;
static ITUBackground* settingExaminePasswordBackground = 0;
static ITUSprite* settingExaminePasswordCorrectSprite = 0;
static ITUIcon* settingExaminePasswordIcon[4] = { 0 };
static ITUIcon* settingExaminePasswordEmptyIcon = 0;
static ITUIcon* settingExaminePasswordEnterIcon = 0;
static ITUIcon* settingExaminePasswordUncorrectIcon = 0;

static ITUCheckBox* settingKeySoundCheckBox = 0;

extern bool memoryTimeAutoCheckbox = false;
extern bool keySound = true;

static int passwordCnt = 0;
static int password = 0;

//wifi
static bool settingWiFiReturnLayer = false;
static WIFI_MGR_SCANAP_LIST pList[64];
static int                  gnApCount = 0;

#if defined(CFG_NET_WIFI_SDIO_NGPL)
extern uint8_t* LwIP_GetIP(struct netif *pnetif);
extern uint8_t* LwIP_GetMAC(struct netif *pnetif);
extern struct netif xnetif[NET_IF_NUM];
#endif

#define MAX_SSID_COUNT 15 // (sizeof(settingWiFiSsidNameArray) / sizeof(settingWiFiSsidNameArray[0]))

typedef struct
{
	ITUBackground* background;
	ITUButton* button;
	ITUText* ssidtext;
	ITUSprite* connectSprite;
	ITUSprite* signalSprite;

} WIFISet;

WIFISet wifiSet[MAX_SSID_COUNT];

static int wifiNum = 0;
static int wifiAllBackgroundHeight = 88;
void showWiFiItem(void);
extern bool wifi_connected = false;
//

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
	struct timeval tv;
	struct tm *tm;
	char tmp[32];
	int i;

	//wifi
	int nRet;
	int nWiFiConnState = 0, nWiFiConnEcode = 0;
	char buf[32];
#ifdef CFG_NET_WIFI_SDIO_NGPL
	unsigned char *mac = LwIP_GetMAC(&xnetif[0]);
	unsigned char *ip = LwIP_GetIP(&xnetif[0]);
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
		if (nWiFiConnEcode == WIFIMGR_ECODE_OK
			|| nWiFiConnEcode == WIFIMGR_ECODE_SET_DISCONNECT
			|| nWiFiConnEcode == -1){
			//Check WIFI connection state from wifimgr
			if (nWiFiConnState == WIFIMGR_CONNSTATE_STOP){
				printf("SettingOnEnter press setting button, wait ....................\n");
				settingWiFiReturnLayer = false;
				//gnApCount = wifiMgr_get_scan_ap_info(pList);
			}
			else if (nWiFiConnState == WIFIMGR_CONNSTATE_CONNECTING){
				printf("SettingOnEnter connecting ,wait ....................\n");
				if (strcmp(pList[0].ssidName, "") == 0)
					gnApCount = wifiMgr_get_scan_ap_info(pList); //If device can not get DHCP before set power-off, get new list again while power-on

				settingWiFiReturnLayer = true;
				usleep(500000);
			}
			else if (nWiFiConnState == WIFIMGR_CONNSTATE_SCANNING){
				printf("SettingOnEnter scanning ,wait ....................\n");
				settingWiFiReturnLayer = true;
				usleep(500000);
			}
		}
		else{
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
	//

	if (!settingStopAnywhere)
	{
		settingStopAnywhere = ituSceneFindWidget(&theScene, "settingStopAnywhere");
		assert(settingStopAnywhere);

		settingTimeYearWheel = ituSceneFindWidget(&theScene, "settingTimeYearWheel");
		assert(settingTimeYearWheel);

		settingTimeMonthWheel = ituSceneFindWidget(&theScene, "settingTimeMonthWheel");
		assert(settingTimeMonthWheel);

		settingTimeDayWheel = ituSceneFindWidget(&theScene, "settingTimeDayWheel");
		assert(settingTimeDayWheel);

		settingTimeHrWheel = ituSceneFindWidget(&theScene, "settingTimeHrWheel");
		assert(settingTimeHrWheel);

		settingTimeMinWheel = ituSceneFindWidget(&theScene, "settingTimeMinWheel");
		assert(settingTimeMinWheel);

		settingTimeText = ituSceneFindWidget(&theScene, "settingTimeText");
		assert(settingTimeText);

		settingTimeAutoCheckBox = ituSceneFindWidget(&theScene, "settingTimeAutoCheckBox");
		assert(settingTimeAutoCheckBox);

		settingTimeManualContainer = ituSceneFindWidget(&theScene, "settingTimeManualContainer");
		assert(settingTimeManualContainer);

		settingTimeSyncContainer = ituSceneFindWidget(&theScene, "settingTimeSyncContainer");
		assert(settingTimeSyncContainer);

		settingTimeSyncYearText = ituSceneFindWidget(&theScene, "settingTimeSyncYearText");
		assert(settingTimeSyncYearText);

		settingTimeSyncMonthText = ituSceneFindWidget(&theScene, "settingTimeSyncMonthText");
		assert(settingTimeSyncMonthText);

		settingTimeSyncDayText = ituSceneFindWidget(&theScene, "settingTimeSyncDayText");
		assert(settingTimeSyncDayText);

		settingTimeSyncHourText = ituSceneFindWidget(&theScene, "settingTimeSyncHourText");
		assert(settingTimeSyncHourText);

		settingTimeSyncMinText = ituSceneFindWidget(&theScene, "settingTimeSyncMinText");
		assert(settingTimeSyncMinText);
		//settingLightAutoCheckBox = ituSceneFindWidget(&theScene, "settingLightAutoCheckBox");
		//assert(settingLightAutoCheckBox);

		//settingScreenLightProgressBar = ituSceneFindWidget(&theScene, "settingScreenLightProgressBar");
		//assert(settingScreenLightProgressBar);

		//settingScreenLightTrackBar = ituSceneFindWidget(&theScene, "settingScreenLightTrackBar");
		//assert(settingScreenLightTrackBar);

		//settingIndLightProgressBar = ituSceneFindWidget(&theScene, "settingIndLightProgressBar");
		//assert(settingIndLightProgressBar);

		//settingIndLightTrackBar = ituSceneFindWidget(&theScene, "settingIndLightTrackBar");
		//assert(settingIndLightTrackBar);

		settingScreenLockTimeSprite = ituSceneFindWidget(&theScene, "settingScreenLockTimeSprite");
		assert(settingScreenLockTimeSprite);
		/*
		settingWiFiSsidNameText = ituSceneFindWidget(&theScene, "settingWiFiSsidNameText");
		assert(settingWiFiSsidNameText);
		
		settingWiFiStopAnywhere = ituSceneFindWidget(&theScene, "settingWiFiStopAnywhere");
		assert(settingWiFiStopAnywhere);

		settingWiFiAllBackground = ituSceneFindWidget(&theScene, "settingWiFiAllBackground");
		assert(settingWiFiAllBackground);

		settingWiFiTmpBackground = ituSceneFindWidget(&theScene, "settingWiFiTmpBackground");
		assert(settingWiFiTmpBackground);
		
		wifiSet[0].background  = ituSceneFindWidget(&theScene, "settingWiFiSubBackground0");
		assert(wifiSet[0].background);

		wifiSet[0].button = ituSceneFindWidget(&theScene, "settingWiFiSubButton0");
		assert(wifiSet[0].button);

		wifiSet[0].ssidtext = ituSceneFindWidget(&theScene, "settingWiFiSsidNameSubText0");
		assert(wifiSet[0].ssidtext);

		wifiSet[0].connectSprite = ituSceneFindWidget(&theScene, "settingWiFiConnectSprite0");
		assert(wifiSet[0].connectSprite);

		wifiSet[0].signalSprite = ituSceneFindWidget(&theScene, "settingWiFiSignalSprite0");
		assert(wifiSet[0].signalSprite);

		settingWiFiOpenCheckBox = ituSceneFindWidget(&theScene, "settingWiFiOpenCheckBox");
		assert(settingWiFiOpenCheckBox);
		*/
		settingWarningLightCheckBox = ituSceneFindWidget(&theScene, "settingWarningLightCheckBox");
		assert(settingWarningLightCheckBox);

		settingExamineBackground = ituSceneFindWidget(&theScene, "settingExamineBackground");
		assert(settingExamineBackground);

		settingExaminePasswordBackground = ituSceneFindWidget(&theScene, "settingExaminePasswordBackground");
		assert(settingExaminePasswordBackground);

		settingExaminePasswordCorrectSprite = ituSceneFindWidget(&theScene, "settingExaminePasswordCorrectSprite");
		assert(settingExaminePasswordCorrectSprite);

		settingExaminePasswordEmptyIcon = ituSceneFindWidget(&theScene, "settingExaminePasswordEmptyIcon");
		assert(settingExaminePasswordEmptyIcon);

		settingExaminePasswordEnterIcon = ituSceneFindWidget(&theScene, "settingExaminePasswordEnterIcon");
		assert(settingExaminePasswordEnterIcon);

		settingExaminePasswordUncorrectIcon = ituSceneFindWidget(&theScene, "settingExaminePasswordUncorrectIcon");
		assert(settingExaminePasswordUncorrectIcon);

		settingKeySoundCheckBox = ituSceneFindWidget(&theScene, "settingKeySoundCheckBox");
		assert(settingKeySoundCheckBox);


		//for (i = 0; i < 2; i++)
		//{
		//	sprintf(tmp, "settingLightTopBarBtnIcon%d", i);
		//	settingLightTopBarBtnIcon[i] = ituSceneFindWidget(&theScene, tmp);
		//	assert(settingLightTopBarBtnIcon[i]);
		//}

		for (i = 0; i < 3; i++)
		{
			sprintf(tmp, "settingScreenLockRadioBox%d", i);
			settingScreenLockRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(settingScreenLockRadioBox[i]);

		}

		for (i = 0; i < 4; i++)
		{
			sprintf(tmp, "settingExaminePasswordIcon%d", i);
			settingExaminePasswordIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(settingExaminePasswordIcon[i]);

		}
	}

	
	if (memoryTimeAutoCheckbox)
	//if (ituCheckBoxIsChecked(settingTimeAutoCheckBox))
	{
		ituCheckBoxSetChecked(settingTimeAutoCheckBox, true);
		sprintf(tmp, "%s-%s-%s %s:%s", ituTextGetString(settingTimeSyncYearText),
			ituTextGetString(settingTimeSyncMonthText),
			ituTextGetString(settingTimeSyncDayText),
			ituTextGetString(settingTimeSyncHourText),
			ituTextGetString(settingTimeSyncMinText));
		ituTextSetString(settingTimeText, tmp);
	}
	else
	{
		ituCheckBoxSetChecked(settingTimeAutoCheckBox, false);
	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);
	sprintf(tmp, "%04d-%02d-%02d %02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min);
	ituTextSetString(settingTimeText, tmp);
	}

	for (i = 0; i < 3; i++)
	{
		ituRadioBoxSetChecked(settingScreenLockRadioBox[i], false);
	}
	if (theConfig.screensaver_time == 60)
	{
		ituRadioBoxSetChecked(settingScreenLockRadioBox[0], true);
		ituSpriteGoto(settingScreenLockTimeSprite, 0);
	}
	else if (theConfig.screensaver_time == 180)
	{
		ituRadioBoxSetChecked(settingScreenLockRadioBox[1], true);
		ituSpriteGoto(settingScreenLockTimeSprite, 1);
	}
	else
	{
		ituRadioBoxSetChecked(settingScreenLockRadioBox[2], true);
		ituSpriteGoto(settingScreenLockTimeSprite, 2);
	}

	ituCheckBoxSetChecked(settingKeySoundCheckBox, keySound);
	ituCheckBoxSetChecked(settingWarningLightCheckBox, indicatorLightEnable);
#if 0
//wifi
	/* Auto reconnection after system reboot, check IP/MAC first time */
	//if (theConfig.wifi_on_off == WIFIMGR_SWITCH_ON) {
	//	ituCheckBoxSetChecked(settingWiFiOpenCheckBox, true);
	//	ituWidgetSetVisible(settingWiFiAllBackground, true);
		//ituRadioBoxSetChecked(settingCloseWiFiRadioBox, false);
		//ituRadioBoxSetChecked(settingOpenWiFiRadioBox, true);
		//ituWidgetSetVisible(settingWiFiSsidContainer, true);
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

			//ituTextSetString(settingMACAddressText, buf);
			//ituTextSetString(settingIPText, ipaddr_ntoa(&xnetif[0].ip_addr));

		}
//		else {
//			ituTextSetString(settingIPText, "0.0.0.0");
//		}
#endif
#endif
	}
	else {
		ituCheckBoxSetChecked(settingWiFiOpenCheckBox, false);
		ituWidgetSetVisible(settingWiFiAllBackground, false);
		//ituRadioBoxSetChecked(settingOpenWiFiRadioBox, false);
		//ituRadioBoxSetChecked(settingCloseWiFiRadioBox, true);
		//ituWidgetSetVisible(settingWiFiSsidContainer, false);
		//ituTextSetString(settingIPText, "0.0.0.0");
	}




	if (theConfig.wifi_on_off == WIFIMGR_SWITCH_ON)
	//if (ituRadioBoxIsChecked(settingOpenWiFiRadioBox))
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

		//ituTextSetString(settingIPText, ipaddr_ntoa(&xnetif[0].ip_addr)); //set ip address string
		//ituTextSetString(settingMACAddressText, buf); //set mac address string
#endif
		//ituListBoxReload((ITUListBox*)settingWiFiSsidNameScrollListBox);
		//ituListBoxReload((ITUListBox*)settingWiFiSsidStatusScrollListBox);
		//ituListBoxReload((ITUListBox*)settingWiFiSsidSignalScrollListBox);

	}
	else //if (ituRadioBoxIsChecked(settingCloseWiFiRadioBox))
	{
		//close WiFi

		//ituTextSetString(settingIPText, ""); //set ip address string
		//ituTextSetString(settingMACAddressText, "FF:FF:FF:FF:FF:FF"); //set mac address string

		memset(theConfig.ssid, 0, sizeof(theConfig.ssid));
		memset(theConfig.password, 0, sizeof(theConfig.password));
	}

#ifdef CFG_NET_WIFI_SDIO_NGPL
	if (ioctl(ITP_DEVICE_WIFI_NGPL, ITP_IOCTL_IS_AVAIL, NULL)){
		if (ip[0] != 0)
			//ituSpriteGoto(topWiFiSprite, 2);
			wifi_status = 0;
		else
			//ituSpriteGoto(topWiFiSprite, 1);
			wifi_status = 1;
	}
	else
#endif
	{
		//ituSpriteGoto(topWiFiSprite, 0);
		wifi_status = 2;
	}



	if (gnApCount > 0)
	{
		//if (wifiNum == 0)
		//{
		//	wifiNum = 1;
		if (wifi_connected)
		{
			sprintf(buf, "%s", theConfig.ssid);
			ituTextSetString(settingWiFiSsidNameText, buf);
		}
		else
		{
			sprintf(buf, "%s", pList[0].ssidName);
			ituTextSetString(settingWiFiSsidNameText, buf);
			ituTextSetString(wifiSet[0].ssidtext, buf);

			if (pList[0].rfQualityQuant < 10)
			{
				ituSpriteGoto(wifiSet[0].signalSprite, 0);
			}
			else if (pList[0].rfQualityQuant < 20)
			{
				ituSpriteGoto(wifiSet[0].signalSprite, 1);
			}
			else if (pList[0].rfQualityQuant < 30)
			{
				ituSpriteGoto(wifiSet[0].signalSprite, 2);
			}
			else
			{
				ituSpriteGoto(wifiSet[0].signalSprite, 3);
			}

			ituWidgetSetVisible(wifiSet[0].background, true);
		}
			

			
			
		//}
		
	}
	else
	{
		ituWidgetSetVisible(wifiSet[0].background, false);
		//wifiNum = 0;
	}
//
#endif 
	return true;
}
bool SettingTimeBtnOnMouseUp(ITUWidget* widget, char* param)
{
	if (ituCheckBoxIsChecked(settingTimeAutoCheckBox))
	{
		//memoryTimeAutoCheckbox = true;
		ituWidgetSetVisible(settingTimeManualContainer, false);
		ituWidgetSetVisible(settingTimeSyncContainer, true);
		//sync internet time
	}
	else
	{
		//memoryTimeAutoCheckbox = false;
		ituWidgetSetVisible(settingTimeManualContainer, true);
		ituWidgetSetVisible(settingTimeSyncContainer, false);

	struct timeval tv;
	struct tm *tm;

	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	ituWheelGoto(settingTimeYearWheel, tm->tm_year - 120);//2020-1900
	ituWheelGoto(settingTimeMonthWheel, tm->tm_mon);//0-11
	ituWheelGoto(settingTimeDayWheel, tm->tm_mday-1);//1-31
	ituWheelGoto(settingTimeHrWheel, tm->tm_hour);
	ituWheelGoto(settingTimeMinWheel, tm->tm_min);
	}

	

	settingStopAnywhere->widget.flags &= ~ITU_DRAGGABLE;

	return true;
}

bool SettingTimeWheelOnChanged(ITUWidget* widget, char* param)
{
	if ((settingTimeYearWheel->focusIndex % 4) == 0)
	{
		if (settingTimeMonthWheel->focusIndex == 1)
		{
			if (settingTimeDayWheel->focusIndex > 28)
				ituWheelGoto(settingTimeDayWheel, 28);
		}
	}

	else
	{
		if (settingTimeMonthWheel->focusIndex == 1)
		{
			if (settingTimeDayWheel->focusIndex > 27)
				ituWheelGoto(settingTimeDayWheel, 27);
		}
	}


	if ((settingTimeMonthWheel->focusIndex == 3) || (settingTimeMonthWheel->focusIndex == 5) || (settingTimeMonthWheel->focusIndex == 8) || (settingTimeMonthWheel->focusIndex == 10))
	{
		if (settingTimeDayWheel->focusIndex > 29)
			ituWheelGoto(settingTimeDayWheel, 29);
	}
	return true;
}

bool SettingTimeSaveBtnOnPress(ITUWidget* widget, char* param)
{
	bool save = atoi(param);
	char tmp[32];

	if (save)
	{
		if (ituCheckBoxIsChecked(settingTimeAutoCheckBox))
		{
			memoryTimeAutoCheckbox = true;
			sprintf(tmp, "%s-%s-%s %s:%s", ituTextGetString(settingTimeSyncYearText),
				ituTextGetString(settingTimeSyncMonthText),
				ituTextGetString(settingTimeSyncDayText),
				ituTextGetString(settingTimeSyncHourText),
				ituTextGetString(settingTimeSyncMinText));
			ituTextSetString(settingTimeText, tmp);
		}
		else
		{
			memoryTimeAutoCheckbox = false;
		struct timeval tv;
		struct tm *tm, mytime;


		if ((settingTimeYearWheel->focusIndex % 4) == 0)
		{
			if (settingTimeMonthWheel->focusIndex == 1)
			{
				if (settingTimeDayWheel->focusIndex > 28)
					ituWheelGoto(settingTimeDayWheel, 28);
			}
		}

		else
		{
			if (settingTimeMonthWheel->focusIndex == 1)
			{
				if (settingTimeDayWheel->focusIndex > 27)
					ituWheelGoto(settingTimeDayWheel, 27);
			}
		}


		if ((settingTimeMonthWheel->focusIndex == 3) || (settingTimeMonthWheel->focusIndex == 5) || (settingTimeMonthWheel->focusIndex == 8) || (settingTimeMonthWheel->focusIndex == 10))
		{
			if (settingTimeDayWheel->focusIndex > 29)
				ituWheelGoto(settingTimeDayWheel, 29);
		}

		sprintf(tmp, "%04d-%02d-%02d %02d:%02d", settingTimeYearWheel->focusIndex + 2020,
			settingTimeMonthWheel->focusIndex + 1,
			settingTimeDayWheel->focusIndex + 1,
			settingTimeHrWheel->focusIndex,
			settingTimeMinWheel->focusIndex);
		ituTextSetString(settingTimeText, tmp);

		//set time
		gettimeofday(&tv, NULL);
		tm = localtime(&tv.tv_sec);

		memcpy(&mytime, tm, sizeof (struct tm));

		mytime.tm_year = settingTimeYearWheel->focusIndex + 120;
		mytime.tm_mon = settingTimeMonthWheel->focusIndex;
		mytime.tm_mday = settingTimeDayWheel->focusIndex + 1;
		mytime.tm_hour = settingTimeHrWheel->focusIndex;
		mytime.tm_min = settingTimeMinWheel->focusIndex;


		tv.tv_sec = mktime(&mytime);
		tv.tv_usec = 0;

		settimeofday(&tv, NULL);
	}

	}
	else
	{
		ituCheckBoxSetChecked(settingTimeAutoCheckBox, memoryTimeAutoCheckbox);
	}

	settingStopAnywhere->widget.flags |= ITU_DRAGGABLE;

    return true;
}
bool SettingTimeAutoChkBoxOnPress(ITUWidget* widget, char* param)
{
	if (ituCheckBoxIsChecked(settingTimeAutoCheckBox))
	{
		ituWidgetSetVisible(settingTimeManualContainer, false);
		ituWidgetSetVisible(settingTimeSyncContainer, true);
		//sync internet time
	}
	else
	{
		ituWidgetSetVisible(settingTimeManualContainer, true);
		ituWidgetSetVisible(settingTimeSyncContainer, false);

		struct timeval tv;
		struct tm *tm;

		gettimeofday(&tv, NULL);
		tm = localtime(&tv.tv_sec);

		ituWheelGoto(settingTimeYearWheel, tm->tm_year - 120);//2020-1900
		ituWheelGoto(settingTimeMonthWheel, tm->tm_mon);//0-11
		ituWheelGoto(settingTimeDayWheel, tm->tm_mday - 1);//1-31
		ituWheelGoto(settingTimeHrWheel, tm->tm_hour);
		ituWheelGoto(settingTimeMinWheel, tm->tm_min);
	}
	return true;
}

//
//bool SettingLightBtnOnMouseUp(ITUWidget* widget, char* param)
//{
//
//	if (ituCheckBoxIsChecked(settingLightAutoCheckBox))
//	{
//		ituIconLinkSurface(&settingScreenLightTrackBar->tracker->bg.icon, settingLightTopBarBtnIcon[0]);
//		ituIconLinkSurface(&settingIndLightTrackBar->tracker->bg.icon, settingLightTopBarBtnIcon[0]);
//
//		ituWidgetDisable(settingScreenLightTrackBar);
//		ituWidgetSetVisible(settingScreenLightProgressBar, false);
//		ituWidgetDisable(settingIndLightTrackBar);
//		ituWidgetSetVisible(settingIndLightProgressBar, false);
//	}
//	else
//	{
//		ituIconLinkSurface(&settingScreenLightTrackBar->tracker->bg.icon, settingLightTopBarBtnIcon[1]);
//		ituIconLinkSurface(&settingIndLightTrackBar->tracker->bg.icon, settingLightTopBarBtnIcon[1]);
//
//		ituWidgetEnable(settingScreenLightTrackBar);
//		ituWidgetSetVisible(settingScreenLightProgressBar, true);
//		ituWidgetEnable(settingIndLightTrackBar);
//		ituWidgetSetVisible(settingIndLightProgressBar, true);
//	}
//
//
//	settingStopAnywhere->widget.flags &= ~ITU_DRAGGABLE;
//
//    return true;
//}
//
//bool SettingLightBtnOnSlideUp(ITUWidget* widget, char* param)
//{
//	settingStopAnywhere->widget.flags |= ITU_DRAGGABLE;
//
//	return true;
//}

bool SettingScreenLockBtnOnMouseUp(ITUWidget* widget, char* param)
{
	ituRadioBoxSetChecked(settingScreenLockRadioBox[0], false);
	ituRadioBoxSetChecked(settingScreenLockRadioBox[1], false);
	ituRadioBoxSetChecked(settingScreenLockRadioBox[2], false);

	ituRadioBoxSetChecked(settingScreenLockRadioBox[settingScreenLockTimeSprite->frame], true);

	settingStopAnywhere->widget.flags &= ~ITU_DRAGGABLE;
	return true;
}

bool SettingScreenLockSaveBtnOnPress(ITUWidget* widget, char* param)
{
	bool save = atoi(param);

	if (save)
	{
		if (ituRadioBoxIsChecked(settingScreenLockRadioBox[0]))
		{
			ituSpriteGoto(settingScreenLockTimeSprite, 0);
			theConfig.screensaver_time = 60;
		}
		else if (ituRadioBoxIsChecked(settingScreenLockRadioBox[1]))
		{
			ituSpriteGoto(settingScreenLockTimeSprite, 1);
			theConfig.screensaver_time = 180;
		}
		else
		{
			ituSpriteGoto(settingScreenLockTimeSprite, 2);
			theConfig.screensaver_time = 600;
		}
		ConfigSave();
			
	}
	

	settingStopAnywhere->widget.flags |= ITU_DRAGGABLE;
	return true;
}

bool SettingScreenLockRadBoxOnPress(ITUWidget* widget, char* param)
{
	int radioboxIndex = atoi(param);

	switch (radioboxIndex)
	{
	case 0:
		ituRadioBoxSetChecked(settingScreenLockRadioBox[0], true);
		ituRadioBoxSetChecked(settingScreenLockRadioBox[1], false);
		ituRadioBoxSetChecked(settingScreenLockRadioBox[2], false);
		break;
	case 1:
		ituRadioBoxSetChecked(settingScreenLockRadioBox[0], false);
		ituRadioBoxSetChecked(settingScreenLockRadioBox[1], true);
		ituRadioBoxSetChecked(settingScreenLockRadioBox[2], false);
		break;
	case 2:
		ituRadioBoxSetChecked(settingScreenLockRadioBox[0], false);
		ituRadioBoxSetChecked(settingScreenLockRadioBox[1], false);
		ituRadioBoxSetChecked(settingScreenLockRadioBox[2], true);
		break;
	}

	return true;
}
bool SettingWarningLightCheckBoxOnPress(ITUWidget* widget, char* param)
{
	if (ituCheckBoxIsChecked(settingWarningLightCheckBox))
	{
		indicatorLightEnable = true;
	}
	else
	{
		indicatorLightEnable = false;
	}
	return true;
}
bool SettingWiFiBtnOnMouseUp(ITUWidget* widget, char* param)
{
	if (theConfig.wifi_on_off == WIFIMGR_SWITCH_ON)
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

		showWiFiItem();
	}

//	int	i;
//	int nRet;
//	int nConnect = 0;
//	int bIsAvail = 0;
//
//#ifdef CFG_NET_WIFI
//	nRet = wifiMgr_is_wifi_available(&bIsAvail);
//#endif
//	printf("SettingWiFiBtnOnMouseUp,    IsAvail = %d,   Max Ssid Count = %d,   switch status = %d  \n", bIsAvail, getMaxSsidCount(), theConfig.wifi_on_off);
//	for (i = 0; i < gnApCount; i++)
//	{
//		if (!strncmp(theConfig.ssid, pList[i].ssidName, strlen(pList[i].ssidName)))
//		{
//			if (bIsAvail && nConnect == 0)
//			{
//#ifdef CFG_NET_WIFI_SDIO_NGPL
//				if (settingWiFiReturnLayer){
//					ituSpriteGoto(wifiSet[i].connectSprite,0);
//					return true;
//				}
//#endif
//				//if (language == LANG_CHT)
//				//	ituScrollTextSetString(scrolltext, (char *)StringGetWiFiConnected());
//				//else
//				//	ituScrollTextSetString(scrolltext, (char *)StringGetWiFiConnectedEng());
//
//				ituSpriteGoto(wifiSet[i].connectSprite, 1);
//
//				printf("SsidStatus %s \n", theConfig.ssid);
//				nConnect++;
//			}
//			else
//			{
//				ituSpriteGoto(wifiSet[i].connectSprite, 0);
//			}
//		}
//		else
//		{
//			ituSpriteGoto(wifiSet[i].connectSprite, 0);
//		}
//
//		printf("bIsAvail %d SsidStatus %s  nConnect %d \n", bIsAvail, theConfig.ssid, nConnect);
//	}


	return true;
}
#ifdef CFG_NET_WIFI_SDIO_NGPL
extern int SettingWiFiPasswordSetData(char *ssid, unsigned long securityMode);
#else
extern int SettingWiFiPasswordSetData(char *ssid, int securityMode);
#endif
bool SettingWiFiSubBtnOnPress(ITUWidget* widget, char* param)
{

	return true;
}

bool SettingWiFiOpenCheckBoxOnPress(ITUWidget* widget, char* param)
{

	return true;
}
bool SettingRestoreAllSettingBtnOnMouseUp(ITUWidget* widget, char* param)
{
	//restore all setting, clear customer's content
	// Restore System Default
	SceneQuit(QUIT_RESET_FACTORY);

	return true;
}

bool SettingRestoreSettingBtnOnMouseUp(ITUWidget* widget, char* param)
{
	//restore setting but keep customer's content
	return true;
}

bool SettingExaminePasswordBtnOnMouseUp(ITUWidget* widget, char* param)
{
	int input = atoi(param);
	int i;

	if (input == 10)
	{
		ituSpriteGoto(settingExaminePasswordCorrectSprite, 0);
		if (passwordCnt != 0)
		{
			
			password = password - password % ((int)pow(10, 5 - passwordCnt));
			passwordCnt--;
			ituIconLinkSurface(settingExaminePasswordIcon[passwordCnt], settingExaminePasswordEmptyIcon);
			
		}
			
	}
	else
	{
		if (passwordCnt < 4)
		{
			for (i = 0; i < passwordCnt; i++)
			{
				ituIconLinkSurface(settingExaminePasswordIcon[i], settingExaminePasswordEnterIcon);
			}

			ituIconLinkSurface(settingExaminePasswordIcon[passwordCnt], settingExaminePasswordEnterIcon);
			password = input*pow(10, 3 - passwordCnt) + password;
			passwordCnt++;


		}

		if (passwordCnt == 4)
		{
			if (password == theConfig.examine_pw)
			{
				ituWidgetSetVisible(settingExamineBackground, true);
				ituWidgetSetVisible(settingExaminePasswordBackground, false);
			}
			else
			{
				for (i = 0; i < 4; i++)
				{
					ituIconLinkSurface(settingExaminePasswordIcon[i], settingExaminePasswordUncorrectIcon);
				}
				ituSpriteGoto(settingExaminePasswordCorrectSprite, 1);
			}
		}

		
	}
	return true;
}

bool SettingExamineButtonOnMouseUp(ITUWidget* widget, char* param)
{
	passwordCnt = 0;
	password = 0;
	for (int i = 0; i < 4; i++)
	{
		ituIconLinkSurface(settingExaminePasswordIcon[i], settingExaminePasswordEmptyIcon);
	}
	ituSpriteGoto(settingExaminePasswordCorrectSprite, 0);
	return true;
}

bool SettingKeySoundCheckBoxOnMouseUp(ITUWidget* widget, char* param)
{
	if (ituCheckBoxIsChecked(settingKeySoundCheckBox))
	{
		keySound = true;
	}
	else
	{
		keySound = false;
	}
	return true;
}
void showWiFiItem(void)
{
	int i;
	char buf[32];


	if (gnApCount >= 1)
	{
		sprintf(buf, "%s", pList[0].ssidName);
		ituTextSetString(settingWiFiSsidNameText, buf);
		ituTextSetString(wifiSet[0].ssidtext, buf);

		if (pList[0].rfQualityQuant < 10)
		{
			ituSpriteGoto(wifiSet[0].signalSprite, 0);
		}
		else if (pList[0].rfQualityQuant < 20)
		{
			ituSpriteGoto(wifiSet[0].signalSprite, 1);
		}
		else if (pList[0].rfQualityQuant < 30)
		{
			ituSpriteGoto(wifiSet[0].signalSprite, 2);
		}
		else
		{
			ituSpriteGoto(wifiSet[0].signalSprite, 3);
		}

		ituWidgetSetVisible(wifiSet[0].background, true);

		if (gnApCount>1)
		{
			for (i = 1; i < gnApCount; i++)
			{
				ITCTree* node = NULL;
				ITUBackground* cloneBackground = NULL;

				char id[8];

				ituBackgroundClone((ITUWidget*)settingWiFiTmpBackground, (ITUWidget**)&cloneBackground);
				wifiSet[i].background = cloneBackground;

				ITUWidget* widget = (ITUWidget*)cloneBackground;

				node = ((ITCTree*)widget)->child;
				ITUButton* button = (ITUButton*)node;
				wifiSet[i].button = button;
				sprintf(id, "%d", i);
				strcpy(button->actions[1].param, id);

				node = node->sibling;
				ITUText* ssidText = (ITUText*)node;
				wifiSet[i].ssidtext = ssidText;
				sprintf(buf, "%s", pList[i].ssidName);
				ituTextSetString(wifiSet[i].ssidtext, buf);


				node = node->sibling;
				ITUSprite* connectSprite = (ITUSprite*)node;
				wifiSet[i].connectSprite = connectSprite;

				node = node->sibling;
				ITUSprite* signalSprite = (ITUSprite*)node;
				wifiSet[i].signalSprite = signalSprite;
				if (pList[i].rfQualityQuant < 10)
				{
					ituSpriteGoto(wifiSet[i].signalSprite, 0);
				}
				else if (pList[i].rfQualityQuant < 20)
				{
					ituSpriteGoto(wifiSet[i].signalSprite, 1);
				}
				else if (pList[i].rfQualityQuant < 30)
				{
					ituSpriteGoto(wifiSet[i].signalSprite, 2);
				}
				else
				{
					ituSpriteGoto(wifiSet[i].signalSprite, 3);
				}

				ituWidgetSetVisible(wifiSet[i].background, true);

				ituWidgetAdd(settingWiFiAllBackground, wifiSet[i].background);
				ituWidgetSetPosition(wifiSet[i].background, 0, i * 94);
				ituWidgetSetDimension(settingWiFiAllBackground, 670, i * 94 + 88);
				wifiAllBackgroundHeight = ituWidgetGetHeight(settingWiFiAllBackground); // = weekTotalItem * 150 + 130;
				if (wifiAllBackgroundHeight > STOPANYWHERE_H)
					ituWidgetSetDimension(settingWiFiStopAnywhere, 670, STOPANYWHERE_H);
				else
					ituWidgetSetDimension(settingWiFiStopAnywhere, 670, wifiAllBackgroundHeight);

				ituWidgetUpdate(settingWiFiAllBackground, ITU_EVENT_LAYOUT, 0, 0, 0);// update the container widget


			}
		}

	}
	else
	{
		ituWidgetSetVisible(wifiSet[0].background, false);
	}

}