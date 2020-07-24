#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include "ite/itp.h"

//#define TEST_PWM		ITH_PWM1
//#define TEST_PWM_GPIO	59
//#define TEST_PWM_DUTY	50

#if defined(CFG_IR0_ENABLE)
#define TEST_IR_PORT	ITH_IR0
#define TEST_IR_GPIO	CFG_GPIO_IR0_RX
#elif defined(CFG_IR1_ENABLE)
#define TEST_IR_PORT	ITH_IR1
#define TEST_IR_GPIO	CFG_GPIO_IR1_RX
#elif defined(CFG_IR2_ENABLE)
#define TEST_IR_PORT	ITH_IR2
#define TEST_IR_GPIO	CFG_GPIO_IR2_RX
#elif defined(CFG_IR3_ENABLE)
#define TEST_IR_PORT	ITH_IR3
#define TEST_IR_GPIO	CFG_GPIO_IR3_RX
#endif

void PrintTask(void *arg)
{
	uint32_t fastFreq = 0x0, slowFreq = 0x0, avgFreq = 0x0, newFreq = 0x0;
	uint32_t highFastDC = 0x0, lowFastDC = 0x0;
	uint32_t highSlowDC = 0x0, lowSlowDC = 0x0;
	uint32_t highAvgDC = 0x0, lowAvgDC = 0x0;
	uint32_t highNewDC = 0x0, lowNewDC = 0x0;
	uint32_t port = TEST_IR_PORT;
	int count = 1;
	
	while(1) {
		printf("===========Count %d===========\n", count);
		fastFreq = ithGetFreqFast(port);
		slowFreq = ithGetFreqSlow(port);
		avgFreq = ithGetFreqAvg(port);
		newFreq = ithGetFreqNew(port);
		printf("fastest freq = %x, slowest freq = %x, average freq = %x, newest freq = %x\n", fastFreq, slowFreq, avgFreq, newFreq);

		highFastDC = ithGetHighDCFast(port);
		lowFastDC = ithGetLowDCFast(port);
		printf("Fastest Duty Cycle: high-level(%x) low-level(%x)\n", highFastDC, lowFastDC);

		highSlowDC = ithGetHighDCSlow(port);
		lowSlowDC = ithGetLowDCSlow(port);
		printf("Slowest Duty Cycle: high-level(%x) low-level(%x)\n", highSlowDC, lowSlowDC);

		highAvgDC = ithGetHighDCAvg(port);
		lowAvgDC = ithGetLowDCAvg(port);
		printf("Average Duty Cycle: high-level(%x) low-level(%x)\n", highAvgDC, lowAvgDC);

		highNewDC = ithGetHighDCNew(port);
		lowNewDC = ithGetLowDCNew(port);
		printf("Newest Duty Cycle: high-level(%x) low-level(%x)\n", highNewDC, lowNewDC);

		count++;
		sleep(1);
	}
}

void* TestFunc(void* arg)
{
	unsigned int preScale = 0x0;
	pthread_t task1, task2;
	pthread_attr_t attr1, attr2;
	
	printf("Test Clock Sample Function\n");

#if 0
	// Init PWM
	ithPwmInit(TEST_PWM, 1000, TEST_PWM_DUTY);
	ithPwmReset(TEST_PWM, TEST_PWM_GPIO, 0);
	ithPwmSetDutyCycle(TEST_PWM, TEST_PWM_DUTY);
	ithPwmEnable(TEST_PWM, TEST_PWM_GPIO, 0);
	sleep(1);
#endif

	// Init Clock Sample function
	ithClkSampleInit(TEST_IR_PORT, preScale, TEST_IR_GPIO);

	// Set Min & Max Filter
	//ithIrRxSetModFilter(TEST_IR_PORT, 0x6800, 0x6A00); //ex: 27000Hz=0x6978

	// Enable Clock Sample function
	ithIrRxMode(TEST_IR_PORT, ITH_SAMPLE_CLK);

	sleep(1);

	// Creat Print Clock Sample task
	pthread_attr_init(&attr2);
	pthread_create(&task2, &attr2, PrintTask, NULL); 

	printf("*******Test End\n");
}
