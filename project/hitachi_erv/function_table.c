#include "ite/itu.h"

extern bool LogoOnEnter(ITUWidget* widget, char* param);

extern bool MainOnEnter(ITUWidget* widget, char* param);
extern bool MainOnTimer(ITUWidget* widget, char* param);
extern bool MainCoverFlowOnChanged(ITUWidget* widget, char* param);
extern bool MainCModeButtonOnMouseUp(ITUWidget* widget, char* param);
extern bool MainCAirForceButtonOnMouseUp(ITUWidget* widget, char* param);
extern bool MainAttachButtonOnMouseUp(ITUWidget* widget, char* param);
extern bool MainAirForceHideButtonOnMouseUp(ITUWidget* widget, char* param);
extern bool MainCModeSelectHideButtonOnSlideDown(ITUWidget* widget, char* param);
extern bool MainAttachHideButtonOnSlideDown(ITUWidget* widget, char* param);
extern bool MainCModeRadBoxOnMouseUp(ITUWidget* widget, char* param);
extern bool MainCAttachRadBoxOnMouseUp(ITUWidget* widget, char* param);
extern bool MainAirForceTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool MainTopIndLightTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool MainTopScreenLightTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool MainTopAutoChkBoxOnPress(ITUWidget* widget, char* param);
extern bool MainCModeShowButtonOnPress(ITUWidget* widget, char* param);
extern bool MainLQualityRadioBoxOnMouseUp(ITUWidget* widget, char* param);
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

extern bool FilterOnEnter(ITUWidget* widget, char* param);
extern bool FilterOkBtnOnPress(ITUWidget* widget, char* param);

extern bool EmergencyBackgroundBtnOnPress(ITUWidget* widget, char* param);

extern bool ScreenLockOnEnter(ITUWidget* widget, char* param);
extern bool ScreenLockOnTimer(ITUWidget* widget, char* param);

extern bool TimingOnEnter(ITUWidget* widget, char* param);
extern bool TimingOnLeave(ITUWidget* widget, char* param);
extern bool TimingOnTimer(ITUWidget* widget, char* param);
extern bool TimingSingleSettingSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingSingleSettingBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool TimingSingleChkBoxOnPress(ITUWidget* widget, char* param);
extern bool TimingSingleSettingHrWheelOnChanged(ITUWidget* widget, char* param);
extern bool TimingWeekSettingBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool TimingWeekSettingDaySaveBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingWeekSettingTimeSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingWeekSettingRadBoxOnPress(ITUWidget* widget, char* param);
extern bool TimingWeekSettingSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingWeekSettingTimeBtnOnPress(ITUWidget* widget, char* param);
extern bool TimingWeekSettingDayBtnOnPress(ITUWidget* widget, char* param);
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

extern bool SettingOnEnter(ITUWidget* widget, char* param);
extern bool SettingTimeSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool SettingTimeWheelOnChanged(ITUWidget* widget, char* param);
extern bool SettingTimeBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingTimeAutoChkBoxOnPress(ITUWidget* widget, char* param);
//extern bool SettingLightBtnOnMouseUp(ITUWidget* widget, char* param);
//extern bool SettingLightBtnOnSlideUp(ITUWidget* widget, char* param);
extern bool SettingScreenLockBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingScreenLockSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool SettingScreenLockRadBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingWarningLightCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingWiFiSubBtnOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiOpenCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingRestoreAllSettingBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingRestoreSettingBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingExaminePasswordBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingExamineButtonOnMouseUp(ITUWidget* widget, char* param);
extern bool SettingKeySoundCheckBoxOnMouseUp(ITUWidget* widget, char* param);

