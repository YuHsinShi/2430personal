#include "uart/uart.h"

#ifdef CFG_GPIO_RS485_0_TX_ENABLE
#define UART0_EN CFG_GPIO_RS485_0_TX_ENABLE
#else
#define UART0_EN -1
#endif
#ifdef CFG_GPIO_RS485_1_TX_ENABLE
#define UART1_EN CFG_GPIO_RS485_1_TX_ENABLE
#else
#define UART1_EN -1
#endif
#ifdef CFG_GPIO_RS485_2_TX_ENABLE
#define UART2_EN CFG_GPIO_RS485_2_TX_ENABLE
#else
#define UART2_EN -1
#endif
#ifdef CFG_GPIO_RS485_3_TX_ENABLE
#define UART3_EN CFG_GPIO_RS485_3_TX_ENABLE
#else
#define UART3_EN -1
#endif
#ifdef CFG_GPIO_RS485_4_TX_ENABLE
#define UART4_EN CFG_GPIO_RS485_4_TX_ENABLE
#else
#define UART4_EN -1
#endif
#ifdef CFG_GPIO_RS485_5_TX_ENABLE
#define UART5_EN CFG_GPIO_RS485_5_TX_ENABLE
#else
#define UART5_EN -1
#endif

typedef struct _RS485_OBJ RS485_OBJ;

typedef struct _RS485_OBJ
{
	ITHUartPort		port;
	ITHUartParity   parity;
	int				txPin;
	int				rxPin;
	int				enPin;
	int				baud;
	uint32_t		timeout;
	uint8_t			mode;
	bool			forDbg;
	fpUartInit		init;
	fpUartDele		dele;
	fpUartSend		send;
	fpUartRead		read;
	void			*pMode;
}RS485_OBJ;

#define INIT_RS485_OBJ_DEFAULT(port) UART##port##_EN

void iteRs485Init(ITHUartPort port, RS485_OBJ *pRs485Obj);

int iteRs485Write(ITHUartPort port, char *ptr, int len);

int iteRs485Read(ITHUartPort port, char *ptr, int len);
