#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"

#ifdef CFG_SHT20_ENABLE
    #include "sht20_sensor.h"
#endif

#ifdef CFG_NCP18_ENABLE
    #include "ncp18_sensor.h"
#endif


ITUCoverFlow* mainCoverFlow = 0;

ITUButton* mainLDWChangeButton = 0;
ITUSprite* mainLDWSprite = 0;
ITURadioBox* mainLQualityRadioBox[4] = { 0 };
ITUSprite* mainLQualitySprite = 0;
ITUText* mainLInPMDayText[6] = { 0 };
ITUText* mainLCO2DayText[6] = { 0 };
ITUText* mainLTVOCDayText[6] = { 0 };
ITUText* mainLOutPMDayText[6] = { 0 };
ITUIcon* mainLQualityInPMDayIcon = 0;
ITUSprite* mainLQualityInPMSprite[8] = { 0 };
ITUButton* mainLQualityInPMButton[8] = { 0 };
ITUSprite* mainLQualityInPMNumSprite = 0;
ITUText* mainLQualityInPMNumText[3] = { 0 };
ITUIcon* mainLQualityTVOCDayIcon = 0;
ITUSprite* mainLQualityTVOCSprite[8] = { 0 };
ITUButton* mainLQualityTVOCButton[8] = { 0 };
ITUSprite* mainLQualityTVOCNumSprite = 0;
ITUText* mainLQualityTVOCNumText[3] = { 0 };
ITUIcon* mainLQualityCO2DayIcon = 0;
ITUSprite* mainLQualityCO2Sprite[8] = { 0 };
ITUButton* mainLQualityCO2Button[8] = { 0 };
ITUSprite* mainLQualityCO2NumSprite = 0;
ITUText* mainLQualityCO2NumText[3] = { 0 };
ITUIcon* mainLQualityOutPMDayIcon = 0;
ITUSprite* mainLQualityOutPMSprite[8] = { 0 };
ITUButton* mainLQualityOutPMButton[8] = { 0 };
ITUSprite* mainLQualityOutPMNumSprite = 0;
ITUText* mainLQualityOutPMNumText[3] = { 0 };
ITUIcon* mainLQualityInPMWeekIcon = 0;
ITUSprite* mainLQualityInPMWSprite[7] = { 0 };
ITUButton* mainLQualityInPMWButton[7] = { 0 };
ITUSprite* mainLQualityInPMWNumSprite = 0;
ITUText* mainLQualityInPMWNumText[3] = { 0 };
ITUIcon* mainLQualityTVOCWeekIcon = 0;
ITUSprite* mainLQualityTVOCWSprite[7] = { 0 };
ITUButton* mainLQualityTVOCWButton[7] = { 0 };
ITUSprite* mainLQualityTVOCWNumSprite = 0;
ITUText* mainLQualityTVOCWNumText[3] = { 0 };
ITUIcon* mainLQualityCO2WeekIcon = 0;
ITUSprite* mainLQualityCO2WSprite[7] = { 0 };
ITUButton* mainLQualityCO2WButton[7] = { 0 };
ITUSprite* mainLQualityCO2WNumSprite = 0;
ITUText* mainLQualityCO2WNumText[3] = { 0 };
ITUIcon* mainLQualityOutPMWeekIcon = 0;
ITUSprite* mainLQualityOutPMWSprite[7] = { 0 };
ITUButton* mainLQualityOutPMWButton[7] = { 0 };
ITUSprite* mainLQualityOutPMWNumSprite = 0;
ITUText* mainLQualityOutPMWNumText[3] = { 0 };

ITUText* mainCInHumidityText = 0;
ITUText* mainCInTempText = 0;
ITUText* mainCInPM25Text = 0;
ITUText* mainCInCO2Text = 0;
ITUText* mainCInTVOCText = 0;
ITUText* mainCInHCHOText = 0;
ITUText* mainCOutHumidityText = 0;
ITUText* mainCOutTempText = 0;
ITUText* mainCOutPM25Text = 0;
ITUBackgroundButton* mainCFilterBackgroundButton = 0;
ITUBackgroundButton* mainCWarningBackgroundButton = 0;
//ITUIcon* mainCModeIcon = 0;
ITUButton* mainCModeButton = 0;
ITUSprite* mainModeTextSprite = 0;
ITUIcon* mainCModeIconSet[MODE_NUM] = { 0 };
//ITUIcon* mainCAttachIcon[ATTACH_NUM] = { 0 };

ITUButton* mainCAirForceButton = 0;
ITUSprite* mainAirForceTextSprite = 0;
ITUTrackBar* mainAirForceTrackBar = 0;
ITUText* mainAirForceValueText = 0;
ITUProgressBar* mainAirForceProgressBar = 0;
ITUCheckBox* mainAirForceAutoCheckBox = 0;
ITUSprite* mainAirForceSprite = 0;
ITUSprite* mainAirForceLineSprite = 0;
ITUIcon* mainAirForceTrackBarIcon = 0;
ITUIcon* mainBarIcon[2] = { 0 };
ITUBackground* mainAirForceTipShowBackground = 0;
ITUText* mainAirForceTipShowText = 0;

ITUCheckBox* mainRTimingCheckBox = 0;
ITUBackgroundButton* mainRAttachBackgroundButton = 0;
ITUIcon* mainRAttachIcon = 0;
ITUSprite* mainRAttachTextSprite = 0;
ITUIcon* mainRAttachBgBtnBg[2] = { 0 };
ITUIcon* mainRAttachIconSet[ATTACH_NUM + 2] = { 0 };

ITUSprite* mainSprite = 0;

ITUIcon* mainModeShowIcon[MODESHOW_NUM] = { 0 };
ITUButton* mainCModeShowButton = 0;
ITUIcon* mainModeShowSubIcon[MODESHOW_NUM] = { 0 };
ITUIcon* mainModeShowLineIcon[MODESHOW_NUM] = { 0 };
ITUIcon* mainModeShowLineIconSet[MODESHOW_NUM] = { 0 };
ITUText* mainModeShowText[MODESHOW_NUM] = { 0 };

ITUTrackBar*	mainTopIndLightTrackBar = 0;
ITUProgressBar*	mainTopIndLightProgressBar = 0;
ITUTrackBar*	mainTopScreenLightTrackBar = 0;
ITUProgressBar*	mainTopScreenLightProgressBar = 0;
ITUCheckBox*	mainTopAutoCheckBox = 0;
ITUIcon* mainTopBarBtnIcon[2] = { 0 };



static int sprite_index = 1;
extern int mode_show[MODESHOW_NUM] = { 0, 0, 0, 0, 0, 0, 0, 0 };
extern int attach_show[ATTACH_NUM] = { 0 };
static int attachNum = 0;
static int attachContainerLength = 0;
static int attachPosX = 0;
static int modeshowNum = 0;
static int modeshowPosX = 0;
extern bool warning_btn_show = true;
extern bool emergency_btn_show = true;
extern bool filter_btn_show = true;
extern int modeIndex = 0;
extern bool timingSet = false;
static int humidityInIndex = 62;
static int preHumidityInIndex = 0;
extern int temperatureInIndex = 29;
static int preTempInIndex = 0;
static int humidityOutIndex = 65;
static int preHumidityOutIndex = 0;
static int temperatureOutIndex = 23;
static int preTempOutIndex = 0;
extern int PM25In = 12;
extern int CO2In = 860;
extern int TVOCIn = 32;
extern double HCHOIn = 0.02;
static int PM25Out = 32;
static int DWSpriteIndex = 0;
static int QualitySpriteIndex = 0;
static int InPMDayValue[8] = { 25, 45, 36, 75, 50, 30, 42, 43 };
static int TVOCDayValue[8] = { 10, 0, 20, 10, 30, 20, 5, 10 };
static int CO2DayValue[8] = { 700, 690, 1150, 1863, 2500, 1120, 1000, 750 };
static int OutPMDayValue[8] = { 25, 45, 75, 220, 240, 100, 85, 95 };
static int InPMWeekValue[7] = { 25, 45, 75, 50, 30, 45, 50 };
static int TVOCWeekValue[7] = { 10, 0, 20, 10, 30, 20, 10 };
static int CO2WeekValue[7] = { 700, 658, 900, 1000, 850, 725, 750 };
static int OutPMWeekValue[7] = { 25, 45, 75, 220, 85, 300, 368 };
static int airForceIndex = 0;

static uint32_t gtTick = 0, gtLastTick = 0, gtRefreshTime = 3000;
static bool gtTickFirst = true;

#if defined(CFG_SHT20_ENABLE) || defined(CFG_NCP18_ENABLE)
    static float current_tmp_float = 0;
#endif

bool DayQualitySet(ITUWidget* widget, int index, int i0, int i1, int i2, int i3, int i4, int i5, int i6, int i7);
bool WeekQualitySet(ITUWidget* widget, int index, int i0, int i1, int i2, int i3, int i4, int i5, int i6);

