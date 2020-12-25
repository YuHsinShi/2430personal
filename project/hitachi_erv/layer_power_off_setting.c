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


static ITUStopAnywhere* powerOffSettingStopAnywhere = 0;
static ITUCheckBox* powerOffSettingWarningLightCheckBox = 0;

static ITUText* powerOffSettingWiFiSsidNameText = 0;
static ITUStopAnywhere* powerOffSettingWiFiStopAnywhere = 0;
static ITUBackground* powerOffSettingWiFiAllBackground = 0;
//static ITUBackground* powerOffSettingWiFiSubBackground0 = 0;
//static ITUButton* powerOffSettingWiFiSubButton0 = 0;
//static ITUText* powerOffSettingWiFiSsidNameSubText0 = 0;
//static ITUSprite* powerOffSettingWiFiConnectSprite0 = 0;
//static ITUSprite* powerOffSettingWiFiSignalSprite0 = 0;
static ITUCheckBox* powerOffSettingWiFiOpenCheckBox = 0;

static ITUBackground* powerOffSettingWiFiTmpBackground = 0;

static ITUWheel* powerOffSettingTimeYearWheel = 0;
static ITUWheel* powerOffSettingTimeMonthWheel = 0;
static ITUWheel* powerOffSettingTimeDayWheel = 0;
static ITUWheel* powerOffSettingTimeHrWheel = 0;
static ITUWheel* powerOffSettingTimeMinWheel = 0;
static ITUText* powerOffSettingTimeText = 0;
static ITUCheckBox* powerOffSettingTimeAutoCheckBox = 0;
static ITUContainer* powerOffSettingTimeManualContainer = 0;
static ITUContainer* powerOffSettingTimeSyncContainer = 0;
static ITUText* powerOffSettingTimeSyncYearText = 0;
static ITUText* powerOffSettingTimeSyncMonthText = 0;
static ITUText* powerOffSettingTimeSyncDayText = 0;
static ITUText* powerOffSettingTimeSyncHourText = 0;
static ITUText* powerOffSettingTimeSyncMinText = 0;

//static ITUCheckBox* powerOffSettingLightAutoCheckBox = 0;

//static ITUProgressBar* powerOffSettingScreenLightProgressBar = 0;
//static ITUTrackBar* powerOffSettingScreenLightTrackBar = 0;
//static ITUProgressBar* powerOffSettingIndLightProgressBar = 0;
//static ITUTrackBar* powerOffSettingIndLightTrackBar = 0;
//static ITUIcon* powerOffSettingLightTopBarBtnIcon[2] = { 0 };

//static ITUSprite* powerOffSettingScreenLockTimeSprite = 0;
//static ITURadioBox* powerOffSettingScreenLockRadioBox[3] = { 0 };

static ITUBackground* powerOffSettingExamineBackground = 0;
static ITUBackground* powerOffSettingExaminePasswordBackground = 0;
static ITUSprite* powerOffSettingExaminePasswordCorrectSprite = 0;
static ITUIcon* powerOffSettingExaminePasswordIcon[4] = { 0 };
static ITUIcon* powerOffSettingExaminePasswordEmptyIcon = 0;
static ITUIcon* powerOffSettingExaminePasswordEnterIcon = 0;
static ITUIcon* powerOffSettingExaminePasswordUncorrectIcon = 0;

static ITUCheckBox* powerOffSettingKeySoundCheckBox = 0;

static int powerOffPasswordCnt = 0;
static int powerOffPassword = 0;

//wifi
static bool powerOffSettingWiFiReturnLayer = false;
static WIFI_MGR_SCANAP_LIST powerOffPList[64];
static int                  powerOffGnApCount = 0;

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

WIFISet powerOffWifiSet[MAX_SSID_COUNT];

static int powerOffWifiNum = 0;
static int powerOffWifiAllBackgroundHeight = 88;
void powerOffShowWiFiItem(void);
//

static int getMaxSsidCount()
{
	if (powerOffGnApCount <= 0)
	{
		return MAX_SSID_COUNT;
	}
	else
	{
		return powerOffGnApCount;
	}
}

bool PowerOffSettingOnEnter(ITUWidget* widget, char* param)
{
	struct timeval tv;
	struct tm *tm;
	char tmp[64];
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
		printf("PowerOffSettingOnEnter connecting ,wait ....................\n");
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
				printf("PowerOffSettingOnEnter press setting button, wait ....................\n");
				powerOffSettingWiFiReturnLayer = false;
				//powerOffGnApCount = wifiMgr_get_scan_ap_info(powerOffPList);
			}
			else if (nWiFiConnState == WIFIMGR_CONNSTATE_CONNECTING){
				printf("PowerOffSettingOnEnter connecting ,wait ....................\n");
				if (strcmp(powerOffPList[0].ssidName, "") == 0)
					powerOffGnApCount = wifiMgr_get_scan_ap_info(powerOffPList); //If device can not get DHCP before set power-off, get new list again while power-on

				powerOffSettingWiFiReturnLayer = true;
				usleep(500000);
			}
			else if (nWiFiConnState == WIFIMGR_CONNSTATE_SCANNING){
				printf("PowerOffSettingOnEnter scanning ,wait ....................\n");
				powerOffSettingWiFiReturnLayer = true;
				usleep(500000);
			}
		}
		else{
			printf("PowerOffSettingOnEnter wifimgr is doing something(%d) ,wait ....................\n", nWiFiConnEcode);
			if (nWiFiConnEcode == WIFIMGR_ECODE_DHCP_ERROR)
				powerOffSettingWiFiReturnLayer = true;

			usleep(500000);
		}
