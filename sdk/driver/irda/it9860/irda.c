/*
* Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
*/
/** @file
* PAL Remote IR functions.
*
* @author Jim Tan
* @version 1.0
*/
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include "openrtos/FreeRTOS.h"
#include "ite/ith.h"
#include "ite/itp.h"
#include "irda/irda.h"

//#define DEBUG_MSG

static const uint32_t irProtocol[4][12] =
{
#ifdef CFG_IR0_RX_PROTOCOL
	{
#include "ir0_protocol.inc" 
	},
#else
	{},
#endif
#ifdef CFG_IR1_RX_PROTOCOL
	{
#include "ir1_protocol.inc" 
	},
#else
	{},
#endif
#ifdef CFG_IR2_RX_PROTOCOL
	{
#include "ir2_protocol.inc" 
	},
#else
	{},
#endif
#ifdef CFG_IR3_RX_PROTOCOL
	{
#include "ir3_protocol.inc" 
	},
#else
	{},
#endif
};

static int irThresholds[4][THRESHOLD_NUM][2];     // min & max for a signal stste

static IR_OBJ IrObj[4] =
{
	INIT_IR_OBJECT(0),
	INIT_IR_OBJECT(1),
	INIT_IR_OBJECT(2),
	INIT_IR_OBJECT(3),
};


static TickType_t start;
IR_OBJ *tmp_obj;
static int timout_counting=0;
static void start_timeout_tick(void)
{
    /* Set first ticks value */
    start = xTaskGetTickCount();
	timout_counting=1;
}

static unsigned int get_elapsed_timeout_msec()
{
    TickType_t tick = xTaskGetTickCount();
    if (tick >= start)
        return ((tick - start) / portTICK_PERIOD_MS);
    else
        return ((0xFFFFFFFF - start + tick) / portTICK_PERIOD_MS);

}


void task_timeoutcheck()
{
		ithPrintf("task_timeoutcheck \n");
int i;
		while(1)
		{
			if(1== timout_counting ) 
			{
				{	
					if(50 < get_elapsed_timeout_msec())
					{
						//force end
						/*
					  ithPrintf("(IR) RECV %d BYTES\n",(tmp_obj->index_irCodeArray-1));
						for(i=1;i<tmp_obj->index_irCodeArray;i++)
								ithPrintf("0x%x ",tmp_obj->irCodeArray[i]);
						ithPrintf("\n");
						*/
					tmp_obj->irCodeArray[0] = (tmp_obj->index_irCodeArray-1);//take first byte as the cmd length


						

					 if( xQueueSend(tmp_obj->RxQueue,  (void *) &(tmp_obj->irCodeArray[0]), (TickType_t)100) != pdPASS )
                    {
                        /* 发送失败，即使等待了10个时钟节拍 */
                    
					ithPrintf("task_timeoutcheck QUEUE Send fail \n");
                    }
                    else
                    {
                        /* 发送成功 */
                                           
                    }
	
						//tmp_obj->irCurrState = WAIT_RISING;
						timout_counting=0;

						//send to cmd quee
						tmp_obj->irCurrState=WAIT_RISING;
						tmp_obj->irRecvBitCount = 0;
						tmp_obj->irRecvCode = 0;
						
						tmp_obj->index_irCodeArray=1;
							
					//	ithPrintf("irRecvBitCount %d \n",tmp_obj->irRecvBitCount);
						ithPrintf("Restart\n");

					}
				}
			}
		
			usleep(10*1000);
		}

}




