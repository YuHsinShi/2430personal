#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"
/* widgets:
airFlowLayer
Background8
Container253
airFlowButtonIcon1
airFlowButtonIcon0
airFlowVBackground
airFlowVTrackBar
airFlowVValueBackground
airFlowVValueText
airFlowVButton
Icon252
airFlowHBackground
airFlowHTrackBar
airFlowHValueBackground
airFlowHValueText
airFlowHButton
Icon251
Text85
airFlowAutoCheckBox
Background144
RadioBox145
RadioBox142
Text99
*/
ITUIcon*	airFlowButtonIcon[2] = { 0 };
ITUBackground*	airFlowHValueBackground = 0;
ITUBackground*	airFlowVValueBackground = 0;
ITUBackground*	airFlowHTipShowBackground = 0;
ITUBackground*	airFlowVTipShowBackground = 0;
ITUText*	airFlowHTipShowText = 0;
ITUText*	airFlowVTipShowText = 0;
ITUCheckBox*	airFlowAutoCheckBox = 0;
ITUTrackBar*	airFlowHTrackBar = 0;
ITUTrackBar*	airFlowHTrackBar1 = 0;
ITUSprite*	airFlowHSprite = 0;
ITUTrackBar*	airFlowVTrackBar = 0;
ITUTrackBar*	airFlowVTrackBar1 = 0;
ITUSprite*	airFlowVSprite = 0;
ITURadioBox* airFlowRadioBox[2] = { 0 };
ITUBackground* airFlowRadioBoxBackground[2] = { 0 };
ITUIcon* airFlowRadioBoxBgIcon[2] = { 0 };
static int hAirFlow = 1;
static int vAirFlow = 1;

bool AirFlowOnEnter(ITUWidget* widget, char* param)
{
	int i,x,y;
	char tmp[32];
	if (!airFlowHValueBackground)
	{
		airFlowHValueBackground = ituSceneFindWidget(&theScene, "airFlowHValueBackground");
		assert(airFlowHValueBackground);

		airFlowVValueBackground = ituSceneFindWidget(&theScene, "airFlowVValueBackground");
		assert(airFlowVValueBackground);

		airFlowAutoCheckBox = ituSceneFindWidget(&theScene, "airFlowAutoCheckBox");
		assert(airFlowAutoCheckBox);

		airFlowHTrackBar = ituSceneFindWidget(&theScene, "airFlowHTrackBar");
		assert(airFlowHTrackBar);

		airFlowHTrackBar1 = ituSceneFindWidget(&theScene, "airFlowHTrackBar1");
		assert(airFlowHTrackBar1);

		airFlowHSprite = ituSceneFindWidget(&theScene, "airFlowHSprite");
		assert(airFlowHSprite);

		airFlowVTrackBar = ituSceneFindWidget(&theScene, "airFlowVTrackBar");
		assert(airFlowVTrackBar);
		
		airFlowVTrackBar1 = ituSceneFindWidget(&theScene, "airFlowVTrackBar1");
		assert(airFlowVTrackBar1);

		airFlowVSprite = ituSceneFindWidget(&theScene, "airFlowVSprite");
		assert(airFlowVSprite);
		
		airFlowHTipShowBackground = ituSceneFindWidget(&theScene, "airFlowHTipShowBackground");
		assert(airFlowHTipShowBackground);

		airFlowVTipShowBackground = ituSceneFindWidget(&theScene, "airFlowVTipShowBackground");
		assert(airFlowVTipShowBackground);

		airFlowHTipShowText = ituSceneFindWidget(&theScene, "airFlowHTipShowText");
		assert(airFlowHTipShowText);

		airFlowVTipShowText = ituSceneFindWidget(&theScene, "airFlowVTipShowText");
		assert(airFlowVTipShowText);

		for (i = 0; i < 2; i++)
		{
			sprintf(tmp, "airFlowButtonIcon%d", i);
			airFlowButtonIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(airFlowButtonIcon[i]);

			sprintf(tmp, "airFlowRadioBox%d", i);
			airFlowRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(airFlowRadioBox[i]);

			sprintf(tmp, "airFlowRadioBoxBackground%d", i);
			airFlowRadioBoxBackground[i] = ituSceneFindWidget(&theScene, tmp);
			assert(airFlowRadioBoxBackground[i]);

			sprintf(tmp, "airFlowRadioBoxBgIcon%d", i);
			airFlowRadioBoxBgIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(airFlowRadioBoxBgIcon[i]);

		}
	}


	ituWidgetGetGlobalPosition(airFlowHTrackBar->tip, &x, &y);
	ituWidgetSetPosition(airFlowHTipShowBackground, x-28, y-505);

	ituWidgetGetGlobalPosition(airFlowVTrackBar->tip, &x, &y);
	ituWidgetSetPosition(airFlowVTipShowBackground, x - 628, y - 227);

	
    return true;
}

