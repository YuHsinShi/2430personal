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
static ITUText* uart_baud[6];
bool UartCaptureOnEnter(ITUWidget* widget, char* param)
{
	//Text_Ch3Baud 

	char tmp[64] = { 0 };
	int i;
	for (i = 1; i <= 5; i++)
	{
		snprintf(tmp, 64, "Text_Ch%dBaud", i);
		uart_baud[i-1] = ituSceneFindWidget(&theScene, tmp);
		if (NULL == uart_baud[i-1])
			assert(uart_baud[i-1]);

		snprintf(tmp, 64, "%d", theConfig.uart[i-1].baud_rate);
		ituTextSetString(uart_baud[i-1], tmp);

	}
	return true;
}
#endif