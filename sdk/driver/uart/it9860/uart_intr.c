#include <stdlib.h>
#include <stdio.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/queue.h"
#include "ite/ith.h"
#include "ite/itp.h"
#include "uart_intr.h"

static ITHIntr _UartIthIntrBase[6] =
{
	ITH_INTR_UART0,
	ITH_INTR_UART1,
	ITH_INTR_UART2,
	ITH_INTR_UART3,
	ITH_INTR_UART4,
	ITH_INTR_UART5,
};

static UART_INTR_OBJ UART_STATIC_INTR_OBJ[6] = {};

static int UartIntrObjInit(UART_OBJ *pUartObj);
static int UartIntrObjSend(UART_OBJ *pUartObj, char *ptr, int len);
static int UartIntrObjRead(UART_OBJ *pUartObj, char *ptr, int len);
static void UartIntrObjTerminate(UART_OBJ *pUartObj);

UART_OBJ* iteNewUartIntrObj(ITHUartPort port, UART_OBJ *pUartObj)
{
	UART_OBJ* pObj = iteNewUartObj(port, pUartObj);
	if (pObj == NULL)
		return NULL;

	UART_INTR_OBJ* pUartIntrObj = &UART_STATIC_INTR_OBJ[UART_JUDGE_PORT(port)];

	pObj->pMode = pUartIntrObj;

	//initialising pMode class members
	pUartIntrObj->Intr = _UartIthIntrBase[UART_JUDGE_PORT(pObj->port)];
	pUartIntrObj->xRxQueue = NULL;
	pUartIntrObj->xTxQueue = NULL;
	pUartIntrObj->RxQueueFull = 0;
	pUartIntrObj->UartDeferIntrOn = 0;
	pUartIntrObj->itpUartDeferIntrHandler = NULL;

	//Changing base class interface to access pMode class functions
	pObj->init = UartIntrObjInit;
	pObj->send = UartIntrObjSend;
	pObj->read = UartIntrObjRead;
	pObj->dele = UartIntrObjTerminate;

	return pObj;
}

static void _UartDefaultIntrHandler(void* arg1, uint32_t arg2)
{
	// DO NOTHING
}

static void _UartIntrHandler(void *arg)
{
	UART_OBJ* pUartObj			= (UART_OBJ*)arg;
	UART_INTR_OBJ* pUartIntrObj	= pUartObj->pMode;

	uint32_t      status = ithUartClearIntr(pUartObj->port);
	signed char   cChar = 0;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	if (status & ITH_UART_RECV_READY)
	{
		/* The interrupt was caused by a character being received. Grab the
		character from the RHR and place it in the queue or received
		characters. */
		while (ithUartIsRxReady(pUartObj->port))
		{
			cChar = ithUartGetChar(pUartObj->port);

			if (xQueueSendFromISR(pUartIntrObj->xRxQueue, &cChar, &xHigherPriorityTaskWoken) == pdFALSE)
				pUartIntrObj->RxQueueFull = 1;
			if (pUartIntrObj->UartDeferIntrOn)
				itpPendFunctionCallFromISR(pUartIntrObj->itpUartDeferIntrHandler, NULL, 0);
			else
				pUartIntrObj->itpUartDeferIntrHandler(NULL, 0);
		}
	}
	//else if (status & ITH_UART_THR_EMPTY)
	if (status & ITH_UART_THR_EMPTY)
	{
		/* The interrupt was caused by the THR becoming empty. Are there any
		more characters to transmit? */
		for (;;)
		{
			if (xQueueReceiveFromISR(pUartIntrObj->xTxQueue, &cChar, &xHigherPriorityTaskWoken) == pdTRUE)
			{
				/* A character was retrieved from the queue so can be sent to the THR now. */
				ithUartPutChar(pUartObj->port, cChar);

				if (ithUartIsTxFull(pUartObj->port))
					break;
			}
			else
			{
				/* Queue empty, nothing to send so turn off the Tx interrupt. */
				ithUartDisableIntr(pUartObj->port, ITH_UART_TX_READY);
				break;
			}
		}
	}
}

