#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include "ite/itp.h"

#if defined(CFG_IR0_ENABLE)
#define TEST_PORT       ITP_DEVICE_IR0
#define TEST_DEVICE     itpDeviceIr0
#elif defined(CFG_IR1_ENABLE)
#define TEST_PORT       ITP_DEVICE_IR1
#define TEST_DEVICE     itpDeviceIr1
#elif defined(CFG_IR2_ENABLE)
#define TEST_PORT       ITP_DEVICE_IR2
#define TEST_DEVICE     itpDeviceIr2
#elif defined(CFG_IR3_ENABLE)
#define TEST_PORT       ITP_DEVICE_IR3
#define TEST_DEVICE     itpDeviceIr3
#endif

#define TX_SIG_REPEAT_TIMES 3

uint16_t	TxBuf[] = { // 0x11
	9000, 4500, 560, 560*3, 560, 560*3, 560, 560, 560, 560,
	560, 560, 560, 560, 560, 560, 560, 560, 560, 560,
	560, 560, 560, 560*3, 560, 560*3, 560, 560*3, 560, 560*3,
	560, 560*3, 560, 560*3, 560, 560*3, 560, 560, 560, 560,
	560, 560, 560, 560*3, 560, 560, 560, 560, 560, 560,
	560, 560, 560, 560*3, 560, 560*3, 560, 560*3, 560, 560,
	560, 560*3, 560, 560*3, 560, 560*3, 560,
};
void* TestFunc(void* arg)
{
	int cnt = 0, total = sizeof(TxBuf) / sizeof(uint16_t), times = 1;

    // Register device Remote control
    itpRegisterDevice(TEST_PORT, &TEST_DEVICE);

    // Do initialization
    ioctl(TEST_PORT, ITP_IOCTL_INIT, (void *) 0);

	
	ITPKeypadEvent ev;
    for (;;)
    {
		ev.code = 0;
		ev.code = TxBuf[cnt++];
		write(TEST_PORT, &ev, sizeof (ITPKeypadEvent));

		if (cnt >= total)
		{
			printf("one set of IR data been sent! times: %d\n", times);
			cnt = 0;
			usleep(1000 * 500);
			if (times++ >= TX_SIG_REPEAT_TIMES)
				break;
		}
    }
	printf("END testing\n");
    return NULL;
}