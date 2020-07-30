#include "ite/itu.h"

extern bool LogoOnEnter(ITUWidget* widget, char* param);
extern bool MainLayerOnEnter(ITUWidget* widget, char* param);
extern bool MainLayerOnTimer(ITUWidget* widget, char* param);
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

ITUActionFunction actionFunctions[] =
{
    "LogoOnEnter", LogoOnEnter,
    "MainLayerOnEnter", MainLayerOnEnter,
    "MainLayerOnTimer", MainLayerOnTimer,
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
    NULL, NULL
};
