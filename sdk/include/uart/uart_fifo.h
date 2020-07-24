#include "uart/uart.h"

typedef struct _UART_FIFO_OBJ UART_FIFO_OBJ;

typedef struct _UART_FIFO_OBJ
{
	UART_OBJ*	pBaseObj;
}UART_FIFO_OBJ;

UART_OBJ* iteNewUartFifoObj(ITHUartPort port, UART_OBJ *uart_obj);
