﻿#include <assert.h>
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

#define MAIN_TEXT_NUM 4
#define MAIN_AIRFLOW_NUM 5

static ITUSprite* maneBackgroundSprite;
static ITUContainer* mainContainer2;
static ITUBackground* mainCurrentTmpBackground;
static ITUBackground* mainSetTmpBackground;
static ITUIcon* mainBigDigitalIcon[10];
static ITUIcon* mainSColdDigitalIcon[10];
static ITUIcon* mainSHotDigitalIcon[10];

static ITUDigitalClock* mainDigitalClock;
static ITUIcon* mainTimeHourIcon[2];
static ITUIcon* mainTimeMinuteIcon[2];
static ITUSprite* mainTimeColonSprite;
static ITUSprite* mainAMPMSprite;


static ITUIcon* mainSetTemperatureIcon[2];
static ITUIcon* mainTemperatureIcon[2];

static ITUSprite* mainSprite;

static ITUSprite* mainSetTemperatureSprite;
static ITUMeter* mainColdTemperatureMeter;
static ITUMeter* mainHotTemperatureMeter;

static ITUText* mainText[MAIN_TEXT_NUM] = { 0 };

static ITUBackground* mainAirFlowBackground[MAIN_AIRFLOW_NUM] = { 0 };
static ITUBackground* mainAirFlowEnBackground[MAIN_AIRFLOW_NUM] = { 0 };
static ITUBackground* mainAirFlowChBackground[MAIN_AIRFLOW_NUM] = { 0 };

static uint32_t gtTick = 0, gtLastTick = 0, gtRefreshTime = 3000;
static bool gtTickFirst = true;

ITUSprite* topWiFiSprite;

int container_posX;
uint32_t hide_start_timer;
bool is_current_tmp_hide = false;
int set_tmp;
int current_tmp;//current temperature

#if defined(CFG_SHT20_ENABLE) || defined(CFG_NCP18_ENABLE)
    static float current_tmp_float = 0;
#endif

int pre_tmp;//pre temperatue

static int is_setting_cold = 1;
int current_time_hour = 0;
int current_time_minute = 0;
int pre_time_hour = 0;
int pre_time_minute = 0;

extern uint32_t SDL_GetTicks(void);

void ChtNameMain(void);
void EngNameMain(void);

