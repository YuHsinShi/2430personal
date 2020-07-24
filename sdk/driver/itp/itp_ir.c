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
#include "itp_cfg.h"
#include "irda/irda.h"

static int IrRead(int file, char *ptr, int len, void* info)
{
	ITHIrPort port = (ITHIrPort)info;

	len = iteIrRead(port, ptr);

	return len;
}

static int IrWrite(int file, char *ptr, int len, void* info)
{
	ITHIrPort port = (ITHIrPort)info;

	len = iteIrWrite(port, ptr);

	return len;
}

static int IrIoctl(int file, unsigned long request, void* ptr, void* info)
{
	ITHIrPort port = (ITHIrPort)info;

	switch (request)
	{
	case ITP_IOCTL_INIT:
		iteIrInit(port);
		break;

	default:
		errno = (ITP_DEVICE_IR0 << ITP_DEVICE_ERRNO_BIT) | __LINE__;
		return -1;
	}
	return 0;
}

const ITPDevice itpDeviceIr0 =
{
	":ir0",
	itpOpenDefault,
	itpCloseDefault,
	IrRead,
	IrWrite,
	itpLseekDefault,
	IrIoctl,
	(void *)ITH_IR0
};

const ITPDevice itpDeviceIr1 =
{
	":ir1",
	itpOpenDefault,
	itpCloseDefault,
	IrRead,
	IrWrite,
	itpLseekDefault,
	IrIoctl,
	(void *)ITH_IR1
};

const ITPDevice itpDeviceIr2 =
{
	":ir2",
	itpOpenDefault,
	itpCloseDefault,
	IrRead,
	IrWrite,
	itpLseekDefault,
	IrIoctl,
	(void *)ITH_IR2
};

const ITPDevice itpDeviceIr3 =
{
	":ir3",
	itpOpenDefault,
	itpCloseDefault,
	IrRead,
	IrWrite,
	itpLseekDefault,
	IrIoctl,
	(void *)ITH_IR3
};
