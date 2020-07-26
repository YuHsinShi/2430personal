#include "ite/itu.h"


extern bool MainOnEnter(ITUWidget* widget, char* param);
extern bool MainOnTimer(ITUWidget* widget, char* param);
//extern bool MainButtonOnMouseUp(ITUWidget* widget, char* param);
//extern bool mainCoverFlowOnChanged(ITUWidget* widget, char* param);
extern bool MainTemperatureMeterOnChanged(ITUWidget* widget, char* param);

extern bool AudioPlayerOnEnter(ITUWidget* widget, char* param);
extern bool AudioPlayerOnLeave(ITUWidget* widget, char* param);
extern bool AudioPlayerVoiceUpButtonOnMouseUp(ITUWidget* widget, char* param);
extern bool AudioPlayerVoiceDownButtonOnMouseUp(ITUWidget* widget, char* param);
extern bool AudioPlayerPreButtonOnMouseUp(ITUWidget* widget, char* param);
extern bool AudioPlayerNextButtonOnMouseUp(ITUWidget* widget, char* param);
extern bool AudioPlayerPlayButtonOnMouseUp(ITUWidget* widget, char* param);
extern bool AudioPlayerStopButtonOnMouseUp(ITUWidget* widget, char* param);
extern bool AudioPlayerTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool AudioPlayerOnTimer(ITUWidget* widget, char* param);

extern bool ScenesOnEnter(ITUWidget* widget, char* param);
extern bool ScenesOnLeave(ITUWidget* widget, char* param);
extern bool PlayAnFunction(ITUWidget* widget, char* param);
extern bool ScenesRtAn2OnStopped(ITUWidget* widget, char* param);