void MainReset(void)
{
}
bool MainOnEnter(ITUWidget* widget, char* param)
{
	int wifi_established = -1;
	int i,x,y;
	char tmp[32];

	struct timeval tv;
	struct tm *tm;
	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);
	
	if (!mainCoverFlow)
	{
		mainCoverFlow = ituSceneFindWidget(&theScene, "mainCoverFlow");
		assert(mainCoverFlow);

		mainLDWChangeButton = ituSceneFindWidget(&theScene, "mainLDWChangeButton");
		assert(mainLDWChangeButton);

		mainLDWSprite = ituSceneFindWidget(&theScene, "mainLDWSprite");
		assert(mainLDWSprite);

		mainLQualitySprite = ituSceneFindWidget(&theScene, "mainLQualitySprite");
		assert(mainLQualitySprite);

		mainCInHumidityText = ituSceneFindWidget(&theScene, "mainCInHumidityText");
		assert(mainCInHumidityText);

		mainCInTempText = ituSceneFindWidget(&theScene, "mainCInTempText");
		assert(mainCInTempText);

		mainCInPM25Text = ituSceneFindWidget(&theScene, "mainCInPM25Text");
		assert(mainCInPM25Text);

		mainCInCO2Text = ituSceneFindWidget(&theScene, "mainCInCO2Text");
		assert(mainCInCO2Text);

		mainCInTVOCText = ituSceneFindWidget(&theScene, "mainCInTVOCText");
		assert(mainCInTVOCText);

		mainCInHCHOText = ituSceneFindWidget(&theScene, "mainCInHCHOText");
		assert(mainCInHCHOText);

		mainCOutHumidityText = ituSceneFindWidget(&theScene, "mainCOutHumidityText");
		assert(mainCOutHumidityText);

		mainCOutTempText = ituSceneFindWidget(&theScene, "mainCOutTempText");
		assert(mainCOutTempText);

		mainCOutPM25Text = ituSceneFindWidget(&theScene, "mainCOutPM25Text");
		assert(mainCOutPM25Text);

		mainCFilterBackgroundButton = ituSceneFindWidget(&theScene, "mainCFilterBackgroundButton");
		assert(mainCFilterBackgroundButton);

		mainCWarningBackgroundButton = ituSceneFindWidget(&theScene, "mainCWarningBackgroundButton");
		assert(mainCWarningBackgroundButton);

		mainCModeButton = ituSceneFindWidget(&theScene, "mainCModeButton");
		assert(mainCModeButton);
		
		mainRTimingCheckBox = ituSceneFindWidget(&theScene, "mainRTimingCheckBox");
		assert(mainRTimingCheckBox);

		mainRAttachBackgroundButton = ituSceneFindWidget(&theScene, "mainRAttachBackgroundButton");
		assert(mainRAttachBackgroundButton);

		mainRAttachIcon = ituSceneFindWidget(&theScene, "mainRAttachIcon");
		assert(mainRAttachIcon);

		mainRAttachTextSprite = ituSceneFindWidget(&theScene, "mainRAttachTextSprite");
		assert(mainRAttachTextSprite);

		mainSprite = ituSceneFindWidget(&theScene, "mainSprite");
		assert(mainSprite);

		mainCModeShowButton = ituSceneFindWidget(&theScene, "mainCModeShowButton");
		assert(mainCModeShowButton);

		mainModeTextSprite = ituSceneFindWidget(&theScene, "mainModeTextSprite");
		assert(mainModeTextSprite);

		mainTopIndLightTrackBar = ituSceneFindWidget(&theScene, "mainTopIndLightTrackBar");
		assert(mainTopIndLightTrackBar);

		mainTopIndLightProgressBar = ituSceneFindWidget(&theScene, "mainTopIndLightProgressBar");
		assert(mainTopIndLightProgressBar);

		mainTopScreenLightTrackBar = ituSceneFindWidget(&theScene, "mainTopScreenLightTrackBar");
		assert(mainTopScreenLightTrackBar);

		mainTopScreenLightProgressBar = ituSceneFindWidget(&theScene, "mainTopScreenLightProgressBar");
		assert(mainTopScreenLightProgressBar);

		mainTopAutoCheckBox = ituSceneFindWidget(&theScene, "mainTopAutoCheckBox");
		assert(mainTopAutoCheckBox);

		mainCAirForceButton = ituSceneFindWidget(&theScene, "mainCAirForceButton");
		assert(mainCAirForceButton);

		mainAirForceTrackBar = ituSceneFindWidget(&theScene, "mainAirForceTrackBar");
		assert(mainAirForceTrackBar);

		mainAirForceValueText = ituSceneFindWidget(&theScene, "mainAirForceValueText");
		assert(mainAirForceValueText);

		mainAirForceProgressBar = ituSceneFindWidget(&theScene, "mainAirForceProgressBar");
		assert(mainAirForceProgressBar);

		mainAirForceAutoCheckBox = ituSceneFindWidget(&theScene, "mainAirForceAutoCheckBox");
		assert(mainAirForceAutoCheckBox);

		mainAirForceSprite = ituSceneFindWidget(&theScene, "mainAirForceSprite");
		assert(mainAirForceSprite);

		mainAirForceLineSprite = ituSceneFindWidget(&theScene, "mainAirForceLineSprite");
		assert(mainAirForceLineSprite);

		mainAirForceTrackBarIcon = ituSceneFindWidget(&theScene, "mainAirForceTrackBarIcon");
		assert(mainAirForceTrackBarIcon);

		mainAirForceTipShowBackground = ituSceneFindWidget(&theScene, "mainAirForceTipShowBackground");
		assert(mainAirForceTipShowBackground);

		mainAirForceTipShowText = ituSceneFindWidget(&theScene, "mainAirForceTipShowText");
		assert(mainAirForceTipShowText);

		mainAirForceTextSprite = ituSceneFindWidget(&theScene, "mainAirForceTextSprite");
		assert(mainAirForceTextSprite);

		mainLQualityInPMDayIcon = ituSceneFindWidget(&theScene, "mainLQualityInPMDayIcon");
		assert(mainLQualityInPMDayIcon);

		mainLQualityTVOCDayIcon = ituSceneFindWidget(&theScene, "mainLQualityTVOCDayIcon");
		assert(mainLQualityTVOCDayIcon);

		mainLQualityCO2DayIcon = ituSceneFindWidget(&theScene, "mainLQualityCO2DayIcon");
		assert(mainLQualityCO2DayIcon);

		mainLQualityOutPMDayIcon = ituSceneFindWidget(&theScene, "mainLQualityOutPMDayIcon");
		assert(mainLQualityOutPMDayIcon);

		mainLQualityInPMWeekIcon = ituSceneFindWidget(&theScene, "mainLQualityInPMWeekIcon");
		assert(mainLQualityInPMWeekIcon);

		mainLQualityTVOCWeekIcon = ituSceneFindWidget(&theScene, "mainLQualityTVOCWeekIcon");
		assert(mainLQualityTVOCWeekIcon);

		mainLQualityCO2WeekIcon = ituSceneFindWidget(&theScene, "mainLQualityCO2WeekIcon");
		assert(mainLQualityCO2WeekIcon);

		mainLQualityOutPMWeekIcon = ituSceneFindWidget(&theScene, "mainLQualityOutPMWeekIcon");
		assert(mainLQualityOutPMWeekIcon);

		mainLQualityInPMNumSprite = ituSceneFindWidget(&theScene, "mainLQualityInPMNumSprite");
		assert(mainLQualityInPMNumSprite);

		mainLQualityTVOCNumSprite = ituSceneFindWidget(&theScene, "mainLQualityTVOCNumSprite");
		assert(mainLQualityTVOCNumSprite);

		mainLQualityCO2NumSprite = ituSceneFindWidget(&theScene, "mainLQualityCO2NumSprite");
		assert(mainLQualityCO2NumSprite);

		mainLQualityOutPMNumSprite = ituSceneFindWidget(&theScene, "mainLQualityOutPMNumSprite");
		assert(mainLQualityOutPMNumSprite);

		mainLQualityInPMWNumSprite = ituSceneFindWidget(&theScene, "mainLQualityInPMWNumSprite");
		assert(mainLQualityInPMWNumSprite);

		mainLQualityTVOCWNumSprite = ituSceneFindWidget(&theScene, "mainLQualityTVOCWNumSprite");
		assert(mainLQualityTVOCWNumSprite);

		mainLQualityCO2WNumSprite = ituSceneFindWidget(&theScene, "mainLQualityCO2WNumSprite");
		assert(mainLQualityCO2WNumSprite);

		mainLQualityOutPMWNumSprite = ituSceneFindWidget(&theScene, "mainLQualityOutPMWNumSprite");
		assert(mainLQualityOutPMWNumSprite);


		//for (i = 0; i < (ATTACH_NUM ); i++)
		//{
		//	sprintf(tmp, "mainCAttachIcon%d", i);
		//	mainCAttachIcon[i] = ituSceneFindWidget(&theScene, tmp);
		//	assert(mainCAttachIcon[i]);
		//}

		for (i = 0; i < (MODE_NUM); i++)
		{
			sprintf(tmp, "mainCModeIcon%d", i);
			mainCModeIconSet[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainCModeIconSet[i]);
		}

		for (i = 0; i < (ATTACH_NUM + 2); i++)
		{
			sprintf(tmp, "mainRAttachIcon%d", i);
			mainRAttachIconSet[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainRAttachIconSet[i]);
		}
		

		for (i = 0; i < MODESHOW_NUM; i++)
		{
			sprintf(tmp, "mainModeShowIcon%d", i);
			mainModeShowIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainModeShowIcon[i]);

			sprintf(tmp, "mainModeShowSubIcon%d", i);
			mainModeShowSubIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainModeShowSubIcon[i]);

			sprintf(tmp, "mainModeShowLineIcon%d", i);
			mainModeShowLineIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainModeShowLineIcon[i]);

			sprintf(tmp, "mainModeShowLineIconSet%d", i);
			mainModeShowLineIconSet[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainModeShowLineIconSet[i]);

			sprintf(tmp, "mainModeShowText%d", i);
			mainModeShowText[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainModeShowText[i]);
		}

		for (i = 0; i < 2; i++)
		{
			sprintf(tmp, "mainTopBarBtnIcon%d", i);
			mainTopBarBtnIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainTopBarBtnIcon[i]);

			sprintf(tmp, "mainRAttachBgBtnBg%d", i);
			mainRAttachBgBtnBg[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainRAttachBgBtnBg[i]);

			sprintf(tmp, "mainBarIcon%d", i);
			mainBarIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainBarIcon[i]);

		}

		for (i = 0; i < 4; i++)
		{
			sprintf(tmp, "mainLQualityRadioBox%d", i);
			mainLQualityRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityRadioBox[i]);
		}

		for (i = 0; i < 6; i++)
		{
			sprintf(tmp, "mainLInPMDayText%d", i);
			mainLInPMDayText[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLInPMDayText[i]);

			sprintf(tmp, "mainLCO2DayText%d", i);
			mainLCO2DayText[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLCO2DayText[i]);

			sprintf(tmp, "mainLTVOCDayText%d", i);
			mainLTVOCDayText[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLTVOCDayText[i]);

			sprintf(tmp, "mainLOutPMDayText%d", i);
			mainLOutPMDayText[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLOutPMDayText[i]);
		}

		for (i = 0; i < 8; i++)
		{
			sprintf(tmp, "mainLQualityInPMSprite%d", i);
			mainLQualityInPMSprite[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityInPMSprite[i]);

			sprintf(tmp, "mainLQualityTVOCSprite%d", i);
			mainLQualityTVOCSprite[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityTVOCSprite[i]);

			sprintf(tmp, "mainLQualityCO2Sprite%d", i);
			mainLQualityCO2Sprite[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityCO2Sprite[i]);

			sprintf(tmp, "mainLQualityOutPMSprite%d", i);
			mainLQualityOutPMSprite[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityOutPMSprite[i]);

			sprintf(tmp, "mainLQualityInPMButton%d", i);
			mainLQualityInPMButton[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityInPMButton[i]);

			sprintf(tmp, "mainLQualityTVOCButton%d", i);
			mainLQualityTVOCButton[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityTVOCButton[i]);

			sprintf(tmp, "mainLQualityCO2Button%d", i);
			mainLQualityCO2Button[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityCO2Button[i]);

			sprintf(tmp, "mainLQualityOutPMButton%d", i);
			mainLQualityOutPMButton[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityOutPMButton[i]);

		}

		for (i = 0; i < 7; i++)
		{
			sprintf(tmp, "mainLQualityInPMWSprite%d", i);
			mainLQualityInPMWSprite[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityInPMWSprite[i]);

			sprintf(tmp, "mainLQualityTVOCWSprite%d", i);
			mainLQualityTVOCWSprite[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityTVOCWSprite[i]);

			sprintf(tmp, "mainLQualityCO2WSprite%d", i);
			mainLQualityCO2WSprite[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityCO2WSprite[i]);

			sprintf(tmp, "mainLQualityOutPMWSprite%d", i);
			mainLQualityOutPMWSprite[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityOutPMWSprite[i]);

			sprintf(tmp, "mainLQualityInPMWButton%d", i);
			mainLQualityInPMWButton[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityInPMWButton[i]);

			sprintf(tmp, "mainLQualityTVOCWButton%d", i);
			mainLQualityTVOCWButton[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityTVOCWButton[i]);

			sprintf(tmp, "mainLQualityCO2WButton%d", i);
			mainLQualityCO2WButton[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityCO2WButton[i]);

			sprintf(tmp, "mainLQualityOutPMWButton%d", i);
			mainLQualityOutPMWButton[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityOutPMWButton[i]);
		}

		for (i = 0; i < 3; i++)
		{
			sprintf(tmp, "mainLQualityInPMNumText%d", i);
			mainLQualityInPMNumText[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityInPMNumText[i]);

			sprintf(tmp, "mainLQualityTVOCNumText%d", i);
			mainLQualityTVOCNumText[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityTVOCNumText[i]);

			sprintf(tmp, "mainLQualityCO2NumText%d", i);
			mainLQualityCO2NumText[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityCO2NumText[i]);

			sprintf(tmp, "mainLQualityOutPMNumText%d", i);
			mainLQualityOutPMNumText[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityOutPMNumText[i]);


			sprintf(tmp, "mainLQualityInPMWNumText%d", i);
			mainLQualityInPMWNumText[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityInPMWNumText[i]);

			sprintf(tmp, "mainLQualityTVOCWNumText%d", i);
			mainLQualityTVOCWNumText[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityTVOCWNumText[i]);

			sprintf(tmp, "mainLQualityCO2WNumText%d", i);
			mainLQualityCO2WNumText[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityCO2WNumText[i]);

			sprintf(tmp, "mainLQualityOutPMWNumText%d", i);
			mainLQualityOutPMWNumText[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityOutPMWNumText[i]);


		}

	}

	for (i = 0; i < 6; i++)
	{
		if (tm->tm_mday > 6)
			sprintf(tmp, "%d", tm->tm_mday - 6 + i);
		else
		{
			switch (tm->tm_mon)
			{
			case 0:
			case 1:
			case 3:
			case 5:
			case 7:
			case 8:
			case 10:
				sprintf(tmp, "%d", tm->tm_mday + 31 - 6 + i);
				break;
			case 4:
			case 6:
			case 9:
			case 11:
				sprintf(tmp, "%d", tm->tm_mday + 30 - 6 + i);
				break;
			case 2:
				if (tm->tm_year % 4 == 0)
					sprintf(tmp, "%d", tm->tm_mday + 29 - 6 + i);
				else
					sprintf(tmp, "%d", tm->tm_mday + 28 - 6 + i);
				break;

			}

		}

		ituTextSetString(mainLInPMDayText[i], tmp);
		ituTextSetString(mainLCO2DayText[i], tmp);
		ituTextSetString(mainLTVOCDayText[i], tmp);
		ituTextSetString(mainLOutPMDayText[i], tmp);		
	}

	ituSpriteGoto(mainSprite, sprite_index);

	sprintf(tmp, "%d", PM25In);
	ituTextSetString(mainCInPM25Text, tmp);

	sprintf(tmp, "%d", CO2In);
	ituTextSetString(mainCInCO2Text, tmp);

	sprintf(tmp, "%d", TVOCIn);
	ituTextSetString(mainCInTVOCText, tmp);

	sprintf(tmp, "%2.2f", HCHOIn);
	ituTextSetString(mainCInHCHOText, tmp);

	sprintf(tmp, "%d", PM25Out);
	ituTextSetString(mainCOutPM25Text, tmp);

	sprintf(tmp, "%d", temperatureOutIndex);
	ituTextSetString(mainCOutTempText, tmp);

	sprintf(tmp, "%d", humidityOutIndex);
	ituTextSetString(mainCOutHumidityText, tmp);


	ituWidgetSetVisible(mainCWarningBackgroundButton, warning_btn_show);
	ituWidgetSetVisible(mainCFilterBackgroundButton, filter_btn_show);

	attachPosX = 60;
	attachNum = 0;
	for (i = 0; i < ATTACH_NUM; i++)
	{
		if (attach_show[i])
		{
			//ituWidgetSetPosition(mainCAttachIcon[i], attachPosX, 0);
			//attachNum++;
			//attachPosX = attachPosX + 60;

			ituIconLinkSurface(mainRAttachIcon, mainRAttachIconSet[i]);
			ituIconLinkSurface(&mainRAttachBackgroundButton->bg.icon, mainRAttachBgBtnBg[1]);
			ituSpriteGoto(mainRAttachTextSprite, i);
		}
		else
		{
			//ituWidgetSetPosition(mainCAttachIcon[i], 0, 100);
		}

	}

	if (attachNum == 0)
	{
		ituIconLinkSurface(mainRAttachIcon, mainRAttachIconSet[5]);
		ituIconLinkSurface(&mainRAttachBackgroundButton->bg.icon, mainRAttachBgBtnBg[0]);
		ituSpriteGoto(mainRAttachTextSprite, 4);
	}

	ituSpriteGoto(mainModeTextSprite, modeIndex);
	ituIconLinkSurface(&mainCModeButton->bg.icon, mainCModeIconSet[modeIndex]);
	ituCheckBoxSetChecked(mainRTimingCheckBox, timingSet);

	modeshowPosX = 0;
	modeshowNum = 0;
	for (i = 0; i < MODESHOW_NUM; i++)
	{
		if (mode_show[i])
		{
			ituWidgetSetPosition(mainModeShowIcon[i], modeshowPosX, 0);
			ituWidgetSetPosition(mainModeShowSubIcon[i], modeshowPosX, 0);
			modeshowNum++;
			modeshowPosX = modeshowPosX + 46;
		}
		else
		{
			ituWidgetSetPosition(mainModeShowIcon[i], 0, 100);
			ituWidgetSetPosition(mainModeShowSubIcon[i], 0, 100);
		}

	}

	ituWidgetSetDimension(mainCModeShowButton, modeshowPosX, 28);

	ituWidgetSetVisible(mainLQualityInPMNumSprite, false);
	ituWidgetSetVisible(mainLQualityTVOCNumSprite, false);
	ituWidgetSetVisible(mainLQualityCO2NumSprite, false);
	ituWidgetSetVisible(mainLQualityOutPMNumSprite, false);
	ituWidgetSetVisible(mainLQualityInPMWNumSprite, false);
	ituWidgetSetVisible(mainLQualityTVOCWNumSprite, false);
	ituWidgetSetVisible(mainLQualityCO2WNumSprite, false);
	ituWidgetSetVisible(mainLQualityOutPMWNumSprite, false);

	sprintf(tmp, "%d", humidityInIndex);
	ituTextSetString(mainCInHumidityText, tmp);

	ituSpriteGoto(mainAirForceTextSprite, airForceIndex);
	//ituIconLinkSurface(&mainCAirForceButton->bg.icon, mainCAirForceIconSet[airForceIndex]);

	ituWidgetSetVisible(mainAirForceTipShowBackground, false);
	ituWidgetGetGlobalPosition(mainAirForceTrackBar->tip, &x, &y);
	ituWidgetSetPosition(mainAirForceTipShowBackground, x, y);
	sprintf(tmp, "%d", mainAirForceTrackBar->value);
	ituTextSetString(mainAirForceTipShowText, tmp);
	ituWidgetSetVisible(mainAirForceTipShowBackground, true);

	if (gtTickFirst)
	{
		gtTick = gtRefreshTime;
		gtTickFirst = false;
	}
	else
	{
		gtTick = SDL_GetTicks();
	}

	if (gtTick - gtLastTick >= gtRefreshTime)
	{
#ifdef CFG_SHT20_ENABLE
		current_tmp_float = SHT20_Detect();
		if (current_tmp_float > 0)
		{
			temperatureInIndex = (int) (current_tmp_float + 0.5);
		}
		else
		{
			temperatureInIndex = (int) (-current_tmp_float + 0.5);
			temperatureInIndex = -temperatureInIndex;
		}

		preTempInIndex = temperatureInIndex;
#else
#ifdef CFG_NCP18_ENABLE
		current_tmp_float = NCP18_Detect();
		if (current_tmp_float > 0)
		{
			temperatureInIndex = (int) (current_tmp_float + 0.5);
		}
		else
		{
			temperatureInIndex = (int) (-current_tmp_float + 0.5);
			temperatureInIndex = -temperatureInIndex;
		}

		preTempInIndex = temperatureInIndex;
#else
		temperatureInIndex = 29;//get from sensor
		preTempInIndex = temperatureInIndex;

		humidityInIndex = 62;//get from sensor
		preHumidityInIndex = humidityInIndex;
#endif
#endif

		gtLastTick = gtTick;
	}

	sprintf(tmp, "%d", temperatureInIndex);
	ituTextSetString(mainCInTempText, tmp);

	sprintf(tmp, "%d", humidityInIndex);
	ituTextSetString(mainCInHumidityText, tmp);



	if (theConfig.wifi_on_off == 1)
	{
#ifdef CFG_NET_WIFI_SDIO_NGPL
	    wifi_established = ioctl(ITP_DEVICE_WIFI_NGPL, ITP_IOCTL_IS_AVAIL, NULL);
#endif

	 //   if (wifi_established)
	 //       ituSpriteGoto(topWiFiSprite, 2); //0: wifi is off, 1: wifi is on but no connection, 2: connection is established
		//else
  //          ituSpriteGoto(topWiFiSprite, 1);
	}
	else
	{
		//ituSpriteGoto(topWiFiSprite, 0); //0: wifi is off, 1: wifi is on but no connection, 2: connection is established
	}

	return true;
}
bool MainOnTimer(ITUWidget* widget, char* param)
{
	bool ret = false;
	char tmp[32];

	uint32_t tick = 0;

	if (gtTickFirst)
	{
		gtTick = gtRefreshTime;
		gtTickFirst = false;
	}
	else
	{
		gtTick = SDL_GetTicks();
	}

	if (gtTick - gtLastTick >= gtRefreshTime)
	{
#ifdef CFG_SHT20_ENABLE
		current_tmp_float = SHT20_Detect();
		if (current_tmp_float > 0)
		{
			temperatureInIndex = (int) (current_tmp_float + 0.5);
		}
		else
		{
			temperatureInIndex = (int) (-current_tmp_float + 0.5);
			temperatureInIndex = -temperatureInIndex;
		}
#else
#ifdef CFG_NCP18_ENABLE
		current_tmp_float = NCP18_Detect();
		if (current_tmp_float > 0)
		{
			temperatureInIndex = (int) (current_tmp_float + 0.5);
		}
		else
		{
			temperatureInIndex = (int) (-current_tmp_float + 0.5);
			temperatureInIndex = -temperatureInIndex;
		}
#else
		temperatureInIndex = 29;//get from sensor
		humidityInIndex = 62;//get from sensor
#endif
#endif

		gtLastTick = gtTick;
	}

	if (preTempInIndex != temperatureInIndex)
	{
		sprintf(tmp, "%d", temperatureInIndex);
		ituTextSetString(mainCInTempText, tmp);

		preTempInIndex = temperatureInIndex;
		ret = true;
	}

	if (preHumidityInIndex != humidityInIndex)
	{
		sprintf(tmp, "%d", humidityInIndex);
		ituTextSetString(mainCInHumidityText, tmp);


		preHumidityInIndex = humidityInIndex;
		ret = true;
	}


	return ret;
}
bool MainCoverFlowOnChanged(ITUWidget* widget, char* param)
{
	int i;

	sprite_index = mainCoverFlow->focusIndex;
	ituSpriteGoto(mainSprite, sprite_index);

	if (sprite_index == 1)
	{

		//attachPosX = 60;
		//attachNum = 0;
		//for (i = 0; i < ATTACH_NUM; i++)
		//{
		//	if (attach_show[i])
		//	{
		//		ituWidgetSetPosition(mainCAttachIcon[i], attachPosX, 0);
		//		attachNum++;
		//		attachPosX = attachPosX + 60;
		//	}
		//	else
		//	{
		//		ituWidgetSetPosition(mainCAttachIcon[i], 0, 100);
		//	}

		//}
	}
	else if (sprite_index == 0)
	{
		ituWidgetSetVisible(mainLQualityInPMNumSprite, false);
		ituWidgetSetVisible(mainLQualityTVOCNumSprite, false);
		ituWidgetSetVisible(mainLQualityCO2NumSprite, false);
		ituWidgetSetVisible(mainLQualityOutPMNumSprite, false);
		ituWidgetSetVisible(mainLQualityInPMWNumSprite, false);
		ituWidgetSetVisible(mainLQualityTVOCWNumSprite, false);
		ituWidgetSetVisible(mainLQualityCO2WNumSprite, false);
		ituWidgetSetVisible(mainLQualityOutPMWNumSprite, false);

		ituSpriteGoto(mainLQualitySprite, 0);
		ituSpriteGoto(mainLDWSprite, 0);
		ituRadioBoxSetChecked(mainLQualityRadioBox[0], true);
		DayQualitySet(mainLQualityInPMDayIcon, 0, InPMDayValue[0], InPMDayValue[1], InPMDayValue[2], InPMDayValue[3], InPMDayValue[4], InPMDayValue[5], InPMDayValue[6], InPMDayValue[7]);
		
	}
	return true;
}

