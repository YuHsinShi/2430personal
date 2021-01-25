#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"
#include <math.h>

#ifdef CFG_SHT20_ENABLE
    #include "sht20_sensor.h"
#endif

#ifdef CFG_NCP18_ENABLE
    #include "ncp18_sensor.h"
#endif

ITUCoverFlow* mainCoverFlow = 0;

static ITUButton* mainLDWChangeButton = 0;
static ITUSprite* mainLDWSprite = 0;
static ITURadioBox* mainLQualityRadioBox[4] = { 0 };
static ITUSprite* mainLQualitySprite = 0;
static ITUText* mainLInPMDayText[6] = { 0 };
static ITUText* mainLCO2DayText[6] = { 0 };
static ITUText* mainLTVOCDayText[6] = { 0 };
static ITUText* mainLOutPMDayText[6] = { 0 };
static ITUIcon* mainLQualityInPMDayIcon = 0;
static ITUIcon* mainLQualityInPMBarIcon = 0;
//static ITUSprite* mainLQualityInPMSprite[8] = { 0 };
static ITUIcon* mainLQualityInPMIcon[8] = { 0 };
static ITUButton* mainLQualityInPMButton[8] = { 0 };
static ITUSprite* mainLQualityInPMNumSprite = 0;
static ITUText* mainLQualityInPMNumText[3] = { 0 };
static ITUIcon* mainLQualityTVOCDayIcon = 0;
static ITUIcon* mainLQualityTVOCBarIcon = 0;
//static ITUSprite* mainLQualityTVOCSprite[8] = { 0 };
static ITUIcon* mainLQualityTVOCIcon[8] = { 0 };
static ITUButton* mainLQualityTVOCButton[8] = { 0 };
static ITUSprite* mainLQualityTVOCNumSprite = 0;
static ITUText* mainLQualityTVOCNumText[3] = { 0 };
static ITUIcon* mainLQualityCO2DayIcon = 0;
static ITUIcon* mainLQualityCO2BarIcon = 0;
//static ITUSprite* mainLQualityCO2Sprite[8] = { 0 };
static ITUIcon* mainLQualityCO2Icon[8] = { 0 };
static ITUButton* mainLQualityCO2Button[8] = { 0 };
static ITUSprite* mainLQualityCO2NumSprite = 0;
static ITUText* mainLQualityCO2NumText[3] = { 0 };
static ITUIcon* mainLQualityOutPMDayIcon = 0;
static ITUIcon* mainLQualityOutPMBarIcon = 0;
//static ITUSprite* mainLQualityOutPMSprite[8] = { 0 };
static ITUIcon* mainLQualityOutPMIcon[8] = { 0 };
static ITUButton* mainLQualityOutPMButton[8] = { 0 };
static ITUSprite* mainLQualityOutPMNumSprite = 0;
static ITUText* mainLQualityOutPMNumText[3] = { 0 };
static ITUIcon* mainLQualityInPMWeekIcon = 0;
static ITUIcon* mainLQualityInPMWBarIcon = 0;
//static ITUSprite* mainLQualityInPMWSprite[7] = { 0 };
static ITUIcon* mainLQualityInPMWIcon[7] = { 0 };
static ITUButton* mainLQualityInPMWButton[7] = { 0 };
static ITUSprite* mainLQualityInPMWNumSprite = 0;
static ITUText* mainLQualityInPMWNumText[3] = { 0 };
static ITUIcon* mainLQualityTVOCWeekIcon = 0;
static ITUIcon* mainLQualityTVOCWBarIcon = 0;
//static ITUSprite* mainLQualityTVOCWSprite[7] = { 0 };
static ITUIcon* mainLQualityTVOCWIcon[7] = { 0 };
static ITUButton* mainLQualityTVOCWButton[7] = { 0 };
static ITUSprite* mainLQualityTVOCWNumSprite = 0;
static ITUText* mainLQualityTVOCWNumText[3] = { 0 };
static ITUIcon* mainLQualityCO2WeekIcon = 0;
static ITUIcon* mainLQualityCO2WBarIcon = 0;
//static ITUSprite* mainLQualityCO2WSprite[7] = { 0 };
static ITUIcon* mainLQualityCO2WIcon[7] = { 0 };
static ITUButton* mainLQualityCO2WButton[7] = { 0 };
static ITUSprite* mainLQualityCO2WNumSprite = 0;
static ITUText* mainLQualityCO2WNumText[3] = { 0 };
static ITUIcon* mainLQualityOutPMWeekIcon = 0;
static ITUIcon* mainLQualityOutPMWBarIcon = 0;
//static ITUSprite* mainLQualityOutPMWSprite[7] = { 0 };
static ITUIcon* mainLQualityOutPMWIcon[7] = { 0 };
static ITUButton* mainLQualityOutPMWButton[7] = { 0 };
static ITUSprite* mainLQualityOutPMWNumSprite = 0;
static ITUText* mainLQualityOutPMWNumText[3] = { 0 };
static ITUIcon* mainLQualityIcon[3] = { 0 };
static ITUIcon* mainLQualityBIcon[3] = { 0 };

static ITUBackground* mainLeftBackground = 0;
static ITUIcon* mainLQualityInPMDayBgIcon = 0;
static ITUIcon* mainLQualityCO2DayBgIcon = 0;
static ITUIcon* mainLQualityOutPMDayBgIcon = 0;
static ITUIcon* mainLQualityInPMWeekBgIcon = 0;
static ITUIcon* mainLQualityCO2WeekBgIcon = 0;
static ITUIcon* mainLQualityOutPMWeekBgIcon = 0;


static ITUText* mainCInHumidityText = 0;
static ITUText* mainCInTempText = 0;
static ITUText* mainCInPM25Text = 0;
static ITUText* mainCInCO2Text = 0;
static ITUText* mainCInTVOCText = 0;
static ITUText* mainCOutHumidityText = 0;
static ITUText* mainCOutTempText = 0;
static ITUText* mainCOutPM25Text = 0;
static ITUBackgroundButton* mainCFilterBackgroundButton = 0;
static ITUBackgroundButton* mainCWarningBackgroundButton = 0;
static ITUButton* mainCModeButton = 0;
static ITUIcon* mainCModeButtonIcon = 0;
static ITUSprite* mainModeTextSprite = 0;
static ITUIcon* mainCModeIconSet[MODE_NUM] = { 0 };

static ITUButton* mainCAirForceButton = 0;
static ITUSprite* mainAirForceTextSprite = 0;
static ITUTrackBar* mainAirForceTrackBar = 0;
//static ITUText* mainAirForceValueText = 0;
static ITUProgressBar* mainAirForceProgressBar = 0;
//static ITUCheckBox* mainAirForceAutoCheckBox = 0;
//static ITUSprite* mainAirForceSprite = 0;
//static ITUSprite* mainAirForceLineSprite = 0;
static ITUIcon* mainAirForceTrackBarIcon = 0;
static ITUIcon* mainBarIcon[2] = { 0 };
static ITUBackground* mainAirForceTipShowBackground = 0;
//static ITUText* mainAirForceTipShowText = 0;
static ITUSprite* mainAirForceValueSprite = 0;




static ITUSprite* mainSprite = 0;

static ITUIcon* mainModeShowIcon[MODESHOW_NUM] = { 0 };
static ITUSprite* mainModeShowIconSprite = 0;
static ITUButton* mainCModeShowButton = 0;
static ITUIcon* mainModeShowSubIcon[MODESHOW_NUM] = { 0 };
static ITUSprite* mainModeShowSubIconSprite = 0;
static ITUIcon* mainModeShowLineIcon[MODESHOW_NUM] = { 0 };
static ITUIcon* mainModeShowLineIconSet[MODESHOW_NUM] = { 0 };
static ITUText* mainModeShowText[MODESHOW_NUM] = { 0 };
static ITUSprite* mainModeShowTextSprite = 0;

static ITUTrackBar*	mainTopIndLightTrackBar = 0;
static ITUProgressBar*	mainTopIndLightProgressBar = 0;
static ITUTrackBar*	mainTopScreenLightTrackBar = 0;
static ITUProgressBar*	mainTopScreenLightProgressBar = 0;
static ITUCheckBox*	mainTopAutoCheckBox = 0;
static ITUIcon* mainTopBarBtnIcon[2] = { 0 };
static ITUCheckBox* mainTopTimingCheckBox = 0;


static ITUContainer* mainTimingTextContainer = 0;
static ITUText* mainTimingText = 0;

static ITULayer* powerOffLayer = 0;


static int sprite_index = 1;
extern int mode_show[MODESHOW_NUM] = { 1, 0, 0, 0, 0, 0, 0, 0 };
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
static int PM25Out = 32;
static int DWSpriteIndex = 0;
static int QualitySpriteIndex = 0;
static int InPMDayValue[8] = { 25, 45, 36, 75, 50, 30, 42, 43 };
static int TVOCDayValue[8] = { 1, 0, 2, 1, 3, 2, 0, 1};
static int CO2DayValue[8] = { 700, 690, 1150, 1863, 2500, 1120, 1000, 750 };
static int OutPMDayValue[8] = { 25, 45, 75, 220, 240, 100, 85, 95 };
static int InPMWeekValue[7] = { 25, 45, 75, 50, 30, 45, 50 };
static int TVOCWeekValue[7] = { 1, 0, 2, 1, 3, 2, 1 };
static int CO2WeekValue[7] = { 700, 658, 900, 1000, 850, 725, 750 };
static int OutPMWeekValue[7] = { 25, 45, 75, 220, 85, 300, 368 };
static int InPMDayColor[8] = { 0 };
static int TVOCDayColor[8] = { 0 };
static int CO2DayColor[8] = { 0 };
static int OutPMDayColor[8] = { 0 };
static int InPMWeekColor[7] = { 0 };
static int TVOCWeekColor[7] = { 0 };
static int CO2WeekColor[7] = { 0 };
static int OutPMWeekColor[7] = { 0 };
static int InPMDayH[8] = { 0 };
static int TVOCDayH[8] = { 0 };
static int CO2DayH[8] = { 0 };
static int OutPMDayH[8] = { 0 };
static int InPMWeekH[7] = { 0 };
static int TVOCWeekH[7] = { 0 };
static int CO2WeekH[7] = { 0 };
static int OutPMWeekH[7] = { 0 };
static int airForceIndex = 0;

extern bool lightAuto = false;
extern int screenLight = 50;
extern int indLight = 40;

extern bool indicatorLightEnable = true;
extern int wifi_status = 0;

static int TimeSec = 0;
extern int powerOffTimeIndex = -1;
double lastTime;
extern int powerOffTmHr = 0;
extern int powerOffTmMin = 0;

static uint32_t gtTick = 0, gtLastTick = 0, gtRefreshTime = 3000;
static bool gtTickFirst = true;

#if defined(CFG_SHT20_ENABLE) || defined(CFG_NCP18_ENABLE)
    static float current_tmp_float = 0;
#endif