void MainReset(void)
{
}
bool MainOnEnter(ITUWidget* widget, char* param)
{
	int i = 0, wifi_established = -1;
	char tmp[32];
	
	if (!maneBackgroundSprite)
	{
		maneBackgroundSprite = ituSceneFindWidget(&theScene, "maneBackgroundSprite");
		assert(maneBackgroundSprite);

		mainContainer2 = ituSceneFindWidget(&theScene, "mainContainer2");
		assert(mainContainer2);

		mainCurrentTmpBackground = ituSceneFindWidget(&theScene, "mainCurrentTmpBackground");
		assert(mainCurrentTmpBackground);

		mainSetTmpBackground = ituSceneFindWidget(&theScene, "mainSetTmpBackground");
		assert(mainSetTmpBackground);

		mainSprite = ituSceneFindWidget(&theScene, "mainSprite");
		assert(mainSprite);

		mainSetTemperatureSprite = ituSceneFindWidget(&theScene, "mainSetTemperatureSprite");
		assert(mainSetTemperatureSprite);

		mainColdTemperatureMeter = ituSceneFindWidget(&theScene, "mainColdTemperatureMeter");
		assert(mainColdTemperatureMeter);

		mainHotTemperatureMeter = ituSceneFindWidget(&theScene, "mainHotTemperatureMeter");
		assert(mainHotTemperatureMeter);

		mainDigitalClock = ituSceneFindWidget(&theScene, "mainDigitalClock");
		assert(mainDigitalClock);

		mainTimeColonSprite = ituSceneFindWidget(&theScene, "mainTimeColonSprite");
		assert(mainTimeColonSprite);

		mainAMPMSprite = ituSceneFindWidget(&theScene, "mainAMPMSprite");
		assert(mainAMPMSprite);

		topWiFiSprite = ituSceneFindWidget(&theScene, "topWiFiSprite");
		assert(topWiFiSprite);

		for (i = 0; i < 10; i++)
		{
			sprintf(tmp, "mainBigDigitalIcon%d", i);
			mainBigDigitalIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainBigDigitalIcon[i]);

			sprintf(tmp, "mainSColdDigitalIcon%d", i);
			mainSColdDigitalIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainSColdDigitalIcon[i]);

			sprintf(tmp, "mainSHotDigitalIcon%d", i);
			mainSHotDigitalIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainSHotDigitalIcon[i]);
		}

		for (i = 0; i < 2; i++)
		{
			sprintf(tmp, "mainSetTemperatureIcon%d", i);
			mainSetTemperatureIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainSetTemperatureIcon[i]);

			sprintf(tmp, "mainTemperatureIcon%d", i);
			mainTemperatureIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainTemperatureIcon[i]);

			sprintf(tmp, "mainTimeHourIcon%d", i);
			mainTimeHourIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainTimeHourIcon[i]);

			sprintf(tmp, "mainTimeMinuteIcon%d", i);
			mainTimeMinuteIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainTimeMinuteIcon[i]);
		}

		for (i = 0; i < MAIN_TEXT_NUM; i++)
		{
			sprintf(tmp, "mainText%d", i);
			mainText[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainText[i]);
		}

		language = LANG_CHT;

		for (i = 0; i < MAIN_AIRFLOW_NUM; i++)
		{
			sprintf(tmp, "mainAirFlowBackground%d", i);
			mainAirFlowBackground[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainAirFlowBackground[i]);

			sprintf(tmp, "mainAirFlowEnBackground%d", i);
			mainAirFlowEnBackground[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainAirFlowEnBackground[i]);

			sprintf(tmp, "mainAirFlowChBackground%d", i);
			mainAirFlowChBackground[i] = ituSceneFindWidget(&theScene, tmp);
			assert(mainAirFlowChBackground[i]);
		}
	}

	if (language == LANG_ENG)
	{
		EngNameMain();
	}
	else
	{
		ChtNameMain();
	}

	current_time_hour = mainDigitalClock->hour;
	current_time_minute = mainDigitalClock->minute;
	pre_time_hour = current_time_hour;
	pre_time_minute = current_time_minute;

	//read set_temperratue >= 26, is_setting_cold = 0, else is_setting_cold = 1
	if (is_setting_cold == 1)
	{
		ituSpriteGoto(mainSetTemperatureSprite, 0);
		ituSpriteGoto(maneBackgroundSprite, 0);
		ituSpriteGoto(mainTimeColonSprite, 0);

		ituIconLinkSurface(mainTimeHourIcon[1], mainSColdDigitalIcon[current_time_hour / 10]);
		ituIconLinkSurface(mainTimeHourIcon[0], mainSColdDigitalIcon[current_time_hour % 10]);

		ituIconLinkSurface(mainTimeMinuteIcon[1], mainSColdDigitalIcon[current_time_minute / 10]);
		ituIconLinkSurface(mainTimeMinuteIcon[0], mainSColdDigitalIcon[current_time_minute % 10]);

		if (current_time_hour < 12)
		{
			ituSpriteGoto(mainAMPMSprite, 0);//AM
		}
		else
			ituSpriteGoto(mainAMPMSprite, 1);//PM
	}
	else
	{
		ituSpriteGoto(mainSetTemperatureSprite, 1);
		ituSpriteGoto(maneBackgroundSprite, 1);
		ituSpriteGoto(mainTimeColonSprite, 1);

		ituIconLinkSurface(mainTimeHourIcon[1], mainSHotDigitalIcon[current_time_hour / 10]);
		ituIconLinkSurface(mainTimeHourIcon[0], mainSHotDigitalIcon[current_time_hour % 10]);

		ituIconLinkSurface(mainTimeMinuteIcon[1], mainSHotDigitalIcon[current_time_minute / 10]);
		ituIconLinkSurface(mainTimeMinuteIcon[0], mainSHotDigitalIcon[current_time_minute % 10]);

		if (current_time_hour < 12)
		{
			ituSpriteGoto(mainAMPMSprite, 2);//AM
		}
		else
			ituSpriteGoto(mainAMPMSprite, 3);//PM
	}

	container_posX = ituWidgetGetX(mainContainer2);

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
            current_tmp = (int)(current_tmp_float + 0.5);
        }
        else
        {
            current_tmp = (int)(-current_tmp_float + 0.5);
            current_tmp = -current_tmp;
        }

        pre_tmp = current_tmp;
    #else
        #ifdef CFG_NCP18_ENABLE
            current_tmp_float = NCP18_Detect();
            if (current_tmp_float > 0)
            {
                current_tmp = (int)(current_tmp_float + 0.5);
            }
            else
            {
                current_tmp = (int)(-current_tmp_float + 0.5);
                current_tmp = -current_tmp;
            }

            pre_tmp = current_tmp;
        #else
            current_tmp = 22.9;//get from sensor
            pre_tmp = current_tmp;
        #endif
    #endif

        gtLastTick = gtTick;
    }

	ituIconLinkSurface(mainTemperatureIcon[1], mainBigDigitalIcon[current_tmp / 10]);
	ituIconLinkSurface(mainTemperatureIcon[0], mainBigDigitalIcon[current_tmp % 10]);


	if (container_posX >= 0 && container_posX < 240)//0~240
		ituSpriteGoto(mainSprite, 2);
	else if (container_posX >= 240 && container_posX < 480)
		ituSpriteGoto(mainSprite, 1);
	else if (container_posX >= 480)
		ituSpriteGoto(mainSprite, 0);

	if (theConfig.wifi_on_off == 1)
	{
#ifdef CFG_NET_WIFI_SDIO_NGPL
	    wifi_established = ioctl(ITP_DEVICE_WIFI_NGPL, ITP_IOCTL_IS_AVAIL, NULL);
#endif

	    if (wifi_established)
	        ituSpriteGoto(topWiFiSprite, 2); //0: wifi is off, 1: wifi is on but no connection, 2: connection is established
		else
            ituSpriteGoto(topWiFiSprite, 1);
	}
	else
	{
		ituSpriteGoto(topWiFiSprite, 0); //0: wifi is off, 1: wifi is on but no connection, 2: connection is established
	}

	return true;
}

