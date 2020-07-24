/*
* Copyright (c) 2018 ITE Tech. Inc. All Rights Reserved.
*/
/** @file
* PAL Wiegand functions.
*
* @author YungHsiang Tu
* @version 0.1
*/

#include <errno.h>
#include "itp_cfg.h"
#include "wiegand/wiegandout.h"

typedef struct
{
	int index;
	int bit_count;
	WIEGAND_LOOPBACK_DIRECTION loopback_direction;
	WIEGAND_OUT_BIT_ORDER bit_order;
} ITPWiegandOut;

#if(!CFG_SW_WIEGAND_ENABLE)

static ITPWiegandOut itpWiegand0 = { 0, 34, BYPASS, MSB };
static ITPWiegandOut itpWiegand1 = { 1, 34, BYPASS, MSB };

static int WiegandoutIoctl(int file, unsigned long request, void* ptr, void* info)
{
	ITPWiegandOut* ctxt = (ITPWiegandOut*)info;

	switch (request)
	{
	case ITP_IOCTL_INIT:
		init_wiegandout_controller(ctxt->index);
		break;

	case ITP_IOCTL_SET_BIT_COUNT:
		ctxt->bit_count = *(int*)ptr;
		break;

	case ITP_IOCTL_GET_BIT_COUNT:
		*(int*)ptr = ctxt->bit_count;
		break;

	case ITP_IOCTL_CUSTOM_CTL_ID0:
		ctxt->loopback_direction = (WIEGAND_LOOPBACK_DIRECTION)ptr;
		break;

	case ITP_IOCTL_ENABLE:
		wiegandout_controller_enable(ctxt->index, ctxt->bit_count, ctxt->loopback_direction, ctxt->bit_order);
		break;

	default:
		errno = (ctxt->index ? ITP_DEVICE_WIEGAND1 : ITP_DEVICE_WIEGAND0 << ITP_DEVICE_ERRNO_BIT) | 1;
		return -1;
	}
	return 0;
}

const ITPDevice itpDeviceWiegandout0 =
{
	":wiegand0",
	itpOpenDefault,
	itpCloseDefault,
	itpReadDefault,
	itpWriteDefault,
	itpLseekDefault,
	WiegandoutIoctl,
	(void*)&itpWiegand0
};

const ITPDevice itpDeviceWiegandout1 =
{
	":wiegand1",
	itpOpenDefault,
	itpCloseDefault,
	itpReadDefault,
	itpWriteDefault,
	itpLseekDefault,
	WiegandoutIoctl,
	(void*)&itpWiegand1
};
#endif