static int UartIntrObjInit(UART_OBJ *pUartObj)
{
	UART_INTR_OBJ* pUartIntrObj = pUartObj->pMode;

	/* Set the required protocol. */
	ithUartReset(pUartObj->port, pUartObj->baud, pUartObj->parity, 1, 8);

	ithUartSetMode(pUartObj->port, ITH_UART_DEFAULT, pUartObj->txPin, pUartObj->rxPin);

	if (!pUartIntrObj->xTxQueue && !pUartIntrObj->xRxQueue)
	{
		/* Create the queues used to hold Rx and Tx characters. */
		pUartIntrObj->xRxQueue = xQueueCreate(UART_INTR_BUF_SIZE, (unsigned portBASE_TYPE) sizeof(signed char));
		pUartIntrObj->xTxQueue = xQueueCreate(UART_INTR_BUF_SIZE + 1, (unsigned portBASE_TYPE) sizeof(signed char));
	}

	pUartIntrObj->UartDeferIntrOn = 0;
	pUartIntrObj->itpUartDeferIntrHandler = _UartDefaultIntrHandler;

	ithEnterCritical();
	/* Enable the Rx interrupts.  The Tx interrupts are not enabled
	until there are characters to be transmitted. */
	ithIntrDisableIrq(pUartIntrObj->Intr);
	ithUartClearIntr(pUartObj->port);
	ithIntrClearIrq(pUartIntrObj->Intr);

	ithIntrSetTriggerModeIrq(pUartIntrObj->Intr, ITH_INTR_LEVEL);
	ithIntrRegisterHandlerIrq(pUartIntrObj->Intr, _UartIntrHandler, (void *)pUartObj);
	ithUartEnableIntr(pUartObj->port, ITH_UART_RX_READY);

	/* Enable the interrupts. */
	ithIntrEnableIrq(pUartIntrObj->Intr);
	ithExitCritical();

	return 0;
}

static int UartIntrObjSend(UART_OBJ *pUartObj, char *ptr, int len)
{
	UART_INTR_OBJ* pUartIntrObj = pUartObj->pMode;
	int count = 0;

	do
	{
		/* Place the character in the queue of characters to be transmitted. */
		if (xQueueSend(pUartIntrObj->xTxQueue, &ptr[count], 0) != pdPASS)
		{
			// queue stuck use FIFO instead
			if (ithUartIsTxEmpty(pUartObj->port))
			{
				signed char cChar = 0;
				if (xQueueReceive(pUartIntrObj->xTxQueue, &cChar, 0) == pdTRUE)
					ithUartPutChar(pUartObj->port, cChar);
			}
			ithUartEnableIntr(pUartObj->port, ITH_UART_TX_READY);
		}
		else
			count++;
	} while (count < len);
	
	if (ithUartIsTxEmpty(pUartObj->port)) /* prevent dbg stuck */
	{
		signed char cChar;

		if (xQueueReceive(pUartIntrObj->xTxQueue, &cChar, 0) == pdTRUE)
		{
			ithUartPutChar(pUartObj->port, cChar);
		}
	}

	ithUartEnableIntr(pUartObj->port, ITH_UART_TX_READY);

	return count;
}

static int UartIntrObjRead(UART_OBJ *pUartObj, char *ptr, int len)
{
	UART_INTR_OBJ* pUartIntrObj = pUartObj->pMode;
	uint32_t  lasttime = 0;
	int count = 0;
	
	if (pUartObj->timeout)
	{
		if (uxQueueMessagesWaiting(pUartIntrObj->xRxQueue))
		{
			lasttime = itpGetTickCount();
			while (count < len)
			{
				if (xQueueReceive(pUartIntrObj->xRxQueue, ptr, 0))
				{
					count++;
					ptr++;
				}
				else if (itpGetTickDuration(lasttime) < pUartObj->timeout)
				{
					usleep(50);
				}
				else
					break;
			}
		}
	}
	else
	{
		if (uxQueueMessagesWaiting(pUartIntrObj->xRxQueue))
		{
			while (count < len)
			{
				if (xQueueReceive(pUartIntrObj->xRxQueue, ptr, 0))
				{
					count++;
					ptr++;
				}
				else
					break;
			}
		}
	}
	if (pUartIntrObj->RxQueueFull)
		ithPrintf("rxqueue is Full\n");
	return count;
}

static void UartIntrObjTerminate(UART_OBJ *pUartObj)
{
	UART_INTR_OBJ* pUartIntrObj = pUartObj->pMode;

	if (pUartIntrObj->xTxQueue && pUartIntrObj->xRxQueue)
	{
		/* Delete the queues used to hold Rx and Tx characters. */
		vQueueDelete(pUartIntrObj->xRxQueue);
		vQueueDelete(pUartIntrObj->xTxQueue);
	}

	pUartIntrObj->UartDeferIntrOn = 0;
	pUartIntrObj->itpUartDeferIntrHandler = NULL;

	ithEnterCritical();

	ithIntrDisableIrq(pUartIntrObj->Intr);
	ithUartClearIntr(pUartObj->port);
	ithIntrClearIrq(pUartIntrObj->Intr);

	ithExitCritical();

	pUartIntrObj->Intr = -1;
	pUartIntrObj->xRxQueue = NULL;
	pUartIntrObj->xTxQueue = NULL;
	pUartIntrObj->RxQueueFull = 0;
	pUartIntrObj->UartDeferIntrOn = 0;
	pUartIntrObj->itpUartDeferIntrHandler = NULL;

	pUartObj->init = NULL;
	pUartObj->dele = NULL;
	pUartObj->send = NULL;
	pUartObj->read = NULL;

	pUartIntrObj = NULL;

	pUartObj = NULL;
}
