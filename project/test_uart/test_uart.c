#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "openrtos/FreeRTOS.h"
#include "openrtos/queue.h"
#include "uart/uart.h"

#define UartCommandLen 9

#ifdef CFG_DBG_UART0
#undef CFG_UART0_INTR
#undef CFG_UART0_DMA
#undef CFG_UART0_FIFO
#undef CFG_UART0_ENABLE
#endif
#ifdef CFG_DBG_UART1
#undef CFG_UART1_INTR
#undef CFG_UART1_DMA
#undef CFG_UART1_FIFO
#undef CFG_UART1_ENABLE
#endif
#ifdef CFG_DBG_UART2
#undef CFG_UART2_INTR
#undef CFG_UART2_DMA
#undef CFG_UART2_FIFO
#undef CFG_UART2_ENABLE
#endif
#ifdef CFG_DBG_UART3
#undef CFG_UART3_INTR
#undef CFG_UART3_DMA
#undef CFG_UART3_FIFO
#undef CFG_UART3_ENABLE
#endif
#ifdef CFG_DBG_UART4
#undef CFG_UART4_INTR
#undef CFG_UART4_DMA
#undef CFG_UART4_FIFO
#undef CFG_UART4_ENABLE
#endif
#ifdef CFG_DBG_UART5
#undef CFG_UART5_INTR
#undef CFG_UART5_DMA
#undef CFG_UART5_FIFO
#undef CFG_UART5_ENABLE
#endif

#if defined(CFG_UART0_INTR) || defined(CFG_UART1_INTR)\
	|| defined(CFG_UART2_INTR) || defined(CFG_UART3_INTR)\
	|| defined(CFG_UART4_INTR) || defined(CFG_UART5_INTR)
#define TEST_INTR_MODE
#endif

#if defined(CFG_UART0_DMA) || defined(CFG_UART1_DMA)\
	|| defined(CFG_UART2_DMA) || defined(CFG_UART3_DMA)\
	|| defined(CFG_UART4_DMA) || defined(CFG_UART5_DMA)
#define TEST_DMA_MODE
#endif

#if defined(CFG_UART0_FIFO) || defined(CFG_UART1_FIFO)\
	|| defined(CFG_UART2_FIFO) || defined(CFG_UART3_FIFO)\
	|| defined(CFG_UART4_FIFO) || defined(CFG_UART5_FIFO)
#define TEST_FIFO_MODE
#endif

#if defined(CFG_UART0_ENABLE)
#define TEST_PORT       ITP_DEVICE_UART0
#define TEST_ITH_PORT	ITH_UART0
#define TEST_DEVICE     itpDeviceUart0
#define TEST_BAUDRATE   CFG_UART0_BAUDRATE
#define TEST_GPIO_RX    CFG_GPIO_UART0_RX
#define TEST_GPIO_TX    CFG_GPIO_UART0_TX
#elif defined(CFG_UART1_ENABLE)
#define TEST_PORT       ITP_DEVICE_UART1
#define TEST_ITH_PORT	ITH_UART1
#define TEST_DEVICE     itpDeviceUart1
#define TEST_BAUDRATE   CFG_UART1_BAUDRATE
#define TEST_GPIO_RX    CFG_GPIO_UART1_RX
#define TEST_GPIO_TX    CFG_GPIO_UART1_TX
#elif defined(CFG_UART2_ENABLE)
#define TEST_PORT       ITP_DEVICE_UART2
#define TEST_ITH_PORT	ITH_UART2
#define TEST_DEVICE     itpDeviceUart2
#define TEST_BAUDRATE   CFG_UART2_BAUDRATE
#define TEST_GPIO_RX    CFG_GPIO_UART2_RX
#define TEST_GPIO_TX    CFG_GPIO_UART2_TX
#elif defined(CFG_UART3_ENABLE)
#define TEST_PORT       ITP_DEVICE_UART3
#define TEST_ITH_PORT	ITH_UART3
#define TEST_DEVICE     itpDeviceUart3
#define TEST_BAUDRATE   CFG_UART3_BAUDRATE
#define TEST_GPIO_RX    CFG_GPIO_UART3_RX
#define TEST_GPIO_TX    CFG_GPIO_UART3_TX
#elif defined(CFG_UART4_ENABLE)
#define TEST_PORT       ITP_DEVICE_UART4
#define TEST_ITH_PORT	ITH_UART4
#define TEST_DEVICE     itpDeviceUart4
#define TEST_BAUDRATE   CFG_UART4_BAUDRATE
#define TEST_GPIO_RX    CFG_GPIO_UART4_RX
#define TEST_GPIO_TX    CFG_GPIO_UART4_TX
#elif defined(CFG_UART5_ENABLE)
#define TEST_PORT       ITP_DEVICE_UART5
#define TEST_ITH_PORT	ITH_UART5
#define TEST_DEVICE     itpDeviceUart5
#define TEST_BAUDRATE   CFG_UART5_BAUDRATE
#define TEST_GPIO_RX    CFG_GPIO_UART5_RX
#define TEST_GPIO_TX    CFG_GPIO_UART5_TX
#endif