bool MainCModeRadBoxOnMouseUp(ITUWidget* widget, char* param)
{
	modeIndex = atoi(param);
	ituSpriteGoto(mainModeTextSprite, modeIndex);
	ituIconLinkSurface(&mainCModeButton->bg.icon, mainCModeIconSet[modeIndex]);


	return true;
}

bool MainRAttachBackgroundBtnOnMouseUp(ITUWidget* widget, char* param)
{
	ituIconLinkSurface(mainRAttachIcon, mainRAttachIconSet[4]);
	ituIconLinkSurface(&mainRAttachBackgroundButton->bg.icon, mainRAttachBgBtnBg[1]);

	return true;
}



bool MainTopIndLightTrackBarOnChanged(ITUWidget* widget, char* param)
{
	return true;
}

bool MainTopScreenLightTrackBarOnChanged(ITUWidget* widget, char* param)
{
	return true;
}

bool MainTopAutoChkBoxOnPress(ITUWidget* widget, char* param)
{
	if (ituCheckBoxIsChecked(mainTopAutoCheckBox))
	{

		ituIconLinkSurface(&mainTopScreenLightTrackBar->tracker->bg.icon, mainTopBarBtnIcon[0]);
		ituIconLinkSurface(&mainTopIndLightTrackBar->tracker->bg.icon, mainTopBarBtnIcon[0]);

		ituWidgetDisable(mainTopScreenLightTrackBar);
		ituWidgetSetVisible(mainTopScreenLightProgressBar, false);
		ituWidgetDisable(mainTopIndLightTrackBar);
		ituWidgetSetVisible(mainTopIndLightProgressBar, false);
	}
	else
	{

		ituIconLinkSurface(&mainTopScreenLightTrackBar->tracker->bg.icon, mainTopBarBtnIcon[1]);
		ituIconLinkSurface(&mainTopIndLightTrackBar->tracker->bg.icon, mainTopBarBtnIcon[1]);

		ituWidgetEnable(mainTopScreenLightTrackBar);
		ituWidgetSetVisible(mainTopScreenLightProgressBar, true);
		ituWidgetEnable(mainTopIndLightTrackBar);
		ituWidgetSetVisible(mainTopIndLightProgressBar, true);
	}
	return true;
}



