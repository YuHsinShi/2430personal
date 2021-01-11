#ifndef IRDA_H
#define IRDA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "openrtos/queue.h"

#define IR_JUDGE_PORT(port) ((port - ITH_IR0) >> 20 & 3)

#if defined(CFG_IR0_INTR) || defined(CFG_IR1_INTR)\
	|| defined(CFG_IR2_INTR) || defined(CFG_IR3_INTR)
#define ENABLE_IR_INTR
#endif

#if defined(CFG_IR0_DMA) || defined(CFG_IR1_DMA)\
	|| defined(CFG_IR2_DMA) || defined(CFG_IR3_DMA)
#define ENABLE_IR_DMA
#endif

#if defined(CFG_IR0_FIFO) || defined(CFG_IR1_FIFO)\
	|| defined(CFG_IR2_FIFO) || defined(CFG_IR3_FIFO)
#define ENABLE_IR_FIFO
#endif

#if defined(CFG_IR0_RX_MOD) || defined(CFG_IR1_RX_MOD)\
	|| defined(CFG_IR2_RX_MOD) || defined(CFG_IR3_RX_MOD)
#define ENABLE_IR_RX_MOD
#endif

#if defined(CFG_IR0_TX_MOD) || defined(CFG_IR1_TX_MOD)\
	|| defined(CFG_IR2_TX_MOD) || defined(CFG_IR3_TX_MOD)
#define ENABLE_IR_TX_MOD
#endif

#if defined(CFG_IR0_RX_SAMPLE) || defined(CFG_IR1_RX_SAMPLE)\
	|| defined(CFG_IR2_RX_SAMPLE) || defined(CFG_IR3_RX_SAMPLE)
#define ENABLE_IR_RX_SAMPLE
#endif

#ifdef CHECK_VALID_TX_CODE
#define MAX_CODE_INDEX  2
static const uint32_t irValidTxCodeTable[] =
{
#include "ir_TxTable.inc"
};
#endif
#ifdef ENABLE_DBG_COMPARE_CODE
uint16_t g_IrTxBuf[] = {
	0x7c, 0x15, 0x15, 0x0a, 0x0a, 0x0b, 0x0b, 0x0a,
	0x0b, 0x0a, 0x0a, 0x15, 0x15, 0x14, 0x15, 0x15,
	0x15, 0x14, 0x0b, 0x0a, 0x0a, 0x15, 0x0a, 0x0b,
	0x0a, 0x0b, 0x14, 0x15, 0x15, 0x0a, 0x15, 0x14,
	0x15, 0x174, 0x68
};
static int  g_RxIndex = 0;
#endif

#ifdef CFG_GPIO_IR0_RX
#define IR0_TX CFG_GPIO_IR0_TX
#define IR0_RX CFG_GPIO_IR0_RX
#else
#define IR0_TX -1
#define IR0_RX -1
#endif

#ifdef CFG_IR0_VENDOR_CODE
#define IR0_VENDOR_CODE CFG_IR0_VENDOR_CODE
#else
#define IR0_VENDOR_CODE 0
#endif

#ifdef CFG_IR0_PRESS_INTERVAL
#define IR0_PRESS_INTERVAL CFG_IR0_PRESS_INTERVAL
#else
#define IR0_PRESS_INTERVAL 0
#endif

#ifdef CFG_IR0_RX_MOD
#define IR0_RX_MOD 1
#else
#define IR0_RX_MOD 0
#endif

#ifdef CFG_IR0_TX_MOD
#define IR0_TX_MOD 1
#else
#define IR0_TX_MOD 0
#endif

#ifdef CFG_IR0_INTR
#define IR0_MODE ITH_INTR_MODE
#elif CFG_IR0_DMA
#define IR0_MODE ITH_DMA_MODE
#else
#define IR0_MODE ITH_FIFO_MODE
#endif

#ifdef CFG_GPIO_IR1_RX
#define IR1_TX CFG_GPIO_IR1_TX
#define IR1_RX CFG_GPIO_IR1_RX
#else
#define IR1_TX -1
#define IR1_RX -1
#endif

