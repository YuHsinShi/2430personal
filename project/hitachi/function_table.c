#include "ite/itu.h"

extern bool LogoOnEnter(ITUWidget* widget, char* param);

extern bool MainOnEnter(ITUWidget* widget, char* param);
extern bool MainOnTimer(ITUWidget* widget, char* param);
extern bool MainCoverFlowOnChanged(ITUWidget* widget, char* param);
//extern bool MainRModeRadBoxOnPress(ITUWidget* widget, char* param);
//extern bool MainRMoreModeBackgroundBtnOnPress(ITUWidget* widget, char* param);
extern bool MainHumidityBtnOnPress(ITUWidget* widget, char* param);
extern bool MainTempBtnOnPress(ITUWidget* widget, char* param);
extern bool MainLStatusChkBoxOnMouseUp(ITUWidget* widget, char* param);
extern bool MainTopIndLightTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool MainTopScreenLightTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool MainTopAutoChkBoxOnPress(ITUWidget* widget, char* param);
extern bool MainAirForceTrackBarOnChanged(ITUWidget* widget, char* param);
//extern bool MainAirForceAutoChkBoxOnPress(ITUWidget* widget, char* param);
//extern bool MainRAICheckBoxOnPress(ITUWidget* widget, char* param);
extern bool MainCModeShowButtonOnPress(ITUWidget* widget, char* param);
//extern bool MainOnSlideDown(ITUWidget* widget, char* param);
//extern bool MainOnSlideUp(ITUWidget* widget, char* param);

extern bool WarningOnEnter(ITUWidget* widget, char* param);
extern bool WarningResetBtnOnPress(ITUWidget* widget, char* param);

extern bool SetTemp1OnEnter(ITUWidget* widget, char* param);
extern bool SetTemp1BackgroundBtnOnPress(ITUWidget* widget, char* param);
extern bool SetTemp1WheelOnChanged(ITUWidget* widget, char* param);

extern bool SetTempOnEnter(ITUWidget* widget, char* param);
extern bool SetTempSetBtnOnPress(ITUWidget* widget, char* param);
extern bool SetTempWheelOnChanged(ITUWidget* widget, char* param);

extern bool SetHumidityOnEnter(ITUWidget* widget, char* param);
extern bool SetHumiditySetBtnOnPress(ITUWidget* widget, char* param);
extern bool SetHumidityWheelOnChanged(ITUWidget* widget, char* param);

extern bool MoreModeOnEnter(ITUWidget* widget, char* param);
extern bool MoreModeRadioBoxOnMouseUp(ITUWidget* widget, char* param);

extern bool InductionSettingOnEnter(ITUWidget* widget, char* param);
extern bool InductionSettingTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool InductionSettingActRadBoxOnPress(ITUWidget* widget, char* param);
extern bool InductionSettingRadBoxOnPress(ITUWidget* widget, char* param);

extern bool InductionOnEnter(ITUWidget* widget, char* param);
extern bool InductionRadBoxOnPress(ITUWidget* widget, char* param);

extern bool FilterOnEnter(ITUWidget* widget, char* param);
extern bool FilterOkBtnOnPress(ITUWidget* widget, char* param);

//extern bool EmergencyOnEnter(ITUWidget* widget, char* param);
extern bool EmergencyBackgroundBtnOnPress(ITUWidget* widget, char* param);

extern bool ScreenLockOnEnter(ITUWidget* widget, char* param);
extern bool ScreenLockOnTimer(ITUWidget* widget, char* param);

extern bool TimingOnEnter(ITUWidget* widget, char* param);
extern bool TimingOnLeave(ITUWidget* widget, char* param);
extern bool TimingOnTimer(ITUWidget* widget, char* param);
extern bool TimingRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool TimingSingleSettingSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingSingleSettingBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingSingleChkBoxOnPress(ITUWidget* widget, char* param);
extern bool TimingSingleSettingHrWheelOnChanged(ITUWidget* widget, char* param);
extern bool TimingPowerOffSingleSettingSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingPowerOffSingleSettingBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool TimingPowerOffSingleChkBoxOnPress(ITUWidget* widget, char* param);
extern bool TimingPowerOffSingleSettingHrWheelOnChanged(ITUWidget* widget, char* param);
extern bool TimingWeekSettingBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool TimingWeekSettingTempSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingWeekSettingDaySaveBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingWeekSettingTimeSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingWeekSettingRadBoxOnPress(ITUWidget* widget, char* param);
extern bool TimingWeekSettingSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingWeekSettingTimeBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingWeekSettingDayBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingWeekSettingTempBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingWeekAddBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool TimingWeekChkBoxOnPress(ITUWidget* widget, char* param);
extern bool TimingRestSettingSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingRestAddBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool TimingRestChkBoxOnPress(ITUWidget* widget, char* param);
extern bool TimingRestSettingWheelOnChanged(ITUWidget* widget, char* param);
extern bool TimingWeekSlideBtnOnSlideLeft(ITUWidget* widget, char* param);
extern bool TimingWeekSlideBtnOnSlideRight(ITUWidget* widget, char* param);
extern bool TimingWeekDeleteBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingRestSlideBtnOnSlideLeft(ITUWidget* widget, char* param);
extern bool TimingRestSlideBtnOnSlideRight(ITUWidget* widget, char* param);
extern bool TimingRestDeleteBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingBackBackgroundBtnOnPress(ITUWidget* widget, char* param);

