#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"



ITUText* moreModeText[MOREMODE_NUM] = { 0 };
ITURadioBox* moreModeRadioBox[MOREMODE_NUM] = { 0 };


bool MoreModeOnEnter(ITUWidget* widget, char* param)
{
	int i;
	char tmp[32];

	if (!moreModeText[0])
	{
		for (i = 0; i < MOREMODE_NUM; i++)
		{
			sprintf(tmp, "moreModeText%d", i);
			moreModeText[i] = ituSceneFindWidget(&theScene, tmp);
			assert(moreModeText[i]);

			sprintf(tmp, "moreModeRadioBox%d", i);
			moreModeRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(moreModeRadioBox[i]);
		}
	}

	//if (modeIndex < 4)
	//{
	//	for (i = 0; i < MOREMODE_NUM; i++)
	//	{
	//		ituRadioBoxSetChecked(moreModeRadioBox[i], false);
	//	}
	//}
	return true;
}

bool MoreModeRadioBoxOnMouseUp(ITUWidget* widget, char* param)
{
	modeIndex = atoi(param);

    return true;
}

