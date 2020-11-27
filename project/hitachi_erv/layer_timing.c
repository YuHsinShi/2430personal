#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"


#define STOPANYWHERE_H 496
#define MAX_WEEK_COUNT 12
#define MAX_REST_COUNT 6

typedef struct
{
	ITUContainer* weekContainer;
	ITUButton* deleteButton;
	ITUBackground* background;
	ITUText* timeText;
	ITUCheckBox* selectCheckbox;
	ITUButton* setButton;
	ITUSprite* modeTextSprite;
	ITUContainer* dayTextContainer;
	ITUText* dayText[8];
	ITUButton* slideButton;

} WEEKSet;

typedef struct
{
	ITUContainer* restContainer;
	ITUButton* deleteButton;
	ITUBackground* background;
	ITUText* yearText;
	ITUText* monthText;
	ITUText* dayText;
	ITUCheckBox* selectCheckbox;
	ITUButton* slideButton;

} RESTSet;

static WEEKSet weekSet[MAX_WEEK_COUNT];
static RESTSet restSet[MAX_REST_COUNT];

static ITUText* timingSingleTimeText0 = 0;
static ITUCheckBox* timingSingleCheckBox0 = 0;
static ITUWheel* timingSingleSettingHrWheel = 0;
static ITUSprite* timingSingleTimeSprite = 0;
static ITUSprite* timingSingleSettingSprite = 0;

static ITUStopAnywhere* timingWeekStopAnywhere = 0;
static ITUContainer* timingWeekAllContainer = 0;
static ITUContainer* timingWeekAddContainer = 0;
static ITUBackground* timingWeekSettingBackground = 0;
static ITUBackground* timingWeekSettingRadioBoxBackground[2] = { 0 };
static ITUIcon* timingWeekSettingRadBoxBgIcon[2] = { 0 };
static ITURadioBox* timingWeekSettingRadioBox[2] = { 0 };
static ITUText* timingWeekSettingTimeText = 0;
static ITUContainer* timingWeekSettingDayTextContainer = 0;
static ITUText* timingWeekSettingDayText[8] = { 0 };
static ITUWheel* timingWeekSettingTimeHrWheel = 0;
static ITUWheel* timingWeekSettingTimeMinWheel = 0;
static ITUCheckBox* timingWeekSettingDayCheckBox[7] = { 0 };

static ITUStopAnywhere* timingRestStopAnywhere = 0;
static ITUContainer* timingRestAllContainer = 0;
static ITUContainer* timingRestAddContainer = 0;
static ITUBackground* timingRestSetttingBackground = 0;
//static ITUWheel* timingRestSettingYearWheel = 0;
static ITUWheel* timingRestSettingMonthWheel = 0;
static ITUWheel* timingRestSettingDayWheel = 0;

static ITUContainer* timingTmpWeekContainer = 0;
static ITUContainer* timingTmpRestContainer = 0;

static int SingleTimeHrIndex = 10;

static int weekItemIndex = 0;
static int weekTotalItem = 2;
static int tmpWeekTimeHrIndex = 0;
static int WeekTimeHrIndex[MAX_WEEK_COUNT] = { 10, 14 };
static int tmpWeekTimeMinIndex = 0;
static int WeekTimeMinIndex[MAX_WEEK_COUNT] = { 30, 30 };
static int tmpWeekDay[8] = {0};
static int WeekDay[MAX_WEEK_COUNT][8] = { { 1, 1, 1, 1, 1, 1, 1, 1 }, { 1, 1, 1, 1, 1, 1, 0, 0 } };
static bool tmpPowerOn = false;
static bool PowerOn[MAX_WEEK_COUNT] = { false, true };
static int weekAllContainerHeight;
static bool addingWeek = false;

static int restItemIndex = 0;
static int restTotalItem = 4;
static int restYearIndex[MAX_REST_COUNT] = { 2020, 2020, 2020, 2020 };
static int restMonthIndex[MAX_REST_COUNT] = { 3, 5, 5, 5 };
static int restDayIndex[MAX_REST_COUNT] = { 8, 1, 2, 3 };
static int restAllContainerHeight;
static bool addingRest = false;