bool DayQualitySet(ITUWidget* widget, int index, int i0, int i1, int i2, int i3, int i4, int i5, int i6, int i7, ITUIcon* bgIcon);
bool WeekQualitySet(ITUWidget* widget, int index, int i0, int i1, int i2, int i3, int i4, int i5, int i6, ITUIcon* bgIcon);
void drawTriangle(ITUWidget* widget, int x0, int y0, int x1, int y1, int x2, int y2, bool up);

void GotoPowerOff(void);
void MainReset(void)
{
}
bool MainOnEnter(ITUWidget* widget, char* param)
{
	int wifi_established = 0;
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
		
		mainCModeButtonIcon = ituSceneFindWidget(&theScene, "mainCModeButtonIcon");
		assert(mainCModeButtonIcon);

		mainTopTimingCheckBox = ituSceneFindWidget(&theScene, "mainTopTimingCheckBox");
		assert(mainTopTimingCheckBox);

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

		//mainAirForceValueText = ituSceneFindWidget(&theScene, "mainAirForceValueText");
		//assert(mainAirForceValueText);

		mainAirForceProgressBar = ituSceneFindWidget(&theScene, "mainAirForceProgressBar");
		assert(mainAirForceProgressBar);

		//mainAirForceAutoCheckBox = ituSceneFindWidget(&theScene, "mainAirForceAutoCheckBox");
		//assert(mainAirForceAutoCheckBox);

		//mainAirForceSprite = ituSceneFindWidget(&theScene, "mainAirForceSprite");
		//assert(mainAirForceSprite);

		//mainAirForceLineSprite = ituSceneFindWidget(&theScene, "mainAirForceLineSprite");
		//assert(mainAirForceLineSprite);

		mainAirForceTrackBarIcon = ituSceneFindWidget(&theScene, "mainAirForceTrackBarIcon");
		assert(mainAirForceTrackBarIcon);

		mainAirForceTipShowBackground = ituSceneFindWidget(&theScene, "mainAirForceTipShowBackground");
		assert(mainAirForceTipShowBackground);

		//mainAirForceTipShowText = ituSceneFindWidget(&theScene, "mainAirForceTipShowText");
		//assert(mainAirForceTipShowText);

		mainAirForceValueSprite = ituSceneFindWidget(&theScene, "mainAirForceValueSprite");
		assert(mainAirForceValueSprite);

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

		mainLQualityInPMBarIcon = ituSceneFindWidget(&theScene, "mainLQualityInPMBarIcon");
		assert(mainLQualityInPMBarIcon);

		mainLQualityTVOCBarIcon = ituSceneFindWidget(&theScene, "mainLQualityTVOCBarIcon");
		assert(mainLQualityTVOCBarIcon);

		mainLQualityCO2BarIcon = ituSceneFindWidget(&theScene, "mainLQualityCO2BarIcon");
		assert(mainLQualityCO2BarIcon);

		mainLQualityOutPMBarIcon = ituSceneFindWidget(&theScene, "mainLQualityOutPMBarIcon");
		assert(mainLQualityOutPMBarIcon);

		mainLQualityInPMWBarIcon = ituSceneFindWidget(&theScene, "mainLQualityInPMWBarIcon");
		assert(mainLQualityInPMWBarIcon);

		mainLQualityTVOCWBarIcon = ituSceneFindWidget(&theScene, "mainLQualityTVOCWBarIcon");
		assert(mainLQualityTVOCWBarIcon);

		mainLQualityCO2WBarIcon = ituSceneFindWidget(&theScene, "mainLQualityCO2WBarIcon");
		assert(mainLQualityCO2WBarIcon);

		mainLQualityOutPMWBarIcon = ituSceneFindWidget(&theScene, "mainLQualityOutPMWBarIcon");
		assert(mainLQualityOutPMWBarIcon);

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

		mainLeftBackground = ituSceneFindWidget(&theScene, "mainLeftBackground");
		assert(mainLeftBackground);

		mainLQualityInPMDayBgIcon = ituSceneFindWidget(&theScene, "mainLQualityInPMDayBgIcon");
		assert(mainLQualityInPMDayBgIcon);

		mainLQualityCO2DayBgIcon = ituSceneFindWidget(&theScene, "mainLQualityCO2DayBgIcon");
		assert(mainLQualityCO2DayBgIcon);

		mainLQualityOutPMDayBgIcon = ituSceneFindWidget(&theScene, "mainLQualityOutPMDayBgIcon");
		assert(mainLQualityOutPMDayBgIcon);

		mainLQualityInPMWeekBgIcon = ituSceneFindWidget(&theScene, "mainLQualityInPMWeekBgIcon");
		assert(mainLQualityInPMWeekBgIcon);

		mainLQualityCO2WeekBgIcon = ituSceneFindWidget(&theScene, "mainLQualityCO2WeekBgIcon");
		assert(mainLQualityCO2WeekBgIcon);

		mainLQualityOutPMWeekBgIcon = ituSceneFindWidget(&theScene, "mainLQualityOutPMWeekBgIcon");
		assert(mainLQualityOutPMWeekBgIcon);

		mainModeShowIconSprite = ituSceneFindWidget(&theScene, "mainModeShowIconSprite");
		assert(mainModeShowIconSprite);

		mainModeShowSubIconSprite = ituSceneFindWidget(&theScene, "mainModeShowSubIconSprite");
		assert(mainModeShowSubIconSprite);

		mainModeShowTextSprite = ituSceneFindWidget(&theScene, "mainModeShowTextSprite");
		assert(mainModeShowTextSprite);

		mainTimingTextContainer = ituSceneFindWidget(&theScene, "mainTimingTextContainer");
		assert(mainTimingTextContainer);

		mainTimingText = ituSceneFindWidget(&theScene, "mainTimingText");
		assert(mainTimingText);


		for (i = 0; i < (MODE_NUM); i++)
		{
			sprintf(tmp, "mainCModeIcon%d", i);
			mainCModeIconSet[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainCModeIconSet[i]);
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
			sprintf(tmp, "mainLQualityInPMIcon%d", i);
			mainLQualityInPMIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityInPMIcon[i]);

			sprintf(tmp, "mainLQualityTVOCIcon%d", i);
			mainLQualityTVOCIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityTVOCIcon[i]);

			sprintf(tmp, "mainLQualityCO2Icon%d", i);
			mainLQualityCO2Icon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityCO2Icon[i]);

			sprintf(tmp, "mainLQualityOutPMIcon%d", i);
			mainLQualityOutPMIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityOutPMIcon[i]);

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
			sprintf(tmp, "mainLQualityInPMWIcon%d", i);
			mainLQualityInPMWIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityInPMWIcon[i]);

			sprintf(tmp, "mainLQualityTVOCWIcon%d", i);
			mainLQualityTVOCWIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityTVOCWIcon[i]);

			sprintf(tmp, "mainLQualityCO2WIcon%d", i);
			mainLQualityCO2WIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityCO2WIcon[i]);

			sprintf(tmp, "mainLQualityOutPMWIcon%d", i);
			mainLQualityOutPMWIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityOutPMWIcon[i]);

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

			sprintf(tmp, "mainLQualityBIcon%d", i);
			mainLQualityBIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityBIcon[i]);

			sprintf(tmp, "mainLQualityIcon%d", i);
			mainLQualityIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLQualityIcon[i]);

			


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

	sprintf(tmp, "%d", PM25Out);
	ituTextSetString(mainCOutPM25Text, tmp);

	sprintf(tmp, "%d", temperatureOutIndex);
	ituTextSetString(mainCOutTempText, tmp);

	sprintf(tmp, "%d", humidityOutIndex);
	ituTextSetString(mainCOutHumidityText, tmp);


	ituWidgetSetVisible(mainCWarningBackgroundButton, warning_btn_show);
	ituWidgetSetVisible(mainCFilterBackgroundButton, filter_btn_show);


	ituSpriteGoto(mainModeTextSprite, modeIndex);
	ituIconLinkSurface(mainCModeButtonIcon, mainCModeIconSet[modeIndex]);
	ituCheckBoxSetChecked(mainTopTimingCheckBox, timingSet);
	
	
	
	modeshowPosX = 0;
	modeshowNum = 0;
	for (i = 0; i < MODESHOW_NUM; i++)
	{
		if (mode_show[i])
		{
			if (i == 0)
			{
				ituWidgetSetPosition(mainModeShowIconSprite, modeshowPosX, 0);
				ituWidgetSetPosition(mainModeShowSubIconSprite, modeshowPosX, 0);
			}
			else
			{
				ituWidgetSetPosition(mainModeShowIcon[i], modeshowPosX, 0);
				ituWidgetSetPosition(mainModeShowSubIcon[i], modeshowPosX, 0);
			}
			
			modeshowNum++;
			if (i != 7)
				modeshowPosX = modeshowPosX + 46;
			else
				modeshowPosX = modeshowPosX + 88;
		}
		else
		{
			if (i == 0)
			{
				ituWidgetSetPosition(mainModeShowIconSprite, 0, 100);
				ituWidgetSetPosition(mainModeShowSubIconSprite, 0, 100);
			}
			else
			{
				ituWidgetSetPosition(mainModeShowIcon[i], 0, 100);
				ituWidgetSetPosition(mainModeShowSubIcon[i], 0, 100);
			}
			
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
	ituWidgetSetVisible(mainLQualityInPMBarIcon, false);
	ituWidgetSetVisible(mainLQualityTVOCBarIcon, false);
	ituWidgetSetVisible(mainLQualityCO2BarIcon, false);
	ituWidgetSetVisible(mainLQualityOutPMBarIcon, false);
	ituWidgetSetVisible(mainLQualityInPMWBarIcon, false);
	ituWidgetSetVisible(mainLQualityTVOCWBarIcon, false);
	ituWidgetSetVisible(mainLQualityCO2WBarIcon, false);
	ituWidgetSetVisible(mainLQualityOutPMWBarIcon, false);

	sprintf(tmp, "%d", humidityInIndex);
	ituTextSetString(mainCInHumidityText, tmp);

	ituSpriteGoto(mainAirForceTextSprite, airForceIndex);
	//ituIconLinkSurface(&mainCAirForceButton->bg.icon, mainCAirForceIconSet[airForceIndex]);

	ituWidgetSetVisible(mainAirForceTipShowBackground, false);
	ituWidgetGetGlobalPosition(mainAirForceTrackBar->tip, &x, &y);
	ituWidgetSetPosition(mainAirForceTipShowBackground, x, y);
	//sprintf(tmp, "%d", mainAirForceTrackBar->value);
	//ituTextSetString(mainAirForceTipShowText, tmp);
	ituSpriteGoto(mainAirForceValueSprite, mainAirForceTrackBar->value);
	ituWidgetSetVisible(mainAirForceTipShowBackground, true);

	ituWidgetSetPosition(mainLQualityInPMDayBgIcon, 720, 720);
	ituWidgetSetPosition(mainLQualityCO2DayBgIcon, 720, 720);
	ituWidgetSetPosition(mainLQualityOutPMDayBgIcon, 720, 720);
	ituWidgetSetPosition(mainLQualityInPMWeekBgIcon, 720, 720);
	ituWidgetSetPosition(mainLQualityCO2WeekBgIcon, 720, 720);
	ituWidgetSetPosition(mainLQualityOutPMWeekBgIcon, 720, 720);

	powerOnTimeIndex = -1;
	powerOnTmHr = 0;
	powerOnTmMin = 0;

	if (powerOffTimeIndex > -1)
		ituWidgetSetVisible(mainTimingTextContainer, true);
	else
		ituWidgetSetVisible(mainTimingTextContainer, false);

	ituProgressBarSetValue(mainTopScreenLightProgressBar, screenLight);
	ituTrackBarSetValue(mainTopScreenLightTrackBar, screenLight);
	ituProgressBarSetValue(mainTopIndLightProgressBar, indLight);
	ituTrackBarSetValue(mainTopIndLightTrackBar, indLight);

	if (!indicatorLightEnable)
	{
		ituIconLinkSurface(&mainTopIndLightTrackBar->tracker->bg.icon, mainTopBarBtnIcon[0]);

		ituWidgetDisable(mainTopIndLightTrackBar);
		ituWidgetSetVisible(mainTopIndLightProgressBar, false);
	}

	if (lightAuto)
	{
		ituCheckBoxSetChecked(mainTopAutoCheckBox, true);
		ituIconLinkSurface(&mainTopScreenLightTrackBar->tracker->bg.icon, mainTopBarBtnIcon[0]);


		ituWidgetDisable(mainTopScreenLightTrackBar);
		ituWidgetSetVisible(mainTopScreenLightProgressBar, false);


		if (indicatorLightEnable)
		{
			ituIconLinkSurface(&mainTopIndLightTrackBar->tracker->bg.icon, mainTopBarBtnIcon[0]);
			ituWidgetDisable(mainTopIndLightTrackBar);
			ituWidgetSetVisible(mainTopIndLightProgressBar, false);
		}
	}
	else
	{
		ituCheckBoxSetChecked(mainTopAutoCheckBox, false);
		ituIconLinkSurface(&mainTopScreenLightTrackBar->tracker->bg.icon, mainTopBarBtnIcon[1]);
		ituWidgetEnable(mainTopScreenLightTrackBar);
		ituWidgetSetVisible(mainTopScreenLightProgressBar, true);

		if (indicatorLightEnable)
		{
			ituIconLinkSurface(&mainTopIndLightTrackBar->tracker->bg.icon, mainTopBarBtnIcon[1]);
			ituWidgetEnable(mainTopIndLightTrackBar);
			ituWidgetSetVisible(mainTopIndLightProgressBar, true);
		}
	}

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

	    if (wifi_established)
	        //ituSpriteGoto(topWiFiSprite, 2); //0: wifi is off, 1: wifi is on but no connection, 2: connection is established
			wifi_status = 0;
		else
            //ituSpriteGoto(topWiFiSprite, 1);
			wifi_status = 1;

	}
	else
	{
		//ituSpriteGoto(topWiFiSprite, 0); //0: wifi is off, 1: wifi is on but no connection, 2: connection is established
		wifi_status = 2;

	}
	ituSpriteGoto(mainModeShowIconSprite, wifi_status);
	ituSpriteGoto(mainModeShowSubIconSprite, wifi_status);
	ituSpriteGoto(mainModeShowTextSprite, wifi_status);
	return true;
}
bool MainOnTimer(ITUWidget* widget, char* param)
{
	bool ret = false;
	char tmp[32];

	struct timeval tv;
	struct tm *tm;
	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	int curSec = 59 - tm->tm_sec;

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

	if (powerOffTimeIndex != -1)
	{
		if (powerOffTmHr == tm->tm_hour && powerOffTmMin == tm->tm_min)
		{
			ituWidgetSetVisible(mainTimingTextContainer, false);
			GotoPowerOff();

		}

		if (curSec != TimeSec)
		{
			TimeSec = curSec;

			if (powerOffTmMin >= tm->tm_min)
			{
				lastTime = (double)(powerOffTmMin - tm->tm_min) / 60;

				if (powerOffTmHr >= tm->tm_hour)
				{
					lastTime = lastTime + (powerOffTmHr - tm->tm_hour);
				}
				else
				{
					lastTime = lastTime + (powerOffTmHr + 24 - tm->tm_hour);
				}
			}
			else
			{
				lastTime = (double)(powerOffTmMin + 60 - tm->tm_min) / 60;

				if ((powerOffTmHr - 1) >= tm->tm_hour)
				{
					lastTime = lastTime + (powerOffTmHr - 1 - tm->tm_hour);
				}
				else
				{
					lastTime = lastTime + (powerOffTmHr + 23 - tm->tm_hour);
				}

			}


			sprintf(tmp, "%2.1f", lastTime);
			ituTextSetString(mainTimingText, tmp);

			ret = true;
		}
	}


	return ret;
}
bool MainCoverFlowOnChanged(ITUWidget* widget, char* param)
{
	//int i;

	sprite_index = mainCoverFlow->focusIndex;
	ituSpriteGoto(mainSprite, sprite_index);

	if (sprite_index == 0)
	{
		ituWidgetSetVisible(mainLQualityInPMNumSprite, false);
		ituWidgetSetVisible(mainLQualityTVOCNumSprite, false);
		ituWidgetSetVisible(mainLQualityCO2NumSprite, false);
		ituWidgetSetVisible(mainLQualityOutPMNumSprite, false);
		ituWidgetSetVisible(mainLQualityInPMWNumSprite, false);
		ituWidgetSetVisible(mainLQualityTVOCWNumSprite, false);
		ituWidgetSetVisible(mainLQualityCO2WNumSprite, false);
		ituWidgetSetVisible(mainLQualityOutPMWNumSprite, false);
		ituWidgetSetVisible(mainLQualityInPMBarIcon, false);
		ituWidgetSetVisible(mainLQualityTVOCBarIcon, false);
		ituWidgetSetVisible(mainLQualityCO2BarIcon, false);
		ituWidgetSetVisible(mainLQualityOutPMBarIcon, false);
		ituWidgetSetVisible(mainLQualityInPMWBarIcon, false);
		ituWidgetSetVisible(mainLQualityTVOCWBarIcon, false);
		ituWidgetSetVisible(mainLQualityCO2WBarIcon, false);
		ituWidgetSetVisible(mainLQualityOutPMWBarIcon, false);
		

		ituSpriteGoto(mainLQualitySprite, 0);
		ituSpriteGoto(mainLDWSprite, 0);
		ituRadioBoxSetChecked(mainLQualityRadioBox[0], true);
		DayQualitySet((ITUWidget*) mainLQualityInPMDayIcon, 0, InPMDayValue[0], InPMDayValue[1], InPMDayValue[2], InPMDayValue[3], InPMDayValue[4], InPMDayValue[5], InPMDayValue[6], InPMDayValue[7], mainLQualityInPMDayBgIcon);

	}
	return true;
}
bool MainCModeButtonOnMouseUp(ITUWidget* widget, char* param)
{
	mainCoverFlow->slideMaxCount = 0;
	return true;
}
bool MainCAirForceButtonOnMouseUp(ITUWidget* widget, char* param)
{
	mainCoverFlow->slideMaxCount = 0;
	return true;
}
bool MainAttachButtonOnMouseUp(ITUWidget* widget, char* param)
{
	mainCoverFlow->slideMaxCount = 0;
	return true;
}
bool MainAirForceHideButtonOnMouseUp(ITUWidget* widget, char* param)
{
	mainCoverFlow->slideMaxCount = 1;
	return true;
}
bool MainCModeSelectHideButtonOnSlideDown(ITUWidget* widget, char* param)
{
	mainCoverFlow->slideMaxCount = 1;
	return true;
}
bool MainAttachHideButtonOnSlideDown(ITUWidget* widget, char* param)
{
	mainCoverFlow->slideMaxCount = 1;
	return true;
}
bool MainCModeRadBoxOnMouseUp(ITUWidget* widget, char* param)
{
	modeIndex = atoi(param);
	ituSpriteGoto(mainModeTextSprite, modeIndex);
	ituIconLinkSurface(mainCModeButtonIcon, mainCModeIconSet[modeIndex]);


	mainCoverFlow->slideMaxCount = 1;
	return true;
}

