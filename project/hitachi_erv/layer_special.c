#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"

/* widgets:
specialLayer
specialAirForceBackground
Background93
RadioBox336
Text102
Background104
RadioBox335
Text109
Background111
specialAirForceRadioBox0
Text272
BackgroundButton274
Text276
specialStaticPressureDoneBackground
BackgroundButton375
Text164
Icon163
specialStaticPressureOperateBackground
specialStaticPressureOperateProgressBar
Text380
Text378
specialStaticPressureManualBackground
Background390
Text392
specialStaticPressureManualText
specialStaticPressureManualMinusButton
specialStaticPressureManualPlusButton
Background387
specialStaticPressureManualEnterButton
specialStaticPressureManualCancelButton
Text386
specialStaticPressureBackground
Background242
specialStaticPressureEnterButton
specialStaticPressureCancelButton
Background368
Text374
specialStaticPressureRadioBox1
Text370
Background371
specialStaticPressureRadioBox0
Text373
Text367
specialPositivePressureManualBackground
Background481
specialPositivePressureManualEnterButton
specialPositivePressureManualCancelButton
Background476
Text477
specialPositivePressureManualText
specialPositivePressureManualMinusButton
specialPositivePressureManualPlusButton
Background485
specialPositivePressureManualRadioBox1
specialPositivePressureManualRadioBox0
Text484
specialMicroPositivePressureBackground
BackgroundButton427
Background428
specialMicroPositivePressureRadioBox2
Text435
Background436
specialMicroPositivePressureRadioBox1
Text450
Background451
specialMicroPositivePressureRadioBox0
Text453
Text454
specialPositivePressureBackground
BackgroundButton410
Background405
specialPositivePressureFanText
Text415
specialPositivePressureExhaustText
Text425
specialPositivePressureRadioBox2
Text409
Background397
specialMicroPositivePressureSprite
Text412
Text411
Text398
specialPositivePressureRadioBox1
Text400
Background401
specialPositivePressureRadioBox0
Text403
Text404
specialBackground
Background72
specialAirForceSprite
Text18
Text17
Text12
specialAirForceButton
Text76
Button95
Background68
specialStaticPressureSprite
specialStaticPressureText
Text29
specialStaticPressureButton
Text70
Button94
Background7
specialPostivePressureSprite
Text90
Text89
Text33
specialPositivePressureButton
Text67
Button93
specialBackgroundButton
Text108
*/

ITUSprite* specialPostivePressureSprite = 0;
ITUSprite* specialStaticPressureSprite = 0;
ITUText* specialStaticPressureText = 0;
ITUSprite* specialAirForceSprite = 0;

ITURadioBox* specialPositivePressureRadioBox[3] = { 0 };
ITUSprite* specialMicroPositivePressureSprite = 0;
ITUText* specialPositivePressureExhaustText = 0;
ITUText* specialPositivePressureFanText = 0;
ITURadioBox* specialMicroPositivePressureRadioBox[3] = { 0 };
ITUText* specialPositivePressureManualText = 0;

ITURadioBox* specialStaticPressureRadioBox[2] = { 0 };
ITUText* specialStaticPressureManualValueText = 0;
ITUText* specialStaticPressureManualText = 0;
ITUProgressBar* specialStaticPressureOperateProgressBar = 0;
ITUBackground* specialStaticPressureOperateBackground = 0;
ITUBackground* specialStaticPressureDoneBackground = 0;

ITURadioBox* specialAirForceRadioBox[3] = { 0 };

static int positivePressureIndex = 0;
static int staticPressureIndex = 1;
static int staticPressureValue = 80;
static int airForceIndex = 1;
static int positivePressureExhaustValue = 40;
static int positivePressureFanValue = 40;
static int microPositivePressureIndex = 1;
static int positivePressureManualIndex = 0;
static int tmpPositivePressureExhaustValue = 0;
static int tmpPositivePressureFanValue = 0;
static int tmpStaticPressureIndex = 0;
static int staticPressureManualValue = 80;
static int tmpStaticPressureManualValue = 0;
static int tmpStaticPressureManualSubValue = 0;
static bool staticPressureProgressing = false;

