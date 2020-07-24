#ifndef ITE_USB_HID_H
#define ITE_USB_HID_H

#include <linux/hid.h>

#ifdef __cplusplus
extern "C" {
#endif

int iteUsbHidRegister(void);

/**
 * @brief 
 * 
 * @param hid   hid device handle
 * @param type  Descriptor type. USB_DT_DEVICE, USB_DT_CONFIG, USB_DT_STRING,
 *              USB_DT_INTERFACE, USB_DT_ENDPOINT, HID_DT_HID, HID_DT_REPORT...etc.
 * @param total The total number of descriptors of the same type
 * @param index The index number (start from 0) of this descriptor. 
 * @param start start of descriptor's data
 * @param size  size of descriptor
 */
void iteUsbHidDescriptorReceivedCallback(
    struct hid_device *hid, 
    uint32_t type, 
    uint32_t total,
    uint32_t index,
    void* start, 
    int size);
    
/**
 * @brief 
 * 
 * @param hid   hid device handle
 * @param type  Report type. HID_INPUT_REPORT, HID_OUTPUT_REPORT, HID_FEATURE_REPORT.
 * @param data  pointer to the received report data
 * @param size  size of the received report
 */   
void iteUsbHidInputReportReceivedCallback(
    struct hid_device *hid, 
    int type, 
    void* data, 
    int size, int interrupt);

/**
 * @brief 
 * 
 * @param hid   hid device handle
 */
void iteUsbHidConnectedCallback(
    struct hid_device *hid, uint32_t usbIdx);

/**
 * @brief 
 * 
 * @param hid   hid device handle
 */
void iteUsbHidDisconnectingCallback(
    struct hid_device *hid, uint32_t usbIdx);

ssize_t iteUsbHidWrite(struct hid_device *hid, const char *buffer, size_t count);

#ifdef __cplusplus
}
#endif

#endif // ITE_USB_HID_H