bool MainTopIndLightTrackBarOnChanged(ITUWidget* widget, char* param)
{
	indLight = mainTopIndLightTrackBar->value;
	return true;
}

bool MainTopScreenLightTrackBarOnChanged(ITUWidget* widget, char* param)
{
	screenLight = mainTopScreenLightTrackBar->value;
	printf("mainTopScreenLightTrackBar %d \n", mainTopScreenLightTrackBar->value);
	ScreenSetBrightness(mainTopScreenLightTrackBar->value);

	return true;
}

bool MainTopAutoChkBoxOnPress(ITUWidget* widget, char* param)
{
	if (ituCheckBoxIsChecked(mainTopAutoCheckBox))
	{
		lightAuto = true;
		ituIconLinkSurface(&mainTopScreenLightTrackBar->tracker->bg.icon, mainTopBarBtnIcon[0]);
		

		ituWidgetDisable(mainTopScreenLightTrackBar);
		ituWidgetSetVisible(mainTopScreenLightProgressBar, false);
		

		if (indicatorLightEnable)
		{
			ituIconLinkSurface(&mainTopIndLightTrackBar->tracker->bg.icon, mainTopBarBtnIcon[0]);
			ituWidgetDisable(mainTopIndLightTrackBar);
			ituWidgetSetVisible(mainTopIndLightProgressBar, false);
		}
	}
	else
	{
		lightAuto = false;
		ituIconLinkSurface(&mainTopScreenLightTrackBar->tracker->bg.icon, mainTopBarBtnIcon[1]);
		ituWidgetEnable(mainTopScreenLightTrackBar);
		ituWidgetSetVisible(mainTopScreenLightProgressBar, true);
		
		if (indicatorLightEnable)
		{
			ituIconLinkSurface(&mainTopIndLightTrackBar->tracker->bg.icon, mainTopBarBtnIcon[1]);
			ituWidgetEnable(mainTopIndLightTrackBar);
			ituWidgetSetVisible(mainTopIndLightProgressBar, true);
		}
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
			
			if (i == 0)
			{
				ituWidgetSetPosition(mainModeShowLineIcon[i], ituWidgetGetX(mainModeShowSubIconSprite), 0);
				ituWidgetSetPosition(mainModeShowTextSprite, ituWidgetGetX(mainModeShowSubIconSprite), (modeshowNumTmp - 1) * 60);
			}
				
			else
			{
				ituWidgetSetPosition(mainModeShowLineIcon[i], ituWidgetGetX(mainModeShowSubIcon[i]), 0);
				ituWidgetSetPosition(mainModeShowText[i], ituWidgetGetX(mainModeShowSubIcon[i]), (modeshowNumTmp - 1) * 60);
			}
				
			modeshowNumTmp--;

		}
		else
		{
			ituWidgetSetPosition(mainModeShowLineIcon[i], 0, 501);
			if (i == 0)
				ituWidgetSetPosition(mainModeShowTextSprite, 0, 500);
			else				
				ituWidgetSetPosition(mainModeShowText[i], 0, 500);
			
		}
	}

	return true;
}


