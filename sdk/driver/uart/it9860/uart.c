/** @file
* PAL UART functions.
*
* @author Jim Tan
* @version 1.0
* @date 2011-2012
* @copyright ITE Tech. Inc. All Rights Reserved.
*/
#include <errno.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/queue.h"
#include "ite/ith.h"
#include "ite/itp.h"
#include "uart/uart.h"
#include "uart/uart_intr.h"
#include "uart/uart_dma.h"
#include "uart/uart_fifo.h"

static UART_OBJ UART_CFG_UART_OBJ[6] =
{
	INIT_UART_OBJ_DEFAULT(0),
	INIT_UART_OBJ_DEFAULT(1),
	INIT_UART_OBJ_DEFAULT(2),
	INIT_UART_OBJ_DEFAULT(3),
	INIT_UART_OBJ_DEFAULT(4),
	INIT_UART_OBJ_DEFAULT(5),
};

static UART_OBJ UART_STATIC_OBJ[6] = {};
static UART_OBJ UART_STATIC_BACKUP_OBJ = {};

static UART_OBJ* pUartPortObj[6] = { NULL };

static UART_OBJ* pUartDbgObj;
static int UartDbgPutchar(int c);
static void UartSwtichMode(ITHUartPort port, int mode);
static void UartObjDelete(UART_OBJ *pUartObj);

UART_OBJ* iteNewUartObj(ITHUartPort port, UART_OBJ *pUartObj)
{
	UART_OBJ *pObj = &UART_STATIC_OBJ[UART_JUDGE_PORT(port)];

	pObj->port = pUartObj->port;
	pObj->parity = pUartObj->parity;
	pObj->txPin = pUartObj->txPin;
	pObj->rxPin = pUartObj->rxPin;
	pObj->baud = pUartObj->baud;
	pObj->timeout = pUartObj->timeout;
	pObj->mode = pUartObj->mode;
	pObj->forDbg = pUartObj->forDbg;
	pObj->pMode = pObj;
	pObj->init = NULL;
	pObj->dele = UartObjDelete;
	pObj->send = NULL;
	pObj->read = NULL;

	return pObj;
}

static void UartObjDelete(UART_OBJ *pUartObj)
{
	pUartObj->dele = NULL;
	pUartObj = NULL;
}

void iteUartInit(ITHUartPort port, UART_OBJ* pUartObj)
{
	int port_num = UART_JUDGE_PORT(port);
	if (pUartObj == NULL)
		pUartObj = &UART_CFG_UART_OBJ[port_num];
	switch (pUartObj->mode)
	{
	case UART_INTR_MODE:
		pUartPortObj[port_num] = iteNewUartIntrObj(port, pUartObj);
		break;
	case UART_DMA_MODE:
		pUartPortObj[port_num] = iteNewUartDmaObj(port, pUartObj);
		break;
	case UART_FIFO_MODE:
		pUartPortObj[port_num] = iteNewUartFifoObj(port, pUartObj);
		break;
	}

	if (pUartPortObj[port_num]->init(pUartPortObj[port_num]))
	{
		ithPrintf("UART port%d init failed!\n", port_num);
		return;
	}

	if (pUartPortObj[port_num]->forDbg)
	{
		pUartDbgObj = pUartPortObj[port_num];
		ithPutcharFunc = UartDbgPutchar;
	}
}

void iteUartTerminate(ITHUartPort port)
{
	UART_OBJ *pObj = pUartPortObj[UART_JUDGE_PORT(port)];
	pObj->dele(pObj);
}

void iteUartReset(ITHUartPort port, UART_OBJ* pUartObj)
{
	if (pUartPortObj[UART_JUDGE_PORT(port)] != NULL)
		iteUartTerminate(port);
	iteUartInit(port, pUartObj);
}

void iteUartOpen(ITHUartPort port, ITHUartParity Parity)
{
	/* Wait for removing. */
}

int iteUartWrite(ITHUartPort port, char *ptr, int len)
{
	UART_OBJ *pObj = pUartPortObj[UART_JUDGE_PORT(port)];

	return pObj->send(pObj, ptr, len);
}

int iteUartRead(ITHUartPort port, char *ptr, int len)
{
	UART_OBJ *pObj = pUartPortObj[UART_JUDGE_PORT(port)];

	return pObj->read(pObj, ptr, len);
}

void iteUartRegisterCallBack(ITHUartPort port, void *ptr)
{
	UART_INTR_OBJ* pUartIntrObj = pUartPortObj[UART_JUDGE_PORT(port)]->pMode;
	pUartIntrObj->itpUartDeferIntrHandler = (ITPPendFunction)ptr;
}

void iteUartRegisterDeferCallBack(ITHUartPort port, void *ptr)
{
	UART_INTR_OBJ* pUartIntrObj = pUartPortObj[UART_JUDGE_PORT(port)]->pMode;
	pUartIntrObj->UartDeferIntrOn = 1;
	pUartIntrObj->itpUartDeferIntrHandler = (ITPPendFunction)ptr;
}

void iteUartSetTimeout(ITHUartPort port, uint32_t ptr)
{
	UART_OBJ *pObj = pUartPortObj[UART_JUDGE_PORT(port)];
	pObj->timeout = ptr;
}

void iteUartSetGpio(ITHUartPort port, ITHUartConfig *pUartConfig)
{
	/* Wait for removing. */
}

void iteUartStopDMA(ITHUartPort port)
{
	UartSwtichMode(port, UART_FIFO_MODE);
}

static int UartDbgPutchar(int c)
{
	if (ithGetCpuMode() == ITH_CPU_SYS)
	{
		char cChar = (char)c;
		pUartDbgObj->send(pUartDbgObj, &cChar, 1);
	}
	else
	{
		while (ithUartIsTxFull(pUartDbgObj->port));
		ithUartPutChar(pUartDbgObj->port, (char)c);
	}
	return c;
}

static void UartSwtichMode(ITHUartPort port, int mode)
{
	UART_STATIC_BACKUP_OBJ = UART_STATIC_OBJ[UART_JUDGE_PORT(port)];

	UART_STATIC_BACKUP_OBJ.mode = mode;

	iteUartReset(port, &UART_STATIC_BACKUP_OBJ);
}