bool MainCModeShowButtonOnPress(ITUWidget* widget, char* param)
{
	int i;

	int modeshowNumTmp = modeshowNum;
	for (i = 0; i < MODESHOW_NUM; i++)
	{
		if (mode_show[i])
		{
			ituIconLinkSurface(mainModeShowLineIcon[i], mainModeShowLineIconSet[modeshowNumTmp - 1]);
			ituWidgetSetPosition(mainModeShowLineIcon[i], ituWidgetGetX(mainModeShowSubIcon[i]), 0);
			ituWidgetSetPosition(mainModeShowText[i], ituWidgetGetX(mainModeShowSubIcon[i]), (modeshowNumTmp - 1) * 60);
			modeshowNumTmp--;

		}
		else
		{
			ituWidgetSetPosition(mainModeShowLineIcon[i], 0, 501);
			ituWidgetSetPosition(mainModeShowText[i], 0, 500);
		}
	}

	return true;
}


bool MainLQualityRadioBoxOnPress(ITUWidget* widget, char* param)
{
	ituWidgetSetVisible(mainLQualityInPMNumSprite, false);
	ituWidgetSetVisible(mainLQualityTVOCNumSprite, false);
	ituWidgetSetVisible(mainLQualityCO2NumSprite, false);
	ituWidgetSetVisible(mainLQualityOutPMNumSprite, false);
	ituWidgetSetVisible(mainLQualityInPMWNumSprite, false);
	ituWidgetSetVisible(mainLQualityTVOCWNumSprite, false);
	ituWidgetSetVisible(mainLQualityCO2WNumSprite, false);
	ituWidgetSetVisible(mainLQualityOutPMWNumSprite, false);

	QualitySpriteIndex = DWSpriteIndex * 4 + atoi(param);

	ituSpriteGoto(mainLQualitySprite, QualitySpriteIndex);

	if (DWSpriteIndex == 0)
	{
		switch (atoi(param))
		{
		case 0:
			//DayQualitySet(mainLQualityInPMDayIcon, 0, 280, 230, 250, 191, 220, 265, 230, 220);
			DayQualitySet(mainLQualityInPMDayIcon, 0, InPMDayValue[0], InPMDayValue[1], InPMDayValue[2], InPMDayValue[3], InPMDayValue[4], InPMDayValue[5], InPMDayValue[6], InPMDayValue[7]);
			break;
		case 1:
			//DayQualitySet(mainLQualityTVOCDayIcon, 1, 165, 240, 90, 165, 10, 90, 200, 165);
			DayQualitySet(mainLQualityTVOCDayIcon, 1, TVOCDayValue[0], TVOCDayValue[1], TVOCDayValue[2], TVOCDayValue[3], TVOCDayValue[4], TVOCDayValue[5], TVOCDayValue[6], TVOCDayValue[7]);
			break;
		case 2:
			//DayQualitySet(mainLQualityCO2DayIcon, 2, 280, 280, 200, 165, 155, 200, 230, 260);
			DayQualitySet(mainLQualityCO2DayIcon, 2, CO2DayValue[0], CO2DayValue[1], CO2DayValue[2], CO2DayValue[3], CO2DayValue[4], CO2DayValue[5], CO2DayValue[6], CO2DayValue[7]);
			break;
		case 3:
			//DayQualitySet(mainLQualityOutPMDayIcon, 3, 280, 230, 191, 100, 100, 180, 185, 180);
			DayQualitySet(mainLQualityOutPMDayIcon, 3, OutPMDayValue[0], OutPMDayValue[1], OutPMDayValue[2], OutPMDayValue[3], OutPMDayValue[4], OutPMDayValue[5], OutPMDayValue[6], OutPMDayValue[7]);
			break;

		}
	}
	else
	{
		switch (atoi(param))
		{
		case 0:
			//WeekQualitySet(mainLQualityInPMWeekIcon, 0, 280, 220, 191, 215, 270, 240, 235);
			WeekQualitySet(mainLQualityInPMWeekIcon, 0, InPMWeekValue[0], InPMWeekValue[1], InPMWeekValue[2], InPMWeekValue[3], InPMWeekValue[4], InPMWeekValue[5], InPMWeekValue[6]);
			break;
		case 1:
			//WeekQualitySet(mainLQualityTVOCWeekIcon, 1, 165, 240, 90, 165, 10, 90, 165);
			WeekQualitySet(mainLQualityTVOCWeekIcon, 1, TVOCWeekValue[0], TVOCWeekValue[1], TVOCWeekValue[2], TVOCWeekValue[3], TVOCWeekValue[4], TVOCWeekValue[5], TVOCWeekValue[6]);
			break;
		case 2:
			//WeekQualitySet(mainLQualityCO2WeekIcon, 2, 280, 285, 220, 215, 225, 270, 260);
			WeekQualitySet(mainLQualityCO2WeekIcon, 2, CO2WeekValue[0], CO2WeekValue[1], CO2WeekValue[2], CO2WeekValue[3], CO2WeekValue[4], CO2WeekValue[5], CO2WeekValue[6]);
			break;
		case 3:
			//WeekQualitySet(mainLQualityOutPMWeekIcon, 3, 280, 220, 191, 150, 185, 100, 95);
			WeekQualitySet(mainLQualityOutPMWeekIcon, 3, OutPMWeekValue[0], OutPMWeekValue[1], OutPMWeekValue[2], OutPMWeekValue[3], OutPMWeekValue[4], OutPMWeekValue[5], OutPMWeekValue[6]);
			break;

		}
	}
		
		


	return true;
}


