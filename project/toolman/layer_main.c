#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"


#define LOG_DISK_TEST
#if defined(LOG_DISK_TEST)
#include "ite/itp.h"

static bool bLogDisk = false;
#endif


static ITUScaleCoverFlow* ScaleCoverFlowMain;

void MainReset(void)
{
}


bool MainSelectedPressed(ITUWidget* widget, char* param)
{
	printf("==%d== ", ScaleCoverFlowMain->coverFlow.focusIndex);
	switch (ScaleCoverFlowMain->coverFlow.focusIndex)
	{
		case 0:

			ituLayerGoto(ituSceneFindWidget(&theScene, "Layer_Power"));
			break;
		case 1:
			ituLayerGoto(ituSceneFindWidget(&theScene, "Layer_Burner"));

			break;
		case 2:
			ituLayerGoto(ituSceneFindWidget(&theScene, "Layer_Edit"));

			break;
		case 3:
			ituLayerGoto(ituSceneFindWidget(&theScene, "Layer_Settings"));

			break;
		case 4:
			ituLayerGoto(ituSceneFindWidget(&theScene, "Layer_uartcapture"));
			break;

		case 5:
			ituLayerGoto(ituSceneFindWidget(&theScene, "Layer_TimerRelayOnOff"));
			break;
	}
	return true;
}

bool MainOnEnter(ITUWidget* widget, char* param)
{
	
	ScaleCoverFlowMain = ituSceneFindWidget(&theScene, "ScaleCoverFlowMain");


	return true;
}

bool MainButtonOnMouseUp(ITUWidget* widget, char* param)
{
		
	return true;


}

#if 1
static ITUBackground* BackgroundWarning;
static ITUText* uart_baud[6];
static ITUText* uart_status[6];
static ITUText* uart_files_no[6];
static ITUSprite* uart_light[6];

static int iuart_counter[6];

bool UartCaptureOnLeave(ITUWidget* widget, char* param)
{
	log_writer_stop();

	return false;
}

bool UartCaptureOnTimer(ITUWidget* widget, char* param)
{

	static int counter;
	counter++;
	if (0 != counter % 4)
		return false;

	int i;
	for (i = 1; i <= 5; i++)
	{
		if (log_writer_get_alive_count(i-1) > 0)
		{
		
			log_writer_alive_reset(i-1);
			iuart_counter[i-1]=0; //idle counter
			ituSpriteGoto(uart_light[i - 1], 1);
		}
		else
		{
			iuart_counter[i-1]++; //idle counter
			ituSpriteGoto(uart_light[i - 1], 0);
		}
	
		ituTextSetStringInt(uart_status[i-1] ,iuart_counter[i-1]/10);
		ituTextSetStringInt(uart_files_no[i - 1], log_writer_get_number_index(i - 1));

	}


	if (StorageCheckUSBAvailabe())
			ituWidgetSetVisible(BackgroundWarning,0);
	else 
			ituWidgetSetVisible(BackgroundWarning, 1);

	return true;
}

bool UartCaptureOnEnter(ITUWidget* widget, char* param)
{

	char tmp[64] = { 0 };
	int i;

		if (NULL==BackgroundWarning)
		{
			BackgroundWarning= ituSceneFindWidget(&theScene, "BackgroundWarning");
			if (NULL == BackgroundWarning)
				assert(BackgroundWarning);
			for (i = 1; i <= 5; i++)
			{
				//=========================================
				snprintf(tmp, 64, "Text_Ch%dBaud", i);
				uart_baud[i-1] = ituSceneFindWidget(&theScene, tmp);
				if (NULL == uart_baud[i-1])
					assert(uart_baud[i-1]);

				snprintf(tmp, 64, "%d",uart[i-1].baud_rate);
				ituTextSetString(uart_baud[i-1], tmp);		
				//=========================================

				snprintf(tmp, 64, "SpriteCH%d", i);
				uart_light[i - 1] = ituSceneFindWidget(&theScene, tmp);
				if (NULL == uart_light[i - 1])
						assert(uart_light[i - 1]);

				//=========================================
				snprintf(tmp, 64, "Text_Status_CH%d", i);
				uart_status[i - 1] = ituSceneFindWidget(&theScene, tmp);
				if (NULL == uart_status[i - 1])
					assert(uart_status[i - 1]);

				ituTextSetString(uart_status[i - 1], "IDLE");
				//=========================================
				
				snprintf(tmp, 64, "Text_fils_ch%d", i);
				uart_files_no[i - 1] = ituSceneFindWidget(&theScene, tmp);
				if (NULL == uart_files_no[i - 1])
					assert(uart_files_no[i - 1]);

				snprintf(tmp, 64, "%d/500", i);
				ituTextSetString(uart_files_no[i - 1], tmp);
				//=========================================


			}

		}

		
		for (i = 1; i <= 5; i++){
		iuart_counter[i-1]=0; //idle counter
			}
	log_writer_normal_mode();
	log_writer_start();


	return true;
}