bool MainOnTimer(ITUWidget* widget, char* param)
{
	bool ret = false;
	int background_posX = 0;
	uint32_t tick = 0;

	int current_posX = ituWidgetGetX(mainContainer2);
	if (current_posX != container_posX)
	{
		background_posX = -192 + current_posX / 5 * 2;
		if (background_posX<-192 || background_posX>0)
			ret = false;
		else
		{
			ituWidgetSetX(maneBackgroundSprite, background_posX);
		}

		if (current_posX < 120)
			ituSpriteGoto(mainSprite, 2);
		else if (current_posX >= 120 && current_posX < 360)
			ituSpriteGoto(mainSprite, 1);
		else if (current_posX >= 360)
			ituSpriteGoto(mainSprite, 0);

		container_posX = current_posX;
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
            current_tmp = (int)(current_tmp_float + 0.5);
        }
        else
        {
            current_tmp = (int)(-current_tmp_float + 0.5);
            current_tmp = -current_tmp;
        }
    #else
        #ifdef CFG_NCP18_ENABLE
            current_tmp_float = NCP18_Detect();
            if (current_tmp_float > 0)
            {
                current_tmp = (int)(current_tmp_float + 0.5);
            }
            else
            {
                current_tmp = (int)(-current_tmp_float + 0.5);
                current_tmp = -current_tmp;
            }
        #else
            current_tmp = 22.9;//get from sensor
        #endif
    #endif

        gtLastTick = gtTick;
    }

	if (pre_tmp != current_tmp)
	{
		ituIconLinkSurface(mainTemperatureIcon[1], mainBigDigitalIcon[current_tmp / 10]);
		ituIconLinkSurface(mainTemperatureIcon[0], mainBigDigitalIcon[current_tmp % 10]);
		pre_tmp = current_tmp;
		ret = true;
	}


	if (is_current_tmp_hide == true)
	{
		tick = SDL_GetTicks();
		if (tick - hide_start_timer >= 1000)
		{
			is_current_tmp_hide = false;
			//set temperatue to set_tmp 
			ituWidgetHide(mainSetTmpBackground, 1, 30);

			ituIconLinkSurface(mainTemperatureIcon[1], mainBigDigitalIcon[current_tmp / 10]);
			ituIconLinkSurface(mainTemperatureIcon[0], mainBigDigitalIcon[current_tmp % 10]);

			ituWidgetShow(mainCurrentTmpBackground, 1, 30);
			ret = true;
		}
	}

	current_time_hour = mainDigitalClock->hour;
	current_time_minute = mainDigitalClock->minute;
	if (current_time_hour != pre_time_hour || current_time_minute != pre_time_minute)
	{
		if (is_setting_cold == 1)
		{
			ituIconLinkSurface(mainTimeHourIcon[1], mainSColdDigitalIcon[current_time_hour / 10]);
			ituIconLinkSurface(mainTimeHourIcon[0], mainSColdDigitalIcon[current_time_hour % 10]);

			ituIconLinkSurface(mainTimeMinuteIcon[1], mainSColdDigitalIcon[current_time_minute / 10]);
			ituIconLinkSurface(mainTimeMinuteIcon[0], mainSColdDigitalIcon[current_time_minute % 10]);

			if (current_time_hour < 12)
			{
				ituSpriteGoto(mainAMPMSprite, 0);//AM
			}
			else
				ituSpriteGoto(mainAMPMSprite, 1);//PM
		}
		else
		{
			ituIconLinkSurface(mainTimeHourIcon[1], mainSHotDigitalIcon[current_time_hour / 10]);
			ituIconLinkSurface(mainTimeHourIcon[0], mainSHotDigitalIcon[current_time_hour % 10]);

			ituIconLinkSurface(mainTimeMinuteIcon[1], mainSHotDigitalIcon[current_time_minute / 10]);
			ituIconLinkSurface(mainTimeMinuteIcon[0], mainSHotDigitalIcon[current_time_minute % 10]);

			if (current_time_hour < 12)
			{
				ituSpriteGoto(mainAMPMSprite, 2);//AM
			}
			else
				ituSpriteGoto(mainAMPMSprite, 3);//PM
		}
		pre_time_hour = current_time_hour;
		pre_time_minute = current_time_minute;
		ret = true;
	}

	return ret;
}