bool AirFlowAutoChkBoxOnPress(ITUWidget* widget, char* param)
{
	int x, y;
	char tmp[32];

	if (ituCheckBoxIsChecked(airFlowAutoCheckBox))
	{
		ituWidgetDisable(airFlowHTrackBar);
		ituWidgetDisable(airFlowVTrackBar);
		ituWidgetDisable(airFlowHTrackBar1);
		ituWidgetDisable(airFlowVTrackBar1);
		ituWidgetSetVisible(airFlowHTrackBar->tracker, false);
		ituWidgetSetVisible(airFlowVTrackBar->tracker, false);

		ituIconLinkSurface(&airFlowHTipShowBackground->icon, airFlowButtonIcon[1]);
		ituIconLinkSurface(&airFlowVTipShowBackground->icon, airFlowButtonIcon[1]);
		ituTextSetString(airFlowHTipShowText, "A");
		ituTextSetString(airFlowVTipShowText, "A");

		ituTrackBarSetValue(airFlowHTrackBar, 4);
		ituTrackBarSetValue(airFlowVTrackBar, 4);
		ituTrackBarSetValue(airFlowHTrackBar1, 4);
		ituTrackBarSetValue(airFlowVTrackBar1, 4);
		ituSpriteGoto(airFlowHSprite, 4);
		ituSpriteGoto(airFlowVSprite, 4);
		ituWidgetGetGlobalPosition(airFlowHTrackBar->tip, &x, &y);
		ituWidgetSetPosition(airFlowHTipShowBackground, x - 28, y - 505);
		ituWidgetGetGlobalPosition(airFlowVTrackBar->tip, &x, &y);
		ituWidgetSetPosition(airFlowVTipShowBackground, x - 628, y - 227);

	}
	else
	{
		ituWidgetEnable(airFlowHTrackBar);
		ituWidgetEnable(airFlowVTrackBar);
		ituWidgetEnable(airFlowHTrackBar1);
		ituWidgetEnable(airFlowVTrackBar1);
		ituWidgetSetVisible(airFlowHTrackBar->tracker, true);
		ituWidgetSetVisible(airFlowVTrackBar->tracker, true);

		ituIconLinkSurface(&airFlowHTipShowBackground->icon, airFlowButtonIcon[0]);
		ituIconLinkSurface(&airFlowVTipShowBackground->icon, airFlowButtonIcon[0]);
		sprintf(tmp, "%d", hAirFlow);
		ituTextSetString(airFlowHTipShowText, tmp);
		sprintf(tmp, "%d", vAirFlow);
		ituTextSetString(airFlowVTipShowText, tmp);


		ituTrackBarSetValue(airFlowHTrackBar, hAirFlow);
		ituTrackBarSetValue(airFlowVTrackBar, vAirFlow);
		ituTrackBarSetValue(airFlowHTrackBar1, hAirFlow);
		ituTrackBarSetValue(airFlowVTrackBar1, vAirFlow);
		ituSpriteGoto(airFlowHSprite, hAirFlow);
		ituSpriteGoto(airFlowVSprite, vAirFlow);

		ituWidgetGetGlobalPosition(airFlowHTrackBar->tip, &x, &y);
		ituWidgetSetPosition(airFlowHTipShowBackground, x - 28, y - 505);
		ituWidgetGetGlobalPosition(airFlowVTrackBar->tip, &x, &y);
		ituWidgetSetPosition(airFlowVTipShowBackground, x - 628, y - 227);
		
		

	}
    return true;
}
bool AirFlowHTrackBarOnChanged(ITUWidget* widget, char* param)
{
	int x, y;
	char tmp[32];
	
	ituWidgetSetVisible(airFlowHTipShowBackground, false);
	ituWidgetGetGlobalPosition(airFlowHTrackBar->tip, &x, &y);
	ituWidgetSetPosition(airFlowHTipShowBackground, x - 28, y - 505);
	sprintf(tmp, "%d", airFlowHTrackBar->value);
	ituTextSetString(airFlowHTipShowText, tmp);
	ituWidgetSetVisible(airFlowHTipShowBackground, true);

	hAirFlow = airFlowHTrackBar->value;

	return true;
}

bool AirFlowVTrackBarOnChanged(ITUWidget* widget, char* param)
{
	int x, y;
	char tmp[32];

	ituWidgetSetVisible(airFlowVTipShowBackground, false);
	ituWidgetGetGlobalPosition(airFlowVTrackBar->tip, &x, &y);
	ituWidgetSetPosition(airFlowVTipShowBackground, x - 628, y - 227);
	sprintf(tmp, "%d", airFlowVTrackBar->value);
	ituTextSetString(airFlowVTipShowText, tmp);
	ituWidgetSetVisible(airFlowVTipShowBackground, true);

	vAirFlow = airFlowVTrackBar->value;

	return true;
}

bool AirFlowRadioBoxOnPress(ITUWidget* widget, char* param)
{
	airFlow3D = atoi(param);

	if (airFlow3D)
	{
		ituRadioBoxSetChecked(airFlowRadioBox[0], true);
		ituRadioBoxSetChecked(airFlowRadioBox[1], false);
		ituIconLinkSurface(&airFlowRadioBoxBackground[0]->icon, airFlowRadioBoxBgIcon[0]);
		ituIconLinkSurface(&airFlowRadioBoxBackground[1]->icon, airFlowRadioBoxBgIcon[1]);
	}
	else
	{
		ituRadioBoxSetChecked(airFlowRadioBox[0], false);
		ituRadioBoxSetChecked(airFlowRadioBox[1], true);
		ituIconLinkSurface(&airFlowRadioBoxBackground[0]->icon, airFlowRadioBoxBgIcon[1]);
		ituIconLinkSurface(&airFlowRadioBoxBackground[1]->icon, airFlowRadioBoxBgIcon[0]);
	}

	return true;
}
