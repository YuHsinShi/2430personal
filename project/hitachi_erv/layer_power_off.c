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
ITUText* powerOffTempText = 0;
ITUText* powerOffPM25Text = 0;
ITUText* powerOffTVOCText = 0;
ITUText* powerOffCO2Text = 0;
ITUSprite* powerOffTimingSprite = 0;
ITUWheel* powerOffTimingHrWheel = 0;
ITUWheel* powerOffTimingMinWheel = 0;
ITUText* powerOffTimingText = 0;


int orgContainerPosX = 0;
int curContainerPosX = 0;

static int TimeHr = 0;
static int TimeMin = 0;
static int TimeSec = 0;

void StopPowerOff(void);


static int preTempIndex = 0;

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

		powerOffTempText = ituSceneFindWidget(&theScene, "powerOffTempText");
		assert(powerOffTempText);

		powerOffPM25Text = ituSceneFindWidget(&theScene, "powerOffPM25Text");
		assert(powerOffPM25Text);

		powerOffTVOCText = ituSceneFindWidget(&theScene, "powerOffTVOCText");
		assert(powerOffTVOCText);

		powerOffCO2Text = ituSceneFindWidget(&theScene, "powerOffCO2Text");
		assert(powerOffCO2Text);

		

		powerOffTimingHrWheel = ituSceneFindWidget(&theScene, "powerOffTimingHrWheel");
		assert(powerOffTimingHrWheel);

		powerOffTimingMinWheel = ituSceneFindWidget(&theScene, "powerOffTimingMinWheel");
		assert(powerOffTimingMinWheel);

		powerOffTimingText = ituSceneFindWidget(&theScene, "powerOffTimingText");
		assert(powerOffTimingText);

		powerOffTimingSprite = ituSceneFindWidget(&theScene, "powerOffTimingSprite");
		assert(powerOffTimingSprite);


		for (i = 0; i < 3; i++)
		{
			sprintf(tmp, "powerOffPwrOnContainer%d", i);
			powerOffPwrOnContainer[i] = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffPwrOnContainer[i]);
		}

	}

	orgContainerPosX = ituWidgetGetX(powerOffPwrOnContainer[1]);
	ituCoverFlowGoto(powerOffPwrOnCoverFlow, 1);
	ituSpriteGoto(powerOffTimingSprite, 0);


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
	ituTextSetString(powerOffTempText, tmp);



	

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
		ituTextSetString(powerOffTempText, tmp);


		preTempIndex = temperatureInIndex;
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