#endif
#if 1

static ITUText* power_fail[6];
static ITUText* power_count[6];
static ITUText* power_elapsed_time[6];

static ITUBackground* BackgroundPower;



bool PowerOnEnter(ITUWidget* widget, char* param)
{
	int i;
	char tmp[64] = { 0 };
	if(NULL == BackgroundPower)
	{
		BackgroundPower = ituSceneFindWidget(&theScene, "BackgroundPower");
		if (NULL == BackgroundPower)
			assert(BackgroundPower);
		for (i = 1; i <= 5; i++)
		{
			//=========================================
			snprintf(tmp, 64, "Text_PowerFail%d", i);
			power_fail[i - 1] = ituSceneFindWidget(&theScene, tmp);
			if (NULL == power_fail[i - 1])
				assert(power_fail[i - 1]);
			
			snprintf(tmp, 64, "Tex_PowerCount%d", i);
			power_count[i - 1] = ituSceneFindWidget(&theScene, tmp);
			if (NULL == power_count[i - 1])
				assert(power_count[i - 1]);
			
			snprintf(tmp, 64, "Text_ElasedTime%d", i);

			power_elapsed_time[i - 1] = ituSceneFindWidget(&theScene, tmp);
			if (NULL == power_elapsed_time[i - 1])
				assert(power_elapsed_time[i - 1]);

		}

	}

	
	
	log_writer_poweron_mode();
	log_writer_start();
	return false;
}
bool PowerOnTimer(ITUWidget* widget, char* param)
{
	static unsigned int counter;
	counter++;
	if (1 != counter%8)
		 return false;
	unsigned int elasped=0;

	char tmp[64] = {0};
	int i;
	for (i = 1; i <= 5; i++)
	{
		//elapse_time = SDL_GetTicks() - log_writer[i].action_time
		//
		elasped=get_elapsed_time_channel(i);
	//	printf("elased =%d ",elasped);
		get_time_format_string(elasped, tmp);
		ituTextSetString(power_elapsed_time[i - 1], tmp);	
		
		ituTextSetStringInt(power_fail[i - 1], get_fail_count(i));
		
		get_fail_total_stringcount(i,tmp);
		ituTextSetString(power_count[i - 1], tmp);
	}
//			printf("END\n ");
	return true;
}

bool PowerOnLeave(ITUWidget* widget, char* param)
{
	log_writer_stop();
	return false;
}

#endif

#if 1
static ITUProgressBar* ProgressBarCh[6];
static ITUProgressBar* ProgressBarOnGoing;

static ITUText* Text_BurnChannel;
static ITUText* Text_Progress1;

static ITUText* Text_burnstatus;
static ITUText* Text_info;

static int ProgressValue[6];
static int cur=1;

void BurnerUpdateBars()
{
	int i;
#ifndef WIN32	
	ProgressValue[cur] = get_progress_percent();

	
	ituTextSetStringImpl(Text_info, get_burn_info_string());
	ituTextSetStringImpl(Text_burnstatus, 	get_burn_status_string());

	ituTextSetStringInt(Text_Progress1, ProgressValue[cur]);
	ituTextSetStringInt(Text_BurnChannel, cur);
	ituProgressBarSetValue(ProgressBarOnGoing, ProgressValue[cur]);

	for (i = 1; i <= 5; i++)
	{
		ituProgressBarSetValue(ProgressBarCh[i], ProgressValue[i]);
	}
#endif
}

