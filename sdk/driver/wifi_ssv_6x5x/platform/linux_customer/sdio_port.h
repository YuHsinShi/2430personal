/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _SDIO_H_
#define _SDIO_H_

//output timing
// 0: cmd  [0]:positive [1]:negative
// 1: data [0]:positive [1]:negative
#define SDIO_DEF_OUTPUT_TIMING 		0 // 3

extern ssv_type_bool sdio_writeb_cmd52(ssv_type_u32 addr, ssv_type_u8 data);
extern ssv_type_u8	sdio_readb_cmd52(ssv_type_u32 addr, ssv_type_u8 *data);
extern ssv_type_bool sdio_write_cmd53(ssv_type_u32 dataPort,ssv_type_u8 *dat, size_t size);
extern ssv_type_bool sdio_read_cmd53(ssv_type_u32 dataPort,ssv_type_u8 *dat, size_t size);
extern ssv_type_bool _sdio_read_reg(ssv_type_u32 addr, ssv_type_u32 *data);
//extern ssv_type_bool sdio_set_block_size(unsigned int blksize);
extern ssv_type_u32 sdio_get_block_size(void);
extern ssv_type_bool sdio_host_detect_card(void);
extern ssv_type_bool sdio_host_init(void (*sdio_isr)(void));
extern ssv_type_bool sdio_host_enable_isr(ssv_type_bool enable);
extern int sdio_set_clk(ssv_type_u32 clk);
extern ssv_type_bool is_truly_isr(void);
#endif /* _SDIO_H_ */

