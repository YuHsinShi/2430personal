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

ITUSprite* mainBackgroundSprite = 0;
ITUCoverFlow* mainCoverFlow = 0;
ITUCheckBox* mainLStatusCheckBox[STATUS_NUM] = { 0 };
//ITUIcon* mainLStatusDisableIcon[8] = { 0 };
ITUContainer* mainCHumidityContainer = 0;
ITUText* mainCHumidityText = 0;
ITUContainer* mainCTempBigContainer = 0;
ITUContainer* mainSetTempContainer = 0;
ITUText* mainTempValueText = 0;
ITUText* mainTempPointText = 0;
ITUText* mainTempSetText = 0;
ITUContainer* mainCurTempContainer = 0;
ITUText* mainCurTempValueText = 0;
ITUText* mainCurTempPointText = 0;
ITUText* mainTempIndoorText = 0;
ITUContainer* mainCHumidityBigContainer = 0;
ITUContainer* mainSetHumidityContainer = 0;
ITUText* mainHumidityValueText = 0;
ITUText* mainHumiditySetText = 0;
ITUContainer* mainCurHumidityContainer = 0;
ITUText* mainCurHumidityValueText = 0;
ITUText* mainHumidityIndoorText = 0;
ITUIcon* mainCStatusIcon[STATUS_NUM] = { 0 };
ITUContainer* mainCStatusContainer = 0;
ITUButton* mainCModeButton = 0;
ITUButton* mainCAirForceButton = 0;
ITUButton* mainCAirFlowButton = 0;
ITUIcon* mainCModeIcon[MOREMODE_NUM] = { 0 };
ITUIcon* mainCAirForceIconSet[AIRFORCE_NUM] = { 0 };
ITUIcon* mainCAirFlowIcon[2] = { 0 };
ITUBackgroundButton* mainCFilterBackgroundButton = 0;
ITUBackgroundButton* mainCEmergencyBackgroundButton = 0;
ITUBackgroundButton* mainCWarningBackgroundButton = 0;
ITUIcon* mainModeShowIcon[MODESHOW_NUM] = { 0 };
ITUSprite* mainModeShowIconSprite = 0;
ITUIcon* mainModeShowSubIcon[MODESHOW_NUM] = { 0 };
ITUSprite* mainModeShowSubIconSprite = 0;
ITUIcon* mainModeShowLineIcon[MODESHOW_NUM] = { 0 };
ITUIcon* mainModeShowLineIconSet[MODESHOW_NUM] = { 0 };
ITUText* mainModeShowText[MODESHOW_NUM] = { 0 };
ITUSprite* mainModeShowTextSprite = 0;
//ITUCheckBox* mainTopTimingCheckBox = 0;
ITUButton* mainCModeShowButton = 0;
ITUSprite* mainModeTextSprite = 0;
ITUSprite* mainAirForceTextSprite = 0;
ITUSprite* mainAirFlowTextSprite = 0;


ITUTrackBar*	mainTopIndLightTrackBar = 0;
ITUProgressBar*	mainTopIndLightProgressBar = 0;
ITUTrackBar*	mainTopScreenLightTrackBar = 0;
ITUProgressBar*	mainTopScreenLightProgressBar = 0;
ITUCheckBox*	mainTopAutoCheckBox = 0;

ITUTrackBar* mainAirForceTrackBar = 0;
ITUText* mainAirForceValueText = 0;
ITUProgressBar* mainAirForceProgressBar = 0;
ITUCheckBox* mainAirForceAutoCheckBox = 0;
ITUSprite* mainAirForceSprite = 0;
ITUSprite* mainAirForceLineSprite = 0;
ITUIcon* mainTopBarBtnIcon[2] = { 0 };
ITUIcon* mainAirForceTrackBarIcon = 0;
ITUIcon* mainBarIcon[2] = { 0 };
ITUBackground* mainAirForceTipShowBackground = 0;
ITUText* mainAirForceTipShowText = 0;
static ITUVideo* VideoBackGround;