#else
		powerOffGnApCount = wifiMgr_get_scan_ap_info(powerOffPList);
#endif
	}
	else
	{
		printf("Wifi is closed, no SSID list...\n");
	}
#endif
	//

	if (!powerOffSettingStopAnywhere)
	{
		powerOffSettingStopAnywhere = ituSceneFindWidget(&theScene, "powerOffSettingStopAnywhere");
		assert(powerOffSettingStopAnywhere);

		powerOffSettingTimeYearWheel = ituSceneFindWidget(&theScene, "powerOffSettingTimeYearWheel");
		assert(powerOffSettingTimeYearWheel);

		powerOffSettingTimeMonthWheel = ituSceneFindWidget(&theScene, "powerOffSettingTimeMonthWheel");
		assert(powerOffSettingTimeMonthWheel);

		powerOffSettingTimeDayWheel = ituSceneFindWidget(&theScene, "powerOffSettingTimeDayWheel");
		assert(powerOffSettingTimeDayWheel);

		powerOffSettingTimeHrWheel = ituSceneFindWidget(&theScene, "powerOffSettingTimeHrWheel");
		assert(powerOffSettingTimeHrWheel);

		powerOffSettingTimeMinWheel = ituSceneFindWidget(&theScene, "powerOffSettingTimeMinWheel");
		assert(powerOffSettingTimeMinWheel);

		powerOffSettingTimeText = ituSceneFindWidget(&theScene, "powerOffSettingTimeText");
		assert(powerOffSettingTimeText);

		powerOffSettingTimeAutoCheckBox = ituSceneFindWidget(&theScene, "powerOffSettingTimeAutoCheckBox");
		assert(powerOffSettingTimeAutoCheckBox);

		powerOffSettingTimeManualContainer = ituSceneFindWidget(&theScene, "powerOffSettingTimeManualContainer");
		assert(powerOffSettingTimeManualContainer);

		powerOffSettingTimeSyncContainer = ituSceneFindWidget(&theScene, "powerOffSettingTimeSyncContainer");
		assert(powerOffSettingTimeSyncContainer);

		powerOffSettingTimeSyncYearText = ituSceneFindWidget(&theScene, "powerOffSettingTimeSyncYearText");
		assert(powerOffSettingTimeSyncYearText);

		powerOffSettingTimeSyncMonthText = ituSceneFindWidget(&theScene, "powerOffSettingTimeSyncMonthText");
		assert(powerOffSettingTimeSyncMonthText);

		powerOffSettingTimeSyncDayText = ituSceneFindWidget(&theScene, "powerOffSettingTimeSyncDayText");
		assert(powerOffSettingTimeSyncDayText);

		powerOffSettingTimeSyncHourText = ituSceneFindWidget(&theScene, "powerOffSettingTimeSyncHourText");
		assert(powerOffSettingTimeSyncHourText);

		powerOffSettingTimeSyncMinText = ituSceneFindWidget(&theScene, "powerOffSettingTimeSyncMinText");
		assert(powerOffSettingTimeSyncMinText);

		//powerOffSettingLightAutoCheckBox = ituSceneFindWidget(&theScene, "powerOffSettingLightAutoCheckBox");
		//assert(powerOffSettingLightAutoCheckBox);

		//powerOffSettingScreenLightProgressBar = ituSceneFindWidget(&theScene, "powerOffSettingScreenLightProgressBar");
		//assert(powerOffSettingScreenLightProgressBar);

		//powerOffSettingScreenLightTrackBar = ituSceneFindWidget(&theScene, "powerOffSettingScreenLightTrackBar");
		//assert(powerOffSettingScreenLightTrackBar);

		//powerOffSettingIndLightProgressBar = ituSceneFindWidget(&theScene, "powerOffSettingIndLightProgressBar");
		//assert(powerOffSettingIndLightProgressBar);

		//powerOffSettingIndLightTrackBar = ituSceneFindWidget(&theScene, "powerOffSettingIndLightTrackBar");
		//assert(powerOffSettingIndLightTrackBar);

		//powerOffSettingScreenLockTimeSprite = ituSceneFindWidget(&theScene, "powerOffSettingScreenLockTimeSprite");
		//assert(powerOffSettingScreenLockTimeSprite);

		powerOffSettingWiFiSsidNameText = ituSceneFindWidget(&theScene, "powerOffSettingWiFiSsidNameText");
		assert(powerOffSettingWiFiSsidNameText);

		powerOffSettingWiFiStopAnywhere = ituSceneFindWidget(&theScene, "powerOffSettingWiFiStopAnywhere");
		assert(powerOffSettingWiFiStopAnywhere);

		powerOffSettingWiFiAllBackground = ituSceneFindWidget(&theScene, "powerOffSettingWiFiAllBackground");
		assert(powerOffSettingWiFiAllBackground);

		powerOffSettingWiFiTmpBackground = ituSceneFindWidget(&theScene, "powerOffSettingWiFiTmpBackground");
		assert(powerOffSettingWiFiTmpBackground);

		powerOffWifiSet[0].background = ituSceneFindWidget(&theScene, "powerOffSettingWiFiSubBackground0");
		assert(powerOffWifiSet[0].background);

		powerOffWifiSet[0].button = ituSceneFindWidget(&theScene, "powerOffSettingWiFiSubButton0");
		assert(powerOffWifiSet[0].button);

		powerOffWifiSet[0].ssidtext = ituSceneFindWidget(&theScene, "powerOffSettingWiFiSsidNameSubText0");
		assert(powerOffWifiSet[0].ssidtext);

		powerOffWifiSet[0].connectSprite = ituSceneFindWidget(&theScene, "powerOffSettingWiFiConnectSprite0");
		assert(powerOffWifiSet[0].connectSprite);

		powerOffWifiSet[0].signalSprite = ituSceneFindWidget(&theScene, "powerOffSettingWiFiSignalSprite0");
		assert(powerOffWifiSet[0].signalSprite);

		powerOffSettingWiFiOpenCheckBox = ituSceneFindWidget(&theScene, "powerOffSettingWiFiOpenCheckBox");
		assert(powerOffSettingWiFiOpenCheckBox);

		powerOffSettingWarningLightCheckBox = ituSceneFindWidget(&theScene, "powerOffSettingWarningLightCheckBox");
		assert(powerOffSettingWarningLightCheckBox);

		powerOffSettingExamineBackground = ituSceneFindWidget(&theScene, "powerOffSettingExamineBackground");
		assert(powerOffSettingExamineBackground);

		powerOffSettingExaminePasswordBackground = ituSceneFindWidget(&theScene, "powerOffSettingExaminePasswordBackground");
		assert(powerOffSettingExaminePasswordBackground);

		powerOffSettingExaminePasswordCorrectSprite = ituSceneFindWidget(&theScene, "powerOffSettingExaminePasswordCorrectSprite");
		assert(powerOffSettingExaminePasswordCorrectSprite);

		powerOffSettingExaminePasswordEmptyIcon = ituSceneFindWidget(&theScene, "powerOffSettingExaminePasswordEmptyIcon");
		assert(powerOffSettingExaminePasswordEmptyIcon);

		powerOffSettingExaminePasswordEnterIcon = ituSceneFindWidget(&theScene, "powerOffSettingExaminePasswordEnterIcon");
		assert(powerOffSettingExaminePasswordEnterIcon);

		powerOffSettingExaminePasswordUncorrectIcon = ituSceneFindWidget(&theScene, "powerOffSettingExaminePasswordUncorrectIcon");
		assert(powerOffSettingExaminePasswordUncorrectIcon);

		powerOffSettingKeySoundCheckBox = ituSceneFindWidget(&theScene, "powerOffSettingKeySoundCheckBox");
		assert(powerOffSettingKeySoundCheckBox);


		//for (i = 0; i < 2; i++)
		//{
		//	sprintf(tmp, "powerOffSettingLightTopBarBtnIcon%d", i);
		//	powerOffSettingLightTopBarBtnIcon[i] = ituSceneFindWidget(&theScene, tmp);
		//	assert(powerOffSettingLightTopBarBtnIcon[i]);
		//}

		//for (i = 0; i < 3; i++)
		//{
		//	sprintf(tmp, "powerOffSettingScreenLockRadioBox%d", i);
		//	powerOffSettingScreenLockRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
		//	assert(powerOffSettingScreenLockRadioBox[i]);

		//}

		for (i = 0; i < 4; i++)
		{
			sprintf(tmp, "powerOffSettingExaminePasswordIcon%d", i);
			powerOffSettingExaminePasswordIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffSettingExaminePasswordIcon[i]);

		}
	}

	if (memoryTimeAutoCheckbox)
	//if (ituCheckBoxIsChecked(powerOffSettingTimeAutoCheckBox))
	{
		ituCheckBoxSetChecked(powerOffSettingTimeAutoCheckBox, true);
		sprintf(tmp, "%s-%s-%s %s:%s", ituTextGetString(powerOffSettingTimeSyncYearText),
			ituTextGetString(powerOffSettingTimeSyncMonthText),
			ituTextGetString(powerOffSettingTimeSyncDayText),
			ituTextGetString(powerOffSettingTimeSyncHourText),
			ituTextGetString(powerOffSettingTimeSyncMinText));
		ituTextSetString(powerOffSettingTimeText, tmp);
	}
	else
	{
		ituCheckBoxSetChecked(powerOffSettingTimeAutoCheckBox, false);
	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	sprintf(tmp, "%04d-%02d-%02d %02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min);
	ituTextSetString(powerOffSettingTimeText, tmp);
	}

	

	//for (i = 0; i < 3; i++)
	//{
	//	ituRadioBoxSetChecked(powerOffSettingScreenLockRadioBox[i], false);
	//}
	//if (theConfig.screensaver_time == 60)
	//{
	//	ituRadioBoxSetChecked(powerOffSettingScreenLockRadioBox[0], true);
	//	ituSpriteGoto(powerOffSettingScreenLockTimeSprite, 0);
	//}
	//else if (theConfig.screensaver_time == 180)
	//{
	//	ituRadioBoxSetChecked(powerOffSettingScreenLockRadioBox[1], true);
	//	ituSpriteGoto(powerOffSettingScreenLockTimeSprite, 1);
	//}
	//else
	//{
	//	ituRadioBoxSetChecked(powerOffSettingScreenLockRadioBox[2], true);
	//	ituSpriteGoto(powerOffSettingScreenLockTimeSprite, 2);
	//}
	ituCheckBoxSetChecked(powerOffSettingKeySoundCheckBox, keySound);
	ituCheckBoxSetChecked(powerOffSettingWarningLightCheckBox, indicatorLightEnable);
//wifi
	/* Auto reconnection after system reboot, check IP/MAC first time */
	if (theConfig.wifi_on_off == WIFIMGR_SWITCH_ON) {
		ituCheckBoxSetChecked(powerOffSettingWiFiOpenCheckBox, true);
		ituWidgetSetVisible(powerOffSettingWiFiAllBackground, true);
#ifdef CFG_NET_WIFI
		if (strcmp(powerOffPList[0].ssidName, "") == 0)
			powerOffGnApCount = wifiMgr_get_scan_ap_info(powerOffPList);

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
		ituCheckBoxSetChecked(powerOffSettingWiFiOpenCheckBox, false);
		ituWidgetSetVisible(powerOffSettingWiFiAllBackground, false);
		//ituTextSetString(settingIPText, "0.0.0.0");
	}
	if (theConfig.wifi_on_off == WIFIMGR_SWITCH_ON)
	//if (ituRadioBoxIsChecked(powerOffSettingWiFiOpenCheckBox))
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



	if (powerOffGnApCount > 0)
	{
		//if (wifiNum == 0)
		//{
		//	wifiNum = 1;
		if (wifi_connected)
		{
			sprintf(buf, "%s", theConfig.ssid);
			ituTextSetString(powerOffSettingWiFiSsidNameText, buf);
		}
		else
		{
			sprintf(buf, "%s", powerOffPList[0].ssidName);
			ituTextSetString(powerOffSettingWiFiSsidNameText, buf);
			ituTextSetString(powerOffWifiSet[0].ssidtext, buf);

			if (powerOffPList[0].rfQualityQuant < 10)
			{
				ituSpriteGoto(powerOffWifiSet[0].signalSprite, 0);
			}
			else if (powerOffPList[0].rfQualityQuant < 20)
			{
				ituSpriteGoto(powerOffWifiSet[0].signalSprite, 1);
			}
			else if (powerOffPList[0].rfQualityQuant < 30)
			{
				ituSpriteGoto(powerOffWifiSet[0].signalSprite, 2);
			}
			else
			{
				ituSpriteGoto(powerOffWifiSet[0].signalSprite, 3);
			}

			ituWidgetSetVisible(powerOffWifiSet[0].background, true);
		}
			

			
			
		//}
		
	}
	else
	{
		ituWidgetSetVisible(powerOffWifiSet[0].background, false);
		//wifiNum = 0;
	}
//

	return true;
}
bool PowerOffSettingTimeBtnOnMouseUp(ITUWidget* widget, char* param)
{
	if (ituCheckBoxIsChecked(powerOffSettingTimeAutoCheckBox))
	{
		//memoryTimeAutoCheckbox = true;
		ituWidgetSetVisible(powerOffSettingTimeManualContainer, false);
		ituWidgetSetVisible(powerOffSettingTimeSyncContainer, true);
		//sync internet time
	}
	else
	{
		//memoryTimeAutoCheckbox = false;
		ituWidgetSetVisible(powerOffSettingTimeManualContainer, true);
		ituWidgetSetVisible(powerOffSettingTimeSyncContainer, false);

	struct timeval tv;
	struct tm *tm;

	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	ituWheelGoto(powerOffSettingTimeYearWheel, tm->tm_year - 120);//2020-1900
	ituWheelGoto(powerOffSettingTimeMonthWheel, tm->tm_mon);//0-11
	ituWheelGoto(powerOffSettingTimeDayWheel, tm->tm_mday - 1);//1-31
	ituWheelGoto(powerOffSettingTimeHrWheel, tm->tm_hour);
	ituWheelGoto(powerOffSettingTimeMinWheel, tm->tm_min);
	}

	

	powerOffSettingStopAnywhere->widget.flags &= ~ITU_DRAGGABLE;

	return true;
}

