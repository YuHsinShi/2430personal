#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"



ITUText* trialRunTrialNumText = 0;
ITURadioBox* trialRunTrialAirForceRadioBox[3] = { 0 };
ITUSprite* trialRunTrialAirForceTextSprite = 0;
ITUWheel* trialRunTrialTimeWheel = 0;
ITUText* trialRunTrialTimeText = 0;
ITUWheel* trialRunTrialFreqWheel = 0;
ITUText* trialRunTrialFreqText = 0;
ITUSprite* trialRunModelSelectValueTitleSprite = 0;
ITUText* trialRunModelSelectValueTitleText = 0;
ITURadioBox* trialRunModelSelectRadioBox[16] = { 0 };
ITURadioBox* trialRunModelSelectItemRadioBox[7][2] = { 0 };
ITUSprite* trialRunInputOutputValueTitleSprite = 0;
ITUText* trialRunInputOutputValueTitleText = 0;
ITURadioBox* trialRunInputOutputRadioBox[16] = { 0 };
ITURadioBox* trialRunInputOutputItemRadioBox[5][2] = { 0 };
ITURadioBox* trialRunAddrUpdateRadioBox[16] = { 0 };
ITUText* trialRunAddrUpdateSText = 0;
ITUText* trialRunAddrUpdateAText = 0;
ITURadioBox* trialRunAddrUpdateSRadioBox = 0;
ITURadioBox* trialRunAddrUpdateARadioBox = 0;
ITUSprite* trialRunAddrInitTitleSprite = 0;
ITUText* trialRunAddrInitTitleText = 0;
ITURadioBox* trialRunAddrInitRadioBox[16] = { 0 };
ITUSprite* trialRunModelSelInitTitleSprite = 0;
ITUText* trialRunModelSelInitTitleText = 0;
ITURadioBox* trialRunModelSelInitRadioBox[16] = { 0 };
ITURadioBox* trialRunAddrCheckRadioBox[16] = { 0 };

static int trialAirForce = 0;
static int trialTime = 43;
static int trialFreq = 70;
static int tmpAirForce = 0;
static int tmpTime = 43;
static int tmpFreq = 70;

static int modelselIndex = 0;
static int modelselItemIndex = 0;
static int modeselItemValue[17][7] = { { 0, 1, 0, 0, 2, 1, 0 }, { 2, 1, 0, 0, 1, 1, 0 } };

static int IOIndex = 0;
static int IOItemIndex = 0;
static int IOItemValue[17][5] = { { 0, 1, 0, 0, 2}, { 2, 1, 0, 0, 1 } };

