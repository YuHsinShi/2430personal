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
ITUCoverFlow* screenLockCoverFlow = 0;
ITUButton* screenLockButton = 0;
ITUContainer* screenLockHumidityBigContainer = 0;
ITUText* screenLockHumidityValueText = 0;
ITUContainer* screenLockTempBigContainer = 0;
ITUText* screenLockTempValueText = 0;
ITUText* screenLockTempPointText = 0;
ITUIcon* screenLockModeShowIcon[MODESHOW_NUM] = { 0 };
ITUSprite* screenLockModeShowIconSprite = 0;
ITUContainer* screenLockHumidityContainer = 0;
ITUText* screenLockHumidityText = 0;
ITUContainer* screenLockContainer[3] = { 0 };
ITUVideo* screenLockVideoBackGround = 0;

static ITUContainer* screenLockTimingTextContainer = 0;
static ITUText* screenLockTimingText = 0;

static ITULayer* powerOffLayer = 0;

int orgPosX = 0;
int curPosX = 0;

static int modeshowPosX = 0;
static int modeshowNum = 0;


void StopScreenLock(void);

static int preHumidityValueIndex = 0;
static int preTempValueIndex = 0;

static int TimeSec = 0;
double lastTime;
extern int powerOffTimeIndex = -1;
extern int powerOffTmHr = 0;
extern int powerOffTmMin = 0;

static uint32_t gtTick = 0, gtLastTick = 0, gtRefreshTime = 3000;
static bool gtTickFirst = true;

#if defined(CFG_SHT20_ENABLE) || defined(CFG_NCP18_ENABLE)
static float current_tmp_float = 0;
#endif

void screenLockGotoPowerOff(void);