bool TimingOnEnter(ITUWidget* widget, char* param)
{
	int i, j, dayItemCnt,W;
	char tmp[64];

	//ITCTree* node;

	if (!timingSingleTimeText0)
	{
		timingSingleTimeText0 = ituSceneFindWidget(&theScene,"timingSingleTimeText0");
		assert(timingSingleTimeText0);

		timingSingleCheckBox0 = ituSceneFindWidget(&theScene, "timingSingleCheckBox0");
		assert(timingSingleCheckBox0);

		timingSingleSettingHrWheel = ituSceneFindWidget(&theScene, "timingSingleSettingHrWheel");
		assert(timingSingleSettingHrWheel);

		timingSingleTimeSprite = ituSceneFindWidget(&theScene, "timingSingleTimeSprite");
		assert(timingSingleTimeSprite);

		timingSingleSettingSprite = ituSceneFindWidget(&theScene, "timingSingleSettingSprite");
		assert(timingSingleSettingSprite);

		timingWeekStopAnywhere = ituSceneFindWidget(&theScene, "timingWeekStopAnywhere");
		assert(timingWeekStopAnywhere);

		timingWeekAllContainer = ituSceneFindWidget(&theScene, "timingWeekAllContainer");
		assert(timingWeekAllContainer);

		timingWeekAddContainer = ituSceneFindWidget(&theScene, "timingWeekAddContainer");
		assert(timingWeekAddContainer);

		timingWeekSettingBackground = ituSceneFindWidget(&theScene, "timingWeekSettingBackground");
		assert(timingWeekSettingBackground);

		timingWeekSettingTimeText = ituSceneFindWidget(&theScene, "timingWeekSettingTimeText");
		assert(timingWeekSettingTimeText);

		timingWeekSettingDayTextContainer = ituSceneFindWidget(&theScene, "timingWeekSettingDayTextContainer");
		assert(timingWeekSettingDayTextContainer);

		timingWeekSettingTimeHrWheel = ituSceneFindWidget(&theScene, "timingWeekSettingTimeHrWheel");
		assert(timingWeekSettingTimeHrWheel);

		timingWeekSettingTimeMinWheel = ituSceneFindWidget(&theScene, "timingWeekSettingTimeMinWheel");
		assert(timingWeekSettingTimeMinWheel);


		for (i = 0; i < 2; i++)
		{
			sprintf(tmp, "timingWeekContainer%d", i);
			weekSet[i].weekContainer = ituSceneFindWidget(&theScene, tmp);
			assert(weekSet[i].weekContainer);

			sprintf(tmp, "timingWeekDeleteBtn%d", i);
			weekSet[i].deleteButton = ituSceneFindWidget(&theScene, tmp);
			assert(weekSet[i].deleteButton);

			sprintf(tmp, "timingWeekBackground%d", i);
			weekSet[i].background = ituSceneFindWidget(&theScene, tmp);
			assert(weekSet[i].background);

			sprintf(tmp, "timingWeekTimeText%d", i);
			weekSet[i].timeText = ituSceneFindWidget(&theScene, tmp);
			assert(weekSet[i].timeText);

			sprintf(tmp, "timingWeekSetButton%d", i);
			weekSet[i].setButton = ituSceneFindWidget(&theScene, tmp);
			assert(weekSet[i].setButton);

			sprintf(tmp, "timingWeekCheckBox%d", i);
			weekSet[i].selectCheckbox = ituSceneFindWidget(&theScene, tmp);
			assert(weekSet[i].selectCheckbox);

			sprintf(tmp, "timingWeekModeTextSprite%d", i);
			weekSet[i].modeTextSprite = ituSceneFindWidget(&theScene, tmp);
			assert(weekSet[i].modeTextSprite);

			sprintf(tmp, "timingWeekDayTextContainer%d", i);
			weekSet[i].dayTextContainer = ituSceneFindWidget(&theScene, tmp);
			assert(weekSet[i].dayTextContainer);

			sprintf(tmp, "timingWeekSlideButton%d", i);
			weekSet[i].slideButton = ituSceneFindWidget(&theScene, tmp);
			assert(weekSet[i].slideButton);

			sprintf(tmp, "timingWeekSettingRadioBoxBackground%d", i);
			timingWeekSettingRadioBoxBackground[i] = ituSceneFindWidget(&theScene, tmp);
			assert(timingWeekSettingRadioBoxBackground[i]);

			sprintf(tmp, "timingWeekSettingRadBoxBgIcon%d", i);
			timingWeekSettingRadBoxBgIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(timingWeekSettingRadBoxBgIcon[i]);

			sprintf(tmp, "timingWeekSettingRadioBox%d", i);
			timingWeekSettingRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(timingWeekSettingRadioBox[i]);
		}

		for (i = 0; i < 7; i++)
		{
			sprintf(tmp, "timingWeekSettingDayCheckBox%d", i);
			timingWeekSettingDayCheckBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(timingWeekSettingDayCheckBox[i]);

		}

		for (i = 0; i < 8; i++)
		{
			sprintf(tmp, "timingWeekSettingDayText%d", i);
			timingWeekSettingDayText[i] = ituSceneFindWidget(&theScene, tmp);
			assert(timingWeekSettingDayText[i]);
		}

		for (i = 0; i < 2; i++)
		{
			for (j = 0; j < 8; j++)
			{
				sprintf(tmp, "timingWeekDayText%d%d", i,j);
				weekSet[i].dayText[j] = ituSceneFindWidget(&theScene, tmp);
				assert(weekSet[i].dayText[j]);
			}
		}

		timingRestStopAnywhere = ituSceneFindWidget(&theScene, "timingRestStopAnywhere");
		assert(timingRestStopAnywhere);

		timingRestAllContainer = ituSceneFindWidget(&theScene, "timingRestAllContainer");
		assert(timingRestAllContainer);

		timingRestAddContainer = ituSceneFindWidget(&theScene, "timingRestAddContainer");
		assert(timingRestAddContainer);

		timingRestSetttingBackground = ituSceneFindWidget(&theScene, "timingRestSetttingBackground");
		assert(timingRestSetttingBackground);

		//timingRestSettingYearWheel = ituSceneFindWidget(&theScene, "timingRestSettingYearWheel");
		//assert(timingRestSettingYearWheel);

		timingRestSettingMonthWheel = ituSceneFindWidget(&theScene, "timingRestSettingMonthWheel");
		assert(timingRestSettingMonthWheel);

		timingRestSettingDayWheel = ituSceneFindWidget(&theScene, "timingRestSettingDayWheel");
		assert(timingRestSettingDayWheel);

		for (i = 0; i < 4; i++)
		{
			sprintf(tmp, "timingRestContainer%d", i);
			restSet[i].restContainer = ituSceneFindWidget(&theScene, tmp);
			assert(restSet[i].restContainer);

			sprintf(tmp, "timingRestDeleteButton%d", i);
			restSet[i].deleteButton = ituSceneFindWidget(&theScene, tmp);
			assert(restSet[i].deleteButton);

			sprintf(tmp, "timingRestBackground%d", i);
			restSet[i].background = ituSceneFindWidget(&theScene, tmp);
			assert(restSet[i].background);

			sprintf(tmp, "timingRestYearText%d", i);
			restSet[i].yearText = ituSceneFindWidget(&theScene, tmp);
			assert(restSet[i].yearText);

			sprintf(tmp, "timingRestMonthText%d", i);
			restSet[i].monthText = ituSceneFindWidget(&theScene, tmp);
			assert(restSet[i].monthText);

			sprintf(tmp, "timingRestDayText%d", i);
			restSet[i].dayText = ituSceneFindWidget(&theScene, tmp);
			assert(restSet[i].dayText);

			sprintf(tmp, "timingRestCheckBox%d", i);
			restSet[i].selectCheckbox = ituSceneFindWidget(&theScene, tmp);
			assert(restSet[i].selectCheckbox);

			sprintf(tmp, "timingRestSlideButton%d", i);
			restSet[i].slideButton = ituSceneFindWidget(&theScene, tmp);
			assert(restSet[i].slideButton);
		}

		timingTmpWeekContainer = ituSceneFindWidget(&theScene, "timingTmpWeekContainer");
		assert(timingTmpWeekContainer);

		timingTmpRestContainer = ituSceneFindWidget(&theScene, "timingTmpRestContainer");
		assert(timingTmpRestContainer);

		
		
	}

	if (SingleTimeHrIndex == 0)
	{
		ituSpriteGoto(timingSingleTimeSprite, 0);
	}
	else
	{
		ituSpriteGoto(timingSingleTimeSprite, 1);
		sprintf(tmp, "%2.1f", (SingleTimeHrIndex + 1)*0.5);
		ituTextSetString(timingSingleTimeText0, tmp);
	}
	
	if (powerOffTimeIndex == -1)
		ituCheckBoxSetChecked(timingSingleCheckBox0, false);
	else
		ituCheckBoxSetChecked(timingSingleCheckBox0, true);

	for (i = 0; i < weekTotalItem; i++)
	{
		ituSpriteGoto(weekSet[i].modeTextSprite, PowerOn[i]);

		sprintf(tmp, "%02d:%02d", WeekTimeHrIndex[i], WeekTimeMinIndex[i]);
		ituTextSetString(weekSet[i].timeText, tmp);

		if (WeekDay[i][7])
		{
			ituWidgetSetVisible(weekSet[i].dayText[7], true);

			for (j = 0; j < 7; j++)
			{
				ituWidgetSetVisible(weekSet[i].dayText[j], false);
			}
		}
		else
		{
			ituWidgetSetVisible(weekSet[i].dayText[7], false);

			dayItemCnt = 0;
			for (j = 0; j < 7; j++)
			{
				if (WeekDay[i][j])
				{
					ituWidgetSetVisible(weekSet[i].dayText[j], true);
					ituWidgetSetX(weekSet[i].dayText[j], dayItemCnt * 56);
					dayItemCnt++;
				}
				else
				{
					ituWidgetSetVisible(weekSet[i].dayText[j], false);
				}
			}
		}
		
	}

	for (i = 0; i < restTotalItem; i++)
	{

		sprintf(tmp, "%04d", restYearIndex[i]);
		ituTextSetString(restSet[i].yearText, tmp);

		sprintf(tmp, "%02d", restMonthIndex[i]);
		ituTextSetString(restSet[i].monthText, tmp);

		sprintf(tmp, "%02d", restDayIndex[i]);
		ituTextSetString(restSet[i].dayText, tmp);

		if (restMonthIndex[i] == 1 || restMonthIndex[i] == 2)
		{
			W = (restDayIndex[i] + 2 * (restMonthIndex[i] + 12) + 3 * ((restMonthIndex[i] + 12) + 1) / 5 + restYearIndex[i] + restYearIndex[i] / 4 - restYearIndex[i] / 100 + restYearIndex[i] / 400) % 7;
		}
		else
		{
			W = (restDayIndex[i] + 2 * restMonthIndex[i] + 3 * (restMonthIndex[i] + 1) / 5 + restYearIndex[i] + restYearIndex[i] / 4 - restYearIndex[i] / 100 + restYearIndex[i] / 400) % 7;
		}

		if (W == 5 || W == 6)
		{
			ituCheckBoxSetChecked(restSet[i].selectCheckbox, true);
		}
		else
		{
			ituCheckBoxSetChecked(restSet[i].selectCheckbox, false);
		}
		
	}

	

	
	
	return true;
}