bool MainLDWChangeBtnOnPress(ITUWidget* widget, char* param)
{
	ituWidgetSetVisible(mainLQualityInPMNumSprite, false);
	ituWidgetSetVisible(mainLQualityTVOCNumSprite, false);
	ituWidgetSetVisible(mainLQualityCO2NumSprite, false);
	ituWidgetSetVisible(mainLQualityOutPMNumSprite, false);
	ituWidgetSetVisible(mainLQualityInPMWNumSprite, false);
	ituWidgetSetVisible(mainLQualityTVOCWNumSprite, false);
	ituWidgetSetVisible(mainLQualityCO2WNumSprite, false);
	ituWidgetSetVisible(mainLQualityOutPMWNumSprite, false);

	DWSpriteIndex++;
	DWSpriteIndex = DWSpriteIndex % 2;

	ituSpriteGoto(mainLDWSprite, DWSpriteIndex);

	if (DWSpriteIndex == 0)
	{
		QualitySpriteIndex = QualitySpriteIndex - 4;
	}
	else
	{
		QualitySpriteIndex = QualitySpriteIndex + 4;
	}

	ituSpriteGoto(mainLQualitySprite, QualitySpriteIndex);

	switch (QualitySpriteIndex)
	{
	case 0:
		//DayQualitySet(mainLQualityInPMDayIcon, 0, 280, 230, 250, 191, 220, 265, 230, 220);
		DayQualitySet(mainLQualityInPMDayIcon, 0, InPMDayValue[0], InPMDayValue[1], InPMDayValue[2], InPMDayValue[3], InPMDayValue[4], InPMDayValue[5], InPMDayValue[6], InPMDayValue[7]);
		break;
	case 1:
		//DayQualitySet(mainLQualityTVOCDayIcon, 1, 165, 240, 90, 165, 10, 90, 200, 165);
		DayQualitySet(mainLQualityTVOCDayIcon, 1, TVOCDayValue[0], TVOCDayValue[1], TVOCDayValue[2], TVOCDayValue[3], TVOCDayValue[4], TVOCDayValue[5], TVOCDayValue[6], TVOCDayValue[7]);
		break;
	case 2:
		//DayQualitySet(mainLQualityCO2DayIcon, 2, 280, 280, 200, 165, 155, 200, 230, 260);
		DayQualitySet(mainLQualityCO2DayIcon, 2, CO2DayValue[0], CO2DayValue[1], CO2DayValue[2], CO2DayValue[3], CO2DayValue[4], CO2DayValue[5], CO2DayValue[6], CO2DayValue[7]);
		break;
	case 3:
		//DayQualitySet(mainLQualityOutPMDayIcon, 3, 280, 230, 191, 100, 100, 180, 185, 180);
		DayQualitySet(mainLQualityOutPMDayIcon, 3, OutPMDayValue[0], OutPMDayValue[1], OutPMDayValue[2], OutPMDayValue[3], OutPMDayValue[4], OutPMDayValue[5], OutPMDayValue[6], OutPMDayValue[7]);
		break;
	case 4:
		//WeekQualitySet(mainLQualityInPMWeekIcon, 0, 280, 220, 191, 215, 270, 240, 235);
		WeekQualitySet(mainLQualityInPMWeekIcon, 0, InPMWeekValue[0], InPMWeekValue[1], InPMWeekValue[2], InPMWeekValue[3], InPMWeekValue[4], InPMWeekValue[5], InPMWeekValue[6]);
		break;
	case 5:
		//WeekQualitySet(mainLQualityTVOCWeekIcon, 1, 165, 240, 90, 165, 10, 90, 165);
		WeekQualitySet(mainLQualityTVOCWeekIcon, 1, TVOCWeekValue[0], TVOCWeekValue[1], TVOCWeekValue[2], TVOCWeekValue[3], TVOCWeekValue[4], TVOCWeekValue[5], TVOCWeekValue[6]);
		break;
	case 6:
		//WeekQualitySet(mainLQualityCO2WeekIcon, 2, 280, 280, 200, 165, 155, 200, 230);
		WeekQualitySet(mainLQualityCO2WeekIcon, 2, CO2WeekValue[0], CO2WeekValue[1], CO2WeekValue[2], CO2WeekValue[3], CO2WeekValue[4], CO2WeekValue[5], CO2WeekValue[6]);
		break;
	case 7:
		//WeekQualitySet(mainLQualityOutPMWeekIcon, 3, 280, 220, 191, 150, 185, 100, 95);
		WeekQualitySet(mainLQualityOutPMWeekIcon, 3, OutPMWeekValue[0], OutPMWeekValue[1], OutPMWeekValue[2], OutPMWeekValue[3], OutPMWeekValue[4], OutPMWeekValue[5], OutPMWeekValue[6]);
		break;

	}

	


	return true;
}
bool MainLQualityInPMBtnOnPress(ITUWidget* widget, char* param)
{
	int index = atoi(param);
	char tmp[32];

	ituWidgetSetPosition(mainLQualityInPMNumSprite, ituWidgetGetX(mainLQualityInPMSprite[index]) - 35, ituWidgetGetY(mainLQualityInPMSprite[index]) - 70);
	sprintf(tmp, "%d", InPMDayValue[index]);
	ituTextSetString(mainLQualityInPMNumText[mainLQualityInPMSprite[index]->frame], tmp);
	ituSpriteGoto(mainLQualityInPMNumSprite, mainLQualityInPMSprite[index]->frame);
	ituWidgetSetVisible(mainLQualityInPMNumSprite, true);


	return true;
}