#ifdef CFG_IR1_VENDOR_CODE
#define IR1_VENDOR_CODE CFG_IR1_VENDOR_CODE
#else
#define IR1_VENDOR_CODE 0
#endif

#ifdef CFG_IR1_PRESS_INTERVAL
#define IR1_PRESS_INTERVAL CFG_IR1_PRESS_INTERVAL
#else
#define IR1_PRESS_INTERVAL 0
#endif

#ifdef CFG_IR1_RX_MOD
#define IR1_RX_MOD 1
#else
#define IR1_RX_MOD 0
#endif

#ifdef CFG_IR1_TX_MOD
#define IR1_TX_MOD 1
#else
#define IR1_TX_MOD 0
#endif

#ifdef CFG_IR1_INTR
#define IR1_MODE ITH_INTR_MODE
#elif CFG_IR1_DMA
#define IR1_MODE ITH_DMA_MODE
#else
#define IR1_MODE ITH_FIFO_MODE
#endif

#ifdef CFG_GPIO_IR2_RX
#define IR2_TX CFG_GPIO_IR2_TX
#define IR2_RX CFG_GPIO_IR2_RX
#else
#define IR2_TX -1
#define IR2_RX -1
#endif

#ifdef CFG_IR2_VENDOR_CODE
#define IR2_VENDOR_CODE CFG_IR2_VENDOR_CODE
#else
#define IR2_VENDOR_CODE 0
#endif

#ifdef CFG_IR2_PRESS_INTERVAL
#define IR2_PRESS_INTERVAL CFG_IR2_PRESS_INTERVAL
#else
#define IR2_PRESS_INTERVAL 0
#endif

#ifdef CFG_IR2_RX_MOD
#define IR2_RX_MOD 1
#else
#define IR2_RX_MOD 0
#endif

#ifdef CFG_IR2_TX_MOD
#define IR2_TX_MOD 1
#else
#define IR2_TX_MOD 0
#endif

#ifdef CFG_IR2_INTR
#define IR2_MODE ITH_INTR_MODE
#elif CFG_IR2_DMA
#define IR2_MODE ITH_DMA_MODE
#else
#define IR2_MODE ITH_FIFO_MODE
#endif

#ifdef CFG_GPIO_IR3_RX
#define IR3_TX CFG_GPIO_IR3_TX
#define IR3_RX CFG_GPIO_IR3_RX
#else
#define IR3_TX -1
#define IR3_RX -1
#endif

#ifdef CFG_IR3_VENDOR_CODE
#define IR3_VENDOR_CODE CFG_IR3_VENDOR_CODE
#else
#define IR3_VENDOR_CODE 0
#endif

#ifdef CFG_IR3_PRESS_INTERVAL
#define IR3_PRESS_INTERVAL CFG_IR3_PRESS_INTERVAL
#else
#define IR3_PRESS_INTERVAL 0
#endif

#ifdef CFG_IR3_RX_MOD
#define IR3_RX_MOD 1
#else
#define IR3_RX_MOD 0
#endif

#ifdef CFG_IR3_TX_MOD
#define IR3_TX_MOD 1
#else
#define IR3_TX_MOD 0
#endif

#ifdef CFG_IR3_INTR
#define IR3_MODE ITH_INTR_MODE
#elif CFG_IR3_DMA
#define IR3_MODE ITH_DMA_MODE 
#else
#define IR3_MODE ITH_FIFO_MODE
#endif

#ifdef ENABLE_IR_DMA
#define INIT_IR_OBJECT(port) \
	{\
	ITH_IR##port, \
	port, \
	ITH_INTR_IR##port##TX, \
	ITH_INTR_IR##port##RX, \
	IR##port##_TX, \
	IR##port##_RX, \
	IR##port##_VENDOR_CODE, \
	IR##port##_PRESS_INTERVAL, \
	IR##port##_RX_MOD, \
	IR##port##_TX_MOD, \
	IR##port##_MODE, \
	"dma_ir"#port"_write", \
	"dma_ir"#port"_read", \
	ITH_DMA_IR_CAP##port##_TX, \
	ITH_DMA_IR_CAP##port##_RX, \
	}