bool TimingOnLeave(ITUWidget* widget, char* param)
{
	int i;
	timingSet = false;
	if (ituCheckBoxIsChecked(timingSingleCheckBox0))
		timingSet = true;
	for (i = 0; i < weekTotalItem; i++)
	{
		if (ituCheckBoxIsChecked(weekSet[i].selectCheckbox))
			timingSet = true;
	}
	for (i = 0; i < restTotalItem; i++)
	{
		if (ituCheckBoxIsChecked(restSet[i].selectCheckbox))
			timingSet = true;
	}

	return true;
}
bool TimingSingleSettingSaveBtnOnPress(ITUWidget* widget, char* param)
{

	char tmp[32];

	SingleTimeHrIndex = timingSingleSettingHrWheel->focusIndex;

	if (SingleTimeHrIndex == 0)
	{
		ituSpriteGoto(timingSingleTimeSprite, 0);
	}
	else
	{
		ituSpriteGoto(timingSingleTimeSprite, 1);
		sprintf(tmp, "%2.1f", (SingleTimeHrIndex + 1)*0.5);
		ituTextSetString(timingSingleTimeText0, tmp);
	}

	ituCheckBoxSetChecked(timingSingleCheckBox0, false);

	return true;
}

bool TimingSingleSettingBtnOnMouseUp(ITUWidget* widget, char* param)
{

	ituWheelGoto(timingSingleSettingHrWheel, SingleTimeHrIndex);
	if (SingleTimeHrIndex == 0)
	{
		ituSpriteGoto(timingSingleSettingSprite, 0);
	}
	else
	{
		ituSpriteGoto(timingSingleSettingSprite, 1);
	}

	return true;
}