extern bool SettingOnEnter(ITUWidget* widget, char* param);
extern bool SettingOnLeave(ITUWidget* widget, char* param);
extern bool SettingScreenSaverType(ITUWidget* widget, char* param);
extern bool SettingProtectTimeTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool SettingProtectTimeUpOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingProtectTimeDownOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingVoiceTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool SettingVoiceUpOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingVoiceDownOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingLightTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool SettingLightUpOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingLightDownOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingSetDefaultOKBtOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingRebootBtOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidNameScrollListBoxOnLoad(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidScrollListBoxOnSelect(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidStatusScrollListBoxOnLoad(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidSignalScrollListBoxOnLoad(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidOthersBtOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingOpenWiFiRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidCloseRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingCoverFlowOnChanged(ITUWidget* widget, char* param);
extern bool settingChtRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool settingEngRadioBoxOnPress(ITUWidget* widget, char* param);


extern bool CurtainOnEnter(ITUWidget* widget, char* param);
extern bool CurtainCloseBtOnMousUp(ITUWidget* widget, char* param);
extern bool CurtainStopBtOnMousUp(ITUWidget* widget, char* param);
extern bool CurtainOpenBtOnMousUp(ITUWidget* widget, char* param);
extern bool CurtainCloseSpriteOnStopped(ITUWidget* widget, char* param);
extern bool CurtainOpenSpriteOnStopped(ITUWidget* widget, char* param);

extern bool ScreensaverOnEnter(ITUWidget* widget, char* param);
extern bool ScreenSaverOnTimer(ITUWidget* widget, char* param);

extern bool LightOnEnter(ITUWidget* widget, char* param);
extern bool LightCheckBoxOnMouseUp(ITUWidget* widget, char* param);
extern bool LightFunctionCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool LightAddBtOnMouseUp(ITUWidget* widget, char* param);
extern bool lightCheckBoxOnMouseLongPress(ITUWidget* widget, char* param);
extern bool lightBackButtonOnPress(ITUWidget* widget, char* param);
extern bool lightOnLeave(ITUWidget* widget, char* param);
extern bool lightGroupCheckBoxOnPress(ITUWidget* widget, char* param);

extern bool ClockOnEnter(ITUWidget* widget, char* param);
extern bool ClockConfirmButtonOnPress(ITUWidget* widget, char* param);

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

//for temperature
extern bool SettingCalibrationTempBtOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingOnTimer(ITUWidget* widget, char* param);



ITUActionFunction actionFunctions[] =
{
	"MainOnEnter", MainOnEnter,
	"MainOnTimer", MainOnTimer,
	"MainTemperatureMeterOnChanged", MainTemperatureMeterOnChanged,

	"AudioPlayerOnEnter", AudioPlayerOnEnter,
	"AudioPlayerOnLeave", AudioPlayerOnLeave,
	"AudioPlayerVoiceUpButtonOnMouseUp", AudioPlayerVoiceUpButtonOnMouseUp,
	"AudioPlayerVoiceDownButtonOnMouseUp", AudioPlayerVoiceDownButtonOnMouseUp,
	"AudioPlayerPreButtonOnMouseUp", AudioPlayerPreButtonOnMouseUp,
	"AudioPlayerNextButtonOnMouseUp", AudioPlayerNextButtonOnMouseUp,
	"AudioPlayerPlayButtonOnMouseUp", AudioPlayerPlayButtonOnMouseUp,
	"AudioPlayerStopButtonOnMouseUp", AudioPlayerStopButtonOnMouseUp,
	"AudioPlayerTrackBarOnChanged", AudioPlayerTrackBarOnChanged,
	"AudioPlayerOnTimer", AudioPlayerOnTimer,

	"ScenesOnEnter", ScenesOnEnter,
	"ScenesOnLeave", ScenesOnLeave,
	"PlayAnFunction", PlayAnFunction,
	"ScenesRtAn2OnStopped", ScenesRtAn2OnStopped,

	"SettingOnEnter", SettingOnEnter,
	"SettingOnLeave", SettingOnLeave,
	"SettingScreenSaverType", SettingScreenSaverType,
	"SettingProtectTimeTrackBarOnChanged", SettingProtectTimeTrackBarOnChanged,
	"SettingProtectTimeUpOnMouseUp", SettingProtectTimeUpOnMouseUp,
	"SettingProtectTimeDownOnMouseUp", SettingProtectTimeDownOnMouseUp,
	"SettingVoiceTrackBarOnChanged", SettingVoiceTrackBarOnChanged,
	"SettingVoiceUpOnMouseUp", SettingVoiceUpOnMouseUp,
	"SettingVoiceDownOnMouseUp", SettingVoiceDownOnMouseUp,
	"SettingLightTrackBarOnChanged", SettingLightTrackBarOnChanged,
	"SettingLightUpOnMouseUp", SettingLightUpOnMouseUp,
	"SettingLightDownOnMouseUp", SettingLightDownOnMouseUp,
	"SettingSetDefaultOKBtOnMouseUp", SettingSetDefaultOKBtOnMouseUp,
	"SettingRebootBtOnMouseUp", SettingRebootBtOnMouseUp, 
	"SettingWiFiSsidNameScrollListBoxOnLoad", SettingWiFiSsidNameScrollListBoxOnLoad,
	"SettingWiFiSsidScrollListBoxOnSelect", SettingWiFiSsidScrollListBoxOnSelect,
	"SettingWiFiSsidStatusScrollListBoxOnLoad", SettingWiFiSsidStatusScrollListBoxOnLoad,
	"SettingWiFiSsidSignalScrollListBoxOnLoad", SettingWiFiSsidSignalScrollListBoxOnLoad,
	"SettingWiFiSsidOthersBtOnMouseUp", SettingWiFiSsidOthersBtOnMouseUp,
	"SettingOpenWiFiRadioBoxOnPress", SettingOpenWiFiRadioBoxOnPress,
	"SettingWiFiSsidCloseRadioBoxOnPress", SettingWiFiSsidCloseRadioBoxOnPress,
	"SettingCoverFlowOnChanged", SettingCoverFlowOnChanged,
	"settingChtRadioBoxOnPress", settingChtRadioBoxOnPress,
	"settingEngRadioBoxOnPress", settingEngRadioBoxOnPress,

	"CurtainOnEnter", CurtainOnEnter,
	"CurtainCloseBtOnMousUp", CurtainCloseBtOnMousUp,
	"CurtainStopBtOnMousUp", CurtainStopBtOnMousUp,
	"CurtainOpenBtOnMousUp", CurtainOpenBtOnMousUp,
	"CurtainCloseSpriteOnStopped", CurtainCloseSpriteOnStopped,
	"CurtainOpenSpriteOnStopped", CurtainOpenSpriteOnStopped,
	
	"ScreensaverOnEnter", ScreensaverOnEnter,
	"ScreenSaverOnTimer", ScreenSaverOnTimer,

	"LightOnEnter", LightOnEnter,
	"LightCheckBoxOnMouseUp", LightCheckBoxOnMouseUp,
	"LightFunctionCheckBoxOnPress", LightFunctionCheckBoxOnPress,
	"LightAddBtOnMouseUp", LightAddBtOnMouseUp,
	"lightCheckBoxOnMouseLongPress", lightCheckBoxOnMouseLongPress,
	"lightBackButtonOnPress", lightBackButtonOnPress,
	"lightOnLeave", lightOnLeave,
	"lightGroupCheckBoxOnPress", lightGroupCheckBoxOnPress,

	"ClockOnEnter", ClockOnEnter,
	"ClockConfirmButtonOnPress", ClockConfirmButtonOnPress,

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
	"SettingCalibrationTempBtOnMouseUp", SettingCalibrationTempBtOnMouseUp,
	"SettingOnTimer", SettingOnTimer,

    NULL, NULL
};
