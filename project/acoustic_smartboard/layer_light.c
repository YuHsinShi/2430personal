#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"

#define MAX_ADD_COUNT 3

static ITUAnimation* lightOffAnimation;
static ITUAnimation* lightOnAnimation;
static ITUBackground* lightTmpBackground;
static ITUCoverFlow* CoverFlow92;
static ITUCheckBox* lightCheckBox0;
static ITUCheckBox* lightCheckBox1;
static ITUCheckBox* lightCheckBox00;
static ITUCheckBox* lightCheckBox01;

static ITUBackground* AddLight[MAX_ADD_COUNT];
static int add_index = 0;

int light_status = 0; //close

bool LightOnEnter(ITUWidget* widget, char* param)
{
	int i = 0;
	if (!lightOffAnimation)
	{
		lightOffAnimation = ituSceneFindWidget(&theScene, "lightOffAnimation");
		assert(lightOffAnimation);

		lightOnAnimation = ituSceneFindWidget(&theScene, "lightOnAnimation");
		assert(lightOnAnimation);

		lightTmpBackground = ituSceneFindWidget(&theScene, "lightTmpBackground");
		assert(lightTmpBackground);

		CoverFlow92 = ituSceneFindWidget(&theScene, "CoverFlow92");
		assert(CoverFlow92);

		lightCheckBox0 = ituSceneFindWidget(&theScene, "lightCheckBox0");
		assert(lightCheckBox0);

		lightCheckBox1 = ituSceneFindWidget(&theScene, "lightCheckBox1");
		assert(lightCheckBox1);

		lightCheckBox00 = ituSceneFindWidget(&theScene, "lightCheckBox00");
		assert(lightCheckBox00);

		lightCheckBox01 = ituSceneFindWidget(&theScene, "lightCheckBox01");
		assert(lightCheckBox01);

		for (i = 0; i < MAX_ADD_COUNT; i++)
			AddLight[i] = NULL;
	}
	if (ituWidgetIsVisible(lightOffAnimation) == true)
		light_status = 0;
	else
		light_status = 1;

    return true;
}

bool LightCheckBoxOnMouseUp(ITUWidget* widget, char* param)
{
	ITUCheckBox* checkBox = (ITUCheckBox*)widget;
	if (ituCheckBoxIsChecked(checkBox))//turn on light
	{
		ituWidgetSetVisible(lightOnAnimation, true);
		ituWidgetSetVisible(lightOffAnimation, false);
		ituAnimationPlay(lightOnAnimation, 0);
		light_status = 1;
	}
	else
	{
		ituWidgetSetVisible(lightOffAnimation, true);
		ituWidgetSetVisible(lightOnAnimation, false);
		ituAnimationPlay(lightOffAnimation, 0);
		light_status = 0;
	}

	if (strcmp("lightCheckBox00", widget->name) == 0)
	{
		ituCheckBoxSetChecked(lightCheckBox0, ituCheckBoxIsChecked(checkBox));
	}
	else if (strcmp("lightCheckBox01", widget->name) == 0)
	{
		ituCheckBoxSetChecked(lightCheckBox1, ituCheckBoxIsChecked(checkBox));
	}

    return true;
}

bool LightFunctionCheckBoxOnPress(ITUWidget* widget, char* param)
{
	if (light_status)
		ituAnimationPlay(lightOnAnimation, 0);
	else
		ituAnimationPlay(lightOffAnimation, 0);

    return true;
}

bool LightAddBtOnMouseUp(ITUWidget* widget, char* param)
{
	if (add_index < MAX_ADD_COUNT)
	{
		ITUBackground* clonedBg = NULL;
		char id[8];
		ITUWidget* widget = (ITUWidget*)lightTmpBackground;
		ITUCheckBox* checkbox = (ITUCheckBox*)widget->tree.child;
		ITUText* text = &checkbox->btn.text;
		sprintf(id, "%d", add_index + 3);
		ituTextSetString(text, id);

		ituBackgroundClone((ITUWidget*)lightTmpBackground, &clonedBg);
		AddLight[add_index] = clonedBg;
		ituWidgetAdd(CoverFlow92, AddLight[add_index]);
		


		ituWidgetUpdate(CoverFlow92, ITU_EVENT_LAYOUT, 0, 0, 0);// update the coverflow widget
		add_index++;
	}

	
	return true;
}

