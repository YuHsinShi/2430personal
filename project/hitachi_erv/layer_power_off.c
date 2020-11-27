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

static ITULayer* mainLayer;
static ITUCoverFlow* mainCoverFlow;
//static ITUCoverFlow* powerOffPwrOnCoverFlow = 0;
//static ITUContainer* powerOffPwrOnContainer[3] = { 0 };
//static ITUButton* powerOffPwrOnButton = 0;
//static ITUText* powerOffTempText = 0;
static ITUText* powerOffPM25Text = 0;
static ITUText* powerOffTVOCText = 0;
static ITUText* powerOffCO2Text = 0;
static ITUSprite* powerOffTimingSprite = 0;
//static ITUWheel* powerOffTimingHrWheel = 0;
//static ITUWheel* powerOffTimingMinWheel = 0;
static ITUText* powerOffTimingText = 0;
//static ITUSprite* powerOffTimingSetSprite = 0;
//static ITUWheel* powerOffTimingWheel = 0;
static ITUTrackBar* powerOffTopIndLightTrackBar = 0;
static ITUProgressBar* powerOffTopIndLightProgressBar = 0;
static ITUTrackBar* powerOffTopScreenLightTrackBar = 0;
static ITUProgressBar* powerOffTopScreenLightProgressBar = 0;
static ITUIcon* powerOffTopBarBtnIcon[2] = { 0 };
static ITUCheckBox* powerOffTopAutoCheckBox = 0;

static int orgContainerPosX = 0;
static int curContainerPosX = 0;

static int TimeHr = 0;
static int TimeMin = 0;
static int TimeSec = 0;
extern int powerOnTimeIndex = -1;
double lastTime;
extern int powerOnTmHr = 0;
extern int powerOnTmMin = 0;

void StopPowerOff(void);


//static int preTempIndex = 0;

//static uint32_t gtTick = 0, gtLastTick = 0, gtRefreshTime = 3000;
//static bool gtTickFirst = true;

#if defined(CFG_SHT20_ENABLE) || defined(CFG_NCP18_ENABLE)
static float current_tmp_float = 0;
#endif

bool PowerOffOnEnter(ITUWidget* widget, char* param)
{
	int i;
	//int x, y;
	char tmp[32];
	if (!powerOffPM25Text)
	{
		//powerOffPwrOnCoverFlow = ituSceneFindWidget(&theScene, "powerOffPwrOnCoverFlow");
		//assert(powerOffPwrOnCoverFlow);

		//powerOffPwrOnButton = ituSceneFindWidget(&theScene, "powerOffPwrOnButton");
		//assert(powerOffPwrOnButton);

		//powerOffTempText = ituSceneFindWidget(&theScene, "powerOffTempText");
		//assert(powerOffTempText);

		powerOffPM25Text = ituSceneFindWidget(&theScene, "powerOffPM25Text");
		assert(powerOffPM25Text);

		powerOffTVOCText = ituSceneFindWidget(&theScene, "powerOffTVOCText");
		assert(powerOffTVOCText);

		powerOffCO2Text = ituSceneFindWidget(&theScene, "powerOffCO2Text");
		assert(powerOffCO2Text);

		

		//powerOffTimingHrWheel = ituSceneFindWidget(&theScene, "powerOffTimingHrWheel");
		//assert(powerOffTimingHrWheel);

		//powerOffTimingMinWheel = ituSceneFindWidget(&theScene, "powerOffTimingMinWheel");
		//assert(powerOffTimingMinWheel);

		powerOffTimingText = ituSceneFindWidget(&theScene, "powerOffTimingText");
		assert(powerOffTimingText);

		powerOffTimingSprite = ituSceneFindWidget(&theScene, "powerOffTimingSprite");
		assert(powerOffTimingSprite);

		//powerOffTimingWheel = ituSceneFindWidget(&theScene, "powerOffTimingWheel");
		//assert(powerOffTimingWheel);

		//powerOffTimingSetSprite = ituSceneFindWidget(&theScene, "powerOffTimingSetSprite");
		//assert(powerOffTimingSetSprite);

		powerOffTopIndLightTrackBar = ituSceneFindWidget(&theScene, "powerOffTopIndLightTrackBar");
		assert(powerOffTopIndLightTrackBar);

		powerOffTopIndLightProgressBar = ituSceneFindWidget(&theScene, "powerOffTopIndLightProgressBar");
		assert(powerOffTopIndLightProgressBar);

		powerOffTopScreenLightTrackBar = ituSceneFindWidget(&theScene, "powerOffTopScreenLightTrackBar");
		assert(powerOffTopScreenLightTrackBar);

		powerOffTopScreenLightProgressBar = ituSceneFindWidget(&theScene, "powerOffTopScreenLightProgressBar");
		assert(powerOffTopScreenLightProgressBar);

		powerOffTopAutoCheckBox = ituSceneFindWidget(&theScene, "powerOffTopAutoCheckBox");
		assert(powerOffTopAutoCheckBox);

		//for (i = 0; i < 3; i++)
		//{
		//	sprintf(tmp, "powerOffPwrOnContainer%d", i);
		//	powerOffPwrOnContainer[i] = ituSceneFindWidget(&theScene, tmp);
		//	assert(powerOffPwrOnContainer[i]);
		//}

		for (i = 0; i < 2; i++)
		{
			sprintf(tmp, "powerOffTopBarBtnIcon%d", i);
			powerOffTopBarBtnIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffTopBarBtnIcon[i]);

		}

	}

	//orgContainerPosX = ituWidgetGetX(powerOffPwrOnContainer[1]);
	//ituCoverFlowGoto(powerOffPwrOnCoverFlow, 1);