extern const int BgIndex[16] = {0,3,1,2,2,1,1,1,2,2,1,3,3,3,3,3};


static ITUSprite* mainSprite;
ITUDigitalClock* mainDigitalClock = 0;

static int sprite_index = 1;


static int status_show[STATUS_NUM] = { 0 };
//static int status_disable_show[STATUS_NUM] = { 0 ,1,0,0,1,0,0,0};
extern int mode_show[MODESHOW_NUM] = { 1, 0, 0, 0, 0, 0, 0, 0 };
extern bool temp_big_show[16] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 };
extern bool humidity_container_show[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 };
static int statusNum = 0;
static int statusContainerLength = 0;
static int statusPosX = 0; 
static int modeshowNum = 0;
//static int ContainerLength = 0;
static int modeshowPosX = 0;

extern bool warning_btn_show = true;
extern bool emergency_btn_show = true;
extern bool filter_btn_show = true;
extern bool inductionSetting_checked = false;
static int airForceIndex = 0;

extern int modeIndex = 0;
extern bool timingSet = false;
extern bool lightAuto = false;
extern int screenLight = 50;
extern int indLight = 40;

extern bool indicatorLightEnable = true;
extern int wifi_status = 0;

extern bool airFlow3D = true;

//const int humidityValueSet[101] = {
//	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
//	10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 
//	20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
//	30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
//	40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 
//	50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 
//	60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 
//	70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 
//	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 
//	90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100};

extern int humidityValueIndex = 62;
extern int curHumidityValueIndex = 62;
static int preHumidityValueIndex = 0;
bool is_current_humidity_hide = false;

extern const int temperatureValueSet[21] = {
	20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 
	25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30};

extern int statusPos[12][2] = {	{ 0, 140 }, { 0, 210 }, { 0, 70 }, { 0, 280 }, { 0, 0 }, { 0, 350 }, 
										{ 70, 140 }, { 70, 210 }, { 70, 70 }, { 70, 280 }, { 70, 0 }, { 70, 350 } };

extern int temperatureValueIndex = 19;
extern int curTempValueIndex = 19;
static int preTempValueIndex = 0;
bool is_current_tmp_hide = false;
uint32_t hide_start_timer;

static uint32_t gtTick = 0, gtLastTick = 0, gtRefreshTime = 3000;
static bool gtTickFirst = true;

#if defined(CFG_SHT20_ENABLE) || defined(CFG_NCP18_ENABLE)
    static float current_tmp_float = 0;
#endif



void LoadVideoBackGround()
{
	if(!VideoBackGround)
	{
			VideoBackGround = ituSceneFindWidget(&theScene, "VideoBackGround");
			assert(VideoBackGround);
	}
	ituVideoStop(VideoBackGround);

	printf("path=%s modeIndex=%d\n",VideoBackGround->filePath,modeIndex);
		switch(modeIndex)
		{
			case 0:
			snprintf(VideoBackGround->filePath,32,"%s","B:/media/cooler.mkv");

			break;

			case 1:
			snprintf(VideoBackGround->filePath,32,"%s","B:/media/dehumid.mkv");

			break;

			case 2:
			snprintf(VideoBackGround->filePath,32,"%s","B:/media/fans.mkv");
			break;

			case 3:
			snprintf(VideoBackGround->filePath,32,"%s","B:/media/heater.mkv");
			break;

			case 4:
			snprintf(VideoBackGround->filePath,32,"%s","B:/media/humidify.mkv");
			break;

			default:
				
			printf(" mode error ?(%d)",modeIndex);
			snprintf(VideoBackGround->filePath,32,"%s","B:/media/cooler.mkv");

			break;
		}
	printf("path change=%s \n",VideoBackGround->filePath);

	ituVideoPlay(VideoBackGround,1);
	VideoBackGround->repeat=1;



}