bool MainTemperatureMeterOnChanged(ITUWidget* widget, char* param)
{
	ITUMeter* meter = (ITUMeter*)widget;

	ituWidgetHide(mainCurrentTmpBackground, 0, 0);

	set_tmp = 16 + meter->value * 16 / 100;	
	is_current_tmp_hide = true;

	if (set_tmp >= 26 && (strcmp(widget->name, "mainColdTemperatureMeter") == 0))//to hot
	{
		is_setting_cold = 0;
		ituMeterSetValue(mainHotTemperatureMeter, meter->value);
		ituSpriteGoto(mainSetTemperatureSprite, 1);
		ituSpriteGoto(maneBackgroundSprite, 1);

		ituSpriteGoto(mainTimeColonSprite, 1);
		ituIconLinkSurface(mainTimeHourIcon[1], mainSHotDigitalIcon[current_time_hour / 10]);
		ituIconLinkSurface(mainTimeHourIcon[0], mainSHotDigitalIcon[current_time_hour % 10]);

		ituIconLinkSurface(mainTimeMinuteIcon[1], mainSHotDigitalIcon[current_time_minute / 10]);
		ituIconLinkSurface(mainTimeMinuteIcon[0], mainSHotDigitalIcon[current_time_minute % 10]);
		
		if (current_time_hour < 12)
		{
			ituSpriteGoto(mainAMPMSprite, 2);//AM
		}
		else
			ituSpriteGoto(mainAMPMSprite, 3);//PM
		
	}
	else if (set_tmp <= 25 && (strcmp(widget->name, "mainHotTemperatureMeter") == 0))//to cold
	{
		is_setting_cold = 1;
		ituMeterSetValue(mainColdTemperatureMeter, meter->value);
		ituSpriteGoto(mainSetTemperatureSprite, 0);
		ituSpriteGoto(maneBackgroundSprite, 0);

		ituSpriteGoto(mainTimeColonSprite, 0);
		ituIconLinkSurface(mainTimeHourIcon[1], mainSColdDigitalIcon[current_time_hour / 10]);
		ituIconLinkSurface(mainTimeHourIcon[0], mainSColdDigitalIcon[current_time_hour % 10]);

		ituIconLinkSurface(mainTimeMinuteIcon[1], mainSColdDigitalIcon[current_time_minute / 10]);
		ituIconLinkSurface(mainTimeMinuteIcon[0], mainSColdDigitalIcon[current_time_minute % 10]);

		if (current_time_hour < 12)
		{
			ituSpriteGoto(mainAMPMSprite, 0);//AM
		}
		else
			ituSpriteGoto(mainAMPMSprite, 1);//PM
	}

	ituIconLinkSurface(mainSetTemperatureIcon[1], mainBigDigitalIcon[set_tmp / 10]);
	ituIconLinkSurface(mainSetTemperatureIcon[0], mainBigDigitalIcon[set_tmp % 10]);


	ituWidgetShow(mainSetTmpBackground, 0, 0);

	hide_start_timer = SDL_GetTicks();

	return true;
}
void ChtNameMain(void)
{
	int i;
	for (i = 0; i < MAIN_TEXT_NUM; i++)
	{
		ituTextSetString(mainText[i], StringGetMainChtName(i));
		ituTextSetFontSize(mainText[i], 29);
	}

	for (i = 0; i < MAIN_AIRFLOW_NUM; i++)
	{
		ituIconLinkSurface(&mainAirFlowBackground[i]->icon, &mainAirFlowChBackground[i]->icon);
	}
}

void EngNameMain(void)
{
	int i;
	for (i = 0; i < MAIN_TEXT_NUM; i++)
	{
		ituTextSetString(mainText[i], StringGetMainEngName(i));
		ituTextSetFontSize(mainText[i], 35);
	}

	for (i = 0; i < MAIN_AIRFLOW_NUM; i++)
	{
		ituIconLinkSurface(&mainAirFlowBackground[i]->icon, &mainAirFlowEnBackground[i]->icon);
	}
}