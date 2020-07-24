/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _TXRX_HDL_H
#define _TXRX_HDL_H
#include <ssv_devinfo.h>
#include <host_apis.h>

struct wifi_flt
{
    ssv_type_u8 fc_b7b2;
    ssv_type_u8 b7b2mask;
    data_handler cb_fn;
};

struct eth_flt
{
    ssv_type_u16 ethtype;
    data_handler cb_fn;    
};

ssv_type_bool TxHdl_prepare_wifi_txreq(ssv_type_u8 vif_idx, void *frame, ssv_type_u32 len, ssv_type_bool f80211, ssv_type_u32 priority, ssv_type_u8 tx_dscrp_flag);
ssv_type_bool TxHdl_FrameProc(void *frame, ssv_type_bool apFrame, ssv_type_u32 priority, ssv_type_u32 flags, ssv_vif* vif);
ssv_type_s32 TxHdl_FlushFrame(void);

ssv_type_s32 TxRxHdl_Init(void);
ssv_type_s32 RxHdl_FrameProc(void* frame);
ssv_type_s32 RxHdl_SetWifiRxFlt(struct wifi_flt *flt, ssv6xxx_cb_action act);
ssv_type_s32 RxHdl_SetEthRxFlt(struct eth_flt *flt, ssv6xxx_cb_action act);
void ieee80211_release_reorder_frames(struct rx_ba_session_desc *tid_agg_rx,
	ssv_type_u16 head_seq_num);
void ieee80211_delete_ampdu_rx(ssv_type_u8 wsid);
void ieee80211_delete_all_ampdu_rx(void);
void ieee80211_addba_handler(void *data);


#endif //_TXRX_HDL_H