void MainReset(void)
{
}
bool MainOnEnter(ITUWidget* widget, char* param)
{
	//int i = 0, wifi_established = -1;
	int wifi_established = -1;
	int i,x,y;
	char tmp[32];


	LoadVideoBackGround();

	
	if (!mainCoverFlow)
	{
		mainCoverFlow = ituSceneFindWidget(&theScene, "mainCoverFlow");
		assert(mainCoverFlow);

		mainBackgroundSprite = ituSceneFindWidget(&theScene, "mainBackgroundSprite");
		assert(mainBackgroundSprite);

		mainCHumidityContainer = ituSceneFindWidget(&theScene, "mainCHumidityContainer");
		assert(mainCHumidityContainer);

		mainCHumidityText = ituSceneFindWidget(&theScene, "mainCHumidityText");
		assert(mainCHumidityText);

		mainCTempBigContainer = ituSceneFindWidget(&theScene, "mainCTempBigContainer");
		assert(mainCTempBigContainer);

		mainSetTempContainer = ituSceneFindWidget(&theScene, "mainSetTempContainer");
		assert(mainSetTempContainer);

		mainTempValueText = ituSceneFindWidget(&theScene, "mainTempValueText");
		assert(mainTempValueText);

		mainTempPointText = ituSceneFindWidget(&theScene, "mainTempPointText");
		assert(mainTempPointText);

		mainTempSetText = ituSceneFindWidget(&theScene, "mainTempSetText");
		assert(mainTempSetText);

		mainCurTempContainer = ituSceneFindWidget(&theScene, "mainCurTempContainer");
		assert(mainCurTempContainer);

		mainCurTempValueText = ituSceneFindWidget(&theScene, "mainCurTempValueText");
		assert(mainCurTempValueText);

		mainCurTempPointText = ituSceneFindWidget(&theScene, "mainCurTempPointText");
		assert(mainCurTempPointText);

		mainTempIndoorText = ituSceneFindWidget(&theScene, "mainTempIndoorText");
		assert(mainTempIndoorText);

		mainCHumidityBigContainer = ituSceneFindWidget(&theScene, "mainCHumidityBigContainer");
		assert(mainCHumidityBigContainer);

		mainSetHumidityContainer = ituSceneFindWidget(&theScene, "mainSetHumidityContainer");
		assert(mainSetHumidityContainer);

		mainHumidityValueText = ituSceneFindWidget(&theScene, "mainHumidityValueText");
		assert(mainHumidityValueText);

		mainHumiditySetText = ituSceneFindWidget(&theScene, "mainHumiditySetText");
		assert(mainHumiditySetText);

		mainCurHumidityContainer = ituSceneFindWidget(&theScene, "mainCurHumidityContainer");
		assert(mainCurHumidityContainer);

		mainCurHumidityValueText = ituSceneFindWidget(&theScene, "mainCurHumidityValueText");
		assert(mainCurHumidityValueText);

		mainHumidityIndoorText = ituSceneFindWidget(&theScene, "mainHumidityIndoorText");
		assert(mainHumidityIndoorText);

		mainCStatusContainer = ituSceneFindWidget(&theScene, "mainCStatusContainer");
		assert(mainCStatusContainer);

		mainCFilterBackgroundButton = ituSceneFindWidget(&theScene, "mainCFilterBackgroundButton");
		assert(mainCFilterBackgroundButton);

		mainCEmergencyBackgroundButton = ituSceneFindWidget(&theScene, "mainCEmergencyBackgroundButton");
		assert(mainCEmergencyBackgroundButton);

		mainCWarningBackgroundButton = ituSceneFindWidget(&theScene, "mainCWarningBackgroundButton");
		assert(mainCWarningBackgroundButton);

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

		mainSprite = ituSceneFindWidget(&theScene, "mainSprite");
		assert(mainSprite);

		mainDigitalClock = ituSceneFindWidget(&theScene, "mainDigitalClock");
		assert(mainDigitalClock);

		mainCModeButton = ituSceneFindWidget(&theScene, "mainCModeButton");
		assert(mainCModeButton);

		mainCAirForceButton = ituSceneFindWidget(&theScene, "mainCAirForceButton");
		assert(mainCAirForceButton);

		mainCAirFlowButton = ituSceneFindWidget(&theScene, "mainCAirFlowButton");
		assert(mainCAirFlowButton);

		//mainTopTimingCheckBox = ituSceneFindWidget(&theScene, "mainTopTimingCheckBox");
		//assert(mainTopTimingCheckBox);

		mainCModeShowButton = ituSceneFindWidget(&theScene, "mainCModeShowButton");
		assert(mainCModeShowButton);

		mainModeShowIconSprite = ituSceneFindWidget(&theScene, "mainModeShowIconSprite");
		assert(mainModeShowIconSprite);

		mainModeShowSubIconSprite = ituSceneFindWidget(&theScene, "mainModeShowSubIconSprite");
		assert(mainModeShowSubIconSprite);

		mainModeShowTextSprite = ituSceneFindWidget(&theScene, "mainModeShowTextSprite");
		assert(mainModeShowTextSprite);

		mainModeTextSprite = ituSceneFindWidget(&theScene, "mainModeTextSprite");
		assert(mainModeTextSprite);

		mainAirForceTextSprite = ituSceneFindWidget(&theScene, "mainAirForceTextSprite");
		assert(mainAirForceTextSprite);

		mainAirFlowTextSprite = ituSceneFindWidget(&theScene, "mainAirFlowTextSprite");
		assert(mainAirFlowTextSprite);



		for (i = 0; i < STATUS_NUM; i++)
		{
			sprintf(tmp, "mainLStatusCheckBox%d", i);
			mainLStatusCheckBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainLStatusCheckBox[i]);

			sprintf(tmp, "mainCStatusIcon%d", i);
			mainCStatusIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainCStatusIcon[i]);

			

			status_show[i] = 0;

		}



		for (i = 0; i < AIRFORCE_NUM; i++)
		{
			sprintf(tmp, "mainCAirForceIcon%d", i);
			mainCAirForceIconSet[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainCAirForceIconSet[i]);
		}


		for (i = 0; i < (MOREMODE_NUM ); i++)
		{
			sprintf(tmp, "mainCModeIcon%d", i);
			mainCModeIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainCModeIcon[i]);
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

			sprintf(tmp, "mainCAirFlowIcon%d", i);
			mainCAirFlowIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainCAirFlowIcon[i]);
		}





	}

	ituSpriteGoto(mainSprite, sprite_index);

	ituWidgetSetVisible(mainCHumidityContainer, humidity_container_show[modeIndex]);
	ituWidgetSetVisible(mainCWarningBackgroundButton, warning_btn_show);
	ituWidgetSetVisible(mainCEmergencyBackgroundButton, emergency_btn_show);
	ituWidgetSetVisible(mainCFilterBackgroundButton, filter_btn_show);

	//statusPosX = 0;
	statusNum = 0;
	for (i = 0; i < STATUS_NUM; i++)
	{
		if (status_show[i])
		{
			//ituWidgetSetPosition(mainCStatusIcon[i], statusPosX, 0);
			ituWidgetSetPosition(mainCStatusIcon[i], statusPos[statusNum][0], statusPos[statusNum][1]);
			statusNum++;
			//statusPosX = statusPosX + 60;
		}
		else
		{
			ituWidgetSetPosition(mainCStatusIcon[i], -70, 100);
		}
			
	}

	//for (i = 0; i < 8; i++)
	//{
	//	if (status_disable_show[i])
	//	{
	//		ituWidgetSetVisible(mainLStatusDisableIcon[i], true);
	//		ituWidgetSetVisible(mainLStatusCheckBox[i], false);

	//	}
	//	else
	//	{
	//		ituWidgetSetVisible(mainLStatusDisableIcon[i], false);
	//		ituWidgetSetVisible(mainLStatusCheckBox[i], true);
	//	}

	//}
	//statusContainerLength = statusNum * 60;
	//ituWidgetSetDimension(mainCStatusContainer, statusContainerLength, 54);

	ituCheckBoxSetChecked(mainLStatusCheckBox[5], inductionSetting_checked);
	if (inductionSetting_checked)
		status_show[5] = 1;
	else
		status_show[5] = 0;


	//ituCheckBoxSetChecked(mainTopTimingCheckBox, timingSet);
	mode_show[5] = timingSet;

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
	sprintf(tmp, "%d", curHumidityValueIndex);
	ituTextSetString(mainCHumidityText, tmp);

	ituSpriteGoto(mainAirForceTextSprite, airForceIndex);
	//ituIconLinkSurface(&mainCAirForceButton->bg.icon, mainCAirForceIconSet[airForceIndex]);

	ituSpriteGoto(mainModeTextSprite, modeIndex);
	ituIconLinkSurface(&mainCModeButton->bg.icon, mainCModeIcon[modeIndex]);
	ituSpriteGoto(mainBackgroundSprite, BgIndex[modeIndex]);


	ituWidgetSetVisible(mainAirForceTipShowBackground, false);
	ituWidgetGetGlobalPosition(mainAirForceTrackBar->tip, &x, &y);
	ituWidgetSetPosition(mainAirForceTipShowBackground, x, y);
	sprintf(tmp, "%d", mainAirForceTrackBar->value);
	ituTextSetString(mainAirForceTipShowText, tmp);
	ituWidgetSetVisible(mainAirForceTipShowBackground, true);

	ituSpriteGoto(mainAirFlowTextSprite, !airFlow3D);
	ituIconLinkSurface(&mainCAirFlowButton->bg.icon, mainCAirFlowIcon[!airFlow3D]);
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
			curTempValueIndex =(int) ((current_tmp_float + 0.5)-20)/0.5;
		}
		else
		{
			curTempValueIndex =(int) ((-current_tmp_float + 0.5)-20)/0.5;
			curTempValueIndex = -curTempValueIndex;
		}

		preTempValueIndex = curTempValueIndex;
