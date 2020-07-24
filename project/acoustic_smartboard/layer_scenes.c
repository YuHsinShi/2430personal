#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"

static ITUAnimation* scenesRtAnimation[4];
static ITUIcon* scenesRtAnIcon[4][2];
static ITUText* scenesRtAnText[4][2];
static ITUAnimation* scenesLtAnimation[4];
static ITUIcon* scenesLtAnIcon[4][2];
static ITUText* scenesLtAnText[4][2];
static ITUAnimation* scenesFocusAnimation;
static ITUIcon* scenesFocusAnIcon[3];
static ITUText* scenesFocusAnText[3];

static ITUIcon* scenesIcon[4];
static ITUContainer* scenesRtContainer;
static ITUContainer* scenesLtContainer;
static ITUSprite* scenesSprite;


static ITUCoverFlow* scenesCoverFlow;

int scenes_order[4] = { 0, 1, 2, 3 };//scenes_order[pos] = scenes_id

int Scenes_index = 2;

bool ScenesOnEnter(ITUWidget* widget, char* param)
{
	int i = 0;
	int j = 0;

	if (!scenesCoverFlow)
	{
		char tmp[32];
		
		scenesCoverFlow = ituSceneFindWidget(&theScene, "scenesCoverFlow");
		assert(scenesCoverFlow);

		scenesRtContainer = ituSceneFindWidget(&theScene, "scenesRtContainer");
		assert(scenesRtContainer);

		scenesLtContainer = ituSceneFindWidget(&theScene, "scenesLtContainer");
		assert(scenesLtContainer);

		scenesSprite = ituSceneFindWidget(&theScene, "scenesSprite");
		assert(scenesSprite);

		scenesFocusAnimation = ituSceneFindWidget(&theScene, "scenesFocusAnimation");
		assert(scenesFocusAnimation);

		for (i = 0; i < 4; i++)
		{
			sprintf(tmp, "scenesRtAnimation%d", i);
			scenesRtAnimation[i] = ituSceneFindWidget(&theScene, tmp);
			assert(scenesRtAnimation[i]);

			sprintf(tmp, "scenesLtAnimation%d", i);
			scenesLtAnimation[i] = ituSceneFindWidget(&theScene, tmp);
			assert(scenesLtAnimation[i]);

			sprintf(tmp, "scenesIcon%d", i);
			scenesIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(scenesIcon[i]);

			for (j = 0; j < 2; j++)
			{
				sprintf(tmp, "scenesRtAnIcon%d%d", i,j);
				scenesRtAnIcon[i][j] = ituSceneFindWidget(&theScene, tmp);
				assert(scenesRtAnIcon[i][j]);

				sprintf(tmp, "scenesRtAnText%d%d", i, j);
				scenesRtAnText[i][j] = ituSceneFindWidget(&theScene, tmp);
				assert(scenesRtAnText[i][j]);

				sprintf(tmp, "scenesLtAnIcon%d%d", i, j);
				scenesLtAnIcon[i][j] = ituSceneFindWidget(&theScene, tmp);
				assert(scenesLtAnIcon[i][j]);

				sprintf(tmp, "scenesLtAnText%d%d", i, j);
				scenesLtAnText[i][j] = ituSceneFindWidget(&theScene, tmp);
				assert(scenesLtAnText[i][j]);
			}
		}

		for (i = 0; i < 3; i++)
		{
			sprintf(tmp, "scenesFocusAnIcon%d", i);
			scenesFocusAnIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(scenesFocusAnIcon[i]);

			sprintf(tmp, "scenesFocusAnText%d", i);
			scenesFocusAnText[i] = ituSceneFindWidget(&theScene, tmp);
			assert(scenesFocusAnText[i]);
		}
	}

	Scenes_index = theConfig.scene;
	ituCoverFlowGoto(scenesCoverFlow, Scenes_index);
	ituSpriteGoto(scenesSprite, Scenes_index);

	for (i = 0; i < 4; i++)
	{
		int idx = Scenes_index  + (i - 2);
		if (idx < 0)
			idx = idx + 4;
		if (idx > 3)
			idx = 0;

		scenes_order[i] = idx;
		for (j = 0; j < 2; j++)
		{
			ituIconLinkSurface(scenesRtAnIcon[i][j], scenesIcon[idx]);
			ituIconLinkSurface(scenesLtAnIcon[i][j], scenesIcon[idx]);
			ituTextSetString(scenesLtAnText[i][j], StringGetScenesName(scenes_order[i]));
			ituTextSetString(scenesRtAnText[i][j], StringGetScenesName(scenes_order[i]));
		}	
	}
	printf("%d %d %d %d\n", scenes_order[0], scenes_order[1], scenes_order[2], scenes_order[3]);

	return true;
}

