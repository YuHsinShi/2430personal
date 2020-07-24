#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include "ite/itp.h"

#define TEST_PWM		ITH_PWM1
#define TEST_PWM_GPIO	14
#define TEST_PWM_DUTY	50

#if defined(CFG_IR0_ENABLE)
#define TEST_IR_PORT	ITH_IR0
#define TEST_IR_GPIO	CFG_GPIO_IR0_RX
#define TEST_PORT       ITP_DEVICE_IR0
#define TEST_DEVICE     itpDeviceIr0
#elif defined(CFG_IR1_ENABLE)
#define TEST_IR_PORT	ITH_IR1
#define TEST_IR_GPIO	CFG_GPIO_IR1_RX
#define TEST_PORT       ITP_DEVICE_IR1
#define TEST_DEVICE     itpDeviceIr1
#elif defined(CFG_IR2_ENABLE)
#define TEST_IR_PORT	ITH_IR2
#define TEST_IR_GPIO	CFG_GPIO_IR2_RX
#define TEST_PORT       ITP_DEVICE_IR2
#define TEST_DEVICE     itpDeviceIr2
#elif defined(CFG_IR3_ENABLE)
#define TEST_IR_PORT	ITH_IR3
#define TEST_IR_GPIO	CFG_GPIO_IR3_RX
#define TEST_PORT       ITP_DEVICE_IR3
#define TEST_DEVICE     itpDeviceIr3
#endif

void *PrintTask(void *arg)
{
	uint32_t fastFreq = 0x0, slowFreq = 0x0, avgFreq = 0x0, newFreq = 0x0;
	uint32_t highFastDC = 0x0, lowFastDC = 0x0;
	uint32_t highSlowDC = 0x0, lowSlowDC = 0x0;
	uint32_t highAvgDC = 0x0, lowAvgDC = 0x0;
	uint32_t highNewDC = 0x0, lowNewDC = 0x0;
	uint32_t port = TEST_IR_PORT;
	int count = 0;

	while(1) 
	{
		printf("===========Count %d===========\n", count);
		fastFreq = iteIrGetFreqFast(port);
		slowFreq = iteIrGetFreqSlow(port);
		avgFreq = iteIrGetFreqAvg(port);
		newFreq = iteIrGetFreqNew(port);
		printf("fastest freq = %d(hz), slowest freq = %d(hz)\n", (int)fastFreq, (int)slowFreq);
		printf("average freq = %d(hz), newest freq = %d(hz)\n", (int)avgFreq, (int)newFreq);

		highFastDC = iteIrGetHighDCFast(port);
		lowFastDC = iteIrGetLowDCFast(port);
		printf("Fastest Duty Cycle: high-level(%d) low-level(%d)\n", (int)highFastDC, (int)lowFastDC);

		highSlowDC = iteIrGetHighDCSlow(port);
		lowSlowDC = iteIrGetLowDCSlow(port);
		printf("Slowest Duty Cycle: high-level(%d) low-level(%d)\n", (int)highSlowDC, (int)lowSlowDC);

		highAvgDC = iteIrGetHighDCAvg(port);
		lowAvgDC = iteIrGetLowDCAvg(port);
		printf("Average Duty Cycle: high-level(%d) low-level(%d)\n", (int)highAvgDC, (int)lowAvgDC);

		highNewDC = iteIrGetHighDCNew(port);
		lowNewDC = iteIrGetLowDCNew(port);
		printf("Newest Duty Cycle: high-level(%d) low-level(%d)\n", (int)highNewDC, (int)lowNewDC);

		count++;

		iteIrClearClkSample(port);

		usleep(110 * 1000);
	}
}

void *TestFunc(void* arg)
{
	unsigned int preScale = 0x0;
	pthread_t task1, task2, taskDetect;
	pthread_attr_t attr1, attr2, attrDetect;

	int ir_handle = 0, flags = O_RDWR;
	
	// Register device Remote control
	itpRegisterDevice(TEST_PORT, &TEST_DEVICE);

	// Do initialization
	ioctl(TEST_PORT, ITP_IOCTL_INIT, (void *)0);

	printf("Test Clock Sample Function\n");

	// Init PWM
	ithPwmInit(TEST_PWM, 38000, TEST_PWM_DUTY); //38khz
	ithPwmReset(TEST_PWM, TEST_PWM_GPIO, 0);
	ithPwmSetDutyCycle(TEST_PWM, TEST_PWM_DUTY);
	ithPwmEnable(TEST_PWM, TEST_PWM_GPIO, 0);

	sleep(1);

	// Creat Print Clock Sample task
	pthread_attr_init(&attr2);
	pthread_create(&task2, &attr2, PrintTask, NULL); 

	printf("*******Test End\n");
}