//layer burner
bool BurnerOnEnter(ITUWidget* widget, char* param)
{
	int i;
	char tmp[64];
	ProgressBarOnGoing = ituSceneFindWidget(&theScene, "ProgressBarOnGoing");
	if (NULL == ProgressBarOnGoing)
		assert(ProgressBarOnGoing);

	
		Text_BurnChannel = ituSceneFindWidget(&theScene, "Text_BurnChannel");
		Text_Progress1 = ituSceneFindWidget(&theScene, "Text_Progress1");
		Text_burnstatus =ituSceneFindWidget(&theScene, "Text_burnstatus");
		Text_info = ituSceneFindWidget(&theScene, "Text_info");

	for (i = 1; i <= 5; i++)
	{
		//=========================================
		snprintf(tmp, 64, "ProgressBarCH%d", i);
		ProgressBarCh[i] = ituSceneFindWidget(&theScene, tmp);
		if (NULL == ProgressBarCh[i])
			assert(ProgressBarCh[i]);

	}
	BurnerUpdateBars();
#ifndef WIN32
	burn_switching_start();
#endif	
	return true;
}

bool BurnerOnLeave(ITUWidget* widget, char* param)
{
#ifndef WIN32
	burn_switching_stop();
#endif
	return true;
}
bool BurnerOnTimer(ITUWidget* widget, char* param)
{
	/*

	float percent;
	int value;

	percent = (float)ProgressValue[cur] / 100.0;
	
	value = percent * 100;
	ituProgressBarSetValue(ProgressBarOnGoing, value);
	
	ProgressValue[cur]++;
	if (ProgressValue[cur] > 100)
	{
		ProgressValue[cur] = 0;
		cur++;
	}

	if (0== (ProgressValue%20) )
		ituWidgetSetVisible(ProgressBarCh1, 0);
	else
		ituWidgetSetVisible(ProgressBarCh1, 1);
		*/
	

	BurnerUpdateBars();



	return false;
}

void BurnerOnTimer_ui_set(int set_cur)
{
	cur = set_cur;
}

#endif
#if 1 //for edit layer
#define MAX_ITEMS 5

typedef struct
{
	int id;
	char title[32];
	char description[128];
	int items;//number of items
	char content[5][64];//selection content , max five
} Edit_item;


#define ID_BAUD_RATE		1
#define ID_PARITY			2
#define ID_DBIT				3
#define ID_SBIT				4
#define ID_WRITE_FILESIZE	5
#define ID_WRITE_FILENUM	6
#define ID_TIMESTAMP		7
#define ID_TIMEINTERVAL			8

Edit_item editems[] =
{ 
	{ 1, "buad rate", "set buad rate of uart", 3,{ "9600", "4800", "115200" } },
	{ 2, "parity", "set parity of uart", 3, { "NONE", "ODD", "EVEN" } },
	{ 3, "data bit", "set data bit of uart", 3, { "6", "7", "8" } },
	{ 4, "stop bit", "set stop bit of uart", 2, { "1", "2" } },
	{ 5, "FILE SIZE", "set the max file size of each capture logs \n (unit: KB)", 3, { "5", "20","100" } },
	{ 6, "FILE NUMBER", "set file number to close files ", 3, { "10", "20", "30" } },
	{ 7, "TIME STAMP", "Add time stamp to capture logs", 2, { "NO", "YES"} },
	{ 8, "TIME INTERVAL", "Set time interval to close file and jump to next \n (unit: minutes)", 4, { "1", "30", "60","100" } },
	{ 0, "========", "==", 3, { "x", "x", "x" } },//END ITEMS

};

static ITUText* Text_Title;
static ITUText* Text_EditChannel;
static ITUTextBox* TextBox_Description;
static ITUWheel* Wheel_setting;
static int channel_select=0;
static int focus_edit;//focusing change //0:channel 1: wheel item