bool TimingSingleSettingHrWheelOnChanged(ITUWidget* widget, char* param)
{
	SingleTimeHrIndex = timingSingleSettingHrWheel->focusIndex;

	if (SingleTimeHrIndex == 0)
	{
		ituSpriteGoto(timingSingleSettingSprite, 0);
	}
	else
	{
		ituSpriteGoto(timingSingleSettingSprite, 1);
	}

	return true;
}

bool TimingSingleChkBoxOnPress(ITUWidget* widget, char* param)
{
	struct timeval tv;
	struct tm *tm;


	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	if (ituCheckBoxIsChecked(timingSingleCheckBox0))
	{
		powerOffTimeIndex = SingleTimeHrIndex;



		if (powerOffTimeIndex > 0)
			powerOffTmHr = powerOffTimeIndex / 2;
		else
			powerOffTmHr = 0;

		if (powerOffTimeIndex % 2 == 0)
			powerOffTmMin = 30;
		else
		{
			powerOffTmMin = 0;
			powerOffTmHr++;
		}


		powerOffTmHr = tm->tm_hour + powerOffTmHr;
		powerOffTmMin = tm->tm_min + powerOffTmMin;

		if (powerOffTmMin > 60)
		{
			powerOffTmMin = powerOffTmMin - 60;
			powerOffTmHr++;
		}
		if (powerOffTmHr > 24)
		{
			powerOffTmHr = powerOffTmHr - 24;
		}

	}
	else
	{
		powerOffTimeIndex = -1;
		powerOffTmHr = 0;
		powerOffTmMin = 0;
	}

	return true;
}
bool TimingWeekSettingBtnOnMouseUp(ITUWidget* widget, char* param)
{
	int i;
	char tmp[32];
	int dayItemCnt = 0;

	weekItemIndex = atoi(param);

	ituRadioBoxSetChecked(timingWeekSettingRadioBox[1], PowerOn[weekItemIndex]);
	ituRadioBoxSetChecked(timingWeekSettingRadioBox[0], !PowerOn[weekItemIndex]);
	ituIconLinkSurface(&timingWeekSettingRadioBoxBackground[1]->icon, timingWeekSettingRadBoxBgIcon[PowerOn[weekItemIndex]]);
	ituIconLinkSurface(&timingWeekSettingRadioBoxBackground[0]->icon, timingWeekSettingRadBoxBgIcon[!PowerOn[weekItemIndex]]);
	tmpPowerOn = PowerOn[weekItemIndex];
	
	sprintf(tmp, "%02d:%02d", WeekTimeHrIndex[weekItemIndex], WeekTimeMinIndex[weekItemIndex]);
	ituTextSetString(timingWeekSettingTimeText, tmp);
	tmpWeekTimeHrIndex = WeekTimeHrIndex[weekItemIndex];
	tmpWeekTimeMinIndex = WeekTimeMinIndex[weekItemIndex];

	if (WeekDay[weekItemIndex][7])
	{
		ituWidgetSetVisible(timingWeekSettingDayText[7], true);
		for (i = 6; i >= 0; i--)
		{
			ituWidgetSetVisible(timingWeekSettingDayText[i], false);
			tmpWeekDay[i] = WeekDay[weekItemIndex][i];
		}
	}
	else
	{
		ituWidgetSetVisible(timingWeekSettingDayText[7], false);
		for (i = 6; i >= 0; i--)
		{
			if (WeekDay[weekItemIndex][i])
			{
				ituWidgetSetVisible(timingWeekSettingDayText[i], true);
				ituWidgetSetX(timingWeekSettingDayText[i], 336 - (dayItemCnt * 56));
				dayItemCnt++;
			}
			else
			{
				ituWidgetSetVisible(timingWeekSettingDayText[i], false);
			}
			tmpWeekDay[i] = WeekDay[weekItemIndex][i];
		}
	}
	
	
	//sprintf(tmp, "%02.1f", 18 + (0.5 * WeekTempIndex[weekItemIndex]));
	//ituTextSetString(timingWeekSettingTempText, tmp);
	//tmpWeekTempIndex = WeekTempIndex[weekItemIndex];

	timingWeekStopAnywhere->widget.flags &= ~ITU_DRAGGABLE;

	return true;
}


