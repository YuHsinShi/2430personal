#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"

ITUSprite* warningBackgroundSprite = 0;


bool WarningOnEnter(ITUWidget* widget, char* param)
{
	if (!warningBackgroundSprite)
	{
		warningBackgroundSprite = ituSceneFindWidget(&theScene, "warningBackgroundSprite");
		assert(warningBackgroundSprite);
	}

	ituSpriteGoto(warningBackgroundSprite, BgIndex[modeIndex]);
	return true;
}

bool WarningResetBtnOnPress(ITUWidget* widget, char* param)
{
	struct timeval tv;
	struct tm *tm;

	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	warning_btn_show = false;
	if (examineRecordTotalItem < EXAM_REC_MAX_ITEM)
	{
		examineDayRecord[examineRecordTotalItem][0] = tm->tm_year + 1900;
		examineDayRecord[examineRecordTotalItem][1] = tm->tm_mon + 1;
		examineDayRecord[examineRecordTotalItem][2] = tm->tm_mday;
		examineTimeRecord[examineRecordTotalItem][0] = tm->tm_hour;
		examineTimeRecord[examineRecordTotalItem][1] = tm->tm_min;
		examineAddrRecord[examineRecordTotalItem] = 1;
		examineNoRecord[examineRecordTotalItem] = 35;
		examineRecordTotalItem++;
	}
		

    return true;
}