static ITUIcon* IconBoarder;

static int id_select;
int matchid=1;//showing in the beginning

//used in other UI layers 
unsigned char* uart_parity_to_string(int value)
{

	if (value < editems[ID_PARITY - 1].items)
		return &(editems[ID_PARITY - 1].content[value]);
	else
		return NULL;

		
}

static void draw_border(ITUWidget* widget)
{
	int x, y, wid, hei;
	
	wid=ituWidgetGetWidth(widget);
	//IconBoarder
	hei = ituWidgetGetHeight(widget);
	x = ituWidgetGetX(widget);
	y = ituWidgetGetY(widget);

	ituWidgetSetDimension(IconBoarder, wid+4,hei+4);
	ituWidgetSetPosition(IconBoarder, x-2, y-2);
	//ituDrawLine();
	return;
}


//from wheel foucs index save to struceture
static void foucusindex_conver_struc(int id,int index_focus)
{
	int baud;
	printf("id %d ,focus index =%d \n",id, index_focus);
	int intvalue;
	intvalue = atoi(editems[id - 1].content[index_focus]);

	switch (id)
	{
	case ID_BAUD_RATE:
		uart[channel_select].baud_rate = intvalue;
		break;

	case ID_DBIT:
		uart[channel_select].databit = intvalue;
		break;
	case ID_SBIT:
		uart[channel_select].stopbit = intvalue;
		break;
	case ID_WRITE_FILESIZE:
		uart[channel_select].fileMaxsize = intvalue;
		break;

	case ID_WRITE_FILENUM:
		uart[channel_select].fileNum = intvalue;

		break;
	case ID_TIMEINTERVAL:
		uart[channel_select].fileInterval = intvalue;
		break;
	
	case ID_PARITY:
		uart[channel_select].parity = index_focus;// 0:NONE,1:ODD, 2:EVEN
		printf("ch %d parity %d \n", channel_select + 1, uart[channel_select].parity);
		break;
	case ID_TIMESTAMP: //display type
		uart[channel_select].timestamp = index_focus; // 0: NO, 1:YES

		default:
		break;
	}
}



//from structure to current wheel
static int struc_conver_foucusindex(int id)
{
	int baud;
	int cmpvalue;

	switch (id)
	{
	case ID_BAUD_RATE:
		cmpvalue = uart[channel_select].baud_rate;
		break;
	case ID_DBIT:
		cmpvalue = uart[channel_select].databit;
		break;
	case ID_SBIT:
		cmpvalue = uart[channel_select].stopbit;
		break;
	case ID_WRITE_FILESIZE:
		cmpvalue = uart[channel_select].fileMaxsize;
		break;
	case ID_WRITE_FILENUM:
		cmpvalue = uart[channel_select].fileNum;
		break;
	case ID_TIMEINTERVAL:
		cmpvalue = uart[channel_select].fileInterval;
		break;

	case ID_PARITY: //display type
		if (-1 == uart[channel_select].parity )
		{ 
			return editems[id - 1].items;
		}
		else
		{
			return uart[channel_select].parity; // 0=>NONE, 1->ODD, 2->EVEN
		}
		
	case ID_TIMESTAMP: //display type
			return uart[channel_select].timestamp; // 1=YES,0=NO
	default:
		break;
	}

	for (int j = 0; j < editems[id - 1].items; j++)
	{
		if (cmpvalue == atoi(editems[id - 1].content[j]))
		{
			return j;
		}

	}
	return  editems[id - 1].items;


}
static void update_settingWheel_ui()
{
	char* array[10] = { 0 };
	int j;
	int foucs_index;

	//find match id
	char tmp[64];
	snprintf(tmp,64, "Channel %d", channel_select+1);
	ituTextSetStringImpl(Text_EditChannel, tmp);
	ituTextSetStringImpl(Text_Title, editems[matchid-1].title);
	ituTextBoxSetString(TextBox_Description, editems[matchid-1].description);


	for (j = 0; j<editems[matchid-1].items; j++)
	{
		array[j] = editems[matchid-1].content[j];
	}
	
	ituWheelSetItemTree(Wheel_setting, &array, j);

	//list_conver_struc(Wheel_setting->focusIndex); //from focus to memory
	//set focus item to current value
	foucs_index= struc_conver_foucusindex(matchid);
	ituWheelGoto(Wheel_setting, foucs_index);



}