#if defined(TEST_INTR_MODE)
static sem_t UartSemIntr;
#elif defined(TEST_DMA_MODE)
static sem_t UartSemDma;
#elif defined(TEST_FIFO_MODE)
static sem_t UartSemFifo;
#endif

#if defined(TEST_INTR_MODE)
static void UartCallback(void* arg1, uint32_t arg2)
{
#if 1  // Turn on  ITP_IOCTL_REG_UART_CB  flag , should using these codes.
	sem_post(&UartSemIntr);
#else  // Turn on ITP_IOCTL_REG_UART_DEFER_CB flag , should using these codes.
	uint8_t getstr1[256];
	uint8_t sendstr1[8] = { 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A };
	int len = 0;
	static int totalcount = 0;
	len = read(TEST_PORT,getstr1+totalcount,256);
	totalcount += len;
	
	if(totalcount >= UartCommandLen)
	{	  
		write(TEST_PORT,sendstr1, 8);
		totalcount =0;
		memset(getstr1, 0, 256);
	}
#endif
}
#endif

#if defined(TEST_DMA_MODE) || defined(TEST_FIFO_MODE)
void timer_isr(void* data)
{
    uint32_t timer = (uint32_t)data;

    ithTimerClearIntr(timer);
#if defined(TEST_DMA_MODE)
	sem_post(&UartSemDma);
#elif defined(TEST_FIFO_MODE)
	sem_post(&UartSemFifo);
#endif
}

static void UartIntrHandler(unsigned int pin, void *arg)
{
	ITHTimer timer = (ITHTimer)arg;
	//ithPrintf("timer=%d\n", timer);
	ithGpioClearIntr(pin);
	ithTimerCtrlDisable(timer, ITH_TIMER_EN);	
	ithTimerSetTimeout(timer, 1000);//us
	ithTimerCtrlEnable(timer, ITH_TIMER_EN);	
}

/*
static void UartIntrHandler2(void * arg)
{
	ITHUartPort port = (ITHUartPort)arg;
	ithUartClearIntr(port);
	ithPrintf("-- uart handle2 --\n");
}*/

void InitUartIntr(ITHUartPort port)
{
	ithEnterCritical();
#if 1
    ithGpioClearIntr(TEST_GPIO_RX);
    ithGpioSetIn(TEST_GPIO_RX);
    ithGpioRegisterIntrHandler((unsigned int)TEST_GPIO_RX, UartIntrHandler, (void*)ITH_TIMER5);
	
    ithGpioCtrlDisable(TEST_GPIO_RX, ITH_GPIO_INTR_LEVELTRIGGER);   /* use edge trigger mode */
    ithGpioCtrlEnable(TEST_GPIO_RX, ITH_GPIO_INTR_BOTHEDGE); /* both edge */    
    ithIntrEnableIrq(ITH_INTR_GPIO);
    ithGpioEnableIntr(TEST_GPIO_RX);
#else
	//printf("port = 0x%x\n", port);
	ithIntrDisableIrq(ITH_INTR_UART2);
    ithUartClearIntr(ITH_UART2);
    ithIntrClearIrq(ITH_INTR_UART2);

    ithIntrSetTriggerModeIrq(ITH_INTR_UART2, ITH_INTR_LEVEL);
    ithIntrRegisterHandlerIrq(ITH_INTR_UART2, UartIntrHandler2, (void *)ITH_UART2);
    ithUartEnableIntr(ITH_UART2, ITH_UART_RX_READY);

    /* Enable the interrupts. */
    ithIntrEnableIrq(ITH_INTR_UART2);
#endif
    ithExitCritical();
}

void InitTimer(ITHTimer timer, ITHIntr intr)
{
	ithTimerReset(timer);
	
	// Initialize Timer IRQ
	ithIntrDisableIrq(intr);
	ithIntrClearIrq(intr);

	// register Timer Handler to IRQ
	ithIntrRegisterHandlerIrq(intr, timer_isr, (void*)timer);

	// set Timer IRQ to edge trigger
	ithIntrSetTriggerModeIrq(intr, ITH_INTR_EDGE);

	// set Timer IRQ to detect rising edge
	ithIntrSetTriggerLevelIrq(intr, ITH_INTR_HIGH_RISING);

	// Enable Timer IRQ
	ithIntrEnableIrq(intr);
}
#endif

