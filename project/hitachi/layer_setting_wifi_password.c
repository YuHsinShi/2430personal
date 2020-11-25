#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "ite/ith.h"
#include "wifiMgr.h"
#ifdef CFG_NET_WIFI_SDIO_NGPL
#include "wifi_constants.h"
#endif

extern unsigned char  py_ime(unsigned char *input_py_val,unsigned char * get_hanzi,unsigned short*hh);



static ITULayer      *settingLayer;
static ITUTextBox    *settingWiFiPasswordTextBox;
static ITUKeyboard   *settingWiFiPasswordKeyboard;
static ITUBackground *settingWiFiPasswordEnUpperBackground;
static ITUBackground *settingWiFiPasswordEnLowerBackground;
static ITUBackground *settingWiFiPasswordNumBackground;
static ITUBackground *settingWiFiPasswordSymbolBackground;
static ITUButton     *settingWiFiPasswordSymbolSButton;
static ITUButton     *settingWiFiPasswordNumSButton;
static ITUButton     *settingWiFiPasswordNumButton;
static ITUButton     *settingWiFiPasswordEnButton;
static ITUCheckBox   *settingWiFiPasswordEnUpperCheckBox;
static ITUText		 *settingWiFiPasswordSsidNameText;


#ifdef CFG_NET_WIFI_SDIO_NGPL
static char preSsidName[32];
static unsigned long gAPSecurityMode;
#else
static int           gAPSecurityMode;
#endif


bool SettingWiFiPasswordEnterButtonOnPress(ITUWidget *widget, char *param)
{
    char buf[512];
#ifdef CFG_NET_WIFI_SDIO_NGPL
    unsigned long security;
    char security_ite[3];
#else
    char security[3];
#endif

    strcpy(buf, ituTextGetString(settingWiFiPasswordKeyboard->target));

        memset(theConfig.password, 0, sizeof(theConfig.password));
#ifdef CFG_NET_WIFI_SDIO_NGPL
		security = gAPSecurityMode;

		/*printf("====>security: %16s, gAPSecurityMode: 0x%X\n",
			((security == RTW_SECURITY_OPEN ) ? "Open" :
			(security == RTW_SECURITY_WEP_PSK ) ? "WEP" :
			(security == RTW_SECURITY_WPA_TKIP_PSK ) ? "WPA TKIP" :
			(security == RTW_SECURITY_WPA_AES_PSK ) ? "WPA AES" :
			(security == RTW_SECURITY_WPA2_AES_PSK ) ? "WPA2 AES" :
			(security == RTW_SECURITY_WPA2_TKIP_PSK ) ? "WPA2 TKIP" :
			(security == RTW_SECURITY_WPA2_MIXED_PSK ) ? "WPA2 Mixed" :
			(security == RTW_SECURITY_WPA_WPA2_MIXED ) ? "WPA/WPA2 AES" : "Unknown"), gAPSecurityMode);*/
#else
        sprintf(security, "%d", gAPSecurityMode);
#endif

#ifdef CFG_NET_WIFI
        wifiMgr_clientMode_disconnect();
        //Do not set to WIFI_SWITCH_Off
#endif
        if (gAPSecurityMode > 0)
        {
            strcpy(theConfig.password, buf);
        }

#ifdef CFG_NET_WIFI_SDIO_NGPL
        strcpy(security_ite, WifiMgr_Secu_8189F_To_ITE(security));
        strcpy(theConfig.secumode, security_ite);
        strcpy(theConfig.ssid, preSsidName);
#else
        strcpy(theConfig.secumode, security);
#endif
        ConfigSave();
        printf("SettingWiFiPasswordEnterButtonOnPress  %s\n", buf);
        if (1)
        {
    #ifdef CFG_NET_WIFI
#ifdef CFG_NET_WIFI_SDIO_NGPL
            wifiMgr_clientMode_connect_ap(preSsidName, theConfig.password, theConfig.secumode);
#else
            wifiMgr_clientMode_connect_ap(theConfig.ssid, theConfig.password, security);
#endif
#endif
		wifi_connected = true;
			ituLayerGoto(settingLayer);
        }
        else
        {
            ituTextBoxSetString(settingWiFiPasswordTextBox, NULL);
        }
    
    return true;
}

bool SettingWiFiPasswordBackButtonOnPress(ITUWidget *widget, char *param)
{

        ituTextBoxBack((ITUTextBox *)settingWiFiPasswordKeyboard->target);

	return true;
}