static bool LayerEditCheckLegal(int want_id)
{
	int j;

	
	for (j = 0; editems[j].id != 0; j++)
	{
		if (editems[j].id == want_id)
		{
			matchid = editems[j].id;
			return true;
		}
	}

	return false;
}
bool LayerEditOnEnter(ITUWidget* widget, char* param)
{

	Text_Title = ituSceneFindWidget(&theScene, "Text_Title");
	assert(Text_Title);
	
	Text_EditChannel = ituSceneFindWidget(&theScene, "Text_EditChannel");
	TextBox_Description = ituSceneFindWidget(&theScene, "TextBox_Description");

	Wheel_setting = ituSceneFindWidget(&theScene, "Wheel_setting");

	IconBoarder = ituSceneFindWidget(&theScene, "IconSelectedBoader");

	focus_edit = 0;
	
	draw_border(Text_EditChannel);

	id_select = matchid;
	
	update_settingWheel_ui();

	//ituLayerUpdate(ituSceneFindWidget(&theScene, "Layer_Edit"), ITU_EVENT_LAYOUT, 0, 0, 0);

	return true;
}

bool LayerEditOnTimer(ITUWidget* widget, char* param)
{
		return true;
}

void SetSelectedWheelItem()
{
/*
	ituTextSetStringInt(Text_settingTimeInterval, uart[index_selected].fileInterval);
	ituTextSetStringInt(Text_settingTimeStamp, uart[index_selected].timestamp);
	ituTextSetStringInt(Text_settingFileNum, uart[index_selected].fileNum);
	ituTextSetStringInt(Text_settingFilesize, uart[index_selected].fileMaxsize);
	ituTextSetStringInt(Text_settingDatabit, uart[index_selected].databit);
	ituTextSetStringInt(Text_settingStopbit, uart[index_selected].stopbit);
	ituTextSetStringInt(Text_settingBaud, uart[index_selected].baud_rate);
*/

//editems[matchid].content[j];

}

void LayerEditNextItem_wheel()
{
	int want_id;
	want_id = id_select + 1;
	if (LayerEditCheckLegal(want_id))
	{
		foucusindex_conver_struc(id_select, Wheel_setting->focusIndex);//save previous setting 

		id_select = want_id;
	
	}
	else
		printf("ITEM END\n");

}
bool LayerEditNextItem(ITUWidget* widget, char* param)
{

	if (0 == focus_edit)
	{ 
			channel_select++;
		if (channel_select >= 5)
			channel_select = 0;
	}
	else
		LayerEditNextItem_wheel();

	update_settingWheel_ui();

	return false;

}
void LayerEditPrevItem_wheel()
{
	int want_id;
	want_id = id_select - 1;
	if (LayerEditCheckLegal(want_id))
	{
		foucusindex_conver_struc(id_select, Wheel_setting->focusIndex);//save previous setting 

		id_select = want_id;
	}
	else
		printf("ITEM END\n");

}

bool LayerEditPrevItem(ITUWidget* widget, char* param)
{

	if (0 == focus_edit)
	{
		channel_select--;
		if (channel_select < 0)
			channel_select = 4;
	}
	else
		LayerEditPrevItem_wheel();

	update_settingWheel_ui();

	return false;

}


bool LayerEditSave(ITUWidget* widget, char* param)
{
	ConfigSave();
	return true;

}

bool LayerEditSelectNext(ITUWidget* widget, char* param)
{

	if (0 == focus_edit)
	{
		focus_edit = 1;
		draw_border(Wheel_setting);
	}
	else
	{
		foucusindex_conver_struc(id_select, Wheel_setting->focusIndex);//save previous setting 
		ituLayerGoto(ituSceneFindWidget(&theScene, "Layer_main"));
	}

	return false;
}

#endif
