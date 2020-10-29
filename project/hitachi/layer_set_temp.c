#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"



ITUText* setTempPointText = 0;
ITUWheel* setTempWheel = 0;

bool SetTempOnEnter(ITUWidget* widget, char* param)
{
	if (!setTempPointText)
	{
		setTempPointText = ituSceneFindWidget(&theScene, "setTempPointText");
		assert(setTempPointText);

		setTempWheel = ituSceneFindWidget(&theScene, "setTempWheel");
		assert(setTempWheel);
	}
	return true;
}

bool SetTempSetBtnOnPress(ITUWidget* widget, char* param)
{
    return true;
}

bool SetTempWheelOnChanged(ITUWidget* widget, char* param)
{
    return true;
}

