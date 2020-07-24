/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 *
 * @author Irene Lin
 * @version 1.0
 */
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include "ite/itp.h"
#include "ite/ite_acm.h"


static int AcmTtyRead(int file, char *ptr, int len, void *info)
{
    return iteAcmTtyRead(ptr, len);
}

static int AcmTtyWrite(int file, char *ptr, int len, void *info)
{
    int wlen;

    wlen = iteAcmTtyWrite(ptr, len);
	
	while (wlen < len) {
		usleep(1000);
		wlen += iteAcmTtyWrite(ptr + wlen, len - wlen);
	}
	
    return wlen;
}

static int UsbdAcmIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
		iteAcmRegister();
        break;

    case ITP_IOCTL_ENABLE:
        iteAcmTtyOpen();
        break;

    case ITP_IOCTL_DISABLE:
        iteAcmTtyClose();
        break;
		
	case ITP_IOCTL_IS_CONNECTED:
        return iteAcmTtyIsReady();

    default:
        errno = (ITP_DEVICE_USBDACM << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}


const ITPDevice itpDeviceUsbdAcm =
{
    ":usbd acm",
    itpOpenDefault,
    itpCloseDefault,
    AcmTtyRead,
    AcmTtyWrite,
    itpLseekDefault,
    UsbdAcmIoctl,
    NULL
};



#if defined(CFG_DBG_USB)

static int AcmConsolePutchar(int c)
{
    char cc = (char)c;
	
    iteAcmConsoleWrite((const char*)&cc, 1);
        
    return c;
}

static int AcmConsoleWrite(int file, char *ptr, int len, void *info)
{
    return iteAcmConsoleWrite(ptr, len);
}

static int UsbdConsoleIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
		ithPutcharFunc = AcmConsolePutchar;
        break;

    default:
        errno = (ITP_DEVICE_USBDCONSOLE << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}


const ITPDevice itpDeviceUsbdConsole =
{
    ":usbd console",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    AcmConsoleWrite,
    itpLseekDefault,
    UsbdConsoleIoctl,
    NULL
};

#endif

