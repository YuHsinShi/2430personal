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



void MainReset(void)
{
}
bool MainOnEnter(ITUWidget* widget, char* param)
{
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

	burn_switching_start();
	return true;
}

bool BurnerOnLeave(ITUWidget* widget, char* param)
{

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
