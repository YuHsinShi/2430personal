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

static WEEKSet powerOffWeekSet[MAX_WEEK_COUNT];
static RESTSet powerOffRestSet[MAX_REST_COUNT];

static ITUText* powerOffTimingSingleTimeText0 = 0;
static ITUCheckBox* powerOffTimingSingleCheckBox0 = 0;
static ITUWheel* powerOffTimingSingleSettingHrWheel = 0;
static ITUSprite* powerOffTimingSingleTimeSprite = 0;
static ITUSprite* powerOffTimingSingleSettingSprite = 0;

static ITUStopAnywhere* powerOffTimingWeekStopAnywhere = 0;
static ITUContainer* powerOffTimingWeekAllContainer = 0;
static ITUContainer* powerOffTimingWeekAddContainer = 0;
static ITUBackground* powerOffTimingWeekSettingBackground = 0;
static ITUText* powerOffTimingWeekSettingTimeText = 0;
static ITUContainer* powerOffTimingWeekSettingDayTextContainer = 0;
static ITUText* powerOffTimingWeekSettingDayText[8] = { 0 };
static ITUWheel* powerOffTimingWeekSettingTimeHrWheel = 0;
static ITUWheel* powerOffTimingWeekSettingTimeMinWheel = 0;
static ITUCheckBox* powerOffTimingWeekSettingDayCheckBox[7] = { 0 };

static ITUStopAnywhere* powerOffTimingRestStopAnywhere = 0;
static ITUContainer* powerOffTimingRestAllContainer = 0;
static ITUContainer* powerOffTimingRestAddContainer = 0;
static ITUBackground* powerOffTimingRestSettingBackground = 0;
static ITUWheel* powerOffTimingRestSettingMonthWheel = 0;
static ITUWheel* powerOffTimingRestSettingDayWheel = 0;

static ITUContainer* powerOffTimingTmpWeekContainer = 0;
static ITUContainer* powerOffTimingTmpRestContainer = 0;

static int powerOffSingleTimeHrIndex = 10;

static int powerOffWeekItemIndex = 0;
static int powerOffWeekTotalItem = 2;
static int tmpPowerOffWeekTimeHrIndex = 0;
static int powerOffWeekTimeHrIndex[MAX_WEEK_COUNT] = { 10, 14 };
static int tmpPowerOffWeekTimeMinIndex = 0;
static int powerOffWeekTimeMinIndex[MAX_WEEK_COUNT] = { 30, 30 };
static int tmpPowerOffWeekDay[8] = { 0 };
static int powerOffWeekDay[MAX_WEEK_COUNT][8] = { { 1, 1, 1, 1, 1, 1, 1, 1 }, { 1, 1, 1, 1, 1, 1, 0, 0 } };
int powerOffWeekAllContainerHeight;
static bool powerOffAddingWeek = false;

static int powerOffRestItemIndex = 0;
static int powerOffRestTotalItem = 4;
static int powerOffRestYearIndex[MAX_REST_COUNT] = { 2020, 2020, 2020, 2020 };
static int powerOffRestMonthIndex[MAX_REST_COUNT] = { 3, 5, 5, 5 };
static int powerOffRestDayIndex[MAX_REST_COUNT] = { 8, 1, 2, 3 };
int powerOffRestAllContainerHeight;
static bool powerOffAddingRest = false;

static bool sliding = false;
static bool slidingLeft = false;
static ITUBackground* slidingBackground = 0;
static uint32_t lastTick = 0;