//	if (gtTickFirst)
//	{
//		gtTick = gtRefreshTime;
//		gtTickFirst = false;
//	}
//	else
//	{
//		gtTick = SDL_GetTicks();
//	}
//
//	if (gtTick - gtLastTick >= gtRefreshTime)
//	{
//#ifdef CFG_SHT20_ENABLE
//		current_tmp_float = SHT20_Detect();
//		if (current_tmp_float > 0)
//		{
//			temperatureInIndex = (int)(current_tmp_float + 0.5);
//		}
//		else
//		{
//			temperatureInIndex = (int)(-current_tmp_float + 0.5);
//			temperatureInIndex = -temperatureInIndex;
//		}
//
//		preTempIndex = temperatureInIndex;
//#else
//#ifdef CFG_NCP18_ENABLE
//		current_tmp_float = NCP18_Detect();
//		if (current_tmp_float > 0)
//		{
//			temperatureInIndex = (int)(current_tmp_float + 0.5);
//		}
//		else
//		{
//			temperatureInIndex = (int)(-current_tmp_float + 0.5);
//			temperatureInIndex = -temperatureInIndex;
//		}
//
//		preTempIndex = temperatureInIndex;
//#else
//		temperatureInIndex = 29;//get from sensor
//		preTempIndex = temperatureInIndex;
//#endif
//#endif
//
//		gtLastTick = gtTick;
//	}


	//sprintf(tmp, "%d", temperatureInIndex);
	//ituTextSetString(powerOffTempText, tmp);

	powerOffTimeIndex = -1;
	powerOffTmHr = 0;
	powerOffTmMin = 0;

	if (powerOnTimeIndex > -1)
	{
		ituSpriteGoto(powerOffTimingSprite, 1);

	}
	else
		ituSpriteGoto(powerOffTimingSprite, 0);


	ituProgressBarSetValue(powerOffTopScreenLightProgressBar, screenLight);
	ituTrackBarSetValue(powerOffTopScreenLightTrackBar, screenLight);
	ituProgressBarSetValue(powerOffTopIndLightProgressBar, indLight);
	ituTrackBarSetValue(powerOffTopIndLightTrackBar, indLight);

	if (!indicatorLightEnable)
	{
		ituIconLinkSurface(&powerOffTopIndLightTrackBar->tracker->bg.icon, powerOffTopBarBtnIcon[0]);

		ituWidgetDisable(powerOffTopIndLightTrackBar);
		ituWidgetSetVisible(powerOffTopIndLightProgressBar, false);
	}

	if (lightAuto)
	{
		ituCheckBoxSetChecked(powerOffTopAutoCheckBox, true);
		ituIconLinkSurface(&powerOffTopScreenLightTrackBar->tracker->bg.icon, powerOffTopBarBtnIcon[0]);


		ituWidgetDisable(powerOffTopScreenLightTrackBar);
		ituWidgetSetVisible(powerOffTopScreenLightProgressBar, false);


		if (indicatorLightEnable)
		{
			ituIconLinkSurface(&powerOffTopIndLightTrackBar->tracker->bg.icon, powerOffTopBarBtnIcon[0]);
			ituWidgetDisable(powerOffTopIndLightTrackBar);
			ituWidgetSetVisible(powerOffTopIndLightProgressBar, false);
		}
	}
	else
	{
		ituCheckBoxSetChecked(powerOffTopAutoCheckBox, false);
		ituIconLinkSurface(&powerOffTopScreenLightTrackBar->tracker->bg.icon, powerOffTopBarBtnIcon[1]);
		ituWidgetEnable(powerOffTopScreenLightTrackBar);
		ituWidgetSetVisible(powerOffTopScreenLightProgressBar, true);

		if (indicatorLightEnable)
		{
			ituIconLinkSurface(&powerOffTopIndLightTrackBar->tracker->bg.icon, powerOffTopBarBtnIcon[1]);
			ituWidgetEnable(powerOffTopIndLightTrackBar);
			ituWidgetSetVisible(powerOffTopIndLightProgressBar, true);
		}
	}

	

    return true;
}