bool TimingWeekSettingDaySaveBtnOnPress(ITUWidget* widget, char* param)
{
	int i;
	int dayItemCnt = 0;

	for (i = 6; i >= 0 ; i--)
	{
		if (ituCheckBoxIsChecked(timingWeekSettingDayCheckBox[i]))
		{
			tmpWeekDay[i] = 1;
			ituWidgetSetVisible(timingWeekSettingDayText[i], true);
			ituWidgetSetX(timingWeekSettingDayText[i], 336 - (dayItemCnt * 56));
			dayItemCnt++;
		}
		else
		{
			tmpWeekDay[i] = 0;
			ituWidgetSetVisible(timingWeekSettingDayText[i], false);
		}
			
	}

	if (dayItemCnt == 7)
	{
		tmpWeekDay[7] = 1;
		ituWidgetSetVisible(timingWeekSettingDayText[7], true);
		for (i = 0; i < 7; i++)
		{
			ituWidgetSetVisible(timingWeekSettingDayText[i], false);
		}
	}
	else
	{
		tmpWeekDay[7] = 0;
		ituWidgetSetVisible(timingWeekSettingDayText[7], false);
	}

    return true;
}

bool TimingWeekSettingTimeSaveBtnOnPress(ITUWidget* widget, char* param)
{

	char tmp[32];

	tmpWeekTimeHrIndex = timingWeekSettingTimeHrWheel->focusIndex;
	tmpWeekTimeMinIndex = timingWeekSettingTimeMinWheel->focusIndex;

	sprintf(tmp, "%02d:%02d", tmpWeekTimeHrIndex, tmpWeekTimeMinIndex);
	ituTextSetString(timingWeekSettingTimeText, tmp);

    return true;
}

bool TimingWeekSettingRadBoxOnPress(ITUWidget* widget, char* param)
{
	tmpPowerOn = atoi(param);

	ituRadioBoxSetChecked(timingWeekSettingRadioBox[1], tmpPowerOn);
	ituRadioBoxSetChecked(timingWeekSettingRadioBox[0], !tmpPowerOn);
	ituIconLinkSurface(&timingWeekSettingRadioBoxBackground[1]->icon, timingWeekSettingRadBoxBgIcon[tmpPowerOn]);
	ituIconLinkSurface(&timingWeekSettingRadioBoxBackground[0]->icon, timingWeekSettingRadBoxBgIcon[!tmpPowerOn]);

    return true;
}

bool TimingWeekSettingSaveBtnOnPress(ITUWidget* widget, char* param)
{
	char tmp[32];
	int i;
	int dayItemCnt = 0;
	bool save = atoi(param);

	if (save)
	{
		PowerOn[weekItemIndex] = tmpPowerOn;
		ituSpriteGoto(weekSet[weekItemIndex].modeTextSprite, PowerOn[weekItemIndex]);

		WeekTimeHrIndex[weekItemIndex] = tmpWeekTimeHrIndex;
		WeekTimeMinIndex[weekItemIndex] = tmpWeekTimeMinIndex;
		sprintf(tmp, "%02d:%02d", WeekTimeHrIndex[weekItemIndex], WeekTimeMinIndex[weekItemIndex]);
		ituTextSetString(weekSet[weekItemIndex].timeText, tmp);


		for (i = 0; i < 7; i++)
		{
			WeekDay[weekItemIndex][i] = tmpWeekDay[i];

			if (WeekDay[weekItemIndex][i])
			{
				ituWidgetSetVisible(weekSet[weekItemIndex].dayText[i], true);
				ituWidgetSetX(weekSet[weekItemIndex].dayText[i], dayItemCnt * 56);
				dayItemCnt++;
			}
			else
			{
				ituWidgetSetVisible(weekSet[weekItemIndex].dayText[i], false);
			}

		}

		if (dayItemCnt == 7)
		{
			WeekDay[weekItemIndex][7] = 1;
			ituWidgetSetVisible(weekSet[weekItemIndex].dayText[7], true);
			for (i = 0; i < 7; i++)
			{
				ituWidgetSetVisible(weekSet[weekItemIndex].dayText[i], false);
			}
		}
		else
		{
			WeekDay[weekItemIndex][7] = 0;
			ituWidgetSetVisible(weekSet[weekItemIndex].dayText[7], false);
		}

		//WeekTempIndex[weekItemIndex] = tmpWeekTempIndex;

		ituCheckBoxSetChecked(weekSet[weekItemIndex].selectCheckbox, true);

		if (addingWeek)
		{
			ituWidgetAdd(timingWeekAllContainer, weekSet[weekTotalItem].weekContainer);
			ituWidgetSetPosition(weekSet[weekTotalItem].weekContainer, 0, weekTotalItem * 150);
			ituWidgetSetY(timingWeekAddContainer, (weekTotalItem + 1) * 150);
			ituWidgetSetDimension(timingWeekAllContainer, 670, (weekTotalItem + 1) * 150 + 130);
			weekAllContainerHeight = ituWidgetGetHeight(timingWeekAllContainer); // = weekTotalItem * 150 + 130;
			if (weekAllContainerHeight > STOPANYWHERE_H)
				ituWidgetSetDimension(timingWeekStopAnywhere, 670, STOPANYWHERE_H);
			else
				ituWidgetSetDimension(timingWeekStopAnywhere, 670, weekAllContainerHeight);

			ituWidgetUpdate(timingWeekAllContainer, ITU_EVENT_LAYOUT, 0, 0, 0);// update the container widget

			addingWeek = false;
			weekTotalItem++;
		}
	}
	

	timingWeekStopAnywhere->widget.flags |= ITU_DRAGGABLE;

    return true;
}
bool TimingWeekSettingTimeBtnOnPress(ITUWidget* widget, char* param)
{
	struct timeval tv;
	struct tm *tm;

	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	if (addingWeek)
	{

		ituWheelGoto(timingWeekSettingTimeHrWheel, tm->tm_hour);
		ituWheelGoto(timingWeekSettingTimeMinWheel, tm->tm_min);
	}
	else
	{
		ituWheelGoto(timingWeekSettingTimeHrWheel, tmpWeekTimeHrIndex);
		ituWheelGoto(timingWeekSettingTimeMinWheel, tmpWeekTimeMinIndex);
	}
	

	return true;
}

