/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _SDIO_H_
#define _SDIO_H_

#if (defined __linux__)
    #include <ssv_types.h>
	#include "sdio_def.h"
	#include "sdio_ioctl.h"
	#include "../ssv_drv_config.h"
	#include <log.h>
#elif (defined _WIN32)
	#error For now, SDIO driver is NOT supported in Win32 platform.
#else
	#error Unknown build platform!!!
#endif

ssv_type_u8		sdio_get_bus_width(void);
ssv_type_u32		sdio_get_bus_clock(void);
ssv_type_bool	sdio_set_bus_clock(ssv_type_u32 clock_rate);
ssv_type_bool	sdio_set_force_block_mode(ssv_type_bool mode);
ssv_type_bool	sdio_set_block_mode(ssv_type_bool mode);
ssv_type_bool	sdio_set_block_size(ssv_type_u16 size);
ssv_type_u16		sdio_get_block_size(void);				// will really call DeviceIoControl(...)
ssv_type_u16		sdio_get_cur_block_size(void);			// cheat func, will return 's_cur_block_size' value
ssv_type_bool	sdio_set_multi_byte_io_port(ssv_type_u32 port);
ssv_type_bool	sdio_get_multi_byte_io_port(ssv_type_u32 *port);
ssv_type_bool	sdio_set_multi_byte_reg_io_port(ssv_type_u32 port);
ssv_type_bool	sdio_get_multi_byte_reg_io_port(ssv_type_u32 *port);
ssv_type_bool	sdio_ack_int(void);
ssv_type_bool	sdio_get_driver_version(ssv_type_u16 *version);
ssv_type_bool	sdio_get_function_number(ssv_type_u8 *num);
/*
	the difference between r/w 'byte' & 'data'
	'byte' -> command mode,    need 'addr' 
	'data' -> data    mode, NO need 'addr'
*/
ssv_type_u8		sdio_read_byte(ssv_type_u8 func,ssv_type_u32 addr);
ssv_type_bool	sdio_write_byte(ssv_type_u8 func,ssv_type_u32 addr, ssv_type_u8 data);
void    sdio_set_data_mode(ssv_type_bool use_data_mode);
ssv_type_bool    sdio_read_reg(ssv_type_u32 addr, ssv_type_u32 *data);
ssv_type_bool	sdio_write_reg(ssv_type_u32 addr, ssv_type_u32 data);
ssv_type_bool    sdio_read_sram(ssv_type_u32 addr, ssv_type_u8 *data, ssv_type_u32 size);
ssv_type_bool    sdio_write_sram(ssv_type_u32 addr, ssv_type_u8 *data, ssv_type_u32 size);
ssv_type_s32		sdio_write_data(void *dat, ssv_type_size_t size);

// return
//  < 0 : fail
// >= 0 : # of bytes recieved
ssv_type_s32		sdio_read_data(ssv_type_u8 *dat, ssv_type_size_t size);
ssv_type_s32		sdio_ask_rx_data_len(void);
ssv_type_s32		sdio_read_dataEx(ssv_type_u8 *dat, ssv_type_size_t size);

ssv_type_bool	sdio_open(void);
ssv_type_bool	sdio_close(void);
ssv_type_bool	sdio_init(void);

// cheat func
ssv_type_u32		sdio_handle(void);

#endif /* _SDIO_H_ */

