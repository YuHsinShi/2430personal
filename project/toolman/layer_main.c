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
	
	
#if defined(LOG_DISK_TEST)
if(bLogDisk == false)
{
    if(StorageCheckSpace("E:/", 512) && !bLogDisk)
    {
        ioctl(ITP_DEVICE_LOGDISK, ITP_IOCTL_INIT, "E:/log.txt"); 
        itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceLogDisk);
        bLogDisk = true;
    }
}
else
{
	#if defined(CFG_DBG_UART1)
        itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceUart1);
	#elif defined(CFG_DBG_UART0)
	        itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceUart0);
	#endif
	        ioctl(ITP_DEVICE_LOGDISK, ITP_IOCTL_OFF, NULL);
	        bLogDisk = false;
	#endif
		
	return true;
}

}
