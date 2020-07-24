#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>
#include "ite/itp.h"
#include "ite/ite_usbhid.h"
#include "linux/usb/ch9.h"
#include "hid_transparent.h"

static HID_TRANSPARENT_CONFIG gtConf;

void iteUsbHidConnectedCallback(struct hid_device *hid, uint32_t usbIdx)
{
    ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);    
    ithPrintf("=>Vendor(%X)Product(%X): (%p) usb idx(%u)\n",
        hid->vendor, hid->product, hid, usbIdx);
    gtConf.pFc = calloc(1, sizeof(HIDG_FUNC_CONFIG));
    assert(gtConf.pFc);
    gtConf.pFc->idx = HidTransparent_GetUsbIdxID(usbIdx);
    gtConf.pFc->idVendor = hid->vendor;
    gtConf.pFc->idProduct = hid->product;
    gtConf.pHid = hid;
}

void iteUsbHidDescriptorReceivedCallback(
    struct hid_device *hid, 
    uint32_t type, 
    uint32_t total,
    uint32_t index,
    void* start, 
    int size)
{
    const char* desc_type = "UNKNOWN";
    switch (type)
    {
    case USB_DT_DEVICE:
    {
        desc_type = "DEVICE";
        struct usb_device_descriptor *ptr = (struct usb_device_descriptor *)start;
        ithPrintf("=>Vendor(%X)Product(%X): %u/%u - DESC_TYPE(%s) hid type(%u)\n",
            hid->vendor, hid->product, index, total, desc_type, hid->type);
        ithPrintf("=>Vendor(%X)Product(%X): %u/%u - DESC_TYPE(%s) hid dev_rsize(%u)\n",
            hid->vendor, hid->product, index, total, desc_type, hid->dev_rsize);
        ithPrintf("=>Vendor(%X)Product(%X): %u/%u - DESC_TYPE(%s) hid rsize(%u)\n",
            hid->vendor, hid->product, index, total, desc_type, hid->rsize);
        ithPrintf("=>Vendor(%X)Product(%X): %u/%u - DESC_TYPE(%s) hid collection_size(%u)\n",
            hid->vendor, hid->product, index, total, desc_type, hid->collection_size);
        ithPrintf("=>Vendor(%X)Product(%X): %u/%u - DESC_TYPE(%s) hid maxcollection(%u)\n",
                    hid->vendor, hid->product, index, total, desc_type, hid->maxcollection);
        ithPrintf("=>Vendor(%X)Product(%X): %u/%u - DESC_TYPE(%s) hid maxapplication(%u)\n",
            hid->vendor, hid->product, index, total, desc_type, hid->maxapplication);
        ithPrintf("=>Vendor(%X)Product(%X): %u/%u - DESC_TYPE(%s) hid group(%u)\n",
            hid->vendor, hid->product, index, total, desc_type, hid->group);
        break;
    }
    case USB_DT_CONFIG:
    {
        desc_type = "CONFIG";
        struct usb_config_descriptor *ptr = (struct usb_config_descriptor *)start;
        ithPrintf("=>Vendor(%X)Product(%X): %u/%u - DESC_TYPE(%s) bNumInterfaces(%u)\n",
            hid->vendor, hid->product, index, total, desc_type, ptr->bNumInterfaces);
        gtConf.bMaxInterfaces = ptr->bNumInterfaces;
        break;
    }
    case USB_DT_INTERFACE:
    {
        desc_type = "INTERFACE";
        struct usb_interface_descriptor *ptr = (struct usb_interface_descriptor *)start;
        ithPrintf("=>Vendor(%X)Product(%X): %u/%u - DESC_TYPE(%s) bInterfaceNumber(%u)\n",
            hid->vendor, hid->product, index, total, desc_type, ptr->bInterfaceNumber);
        ithPrintf("=>Vendor(%X)Product(%X): %u/%u - DESC_TYPE(%s) bInterfaceSubClass(%u)\n",
            hid->vendor, hid->product, index, total, desc_type, ptr->bInterfaceSubClass);
        ithPrintf("=>Vendor(%X)Product(%X): %u/%u - DESC_TYPE(%s) bInterfaceProtocol(%u)\n",
            hid->vendor, hid->product, index, total, desc_type, ptr->bInterfaceProtocol);
        gtConf.pFc->subclass = ptr->bInterfaceSubClass;
        gtConf.pFc->protocol = ptr->bInterfaceProtocol;
        gtConf.bInterfaceID = ptr->bInterfaceNumber;
        break;
    }
    case USB_DT_ENDPOINT:
    {
        desc_type = "ENDPOINT";
        struct usb_endpoint_descriptor *ptr = (struct usb_endpoint_descriptor *)start;        
        ithPrintf("=>Vendor(%X)Product(%X): %u/%u - DESC_TYPE(%s) wMaxPacketSize(%u)\n",
            hid->vendor, hid->product, index, total, desc_type, ptr->wMaxPacketSize);
        gtConf.pFc->report_length = ptr->wMaxPacketSize;
        break;
    }
    case HID_DT_HID:
    {
        desc_type = "HID_CLASS";
        struct hid_descriptor *ptr = (struct hid_descriptor *)start;        
        ithPrintf("=>Vendor(%X)Product(%X): %u/%u - DESC_TYPE(%s) wDescriptorLength(%u)\n",
            hid->vendor, hid->product, index, total, desc_type, ptr->desc[0].wDescriptorLength);
        gtConf.pFc->report_desc_length = ptr->desc[0].wDescriptorLength;
        break;
    }
    case HID_DT_REPORT:
        desc_type = "HID_REPORT";
        ithPrintf("=>Vendor(%X)Product(%X): %u/%u - DESC_TYPE(%s) report_desc : (%u)\n",
            hid->vendor, hid->product, index, total, desc_type, size);
        ithPrintVram8((uint32_t)start, size);
        gtConf.pFc->report_desc = kmemdup((unsigned char *)start, size, GFP_KERNEL);
        assert(gtConf.pFc->report_desc);
        HidTransparent_AddConfig(&gtConf);
        free(gtConf.pFc->report_desc);
        free(gtConf.pFc);
        break;
    }
//    ithPrintf("=>Vendor(%X)Product(%X): %u/%u - DESC_TYPE(%s)\n",
//        hid->vendor, hid->product, index, total, desc_type);
//    ithPrintVram8((uint32_t)start, size);
}
    