bool PowerOffTimingOnEnter(ITUWidget* widget, char* param)
{
	int i, j, dayItemCnt,W;
	char tmp[64];

//	ITCTree* node;

	if (!powerOffTimingSingleTimeText0)
	{
		powerOffTimingSingleTimeText0 = ituSceneFindWidget(&theScene, "powerOffTimingSingleTimeText0");
		assert(powerOffTimingSingleTimeText0);

		powerOffTimingSingleCheckBox0 = ituSceneFindWidget(&theScene, "powerOffTimingSingleCheckBox0");
		assert(powerOffTimingSingleCheckBox0);

		powerOffTimingSingleSettingHrWheel = ituSceneFindWidget(&theScene, "powerOffTimingSingleSettingHrWheel");
		assert(powerOffTimingSingleSettingHrWheel);

		powerOffTimingSingleTimeSprite = ituSceneFindWidget(&theScene, "powerOffTimingSingleTimeSprite");
		assert(powerOffTimingSingleTimeSprite);

		powerOffTimingSingleSettingSprite = ituSceneFindWidget(&theScene, "powerOffTimingSingleSettingSprite");
		assert(powerOffTimingSingleSettingSprite);

		powerOffTimingWeekStopAnywhere = ituSceneFindWidget(&theScene, "powerOffTimingWeekStopAnywhere");
		assert(powerOffTimingWeekStopAnywhere);

		powerOffTimingWeekAllContainer = ituSceneFindWidget(&theScene, "powerOffTimingWeekAllContainer");
		assert(powerOffTimingWeekAllContainer);

		powerOffTimingWeekAddContainer = ituSceneFindWidget(&theScene, "powerOffTimingWeekAddContainer");
		assert(powerOffTimingWeekAddContainer);

		powerOffTimingWeekSettingBackground = ituSceneFindWidget(&theScene, "powerOffTimingWeekSettingBackground");
		assert(powerOffTimingWeekSettingBackground);

		powerOffTimingWeekSettingTimeText = ituSceneFindWidget(&theScene, "powerOffTimingWeekSettingTimeText");
		assert(powerOffTimingWeekSettingTimeText);

		powerOffTimingWeekSettingDayTextContainer = ituSceneFindWidget(&theScene, "powerOffTimingWeekSettingDayTextContainer");
		assert(powerOffTimingWeekSettingDayTextContainer);

		powerOffTimingWeekSettingTimeHrWheel = ituSceneFindWidget(&theScene, "powerOffTimingWeekSettingTimeHrWheel");
		assert(powerOffTimingWeekSettingTimeHrWheel);

		powerOffTimingWeekSettingTimeMinWheel = ituSceneFindWidget(&theScene, "powerOffTimingWeekSettingTimeMinWheel");
		assert(powerOffTimingWeekSettingTimeMinWheel);


		for (i = 0; i < 2; i++)
		{
			sprintf(tmp, "powerOffTimingWeekContainer%d", i);
			powerOffWeekSet[i].weekContainer = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffWeekSet[i].weekContainer);

			sprintf(tmp, "powerOffTimingWeekDeleteBtn%d", i);
			powerOffWeekSet[i].deleteButton = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffWeekSet[i].deleteButton);

			sprintf(tmp, "powerOffTimingWeekBackground%d", i);
			powerOffWeekSet[i].background = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffWeekSet[i].background);

			sprintf(tmp, "powerOffTimingWeekTimeText%d", i);
			powerOffWeekSet[i].timeText = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffWeekSet[i].timeText);

			sprintf(tmp, "powerOffTimingWeekSetButton%d", i);
			powerOffWeekSet[i].setButton = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffWeekSet[i].setButton);

			sprintf(tmp, "powerOffTimingWeekCheckBox%d", i);
			powerOffWeekSet[i].selectCheckbox = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffWeekSet[i].selectCheckbox);

			sprintf(tmp, "powerOffTimingWeekModeTextSprite%d", i);
			powerOffWeekSet[i].modeTextSprite = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffWeekSet[i].modeTextSprite);

			sprintf(tmp, "powerOffTimingWeekDayTextContainer%d", i);
			powerOffWeekSet[i].dayTextContainer = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffWeekSet[i].dayTextContainer);

			sprintf(tmp, "powerOffTimingWeekSlideButton%d", i);
			powerOffWeekSet[i].slideButton = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffWeekSet[i].slideButton);

			//sprintf(tmp, "timingWeekSettingRadioBoxBackground%d", i);
			//timingWeekSettingRadioBoxBackground[i] = ituSceneFindWidget(&theScene, tmp);
			//assert(timingWeekSettingRadioBoxBackground[i]);

			//sprintf(tmp, "timingWeekSettingRadBoxBgIcon%d", i);
			//timingWeekSettingRadBoxBgIcon[i] = ituSceneFindWidget(&theScene, tmp);
			//assert(timingWeekSettingRadBoxBgIcon[i]);

			//sprintf(tmp, "timingWeekSettingRadioBox%d", i);
			//timingWeekSettingRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			//assert(timingWeekSettingRadioBox[i]);
		}

		for (i = 0; i < 7; i++)
		{
			sprintf(tmp, "powerOffTimingWeekSettingDayCheckBox%d", i);
			powerOffTimingWeekSettingDayCheckBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffTimingWeekSettingDayCheckBox[i]);

		}

		for (i = 0; i < 8; i++)
		{
			sprintf(tmp, "powerOffTimingWeekSettingDayText%d", i);
			powerOffTimingWeekSettingDayText[i] = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffTimingWeekSettingDayText[i]);
		}

		for (i = 0; i < 2; i++)
		{
			for (j = 0; j < 8; j++)
			{
				sprintf(tmp, "powerOffTimingWeekDayText%d%d", i,j);
				powerOffWeekSet[i].dayText[j] = ituSceneFindWidget(&theScene, tmp);
				assert(powerOffWeekSet[i].dayText[j]);
			}
		}

		powerOffTimingRestStopAnywhere = ituSceneFindWidget(&theScene, "powerOffTimingRestStopAnywhere");
		assert(powerOffTimingRestStopAnywhere);

		powerOffTimingRestAllContainer = ituSceneFindWidget(&theScene, "powerOffTimingRestAllContainer");
		assert(powerOffTimingRestAllContainer);

		powerOffTimingRestAddContainer = ituSceneFindWidget(&theScene, "powerOffTimingRestAddContainer");
		assert(powerOffTimingRestAddContainer);

		powerOffTimingRestSettingBackground = ituSceneFindWidget(&theScene, "powerOffTimingRestSettingBackground");
		assert(powerOffTimingRestSettingBackground);

		//powerOffTimingRestSettingYearWheel = ituSceneFindWidget(&theScene, "powerOffTimingRestSettingYearWheel");
		//assert(powerOffTimingRestSettingYearWheel);

		powerOffTimingRestSettingMonthWheel = ituSceneFindWidget(&theScene, "powerOffTimingRestSettingMonthWheel");
		assert(powerOffTimingRestSettingMonthWheel);

		powerOffTimingRestSettingDayWheel = ituSceneFindWidget(&theScene, "powerOffTimingRestSettingDayWheel");
		assert(powerOffTimingRestSettingDayWheel);

		for (i = 0; i < 4; i++)
		{
			sprintf(tmp, "powerOffTimingRestContainer%d", i);
			powerOffRestSet[i].restContainer = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffRestSet[i].restContainer);

			sprintf(tmp, "powerOffTimingRestDeleteButton%d", i);
			powerOffRestSet[i].deleteButton = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffRestSet[i].deleteButton);

			sprintf(tmp, "powerOffTimingRestBackground%d", i);
			powerOffRestSet[i].background = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffRestSet[i].background);

			sprintf(tmp, "powerOffTimingRestYearText%d", i);
			powerOffRestSet[i].yearText = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffRestSet[i].yearText);

			sprintf(tmp, "powerOffTimingRestMonthText%d", i);
			powerOffRestSet[i].monthText = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffRestSet[i].monthText);

			sprintf(tmp, "powerOffTimingRestDayText%d", i);
			powerOffRestSet[i].dayText = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffRestSet[i].dayText);

			sprintf(tmp, "powerOffTimingRestCheckBox%d", i);
			powerOffRestSet[i].selectCheckbox = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffRestSet[i].selectCheckbox);

			sprintf(tmp, "powerOffTimingRestSlideButton%d", i);
			powerOffRestSet[i].slideButton = ituSceneFindWidget(&theScene, tmp);
			assert(powerOffRestSet[i].slideButton);
		}

		powerOffTimingTmpWeekContainer = ituSceneFindWidget(&theScene, "powerOffTimingTmpWeekContainer");
		assert(powerOffTimingTmpWeekContainer);

		powerOffTimingTmpRestContainer = ituSceneFindWidget(&theScene, "powerOffTimingTmpRestContainer");
		assert(powerOffTimingTmpRestContainer);

		
		
	}

	if (powerOffSingleTimeHrIndex == 0)
	{
		ituSpriteGoto(powerOffTimingSingleTimeSprite, 0);
	}
	else
	{
		ituSpriteGoto(powerOffTimingSingleTimeSprite, 1);
		sprintf(tmp, "%2.1f", (powerOffSingleTimeHrIndex + 1)*0.5);
		ituTextSetString(powerOffTimingSingleTimeText0, tmp);
	}
	if (powerOnTimeIndex == -1)
		ituCheckBoxSetChecked(powerOffTimingSingleCheckBox0, false);
	else
		ituCheckBoxSetChecked(powerOffTimingSingleCheckBox0, true);
	

	for (i = 0; i < powerOffWeekTotalItem; i++)
	{
		ituSpriteGoto(powerOffWeekSet[i].modeTextSprite,1);

		sprintf(tmp, "%02d:%02d", powerOffWeekTimeHrIndex[i], powerOffWeekTimeMinIndex[i]);
		ituTextSetString(powerOffWeekSet[i].timeText, tmp);

		if (powerOffWeekDay[i][7])
		{
			ituWidgetSetVisible(powerOffWeekSet[i].dayText[7], true);

			for (j = 0; j < 7; j++)
			{
				ituWidgetSetVisible(powerOffWeekSet[i].dayText[j], false);
			}
		}
		else
		{
			ituWidgetSetVisible(powerOffWeekSet[i].dayText[7], false);

			dayItemCnt = 0;
			for (j = 0; j < 7; j++)
			{
				if (powerOffWeekDay[i][j])
				{
					ituWidgetSetVisible(powerOffWeekSet[i].dayText[j], true);
					ituWidgetSetX(powerOffWeekSet[i].dayText[j], dayItemCnt * 56);
					dayItemCnt++;
				}
				else
				{
					ituWidgetSetVisible(powerOffWeekSet[i].dayText[j], false);
				}
			}
		}
		
	}

	for (i = 0; i < powerOffRestTotalItem; i++)
	{

		sprintf(tmp, "%04d", powerOffRestYearIndex[i]);
		ituTextSetString(powerOffRestSet[i].yearText, tmp);

		sprintf(tmp, "%02d", powerOffRestMonthIndex[i]);
		ituTextSetString(powerOffRestSet[i].monthText, tmp);

		sprintf(tmp, "%02d", powerOffRestDayIndex[i]);
		ituTextSetString(powerOffRestSet[i].dayText, tmp);

		if (powerOffRestMonthIndex[i] == 1 || powerOffRestMonthIndex[i] == 2)
		{
			W = (powerOffRestDayIndex[i] + 2 * (powerOffRestMonthIndex[i] + 12) + 3 * ((powerOffRestMonthIndex[i] + 12) + 1) / 5 + powerOffRestYearIndex[i] + powerOffRestYearIndex[i] / 4 - powerOffRestYearIndex[i] / 100 + powerOffRestYearIndex[i] / 400) % 7;
		}
		else
		{
			W = (powerOffRestDayIndex[i] + 2 * powerOffRestMonthIndex[i] + 3 * (powerOffRestMonthIndex[i] + 1) / 5 + powerOffRestYearIndex[i] + powerOffRestYearIndex[i] / 4 - powerOffRestYearIndex[i] / 100 + powerOffRestYearIndex[i] / 400) % 7;
		}

		if (W == 5 || W == 6)
		{
			ituCheckBoxSetChecked(powerOffRestSet[i].selectCheckbox, true);
		}
		else
		{
			ituCheckBoxSetChecked(powerOffRestSet[i].selectCheckbox, false);
		}
		
	}

	

	
	lastTick = SDL_GetTicks();
	return true;
}

