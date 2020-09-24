#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
/* widgets:
Layer_Settings
Background5
Text_settingChNum
Button41
Background27
Text28
Text_settingTimeInterval
Text30
Text_settingTimeStamp
Text36
Text_settingFileNum
Text39
Text_settingFilesize
Background26
Text35
Text_settingDatabit
Text21
Text_settingStopbit
Text23
Text_settingParity
Text25
Text_settingBaud
Button73
Button72
Text33
*/

#if 1 //setting layer code

static ITUText* Text_settingChNum;
static ITUText* Text_settingTimeInterval;
static ITUText* Text_settingTimeStamp;
static ITUText* Text_settingFileNum;
static ITUText* Text_settingFilesize;
static ITUText* Text_settingDatabit;
static ITUText* Text_settingStopbit;
static ITUText* Text_settingParity;
static ITUText* Text_settingBaud; 
int index_selected;


void ituTextSetStringInt(ITUText* widget, int value)
{
	char str[8] = { 0 };
	snprintf(str, 8, "%d", value);
	ituTextSetString(widget,str);
}

void setting_para_update()
{
	ituTextSetStringInt(Text_settingChNum, index_selected+1);


	ituTextSetStringInt(Text_settingTimeInterval, uart[index_selected].fileInterval);
	ituTextSetStringInt(Text_settingTimeStamp, uart[index_selected].timestamp);
	ituTextSetStringInt(Text_settingFileNum, uart[index_selected].fileNum);
	ituTextSetStringInt(Text_settingFilesize, uart[index_selected].fileMaxsize);
	ituTextSetStringInt(Text_settingDatabit, uart[index_selected].databit);
	ituTextSetStringInt(Text_settingStopbit, uart[index_selected].stopbit);
	ituTextSetStringInt(Text_settingBaud, uart[index_selected].baud_rate);
	
	ituTextSetString(Text_settingParity, uart[index_selected].parity);




}

bool Setting_next(ITUWidget* widget, char* param)
{
	if (index_selected == 4)
		return false;

	index_selected++;
	setting_para_update();
	return true;
}

bool Setting_prev(ITUWidget* widget, char* param)
{
	if (index_selected==0)
		return false;
	index_selected--;
	setting_para_update();
	return true;
}


bool SettingOnEnter(ITUWidget* widget, char* param)
{
	
	

	Text_settingChNum = ituSceneFindWidget(&theScene, "Text_settingChNum");
	assert(Text_settingChNum);


	Text_settingTimeInterval= ituSceneFindWidget(&theScene, "Text_settingTimeInterval");
	assert(Text_settingTimeInterval);
	Text_settingTimeStamp = ituSceneFindWidget(&theScene, "Text_settingTimeStamp");
	assert(Text_settingTimeStamp);
	Text_settingTimeInterval = ituSceneFindWidget(&theScene, "Text_settingTimeInterval");
	assert(Text_settingTimeInterval);
	Text_settingFileNum = ituSceneFindWidget(&theScene, "Text_settingFileNum");
	assert(Text_settingFileNum);
	Text_settingFilesize = ituSceneFindWidget(&theScene, "Text_settingFilesize");
	assert(Text_settingFilesize);
	Text_settingDatabit = ituSceneFindWidget(&theScene, "Text_settingDatabit");
	assert(Text_settingDatabit);
	Text_settingStopbit = ituSceneFindWidget(&theScene, "Text_settingStopbit");
	assert(Text_settingStopbit);
	Text_settingParity = ituSceneFindWidget(&theScene, "Text_settingParity");
	assert(Text_settingParity);
	Text_settingBaud = ituSceneFindWidget(&theScene, "Text_settingBaud");
	assert(Text_settingBaud);	
	
	
	index_selected = 0;

	setting_para_update();


	//burn_writer_start();
	
	return false;
}


#endif