#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>
#include "ite/itp.h"
#include "ite/ite_usbhid.h"
#include "linux/usb/ch9.h"

void iteUsbHidConnectedCallback(struct hid_device *hid, uint32_t usbIdx)
{
    ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);    
    ithPrintf("=>Vendor(%X)Product(%X): (%p) usb idx(%u)\n",
        hid->vendor, hid->product, hid, usbIdx);
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
        break;
    }
    case USB_DT_CONFIG:
    {
        desc_type = "CONFIG";
        break;
    }
    case USB_DT_INTERFACE:
    {
        desc_type = "INTERFACE";
        break;
    }
    case USB_DT_ENDPOINT:
    {
        desc_type = "ENDPOINT";
        break;
    }
    case HID_DT_HID:
    {
        desc_type = "HID_CLASS";
        break;
    }
    case HID_DT_REPORT:
        desc_type = "HID_REPORT";
        break;
    }
   ithPrintf("=>Vendor(%X)Product(%X): %u/%u - DESC_TYPE(%s)\n",
       hid->vendor, hid->product, index, total, desc_type);
   ithPrintVram8((uint32_t)start, size);
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
        //if need to output data to hid device:
        //iteUsbHidWrite(hid, dataBuf, dataLen);
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
}

void *TestFunc(void *arg)
{
    ithPrintf("%s(%d)\n", __FILE__, __LINE__);
    for (;;)
    {
        usleep(1000);
    }
}