bool staticPressureProgress(void);
static int progress = 0;

static uint32_t lastTick = 0;

bool staticPressureProgress(void)
{
	
	
	if (progress < 100)
	{
		progress++;
		ituProgressBarSetValue(specialStaticPressureOperateProgressBar, progress);
	}
	else
	{
		staticPressureProgressing = false;

		ituWidgetSetVisible(specialStaticPressureOperateBackground, false);
		ituWidgetSetVisible(specialStaticPressureDoneBackground, true);
	}
	
	

	return true;
}
bool SpecialOnEnter(ITUWidget* widget, char* param)
{
	int i;
	char tmp[64];

	if (!specialPostivePressureSprite)
	{
		specialPostivePressureSprite = ituSceneFindWidget(&theScene, "specialPostivePressureSprite");
		assert(specialPostivePressureSprite);

		specialStaticPressureSprite = ituSceneFindWidget(&theScene, "specialStaticPressureSprite");
		assert(specialStaticPressureSprite);

		specialStaticPressureText = ituSceneFindWidget(&theScene, "specialStaticPressureText");
		assert(specialStaticPressureText);

		specialAirForceSprite = ituSceneFindWidget(&theScene, "specialAirForceSprite");
		assert(specialAirForceSprite);

		specialMicroPositivePressureSprite = ituSceneFindWidget(&theScene, "specialMicroPositivePressureSprite");
		assert(specialMicroPositivePressureSprite);

		specialPositivePressureExhaustText = ituSceneFindWidget(&theScene, "specialPositivePressureExhaustText");
		assert(specialPositivePressureExhaustText);

		specialPositivePressureFanText = ituSceneFindWidget(&theScene, "specialPositivePressureFanText");
		assert(specialPositivePressureFanText);

		specialPositivePressureManualText = ituSceneFindWidget(&theScene, "specialPositivePressureManualText");
		assert(specialPositivePressureManualText);

		specialStaticPressureManualValueText = ituSceneFindWidget(&theScene, "specialStaticPressureManualValueText");
		assert(specialStaticPressureManualValueText);

		specialStaticPressureManualText = ituSceneFindWidget(&theScene, "specialStaticPressureManualText");
		assert(specialStaticPressureManualText);

		specialStaticPressureOperateProgressBar = ituSceneFindWidget(&theScene, "specialStaticPressureOperateProgressBar");
		assert(specialStaticPressureOperateProgressBar);

		specialStaticPressureOperateBackground = ituSceneFindWidget(&theScene, "specialStaticPressureOperateBackground");
		assert(specialStaticPressureOperateBackground);

		specialStaticPressureDoneBackground = ituSceneFindWidget(&theScene, "specialStaticPressureDoneBackground");
		assert(specialStaticPressureDoneBackground);

		for (i = 0; i < 3; i++)
		{
			sprintf(tmp, "specialPositivePressureRadioBox%d", i);
			specialPositivePressureRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(specialPositivePressureRadioBox[i]);

			sprintf(tmp, "specialMicroPositivePressureRadioBox%d", i);
			specialMicroPositivePressureRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(specialMicroPositivePressureRadioBox[i]);

			sprintf(tmp, "specialAirForceRadioBox%d", i);
			specialAirForceRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(specialAirForceRadioBox[i]);

		}

		for (i = 0; i < 2; i++)
		{
			sprintf(tmp, "specialStaticPressureRadioBox%d", i);
			specialStaticPressureRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(specialStaticPressureRadioBox[i]);
		}
	}

	ituSpriteGoto(specialPostivePressureSprite, positivePressureIndex);
	ituSpriteGoto(specialStaticPressureSprite, staticPressureIndex);
	sprintf(tmp, "%dpa", staticPressureValue);
	ituTextSetString(specialStaticPressureText, tmp);
	ituSpriteGoto(specialAirForceSprite, airForceIndex);
	ituSpriteGoto(specialMicroPositivePressureSprite, microPositivePressureIndex);
	sprintf(tmp, "%d%%", positivePressureExhaustValue);
	ituTextSetString(specialPositivePressureExhaustText, tmp);
	sprintf(tmp, "%d%%", positivePressureFanValue);
	ituTextSetString(specialPositivePressureFanText, tmp);

	lastTick = SDL_GetTicks();

    return true;
}

