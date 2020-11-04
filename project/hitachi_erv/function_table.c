#include "ite/itu.h"


extern bool MainOnEnter(ITUWidget* widget, char* param);
extern bool MainOnTimer(ITUWidget* widget, char* param);
extern bool MainCoverFlowOnChanged(ITUWidget* widget, char* param);
extern bool MainCModeRadBoxOnMouseUp(ITUWidget* widget, char* param);
extern bool MainAirForceTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool MainAirForceAutoChkBoxOnPress(ITUWidget* widget, char* param);
extern bool MainRAttachBackgroundBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool MainTopIndLightTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool MainTopScreenLightTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool MainTopAutoChkBoxOnPress(ITUWidget* widget, char* param);
extern bool MainCModeShowButtonOnPress(ITUWidget* widget, char* param);
extern bool MainLQualityRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool MainLDWChangeBtnOnPress(ITUWidget* widget, char* param);
extern bool MainLQualityInPMBtnOnPress(ITUWidget* widget, char* param);
extern bool MainLQualityTVOCBtnOnPress(ITUWidget* widget, char* param);
extern bool MainLQualityCO2BtnOnPress(ITUWidget* widget, char* param);
extern bool MainLQualityOutPMBtnOnPress(ITUWidget* widget, char* param);
extern bool MainLQualityInPMWBtnOnPress(ITUWidget* widget, char* param);
extern bool MainLQualityTVOCWBtnOnPress(ITUWidget* widget, char* param);
extern bool MainLQualityCO2WBtnOnPress(ITUWidget* widget, char* param);
extern bool MainLQualityOutPMWBtnOnPress(ITUWidget* widget, char* param);

extern bool WarningResetBtnOnPress(ITUWidget* widget, char* param);

extern bool AttachOnEnter(ITUWidget* widget, char* param);
extern bool AttachCheckBoxOnMouseUp(ITUWidget* widget, char* param);

extern bool CommonToolOnEnter(ITUWidget* widget, char* param);

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
extern bool SettingTimeBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingLightBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingLightBtnOnSlideUp(ITUWidget* widget, char* param);
extern bool SettingScreenLockBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingScreenLockSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool SettingScreenLockRadBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingWiFiSubBtnOnPress(ITUWidget* widget, char* param);

extern bool ExamineOnEnter(ITUWidget* widget, char* param);
extern bool ExamineSubEnterBtnOnPress(ITUWidget* widget, char* param);
extern bool ExamineSubRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool ExamineBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool ExamineShowModelRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool ExamineModelTestRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool ExamineModelTestEnterBtnOnPress(ITUWidget* widget, char* param);

extern bool PowerOffOnEnter(ITUWidget* widget, char* param);
extern bool PowerOffOnTimer(ITUWidget* widget, char* param);
extern bool PowerOffTimingSaveBtnOnPress(ITUWidget* widget, char* param);

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


extern bool SpecialOnEnter(ITUWidget* widget, char* param);
extern bool SpecialOnTimer(ITUWidget* widget, char* param);
extern bool SpecialAirForceRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool SpecialStaticPressureManualPMBtnOnPress(ITUWidget* widget, char* param);
extern bool SpecialStaticPressureManualEnterBtnOnPress(ITUWidget* widget, char* param);
extern bool SpecialStaticPressureEnterBtnOnPress(ITUWidget* widget, char* param);
extern bool SpecialStaticPressureRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool SpecialPositivePressureManualEnterBtnOnPress(ITUWidget* widget, char* param);
extern bool SpecialPositivePressureManualPMBtnOnPress(ITUWidget* widget, char* param);
extern bool SpecialPositivePressureManualRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool SpecialMicroPositivePressureRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool SpecialPositivePressureRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool SpecialStaticPressureBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool SpecialPositivePressureBtnOnMouseUp(ITUWidget* widget, char* param);


