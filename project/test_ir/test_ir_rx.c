#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include "ite/itp.h"

#if defined(CFG_IR0_ENABLE)
#define TEST_PORT       ITP_DEVICE_IR0
#define TEST_DEVICE     itpDeviceIr0
#define TEST_OPEN_DEVICE	":ir0"
#elif defined(CFG_IR1_ENABLE)
#define TEST_PORT       ITP_DEVICE_IR1
#define TEST_DEVICE     itpDeviceIr1
#define TEST_OPEN_DEVICE	":ir1"
#elif defined(CFG_IR2_ENABLE)
#define TEST_PORT       ITP_DEVICE_IR2
#define TEST_DEVICE     itpDeviceIr2
#define TEST_OPEN_DEVICE	":ir2"
#elif defined(CFG_IR3_ENABLE)
#define TEST_PORT       ITP_DEVICE_IR3
#define TEST_DEVICE     itpDeviceIr3
#define TEST_OPEN_DEVICE	":ir3"
#endif

void* TestFunc(void* arg)
{
	// initialize IrDA, the method below could be found in itpInit();
	ITPKeypadEvent ev;
	unsigned char buff[64]={0};
	int i;
    // Register device Remote control
    itpRegisterDevice(TEST_PORT, &TEST_DEVICE);

    // Do initialization
    ioctl(TEST_PORT, ITP_IOCTL_INIT, (void *) 0);
	// initialize IrDA end.
unsigned cmd_len;
    for (;;)
    {
		cmd_len = read(TEST_PORT, buff, 64);
        if (cmd_len)
		{
				ithPrintf("cmd_len=0x%x \n",cmd_len);

			  for(i=0;i<cmd_len;i++)
				ithPrintf("0x%x ",buff[i]);
				
				ithPrintf("\n");
		}
		else
		{
			
			
		}
		usleep(33000);
    }
    return NULL;
}