#else
#ifdef CFG_NCP18_ENABLE
		current_tmp_float = NCP18_Detect();
		if (current_tmp_float > 0)
		{
			curTempValueIndex =(int) ((current_tmp_float + 0.5)-20)/0.5;
		}
		else
		{
			curTempValueIndex =(int) ((-current_tmp_float + 0.5)-20)/0.5;
			curTempValueIndex = -curTempValueIndex;
		}

		preTempValueIndex = curTempValueIndex;
#else
		curTempValueIndex = 19;//get from sensor
		preTempValueIndex = curTempValueIndex;

		curHumidityValueIndex = 62;//get from sensor
		preHumidityValueIndex = curTempValueIndex;
#endif
#endif

		gtLastTick = gtTick;
	}

	if (curTempValueIndex > 20)
		curTempValueIndex = 20;
	else if (curTempValueIndex < 0)
		curTempValueIndex = 0;




	if (temp_big_show[modeIndex])
	{
		ituWidgetSetVisible(mainCTempBigContainer, true);
		ituWidgetSetVisible(mainCHumidityBigContainer, false);

		sprintf(tmp, "%d", temperatureValueSet[curTempValueIndex]);
		ituTextSetString(mainCurTempValueText, tmp);

		if (curTempValueIndex % 2)
			ituWidgetSetVisible(mainCurTempPointText, true);
		else
			ituWidgetSetVisible(mainCurTempPointText, false);
	}
	else
	{
		ituWidgetSetVisible(mainCTempBigContainer, false);
		ituWidgetSetVisible(mainCHumidityBigContainer, true);

		sprintf(tmp, "%d", curHumidityValueIndex);
		ituTextSetString(mainCurHumidityValueText, tmp);
	}

	if (theConfig.wifi_on_off == 1)
	{
#ifdef CFG_NET_WIFI_SDIO_NGPL
	    wifi_established = ioctl(ITP_DEVICE_WIFI_NGPL, ITP_IOCTL_IS_AVAIL, NULL);
#endif

	    if (wifi_established)
//	        ituSpriteGoto(topWiFiSprite, 2); //0: wifi is off, 1: wifi is on but no connection, 2: connection is established
			wifi_status = 0;
		else
//            ituSpriteGoto(topWiFiSprite, 1);
			wifi_status = 1;

	}
	else
	{
//		ituSpriteGoto(topWiFiSprite, 0); //0: wifi is off, 1: wifi is on but no connection, 2: connection is established
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
			curTempValueIndex = (int)((current_tmp_float + 0.5) - 20) / 0.5;
		}
		else
		{
			curTempValueIndex = (int)((-current_tmp_float + 0.5) - 20) / 0.5;
			curTempValueIndex = -curTempValueIndex;
		}
