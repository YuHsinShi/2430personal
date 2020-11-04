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

ITULayer* mainLayer ;
ITUCoverFlow* mainCoverFlow ;

ITUSprite* screenLockBackgroundSprite = 0;
ITUText* screenLockTempText = 0;
ITUText* screenLockPM25Text = 0;
ITUText* screenLockCO2Text = 0;
ITUText* screenLockTVOCText = 0;
ITUText* screenLockHCHOText = 0;
ITUSprite* screenLockWeatherIconSprite = 0;
ITUSprite* screenLockWeatherSprite = 0;

ITUCoverFlow* screenLockCoverFlow = 0;
ITUButton* screenLockButton = 0;
ITUContainer* screenLockContainer[3] = { 0 };

int orgPosX = 0;
int curPosX = 0;

static int weatherIndex = 0;

void StopScreenLock(void);

static int preTempIndex = 0;

static uint32_t gtTick = 0, gtLastTick = 0, gtRefreshTime = 3000;
static bool gtTickFirst = true;

#if defined(CFG_SHT20_ENABLE) || defined(CFG_NCP18_ENABLE)
static float current_tmp_float = 0;
#endif

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

		screenLockHCHOText = ituSceneFindWidget(&theScene, "screenLockHCHOText");
		assert(screenLockHCHOText);

		screenLockWeatherIconSprite = ituSceneFindWidget(&theScene, "screenLockWeatherIconSprite");
		assert(screenLockWeatherIconSprite);

		screenLockWeatherSprite = ituSceneFindWidget(&theScene, "screenLockWeatherSprite");
		assert(screenLockWeatherSprite);


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

	sprintf(tmp, "%2.2f", HCHOIn);
	ituTextSetString(screenLockHCHOText, tmp);


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

	orgPosX = ituWidgetGetX(screenLockContainer[1]);

	ituCoverFlowGoto(screenLockCoverFlow, 1);


	return true;
}

bool ScreenLockOnTimer(ITUWidget* widget, char* param)
{
	bool ret = false;
	int dist = 0;
	char tmp[32];

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

