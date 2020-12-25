#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"


#define EXAM_REC_STOPANYWHERE_H 530

//static ITUText* examineSubTitleText = 0;
static ITUText* examineValueTitleText = 0;
static ITUText* examineValueText[7] = { 0 };
static ITURadioBox* examineSubRadioBox[16] = { 0 };

static ITUStopAnywhere* examineRecordStopAnywhere = 0;
static ITUContainer* examineRecordAllContainer = 0;
static ITUContainer* examineRecordTmpContainer = 0;

static ITUText* examineShowModelText = 0;
static ITURadioBox* examineShowModelRadioBox[16] = { 0 };

//static ITUSprite* examineModelTestResultSprite[3] = { 0 };
//static ITUText* examineModelTestResultText[3] = { 0 };

static ITUText* examineSelfTestText = 0;

typedef struct
{
	ITUContainer* Container;
	ITUText* dayText;
	ITUText* timeText;
	ITUText* addrText;
	ITUText* noText;

}EXAMINERecordSet;
static EXAMINERecordSet examineRecordSet[EXAM_REC_MAX_ITEM];

//static int examineIndex = 0;
static int examineSubAddrIndex = 0;
static int examineValue[16][7] = { {32,25,38,43,0,0,0} };


static int examRecordAllContainerHeight = 0;
extern int examineRecordTotalItem = 2;
static int examineRecordShowTotalItem = 2;
extern int examineDayRecord[EXAM_REC_MAX_ITEM][3] = { { 2020, 10, 12 }, { 2020, 10, 12 }, { 2020, 10, 12 }, { 2020, 10, 12 }, { 2020, 10, 12 }, { 2020, 10, 12 } };
extern int examineTimeRecord[EXAM_REC_MAX_ITEM][2] = { { 12, 53 }, { 12, 42 }, { 12, 23 }, { 12, 14 }, { 12, 13 }, { 12, 8 } };
extern int examineAddrRecord[EXAM_REC_MAX_ITEM] = { 0, 0, 0, 0, 0, 0 };
extern int examineNoRecord[EXAM_REC_MAX_ITEM] = { 32, 32, 32, 32, 32, 32 };

//static int examineModelTestIndex = 0;
//static int examineModelTestValue[16][3] = { { 3, 3, 2 }, { 2, 2, 3 }, };

void AddExamineRecordContainer(void);
void DeleteExamineRecordContainer(void);



bool ExamineOnEnter(ITUWidget* widget, char* param)
{
	int i;
	char tmp[32];

	if (!examineValueTitleText)
	{
		//examineSubTitleText = ituSceneFindWidget(&theScene, "examineSubTitleText");
		//assert(examineSubTitleText);

		examineValueTitleText = ituSceneFindWidget(&theScene, "examineValueTitleText");
		assert(examineValueTitleText);

		examineRecordStopAnywhere = ituSceneFindWidget(&theScene, "examineRecordStopAnywhere");
		assert(examineRecordStopAnywhere);

		examineRecordAllContainer = ituSceneFindWidget(&theScene, "examineRecordAllContainer");
		assert(examineRecordAllContainer);

		examineRecordTmpContainer = ituSceneFindWidget(&theScene, "examineRecordTmpContainer");
		assert(examineRecordTmpContainer);

		examineShowModelText = ituSceneFindWidget(&theScene, "examineShowModelText");
		assert(examineShowModelText);

		examineSelfTestText = ituSceneFindWidget(&theScene, "examineSelfTestText");
		assert(examineSelfTestText);

		for (i = 0; i < 7; i++)
		{
			sprintf(tmp, "examineB%dText", i + 1);
			examineValueText[i] = ituSceneFindWidget(&theScene, tmp);
			assert(examineValueText[i]);
		}

		for (i = 0; i < 16; i++)
		{
			sprintf(tmp, "examineSubRadioBox%d", i);
			examineSubRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(examineSubRadioBox[i]);

			sprintf(tmp, "examineShowModelRadioBox%d", i);
			examineShowModelRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(examineShowModelRadioBox[i]);

			
		}

		for (i = 0; i < 2; i++)
		{
			sprintf(tmp, "examineRecordContainer%d", i);
			examineRecordSet[i].Container = ituSceneFindWidget(&theScene, tmp);
			assert(examineRecordSet[i].Container);
			
			sprintf(tmp, "examineRecordDayText%d", i);
			examineRecordSet[i].dayText = ituSceneFindWidget(&theScene, tmp);
			assert(examineRecordSet[i].dayText);

			sprintf(tmp, "examineRecordTimeText%d", i);
			examineRecordSet[i].timeText = ituSceneFindWidget(&theScene, tmp);
			assert(examineRecordSet[i].timeText);

			sprintf(tmp, "examineRecordAddrText%d", i);
			examineRecordSet[i].addrText = ituSceneFindWidget(&theScene, tmp);
			assert(examineRecordSet[i].addrText);

			sprintf(tmp, "examineRecordNoText%d", i);
			examineRecordSet[i].noText = ituSceneFindWidget(&theScene, tmp);
			assert(examineRecordSet[i].noText);
		}

		//for (i = 0; i < 3; i++)
		//{
		//	sprintf(tmp, "examineModelTestResultSprite%d", i);
		//	examineModelTestResultSprite[i] = ituSceneFindWidget(&theScene, tmp);
		//	assert(examineModelTestResultSprite[i]);

		//	sprintf(tmp, "examineModelTestResultText%d", i);
		//	examineModelTestResultText[i] = ituSceneFindWidget(&theScene, tmp);
		//	assert(examineModelTestResultText[i]);
		//}
	}

	if (examineRecordTotalItem > examineRecordShowTotalItem)
	{
		AddExamineRecordContainer();
	}


	for (i = 0; i < examineRecordTotalItem; i++)
	{
		sprintf(tmp, "%04d.%02d.%02d", examineDayRecord[i][0], examineDayRecord[i][1], examineDayRecord[i][2]);
		ituTextSetString(examineRecordSet[i].dayText, tmp);

		sprintf(tmp, "%02d:%02d", examineTimeRecord[i][0], examineTimeRecord[i][1]);
		ituTextSetString(examineRecordSet[i].timeText, tmp);

		sprintf(tmp, "00-%02d", examineAddrRecord[i] + 1);
		ituTextSetString(examineRecordSet[i].addrText, tmp);

		sprintf(tmp, "%02d", examineNoRecord[i]);
		ituTextSetString(examineRecordSet[i].noText, tmp);
	}

	return true;
}