#else
#ifdef CFG_NCP18_ENABLE
		current_tmp_float = NCP18_Detect();
		if (current_tmp_float > 0)
		{
			curTempValueIndex = (int)((current_tmp_float + 0.5) - 20) / 0.5;
		}
		else
		{
			curTempValueIndex =(int) ((-current_tmp_float + 0.5)-20)/0.5;
			curTempValueIndex = -curTempValueIndex;
		}
#else
		curTempValueIndex = 19;//get from sensor
		curHumidityValueIndex = 62;//get from sensor
#endif
#endif

		gtLastTick = gtTick;
	}

	if (curTempValueIndex > 20)
		curTempValueIndex = 20;
	else if (curTempValueIndex < 0)
		curTempValueIndex = 0;

	if (preTempValueIndex != curTempValueIndex)
	{
		sprintf(tmp, "%d", temperatureValueSet[curTempValueIndex]);
		ituTextSetString(mainCurTempValueText, tmp);

		if (curTempValueIndex % 2)
			ituWidgetSetVisible(mainCurTempPointText, true);
		else
			ituWidgetSetVisible(mainCurTempPointText, false);

		preTempValueIndex = curTempValueIndex;
		ret = true;
	}

	if (preHumidityValueIndex != curHumidityValueIndex)
	{
		sprintf(tmp, "%d", curHumidityValueIndex);
		ituTextSetString(mainCurHumidityValueText, tmp);


		preHumidityValueIndex = curHumidityValueIndex;
		ret = true;
	}


	if (is_current_tmp_hide == true)
	{
		tick = SDL_GetTicks();
		if (tick - hide_start_timer >= 1000)
		{
			is_current_tmp_hide = false;
			//set temperatue to set_tmp 
			ituWidgetSetVisible(mainSetTempContainer, false);


			sprintf(tmp, "%d", temperatureValueSet[curTempValueIndex]);
			ituTextSetString(mainCurTempValueText, tmp);

			if (curTempValueIndex % 2)
				ituWidgetSetVisible(mainCurTempPointText, true);
			else
				ituWidgetSetVisible(mainCurTempPointText, false);

			ituWidgetSetVisible(mainCurTempContainer, true);
			ret = true;
		}
	}

	if (is_current_humidity_hide == true)
	{
		tick = SDL_GetTicks();
		if (tick - hide_start_timer >= 1000)
		{
			is_current_humidity_hide = false;
			//set temperatue to set_tmp 
			ituWidgetSetVisible(mainSetHumidityContainer, false);


			sprintf(tmp, "%d", curHumidityValueIndex);
			ituTextSetString(mainCurHumidityValueText, tmp);

			ituWidgetSetVisible(mainCurHumidityContainer, true);
			ret = true;
		}
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
		//statusPosX = 0;
		statusNum = 0;
		for (i = 0; i < STATUS_NUM; i++)
		{
			if (status_show[i])
			{
				//ituWidgetSetPosition(mainCStatusIcon[i], statusPosX,0);
				ituWidgetSetPosition(mainCStatusIcon[i], statusPos[statusNum][0], statusPos[statusNum][1]);
				statusNum++;
				//statusPosX = statusPosX + 60;
			}
			else
			{
				ituWidgetSetPosition(mainCStatusIcon[i], -70, 100);
			}

		}
		//statusContainerLength = statusNum * 60;
		//ituWidgetSetDimension(mainCStatusContainer, statusContainerLength, 54);
	}
	return true;
}