extern bool SettingOnEnter(ITUWidget* widget, char* param);
extern bool SettingOnTimer(ITUWidget* widget, char* param);
extern bool SettingTimeSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool SettingTimeWheelOnChanged(ITUWidget* widget, char* param);
extern bool SettingTimeBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingTimeAutoChkBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingLightBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingLightBtnOnSlideUp(ITUWidget* widget, char* param);
extern bool SettingScreenLightTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool SettingIndLightTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool SettingLightAutoCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingScreenLockBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingScreenLockSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool SettingScreenLockRadBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingWarningLightCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingWiFiSubBtnOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiOpenCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingRestoreAllSettingBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingRestoreSettingBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingRestoreEnterBtnOnPress(ITUWidget* widget, char* param);
extern bool SettingExaminePasswordBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingExamineButtonOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingKeySoundCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingOnlineUpdateBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingOnlineUpdateDownloadBtnOnPress(ITUWidget* widget, char* param);
extern bool SettingOnlineUpdatingBtnOnPress(ITUWidget* widget, char* param);
extern bool SettingBackBackgroundBtnOnPress(ITUWidget* widget, char* param);

extern bool ExamineOnEnter(ITUWidget* widget, char* param);
extern bool ExamineSubEnterBtnOnPress(ITUWidget* widget, char* param);
extern bool ExamineSubRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool ExamineBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool ExamineShowBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool ExamineModelTestBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool ExamineShowModelRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool ExamineModelTestRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool ExamineModelTestEnterBtnOnPress(ITUWidget* widget, char* param);
extern bool ExamineBackgroundBtnOnPress(ITUWidget* widget, char* param);

extern bool PowerOffOnEnter(ITUWidget* widget, char* param);
extern bool PowerOffOnTimer(ITUWidget* widget, char* param);
extern bool PowerOffPowerOnBtnOnPress(ITUWidget* widget, char* param);
extern bool PowerOffTopIndLightTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool PowerOffTopScreenLightTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool PowerOffTopAutoChkBoxOnPress(ITUWidget* widget, char* param);
extern bool PowerOffDisinfectEnterBtnOnPress(ITUWidget* widget, char* param);
extern bool PowerOffDisinfectStopEnterBtnOnPress(ITUWidget* widget, char* param);
extern bool PowerOffCleanEnterBtnOnPress(ITUWidget* widget, char* param);
extern bool PowerOffCleanStopEnterBtnOnPress(ITUWidget* widget, char* param);