bool TimingWeekSettingDayBtnOnPress(ITUWidget* widget, char* param)
{

	int i;

	for (i = 0; i < 7; i++)
		ituCheckBoxSetChecked(timingWeekSettingDayCheckBox[i], tmpWeekDay[i]);
		
	return true;
}

bool TimingWeekAddBtnOnMouseUp(ITUWidget* widget, char* param)
{
	bool ret = false;
	int i;

	

	if (weekTotalItem < MAX_WEEK_COUNT)
	{
		addingWeek = true;

		ITCTree* node = NULL;
		ITUContainer* cloneContainer = NULL;

		char id[8];

		ituContainerClone((ITUWidget*)timingTmpWeekContainer, (ITUWidget**)&cloneContainer);
		weekSet[weekTotalItem].weekContainer = cloneContainer;
		
		ITUWidget* widget = (ITUWidget*)cloneContainer;

		node = ((ITCTree*)widget)->child;
		ITUButton* deleteButton = (ITUButton*)node;
		weekSet[weekTotalItem].deleteButton = deleteButton;

		node = node->sibling;
		ITUBackground* background = (ITUBackground*)node;
		weekSet[weekTotalItem].background = background;

		node = ((ITCTree*)background)->child; 
		ITUButton* slideButton = (ITUButton*)node;
		weekSet[weekTotalItem].slideButton = slideButton;

		node = node->sibling;
		ITUText* timeText = (ITUText*)node;
		weekSet[weekTotalItem].timeText = timeText;

		node = node->sibling;
		ITUButton* settingBtn = (ITUButton*)node;
		weekSet[weekTotalItem].setButton = settingBtn;

		node = node->sibling;
		ITUCheckBox* checkbox = (ITUCheckBox*)node;
		weekSet[weekTotalItem].selectCheckbox = checkbox;

		node = node->sibling;
		ITUSprite* sprite = (ITUSprite*)node;
		weekSet[weekTotalItem].modeTextSprite = sprite;

		node = node->sibling;
		ITUContainer* textContainer = (ITUContainer*)node;
		weekSet[weekTotalItem].dayTextContainer = textContainer;

		node = ((ITCTree*)textContainer)->child;
		ITUText* dayText[8] = { 0 };
		for (i = 0; i < 8; i++)
		{
			dayText[i] = (ITUText*)node;
			weekSet[weekTotalItem].dayText[i] = dayText[i];
			node = node->sibling;
		}
		
		sprintf(id, "%d", weekTotalItem);
		strcpy(settingBtn->actions[1].param, id);
		strcpy(settingBtn->actions[5].param, id);
		strcpy(settingBtn->actions[6].param, id);
		strcpy(checkbox->btn.actions[0].param, id);
		strcpy(deleteButton->actions[0].param, id);
		strcpy(slideButton->actions[0].param, id);
		strcpy(slideButton->actions[1].param, id); 

		ituWidgetSetVisible(timingWeekSettingBackground, true);
		TimingWeekSettingBtnOnMouseUp(NULL, settingBtn->actions[1].param);

		ret = true;

	}
    return ret;
}

bool TimingWeekChkBoxOnPress(ITUWidget* widget, char* param)
{
    return true;
}

bool TimingWeekSlideBtnOnSlideLeft(ITUWidget* widget, char* param)
{
	int slideIndex = atoi(param);

	ituWidgetSetX(weekSet[slideIndex].background, -124);

	return true;
}


bool TimingWeekSlideBtnOnSlideRight(ITUWidget* widget, char* param)
{
	int slideIndex = atoi(param);

	ituWidgetSetX(weekSet[slideIndex].background, 0);

	return true;
}