//bool MainRModeRadBoxOnPress(ITUWidget* widget, char* param)
//{
//	modeIndex = atoi(param);
//	ituIconLinkSurface(mainCModeIcon, mainCModeIconSet[modeIndex]);
//
//	ituIconLinkSurface(mainRMoreModeIcon, mainRMoreModeIconSet[13]);
//	ituSpriteGoto(mainRMoreModeTextSprite, 12);
//	ituIconLinkSurface(&mainRMoreModeBackgroundButton->bg.icon, mainRMoreModeBgBtnBg[0]);
//
//	ituSpriteGoto(mainBackgroundSprite, BgIndex[modeIndex]);
//
//	return true;
//}
//
//bool MainRMoreModeBackgroundBtnOnPress(ITUWidget* widget, char* param)
//{
//	if (modeIndex < 4)
//	{
//		ituRadioBoxSetChecked(mainRModeRadioBox[modeIndex], false);
//	}
//	ituIconLinkSurface(mainRMoreModeIcon, mainRMoreModeIconSet[12]);
//	ituIconLinkSurface(&mainRMoreModeBackgroundButton->bg.icon, mainRMoreModeBgBtnBg[1]);
//
//	
//	return true;
//}

bool MainHumidityBtnOnPress(ITUWidget* widget, char* param)
{
	bool add = atoi(param);
	char tmp[32];

	ituWidgetSetVisible(mainCurHumidityContainer, false);

	is_current_humidity_hide = true;

	if (add)
	{
		humidityValueIndex++;
		if (humidityValueIndex > 100)
			humidityValueIndex = 100;
	}
	else
	{
		humidityValueIndex--;
		if (humidityValueIndex < 0)
			humidityValueIndex = 0;
	}
	//sprintf(tmp, "%d", humidityValueSet[humidityValueIndex]);
	sprintf(tmp, "%d", humidityValueIndex);
	ituTextSetString(mainHumidityValueText, tmp);

	ituWidgetSetVisible(mainSetHumidityContainer, true);

	hide_start_timer = SDL_GetTicks();

	return true;
}

