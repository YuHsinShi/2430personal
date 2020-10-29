#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "wifiMgr.h"

/* widgets:
settingWiFiOthersLayer
settingWiFOthersBackground
settingWiFiOthersCancelButton
Text64
Text43
settingWiFiOthersConnectButton
settingWiFiOthersPasswordTextBox
settingWiFiOthersNameTextBox
Icon1
Icon21
*/
ITUTextBox* settingWiFiOthersNameTextBox;
ITUTextBox* settingWiFiOthersPasswordTextBox;
static ITULayer*  settingLayer;

// status
static int settingWiFiMainModeOld;
static char settingWiFiMainSsidOld[64];
static char settingWiFiMainPasswordOld[256];

bool SettingWiFiOthersOnEnter(ITUWidget* widget, char* param)
{
	if (!settingWiFiOthersNameTextBox)
	{
		settingWiFiOthersNameTextBox = ituSceneFindWidget(&theScene, "settingWiFiOthersNameTextBox");
		assert(settingWiFiOthersNameTextBox);

		settingWiFiOthersPasswordTextBox = ituSceneFindWidget(&theScene, "settingWiFiOthersPasswordTextBox");
		assert(settingWiFiOthersPasswordTextBox);

		settingLayer = ituSceneFindWidget(&theScene, "settingLayer");
		assert(settingLayer);
	}

	settingWiFiMainModeOld = theConfig.wifi_mode;

	ituTextBoxSetString(settingWiFiOthersNameTextBox, theConfig.ssid);
	strcpy(settingWiFiMainSsidOld, theConfig.ssid);

	ituTextBoxSetString(settingWiFiOthersPasswordTextBox, theConfig.password);
	strcpy(settingWiFiMainPasswordOld, theConfig.password);
    return true;
}

bool SettingWiFiOthersOnLeave(ITUWidget* widget, char* param)
{
	if (settingWiFiMainModeOld != theConfig.wifi_mode ||
		strcmp(settingWiFiMainSsidOld, theConfig.ssid) ||
		strcmp(settingWiFiMainPasswordOld, theConfig.password))
	{
		ConfigSave();
	}
    return true;
}

bool SettingWiFiOthersConnectBtOnMouseUp(ITUWidget* widget, char* param)
{
	//connect to AP
	ituLayerGoto(settingLayer);
    return true;
}

