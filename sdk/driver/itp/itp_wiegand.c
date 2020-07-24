/*
 * Copyright (c) 2014 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Wiegand functions.
 *
 * @author Vincent Lee
 * @version 1.0
 */

#include <errno.h>
#include <memory.h>
#include "itp_cfg.h"
#include "wiegand/wiegand.h"
#if defined(CFG_SW_WIEGAND_ENABLE)
#include "ite/audio.h"
#include "ite/ite_risc.h"
#endif

enum {
	CMD_WIEGAND_ENABLE = 11
};

typedef struct
{
	int index;
	int bit_count;
	int gpio_d0;
	int gpio_d1;
	char card_id[17];
} ITPWiegand;

#if defined(CFG_SW_WIEGAND_ENABLE)
typedef struct WIEGAND_CONTEXT
{
	uint32_t cmdBuffer;
	uint32_t cmdBufferSize;
	uint32_t wg0wiegandBuffer;
	uint32_t wg0wiegandBufferSize;
	uint32_t wg1wiegandBuffer;
	uint32_t wg1wiegandBufferSize;
}WIEGAND_CONTEXT;

WIEGAND_CONTEXT* pWiegandCtxt = NULL;

#define TOINT(n)   ((((n) >> 24) & 0xff) + (((n) >> 8) & 0xff00) + (((n) << 8) & 0xff0000) + (((n) << 24) & 0xff000000))
#define CODEC_BASE ((int)iteRiscGetTargetMemAddress(RISC1_IMAGE_MEM_TARGET))

static uint8_t gWiegandCodec[] =
{
#include "itewiegand.hex"
};

static uint32_t         Bootmode         = 1;

static _Risc1OpenEngine(void)
{
	uint8_t*  pBuffer = NULL;
	uint8_t*  pTest = NULL;

	iteRiscLoadData(RISC1_IMAGE_MEM_TARGET, gWiegandCodec, sizeof(gWiegandCodec));
	iteRiscResetCpu(RISC1_CPU);
	iteRiscFireCpu(RISC1_CPU);
	usleep(10* 1000);

	pWiegandCtxt = (WIEGAND_CONTEXT*)iteRiscGetTargetMemAddress(SHARE_MEM1_TARGET);
	pBuffer = (uint8_t*) pWiegandCtxt;
	ithInvalidateDCacheRange(pWiegandCtxt, sizeof(WIEGAND_CONTEXT));
	printf("pWiegandCtxt: 0x%X\n", pWiegandCtxt);
	printf("cmd Buffer: 0x%X\n", pWiegandCtxt->cmdBuffer);
	printf("cmd Buffer Size: %d\n", pWiegandCtxt->cmdBufferSize);
	printf("wiegand Buffer: 0x%X\n", pWiegandCtxt->wg1wiegandBuffer);
	printf("wiegand Buffer Size: %d\n", pWiegandCtxt->wg1wiegandBufferSize);
	printf("0x%X, 0x%X, 0x%X, 0x%X\n", pBuffer[0], pBuffer[1], pBuffer[2], pBuffer[3]);
}

__attribute__((used)) static bool
_ithCodecCommand(
int command,
int parameter0,
int parameter1,
int parameter2)
{
	int timeout             = 10;
	int commandBuffer       = pWiegandCtxt->cmdBuffer + CODEC_BASE;
	int *oriCommandBuffer   = (int *) commandBuffer;
	int commandBufferLength = pWiegandCtxt->cmdBufferSize;

	if (pWiegandCtxt->cmdBuffer == 0)
	{
		return false;
	}
	//printf("oriCommandBuffer = 0x%x\n", oriCommandBuffer);
	ithInvalidateDCacheRange((void*)oriCommandBuffer, 4);
	timeout = 1000;

	while (*oriCommandBuffer && timeout && ithReadRegH(0x16A4) != 0x0)
	{
		if (Bootmode)
		{
			int i;
			for (i = 0; i < 10000; i++)
				asm ("");
		}
		else
			usleep(1000);
		timeout--;
		ithInvalidateDCacheRange((void*)oriCommandBuffer, 4);
	}

	if (timeout == 0)
	{
		return false;
	}
	/*RISC command should final adding , because RISC will get the command first and start it.*/
	oriCommandBuffer[1] = parameter0;
	oriCommandBuffer[2] = parameter1;
	oriCommandBuffer[3] = parameter2;
	oriCommandBuffer[0] = command;

	ithWriteRegH(0x16A4, 0x1111);
	ithFlushDCacheRange((void*)oriCommandBuffer, commandBufferLength);
	ithFlushMemBuffer();

	ithInvalidateDCacheRange((void*)oriCommandBuffer, 4);
	timeout = 1000;
	while (*oriCommandBuffer && timeout)
	{
		if (!Bootmode)
			usleep(1000);
		timeout--;
		ithInvalidateDCacheRange((void*)oriCommandBuffer, 4);
	}
	if (timeout == 0)
		return false;

	return true;
}

