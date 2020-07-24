/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


/*
 * This file contains definitions and data structures specific
 * to South Silicon Valley Cabrio 802.11 NIC. It contains the
 * Device Information structure struct cabrio_private..
 */
#ifndef _CABRIO_DEV_H_
#define _CABRIO_DEV_H_

#define REG32_R(addr)   ssv6xxx_drv_read_reg(addr)
#define REG32_W(addr, value)    ssv6xxx_drv_write_reg(addr, value)

#define MAC_REG_WRITE(_r, _v)    REG32_W(_r,_v)
#define MAC_REG_READ(_r, _v)    _v = REG32_R(_r)

#define REG32(addr) REG32_R(addr)

#define MAC_LOAD_FW(_s, _l) ssv_hal_download_fw(_s,_l)

#define MAC_REG_SET_BITS(_reg,_set,_clr) ssv6xxx_drv_set_reg((_reg),(_set),0,(_clr))

#define PRINT LOG_PRINTF
#define MSLEEP(x) \
if(x<TICK_RATE_MS){             \
    platform_udelay(x*1000);    \
}else{                          \
    OS_MsDelay(x);              \
}
#define UDELAY(x) platform_udelay(x) //OS_TickDelay(1)
#define MDELAY(x) UDELAY(x*1000)

#endif
