/** @file
 * ITE USB Driver API header file.
 *
 * @author Irene Lin
 * @version 1.0
 * @date 2011-2012
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */

#ifndef ITE_USB_EX_H
#define ITE_USB_EX_H

#define USBEX_API extern

//=============================================================================
//                              Include Files
//=============================================================================
#include "ite/ith.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup ith ITE Hardware Library
 *  @{
 */

/** @addtogroup ith_usb USB
 *  The supported driver API for ITE usb controller.
 *  @{
 */

//=============================================================================
//                              Constant Definition
//=============================================================================

#define USB_MAX_INTERFACE   3

//=========================
/**
 * For HOST Driver
 */
//=========================
enum
{
    USB0 = 0,
    USB1
};

/**
 * The speed of connected device.
 *
 * - Example
 * @code
 * int speed = mmpUsbExGetSpeed(USB0);
 * @endcode
 *
 * @internal Irene: 2010_1216
 */
enum
{
    USBEX_SPEED_UNKNOWN = 0,            ///< Not Connected
    USBEX_SPEED_LOW,                    ///< USB 1.1 low speed
    USBEX_SPEED_FULL,                   ///< USB 1.1 full speed
    USBEX_SPEED_HIGH                    ///< USB 2.0 high speed
};

/**
 * - Example
 * @code
 * uint32_t state = mmpUsbExCheckDeviceState();
 * @endcode
 */
enum
{
    USB_DEVICE_TYPE_MSC         = 0x01,
    USB_DEVICE_TYPE_UAS         = 0x02,
    USB_DEVICE_TYPE_WIFI        = 0x03,
    USB_DEVICE_TYPE_KBD         = 0x04,
    USB_DEVICE_TYPE_MOUSE       = 0x05,
    USB_DEVICE_TYPE_DEMOD       = 0x06,
    USB_DEVICE_TYPE_UVC         = 0x07,
	USB_DEVICE_TYPE_ECM 		= 0x08,
    USB_DEVICE_TYPE_SERIAL      = 0x09,
    USB_DEVICE_TYPE_OPTION		= 0x0A,
    USB_DEVICE_TYPE_NOT_MATCH   = 0
};

enum
{
    USB_DEVICE_STATE_NOCHAGNE,
    USB_DEVICE_STATE_CONNECT,
    USB_DEVICE_STATE_DISCONNECT
};

#define USB_DEVICE_CONNECT(x)       ((x)==USB_DEVICE_STATE_CONNECT)
#define USB_DEVICE_DISCONNECT(x)    ((x)==USB_DEVICE_STATE_DISCONNECT)
#define USB_DEVICE_WIFI(x)          ((x)==USB_DEVICE_TYPE_WIFI)
#define USB_DEVICE_UAS(x)           ((x)==USB_DEVICE_TYPE_UAS)
#define USB_DEVICE_MSC(x)           ((x)==USB_DEVICE_TYPE_MSC)
#define USB_DEVICE_KBD(x)           ((x)==USB_DEVICE_TYPE_KBD)
#define USB_DEVICE_MOUSE(x)         ((x)==USB_DEVICE_TYPE_MOUSE)
#define USB_DEVICE_DEMOD(x)         ((x)==USB_DEVICE_TYPE_DEMOD)
#define USB_DEVICE_UVC(x)           ((x)==USB_DEVICE_TYPE_UVC)
#define USB_DEVICE_ECM(x)           ((x)==USB_DEVICE_TYPE_ECM)
#define USB_DEVICE_SERIAL(x)        ((x)==USB_DEVICE_TYPE_SERIAL)
#define USB_DEVICE_OPTION(x)        ((x)==USB_DEVICE_TYPE_OPTION)

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct
{
    int     type;
    void*   ctxt;
} USB_DEVICE_INFO;

//=============================================================================
//                              Enumeration Type Definition
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

//=========================
/**
 * For HOST Driver
 */
//=========================
USBEX_API void* USBEX_ThreadFunc(void* data);


/**
 *
 * - Example
 * @code
 * int res;
 * int usb_en = 0;
 *
 * #if defined(CFG_USB0_ENABLE)
 * usb_en |= (1 << USB0);
 * #endif
 * #if defined(CFG_USB1_ENABLE)
 * usb_en |= (1 << USB1);
 * #endif
 *
 * res = mmpUsbExInitialize(usb_en);
 *
 * if (res) // error
 *      return res;
 * else
 * {
 *     // success
 * }
 * @endcode
 */
USBEX_API int mmpUsbExInitialize(int);

//=============================================================================
/**
 * Get newly USB device state.
 *
 * @param  usb      choose USB0 or USB1
 * @param  state    usb device is first connect or disconnect or no change.
 * @param  device_info      get this device information
 * @return MMP_RESULT_SUCCESS if succeed.
 *
 * @see USB_DEVICE_INFO
 */
//=============================================================================
USBEX_API uint32_t
mmpUsbExCheckDeviceState(
    int usb,
    int* state,
    USB_DEVICE_INFO* device_info
);

USBEX_API int iteUsbExIsPortConnect(int usb);

/**
 * @brief  Get if USB0 could be function as a device.
 *
 * @retval true   YES
 * @retval false  NO
 */
USBEX_API bool
mmpUsbExUsb0IsOtg(void);

/**
 * Get the speed of connected device.
 *
 * @param[in] usb choose USB0 or USB1
 * @retval  USBEX_SPEED_UNKNOWN
 * @retval  USBEX_SPEED_LOW
 * @retval  USBEX_SPEED_FULL
 * @retval  USBEX_SPEED_HIGH
 *
 *
 * - Example
 * @code
 * int speed = mmpUsbExGetSpeed(USB0);
 * @endcode
 *
 * @internal Irene: 2010_1216
 */
USBEX_API int
mmpUsbExGetSpeed(int usb);


#if defined(CFG_USB_DEVICE)
//=============================================================================
/**
 * Return OTG Device mode device is connect or not
 *
 * @return true if device is connect, return false if device is not connect.
 */
//=============================================================================
USBEX_API bool iteOtgIsDeviceMode(void);

USBEX_API void iteUsbExForceDeviceMode(bool enable);

#endif // #if defined(CFG_USB_DEVICE)


//@}

//@}

enum
{
    USB_PORT_TEST_J_STATE  = 0x1,   /* Test J_STATE */
    USB_PORT_TEST_K_STATE  = 0x2,   /* Test K_STATE */
    USB_PORT_TEST_SE0_NAK  = 0x3,   /* Test SE0_NAK */
    USB_PORT_TEST_PACKET   = 0x4,   /* Test Packet */
    USB_PORT_TEST_FORCE_EN = 0x5    /* Test FORCE_ENABLE */
};

USBEX_API int
mmpUsbExPortControl(
    int     usb,
    uint32_t  ctrl);

#if defined(USB_LOGO_TEST)

#define HUB_ERROR       -9999

#if 0
/** @see mmpUsbExPortControl() param ctrl */
enum
{
    USB_PORT_TEST_J_STATE  = 0x1,   /* Test J_STATE */
    USB_PORT_TEST_K_STATE  = 0x2,   /* Test K_STATE */
    USB_PORT_TEST_SE0_NAK  = 0x3,   /* Test SE0_NAK */
    USB_PORT_TEST_PACKET   = 0x4,   /* Test Packet */
    USB_PORT_TEST_FORCE_EN = 0x5    /* Test FORCE_ENABLE */
};
#endif

/** @see mmpUsbExDeviceControl() */
enum
{
    USB_DEV_CTRL_SINGLE_STEP_GET_DEV      = 0x1,
    USB_DEV_CTRL_SINGLE_STEP_SET_FEATURE  = 0x2
};

#if 0
/** For USB logo test "Host Port Control". */
USBEX_API int
mmpUsbExPortControl(
    int     usb,
    uint32_t  ctrl);
#endif

USBEX_API int
mmpUsbExDeviceControl(
    void* usb_dev,
    uint32_t  ctrl,
    uint32_t  step,
    uint8_t*  data);

USBEX_API bool
mmpUsbExIsDeviceConnect(int usb);

USBEX_API int
mmpUsbExSuspend(int usb);

USBEX_API int
mmpUsbExResume(int usb);


#endif // #if defined(USB_LOGO_TEST)


#ifdef __cplusplus
}
#endif

#endif