bool PowerOffSettingTimeWheelOnChanged(ITUWidget* widget, char* param)
{
	if ((powerOffSettingTimeYearWheel->focusIndex % 4) == 0)
	{
		if (powerOffSettingTimeMonthWheel->focusIndex == 1)
		{
			if (powerOffSettingTimeDayWheel->focusIndex > 28)
				ituWheelGoto(powerOffSettingTimeDayWheel, 28);
		}
	}

	else
	{
		if (powerOffSettingTimeMonthWheel->focusIndex == 1)
		{
			if (powerOffSettingTimeDayWheel->focusIndex > 27)
				ituWheelGoto(powerOffSettingTimeDayWheel, 27);
		}
	}


	if ((powerOffSettingTimeMonthWheel->focusIndex == 3) || (powerOffSettingTimeMonthWheel->focusIndex == 5) || (powerOffSettingTimeMonthWheel->focusIndex == 8) || (powerOffSettingTimeMonthWheel->focusIndex == 10))
	{
		if (powerOffSettingTimeDayWheel->focusIndex > 29)
			ituWheelGoto(powerOffSettingTimeDayWheel, 29);
	}
	return true;
}

bool PowerOffSettingTimeSaveBtnOnPress(ITUWidget* widget, char* param)
{
	bool save = atoi(param);
	char tmp[32];
	if (save)
	{
		if (ituCheckBoxIsChecked(powerOffSettingTimeAutoCheckBox))
		{
			memoryTimeAutoCheckbox = true;
			sprintf(tmp, "%s-%s-%s %s:%s", ituTextGetString(powerOffSettingTimeSyncYearText),
				ituTextGetString(powerOffSettingTimeSyncMonthText),
				ituTextGetString(powerOffSettingTimeSyncDayText),
				ituTextGetString(powerOffSettingTimeSyncHourText),
				ituTextGetString(powerOffSettingTimeSyncMinText));
			ituTextSetString(powerOffSettingTimeText, tmp);
		}
		else
		{
			memoryTimeAutoCheckbox = false;
		struct timeval tv;
		struct tm *tm, mytime;


		if ((powerOffSettingTimeYearWheel->focusIndex % 4) == 0)
		{
			if (powerOffSettingTimeMonthWheel->focusIndex == 1)
			{
				if (powerOffSettingTimeDayWheel->focusIndex > 28)
					ituWheelGoto(powerOffSettingTimeDayWheel, 28);
			}
		}

		else
		{
			if (powerOffSettingTimeMonthWheel->focusIndex == 1)
			{
				if (powerOffSettingTimeDayWheel->focusIndex > 27)
					ituWheelGoto(powerOffSettingTimeDayWheel, 27);
			}
		}


		if ((powerOffSettingTimeMonthWheel->focusIndex == 3) || (powerOffSettingTimeMonthWheel->focusIndex == 5) || (powerOffSettingTimeMonthWheel->focusIndex == 8) || (powerOffSettingTimeMonthWheel->focusIndex == 10))
		{
			if (powerOffSettingTimeDayWheel->focusIndex > 29)
				ituWheelGoto(powerOffSettingTimeDayWheel, 29);
		}

		sprintf(tmp, "%04d-%02d-%02d %02d:%02d", powerOffSettingTimeYearWheel->focusIndex + 2020,
			powerOffSettingTimeMonthWheel->focusIndex + 1,
			powerOffSettingTimeDayWheel->focusIndex + 1,
			powerOffSettingTimeHrWheel->focusIndex,
			powerOffSettingTimeMinWheel->focusIndex);
		ituTextSetString(powerOffSettingTimeText, tmp);

		//set time
		gettimeofday(&tv, NULL);
		tm = localtime(&tv.tv_sec);

		memcpy(&mytime, tm, sizeof (struct tm));

		mytime.tm_year = powerOffSettingTimeYearWheel->focusIndex + 120;
		mytime.tm_mon = powerOffSettingTimeMonthWheel->focusIndex;
		mytime.tm_mday = powerOffSettingTimeDayWheel->focusIndex + 1;
		mytime.tm_hour = powerOffSettingTimeHrWheel->focusIndex;
		mytime.tm_min = powerOffSettingTimeMinWheel->focusIndex;


		tv.tv_sec = mktime(&mytime);
		tv.tv_usec = 0;

		settimeofday(&tv, NULL);
	}

	}
	else
	{
		ituCheckBoxSetChecked(powerOffSettingTimeAutoCheckBox, memoryTimeAutoCheckbox);
	}

	powerOffSettingStopAnywhere->widget.flags |= ITU_DRAGGABLE;

    return true;
}
bool PowerOffSettingTimeAutoChkBoxOnPress(ITUWidget* widget, char* param)
{
	if (ituCheckBoxIsChecked(powerOffSettingTimeAutoCheckBox))
	{

		ituWidgetSetVisible(powerOffSettingTimeManualContainer, false);
		ituWidgetSetVisible(powerOffSettingTimeSyncContainer, true);
		//sync internet time
	}
	else
	{
		ituWidgetSetVisible(powerOffSettingTimeManualContainer, true);
		ituWidgetSetVisible(powerOffSettingTimeSyncContainer, false);

		struct timeval tv;
		struct tm *tm;

		gettimeofday(&tv, NULL);
		tm = localtime(&tv.tv_sec);

		ituWheelGoto(powerOffSettingTimeYearWheel, tm->tm_year - 120);//2020-1900
		ituWheelGoto(powerOffSettingTimeMonthWheel, tm->tm_mon);//0-11
		ituWheelGoto(powerOffSettingTimeDayWheel, tm->tm_mday - 1);//1-31
		ituWheelGoto(powerOffSettingTimeHrWheel, tm->tm_hour);
		ituWheelGoto(powerOffSettingTimeMinWheel, tm->tm_min);
	}
	return true;
}
//
//bool PowerOffSettingLightBtnOnMouseUp(ITUWidget* widget, char* param)
//{
//
//	if (ituCheckBoxIsChecked(powerOffSettingLightAutoCheckBox))
//	{
//		ituIconLinkSurface(&powerOffSettingScreenLightTrackBar->tracker->bg.icon, powerOffSettingLightTopBarBtnIcon[0]);
//		ituIconLinkSurface(&powerOffSettingIndLightTrackBar->tracker->bg.icon, powerOffSettingLightTopBarBtnIcon[0]);
//
//		ituWidgetDisable(powerOffSettingScreenLightTrackBar);
//		ituWidgetSetVisible(powerOffSettingScreenLightProgressBar, false);
//		ituWidgetDisable(powerOffSettingIndLightTrackBar);
//		ituWidgetSetVisible(powerOffSettingIndLightProgressBar, false);
//	}
//	else
//	{
//		ituIconLinkSurface(&powerOffSettingScreenLightTrackBar->tracker->bg.icon, powerOffSettingLightTopBarBtnIcon[1]);
//		ituIconLinkSurface(&powerOffSettingIndLightTrackBar->tracker->bg.icon, powerOffSettingLightTopBarBtnIcon[1]);
//
//		ituWidgetEnable(powerOffSettingScreenLightTrackBar);
//		ituWidgetSetVisible(powerOffSettingScreenLightProgressBar, true);
//		ituWidgetEnable(powerOffSettingIndLightTrackBar);
//		ituWidgetSetVisible(powerOffSettingIndLightProgressBar, true);
//	}
//
//
//	powerOffSettingStopAnywhere->widget.flags &= ~ITU_DRAGGABLE;
//
//    return true;
//}
//
//bool PowerOffSettingLightBtnOnSlideUp(ITUWidget* widget, char* param)
//{
//	powerOffSettingStopAnywhere->widget.flags |= ITU_DRAGGABLE;
//
//	return true;
//}