extern int systemNo[16] = { 0 };
extern int addr[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
static int tmpAddr[16] = { 0 };
static int tmpSystem[16] = { 0 };
static int tmpAddrText = 1;
static int tmpSystemText = 0;
static bool addrChange = true;
static int addrupdateItem = 0;
static int addrInitIndex = 0;
static int modelSelInitIndex = 0;
bool TrialRunOnEnter(ITUWidget* widget, char* param)
{
	int i,j;
	char tmp[64];
	if (!trialRunTrialNumText)
	{
		trialRunTrialNumText = ituSceneFindWidget(&theScene, "trialRunTrialNumText");
		assert(trialRunTrialNumText);
		
		trialRunTrialAirForceTextSprite = ituSceneFindWidget(&theScene, "trialRunTrialAirForceTextSprite");
		assert(trialRunTrialAirForceTextSprite);

		trialRunTrialTimeWheel = ituSceneFindWidget(&theScene, "trialRunTrialTimeWheel");
		assert(trialRunTrialTimeWheel);

		trialRunTrialTimeText = ituSceneFindWidget(&theScene, "trialRunTrialTimeText");
		assert(trialRunTrialTimeText);

		trialRunTrialFreqWheel = ituSceneFindWidget(&theScene, "trialRunTrialFreqWheel");
		assert(trialRunTrialFreqWheel);

		trialRunTrialFreqText = ituSceneFindWidget(&theScene, "trialRunTrialFreqText");
		assert(trialRunTrialFreqText);

		trialRunModelSelectValueTitleSprite = ituSceneFindWidget(&theScene, "trialRunModelSelectValueTitleSprite");
		assert(trialRunModelSelectValueTitleSprite);

		trialRunModelSelectValueTitleText = ituSceneFindWidget(&theScene, "trialRunModelSelectValueTitleText");
		assert(trialRunModelSelectValueTitleText);

		trialRunInputOutputValueTitleSprite = ituSceneFindWidget(&theScene, "trialRunInputOutputValueTitleSprite");
		assert(trialRunInputOutputValueTitleSprite);

		trialRunInputOutputValueTitleText = ituSceneFindWidget(&theScene, "trialRunInputOutputValueTitleText");
		assert(trialRunInputOutputValueTitleText);

		trialRunAddrUpdateSText = ituSceneFindWidget(&theScene, "trialRunAddrUpdateSText");
		assert(trialRunInputOutputValueTitleText);

		trialRunAddrUpdateAText = ituSceneFindWidget(&theScene, "trialRunAddrUpdateAText");
		assert(trialRunInputOutputValueTitleText);

		trialRunAddrUpdateSRadioBox = ituSceneFindWidget(&theScene, "trialRunAddrUpdateSRadioBox");
		assert(trialRunInputOutputValueTitleText);

		trialRunAddrUpdateARadioBox = ituSceneFindWidget(&theScene, "trialRunAddrUpdateARadioBox");
		assert(trialRunInputOutputValueTitleText);

		trialRunAddrInitTitleSprite = ituSceneFindWidget(&theScene, "trialRunAddrInitTitleSprite");
		assert(trialRunAddrInitTitleSprite);

		trialRunAddrInitTitleText = ituSceneFindWidget(&theScene, "trialRunAddrInitTitleText");
		assert(trialRunAddrInitTitleText);

		trialRunModelSelInitTitleSprite = ituSceneFindWidget(&theScene, "trialRunModelSelInitTitleSprite");
		assert(trialRunModelSelInitTitleSprite);

		trialRunModelSelInitTitleText = ituSceneFindWidget(&theScene, "trialRunModelSelInitTitleText");
		assert(trialRunModelSelInitTitleText);

		for (i = 0; i < 3; i++)
		{
			sprintf(tmp, "trialRunTrialAirForceRadioBox%d", i);
			trialRunTrialAirForceRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(trialRunTrialAirForceRadioBox[i]);

		}

		for (i = 0; i < 7; i++)
		{
			for (j = 0; j < 2; j++)
			{
				sprintf(tmp, "trialRunModelSelectItemRadioBox%d%d", i, j);
				trialRunModelSelectItemRadioBox[i][j] = ituSceneFindWidget(&theScene, tmp);
				assert(trialRunModelSelectItemRadioBox[i][j]);
			}
		}

		for (i = 0; i < 5; i++)
		{
			for (j = 0; j < 2; j++)
			{
				sprintf(tmp, "trialRunInputOutputItemRadioBox%d%d", i, j);
				trialRunInputOutputItemRadioBox[i][j] = ituSceneFindWidget(&theScene, tmp);
				assert(trialRunInputOutputItemRadioBox[i][j]);
			}
		}

		for (i = 0; i < 16; i++)
		{
			sprintf(tmp, "trialRunAddrUpdateRadioBox%d", i);
			trialRunAddrUpdateRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(trialRunAddrUpdateRadioBox[i]);

			sprintf(tmp, "trialRunModelSelectRadioBox%d", i);
			trialRunModelSelectRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(trialRunModelSelectRadioBox[i]);

			sprintf(tmp, "trialRunInputOutputRadioBox%d", i);
			trialRunInputOutputRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(trialRunInputOutputRadioBox[i]);

			sprintf(tmp, "trialRunAddrInitRadioBox%d", i);
			trialRunAddrInitRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(trialRunAddrInitRadioBox[i]);	

			sprintf(tmp, "trialRunModelSelInitRadioBox%d", i);
			trialRunModelSelInitRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(trialRunModelSelInitRadioBox[i]);
			
			sprintf(tmp, "trialRunAddrCheckRadioBox%d", i);
			trialRunAddrCheckRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(trialRunAddrCheckRadioBox[i]);


			

		}

	}

	ituSpriteGoto(trialRunTrialAirForceTextSprite, 0);

	for (i = 0; i < 16; i++)
	{
		tmpAddr[i] = addr[i];
		tmpSystem[i] = systemNo[i];
	}

	ituWidgetSetColor(trialRunAddrUpdateAText, 255, 255, 255, 255);
	ituWidgetSetColor(trialRunAddrUpdateSText, 255, 0, 0, 0);

    return true;
}

bool TrialRunTrialSaveBtnOnPress(ITUWidget* widget, char* param)
{
	bool save = atoi(param);
	char tmp[32];
	int i;

	

	if (save)
	{
		trialAirForce = tmpAirForce;
		trialTime = tmpTime;
		trialFreq = tmpFreq;
	}
	else
	{
		for (i = 0; i < 3; i++)
		{
			ituRadioBoxSetChecked(trialRunTrialAirForceRadioBox[i], false);
		}
		ituRadioBoxSetChecked(trialRunTrialAirForceRadioBox[trialAirForce], true);
		ituSpriteGoto(trialRunTrialAirForceTextSprite, trialAirForce);

		trialRunTrialTimeWheel->focusIndex = trialTime;
		sprintf(tmp, "%2.1f", 23.5 - trialTime*0.5);
		ituTextSetString(trialRunTrialTimeText, tmp);

		trialRunTrialFreqWheel->focusIndex = trialFreq;
		sprintf(tmp, "%dHz", 100 - trialFreq);
		ituTextSetString(trialRunTrialFreqText, tmp);

	}

	

	return true;
}

bool TrialRunTrialAirForceEnterBtnOnPress(ITUWidget* widget, char* param)
{
	if (ituRadioBoxIsChecked(trialRunTrialAirForceRadioBox[0]))
	{
		ituSpriteGoto(trialRunTrialAirForceTextSprite, 0);
		tmpAirForce = 0;
	}
	else if (ituRadioBoxIsChecked(trialRunTrialAirForceRadioBox[1]))
	{
		ituSpriteGoto(trialRunTrialAirForceTextSprite, 1);
		tmpAirForce = 1;
	}
	else
	{
		ituSpriteGoto(trialRunTrialAirForceTextSprite, 2);
		tmpAirForce = 2;
	}

    return true;
}

bool TrialRunTrialAirForceRadBoxOnPress(ITUWidget* widget, char* param)
{
	int index = atoi(param);
	int i;

	for (i = 0; i < 3; i++)
	{
		ituRadioBoxSetChecked(trialRunTrialAirForceRadioBox[i], false);
	}

	ituRadioBoxSetChecked(trialRunTrialAirForceRadioBox[index], true);

    return true;
}

bool TrialRuntTrialTimeSaveBackgroundBtnOnPress(ITUWidget* widget, char* param)
{
	char tmp[32];
	sprintf(tmp, "%2.1f", 23.5 - trialRunTrialTimeWheel->focusIndex*0.5);

	ituTextSetString(trialRunTrialTimeText, tmp);
	tmpTime = trialRunTrialTimeWheel->focusIndex;

	return true;
}

bool TrialRuntTrialFreqSaveBackgroundBtnOnPress(ITUWidget* widget, char* param)
{
	char tmp[32];
	sprintf(tmp, "%dHz", 100 - trialRunTrialFreqWheel->focusIndex);

	ituTextSetString(trialRunTrialFreqText, tmp);
	tmpFreq = trialRunTrialFreqWheel->focusIndex;

	return true;
}

bool TrialRunModelSelectRadioBoxOnPress(ITUWidget* widget, char* param)
{

	modelselIndex = atoi(param);

	return true;
}
bool TrialRunModelSelectEnterBtnOnPress(ITUWidget* widget, char* param)
{
	char tmp[32];
	int i;
	
	if (modelselIndex == 16)
	{
		ituSpriteGoto(trialRunModelSelectValueTitleSprite, 1);
	}
	else
	{
		sprintf(tmp, "%s", ituTextGetString(&trialRunModelSelectRadioBox[modelselIndex]->checkbox.btn.text));
		ituTextSetString(trialRunModelSelectValueTitleText, tmp);
		ituSpriteGoto(trialRunModelSelectValueTitleSprite, 0);
		
	}

	for (i = 0; i < 7; i++)
	{
		sprintf(tmp, "%02d", modeselItemValue[modelselIndex][i]);
		ituTextSetString(&trialRunModelSelectItemRadioBox[i][1]->checkbox.btn.text, tmp);
	}
    return true;
}

bool TrialRunModelSelectItemRadBoxOnPress(ITUWidget* widget, char* param)
{
	modelselItemIndex = atoi(param);

	ituRadioBoxSetChecked(trialRunModelSelectItemRadioBox[modelselItemIndex][0], true);
	ituRadioBoxSetChecked(trialRunModelSelectItemRadioBox[modelselItemIndex][1], true);

	return true;
}

bool TrialRunModelSelectPMBtnOnPress(ITUWidget* widget, char* param)
{
	bool add = atoi(param);
	char tmp[32];

	if (add)
	{
		modeselItemValue[modelselIndex][modelselItemIndex]++;
	}
	else
	{
		if (modeselItemValue[modelselIndex][modelselItemIndex] > 0)
		{
			modeselItemValue[modelselIndex][modelselItemIndex]--;
		}
	}

	sprintf(tmp, "%02d", modeselItemValue[modelselIndex][modelselItemIndex]);
	ituTextSetString(&trialRunModelSelectItemRadioBox[modelselItemIndex][1]->checkbox.btn.text, tmp);

	return true;
}
bool TrialRunInputOutputRadioBoxOnPress(ITUWidget* widget, char* param)
{

	IOIndex = atoi(param);

	return true;
}
bool TrialRunInputOutputEnterBtnOnPress(ITUWidget* widget, char* param)
{
	char tmp[32];
	int i;

	if (IOIndex == 16)
	{
		ituSpriteGoto(trialRunInputOutputValueTitleSprite, 1);
	}
	else
	{
		sprintf(tmp, "%s", ituTextGetString(&trialRunInputOutputRadioBox[IOIndex]->checkbox.btn.text));
		ituTextSetString(trialRunInputOutputValueTitleText, tmp);
		ituSpriteGoto(trialRunInputOutputValueTitleSprite, 0);

	}

	for (i = 0; i < 5; i++)
	{
		sprintf(tmp, "%02d", IOItemValue[IOIndex][i]);
		ituTextSetString(&trialRunInputOutputItemRadioBox[i][1]->checkbox.btn.text, tmp);
	}
	return true;
}

bool TrialRunInputOutputItemRadBoxOnPress(ITUWidget* widget, char* param)
{
	IOItemIndex = atoi(param);

	ituRadioBoxSetChecked(trialRunInputOutputItemRadioBox[IOItemIndex][0], true);
	ituRadioBoxSetChecked(trialRunInputOutputItemRadioBox[IOItemIndex][1], true);

	return true;
}

bool TrialRunInputOutputPMBtnOnPress(ITUWidget* widget, char* param)
{
	bool add = atoi(param);
	char tmp[32];

	if (add)
	{
		IOItemValue[IOIndex][IOItemIndex]++;
	}
	else
	{
		if (IOItemValue[IOIndex][IOItemIndex] > 0)
		{
			IOItemValue[IOIndex][IOItemIndex]--;
		}
	}

	sprintf(tmp, "%02d", IOItemValue[IOIndex][IOItemIndex]);
	ituTextSetString(&trialRunInputOutputItemRadioBox[IOItemIndex][1]->checkbox.btn.text, tmp);



	return true;
}
bool TrialRunAddrUpdateSaveBtnOnPress(ITUWidget* widget, char* param)
{
	int i;
	bool save = atoi(param);
	char tmp[32];
	if (save)
	{
		for (i = 0; i < 16; i++)
		{
			addr[i] = tmpAddr[i];
			systemNo[i] = tmpSystem[i];
			sprintf(tmp, "%02d-%02d", systemNo[i], addr[i]);
			ituTextSetString(&trialRunModelSelectRadioBox[i]->checkbox.btn.text, tmp);
			ituTextSetString(&trialRunInputOutputRadioBox[i]->checkbox.btn.text, tmp);
			ituTextSetString(&trialRunAddrInitRadioBox[i]->checkbox.btn.text, tmp);
			ituTextSetString(&trialRunModelSelInitRadioBox[i]->checkbox.btn.text, tmp);
			ituTextSetString(&trialRunAddrCheckRadioBox[i]->checkbox.btn.text, tmp);

		}
	}
	else
	{
		for (i = 0; i < 16; i++)
		{
			tmpAddr[i] = addr[i];
			tmpSystem[i] = systemNo[i];
		}
	}

	return true;
}
bool TrialRunAddrUpdateSARadioBoxOnPress(ITUWidget* widget, char* param)
{
	addrChange = atoi(param);
	if (addrChange)
	{
		ituWidgetSetColor(trialRunAddrUpdateSText, 255, 0, 0, 0);
		ituWidgetSetColor(trialRunAddrUpdateAText, 255, 255, 255, 255);
	}
	else
	{
		ituWidgetSetColor(trialRunAddrUpdateAText, 255, 0, 0, 0);
		ituWidgetSetColor(trialRunAddrUpdateSText, 255, 255, 255, 255);
	}

	return true;
}
bool TrialRunAddrUpdatePMBtnOnPress(ITUWidget* widget, char* param)
{
	int add = atoi(param);
	char tmp[32];


	if (addrChange)
	{
		if (add)
		{
			tmpAddrText ++;
		}
		else
		{
			if (tmpAddrText  > 1)
			{
				tmpAddrText--;
			}
		}

		sprintf(tmp, "%02d", tmpAddrText);
		ituTextSetString(&trialRunAddrUpdateARadioBox->checkbox.btn.text, tmp);
	}
	else
	{
		if (add)
		{

			tmpSystemText ++;
		}
		else
		{
			if (tmpSystemText  > 0)
			{
				tmpSystemText--;
			}
		}
	
		sprintf(tmp, "%02d", tmpSystemText);
		ituTextSetString(&trialRunAddrUpdateSRadioBox->checkbox.btn.text, tmp);
	}
	return true;
}
bool TrialRunAddrUpdateValueEnterBtnOnPress(ITUWidget* widget, char* param)
{
	char tmp[32];
	bool enter = atoi(param);

	if (enter)
	{
		tmpSystem[addrupdateItem] = tmpSystemText;
		tmpAddr[addrupdateItem] = tmpAddrText;

		sprintf(tmp, "%02d-%02d", tmpSystem[addrupdateItem], tmpAddr[addrupdateItem]);
		ituTextSetString(&trialRunAddrUpdateRadioBox[addrupdateItem]->checkbox.btn.text, tmp);
	}
	

	return true;
}

bool TrialRunAddrUpdateRadioBoxOnPress(ITUWidget* widget, char* param)
{
	char tmp[32];


	addrupdateItem = atoi(param);

	tmpAddrText = tmpAddr[addrupdateItem];
	sprintf(tmp, "%02d", tmpAddrText);
	ituTextSetString(&trialRunAddrUpdateARadioBox->checkbox.btn.text, tmp);
	

	tmpSystemText = tmpSystem[addrupdateItem];
	sprintf(tmp, "%02d", tmpSystemText);
	ituTextSetString(&trialRunAddrUpdateSRadioBox->checkbox.btn.text, tmp);
	
	




	return true;
}

bool TrialRunAddrUpdateBtnOnMouseUp(ITUWidget* widget, char* param)
{
	int i;
	char tmp[32];

	for (i = 0; i < 16; i++)
	{
		sprintf(tmp, "%02d-%02d", systemNo[i], addr[i]);
		ituTextSetString(&trialRunAddrUpdateRadioBox[i]->checkbox.btn.text, tmp);
	}

	return true;
}

bool TrialRunAddrInitRadioBoxOnPress(ITUWidget* widget, char* param)
{

	addrInitIndex = atoi(param);

	return true;
}
bool TrialRunAddrInitEnterBtnOnPress(ITUWidget* widget, char* param)
{
	char tmp[32];
	int i;

	if (addrInitIndex == 16)
	{
		ituSpriteGoto(trialRunAddrInitTitleSprite, 1);
	}
	else
	{
		sprintf(tmp, "%s", ituTextGetString(&trialRunAddrInitRadioBox[addrInitIndex]->checkbox.btn.text));
		ituTextSetString(trialRunAddrInitTitleText, tmp);
		ituSpriteGoto(trialRunAddrInitTitleSprite, 0);

	}
	return true;
}

bool TrialRunModelSelInitRadioBoxOnPress(ITUWidget* widget, char* param)
{

	modelSelInitIndex = atoi(param);

	return true;
}
bool TrialRunModelSelInitEnterBtnOnPress(ITUWidget* widget, char* param)
{
	char tmp[32];
	int i;

	if (modelSelInitIndex == 16)
	{
		ituSpriteGoto(trialRunModelSelInitTitleSprite, 1);
	}
	else
	{

		sprintf(tmp, "%s", ituTextGetString(&trialRunModelSelInitRadioBox[modelSelInitIndex]->checkbox.btn.text));
		ituTextSetString(trialRunModelSelInitTitleText, tmp);
		ituSpriteGoto(trialRunModelSelInitTitleSprite, 0);

	}
	return true;
}

bool TrialRunBackgroundBtnOnPress(ITUWidget* widget, char* param)
{
	ITULayer* layer;

	layer = (ITULayer*)ituGetVarTarget(1);

	ituLayerGoto(layer);
	return true;
}