static int _IrProbe(ITHIrPort port, int signal)
{
	int completeACode = 0, code;
	unsigned char getBit;
	IR_OBJ *ir_obj = &IrObj[IR_JUDGE_PORT(port)];

	if (signal != -1)
	{
#if 0 //def DEBUG_MSG
		ithPrintf("IrProbe(),signal=%d\n", signal);
#endif

#ifdef  ENABLE_DBG_COMPARE_CODE
		{
			uint32_t modFreq = ithReadRegA(ir_obj->port + ITH_IR_RX_MOD_FILTER_REG);

			if (signal == 0x3FFF)
			{
				g_RxIndex = 0;
			}
			else
			{
				int highR, lowR;

				if (20 > g_IrTxBuf[g_RxIndex])
				{
					highR = g_IrTxBuf[g_RxIndex] + 1;
					if (g_IrTxBuf[g_RxIndex]>1)  lowR = g_IrTxBuf[g_RxIndex] - 1;
					else                        lowR = 0;
				}
				else
				{
					highR = g_IrTxBuf[g_RxIndex] + (g_IrTxBuf[g_RxIndex] * 5) / 100;
					lowR = g_IrTxBuf[g_RxIndex] - (g_IrTxBuf[g_RxIndex] * 5) / 100;
				}

				ithPrintf(" ### check ir-RX code, signal=%x, g_RxIndex=%x,H&L=[%x,%x], modfrq=%x\n", signal, g_RxIndex, highR, lowR, modFreq);
				if ((signal > highR) || (signal < lowR))
					//if( signal != g_IrTxBuf[g_RxIndex] )
				{
					ithPrintf("error, IR code RX != TX, index=%x,[%x,%x] \n", g_RxIndex, signal, g_IrTxBuf[g_RxIndex]);
					//while(1);
				}
				g_RxIndex++;
			}
		}
#endif

		switch (ir_obj->irCurrState)
		{
		case WAIT_RISING:
			if ((signal >= irThresholds[ir_obj->port_num][SIGNAL_RISING][0]) && (signal <= irThresholds[ir_obj->port_num][SIGNAL_RISING][1]))
			{
#ifdef DEBUG_MSG
				ithPrintf("RISING\n");
#endif
				ir_obj->irCurrState = JUDGE_START_OR_REPEAT;
			}

			break;
		case JUDGE_START_OR_REPEAT:
			if ((signal >= irThresholds[ir_obj->port_num][START][0]) && (signal <= irThresholds[ir_obj->port_num][START][1]))
			{
#if 1 //def DEBUG_MSG
				ithPrintf("START\n");
#endif
				ir_obj->irCurrState = WAIT_BIT_RISING;
				ir_obj->index_irCodeArray=1;

			}
#ifdef CFG_IR_REPEAT
			else if ((signal >= irThresholds[ir_obj->port_num][REPEAT][0]) && (signal <= irThresholds[ir_obj->port_num][REPEAT][1]))
			{
				ir_obj->irCurrState = WAIT_END;
				code = ir_obj->irRecvCode;
			}
#endif // CFG_IR_REPEAT
			break;
		case WAIT_BIT_RISING:
			
			//ithPrintf(" %d ",signal);
			if ((signal >= irThresholds[ir_obj->port_num][BIT_RISING][0]) && (signal <= irThresholds[ir_obj->port_num][BIT_RISING][1]))
			{
#if 0// def DEBUG_MSG
				ithPrintf("BIT_RISING get\n");
#endif
				ir_obj->irCurrState = JUDGE_BIT_ZERO_OR_ONE;
			}
			else
			{
				ir_obj->irRecvBitCount = 0;
				ir_obj->irRecvCode = 0;
				ir_obj->irCurrState = WAIT_RISING;
			}
			break;
		case JUDGE_BIT_ZERO_OR_ONE:
			
			//ithPrintf(" %d ",signal);
			if ((signal >= irThresholds[ir_obj->port_num][BIT_ZERO][0]) && (signal <= irThresholds[ir_obj->port_num][BIT_ZERO][1]))
			{
				//getBit = 0;
				ir_obj->irCodeArray[ir_obj->index_irCodeArray]&=~(0x01<<ir_obj->irRecvBitCount);
				//ithPrintf("0");
			}
			else if ((signal >= irThresholds[ir_obj->port_num][BIT_ONE][0]) && (signal <= irThresholds[ir_obj->port_num][BIT_ONE][1]))
			{

			
				ir_obj->irCodeArray[ir_obj->index_irCodeArray]|=0x01<<(ir_obj->irRecvBitCount);
				/*
				if (irProtocol[ir_obj->port_num][LSB]) //LSB or not
					getBit = 0x80000000L;
				else
					getBit = 0x00000001L;
				*/
				//ithPrintf("1");
			}
			else
			{
#if 1//def DEBUG_MSG
				ithPrintf("checking bit 0 or 1 error, signal at: %d\n", signal);
				ithPrintf(" zero : [%d, %d] \n", irThresholds[ir_obj->port_num][BIT_ZERO][0], irThresholds[ir_obj->port_num][BIT_ZERO][1]);
				ithPrintf(" one : [%d, %d] \n", irThresholds[ir_obj->port_num][BIT_ONE][0],  irThresholds[ir_obj->port_num][BIT_ONE][1]);

#endif


				ir_obj->irRecvBitCount = 0;
				ir_obj->irRecvCode = 0;
				ir_obj->irCurrState = WAIT_RISING;
				break;
			}
/*
			if (irProtocol[ir_obj->port_num][LSB])
			{
				ir_obj->irRecvCode = (ir_obj->irRecvCode >> 1) | getBit;
			}
			else
			{
				ir_obj->irRecvCode = (ir_obj->irRecvCode << 1) | getBit;
			}
*/
			ir_obj->irRecvBitCount++;

			if(ir_obj->irRecvBitCount==8)
			{
				//ir_obj->irCodeArray[0]=(unsigned char) (ir_obj->irRecvCode);
				//ithPrintf("=[B]%x=\n",ir_obj->irCodeArray[0]);
				//ithPrintf("=%d [0x%x]=\n",ir_obj->index_irCodeArray,ir_obj->irCodeArray[ir_obj->index_irCodeArray]);
				ir_obj->irRecvBitCount=0;
				ir_obj->index_irCodeArray++;
			}

			
			//LAW//if (ir_obj->irRecvBitCount < irProtocol[ir_obj->port_num][BIT_PER_KEY]) // bits per key
				ir_obj->irCurrState = WAIT_BIT_RISING;   // not yet complet a code
			//LAW//else
			//LAW//	ir_obj->irCurrState = WAIT_END;
			//LAW// RECORD the waiting state and use a thread to polling  
			//wait timeout
			start_timeout_tick();

			
			break;

		case WAIT_END:
			if (1) //((signal >= irThresholds[ir_obj->port_num][WAIT_END][0]) && (signal <= irThresholds[ir_obj->port_num][WAIT_END][1]))
			{
				completeACode = 1;
				code = ir_obj->irRecvCode;
				ir_obj->irRecvBitCount = 0;
				ir_obj->irRecvCode = 0;
				
				ir_obj->index_irCodeArray=1;
				ir_obj->irCurrState = WAIT_RISING;
#if 1 //def DEBUG_MSG
				ithPrintf("end\n");
#endif
			}
			else
			{
				ir_obj->irRecvBitCount = 0;
				ir_obj->irRepeatKeyPress = 0;
				ir_obj->irRepeatKeyHold = 0;
				ir_obj->irRepeatKeyFast = 0;
				ir_obj->irRepeatKeyCnt = 0;
				ir_obj->irRecvCode = 0;
				completeACode = 0;
				ir_obj->irCurrState = WAIT_RISING;
			}
			break;

		default:
			ithPrintf(" probe_default!!\n");
			break;
		}

		if (completeACode)
			return code;
	}
	return -1;
}

