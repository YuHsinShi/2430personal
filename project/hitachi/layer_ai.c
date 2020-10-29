#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"

/* widgets:
aiLayer
Background257
Background278
aiDefrostRadioBox
Text280
Icon281
Background273
aiSleepRadioBox
Text276
Icon277
Background265
aiGoHomeRadioBox
Text267
Icon268
Background259
aiLeaveHomeRadioBox
Text263
Icon260
aiAutoRadioBox
aiBackgroundButton
Text108
*/
ITUCheckBox* aiAutoCheckBox = 0;
ITURadioBox* aiLeaveHomeRadioBox = 0;
ITURadioBox* aiGoHomeRadioBox = 0;
ITURadioBox* aiSleepRadioBox = 0;
ITURadioBox* aiDefrostRadioBox = 0;

extern int aiMode = 0;

bool AiOnEnter(ITUWidget* widget, char* param)
{
	if (!aiAutoCheckBox)
	{
		aiAutoCheckBox = ituSceneFindWidget(&theScene, "aiAutoCheckBox");
		assert(aiAutoCheckBox);

		aiLeaveHomeRadioBox = ituSceneFindWidget(&theScene, "aiLeaveHomeRadioBox");
		assert(aiLeaveHomeRadioBox);

		aiGoHomeRadioBox = ituSceneFindWidget(&theScene, "aiGoHomeRadioBox");
		assert(aiGoHomeRadioBox);

		aiSleepRadioBox = ituSceneFindWidget(&theScene, "aiSleepRadioBox");
		assert(aiSleepRadioBox);

		aiDefrostRadioBox = ituSceneFindWidget(&theScene, "aiDefrostRadioBox");
		assert(aiDefrostRadioBox);
	}
	return true;
}
bool AiRadioBoxOnPress(ITUWidget* widget, char* param)
{
	aiMode = atoi(param);


	ituRadioBoxSetChecked(aiLeaveHomeRadioBox, false);
	ituRadioBoxSetChecked(aiGoHomeRadioBox, false);
	ituRadioBoxSetChecked(aiSleepRadioBox, false);
	ituRadioBoxSetChecked(aiDefrostRadioBox, false);

	switch (aiMode)
	{
	case 0:
		ituRadioBoxSetChecked(aiLeaveHomeRadioBox, true);
		break;
	case 1:
		ituRadioBoxSetChecked(aiGoHomeRadioBox, true);
		break;
	case 2:
		ituRadioBoxSetChecked(aiSleepRadioBox, true);
		break;
	case 3:
		ituRadioBoxSetChecked(aiDefrostRadioBox, true);
		break;
	}


    return true;
}

bool AiAutoCheckBoxOnPress(ITUWidget* widget, char* param)
{
	if (ituCheckBoxIsChecked(aiAutoCheckBox))
	{
		aiMode = 5;
		ituWidgetDisable(aiLeaveHomeRadioBox);
		ituWidgetDisable(aiGoHomeRadioBox);
		ituWidgetDisable(aiSleepRadioBox);
		ituWidgetDisable(aiDefrostRadioBox);
		ituRadioBoxSetChecked(aiLeaveHomeRadioBox, false);
		ituRadioBoxSetChecked(aiGoHomeRadioBox, false);
		ituRadioBoxSetChecked(aiSleepRadioBox, false);
		ituRadioBoxSetChecked(aiDefrostRadioBox, false);
		mode_show[5] = 1;
	}
	else
	{
		aiMode = 0;
		ituWidgetEnable(aiLeaveHomeRadioBox);
		ituWidgetEnable(aiGoHomeRadioBox);
		ituWidgetEnable(aiSleepRadioBox);
		ituWidgetEnable(aiDefrostRadioBox);
		ituRadioBoxSetChecked(aiLeaveHomeRadioBox, true);
		ituRadioBoxSetChecked(aiGoHomeRadioBox, false);
		ituRadioBoxSetChecked(aiSleepRadioBox, false);
		ituRadioBoxSetChecked(aiDefrostRadioBox, false);
		mode_show[5] = 0;
	}
    return true;
}

