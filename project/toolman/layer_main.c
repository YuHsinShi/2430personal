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

	log_writer_normal_mode();
	log_writer_start();


	return true;
}


#endif
#if 1


bool PowerOnEnter(ITUWidget* widget, char* param)
{


	log_writer_poweron_mode();

	log_writer_start();
	return false;
}
bool PowerOnTimer(ITUWidget* widget, char* param)
{
	return false;
}

bool PowerOnLeave(ITUWidget* widget, char* param)
{
	log_writer_stop();
	return false;
}

#endif