bool PowerOffTimingOnLeave(ITUWidget* widget, char* param)
{
	int i;
	powerOffTimingSet = false;
	if (ituCheckBoxIsChecked(powerOffTimingSingleCheckBox0))
		powerOffTimingSet = true;
	for (i = 0; i < powerOffWeekTotalItem; i++)
	{
		if (ituCheckBoxIsChecked(powerOffWeekSet[i].selectCheckbox))
			powerOffTimingSet = true;
	}
	for (i = 0; i < powerOffRestTotalItem; i++)
	{
		if (ituCheckBoxIsChecked(powerOffRestSet[i].selectCheckbox))
			powerOffTimingSet = true;
	}

	return true;
}

bool PowerOffTimingOnTimer(ITUWidget* widget, char* param)
{
	int i;
	bool ret = false;

	uint32_t diff, tick = SDL_GetTicks();
	static uint32_t pre_diff = 0;

	if (tick >= lastTick)
		diff = tick - lastTick;
	else
		diff = 0xFFFFFFFF - lastTick + tick;

	if (diff - pre_diff > 50)
	{
		pre_diff = diff;
		if (sliding)
		{
			if (slidingLeft)
			{
				ituWidgetSetX(slidingBackground, ituWidgetGetX(slidingBackground) - 31);
				if (ituWidgetGetX(slidingBackground) == -124)
					sliding = false;
			}
			else
			{
				ituWidgetSetX(slidingBackground, ituWidgetGetX(slidingBackground) + 31);
				if (ituWidgetGetX(slidingBackground) == 0)
					sliding = false;
			}

			ret = true;

		}


	}


	return ret;
}
bool PowerOffTimingSingleSettingSaveBtnOnPress(ITUWidget* widget, char* param)
{

	char tmp[32];

	powerOffSingleTimeHrIndex = powerOffTimingSingleSettingHrWheel->focusIndex;

	if (powerOffSingleTimeHrIndex == 0)
	{
		ituSpriteGoto(powerOffTimingSingleTimeSprite, 0);
	}
	else
	{
		ituSpriteGoto(powerOffTimingSingleTimeSprite, 1);
		sprintf(tmp, "%2.1f", (powerOffSingleTimeHrIndex + 1)*0.5);
		ituTextSetString(powerOffTimingSingleTimeText0, tmp);
	}

	ituCheckBoxSetChecked(powerOffTimingSingleCheckBox0, false);


	return true;
}

bool PowerOffTimingSingleSettingBtnOnMouseUp(ITUWidget* widget, char* param)
{

	ituWheelGoto(powerOffTimingSingleSettingHrWheel, powerOffSingleTimeHrIndex);
	if (powerOffSingleTimeHrIndex == 0)
	{
		ituSpriteGoto(powerOffTimingSingleSettingSprite, 0);
	}
	else
	{
		ituSpriteGoto(powerOffTimingSingleSettingSprite, 1);
	}

	return true;
}

bool PowerOffTimingSingleSettingHrWheelOnChanged(ITUWidget* widget, char* param)
{
	powerOffSingleTimeHrIndex = powerOffTimingSingleSettingHrWheel->focusIndex;

	if (powerOffSingleTimeHrIndex == 0)
	{
		ituSpriteGoto(powerOffTimingSingleSettingSprite, 0);
	}
	else
	{
		ituSpriteGoto(powerOffTimingSingleSettingSprite, 1);
	}

	return true;
}