bool PowerOffOnTimer(ITUWidget* widget, char* param)
{
	bool ret = false;
	int dist = 0;
	char tmp[32];

	struct timeval tv;
	struct tm *tm;
	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	int curSec = 59 - tm->tm_sec;

	//curContainerPosX = ituWidgetGetX(powerOffPwrOnContainer[1]);


	//dist = curContainerPosX - orgContainerPosX;

	//if (abs(dist) > 35)
	//{
	//	StopPowerOff();
	//	ret = true;
	//}

//	if (gtTickFirst)
//	{
//		gtTick = gtRefreshTime;
//		gtTickFirst = false;
//	}
//	else
//	{
//		gtTick = SDL_GetTicks();
//	}
//
//	if (gtTick - gtLastTick >= gtRefreshTime)
//	{
//#ifdef CFG_SHT20_ENABLE
//		current_tmp_float = SHT20_Detect();
//		if (current_tmp_float > 0)
//		{
//			temperatureInIndex = (int)(current_tmp_float + 0.5);
//		}
//		else
//		{
//			temperatureInIndex = (int)(-current_tmp_float + 0.5);
//			temperatureInIndex = -temperatureInIndex;
//		}
//#else
//#ifdef CFG_NCP18_ENABLE
//		current_tmp_float = NCP18_Detect();
//		if (current_tmp_float > 0)
//		{
//			temperatureInIndex = (int)(current_tmp_float + 0.5);
//		}
//		else
//		{
//			temperatureInIndex = (int)(-current_tmp_float + 0.5);
//			temperatureInIndex = -temperatureInIndex;
//		}
//#else
//		temperatureInIndex = 29;//get from sensor
//
//#endif
//#endif
//
//		gtLastTick = gtTick;
//	}


	//if (preTempIndex != temperatureInIndex)
	//{
	//	sprintf(tmp, "%d", temperatureInIndex);
	//	ituTextSetString(powerOffTempText, tmp);


	//	preTempIndex = temperatureInIndex;
	//	ret = true;
	//}

	if (powerOffTimingSprite->frame == 1)
	//if (powerOnTimeIndex != -1)
	{
		if (powerOnTmHr == tm->tm_hour && powerOnTmMin == tm->tm_min)
		{
			ituSpriteGoto(powerOffTimingSprite, 0);
			StopPowerOff();

		}

		if (curSec != TimeSec)
		{
			TimeSec = curSec;			

			if (powerOnTmMin >= tm->tm_min)
			{
				lastTime = (double)(powerOnTmMin - tm->tm_min) / 60;

				if (powerOnTmHr >= tm->tm_hour)
				{
					lastTime = lastTime + (powerOnTmHr - tm->tm_hour);
				}
				else
				{
					lastTime = lastTime + (powerOnTmHr + 24 - tm->tm_hour);
				}
			}
			else
			{
				lastTime = (double)(powerOnTmMin + 60 - tm->tm_min) / 60;

				if ((powerOnTmHr - 1) >= tm->tm_hour)
				{
					lastTime = lastTime + (powerOnTmHr - 1 - tm->tm_hour);
				}
				else
				{
					lastTime = lastTime + (powerOnTmHr + 23 - tm->tm_hour);
				}

			}


			sprintf(tmp, "%2.1f", lastTime);
			ituTextSetString(powerOffTimingText, tmp);

			ret = true;
		}
	}




	return ret;
}
//bool PowerOffTimingWheelOnChanged(ITUWidget* widget, char* param)
//{
//	powerOnTimeIndex = powerOffTimingWheel->focusIndex;
//
//	if (powerOnTimeIndex == 0)
//	{
//		ituSpriteGoto(powerOffTimingSetSprite, 0);
//	}
//	else
//	{
//		ituSpriteGoto(powerOffTimingSetSprite, 1);
//	}
//
//	return true;
//}
//bool PowerOffTimingSaveBtnOnPress(ITUWidget* widget, char* param)
//{
//	char tmp[32];
//	struct timeval tv;
//	struct tm *tm;
//	
//
//	gettimeofday(&tv, NULL);
//	tm = localtime(&tv.tv_sec);
//
//	if (powerOnTimeIndex > 1)
//		TimeHr = powerOnTimeIndex / 2;
//	else
//		TimeHr = powerOnTimeIndex % 2;
//
//	if (powerOnTimeIndex % 2 == 0)
//		TimeMin = 30;
//	else
//		TimeMin = 0;
//
//	powerOnTmHr = tm->tm_hour + TimeHr;
//	powerOnTmMin = tm->tm_min + TimeMin;
//
//	if (powerOnTmMin > 60)
//	{
//		powerOnTmMin = powerOnTmMin - 60;
//		powerOnTmHr++;
//	}
//	if (powerOnTmHr > 24)
//	{
//		powerOnTmHr = powerOnTmHr - 24;
//	}
//
//	lastTime = powerOnTmHr + (double)powerOnTmMin / 60;
//
//	sprintf(tmp, "%2.1f", lastTime);
//	ituTextSetString(powerOffTimingText, tmp);
//	ituSpriteGoto(powerOffTimingSprite, 1);
//
//    return true;
//}
bool PowerOffTopIndLightTrackBarOnChanged(ITUWidget* widget, char* param)
{
	indLight = powerOffTopIndLightTrackBar->value;
	return true;
}