#if defined(TEST_INTR_MODE)
void* TestFuncUseINTR(void* arg)
{
	char getstr[256] = "", sendstr[256] = "";
	int len = 0, count = 0;

	UART_OBJ *pUartInfo	= (UART_OBJ*)malloc(sizeof(UART_OBJ));
	pUartInfo->port		= TEST_ITH_PORT;
	pUartInfo->parity	= 0;
	pUartInfo->txPin	= TEST_GPIO_TX;
	pUartInfo->rxPin	= TEST_GPIO_RX;
	pUartInfo->baud		= TEST_BAUDRATE;
	pUartInfo->timeout	= 0;
	pUartInfo->mode		= UART_INTR_MODE;
	pUartInfo->forDbg	= false;

	printf("Start uart Interrupt mode test!\n");

	itpRegisterDevice(TEST_PORT, &TEST_DEVICE);

	ioctl(TEST_PORT, ITP_IOCTL_INIT, (void*)pUartInfo);
	ioctl(TEST_PORT, ITP_IOCTL_REG_UART_CB, (void*)UartCallback);
	////ioctl(TEST_PORT, ITP_IOCTL_REG_UART_DEFER_CB , (void*)UartCallback);

	//ithPrintf("\n");
	sem_init(&UartSemIntr, 0, 0);

	while (1)
	{
		sem_wait(&UartSemIntr);

		len = read(TEST_PORT, getstr + count, 256);
		printf("len = %d, getstr = %s\n", len, getstr);
		count += len;

		if (count >= UartCommandLen)
		{
			printf("uart read: %s,count=%d\n", getstr, count);
			memcpy(sendstr, getstr, count);
			sendstr[count] = '\n';
			write(TEST_PORT, sendstr, count + 1);

			memset(getstr, 0, count + 1);
			memset(sendstr, 0, count + 1);
			count = 0;
		}
	}
}
#elif defined(TEST_DMA_MODE)
void* TestFuncUseDMA(void* arg)
{
	int len = 0, count = 0;
	char getstr[256] = "", sendstr[256] = "";

	UART_OBJ *pUartInfo	= (UART_OBJ*)malloc(sizeof(UART_OBJ));
	pUartInfo->port		= TEST_ITH_PORT;
	pUartInfo->parity	= 0;
	pUartInfo->txPin	= TEST_GPIO_TX;
	pUartInfo->rxPin	= TEST_GPIO_RX;
	pUartInfo->baud		= TEST_BAUDRATE;
	pUartInfo->timeout	= 0;
	pUartInfo->mode		= UART_DMA_MODE;
	pUartInfo->forDbg	= false;

	itpRegisterDevice(TEST_PORT, &TEST_DEVICE);
	ioctl(TEST_PORT, ITP_IOCTL_INIT, (void*)pUartInfo);
	//ioctl(TEST_PORT, ITP_IOCTL_RESET, (void*)pUartInfo);
	
	InitTimer(ITH_TIMER5, ITH_INTR_TIMER5);
	InitUartIntr(TEST_PORT);

	printf("Start uart DMA mode test!\n");

	sem_init(&UartSemDma, 0, 0);

	while(1)
	{
		sem_wait(&UartSemDma);

		len = read(TEST_PORT, getstr + count, 256);
		printf("len = %d, getstr = %s\n", len, getstr);
	    count += len;

	    if(count >= UartCommandLen)
	    {
			printf("uart read: %s,count=%d\n", getstr, count);
			memcpy(sendstr, getstr, count);
			sendstr[count] = '\n';
			write(TEST_PORT, sendstr, count + 1);

			memset(getstr, 0, count + 1);
			memset(sendstr, 0, count + 1);
			count = 0;
	    }
	}
}
#elif defined(TEST_FIFO_MODE)
void* TestFuncUseFIFO(void* arg)
{
	int len = 0, count = 0;
	char getstr[256] = "", sendstr[256] = "";

	UART_OBJ *pUartInfo	= (UART_OBJ*)malloc(sizeof(UART_OBJ));
	pUartInfo->port		= TEST_ITH_PORT;
	pUartInfo->parity	= 0;
	pUartInfo->txPin	= TEST_GPIO_TX;
	pUartInfo->rxPin	= TEST_GPIO_RX;
	pUartInfo->baud		= TEST_BAUDRATE;
	pUartInfo->timeout	= 0;
	pUartInfo->mode		= UART_FIFO_MODE;
	pUartInfo->forDbg	= false;

	itpRegisterDevice(TEST_PORT, &TEST_DEVICE);
	ioctl(TEST_PORT, ITP_IOCTL_INIT, (void*)pUartInfo);
	//ioctl(TEST_PORT, ITP_IOCTL_RESET, (void*)pUartInfo);

	InitTimer(ITH_TIMER5, ITH_INTR_TIMER5);
	InitUartIntr(TEST_PORT);

	printf("Start uart FIFO mode test!\n");

	sem_init(&UartSemFifo, 0, 0);

	while (1)
	{
		sem_wait(&UartSemFifo);
		// 9860 FIFO buffer maximum depth : 128
		len = read(TEST_PORT, getstr + count, 128);
		printf("len = %d, getstr = %s\n", len, getstr);
		count += len;

		if (count >= UartCommandLen)
		{
			printf("uart read: %s,count=%d\n", getstr, count);
			memcpy(sendstr, getstr, count);
			sendstr[count] = '\n';
			write(TEST_PORT, sendstr, count+1);

			memset(getstr, 0, count + 1);
			memset(sendstr, 0, count + 1);
			count = 0;
		}
	}
}
#endif
