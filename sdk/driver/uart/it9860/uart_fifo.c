#include <stdlib.h>
#include <stdio.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "uart_fifo.h"

#define TIMEOUT 1000

static int UartFifoObjInit(UART_OBJ *pUartObj);
static int UartFifoObjSend(UART_OBJ *pUartObj, char *ptr, int len);
static int UartFifoObjRead(UART_OBJ *pUartObj, char *ptr, int len);
static void UartFifoObjTerminate(UART_OBJ *pUartObj);

static UART_FIFO_OBJ UART_STATIC_FIFO_OBJ[6] = {};

UART_OBJ* iteNewUartFifoObj(ITHUartPort port, UART_OBJ *pUartObj)
{
	UART_OBJ* pObj = iteNewUartObj(port, pUartObj);
	if (pObj == NULL)
		return NULL;

	UART_FIFO_OBJ* pUartFifoObj = &UART_STATIC_FIFO_OBJ[UART_JUDGE_PORT(port)];

	pObj->pMode = pUartFifoObj;

	//Changing base class interface to access pMode class functions
	pObj->init = UartFifoObjInit;
	pObj->send = UartFifoObjSend;
	pObj->read = UartFifoObjRead;
	pObj->dele = UartFifoObjTerminate;

	return pObj;
}

static int UartFifoObjInit(UART_OBJ *pUartObj)
{
	UART_FIFO_OBJ* pUartFifoObj = pUartObj->pMode;

	/* Set the required protocol. */
	ithUartReset(pUartObj->port, pUartObj->baud, pUartObj->parity, 1, 8);

	ithUartSetMode(pUartObj->port, ITH_UART_DEFAULT, pUartObj->txPin, pUartObj->rxPin);

	return 0;
}

static int UartFifoObjSend(UART_OBJ *pUartObj, char *ptr, int len)
{
	UART_FIFO_OBJ* pUartFifoObj = pUartObj->pMode;
	int count = 0, timeout = TIMEOUT;

#ifdef CFG_UART_FORCE_FLUSH
	ithEnterCritical();
#endif
	while (count < len)
	{
		// Is transmitter ready?
		if (!ithUartIsTxFull(pUartObj->port))
		{
			ithUartPutChar(pUartObj->port, *ptr++);// Write character from uart
			timeout = TIMEOUT;
			count++;
		}
		else if (timeout-- <= 0)
		{
			//ithPrintf("Uart FIFO write timeout!!! count=%d len=%d\n", count, len);
#ifdef CFG_UART_FORCE_FLUSH
			ithExitCritical();
#endif
			return count;
		}
	}
#ifdef CFG_UART_FORCE_FLUSH
	ithExitCritical();
#endif
	return len;
}

static int UartFifoObjRead(UART_OBJ *pUartObj, char *ptr, int len)
{
	UART_FIFO_OBJ* pUartFifoObj = pUartObj->pMode;
	int count = 0, timeout = TIMEOUT;

	while (count < len)
	{
		// Is a character waiting?
		if (ithUartIsRxReady(pUartObj->port))
		{
			*ptr++ = ithUartGetChar(pUartObj->port);// Read character from uart
			timeout = TIMEOUT;
			count++;
		}
		else if (timeout-- <= 0)
		{
			//ithPrintf("Uart FIFO read timeout!!! count=%d len=%d\n", count, len);
			return count;
		}
	}
	return count;
}

static void UartFifoObjTerminate(UART_OBJ *pUartObj)
{
	UART_FIFO_OBJ* pUartFifoObj = pUartObj->pMode;

	pUartObj->init = NULL;
	pUartObj->dele = NULL;
	pUartObj->send = NULL;
	pUartObj->read = NULL;

	pUartFifoObj = NULL;

	pUartObj = NULL;
}
