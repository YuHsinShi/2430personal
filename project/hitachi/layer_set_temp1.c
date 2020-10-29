#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"




ITUWheel* setTemp1Wheel = 0;

bool SetTemp1OnEnter(ITUWidget* widget, char* param)
{
	if (!setTemp1Wheel)
	{
		setTemp1Wheel = ituSceneFindWidget(&theScene, "setTemp1Wheel");
		assert(setTemp1Wheel);
	}
	return true;
}

bool SetTemp1BackgroundBtnOnPress(ITUWidget* widget, char* param)
{
    return true;
}

bool SetTemp1WheelOnChanged(ITUWidget* widget, char* param)
{
    return true;
}

