#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"

static ITUSprite* curtainOpenSprite;
static ITUSprite* curtainCloseSprite;

int curtain_status = 0; //0: close, 1:open

bool CurtainOnEnter(ITUWidget* widget, char* param)
{
	if (!curtainOpenSprite)
	{
		curtainOpenSprite = ituSceneFindWidget(&theScene, "curtainOpenSprite");
		assert(curtainOpenSprite);

		curtainCloseSprite = ituSceneFindWidget(&theScene, "curtainCloseSprite");
		assert(curtainCloseSprite);
	}
	curtain_status = 0;
	ituWidgetSetVisible(curtainOpenSprite, false);
	ituWidgetSetVisible(curtainCloseSprite, true);
	ituSpriteGoto(curtainOpenSprite, 11 - curtainCloseSprite->frame);



    return true;
}

bool CurtainCloseBtOnMousUp(ITUWidget* widget, char* param)
{
	ituWidgetSetVisible(curtainOpenSprite, false);
	ituWidgetSetVisible(curtainCloseSprite, true);

	curtain_status = 0;

	ituSpritePlay(curtainCloseSprite, -1);

    return true;
}

bool CurtainStopBtOnMousUp(ITUWidget* widget, char* param)
{
	if (curtain_status == 0)//close
	{
		ituSpriteStop(curtainCloseSprite);
		ituSpriteGoto(curtainOpenSprite, 11 - curtainCloseSprite->frame);
	}
	else if (curtain_status == 1)//open
	{
		ituSpriteStop(curtainOpenSprite);
		ituSpriteGoto(curtainCloseSprite, 11 - curtainOpenSprite->frame);
	}
    return true;
}

bool CurtainOpenBtOnMousUp(ITUWidget* widget, char* param)
{
	ituWidgetSetVisible(curtainOpenSprite, true);
	ituWidgetSetVisible(curtainCloseSprite, false);

	curtain_status = 1;

	ituSpritePlay(curtainOpenSprite, -1);
    return true;
}

bool CurtainCloseSpriteOnStopped(ITUWidget* widget, char* param)
{
	ituSpriteGoto(curtainOpenSprite, 11 - curtainCloseSprite->frame);
	return true;
}

bool CurtainOpenSpriteOnStopped(ITUWidget* widget, char* param)
{
	ituSpriteGoto(curtainCloseSprite, 11 - curtainOpenSprite->frame);
	return true;
}

