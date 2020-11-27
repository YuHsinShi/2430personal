#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"

static ITUSprite* filterSprite[4] = { 0 };

bool FilterOnEnter(ITUWidget* widget, char* param)
{
	int i;
	char tmp[32];
	if (!filterSprite[0])
	{
		for (i = 0; i < 4; i++)
		{
			sprintf(tmp, "filterSprite%d", i);
			filterSprite[i] = ituSceneFindWidget(&theScene, tmp);
			assert(filterSprite[i]);
		}
	}

	ituSpriteGoto(filterSprite[0], 0);
	ituSpriteGoto(filterSprite[1], 1);
	ituSpriteGoto(filterSprite[2], 2);
	ituSpriteGoto(filterSprite[3], 1);
	return true;
}

bool FilterOkBtnOnPress(ITUWidget* widget, char* param)
{
	filter_btn_show = false;

    return true;
}