bool PowerOffTimingSingleChkBoxOnPress(ITUWidget* widget, char* param)
{
//	char tmp[32];
	struct timeval tv;
	struct tm *tm;


	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	if (ituCheckBoxIsChecked(powerOffTimingSingleCheckBox0))
	{
		powerOnTimeIndex = powerOffSingleTimeHrIndex;

		

		if (powerOnTimeIndex > 0)
			powerOnTmHr = powerOnTimeIndex / 2;
		else
			powerOnTmHr = 0;

		if (powerOnTimeIndex % 2 == 0)
			powerOnTmMin = 30;
		else
		{
			powerOnTmMin = 0;
			powerOnTmHr++;
		}
			

		powerOnTmHr = tm->tm_hour + powerOnTmHr;
		powerOnTmMin = tm->tm_min + powerOnTmMin;

		if (powerOnTmMin > 60)
		{
			powerOnTmMin = powerOnTmMin - 60;
			powerOnTmHr++;
		}
		if (powerOnTmHr > 24)
		{
			powerOnTmHr = powerOnTmHr - 24;
		}

	}
	else
	{
		powerOnTimeIndex = -1;
		powerOnTmHr = 0;
		powerOnTmMin = 0;
	}

	return true;
}
bool PowerOffTimingWeekSettingBtnOnMouseUp(ITUWidget* widget, char* param)
{
	int i;
	char tmp[32];
	int dayItemCnt = 0;

	powerOffWeekItemIndex = atoi(param);

	//ituRadioBoxSetChecked(powerOffTimingWeekSettingRadioBox[1], powerOffPowerOn[powerOffWeekItemIndex]);
	//ituRadioBoxSetChecked(powerOffTimingWeekSettingRadioBox[0], !powerOffPowerOn[powerOffWeekItemIndex]);
	//ituIconLinkSurface(&powerOffTimingWeekSettingRadioBoxBackground[1]->icon, powerOffTimingWeekSettingRadBoxBgIcon[powerOffPowerOn[powerOffWeekItemIndex]]);
	//ituIconLinkSurface(&powerOffTimingWeekSettingRadioBoxBackground[0]->icon, powerOffTimingWeekSettingRadBoxBgIcon[!powerOffPowerOn[powerOffWeekItemIndex]]);
	//tmpPowerOffPowerOn = powerOffPowerOn[powerOffWeekItemIndex];
	
	sprintf(tmp, "%02d:%02d", powerOffWeekTimeHrIndex[powerOffWeekItemIndex], powerOffWeekTimeMinIndex[powerOffWeekItemIndex]);
	ituTextSetString(powerOffTimingWeekSettingTimeText, tmp);
	tmpPowerOffWeekTimeHrIndex = powerOffWeekTimeHrIndex[powerOffWeekItemIndex];
	tmpPowerOffWeekTimeMinIndex = powerOffWeekTimeMinIndex[powerOffWeekItemIndex];

	if (powerOffWeekDay[powerOffWeekItemIndex][7])
	{
		ituWidgetSetVisible(powerOffTimingWeekSettingDayText[7], true);
		for (i = 6; i >= 0; i--)
		{
			ituWidgetSetVisible(powerOffTimingWeekSettingDayText[i], false);
			tmpPowerOffWeekDay[i] = powerOffWeekDay[powerOffWeekItemIndex][i];
		}
	}
	else
	{
		ituWidgetSetVisible(powerOffTimingWeekSettingDayText[7], false);
		for (i = 6; i >= 0; i--)
		{
			if (powerOffWeekDay[powerOffWeekItemIndex][i])
			{
				ituWidgetSetVisible(powerOffTimingWeekSettingDayText[i], true);
				ituWidgetSetX(powerOffTimingWeekSettingDayText[i], 336 - (dayItemCnt * 56));
				dayItemCnt++;
			}
			else
			{
				ituWidgetSetVisible(powerOffTimingWeekSettingDayText[i], false);
			}
			tmpPowerOffWeekDay[i] = powerOffWeekDay[powerOffWeekItemIndex][i];
		}
	}
	
	
	//sprintf(tmp, "%02.1f", 18 + (0.5 * powerOffWeekTempIndex[powerOffWeekItemIndex]));
	//ituTextSetString(powerOffTimingWeekSettingTempText, tmp);
	//tmpPowerOffWeekTempIndex = powerOffWeekTempIndex[powerOffWeekItemIndex];

	powerOffTimingWeekStopAnywhere->widget.flags &= ~ITU_DRAGGABLE;

	return true;
}


bool PowerOffTimingWeekSettingDaySaveBtnOnPress(ITUWidget* widget, char* param)
{
	int i;
	int dayItemCnt = 0;

	for (i = 6; i >= 0 ; i--)
	{
		if (ituCheckBoxIsChecked(powerOffTimingWeekSettingDayCheckBox[i]))
		{
			tmpPowerOffWeekDay[i] = 1;
			ituWidgetSetVisible(powerOffTimingWeekSettingDayText[i], true);
			ituWidgetSetX(powerOffTimingWeekSettingDayText[i], 336 - (dayItemCnt * 56));
			dayItemCnt++;
		}
		else
		{
			tmpPowerOffWeekDay[i] = 0;
			ituWidgetSetVisible(powerOffTimingWeekSettingDayText[i], false);
		}
			
	}

	if (dayItemCnt == 7)
	{
		tmpPowerOffWeekDay[7] = 1;
		ituWidgetSetVisible(powerOffTimingWeekSettingDayText[7], true);
		for (i = 0; i < 7; i++)
		{
			ituWidgetSetVisible(powerOffTimingWeekSettingDayText[i], false);
		}
	}
	else
	{
		tmpPowerOffWeekDay[7] = 0;
		ituWidgetSetVisible(powerOffTimingWeekSettingDayText[7], false);
	}

    return true;
}

bool PowerOffTimingWeekSettingTimeSaveBtnOnPress(ITUWidget* widget, char* param)
{

	char tmp[32];

	tmpPowerOffWeekTimeHrIndex = powerOffTimingWeekSettingTimeHrWheel->focusIndex;
	tmpPowerOffWeekTimeMinIndex = powerOffTimingWeekSettingTimeMinWheel->focusIndex;

	sprintf(tmp, "%02d:%02d", tmpPowerOffWeekTimeHrIndex, tmpPowerOffWeekTimeMinIndex);
	ituTextSetString(powerOffTimingWeekSettingTimeText, tmp);

    return true;
}