static int
_ithCodecWiegandReadCard0(
int index,
unsigned long long *card_id)
{
	unsigned long *value;
	int           bit_count            = 0;
	int           wiegandBuffer        = pWiegandCtxt->wg0wiegandBuffer + CODEC_BASE;
	int           wiegandBufferLength  = pWiegandCtxt->wg0wiegandBufferSize;

	if (pWiegandCtxt->wg0wiegandBuffer == 0)
		return;

	// get back card id from buffer of wiegand
	value     = (unsigned long *)wiegandBuffer;
	ithInvalidateDCacheRange((void*)wiegandBuffer, wiegandBufferLength);

	//get back bit count of wiegand
	bit_count = TOINT(value[0]);
	*card_id  = (((unsigned long long)TOINT(value[1]) << 32) | (unsigned long long)TOINT(value[2]));

	// reset buffer
	memset((void *)wiegandBuffer, 0, wiegandBufferLength);
	ithFlushDCacheRange((void *)wiegandBuffer, wiegandBufferLength);
	return bit_count;
}

static int
_ithCodecWiegandReadCard1(
int index,
unsigned long long *card_id)
{
	unsigned long *value;
	int           bit_count            = 0;
	int           wiegandBuffer        = pWiegandCtxt->wg1wiegandBuffer + CODEC_BASE;
	int           wiegandBufferLength  = pWiegandCtxt->wg1wiegandBufferSize;

	if (pWiegandCtxt->wg1wiegandBuffer == 0)
		return;

	// get back card id from buffer of wiegand
	value     = (unsigned long *)wiegandBuffer;
	ithInvalidateDCacheRange((void*)wiegandBuffer, wiegandBufferLength);

	// get back bit count of wiegand
	bit_count = TOINT(value[0]);
	*card_id  = (((unsigned long long)TOINT(value[1]) << 32) | (unsigned long long)TOINT(value[2]));

	// reset buffer
	memset((void *)wiegandBuffer, 0, wiegandBufferLength);
	ithFlushDCacheRange((void *)wiegandBuffer, wiegandBufferLength);
	return bit_count;
}

static int
_ithCodecWiegandReadCardTest(
int index,
unsigned long  *card_id)
{
	unsigned long *value;
	int           bit_count            = 0;
	int           wiegandBuffer        = pWiegandCtxt->wg1wiegandBuffer + CODEC_BASE;
	int           wiegandBufferLength  = pWiegandCtxt->wg1wiegandBufferSize;

	if (pWiegandCtxt->wg1wiegandBuffer == 0)
		return;

	// get back card id from buffer of wiegand
	value     = (unsigned long *)wiegandBuffer;
	ithInvalidateDCacheRange((void*)wiegandBuffer, wiegandBufferLength);

	bit_count = TOINT(value[0]);
	if(bit_count)
	{
		int index = bit_count >> 5;
		int i;
		for (i=0; i<=index; i++)
		{
			card_id[i] = TOINT(value[i+1]);
		}
	}
	// reset buffer
	memset((void *)wiegandBuffer, 0, wiegandBufferLength);
	ithFlushDCacheRange((void *)wiegandBuffer, wiegandBufferLength);
	return bit_count;
}

static ITPWiegand itpWiegand0 = { 0, 34, 25, 26 };
static ITPWiegand itpWiegand1 = { 1, 34, 23, 24 };

static int WiegandRead(int file, char *ptr, int len, void* info)
{
    ITPWiegand* ctxt = (ITPWiegand*)info;
    unsigned long long value = 0;
    int  bit_count = 0;

    *(int*)ptr = NULL;

    if (ctxt->index)
        bit_count = _ithCodecWiegandReadCard1(ctxt->index, &value);
    else
        bit_count = _ithCodecWiegandReadCard0(ctxt->index, &value);
#if 1
    if ((bit_count == ctxt->bit_count) && value)
    {
        switch (ctxt->bit_count)
        {
        case 26:
            value = (value & 0x1FFFFFE) >> 1;
            break;

        case 34:
            value = (value & 0x1FFFFFFFE) >> 1;
            break;

        case 37:
            value = (value & 0xFFFFFFFFE) >> 1;
            break;

        default:
            break;
        }

        sprintf(ctxt->card_id, "%08X%03d%05d", (unsigned long)(value >> 32), ((unsigned long)value & 0xFFFF0000) >> 16, (unsigned long)value & 0xFFFF);
        printf("sw card_id: %s, bit_count: %d\n", ctxt->card_id, bit_count);
        *(int*)ptr = (int)ctxt->card_id;
    }
#else
    if (bit_count)
    {
        printf("--bit count = %d --\n", bit_count);
        int index = bit_count >> 5;
        printf("index = %d\n", index);
        int i;
        for (i = 0; i <= index; i++)
            printf("i=%d value = [%lu]\n", i, pValue[i]);
    }
#endif
    return 0;
}