//bool PowerOffSettingScreenLockBtnOnMouseUp(ITUWidget* widget, char* param)
//{
//	ituRadioBoxSetChecked(powerOffSettingScreenLockRadioBox[0], false);
//	ituRadioBoxSetChecked(powerOffSettingScreenLockRadioBox[1], false);
//	ituRadioBoxSetChecked(powerOffSettingScreenLockRadioBox[2], false);
//
//	ituRadioBoxSetChecked(powerOffSettingScreenLockRadioBox[powerOffSettingScreenLockTimeSprite->frame], true);
//
//	powerOffSettingStopAnywhere->widget.flags &= ~ITU_DRAGGABLE;
//	return true;
//}

//bool PowerOffSettingScreenLockSaveBtnOnPress(ITUWidget* widget, char* param)
//{
//	bool save = atoi(param);
//
//	if (save)
//	{
//		if (ituRadioBoxIsChecked(powerOffSettingScreenLockRadioBox[0]))
//		{
//			ituSpriteGoto(powerOffSettingScreenLockTimeSprite, 0);
//			theConfig.screensaver_time = 60;
//		}
//		else if (ituRadioBoxIsChecked(powerOffSettingScreenLockRadioBox[1]))
//		{
//			ituSpriteGoto(powerOffSettingScreenLockTimeSprite, 1);
//			theConfig.screensaver_time = 180;
//		}
//		else
//		{
//			ituSpriteGoto(powerOffSettingScreenLockTimeSprite, 2);
//			theConfig.screensaver_time = 600;
//		}
//		ConfigSave();
//			
//	}
//	
//
//	powerOffSettingStopAnywhere->widget.flags |= ITU_DRAGGABLE;
//	return true;
//}

