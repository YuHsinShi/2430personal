/** @file
 * ITE Hardware Library.
 *
 * @author Jim Tan
 * @version 1.0
 * @date 2011-2012
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
#ifndef ITH_GENERIC_H
#define ITH_GENERIC_H

#ifndef ITH_H
    #error "Never include this file directly. Use ite/ith.h instead."
#endif

#include "ith/ith_platform.h"
#include "ith/ith_defs.h"
#include "ith/ith_vram.h"
#include "ith/ith_reg.h"
#include "ith/ith_chipid.h"
#include "ith/ith_clock.h"
#include "ith/ith_memdbg.h"
#include "ith/ith_memstat.h"
#include "ith/ith_memwrap.h"
#include "ith/ith_host.h"
#include "ith/ith_ahb0.h"
#include "ith/ith_isp.h"
#include "ith/ith_saradc.h"
#include "ith/ith_usb.h"
#include "ith/ith_jpeg.h"
#include "ith/ith_uienc.h"
#include "ith/ith_video.h"
#include "ith/ith_lcd.h"
#include "ith/ith_cmdq.h"
#include "ith/ith_openvg.h"
#include "ith/ith_tve.h"
#include "ith/ith_fpc.h"
#include "ith/ith_nand.h"
#include "ith/ith_dpu.h"
#include "ith/ith_dcps.h"
#include "ith/ith_rtc.h"
#include "ith/ith_uart.h"
#include "ith/ith_intr.h"
#include "ith/ith_rs485.h"
#include "ith/ith_ioex.h"
#include "ith/ith_i2c.h"
#include "ith/ith_timer.h"
#include "ith/ith_wd.h"
#include "ith/ith_ir.h"
#include "ith/ith_pwm.h"
#include "ith/ith_keypad.h"
#include "ith/ith_xd.h"
#include "ith/ith_storage.h"
#include "ith/ith_utility.h"
#include "ith/ith_codec.h"
#include "ith_dma.h"
#include "ith_gpio.h"

/** @addtogroup ith ITE Hardware Library
 *  @brief Hardware abstract layer
 *  @{
 */
/** @} */ // end of ith

#endif // ITH_GENERIC_H