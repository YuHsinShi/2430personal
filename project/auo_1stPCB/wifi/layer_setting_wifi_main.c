#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "ctrlboard.h"
#include "wifiMgr.h"

static ITUSprite* settingWiFiMainTypeSprite;
static ITUBackground* settingWiFiMainGrayBackground;
static ITURadioBox* settingWiFiMainDisableRadioBox;
static ITURadioBox* settingWiFiMainAPRadioBox;
static ITURadioBox* settingWiFiMainConnectRadioBox;
ITUTextBox* settingWiFiMainNameTextBox;
ITUTextBox* settingWiFiMainPasswordTextBox;

// status
static int settingWiFiMainModeOld;
static char settingWiFiMainSsidOld[64];
static char settingWiFiMainPasswordOld[256];

bool SettingWiFiMainRadioBoxOnPress(ITUWidget* widget, char* param)
{
    if (ituRadioBoxIsChecked(settingWiFiMainDisableRadioBox))
    {
        theConfig.wifi_mode = WIFIMGR_MODE_CLIENT;
        theConfig.wifi_on_off = WIFIMGR_SWITCH_OFF;
#ifdef CFG_NET_WIFI_SDIO_NGPL     
        WifiMgr_clientMode_switch(theConfig.wifi_on_off);
#endif
		/*Clean wifi info when wifi is turned off*/
		memset(theConfig.ssid    , 0, sizeof(theConfig.ssid));
		memset(theConfig.password, 0, sizeof(theConfig.password));
		memset(theConfig.secumode, 0, sizeof(theConfig.secumode));

        #ifdef CFG_NET_WIFI
        wifiMgr_clientMode_disconnect();
        #endif
        ituSpriteGoto(settingWiFiMainTypeSprite, 0);
    }
    else if (ituRadioBoxIsChecked(settingWiFiMainAPRadioBox))
    {
        theConfig.wifi_mode = WIFIMGR_MODE_SOFTAP;
        ituSpriteGoto(settingWiFiMainTypeSprite, 1);
    }
    else if (ituRadioBoxIsChecked(settingWiFiMainConnectRadioBox))
    {
        theConfig.wifi_mode = WIFIMGR_MODE_MAX;
		theConfig.wifi_on_off = WIFIMGR_SWITCH_ON;
		#ifdef CFG_NET_WIFI
		WifiMgr_clientMode_switch(theConfig.wifi_on_off);
		#endif
        ituSpriteGoto(settingWiFiMainTypeSprite, 2);
    }

	ConfigSave();

    return true;
}

bool SettingWiFiMainOnEnter(ITUWidget* widget, char* param)
{
    if (!settingWiFiMainTypeSprite)
    {
        settingWiFiMainTypeSprite = ituSceneFindWidget(&theScene, "settingWiFiMainTypeSprite");
        assert(settingWiFiMainTypeSprite);

        settingWiFiMainGrayBackground = ituSceneFindWidget(&theScene, "settingWiFiMainGrayBackground");
        assert(settingWiFiMainGrayBackground);

        settingWiFiMainDisableRadioBox = ituSceneFindWidget(&theScene, "settingWiFiMainDisableRadioBox");
        assert(settingWiFiMainDisableRadioBox);

        settingWiFiMainAPRadioBox = ituSceneFindWidget(&theScene, "settingWiFiMainAPRadioBox");
        assert(settingWiFiMainAPRadioBox);

        settingWiFiMainConnectRadioBox = ituSceneFindWidget(&theScene, "settingWiFiMainConnectRadioBox");
        assert(settingWiFiMainConnectRadioBox);

        settingWiFiMainNameTextBox = ituSceneFindWidget(&theScene, "settingWiFiMainNameTextBox");
        assert(settingWiFiMainNameTextBox);

        settingWiFiMainPasswordTextBox = ituSceneFindWidget(&theScene, "settingWiFiMainPasswordTextBox");
        assert(settingWiFiMainPasswordTextBox);
    }

    // current settings
    switch (theConfig.wifi_mode)
    {
    case 0:
        ituRadioBoxSetChecked(settingWiFiMainDisableRadioBox, true);
        ituSpriteGoto(settingWiFiMainTypeSprite, 0);
        break;

    case 1:
        ituRadioBoxSetChecked(settingWiFiMainAPRadioBox, true);
        ituSpriteGoto(settingWiFiMainTypeSprite, 1);
        break;

    case 2:
        ituRadioBoxSetChecked(settingWiFiMainConnectRadioBox, true);
        ituSpriteGoto(settingWiFiMainTypeSprite, 2);
        break;
	}
    settingWiFiMainModeOld = theConfig.wifi_mode;

    ituTextBoxSetString(settingWiFiMainNameTextBox, theConfig.ssid);
    strcpy(settingWiFiMainSsidOld, theConfig.ssid);

    ituTextBoxSetString(settingWiFiMainPasswordTextBox, theConfig.password);
    strcpy(settingWiFiMainPasswordOld, theConfig.password);

    return true;
}

bool SettingWiFiMainOnLeave(ITUWidget* widget, char* param)
{
    if (settingWiFiMainModeOld != theConfig.wifi_mode ||
        strcmp(settingWiFiMainSsidOld, theConfig.ssid) ||
        strcmp(settingWiFiMainPasswordOld, theConfig.password))
    {
        ConfigSave();
    }
    return true;
}

void SettingWiFiMainReset(void)
{
    settingWiFiMainTypeSprite = NULL;
}