//bool PowerOffTimingWeekSettingRadBoxOnPress(ITUWidget* widget, char* param)
//{
//	tmpPowerOffPowerOn = atoi(param);
//
//	ituRadioBoxSetChecked(powerOffTimingWeekSettingRadioBox[1], tmpPowerOffPowerOn);
//	ituRadioBoxSetChecked(powerOffTimingWeekSettingRadioBox[0], !tmpPowerOffPowerOn);
//	ituIconLinkSurface(&powerOffTimingWeekSettingRadioBoxBackground[1]->icon, powerOffTimingWeekSettingRadBoxBgIcon[tmpPowerOffPowerOn]);
//	ituIconLinkSurface(&powerOffTimingWeekSettingRadioBoxBackground[0]->icon, powerOffTimingWeekSettingRadBoxBgIcon[!tmpPowerOffPowerOn]);
//
//    return true;
//}

bool PowerOffTimingWeekSettingSaveBtnOnPress(ITUWidget* widget, char* param)
{
	char tmp[32];
	int i;
	int dayItemCnt = 0;
	bool save = atoi(param);

	if (save)
	{
		//powerOffPowerOn[powerOffWeekItemIndex] = tmpPowerOffPowerOn;
		ituSpriteGoto(powerOffWeekSet[powerOffWeekItemIndex].modeTextSprite, 1);

		powerOffWeekTimeHrIndex[powerOffWeekItemIndex] = tmpPowerOffWeekTimeHrIndex;
		powerOffWeekTimeMinIndex[powerOffWeekItemIndex] = tmpPowerOffWeekTimeMinIndex;
		sprintf(tmp, "%02d:%02d", powerOffWeekTimeHrIndex[powerOffWeekItemIndex], powerOffWeekTimeMinIndex[powerOffWeekItemIndex]);
		ituTextSetString(powerOffWeekSet[powerOffWeekItemIndex].timeText, tmp);


		for (i = 0; i < 7; i++)
		{
			powerOffWeekDay[powerOffWeekItemIndex][i] = tmpPowerOffWeekDay[i];

			if (powerOffWeekDay[powerOffWeekItemIndex][i])
			{
				ituWidgetSetVisible(powerOffWeekSet[powerOffWeekItemIndex].dayText[i], true);
				ituWidgetSetX(powerOffWeekSet[powerOffWeekItemIndex].dayText[i], dayItemCnt * 56);
				dayItemCnt++;
			}
			else
			{
				ituWidgetSetVisible(powerOffWeekSet[powerOffWeekItemIndex].dayText[i], false);
			}

		}

		if (dayItemCnt == 7)
		{
			powerOffWeekDay[powerOffWeekItemIndex][7] = 1;
			ituWidgetSetVisible(powerOffWeekSet[powerOffWeekItemIndex].dayText[7], true);
			for (i = 0; i < 7; i++)
			{
				ituWidgetSetVisible(powerOffWeekSet[powerOffWeekItemIndex].dayText[i], false);
			}
		}
		else
		{
			powerOffWeekDay[powerOffWeekItemIndex][7] = 0;
			ituWidgetSetVisible(powerOffWeekSet[powerOffWeekItemIndex].dayText[7], false);
		}

		//WeekTempIndex[powerOffWeekItemIndex] = tmpWeekTempIndex;

		ituCheckBoxSetChecked(powerOffWeekSet[powerOffWeekItemIndex].selectCheckbox, true);

		if (powerOffAddingWeek)
		{
			ituWidgetAdd(powerOffTimingWeekAllContainer, powerOffWeekSet[powerOffWeekTotalItem].weekContainer);
			ituWidgetSetPosition(powerOffWeekSet[powerOffWeekTotalItem].weekContainer, 0, powerOffWeekTotalItem * 150);
			ituWidgetSetY(powerOffTimingWeekAddContainer, (powerOffWeekTotalItem + 1) * 150);
			ituWidgetSetDimension(powerOffTimingWeekAllContainer, 670, (powerOffWeekTotalItem + 1) * 150 + 130);
			powerOffWeekAllContainerHeight = ituWidgetGetHeight(powerOffTimingWeekAllContainer); // = powerOffWeekTotalItem * 150 + 130;
			if (powerOffWeekAllContainerHeight > STOPANYWHERE_H)
				ituWidgetSetDimension(powerOffTimingWeekStopAnywhere, 670, STOPANYWHERE_H);
			else
				ituWidgetSetDimension(powerOffTimingWeekStopAnywhere, 670, powerOffWeekAllContainerHeight);

			ituWidgetUpdate(powerOffTimingWeekAllContainer, ITU_EVENT_LAYOUT, 0, 0, 0);// update the container widget

			powerOffAddingWeek = false;
			powerOffWeekTotalItem++;
		}
	}
	

	powerOffTimingWeekStopAnywhere->widget.flags |= ITU_DRAGGABLE;

    return true;
}
bool PowerOffTimingWeekSettingTimeBtnOnPress(ITUWidget* widget, char* param)
{
	struct timeval tv;
	struct tm *tm;

	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	if (powerOffAddingWeek)
	{

		ituWheelGoto(powerOffTimingWeekSettingTimeHrWheel, tm->tm_hour);
		ituWheelGoto(powerOffTimingWeekSettingTimeMinWheel, tm->tm_min);
	}
	else
	{
		ituWheelGoto(powerOffTimingWeekSettingTimeHrWheel, tmpPowerOffWeekTimeHrIndex);
		ituWheelGoto(powerOffTimingWeekSettingTimeMinWheel, tmpPowerOffWeekTimeMinIndex);
	}
	

	return true;
}

bool PowerOffTimingWeekSettingDayBtnOnPress(ITUWidget* widget, char* param)
{

	int i;

	for (i = 0; i < 7; i++)
		ituCheckBoxSetChecked(powerOffTimingWeekSettingDayCheckBox[i], tmpPowerOffWeekDay[i]);
		
	return true;
}

