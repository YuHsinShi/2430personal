/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _SSV_DRV_IF_H_
#define _SSV_DRV_IF_H_

#include <ssv_types.h>
#include "ssv_drv_config.h"
#include "ssv_drv.h"

typedef enum
{
	SSV6XXX_DRV_SIM = 1,
	SSV6XXX_DRV_UART,
	SSV6XXX_DRV_USB,
	SSV6XXX_DRV_SDIO,
	SSV6XXX_DRV_SPI,
	SSV6XXX_DRV_NUM
} SSV6XXX_DRV_TYPE;

struct ssv6xxx_drv_ops
{
	char	name[32];
	// SSV6XXX_DRV_TYPE		type;
	ssv_type_bool	(* open)(void);
	ssv_type_bool	(* close)(void);
	ssv_type_bool	(* init)(void);
	// return
	//  < 0 : fail
	// >= 0 : # of bytes recieve
	ssv_type_s32 	(* recv)(ssv_type_u8 *dat, ssv_type_size_t len);
    // return
	//  < 0 : fail
	// >= 0 : # of bytes send
	ssv_type_s32 	(* send)(void *dat, ssv_type_size_t len);
	ssv_type_bool	(* get_name)(char name[32]);
	ssv_type_bool	(* ioctl)(ssv_type_u32 ctl_code,
					  void *in_buf, ssv_type_size_t in_size,
					  void *out_buf, ssv_type_size_t out_size,
					  ssv_type_size_t *bytes_ret);
	ssv_type_u32		(*handle)(void);
    ssv_type_bool	(*ack_int)(void);
    ssv_type_bool    (*write_sram)(ssv_type_u32 addr, ssv_type_u8 *data, ssv_type_u32 size);
    ssv_type_bool    (*read_sram)(ssv_type_u32 addr, ssv_type_u8 *data, ssv_type_u32 size);
    ssv_type_bool    (*write_reg)(ssv_type_u32 addr, ssv_type_u32 data);
    ssv_type_u32     (*read_reg)(ssv_type_u32 addr);
    ssv_type_bool    (*write_byte)(ssv_type_u8 func,ssv_type_u32 addr, ssv_type_u8 data);
    ssv_type_u32    (*read_byte)(ssv_type_u8 func,ssv_type_u32 addr);
	ssv_type_u32    (*write_fw_to_sram)(ssv_type_u8 *bin, ssv_type_u32 bin_len, ssv_type_u32 block_size);
    ssv_type_s32    (*start)(void);
    ssv_type_s32    (*stop)(void);
	void    (*ssv_irq_enable)(void);
    void    (*ssv_irq_disable)(void);
    ssv_type_bool    (*wakeup_wifi)(ssv_type_bool sw);
    ssv_type_bool	(* detect_card)(void);
};
#if ((defined(SDRV_INCLUDE_SIM)) && (SDRV_INCLUDE_SIM))
	extern struct ssv6xxx_drv_ops	g_drv_sim;
#endif

#if ((defined(SDRV_INCLUDE_UART)) && (SDRV_INCLUDE_UART))
	extern struct ssv6xxx_drv_ops	g_drv_uart;
#endif

#if ((defined(SDRV_INCLUDE_USB)) && (SDRV_INCLUDE_USB))
	extern struct ssv6xxx_drv_ops	g_drv_usb;
#endif

#if ((defined(SDRV_INCLUDE_SDIO)) && (SDRV_INCLUDE_SDIO))
	extern const struct ssv6xxx_drv_ops	g_drv_sdio;
#endif

#if ((defined(SDRV_INCLUDE_SPI)) && (SDRV_INCLUDE_SPI))
	extern struct ssv6xxx_drv_ops	g_drv_spi;
#endif

#endif	/* _HDRV_SIM_IF_H_ */