//bool PowerOffSettingScreenLockRadBoxOnPress(ITUWidget* widget, char* param)
//{
//	int radioboxIndex = atoi(param);
//
//	switch (radioboxIndex)
//	{
//	case 0:
//		ituRadioBoxSetChecked(powerOffSettingScreenLockRadioBox[0], true);
//		ituRadioBoxSetChecked(powerOffSettingScreenLockRadioBox[1], false);
//		ituRadioBoxSetChecked(powerOffSettingScreenLockRadioBox[2], false);
//		break;
//	case 1:
//		ituRadioBoxSetChecked(powerOffSettingScreenLockRadioBox[0], false);
//		ituRadioBoxSetChecked(powerOffSettingScreenLockRadioBox[1], true);
//		ituRadioBoxSetChecked(powerOffSettingScreenLockRadioBox[2], false);
//		break;
//	case 2:
//		ituRadioBoxSetChecked(powerOffSettingScreenLockRadioBox[0], false);
//		ituRadioBoxSetChecked(powerOffSettingScreenLockRadioBox[1], false);
//		ituRadioBoxSetChecked(powerOffSettingScreenLockRadioBox[2], true);
//		break;
//	}
//
//	return true;
//}
bool PowerOffSettingWarningLightCheckBoxOnPress(ITUWidget* widget, char* param)
{
	if (ituCheckBoxIsChecked(powerOffSettingWarningLightCheckBox))
	{
		indicatorLightEnable = true;
	}
	else
	{
		indicatorLightEnable = false;
	}
	return true;
}
bool PowerOffSettingWiFiBtnOnMouseUp(ITUWidget* widget, char* param)
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
		powerOffGnApCount = wifiMgr_get_scan_ap_info(powerOffPList);