bool MainLQualityTVOCBtnOnPress(ITUWidget* widget, char* param)
{
	int index = atoi(param);
	char tmp[32];

	
	
	if (TVOCDayValue[index] % 10 == 0)
		sprintf(tmp, "%d", TVOCDayValue[index] / 10);
	else
		sprintf(tmp, "%1.1f", (float)TVOCDayValue[index]/10);
	ituTextSetString(mainLQualityTVOCNumText[mainLQualityTVOCSprite[index]->frame], tmp);
	ituSpriteGoto(mainLQualityTVOCNumSprite, mainLQualityTVOCSprite[index]->frame);

	ituWidgetSetHeight(mainLQualityTVOCNumSprite, ituWidgetGetHeight(mainLQualityTVOCButton[index]) + 35);
	ituWidgetSetPosition(mainLQualityTVOCNumSprite, ituWidgetGetX(mainLQualityTVOCSprite[index]) - 21, ituWidgetGetY(mainLQualityTVOCSprite[index]) - 21);
	ituWidgetSetVisible(mainLQualityTVOCNumSprite, true);


	return true;
}

bool MainLQualityCO2BtnOnPress(ITUWidget* widget, char* param)
{
	int index = atoi(param);
	char tmp[32];

	ituWidgetSetPosition(mainLQualityCO2NumSprite, ituWidgetGetX(mainLQualityCO2Sprite[index]) - 35, ituWidgetGetY(mainLQualityCO2Sprite[index]) - 70);
	sprintf(tmp, "%d", CO2DayValue[index]);
	ituTextSetString(mainLQualityCO2NumText[mainLQualityCO2Sprite[index]->frame], tmp);
	ituSpriteGoto(mainLQualityCO2NumSprite, mainLQualityCO2Sprite[index]->frame);
	ituWidgetSetVisible(mainLQualityCO2NumSprite, true);


	return true;
}

bool MainLQualityOutPMBtnOnPress(ITUWidget* widget, char* param)
{
	int index = atoi(param);
	char tmp[32];

	ituWidgetSetPosition(mainLQualityOutPMNumSprite, ituWidgetGetX(mainLQualityOutPMSprite[index]) - 35, ituWidgetGetY(mainLQualityOutPMSprite[index]) - 70);
	sprintf(tmp, "%d", OutPMDayValue[index]);
	ituTextSetString(mainLQualityOutPMNumText[mainLQualityOutPMSprite[index]->frame], tmp);
	ituSpriteGoto(mainLQualityOutPMNumSprite, mainLQualityOutPMSprite[index]->frame);
	ituWidgetSetVisible(mainLQualityOutPMNumSprite, true);


	return true;
}

bool MainLQualityInPMWBtnOnPress(ITUWidget* widget, char* param)
{
	int index = atoi(param);
	char tmp[32];

	ituWidgetSetPosition(mainLQualityInPMWNumSprite, ituWidgetGetX(mainLQualityInPMWSprite[index]) - 35, ituWidgetGetY(mainLQualityInPMWSprite[index]) - 70);
	sprintf(tmp, "%d", InPMWeekValue[index]);
	ituTextSetString(mainLQualityInPMWNumText[mainLQualityInPMWSprite[index]->frame], tmp);
	ituSpriteGoto(mainLQualityInPMWNumSprite, mainLQualityInPMWSprite[index]->frame);
	ituWidgetSetVisible(mainLQualityInPMWNumSprite, true);


	return true;
}

bool MainLQualityTVOCWBtnOnPress(ITUWidget* widget, char* param)
{
	int index = atoi(param);
	char tmp[32];



	if (TVOCWeekValue[index] % 10 == 0)
		sprintf(tmp, "%d", TVOCWeekValue[index] / 10);
	else
		sprintf(tmp, "%1.1f", (float)TVOCWeekValue[index] / 10);
	ituTextSetString(mainLQualityTVOCWNumText[mainLQualityTVOCWSprite[index]->frame], tmp);
	ituSpriteGoto(mainLQualityTVOCWNumSprite, mainLQualityTVOCWSprite[index]->frame);

	ituWidgetSetHeight(mainLQualityTVOCWNumSprite, ituWidgetGetHeight(mainLQualityTVOCWButton[index]) + 35);
	ituWidgetSetPosition(mainLQualityTVOCWNumSprite, ituWidgetGetX(mainLQualityTVOCWSprite[index]) - 21, ituWidgetGetY(mainLQualityTVOCWSprite[index]) - 21);
	ituWidgetSetVisible(mainLQualityTVOCWNumSprite, true);


	return true;
}

bool MainLQualityCO2WBtnOnPress(ITUWidget* widget, char* param)
{
	int index = atoi(param);
	char tmp[32];

	ituWidgetSetPosition(mainLQualityCO2WNumSprite, ituWidgetGetX(mainLQualityCO2WSprite[index]) - 35, ituWidgetGetY(mainLQualityCO2WSprite[index]) - 70);
	sprintf(tmp, "%d", CO2WeekValue[index]);
	ituTextSetString(mainLQualityCO2WNumText[mainLQualityCO2WSprite[index]->frame], tmp);
	ituSpriteGoto(mainLQualityCO2WNumSprite, mainLQualityCO2WSprite[index]->frame);
	ituWidgetSetVisible(mainLQualityCO2WNumSprite, true);


	return true;
}

bool MainLQualityOutPMWBtnOnPress(ITUWidget* widget, char* param)
{
	int index = atoi(param);
	char tmp[32];

	ituWidgetSetPosition(mainLQualityOutPMWNumSprite, ituWidgetGetX(mainLQualityOutPMWSprite[index]) - 35, ituWidgetGetY(mainLQualityOutPMWSprite[index]) - 70);
	sprintf(tmp, "%d", OutPMWeekValue[index]);
	ituTextSetString(mainLQualityOutPMWNumText[mainLQualityOutPMWSprite[index]->frame], tmp);
	ituSpriteGoto(mainLQualityOutPMWNumSprite, mainLQualityOutPMWSprite[index]->frame);
	ituWidgetSetVisible(mainLQualityOutPMWNumSprite, true);


	return true;
}

bool MainAirForceTrackBarOnChanged(ITUWidget* widget, char* param)
{
	int x, y;
	char tmp[32];
	airForceIndex = mainAirForceTrackBar->value - 1;
	ituSpriteGoto(mainAirForceSprite, airForceIndex);
	ituSpriteGoto(mainAirForceTextSprite, airForceIndex);
	//ituIconLinkSurface(&mainCAirForceButton->bg.icon, mainCAirForceIconSet[airForceIndex]);
	ituSpriteGoto(mainAirForceLineSprite, airForceIndex);


	ituWidgetSetVisible(mainAirForceTipShowBackground, false);
	ituWidgetGetGlobalPosition(mainAirForceTrackBar->tip, &x, &y);
	ituWidgetSetPosition(mainAirForceTipShowBackground, x, y);
	sprintf(tmp, "%d", mainAirForceTrackBar->value);
	ituTextSetString(mainAirForceTipShowText, tmp);
	ituWidgetSetVisible(mainAirForceTipShowBackground, true);

	return true;
}

