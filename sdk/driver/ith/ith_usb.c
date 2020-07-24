/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL USB functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <unistd.h>
#include "ith_cfg.h"


void ithUsbSuspend(ITHUsbModule usb)
{
#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9910) || (CFG_CHIP_FAMILY == 9850))
    switch (usb)
    {
    case ITH_USB0:
        ithClearRegBitH(ITH_USB0_PHY_CTRL_REG, ITH_USB0_PHY_OSC_OUT_EN_BIT);
        ithClearRegBitH(ITH_USB0_PHY_CTRL_REG, ITH_USB0_PHY_PLL_ALIV_BIT);
        break;

    case ITH_USB1:
        ithClearRegBitH(ITH_USB1_PHY_CTRL_REG, ITH_USB1_PHY_OSC_OUT_EN_BIT);
        ithClearRegBitH(ITH_USB1_PHY_CTRL_REG, ITH_USB1_PHY_PLL_ALIV_BIT);
        break;
    }
#else
    ithWriteRegMaskA(usb + 0x3C, 0x0, 0x14);
#endif
    ithSetRegBitA(usb + ITH_USB_HC_MISC_REG, ITH_USB_HOSTPHY_SUSPEND_BIT);
}

void ithUsbResume(ITHUsbModule usb)
{
#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9910) || (CFG_CHIP_FAMILY == 9850))
    switch (usb)
    {
    case ITH_USB0:
        ithSetRegBitH(ITH_USB0_PHY_CTRL_REG, ITH_USB0_PHY_OSC_OUT_EN_BIT);
        ithSetRegBitH(ITH_USB0_PHY_CTRL_REG, ITH_USB0_PHY_PLL_ALIV_BIT);
        break;

    case ITH_USB1:
        ithSetRegBitH(ITH_USB1_PHY_CTRL_REG, ITH_USB1_PHY_OSC_OUT_EN_BIT);
        ithSetRegBitH(ITH_USB1_PHY_CTRL_REG, ITH_USB1_PHY_PLL_ALIV_BIT);
        break;
    }
#else
    ithWriteRegMaskA(usb + 0x3C, 0x10, 0x14);
#endif
    ithClearRegBitA(usb + ITH_USB_HC_MISC_REG, ITH_USB_HOSTPHY_SUSPEND_BIT);
}

void ithUsbEnableClock(void)
{
#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9910) || (CFG_CHIP_FAMILY == 9850))
    ithSetRegBitH(ITH_USB_CLK_REG, ITH_EN_N6CLK_BIT);
    ithSetRegBitH(ITH_USB_CLK_REG, ITH_EN_M11CLK_BIT);
#else
    ithWriteRegMaskA(ITH_HOST_BASE + ITH_USB_CLK_REG, 0xA, 0xA);
#endif
}

void ithUsbDisableClock(void)
{
#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9910) || (CFG_CHIP_FAMILY == 9850))
    ithClearRegBitH(ITH_USB_CLK_REG, ITH_EN_N6CLK_BIT);
    ithClearRegBitH(ITH_USB_CLK_REG, ITH_EN_M11CLK_BIT);
#else
    ithWriteRegMaskA(ITH_HOST_BASE + ITH_USB_CLK_REG, 0x0, 0xA);
#endif
}

void ithUsbReset(void)
{
    // for 9860
    ithWriteRegA(ITH_HOST_BASE + ITH_USB_CLK_REG, 0xC0A80000);
    usleep(5*1000);
    ithWriteRegA(ITH_HOST_BASE + ITH_USB_CLK_REG, 0x00A80000);
    usleep(5*1000);
}

void ithUsbInterfaceSel(ITHUsbInterface intf)
{
    #if 0 // wrap default setting fail
    if (intf == ITH_USB_AMBA)
        ithWriteRegMaskA(ITH_USB0_BASE + 0x34, 0x1, 0x1);
    else
        ithWriteRegMaskA(ITH_USB0_BASE + 0x34, 0x0, 0x1);
    #else
    //uint32_t reg = 0x422A0800;
    uint32_t reg = 0x442A0800;
    if (intf == ITH_USB_AMBA)
        reg |= 0x1;
    else
        reg &= ~0x1;
    ithWriteRegA(ITH_USB0_BASE + 0x34, reg);
    #endif
}

void ithUsbPhyPowerOn(ITHUsbModule usb)
{
    ithWriteRegA(usb + 0x3C, 0x00031907);
}

#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9910) || (CFG_CHIP_FAMILY == 9850))
#else // 970
#define USB_WRAP_REG						0x34
#define USB_WRAP_FLUSH_EN				(0x1 << 22)
#define USB_WRAP_FLUSH_FIRE_END (0x1 << 23)
#endif

void ithUsbWrapFlush(void)
{
#if (CFG_CHIP_FAMILY == 9070)
    ithFlushAhbWrap();
#elif ((CFG_CHIP_FAMILY == 9910) || (CFG_CHIP_FAMILY == 9850))
#else // 970
    {
        uint32_t reg = ithReadRegA(ITH_USB0_BASE + 0x34);
        /* only for wrap path */
        if (!(reg & 0x1)) {
			int timeout = 100;

            ithEnterCritical();

            /* flush usb's ahb wrap */
            ithWriteRegMaskA(ITH_USB0_BASE + USB_WRAP_REG, USB_WRAP_FLUSH_EN, USB_WRAP_FLUSH_EN);
            ithWriteRegMaskA(ITH_USB0_BASE + USB_WRAP_REG, USB_WRAP_FLUSH_FIRE_END, USB_WRAP_FLUSH_FIRE_END);

            // wait AHB Wrap flush finish!
            #if 1 // workaround: sometimes willn't flush end when CPU > 700MHz
			while ((ithReadRegA(ITH_USB0_BASE + USB_WRAP_REG) & USB_WRAP_FLUSH_FIRE_END) && timeout--);
			if (timeout <= 0)
				ithPrintf("$\n");
			#else
            while ((ithReadRegA(ITH_USB0_BASE + USB_WRAP_REG) & USB_WRAP_FLUSH_FIRE_END));
			#endif

            ithWriteRegMaskA(ITH_USB0_BASE + USB_WRAP_REG, 0x0, USB_WRAP_FLUSH_EN);

            /* flush axi2 wrap */
            if(ithMemWrapFlush(ITH_MEM_AXI2WRAP))
                ithPrintf("+\n");

            ithExitCritical();
        }
    }
#endif
}
