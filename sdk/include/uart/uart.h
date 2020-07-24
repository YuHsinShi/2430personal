#ifndef UART_H
#define UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "openrtos/FreeRTOS.h"
#include "openrtos/queue.h"

#if defined(CFG_UART0_INTR) || defined(CFG_UART1_INTR)\
	|| defined(CFG_UART2_INTR) || defined(CFG_UART3_INTR)\
	|| defined(CFG_UART4_INTR) || defined(CFG_UART5_INTR)
#define ENABLE_UART_INTR
#endif

#if defined(CFG_UART0_DMA) || defined(CFG_UART1_DMA)\
	|| defined(CFG_UART2_DMA) || defined(CFG_UART3_DMA)\
	|| defined(CFG_UART4_DMA) || defined(CFG_UART5_DMA)
#define ENABLE_UART_DMA
#endif

#if defined(CFG_UART0_FIFO) || defined(CFG_UART1_FIFO)\
	|| defined(CFG_UART2_FIFO) || defined(CFG_UART3_FIFO)\
	|| defined(CFG_UART4_FIFO) || defined(CFG_UART5_FIFO)
#define ENABLE_UART_FIFO
#endif

#ifdef CFG_GPIO_UART0_RX
#define UART0_TX CFG_GPIO_UART0_TX
#define UART0_RX CFG_GPIO_UART0_RX
#else
#define UART0_TX -1
#define UART0_RX -1
#endif

#ifdef CFG_UART0_INTR
#define UART0_MODE UART_INTR_MODE
#elif CFG_UART0_DMA
#define UART0_MODE UART_DMA_MODE
#else
#define UART0_MODE UART_FIFO_MODE
#endif

#ifdef CFG_GPIO_UART1_RX
#define UART1_TX CFG_GPIO_UART1_TX
#define UART1_RX CFG_GPIO_UART1_RX
#else
#define UART1_TX -1
#define UART1_RX -1
#endif

#ifdef CFG_UART1_INTR
#define UART1_MODE UART_INTR_MODE
#elif CFG_UART1_DMA
#define UART1_MODE UART_DMA_MODE
#else
#define UART1_MODE UART_FIFO_MODE
#endif

#ifdef CFG_GPIO_UART2_RX
#define UART2_TX CFG_GPIO_UART2_TX
#define UART2_RX CFG_GPIO_UART2_RX
#else
#define UART2_TX -1
#define UART2_RX -1
#endif

#ifdef CFG_UART2_INTR
#define UART2_MODE UART_INTR_MODE
#elif CFG_UART2_DMA
#define UART2_MODE UART_DMA_MODE
#else
#define UART2_MODE UART_FIFO_MODE
#endif

#ifdef CFG_GPIO_UART3_RX
#define UART3_TX CFG_GPIO_UART3_TX
#define UART3_RX CFG_GPIO_UART3_RX
#else
#define UART3_TX -1
#define UART3_RX -1
#endif

#ifdef CFG_UART3_INTR
#define UART3_MODE UART_INTR_MODE
#elif CFG_UART3_DMA
#define UART3_MODE UART_DMA_MODE
#else
#define UART3_MODE UART_FIFO_MODE
#endif

#ifdef CFG_GPIO_UART4_RX
#define UART4_TX CFG_GPIO_UART4_TX
#define UART4_RX CFG_GPIO_UART4_RX
#else
#define UART4_TX -1
#define UART4_RX -1
#endif

#ifdef CFG_UART4_INTR
#define UART4_MODE UART_INTR_MODE
#elif CFG_UART4_DMA
#define UART4_MODE UART_DMA_MODE
#else
#define UART4_MODE UART_FIFO_MODE
#endif

#ifdef CFG_GPIO_UART5_RX
#define UART5_TX CFG_GPIO_UART5_TX
#define UART5_RX CFG_GPIO_UART5_RX
#else
#define UART5_TX -1
#define UART5_RX -1
#endif

#ifdef CFG_UART5_INTR
#define UART5_MODE UART_INTR_MODE
#elif CFG_UART5_DMA
#define UART5_MODE UART_DMA_MODE
#else
#define UART5_MODE UART_FIFO_MODE
#endif

#ifdef CFG_DBG_UART0
#define UART0_DBG 1
#else
#define UART0_DBG 0
#endif

#ifdef CFG_DBG_UART1
#define UART1_DBG 1
#else
#define UART1_DBG 0
#endif

#ifdef CFG_DBG_UART2
#define UART2_DBG 1
#else
#define UART2_DBG 0
#endif

#ifdef CFG_DBG_UART3
#define UART3_DBG 1
#else
#define UART3_DBG 0
#endif

#ifdef CFG_DBG_UART4
#define UART4_DBG 1
#else
#define UART4_DBG 0
#endif

