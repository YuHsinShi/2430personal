#include "ite/itu.h"


extern bool MainOnEnter(ITUWidget* widget, char* param);
extern bool MainOnTimer(ITUWidget* widget, char* param);
extern bool MainCoverFlowOnChanged(ITUWidget* widget, char* param);
//extern bool MainRModeRadBoxOnPress(ITUWidget* widget, char* param);
//extern bool MainRMoreModeBackgroundBtnOnPress(ITUWidget* widget, char* param);
extern bool MainHumidityBtnOnPress(ITUWidget* widget, char* param);
extern bool MainTempBtnOnPress(ITUWidget* widget, char* param);
extern bool MainLStatusChkBoxOnPress(ITUWidget* widget, char* param);
extern bool MainTopIndLightTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool MainTopScreenLightTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool MainTopAutoChkBoxOnPress(ITUWidget* widget, char* param);
extern bool MainAirForceTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool MainAirForceAutoChkBoxOnPress(ITUWidget* widget, char* param);
//extern bool MainRAICheckBoxOnPress(ITUWidget* widget, char* param);
extern bool MainCModeShowButtonOnPress(ITUWidget* widget, char* param);

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
extern bool MoreModeRadioBoxOnPress(ITUWidget* widget, char* param);

extern bool InductionSettingOnEnter(ITUWidget* widget, char* param);
extern bool InductionSettingTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool InductionSettingActRadBoxOnPress(ITUWidget* widget, char* param);
extern bool InductionSettingRadBoxOnPress(ITUWidget* widget, char* param);

extern bool InductionOnEnter(ITUWidget* widget, char* param);
extern bool InductionRadBoxOnPress(ITUWidget* widget, char* param);

extern bool FilterOkBtnOnPress(ITUWidget* widget, char* param);

extern bool EmergencyBackgroundBtnOnPress(ITUWidget* widget, char* param);

extern bool ScreenLockOnEnter(ITUWidget* widget, char* param);
extern bool ScreenLockOnTimer(ITUWidget* widget, char* param);

extern bool TimingOnEnter(ITUWidget* widget, char* param);
extern bool TimingOnLeave(ITUWidget* widget, char* param);
extern bool TimingSingleSettingSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingSingleSettingBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingSingleChkBoxOnPress(ITUWidget* widget, char* param);
extern bool TimingSingleSettingHrWheelOnChanged(ITUWidget* widget, char* param);
extern bool TimingWeekSettingBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingWeekSettingTempSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingWeekSettingDaySaveBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingWeekSettingTimeSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingWeekSettingRadBoxOnPress(ITUWidget* widget, char* param);
extern bool TimingWeekSettingSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingWeekSettingTimeBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingWeekSettingDayBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingWeekSettingTempBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingWeekAddBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingWeekChkBoxOnPress(ITUWidget* widget, char* param);
extern bool TimingRestSettingSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingRestAddBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingRestChkBoxOnPress(ITUWidget* widget, char* param);
extern bool TimingRestSettingWheelOnChanged(ITUWidget* widget, char* param);
extern bool TimingWeekSlideBtnOnSlideLeft(ITUWidget* widget, char* param);
extern bool TimingWeekSlideBtnOnSlideRight(ITUWidget* widget, char* param);
extern bool TimingWeekDeleteBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingRestSlideBtnOnSlideLeft(ITUWidget* widget, char* param);
extern bool TimingRestSlideBtnOnSlideRight(ITUWidget* widget, char* param);
extern bool TimingRestDeleteBtnOnPress(ITUWidget* widget, char* param);

extern bool SettingOnEnter(ITUWidget* widget, char* param);
extern bool SettingTimeSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool SettingTimeWheelOnChanged(ITUWidget* widget, char* param);
extern bool SettingTimeBtnOnPress(ITUWidget* widget, char* param);
extern bool SettingLightBtnOnPress(ITUWidget* widget, char* param);
extern bool SettingLightBtnOnSlideUp(ITUWidget* widget, char* param);
extern bool SettingScreenLockBtnOnPress(ITUWidget* widget, char* param);
extern bool SettingScreenLockSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool SettingScreenLockRadBoxOnPress(ITUWidget* widget, char* param);

extern bool ExamineOnEnter(ITUWidget* widget, char* param);
extern bool ExamineSubEnterBtnOnPress(ITUWidget* widget, char* param);
extern bool ExamineSubRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool ExamineBtnOnPress(ITUWidget* widget, char* param);
extern bool ExamineShowModelRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool ExamineModelTestRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool ExamineModelTestEnterBtnOnPress(ITUWidget* widget, char* param);

