/*
 * Copyright (c) 2015 ITE Tech. Inc. All Rights Reserved.
 *
 * @file (%project)/project/test_rs485/test_rs485.c
 *
 * @author Benson Lin
 * @version 1.0.0
 *
 * example code for how to using rs485.
 */
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "ite/itp.h"	//for all ith driver
#include "uart/interface/rs485.h"

#define RS485CommandLen 4

/*
UART0: FIFO 32 Bytes , It means if we choice UART0 , we can transter/recevier 32 bytes once a time.
UART1: FIFO 16 Bytes , It means if we choice UART1 , we can transter/recevier 16 bytes once a time.
UART2: FIFO   8 Bytes,  It means if we choice UART2 , we can transter/recevier  8 bytes once a time.
UART3: FIFO   8 Bytes,  It means if we choice UART3 , we can transter/recevier  8 bytes once a time.
*/

#ifdef CFG_DBG_UART0
#undef CFG_UART0_INTR
#undef CFG_UART0_DMA
#undef CFG_UART0_FIFO
#endif
#ifdef CFG_DBG_UART1
#undef CFG_UART1_INTR
#undef CFG_UART1_DMA
#undef CFG_UART1_FIFO
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

#if defined(CFG_UART0_RS485)
#define TEST_PORT       ITP_DEVICE_RS485_0
#define TEST_ITH_PORT	ITH_UART0
#define TEST_DEVICE     itpDeviceRS485_0
#define TEST_BAUDRATE   CFG_UART0_BAUDRATE
#define TEST_GPIO_RX    CFG_GPIO_UART0_RX
#define TEST_GPIO_TX    CFG_GPIO_UART0_TX
#define TEST_GPIO_EN    CFG_GPIO_RS485_0_TX_ENABLE
#elif defined(CFG_UART1_RS485)
#define TEST_PORT       ITP_DEVICE_RS485_1
#define TEST_ITH_PORT	ITH_UART1
#define TEST_DEVICE     itpDeviceRS485_1
#define TEST_BAUDRATE   CFG_UART1_BAUDRATE
#define TEST_GPIO_RX    CFG_GPIO_UART1_RX
#define TEST_GPIO_TX    CFG_GPIO_UART1_TX
#define TEST_GPIO_EN    CFG_GPIO_RS485_1_TX_ENABLE
#elif defined(CFG_UART2_RS485)
#define TEST_PORT       ITP_DEVICE_RS485_2
#define TEST_ITH_PORT	ITH_UART2
#define TEST_DEVICE     itpDeviceRS485_2
#define TEST_BAUDRATE   CFG_UART2_BAUDRATE
#define TEST_GPIO_RX    CFG_GPIO_UART2_RX
#define TEST_GPIO_TX    CFG_GPIO_UART2_TX
#define TEST_GPIO_EN    CFG_GPIO_RS485_2_TX_ENABLE
#elif defined(CFG_UART3_RS485)
#define TEST_PORT       ITP_DEVICE_RS485_3
#define TEST_ITH_PORT	ITH_UART3
#define TEST_DEVICE     itpDeviceRS485_3
#define TEST_BAUDRATE   CFG_UART3_BAUDRATE
#define TEST_GPIO_RX    CFG_GPIO_UART3_RX
#define TEST_GPIO_TX    CFG_GPIO_UART3_TX
#define TEST_GPIO_EN    CFG_GPIO_RS485_3_TX_ENABLE
#elif defined(CFG_UART4_RS485)
#define TEST_PORT       ITP_DEVICE_RS485_4
#define TEST_ITH_PORT	ITH_UART4
#define TEST_DEVICE     itpDeviceRS485_4
#define TEST_BAUDRATE   CFG_UART4_BAUDRATE
#define TEST_GPIO_RX    CFG_GPIO_UART4_RX
#define TEST_GPIO_TX    CFG_GPIO_UART4_TX
#define TEST_GPIO_EN    CFG_GPIO_RS485_4_TX_ENABLE
#elif defined(CFG_UART5_RS485)
#define TEST_PORT       ITP_DEVICE_RS485_5
#define TEST_ITH_PORT	ITH_UART5
#define TEST_DEVICE     itpDeviceRS485_5
#define TEST_BAUDRATE   CFG_UART5_BAUDRATE
#define TEST_GPIO_RX    CFG_GPIO_UART5_RX
#define TEST_GPIO_TX    CFG_GPIO_UART5_TX
#define TEST_GPIO_EN    CFG_GPIO_RS485_5_TX_ENABLE
#endif

#if defined(TEST_INTR_MODE)
static sem_t RS485SemIntr;
#elif defined(TEST_DMA_MODE)
static sem_t RS485SemDma;
#elif defined(TEST_FIFO_MODE)
static sem_t RS485SemFifo;
#endif

