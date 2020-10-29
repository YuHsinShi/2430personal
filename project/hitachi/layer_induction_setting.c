#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"



ITUTrackBar*	inductionSettingTrackBar = 0;
ITUProgressBar*	inductionSettingProgressBar = 0;
ITURadioBox*	inductionSettingActRadioBox[3] = { 0 };
ITURadioBox* 	inductionSettingRadioBox[2] = { 0 };

bool InductionSettingOnEnter(ITUWidget* widget, char* param)
{
	int i;
	char tmp[32];

	if (!inductionSettingTrackBar)
	{
		inductionSettingTrackBar = ituSceneFindWidget(&theScene, "inductionSettingTrackBar");
		assert(inductionSettingTrackBar);

		inductionSettingProgressBar = ituSceneFindWidget(&theScene, "inductionSettingProgressBar");
		assert(inductionSettingProgressBar);
		
		for (i = 0; i < 3; i++)
		{
			sprintf(tmp, "inductionSettingActRadioBox%d", i);
			inductionSettingActRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(inductionSettingActRadioBox[i]);
		}

		for (i = 0; i < 2; i++)
		{
			sprintf(tmp, "inductionSettingRadioBox%d", i);
			inductionSettingRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(inductionSettingRadioBox[i]);
		}
			
			

	}
	return true;
}

bool InductionSettingTrackBarOnChanged(ITUWidget* widget, char* param)
{
    return true;
}

bool InductionSettingActRadBoxOnPress(ITUWidget* widget, char* param)
{
    return true;
}

bool InductionSettingRadBoxOnPress(ITUWidget* widget, char* param)
{
	inductionSetting_checked = atoi(param);

    return true;
}

