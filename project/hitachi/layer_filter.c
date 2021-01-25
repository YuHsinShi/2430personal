#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"

ITUSprite* filterBackgroundSprite = 0;

bool FilterOnEnter(ITUWidget* widget, char* param)
{
	if (!filterBackgroundSprite)
	{
		filterBackgroundSprite = ituSceneFindWidget(&theScene, "filterBackgroundSprite");
		assert(filterBackgroundSprite);
	}
	ituSpriteGoto(filterBackgroundSprite, BgIndex[modeIndex]);
	return true;
}

bool FilterOkBtnOnPress(ITUWidget* widget, char* param)
{
	filter_btn_show = false;

    return true;
}

