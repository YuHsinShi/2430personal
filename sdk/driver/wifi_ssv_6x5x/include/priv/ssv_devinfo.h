/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _SSV_DEVINFO_H_
#define _SSV_DEVINFO_H_
#include <host_apis.h>
//#include "cmd_def.h"
//#include <rtos.h>

//#ifdef ARRAY_SIZE
#undef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
//#endif




typedef ssv_type_u32 tx_result;
#define TX_CONTINUE	((tx_result) 0u)
#define TX_DROP		((tx_result) 1u)
#define TX_QUEUED	((tx_result) 2u)


//Off->Init->Running->Pause->Off


enum TXHDR_ENCAP {
	TXREQ_ENCAP_ADDR4               =0,
	TXREQ_ENCAP_QOS                 ,
	TXREQ_ENCAP_HT                  
};


#define IS_TXREQ_WITH_ADDR4(x)          ((x)->txhdr_mask & (1<<TXREQ_ENCAP_ADDR4) )
#define IS_TXREQ_WITH_QOS(x)            ((x)->txhdr_mask & (1<<TXREQ_ENCAP_QOS)   )
#define IS_TXREQ_WITH_HT(x)             ((x)->txhdr_mask & (1<<TXREQ_ENCAP_HT)    )

#define SET_TXREQ_WITH_ADDR4(x, _tf)    (x)->txhdr_mask = (((x)->txhdr_mask & ~(1<<TXREQ_ENCAP_ADDR4))	| ((_tf)<<TXREQ_ENCAP_ADDR4) )
#define SET_TXREQ_WITH_QOS(x, _tf)      (x)->txhdr_mask = (((x)->txhdr_mask & ~(1<<TXREQ_ENCAP_QOS))	| ((_tf)<<TXREQ_ENCAP_QOS)   )
#define SET_TXREQ_WITH_HT(x, _tf)       (x)->txhdr_mask = (((x)->txhdr_mask & ~(1<<TXREQ_ENCAP_HT))		| ((_tf)<<TXREQ_ENCAP_HT)    )



typedef enum txreq_type_en {
	TX_TYPE_M0 = 0,
	TX_TYPE_M1,
	TX_TYPE_M2
} txreq_type;


#define HOST_RECOVERY_CB_NUM  1
#define HOST_DATA_CB_NUM  2
#define HOST_EVT_CB_NUM  4
#define PRMOISCUOUS_CB_NUM  1

typedef enum {
	SSV6XXX_CB_ADD		,
	SSV6XXX_CB_REMOVE	,
	SSV6XXX_CB_MOD		
} ssv6xxx_cb_action;

typedef enum vif_state_en {
    VIF_ALL_DEACTIVE = 0,
    VIF_ACTIVE_STA,
    VIF_ACTIVE_AP
} vif_state;

typedef struct StaInfo{
    struct cfg_join_request *joincfg; // Station mode used
    struct cfg_join_request *joincfg_backup; // Station mode used
    enum conn_status    status;                 //Auth,Assoc,Eapol
#if(SW_8023TO80211==1)
    ssv_type_u16 seq_ctl[8]; //Station mode used
#endif
    void* vif;
    ssv_type_u8 wsid;    
    ssv_type_u16 rssi;
}StaInfo_st;

typedef struct ssv_vif_st {
    ssv6xxx_hw_mode hw_mode;
    ssv_type_u8 self_mac[ETH_ALEN];
    ssv_type_u8 idx;
    struct freq_params freq;
    union{
        struct ApInfo *APInfo; // AP mode used
        struct StaInfo *StaInfo; //STA mode used
    }m_info;
}ssv_vif;

