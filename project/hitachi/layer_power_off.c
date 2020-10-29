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
ITUCoverFlow* powerOffPwrOnCoverFlow = 0;
ITUContainer* powerOffPwrOnContainer[3] = { 0 };
ITUButton* powerOffPwrOnButton = 0;
ITUText* powerOffHumidityText = 0;
ITUContainer* powerOffTempBigContainer = 0;
ITUText* powerOffTempValueText = 0;
ITUText* powerOffTempPointText = 0;
ITUWheel* powerOffTimingHrWheel = 0;
ITUWheel* powerOffTimingMinWheel = 0;
ITUText* powerOffTimingText = 0;
ITUSprite* powerOffTimingSprite = 0;
ITUSprite* powerOffModeTextSprite = 0;
ITUWheel* powerOffTempWheel = 0;
ITUText* powerOffTempText = 0;
ITUTrackBar* powerOffAirForceTrackBar = 0;
ITUProgressBar* powerOffAirForceProgressBar = 0;
ITUSprite* powerOffAirForceTextSprite = 0;
ITUSprite* powerOffAirForceSprite = 0;
ITUSprite* powerOffAirForceLineSprite = 0;
ITUCheckBox* powerOffAirForceAutoCheckBox = 0;
ITUTrackBar* powerOffPreHumidityTrackBar = 0;
ITUText* powerOffPreHumidityText = 0;
ITUIcon* powerOffAirForceTrackBarIcon = 0;
ITUIcon* powerOffBarIcon[2] = { 0 };
ITUBackground* powerOffAirForceTipShowBackground = 0;
ITUText* powerOffAirForceTipShowText = 0;

int orgContainerPosX = 0;
int curContainerPosX = 0;

static int TimeHr = 0;
static int TimeMin = 0;
static int TimeSec = 0;

static int airForceIndex = 0;
static int preHumidityIndex = 50;

void StopPowerOff(void);


static int preTempValueIndex = 0;

static uint32_t gtTick = 0, gtLastTick = 0, gtRefreshTime = 3000;
static bool gtTickFirst = true;

#if defined(CFG_SHT20_ENABLE) || defined(CFG_NCP18_ENABLE)
static float current_tmp_float = 0;
#endif

