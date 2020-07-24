#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include "ite/itp.h"

/*#define MACRO_RETURN_ERR(x)	{\
								printf("err %s\n", x);\
								return -1;\
							}*/

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

//#define IR_TX_TEST
#define IR_TX_COUNT		5
#define ForCleanShit
uint16_t	TxBuf[] = { 0xff, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c,
						0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c,
						0x40, 0x44, 0x48, 0x4c, 0x50, 0x54, 0x58, 0x5c,
						0x60, 0x64, 0x68, 0x6c, 0x70, 0x74, 0x78, 0x7c,
						0x80, 0x84, 0x88 };
void* TestFunc(void* arg)
{
    int ir_handle = 0;
    int flags = O_RDWR;
	int cnt = 0, total = sizeof(TxBuf) / sizeof(uint16_t);

    //ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);

    // Register device Remote control
    itpRegisterDevice(TEST_PORT, &TEST_DEVICE);

    // Do initialization
    ioctl(TEST_PORT, ITP_IOCTL_INIT, (void *) 0);

    // Open device
    ir_handle = open(TEST_OPEN_DEVICE, 0, flags);

    for (;;)
    {
        ITPKeypadEvent ev;

#ifdef IR_TX_TEST
		ev.code = 0;
		ev.code = TxBuf[cnt++];
		if (cnt >= total) 
			cnt = 0;
		printf("IR Tx--------Start Transmit--------code=0x%X\n", ev.code);
		write(TEST_PORT, &ev, sizeof (ITPKeypadEvent));
		usleep(5*33000);
#else
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
#endif
    }
    return NULL;
}

//TODO: how to deal with interrupt occur? where to put code or how to be invoked?
//Answer, same as keypad
