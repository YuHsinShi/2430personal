/**
 * @file itp_usbd_hid.c
 * @author I-Chun Lai (ichun.lai@ite.com.tw)
 * @brief
 * @version 0.1
 * @date 2019-03-14
 *
 * @copyright Copyright (c) 2019 ITE Tech. Inc. All Rights Reserved.
 *
 */
#include "ite/itp.h"
#include <linux/usb/g_hid.h>

extern int iteHiddInitialize(void);
extern int iteHiddTerminate(void);
extern int iteUdcRestart(void);
extern int iteUsbCompositeOverwrite(struct usb_composite_overwrite *covr);
extern int hidg_set_config(HIDG_FUNC_CONFIG *pUsbData);
extern ssize_t hidg_read(const char *buffer, size_t count);
extern ssize_t hidg_write(const char *buffer, size_t count);
extern ssize_t hidg_read_by_ID(HIDG_FUNC_CMD_DATA *pUsbData);
extern ssize_t hidg_write_by_ID(HIDG_FUNC_CMD_DATA *pUsbData);


static int UsbdHidRead(int file, char *ptr, int len, void* info)
{
	return hidg_read(ptr, len);
}

static int UsbdHidWrite(int file, char *ptr, int len, void* info)
{
    return hidg_write(ptr, len);
}

static int UsbdHidIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
        iteHiddInitialize();
        break;

    case ITP_IOCTL_USBD_HIHG_REFRESH:
        iteUdcRestart();
        break;

    case ITP_IOCTL_USBD_HIHG_SET_CONFIG:
        return hidg_set_config((HIDG_FUNC_CONFIG *) ptr);

    case ITP_IOCTL_USBD_HIHG_READ:
        return hidg_read_by_ID((HIDG_FUNC_CMD_DATA*) ptr);

    case ITP_IOCTL_USBD_HIHG_WRITE:
        return hidg_write_by_ID((HIDG_FUNC_CMD_DATA*) ptr);

    case ITP_IOCTL_USBD_HIHG_OVERWRITE_VIDPID:
        iteUsbCompositeOverwrite((struct usb_composite_overwrite*) ptr);
        break;        

    case ITP_IOCTL_DISABLE:
        iteHiddTerminate();
        break;
    }
    return 0;
}

const ITPDevice itpDeviceUsbdHid =
{
    ":usbd hid",
    itpOpenDefault,
    itpCloseDefault,
    UsbdHidRead,
    UsbdHidWrite,
    itpLseekDefault,
    UsbdHidIoctl,
    NULL
};