bool MainLQualityRadioBoxOnMouseUp(ITUWidget* widget, char* param)
{
	ituWidgetSetVisible(mainLQualityInPMNumSprite, false);
	ituWidgetSetVisible(mainLQualityTVOCNumSprite, false);
	ituWidgetSetVisible(mainLQualityCO2NumSprite, false);
	ituWidgetSetVisible(mainLQualityOutPMNumSprite, false);
	ituWidgetSetVisible(mainLQualityInPMWNumSprite, false);
	ituWidgetSetVisible(mainLQualityTVOCWNumSprite, false);
	ituWidgetSetVisible(mainLQualityCO2WNumSprite, false);
	ituWidgetSetVisible(mainLQualityOutPMWNumSprite, false);
	ituWidgetSetVisible(mainLQualityInPMBarIcon, false);
	ituWidgetSetVisible(mainLQualityTVOCBarIcon, false);
	ituWidgetSetVisible(mainLQualityCO2BarIcon, false);
	ituWidgetSetVisible(mainLQualityOutPMBarIcon, false);
	ituWidgetSetVisible(mainLQualityInPMWBarIcon, false);
	ituWidgetSetVisible(mainLQualityTVOCWBarIcon, false);
	ituWidgetSetVisible(mainLQualityCO2WBarIcon, false);
	ituWidgetSetVisible(mainLQualityOutPMWBarIcon, false);

	QualitySpriteIndex = DWSpriteIndex * 4 + atoi(param);

	ituSpriteGoto(mainLQualitySprite, QualitySpriteIndex);

	if (DWSpriteIndex == 0)
	{
		switch (atoi(param))
		{
		case 0:
			//DayQualitySet(mainLQualityInPMDayIcon, 0, 280, 230, 250, 191, 220, 265, 230, 220);
			DayQualitySet((ITUWidget*)mainLQualityInPMDayIcon, 0, InPMDayValue[0], InPMDayValue[1], InPMDayValue[2], InPMDayValue[3], InPMDayValue[4], InPMDayValue[5], InPMDayValue[6], InPMDayValue[7], mainLQualityInPMDayBgIcon);
			break;
		case 1:
			//DayQualitySet(mainLQualityTVOCDayIcon, 1, 165, 240, 90, 165, 10, 90, 200, 165);
			DayQualitySet((ITUWidget*)mainLQualityTVOCDayIcon, 1, TVOCDayValue[0], TVOCDayValue[1], TVOCDayValue[2], TVOCDayValue[3], TVOCDayValue[4], TVOCDayValue[5], TVOCDayValue[6], TVOCDayValue[7], NULL);
			break;
		case 2:
			//DayQualitySet(mainLQualityCO2DayIcon, 2, 280, 280, 200, 165, 155, 200, 230, 260);
			DayQualitySet((ITUWidget*)mainLQualityCO2DayIcon, 2, CO2DayValue[0], CO2DayValue[1], CO2DayValue[2], CO2DayValue[3], CO2DayValue[4], CO2DayValue[5], CO2DayValue[6], CO2DayValue[7], mainLQualityCO2DayBgIcon);
			break;
		case 3:
			//DayQualitySet(mainLQualityOutPMDayIcon, 3, 280, 230, 191, 100, 100, 180, 185, 180);
			DayQualitySet((ITUWidget*)mainLQualityOutPMDayIcon, 3, OutPMDayValue[0], OutPMDayValue[1], OutPMDayValue[2], OutPMDayValue[3], OutPMDayValue[4], OutPMDayValue[5], OutPMDayValue[6], OutPMDayValue[7], mainLQualityOutPMDayBgIcon);
			break;

		}
	}
	else
	{
		switch (atoi(param))
		{
		case 0:
			//WeekQualitySet(mainLQualityInPMWeekIcon, 0, 280, 220, 191, 215, 270, 240, 235);
			WeekQualitySet((ITUWidget*)mainLQualityInPMWeekIcon, 0, InPMWeekValue[0], InPMWeekValue[1], InPMWeekValue[2], InPMWeekValue[3], InPMWeekValue[4], InPMWeekValue[5], InPMWeekValue[6], mainLQualityInPMWeekBgIcon);
			break;
		case 1:
			//WeekQualitySet(mainLQualityTVOCWeekIcon, 1, 165, 240, 90, 165, 10, 90, 165);
			WeekQualitySet((ITUWidget*)mainLQualityTVOCWeekIcon, 1, TVOCWeekValue[0], TVOCWeekValue[1], TVOCWeekValue[2], TVOCWeekValue[3], TVOCWeekValue[4], TVOCWeekValue[5], TVOCWeekValue[6], NULL);
			break;
		case 2:
			//WeekQualitySet(mainLQualityCO2WeekIcon, 2, 280, 285, 220, 215, 225, 270, 260);
			WeekQualitySet((ITUWidget*)mainLQualityCO2WeekIcon, 2, CO2WeekValue[0], CO2WeekValue[1], CO2WeekValue[2], CO2WeekValue[3], CO2WeekValue[4], CO2WeekValue[5], CO2WeekValue[6], mainLQualityCO2WeekBgIcon);
			break;
		case 3:
			//WeekQualitySet(mainLQualityOutPMWeekIcon, 3, 280, 220, 191, 150, 185, 100, 95);
			WeekQualitySet((ITUWidget*)mainLQualityOutPMWeekIcon, 3, OutPMWeekValue[0], OutPMWeekValue[1], OutPMWeekValue[2], OutPMWeekValue[3], OutPMWeekValue[4], OutPMWeekValue[5], OutPMWeekValue[6], mainLQualityOutPMWeekBgIcon);
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
	ituWidgetSetVisible(mainLQualityOutPMWBarIcon, false);
	ituWidgetSetVisible(mainLQualityInPMBarIcon, false);
	ituWidgetSetVisible(mainLQualityTVOCBarIcon, false);
	ituWidgetSetVisible(mainLQualityCO2BarIcon, false);
	ituWidgetSetVisible(mainLQualityOutPMBarIcon, false);
	ituWidgetSetVisible(mainLQualityInPMWBarIcon, false);
	ituWidgetSetVisible(mainLQualityTVOCWBarIcon, false);
	ituWidgetSetVisible(mainLQualityCO2WBarIcon, false);
	ituWidgetSetVisible(mainLQualityOutPMWBarIcon, false);
	

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
		DayQualitySet((ITUWidget*)mainLQualityInPMDayIcon, 0, InPMDayValue[0], InPMDayValue[1], InPMDayValue[2], InPMDayValue[3], InPMDayValue[4], InPMDayValue[5], InPMDayValue[6], InPMDayValue[7], mainLQualityInPMDayBgIcon);
		break;
	case 1:
		//DayQualitySet(mainLQualityTVOCDayIcon, 1, 165, 240, 90, 165, 10, 90, 200, 165);
		DayQualitySet((ITUWidget*)mainLQualityTVOCDayIcon, 1, TVOCDayValue[0], TVOCDayValue[1], TVOCDayValue[2], TVOCDayValue[3], TVOCDayValue[4], TVOCDayValue[5], TVOCDayValue[6], TVOCDayValue[7], NULL);
		break;
	case 2:
		//DayQualitySet(mainLQualityCO2DayIcon, 2, 280, 280, 200, 165, 155, 200, 230, 260);
		DayQualitySet((ITUWidget*)mainLQualityCO2DayIcon, 2, CO2DayValue[0], CO2DayValue[1], CO2DayValue[2], CO2DayValue[3], CO2DayValue[4], CO2DayValue[5], CO2DayValue[6], CO2DayValue[7], mainLQualityCO2DayBgIcon);
		break;
	case 3:
		//DayQualitySet(mainLQualityOutPMDayIcon, 3, 280, 230, 191, 100, 100, 180, 185, 180);
		DayQualitySet((ITUWidget*)mainLQualityOutPMDayIcon, 3, OutPMDayValue[0], OutPMDayValue[1], OutPMDayValue[2], OutPMDayValue[3], OutPMDayValue[4], OutPMDayValue[5], OutPMDayValue[6], OutPMDayValue[7], mainLQualityOutPMDayBgIcon);
		break;
	case 4:
		//WeekQualitySet(mainLQualityInPMWeekIcon, 0, 280, 220, 191, 215, 270, 240, 235);
		WeekQualitySet((ITUWidget*)mainLQualityInPMWeekIcon, 0, InPMWeekValue[0], InPMWeekValue[1], InPMWeekValue[2], InPMWeekValue[3], InPMWeekValue[4], InPMWeekValue[5], InPMWeekValue[6], mainLQualityInPMWeekBgIcon);
		break;
	case 5:
		//WeekQualitySet(mainLQualityTVOCWeekIcon, 1, 165, 240, 90, 165, 10, 90, 165);
		WeekQualitySet((ITUWidget*)mainLQualityTVOCWeekIcon, 1, TVOCWeekValue[0], TVOCWeekValue[1], TVOCWeekValue[2], TVOCWeekValue[3], TVOCWeekValue[4], TVOCWeekValue[5], TVOCWeekValue[6], NULL);
		break;
	case 6:
		//WeekQualitySet(mainLQualityCO2WeekIcon, 2, 280, 280, 200, 165, 155, 200, 230);
		WeekQualitySet((ITUWidget*)mainLQualityCO2WeekIcon, 2, CO2WeekValue[0], CO2WeekValue[1], CO2WeekValue[2], CO2WeekValue[3], CO2WeekValue[4], CO2WeekValue[5], CO2WeekValue[6], mainLQualityCO2WeekBgIcon);
		break;
	case 7:
		//WeekQualitySet(mainLQualityOutPMWeekIcon, 3, 280, 220, 191, 150, 185, 100, 95);
		WeekQualitySet((ITUWidget*)mainLQualityOutPMWeekIcon, 3, OutPMWeekValue[0], OutPMWeekValue[1], OutPMWeekValue[2], OutPMWeekValue[3], OutPMWeekValue[4], OutPMWeekValue[5], OutPMWeekValue[6], mainLQualityOutPMWeekBgIcon);
		break;

	}

	


	return true;
}
bool MainLQualityInPMBtnOnPress(ITUWidget* widget, char* param)
{
	int index = atoi(param);
	char tmp[32];

	ituWidgetSetHeight(mainLQualityInPMBarIcon, InPMDayH[index]);
	ituWidgetSetPosition(mainLQualityInPMBarIcon, ituWidgetGetX(mainLQualityInPMIcon[index]), ituWidgetGetY(mainLQualityInPMIcon[index]) + (ituWidgetGetHeight(mainLQualityInPMIcon[index]) / 2));
	ituWidgetSetPosition(mainLQualityInPMNumSprite, ituWidgetGetX(mainLQualityInPMIcon[index]) - 28, ituWidgetGetY(mainLQualityInPMIcon[index]) - 70);
	sprintf(tmp, "%d", InPMDayValue[index]);
	ituTextSetString(mainLQualityInPMNumText[InPMDayColor[index]], tmp);
	ituSpriteGoto(mainLQualityInPMNumSprite, InPMDayColor[index]);
	ituWidgetSetVisible(mainLQualityInPMNumSprite, true);
	ituWidgetSetVisible(mainLQualityInPMBarIcon, true);


	return true;
}

bool MainLQualityTVOCBtnOnPress(ITUWidget* widget, char* param)
{
	int index = atoi(param);
	char tmp[32];

	
	

	sprintf(tmp, "%d", TVOCDayValue[index]);
	ituTextSetString(mainLQualityTVOCNumText[TVOCDayColor[index]], tmp);
	ituSpriteGoto(mainLQualityTVOCNumSprite, TVOCDayColor[index]);

	ituWidgetSetHeight(mainLQualityTVOCBarIcon, TVOCDayH[index]);
	ituWidgetSetPosition(mainLQualityTVOCBarIcon, ituWidgetGetX(mainLQualityTVOCIcon[index]), ituWidgetGetY(mainLQualityTVOCIcon[index]) + (ituWidgetGetHeight(mainLQualityTVOCIcon[index]) / 2));
	ituWidgetSetPosition(mainLQualityTVOCNumSprite, ituWidgetGetX(mainLQualityTVOCIcon[index]), ituWidgetGetY(mainLQualityTVOCIcon[index]));
	ituWidgetSetVisible(mainLQualityTVOCNumSprite, true);
	ituWidgetSetVisible(mainLQualityTVOCBarIcon, true);

	return true;
}

bool MainLQualityCO2BtnOnPress(ITUWidget* widget, char* param)
{
	int index = atoi(param);
	char tmp[32];

	ituWidgetSetHeight(mainLQualityCO2BarIcon, CO2DayH[index]);
	ituWidgetSetPosition(mainLQualityCO2BarIcon, ituWidgetGetX(mainLQualityCO2Icon[index]), ituWidgetGetY(mainLQualityCO2Icon[index]) + (ituWidgetGetHeight(mainLQualityCO2Icon[index]) / 2));
	ituWidgetSetPosition(mainLQualityCO2NumSprite, ituWidgetGetX(mainLQualityCO2Icon[index]) - 28, ituWidgetGetY(mainLQualityCO2Icon[index]) - 70);
	sprintf(tmp, "%d", CO2DayValue[index]);
	ituTextSetString(mainLQualityCO2NumText[CO2DayColor[index]], tmp);
	ituSpriteGoto(mainLQualityCO2NumSprite, CO2DayColor[index]);
	ituWidgetSetVisible(mainLQualityCO2NumSprite, true);
	ituWidgetSetVisible(mainLQualityCO2BarIcon, true);


	return true;
}

bool MainLQualityOutPMBtnOnPress(ITUWidget* widget, char* param)
{
	int index = atoi(param);
	char tmp[32];

	ituWidgetSetHeight(mainLQualityOutPMBarIcon, OutPMDayH[index]);
	ituWidgetSetPosition(mainLQualityOutPMBarIcon, ituWidgetGetX(mainLQualityOutPMIcon[index]), ituWidgetGetY(mainLQualityOutPMIcon[index]) + (ituWidgetGetHeight(mainLQualityOutPMIcon[index]) / 2));
	ituWidgetSetPosition(mainLQualityOutPMNumSprite, ituWidgetGetX(mainLQualityOutPMIcon[index]) - 28, ituWidgetGetY(mainLQualityOutPMIcon[index]) - 70);
	sprintf(tmp, "%d", OutPMDayValue[index]);
	ituTextSetString(mainLQualityOutPMNumText[OutPMDayColor[index]], tmp);
	ituSpriteGoto(mainLQualityOutPMNumSprite, OutPMDayColor[index]);
	ituWidgetSetVisible(mainLQualityOutPMNumSprite, true);
	ituWidgetSetVisible(mainLQualityOutPMBarIcon, true);

	return true;
}

bool MainLQualityInPMWBtnOnPress(ITUWidget* widget, char* param)
{
	int index = atoi(param);
	char tmp[32];

	ituWidgetSetHeight(mainLQualityInPMWBarIcon, InPMWeekH[index]);
	ituWidgetSetPosition(mainLQualityInPMWBarIcon, ituWidgetGetX(mainLQualityInPMWIcon[index]), ituWidgetGetY(mainLQualityInPMWIcon[index]) + (ituWidgetGetHeight(mainLQualityInPMWIcon[index]) / 2));
	ituWidgetSetPosition(mainLQualityInPMWNumSprite, ituWidgetGetX(mainLQualityInPMWIcon[index]) - 28, ituWidgetGetY(mainLQualityInPMWIcon[index]) - 70);
	sprintf(tmp, "%d", InPMWeekValue[index]);
	ituTextSetString(mainLQualityInPMWNumText[InPMWeekColor[index]], tmp);
	ituSpriteGoto(mainLQualityInPMWNumSprite, InPMWeekColor[index]);
	ituWidgetSetVisible(mainLQualityInPMWNumSprite, true);
	ituWidgetSetVisible(mainLQualityInPMWBarIcon, true);


	return true;
}

bool MainLQualityTVOCWBtnOnPress(ITUWidget* widget, char* param)
{
	int index = atoi(param);
	char tmp[32];




	sprintf(tmp, "%d", TVOCWeekValue[index]);
	ituTextSetString(mainLQualityTVOCWNumText[TVOCWeekColor[index]], tmp);
	ituSpriteGoto(mainLQualityTVOCWNumSprite, TVOCWeekColor[index]);

	ituWidgetSetHeight(mainLQualityTVOCWBarIcon, TVOCWeekH[index]);
	ituWidgetSetPosition(mainLQualityTVOCWBarIcon, ituWidgetGetX(mainLQualityTVOCWIcon[index]), ituWidgetGetY(mainLQualityTVOCWIcon[index]) + (ituWidgetGetHeight(mainLQualityTVOCWIcon[index]) / 2));
	ituWidgetSetPosition(mainLQualityTVOCWNumSprite, ituWidgetGetX(mainLQualityTVOCWIcon[index]), ituWidgetGetY(mainLQualityTVOCWIcon[index]));
	ituWidgetSetVisible(mainLQualityTVOCWNumSprite, true);
	ituWidgetSetVisible(mainLQualityTVOCWBarIcon, true);


	return true;
}

bool MainLQualityCO2WBtnOnPress(ITUWidget* widget, char* param)
{
	int index = atoi(param);
	char tmp[32];

	ituWidgetSetHeight(mainLQualityCO2WBarIcon, CO2WeekH[index]);
	ituWidgetSetPosition(mainLQualityCO2WBarIcon, ituWidgetGetX(mainLQualityCO2WIcon[index]), ituWidgetGetY(mainLQualityCO2WIcon[index]) + (ituWidgetGetHeight(mainLQualityCO2WIcon[index]) / 2));
	ituWidgetSetPosition(mainLQualityCO2WNumSprite, ituWidgetGetX(mainLQualityCO2WIcon[index]) - 28, ituWidgetGetY(mainLQualityCO2WIcon[index]) - 70);
	sprintf(tmp, "%d", CO2WeekValue[index]);
	ituTextSetString(mainLQualityCO2WNumText[CO2WeekColor[index]], tmp);
	ituSpriteGoto(mainLQualityCO2WNumSprite, CO2WeekColor[index]);
	ituWidgetSetVisible(mainLQualityCO2WNumSprite, true);
	ituWidgetSetVisible(mainLQualityCO2WBarIcon, true);


	return true;
}

bool MainLQualityOutPMWBtnOnPress(ITUWidget* widget, char* param)
{
	int index = atoi(param);
	char tmp[32];
	
	ituWidgetSetHeight(mainLQualityOutPMWBarIcon, OutPMWeekH[index]);
	ituWidgetSetPosition(mainLQualityOutPMWBarIcon, ituWidgetGetX(mainLQualityOutPMWIcon[index]), ituWidgetGetY(mainLQualityOutPMWIcon[index]) + (ituWidgetGetHeight(mainLQualityOutPMWIcon[index]) / 2));
	ituWidgetSetPosition(mainLQualityOutPMWNumSprite, ituWidgetGetX(mainLQualityOutPMWIcon[index]) - 28, ituWidgetGetY(mainLQualityOutPMWIcon[index]) - 70);
	sprintf(tmp, "%d", OutPMWeekValue[index]);
	ituTextSetString(mainLQualityOutPMWNumText[OutPMWeekColor[index]], tmp);
	ituSpriteGoto(mainLQualityOutPMWNumSprite, OutPMWeekColor[index]);
	ituWidgetSetVisible(mainLQualityOutPMWNumSprite, true);
	ituWidgetSetVisible(mainLQualityOutPMWBarIcon, true);


	return true;
}

bool MainAirForceTrackBarOnChanged(ITUWidget* widget, char* param)
{
	int x, y;
	//char tmp[32];
	airForceIndex = mainAirForceTrackBar->value;// -1;
	//ituSpriteGoto(mainAirForceSprite, airForceIndex);
	ituSpriteGoto(mainAirForceTextSprite, airForceIndex);
	//ituIconLinkSurface(&mainCAirForceButton->bg.icon, mainCAirForceIconSet[airForceIndex]);
	//ituSpriteGoto(mainAirForceLineSprite, airForceIndex);


	ituWidgetSetVisible(mainAirForceTipShowBackground, false);
	ituWidgetGetGlobalPosition(mainAirForceTrackBar->tip, &x, &y);
	ituWidgetSetPosition(mainAirForceTipShowBackground, x, y);
	//sprintf(tmp, "%d", mainAirForceTrackBar->value);
	//ituTextSetString(mainAirForceTipShowText, tmp);
	ituSpriteGoto(mainAirForceValueSprite, mainAirForceTrackBar->value);
	ituWidgetSetVisible(mainAirForceTipShowBackground, true);

	return true;
}

//bool MainAirForceAutoChkBoxOnPress(ITUWidget* widget, char* param)
//{
//	int x, y;
//	char tmp[32];
//
//	if (ituCheckBoxIsChecked(mainAirForceAutoCheckBox))
//	{
//
//		ituTrackBarSetValue(mainAirForceTrackBar, 3);
//		ituProgressBarSetValue(mainAirForceProgressBar, 3);
//		//ituSpriteGoto(mainAirForceSprite, 6);
//		ituSpriteGoto(mainAirForceTextSprite, 6);
//		//ituIconLinkSurface(&mainCAirForceButton->bg.icon, mainCAirForceIconSet[6]);
//		//ituSpriteGoto(mainAirForceLineSprite, 2);
//		ituWidgetDisable(mainAirForceTrackBar);
//		ituWidgetSetVisible(mainAirForceProgressBar, false);
//		ituIconLinkSurface(mainAirForceTrackBarIcon, mainBarIcon[1]);
//
//
//		//ituWidgetGetGlobalPosition(mainAirForceTrackBar->tip, &x, &y);
//		//ituWidgetSetPosition(mainAirForceTipShowBackground, x, y);
//		//ituTextSetString(mainAirForceTipShowText, "A");
//
//	}
//	else
//	{
//
//		ituTrackBarSetValue(mainAirForceTrackBar, airForceIndex + 1);
//		ituProgressBarSetValue(mainAirForceProgressBar, airForceIndex + 1);
//		//ituSpriteGoto(mainAirForceSprite, airForceIndex);
//		ituSpriteGoto(mainAirForceTextSprite, airForceIndex);
//		//ituIconLinkSurface(&mainCAirForceButton->bg.icon, mainCAirForceIconSet[airForceIndex]);
//		//ituSpriteGoto(mainAirForceLineSprite, airForceIndex);
//		ituWidgetEnable(mainAirForceTrackBar);
//		ituWidgetSetVisible(mainAirForceProgressBar, true);
//		ituIconLinkSurface(mainAirForceTrackBarIcon, mainBarIcon[0]);
//
//		ituWidgetGetGlobalPosition(mainAirForceTrackBar->tip, &x, &y);
//		ituWidgetSetPosition(mainAirForceTipShowBackground, x, y);
//		//sprintf(tmp, "%d", mainAirForceTrackBar->value);
//		//ituTextSetString(mainAirForceTipShowText, tmp);
//		ituSpriteGoto(mainAirForceValueSprite, mainAirForceTrackBar->value);
//	}
//
//	return true;
//}

bool MainCAttachRadBoxOnMouseUp(ITUWidget* widget, char* param)
{
	int attachIndex = atoi(param);

	mainCoverFlow->slideMaxCount = 1;

	return true;
}

bool DayQualitySet(ITUWidget* widget, int index, int i0, int i1, int i2, int i3, int i4, int i5, int i6, int i7,ITUIcon* bgIcon)
//bool DayQualitySet(ITUWidget* widget,int index, int i0, int i1, int i2, int i3, int i4, int i5, int i6, int i7)
{
	

	ITUIcon* pIcon = (ITUIcon*)widget;
	ITUColor emptyColor = { 255, 157, 193, 242 };
	ITUSurface* bgSurf = NULL;

	

	int pQtyValue[8] = { i0, i1, i2, i3, i4, i5, i6, i7 };
	int i = 0;
	int xQtyValue[8] = { 50, 120, 190, 260, 340, 410, 480, 550};
	int x_offset = ituWidgetGetX(mainLQualityInPMDayIcon) - (ituWidgetGetWidth(mainLQualityIcon[0]) / 2);
	int y_offset = ituWidgetGetY(mainLQualityInPMDayIcon) - (ituWidgetGetHeight(mainLQualityIcon[0]) / 2);
	int x_offsetB = ituWidgetGetX(mainLQualityInPMDayIcon) - (ituWidgetGetWidth(mainLQualityBIcon[0]) / 2);
	int y_offsetB = ituWidgetGetY(mainLQualityInPMDayIcon) - (ituWidgetGetHeight(mainLQualityBIcon[0]) / 2);

	ITUColor color = { 200, 255, 255, 255 };

	if (bgIcon)
	{
		ituWidgetSetPosition(bgIcon, ituWidgetGetX(pIcon),ituWidgetGetY(pIcon));
	}

	if (pIcon)
	{
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
				case 1:
					pQtyValue[i] = 165;
					break;
				case 2:
					pQtyValue[i] = 85;
					break;
				case 3:
					pQtyValue[i] = 10;
					break;
				default:
					if (pQtyValue[i] > 2)
						pQtyValue[i] = 48;
					else if (pQtyValue[i] > 1)
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

		if (bgIcon)
		{
			bgSurf = ituCreateSurface(xQtyValue[0], pQtyValue[0], xQtyValue[0] * 2, bgIcon->surf->format, NULL, 0);
			ituColorFill(bgSurf, 0, 0, bgSurf->width, bgSurf->height, &emptyColor);	
			ituBitBlt(bgIcon->surf, 0, 0, bgIcon->widget.rect.width, bgIcon->widget.rect.height, bgSurf, 0, 0);
			ituDestroySurface(bgSurf);

			drawTriangle((ITUWidget*)bgIcon, 0, pQtyValue[0], xQtyValue[0], pQtyValue[0], 0, pQtyValue[0] + 10, true);
			ituDrawLine(pIcon->surf, 0, pQtyValue[0] + 10, xQtyValue[0], pQtyValue[0], &color, 5);
		}

		

		switch (index)
		{
		case 0:
			for (i = 0; i < 7; i++)
			{
				if (bgIcon)
				{
					if (pQtyValue[i + 1] < pQtyValue[i])
						bgSurf = ituCreateSurface(xQtyValue[i + 1] - xQtyValue[i], pQtyValue[i + 1], (xQtyValue[i + 1] - xQtyValue[i]) * 2, bgIcon->surf->format, NULL, 0);
					else
						bgSurf = ituCreateSurface(xQtyValue[i + 1] - xQtyValue[i], pQtyValue[i], (xQtyValue[i + 1] - xQtyValue[i]) * 2, bgIcon->surf->format, NULL, 0);

					ituColorFill(bgSurf, 0, 0, bgSurf->width, bgSurf->height, &emptyColor);
					ituBitBlt(bgIcon->surf, xQtyValue[i], 0, bgIcon->widget.rect.width, bgIcon->widget.rect.height, bgSurf, 0, 0);
					ituDestroySurface(bgSurf);

					if (pQtyValue[i + 1] < pQtyValue[i])
						drawTriangle((ITUWidget*)bgIcon, xQtyValue[i], pQtyValue[i + 1], xQtyValue[i + 1], pQtyValue[i + 1], xQtyValue[i], pQtyValue[i], true);
					else
						drawTriangle((ITUWidget*)bgIcon, xQtyValue[i], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i + 1], false);
				}

				ituDrawLine(pIcon->surf, xQtyValue[i], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i + 1], &color, 5);

				if (pQtyValue[i] >= 254)
				{
					ituIconLinkSurface(mainLQualityInPMIcon[i], mainLQualityIcon[0]);
					InPMDayColor[i] = 0;
				}
				else if (pQtyValue[i] >= 194)
				{
					ituIconLinkSurface(mainLQualityInPMIcon[i], mainLQualityIcon[1]);
					InPMDayColor[i] = 1;
				}
				else
				{
					ituIconLinkSurface(mainLQualityInPMIcon[i], mainLQualityIcon[2]);
					InPMDayColor[i] = 2;
				}
					
				InPMDayH[i] = 302 - pQtyValue[i];
				ituWidgetSetPosition(mainLQualityInPMIcon[i], xQtyValue[i] + x_offset, pQtyValue[i] + y_offset);
				ituWidgetSetPosition(mainLQualityInPMButton[i], xQtyValue[i] + x_offset, pQtyValue[i] + y_offset);

				if (pQtyValue[i + 1] >= 254)
				{
					ituIconLinkSurface(mainLQualityInPMIcon[i + 1], mainLQualityIcon[0]);
					InPMDayColor[i + 1] = 0;
				}
				else if (pQtyValue[i + 1] >= 194)
				{
					ituIconLinkSurface(mainLQualityInPMIcon[i + 1], mainLQualityIcon[1]);
					InPMDayColor[i + 1] = 1;
				}
				else
				{
					ituIconLinkSurface(mainLQualityInPMIcon[i + 1], mainLQualityIcon[2]);
					InPMDayColor[i + 1] = 2;
				}
				InPMDayH[i + 1] = 302 - pQtyValue[i + 1];
				ituWidgetSetPosition(mainLQualityInPMIcon[i + 1], xQtyValue[i + 1] + x_offset, pQtyValue[i + 1] + y_offset);
				ituWidgetSetPosition(mainLQualityInPMButton[i + 1], xQtyValue[i + 1] + x_offset, pQtyValue[i + 1] + y_offset);
			}
			break;
		case 1:
			for (i = 0; i < 8; i++)
			{
				if (pQtyValue[i] >= 165)
				{
					ituIconLinkSurface(mainLQualityTVOCIcon[i], mainLQualityBIcon[0]);
					TVOCDayColor[i] = 0;
				}				
				else if (pQtyValue[i] >= 85)
				{
					ituIconLinkSurface(mainLQualityTVOCIcon[i], mainLQualityBIcon[1]);
					TVOCDayColor[i] = 1;
				}
				else
				{
					ituIconLinkSurface(mainLQualityTVOCIcon[i], mainLQualityBIcon[2]);
					TVOCDayColor[i] = 2;
				}
					
				TVOCDayH[i] = 302 - pQtyValue[i];
				ituWidgetSetPosition(mainLQualityTVOCIcon[i], xQtyValue[i] + x_offsetB, pQtyValue[i] + y_offsetB);
				ituWidgetSetPosition(mainLQualityTVOCButton[i], xQtyValue[i] + x_offsetB, pQtyValue[i] + y_offsetB);
			}

			break;
		case 2:
			for (i = 0; i < 7; i++)
			{
				if (bgIcon)
				{
					if (pQtyValue[i + 1] < pQtyValue[i])
						bgSurf = ituCreateSurface(xQtyValue[i + 1] - xQtyValue[i], pQtyValue[i + 1], (xQtyValue[i + 1] - xQtyValue[i]) * 2, bgIcon->surf->format, NULL, 0);
					else
						bgSurf = ituCreateSurface(xQtyValue[i + 1] - xQtyValue[i], pQtyValue[i], (xQtyValue[i + 1] - xQtyValue[i]) * 2, bgIcon->surf->format, NULL, 0);

					ituColorFill(bgSurf, 0, 0, bgSurf->width, bgSurf->height, &emptyColor);
					ituBitBlt(bgIcon->surf, xQtyValue[i], 0, bgIcon->widget.rect.width, bgIcon->widget.rect.height, bgSurf, 0, 0);
					ituDestroySurface(bgSurf);

					if (pQtyValue[i + 1] < pQtyValue[i])
						drawTriangle((ITUWidget*)bgIcon, xQtyValue[i], pQtyValue[i + 1], xQtyValue[i + 1], pQtyValue[i + 1], xQtyValue[i], pQtyValue[i], true);
					else
						drawTriangle((ITUWidget*)bgIcon, xQtyValue[i], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i + 1], false);
				}

				ituDrawLine(pIcon->surf, xQtyValue[i], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i + 1], &color, 5);

				if (pQtyValue[i] >= 254)
				{
					ituIconLinkSurface(mainLQualityCO2Icon[i], mainLQualityIcon[0]);
					CO2DayColor[i] = 0;
				}
				else if (pQtyValue[i] >= 194)
				{
					ituIconLinkSurface(mainLQualityCO2Icon[i], mainLQualityIcon[1]);
					CO2DayColor[i] = 1;
				}					
				else
				{
					ituIconLinkSurface(mainLQualityCO2Icon[i], mainLQualityIcon[2]);
					CO2DayColor[i] = 2;
				}		
				CO2DayH[i] = 302 - pQtyValue[i];
				ituWidgetSetPosition(mainLQualityCO2Icon[i], xQtyValue[i] + x_offset, pQtyValue[i] + y_offset);
				ituWidgetSetPosition(mainLQualityCO2Button[i], xQtyValue[i] + x_offset, pQtyValue[i] + y_offset);

				if (pQtyValue[i + 1] >= 254)
				{
					ituIconLinkSurface(mainLQualityCO2Icon[i + 1], mainLQualityIcon[0]);
					CO2DayColor[i + 1] = 0;
				}
				else if (pQtyValue[i + 1] >= 194)
				{
					ituIconLinkSurface(mainLQualityCO2Icon[i + 1], mainLQualityIcon[1]);
					CO2DayColor[i + 1] = 1;
				}
				else
				{
					ituIconLinkSurface(mainLQualityCO2Icon[i + 1], mainLQualityIcon[2]);
					CO2DayColor[i + 1] = 2;
				}
				CO2DayH[i + 1] = 302 - pQtyValue[i + 1];
				ituWidgetSetPosition(mainLQualityCO2Icon[i + 1], xQtyValue[i + 1] + x_offset, pQtyValue[i + 1] + y_offset);
				ituWidgetSetPosition(mainLQualityCO2Button[i + 1], xQtyValue[i + 1] + x_offset, pQtyValue[i + 1] + y_offset);
			}
			break;
		case 3:
			for (i = 0; i < 7; i++)
			{
				if (bgIcon)
				{
					if (pQtyValue[i + 1] < pQtyValue[i])
						bgSurf = ituCreateSurface(xQtyValue[i + 1] - xQtyValue[i], pQtyValue[i + 1], (xQtyValue[i + 1] - xQtyValue[i]) * 2, bgIcon->surf->format, NULL, 0);
					else
						bgSurf = ituCreateSurface(xQtyValue[i + 1] - xQtyValue[i], pQtyValue[i], (xQtyValue[i + 1] - xQtyValue[i]) * 2, bgIcon->surf->format, NULL, 0);

					ituColorFill(bgSurf, 0, 0, bgSurf->width, bgSurf->height, &emptyColor);
					ituBitBlt(bgIcon->surf, xQtyValue[i], 0, bgIcon->widget.rect.width, bgIcon->widget.rect.height, bgSurf, 0, 0);
					ituDestroySurface(bgSurf);

					if (pQtyValue[i + 1] < pQtyValue[i])
						drawTriangle((ITUWidget*)bgIcon, xQtyValue[i], pQtyValue[i + 1], xQtyValue[i + 1], pQtyValue[i + 1], xQtyValue[i], pQtyValue[i], true);
					else
						drawTriangle((ITUWidget*)bgIcon, xQtyValue[i], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i + 1], false);
				}

				ituDrawLine(pIcon->surf, xQtyValue[i], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i + 1], &color, 5);

				if (pQtyValue[i] >= 254)
				{
					ituIconLinkSurface(mainLQualityOutPMIcon[i], mainLQualityIcon[0]);
					OutPMDayColor[i] = 0;
				}
				else if (pQtyValue[i] >= 194)
				{
					ituIconLinkSurface(mainLQualityOutPMIcon[i], mainLQualityIcon[1]);
					OutPMDayColor[i] = 1;
				}
				else
				{
					ituIconLinkSurface(mainLQualityOutPMIcon[i], mainLQualityIcon[2]);
					OutPMDayColor[i] = 2;
				}
				OutPMDayH[i] = 302 - pQtyValue[i];
				ituWidgetSetPosition(mainLQualityOutPMIcon[i], xQtyValue[i] + x_offset, pQtyValue[i] + y_offset);
				ituWidgetSetPosition(mainLQualityOutPMButton[i], xQtyValue[i] + x_offset, pQtyValue[i] + y_offset);

				if (pQtyValue[i + 1] >= 254)
				{
					ituIconLinkSurface(mainLQualityOutPMIcon[i + 1], mainLQualityIcon[0]);
					OutPMDayColor[i + 1] = 0;
				}
				else if (pQtyValue[i + 1] >= 194)
				{
					ituIconLinkSurface(mainLQualityOutPMIcon[i + 1], mainLQualityIcon[1]);
					OutPMDayColor[i + 1] = 1;
				}
				else
				{
					ituIconLinkSurface(mainLQualityOutPMIcon[i + 1], mainLQualityIcon[2]);
					OutPMDayColor[i + 1] = 2;
				}
				OutPMDayH[i + 1] = 302 - pQtyValue[i + 1];
				ituWidgetSetPosition(mainLQualityOutPMIcon[i + 1], xQtyValue[i + 1] + x_offset, pQtyValue[i + 1] + y_offset);
				ituWidgetSetPosition(mainLQualityOutPMButton[i + 1], xQtyValue[i + 1] + x_offset, pQtyValue[i + 1] + y_offset);
			}
			break;
		}

		if (bgIcon)
		{
			bgSurf = ituCreateSurface(bgIcon->widget.rect.width - xQtyValue[7], pQtyValue[7], (bgIcon->widget.rect.width - xQtyValue[7]) * 2, bgIcon->surf->format, NULL, 0);
			ituColorFill(bgSurf, 0, 0, bgSurf->width, bgSurf->height, &emptyColor);
			ituBitBlt(bgIcon->surf, xQtyValue[7], 0, bgIcon->widget.rect.width, bgIcon->widget.rect.height, bgSurf, 0, 0);
			ituDestroySurface(bgSurf);

			drawTriangle((ITUWidget*)bgIcon, xQtyValue[7], pQtyValue[7], bgIcon->widget.rect.width, pQtyValue[7], bgIcon->widget.rect.width, pQtyValue[7] + 10, false);
			ituDrawLine(pIcon->surf, xQtyValue[7], pQtyValue[7], pIcon->widget.rect.width, pQtyValue[7] + 10, &color, 5);
		}

		


	}
	return true;
}

