#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "uart_dma.h"

/*Note: There is a new DMA Implementation process. This file will be modified someday.*/

static UART_DMA_BASE UartDmaBase[6] =
{
	UART_DMA_BASE(0),
	UART_DMA_BASE(1),
	UART_DMA_BASE(2),
	UART_DMA_BASE(3),
	UART_DMA_BASE(4),
	UART_DMA_BASE(5),
};

static UART_DMA_OBJ UART_STATIC_DMA_OBJ[6] = {};

static int UartDmaObjInit(UART_OBJ *pUartObj);
static int UartDmaObjSend(UART_OBJ *pUartObj, char *ptr, int len);
static int UartDmaObjRead(UART_OBJ *pUartObj, char *ptr, int len);
static void UartDmaObjTerminate(UART_OBJ *pUartObj);
static void DummySleep(void);

UART_OBJ* iteNewUartDmaObj(ITHUartPort port, UART_OBJ *pUartObj)
{
	UART_OBJ* pObj = iteNewUartObj(port, pUartObj);
	if (pObj == NULL)
		return NULL;

	UART_DMA_OBJ* pUartDmaObj = &UART_STATIC_DMA_OBJ[UART_JUDGE_PORT(port)];

	pObj->pMode = pUartDmaObj;

	//initialising pMode class members
	int prot_num = UART_JUDGE_PORT(pObj->port);
	pUartDmaObj->pRxChName = UartDmaBase[prot_num].pRxChName;
	pUartDmaObj->pTxChName = UartDmaBase[prot_num].pTxChName;
	pUartDmaObj->txReqNum = UartDmaBase[prot_num].txReqNum;
	pUartDmaObj->rxReqNum = UartDmaBase[prot_num].rxReqNum;
	pUartDmaObj->rxChNum = 0;
	pUartDmaObj->txChNum = 0;
	pUartDmaObj->pDmaSrc = NULL;
	pUartDmaObj->pDmaBuf = NULL;
	pUartDmaObj->wtIndex = 0;
	pUartDmaObj->rdIndex = 0;
	pUartDmaObj->LLPCtxt = NULL;

	//Changing base class interface to access pMode class functions
	pObj->init = UartDmaObjInit;
	pObj->send = UartDmaObjSend;
	pObj->read = UartDmaObjRead;
	pObj->dele = UartDmaObjTerminate;

	return pObj;
}

static int UartDmaObjInit(UART_OBJ *pUartObj)
{
	UART_DMA_OBJ* pUartDmaObj = pUartObj->pMode;

	/* Set the required protocol. */
	ithUartReset(pUartObj->port, pUartObj->baud, pUartObj->parity, 1, 8);

	ithUartSetMode(pUartObj->port, ITH_UART_DEFAULT, pUartObj->txPin, pUartObj->rxPin);

	pUartDmaObj->rdIndex = pUartDmaObj->wtIndex = 0;

	pUartDmaObj->rxChNum = ithDmaRequestCh(pUartDmaObj->pRxChName, ITH_DMA_CH_PRIO_LOW, NULL, NULL);
	ithDmaReset(pUartDmaObj->rxChNum);
	pUartDmaObj->txChNum = ithDmaRequestCh(pUartDmaObj->pTxChName, ITH_DMA_CH_PRIO_HIGH, NULL, NULL);
	ithDmaReset(pUartDmaObj->txChNum);

	pUartDmaObj->pDmaSrc = (uint8_t *)itpVmemAlloc(UART_DMA_BUF_SIZE);
	pUartDmaObj->pDmaBuf = (uint8_t *)itpVmemAlloc(UART_DMA_BUF_SIZE);

	if (pUartDmaObj->pDmaBuf == NULL || pUartDmaObj->pDmaSrc == NULL)
	{
		ithPrintf("Alloc DMA buffer fail\n");
	}
	else
	{
		LLP *llpaddr = NULL;
		pUartDmaObj->LLPCtxt = (LLP *)itpVmemAlloc(sizeof(LLP)+32);
		llpaddr = (LLP *)(((uint32_t)pUartDmaObj->LLPCtxt + 0x1F) & ~(0x1F));
		llpaddr->SrcAddr = pUartObj->port;
		llpaddr->DstAddr = (uint32_t)pUartDmaObj->pDmaBuf;
		llpaddr->LLP = (uint32_t)llpaddr;
		llpaddr->TotalSize = UART_DMA_BUF_SIZE;
		llpaddr->Control = 0x00210000;

		ithDmaSetSrcAddr(pUartDmaObj->rxChNum, pUartObj->port);
		ithDmaSetDstAddr(pUartDmaObj->rxChNum, (uint32_t)pUartDmaObj->pDmaBuf);
		ithDmaSetRequest(pUartDmaObj->rxChNum, ITH_DMA_HW_HANDSHAKE_MODE, pUartDmaObj->rxReqNum, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM);

		ithDmaSetSrcParamsA(pUartDmaObj->rxChNum, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_FIX);
		ithDmaSetDstParamsA(pUartDmaObj->rxChNum, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_INC);
		ithDmaSetTxSize(pUartDmaObj->rxChNum, UART_DMA_BUF_SIZE);
		ithDmaSetBurst(pUartDmaObj->rxChNum, ITH_DMA_BURST_1);
		ithDmaSetLLPAddr(pUartDmaObj->rxChNum, (uint32_t)llpaddr);
		ithDmaStart(pUartDmaObj->rxChNum);
	}

	return 0;
}

