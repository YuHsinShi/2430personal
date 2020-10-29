#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"


#define INDUCTION_NUM 3

ITURadioBox* inductionRadioBox[INDUCTION_NUM] = { 0 };

bool InductionOnEnter(ITUWidget* widget, char* param)
{
	int i;
	char tmp[32];

	if (!inductionRadioBox[0])
	{
		for (i = 0; i < INDUCTION_NUM; i++)
		{
			sprintf(tmp, "inductionRadioBox%d", i);
			inductionRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(inductionRadioBox[i]);
		}
	}
	return true;
}

bool InductionRadBoxOnPress(ITUWidget* widget, char* param)
{
    return true;
}