extern bool TrialRunOnEnter(ITUWidget* widget, char* param);
extern bool TrialRunTrialSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool TrialRunTrialAirForceEnterBtnOnPress(ITUWidget* widget, char* param);
extern bool TrialRunTrialAirForceRadBoxOnPress(ITUWidget* widget, char* param);
extern bool TrialRuntTrialTimeSaveBackgroundBtnOnPress(ITUWidget* widget, char* param);
extern bool TrialRuntTrialFreqSaveBackgroundBtnOnPress(ITUWidget* widget, char* param);
extern bool TrialRunModelSelectEnterBtnOnPress(ITUWidget* widget, char* param);
extern bool TrialRunModelSelectItemRadBoxOnPress(ITUWidget* widget, char* param);
extern bool TrialRunModelSelectPMBtnOnPress(ITUWidget* widget, char* param);
extern bool TrialRunModelSelectRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool TrialRunInputOutputEnterBtnOnPress(ITUWidget* widget, char* param);
extern bool TrialRunInputOutputItemRadBoxOnPress(ITUWidget* widget, char* param);
extern bool TrialRunInputOutputPMBtnOnPress(ITUWidget* widget, char* param);
extern bool TrialRunInputOutputRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool TrialRunAddrUpdateSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool TrialRunAddrUpdateSARadioBoxOnPress(ITUWidget* widget, char* param);
extern bool TrialRunAddrUpdatePMBtnOnPress(ITUWidget* widget, char* param);
extern bool TrialRunAddrUpdateValueEnterBtnOnPress(ITUWidget* widget, char* param);
extern bool TrialRunAddrUpdateRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool TrialRunAddrUpdateBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool TrialRunAddrInitEnterBtnOnPress(ITUWidget* widget, char* param);
extern bool TrialRunAddrInitRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool TrialRunModelSelInitEnterBtnOnPress(ITUWidget* widget, char* param);
extern bool TrialRunModelSelInitRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool TrialRunBackgroundBtnOnPress(ITUWidget* widget, char* param);

extern bool AirFlowOnEnter(ITUWidget* widget, char* param);
extern bool AirFlowAutoChkBoxOnPress(ITUWidget* widget, char* param);
extern bool AirFlowHTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool AirFlowVTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool AirFlowRadioBoxOnPress(ITUWidget* widget, char* param);

extern bool AiOnEnter(ITUWidget* widget, char* param);
extern bool AiRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool AiAutoCheckBoxOnPress(ITUWidget* widget, char* param);

//extern bool BindOnEnter(ITUWidget* widget, char* param);

extern bool Hlink_send(ITUWidget* widget, char* param);

