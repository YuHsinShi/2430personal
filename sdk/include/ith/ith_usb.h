#ifndef ITH_USB_H
#define ITH_USB_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_usb USB
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * USB module definition.
 */
typedef enum
{
    ITH_USB0    = ITH_USB0_BASE, ///< USB #0
    ITH_USB1    = ITH_USB1_BASE  ///< USB #1
} ITHUsbModule;

/**
 * Enters suspend mode on specified USB module.
 *
 * @param usb The USB module to suspend.
 */
void ithUsbSuspend(ITHUsbModule usb);

/**
 * Resumes from suspend mode on specified USB module.
 *
 * @param usb The USB module to resume.
 */
void ithUsbResume(ITHUsbModule usb);

/**
 * Enables the clock of USB module.
 */
void ithUsbEnableClock(void);

/**
 * Disables the clock of USB module.
 */
void ithUsbDisableClock(void);

/**
 * Reset the USB module.
 */
void ithUsbReset(void);

/**
 * USB AHB master interface type.
 */
typedef enum
{
    ITH_USB_WRAP    = 0, ///< Select USBWrap
    ITH_USB_AMBA    = 1  ///< Select AMBA
} ITHUsbInterface;

/**
 * USB AHB master interface select.
 * 
 * @param intf
 */
void ithUsbInterfaceSel(ITHUsbInterface intf);

/**
 * USB PHY power on.
 *
 * @param usb   ITH_USB0 or ITH_USB1
 *
 * - Example:
 * @code
 * ithUsbPhyPowerOn(ITH_USB0);
 * @endcode
 */
void ithUsbPhyPowerOn(ITHUsbModule usb);

/**
 * USB wrap flush.
 */
void ithUsbWrapFlush(void);

/**
 * Enables the clock of USB's PHY module.
 * 
 * @param usb
 */
static inline void ithUsbPhyEnableClock(ITHUsbModule usb)
{
    ithWriteRegMaskA(usb + 0x3C, 0x2, 0x2);
}

/**
 * Disables the clock of USB's PHY module.
 * 
 * @param usb
 */
static inline void ithUsbPhyDisableClock(ITHUsbModule usb)
{
    ithWriteRegMaskA(usb + 0x3C, 0x0, 0x2);
}


#ifdef __cplusplus
}
#endif

#endif // ITH_USB_H
/** @} */ // end of ith_usb
/** @} */ // end of ith