static int WiegandIoctl(int file, unsigned long request, void* ptr, void* info)
{
    ITPWiegand* ctxt = (ITPWiegand*)info;

    switch (request)
    {
    case ITP_IOCTL_INIT:
        _Risc1OpenEngine();
        break;

    case ITP_IOCTL_ENABLE:
        if (_ithCodecCommand(CMD_WIEGAND_ENABLE, ctxt->index, ctxt->gpio_d0, ctxt->gpio_d1) == false)
            printf("WiegandIoctl risc codec is not running");
        break;

    case ITP_IOCTL_SET_BIT_COUNT:
        ctxt->bit_count = *(int*)ptr;
        break;

    case ITP_IOCTL_GET_BIT_COUNT:
        *(int*)ptr = ctxt->bit_count;
        break;

    case ITP_IOCTL_SET_GPIO_PIN:
        ctxt->gpio_d0 = *(int*)ptr;
        ctxt->gpio_d1 = *((int*)(ptr)+1);
        break;
    default:
        errno = (ctxt->index ? ITP_DEVICE_WIEGAND1 : ITP_DEVICE_WIEGAND0 << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceWiegand0 =
{
    ":wiegand0",
    itpOpenDefault,
    itpCloseDefault,
    WiegandRead,
    itpWriteDefault,
    itpLseekDefault,
    WiegandIoctl,
    (void*)&itpWiegand0
};

const ITPDevice itpDeviceWiegand1 =
{
    ":wiegand1",
    itpOpenDefault,
    itpCloseDefault,
    WiegandRead,
    itpWriteDefault,
    itpLseekDefault,
    WiegandIoctl,
    (void*)&itpWiegand1
};
#else
static ITPWiegand itpWiegand0 = { 0, 34, 25, 26 };
static ITPWiegand itpWiegand1 = { 1, 34, 23, 24 };

static int WiegandRead(int file, char *ptr, int len, void* info)
{
	ITPWiegand* ctxt = (ITPWiegand*)info;
	ITHUartPort port = wiegand_get_uart_base(ctxt->index);

	unsigned long long value = 0;
	char cardid[17] = { 0 };
	char tmp[17] = { 0 };
	int  idlen = 0;
	int count = 0;
	int i = 0;
#define TIMEOUT 1000

	for (;;)
	{
		if (ithUartIsRxReady(port))
		{
			int timeout = TIMEOUT;
			cardid[count] = ithUartGetChar(port); // Read character from uart
			count++;

			while (1)
			{
				// Is a character waiting?
				if (ithUartIsRxReady(port))
				{
					cardid[count] = ithUartGetChar(port); // Read character from uart
					count++;
					timeout = TIMEOUT;
				}
				else if (timeout-- <= 0)
				{
					break;
					//return count;
				}
			}
		}
		else
		{
			break;
			//return count;
		}
	}
	idlen = cardid[1];
	if (idlen)
	{
		if (idlen > 17)
		{
			printf("invalid id len\n");
			return -1;
		}

		for (i = 0; i < idlen; i++)
			tmp[i] = cardid[1 + idlen - i];
		memcpy(&value, tmp, idlen);
		sprintf(ctxt->card_id, "%08X%03d%05d", (unsigned long)(value >> 32), ((unsigned long)value & 0xFFFF0000) >> 16, (unsigned long)value & 0xFFFF);
		printf("card_id: %s, idlen: %d\n", ctxt->card_id, idlen);
		*(int*)ptr = (int)ctxt->card_id;
	}
	return 0;
}

static int WiegandIoctl(int file, unsigned long request, void* ptr, void* info)
{
	ITPWiegand* ctxt = (ITPWiegand*)info;

	switch (request)
	{
	case ITP_IOCTL_INIT:
		init_wiegand_controller(ctxt->index);
		break;

	case ITP_IOCTL_ENABLE:
		wiegand_controller_enable(ctxt->index, ctxt->gpio_d0, ctxt->gpio_d1, ctxt->bit_count);
		break;

	case ITP_IOCTL_SET_BIT_COUNT:
		ctxt->bit_count = *(int*)ptr;
		break;

	case ITP_IOCTL_GET_BIT_COUNT:
		*(int*)ptr = ctxt->bit_count;
		break;

	case ITP_IOCTL_SET_GPIO_PIN:
		ctxt->gpio_d0 = *(int*)ptr;
		ctxt->gpio_d1 = *((int*)(ptr)+1);
		break;
	case ITP_IOCTL_SUSPEND:
		wiegand_suspend(ctxt->index);
		break;
	case ITP_IOCTL_RESUME:
		wiegand_resume(ctxt->index);
		break;
	case ITP_IOCTL_CUSTOM_CTL_ID0:
		wiegand_verify_enable(ctxt->index, 1);
		break;
	default:
		errno = (ctxt->index ? ITP_DEVICE_WIEGAND1 : ITP_DEVICE_WIEGAND0 << ITP_DEVICE_ERRNO_BIT) | 1;
		return -1;
	}
	return 0;
}

const ITPDevice itpDeviceWiegand0 =
{
	":wiegand0",
	itpOpenDefault,
	itpCloseDefault,
	WiegandRead,
	itpWriteDefault,
	itpLseekDefault,
	WiegandIoctl,
	(void*)&itpWiegand0
};

const ITPDevice itpDeviceWiegand1 =
{
	":wiegand1",
	itpOpenDefault,
	itpCloseDefault,
	WiegandRead,
	itpWriteDefault,
	itpLseekDefault,
	WiegandIoctl,
	(void*)&itpWiegand1
};
#endif