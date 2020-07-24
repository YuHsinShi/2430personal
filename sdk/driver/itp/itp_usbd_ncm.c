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
#include "ite/ite_usbd_ncm.h"



static int UsbdNcmIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
        iteUsbdNcmRegister();
        break;

    case ITP_IOCTL_IS_CONNECTED:
        return iteUsbdNcmGetLink();
        break;

    default:
        errno = (ITP_DEVICE_USBDNCM << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}


const ITPDevice itpDeviceUsbdNcm =
{
    ":usbd ncm",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    itpWriteDefault,
    itpLseekDefault,
    UsbdNcmIoctl,
    NULL
};



