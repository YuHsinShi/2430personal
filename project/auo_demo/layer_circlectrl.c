#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "SDL/SDL.h"
#include "scene.h"
#include "ctrlboard.h"

#define UNIT_MS     33
#define MIN_SPEED   1
#define MAX_SPEED   10

static ITUMeter* circleCtrlYellowMeter;
static ITUMeter* circleCtrlBlueMeter;
static ITUCircleProgressBar* circleCtrlYellowCircleProgressBar;
static ITUCircleProgressBar* circleCtrlBlueCircleProgressBar;
static ITUTrackBar* circleCtrlTrackBar;
static ITUProgressBar* circleCtrlProgressBar;
static ITUText *circleCtrlYellowPercentText;
static ITUText *circleCtrlBluePercentText;
static ITUText *circleCtrValueText;

static bool circleCtrlPlaying;
static int circleCtrlSpeed;
static int circleCtrlStep;

bool CircleCtrlOnTimer(ITUWidget* widget, char* param)
{
	bool updated = false;
    if (circleCtrlPlaying)
    {
        static uint32_t lastTick = 0;
        uint32_t tick = SDL_GetTicks();
		char c_value[8];

        if (tick - lastTick >= UNIT_MS)
        {
            circleCtrlStep++;
            lastTick = tick;
        }

        if (circleCtrlStep >= MAX_SPEED)
        {
            int value = circleCtrlYellowCircleProgressBar->value >= 100 ? 0 : circleCtrlYellowCircleProgressBar->value + 1;
            ituCircleProgressBarSetValue(circleCtrlYellowCircleProgressBar, value);
            ituCircleProgressBarSetValue(circleCtrlBlueCircleProgressBar, value);
            ituProgressBarSetValue(circleCtrlProgressBar, value);
            circleCtrlStep = circleCtrlSpeed;
			sprintf(c_value, "%d%%", value);
			ituTextSetString(circleCtrlYellowPercentText, c_value);
			ituTextSetString(circleCtrlBluePercentText, c_value);
			ituTextSetString(circleCtrValueText, c_value);
			updated = true;
        }
    }
	return updated;
}

bool CircleCtrlFastButtonOnPress(ITUWidget* widget, char* param)
{
    if (circleCtrlSpeed < MAX_SPEED)
        circleCtrlSpeed++;

    return true;
}

bool CircleCtrlSlowButtonOnPress(ITUWidget* widget, char* param)
{
    if (circleCtrlSpeed > MIN_SPEED)
        circleCtrlSpeed--;

    return true;
}

bool CircleCtrlStartButtonOnPress(ITUWidget* widget, char* param)
{
    ituWidgetDisable(circleCtrlYellowMeter);
    ituWidgetDisable(circleCtrlBlueMeter);
    ituWidgetDisable(circleCtrlTrackBar);
    circleCtrlPlaying = true;
    return true;
}

bool CircleCtrlStopButtonOnPress(ITUWidget* widget, char* param)
{
    ituWidgetEnable(circleCtrlYellowMeter);
    ituWidgetEnable(circleCtrlBlueMeter);
    ituWidgetEnable(circleCtrlTrackBar);
    circleCtrlPlaying = false;
    return true;
}

bool CircleCtrlOnEnter(ITUWidget* widget, char* param)
{
    if (!circleCtrlYellowCircleProgressBar)
    {
        circleCtrlYellowMeter = ituSceneFindWidget(&theScene, "circleCtrlYellowMeter");
        assert(circleCtrlYellowMeter);
        
        circleCtrlBlueMeter = ituSceneFindWidget(&theScene, "circleCtrlBlueMeter");
        assert(circleCtrlBlueMeter);

        circleCtrlYellowCircleProgressBar = ituSceneFindWidget(&theScene, "circleCtrlYellowCircleProgressBar");
        assert(circleCtrlYellowCircleProgressBar);
        
        circleCtrlBlueCircleProgressBar = ituSceneFindWidget(&theScene, "circleCtrlBlueCircleProgressBar");
        assert(circleCtrlBlueCircleProgressBar);

        circleCtrlTrackBar = ituSceneFindWidget(&theScene, "circleCtrlTrackBar");
        assert(circleCtrlTrackBar);                        

        circleCtrlProgressBar = ituSceneFindWidget(&theScene, "circleCtrlProgressBar");
		assert(circleCtrlProgressBar);

		circleCtrlYellowPercentText = ituSceneFindWidget(&theScene, "circleCtrlYellowPercentText");
		assert(circleCtrlYellowPercentText);

		circleCtrlBluePercentText = ituSceneFindWidget(&theScene, "circleCtrlBluePercentText");
		assert(circleCtrlBluePercentText);

		circleCtrValueText = ituSceneFindWidget(&theScene, "circleCtrValueText");
		assert(circleCtrValueText);
    }
    ituWidgetEnable(circleCtrlYellowMeter);
    ituWidgetEnable(circleCtrlBlueMeter);
    ituWidgetEnable(circleCtrlTrackBar);
    ituCircleProgressBarSetValue(circleCtrlYellowCircleProgressBar, 0);
    ituCircleProgressBarSetValue(circleCtrlBlueCircleProgressBar, 0);
    ituProgressBarSetValue(circleCtrlProgressBar, 0);

	ituTextSetString(circleCtrlYellowPercentText, "0%");
	ituTextSetString(circleCtrlBluePercentText, "0%");
	ituTextSetString(circleCtrValueText, "0%");

    circleCtrlPlaying = false;
    circleCtrlSpeed = MIN_SPEED;
    circleCtrlStep = 0;

    return true;
}

void CircleCtrlReset(void)
{
    circleCtrlYellowCircleProgressBar = NULL;
}

bool CircleCtrlMeterTrackBarOnChange(ITUWidget* widget, char* param)
{
	int value = atoi(param);
	char c_value[8];
	ituCircleProgressBarSetValue(circleCtrlYellowCircleProgressBar, value);
	ituCircleProgressBarSetValue(circleCtrlBlueCircleProgressBar, value);
	ituProgressBarSetValue(circleCtrlProgressBar, value);
	strcpy(c_value, param);
	strcat(c_value, "%");
	ituTextSetString(circleCtrlYellowPercentText, c_value);
	ituTextSetString(circleCtrlBluePercentText, c_value);
	ituTextSetString(circleCtrValueText, c_value);
	return true;
}