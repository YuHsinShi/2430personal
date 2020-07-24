#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"

static ITUAnimation* checkAnimOKAnimation;
static ITUTrackBar* checkAnimSpeedTrackBar;
static ITUProgressBar* checkAnimSpeedProgressBar;
int is_checkBox_pressed = 0;

bool CheckAnimSpeedTrackBarOnChanged(ITUWidget* widget, char* param)
{
    int value = atoi(param);

	if (is_checkBox_pressed)
	{
		if (value == 0)
		{
			checkAnimOKAnimation->playing = false;
		}
		else
		{
			checkAnimOKAnimation->playing = true;
			checkAnimOKAnimation->delay = 100 / value;
		}
	}
   
    return true;
}

bool CheckAnimScaleCheckBoxOnPress(ITUWidget* widget, char* param)
{
    ITUCheckBox* checkbox = (ITUCheckBox*)widget;

	if (ituCheckBoxIsChecked(checkbox))
	{
		checkAnimOKAnimation->animationFlags |= ITU_ANIM_SCALE;
		is_checkBox_pressed++;		
	}     
	else
	{
		checkAnimOKAnimation->animationFlags &= ~ITU_ANIM_SCALE;
		is_checkBox_pressed--;
	}
      
	if (checkAnimOKAnimation->animationFlags & (ITU_ANIM_SCALE | ITU_ANIM_ROTATE | ITU_ANIM_COLOR | ITU_ANIM_MOVE) && checkAnimSpeedTrackBar->value > 0)
	{
		checkAnimOKAnimation->playing = true;
		checkAnimOKAnimation->delay = 100 / (checkAnimSpeedTrackBar->value);
	}
    else
        checkAnimOKAnimation->playing = false;

    return true;
}

bool CheckAnimRotateCheckBox(ITUWidget* widget, char* param)
{
    ITUCheckBox* checkbox = (ITUCheckBox*)widget;

	if (ituCheckBoxIsChecked(checkbox))
	{
		checkAnimOKAnimation->animationFlags |= ITU_ANIM_ROTATE;
		is_checkBox_pressed++;
	}
	else
	{
		checkAnimOKAnimation->animationFlags &= ~ITU_ANIM_ROTATE;
		is_checkBox_pressed--;
	}

	if (checkAnimOKAnimation->animationFlags & (ITU_ANIM_SCALE | ITU_ANIM_ROTATE | ITU_ANIM_COLOR | ITU_ANIM_MOVE) && checkAnimSpeedTrackBar->value > 0)
	{
		checkAnimOKAnimation->playing = true;
		checkAnimOKAnimation->delay = 100 / (checkAnimSpeedTrackBar->value);
	}
	else
        checkAnimOKAnimation->playing = false;

    return true;
}

bool CheckAnimAlphablendCheckBox(ITUWidget* widget, char* param)
{
    ITUCheckBox* checkbox = (ITUCheckBox*)widget;

	if (ituCheckBoxIsChecked(checkbox))
	{
		checkAnimOKAnimation->animationFlags |= ITU_ANIM_COLOR;
		is_checkBox_pressed++;
	}
	else
	{
		checkAnimOKAnimation->animationFlags &= ~ITU_ANIM_COLOR;
		is_checkBox_pressed--;
	}

	if (checkAnimOKAnimation->animationFlags & (ITU_ANIM_SCALE | ITU_ANIM_ROTATE | ITU_ANIM_COLOR | ITU_ANIM_MOVE) && checkAnimSpeedTrackBar->value > 0)
	{
		checkAnimOKAnimation->playing = true;
		checkAnimOKAnimation->delay = 100 / (checkAnimSpeedTrackBar->value);
	}
	else
        checkAnimOKAnimation->playing = false;

    return true;
}

bool CheckAnimMoveCheckBox(ITUWidget* widget, char* param)
{
    ITUCheckBox* checkbox = (ITUCheckBox*)widget;

	if (ituCheckBoxIsChecked(checkbox))
	{
		checkAnimOKAnimation->animationFlags |= ITU_ANIM_MOVE;
		is_checkBox_pressed++;
	}
	else
	{
		checkAnimOKAnimation->animationFlags &= ~ITU_ANIM_MOVE;
		is_checkBox_pressed--;
	}

	if (checkAnimOKAnimation->animationFlags & (ITU_ANIM_SCALE | ITU_ANIM_ROTATE | ITU_ANIM_COLOR | ITU_ANIM_MOVE) && checkAnimSpeedTrackBar->value > 0)
	{
		checkAnimOKAnimation->playing = true;
		checkAnimOKAnimation->delay = 100 / (checkAnimSpeedTrackBar->value);
	}
	else
        checkAnimOKAnimation->playing = false;

    return true;
}

bool CheckAnimOnEnter(ITUWidget* widget, char* param)
{
    if (!checkAnimOKAnimation)
    {
        checkAnimOKAnimation = ituSceneFindWidget(&theScene, "checkAnimOKAnimation");
        assert(checkAnimOKAnimation);

        checkAnimSpeedTrackBar = ituSceneFindWidget(&theScene, "checkAnimSpeedTrackBar");
        assert(checkAnimSpeedTrackBar);

        checkAnimSpeedProgressBar = ituSceneFindWidget(&theScene, "checkAnimSpeedProgressBar");
        assert(checkAnimSpeedProgressBar);
    }
    checkAnimOKAnimation->animationFlags |= ITU_ANIM_SCALE | ITU_ANIM_ROTATE | ITU_ANIM_COLOR | ITU_ANIM_MOVE;
    ituAnimationReset(checkAnimOKAnimation);
    checkAnimOKAnimation->animationFlags &= ~(ITU_ANIM_SCALE | ITU_ANIM_ROTATE | ITU_ANIM_COLOR | ITU_ANIM_MOVE);
    ituTrackBarSetValue(checkAnimSpeedTrackBar, 0);
    ituProgressBarSetValue(checkAnimSpeedProgressBar, 0);
	is_checkBox_pressed = 0;

    return true;
}

void CheckAnimReset(void)
{
    checkAnimOKAnimation = NULL;
}
