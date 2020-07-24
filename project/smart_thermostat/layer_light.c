#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"

#define MAX_ADD_COUNT 3
#define LIGHT_TEXT_NUM 9
#define LIGHT_GROUP_NUM 6

static ITUAnimation* lightOffAnimation;
static ITUAnimation* lightOnAnimation;
static ITUBackground* lightTmpBackground;
static ITUCoverFlow* lightCoverFlow;
static ITUCheckBox* lightCheckBox0;
static ITUCheckBox* lightCheckBox1;
static ITUCheckBox* lightCleanCheckBox;
static ITUCheckBox* lightDisturbCheckBox;
static ITUCheckBox* lightSOSCheckBox;
static ITUCheckBox* LightAddButton;
static ITUButton* lightBackButton;
static ITUText* lightText[LIGHT_TEXT_NUM] = { 0 };
static ITUCheckBox* lightGroupCheckBox[LIGHT_GROUP_NUM] = { 0 };

static int light_group_map[5][LIGHT_GROUP_NUM] = { 0 };
static light_index = 0;

static ITUBackground* AddLight[MAX_ADD_COUNT];
static int add_index = 0;

int light_status = 0; //close
void ChtNameLight(void);
void EngNameLight(void);
bool IslightCheckBoxLongPressed = false;

bool LightOnEnter(ITUWidget* widget, char* param)
{
	char tmp[32];
	int i = 0, j = 0;
	if (!lightOffAnimation)
	{
		lightOffAnimation = ituSceneFindWidget(&theScene, "lightOffAnimation");
		assert(lightOffAnimation);

		lightOnAnimation = ituSceneFindWidget(&theScene, "lightOnAnimation");
		assert(lightOnAnimation);

		lightTmpBackground = ituSceneFindWidget(&theScene, "lightTmpBackground");
		assert(lightTmpBackground);

		lightCoverFlow = ituSceneFindWidget(&theScene, "lightCoverFlow");
		assert(lightCoverFlow);

		lightCheckBox0 = ituSceneFindWidget(&theScene, "lightCheckBox0");
		assert(lightCheckBox0);

		lightCheckBox1 = ituSceneFindWidget(&theScene, "lightCheckBox1");
		assert(lightCheckBox1);

		lightCleanCheckBox = ituSceneFindWidget(&theScene, "lightCleanCheckBox");
		assert(lightCleanCheckBox);

		lightDisturbCheckBox = ituSceneFindWidget(&theScene, "lightDisturbCheckBox");
		assert(lightDisturbCheckBox);

		lightSOSCheckBox = ituSceneFindWidget(&theScene, "lightSOSCheckBox");
		assert(lightSOSCheckBox);

		LightAddButton = ituSceneFindWidget(&theScene, "LightAddButton");
		assert(LightAddButton);

		lightBackButton = ituSceneFindWidget(&theScene, "lightBackButton");
		assert(lightBackButton);

		for (i = 0; i < MAX_ADD_COUNT; i++)
			AddLight[i] = NULL;

		for (i = 0; i < LIGHT_TEXT_NUM; i++)
		{
			sprintf(tmp, "lightText%d", i);
			lightText[i] = ituSceneFindWidget(&theScene, tmp);
			assert(lightText[i]);
		}

		for (i = 0; i < LIGHT_GROUP_NUM; i++)
		{
			sprintf(tmp, "lightGroupCheckBox%d", i);
			lightGroupCheckBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(lightGroupCheckBox[i]);
		}

		for (i = 0; i < 5; i++)
		{
			for (j = 0; j < LIGHT_GROUP_NUM; j++)
				light_group_map[i][j] = 0;
		}


	}
	if (language == LANG_ENG)
	{
		EngNameLight();
	}
	else
	{
		ChtNameLight();
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
	if (!IslightCheckBoxLongPressed)
	{
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
	}
	else
	{
		if (ituCheckBoxIsChecked(checkBox))
			ituCheckBoxSetChecked(checkBox, false);
		else
			ituCheckBoxSetChecked(checkBox, true);
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
		ituWidgetAdd(lightCoverFlow, AddLight[add_index]);
		
		

		ituWidgetUpdate(lightCoverFlow, ITU_EVENT_LAYOUT, 0, 0, 0);// update the coverflow widget
		add_index++;
	}

	
	return true;
}
bool lightOnLeave(ITUWidget* widget, char* param)
{
	IslightCheckBoxLongPressed = false;

	ituWidgetEnable(lightCoverFlow);

	ituWidgetSetVisible(lightBackButton, false);

	return true;
}
bool lightCheckBoxOnMouseLongPress(ITUWidget* widget, char* param)
{
	int i = 0;
	ITUCheckBox* checkbox = (ITUCheckBox*)widget;
	ITUText* text = &checkbox->btn.text;
	char *string = ituTextGetString(text);
	

	IslightCheckBoxLongPressed = true;
	light_index = atoi(string) - 1;

	ituWidgetDisable(lightCoverFlow);
	ituUnPressWidget(widget);

	for (i = 0; i < LIGHT_GROUP_NUM; i++)
	{
		ituCheckBoxSetChecked(lightGroupCheckBox[i], light_group_map[light_index][i]);
	}

	ituWidgetSetVisible(lightBackButton, true);

	return true;
}

bool lightBackButtonOnPress(ITUWidget* widget, char* param)
{
	IslightCheckBoxLongPressed = false;

	ituWidgetEnable(lightCoverFlow);

	ituWidgetSetVisible(lightBackButton, false);

	return true;
}

bool lightGroupCheckBoxOnPress(ITUWidget* widget, char* param)
{
	ITUCheckBox* checkbox = (ITUCheckBox*)widget;
	int i = light_index, j = atoi(param);
	if (!ituCheckBoxIsChecked(checkbox))
		light_group_map[i][j] = 1;
	else
		light_group_map[i][j] = 0;

	return true;
}

void ChtNameLight(void)
{
	int i;
	for (i = 0; i < LIGHT_TEXT_NUM; i++)
	{
		ituTextSetString(lightText[i], StringGetLightChtName(i));
		if (i == 8)
			ituTextSetFontSize(lightText[i], 35);
		else
			ituTextSetFontSize(lightText[i], 40);
	}
}

void EngNameLight(void)
{
	int i;
	for (i = 0; i < LIGHT_TEXT_NUM; i++)
	{
		ituTextSetString(lightText[i], StringGetLightEngName(i));
		if (i <= 5)
			ituTextSetFontSize(lightText[i], 30);
		else if (i == 6)
			ituTextSetFontSize(lightText[i], 25);
		else if (i == 7)
			ituTextSetFontSize(lightText[i], 20);
	}
}