extern bool ExamineOnEnter(ITUWidget* widget, char* param);
extern bool ExamineSubEnterBtnOnPress(ITUWidget* widget, char* param);
extern bool ExamineSubRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool ExamineBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool ExamineShowBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool ExamineShowModelRadioBoxOnPress(ITUWidget* widget, char* param);
//extern bool ExamineModelTestRadioBoxOnPress(ITUWidget* widget, char* param);
//extern bool ExamineModelTestEnterBtnOnPress(ITUWidget* widget, char* param);
extern bool ExamineBackgroundBtnOnPress(ITUWidget* widget, char* param);

extern bool PowerOffOnEnter(ITUWidget* widget, char* param);
extern bool PowerOffOnTimer(ITUWidget* widget, char* param);
//extern bool PowerOffTimingSaveBtnOnPress(ITUWidget* widget, char* param);
//extern bool PowerOffTimingWheelOnChanged(ITUWidget* widget, char* param);
extern bool PowerOffPowerOnBtnOnPress(ITUWidget* widget, char* param);
extern bool PowerOffTopIndLightTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool PowerOffTopScreenLightTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool PowerOffTopAutoChkBoxOnPress(ITUWidget* widget, char* param);

extern bool TrialRunOnEnter(ITUWidget* widget, char* param);
//extern bool TrialRunTrialSaveBtnOnPress(ITUWidget* widget, char* param);
//extern bool TrialRunTrialAirForceEnterBtnOnPress(ITUWidget* widget, char* param);
//extern bool TrialRunTrialAirForceRadBoxOnPress(ITUWidget* widget, char* param);
//extern bool TrialRuntTrialTimeSaveBackgroundBtnOnPress(ITUWidget* widget, char* param);
//extern bool TrialRuntTrialFreqSaveBackgroundBtnOnPress(ITUWidget* widget, char* param);
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
extern bool SpecialBackgroundBtnOnPress(ITUWidget* widget, char* param);

