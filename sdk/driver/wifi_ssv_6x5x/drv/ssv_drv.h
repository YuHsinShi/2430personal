/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _SSV_DRV_H_
#define _SSV_DRV_H_

//#include <log.h>

//#ifndef MAX_BUF
//#define MAX_BUF			(8*1024)
//#endif

#define	_SDRV_DEBUG		0
#define _SDRV_TRACE		0
#define _SDRV_WARN		1
#define SSV_LDO_EN_PIN_ 0

#if (_SDRV_DEBUG)
#define SDRV_DEBUG					LOG_DEBUG
#else
#define SDRV_DEBUG(fmt, ...)		((void)0)
#endif

#if (_SDRV_TRACE)
#define SDRV_TRACE					LOG_TRACE
#else
#define SDRV_TRACE(fmt, ...)		((void)0)
#endif

#if (_SDRV_WARN)
#define SDRV_WARN			         LOG_WARN
#else
#define SDRV_WARN(fmt, ...)			((void)0)
#endif

#define SDRV_INFO					LOG_INFO
#define SDRV_ERROR		            LOG_ERROR
#define SDRV_FAIL	                LOG_FAIL
#define SDRV_FAIL_RET(r, fmt, ...)	{ LOG_FAIL(fmt, ##__VA_ARGS__); return r; }
#define SDRV_FATAL					LOG_FATAL

#define PRI_Q_NUM 5

typedef enum{
    GO_AHEAD    ,
    KEEP_TRY    ,
    NEXT_ONE	,
    PAUSE_TX    
}TX_STATE;
ssv_type_bool	ssv6xxx_drv_module_init(void);
void	ssv6xxx_drv_module_release(void);

/* ---------------------------------------------------------------
Note :
	if success!, this func will automatically do these for you
		1. automatically close() the previous ssv_drv
		2. automatically open()  the current  ssv_drv
		3. automatically init()	 the current  ssv_drv

	if Fail!, this func will do nothing.
--------------------------------------------------------------- */
ssv_type_bool	ssv6xxx_drv_select(char name[32]);
void	ssv6xxx_drv_list(void);


// Caution! : the function prototype must be the same with whose in 'struct ssv6xxx_drv_ops'!!!
ssv_type_bool	ssv6xxx_drv_open(void);
ssv_type_bool	ssv6xxx_drv_close(void);
ssv_type_bool	ssv6xxx_drv_init(void);
ssv_type_s32 	ssv6xxx_drv_recv(ssv_type_u8 *dat, ssv_type_size_t len);
ssv_type_s32 	ssv6xxx_drv_send(void *dat, ssv_type_size_t len);
ssv_type_bool	ssv6xxx_drv_get_name(char name[32]);
ssv_type_bool	ssv6xxx_drv_ioctl(ssv_type_u32 ctl_code,
					  void *in_buf, ssv_type_size_t in_size,
					  void *out_buf, ssv_type_size_t out_size,
					  ssv_type_size_t *bytes_ret);

ssv_type_u32	ssv6xxx_drv_read_reg(ssv_type_u32 addr);
ssv_type_bool	ssv6xxx_drv_write_reg(ssv_type_u32 addr, ssv_type_u32 data);
ssv_type_bool	ssv6xxx_drv_set_reg(ssv_type_u32 _REG_,ssv_type_u32 _VAL_,ssv_type_u32 _SHIFT_, ssv_type_u32 _MASK_);
ssv_type_bool	ssv6xxx_drv_write_byte(ssv_type_u32 addr, ssv_type_u32 data);
ssv_type_bool	ssv6xxx_drv_read_byte(ssv_type_u32 addr);

ssv_type_bool    ssv6xxx_drv_write_sram (ssv_type_u32 addr, ssv_type_u8 *data, ssv_type_u32 size);
ssv_type_bool    ssv6xxx_drv_read_sram (ssv_type_u32 addr, ssv_type_u8 *data, ssv_type_u32 size);
ssv_type_bool    ssv6xxx_drv_start (void);
ssv_type_bool    ssv6xxx_drv_stop (void);
ssv_type_bool    ssv6xxx_drv_irq_enable(ssv_type_bool is_isr);
ssv_type_bool    ssv6xxx_drv_irq_disable(ssv_type_bool is_isr);

ssv_type_s32 ssv6xxx_drv_send_sim(void *dat, ssv_type_size_t len);
ssv_type_u32 ssv6xxx_drv_get_handle(void);
ssv_type_bool ssv6xxx_drv_ack_int(void);
ssv_type_u32 ssv6xxx_drv_get_TRX_time_stamp(void);
ssv_type_bool ssv6xxx_drv_wakeup_wifi(ssv_type_bool sw);
ssv_type_bool ssv6xxx_drv_detect_card(void);
TX_STATE ssv6xxx_drv_tx_resource_enough(ssv_type_u32 frame_len,ssv_type_u16 frame_cnt, ssv_type_u16 q_idx);
ssv_type_bool ssv6xxx_drv_wait_tx_resource(ssv_type_u32 frame_len);
#endif /* _SSV_DRV_H_ */