bool WeekQualitySet(ITUWidget* widget, int index, int i0, int i1, int i2, int i3, int i4, int i5, int i6,ITUIcon* bgIcon)
//bool WeekQualitySet(ITUWidget* widget, int index, int i0, int i1, int i2, int i3, int i4, int i5, int i6)
{
	ITUIcon* pIcon = (ITUIcon*)widget;
	ITUColor emptyColor = { 255, 157, 193, 242 };
	ITUSurface* bgSurf = NULL;

	int pQtyValue[7] = { i0, i1, i2, i3, i4, i5, i6 };
	int i = 0;
	int xQtyValue[7] = { 80, 160, 240, 320, 400, 480, 560 };
	int x_offset = ituWidgetGetX(mainLQualityInPMDayIcon) - (ituWidgetGetWidth(mainLQualityIcon[0]) / 2);
	int y_offset = ituWidgetGetY(mainLQualityInPMDayIcon) - (ituWidgetGetHeight(mainLQualityIcon[0]) / 2);
	int x_offsetB = ituWidgetGetX(mainLQualityInPMDayIcon) - (ituWidgetGetWidth(mainLQualityBIcon[0]) / 2);
	int y_offsetB = ituWidgetGetY(mainLQualityInPMDayIcon) - (ituWidgetGetHeight(mainLQualityBIcon[0]) / 2);

	ITUColor color = { 200, 255, 255, 255 };

	if (bgIcon)
	{
		ituWidgetSetPosition(bgIcon, ituWidgetGetX(pIcon), ituWidgetGetY(pIcon));
	}

	if (pIcon)
	{
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
				case 1:
					pQtyValue[i] = 165;
					break;
				case 2:
					pQtyValue[i] = 85;
					break;
				case 3:
					pQtyValue[i] = 10;
					break;
				default:
					if (pQtyValue[i] > 2)
						pQtyValue[i] = 48;
					else if (pQtyValue[i] > 1)
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

		if (bgIcon)
		{
			bgSurf = ituCreateSurface(xQtyValue[0], pQtyValue[0], xQtyValue[0] * 2, bgIcon->surf->format, NULL, 0);
			ituColorFill(bgSurf, 0, 0, bgSurf->width, bgSurf->height, &emptyColor);
			ituBitBlt(bgIcon->surf, 0, 0, bgIcon->widget.rect.width, bgIcon->widget.rect.height, bgSurf, 0, 0);
			ituDestroySurface(bgSurf);

			drawTriangle((ITUWidget*)bgIcon, 0, pQtyValue[0], xQtyValue[0], pQtyValue[0], 0, pQtyValue[0] + 10, true);
			ituDrawLine(pIcon->surf, 0, pQtyValue[0] + 10, xQtyValue[0], pQtyValue[0], &color, 5);

		}

		

		switch (index)
		{
		case 0:
			for (i = 0; i < 6; i++)
			{
				if (bgIcon)
				{
					if (pQtyValue[i + 1] < pQtyValue[i])
						bgSurf = ituCreateSurface(xQtyValue[i + 1] - xQtyValue[i], pQtyValue[i + 1], (xQtyValue[i + 1] - xQtyValue[i]) * 2, bgIcon->surf->format, NULL, 0);
					else
						bgSurf = ituCreateSurface(xQtyValue[i + 1] - xQtyValue[i], pQtyValue[i], (xQtyValue[i + 1] - xQtyValue[i]) * 2, bgIcon->surf->format, NULL, 0);

					ituColorFill(bgSurf, 0, 0, bgSurf->width, bgSurf->height, &emptyColor);
					ituBitBlt(bgIcon->surf, xQtyValue[i], 0, bgIcon->widget.rect.width, bgIcon->widget.rect.height, bgSurf, 0, 0);
					ituDestroySurface(bgSurf);

					if (pQtyValue[i + 1] < pQtyValue[i])
						drawTriangle((ITUWidget*)bgIcon, xQtyValue[i], pQtyValue[i + 1], xQtyValue[i + 1], pQtyValue[i + 1], xQtyValue[i], pQtyValue[i], true);
					else
						drawTriangle((ITUWidget*)bgIcon, xQtyValue[i], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i + 1], false);
				}

				ituDrawLine(pIcon->surf, xQtyValue[i], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i + 1], &color, 5);

				if (pQtyValue[i]>250)
				{
					ituIconLinkSurface(mainLQualityInPMWIcon[i], mainLQualityIcon[0]);
					InPMWeekColor[i] = 0;
				}
				else if (pQtyValue[i] > 190)
				{
					ituIconLinkSurface(mainLQualityInPMWIcon[i], mainLQualityIcon[1]);
					InPMWeekColor[i] = 1;
				}
				else
				{
					ituIconLinkSurface(mainLQualityInPMWIcon[i], mainLQualityIcon[2]);
					InPMWeekColor[i] = 2;
				}
				InPMWeekH[i] = 302 - pQtyValue[i];
				ituWidgetSetPosition(mainLQualityInPMWIcon[i], xQtyValue[i] + x_offset, pQtyValue[i] + y_offset);
				ituWidgetSetPosition(mainLQualityInPMWButton[i], xQtyValue[i] + x_offset, pQtyValue[i] + y_offset);

				if (pQtyValue[i + 1]>250)
				{
					ituIconLinkSurface(mainLQualityInPMWIcon[i + 1], mainLQualityIcon[0]);
					InPMWeekColor[i + 1] = 0;
				}
				else if (pQtyValue[i + 1] > 190)
				{
					ituIconLinkSurface(mainLQualityInPMWIcon[i + 1], mainLQualityIcon[1]);
					InPMWeekColor[i + 1] = 1;
				}
				else
				{
					ituIconLinkSurface(mainLQualityInPMWIcon[i + 1], mainLQualityIcon[2]);
					InPMWeekColor[i + 1] = 2;
				}
				InPMWeekH[i + 1] = 302 - pQtyValue[i + 1];
				ituWidgetSetPosition(mainLQualityInPMWIcon[i + 1], xQtyValue[i + 1] + x_offset, pQtyValue[i + 1] + y_offset);
				ituWidgetSetPosition(mainLQualityInPMWButton[i + 1], xQtyValue[i + 1] + x_offset, pQtyValue[i + 1] + y_offset);
			}
			break;
		case 1:
				for (i = 0; i < 7; i++)
				{
					if (pQtyValue[i] >= 165)
					{
						ituIconLinkSurface(mainLQualityTVOCWIcon[i], mainLQualityBIcon[0]);
						TVOCWeekColor[i] = 0;
					}						
					else if (pQtyValue[i] >= 85)
					{
						ituIconLinkSurface(mainLQualityTVOCWIcon[i], mainLQualityBIcon[1]);
						TVOCWeekColor[i] = 1;
					}						
					else
					{
						ituIconLinkSurface(mainLQualityTVOCWIcon[i], mainLQualityBIcon[2]);
						TVOCWeekColor[i] = 2;
					}
						
					TVOCWeekH[i] = 302 - pQtyValue[i];
					ituWidgetSetPosition(mainLQualityTVOCWIcon[i], xQtyValue[i] + x_offsetB, pQtyValue[i] + y_offsetB);
					ituWidgetSetPosition(mainLQualityTVOCWButton[i], xQtyValue[i] + x_offsetB, pQtyValue[i] + y_offsetB);
				}
				break;
		case 2:
			for (i = 0; i < 6; i++)
			{
				if (bgIcon)
				{
					if (pQtyValue[i + 1] < pQtyValue[i])
						bgSurf = ituCreateSurface(xQtyValue[i + 1] - xQtyValue[i], pQtyValue[i + 1], (xQtyValue[i + 1] - xQtyValue[i]) * 2, bgIcon->surf->format, NULL, 0);
					else
						bgSurf = ituCreateSurface(xQtyValue[i + 1] - xQtyValue[i], pQtyValue[i], (xQtyValue[i + 1] - xQtyValue[i]) * 2, bgIcon->surf->format, NULL, 0);

					ituColorFill(bgSurf, 0, 0, bgSurf->width, bgSurf->height, &emptyColor);
					ituBitBlt(bgIcon->surf, xQtyValue[i], 0, bgIcon->widget.rect.width, bgIcon->widget.rect.height, bgSurf, 0, 0);
					ituDestroySurface(bgSurf);

					if (pQtyValue[i + 1] < pQtyValue[i])
						drawTriangle((ITUWidget*)bgIcon, xQtyValue[i], pQtyValue[i + 1], xQtyValue[i + 1], pQtyValue[i + 1], xQtyValue[i], pQtyValue[i], true);
					else
						drawTriangle((ITUWidget*)bgIcon, xQtyValue[i], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i + 1], false);
				}

				ituDrawLine(pIcon->surf, xQtyValue[i], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i + 1], &color, 5);

				if (pQtyValue[i]>250)
				{
					ituIconLinkSurface(mainLQualityCO2WIcon[i], mainLQualityIcon[0]);
					CO2WeekColor[i] = 0;
				}
				else if (pQtyValue[i] > 190)
				{
					ituIconLinkSurface(mainLQualityCO2WIcon[i], mainLQualityIcon[1]);
					CO2WeekColor[i] = 1;
				}
				else
				{
					ituIconLinkSurface(mainLQualityCO2WIcon[i], mainLQualityIcon[2]);
					CO2WeekColor[i] = 2;
				}
				CO2WeekH[i] = 302 - pQtyValue[i];
				ituWidgetSetPosition(mainLQualityCO2WIcon[i], xQtyValue[i] + x_offset, pQtyValue[i] + y_offset);
				ituWidgetSetPosition(mainLQualityCO2WButton[i], xQtyValue[i] + x_offset, pQtyValue[i] + y_offset);

				if (pQtyValue[i + 1]>250)
				{
					ituIconLinkSurface(mainLQualityCO2WIcon[i + 1], mainLQualityIcon[0]);
					CO2WeekColor[i + 1] = 0;
				}
				else if (pQtyValue[i + 1] > 190)
				{
					ituIconLinkSurface(mainLQualityCO2WIcon[i + 1], mainLQualityIcon[1]);
					CO2WeekColor[i + 1] = 1;
				}
				else
				{
					ituIconLinkSurface(mainLQualityCO2WIcon[i + 1], mainLQualityIcon[2]);
					CO2WeekColor[i + 1] = 2;
				}
				CO2WeekH[i + 1] = 302 - pQtyValue[i + 1];
				ituWidgetSetPosition(mainLQualityCO2WIcon[i + 1], xQtyValue[i + 1] + x_offset, pQtyValue[i + 1] + y_offset);
				ituWidgetSetPosition(mainLQualityCO2WButton[i + 1], xQtyValue[i + 1] + x_offset, pQtyValue[i + 1] + y_offset);
			}
			break;
		case 3:
			for (i = 0; i < 6; i++)
			{
				if (bgIcon)
				{
					if (pQtyValue[i + 1] < pQtyValue[i])
						bgSurf = ituCreateSurface(xQtyValue[i + 1] - xQtyValue[i], pQtyValue[i + 1], (xQtyValue[i + 1] - xQtyValue[i]) * 2, bgIcon->surf->format, NULL, 0);
					else
						bgSurf = ituCreateSurface(xQtyValue[i + 1] - xQtyValue[i], pQtyValue[i], (xQtyValue[i + 1] - xQtyValue[i]) * 2, bgIcon->surf->format, NULL, 0);

					ituColorFill(bgSurf, 0, 0, bgSurf->width, bgSurf->height, &emptyColor);
					ituBitBlt(bgIcon->surf, xQtyValue[i], 0, bgIcon->widget.rect.width, bgIcon->widget.rect.height, bgSurf, 0, 0);
					ituDestroySurface(bgSurf);

					if (pQtyValue[i + 1] < pQtyValue[i])
						drawTriangle((ITUWidget*)bgIcon, xQtyValue[i], pQtyValue[i + 1], xQtyValue[i + 1], pQtyValue[i + 1], xQtyValue[i], pQtyValue[i], true);
					else
						drawTriangle((ITUWidget*)bgIcon, xQtyValue[i], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i + 1], false);
				}

				ituDrawLine(pIcon->surf, xQtyValue[i], pQtyValue[i], xQtyValue[i + 1], pQtyValue[i + 1], &color, 5);

				if (pQtyValue[i]>250)
				{
					ituIconLinkSurface(mainLQualityOutPMWIcon[i], mainLQualityIcon[0]);
					OutPMWeekColor[i] = 0;
				}
				else if (pQtyValue[i] > 190)
				{
					ituIconLinkSurface(mainLQualityOutPMWIcon[i], mainLQualityIcon[1]);
					OutPMWeekColor[i] = 1;
				}
				else
				{
					ituIconLinkSurface(mainLQualityOutPMWIcon[i], mainLQualityIcon[2]);
					OutPMWeekColor[i] = 2;
				}
				OutPMWeekH[i] = 302 - pQtyValue[i];
				ituWidgetSetPosition(mainLQualityOutPMWIcon[i], xQtyValue[i] + x_offset, pQtyValue[i] + y_offset);
				ituWidgetSetPosition(mainLQualityOutPMWButton[i], xQtyValue[i] + x_offset, pQtyValue[i] + y_offset);

				if (pQtyValue[i + 1]>250)
				{
					ituIconLinkSurface(mainLQualityOutPMWIcon[i + 1], mainLQualityIcon[0]);
					OutPMWeekColor[i + 1] = 0;
				}
				else if (pQtyValue[i + 1] > 190)
				{
					ituIconLinkSurface(mainLQualityOutPMWIcon[i + 1], mainLQualityIcon[1]);
					OutPMWeekColor[i + 1] = 1;
				}
				else
				{
					ituIconLinkSurface(mainLQualityOutPMWIcon[i + 1], mainLQualityIcon[2]);
					OutPMWeekColor[i + 1] = 2;
				}
				OutPMWeekH[i + 1] = 302 - pQtyValue[i + 1];
				ituWidgetSetPosition(mainLQualityOutPMWIcon[i + 1], xQtyValue[i + 1] + x_offset, pQtyValue[i + 1] + y_offset);
				ituWidgetSetPosition(mainLQualityOutPMWButton[i + 1], xQtyValue[i + 1] + x_offset, pQtyValue[i + 1] + y_offset);
			}
			break;
		}

		if (bgIcon)
		{
			bgSurf = ituCreateSurface(bgIcon->widget.rect.width - xQtyValue[6], pQtyValue[6], (bgIcon->widget.rect.width - xQtyValue[6]) * 2, bgIcon->surf->format, NULL, 0);
			ituColorFill(bgSurf, 0, 0, bgSurf->width, bgSurf->height, &emptyColor);
			ituBitBlt(bgIcon->surf, xQtyValue[6], 0, bgIcon->widget.rect.width, bgIcon->widget.rect.height, bgSurf, 0, 0);
			ituDestroySurface(bgSurf);

			drawTriangle((ITUWidget*)bgIcon, xQtyValue[6], pQtyValue[6], bgIcon->widget.rect.width, pQtyValue[6], bgIcon->widget.rect.width, pQtyValue[6] + 10, false);
			ituDrawLine(pIcon->surf, xQtyValue[6], pQtyValue[6], pIcon->widget.rect.width, pQtyValue[6] + 10, &color, 5);
		}

		
	}
	return true;
}