static void _IrTxSend(ITHIrPort port, int code)
{
	code /= SAMP_RATE;
	ithIrTxTransmit(port, code);
}



static void _IrRxIntrHandler(void* arg)
{
	ITHIrPort port = (ITHIrPort)arg;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	IR_OBJ *ir_obj = &IrObj[IR_JUDGE_PORT(port)];
	int code = 0, signal = 0;
	//ithPrintf("@@@@@@@IrRXIntr happend\n");
	signal = ithIrProbe(ir_obj->port);
	code = _IrProbe(ir_obj->port, signal);
	//ithPrintf("###IR code: 0x%X\n", code);
	if (code != -1)
	{
		if (((code & 0xFFFF) == ir_obj->irVendorCode) && (((code >> 16) & 0xFF) == (~((code >> 24) & 0xFF) & 0xFF)))
			code = (code >> 16) & 0xFF;
		ithPrintf("IR code: 0x%X\n", code);
		xQueueSendFromISR(ir_obj->RxQueue, &code, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

#if 0
static void _IrTxIntrHandler(void *arg)
{
	ITHIrPort port = (ITHIrPort)arg;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	IR_OBJ *ir_obj = &IrObj[IR_JUDGE_PORT(port)];
	int code = 0;

	if (xQueueReceiveFromISR(ir_obj->TxQueue, &code, &xHigherPriorityTaskWoken) == pdTRUE) {
#ifdef DEBUG_MSG
		ithPrintf("got irTX code=%x\n", code);
#endif
		_IrTxSend(port, code);
		//ithPrintf("QUUQUQUQUQU: %d\n", uxQueueMessagesWaitingFromISR(ir_obj->TxQueue));
		//IR-TX finish. So turn off IR-TX interrupt.
		if (uxQueueMessagesWaitingFromISR(ir_obj->TxQueue) == 0) // if queue is empty disable interrupt
		{
			ithPrintf("queue empty\n");
			ithIrTxCtrlDisable(port, ITH_IR_INT);
		}

		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}
#else
static void _IrTxIntrHandler(void* arg)
{
	int code = 0;
	ITHIrPort port = (ITHIrPort)arg;
	IR_OBJ *ir_obj = &IrObj[IR_JUDGE_PORT(port)];
	while (1)
	{
		if (xQueueReceive(ir_obj->TxQueue, &code, 0))
		{
			_IrTxSend(port, code);
		}
		usleep(1000);
	}
}
#endif
static void
DummySleep(void)
{
	unsigned int idle = 100;
	unsigned int i = 0;
	unsigned int nothing = 0;

	for (i = 0; i < idle; i++)
	{
		nothing++;
	}
}

#ifdef ENABLE_IR_DMA
static void *_IrRxDMAHandler(void *arg)
{
	ITHIrPort port = (ITHIrPort)arg;
	uint32_t transferSize = 0, dataSize = 0;
	uint16_t tmpBuffer[IR_DMA_BUFFER_SIZE]; //size is IR_DMA_BUFFER_SIZE
	IR_OBJ *ir_obj = &IrObj[IR_JUDGE_PORT(port)];
	int i, code = 0;

	while (1)
	{
		transferSize = ithReadRegA(ITH_DMA_BASE + ITH_DMA_SIZE_CH(ir_obj->RxChannel));
#ifdef DEBUG_MSG
		ithPrintf("transferSize=%d\n", transferSize);
#endif

		ir_obj->RxWriteIdx = (IR_DMA_TRANSFER_SIZE - transferSize);

		if (ir_obj->RxWriteIdx > IR_DMA_TRANSFER_SIZE) {
			ithPrintf("IR DMA Read fail.\n");
			continue;
		}

		if (ir_obj->RxWriteIdx != ir_obj->RxReadIdx)
		{
#ifdef DEBUG_MSG
			ithPrintf("transferSize=%d RxWriteIdx=%d RxReadIdx=%d\n", transferSize, ir_obj->RxWriteIdx, ir_obj->RxReadIdx);
#endif
			memset(tmpBuffer, 0, IR_DMA_BUFFER_SIZE);

			if (ir_obj->RxWriteIdx < ir_obj->RxReadIdx) {
				dataSize = (IR_DMA_TRANSFER_SIZE - ir_obj->RxReadIdx) + ir_obj->RxWriteIdx;
				ithInvalidateDCacheRange(ir_obj->RxBuffer, IR_DMA_BUFFER_SIZE);
				memcpy(tmpBuffer, ir_obj->RxBuffer + ir_obj->RxReadIdx, (IR_DMA_TRANSFER_SIZE - ir_obj->RxReadIdx)*sizeof(uint16_t));
				memcpy(tmpBuffer + (IR_DMA_TRANSFER_SIZE - ir_obj->RxReadIdx), ir_obj->RxBuffer, ir_obj->RxWriteIdx*sizeof(uint16_t));
			}
			else {
				dataSize = ir_obj->RxWriteIdx - ir_obj->RxReadIdx;
				ithInvalidateDCacheRange(ir_obj->RxBuffer, IR_DMA_BUFFER_SIZE);
				memcpy(tmpBuffer, ir_obj->RxBuffer + ir_obj->RxReadIdx, dataSize*sizeof(uint16_t));
			}

			ir_obj->RxReadIdx = ir_obj->RxWriteIdx;
#ifdef DEBUG_MSG
			ithPrintf("dataSize=%d\n", dataSize);
#endif

			for (i = 0; i < dataSize; i++) {
				uint32_t tmp_reg = 0x0;
				tmp_reg = tmpBuffer[i] & ((0x1 << ir_obj->irWidth) - 1);
				code = _IrProbe(ir_obj->port, tmp_reg);

				if (code != -1) {
					if (((code & 0xFFFF) == ir_obj->irVendorCode) && (((code >> 16) & 0xFF) == (~((code >> 24) & 0xFF) & 0xFF)))
						code = (code >> 16) & 0xFF;

					ithPrintf("code: 0x%X\n", code);

					xQueueSend(ir_obj->RxQueue, &code, 0);
				}
			}
		}
		usleep(1000);
	}
}

static void *_IrTxDMAHandler(void *arg)
{
	ITHIrPort port = (ITHIrPort)arg;
	IR_OBJ *ir_obj = &IrObj[IR_JUDGE_PORT(port)];
	uint16_t code;

	while (1)
	{
		if (xQueueReceive(ir_obj->TxQueue, &code, 0))
		{
#ifdef DEBUG_MSG
			ithPrintf("got irTX code=%x, size: %d\n", code, sizeof(code));
#endif
			//1.parse event to get IR TX code(code table)

			ithIrRxCtrlEnable(ir_obj->port, ITH_IR_EN);

			memcpy(ir_obj->TxBuffer, &code, sizeof(code));

			ithDmaSetSrcAddr(ir_obj->TxChannel, (uint32_t)ir_obj->TxBuffer);
			ithDmaSetDstAddr(ir_obj->TxChannel, ir_obj->port + ITH_IR_TX_DATA_REG);

			ithDmaSetRequest(ir_obj->TxChannel, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM, ITH_DMA_HW_HANDSHAKE_MODE, ir_obj->TxChannelReq);
			ithDmaSetSrcParams(ir_obj->TxChannel, ITH_DMA_WIDTH_16, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
			ithDmaSetDstParams(ir_obj->TxChannel, ITH_DMA_WIDTH_16, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);

			ithDmaSetTxSize(ir_obj->TxChannel, sizeof(code));
			ithDmaSetBurst(ir_obj->TxChannel, ITH_DMA_BURST_1);

			ithDmaStart(ir_obj->TxChannel);

			while (ithDmaIsBusy(ir_obj->TxChannel) /*&& --timeout_ms*/)
			{
				DummySleep();
			}
		}
		usleep(1000);
	}

}
#endif

static void *_IrRxDefaultHandler(void* arg)
{
	ITHIrPort port = (ITHIrPort)arg;
	IR_OBJ *ir_obj = &IrObj[IR_JUDGE_PORT(port)];
	int code, signal;

	while (1)
	{
		signal = ithIrProbe(ir_obj->port);
		code = _IrProbe(ir_obj->port, signal);
		if (code != -1)
		{
			if (((code & 0xFFFF) == ir_obj->irVendorCode) && (((code >> 16) & 0xFF) == (~((code >> 24) & 0xFF) & 0xFF)))
				code = (code >> 16) & 0xFF;

			ithPrintf("IR code: 0x%X\n", code);

			//xQueueSendFromISR(irQueue, &code, &xHigherPriorityTaskWoken);
			xQueueSend(ir_obj->RxQueue, &code, 0);
		}
		usleep(2000);
	}
}

static void *_IrTxDefaultHandler(void* arg)
{
	ITHIrPort port = (ITHIrPort)arg;
	int code = 0;
	IR_OBJ *ir_obj = &IrObj[IR_JUDGE_PORT(port)];

	while (1)
	{
		code = 0;
		if (xQueueReceive(ir_obj->TxQueue, &code, 0))
		{
#ifdef DEBUG_MSG
			ithPrintf("got irTX code=%x\n", code);
#endif
			//1.parse event to get IR TX code(code table)
			_IrTxSend(port, code);
		}
		usleep(1000);
	}
}

#ifdef CHECK_VALID_TX_CODE
static int _IrCheckCodeMapping(int code)
{
	int i;
	for (i = 0; i<MAX_CODE_INDEX; i++)
	{
		if (code == irValidTxCodeTable[i])   return 1;
	}
	return 0;
}
#endif

static void _IrCalcThresholds(int port_num)
{
	int i, threshold, fifteenPercent;

	// set the min & max for each threshold
	for (i = 0; i < THRESHOLD_NUM; i++)
	{
		threshold = irProtocol[port_num][i] / SAMP_RATE;
		// 0.85 & 1.15 is experienc value, duration range N is between 0.85*N and 1.15*N.
		fifteenPercent = threshold * 30 / 100;

		irThresholds[port_num][i][0] = threshold - fifteenPercent;
		irThresholds[port_num][i][1] = threshold + fifteenPercent;
#ifdef DEBUG_MSG
		ithPrintf("irThresholds[port_num][%d][0] = %d\n",i, irThresholds[port_num][i][0]);
		ithPrintf("irThresholds[port_num][%d][1] = %d\n",i, irThresholds[port_num][i][1]);
#endif
	}
}

//=============================================================================
//                              Public Function Definition
//=============================================================================
int iteIrGetFreqFast(ITHIrPort port)
{
	int cycle = ithGetFreqFast(port);
	if (cycle)
		return PRECISION / cycle * SAMP_RATE;
	else
		return 0;
}

int iteIrGetFreqSlow(ITHIrPort port)
{
	int cycle = ithGetFreqSlow(port);
	if (cycle)
		return PRECISION / cycle * SAMP_RATE;
	else
		return 0;
}

int iteIrGetFreqAvg(ITHIrPort port)
{
	int cycle = ithGetFreqAvg(port);
	if (cycle)
		return PRECISION / cycle * SAMP_RATE;
	else
		return 0;
}

int iteIrGetFreqNew(ITHIrPort port)
{
	int cycle = ithGetFreqNew(port);
	if (cycle)
		return PRECISION / cycle * SAMP_RATE;
	else
		return 0;
}

int iteIrGetHighDCFast(ITHIrPort port)
{
	return ithGetHighDCFast(port);
}

int iteIrGetLowDCFast(ITHIrPort port)
{
	return ithGetLowDCFast(port);
}

int iteIrGetHighDCSlow(ITHIrPort port)
{
	return ithGetHighDCSlow(port);
}

int iteIrGetLowDCSlow(ITHIrPort port)
{
	return ithGetLowDCSlow(port);
}

int iteIrGetHighDCAvg(ITHIrPort port)
{
	return ithGetHighDCAvg(port);
}

int iteIrGetLowDCAvg(ITHIrPort port)
{
	return ithGetLowDCAvg(port);
}

int iteIrGetHighDCNew(ITHIrPort port)
{
	return ithGetHighDCNew(port);
}

int iteIrGetLowDCNew(ITHIrPort port)
{
	return ithGetLowDCNew(port);
}

void iteIrClearClkSample(ITHIrPort port)
{
	ithSetRegBitA(port + ITH_IR_RX_MOD_FILTER_REG, ITH_IR_RX_MOD_FILTER_RST_BIT);
	ithClearRegBitA(port + ITH_IR_RX_MOD_FILTER_REG, ITH_IR_RX_MOD_FILTER_RST_BIT);
}

int iteIrInit(ITHIrPort port)
{
	IR_OBJ *ir_obj = &IrObj[IR_JUDGE_PORT(port)];

//add for hitachi project mutiple bytes cmd
	static pthread_t irTask;
	tmp_obj=ir_obj;
	pthread_create(&irTask, NULL, task_timeoutcheck, NULL);
	ir_obj->RxQueue = xQueueCreate(QUEUE_LEN, IR_CODE_ARRAY_MAX_SIZE);

	
	//IR-RX init
	ithIrRxInit(ir_obj->port, ir_obj->RxGpio, 0, SAMP_RATE, PRECISION);
	//IR-TX init
	ithIrTxInit(ir_obj->port, ir_obj->TxGpio, 0, SAMP_RATE, PRECISION);

	_IrCalcThresholds(ir_obj->port_num);

	//Init IR_OBJ structure
	ir_obj->irCurrState = WAIT_RISING;
	ir_obj->irRecvCode = 0;
	ir_obj->irRecvBitCount = 0;
	ir_obj->irRepeatKeyCnt = 0;
	ir_obj->irRepeatKeyFast = 0;
	ir_obj->irRepeatKeyHold = 0;
	ir_obj->irRepeatKeyPress = 0;

	ir_obj->index_irCodeArray=1;

	
	ir_obj->irWidth = (ithReadRegA(ir_obj->port + ITH_IR_HWCFG_REG) & ITH_IR_WIDTH_MASK) >> ITH_IR_WIDTH_BIT;
	ir_obj->irLastEvent.code = -1;

//	ir_obj->RxQueue = xQueueCreate(QUEUE_LEN, (unsigned portBASE_TYPE) sizeof(int));
	ir_obj->TxQueue = xQueueCreate(QUEUE_LEN, (unsigned portBASE_TYPE) sizeof(int));

#ifdef ENABLE_IR_INTR
	if (ir_obj->mode == ITH_INTR_MODE)
	{
		ithEnterCritical();
		{
			// Init IR-RX(remote control) interrupt
			ithIntrDisableIrq(ir_obj->RxIntr);
			ithIntrClearIrq(ir_obj->RxIntr);
			ithIntrRegisterHandlerIrq(ir_obj->RxIntr, _IrRxIntrHandler, (void *)ir_obj->port);
			ithIntrEnableIrq(ir_obj->RxIntr);

			ithIrRxCtrlEnable(ir_obj->port, ITH_IR_INT);
			ithIrRxIntrCtrlEnable(ir_obj->port, ITH_IR_DATA);
		}
#if 0
		{
			// Init IR-TX(remote control) interrupt
			ithIntrDisableIrq(ir_obj->TxIntr);
			ithIntrClearIrq(ir_obj->TxIntr);
			ithIntrRegisterHandlerIrq(ir_obj->TxIntr, _IrTxIntrHandler, (void *)ir_obj->port);
			ithIntrEnableIrq(ir_obj->TxIntr);

			ithIrTxIntrCtrlEnable(ir_obj->port, ITH_IR_EMPTY);
		}
#else
		{
			int res;
			pthread_t task;
			pthread_attr_t attr;

			printf("Create IR-TX pthread~~\n");

			pthread_attr_init(&attr);
			res = pthread_create(&task, &attr, _IrTxDefaultHandler, (void *)ir_obj->port);

			if (res)
			{
				printf("[IR]%s() L#%ld: ERROR, create _IrTxDefaultHandler() thread fail! res=%ld\n", res);
				return -1;
			}
		}
#endif
		ithExitCritical();
	}
#endif
#ifdef ENABLE_IR_DMA
	if (ir_obj->mode == ITH_DMA_MODE)
	{
		ithPrintf("IR DMA Support\n");
		{
			LLP_CONTEXT *llpaddr = NULL;

			ir_obj->RxReadIdx = 0;
			ir_obj->RxWriteIdx = 0;

			ir_obj->RxChannel = ithDmaRequestCh(ir_obj->RxChName, ITH_DMA_CH_PRIO_HIGH_3, NULL, NULL);
			ithDmaReset(ir_obj->RxChannel);
			ir_obj->TxChannel = ithDmaRequestCh(ir_obj->TxChName, ITH_DMA_CH_PRIO_HIGHEST, NULL, NULL);
			ithDmaReset(ir_obj->TxChannel);

			ir_obj->RxBuffer = (uint16_t *)itpVmemAlloc(IR_DMA_BUFFER_SIZE);
			ir_obj->TxBuffer = (uint16_t *)itpVmemAlloc(IR_DMA_BUFFER_SIZE);

			if (ir_obj->RxBuffer == NULL || ir_obj->TxBuffer == NULL) {
				ithPrintf("Alloc IR DMA buffer fail\n");
			}
			else
			{
				ir_obj->gLLPCtxt = (LLP_CONTEXT *)itpVmemAlloc(sizeof(LLP_CONTEXT)+32);
#ifdef DEBUG_MSG
				ithPrintf("--- gLLPCtxt addr = 0x%x , sizeof(LLP_CONTEXT) = 0x%x---\n", ir_obj->gLLPCtxt, sizeof(LLP_CONTEXT));
#endif

				llpaddr = (LLP_CONTEXT *)(((uint32_t)ir_obj->gLLPCtxt + 0x1F) & ~(0x1F));
#ifdef DEBUG_MSG
				ithPrintf("--- new llpaddr addr = 0x%x\n", llpaddr);
#endif
				llpaddr->SrcAddr = le32_to_cpu(port + ITH_IR_RX_DATA_REG);
				llpaddr->DstAddr = le32_to_cpu(ir_obj->RxBuffer);
				llpaddr->LLP = le32_to_cpu(llpaddr);
				llpaddr->TotalSize = le32_to_cpu(IR_DMA_TRANSFER_SIZE);
				llpaddr->Control = le32_to_cpu(0x02610000);  //16bit

				ithDmaSetSrcAddr(ir_obj->RxChannel, port + ITH_IR_RX_DATA_REG);
				ithDmaSetDstAddr(ir_obj->RxChannel, (uint32_t)ir_obj->RxBuffer);
				ithDmaSetRequest(ir_obj->RxChannel, ITH_DMA_HW_HANDSHAKE_MODE, ir_obj->RxChannelReq, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM);

				ithDmaSetSrcParams(ir_obj->RxChannel, ITH_DMA_WIDTH_16, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
				ithDmaSetDstParams(ir_obj->RxChannel, ITH_DMA_WIDTH_16, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
				ithDmaSetTxSize(ir_obj->RxChannel, IR_DMA_BUFFER_SIZE);
				ithDmaSetBurst(ir_obj->RxChannel, ITH_DMA_BURST_1);

				ithDmaSetLLPAddr(ir_obj->RxChannel, (uint32_t)llpaddr);
#ifdef DEBUG_MSG
				ithPrintf("--- llpaddr:0x%x\n", (uint32_t)llpaddr);
#endif
				ithDmaStart(ir_obj->RxChannel);
			}
		}

		{
			int res;
			pthread_t task;
			pthread_attr_t attr;

			ithPrintf("Create DMA IR-RX pthread~~\n");

			pthread_attr_init(&attr);
			res = pthread_create(&task, &attr, _IrRxDMAHandler, (void *)port);

			if (res)
			{
				ithPrintf("[IR]%s() L#%ld: ERROR, create _IrRxDMAHandler thread fail! res=%ld\n", __FUNCTION__, __LINE__, res);
				return;
			}
		}

		{
			int res;
			pthread_t task;
			pthread_attr_t attr;

			ithPrintf("Create DMA IR-TX pthread~~\n");

			pthread_attr_init(&attr);
			res = pthread_create(&task, &attr, _IrTxDMAHandler, (void *)port);

			if (res)
			{
				ithPrintf("[IR]%s() L#%ld: ERROR, create _IrTxDMAHandler() thread fail! res=%ld\n", __FUNCTION__, __LINE__, res);
				return;
			}
		}
	}
#endif
#ifdef ENABLE_IR_FIFO
	if (ir_obj->mode == ITH_FIFO_MODE)
	{
		{
			int res;
			pthread_t task;
			pthread_attr_t attr;

			ithPrintf("Create IR-RX pthread in FIFO mode~~\n");

			pthread_attr_init(&attr);
			res = pthread_create(&task, &attr, _IrRxDefaultHandler, (void *)port);

			if (res)
			{
				ithPrintf("[IR]%s() L#%ld: ERROR, create _IrRxDefaultHandler() thread fail! res=%ld\n", res);
				return;
			}
		}

		{
		int res;
		pthread_t task;
		pthread_attr_t attr;

		ithPrintf("Create IR-TX pthread in FIFO mode~~\n");

		pthread_attr_init(&attr);
		res = pthread_create(&task, &attr, _IrTxDefaultHandler, (void *)port);

		if (res)
		{
			ithPrintf("[IR]%s() L#%ld: ERROR, create _IrTxDefaultHandler() thread fail! res=%ld\n", res);
			return;
		}
	}
	}
#endif
#ifdef ENABLE_IR_RX_MOD
	// Set Rx Modulation Filter
	if (ir_obj->irRxMod)
	{
		int WCLK = ithGetBusClock();
		ithPrintf("Modulation initial\n");
		//freq:20~60k.(80M/PreScale/freq.) for it9860, PreScale=798(80M)
		ithIrRxSetModFilter(ir_obj->port,
			WCLK / 798 / 60000,
			WCLK / 798 / 20000);
		ithIrRxMode(ir_obj->port, ITH_IR_RX_MODFILTER); //Enable modulation filter
	}
#endif
	ithIrRxCtrlEnable(ir_obj->port, ITH_IR_EN);

#ifdef ENABLE_IR_TX_MOD
	//Set Tx Modulation Freq.
	if (ir_obj->irTxMod)
	{
		int WCLK = ithGetBusClock();
		//(WCLK/SampleRate), SampleRate=39KHz
		ithIrTxSetModFreq(ir_obj->port, WCLK / 39000);
	}
#endif
	ithIrTxCtrlEnable(ir_obj->port, ITH_IR_EN);

#ifdef ENABLE_IR_RX_SAMPLE
	// Init Clock Sample function
	ithClkSampleInit(ir_obj->port, ir_obj->RxGpio, 0, SAMP_RATE, PRECISION);
	// Set Min & Max Filter
	//ithIrRxSetModFilter(ir_obj->port, 0, 10000);//0x6800, 0x6A00); //ex: 27000Hz=0x6978
	// Enable Clock Sample function
	ithIrRxMode(ir_obj->port, ITH_SAMPLE_CLK);
	ithPrintf("####ir rx sample enable\n");
#endif
}

int iteIrRead(ITHIrPort port, char *ptr)
{
	IR_OBJ *ir_obj = &IrObj[IR_JUDGE_PORT(port)];

	unsigned char irCodeArray[64];
int i;
	if (xQueueReceive(ir_obj->RxQueue,irCodeArray, 0))
	{
	
		ithPrintf("(IR) iteIrRead %d\n",irCodeArray[0]);
	  	for(i=1;i<=irCodeArray[0];i++)
			  ithPrintf("0x%x ",tmp_obj->irCodeArray[i]);
	  	ithPrintf("\n");
			
		memcpy((unsigned char*)ptr,&(tmp_obj->irCodeArray[1]),irCodeArray[0]);
		return irCodeArray[0];
	}
	return 0;

	
	ITPKeypadEvent* ev = (ITPKeypadEvent*)ptr;
	ITPKeypadEvent *LastEvent = NULL;

	LastEvent = &ir_obj->irLastEvent;

	if (xQueueReceive(ir_obj->RxQueue, &ev->code, 0))
	{
		gettimeofday(&ev->time, NULL);
		ev->flags = ITP_KEYPAD_DOWN;

		if (LastEvent->code == ev->code)
			ev->flags |= ITP_KEYPAD_REPEAT;

		LastEvent->code = ev->code;
		LastEvent->time.tv_sec = ev->time.tv_sec;
		LastEvent->time.tv_usec = ev->time.tv_usec;

		return sizeof (ITPKeypadEvent);
	}
	else if (LastEvent->code != -1)
	{
		struct timeval now;

		gettimeofday(&now, NULL);
		if (itpTimevalDiff(&LastEvent->time, &now) >= ir_obj->irPressInterval)
		{
			ev->code = LastEvent->code;
			ev->time.tv_sec = now.tv_sec;
			ev->time.tv_usec = now.tv_usec;
			ev->flags = ITP_KEYPAD_UP;
			LastEvent->code = -1;
			return sizeof (ITPKeypadEvent);
		}
	}

	return 0;
}

int iteIrWrite(ITHIrPort port, char *ptr)
{
	ITPKeypadEvent* ev = (ITPKeypadEvent*)ptr;
	IR_OBJ *ir_obj = &IrObj[IR_JUDGE_PORT(port)];

#ifdef CHECK_VALID_TX_CODE
	if (_IrCheckCodeMapping(ev->code))
	{
		xQueueSend(ir_obj->TxQueue, &ev->code, 0);
		if (ir_obj->mode == ITH_INTR_MODE)
			ithIrTxCtrlEnable(port, ITH_IR_INT);
	}
	else
		return 0;//TODO: maybe return non "0" value??
#else
	xQueueSend(ir_obj->TxQueue, &ev->code, 0);

#if 0
	if (ir_obj->mode == ITH_INTR_MODE) {
		//ithPrintf("ithReadRegA(port + ITH_IR_TX_CTRL_REG) & 0x4) >> 2 is %d\n", (ithReadRegA(port + ITH_IR_TX_CTRL_REG) & 0x4) >> 2);

		if (((ithReadRegA(port + ITH_IR_TX_CTRL_REG) & 0x4) >> 2) == 0)
		{
			ithPrintf("trun on interrupt\n");
			ithIrTxCtrlEnable(port, ITH_IR_INT);// Enable IR-TX Interrupt
		}
	}
#endif
#endif
	return 0;
}
