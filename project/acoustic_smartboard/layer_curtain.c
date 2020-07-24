#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"

static ITUSprite* curtainOpenSprite;
static ITUSprite* curtainCloseSprite;

static int curtain_status = 0; //0: close, 1:open

bool CurtainOnEnter(ITUWidget* widget, char* param)
{
	if (!curtainOpenSprite)
	{
		curtainOpenSprite = ituSceneFindWidget(&theScene, "curtainOpenSprite");
		assert(curtainOpenSprite);

		curtainCloseSprite = ituSceneFindWidget(&theScene, "curtainCloseSprite");
		assert(curtainCloseSprite);
	}

	if (curtain_status == 0)
	{
		ituWidgetSetVisible(curtainOpenSprite, false);
		ituWidgetSetVisible(curtainCloseSprite, true);
		ituSpriteGoto(curtainOpenSprite, 11 - curtainCloseSprite->frame);
	}
	else
	{
		ituWidgetSetVisible(curtainOpenSprite, true);
		ituWidgetSetVisible(curtainCloseSprite, false);
		ituSpriteGoto(curtainCloseSprite, 11 - curtainOpenSprite->frame);
	}

    return true;
}

bool CurtainCloseBtOnMousUp(ITUWidget* widget, char* param)
{
	ituSpriteStop(curtainOpenSprite);

	ituWidgetSetVisible(curtainOpenSprite, false);
	ituWidgetSetVisible(curtainCloseSprite, true);

	curtain_status = 0;

	ituSpritePlay(curtainCloseSprite, 11 - curtainOpenSprite->frame);

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
	ituSpriteStop(curtainCloseSprite);

	ituWidgetSetVisible(curtainOpenSprite, true);
	ituWidgetSetVisible(curtainCloseSprite, false);

	curtain_status = 1;

	ituSpritePlay(curtainOpenSprite, 11 - curtainCloseSprite->frame);

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

void ControlCurtain(int movement)//0: close, 1 : open, 2 : stop
{
	if (!curtainOpenSprite)
	{
		curtainOpenSprite = ituSceneFindWidget(&theScene, "curtainOpenSprite");
		assert(curtainOpenSprite);

		curtainCloseSprite = ituSceneFindWidget(&theScene, "curtainCloseSprite");
		assert(curtainCloseSprite);

		if (curtain_status == 0)
		{
			ituWidgetSetVisible(curtainOpenSprite, false);
			ituWidgetSetVisible(curtainCloseSprite, true);
			ituSpriteGoto(curtainOpenSprite, 11 - curtainCloseSprite->frame);
		}
		else
		{
			ituWidgetSetVisible(curtainOpenSprite, true);
			ituWidgetSetVisible(curtainCloseSprite, false);
			ituSpriteGoto(curtainCloseSprite, 11 - curtainOpenSprite->frame);
		}
	}

	switch (movement)
	{
	case 0:
		printf("Close the curtain\n");
		ituSpriteStop(curtainOpenSprite);
		ituWidgetSetVisible(curtainOpenSprite, false);
		ituWidgetSetVisible(curtainCloseSprite, true);
		curtain_status = 0;
		ituSpritePlay(curtainCloseSprite, 11 - curtainOpenSprite->frame);
		break;

	case 1:
		printf("Open the curtain\n");
		ituSpriteStop(curtainCloseSprite);
		ituWidgetSetVisible(curtainOpenSprite, true);
		ituWidgetSetVisible(curtainCloseSprite, false);
		curtain_status = 1;
		ituSpritePlay(curtainOpenSprite, 11 - curtainCloseSprite->frame);
		break;

	case 2:
		printf("Stop the curtain\n");
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
		break;

	default:
		break;

	}
}