bool SpecialOnTimer(ITUWidget* widget, char* param)
{
	bool ret = false;
	uint32_t diff, tick = SDL_GetTicks();
	static uint32_t pre_diff = 0;

	if (tick >= lastTick)
		diff = tick - lastTick;
	else
		diff = 0xFFFFFFFF - lastTick + tick;

	if (staticPressureProgressing)
	{
		if ((diff - pre_diff) > 9000) //15min*60s*1000ms/100=9000
		{
			ret = ret | staticPressureProgress();
			pre_diff = diff;
		}
	}
	

	return ret;
}

bool SpecialAirForceRadioBoxOnPress(ITUWidget* widget, char* param)
{
	airForceIndex = atoi(param);
	ituRadioBoxSetChecked(specialAirForceRadioBox[0], false);
	ituRadioBoxSetChecked(specialAirForceRadioBox[1], false);
	ituRadioBoxSetChecked(specialAirForceRadioBox[2], false);

	ituRadioBoxSetChecked(specialAirForceRadioBox[airForceIndex], true);
	ituSpriteGoto(specialAirForceSprite, airForceIndex);

    return true;
}

bool SpecialStaticPressureManualPMBtnOnPress(ITUWidget* widget, char* param)
{
	bool add = atoi(param);
	char tmp[32];

	if (add)
	{
		tmpStaticPressureManualSubValue = tmpStaticPressureManualSubValue + 10;
	}
	else
	{
		if (tmpStaticPressureManualSubValue > 0)
		{
			tmpStaticPressureManualSubValue = tmpStaticPressureManualSubValue - 10;
		}
	}

	sprintf(tmp, "%d", tmpStaticPressureManualSubValue);
	ituTextSetString(specialStaticPressureManualText, tmp);
	
    return true;
}

bool SpecialStaticPressureManualEnterBtnOnPress(ITUWidget* widget, char* param)
{
	char tmp[32];

	tmpStaticPressureManualValue = tmpStaticPressureManualSubValue;
	sprintf(tmp, "%dpa", tmpStaticPressureManualValue);
	ituTextSetString(specialStaticPressureManualValueText, tmp);

    return true;
}

bool SpecialStaticPressureEnterBtnOnPress(ITUWidget* widget, char* param)
{
	char tmp[32];

	staticPressureManualValue = tmpStaticPressureManualValue;

	sprintf(tmp, "%dpa", staticPressureManualValue);
	ituTextSetString(specialStaticPressureText, tmp);

	staticPressureProgressing = true;
	progress = 0;

    return true;
}

bool SpecialStaticPressureRadioBoxOnPress(ITUWidget* widget, char* param)
{
	char tmp[32];

	ituRadioBoxSetChecked(specialStaticPressureRadioBox[0], false);
	ituRadioBoxSetChecked(specialStaticPressureRadioBox[1], false);

	staticPressureIndex = atoi(param);
	ituRadioBoxSetChecked(specialStaticPressureRadioBox[staticPressureIndex], true);
	ituSpriteGoto(specialStaticPressureSprite, staticPressureIndex);

	if (staticPressureIndex)
	{
		tmpStaticPressureManualSubValue = tmpStaticPressureManualValue;
		sprintf(tmp, "%d", tmpStaticPressureManualSubValue);
		ituTextSetString(specialStaticPressureManualText, tmp);
	}

    return true;
}

bool SpecialPositivePressureManualEnterBtnOnPress(ITUWidget* widget, char* param)
{
	char tmp[32];
	positivePressureExhaustValue = tmpPositivePressureExhaustValue;
	positivePressureFanValue = tmpPositivePressureFanValue;

	sprintf(tmp, "%d%%", positivePressureExhaustValue);
	ituTextSetString(specialPositivePressureExhaustText, tmp);

	sprintf(tmp, "%d%%", positivePressureFanValue);
	ituTextSetString(specialPositivePressureFanText, tmp);

    return true;
}