void drawTriangle(ITUWidget* widget, int x0, int y0, int x1, int y1, int x2, int y2, bool up)
{
	ITUIcon* pIcon = (ITUIcon*)widget;
	ITUColor emptyColor = { 255, 157, 193, 242 };

	int i;
	float theta = atan((double)(x1 - x0) / (y2 - y0));
	float d = sqrt(pow(x1 - x0, 2) + pow(y2 - y0, 2));
	int x;
	

	if (pIcon)
	{
		for (i = 0; i < (y2 - y0 + 1); i++)
		{
			x = d*(y2 - y0 + 1 - i) / (y2 - y0 + 1)*sin(theta);

			if (up)
				ituDrawLine(pIcon->surf, x0, y0 + i, x0 + x, y0 + i, &emptyColor, 5);
			else
				ituDrawLine(pIcon->surf, x1 - x, y0 + i, x1, y0 + i, &emptyColor, 5);

		}

	}
}
void GotoPowerOff(void)
{
	if (!powerOffLayer)
	{
		powerOffLayer = ituSceneFindWidget(&theScene, "powerOffLayer");
		assert(powerOffLayer);
	}
	powerOffTimeIndex = -1;
	powerOffTmHr = 0;
	powerOffTmMin = 0;

	ituLayerGoto(powerOffLayer);

}