#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "scene.h"
#include "project.h"

static ITUAnalogClock* clockAnalogClock;
static ITUBackground* clockGrayBackground;
static ITUWheel* clockHourWheel;
static ITUWheel* clockMinuteWheel;

bool ClockOnEnter(ITUWidget* widget, char* param)
{
	if (!clockAnalogClock)
	{
		clockAnalogClock = ituSceneFindWidget(&theScene, "clockAnalogClock");
		assert(clockAnalogClock);

		clockGrayBackground = ituSceneFindWidget(&theScene, "clockGrayBackground");
		assert(clockGrayBackground);

		clockHourWheel = ituSceneFindWidget(&theScene, "clockHourWheel");
		assert(clockHourWheel);

		clockMinuteWheel = ituSceneFindWidget(&theScene, "clockMinuteWheel");
		assert(clockMinuteWheel);
	}
	ituAnalogClockUpdate((ITUWidget*)clockAnalogClock, ITU_EVENT_TIMER, 0, 0, 0);
	ituWheelGoto(clockHourWheel, clockAnalogClock->hour);
	ituWheelGoto(clockMinuteWheel, clockAnalogClock->minute);
    return true;
}

bool ClockConfirmButtonOnPress(ITUWidget* widget, char* param)
{
	struct timeval tv;
	struct tm *tm, mytime;

	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	memcpy(&mytime, tm, sizeof (struct tm));

	mytime.tm_hour = clockHourWheel->focusIndex;
	mytime.tm_min = clockMinuteWheel->focusIndex;
	mytime.tm_sec = 0;

	tv.tv_sec = mktime(&mytime);
	tv.tv_usec = 0;

	settimeofday(&tv, NULL);

    return true;
}