bool PowerOffTimingWeekAddBtnOnMouseUp(ITUWidget* widget, char* param)
{
	bool ret = false;
	int i;

	

	if (powerOffWeekTotalItem < MAX_WEEK_COUNT)
	{
		powerOffAddingWeek = true;

		ITCTree* node = NULL;
		ITUContainer* cloneContainer = NULL;

		char id[8];

		ituContainerClone((ITUWidget*)powerOffTimingTmpWeekContainer, (ITUWidget**)&cloneContainer);
		powerOffWeekSet[powerOffWeekTotalItem].weekContainer = cloneContainer;
		
		ITUWidget* widget = (ITUWidget*)cloneContainer;

		node = ((ITCTree*)widget)->child;
		ITUButton* deleteButton = (ITUButton*)node;
		powerOffWeekSet[powerOffWeekTotalItem].deleteButton = deleteButton;

		node = node->sibling;
		ITUBackground* background = (ITUBackground*)node;
		powerOffWeekSet[powerOffWeekTotalItem].background = background;

		node = ((ITCTree*)background)->child; 
		ITUButton* slideButton = (ITUButton*)node;
		powerOffWeekSet[powerOffWeekTotalItem].slideButton = slideButton;

		node = node->sibling;
		ITUText* timeText = (ITUText*)node;
		powerOffWeekSet[powerOffWeekTotalItem].timeText = timeText;

		node = node->sibling;
		ITUButton* settingBtn = (ITUButton*)node;
		powerOffWeekSet[powerOffWeekTotalItem].setButton = settingBtn;

		node = node->sibling;
		ITUCheckBox* checkbox = (ITUCheckBox*)node;
		powerOffWeekSet[powerOffWeekTotalItem].selectCheckbox = checkbox;

		node = node->sibling;
		ITUSprite* sprite = (ITUSprite*)node;
		powerOffWeekSet[powerOffWeekTotalItem].modeTextSprite = sprite;

		node = node->sibling;
		ITUContainer* textContainer = (ITUContainer*)node;
		powerOffWeekSet[powerOffWeekTotalItem].dayTextContainer = textContainer;

		node = ((ITCTree*)textContainer)->child;
		ITUText* dayText[8] = { 0 };
		for (i = 0; i < 8; i++)
		{
			dayText[i] = (ITUText*)node;
			powerOffWeekSet[powerOffWeekTotalItem].dayText[i] = dayText[i];
			node = node->sibling;
		}
		
		sprintf(id, "%d", powerOffWeekTotalItem);
		strcpy(settingBtn->actions[1].param, id);
		strcpy(settingBtn->actions[5].param, id);
		strcpy(settingBtn->actions[6].param, id);
		strcpy(checkbox->btn.actions[0].param, id);
		strcpy(deleteButton->actions[0].param, id);
		strcpy(slideButton->actions[0].param, id);
		strcpy(slideButton->actions[1].param, id); 

		ituWidgetSetVisible(powerOffTimingWeekSettingBackground, true);
		PowerOffTimingWeekSettingBtnOnMouseUp(NULL, settingBtn->actions[1].param);

		ret = true;

	}
    return ret;
}

bool PowerOffTimingWeekChkBoxOnPress(ITUWidget* widget, char* param)
{
    return true;
}

bool PowerOffTimingWeekSlideBtnOnSlideLeft(ITUWidget* widget, char* param)
{
	int slideIndex = atoi(param);

	sliding = true;
	slidingLeft = true;
	slidingBackground = (ITUBackground*)powerOffWeekSet[slideIndex].background;
	//ituWidgetSetX(powerOffWeekSet[slideIndex].background, -124);

	return true;
}


bool PowerOffTimingWeekSlideBtnOnSlideRight(ITUWidget* widget, char* param)
{
	int slideIndex = atoi(param);
	sliding = true;
	slidingLeft = false;
	slidingBackground = (ITUBackground*)powerOffWeekSet[slideIndex].background;
	//ituWidgetSetX(powerOffWeekSet[slideIndex].background, 0);

	return true;
}

bool PowerOffTimingWeekDeleteBtnOnPress(ITUWidget* widget, char* param)
{
	int deleteIndex = atoi(param);
	int i,j;
	char tmp[32];

	ituWidgetSetVisible(powerOffWeekSet[deleteIndex].weekContainer, false);

	for (i = deleteIndex; i < (powerOffWeekTotalItem - 1); i++)
	{
		ituWidgetSetY(powerOffWeekSet[i + 1].weekContainer, i * 150);
		
		sprintf(tmp, "%d", i);
		strcpy(powerOffWeekSet[i + 1].setButton->actions[1].param, tmp);
		strcpy(powerOffWeekSet[i + 1].setButton->actions[5].param, tmp);
		strcpy(powerOffWeekSet[i + 1].setButton->actions[6].param, tmp);
		strcpy(powerOffWeekSet[i + 1].selectCheckbox->btn.actions[0].param, tmp);
		strcpy(powerOffWeekSet[i + 1].deleteButton->actions[0].param, tmp);
		strcpy(powerOffWeekSet[i + 1].slideButton->actions[0].param, tmp);
		strcpy(powerOffWeekSet[i + 1].slideButton->actions[1].param, tmp);

		//powerOffPowerOn[i] = powerOffPowerOn[i + 1];
		powerOffWeekTimeHrIndex[i] = powerOffWeekTimeHrIndex[i + 1];
		powerOffWeekTimeMinIndex[i] =powerOffWeekTimeMinIndex[i + 1];

		for (j = 0; j < 8; j++)
		{
			powerOffWeekDay[i][j] = powerOffWeekDay[i + 1][j];
		}
		//WeekTempIndex[i] = WeekTempIndex[i + 1];
		powerOffWeekSet[i] = powerOffWeekSet[i + 1];
	}

	

	//powerOffPowerOn[powerOffWeekTotalItem - 1] = false;
	powerOffWeekTimeHrIndex[powerOffWeekTotalItem - 1] = 0;
	powerOffWeekTimeMinIndex[powerOffWeekTotalItem - 1] = 0;
	for (j = 0; j < 8; j++)
	{
		powerOffWeekDay[powerOffWeekTotalItem - 1][j] = 0;
	}
	//WeekTempIndex[powerOffWeekTotalItem-1] = 0;


	powerOffWeekTotalItem--;
	
	ituWidgetSetY(powerOffTimingWeekAddContainer, powerOffWeekTotalItem * 150);
	ituWidgetSetDimension(powerOffTimingWeekAllContainer, 670, (powerOffWeekTotalItem)* 150 + 130);
	powerOffWeekAllContainerHeight = ituWidgetGetHeight(powerOffTimingWeekAllContainer); // = powerOffWeekTotalItem * 150 + 130;
	if (powerOffWeekAllContainerHeight > STOPANYWHERE_H)
		ituWidgetSetDimension(powerOffTimingWeekStopAnywhere, 670, STOPANYWHERE_H);
	else
		ituWidgetSetDimension(powerOffTimingWeekStopAnywhere, 670, powerOffWeekAllContainerHeight);

	ituWidgetUpdate(powerOffTimingWeekAllContainer, ITU_EVENT_LAYOUT, 0, 0, 0);// update the container widget

	

	return true;
}