void iteUsbHidInputReportReceivedCallback(
    struct hid_device *hid, 
    int type, 
    void* data, 
    int size, int interrupt)
{
    
    const char* report_type = "UNKNOWN";
    switch (type)
    {
    case HID_INPUT_REPORT:
        report_type = "INPUT_REPORT";
        HIDG_FUNC_CMD_DATA usbData = {0};
        usbData.idVendor = hid->vendor;
        usbData.idProduct = hid->product;
        usbData.buffer = data;
        usbData.count = size;
        HidTransparent_Write(hid, &usbData);
        break;
    case HID_OUTPUT_REPORT:
        report_type = "OUTPUT_REPORT";
        break;
    case HID_FEATURE_REPORT:
        report_type = "FEATURE_REPORT";
        break;
    }
    ithPrintf("=>Vendor(%X)Product(%X): REPORT_TYPE(%s)\n",
        hid->vendor, hid->product, report_type);
    ithPrintVram8((uint32_t)data, size);
}

void iteUsbHidDisconnectingCallback(
    struct hid_device *hid, uint32_t usbIdx)
{
    ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);    
    ithPrintf("=>Vendor(%X)Product(%X): (%p) usb idx(%u)\n",
        hid->vendor, hid->product, hid, usbIdx);
    HidTransparent_DelConfig(hid, HidTransparent_GetUsbIdxID(usbIdx));
}

void *TestFunc(void *arg)
{
    ithPrintf("%s(%d)\n", __FILE__, __LINE__);
    for (;;)
    {
        //ITPMouseEvent ev;

#if 0
        //if (read(ITP_DEVICE_USBMOUSE, &ev, sizeof(ITPMouseEvent)) == sizeof(ITPMouseEvent))
        {
            // f_hidg_mouse_write(&ev, sizeof(ITPMouseEvent), 0);
#if 0
            printf("mouse: (%d, %d, %d) %s%s%s%s %s%s%s%s \r\n",
                   ev.x, ev.y, ev.wheel,
                   (ev.flags & ITP_MOUSE_LBTN_DOWN) ? "L-D " : "",
                   (ev.flags & ITP_MOUSE_RBTN_DOWN) ? "R-D " : "",
                   (ev.flags & ITP_MOUSE_MBTN_DOWN) ? "M-D " : "",
                   (ev.flags & ITP_MOUSE_SBTN_DOWN) ? "S-D " : "",
                   (ev.flags & ITP_MOUSE_LBTN_UP) ? "L-U " : "",
                   (ev.flags & ITP_MOUSE_RBTN_UP) ? "R-U " : "",
                   (ev.flags & ITP_MOUSE_MBTN_UP) ? "M-U " : "",
                   (ev.flags & ITP_MOUSE_SBTN_UP) ? "S-U " : "");
#endif
        }
#endif

        usleep(1000);
    }
}