bool SpecialPositivePressureManualPMBtnOnPress(ITUWidget* widget, char* param)
{
	bool add = atoi(param);
	char tmp[32];

	if (add)
	{
		if (positivePressureManualIndex)
		{
			if (tmpPositivePressureFanValue < 100)
				tmpPositivePressureFanValue = tmpPositivePressureFanValue + 10;

		}
		else
		{
			if (tmpPositivePressureExhaustValue < 100)
				tmpPositivePressureExhaustValue = tmpPositivePressureExhaustValue + 10;
		}
	}
	else
	{
		if (positivePressureManualIndex)
		{
			if (tmpPositivePressureFanValue > 0)
				tmpPositivePressureFanValue = tmpPositivePressureFanValue - 10;
		}
		else
		{
			if (tmpPositivePressureExhaustValue > 0)
				tmpPositivePressureExhaustValue = tmpPositivePressureExhaustValue - 10;
		}
	}

	if (positivePressureManualIndex)
	{
		sprintf(tmp, "%d", tmpPositivePressureFanValue);
	}
	else
	{
		sprintf(tmp, "%d", tmpPositivePressureExhaustValue);
	}

	ituTextSetString(specialPositivePressureManualText, tmp);

    return true;
}

bool SpecialPositivePressureManualRadioBoxOnPress(ITUWidget* widget, char* param)
{
	char tmp[32];

	positivePressureManualIndex = atoi(param);

	if (positivePressureManualIndex)
	{
		sprintf(tmp, "%d", tmpPositivePressureFanValue);
	}
	else
	{
		sprintf(tmp, "%d", tmpPositivePressureExhaustValue);
	}
	ituTextSetString(specialPositivePressureManualText, tmp);

	return true;
}

bool SpecialMicroPositivePressureRadioBoxOnPress(ITUWidget* widget, char* param)
{
	ituRadioBoxSetChecked(specialMicroPositivePressureRadioBox[0], false);
	ituRadioBoxSetChecked(specialMicroPositivePressureRadioBox[1], false);
	ituRadioBoxSetChecked(specialMicroPositivePressureRadioBox[2], false);

	microPositivePressureIndex = atoi(param);
	ituRadioBoxSetChecked(specialMicroPositivePressureRadioBox[microPositivePressureIndex], true);
	ituSpriteGoto(specialMicroPositivePressureSprite, microPositivePressureIndex);


    return true;
}

bool SpecialPositivePressureRadioBoxOnPress(ITUWidget* widget, char* param)
{
	char tmp[32];

	ituRadioBoxSetChecked(specialPositivePressureRadioBox[0], false);
	ituRadioBoxSetChecked(specialPositivePressureRadioBox[1], false);
	ituRadioBoxSetChecked(specialPositivePressureRadioBox[2], false);

	positivePressureIndex = atoi(param);
	ituRadioBoxSetChecked(specialPositivePressureRadioBox[positivePressureIndex], true);
	ituSpriteGoto(specialPostivePressureSprite, positivePressureIndex);

	if (positivePressureIndex == 2)
	{
		tmpPositivePressureExhaustValue = positivePressureExhaustValue;
		tmpPositivePressureFanValue = positivePressureFanValue;
		if (positivePressureManualIndex)
		{
			sprintf(tmp, "%d", tmpPositivePressureFanValue);
		}
		else
		{
			sprintf(tmp, "%d", tmpPositivePressureExhaustValue);
		}
		ituTextSetString(specialPositivePressureManualText, tmp);


	}

    return true;
}

bool SpecialStaticPressureBtnOnMouseUp(ITUWidget* widget, char* param)
{
	char tmp[32];

	tmpStaticPressureManualValue = staticPressureManualValue;
	sprintf(tmp, "%dpa", tmpStaticPressureManualValue);
	ituTextSetString(specialStaticPressureManualValueText, tmp);

    return true;
}

bool SpecialPositivePressureBtnOnMouseUp(ITUWidget* widget, char* param)
{
    return true;
}