void LoadScreenLockVideoBackGround()
{
	if (!screenLockVideoBackGround)
	{
		screenLockVideoBackGround = ituSceneFindWidget(&theScene, "screenLockVideoBackGround");
		assert(screenLockVideoBackGround);
	}

#ifdef CFG_VIDEO_ENABLE
	ituVideoStop(screenLockVideoBackGround);
#endif // CFG_VIDEO_ENABLE



	printf("[s]path=%s modeIndex=%d\n", screenLockVideoBackGround->filePath, BgIndex[modeIndex]);
	switch (BgIndex[modeIndex])
	{
	case 0:
		snprintf(screenLockVideoBackGround->filePath, 32, "%s", "B:/media/cooler.mkv");
		break;

	case 1:
		snprintf(screenLockVideoBackGround->filePath, 32, "%s", "B:/media/dehumid.mkv");
		break;

	case 2:
		snprintf(screenLockVideoBackGround->filePath, 32, "%s", "B:/media/fans.mkv");
		break;

	case 3:
		snprintf(screenLockVideoBackGround->filePath, 32, "%s", "B:/media/heater.mkv");
		break;

	case 4:
		snprintf(screenLockVideoBackGround->filePath, 32, "%s", "B:/media/humidify.mkv");
		break;

	default:

		printf(" [s]mode error ?(%d)", modeIndex);
		snprintf(screenLockVideoBackGround->filePath, 32, "%s", "B:/media/cooler.mkv");

		break;
	}
	printf("path change=%s \n", screenLockVideoBackGround->filePath);
#ifdef CFG_VIDEO_ENABLE
	ituVideoPlay(screenLockVideoBackGround, 1);
#endif // CFG_VIDEO_ENABLE


}
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

		screenLockHumidityBigContainer = ituSceneFindWidget(&theScene, "screenLockHumidityBigContainer");
		assert(screenLockHumidityBigContainer);

		screenLockHumidityValueText = ituSceneFindWidget(&theScene, "screenLockHumidityValueText");
		assert(screenLockHumidityValueText);

		screenLockTempBigContainer = ituSceneFindWidget(&theScene, "screenLockTempBigContainer");
		assert(screenLockTempBigContainer);

		screenLockTempValueText = ituSceneFindWidget(&theScene, "screenLockTempValueText");
		assert(screenLockTempValueText);

		screenLockTempPointText = ituSceneFindWidget(&theScene, "screenLockTempPointText");
		assert(screenLockTempPointText);

		screenLockHumidityContainer = ituSceneFindWidget(&theScene, "screenLockHumidityContainer");
		assert(screenLockHumidityContainer);
		
		screenLockHumidityText = ituSceneFindWidget(&theScene, "screenLockHumidityText");
		assert(screenLockHumidityText);
		
		screenLockModeShowIconSprite = ituSceneFindWidget(&theScene, "screenLockModeShowIconSprite");
		assert(screenLockModeShowIconSprite);

		screenLockTimingTextContainer = ituSceneFindWidget(&theScene, "screenLockTimingTextContainer");
		assert(screenLockTimingTextContainer);

		screenLockTimingText = ituSceneFindWidget(&theScene, "screenLockTimingText");
		assert(screenLockTimingText);

		for (i = 0; i < MODESHOW_NUM; i++)
		{
			sprintf(tmp, "screenLockModeShowIcon%d", i);
			screenLockModeShowIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(screenLockModeShowIcon[i]);
		}

		for (i = 0; i < 3; i++)
		{
			sprintf(tmp, "screenLockContainer%d", i);
			screenLockContainer[i] = ituSceneFindWidget(&theScene, tmp);
			assert(screenLockContainer[i]);
		}





	}

	LoadScreenLockVideoBackGround();

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
		ituWidgetSetVisible(screenLockTempBigContainer, true);
		ituWidgetSetVisible(screenLockHumidityBigContainer, false);


		sprintf(tmp, "%d", temperatureValueSet[curTempValueIndex]);
		ituTextSetString(screenLockTempValueText, tmp);

		if (curTempValueIndex % 2)
			ituWidgetSetVisible(screenLockTempPointText, true);
		else
			ituWidgetSetVisible(screenLockTempPointText, false);
	}
	else
	{
		ituWidgetSetVisible(screenLockTempBigContainer, false);
		ituWidgetSetVisible(screenLockHumidityBigContainer, true);


		sprintf(tmp, "%d", humidityValueIndex);
		ituTextSetString(screenLockHumidityValueText, tmp);
	}



	orgPosX = ituWidgetGetX(screenLockContainer[1]);


	ituWidgetSetVisible(screenLockHumidityContainer, humidity_container_show[modeIndex]);


	sprintf(tmp, "%d", curHumidityValueIndex);
	ituTextSetString(screenLockHumidityText, tmp);

	ituCoverFlowGoto(screenLockCoverFlow, 1);

	modeshowPosX = 0;
	for (i = 0; i < MODESHOW_NUM; i++)
	{
		if (mode_show[i])
		{
			if (i == 0)
			{
				ituWidgetSetPosition(screenLockModeShowIconSprite, modeshowPosX, 0);
			}
			else
			{
			ituWidgetSetPosition(screenLockModeShowIcon[i], modeshowPosX, 0);
			}

			modeshowNum++;
			if (i != MODESHOW_NUM-1)
			modeshowPosX = modeshowPosX + 46;
			else
				modeshowPosX = modeshowPosX + 76;
		}
		else
		{
			if (i == 0)
			{
				ituWidgetSetPosition(screenLockModeShowIconSprite, 0, 100);
		}
		else
		{
			ituWidgetSetPosition(screenLockModeShowIcon[i], 0, 100);
		}
		}

	}

	ituSpriteGoto(screenLockModeShowIconSprite, wifi_status);

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

	if (abs(dist)>72)
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
		ituTextSetString(screenLockTempValueText, tmp);

		if (curTempValueIndex % 2)
			ituWidgetSetVisible(screenLockTempPointText, true);
		else
			ituWidgetSetVisible(screenLockTempPointText, false);

		preTempValueIndex = curTempValueIndex;
		ret = true;
	}

	if (preHumidityValueIndex != curHumidityValueIndex)
	{
		sprintf(tmp, "%d", curHumidityValueIndex);
		ituTextSetString(screenLockHumidityValueText, tmp);


		preHumidityValueIndex = curHumidityValueIndex;
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
	//ITULayer* layer = (ITULayer*)ituGetVarTarget(2);
	//ituLayerGoto(layer);

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