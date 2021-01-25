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

ITULayer* mainLayer;
ITUCoverFlow* mainCoverFlow;
//ITUCoverFlow* powerOffPwrOnCoverFlow = 0;
//ITUContainer* powerOffPwrOnContainer[3] = { 0 };
//ITUButton* powerOffPwrOnButton = 0;
ITUText* powerOffHumidityText = 0;
ITUContainer* powerOffTempBigContainer = 0;
ITUText* powerOffTempValueText = 0;
ITUText* powerOffTempPointText = 0;
ITUText* powerOffTimingText = 0;
ITUSprite* powerOffTimingSprite = 0;
ITUTrackBar* powerOffTopIndLightTrackBar = 0;
ITUProgressBar* powerOffTopIndLightProgressBar = 0;
ITUTrackBar* powerOffTopScreenLightTrackBar = 0;
ITUProgressBar* powerOffTopScreenLightProgressBar = 0;
ITUIcon* powerOffTopBarBtnIcon[2] = { 0 };
ITUCheckBox* powerOffTopAutoCheckBox = 0;
ITUBackground* powerOffDisinfectStartBackground = 0;
ITUBackground* powerOffBackground = 0;
ITUBackground* powerOffCleanStartBackground = 0;

int orgContainerPosX = 0;
int curContainerPosX = 0;

static int TimeHr = 0;
static int TimeMin = 0;
static int TimeSec = 0;
extern int powerOnTimeIndex = -1;
double lastTime;
extern int powerOnTmHr = 0;
extern int powerOnTmMin = 0;

static bool startDisinfect = false;
static int disinfectProgressCnt = 0;

static bool startClean = false;
static int cleanProgressCnt = 0;

void StopPowerOff(void);


static int preTempValueIndex = 0;

static uint32_t gtTick = 0, gtLastTick = 0, gtRefreshTime = 3000;
static bool gtTickFirst = true;

#if defined(CFG_SHT20_ENABLE) || defined(CFG_NCP18_ENABLE)
static float current_tmp_float = 0;
#endif
bool disinfectProgress(void)
{


	if (disinfectProgressCnt < 100)
	{
		disinfectProgressCnt++;
		printf("disinfectProgressCnt = %d\n", disinfectProgressCnt);
	}
	else
	{
		startDisinfect = false;
		disinfectProgressCnt = 0;
		ituWidgetSetVisible(powerOffDisinfectStartBackground, false);
		ituWidgetEnable(powerOffBackground);
	}



	return true;
}