extern bool PowerOffOnEnter(ITUWidget* widget, char* param);
extern bool PowerOffOnTimer(ITUWidget* widget, char* param);
extern bool PowerOffTimingSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool PowerOffModeRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool PowerOffTempSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool PowerOffAirForceTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool PowerOffAirForceAutoChkBoxOnPress(ITUWidget* widget, char* param);
extern bool PowerOffPreHumidityTrackBarOnChanged(ITUWidget* widget, char* param);

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
extern bool TrialRunAddrUpdateBtnOnPress(ITUWidget* widget, char* param);

extern bool AirFlowOnEnter(ITUWidget* widget, char* param);
extern bool AirFlowAutoChkBoxOnPress(ITUWidget* widget, char* param);
extern bool AirFlowHTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool AirFlowVTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool AirFlowRadioBoxOnPress(ITUWidget* widget, char* param);

extern bool AiOnEnter(ITUWidget* widget, char* param);
extern bool AiRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool AiAutoCheckBoxOnPress(ITUWidget* widget, char* param);

ITUActionFunction actionFunctions[] =
{
	"MainOnEnter", MainOnEnter,
	"MainOnTimer", MainOnTimer,
	"MainCoverFlowOnChanged", MainCoverFlowOnChanged,
	//"MainRModeRadBoxOnPress", MainRModeRadBoxOnPress,
	//"MainRMoreModeBackgroundBtnOnPress", MainRMoreModeBackgroundBtnOnPress,
	"MainHumidityBtnOnPress",MainHumidityBtnOnPress,
	"MainTempBtnOnPress", MainTempBtnOnPress,
	"MainLStatusChkBoxOnPress", MainLStatusChkBoxOnPress,
	"MainTopIndLightTrackBarOnChanged", MainTopIndLightTrackBarOnChanged,
	"MainTopScreenLightTrackBarOnChanged", MainTopScreenLightTrackBarOnChanged,
	"MainTopAutoChkBoxOnPress", MainTopAutoChkBoxOnPress,
	"MainAirForceTrackBarOnChanged", MainAirForceTrackBarOnChanged,
	"MainAirForceAutoChkBoxOnPress", MainAirForceAutoChkBoxOnPress,
	//"MainRAICheckBoxOnPress", MainRAICheckBoxOnPress,
	"MainCModeShowButtonOnPress", MainCModeShowButtonOnPress,

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
	"MoreModeRadioBoxOnPress", MoreModeRadioBoxOnPress,

	"InductionSettingOnEnter", InductionSettingOnEnter,
	"InductionSettingTrackBarOnChanged", InductionSettingTrackBarOnChanged,
	"InductionSettingActRadBoxOnPress", InductionSettingActRadBoxOnPress,
	"InductionSettingRadBoxOnPress", InductionSettingRadBoxOnPress,

	"InductionOnEnter", InductionOnEnter,
	"InductionRadBoxOnPress", InductionRadBoxOnPress,


	"FilterOkBtnOnPress", FilterOkBtnOnPress,
	
	"EmergencyBackgroundBtnOnPress", EmergencyBackgroundBtnOnPress,

	"ScreenLockOnEnter", ScreenLockOnEnter,
	"ScreenLockOnTimer", ScreenLockOnTimer,

	"TimingOnEnter", TimingOnEnter,
	"TimingOnLeave", TimingOnLeave,
	"TimingSingleSettingSaveBtnOnPress", TimingSingleSettingSaveBtnOnPress,
	"TimingSingleSettingBtnOnPress", TimingSingleSettingBtnOnPress,
	"TimingSingleSettingHrWheelOnChanged", TimingSingleSettingHrWheelOnChanged,
	"TimingSingleChkBoxOnPress", TimingSingleChkBoxOnPress,
	"TimingWeekSettingBtnOnPress", TimingWeekSettingBtnOnPress,
	"TimingWeekSettingTempSaveBtnOnPress", TimingWeekSettingTempSaveBtnOnPress,
	"TimingWeekSettingDaySaveBtnOnPress", TimingWeekSettingDaySaveBtnOnPress,
	"TimingWeekSettingTimeSaveBtnOnPress", TimingWeekSettingTimeSaveBtnOnPress,
	"TimingWeekSettingRadBoxOnPress", TimingWeekSettingRadBoxOnPress,
	"TimingWeekSettingSaveBtnOnPress", TimingWeekSettingSaveBtnOnPress,
	"TimingWeekSettingTimeBtnOnPress", TimingWeekSettingTimeBtnOnPress,
	"TimingWeekSettingDayBtnOnPress", TimingWeekSettingDayBtnOnPress,
	"TimingWeekSettingTempBtnOnPress", TimingWeekSettingTempBtnOnPress,
	"TimingWeekAddBtnOnPress", TimingWeekAddBtnOnPress,
	"TimingWeekChkBoxOnPress", TimingWeekChkBoxOnPress,
	"TimingWeekSlideBtnOnSlideLeft", TimingWeekSlideBtnOnSlideLeft,
	"TimingWeekSlideBtnOnSlideRight", TimingWeekSlideBtnOnSlideRight,
	"TimingWeekDeleteBtnOnPress", TimingWeekDeleteBtnOnPress,
	"TimingRestSettingSaveBtnOnPress", TimingRestSettingSaveBtnOnPress,
	"TimingRestAddBtnOnPress", TimingRestAddBtnOnPress,
	"TimingRestChkBoxOnPress", TimingRestChkBoxOnPress,
	"TimingRestSettingWheelOnChanged", TimingRestSettingWheelOnChanged,
	"TimingRestSlideBtnOnSlideLeft", TimingRestSlideBtnOnSlideLeft,
	"TimingRestSlideBtnOnSlideRight", TimingRestSlideBtnOnSlideRight,
	"TimingRestDeleteBtnOnPress", TimingRestDeleteBtnOnPress,

	"SettingOnEnter", SettingOnEnter,
	"SettingTimeSaveBtnOnPress", SettingTimeSaveBtnOnPress,
	"SettingTimeWheelOnChanged", SettingTimeWheelOnChanged,
	"SettingTimeBtnOnPress", SettingTimeBtnOnPress,
	"SettingLightBtnOnPress", SettingLightBtnOnPress,
	"SettingLightBtnOnSlideUp", SettingLightBtnOnSlideUp,
	"SettingScreenLockBtnOnPress", SettingScreenLockBtnOnPress,
	"SettingScreenLockSaveBtnOnPress", SettingScreenLockSaveBtnOnPress,
	"SettingScreenLockRadBoxOnPress", SettingScreenLockRadBoxOnPress,

	"ExamineOnEnter", ExamineOnEnter,
	"ExamineSubEnterBtnOnPress", ExamineSubEnterBtnOnPress,
	"ExamineSubRadioBoxOnPress", ExamineSubRadioBoxOnPress,
	"ExamineBtnOnPress", ExamineBtnOnPress,
	"ExamineShowModelRadioBoxOnPress", ExamineShowModelRadioBoxOnPress,
	"ExamineModelTestRadioBoxOnPress", ExamineModelTestRadioBoxOnPress,
	"ExamineModelTestEnterBtnOnPress", ExamineModelTestEnterBtnOnPress,

	"PowerOffOnEnter", PowerOffOnEnter,
	"PowerOffOnTimer", PowerOffOnTimer,
	"PowerOffTimingSaveBtnOnPress", PowerOffTimingSaveBtnOnPress,
	"PowerOffModeRadioBoxOnPress", PowerOffModeRadioBoxOnPress,
	"PowerOffTempSaveBtnOnPress", PowerOffTempSaveBtnOnPress, 
	"PowerOffAirForceTrackBarOnChanged", PowerOffAirForceTrackBarOnChanged,
	"PowerOffAirForceAutoChkBoxOnPress", PowerOffAirForceAutoChkBoxOnPress,
	"PowerOffPreHumidityTrackBarOnChanged", PowerOffPreHumidityTrackBarOnChanged,

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
	"TrialRunAddrUpdateBtnOnPress", TrialRunAddrUpdateBtnOnPress,

	"AirFlowOnEnter", AirFlowOnEnter,
	"AirFlowAutoChkBoxOnPress", AirFlowAutoChkBoxOnPress,
	"AirFlowHTrackBarOnChanged", AirFlowHTrackBarOnChanged,
	"AirFlowVTrackBarOnChanged", AirFlowVTrackBarOnChanged,
	"AirFlowRadioBoxOnPress", AirFlowRadioBoxOnPress,

	"AiOnEnter", AiOnEnter,
	"AiRadioBoxOnPress", AiRadioBoxOnPress,
	"AiAutoCheckBoxOnPress", AiAutoCheckBoxOnPress,

    NULL, NULL
};
