#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"

#ifdef CFG_SHT20_ENABLE
    #include "sht20_sensor.h"
#endif

#ifdef CFG_NCP18_ENABLE
    #include "ncp18_sensor.h"
#endif

static ITUBackground* screenSaverClockBackground;
static ITUText* screenSaverTemperatureText;
static ITUText* screenSaverTemperaturePointText;
static double current_tmp;

#if defined(CFG_SHT20_ENABLE) || defined(CFG_NCP18_ENABLE)
    static float current_tmp_float = 0;
#endif

static int current_tmp_point;
static double pre_tmp;
static uint32_t tick = 0, lasttick = 0, refresh_time = 3000;
static bool tick_first = true;

bool ScreensaverOnEnter(ITUWidget* widget, char* param)
{
	if (!screenSaverClockBackground)
	{
		screenSaverClockBackground = ituSceneFindWidget(&theScene, "screenSaverClockBackground");
		assert(screenSaverClockBackground);

		screenSaverTemperatureText = ituSceneFindWidget(&theScene, "screenSaverTemperatureText");
		assert(screenSaverTemperatureText);

		screenSaverTemperaturePointText = ituSceneFindWidget(&theScene, "screenSaverTemperaturePointText");
		assert(screenSaverTemperaturePointText);
	}

    ituWidgetSetVisible(screenSaverClockBackground, false);

	if(theConfig.screensaver_type == SCREENSAVER_CLOCK)
	{
        if (tick_first)
        {
            tick = refresh_time;
            tick_first = false;
        }
        else
        {
            tick = SDL_GetTicks();
        }

        if (tick - lasttick >= refresh_time)
        {
        #ifdef CFG_SHT20_ENABLE
            current_tmp_float = SHT20_Detect();
            if (current_tmp_float > 0)
            {
                current_tmp = (double)((int)(current_tmp_float * 10 + 0.5) / 10.0);
            }
            else
            {
                current_tmp = (double)((int)(-current_tmp_float * 10 + 0.5) / 10.0);
                current_tmp = -current_tmp;
            }

            pre_tmp = current_tmp;
        #else
            #ifdef CFG_NCP18_ENABLE
                current_tmp_float = NCP18_Detect();
                if (current_tmp_float > 0)
                {
                    current_tmp = (double)((int)(current_tmp_float * 10 + 0.5) / 10.0);
                }
                else
                {
                    current_tmp = (double)((int)(-current_tmp_float * 10 + 0.5) / 10.0);
                    current_tmp = -current_tmp;
                }

                pre_tmp = current_tmp;
            #else
                current_tmp = 22.9;//get from sensor
                pre_tmp = current_tmp;
            #endif
        #endif

            lasttick = tick;
        }

		current_tmp_point = (int)(current_tmp * 10.0) - (int)current_tmp * 10;

		char temperature[8];
		char temperature_point[8];
		sprintf(temperature, "%d", (int)current_tmp);
		ituTextSetString(screenSaverTemperatureText, temperature);
		sprintf(temperature_point, ".%d", current_tmp_point);
		ituTextSetString(screenSaverTemperaturePointText, temperature_point);
		ituWidgetSetVisible(screenSaverClockBackground, true);
	}
    return true;
}

bool ScreenSaverOnTimer(ITUWidget* widget, char* param)
{
	int ret = false;

    if (tick_first)
    {
        tick = refresh_time;
        tick_first = false;
    }
    else
    {
        tick = SDL_GetTicks();
    }

    if (tick - lasttick >= refresh_time)
    {
    #ifdef CFG_SHT20_ENABLE
        current_tmp_float = SHT20_Detect();
        if (current_tmp_float > 0)
        {
            current_tmp = (double)((int)(current_tmp_float * 10 + 0.5) / 10.0);
        }
        else
        {
            current_tmp = (double)((int)(-current_tmp_float * 10 + 0.5) / 10.0);
            current_tmp = -current_tmp;
        }
    #else
        #ifdef CFG_NCP18_ENABLE
            current_tmp_float = NCP18_Detect();
            if (current_tmp_float > 0)
            {
                current_tmp = (double)((int)(current_tmp_float * 10 + 0.5) / 10.0);
            }
            else
            {
                current_tmp = (double)((int)(-current_tmp_float * 10 + 0.5) / 10.0);
                current_tmp = -current_tmp;
            }
        #else
            current_tmp = 22.9;//get from sensor
        #endif
    #endif

        lasttick = tick;
    }

	if (pre_tmp != current_tmp)
	{
		current_tmp_point = (int)(current_tmp * 10.0) - (int)current_tmp * 10;

		char temperature[8];
		char temperature_point[8];
		sprintf(temperature, "%d", (int)current_tmp);
		ituTextSetString(screenSaverTemperatureText, temperature);
		sprintf(temperature_point, ".%d", current_tmp_point);
		ituTextSetString(screenSaverTemperaturePointText, temperature_point);
		pre_tmp = current_tmp;
		
		ret = true;
	}

	return ret;
}