bool cleanProgress(void)
{


	if (cleanProgressCnt < 100)
	{
		cleanProgressCnt++;
		printf("cleanProgressCnt = %d\n", cleanProgressCnt);
	}
	else
	{
		startClean = false;
		cleanProgressCnt = 0;
		ituWidgetSetVisible(powerOffCleanStartBackground, false);
		ituWidgetEnable(powerOffBackground);
	}



	return true;
}
bool PowerOffOnEnter(ITUWidget* widget, char* param)
{
	int i;
	//int x, y;
	char tmp[32];
	if (!powerOffTempBigContainer)
	{
		//powerOffPwrOnCoverFlow = ituSceneFindWidget(&theScene, "powerOffPwrOnCoverFlow");
		//assert(powerOffPwrOnCoverFlow);

		//powerOffPwrOnButton = ituSceneFindWidget(&theScene, "powerOffPwrOnButton");
		//assert(powerOffPwrOnButton);

		powerOffHumidityText = ituSceneFindWidget(&theScene, "powerOffHumidityText");
		assert(powerOffHumidityText);

		powerOffTempBigContainer = ituSceneFindWidget(&theScene, "powerOffTempBigContainer");
		assert(powerOffTempBigContainer);

		powerOffTempValueText = ituSceneFindWidget(&theScene, "powerOffTempValueText");
		assert(powerOffTempValueText);

		powerOffTempPointText = ituSceneFindWidget(&theScene, "powerOffTempPointText");
		assert(powerOffTempPointText);

		powerOffTimingText = ituSceneFindWidget(&theScene, "powerOffTimingText");
		assert(powerOffTimingText);

		powerOffTimingSprite = ituSceneFindWidget(&theScene, "powerOffTimingSprite");
		assert(powerOffTimingSprite);

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

		powerOffDisinfectStartBackground = ituSceneFindWidget(&theScene, "powerOffDisinfectStartBackground");
		assert(powerOffDisinfectStartBackground);

		powerOffBackground = ituSceneFindWidget(&theScene, "powerOffBackground");
		assert(powerOffBackground);

		powerOffCleanStartBackground = ituSceneFindWidget(&theScene, "powerOffCleanStartBackground");
		assert(powerOffCleanStartBackground);


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
	//ituSpriteGoto(powerOffTimingSprite, 0);
	//ituSpriteGoto(powerOffModeTextSprite, 0);

	powerOffTimeIndex = -1;
	powerOffTmHr = 0;
	powerOffTmMin = 0;

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

		preTempValueIndex = curTempValueIndex;
#else
#ifdef CFG_NCP18_ENABLE
		current_tmp_float = NCP18_Detect();
		if (current_tmp_float > 0)
		{
			curTempValueIndex = (int)((current_tmp_float + 0.5) - 20) / 0.5;
		}
		else
		{
			curTempValueIndex = (int)((-current_tmp_float + 0.5) - 20) / 0.5;
			curTempValueIndex = -curTempValueIndex;
		}

		preTempValueIndex = curTempValueIndex;
#else
		curTempValueIndex = 19;//get from sensor
		preTempValueIndex = curTempValueIndex;
#endif
#endif

		gtLastTick = gtTick;
	}

	if (curTempValueIndex > 20)
		curTempValueIndex = 20;
	else if (curTempValueIndex < 0)
		curTempValueIndex = 0;

	sprintf(tmp, "%d", temperatureValueSet[curTempValueIndex]);
	ituTextSetString(powerOffTempValueText, tmp);

	if (curTempValueIndex % 2)
		ituWidgetSetVisible(powerOffTempPointText, true);
	else
		ituWidgetSetVisible(powerOffTempPointText, false);

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
	static int disinfectCnt = 0;
	static int cleanCnt = 0;

	struct timeval tv;
	struct tm *tm;
	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	int curSec = 59 - tm->tm_sec;

	//curContainerPosX = ituWidgetGetX(powerOffPwrOnContainer[1]);


	//dist = curContainerPosX - orgContainerPosX;

	//if (abs(dist) > 80)
	//{
	//	StopPowerOff();
	//	ret = true;
	//}

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
		if (startDisinfect)
		{
			disinfectCnt++;
			if (disinfectCnt > 5)
			{
				disinfectCnt = 0;
				ret = ret | disinfectProgress();
			}
		}	

		if (startClean)
		{
			cleanCnt++;
			if (cleanCnt > 5)
			{
				cleanCnt = 0;
				ret = ret | cleanProgress();
			}
		}
	
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
			curTempValueIndex = (int)((-current_tmp_float + 0.5) - 20) / 0.5;
			curTempValueIndex = -curTempValueIndex;
		}
#else
		curTempValueIndex = 19;//get from sensor

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
		ituTextSetString(powerOffTempValueText, tmp);

		if (curTempValueIndex % 2)
			ituWidgetSetVisible(powerOffTempPointText, true);
		else
			ituWidgetSetVisible(powerOffTempPointText, false);

		preTempValueIndex = curTempValueIndex;
		ret = true;
	}

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

bool PowerOffDisinfectEnterBtnOnPress(ITUWidget* widget, char* param)
{
	startDisinfect = true;
	disinfectProgressCnt = 0;
	return true;
}

bool PowerOffDisinfectStopEnterBtnOnPress(ITUWidget* widget, char* param)
{
	startDisinfect = false;

	return true;
}

bool PowerOffCleanEnterBtnOnPress(ITUWidget* widget, char* param)
{
	startClean = true;
	cleanProgressCnt = 0;
	return true;
}

bool PowerOffCleanStopEnterBtnOnPress(ITUWidget* widget, char* param)
	{
	startClean = false;

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

	startDisinfect = false;
	disinfectProgressCnt = 0;
	ituWidgetSetVisible(powerOffDisinfectStartBackground, false);
	ituWidgetEnable(powerOffBackground);

	startClean = false;
	cleanProgressCnt = 0;
	ituWidgetSetVisible(powerOffCleanStartBackground, false);



	Hlink_send_state(HLINK_POWER_ON);

	ituLayerGoto(mainLayer);
	ituCoverFlowGoto(mainCoverFlow, 1);

}