bool MainAirForceAutoChkBoxOnPress(ITUWidget* widget, char* param)
{
	int x, y;
	char tmp[32];

	if (ituCheckBoxIsChecked(mainAirForceAutoCheckBox))
	{

		ituTrackBarSetValue(mainAirForceTrackBar, 3);
		ituProgressBarSetValue(mainAirForceProgressBar, 3);
		ituSpriteGoto(mainAirForceSprite, 6);
		ituSpriteGoto(mainAirForceTextSprite, 6);
		//ituIconLinkSurface(&mainCAirForceButton->bg.icon, mainCAirForceIconSet[6]);
		ituSpriteGoto(mainAirForceLineSprite, 2);
		ituWidgetDisable(mainAirForceTrackBar);
		ituWidgetSetVisible(mainAirForceProgressBar, false);
		ituIconLinkSurface(mainAirForceTrackBarIcon, mainBarIcon[1]);


		ituWidgetGetGlobalPosition(mainAirForceTrackBar->tip, &x, &y);
		ituWidgetSetPosition(mainAirForceTipShowBackground, x, y);
		ituTextSetString(mainAirForceTipShowText, "A");

	}
	else
	{

		ituTrackBarSetValue(mainAirForceTrackBar, airForceIndex + 1);
		ituProgressBarSetValue(mainAirForceProgressBar, airForceIndex + 1);
		ituSpriteGoto(mainAirForceSprite, airForceIndex);
		ituSpriteGoto(mainAirForceTextSprite, airForceIndex);
		//ituIconLinkSurface(&mainCAirForceButton->bg.icon, mainCAirForceIconSet[airForceIndex]);
		ituSpriteGoto(mainAirForceLineSprite, airForceIndex);
		ituWidgetEnable(mainAirForceTrackBar);
		ituWidgetSetVisible(mainAirForceProgressBar, true);
		ituIconLinkSurface(mainAirForceTrackBarIcon, mainBarIcon[0]);

		ituWidgetGetGlobalPosition(mainAirForceTrackBar->tip, &x, &y);
		ituWidgetSetPosition(mainAirForceTipShowBackground, x, y);
		sprintf(tmp, "%d", mainAirForceTrackBar->value);
		ituTextSetString(mainAirForceTipShowText, tmp);
	}

	return true;
}

bool DayQualitySet(ITUWidget* widget,int index, int i0, int i1, int i2, int i3, int i4, int i5, int i6, int i7)
{
	ITUIcon* pIcon = (ITUIcon*)widget;
	if (pIcon)
	{
		int pQtyValue[8] = { i0, i1, i2, i3, i4, i5, i6, i7 };
		int i = 0;
		int xQtyValue[8] = { 50, 120,190,260,340,425,495,575};

		ITUColor color = { 255,160, 196, 255 };

		switch (index)
		{
		case 0:
			for (i = 0; i < 8; i++)
			{	
				if (pQtyValue[i] > 500)
					pQtyValue[i] = 34;
				else if (pQtyValue[i] > 75)
					pQtyValue[i] = (((float) pQtyValue[i] - 75) / 425) * 160 * (-1) + 194;
				else if (pQtyValue[i] > 35)
					pQtyValue[i] = (((float) pQtyValue[i] - 35) / 40) * 60 * (-1) + 254;
				else
					pQtyValue[i] = ((float) pQtyValue[i] / 35) * 50 * (-1) + 304;
			}
			break;
		case 1:
			for (i = 0; i < 8; i++)
			{
				switch (pQtyValue[i])
				{
				case 0:
					pQtyValue[i] = 237;
					break;
				case 10:
					pQtyValue[i] = 165;
					break;
				case 20:
					pQtyValue[i] = 85;
					break;
				case 30:
					pQtyValue[i] = 10;
					break;
				default:
					if (pQtyValue[i] > 20)
						pQtyValue[i] = 48;
					else if (pQtyValue[i] > 10)
						pQtyValue[i] = 125;
					else
						pQtyValue[i] = 201;

					break;
				}
			}
			break;
		case 2:
			for (i = 0; i < 8; i++)
			{
				if (pQtyValue[i] > 5000)
					pQtyValue[i] = 34;
				else if (pQtyValue[i] > 1200)
					pQtyValue[i] = (((float)pQtyValue[i] - 1200) / 3800) * 160 * (-1) + 194;
				else if (pQtyValue[i] > 800)
					pQtyValue[i] = (((float)pQtyValue[i] - 800) / 400) * 60 * (-1) + 254;
				else
					pQtyValue[i] = (((float)pQtyValue[i] - 400) / 400) * 50 * (-1) + 304;
			}
			break;
		case 3:
			for (i = 0; i < 8; i++)
			{
				if (pQtyValue[i] > 500)
					pQtyValue[i] = 34;
				else if (pQtyValue[i] > 75)
					pQtyValue[i] = (((float)pQtyValue[i] - 75) / 425) * 160 * (-1) + 194;
				else if (pQtyValue[i] > 35)
					pQtyValue[i] = (((float)pQtyValue[i] - 35) / 40) * 60 * (-1) + 254;
				else
					pQtyValue[i] = ((float)pQtyValue[i] / 35) * 50 * (-1) + 304;
			}
			break;

		}

		switch (index)
		{
		case 0:
			for (i = 0; i < 7; i++)
			{
				ituDrawLine(pIcon->surf, xQtyValue[i], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i + 1], &color, 2);

				if (pQtyValue[i] >= 254)
					ituSpriteGoto(mainLQualityInPMSprite[i], 0);
				else if (pQtyValue[i] >= 194)
					ituSpriteGoto(mainLQualityInPMSprite[i], 1);
				else
					ituSpriteGoto(mainLQualityInPMSprite[i], 2);

				ituWidgetSetPosition(mainLQualityInPMSprite[i], xQtyValue[i] + 44, pQtyValue[i] + 34);
				ituWidgetSetPosition(mainLQualityInPMButton[i], xQtyValue[i] + 44, pQtyValue[i] + 34);

				if (pQtyValue[i + 1] >= 254)
					ituSpriteGoto(mainLQualityInPMSprite[i + 1], 0);
				else if (pQtyValue[i + 1] >= 194)
					ituSpriteGoto(mainLQualityInPMSprite[i + 1], 1);
				else
					ituSpriteGoto(mainLQualityInPMSprite[i + 1], 2);
				ituWidgetSetPosition(mainLQualityInPMSprite[i + 1], xQtyValue[i + 1] + 44, pQtyValue[i + 1] + 34);
				ituWidgetSetPosition(mainLQualityInPMButton[i + 1], xQtyValue[i + 1] + 44, pQtyValue[i + 1] + 34);
			}
			break;
		case 1:
			for (i = 0; i < 8; i++)
			{
				if (pQtyValue[i] >= 165)
					ituSpriteGoto(mainLQualityTVOCSprite[i], 0);
				else if (pQtyValue[i] >= 85)
					ituSpriteGoto(mainLQualityTVOCSprite[i], 1);
				else
					ituSpriteGoto(mainLQualityTVOCSprite[i], 2);

				ituWidgetSetPosition(mainLQualityTVOCSprite[i], xQtyValue[i] + 44, pQtyValue[i] + 34);
				ituWidgetSetHeight(mainLQualityTVOCButton[i], 302 - pQtyValue[i]);
				ituWidgetSetPosition(mainLQualityTVOCButton[i], xQtyValue[i] + 44, pQtyValue[i] + 48);
			}

			break;
		case 2:
			for (i = 0; i < 7; i++)
			{
				ituDrawLine(pIcon->surf, xQtyValue[i], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i + 1], &color, 2);

				if (pQtyValue[i] >= 254)
					ituSpriteGoto(mainLQualityCO2Sprite[i], 0);
				else if (pQtyValue[i] >= 194)
					ituSpriteGoto(mainLQualityCO2Sprite[i], 1);
				else
					ituSpriteGoto(mainLQualityCO2Sprite[i], 2);
				ituWidgetSetPosition(mainLQualityCO2Sprite[i], xQtyValue[i] + 44, pQtyValue[i] + 34);
				ituWidgetSetPosition(mainLQualityCO2Button[i], xQtyValue[i] + 44, pQtyValue[i] + 34);

				if (pQtyValue[i + 1] >= 254)
					ituSpriteGoto(mainLQualityCO2Sprite[i + 1], 0);
				else if (pQtyValue[i + 1] >= 194)
					ituSpriteGoto(mainLQualityCO2Sprite[i + 1], 1);
				else
					ituSpriteGoto(mainLQualityCO2Sprite[i + 1], 2);
				ituWidgetSetPosition(mainLQualityCO2Sprite[i + 1], xQtyValue[i + 1] + 44, pQtyValue[i + 1] + 34);
				ituWidgetSetPosition(mainLQualityCO2Button[i + 1], xQtyValue[i + 1] + 44, pQtyValue[i + 1] + 34);
			}
			break;
		case 3:
			for (i = 0; i < 7; i++)
			{
				ituDrawLine(pIcon->surf, xQtyValue[i], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i + 1], &color, 2);

				if (pQtyValue[i] >= 254)
					ituSpriteGoto(mainLQualityOutPMSprite[i], 0);
				else if (pQtyValue[i] >= 194)
					ituSpriteGoto(mainLQualityOutPMSprite[i], 1);
				else
					ituSpriteGoto(mainLQualityOutPMSprite[i], 2);
				ituWidgetSetPosition(mainLQualityOutPMSprite[i], xQtyValue[i] + 44, pQtyValue[i] + 34);
				ituWidgetSetPosition(mainLQualityOutPMButton[i], xQtyValue[i] + 44, pQtyValue[i] + 34);

				if (pQtyValue[i + 1] >= 254)
					ituSpriteGoto(mainLQualityOutPMSprite[i + 1], 0);
				else if (pQtyValue[i + 1] >= 194)
					ituSpriteGoto(mainLQualityOutPMSprite[i + 1], 1);
				else
					ituSpriteGoto(mainLQualityOutPMSprite[i + 1], 2);
				ituWidgetSetPosition(mainLQualityOutPMSprite[i + 1], xQtyValue[i + 1] + 44, pQtyValue[i + 1] + 34);
				ituWidgetSetPosition(mainLQualityOutPMButton[i + 1], xQtyValue[i + 1] + 44, pQtyValue[i + 1] + 34);
			}
			break;
		}
		
	}
	return true;
}