bool PowerOffTimingRestSettingSaveBtnOnPress(ITUWidget* widget, char* param)
{
	char tmp[32];
	bool save = atoi(param);
	struct timeval tv;
	struct tm *tm;

	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	if (save)
	{
		if (powerOffTimingRestSettingMonthWheel->focusIndex < tm->tm_mon)
			powerOffRestYearIndex[powerOffRestTotalItem] = tm->tm_year + 1900 + 1;
		else
		{
			if (powerOffTimingRestSettingDayWheel->focusIndex < (tm->tm_mday - 1))
				powerOffRestYearIndex[powerOffRestTotalItem] = tm->tm_year + 1900 + 1;
			else
				powerOffRestYearIndex[powerOffRestTotalItem] = tm->tm_year + 1900;
		}
			//powerOffRestYearIndex[powerOffRestTotalItem] = tm->tm_year + 1900;

		//if ((powerOffTimingRestSettingYearWheel->focusIndex % 4) == 0)
		if ((powerOffRestYearIndex[powerOffRestTotalItem] % 4) == 0)
		{
			if (powerOffTimingRestSettingMonthWheel->focusIndex == 1)
			{
				if (powerOffTimingRestSettingDayWheel->focusIndex > 28)
					ituWheelGoto(powerOffTimingRestSettingDayWheel, 28);
			}
		}

		else
		{
			if (powerOffTimingRestSettingMonthWheel->focusIndex == 1)
			{
				if (powerOffTimingRestSettingDayWheel->focusIndex > 27)
					ituWheelGoto(powerOffTimingRestSettingDayWheel, 27);
			}
		}


		if ((powerOffTimingRestSettingMonthWheel->focusIndex == 3) || (powerOffTimingRestSettingMonthWheel->focusIndex == 5) || (powerOffTimingRestSettingMonthWheel->focusIndex == 8) || (powerOffTimingRestSettingMonthWheel->focusIndex == 10))
		{
			if (powerOffTimingRestSettingDayWheel->focusIndex > 29)
				ituWheelGoto(powerOffTimingRestSettingDayWheel, 29);
		}


		//powerOffRestYearIndex[powerOffRestTotalItem] = timingRestSettingYearWheel->focusIndex + 2020;
		sprintf(tmp, "%04d", powerOffRestYearIndex[powerOffRestTotalItem]);
		ituTextSetString(powerOffRestSet[powerOffRestTotalItem].yearText, tmp);

		powerOffRestMonthIndex[powerOffRestTotalItem] = powerOffTimingRestSettingMonthWheel->focusIndex + 1;
		sprintf(tmp, "%02d", powerOffRestMonthIndex[powerOffRestTotalItem]);
		ituTextSetString(powerOffRestSet[powerOffRestTotalItem].monthText, tmp);

		powerOffRestDayIndex[powerOffRestTotalItem] = powerOffTimingRestSettingDayWheel->focusIndex + 1;
		sprintf(tmp, "%02d", powerOffRestDayIndex[powerOffRestTotalItem]);
		ituTextSetString(powerOffRestSet[powerOffRestTotalItem].dayText, tmp);

		ituCheckBoxSetChecked(powerOffRestSet[powerOffRestTotalItem].selectCheckbox, true);

		if (powerOffAddingRest)
		{
			ituWidgetAdd(powerOffTimingRestAllContainer, powerOffRestSet[powerOffRestTotalItem].restContainer);
			ituWidgetSetPosition(powerOffRestSet[powerOffRestTotalItem].restContainer, 0, powerOffRestTotalItem * 94);
			ituWidgetSetY(powerOffTimingRestAddContainer, (powerOffRestTotalItem + 1) * 94);
			ituWidgetSetDimension(powerOffTimingRestAllContainer, 670, (powerOffRestTotalItem + 1) * 94 + 88);
			powerOffRestAllContainerHeight = ituWidgetGetHeight(powerOffTimingRestAllContainer); // = powerOffWeekTotalItem * 150 + 130;
			if (powerOffRestAllContainerHeight > STOPANYWHERE_H)
				ituWidgetSetDimension(powerOffTimingRestStopAnywhere, 670, STOPANYWHERE_H);
			else
				ituWidgetSetDimension(powerOffTimingRestStopAnywhere, 670, powerOffRestAllContainerHeight);

			ituWidgetUpdate(powerOffTimingRestAllContainer, ITU_EVENT_LAYOUT, 0, 0, 0);// update the container widget


			powerOffAddingRest = false;
			powerOffRestTotalItem++;
		}
	}

	
	powerOffTimingRestStopAnywhere->widget.flags |= ITU_DRAGGABLE;

	return true;
}

bool PowerOffTimingRestAddBtnOnMouseUp(ITUWidget* widget, char* param)
{
	bool ret = false;
//	int i;
	struct timeval tv;
	struct tm *tm;

	if (powerOffRestTotalItem < MAX_REST_COUNT)
	{
		powerOffAddingRest = true;
		ITCTree* node = NULL;
		ITUContainer* cloneContainer = NULL;

		char id[8];

		ituContainerClone((ITUWidget*)powerOffTimingTmpRestContainer, (ITUWidget**)&cloneContainer);
		powerOffRestSet[powerOffRestTotalItem].restContainer = cloneContainer;
		
		ITUWidget* widget = (ITUWidget*)cloneContainer;

		node = ((ITCTree*)widget)->child;
		ITUButton* deleteButton = (ITUButton*)node;
		powerOffRestSet[powerOffRestTotalItem].deleteButton = deleteButton;

		node = node->sibling;
		ITUBackground* background = (ITUBackground*)node;
		powerOffRestSet[powerOffRestTotalItem].background = background;

		node = ((ITCTree*)background)->child;
		ITUButton* slideButton = (ITUButton*)node;
		powerOffRestSet[powerOffRestTotalItem].slideButton = slideButton;

		node = node->sibling;
		ITUText* text = (ITUText*)node;
		powerOffRestSet[powerOffRestTotalItem].yearText = text;

		node = node->sibling;

		node = node->sibling;
		text = (ITUText*)node;
		powerOffRestSet[powerOffRestTotalItem].monthText = text;

		node = node->sibling;

		node = node->sibling;
		text = (ITUText*)node;
		powerOffRestSet[powerOffRestTotalItem].dayText = text;

		node = node->sibling;

		node = node->sibling;
		ITUCheckBox* checkbox = (ITUCheckBox*)node;
		powerOffRestSet[powerOffRestTotalItem].selectCheckbox = checkbox;

		sprintf(id, "%d", powerOffRestTotalItem);
		strcpy(checkbox->btn.actions[0].param, id);
		strcpy(deleteButton->actions[0].param, id);
		strcpy(slideButton->actions[0].param, id);
		strcpy(slideButton->actions[1].param, id);




		gettimeofday(&tv, NULL);
		tm = localtime(&tv.tv_sec);

		//ituWheelGoto(powerOffTimingRestSettingYearWheel, tm->tm_year - 120);//2020-1900
		ituWheelGoto(powerOffTimingRestSettingMonthWheel, tm->tm_mon);//0-11
		ituWheelGoto(powerOffTimingRestSettingDayWheel, tm->tm_mday - 1);//1-31

		ituWidgetSetVisible(powerOffTimingRestSettingBackground, true);

		ret = true;

		powerOffTimingRestStopAnywhere->widget.flags &= ~ITU_DRAGGABLE;
	}

	return ret;
}

