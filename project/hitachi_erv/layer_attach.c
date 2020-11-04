#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"


ITURadioBox* attachCheckBox[ATTACH_NUM] = { 0 };


bool AttachOnEnter(ITUWidget* widget, char* param)
{
	int i;
	char tmp[32];

	if (!attachCheckBox[0])
	{
		for (i = 0; i < ATTACH_NUM; i++)
		{
			sprintf(tmp, "attachCheckBox%d", i);
			attachCheckBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(attachCheckBox[i]);
		}
	}

	for (i = 0; i < ATTACH_NUM; i++)
	{
		ituCheckBoxSetChecked(attachCheckBox[i], attach_show[i]);
	}

	return true;
}

bool AttachCheckBoxOnMouseUp(ITUWidget* widget, char* param)
{
	int attachIndex = atoi(param);

	if (ituCheckBoxIsChecked(attachCheckBox[attachIndex]))
	{
		attach_show[attachIndex] = 1;
	}
	else
	{
		attach_show[attachIndex] = 0;
	}

    return true;
}

