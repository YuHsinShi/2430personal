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

static ITUSprite* screenLockBackgroundSprite = 0;
static ITUText* screenLockTempText = 0;
static ITUText* screenLockCO2Text = 0;
static ITUText* screenLockTVOCText = 0;
static ITUText* screenLockPM25Text = 0;
static ITUSprite* screenLockWeatherIconSprite = 0;
static ITUSprite* screenLockWeatherSprite = 0;
static ITUSprite* screenLockWeatherStatusSprite = 0;

static ITUCoverFlow* screenLockCoverFlow = 0;
static ITUButton* screenLockButton = 0;
static ITUContainer* screenLockContainer[3] = { 0 };

static ITUContainer* screenLockTimingTextContainer = 0;
static ITUText* screenLockTimingText = 0;

static ITULayer* powerOffLayer = 0;

static int orgPosX = 0;
static int curPosX = 0;

static int weatherIndex = 2;

void StopScreenLock(void);

static int preTempIndex = 0;

static int TimeSec = 0;
double lastTime;

static uint32_t gtTick = 0, gtLastTick = 0, gtRefreshTime = 3000;
static bool gtTickFirst = true;

#if defined(CFG_SHT20_ENABLE) || defined(CFG_NCP18_ENABLE)
static float current_tmp_float = 0;
#endif

void screenLockGotoPowerOff(void);

bool ScreenLockOnEnter(ITUWidget* widget, char* param)
{
	int i;
	char tmp[32];

	if (!screenLockCoverFlow)
	{
		
		screenLockCoverFlow = ituSceneFindWidget(&theScene, "screenLockCoverFlow");
		assert(screenLockCoverFlow);

		screenLockButton = ituSceneFindWidget(&theScene, "screenLockButton");
		assert(screenLockButton);

		screenLockBackgroundSprite = ituSceneFindWidget(&theScene, "screenLockBackgroundSprite");
		assert(screenLockBackgroundSprite);

		screenLockTempText = ituSceneFindWidget(&theScene, "screenLockTempText");
		assert(screenLockTempText);

		screenLockPM25Text = ituSceneFindWidget(&theScene, "screenLockPM25Text");
		assert(screenLockPM25Text);

		screenLockCO2Text = ituSceneFindWidget(&theScene, "screenLockCO2Text");
		assert(screenLockCO2Text);

		screenLockTVOCText = ituSceneFindWidget(&theScene, "screenLockTVOCText");
		assert(screenLockTVOCText);
		
		screenLockWeatherIconSprite = ituSceneFindWidget(&theScene, "screenLockWeatherIconSprite");
		assert(screenLockWeatherIconSprite);

		screenLockWeatherSprite = ituSceneFindWidget(&theScene, "screenLockWeatherSprite");
		assert(screenLockWeatherSprite);

		screenLockWeatherStatusSprite = ituSceneFindWidget(&theScene, "screenLockWeatherStatusSprite");
		assert(screenLockWeatherStatusSprite);

		screenLockTimingTextContainer = ituSceneFindWidget(&theScene, "screenLockTimingTextContainer");
		assert(screenLockTimingTextContainer);

		screenLockTimingText = ituSceneFindWidget(&theScene, "screenLockTimingText");
		assert(screenLockTimingText);

		for (i = 0; i < 3; i++)
		{
			sprintf(tmp, "screenLockContainer%d", i);
			screenLockContainer[i] = ituSceneFindWidget(&theScene, tmp);
			assert(screenLockContainer[i]);
		}


	}

	sprintf(tmp, "%d", PM25In);
	ituTextSetString(screenLockPM25Text, tmp);

	sprintf(tmp, "%d", CO2In);
	ituTextSetString(screenLockCO2Text, tmp);

	sprintf(tmp, "%d", TVOCIn);
	ituTextSetString(screenLockTVOCText, tmp);

	if (powerOffTimeIndex > -1)
		ituWidgetSetVisible(screenLockTimingTextContainer, true);
	else
		ituWidgetSetVisible(screenLockTimingTextContainer, false);


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
			temperatureInIndex = (int)(current_tmp_float + 0.5);
		}
		else
		{
			temperatureInIndex = (int)(-current_tmp_float + 0.5);
			temperatureInIndex = -temperatureInIndex;
		}

		preTempIndex = temperatureInIndex;
#else
#ifdef CFG_NCP18_ENABLE
		current_tmp_float = NCP18_Detect();
		if (current_tmp_float > 0)
		{
			temperatureInIndex = (int)(current_tmp_float + 0.5);
		}
		else
		{
			temperatureInIndex = (int)(-current_tmp_float + 0.5);
			temperatureInIndex = -temperatureInIndex;
		}

		preTempIndex = temperatureInIndex;
#else
		temperatureInIndex = 29;//get from sensor
		preTempIndex = temperatureInIndex;

#endif
#endif

		gtLastTick = gtTick;
	}


	sprintf(tmp, "%d", temperatureInIndex);
	ituTextSetString(screenLockTempText, tmp);

	ituSpriteGoto(screenLockBackgroundSprite, weatherIndex);
	ituSpriteGoto(screenLockWeatherIconSprite, weatherIndex);
	ituSpriteGoto(screenLockWeatherSprite, weatherIndex);
	ituSpriteGoto(screenLockWeatherStatusSprite, 0);

	orgPosX = ituWidgetGetX(screenLockContainer[1]);

	ituCoverFlowGoto(screenLockCoverFlow, 1);


	return true;
}

bool ScreenLockOnTimer(ITUWidget* widget, char* param)
{
	bool ret = false;
	int dist = 0;
	char tmp[32];

	struct timeval tv;
	struct tm *tm;
	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	int curSec = 59 - tm->tm_sec;

	curPosX = ituWidgetGetX(screenLockContainer[1]);


	dist = curPosX - orgPosX;

	if (abs(dist)>35)
	{
		StopScreenLock();
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
			temperatureInIndex = (int)(current_tmp_float + 0.5);
		}
		else
		{
			temperatureInIndex = (int)(-current_tmp_float + 0.5);
			temperatureInIndex = -temperatureInIndex;
		}
#else
#ifdef CFG_NCP18_ENABLE
		current_tmp_float = NCP18_Detect();
		if (current_tmp_float > 0)
		{
			temperatureInIndex = (int)(current_tmp_float + 0.5);
		}
		else
		{
			temperatureInIndex = (int)(-current_tmp_float + 0.5);
			temperatureInIndex = -temperatureInIndex;
		}
#else
		temperatureInIndex = 29;//get from sensor
#endif
#endif

		gtLastTick = gtTick;
	}


	if (preTempIndex != temperatureInIndex)
	{
		sprintf(tmp, "%d", temperatureInIndex);
		ituTextSetString(screenLockTempText, tmp);

		preTempIndex = temperatureInIndex;
		ret = true;
	}

	if (powerOffTimeIndex != -1)
	{
		if (powerOffTmHr == tm->tm_hour && powerOffTmMin == tm->tm_min)
		{
			screenLockGotoPowerOff();
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
			ituTextSetString(screenLockTimingText, tmp);

			ret = true;
		}

	}
	
	return ret;
}
void StopScreenLock(void)
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

void screenLockGotoPowerOff(void)
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