#ifndef HID_TRANSPARENT_H
#define HID_TRANSPARENT_H
#include "linux/usb/g_hid.h"

enum
{
    USB0 = 0,
    USB1
};

typedef struct hid_transparent_config {
    u8 bMaxInterfaces;
    u8 bInterfaceID;
    struct hid_device *pHid;
    HIDG_FUNC_CONFIG *pFc;
}HID_TRANSPARENT_CONFIG;

extern int HidTransparent_AddConfig(HID_TRANSPARENT_CONFIG *ptConf);
extern int HidTransparent_Write(struct hid_device *hid, HIDG_FUNC_CMD_DATA *usbData);
extern int HidTransparent_DelConfig(struct hid_device *hid, uint32_t usbIdx);

static int HidTransparent_GetUsbIdxID(int usbidx) {
    if (usbidx == USB0) return USBD_IDX_0; else if (usbidx == USB1) return USBD_IDX_1; else return USBD_IDX_NONE;
}
#endif