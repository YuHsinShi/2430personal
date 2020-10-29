#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"



ITUStopAnywhere* settingStopAnywhere = 0;

ITUWheel* settingTimeYearWheel = 0;
ITUWheel* settingTimeMonthWheel = 0;
ITUWheel* settingTimeDayWheel = 0;
ITUWheel* settingTimeHrWheel = 0;
ITUWheel* settingTimeMinWheel = 0;
ITUText* settingTimeText = 0;

ITUCheckBox* settingLightAutoCheckBox = 0;

ITUProgressBar* settingScreenLightProgressBar = 0;
ITUTrackBar* settingScreenLightTrackBar = 0;
ITUProgressBar* settingIndLightProgressBar = 0;
ITUTrackBar* settingIndLightTrackBar = 0;
ITUIcon* settingLightTopBarBtnIcon[2] = { 0 };

ITUSprite* settingScreenLockTimeSprite = 0;
ITURadioBox* settingScreenLockRadioBox[3] = { 0 };


bool SettingOnEnter(ITUWidget* widget, char* param)
{
	struct timeval tv;
	struct tm *tm;
	char tmp[32];
	int i;

	if (!settingStopAnywhere)
	{
		settingStopAnywhere = ituSceneFindWidget(&theScene, "settingStopAnywhere");
		assert(settingStopAnywhere);

		settingTimeYearWheel = ituSceneFindWidget(&theScene, "settingTimeYearWheel");
		assert(settingTimeYearWheel);

		settingTimeMonthWheel = ituSceneFindWidget(&theScene, "settingTimeMonthWheel");
		assert(settingTimeMonthWheel);

		settingTimeDayWheel = ituSceneFindWidget(&theScene, "settingTimeDayWheel");
		assert(settingTimeDayWheel);

		settingTimeHrWheel = ituSceneFindWidget(&theScene, "settingTimeHrWheel");
		assert(settingTimeHrWheel);

		settingTimeMinWheel = ituSceneFindWidget(&theScene, "settingTimeMinWheel");
		assert(settingTimeMinWheel);

		settingTimeText = ituSceneFindWidget(&theScene, "settingTimeText");
		assert(settingTimeText);

		settingLightAutoCheckBox = ituSceneFindWidget(&theScene, "settingLightAutoCheckBox");
		assert(settingLightAutoCheckBox);

		settingScreenLightProgressBar = ituSceneFindWidget(&theScene, "settingScreenLightProgressBar");
		assert(settingScreenLightProgressBar);

		settingScreenLightTrackBar = ituSceneFindWidget(&theScene, "settingScreenLightTrackBar");
		assert(settingScreenLightTrackBar);

		settingIndLightProgressBar = ituSceneFindWidget(&theScene, "settingIndLightProgressBar");
		assert(settingIndLightProgressBar);

		settingIndLightTrackBar = ituSceneFindWidget(&theScene, "settingIndLightTrackBar");
		assert(settingIndLightTrackBar);

		settingScreenLockTimeSprite = ituSceneFindWidget(&theScene, "settingScreenLockTimeSprite");
		assert(settingScreenLockTimeSprite);

		for (i = 0; i < 2; i++)
		{
			sprintf(tmp, "settingLightTopBarBtnIcon%d", i);
			settingLightTopBarBtnIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(settingLightTopBarBtnIcon[i]);
		}

		for (i = 0; i < 3; i++)
		{
			sprintf(tmp, "settingScreenLockRadioBox%d", i);
			settingScreenLockRadioBox[i] = ituSceneFindWidget(&theScene, tmp);
			assert(settingScreenLockRadioBox[i]);
		}
	}

	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	sprintf(tmp, "%04d-%02d-%02d %02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min);
	ituTextSetString(settingTimeText, tmp);

	for (i = 0; i < 3; i++)
	{
		ituRadioBoxSetChecked(settingScreenLockRadioBox[i], false);
	}
	if (theConfig.screensaver_time == 60)
	{
		ituRadioBoxSetChecked(settingScreenLockRadioBox[0], true);
		ituSpriteGoto(settingScreenLockTimeSprite, 0);
	}
	else if (theConfig.screensaver_time == 180)
	{
		ituRadioBoxSetChecked(settingScreenLockRadioBox[1], true);
		ituSpriteGoto(settingScreenLockTimeSprite, 1);
	}
	else
	{
		ituRadioBoxSetChecked(settingScreenLockRadioBox[2], true);
		ituSpriteGoto(settingScreenLockTimeSprite, 2);
	}



	return true;
}
bool SettingTimeBtnOnPress(ITUWidget* widget, char* param)
{


	struct timeval tv;
	struct tm *tm;

	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	ituWheelGoto(settingTimeYearWheel, tm->tm_year - 120);//2020-1900
	ituWheelGoto(settingTimeMonthWheel, tm->tm_mon);//0-11
	ituWheelGoto(settingTimeDayWheel, tm->tm_mday-1);//1-31
	ituWheelGoto(settingTimeHrWheel, tm->tm_hour);
	ituWheelGoto(settingTimeMinWheel, tm->tm_min);

	settingStopAnywhere->widget.flags &= ~ITU_DRAGGABLE;

	return true;
}

bool SettingTimeWheelOnChanged(ITUWidget* widget, char* param)
{
	if ((settingTimeYearWheel->focusIndex % 4) == 0)
	{
		if (settingTimeMonthWheel->focusIndex == 1)
		{
			if (settingTimeDayWheel->focusIndex > 28)
				ituWheelGoto(settingTimeDayWheel, 28);
		}
	}

	else
	{
		if (settingTimeMonthWheel->focusIndex == 1)
		{
			if (settingTimeDayWheel->focusIndex > 27)
				ituWheelGoto(settingTimeDayWheel, 27);
		}
	}


	if ((settingTimeMonthWheel->focusIndex == 3) || (settingTimeMonthWheel->focusIndex == 5) || (settingTimeMonthWheel->focusIndex == 8) || (settingTimeMonthWheel->focusIndex == 10))
	{
		if (settingTimeDayWheel->focusIndex > 29)
			ituWheelGoto(settingTimeDayWheel, 29);
	}
	return true;
}

bool SettingTimeSaveBtnOnPress(ITUWidget* widget, char* param)
{
	bool save = atoi(param);
	char tmp[32];

	if (save)
	{
		struct timeval tv;
		struct tm *tm, mytime;


		if ((settingTimeYearWheel->focusIndex % 4) == 0)
		{
			if (settingTimeMonthWheel->focusIndex == 1)
			{
				if (settingTimeDayWheel->focusIndex > 28)
					ituWheelGoto(settingTimeDayWheel, 28);
			}
		}

		else
		{
			if (settingTimeMonthWheel->focusIndex == 1)
			{
				if (settingTimeDayWheel->focusIndex > 27)
					ituWheelGoto(settingTimeDayWheel, 27);
			}
		}


		if ((settingTimeMonthWheel->focusIndex == 3) || (settingTimeMonthWheel->focusIndex == 5) || (settingTimeMonthWheel->focusIndex == 8) || (settingTimeMonthWheel->focusIndex == 10))
		{
			if (settingTimeDayWheel->focusIndex > 29)
				ituWheelGoto(settingTimeDayWheel, 29);
		}

		sprintf(tmp, "%04d-%02d-%02d %02d:%02d", settingTimeYearWheel->focusIndex + 2020,
			settingTimeMonthWheel->focusIndex + 1,
			settingTimeDayWheel->focusIndex + 1,
			settingTimeHrWheel->focusIndex,
			settingTimeMinWheel->focusIndex);
		ituTextSetString(settingTimeText, tmp);

		//set time
		gettimeofday(&tv, NULL);
		tm = localtime(&tv.tv_sec);

		memcpy(&mytime, tm, sizeof (struct tm));

		mytime.tm_year = settingTimeYearWheel->focusIndex + 120;
		mytime.tm_mon = settingTimeMonthWheel->focusIndex;
		mytime.tm_mday = settingTimeDayWheel->focusIndex + 1;
		mytime.tm_hour = settingTimeHrWheel->focusIndex;
		mytime.tm_min = settingTimeMinWheel->focusIndex;


		tv.tv_sec = mktime(&mytime);
		tv.tv_usec = 0;

		settimeofday(&tv, NULL);
	}

	settingStopAnywhere->widget.flags |= ITU_DRAGGABLE;

    return true;
}

bool SettingLightBtnOnPress(ITUWidget* widget, char* param)
{

	if (ituCheckBoxIsChecked(settingLightAutoCheckBox))
	{
		ituIconLinkSurface(&settingScreenLightTrackBar->tracker->bg.icon, settingLightTopBarBtnIcon[0]);
		ituIconLinkSurface(&settingIndLightTrackBar->tracker->bg.icon, settingLightTopBarBtnIcon[0]);

		ituWidgetDisable(settingScreenLightTrackBar);
		ituWidgetSetVisible(settingScreenLightProgressBar, false);
		ituWidgetDisable(settingIndLightTrackBar);
		ituWidgetSetVisible(settingIndLightProgressBar, false);
	}
	else
	{
		ituIconLinkSurface(&settingScreenLightTrackBar->tracker->bg.icon, settingLightTopBarBtnIcon[1]);
		ituIconLinkSurface(&settingIndLightTrackBar->tracker->bg.icon, settingLightTopBarBtnIcon[1]);

		ituWidgetEnable(settingScreenLightTrackBar);
		ituWidgetSetVisible(settingScreenLightProgressBar, true);
		ituWidgetEnable(settingIndLightTrackBar);
		ituWidgetSetVisible(settingIndLightProgressBar, true);
	}


	settingStopAnywhere->widget.flags &= ~ITU_DRAGGABLE;

    return true;
}

bool SettingLightBtnOnSlideUp(ITUWidget* widget, char* param)
{
	settingStopAnywhere->widget.flags |= ITU_DRAGGABLE;

	return true;
}

bool SettingScreenLockBtnOnPress(ITUWidget* widget, char* param)
{
	ituRadioBoxSetChecked(settingScreenLockRadioBox[0], false);
	ituRadioBoxSetChecked(settingScreenLockRadioBox[1], false);
	ituRadioBoxSetChecked(settingScreenLockRadioBox[2], false);

	ituRadioBoxSetChecked(settingScreenLockRadioBox[settingScreenLockTimeSprite->frame], true);

	settingStopAnywhere->widget.flags &= ~ITU_DRAGGABLE;
	return true;
}

bool SettingScreenLockSaveBtnOnPress(ITUWidget* widget, char* param)
{
	bool save = atoi(param);

	if (save)
	{
		if (ituRadioBoxIsChecked(settingScreenLockRadioBox[0]))
		{
			ituSpriteGoto(settingScreenLockTimeSprite, 0);
			theConfig.screensaver_time = 60;
		}
		else if (ituRadioBoxIsChecked(settingScreenLockRadioBox[1]))
		{
			ituSpriteGoto(settingScreenLockTimeSprite, 1);
			theConfig.screensaver_time = 180;
		}
		else
		{
			ituSpriteGoto(settingScreenLockTimeSprite, 2);
			theConfig.screensaver_time = 600;
		}
		ConfigSave();
			
	}
	

	settingStopAnywhere->widget.flags |= ITU_DRAGGABLE;
	return true;
}

bool SettingScreenLockRadBoxOnPress(ITUWidget* widget, char* param)
{
	int radioboxIndex = atoi(param);

	switch (radioboxIndex)
	{
	case 0:
		ituRadioBoxSetChecked(settingScreenLockRadioBox[0], true);
		ituRadioBoxSetChecked(settingScreenLockRadioBox[1], false);
		ituRadioBoxSetChecked(settingScreenLockRadioBox[2], false);
		break;
	case 1:
		ituRadioBoxSetChecked(settingScreenLockRadioBox[0], false);
		ituRadioBoxSetChecked(settingScreenLockRadioBox[1], true);
		ituRadioBoxSetChecked(settingScreenLockRadioBox[2], false);
		break;
	case 2:
		ituRadioBoxSetChecked(settingScreenLockRadioBox[0], false);
		ituRadioBoxSetChecked(settingScreenLockRadioBox[1], false);
		ituRadioBoxSetChecked(settingScreenLockRadioBox[2], true);
		break;
	}

	return true;
}