extern bool SettingWiFiPasswordOnEnter(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordEnUpperCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordEnterButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordBackButtonOnPress(ITUWidget* widget, char* param);

extern bool ConsultBackBackgroundBtnOnPress(ITUWidget* widget, char* param);
extern bool DownloadBackBackgroundBtnOnPress(ITUWidget* widget, char* param);

extern bool PowerOffTimingOnEnter(ITUWidget* widget, char* param);
extern bool PowerOffTimingOnLeave(ITUWidget* widget, char* param);
extern bool PowerOffTimingOnTimer(ITUWidget* widget, char* param);
extern bool PowerOffTimingSingleSettingSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool PowerOffTimingSingleSettingBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool PowerOffTimingSingleChkBoxOnPress(ITUWidget* widget, char* param);
extern bool PowerOffTimingSingleSettingHrWheelOnChanged(ITUWidget* widget, char* param);
extern bool PowerOffTimingWeekSettingBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool PowerOffTimingWeekSettingDaySaveBtnOnPress(ITUWidget* widget, char* param);
extern bool PowerOffTimingWeekSettingTimeSaveBtnOnPress(ITUWidget* widget, char* param);
//extern bool PowerOffTimingWeekSettingRadBoxOnPress(ITUWidget* widget, char* param);
extern bool PowerOffTimingWeekSettingSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool PowerOffTimingWeekSettingTimeBtnOnPress(ITUWidget* widget, char* param);
extern bool PowerOffTimingWeekSettingDayBtnOnPress(ITUWidget* widget, char* param);
extern bool PowerOffTimingWeekAddBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool PowerOffTimingWeekChkBoxOnPress(ITUWidget* widget, char* param);
extern bool PowerOffTimingRestSettingSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool PowerOffTimingRestAddBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool PowerOffTimingRestChkBoxOnPress(ITUWidget* widget, char* param);
extern bool PowerOffTimingRestSettingWheelOnChanged(ITUWidget* widget, char* param);
extern bool PowerOffTimingWeekSlideBtnOnSlideLeft(ITUWidget* widget, char* param);
extern bool PowerOffTimingWeekSlideBtnOnSlideRight(ITUWidget* widget, char* param);
extern bool PowerOffTimingWeekDeleteBtnOnPress(ITUWidget* widget, char* param);
extern bool PowerOffTimingRestSlideBtnOnSlideLeft(ITUWidget* widget, char* param);
extern bool PowerOffTimingRestSlideBtnOnSlideRight(ITUWidget* widget, char* param);
extern bool PowerOffTimingRestDeleteBtnOnPress(ITUWidget* widget, char* param);

extern bool PowerOffSettingOnEnter(ITUWidget* widget, char* param);
extern bool PowerOffSettingTimeSaveBtnOnPress(ITUWidget* widget, char* param);
extern bool PowerOffSettingTimeWheelOnChanged(ITUWidget* widget, char* param);
extern bool PowerOffSettingTimeBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool PowerOffSettingTimeAutoChkBoxOnPress(ITUWidget* widget, char* param);
//extern bool PowerOffSettingLightBtnOnMouseUp(ITUWidget* widget, char* param);
//extern bool PowerOffSettingLightBtnOnSlideUp(ITUWidget* widget, char* param);
//extern bool PowerOffSettingScreenLockBtnOnMouseUp(ITUWidget* widget, char* param);
//extern bool PowerOffSettingScreenLockSaveBtnOnPress(ITUWidget* widget, char* param);
//extern bool PowerOffSettingScreenLockRadBoxOnPress(ITUWidget* widget, char* param);
extern bool PowerOffSettingWarningLightCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool PowerOffSettingWiFiBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool PowerOffSettingWiFiSubBtnOnPress(ITUWidget* widget, char* param);
extern bool PowerOffSettingWiFiOpenCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool PowerOffSettingRestoreAllSettingBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool PowerOffSettingRestoreSettingBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool PowerOffSettingExaminePasswordBtnOnMouseUp(ITUWidget* widget, char* param);
extern bool PowerOffSettingExamineButtonOnMouseUp(ITUWidget* widget, char* param);
extern bool PowerOffSettingKeySoundCheckBoxOnMouseUp(ITUWidget* widget, char* param);

ITUActionFunction actionFunctions[] =
{
	"LogoOnEnter", LogoOnEnter,

	"MainOnEnter", MainOnEnter,
	"MainOnTimer", MainOnTimer,
	"MainCoverFlowOnChanged", MainCoverFlowOnChanged,
	"MainCModeButtonOnMouseUp", MainCModeButtonOnMouseUp,
	"MainCAirForceButtonOnMouseUp", MainCAirForceButtonOnMouseUp,
	"MainAttachButtonOnMouseUp", MainAttachButtonOnMouseUp,
	"MainAirForceHideButtonOnMouseUp", MainAirForceHideButtonOnMouseUp,
	"MainCModeSelectHideButtonOnSlideDown", MainCModeSelectHideButtonOnSlideDown,
	"MainAttachHideButtonOnSlideDown", MainAttachHideButtonOnSlideDown,
	"MainCModeRadBoxOnMouseUp", MainCModeRadBoxOnMouseUp,
	"MainCAttachRadBoxOnMouseUp", MainCAttachRadBoxOnMouseUp,
	"MainAirForceTrackBarOnChanged", MainAirForceTrackBarOnChanged,
	"MainTopIndLightTrackBarOnChanged", MainTopIndLightTrackBarOnChanged,
	"MainTopScreenLightTrackBarOnChanged", MainTopScreenLightTrackBarOnChanged,
	"MainTopAutoChkBoxOnPress", MainTopAutoChkBoxOnPress,
	"MainCModeShowButtonOnPress", MainCModeShowButtonOnPress,
	"MainLQualityRadioBoxOnMouseUp", MainLQualityRadioBoxOnMouseUp,
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

	"FilterOnEnter", FilterOnEnter,
	"FilterOkBtnOnPress", FilterOkBtnOnPress,
	
	"EmergencyBackgroundBtnOnPress", EmergencyBackgroundBtnOnPress,

	"ScreenLockOnEnter", ScreenLockOnEnter,
	"ScreenLockOnTimer", ScreenLockOnTimer,

	"TimingOnEnter", TimingOnEnter,
	"TimingOnLeave", TimingOnLeave,
	"TimingOnTimer", TimingOnTimer,
	"TimingSingleSettingSaveBtnOnPress", TimingSingleSettingSaveBtnOnPress,
	"TimingSingleSettingBtnOnMouseUp", TimingSingleSettingBtnOnMouseUp,
	"TimingSingleSettingHrWheelOnChanged", TimingSingleSettingHrWheelOnChanged,
	"TimingSingleChkBoxOnPress", TimingSingleChkBoxOnPress,
	"TimingWeekSettingBtnOnMouseUp", TimingWeekSettingBtnOnMouseUp,
	"TimingWeekSettingDaySaveBtnOnPress", TimingWeekSettingDaySaveBtnOnPress,
	"TimingWeekSettingTimeSaveBtnOnPress", TimingWeekSettingTimeSaveBtnOnPress,
	"TimingWeekSettingRadBoxOnPress", TimingWeekSettingRadBoxOnPress,
	"TimingWeekSettingSaveBtnOnPress", TimingWeekSettingSaveBtnOnPress,
	"TimingWeekSettingTimeBtnOnPress", TimingWeekSettingTimeBtnOnPress,
	"TimingWeekSettingDayBtnOnPress", TimingWeekSettingDayBtnOnPress,
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

	"SettingOnEnter", SettingOnEnter,
	"SettingTimeSaveBtnOnPress", SettingTimeSaveBtnOnPress,
	"SettingTimeWheelOnChanged", SettingTimeWheelOnChanged,
	"SettingTimeBtnOnMouseUp", SettingTimeBtnOnMouseUp,
	"SettingTimeAutoChkBoxOnPress", SettingTimeAutoChkBoxOnPress,
	//"SettingLightBtnOnMouseUp", SettingLightBtnOnMouseUp,
	//"SettingLightBtnOnSlideUp", SettingLightBtnOnSlideUp,
	"SettingScreenLockBtnOnMouseUp", SettingScreenLockBtnOnMouseUp,
	"SettingScreenLockSaveBtnOnPress", SettingScreenLockSaveBtnOnPress,
	"SettingScreenLockRadBoxOnPress", SettingScreenLockRadBoxOnPress,
	"SettingWarningLightCheckBoxOnPress", SettingWarningLightCheckBoxOnPress,
	"SettingWiFiBtnOnMouseUp", SettingWiFiBtnOnMouseUp,
	"SettingWiFiSubBtnOnPress", SettingWiFiSubBtnOnPress,
	"SettingWiFiOpenCheckBoxOnPress", SettingWiFiOpenCheckBoxOnPress,
	"SettingRestoreAllSettingBtnOnMouseUp", SettingRestoreAllSettingBtnOnMouseUp,
	"SettingRestoreSettingBtnOnMouseUp", SettingRestoreSettingBtnOnMouseUp,
	"SettingExaminePasswordBtnOnMouseUp", SettingExaminePasswordBtnOnMouseUp,
	"SettingExamineButtonOnMouseUp", SettingExamineButtonOnMouseUp,
	"SettingKeySoundCheckBoxOnMouseUp", SettingKeySoundCheckBoxOnMouseUp,

	"ExamineOnEnter", ExamineOnEnter,
	"ExamineSubEnterBtnOnPress", ExamineSubEnterBtnOnPress,
	"ExamineSubRadioBoxOnPress", ExamineSubRadioBoxOnPress,
	"ExamineBtnOnMouseUp", ExamineBtnOnMouseUp,
	"ExamineShowBtnOnMouseUp", ExamineShowBtnOnMouseUp,
	"ExamineShowModelRadioBoxOnPress", ExamineShowModelRadioBoxOnPress,
	//"ExamineModelTestRadioBoxOnPress", ExamineModelTestRadioBoxOnPress,
	//"ExamineModelTestEnterBtnOnPress", ExamineModelTestEnterBtnOnPress,
	"ExamineBackgroundBtnOnPress", ExamineBackgroundBtnOnPress,

	"PowerOffOnEnter", PowerOffOnEnter,
	"PowerOffOnTimer", PowerOffOnTimer,
	//"PowerOffTimingSaveBtnOnPress", PowerOffTimingSaveBtnOnPress,
	//"PowerOffTimingWheelOnChanged", PowerOffTimingWheelOnChanged,
	"PowerOffPowerOnBtnOnPress", PowerOffPowerOnBtnOnPress,
	"PowerOffTopIndLightTrackBarOnChanged", PowerOffTopIndLightTrackBarOnChanged,
	"PowerOffTopScreenLightTrackBarOnChanged", PowerOffTopScreenLightTrackBarOnChanged,
	"PowerOffTopAutoChkBoxOnPress", PowerOffTopAutoChkBoxOnPress,

	"TrialRunOnEnter", TrialRunOnEnter,
	//"TrialRunTrialSaveBtnOnPress", TrialRunTrialSaveBtnOnPress,
	//"TrialRunTrialAirForceRadBoxOnPress", TrialRunTrialAirForceRadBoxOnPress,
	//"TrialRunTrialAirForceEnterBtnOnPress", TrialRunTrialAirForceEnterBtnOnPress,
	//"TrialRuntTrialTimeSaveBackgroundBtnOnPress", TrialRuntTrialTimeSaveBackgroundBtnOnPress,
	//"TrialRuntTrialFreqSaveBackgroundBtnOnPress", TrialRuntTrialFreqSaveBackgroundBtnOnPress,
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
	"SpecialBackgroundBtnOnPress", SpecialBackgroundBtnOnPress,

	"SettingWiFiPasswordOnEnter", SettingWiFiPasswordOnEnter,
	"SettingWiFiPasswordEnUpperCheckBoxOnPress", SettingWiFiPasswordEnUpperCheckBoxOnPress,
	"SettingWiFiPasswordEnterButtonOnPress", SettingWiFiPasswordEnterButtonOnPress,
	"SettingWiFiPasswordBackButtonOnPress", SettingWiFiPasswordBackButtonOnPress,

	"ConsultBackBackgroundBtnOnPress", ConsultBackBackgroundBtnOnPress,
	"DownloadBackBackgroundBtnOnPress", DownloadBackBackgroundBtnOnPress,

	"PowerOffTimingOnEnter", PowerOffTimingOnEnter,
	//"PowerOffTimingOnLeave", PowerOffTimingOnLeave,
	"PowerOffTimingOnTimer", PowerOffTimingOnTimer,
	"PowerOffTimingSingleSettingSaveBtnOnPress", PowerOffTimingSingleSettingSaveBtnOnPress,
	"PowerOffTimingSingleSettingBtnOnMouseUp", PowerOffTimingSingleSettingBtnOnMouseUp,
	"PowerOffTimingSingleSettingHrWheelOnChanged", PowerOffTimingSingleSettingHrWheelOnChanged,
	"PowerOffTimingSingleChkBoxOnPress", PowerOffTimingSingleChkBoxOnPress,
	"PowerOffTimingWeekSettingBtnOnMouseUp", PowerOffTimingWeekSettingBtnOnMouseUp,
	"PowerOffTimingWeekSettingDaySaveBtnOnPress", PowerOffTimingWeekSettingDaySaveBtnOnPress,
	"PowerOffTimingWeekSettingTimeSaveBtnOnPress", PowerOffTimingWeekSettingTimeSaveBtnOnPress,
	//"PowerOffTimingWeekSettingRadBoxOnPress", PowerOffTimingWeekSettingRadBoxOnPress,
	"PowerOffTimingWeekSettingSaveBtnOnPress", PowerOffTimingWeekSettingSaveBtnOnPress,
	"PowerOffTimingWeekSettingTimeBtnOnPress", PowerOffTimingWeekSettingTimeBtnOnPress,
	"PowerOffTimingWeekSettingDayBtnOnPress", PowerOffTimingWeekSettingDayBtnOnPress,
	"PowerOffTimingWeekAddBtnOnMouseUp", PowerOffTimingWeekAddBtnOnMouseUp,
	"PowerOffTimingWeekChkBoxOnPress", PowerOffTimingWeekChkBoxOnPress,
	"PowerOffTimingWeekSlideBtnOnSlideLeft", PowerOffTimingWeekSlideBtnOnSlideLeft,
	"PowerOffTimingWeekSlideBtnOnSlideRight", PowerOffTimingWeekSlideBtnOnSlideRight,
	"PowerOffTimingWeekDeleteBtnOnPress", PowerOffTimingWeekDeleteBtnOnPress,
	"PowerOffTimingRestSettingSaveBtnOnPress", PowerOffTimingRestSettingSaveBtnOnPress,
	"PowerOffTimingRestAddBtnOnMouseUp", PowerOffTimingRestAddBtnOnMouseUp,
	"PowerOffTimingRestChkBoxOnPress", PowerOffTimingRestChkBoxOnPress,
	"PowerOffTimingRestSettingWheelOnChanged", PowerOffTimingRestSettingWheelOnChanged,
	"PowerOffTimingRestSlideBtnOnSlideLeft", PowerOffTimingRestSlideBtnOnSlideLeft,
	"PowerOffTimingRestSlideBtnOnSlideRight", PowerOffTimingRestSlideBtnOnSlideRight,
	"PowerOffTimingRestDeleteBtnOnPress", PowerOffTimingRestDeleteBtnOnPress,

	"PowerOffSettingOnEnter", PowerOffSettingOnEnter,
	"PowerOffSettingTimeSaveBtnOnPress", PowerOffSettingTimeSaveBtnOnPress,
	"PowerOffSettingTimeWheelOnChanged", PowerOffSettingTimeWheelOnChanged,
	"PowerOffSettingTimeBtnOnMouseUp", PowerOffSettingTimeBtnOnMouseUp,
	"PowerOffSettingTimeAutoChkBoxOnPress", PowerOffSettingTimeAutoChkBoxOnPress,
	//"PowerOffSettingLightBtnOnMouseUp", PowerOffSettingLightBtnOnMouseUp,
	//"PowerOffSettingLightBtnOnSlideUp", PowerOffSettingLightBtnOnSlideUp,
	//"PowerOffSettingScreenLockBtnOnMouseUp", PowerOffSettingScreenLockBtnOnMouseUp,
	//"PowerOffSettingScreenLockSaveBtnOnPress", PowerOffSettingScreenLockSaveBtnOnPress,
	//"PowerOffSettingScreenLockRadBoxOnPress", PowerOffSettingScreenLockRadBoxOnPress,
	"PowerOffSettingWarningLightCheckBoxOnPress", PowerOffSettingWarningLightCheckBoxOnPress,
	"PowerOffSettingWiFiBtnOnMouseUp", PowerOffSettingWiFiBtnOnMouseUp,
	"PowerOffSettingWiFiSubBtnOnPress", PowerOffSettingWiFiSubBtnOnPress,
	"PowerOffSettingWiFiOpenCheckBoxOnPress", PowerOffSettingWiFiOpenCheckBoxOnPress,
	"PowerOffSettingRestoreAllSettingBtnOnMouseUp", PowerOffSettingRestoreAllSettingBtnOnMouseUp,
	"PowerOffSettingRestoreSettingBtnOnMouseUp", PowerOffSettingRestoreSettingBtnOnMouseUp,
	"PowerOffSettingExaminePasswordBtnOnMouseUp", PowerOffSettingExaminePasswordBtnOnMouseUp,
	"PowerOffSettingExamineButtonOnMouseUp", PowerOffSettingExamineButtonOnMouseUp,
	"PowerOffSettingKeySoundCheckBoxOnMouseUp", PowerOffSettingKeySoundCheckBoxOnMouseUp,

    NULL, NULL
};
