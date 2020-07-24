/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _AP_RX_H_
#define _AP_RX_H_

#include <ssv_types.h>
#include "common/defs.h"

struct APStaInfo;



// struct cfg_host_rxpkt {
//     /* The definition of WORD_1: */
// 	u32             len:16;
// 	u32             c_type:3;
//     u32             f80211:1;
// 	u32             qos:1;          /* 0: without qos control field, 1: with qos control field */
//     u32             ht:1;           /* 0: without ht control field, 1: with ht control field */
//     u32             use_4addr:1;
// 	u32             l3cs_err:1;
//     u32             l4cs_err:1;
//     u32             RSVD_0:3;
// 	u32             psm:1;
//     u32             stype_b5b4:2;
//     u32             extra_info:1;
// };




//-------------------------------------------------------------
//bit 0~bit2
#define AP_RX_FLAGS_FRAME_TYPE_PS_POLL			BIT(0)
#define AP_RX_FLAGS_FRAME_TYPE_NULL_DATA		BIT(1)
#define AP_RX_FLAGS_FRAME_TYPE_DATA				BIT(2)



#define AP_RX_FLAGS_PM_BIT						BIT(3)
#define AP_RX_FLAGS_QOS_BIT						BIT(4)

//#define AP_RX_FLAGS_PM_BIT						BIT(3)


struct ap_rx_desp {
	ssv_type_u8 *data;					//store struct cfg_host_rxpkt *
	struct APStaInfo *sta;
	ssv_type_u32 flags;
	ssv_type_u8 UP;
};


typedef ssv_type_u32 rx_result;
#define RX_CONTINUE	((rx_result) 0u)
#define RX_DROP		((rx_result) 1u)
//#define RX_QUEUED	((rx_result) 2u)



rx_result rx_h_uapsd_and_pspoll(struct ap_rx_desp *rx);



// enum AP_RECV_FRAME_TYPE{
// };



ssv_type_bool ssv6xxx_data_need_to_be_received(struct ap_rx_desp *rx_desp);



ssv_type_u8* ssv6xxx_host_rx_data_get_qos_ptr(CFG_HOST_RXPKT *rxpkt);



#endif /* _AP_RX_H_ */
