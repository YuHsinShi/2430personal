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
    // Register device Remote control
    itpRegisterDevice(TEST_PORT, &TEST_DEVICE);

    // Do initialization
    ioctl(TEST_PORT, ITP_IOCTL_INIT, (void *) 0);
	// initialize IrDA end.

    for (;;)
    {
        if (read(TEST_PORT, &ev, sizeof (ITPKeypadEvent)) == sizeof (ITPKeypadEvent))
            printf("key: time=%lld.%ld,code=%d,down=%d,up=%d,repeat=%d,flags=0x%X\r\n", 
            ev.time.tv_sec,
            ev.time.tv_usec / 1000,
            ev.code,
            (ev.flags & ITP_KEYPAD_DOWN) ? 1 : 0,
            (ev.flags & ITP_KEYPAD_UP) ? 1 : 0,
            (ev.flags & ITP_KEYPAD_REPEAT) ? 1 : 0,
            ev.flags);
		usleep(33000);
    }
    return NULL;
}