typedef struct DeviceInfo{

    OsMutex g_dev_info_mutex;
	txreq_type tx_type;
	ssv_type_u32 txhdr_mask;
	ETHER_ADDR addr4;
	ssv_type_u16 qos_ctrl;
	ssv_type_u32 ht_ctrl;
    struct ApInfo *APInfo; // AP mode used
    //struct StaInfo *StaInfo[MAX_STA_VIF]; //STA mode used        

    ssv_vif vif[MAX_VIF_NUM];
    ssv_type_u8 used_vif;
    ssv_type_u8 ampdurx_session_cnt;
    //Set key base on VIF.

    #if(ENABLE_DYNAMIC_RX_SENSITIVE==1)
    ssv_type_u16 cci_current_level; //Station mode used
    ssv_type_u16 cci_current_gate; //Station mode used
    #endif
    struct ssv6xxx_ieee80211_bss *ap_list; //station ap info list used (useing "g_dev_info_mutex" to protect it)
    char  alpha2[3];  //country code
    ssv_type_u16 available_2g_channel_mask; //The available channel in the regulatory domain, not hw capability
    ssv_type_u32 available_5g_channel_mask; //The available channel in the regulatory domain, not hw capability    
//-----------------------------
//Data path handler
//-----------------------------
	data_handler data_cb[HOST_DATA_CB_NUM];

//-----------------------------
//Event path handler
//-----------------------------
	evt_handler evt_cb[HOST_EVT_CB_NUM];
    void* evt_cb_priv[HOST_EVT_CB_NUM]; //private data

//-----------------------------
// promiscuous call back function
//-----------------------------
    promiscuous_data_handler promiscuous_cb[PRMOISCUOUS_CB_NUM];

//-----------------------------
// promiscuous call back function
//-----------------------------
    recovery_handler recovery_cb[HOST_RECOVERY_CB_NUM];

//-----------------------------
// fw recovery
//-----------------------------
    ssv_type_s32 reload_fw_cnt;
    ssv_type_bool recovering; //TRUE: in recovering

//timer interrupt count
    ssv_type_u32 fw_timer_interrupt;
    ssv_type_bool bMCC;
    cali_result_info CliRes_info;

}DeviceInfo_st;

extern DeviceInfo_st *gDeviceInfo;

struct rcs_info
{
    ssv_type_u16 free_page;
    ssv_type_u16 free_id;
    ssv_type_u16 free_space; //how many space of HCI input queue for incoming packets
    ssv_type_u16 edca_fifo_cnt[4];
};

struct vif_tx_rcs
{
    ssv_type_u16 free_page;
};

extern struct rcs_info tx_rcs;
extern OsMutex txsrcMutex;

struct vif_flow_ctrl
{
    ssv_type_u32 retry_cnt:16;
    ssv_type_u32 sleep_tick:16;
    ssv_type_u32 priority:1;
    ssv_type_u32 RSVD:31;
};

struct txduty_cfg_st{
    ssv_type_u32 qidx:3;
    ssv_type_u32 txtime:8;
    ssv_type_u32 qktime:8;
    ssv_type_u32 halt:1;
    ssv_type_u32 mode:1;
    ssv_type_u32 RSVD:11;
};
typedef struct txduty_cfg_st txduty_cfg;

struct Host_cfg {     
    //1st 
    ssv_type_u32 upgrade_per:8;
    ssv_type_u32 downgrade_per:8;
    ssv_type_u32 rate_mask:16;

    //2nd 
    ssv_type_u32 rsvd0:8;
    ssv_type_u32 resent_fail_report:1;
    ssv_type_u32 pre_alloc_prb_frm:1;
    ssv_type_u32 direct_rc_down:1;
    ssv_type_u32 tx_power_mode:2;
    ssv_type_u32 ampdu_tx_enable:1;
    ssv_type_u32 ampdu_rx_enable:1;
    ssv_type_u32 ampdu_rx_buf_size:8;
    ssv_type_u32 force_RTSCTS:1;
    ssv_type_u32 bcn_interval:8;

    //3rd 
    ssv_type_u32 pool_size:16;
    ssv_type_u32 recv_buf_size:16;