extern bool SettingWiFiPasswordOnEnter(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordEnUpperCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordEnterButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordBackButtonOnPress(ITUWidget* widget, char* param);

ITUActionFunction actionFunctions[] =
{
	"MainOnEnter", MainOnEnter,
	"MainOnTimer", MainOnTimer,
	"MainCoverFlowOnChanged", MainCoverFlowOnChanged,
	"MainCModeRadBoxOnMouseUp", MainCModeRadBoxOnMouseUp,
	"MainAirForceTrackBarOnChanged", MainAirForceTrackBarOnChanged,
	"MainAirForceAutoChkBoxOnPress", MainAirForceAutoChkBoxOnPress,
	"MainRAttachBackgroundBtnOnMouseUp", MainRAttachBackgroundBtnOnMouseUp,
	"MainTopIndLightTrackBarOnChanged", MainTopIndLightTrackBarOnChanged,
	"MainTopScreenLightTrackBarOnChanged", MainTopScreenLightTrackBarOnChanged,
	"MainTopAutoChkBoxOnPress", MainTopAutoChkBoxOnPress,
	"MainCModeShowButtonOnPress", MainCModeShowButtonOnPress,
	"MainLQualityRadioBoxOnPress", MainLQualityRadioBoxOnPress,
	"MainLDWChangeBtnOnPress", MainLDWChangeBtnOnPress,
	"MainLQualityInPMBtnOnPress", MainLQualityInPMBtnOnPress,
	"MainLQualityTVOCBtnOnPress", MainLQualityTVOCBtnOnPress,
	"MainLQualityCO2BtnOnPress", MainLQualityCO2BtnOnPress,
	"MainLQualityOutPMBtnOnPress", MainLQualityOutPMBtnOnPress,
	"MainLQualityInPMWBtnOnPress", MainLQualityInPMWBtnOnPress,
	"MainLQualityTVOCWBtnOnPress", MainLQualityTVOCWBtnOnPress,
	"MainLQualityCO2WBtnOnPress", MainLQualityCO2WBtnOnPress,
	"MainLQualityOutPMWBtnOnPress", MainLQualityOutPMWBtnOnPress,

	"WarningResetBtnOnPress", WarningResetBtnOnPress,

	"AttachOnEnter", AttachOnEnter,
	"AttachCheckBoxOnMouseUp", AttachCheckBoxOnMouseUp,

	"CommonToolOnEnter", CommonToolOnEnter,

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
	"SettingTimeBtnOnMouseUp", SettingTimeBtnOnMouseUp,
	"SettingLightBtnOnMouseUp", SettingLightBtnOnMouseUp,
	"SettingLightBtnOnSlideUp", SettingLightBtnOnSlideUp,
	"SettingScreenLockBtnOnMouseUp", SettingScreenLockBtnOnMouseUp,
	"SettingScreenLockSaveBtnOnPress", SettingScreenLockSaveBtnOnPress,
	"SettingScreenLockRadBoxOnPress", SettingScreenLockRadBoxOnPress,
	"SettingWiFiBtnOnMouseUp", SettingWiFiBtnOnMouseUp,
	"SettingWiFiSubBtnOnPress", SettingWiFiSubBtnOnPress,

	"ExamineOnEnter", ExamineOnEnter,
	"ExamineSubEnterBtnOnPress", ExamineSubEnterBtnOnPress,
	"ExamineSubRadioBoxOnPress", ExamineSubRadioBoxOnPress,
	"ExamineBtnOnMouseUp", ExamineBtnOnMouseUp,
	"ExamineShowModelRadioBoxOnPress", ExamineShowModelRadioBoxOnPress,
	"ExamineModelTestRadioBoxOnPress", ExamineModelTestRadioBoxOnPress,
	"ExamineModelTestEnterBtnOnPress", ExamineModelTestEnterBtnOnPress,

	"PowerOffOnEnter", PowerOffOnEnter,
	"PowerOffOnTimer", PowerOffOnTimer,
	"PowerOffTimingSaveBtnOnPress", PowerOffTimingSaveBtnOnPress,

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

	"SpecialOnEnter", SpecialOnEnter,
	"SpecialOnTimer", SpecialOnTimer,
	"SpecialAirForceRadioBoxOnPress", SpecialAirForceRadioBoxOnPress,
	"SpecialStaticPressureManualPMBtnOnPress", SpecialStaticPressureManualPMBtnOnPress,
	"SpecialStaticPressureManualEnterBtnOnPress", SpecialStaticPressureManualEnterBtnOnPress,
	"SpecialStaticPressureEnterBtnOnPress", SpecialStaticPressureEnterBtnOnPress,
	"SpecialStaticPressureRadioBoxOnPress", SpecialStaticPressureRadioBoxOnPress,
	"SpecialPositivePressureManualEnterBtnOnPress", SpecialPositivePressureManualEnterBtnOnPress,
	"SpecialPositivePressureManualPMBtnOnPress", SpecialPositivePressureManualPMBtnOnPress,
	"SpecialPositivePressureManualRadioBoxOnPress", SpecialPositivePressureManualRadioBoxOnPress,
	"SpecialMicroPositivePressureRadioBoxOnPress", SpecialMicroPositivePressureRadioBoxOnPress,
	"SpecialPositivePressureRadioBoxOnPress", SpecialPositivePressureRadioBoxOnPress,
	"SpecialStaticPressureBtnOnMouseUp", SpecialStaticPressureBtnOnMouseUp,
	"SpecialPositivePressureBtnOnMouseUp", SpecialPositivePressureBtnOnMouseUp,

	"SettingWiFiPasswordOnEnter", SettingWiFiPasswordOnEnter,
	"SettingWiFiPasswordEnUpperCheckBoxOnPress", SettingWiFiPasswordEnUpperCheckBoxOnPress,
	"SettingWiFiPasswordEnterButtonOnPress", SettingWiFiPasswordEnterButtonOnPress,
	"SettingWiFiPasswordBackButtonOnPress", SettingWiFiPasswordBackButtonOnPress,


    NULL, NULL
};
