#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"



ITUWheel* setHumidityWheel = 0;

bool SetHumidityOnEnter(ITUWidget* widget, char* param)
{
	if (!setHumidityWheel)
	{
		setHumidityWheel = ituSceneFindWidget(&theScene, "setHumidityWheel");
		assert(setHumidityWheel);
	}
	ituWheelGoto(setHumidityWheel, 100 - humidityValueIndex);
	return true;
}

bool SetHumiditySetBtnOnPress(ITUWidget* widget, char* param)
{
    return true;
}

bool SetHumidityWheelOnChanged(ITUWidget* widget, char* param)
{
	humidityValueIndex = 100 - setHumidityWheel->focusIndex;


    return true;
}