bool PowerOffOnEnter(ITUWidget* widget, char* param)
{
	int i,x,y;
	char tmp[32];
	if (!powerOffPwrOnCoverFlow)
	{
		powerOffPwrOnCoverFlow = ituSceneFindWidget(&theScene, "powerOffPwrOnCoverFlow");
		assert(powerOffPwrOnCoverFlow);

		powerOffPwrOnButton = ituSceneFindWidget(&theScene, "powerOffPwrOnButton");
		assert(powerOffPwrOnButton);

		powerOffHumidityText = ituSceneFindWidget(&theScene, "powerOffHumidityText");
		assert(powerOffHumidityText);

		powerOffTempBigContainer = ituSceneFindWidget(&theScene, "powerOffTempBigContainer");
		assert(powerOffTempBigContainer);

		powerOffTempValueText = ituSceneFindWidget(&theScene, "powerOffTempValueText");
		assert(powerOffTempValueText);

		powerOffTempPointText = ituSceneFindWidget(&theScene, "powerOffTempPointText");
		assert(powerOffTempPointText);

		powerOffTimingHrWheel = ituSceneFindWidget(&theScene, "powerOffTimingHrWheel");
		assert(powerOffTimingHrWheel);

		powerOffTimingMinWheel = ituSceneFindWidget(&theScene, "powerOffTimingMinWheel");
		assert(powerOffTimingMinWheel);

		powerOffTimingText = ituSceneFindWidget(&theScene, "powerOffTimingText");
		assert(powerOffTimingText);

		powerOffTimingSprite = ituSceneFindWidget(&theScene, "powerOffTimingSprite");
		assert(powerOffTimingSprite);

		powerOffModeTextSprite = ituSceneFindWidget(&theScene, "powerOffModeTextSprite");
		assert(powerOffModeTextSprite);

		powerOffTempWheel = ituSceneFindWidget(&theScene, "powerOffTempWheel");
		assert(powerOffTempWheel);

		powerOffTempText = ituSceneFindWidget(&theScene, "powerOffTempText");
		assert(powerOffTempText);

		powerOffAirForceTrackBar = ituSceneFindWidget(&theScene, "powerOffAirForceTrackBar");
		assert(powerOffAirForceTrackBar);

		powerOffAirForceProgressBar = ituSceneFindWidget(&theScene, "powerOffAirForceProgressBar");
		assert(powerOffAirForceProgressBar);

		powerOffAirForceTextSprite = ituSceneFindWidget(&theScene, "powerOffAirForceTextSprite");
		assert(powerOffAirForceTextSprite);

		powerOffAirForceSprite = ituSceneFindWidget(&theScene, "powerOffAirForceSprite");
		assert(powerOffAirForceSprite);

		powerOffAirForceLineSprite = ituSceneFindWidget(&theScene, "powerOffAirForceLineSprite");
		assert(powerOffAirForceLineSprite);

		powerOffAirForceAutoCheckBox = ituSceneFindWidget(&theScene, "powerOffAirForceAutoCheckBox");
		assert(powerOffAirForceAutoCheckBox);

		powerOffAirForceTipShowBackground = ituSceneFindWidget(&theScene, "powerOffAirForceTipShowBackground");
		assert(powerOffAirForceTipShowBackground);

		powerOffAirForceTipShowText = ituSceneFindWidget(&theScene, "powerOffAirForceTipShowText");
		assert(powerOffAirForceTipShowText);

		powerOffPreHumidityTrackBar = ituSceneFindWidget(&theScene, "powerOffPreHumidityTrackBar");
		assert(powerOffPreHumidityTrackBar);

		powerOffPreHumidityText = ituSceneFindWidget(&theScene, "powerOffPreHumidityText");
		assert(powerOffPreHumidityText);

		powerOffAirForceTrackBarIcon = ituSceneFindWidget(&theScene, "powerOffAirForceTrackBarIcon");
		assert(powerOffAirForceTrackBarIcon);

		for (i = 0; i < 3; i++)
		{
			sprintf(tmp, "powerOffPwrOnContainer%d", i);
			powerOffPwrOnContainer[i] = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffPwrOnContainer[i]);
		}

		for (i = 0; i < 2; i++)
		{
			sprintf(tmp, "powerOffBarIcon%d", i);
			powerOffBarIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffBarIcon[i]);
		}
	}

	orgContainerPosX = ituWidgetGetX(powerOffPwrOnContainer[1]);
	ituCoverFlowGoto(powerOffPwrOnCoverFlow, 1);
	ituSpriteGoto(powerOffTimingSprite, 0);
	ituSpriteGoto(powerOffModeTextSprite, 0);

	ituWidgetSetVisible(powerOffAirForceTipShowBackground, false);
	ituWidgetGetGlobalPosition(powerOffAirForceTrackBar->tip, &x, &y);
	ituWidgetSetPosition(powerOffAirForceTipShowBackground, x, y);
	sprintf(tmp, "%d", powerOffAirForceTrackBar->value);
	ituTextSetString(powerOffAirForceTipShowText, tmp);
	ituWidgetSetVisible(powerOffAirForceTipShowBackground, true);

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
			curTempValueIndex = = (int)((-current_tmp_float + 0.5) - 20) / 0.5;
			curTempValueIndex = -curTempValueIndex;
		}

		preTempValueIndex = curTempValueIndex;