static int UartDmaObjSend(UART_OBJ *pUartObj, char *ptr, int len)
{
	UART_DMA_OBJ* pUartDmaObj = pUartObj->pMode;
	if (pUartDmaObj->pDmaSrc == NULL)
	{
		return 0;
	}
	
	memcpy(pUartDmaObj->pDmaSrc, ptr, len);
	ithDmaSetSrcAddr(pUartDmaObj->txChNum, (uint32_t)pUartDmaObj->pDmaSrc);
	ithDmaSetDstAddr(pUartDmaObj->txChNum, pUartObj->port);
	ithDmaSetRequest(pUartDmaObj->txChNum, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM, ITH_DMA_HW_HANDSHAKE_MODE, pUartDmaObj->txReqNum);

	ithDmaSetSrcParamsA(pUartDmaObj->txChNum, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_INC);
	ithDmaSetDstParamsA(pUartDmaObj->txChNum, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_FIX);
	ithDmaSetTxSize(pUartDmaObj->txChNum, len);
	ithDmaSetBurst(pUartDmaObj->txChNum, ITH_DMA_BURST_1);

	ithDmaStart(pUartDmaObj->txChNum);

	while (ithDmaIsBusy(pUartDmaObj->txChNum) /*&& --timeout_ms*/)
	{
		DummySleep();
	}

	return len;
}

static int UartDmaObjRead(UART_OBJ *pUartObj, char *ptr, int len)
{
	UART_DMA_OBJ* pUartDmaObj = pUartObj->pMode;
	uint32_t transferSize = 0, dataSize = 0;

	transferSize = ithReadRegA(ITH_DMA_BASE + ITH_DMA_SIZE_CH(pUartDmaObj->rxChNum));
	pUartDmaObj->wtIndex = (UART_DMA_BUF_SIZE - transferSize);
	if (pUartDmaObj->wtIndex > UART_DMA_BUF_SIZE)
	{
		return dataSize;
	}
	if (pUartDmaObj->wtIndex != pUartDmaObj->rdIndex)
	{
		if (pUartDmaObj->wtIndex < pUartDmaObj->rdIndex)
		{
			dataSize = (UART_DMA_BUF_SIZE - pUartDmaObj->rdIndex) + pUartDmaObj->wtIndex;
			ithInvalidateDCacheRange(pUartDmaObj->pDmaBuf, UART_DMA_BUF_SIZE);
			memcpy(ptr, pUartDmaObj->pDmaBuf + pUartDmaObj->rdIndex, UART_DMA_BUF_SIZE - pUartDmaObj->rdIndex);
			memcpy(ptr + (UART_DMA_BUF_SIZE - pUartDmaObj->rdIndex), pUartDmaObj->pDmaBuf, pUartDmaObj->wtIndex);
		}
		else
		{
			dataSize = pUartDmaObj->wtIndex - pUartDmaObj->rdIndex;
			ithInvalidateDCacheRange(pUartDmaObj->pDmaBuf, UART_DMA_BUF_SIZE);
			memcpy(ptr, pUartDmaObj->pDmaBuf + pUartDmaObj->rdIndex, dataSize);
		}
		pUartDmaObj->rdIndex = pUartDmaObj->wtIndex;
	}

	return dataSize;
}

static void UartDmaObjTerminate(UART_OBJ *pUartObj)
{
	UART_DMA_OBJ* pUartDmaObj = pUartObj->pMode;

	ithUartDisableDmaMode2(pUartObj->port);

	ithDmaAbort(pUartDmaObj->rxChNum);
	while (ithDmaIsBusy(pUartDmaObj->rxChNum))
		ithPrintf("DMA read channel busy\n");
	ithDmaFreeCh(pUartDmaObj->rxChNum);

	ithDmaAbort(pUartDmaObj->txChNum);
	while (ithDmaIsBusy(pUartDmaObj->txChNum))
		ithPrintf("DMA write channel busy\n");
	ithDmaFreeCh(pUartDmaObj->txChNum);

	itpVmemFree((uint32_t)pUartDmaObj->LLPCtxt);
	itpVmemFree((uint32_t)pUartDmaObj->pDmaSrc);
	itpVmemFree((uint32_t)pUartDmaObj->pDmaBuf);

	pUartDmaObj->pRxChName = "";
	pUartDmaObj->pTxChName = "";
	pUartDmaObj->txReqNum = -1;
	pUartDmaObj->rxReqNum = -1;
	pUartDmaObj->rxChNum = 0;
	pUartDmaObj->txChNum = 0;
	pUartDmaObj->pDmaSrc = NULL;
	pUartDmaObj->pDmaBuf = NULL;
	pUartDmaObj->wtIndex = 0;
	pUartDmaObj->rdIndex = 0;
	pUartDmaObj->LLPCtxt = NULL;

	pUartObj->init = NULL;
	pUartObj->dele = NULL;
	pUartObj->send = NULL;
	pUartObj->read = NULL;

	pUartDmaObj = NULL;

	pUartObj = NULL;
}

static void DummySleep(void)
{
	unsigned int idle = 100, i = 0, nothing = 0;
	for (i = 0; i < idle; i++)
		nothing++;
}
