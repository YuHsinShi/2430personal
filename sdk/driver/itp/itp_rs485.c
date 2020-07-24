/** @file
 * PAL RS485 functions.
 *
 * @author
 * @version 1.0
 * @date 20141014
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
#include <errno.h>
#include <sys/socket.h>
#include <pthread.h>
#include "itp_cfg.h"
#include "uart/uart.h"
#include "uart/interface/rs485.h"

static int itpRs485Open(const char *name, int flags, int mode, void *info)
{
	return 0;
}

static int itpRs485Read(int file, char *ptr, int len, void* info)
{
	len = iteRs485Read((ITHUartPort)info, ptr, len);

	return len;
}

static int itpRs485Write(int file, char *ptr, int len, void* info)
{
	len = iteRs485Write((ITHUartPort)info, ptr, len);

	return len;
}

static int itpRs485Ioctl(int file, unsigned long request, void *ptr, void *info)
{
	ITHUartPort   port = (ITHUartPort)info;

	switch (request)
	{
	case ITP_IOCTL_ON: //set odd , even parity check.
		iteUartOpen(port, (ITHUartParity)ptr);
		break;
	case ITP_IOCTL_INIT:
		iteRs485Init(port, (RS485_OBJ *)ptr);
		break;
	case ITP_IOCTL_RESET:
		iteRs485Reset(port, (RS485_OBJ *)ptr);
		break;
	case ITP_IOCTL_REG_RS485_CB:
		iteUartRegisterCallBack(port, ptr);
		break;
	case ITP_IOCTL_REG_RS485_DEFER_CB:
		iteUartRegisterDeferCallBack(port, ptr);
		break;
	default:
		errno = -1;
		return -1;
	}
	return 0;
}

const ITPDevice itpDeviceRS485_0 =
{
    ":rs485_0",
    itpOpenDefault,
    itpCloseDefault,
    itpRs485Read,
    itpRs485Write,
    itpLseekDefault,
    itpRs485Ioctl,
    (void*)ITH_UART0
};

const ITPDevice itpDeviceRS485_1 =
{
    ":rs485_1",
    itpOpenDefault,
    itpCloseDefault,
    itpRs485Read,
    itpRs485Write,
    itpLseekDefault,
    itpRs485Ioctl,
    (void*)ITH_UART1
};

const ITPDevice itpDeviceRS485_2 =
{
    ":rs485_2",
    itpOpenDefault,
    itpCloseDefault,
    itpRs485Read,
    itpRs485Write,
    itpLseekDefault,
    itpRs485Ioctl,
    (void*)ITH_UART2
};

const ITPDevice itpDeviceRS485_3 =
{
    ":rs485_3",
    itpOpenDefault,
    itpCloseDefault,
    itpRs485Read,
    itpRs485Write,
    itpLseekDefault,
    itpRs485Ioctl,
    (void*)ITH_UART3
};

const ITPDevice itpDeviceRS485_4 =
{
    ":rs485_4",
    itpOpenDefault,
    itpCloseDefault,
    itpRs485Read,
    itpRs485Write,
    itpLseekDefault,
    itpRs485Ioctl,
    (void*)ITH_UART4
};

const ITPDevice itpDeviceRS485_5 =
{
    ":rs485_5",
    itpOpenDefault,
    itpCloseDefault,
    itpRs485Read,
    itpRs485Write,
    itpLseekDefault,
    itpRs485Ioctl,
    (void*)ITH_UART5
};