bool ScenesOnLeave(ITUWidget* widget, char* param)
{
	ConfigSave();
	return true;
}

bool PlayAnFunction(ITUWidget* widget, char* param)
{
	int tt = atoi(param);
	int i = 0;
	int j = 0;
	if (tt == 0)//to right: A0 A3 A2 A1
	{
		ituWidgetSetVisible(scenesLtContainer, false);
		
		Scenes_index--;
		if (Scenes_index < 0)
			Scenes_index = 3;

		for (i = 0; i < 4; i++)
		{
			int idx = Scenes_index + (i - 2);
			if (idx < 0)
				idx = idx + 4;
			if (idx > 3)
				idx = 0;

			scenes_order[i] = idx;		
		}

		ituWidgetSetVisible(scenesRtContainer, true);
		ituWidgetToBottom(scenesRtAnimation[1]);
		for (i = 0; i < 4; i++)
		{
			ituAnimationPlay(scenesRtAnimation[i], 0);
		}
	}
	else//to left: A0 A3 A1 A2
	{
		ituWidgetSetVisible(scenesRtContainer, false);
		Scenes_index++;
		if (Scenes_index > 3)
			Scenes_index = 0;

		for (i = 0; i < 4; i++)
		{
			int idx = Scenes_index + (i - 2);
			if (idx < 0)
				idx = idx + 4;
			if (idx > 3)
				idx = 0;

			scenes_order[i] = idx;
		}

		ituWidgetSetVisible(scenesLtContainer, true);
		ituWidgetToBottom(scenesLtAnimation[3]);
		for (i = 0; i < 4; i++)
		{
			ituAnimationPlay(scenesLtAnimation[i], 0);
		}
	}
	ituSpriteGoto(scenesSprite, Scenes_index);
    theConfig.scene = Scenes_index;
	printf("%d %d %d %d\n", scenes_order[0], scenes_order[1], scenes_order[2], scenes_order[3]);
    return true;
}

bool ScenesRtAn2OnStopped(ITUWidget* widget, char* param)
{
	int i, j;

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 2; j++)
		{
			ituIconLinkSurface(scenesRtAnIcon[i][j], scenesIcon[scenes_order[i]]);
			ituIconLinkSurface(scenesLtAnIcon[i][j], scenesIcon[scenes_order[i]]);
			ituTextSetString(scenesLtAnText[i][j], StringGetScenesName(scenes_order[i]));
			ituTextSetString(scenesRtAnText[i][j], StringGetScenesName(scenes_order[i]));
		}

		ituAnimationGoto(scenesRtAnimation[i], 0);
		ituAnimationGoto(scenesLtAnimation[i], 0);
	}
	ituWidgetToBottom(scenesRtAnimation[2]);
	ituWidgetToBottom(scenesLtAnimation[2]);

	for (i = 0; i < 3; i++)
	{
		ituIconLinkSurface(scenesFocusAnIcon[i], scenesIcon[scenes_order[2]]);
		ituTextSetString(scenesFocusAnText[i], StringGetScenesName(scenes_order[2]));
	}

	ituWidgetSetVisible(scenesRtAnimation[2], false);
	ituWidgetSetVisible(scenesLtAnimation[2], false);
	ituWidgetSetVisible(scenesFocusAnimation, true);
	ituAnimationPlay(scenesFocusAnimation, 0);

	return true;
}