#endif
		/***************************************************/

		powerOffShowWiFiItem();
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
//	for (i = 0; i < powerOffGnApCount; i++)
//	{
//		if (!strncmp(theConfig.ssid, powerOffPList[i].ssidName, strlen(powerOffPList[i].ssidName)))
//		{
//			if (bIsAvail && nConnect == 0)
//			{
//#ifdef CFG_NET_WIFI_SDIO_NGPL
//				if (powerOffSettingWiFiReturnLayer){
//					ituSpriteGoto(powerOffWifiSet[i].connectSprite,0);
//					return true;
//				}
//#endif
//				//if (language == LANG_CHT)
//				//	ituScrollTextSetString(scrolltext, (char *)StringGetWiFiConnected());
//				//else
//				//	ituScrollTextSetString(scrolltext, (char *)StringGetWiFiConnectedEng());
//
//				ituSpriteGoto(powerOffWifiSet[i].connectSprite, 1);
//
//				printf("SsidStatus %s \n", theConfig.ssid);
//				nConnect++;
//			}
//			else
//			{
//				ituSpriteGoto(powerOffWifiSet[i].connectSprite, 0);
//			}
//		}
//		else
//		{
//			ituSpriteGoto(powerOffWifiSet[i].connectSprite, 0);
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
bool PowerOffSettingWiFiSubBtnOnPress(ITUWidget* widget, char* param)
{
	int nIndex = atoi(param);

	SettingWiFiPasswordSetData(powerOffPList[nIndex].ssidName, powerOffPList[nIndex].securityMode);

	return true;
}