bool TimingWeekDeleteBtnOnPress(ITUWidget* widget, char* param)
{
	int deleteIndex = atoi(param);
	int i,j;
	char tmp[32];

	ituWidgetSetVisible(weekSet[deleteIndex].weekContainer, false);

	for (i = deleteIndex; i < (weekTotalItem - 1); i++)
	{
		ituWidgetSetY(weekSet[i + 1].weekContainer, i * 150);
		
		sprintf(tmp, "%d", i);
		strcpy(weekSet[i + 1].setButton->actions[1].param, tmp);
		strcpy(weekSet[i + 1].setButton->actions[5].param, tmp);
		strcpy(weekSet[i + 1].setButton->actions[6].param, tmp);
		strcpy(weekSet[i + 1].selectCheckbox->btn.actions[0].param, tmp);
		strcpy(weekSet[i + 1].deleteButton->actions[0].param, tmp);
		strcpy(weekSet[i + 1].slideButton->actions[0].param, tmp);
		strcpy(weekSet[i + 1].slideButton->actions[1].param, tmp);

		PowerOn[i] = PowerOn[i + 1];
		WeekTimeHrIndex[i] = WeekTimeHrIndex[i + 1];
		WeekTimeMinIndex[i] = WeekTimeMinIndex[i + 1];

		for (j = 0; j < 7; j++)
		{
			WeekDay[i][j] = WeekDay[i + 1][j];
		}
		//WeekTempIndex[i] = WeekTempIndex[i + 1];
		weekSet[i] = weekSet[i + 1];
	}

	

	PowerOn[weekTotalItem-1] = false;
	WeekTimeHrIndex[weekTotalItem-1] = 0;
	WeekTimeMinIndex[weekTotalItem-1] = 0;
	for (j = 0; j < 7; j++)
	{
		WeekDay[weekTotalItem-1][j] = 0;
	}
	//WeekTempIndex[weekTotalItem-1] = 0;


	weekTotalItem--;
	
	ituWidgetSetY(timingWeekAddContainer, weekTotalItem * 150);
	ituWidgetSetDimension(timingWeekAllContainer, 670, (weekTotalItem) * 150 + 130);
	weekAllContainerHeight = ituWidgetGetHeight(timingWeekAllContainer); // = weekTotalItem * 150 + 130;
	if (weekAllContainerHeight > STOPANYWHERE_H)
		ituWidgetSetDimension(timingWeekStopAnywhere, 670, STOPANYWHERE_H);
	else
		ituWidgetSetDimension(timingWeekStopAnywhere, 670, weekAllContainerHeight);

	ituWidgetUpdate(timingWeekAllContainer, ITU_EVENT_LAYOUT, 0, 0, 0);// update the container widget

	

	return true;
}

bool TimingRestSettingSaveBtnOnPress(ITUWidget* widget, char* param)
{
	char tmp[32];
	bool save = atoi(param);
	struct timeval tv;
	struct tm *tm;

	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	if (save)
	{
		if (timingRestSettingMonthWheel->focusIndex < tm->tm_mon)
			restYearIndex[restTotalItem] = 2020 + 1;
		else
			restYearIndex[restTotalItem] = 2020;

		//if ((timingRestSettingYearWheel->focusIndex % 4) == 0)
		if ((restYearIndex[restTotalItem] % 4) == 0)
		{
			if (timingRestSettingMonthWheel->focusIndex == 1)
			{
				if (timingRestSettingDayWheel->focusIndex > 28)
					ituWheelGoto(timingRestSettingDayWheel, 28);
			}
		}

		else
		{
			if (timingRestSettingMonthWheel->focusIndex == 1)
			{
				if (timingRestSettingDayWheel->focusIndex > 27)
					ituWheelGoto(timingRestSettingDayWheel, 27);
			}
		}


		if ((timingRestSettingMonthWheel->focusIndex == 3) || (timingRestSettingMonthWheel->focusIndex == 5) || (timingRestSettingMonthWheel->focusIndex == 8) || (timingRestSettingMonthWheel->focusIndex == 10))
		{
			if (timingRestSettingDayWheel->focusIndex > 29)
				ituWheelGoto(timingRestSettingDayWheel, 29);
		}


		//restYearIndex[restTotalItem] = timingRestSettingYearWheel->focusIndex + 2020;
		sprintf(tmp, "%04d", restYearIndex[restTotalItem]);
		ituTextSetString(restSet[restTotalItem].yearText, tmp);

		restMonthIndex[restTotalItem] = timingRestSettingMonthWheel->focusIndex + 1;
		sprintf(tmp, "%02d", restMonthIndex[restTotalItem]);
		ituTextSetString(restSet[restTotalItem].monthText, tmp);

		restDayIndex[restTotalItem] = timingRestSettingDayWheel->focusIndex + 1;
		sprintf(tmp, "%02d", restDayIndex[restTotalItem]);
		ituTextSetString(restSet[restTotalItem].dayText, tmp);

		ituCheckBoxSetChecked(restSet[restTotalItem].selectCheckbox, true);

		if (addingRest)
		{
			ituWidgetAdd(timingRestAllContainer, restSet[restTotalItem].restContainer);
			ituWidgetSetPosition(restSet[restTotalItem].restContainer, 0, restTotalItem * 94);
			ituWidgetSetY(timingRestAddContainer, (restTotalItem + 1) * 94);
			ituWidgetSetDimension(timingRestAllContainer, 670, (restTotalItem + 1) * 94 + 88);
			restAllContainerHeight = ituWidgetGetHeight(timingRestAllContainer); // = weekTotalItem * 150 + 130;
			if (restAllContainerHeight > STOPANYWHERE_H)
				ituWidgetSetDimension(timingRestStopAnywhere, 670, STOPANYWHERE_H);
			else
				ituWidgetSetDimension(timingRestStopAnywhere, 670, restAllContainerHeight);

			ituWidgetUpdate(timingRestAllContainer, ITU_EVENT_LAYOUT, 0, 0, 0);// update the container widget


			addingRest = false;
			restTotalItem++;
		}
	}

	
	timingRestStopAnywhere->widget.flags |= ITU_DRAGGABLE;

	return true;
}