bool ExamineSubEnterBtnOnPress(ITUWidget* widget, char* param)
{
	int i;
	char tmp[32];

	sprintf(tmp, "%s", ituTextGetString(&examineSubRadioBox[examineSubAddrIndex]->checkbox.btn.text));
	//sprintf(tmp, "%d: 00-%02d", examineIndex + 1, examineSubAddrIndex + 1);
	ituTextSetString(examineValueTitleText, tmp);

	for (i = 0; i < 7; i++)
	{
		if (examineValue[examineSubAddrIndex][i] == 0)
		{
			ituTextSetString(examineValueText[i], "--");
		}
		else
		{
			sprintf(tmp, "%02d", examineValue[examineSubAddrIndex][i]);
			ituTextSetString(examineValueText[i], tmp);
		}
		
	}

    return true;
}

bool ExamineSubRadioBoxOnPress(ITUWidget* widget, char* param)
{
	examineSubAddrIndex = atoi(param);

    return true;
}

bool ExamineBtnOnMouseUp(ITUWidget* widget, char* param)
{
	char tmp[32];
	int i;

	for (i = 0; i < 16; i++)
	{
		sprintf(tmp, "%02d-%02d", systemNo[i], addr[i]);
		ituTextSetString(&examineSubRadioBox[i]->checkbox.btn.text, tmp);
	}

	//examineIndex = atoi(param);
	//sprintf(tmp, "%d", examineIndex + 1);
	//ituTextSetString(examineSubTitleText, tmp);
	//ituRadioBoxSetChecked(examineSubRadioBox[0], true);
	examineSubAddrIndex = 0;

    return true;
}

