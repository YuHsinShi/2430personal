/** @file
* PAL RS485 functions.
*
* @author
* @version 1.0
* @date 20141014
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
#include "uart/interface/rs485.h"

static int RS485_DEFAULT_OBJ[6] =
{
	INIT_RS485_OBJ_DEFAULT(0),
	INIT_RS485_OBJ_DEFAULT(1),
	INIT_RS485_OBJ_DEFAULT(2),
	INIT_RS485_OBJ_DEFAULT(3),
	INIT_RS485_OBJ_DEFAULT(4),
	INIT_RS485_OBJ_DEFAULT(5),
};

static int rs485EnPin[6] = { NULL };

void iteRs485Init(ITHUartPort port, RS485_OBJ *pRs485Obj)
{
	int port_num = UART_JUDGE_PORT(port);
	if (pRs485Obj == NULL)
	{
		rs485EnPin[port_num] = RS485_DEFAULT_OBJ[port_num];
		iteUartInit(port, NULL);
	}
	else
	{
		UART_OBJ *pObj = (UART_OBJ*)malloc(sizeof(UART_OBJ));
		if (pObj == NULL)
		{
			ithPrintf("UART object malloc failed!\n");
			return;
		}
		pObj->port = pRs485Obj->port;
		pObj->parity = pRs485Obj->parity;
		pObj->txPin = pRs485Obj->txPin;
		pObj->rxPin = pRs485Obj->rxPin;
		pObj->baud = pRs485Obj->baud;
		pObj->timeout = pRs485Obj->timeout;
		pObj->mode = pRs485Obj->mode;
		pObj->forDbg = pRs485Obj->forDbg;

		iteUartInit(port, pObj);
		rs485EnPin[UART_JUDGE_PORT(port)] = pRs485Obj->enPin;
		free(pObj);
	}
}

void iteRs485Reset(ITHUartPort port, RS485_OBJ *pRs485Obj)
{
	int port_num = UART_JUDGE_PORT(port);
	if (pRs485Obj == NULL)
	{
		iteUartReset(port, NULL);
	}
	else
	{
		UART_OBJ *pObj = (UART_OBJ*)malloc(sizeof(UART_OBJ));
		if (pObj == NULL)
		{
			ithPrintf("UART object malloc failed!\n");
			return;
		}
		pObj->port = pRs485Obj->port;
		pObj->parity = pRs485Obj->parity;
		pObj->txPin = pRs485Obj->txPin;
		pObj->rxPin = pRs485Obj->rxPin;
		pObj->baud = pRs485Obj->baud;
		pObj->timeout = pRs485Obj->timeout;
		pObj->mode = pRs485Obj->mode;
		pObj->forDbg = pRs485Obj->forDbg;

		iteUartReset(port, pObj);
		rs485EnPin[UART_JUDGE_PORT(port)] = pRs485Obj->enPin;
		free(pObj);
	}
}

int iteRs485Write(ITHUartPort port, char *ptr, int len)
{
	int en_pin = rs485EnPin[UART_JUDGE_PORT(port)];

	if (en_pin != -1) {
		ithGpioSet(en_pin);
		ithGpioSetMode(en_pin, ITH_GPIO_MODE0);
		ithGpioSetOut(en_pin);
	}

	return iteUartWrite(port, (char*)ptr, len);
}

int iteRs485Read(ITHUartPort port, char *ptr, int len)
{
	int en_pin = rs485EnPin[UART_JUDGE_PORT(port)];

	if (en_pin != -1) {
		//wait Tx and Tx Fifo empty and disable TX_ENABLE gpio.
		while (!ithUartIsTxWIthFifoEmpty(port) && ithGpioGet(en_pin)) {
		}
		ithGpioClear(en_pin);
		ithGpioSetMode(en_pin, ITH_GPIO_MODE0);
		ithGpioSetIn(en_pin);
	}
	ithPrintf("RS485 read!\n");
	return iteUartRead(port, (char*)ptr, len);
}

static void Rs485ObjDelete(RS485_OBJ *pRs485Obj)
{
	pRs485Obj->dele = NULL;
	if (pRs485Obj != NULL)
		free(pRs485Obj);
	pRs485Obj = NULL;
}