bool PowerOffSettingWiFiOpenCheckBoxOnPress(ITUWidget* widget, char* param)
{
	if (ituCheckBoxIsChecked(powerOffSettingWiFiOpenCheckBox))
	{
		ituWidgetSetVisible(powerOffSettingWiFiAllBackground, true);

		//open WiFi
		//get wifi AP data....
		/*Eason*/
		theConfig.wifi_mode = WIFIMGR_MODE_MAX;
		theConfig.wifi_on_off = WIFIMGR_SWITCH_ON;

#ifdef CFG_NET_WIFI
		WifiMgr_clientMode_switch(theConfig.wifi_on_off);
		powerOffGnApCount = wifiMgr_get_scan_ap_info(powerOffPList);
#endif
		/***************************************************/

		powerOffShowWiFiItem();


	}
	else
	{
		ituWidgetSetVisible(powerOffSettingWiFiAllBackground, false);

		//close WiFi
		theConfig.wifi_on_off = WIFIMGR_SWITCH_OFF;
		//ituSpriteGoto(topWiFiSprite, 0);
		wifi_status = 2;
#ifdef CFG_NET_WIFI
		wifiMgr_clientMode_disconnect();
		WifiMgr_clientMode_switch(theConfig.wifi_on_off);
#ifdef CFG_NET_WIFI_SDIO_NGPL
		ip_addr_set_zero(&xnetif[0].ip_addr);
#endif
#endif

		memset(theConfig.ssid, 0, sizeof(theConfig.ssid));
		memset(theConfig.password, 0, sizeof(theConfig.password));
	}
	ConfigSave();
	return true;
}
bool PowerOffSettingRestoreAllSettingBtnOnMouseUp(ITUWidget* widget, char* param)
{
	//restore all setting, clear customer's content
	return true;
}

bool PowerOffSettingRestoreSettingBtnOnMouseUp(ITUWidget* widget, char* param)
{
	//restore setting but keep customer's content
	return true;
}

bool PowerOffSettingExaminePasswordBtnOnMouseUp(ITUWidget* widget, char* param)
{
	int input = atoi(param);
	int i;

	if (input == 10)
	{
		ituSpriteGoto(powerOffSettingExaminePasswordCorrectSprite, 0);
		if (powerOffPasswordCnt != 0)
		{
			
			powerOffPassword = powerOffPassword - powerOffPassword % ((int)pow(10, 5 - powerOffPasswordCnt));
			powerOffPasswordCnt--;
			ituIconLinkSurface(powerOffSettingExaminePasswordIcon[powerOffPasswordCnt], powerOffSettingExaminePasswordEmptyIcon);
			
		}
			
	}
	else
	{
		if (powerOffPasswordCnt < 4)
		{
			for (i = 0; i < powerOffPasswordCnt; i++)
			{
				ituIconLinkSurface(powerOffSettingExaminePasswordIcon[i], powerOffSettingExaminePasswordEnterIcon);
			}

			ituIconLinkSurface(powerOffSettingExaminePasswordIcon[powerOffPasswordCnt], powerOffSettingExaminePasswordEnterIcon);
			powerOffPassword = input*pow(10, 3 - powerOffPasswordCnt) + powerOffPassword;
			powerOffPasswordCnt++;


		}

		if (powerOffPasswordCnt == 4)
		{
			if (powerOffPassword == theConfig.examine_pw)
			{
				ituWidgetSetVisible(powerOffSettingExamineBackground, true);
				ituWidgetSetVisible(powerOffSettingExaminePasswordBackground, false);
			}
			else
			{
				for (i = 0; i < 4; i++)
				{
					ituIconLinkSurface(powerOffSettingExaminePasswordIcon[i], powerOffSettingExaminePasswordUncorrectIcon);
				}
				ituSpriteGoto(powerOffSettingExaminePasswordCorrectSprite, 1);
			}
		}

		
	}
	return true;
}