#if defined(TEST_INTR_MODE)
static void RS485Callback(void* arg1, uint32_t arg2)
{
	uint8_t getstr[256];
	uint8_t sendstr1[8] = {0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
	int len = 0;
	static int totalcount =0;

#if 1  // Turn on  ITP_IOCTL_REG_RS485_CB  flag , should using these codes.
	sem_post(&RS485SemIntr);
#else  // Turn on ITP_IOCTL_REG_RS485_DEFER_CB flag , should using these codes.
	
	len = read(TEST_PORT,getstr+totalcount,256);
	totalcount += len;
	
	if(totalcount >= RS485CommandLen)
	{	  
		write(TEST_PORT,sendstr1,8);
		totalcount =0;
		memset(getstr, 0, 256);
	}
#endif
}
#endif
#if defined(TEST_DMA_MODE) || defined(TEST_FIFO_MODE)
static void gpio_isr(unsigned int pin, void *arg)
{
#if defined(TEST_DMA_MODE)
	sem_post(&RS485SemDma);
#elif defined(TEST_FIFO_MODE)
	sem_post(&RS485SemFifo);
#endif
}

void InitRS485Intr(ITHUartPort port)
{
	ithEnterCritical();

	ithGpioClearIntr(TEST_GPIO_RX);
	ithGpioSetIn(TEST_GPIO_RX);
	ithGpioRegisterIntrHandler(TEST_GPIO_RX, gpio_isr, NULL);
	
	ithGpioCtrlDisable(TEST_GPIO_RX, ITH_GPIO_INTR_LEVELTRIGGER);   /* use edge trigger mode */
	ithGpioCtrlEnable(TEST_GPIO_RX, ITH_GPIO_INTR_BOTHEDGE); /* both edge */
	ithIntrEnableIrq(ITH_INTR_GPIO);
	ithGpioEnableIntr(TEST_GPIO_RX);

	ithExitCritical();
}
#endif

#if defined(TEST_INTR_MODE)
void* TestFuncUseINTR(void* arg)
{
	char getstr[256] = "", sendstr[256] = "";
	int len = 0, count =0;

   	printf("Start RS485 Interrupt mode test !\n");

	RS485_OBJ *pRs485Info = (RS485_OBJ*)malloc(sizeof(RS485_OBJ));
	pRs485Info->port = TEST_ITH_PORT;
	pRs485Info->parity = 0;
	pRs485Info->txPin = TEST_GPIO_TX;
	pRs485Info->rxPin = TEST_GPIO_RX;
	pRs485Info->enPin = TEST_GPIO_EN;
	pRs485Info->baud = TEST_BAUDRATE;
	pRs485Info->timeout = 0;
	pRs485Info->mode = UART_INTR_MODE;
	pRs485Info->forDbg = false;

	itpRegisterDevice(TEST_PORT, &TEST_DEVICE);

	ioctl(TEST_PORT, ITP_IOCTL_INIT, (void*)pRs485Info);
	ioctl(TEST_PORT, ITP_IOCTL_REG_RS485_CB, (void*)RS485Callback);
	//ioctl(TEST_PORT, ITP_IOCTL_REG_RS485_DEFER_CB , (void*)RS485Callback);

	sem_init(&RS485SemIntr, 0, 0);

	while(1)
	{	
		sem_wait(&RS485SemIntr);

		len = read(TEST_PORT, getstr + count, 256);
		printf("len = %d, getstr = %s\n", len, getstr);
		count += len;
	
		if(count >= RS485CommandLen)
		{
			printf("rs485 read: %s,count=%d\n", getstr, count);
			memcpy(sendstr, getstr, count);
			sendstr[count] = '\n';
			write(TEST_PORT, sendstr, count + 1);

			memset(getstr, 0, count + 1);
			memset(sendstr, 0, count + 1);
			count = 0;
		}
	}
	return NULL;
}
#elif defined(TEST_DMA_MODE)
void* TestFuncUseDMA(void* arg)
{
	int len = 0, count = 0;
	char getstr[256] = "", sendstr[256] = "";

	RS485_OBJ *pRs485Info = (RS485_OBJ*)malloc(sizeof(RS485_OBJ));
	pRs485Info->port = TEST_ITH_PORT;
	pRs485Info->parity = 0;
	pRs485Info->txPin = TEST_GPIO_TX;
	pRs485Info->rxPin = TEST_GPIO_RX;
	pRs485Info->enPin = TEST_GPIO_EN;
	pRs485Info->baud = TEST_BAUDRATE;
	pRs485Info->timeout = 0;
	pRs485Info->mode = UART_DMA_MODE;
	pRs485Info->forDbg = false;

	itpRegisterDevice(TEST_PORT, &TEST_DEVICE);
	ioctl(TEST_PORT, ITP_IOCTL_INIT, (void*)pRs485Info);

	InitRS485Intr(TEST_PORT);

	printf("Start RS485 DMA mode test!\n");

	sem_init(&RS485SemDma, 0, 0);

	while (1)
	{
		sem_wait(&RS485SemDma);
		len = read(TEST_PORT, getstr + count, 256);
		printf("len = %d, getstr = %s\n", len, getstr);
		count += len;

		if (count >= RS485CommandLen)
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

	RS485_OBJ *pRs485Info = (RS485_OBJ*)malloc(sizeof(RS485_OBJ));
	pRs485Info->port = TEST_ITH_PORT;
	pRs485Info->parity = 0;
	pRs485Info->txPin = TEST_GPIO_TX;
	pRs485Info->rxPin = TEST_GPIO_RX;
	pRs485Info->enPin = TEST_GPIO_EN;
	pRs485Info->baud = TEST_BAUDRATE;
	pRs485Info->timeout = 0;
	pRs485Info->mode = UART_FIFO_MODE;
	pRs485Info->forDbg = false;

	itpRegisterDevice(TEST_PORT, &TEST_DEVICE);
	ioctl(TEST_PORT, ITP_IOCTL_INIT, (void*)pRs485Info);

	InitRS485Intr(TEST_PORT);

	printf("Start RS485 FIFO mode test!\n");

	sem_init(&RS485SemFifo, 0, 0);

	while (1)
	{
		sem_wait(&RS485SemFifo);
		len = read(TEST_PORT, getstr + count, 256);
		printf("len = %d, getstr = %s\n", len, getstr);
		count += len;
		if (count >= RS485CommandLen)
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