bool TimingRestAddBtnOnMouseUp(ITUWidget* widget, char* param)
{
	bool ret = false;
	//int i;
	struct timeval tv;
	struct tm *tm;

	if (restTotalItem < MAX_REST_COUNT)
	{
		addingRest = true;
		ITCTree* node = NULL;
		ITUContainer* cloneContainer = NULL;

		char id[8];

		ituContainerClone((ITUWidget*)timingTmpRestContainer, (ITUWidget**)&cloneContainer);
		restSet[restTotalItem].restContainer = cloneContainer;
		
		ITUWidget* widget = (ITUWidget*)cloneContainer;

		node = ((ITCTree*)widget)->child;
		ITUButton* deleteButton = (ITUButton*)node;
		restSet[restTotalItem].deleteButton = deleteButton;

		node = node->sibling;
		ITUBackground* background = (ITUBackground*)node;
		restSet[restTotalItem].background = background;

		node = ((ITCTree*)background)->child;
		ITUButton* slideButton = (ITUButton*)node;
		restSet[restTotalItem].slideButton = slideButton;

		node = node->sibling;
		ITUText* text = (ITUText*)node;
		restSet[restTotalItem].yearText = text;

		node = node->sibling;

		node = node->sibling;
		text = (ITUText*)node;
		restSet[restTotalItem].monthText = text;

		node = node->sibling;

		node = node->sibling;
		text = (ITUText*)node;
		restSet[restTotalItem].dayText = text;

		node = node->sibling;

		node = node->sibling;
		ITUCheckBox* checkbox = (ITUCheckBox*)node;
		restSet[restTotalItem].selectCheckbox = checkbox;

		sprintf(id, "%d", restTotalItem);
		strcpy(checkbox->btn.actions[0].param, id);
		strcpy(deleteButton->actions[0].param, id);
		strcpy(slideButton->actions[0].param, id);
		strcpy(slideButton->actions[1].param, id);




		gettimeofday(&tv, NULL);
		tm = localtime(&tv.tv_sec);

		//ituWheelGoto(timingRestSettingYearWheel, tm->tm_year - 120);//2020-1900
		ituWheelGoto(timingRestSettingMonthWheel, tm->tm_mon);//0-11
		ituWheelGoto(timingRestSettingDayWheel, tm->tm_mday - 1);//1-31

		ituWidgetSetVisible(timingRestSetttingBackground, true);

		ret = true;

		timingRestStopAnywhere->widget.flags &= ~ITU_DRAGGABLE;
	}

	return ret;
}

bool TimingRestChkBoxOnPress(ITUWidget* widget, char* param)
{
	return true;
}

bool TimingRestSettingWheelOnChanged(ITUWidget* widget, char* param)
{
	//if ((timingRestSettingYearWheel->focusIndex % 4) == 0)
	//{
		if (timingRestSettingMonthWheel->focusIndex == 1)
		{
			if (timingRestSettingDayWheel->focusIndex > 28)
				ituWheelGoto(timingRestSettingDayWheel, 28);
		}				
	//}
		
	//else
	//{
	//	if (timingRestSettingMonthWheel->focusIndex == 1)
	//	{
	//		if (timingRestSettingDayWheel->focusIndex > 27)
	//			ituWheelGoto(timingRestSettingDayWheel, 27);
	//	}	
	//}
		

	if ((timingRestSettingMonthWheel->focusIndex == 3) || (timingRestSettingMonthWheel->focusIndex == 5) || (timingRestSettingMonthWheel->focusIndex == 8) || (timingRestSettingMonthWheel->focusIndex == 10))
	{
		if (timingRestSettingDayWheel->focusIndex > 29)
			ituWheelGoto(timingRestSettingDayWheel, 29);
	}
	
	return true;
}

bool TimingRestSlideBtnOnSlideLeft(ITUWidget* widget, char* param)
{
	int slideIndex = atoi(param);

	ituWidgetSetX(restSet[slideIndex].background, -124);

	return true;
}


bool TimingRestSlideBtnOnSlideRight(ITUWidget* widget, char* param)
{
	int slideIndex = atoi(param);

	ituWidgetSetX(restSet[slideIndex].background, 0);

	return true;
}

bool TimingRestDeleteBtnOnPress(ITUWidget* widget, char* param)
{
	int deleteIndex = atoi(param);
	int i;
	char tmp[32];

	ituWidgetSetVisible(restSet[deleteIndex].restContainer, false);

	for (i = deleteIndex; i < (restTotalItem - 1); i++)
	{
		ituWidgetSetY(restSet[i + 1].restContainer, i * 94);

		sprintf(tmp, "%d", i);
		strcpy(restSet[i + 1].selectCheckbox->btn.actions[0].param, tmp);
		strcpy(restSet[i + 1].deleteButton->actions[0].param, tmp);
		strcpy(restSet[i + 1].slideButton->actions[0].param, tmp);
		strcpy(restSet[i + 1].slideButton->actions[1].param, tmp);

		restYearIndex[i] = restYearIndex[i + 1];
		restMonthIndex[i] = restMonthIndex[i + 1];
		restDayIndex[i] = restDayIndex[i + 1];

		restSet[i] = restSet[i + 1];
	}


	restYearIndex[restTotalItem-1] = 0;
	restMonthIndex[restTotalItem - 1] = 0;
	restDayIndex[restTotalItem - 1] = 0;

	restTotalItem--;

	ituWidgetSetY(timingRestAddContainer, restTotalItem * 94);
	ituWidgetSetDimension(timingRestAllContainer, 670, (restTotalItem)* 94 + 88);
	restAllContainerHeight = ituWidgetGetHeight(timingRestAllContainer); 
	if (restAllContainerHeight > STOPANYWHERE_H)
		ituWidgetSetDimension(timingRestStopAnywhere, 670, STOPANYWHERE_H);
	else
		ituWidgetSetDimension(timingRestStopAnywhere, 670, restAllContainerHeight);

	ituWidgetUpdate(timingRestAllContainer, ITU_EVENT_LAYOUT, 0, 0, 0);// update the container widget


	return true;
}

