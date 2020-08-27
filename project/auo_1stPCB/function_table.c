#include "ite/itu.h"

extern bool LogoOnEnter(ITUWidget* widget, char* param);
extern bool MainLayerOnEnter(ITUWidget* widget, char* param);
extern bool MainLayerOnTimer(ITUWidget* widget, char* param);
extern bool SettingOnEnter(ITUWidget* widget, char* param);
extern bool SettingOnLeave(ITUWidget* widget, char* param);
extern bool SettingOnTimer(ITUWidget* widget, char* param);
extern bool SettingSetDefaultOKBtOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingRebootBtOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingCoverFlowOnChanged(ITUWidget* widget, char* param);
extern bool SettingCalibrationTempBtOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidCloseRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingOpenWiFiRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidOthersBtOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidNameScrollListBoxOnLoad(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidScrollListBoxOnSelect(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidStatusScrollListBoxOnLoad(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidSignalScrollListBoxOnLoad(ITUWidget* widget, char* param);
extern bool SettingScreenSaverType(ITUWidget* widget, char* param);
extern bool SettingWiFiOthersOnEnter(ITUWidget* widget, char* param);
extern bool SettingWiFiOthersOnLeave(ITUWidget* widget, char* param);
extern bool SettingWiFiOthersConnectBtOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordOnEnter(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordEnUpperButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordChsCharButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordEnterButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordPageDownButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordPageUpButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordBackButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordChsButtonOnPress(ITUWidget* widget, char* param);

extern bool OTA_http(ITUWidget* widget, char* param);

ITUActionFunction actionFunctions[] =
{
    "LogoOnEnter", LogoOnEnter,
    "MainLayerOnEnter", MainLayerOnEnter,
    "MainLayerOnTimer", MainLayerOnTimer,
    "SettingOnEnter", SettingOnEnter,
    "SettingOnLeave", SettingOnLeave,
    "SettingOnTimer", SettingOnTimer,
    "SettingSetDefaultOKBtOnMouseUp", SettingSetDefaultOKBtOnMouseUp,
    "SettingRebootBtOnMouseUp", SettingRebootBtOnMouseUp,
    "SettingCoverFlowOnChanged", SettingCoverFlowOnChanged,
    "SettingCalibrationTempBtOnMouseUp", SettingCalibrationTempBtOnMouseUp,
    "SettingWiFiSsidCloseRadioBoxOnPress", SettingWiFiSsidCloseRadioBoxOnPress,
    "SettingOpenWiFiRadioBoxOnPress", SettingOpenWiFiRadioBoxOnPress,
    "SettingWiFiSsidOthersBtOnMouseUp", SettingWiFiSsidOthersBtOnMouseUp,
    "SettingWiFiSsidNameScrollListBoxOnLoad", SettingWiFiSsidNameScrollListBoxOnLoad,
    "SettingWiFiSsidScrollListBoxOnSelect", SettingWiFiSsidScrollListBoxOnSelect,
    "SettingWiFiSsidStatusScrollListBoxOnLoad", SettingWiFiSsidStatusScrollListBoxOnLoad,
    "SettingWiFiSsidSignalScrollListBoxOnLoad", SettingWiFiSsidSignalScrollListBoxOnLoad,
    "SettingScreenSaverType", SettingScreenSaverType,
    "SettingWiFiOthersOnEnter", SettingWiFiOthersOnEnter,
    "SettingWiFiOthersOnLeave", SettingWiFiOthersOnLeave,
    "SettingWiFiOthersConnectBtOnMouseUp", SettingWiFiOthersConnectBtOnMouseUp,
    "SettingWiFiPasswordOnEnter", SettingWiFiPasswordOnEnter,
    "SettingWiFiPasswordEnUpperButtonOnPress", SettingWiFiPasswordEnUpperButtonOnPress,
    "SettingWiFiPasswordChsCharButtonOnPress", SettingWiFiPasswordChsCharButtonOnPress,
    "SettingWiFiPasswordEnterButtonOnPress", SettingWiFiPasswordEnterButtonOnPress,
    "SettingWiFiPasswordPageDownButtonOnPress", SettingWiFiPasswordPageDownButtonOnPress,
    "SettingWiFiPasswordPageUpButtonOnPress", SettingWiFiPasswordPageUpButtonOnPress,
    "SettingWiFiPasswordBackButtonOnPress", SettingWiFiPasswordBackButtonOnPress,
    "SettingWiFiPasswordChsButtonOnPress", SettingWiFiPasswordChsButtonOnPress,

	"OTA_http", OTA_http,
    NULL, NULL
};