    //4th
    ssv_type_u32 trx_hdr_len:16;
    ssv_type_u32 tx_res_page:8;
    ssv_type_u32 rx_res_page:8;

    //5th
    ssv_type_u32 ap_rx_support_legacy_rate_msk:12; 
    ssv_type_u32 ap_rx_support_mcs_rate_msk:8;    
    ssv_type_u32 ap_rx_short_GI:1;
    ssv_type_u32 erp:1;
    ssv_type_u32 b_short_preamble:1;
    ssv_type_u32 tx_retry_cnt:8;
    ssv_type_u32 tx_sleep:1;
    
    //6th
    ssv_type_u32 rc_drate_endian:1;
    ssv_type_u32 pool_sec_size:16;
    ssv_type_u32 volt_mode:1;
    ssv_type_u32 tx_sleep_tick:8;
    ssv_type_u32 support_ht:1;
    ssv_type_u32 support_tx_SGI:1;
    ssv_type_u32 support_rf_band:1;
    ssv_type_u32 ap_no_dfs:1;
    ssv_type_u32 usePA:1;
    ssv_type_u32 rsvd1:1;    

    //7th
    txduty_cfg txduty;

    //8th
    ssv_type_u32 sta_no_bcn_timeout:8;
    ssv_type_u32 hci_rx_aggr:1;
    ssv_type_u32 hci_aggr_tx:1;
    ssv_type_u32 rxIntGPIO:8;
    ssv_type_u32 extRxInt:1;
    ssv_type_u32 AP_TimAllOne:1;
    ssv_type_u32 ApStaInactiveTime:8;
    ssv_type_u32 ap_supported_sta_num:4;

    //9th	
    ssv_type_u32 xtal:8;
    ssv_type_u32 krack_patch:1;
    ssv_type_u32 DoDPD:1;   
    ssv_type_u32 sw_q_force_unlock_time:8; //unit :10ms
    ssv_type_u32 info_scan_result:1;
    ssv_type_u32 ap_bw:2;    
    ssv_type_u32 rsvd2:11;
    
};
//AMPDU RX

#define RX_AGG_RX_BA_MAX_STATION				AMPDU_RX_MAX_STATION
#define RX_AGG_RX_BA_MAX_SESSIONS				1
#define HT_RX_REORDER_BUF_TIMEOUT               10//ms

/**
 * struct tid_ampdu_rx - TID aggregation information (Rx).
 *
 * @reorder_buf: buffer to reorder incoming aggregated MPDUs
 * @reorder_time: jiffies when skb was added
 * @session_timer: check if peer keeps Tx-ing on the TID (by timeout value)
 * @reorder_timer: releases expired frames from the reorder buffer.
 * @head_seq_num: head sequence number in reordering buffer.
 * @stored_mpdu_num: number of MPDUs in reordering buffer
 * @ssn: Starting Sequence Number expected to be aggregated.
 * @buf_size: buffer size for incoming A-MPDUs
 * @timeout: reset timer value (in TUs).
 * @dialog_token: dialog token for aggregation session
 * @rcu_head: RCU head used for freeing this struct
 * @reorder_lock: serializes access to reorder buffer, see below.
 *
 * This structure's lifetime is managed by RCU, assignments to
 * the array holding it must hold the aggregation mutex.
 *
 * The @reorder_lock is used to protect the members of this
 * struct, except for @timeout, @buf_size and @dialog_token,
 * which are constant across the lifetime of the struct (the
 * dialog token being used only for debugging).
 */
struct rx_ba_session_desc {

	OsMutex reorder_lock;
	void **reorder_buf;//[RX_AGG_RX_BA_MAX_BUF_SIZE];
	unsigned long *reorder_time;//[RX_AGG_RX_BA_MAX_BUF_SIZE];
	ssv_type_u16 head_seq_num;
	ssv_type_u16 stored_mpdu_num;
	ssv_type_u16 ssn;
	ssv_type_u16 buf_size;
};

#endif /* _SSV_DEVINFO_H_ */