#else
#define INIT_IR_OBJECT(port) \
	{\
	ITH_IR##port, \
	port, \
	ITH_INTR_IR##port##TX, \
	ITH_INTR_IR##port##RX, \
	IR##port##_TX, \
	IR##port##_RX, \
	IR##port##_VENDOR_CODE, \
	IR##port##_PRESS_INTERVAL, \
	IR##port##_RX_MOD, \
	IR##port##_TX_MOD, \
	IR##port##_MODE, \
	}
#endif

#define IR_DMA_BUFFER_SIZE      1024
#define IR_DMA_TRANSFER_SIZE    IR_DMA_BUFFER_SIZE/2 //16bit, 1:8bit 2:16bit 4:32bit

#define QUEUE_LEN               256
#define STATE_NUM               (10)
#define THRESHOLD_NUM			(7)
#define MAX_VAL                 ((1<<14)-1)

#define PRECISION               1000000 // in micro second
#define SAMP_RATE               10		// 10 times of ir clk sample once 				

#define REPEAT_THRESHOLD_BEGIN  (5)     // accumulate number of repeat-key will start dispatch key
#define REPEAT_THRESHOLD_SPDUP  (2)     // accumulate number of key will change to high speed mode
#define REPEAT_THRESHOLD_HOLD1  (5)     // dispatch a key foreach number of repeat-key at low speed mode
#define REPEAT_THRESHOLD_HOLD2  (1)     // dispatch a key foreach number of repeat-key at high speed mode

enum RCState {
	WAIT_RISING,
	JUDGE_START_OR_REPEAT,
	WAIT_BIT_RISING,
	JUDGE_BIT_ZERO_OR_ONE,
	WAIT_END
};
enum Thresholds {
	SIGNAL_RISING,
	START,
	REPEAT,
	BIT_RISING,
	BIT_ZERO,
	BIT_ONE,
	SIGNAL_END,
	BIT_PER_KEY,
	LSB
};

typedef struct LLP_CONTEXT_TAG
{
	uint32_t  SrcAddr;
	uint32_t  DstAddr;
	uint32_t  LLP;
	uint32_t  Control;
	uint32_t  TotalSize;
}LLP_CONTEXT;

#define IR_CODE_ARRAY_MAX_SIZE 	64
typedef struct _IR_OBJ {
	ITHIrPort port;
	int port_num;
	ITHIntr TxIntr;                 // Tx interrupt number
	ITHIntr	RxIntr;                 // Rx interrupt number
	int TxGpio;
	int RxGpio;
	int irVendorCode;
	int irPressInterval;
	bool irRxMod;
	bool irTxMod;
	ITHIrMode mode;                 // Fifo default, DMA, interrupt mode
#if defined(ENABLE_IR_DMA)
	/* DMA definaton start */
	char *TxChName;                 // Tx DMA channel name
	char *RxChName;                 // Rx DMA channel name
	int TxChannelReq;
	int RxChannelReq;
	int TxChannel;
	int RxChannel;
	uint16_t *TxBuffer;
	uint16_t *RxBuffer;
	int RxReadIdx;
	int RxWriteIdx;
	LLP_CONTEXT *gLLPCtxt;
	/* DMA defination end */
#endif
	QueueHandle_t TxQueue;
	QueueHandle_t RxQueue;
	unsigned long irRecvCode;        // LSB received code
	unsigned char index_irCodeArray;
	unsigned char irCodeArray[IR_CODE_ARRAY_MAX_SIZE];        // LSB received code
	enum RCState irCurrState;
	int irRepeatKeyPress;
	int irRepeatKeyHold;
	int irRepeatKeyFast;
	int irRepeatKeyCnt;
	unsigned char irRecvBitCount;
	int irWidth;
	ITPKeypadEvent irLastEvent;
} IR_OBJ;

