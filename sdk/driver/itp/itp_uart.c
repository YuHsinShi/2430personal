/** @file
 * PAL UART functions.
 *
 * @author Jim Tan
 * @version 1.0
 * @date 2011-2012
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
#include <errno.h>
#include <sys/socket.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/queue.h"
#include "itp_cfg.h"
#include "uart/uart.h"

static int itpUartOpen(const char *name, int flags, int mode, void *info)
{
    return 0;
}

static int itpUartRead(int file, char *ptr, int len, void* info)
{
	len = iteUartRead((ITHUartPort)info, ptr, len);

	return len;
}

static int itpUartWrite(int file, char *ptr, int len, void* info)
{
	len = iteUartWrite((ITHUartPort)info, ptr, len);

	return len;
}

static int itpUartIoctl(int file, unsigned long request, void *ptr, void *info)
{
    ITHUartPort   port = (ITHUartPort) info;

    switch (request)
    {
    case ITP_IOCTL_ON: //set odd , even parity check.
		iteUartOpen(port, (ITHUartParity)ptr);
        break;
	case ITP_IOCTL_INIT:
		iteUartInit(port, (UART_OBJ *)ptr);
		break;
    case ITP_IOCTL_RESET:
		iteUartReset(port, (UART_OBJ *)ptr);
        break;
	case ITP_IOCTL_REG_UART_CB:
		iteUartRegisterCallBack(port, ptr);
		break;
	case ITP_IOCTL_REG_UART_DEFER_CB:
		iteUartRegisterDeferCallBack(port, ptr);
		break;
	case ITP_IOCTL_UART_TIMEOUT:
		iteUartSetTimeout(port, (uint32_t)ptr);
		break;
	case ITP_IOCTL_UART_SET_GPIO:
		iteUartSetGpio(port, (ITHUartConfig *)ptr);
		break;
#if defined(ENABLE_UART_DMA)
	case ITP_IOCTL_UART_STOP_DMA:
		iteUartStopDMA(port);
		break;
#endif		
    default:
        errno = -1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceUart0 =
{
    ":uart0",
    itpUartOpen,
    itpCloseDefault,
    itpUartRead,
    itpUartWrite,
    itpLseekDefault,
    itpUartIoctl,
    (void *)ITH_UART0
};

const ITPDevice itpDeviceUart1 =
{
    ":uart1",
    itpUartOpen,
    itpCloseDefault,
    itpUartRead,
    itpUartWrite,
    itpLseekDefault,
    itpUartIoctl,
    (void *)ITH_UART1
};

const ITPDevice itpDeviceUart2 =
{
    ":uart2",
    itpUartOpen,
    itpCloseDefault,
    itpUartRead,
    itpUartWrite,
    itpLseekDefault,
    itpUartIoctl,
    (void *)ITH_UART2
};

const ITPDevice itpDeviceUart3 =
{
    ":uart3",
    itpUartOpen,
    itpCloseDefault,
    itpUartRead,
    itpUartWrite,
    itpLseekDefault,
    itpUartIoctl,
    (void *)ITH_UART3
};

const ITPDevice itpDeviceUart4 =
{
    ":uart4",
    itpUartOpen,
    itpCloseDefault,
    itpUartRead,
    itpUartWrite,
    itpLseekDefault,
    itpUartIoctl,
    (void *)ITH_UART4
};

const ITPDevice itpDeviceUart5 =
{
    ":uart5",
    itpUartOpen,
    itpCloseDefault,
    itpUartRead,
    itpUartWrite,
    itpLseekDefault,
    itpUartIoctl,
    (void *)ITH_UART5
};