bool WeekQualitySet(ITUWidget* widget, int index, int i0, int i1, int i2, int i3, int i4, int i5, int i6)
{
	ITUIcon* pIcon = (ITUIcon*)widget;
	if (pIcon)
	{
		int pQtyValue[7] = { i0, i1, i2, i3, i4, i5, i6};
		int i = 0;
		int xQtyValue[7] = { 80, 160, 235, 320, 400, 480, 570 };

		ITUColor color = { 255, 160, 196, 255 };

		switch (index)
		{
		case 0:
			for (i = 0; i < 7; i++)
			{
				if (pQtyValue[i] > 500)
					pQtyValue[i] = 34;
				else if (pQtyValue[i] > 75)
					pQtyValue[i] = (((float)pQtyValue[i] - 75) / 425) * 160 * (-1) + 194;
				else if (pQtyValue[i] > 35)
					pQtyValue[i] = (((float)pQtyValue[i] - 35) / 40) * 60 * (-1) + 254;
				else
					pQtyValue[i] = ((float)pQtyValue[i] / 35) * 50 * (-1) + 304;
			}
			break;
		case 1:
			for (i = 0; i < 7; i++)
			{
				switch (pQtyValue[i])
				{
				case 0:
					pQtyValue[i] = 237;
					break;
				case 10:
					pQtyValue[i] = 165;
					break;
				case 20:
					pQtyValue[i] = 85;
					break;
				case 30:
					pQtyValue[i] = 10;
					break;
				default:
					if (pQtyValue[i] > 20)
						pQtyValue[i] = 48;
					else if (pQtyValue[i] > 10)
						pQtyValue[i] = 125;
					else
						pQtyValue[i] = 201;

					break;
				}
			}
			break;
		case 2:
			for (i = 0; i < 7; i++)
			{
				if (pQtyValue[i] > 5000)
					pQtyValue[i] = 34;
				else if (pQtyValue[i] > 1200)
					pQtyValue[i] = (((float)pQtyValue[i] - 1200) / 3800) * 160 * (-1) + 194;
				else if (pQtyValue[i] > 800)
					pQtyValue[i] = (((float)pQtyValue[i] - 800) / 400) * 60 * (-1) + 254;
				else
					pQtyValue[i] = (((float)pQtyValue[i] - 400) / 400) * 50 * (-1) + 304;
			}
			break;
		case 3:
			for (i = 0; i < 7; i++)
			{
				if (pQtyValue[i] > 500)
					pQtyValue[i] = 34;
				else if (pQtyValue[i] > 75)
					pQtyValue[i] = (((float)pQtyValue[i] - 75) / 425) * 160 * (-1) + 194;
				else if (pQtyValue[i] > 35)
					pQtyValue[i] = (((float)pQtyValue[i] - 35) / 40) * 60 * (-1) + 254;
				else
					pQtyValue[i] = ((float)pQtyValue[i] / 35) * 50 * (-1) + 304;
			}
			break;

		}

		switch (index)
		{
		case 0:
			for (i = 0; i < 6; i++)
			{
				ituDrawLine(pIcon->surf, xQtyValue[i], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i + 1], &color, 2);

				if (pQtyValue[i]>250)
					ituSpriteGoto(mainLQualityInPMWSprite[i], 0);
				else if (pQtyValue[i]>190)
					ituSpriteGoto(mainLQualityInPMWSprite[i], 1);
				else
					ituSpriteGoto(mainLQualityInPMWSprite[i], 2);
				ituWidgetSetPosition(mainLQualityInPMWSprite[i], xQtyValue[i] + 44, pQtyValue[i] + 34);
				ituWidgetSetPosition(mainLQualityInPMWButton[i], xQtyValue[i] + 44, pQtyValue[i] + 34);

				if (pQtyValue[i + 1]>250)
					ituSpriteGoto(mainLQualityInPMWSprite[i + 1], 0);
				else if (pQtyValue[i + 1]>190)
					ituSpriteGoto(mainLQualityInPMWSprite[i + 1], 1);
				else
					ituSpriteGoto(mainLQualityInPMWSprite[i + 1], 2);
				ituWidgetSetPosition(mainLQualityInPMWSprite[i + 1], xQtyValue[i + 1] + 44, pQtyValue[i + 1] + 34);
				ituWidgetSetPosition(mainLQualityInPMWButton[i + 1], xQtyValue[i + 1] + 44, pQtyValue[i + 1] + 34);
			}
			break;
			case 1:
				for (i = 0; i < 7; i++)
				{
					if (pQtyValue[i] >= 165)
						ituSpriteGoto(mainLQualityTVOCWSprite[i], 0);
					else if (pQtyValue[i] >= 85)
						ituSpriteGoto(mainLQualityTVOCWSprite[i], 1);
					else
						ituSpriteGoto(mainLQualityTVOCWSprite[i], 2);

					ituWidgetSetPosition(mainLQualityTVOCWSprite[i], xQtyValue[i] + 44, pQtyValue[i] + 34);
					ituWidgetSetHeight(mainLQualityTVOCWButton[i], 302 - pQtyValue[i]);
					ituWidgetSetPosition(mainLQualityTVOCWButton[i], xQtyValue[i] + 44, pQtyValue[i] + 48);
				}
				break;
		case 2:
			for (i = 0; i < 6; i++)
			{
				ituDrawLine(pIcon->surf, xQtyValue[i], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i + 1], &color, 2);

				if (pQtyValue[i]>250)
					ituSpriteGoto(mainLQualityCO2WSprite[i], 0);
				else if (pQtyValue[i]>190)
					ituSpriteGoto(mainLQualityCO2WSprite[i], 1);
				else
					ituSpriteGoto(mainLQualityCO2WSprite[i], 2);
				ituWidgetSetPosition(mainLQualityCO2WSprite[i], xQtyValue[i] + 44, pQtyValue[i] + 34);
				ituWidgetSetPosition(mainLQualityCO2WButton[i], xQtyValue[i] + 44, pQtyValue[i] + 34);

				if (pQtyValue[i + 1]>250)
					ituSpriteGoto(mainLQualityCO2WSprite[i + 1], 0);
				else if (pQtyValue[i + 1]>190)
					ituSpriteGoto(mainLQualityCO2WSprite[i + 1], 1);
				else
					ituSpriteGoto(mainLQualityCO2WSprite[i + 1], 2);
				ituWidgetSetPosition(mainLQualityCO2WSprite[i + 1], xQtyValue[i + 1] + 44, pQtyValue[i + 1] + 34);
				ituWidgetSetPosition(mainLQualityCO2WButton[i + 1], xQtyValue[i + 1] + 44, pQtyValue[i + 1] + 34);
			}
			break;
		case 3:
			for (i = 0; i < 6; i++)
			{
				ituDrawLine(pIcon->surf, xQtyValue[i], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i + 1], &color, 2);

				if (pQtyValue[i]>250)
					ituSpriteGoto(mainLQualityOutPMWSprite[i], 0);
				else if (pQtyValue[i]>190)
					ituSpriteGoto(mainLQualityOutPMWSprite[i], 1);
				else
					ituSpriteGoto(mainLQualityOutPMWSprite[i], 2);
				ituWidgetSetPosition(mainLQualityOutPMWSprite[i], xQtyValue[i] + 44, pQtyValue[i] + 34);
				ituWidgetSetPosition(mainLQualityOutPMWButton[i], xQtyValue[i] + 44, pQtyValue[i] + 34);

				if (pQtyValue[i + 1]>250)
					ituSpriteGoto(mainLQualityOutPMWSprite[i + 1], 0);
				else if (pQtyValue[i + 1]>190)
					ituSpriteGoto(mainLQualityOutPMWSprite[i + 1], 1);
				else
					ituSpriteGoto(mainLQualityOutPMWSprite[i + 1], 2);
				ituWidgetSetPosition(mainLQualityOutPMWSprite[i + 1], xQtyValue[i + 1] + 44, pQtyValue[i + 1] + 34);
				ituWidgetSetPosition(mainLQualityOutPMWButton[i + 1], xQtyValue[i + 1] + 44, pQtyValue[i + 1] + 34);
			}
			break;
		}

	}
	return true;
}