bool SettingWiFiPasswordEnUpperCheckBoxOnPress(ITUWidget *widget, char *param)
                {
	if (ituCheckBoxIsChecked(settingWiFiPasswordEnUpperCheckBox))
                {
		ituWidgetSetVisible(settingWiFiPasswordEnUpperBackground, true);
		ituWidgetSetVisible(settingWiFiPasswordEnLowerBackground, false);
    }
    else
    {
		ituWidgetSetVisible(settingWiFiPasswordEnUpperBackground, false);
		ituWidgetSetVisible(settingWiFiPasswordEnLowerBackground, true);
    }
}

int SettingWiFiPasswordSetData(char *ssid,
#ifdef CFG_NET_WIFI_SDIO_NGPL
	unsigned long
#else
	int
#endif
	securityMode)
{
#ifdef CFG_NET_WIFI_SDIO_NGPL
    strcpy(preSsidName, ssid);
#else
    strcpy(theConfig.ssid, ssid);
#endif

    gAPSecurityMode = securityMode;
    printf("SettingWiFiPasswordSetData: %s(%d) \n", ssid, securityMode);
    return 0;
}

bool SettingWiFiPasswordOnEnter(ITUWidget *widget, char *param)
{
	char tmp[64];
	if (!settingLayer)
    {
		settingLayer = ituSceneFindWidget(&theScene, "settingLayer");
		assert(settingLayer);

        settingWiFiPasswordTextBox           = ituSceneFindWidget(&theScene, "settingWiFiPasswordTextBox");
        assert(settingWiFiPasswordTextBox);

        settingWiFiPasswordKeyboard          = ituSceneFindWidget(&theScene, "settingWiFiPasswordKeyboard");
        assert(settingWiFiPasswordKeyboard);
        settingWiFiPasswordKeyboard->target  = (ITUWidget *)settingWiFiPasswordTextBox;

        settingWiFiPasswordEnUpperBackground = ituSceneFindWidget(&theScene, "settingWiFiPasswordEnUpperBackground");
        assert(settingWiFiPasswordEnUpperBackground);

        settingWiFiPasswordEnLowerBackground = ituSceneFindWidget(&theScene, "settingWiFiPasswordEnLowerBackground");
        assert(settingWiFiPasswordEnLowerBackground);

        settingWiFiPasswordNumBackground     = ituSceneFindWidget(&theScene, "settingWiFiPasswordNumBackground");
		assert(settingWiFiPasswordNumBackground);

        settingWiFiPasswordSymbolBackground  = ituSceneFindWidget(&theScene, "settingWiFiPasswordSymbolBackground");
        assert(settingWiFiPasswordSymbolBackground);

        settingWiFiPasswordSymbolSButton      = ituSceneFindWidget(&theScene, "settingWiFiPasswordSymbolSButton");
		assert(settingWiFiPasswordSymbolSButton);

        settingWiFiPasswordEnUpperCheckBox     = ituSceneFindWidget(&theScene, "settingWiFiPasswordEnUpperCheckBox");
		assert(settingWiFiPasswordEnUpperCheckBox);

        settingWiFiPasswordEnButton     = ituSceneFindWidget(&theScene, "settingWiFiPasswordEnButton");
		assert(settingWiFiPasswordEnButton);

		settingWiFiPasswordNumSButton = ituSceneFindWidget(&theScene, "settingWiFiPasswordNumSButton");
		assert(settingWiFiPasswordNumSButton);

		settingWiFiPasswordNumButton = ituSceneFindWidget(&theScene, "settingWiFiPasswordNumButton");
		assert(settingWiFiPasswordNumButton);

		settingWiFiPasswordSsidNameText = ituSceneFindWidget(&theScene, "settingWiFiPasswordSsidNameText");
		assert(settingWiFiPasswordSsidNameText);
    }

    ituWidgetSetVisible(settingWiFiPasswordEnUpperBackground, false);
    ituWidgetSetVisible(settingWiFiPasswordEnLowerBackground, true);
	ituWidgetSetVisible(settingWiFiPasswordNumBackground, false);
    ituWidgetSetVisible(settingWiFiPasswordSymbolBackground, false);
	ituWidgetSetVisible(settingWiFiPasswordEnUpperCheckBox, true);
    ituWidgetSetVisible(settingWiFiPasswordEnButton, false);
    ituWidgetSetVisible(settingWiFiPasswordNumButton, true);
	ituWidgetSetVisible(settingWiFiPasswordNumSButton, false);
	ituWidgetSetVisible(settingWiFiPasswordSymbolSButton, false);


    //settingWiFiPasswordTextBox->textboxFlags |= ITU_TEXTBOX_PASSWORD;
        ituTextBoxSetString(settingWiFiPasswordTextBox, NULL);
	sprintf(tmp, "%s", theConfig.ssid);
	ituTextSetString(settingWiFiPasswordSsidNameText, tmp);

	wifi_connected = false;

    return true;
}