extern bool SettingWiFiPasswordOnEnter(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordEnUpperCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordEnterButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordBackButtonOnPress(ITUWidget* widget, char* param);

extern bool ConsultBackBackgroundBtnOnPress(ITUWidget* widget, char* param);
extern bool DownloadBackBackgroundBtnOnPress(ITUWidget* widget, char* param);




ITUActionFunction actionFunctions[] =
{
	"Hlink_send", Hlink_send, //for hlink send , use para as input
	
	"LogoOnEnter", LogoOnEnter,
	
	"MainOnEnter", MainOnEnter,
	"MainOnTimer", MainOnTimer,
	"MainCoverFlowOnChanged", MainCoverFlowOnChanged,
	//"MainRModeRadBoxOnPress", MainRModeRadBoxOnPress,
	//"MainRMoreModeBackgroundBtnOnPress", MainRMoreModeBackgroundBtnOnPress,
	"MainHumidityBtnOnPress",MainHumidityBtnOnPress,
	"MainTempBtnOnPress", MainTempBtnOnPress,
	"MainLStatusChkBoxOnMouseUp", MainLStatusChkBoxOnMouseUp,
	"MainTopIndLightTrackBarOnChanged", MainTopIndLightTrackBarOnChanged,
	"MainTopScreenLightTrackBarOnChanged", MainTopScreenLightTrackBarOnChanged,
	"MainTopAutoChkBoxOnPress", MainTopAutoChkBoxOnPress,
	"MainAirForceTrackBarOnChanged", MainAirForceTrackBarOnChanged,
	//"MainAirForceAutoChkBoxOnPress", MainAirForceAutoChkBoxOnPress,
	//"MainRAICheckBoxOnPress", MainRAICheckBoxOnPress,
	"MainCModeShowButtonOnPress", MainCModeShowButtonOnPress,
	//"MainOnSlideDown", MainOnSlideDown,
	//"MainOnSlideUp", MainOnSlideUp,

	"WarningOnEnter", WarningOnEnter,
	"WarningResetBtnOnPress", WarningResetBtnOnPress,

	"SetTemp1OnEnter", SetTemp1OnEnter,
	"SetTemp1BackgroundBtnOnPress", SetTemp1BackgroundBtnOnPress,
	"SetTemp1WheelOnChanged", SetTemp1WheelOnChanged,

	"SetTempOnEnter", SetTempOnEnter,
	"SetTempSetBtnOnPress", SetTempSetBtnOnPress,
	"SetTempWheelOnChanged", SetTempWheelOnChanged,

	"SetHumidityOnEnter", SetHumidityOnEnter,
	"SetHumiditySetBtnOnPress", SetHumiditySetBtnOnPress,
	"SetHumidityWheelOnChanged", SetHumidityWheelOnChanged,

	"MoreModeOnEnter", MoreModeOnEnter,
	"MoreModeRadioBoxOnMouseUp", MoreModeRadioBoxOnMouseUp,

	"InductionSettingOnEnter", InductionSettingOnEnter,
	"InductionSettingTrackBarOnChanged", InductionSettingTrackBarOnChanged,
	"InductionSettingActRadBoxOnPress", InductionSettingActRadBoxOnPress,
	"InductionSettingRadBoxOnPress", InductionSettingRadBoxOnPress,

	"InductionOnEnter", InductionOnEnter,
	"InductionRadBoxOnPress", InductionRadBoxOnPress,

	"FilterOnEnter", FilterOnEnter,
	"FilterOkBtnOnPress", FilterOkBtnOnPress,
	
	//"EmergencyOnEnter", EmergencyOnEnter,
	"EmergencyBackgroundBtnOnPress", EmergencyBackgroundBtnOnPress,

	"ScreenLockOnEnter", ScreenLockOnEnter,
	"ScreenLockOnTimer", ScreenLockOnTimer,

	"TimingOnEnter", TimingOnEnter,
	"TimingOnLeave", TimingOnLeave,
	"TimingOnTimer", TimingOnTimer,
	"TimingRadioBoxOnPress", TimingRadioBoxOnPress,
	"TimingSingleSettingSaveBtnOnPress", TimingSingleSettingSaveBtnOnPress,
	"TimingSingleSettingBtnOnPress", TimingSingleSettingBtnOnPress,
	"TimingSingleSettingHrWheelOnChanged", TimingSingleSettingHrWheelOnChanged,
	"TimingSingleChkBoxOnPress", TimingSingleChkBoxOnPress,
	"TimingPowerOffSingleSettingSaveBtnOnPress", TimingPowerOffSingleSettingSaveBtnOnPress,
	"TimingPowerOffSingleSettingBtnOnMouseUp", TimingPowerOffSingleSettingBtnOnMouseUp,
	"TimingPowerOffSingleSettingHrWheelOnChanged", TimingPowerOffSingleSettingHrWheelOnChanged,
	"TimingPowerOffSingleChkBoxOnPress", TimingPowerOffSingleChkBoxOnPress,
	"TimingWeekSettingBtnOnMouseUp", TimingWeekSettingBtnOnMouseUp,
	"TimingWeekSettingTempSaveBtnOnPress", TimingWeekSettingTempSaveBtnOnPress,
	"TimingWeekSettingDaySaveBtnOnPress", TimingWeekSettingDaySaveBtnOnPress,
	"TimingWeekSettingTimeSaveBtnOnPress", TimingWeekSettingTimeSaveBtnOnPress,
	"TimingWeekSettingRadBoxOnPress", TimingWeekSettingRadBoxOnPress,
	"TimingWeekSettingSaveBtnOnPress", TimingWeekSettingSaveBtnOnPress,
	"TimingWeekSettingTimeBtnOnPress", TimingWeekSettingTimeBtnOnPress,
	"TimingWeekSettingDayBtnOnPress", TimingWeekSettingDayBtnOnPress,
	"TimingWeekSettingTempBtnOnPress", TimingWeekSettingTempBtnOnPress,
	"TimingWeekAddBtnOnMouseUp", TimingWeekAddBtnOnMouseUp,
	"TimingWeekChkBoxOnPress", TimingWeekChkBoxOnPress,
	"TimingWeekSlideBtnOnSlideLeft", TimingWeekSlideBtnOnSlideLeft,
	"TimingWeekSlideBtnOnSlideRight", TimingWeekSlideBtnOnSlideRight,
	"TimingWeekDeleteBtnOnPress", TimingWeekDeleteBtnOnPress,
	"TimingRestSettingSaveBtnOnPress", TimingRestSettingSaveBtnOnPress,
	"TimingRestAddBtnOnMouseUp", TimingRestAddBtnOnMouseUp,
	"TimingRestChkBoxOnPress", TimingRestChkBoxOnPress,
	"TimingRestSettingWheelOnChanged", TimingRestSettingWheelOnChanged,
	"TimingRestSlideBtnOnSlideLeft", TimingRestSlideBtnOnSlideLeft,
	"TimingRestSlideBtnOnSlideRight", TimingRestSlideBtnOnSlideRight,
	"TimingRestDeleteBtnOnPress", TimingRestDeleteBtnOnPress,
	"TimingBackBackgroundBtnOnPress", TimingBackBackgroundBtnOnPress,

	"SettingOnEnter", SettingOnEnter,
	"SettingOnTimer", SettingOnTimer,
	"SettingTimeSaveBtnOnPress", SettingTimeSaveBtnOnPress,
	"SettingTimeWheelOnChanged", SettingTimeWheelOnChanged,
	"SettingTimeBtnOnMouseUp", SettingTimeBtnOnMouseUp,
	"SettingTimeAutoChkBoxOnPress", SettingTimeAutoChkBoxOnPress,
	"SettingLightBtnOnMouseUp", SettingLightBtnOnMouseUp,
	"SettingLightBtnOnSlideUp", SettingLightBtnOnSlideUp,
	"SettingScreenLightTrackBarOnChanged", SettingScreenLightTrackBarOnChanged,
	"SettingIndLightTrackBarOnChanged", SettingIndLightTrackBarOnChanged,
	"SettingLightAutoCheckBoxOnPress", SettingLightAutoCheckBoxOnPress,
	"SettingScreenLockBtnOnMouseUp", SettingScreenLockBtnOnMouseUp,
	"SettingScreenLockSaveBtnOnPress", SettingScreenLockSaveBtnOnPress,
	"SettingScreenLockRadBoxOnPress", SettingScreenLockRadBoxOnPress,
	"SettingWarningLightCheckBoxOnPress", SettingWarningLightCheckBoxOnPress,
	"SettingWiFiBtnOnMouseUp", SettingWiFiBtnOnMouseUp,
	"SettingWiFiSubBtnOnPress", SettingWiFiSubBtnOnPress,
	"SettingWiFiOpenCheckBoxOnPress", SettingWiFiOpenCheckBoxOnPress,
	"SettingRestoreAllSettingBtnOnMouseUp", SettingRestoreAllSettingBtnOnMouseUp,
	"SettingRestoreSettingBtnOnMouseUp", SettingRestoreSettingBtnOnMouseUp,
	"SettingRestoreEnterBtnOnPress", SettingRestoreEnterBtnOnPress,
	"SettingExaminePasswordBtnOnMouseUp", SettingExaminePasswordBtnOnMouseUp,
	"SettingExamineButtonOnMouseUp", SettingExamineButtonOnMouseUp,
	"SettingKeySoundCheckBoxOnPress", SettingKeySoundCheckBoxOnPress,
	"SettingOnlineUpdateBtnOnMouseUp", SettingOnlineUpdateBtnOnMouseUp,
	"SettingOnlineUpdateDownloadBtnOnPress", SettingOnlineUpdateDownloadBtnOnPress,
	"SettingOnlineUpdatingBtnOnPress", SettingOnlineUpdatingBtnOnPress,
	"SettingBackBackgroundBtnOnPress", SettingBackBackgroundBtnOnPress,


	"ExamineOnEnter", ExamineOnEnter,
	"ExamineSubEnterBtnOnPress", ExamineSubEnterBtnOnPress,
	"ExamineSubRadioBoxOnPress", ExamineSubRadioBoxOnPress,
	"ExamineBtnOnMouseUp", ExamineBtnOnMouseUp,
	"ExamineShowBtnOnMouseUp", ExamineShowBtnOnMouseUp,
	"ExamineModelTestBtnOnMouseUp", ExamineModelTestBtnOnMouseUp,
	"ExamineShowModelRadioBoxOnPress", ExamineShowModelRadioBoxOnPress,
	"ExamineModelTestRadioBoxOnPress", ExamineModelTestRadioBoxOnPress,
	"ExamineModelTestEnterBtnOnPress", ExamineModelTestEnterBtnOnPress,
	"ExamineBackgroundBtnOnPress", ExamineBackgroundBtnOnPress,

	"PowerOffOnEnter", PowerOffOnEnter,
	"PowerOffOnTimer", PowerOffOnTimer,
	"PowerOffPowerOnBtnOnPress", PowerOffPowerOnBtnOnPress,
	"PowerOffTopIndLightTrackBarOnChanged", PowerOffTopIndLightTrackBarOnChanged,
	"PowerOffTopScreenLightTrackBarOnChanged", PowerOffTopScreenLightTrackBarOnChanged,
	"PowerOffTopAutoChkBoxOnPress", PowerOffTopAutoChkBoxOnPress,
	"PowerOffDisinfectEnterBtnOnPress", PowerOffDisinfectEnterBtnOnPress,
	"PowerOffDisinfectStopEnterBtnOnPress", PowerOffDisinfectStopEnterBtnOnPress,
	"PowerOffCleanEnterBtnOnPress", PowerOffCleanEnterBtnOnPress,
	"PowerOffCleanStopEnterBtnOnPress", PowerOffCleanStopEnterBtnOnPress,

	"TrialRunOnEnter", TrialRunOnEnter,
	"TrialRunTrialSaveBtnOnPress", TrialRunTrialSaveBtnOnPress,
	"TrialRunTrialAirForceRadBoxOnPress", TrialRunTrialAirForceRadBoxOnPress,
	"TrialRunTrialAirForceEnterBtnOnPress", TrialRunTrialAirForceEnterBtnOnPress,
	"TrialRuntTrialTimeSaveBackgroundBtnOnPress", TrialRuntTrialTimeSaveBackgroundBtnOnPress,
	"TrialRuntTrialFreqSaveBackgroundBtnOnPress", TrialRuntTrialFreqSaveBackgroundBtnOnPress,
	"TrialRunModelSelectRadioBoxOnPress", TrialRunModelSelectRadioBoxOnPress,
	"TrialRunModelSelectEnterBtnOnPress", TrialRunModelSelectEnterBtnOnPress,
	"TrialRunModelSelectItemRadBoxOnPress", TrialRunModelSelectItemRadBoxOnPress,
	"TrialRunModelSelectPMBtnOnPress", TrialRunModelSelectPMBtnOnPress,
	"TrialRunInputOutputRadioBoxOnPress", TrialRunInputOutputRadioBoxOnPress,
	"TrialRunInputOutputEnterBtnOnPress", TrialRunInputOutputEnterBtnOnPress,
	"TrialRunInputOutputItemRadBoxOnPress", TrialRunInputOutputItemRadBoxOnPress,
	"TrialRunInputOutputPMBtnOnPress", TrialRunInputOutputPMBtnOnPress,
	"TrialRunAddrUpdateSaveBtnOnPress", TrialRunAddrUpdateSaveBtnOnPress,
	"TrialRunAddrUpdateSARadioBoxOnPress", TrialRunAddrUpdateSARadioBoxOnPress,
	"TrialRunAddrUpdatePMBtnOnPress", TrialRunAddrUpdatePMBtnOnPress,
	"TrialRunAddrUpdateValueEnterBtnOnPress", TrialRunAddrUpdateValueEnterBtnOnPress,
	"TrialRunAddrUpdateRadioBoxOnPress", TrialRunAddrUpdateRadioBoxOnPress,
	"TrialRunAddrUpdateBtnOnMouseUp", TrialRunAddrUpdateBtnOnMouseUp,
	"TrialRunAddrInitRadioBoxOnPress", TrialRunAddrInitRadioBoxOnPress,
	"TrialRunAddrInitEnterBtnOnPress", TrialRunAddrInitEnterBtnOnPress,
	"TrialRunModelSelInitRadioBoxOnPress", TrialRunModelSelInitRadioBoxOnPress,
	"TrialRunModelSelInitEnterBtnOnPress", TrialRunModelSelInitEnterBtnOnPress,
	"TrialRunBackgroundBtnOnPress", TrialRunBackgroundBtnOnPress,

	"AirFlowOnEnter", AirFlowOnEnter,
	"AirFlowAutoChkBoxOnPress", AirFlowAutoChkBoxOnPress,
	"AirFlowHTrackBarOnChanged", AirFlowHTrackBarOnChanged,
	"AirFlowVTrackBarOnChanged", AirFlowVTrackBarOnChanged,
	"AirFlowRadioBoxOnPress", AirFlowRadioBoxOnPress,

	"AiOnEnter", AiOnEnter,
	"AiRadioBoxOnPress", AiRadioBoxOnPress,
	"AiAutoCheckBoxOnPress", AiAutoCheckBoxOnPress,

	//"BindOnEnter", BindOnEnter,

	"SettingWiFiPasswordOnEnter", SettingWiFiPasswordOnEnter,
	"SettingWiFiPasswordEnUpperCheckBoxOnPress", SettingWiFiPasswordEnUpperCheckBoxOnPress,
	"SettingWiFiPasswordEnterButtonOnPress", SettingWiFiPasswordEnterButtonOnPress,
	"SettingWiFiPasswordBackButtonOnPress", SettingWiFiPasswordBackButtonOnPress,

	"ConsultBackBackgroundBtnOnPress", ConsultBackBackgroundBtnOnPress,
	"DownloadBackBackgroundBtnOnPress", DownloadBackBackgroundBtnOnPress,



    NULL, NULL
};