bool MainTempBtnOnPress(ITUWidget* widget, char* param)
{
	bool add = atoi(param);
	char tmp[32];

	ituWidgetSetVisible(mainCurTempContainer, false);

	//temperatureValueIndex = curTempValueIndex;
	is_current_tmp_hide = true;

	if (add)
	{
		temperatureValueIndex++;
		if (temperatureValueIndex > 20)
			temperatureValueIndex = 20;
	}
	else
	{
		temperatureValueIndex--;
		if (temperatureValueIndex < 0)
			temperatureValueIndex = 0;
	}
	sprintf(tmp, "%d", temperatureValueSet[temperatureValueIndex]);
	ituTextSetString(mainTempValueText, tmp);

	if (temperatureValueIndex % 2)
		ituWidgetSetVisible(mainTempPointText, true);
	else
		ituWidgetSetVisible(mainTempPointText, false);

	ituWidgetSetVisible(mainSetTempContainer, true);

	hide_start_timer = SDL_GetTicks();

	return true;
}


bool MainLStatusChkBoxOnMouseUp(ITUWidget* widget, char* param)
{
	int status_index = atoi(param);

	if (ituCheckBoxIsChecked(mainLStatusCheckBox[status_index]))
		status_show[status_index] = 1;
	else
		status_show[status_index] = 0;

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
	ituWidgetSetPosition(mainAirForceTipShowBackground, x , y );
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

//bool MainRAICheckBoxOnPress(ITUWidget* widget, char* param)
//{
//	if (ituCheckBoxIsChecked(mainRAICheckBox))
//	{
//		mode_show[6] = 1;
//		ituWidgetSetPosition(mainModeShowIcon[5], modeshowPosX, 0);
//		modeshowNum++;
//		modeshowPosX = modeshowPosX + 46;
//	}
//	else
//	{
//		mode_show[6] = 0;
//		ituWidgetSetPosition(mainModeShowIcon[5], 0, 100);
//		modeshowNum--;
//		modeshowPosX = modeshowPosX - 46;
//	}
//
//
//	return true;
//}
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
