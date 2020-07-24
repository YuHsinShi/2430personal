#include "uart/uart.h"

/* User could change Interrupt buffer size by editing project Kconfig */
#ifdef CFG_UART_INTR_BUF_SIZE 
#define UART_INTR_BUF_SIZE CFG_UART_INTR_BUF_SIZE 
#else
#define UART_INTR_BUF_SIZE 16384
#endif

typedef struct _UART_INTR_OBJ UART_INTR_OBJ;

typedef struct _UART_INTR_OBJ
{
	UART_OBJ*		pBaseObj;
	ITHIntr			Intr;
	QueueHandle_t	xRxQueue;
	QueueHandle_t	xTxQueue;
	int				RxQueueFull;
	uint8_t			UartDeferIntrOn;
	ITPPendFunction	itpUartDeferIntrHandler;
}UART_INTR_OBJ;

UART_OBJ* iteNewUartIntrObj(ITHUartPort port, UART_OBJ *uart_obj);