bool PowerOffTimingRestChkBoxOnPress(ITUWidget* widget, char* param)
{
	return true;
}

bool PowerOffTimingRestSettingWheelOnChanged(ITUWidget* widget, char* param)
{
	struct timeval tv;
	struct tm *tm;

	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	if (powerOffTimingRestSettingMonthWheel->focusIndex < tm->tm_mon)
		powerOffRestYearIndex[powerOffRestTotalItem] = tm->tm_year +1900 + 1;
	else
	{
		if (powerOffTimingRestSettingDayWheel->focusIndex < (tm->tm_mday - 1))
			powerOffRestYearIndex[powerOffRestTotalItem] = tm->tm_year + 1900 + 1;
		else
			powerOffRestYearIndex[powerOffRestTotalItem] = tm->tm_year + 1900;
	}
		//powerOffRestYearIndex[powerOffRestTotalItem] = tm->tm_year +1900;

	

	//if ((powerOffTimingRestSettingYearWheel->focusIndex % 4) == 0)
	if ((powerOffRestYearIndex[powerOffRestTotalItem] % 4) == 0)
	{
	if (powerOffTimingRestSettingMonthWheel->focusIndex == 1)
		{
		if (powerOffTimingRestSettingDayWheel->focusIndex > 28)
			ituWheelGoto(powerOffTimingRestSettingDayWheel, 28);
		}				
	}				
		
	else
	{
		if (powerOffTimingRestSettingMonthWheel->focusIndex == 1)
		{
			if (powerOffTimingRestSettingDayWheel->focusIndex > 27)
				ituWheelGoto(powerOffTimingRestSettingDayWheel, 27);
		}
	}
		

	if ((powerOffTimingRestSettingMonthWheel->focusIndex == 3) || (powerOffTimingRestSettingMonthWheel->focusIndex == 5) || (powerOffTimingRestSettingMonthWheel->focusIndex == 8) || (powerOffTimingRestSettingMonthWheel->focusIndex == 10))
	{
		if (powerOffTimingRestSettingDayWheel->focusIndex > 29)
			ituWheelGoto(powerOffTimingRestSettingDayWheel, 29);
	}
	
	return true;
}

bool PowerOffTimingRestSlideBtnOnSlideLeft(ITUWidget* widget, char* param)
{
	int slideIndex = atoi(param);
	sliding = true;
	slidingLeft = true;
	slidingBackground = (ITUBackground*)powerOffRestSet[slideIndex].background;
	//ituWidgetSetX(powerOffRestSet[slideIndex].background, -124);

	return true;
}


bool PowerOffTimingRestSlideBtnOnSlideRight(ITUWidget* widget, char* param)
{
	int slideIndex = atoi(param);
	sliding = true;
	slidingLeft = false;
	slidingBackground = (ITUBackground*)powerOffRestSet[slideIndex].background;
	//ituWidgetSetX(powerOffRestSet[slideIndex].background, 0);

	return true;
}

bool PowerOffTimingRestDeleteBtnOnPress(ITUWidget* widget, char* param)
{
	int deleteIndex = atoi(param);
	int i;
	char tmp[32];

	ituWidgetSetVisible(powerOffRestSet[deleteIndex].restContainer, false);

	for (i = deleteIndex; i < (powerOffRestTotalItem - 1); i++)
	{
		ituWidgetSetY(powerOffRestSet[i + 1].restContainer, i * 94);

		sprintf(tmp, "%d", i);
		strcpy(powerOffRestSet[i + 1].selectCheckbox->btn.actions[0].param, tmp);
		strcpy(powerOffRestSet[i + 1].deleteButton->actions[0].param, tmp);
		strcpy(powerOffRestSet[i + 1].slideButton->actions[0].param, tmp);
		strcpy(powerOffRestSet[i + 1].slideButton->actions[1].param, tmp);

		powerOffRestYearIndex[i] = powerOffRestYearIndex[i + 1];
		powerOffRestMonthIndex[i] = powerOffRestMonthIndex[i + 1];
		powerOffRestDayIndex[i] = powerOffRestDayIndex[i + 1];

		powerOffRestSet[i] = powerOffRestSet[i + 1];
	}


	powerOffRestYearIndex[powerOffRestTotalItem - 1] = 0;
	powerOffRestMonthIndex[powerOffRestTotalItem - 1] = 0;
	powerOffRestDayIndex[powerOffRestTotalItem - 1] = 0;

	powerOffRestTotalItem--;

	ituWidgetSetY(powerOffTimingRestAddContainer, powerOffRestTotalItem * 94);
	ituWidgetSetDimension(powerOffTimingRestAddContainer, 670, (powerOffRestTotalItem)* 94 + 88);
	powerOffRestAllContainerHeight = ituWidgetGetHeight(powerOffTimingRestAddContainer);
	if (powerOffRestAllContainerHeight > STOPANYWHERE_H)
		ituWidgetSetDimension(powerOffTimingRestStopAnywhere, 670, STOPANYWHERE_H);
	else
		ituWidgetSetDimension(powerOffTimingRestStopAnywhere, 670, powerOffRestAllContainerHeight);

	ituWidgetUpdate(powerOffTimingRestAddContainer, ITU_EVENT_LAYOUT, 0, 0, 0);// update the container widget


	return true;
}