#else
#ifdef CFG_NCP18_ENABLE
		current_tmp_float = NCP18_Detect();
		if (current_tmp_float > 0)
		{
			curTempValueIndex = = (int)((current_tmp_float + 0.5) - 20) / 0.5;
		}
		else
		{
			curTempValueIndex = = (int)((-current_tmp_float + 0.5) - 20) / 0.5;
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

	curContainerPosX = ituWidgetGetX(powerOffPwrOnContainer[1]);


	dist = curContainerPosX - orgContainerPosX;

	if (abs(dist) > 35)
	{
		StopPowerOff();
		ret = true;
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
			curTempValueIndex = (int)((current_tmp_float + 0.5) - 20) / 0.5;
		}
		else
		{
			curTempValueIndex = = (int)((-current_tmp_float + 0.5) - 20) / 0.5;
			curTempValueIndex = -curTempValueIndex;
		}
#else
#ifdef CFG_NCP18_ENABLE
		current_tmp_float = NCP18_Detect();
		if (current_tmp_float > 0)
		{
			curTempValueIndex = = (int)((current_tmp_float + 0.5) - 20) / 0.5;
		}
		else
		{
			curTempValueIndex = = (int)((-current_tmp_float + 0.5) - 20) / 0.5;
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
	{
		if (TimeHr == 0 && TimeMin == 0 && TimeSec == 0)
		{
			StopPowerOff();

		}

		if (curSec != TimeSec)
		{
			TimeSec = curSec;

			TimeHr = powerOffTimingHrWheel->focusIndex;
			TimeMin = powerOffTimingMinWheel->focusIndex;

			if (TimeMin == 0)
			{
				TimeMin = 59;
				TimeHr--;
			}
			else
			{
				TimeMin--;
			}

			

			if (TimeMin >= tm->tm_min)
			{
				TimeMin = TimeMin - tm->tm_min;

				if (TimeHr >= tm->tm_hour)
				{
					TimeHr = TimeHr - tm->tm_hour;
				}
				else
				{
					TimeHr = TimeHr + 24 - tm->tm_hour;
				}
			}
			else
			{
				TimeMin = TimeMin + 60 - tm->tm_min;
				TimeHr--;

				if (TimeHr >= tm->tm_hour)
				{
					TimeHr = TimeHr - tm->tm_hour;
				}
				else
				{
					TimeHr = TimeHr + 24 - tm->tm_hour;
				}

			}


			sprintf(tmp, "%02d:%02d:%02d", TimeHr, TimeMin, TimeSec);
			ituTextSetString(powerOffTimingText, tmp);

			ret = true;
		}
	}




	return ret;
}

bool PowerOffTimingSaveBtnOnPress(ITUWidget* widget, char* param)
{
	char tmp[32];
	struct timeval tv;
	struct tm *tm;

	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	TimeHr = powerOffTimingHrWheel->focusIndex;
	TimeMin = powerOffTimingMinWheel->focusIndex;
	
	if (TimeMin == 0)
	{
		TimeMin = 59;
		TimeHr--;
	}
	else
	{
		TimeMin--;
	}

	TimeSec = 59 - tm->tm_sec;

	if (TimeMin >= tm->tm_min)
	{
		TimeMin = TimeMin - tm->tm_min;

		if (TimeHr >= tm->tm_hour)
		{
			TimeHr = TimeHr - tm->tm_hour;
		}
		else
		{
			TimeHr = TimeHr+ 24 - tm->tm_hour;
		}
	}
	else
	{
		TimeMin = TimeMin + 60 - tm->tm_min;
		TimeHr--;

		if (TimeHr >= tm->tm_hour)
		{
			TimeHr = TimeHr - tm->tm_hour;
		}
		else
		{
			TimeHr = TimeHr + 24 - tm->tm_hour;
		}

	}

	

	sprintf(tmp, "%02d:%02d:%02d", TimeHr, TimeMin, TimeSec);
	ituTextSetString(powerOffTimingText, tmp);
	ituSpriteGoto(powerOffTimingSprite, 1);

    return true;
}
bool PowerOffModeRadioBoxOnPress(ITUWidget* widget, char* param)
{
	int index = atoi(param);

	ituSpriteGoto(powerOffModeTextSprite, index);

	return true;
}

bool PowerOffTempSaveBtnOnPress(ITUWidget* widget, char* param)
{
	char tmp[32];

	sprintf(tmp, "%02.1f", (powerOffTempWheel->focusIndex * 0.5) + 18);
	ituTextSetString(powerOffTempText, tmp);

	return true;
}
bool PowerOffAirForceTrackBarOnChanged(ITUWidget* widget, char* param)
{
	int x, y;
	char tmp[32];

	airForceIndex = powerOffAirForceTrackBar->value - 1;
	ituSpriteGoto(powerOffAirForceSprite, airForceIndex);
	ituSpriteGoto(powerOffAirForceLineSprite, airForceIndex);
	ituSpriteGoto(powerOffAirForceTextSprite, airForceIndex);

	ituWidgetSetVisible(powerOffAirForceTipShowBackground, false);
	ituWidgetGetGlobalPosition(powerOffAirForceTrackBar->tip, &x, &y);
	ituWidgetSetPosition(powerOffAirForceTipShowBackground, x, y);
	sprintf(tmp, "%d", powerOffAirForceTrackBar->value);
	ituTextSetString(powerOffAirForceTipShowText, tmp);
	ituWidgetSetVisible(powerOffAirForceTipShowBackground, true);


	return true;
}

bool PowerOffAirForceAutoChkBoxOnPress(ITUWidget* widget, char* param)
{
	int x, y;
	char tmp[32];
	if (ituCheckBoxIsChecked(powerOffAirForceAutoCheckBox))
	{

		ituTrackBarSetValue(powerOffAirForceTrackBar, 3);
		ituProgressBarSetValue(powerOffAirForceProgressBar, 3);
		ituSpriteGoto(powerOffAirForceSprite, 6);
		ituSpriteGoto(powerOffAirForceLineSprite, 2);
		ituSpriteGoto(powerOffAirForceTextSprite, 6);
		ituWidgetDisable(powerOffAirForceTrackBar);
		ituWidgetSetVisible(powerOffAirForceProgressBar, false);
		ituIconLinkSurface(powerOffAirForceTrackBarIcon, powerOffBarIcon[1]);

		ituWidgetGetGlobalPosition(powerOffAirForceTrackBar->tip, &x, &y);
		ituWidgetSetPosition(powerOffAirForceTipShowBackground, x, y);
		ituTextSetString(powerOffAirForceTipShowText, "A");
	}
	else
	{

		ituTrackBarSetValue(powerOffAirForceTrackBar, airForceIndex + 1);
		ituProgressBarSetValue(powerOffAirForceProgressBar, airForceIndex + 1);
		ituSpriteGoto(powerOffAirForceSprite, airForceIndex);
		ituSpriteGoto(powerOffAirForceLineSprite, airForceIndex);
		ituSpriteGoto(powerOffAirForceTextSprite, airForceIndex);
		ituWidgetEnable(powerOffAirForceTrackBar);
		ituWidgetSetVisible(powerOffAirForceProgressBar, true);
		ituIconLinkSurface(powerOffAirForceTrackBarIcon, powerOffBarIcon[0]);

		ituWidgetGetGlobalPosition(powerOffAirForceTrackBar->tip, &x, &y);
		ituWidgetSetPosition(powerOffAirForceTipShowBackground, x, y);
		sprintf(tmp, "%d", powerOffAirForceTrackBar->value);
		ituTextSetString(powerOffAirForceTipShowText, tmp);
	}

	return true;
}
bool PowerOffPreHumidityTrackBarOnChanged(ITUWidget* widget, char* param)
{
	char tmp[32];
	preHumidityIndex = powerOffPreHumidityTrackBar->value;

	sprintf(tmp, "%3d", preHumidityIndex);
	ituTextSetString(powerOffPreHumidityText, tmp);


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
	ituLayerGoto(mainLayer);
	ituCoverFlowGoto(mainCoverFlow, 1);

}