void AddExamineRecordContainer(void)
{
	while (examineRecordShowTotalItem < examineRecordTotalItem)
	{
		if (examineRecordShowTotalItem < EXAM_REC_MAX_ITEM)
		{
			ITCTree* node = NULL;
			ITCTree* subBackgroundNode = NULL;
			ITUContainer* cloneContainer = NULL;

			ituContainerClone((ITUWidget*)examineRecordTmpContainer, (ITUWidget**)&cloneContainer);
			examineRecordSet[examineRecordShowTotalItem].Container = cloneContainer;

			ITUWidget* widget = (ITUWidget*)cloneContainer;

			node = ((ITCTree*)widget)->child;
			ITUBackground* background = (ITUBackground*)node;

			subBackgroundNode = ((ITCTree*)background)->child;
			ITUBackground* subBackground = (ITUBackground*)subBackgroundNode;

			node = ((ITCTree*)subBackground)->child;
			ITUText* dayText = (ITUText*)node;
			examineRecordSet[examineRecordShowTotalItem].dayText = dayText;

			subBackgroundNode = subBackgroundNode->sibling;
			node = subBackgroundNode->child;
			ITUText* timeText = (ITUText*)node;
			examineRecordSet[examineRecordShowTotalItem].timeText = timeText;

			subBackgroundNode = subBackgroundNode->sibling;
			node = subBackgroundNode->child;
			ITUText* addrText = (ITUText*)node;
			examineRecordSet[examineRecordShowTotalItem].addrText = addrText;

			subBackgroundNode = subBackgroundNode->sibling;
			node = subBackgroundNode->child;
			ITUText* noText = (ITUText*)node;
			examineRecordSet[examineRecordShowTotalItem].noText = noText;

			ituWidgetAdd(examineRecordAllContainer, examineRecordSet[examineRecordShowTotalItem].Container);
			ituWidgetSetPosition(examineRecordSet[examineRecordShowTotalItem].Container, 0, examineRecordShowTotalItem * 94);
			ituWidgetSetDimension(examineRecordAllContainer, 670, (examineRecordShowTotalItem)* 94 + 88);
			examRecordAllContainerHeight = ituWidgetGetHeight(examineRecordAllContainer);
			if (examRecordAllContainerHeight > EXAM_REC_STOPANYWHERE_H)
				ituWidgetSetDimension(examineRecordStopAnywhere, 670, EXAM_REC_STOPANYWHERE_H);
			else
				ituWidgetSetDimension(examineRecordStopAnywhere, 670, examRecordAllContainerHeight);

			ituWidgetUpdate(examineRecordAllContainer, ITU_EVENT_LAYOUT, 0, 0, 0);// update the container widget

			examineRecordShowTotalItem++;
		}
		else
		{
			break;
		}
	}
	
}

void DeleteExamineRecordContainer(void)
{
	int i;

	for (i = 0; i < examineRecordShowTotalItem; i++)
	{
		ituWidgetSetVisible(examineRecordSet[i].Container, false);
	}

	ituWidgetSetDimension(examineRecordAllContainer, 670, 0);
	ituWidgetSetDimension(examineRecordStopAnywhere, 670, 0);
	memset(examineDayRecord, 0, sizeof(examineDayRecord));
	memset(examineTimeRecord, 0, sizeof(examineTimeRecord));
	memset(examineAddrRecord, 0, sizeof(examineAddrRecord));
	memset(examineNoRecord, 0, sizeof(examineNoRecord));
	examineRecordShowTotalItem = 0;
	examineRecordTotalItem = 0;
	
}
bool ExamineShowBtnOnMouseUp(ITUWidget* widget, char* param)
{
	char tmp[32];
	int i;

	for (i = 0; i < 16; i++)
	{
		sprintf(tmp, "%02d-%02d", systemNo[i], addr[i]);
		ituTextSetString(&examineShowModelRadioBox[i]->checkbox.btn.text, tmp);
}
	return true;
}

bool ExamineShowModelRadioBoxOnPress(ITUWidget* widget, char* param)
{
	int index = atoi(param);
	char tmp[32];

	sprintf(tmp, "F.%02d", addr[index]);
	ituTextSetString(examineShowModelText, tmp);

	return true;
	}

//bool ExamineModelTestRadioBoxOnPress(ITUWidget* widget, char* param)
//{
//	examineModelTestIndex = atoi(param);
//
//
//	return true;
//}
//
//bool ExamineModelTestEnterBtnOnPress(ITUWidget* widget, char* param)
//{
//	int i;
//	char tmp[32];
//
//	ituSpriteGoto(examineModelTestResultSprite[0], 0);
//	ituSpriteGoto(examineModelTestResultSprite[1], 0);
//	ituSpriteGoto(examineModelTestResultSprite[2], 1);
//
//	for (i = 0; i < 3; i++)
//	{
//		sprintf(tmp, "%02d ", examineModelTestValue[examineModelTestIndex][i]);
//		ituTextSetString(examineModelTestResultText[i], tmp);
//	}
//
//	return true;
//}
bool ExamineBackgroundBtnOnPress(ITUWidget* widget, char* param)
{
	ITULayer* layer;

	layer = (ITULayer*)ituGetVarTarget(1);

	ituLayerGoto(layer);
	return true;
}