bool PowerOffTopScreenLightTrackBarOnChanged(ITUWidget* widget, char* param)
{
	screenLight = powerOffTopScreenLightTrackBar->value;
	printf("powerOffTopScreenLightTrackBar %d \n", powerOffTopScreenLightTrackBar->value);
	ScreenSetBrightness(powerOffTopScreenLightTrackBar->value);

	return true;
}

bool PowerOffTopAutoChkBoxOnPress(ITUWidget* widget, char* param)
{
	if (ituCheckBoxIsChecked(powerOffTopAutoCheckBox))
	{
		lightAuto = true;
		ituIconLinkSurface(&powerOffTopScreenLightTrackBar->tracker->bg.icon, powerOffTopBarBtnIcon[0]);


		ituWidgetDisable(powerOffTopScreenLightTrackBar);
		ituWidgetSetVisible(powerOffTopScreenLightProgressBar, false);


		if (indicatorLightEnable)
		{
			ituIconLinkSurface(&powerOffTopIndLightTrackBar->tracker->bg.icon, powerOffTopBarBtnIcon[0]);
			ituWidgetDisable(powerOffTopIndLightTrackBar);
			ituWidgetSetVisible(powerOffTopIndLightProgressBar, false);
		}
	}
	else
	{
		lightAuto = false;
		ituIconLinkSurface(&powerOffTopScreenLightTrackBar->tracker->bg.icon, powerOffTopBarBtnIcon[1]);
		ituWidgetEnable(powerOffTopScreenLightTrackBar);
		ituWidgetSetVisible(powerOffTopScreenLightProgressBar, true);

		if (indicatorLightEnable)
		{
			ituIconLinkSurface(&powerOffTopIndLightTrackBar->tracker->bg.icon, powerOffTopBarBtnIcon[1]);
			ituWidgetEnable(powerOffTopIndLightTrackBar);
			ituWidgetSetVisible(powerOffTopIndLightProgressBar, true);
		}
	}
	return true;
}
bool PowerOffPowerOnBtnOnPress(ITUWidget* widget, char* param)
{
	StopPowerOff();

	return true;
}

void StopPowerOff(void)
{
	if (!mainLayer)
	{
		mainLayer = ituSceneFindWidget(&theScene, "mainLayer");
		assert(mainLayer);
		mainCoverFlow = ituSceneFindWidget(&theScene, "mainCoverFlow");
		assert(mainCoverFlow);
	}
	powerOnTimeIndex = -1;
	powerOnTmHr = 0;
	powerOnTmMin = 0;

	ituLayerGoto(mainLayer);
	ituCoverFlowGoto(mainCoverFlow, 1);

}