bool PowerOffSettingExamineButtonOnMouseUp(ITUWidget* widget, char* param)
{
	powerOffPasswordCnt = 0;
	powerOffPassword = 0;
	for (int i = 0; i < 4; i++)
	{
		ituIconLinkSurface(powerOffSettingExaminePasswordIcon[i], powerOffSettingExaminePasswordEmptyIcon);
	}
	ituSpriteGoto(powerOffSettingExaminePasswordCorrectSprite, 0);
	return true;
}

bool PowerOffSettingKeySoundCheckBoxOnMouseUp(ITUWidget* widget, char* param)
{
	if (ituCheckBoxIsChecked(powerOffSettingKeySoundCheckBox))
	{
		keySound = true;
	}
	else
	{
		keySound = false;
	}
	return true;
}
void powerOffShowWiFiItem(void)
{
	int i;
	char buf[32];


	if (powerOffGnApCount >= 1)
	{
		sprintf(buf, "%s", powerOffPList[0].ssidName);
		ituTextSetString(powerOffSettingWiFiSsidNameText, buf);
		ituTextSetString(powerOffWifiSet[0].ssidtext, buf);

		if (powerOffPList[0].rfQualityQuant < 10)
		{
			ituSpriteGoto(powerOffWifiSet[0].signalSprite, 0);
		}
		else if (powerOffPList[0].rfQualityQuant < 20)
		{
			ituSpriteGoto(powerOffWifiSet[0].signalSprite, 1);
		}
		else if (powerOffPList[0].rfQualityQuant < 30)
		{
			ituSpriteGoto(powerOffWifiSet[0].signalSprite, 2);
		}
		else
		{
			ituSpriteGoto(powerOffWifiSet[0].signalSprite, 3);
		}

		ituWidgetSetVisible(powerOffWifiSet[0].background, true);

		if (powerOffGnApCount>1)
		{
			for (i = 1; i < powerOffGnApCount; i++)
			{
				ITCTree* node = NULL;
				ITUBackground* cloneBackground = NULL;

				char id[8];

				ituBackgroundClone((ITUWidget*)powerOffSettingWiFiTmpBackground, (ITUWidget**)&cloneBackground);
				powerOffWifiSet[i].background = cloneBackground;

				ITUWidget* widget = (ITUWidget*)cloneBackground;

				node = ((ITCTree*)widget)->child;
				ITUButton* button = (ITUButton*)node;
				powerOffWifiSet[i].button = button;
				sprintf(id, "%d", i);
				strcpy(button->actions[1].param, id);

				node = node->sibling;
				ITUText* ssidText = (ITUText*)node;
				powerOffWifiSet[i].ssidtext = ssidText;
				sprintf(buf, "%s", powerOffPList[i].ssidName);
				ituTextSetString(powerOffWifiSet[i].ssidtext, buf);


				node = node->sibling;
				ITUSprite* connectSprite = (ITUSprite*)node;
				powerOffWifiSet[i].connectSprite = connectSprite;

				node = node->sibling;
				ITUSprite* signalSprite = (ITUSprite*)node;
				powerOffWifiSet[i].signalSprite = signalSprite;
				if (powerOffPList[i].rfQualityQuant < 10)
				{
					ituSpriteGoto(powerOffWifiSet[i].signalSprite, 0);
				}
				else if (powerOffPList[i].rfQualityQuant < 20)
				{
					ituSpriteGoto(powerOffWifiSet[i].signalSprite, 1);
				}
				else if (powerOffPList[i].rfQualityQuant < 30)
				{
					ituSpriteGoto(powerOffWifiSet[i].signalSprite, 2);
				}
				else
				{
					ituSpriteGoto(powerOffWifiSet[i].signalSprite, 3);
				}

				ituWidgetSetVisible(powerOffWifiSet[i].background, true);

				ituWidgetAdd(powerOffSettingWiFiAllBackground, powerOffWifiSet[i].background);
				ituWidgetSetPosition(powerOffWifiSet[i].background, 0, i * 94);
				ituWidgetSetDimension(powerOffSettingWiFiAllBackground, 670, i * 94 + 88);
				powerOffWifiAllBackgroundHeight = ituWidgetGetHeight(powerOffSettingWiFiAllBackground); // = weekTotalItem * 150 + 130;
				if (powerOffWifiAllBackgroundHeight > STOPANYWHERE_H)
					ituWidgetSetDimension(powerOffSettingWiFiStopAnywhere, 670, STOPANYWHERE_H);
				else
					ituWidgetSetDimension(powerOffSettingWiFiStopAnywhere, 670, powerOffWifiAllBackgroundHeight);

				ituWidgetUpdate(powerOffSettingWiFiAllBackground, ITU_EVENT_LAYOUT, 0, 0, 0);// update the container widget


			}
		}

	}
	else
	{
		ituWidgetSetVisible(powerOffWifiSet[0].background, false);
	}

}