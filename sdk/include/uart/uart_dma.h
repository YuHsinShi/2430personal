#include "uart/uart.h"

/* User could change DMA buffer size by editing project Kconfig */
#ifdef CFG_UART_DMA_BUF_SIZE
#define UART_DMA_BUF_SIZE CFG_UART_DMA_BUF_SIZE 
#else
#define UART_DMA_BUF_SIZE 1024
#endif

#define UART_DMA_BASE(port) \
	{\
	"dma_uart"#port"_read", \
	"dma_uart"#port"_write", \
	ITH_DMA_UART##port##_RX, \
	ITH_DMA_UART##port##_TX, \
	}

typedef struct _UART_DMA_OBJ UART_DMA_OBJ;

typedef struct LLP_TAG
{
	uint32_t  SrcAddr;
	uint32_t  DstAddr;
	uint32_t  LLP;
	uint32_t  Control;
	uint32_t  TotalSize;
}LLP;

typedef struct _UART_DMA_OBJ
{
	UART_OBJ*	pBaseObj;
	char*		pRxChName;
	char*		pTxChName;
	int			rxReqNum;
	int			txReqNum;
	int			rxChNum;
	int			txChNum;
	uint8_t*	pDmaSrc;
	uint8_t*	pDmaBuf;
	uint32_t	wtIndex;
	uint32_t	rdIndex;
	LLP*		LLPCtxt;
}UART_DMA_OBJ;

typedef struct _UART_DMA_BASE
{
	char*	pRxChName;
	char*	pTxChName;
	int		rxReqNum;
	int		txReqNum;
}UART_DMA_BASE;

UART_OBJ* iteNewUartDmaObj(ITHUartPort port, UART_OBJ *uart_obj);