/**
* @brief Get IR fatest frequency recorded since IR sampling function started.
*
* @param port IR device port.
*
* @return int fatest frequency.
*
*/
int iteIrGetFreqFast(ITHIrPort port);

/**
* @brief Get IR slowest frequency recorded since IR sampling function started.
*
* @param port IR device port.
*
* @return int slowest frequency.
*
*/
int iteIrGetFreqSlow(ITHIrPort port);

/**
* @brief Get IR average frequency recorded since IR sampling function started.
*
* @param port IR device port.
*
* @return int average frequency.
*
*/
int iteIrGetFreqAvg(ITHIrPort port);

/**
* @brief Get IR newest frequency recorded since IR sampling function started.
*
* @param port IR device port.
*
* @return int newest frequency.
*
*/
int iteIrGetFreqNew(ITHIrPort port);

/**
* @brief Get the fastest IR waveform in high level duty cycle sampled by chip since IR sampling function started.
*
* @param port IR device port.
*
* @return int the fastest IR waveform in high level duty cycle.
*
*/
int iteIrGetHighDCFast(ITHIrPort port);

/**
* @brief Get the fastest IR waveform in low level duty cycle sampled by chip since IR sampling function started.
*
* @param port IR device port.
*
* @return int the fastest IR waveform in low level duty cycle.
*
*/
int iteIrGetLowDCFast(ITHIrPort port);

/**
* @brief Get the slowest IR waveform in high level duty cycle sampled by chip since IR sampling function started.
*
* @param port IR device port.
*
* @return int the slowest IR waveform in high level duty cycle.
*
*/
int iteIrGetHighDCSlow(ITHIrPort port);

/**
* @brief Get the slowest IR waveform in low level duty cycle sampled by chip since IR sampling function started.
*
* @param port IR device port.
*
* @return int the slowest IR waveform in low level duty cycle.
*
*/
int iteIrGetLowDCSlow(ITHIrPort port);

/**
* @brief Get average IR waveform in high level duty cycle sampled by chip since IR sampling function started.
*
* @param port IR device port.
*
* @return int average IR waveform in high level duty cycle.
*
*/
int iteIrGetHighDCAvg(ITHIrPort port);

/**
* @brief Get average IR waveform in low level duty cycle sampled by chip since IR sampling function started.
*
* @param port IR device port.
*
* @return int average IR waveform in low level duty cycle.
*
*/
int iteIrGetLowDCAvg(ITHIrPort port);

/**
* @brief Get the newest IR waveform in high level duty cycle sampled by chip since IR sampling function started.
*
* @param port IR device port.
*
* @return int the newest IR waveform in high level duty cycle.
*
*/
int iteIrGetHighDCNew(ITHIrPort port);

/**
* @brief Get the fastest IR waveform in low level duty cycle sampled by chip since IR sampling function started.
*
* @param port IR device port.
*
* @return int the newest IR waveform in low level duty cycle.
*
*/
int iteIrGetLowDCNew(ITHIrPort port);

/**
* @brief Clear sampled data and restart sampling.
* If ITH_IR_RX_MOD_FILTER_RST_BIT is '1', register won't sample any data.
* If ITH_IR_RX_MOD_FILTER_RST_BIT is '0', register will start sample data.
*
* @param port  IR device port.
*
*/
void iteIrClearClkSample(ITHIrPort port);

/**
* @brief Initialize IR port. All detail are set in Kconfig setting time.
*
* @param port IR device port.
*
*/
int iteIrInit(ITHIrPort port);

/**
* @brief Fetch data from receive queue.
*
* @param port IR device port.
*
* @return size of ITPKeypadEvent. 
*
*/
int iteIrRead(ITHIrPort port, char *ptr);

/**
* @brief Deliver data to transmit queue.
*
* @param port IR device port.
*
* @return 0.
*
*/
int iteIrWrite(ITHIrPort port, char *prt);

#ifdef __cplusplus
}
#endif

#endif
