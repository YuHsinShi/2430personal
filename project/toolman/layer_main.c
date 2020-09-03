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

bool UartCaptureOnTimer(ITUWidget* widget, char* param)
{
	static int counter;
	counter++;
	if (0 != counter % 4)
		return false;

	int i;
	for (i = 1; i <= 5; i++)
	{
		if (theConfig.uart[i - 1].alive_flag > 0)
		{
		
			theConfig.uart[i - 1].alive_flag = 0;
			ituSpriteGoto(uart_light, 1);
		}
		else
		{
			ituSpriteGoto(uart_light, 0);
		}
	
	}

	if (STORAGE_USB_REMOVED == StorageCheck())
			ituWidgetSetVisible(BackgroundWarning,1);
	else 
			ituWidgetSetVisible(BackgroundWarning, 0);

	return true;
}

bool UartCaptureOnEnter(ITUWidget* widget, char* param)
{

	char tmp[64] = { 0 };
	int i;
	for (i = 1; i <= 5; i++)
	{
		//=========================================
		snprintf(tmp, 64, "Text_Ch%dBaud", i);
		uart_baud[i-1] = ituSceneFindWidget(&theScene, tmp);
		if (NULL == uart_baud[i-1])
			assert(uart_baud[i-1]);

		snprintf(tmp, 64, "%d", theConfig.uart[i-1].baud_rate);
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
	BackgroundWarning= ituSceneFindWidget(&theScene, "BackgroundWarning");
	if (NULL == BackgroundWarning)
		assert(BackgroundWarning);


	return true;
}
#endif