#ifdef CFG_DBG_UART5
#define UART5_DBG 1
#else
#define UART5_DBG 0
#endif

#ifdef CFG_UART0_BAUDRATE
#define UART0_BAUDRATE CFG_UART0_BAUDRATE
#else
#define UART0_BAUDRATE -1
#endif

#ifdef CFG_UART1_BAUDRATE
#define UART1_BAUDRATE CFG_UART1_BAUDRATE
#else
#define UART1_BAUDRATE -1
#endif

#ifdef CFG_UART2_BAUDRATE
#define UART2_BAUDRATE CFG_UART2_BAUDRATE
#else
#define UART2_BAUDRATE -1
#endif

#ifdef CFG_UART3_BAUDRATE
#define UART3_BAUDRATE CFG_UART3_BAUDRATE
#else
#define UART3_BAUDRATE -1
#endif

#ifdef CFG_UART4_BAUDRATE
#define UART4_BAUDRATE CFG_UART4_BAUDRATE
#else
#define UART4_BAUDRATE -1
#endif

#ifdef CFG_UART5_BAUDRATE
#define UART5_BAUDRATE CFG_UART5_BAUDRATE
#else
#define UART5_BAUDRATE -1
#endif



#define UART_JUDGE_PORT(port) ((port - ITH_UART0) >> 8)

typedef struct _UART_OBJ UART_OBJ;

typedef int(*fpUartInit)(UART_OBJ*);
typedef void(*fpUartDele)(UART_OBJ*);
typedef int(*fpUartSend)(UART_OBJ*, char *ptr, int len);
typedef int(*fpUartRead)(UART_OBJ*, char *ptr, int len);

typedef struct _UART_OBJ
{
	ITHUartPort		port;
	ITHUartParity   parity;
	int				txPin;
	int				rxPin;
	int				baud;
	uint32_t		timeout;
	uint8_t			mode;
	bool			forDbg;
	fpUartInit		init;
	fpUartDele		dele;
	fpUartSend		send;
	fpUartRead		read;
	void			*pMode;
}UART_OBJ;

typedef enum
{
	UART_INTR_MODE,
	UART_DMA_MODE,
	UART_FIFO_MODE,
} UartMode;

#define INIT_UART_OBJ_DEFAULT(port) \
{\
	ITH_UART##port, \
	0,\
	UART##port##_TX, \
	UART##port##_RX, \
	UART##port##_BAUDRATE, \
	0, \
	UART##port##_MODE, \
	UART##port##_DBG, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
}

UART_OBJ* iteNewUartObj(ITHUartPort port, UART_OBJ *uart_obj);

/**
* @brief Set debug message callback function.
*
* @param port UART debug port.
*
*/
void iteUartInit(ITHUartPort port, UART_OBJ *uart_obj);

/**
* @brief Set UART port's parity.
*
* @param port UART device port.
*
* @param Parity UART device port's Parity.
*
*/
void iteUartOpen(ITHUartPort port, ITHUartParity Parity);

/**
* @brief Initialize UART port. All detail are set in Kconfig setting time.
*
* @param port UART device port.
*
* @param baud UART device port's baudrate.
*
*/
void iteUartReset(ITHUartPort port, UART_OBJ* pUartObj);

/**
* @brief Fetch data from receive queue.
*
* @param port UART device port.
*
* @return fetched data length.
*
*/
int iteUartRead(ITHUartPort port, char *ptr, int len);

/**
* @brief Deliver data to transmit queue.
*
* @param port UART device port.
*
* @return delivered data length.
*
*/
int iteUartWrite(ITHUartPort port, char *prt, int len);

/**
* @brief Register interrupt callback function.
*
* @param port UART device port.
*
* @param *ptr potinter of callback function.
*
*/
void iteUartRegisterCallBack(ITHUartPort port, void *ptr);

/**
* @brief Register interrupt defer callback function.
*
* @param port UART device port.
*
* @param *ptr potinter of defer callback function.
*
*/
void iteUartRegisterDeferCallBack(ITHUartPort port, void *ptr);

/**
* @brief Set time out value.
*
* @param port UART device port.
*
* @param ptr timeout value.
*
*/
void iteUartSetTimeout(ITHUartPort port, uint32_t ptr);

/**
* @brief Set UART TX/RX GPIO.
*
* @param port UART device port.
*
* @param *pUartConfig a pointer to a structure of ITHUartConfig.
*
*/
void iteUartSetGpio(ITHUartPort port, ITHUartConfig *pUartConfig);

/**
* @brief Stop specific port's DMA function.
*
* @param port UART device port.
*
*/
void iteUartStopDMA(ITHUartPort port);

#ifdef __cplusplus
}
#endif

#endif
