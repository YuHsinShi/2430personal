/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef AP_STA_INFO_H
#define AP_STA_INFO_H

#ifndef CONFIG_NO_WPA2
#include <wpa_auth_i.h>
#endif
#include <cmd_def.h>
#include <pbuf.h>
#include <ssv_ether.h>
#include <ssv_ether.h>
#include "ap_tx.h"
#include "common/ieee802_11_defs.h"


#define AP_PS_FRAME 			BIT(0)		//This frame was buffered by AP.
//-------------------------------------------------------------------------------------------
void ap_handle_timer(void *data1, void *data2);

/**
 * enum ieee80211_max_queues - maximum number of queues
 *
 * @IEEE80211_MAX_QUEUES: Maximum number of regular device queues.
 */
enum ieee80211_max_queues {
	IEEE80211_MAX_QUEUES =		4
};

/**
 * enum ieee80211_ac_numbers - AC numbers as used in mac80211
 * @IEEE80211_AC_VO: voice
 * @IEEE80211_AC_VI: video
 * @IEEE80211_AC_BE: best effort
 * @IEEE80211_AC_BK: background
 */
enum ieee80211_ac_numbers {
	IEEE80211_AC_VO		= 0,
	IEEE80211_AC_VI		= 1,
	IEEE80211_AC_BE		= 2,
	IEEE80211_AC_BK		= 3
};
#define IEEE80211_NUM_ACS	4
//-------------------------------------------------------------------------------------------


/**
 * enum ieee80211_sta_info_flags - Stations flags
 *
 * These flags are used with &struct sta_info's @flags member, but
 * only indirectly with set_sta_flag() and friends.
 *
 * @WLAN_STA_VALID      if flag is set. This station is activate.
 * @WLAN_STA_AUTH: Station is authenticated.
 * @WLAN_STA_ASSOC: Station is associated.
 * @WLAN_STA_PS_STA: Station is in power-save mode
 * @WLAN_STA_AUTHORIZED: Station is authorized to send/receive traffic.
 *	This bit is always checked so needs to be enabled for all stations
 *	when virtual port control is not in use.
 * @WLAN_STA_SHORT_PREAMBLE: Station is capable of receiving short-preamble
 *	frames.
 * @WLAN_STA_WMM: Station is a QoS-STA.
 * @WLAN_STA_WDS: Station is one of our WDS peers.
 * @WLAN_STA_CLEAR_PS_FILT: Clear PS filter in hardware (using the
 *	IEEE80211_TX_CTL_CLEAR_PS_FILT control flag) when the next
 *	frame to this station is transmitted.
 * @WLAN_STA_MFP: Management frame protection is used with this STA.

 * @WLAN_STA_BLOCK_BA: Used to deny ADDBA requests (both TX and RX)
 *	during suspend/resume and station removal.
 * @WLAN_STA_PS_DRIVER: driver requires keeping this station in
 *	power-save mode logically to flush frames that might still
 *	be in the queues
 * @WLAN_STA_PSPOLL: Station sent PS-poll while driver was keeping
 *	station in power-save mode, reply when the driver unblocks.
 * @WLAN_STA_TDLS_PEER: Station is a TDLS peer.
 * @WLAN_STA_TDLS_PEER_AUTH: This TDLS peer is authorized to send direct
 *	packets. This means the link is enabled.
 * @WLAN_STA_UAPSD: Station requested unscheduled SP while driver was
 *	keeping station in power-save mode, reply when the driver
 *	unblocks the station.
 * @WLAN_STA_SP: Station is in a service period, so don't try to
 *	reply to other uAPSD trigger frames or PS-Poll.
 * @WLAN_STA_4ADDR_EVENT: 4-addr event was already sent for this frame.
 * @WLAN_STA_RATE_CONTROL: rate control was initialized for this station.
 */

/* STA flags */
//enum ap_sta_info_flags {
#define WLAN_STA_VALID			 											BIT(0)											//,
#define WLAN_STA_AUTH			 											BIT(1)											//,
#define WLAN_STA_ASSOC			 											BIT(2)											//,
//#define WLAN_STA_PS			 												BIT(3)											//,
//#define WLAN_STA_TIM			 												BIT(4)											//,
//#define WLAN_STA_PERM			 											BIT(5)											//,
#define WLAN_STA_AUTHORIZED	 										BIT(6)											//,
#define WLAN_STA_PENDING_POLL	 									BIT(7)											//, 	statation no activities sned and a frame to station/*pending activity poll not ACKed */
#define WLAN_STA_SHORT_PREAMBLE 									BIT(8)											//,
//#define WLAN_STA_PREAUTH		 											BIT(9)											//,
#define WLAN_STA_WMM			 												BIT(10)											//,
//#define WLAN_STA_MFP			 												BIT(11)// Management Frame Port
#define WLAN_STA_HT			 												BIT(12)											//,
//#define WLAN_STA_WPS			 												BIT(13)											//,
//#define WLAN_STA_MAYBE_WPS		 										BIT(14)											//,
#define WLAN_STA_WDS			 												BIT(15)											//,
#define WLAN_STA_NONERP		 											BIT(16)											//,
#define WLAN_STA_PS_STA													BIT(17)											//,
//#define WLAN_STA_WMM													WLAN_STA_WMM											//,
//WLAN_STA_WDS														BIT()											//,
//#define WLAN_STA_CLEAR_PS_FILT										BIT(19)											//,
//WLAN_STA_MFP														BIT()											//,
//#define WLAN_STA_BLOCK_BA													BIT(20)											//,
//#define WLAN_STA_PS_DRIVER												BIT(21)											//,
#define WLAN_STA_PSPOLL														BIT(22)											//,
//WLAN_STA_TDLS_PEER											BIT()											//,
//WLAN_STA_TDLS_PEER_AUTH									BIT()											//,
//#define WLAN_STA_UAPSD														BIT(23)											//,
#define WLAN_STA_SP																BIT(24)											//,
//WLAN_STA_4ADDR_EVENT											BIT(25)											//,
//WLAN_STA_RATE_CONTROL										BIT()											//,					//XX
                                    			//BIT()											//
#define WLAN_STA_ASSOC_REQ_OK											BIT(25)											//,
//#define WLAN_STA_WPS2															BIT(26)											//,
//WLAN_STA_PENDING_DISASSOC_CB						BIT()											//,
//WLAN_STA_PENDING_DEAUTH_CB							BIT()											//,


//};


/* Maximum number of supported rates (from both Supported Rates and Extended
 * Supported Rates IEs). */
#define WLAN_SUPP_RATES_MAX 32
enum timeout_next_e{
		STA_NULLFUNC = 0, STA_DISASSOC, STA_DEAUTH, STA_REMOVE
	};

#define UNKNOWN_FW_QUEUE_LENGTH -9999
typedef struct APStaInfo{

    //mutex for apsta parameter
    OsMutex       apsta_mutex;

    ssv_type_u8 addr[ETH_ALEN];			//Mac Adderss ETHER_ADDR
	ssv_type_u16 aid; /* STA's unique AID (1 .. 2007) or 0 if not yet assigned */

	ssv_type_u32 _flags; /* Bitfield of WLAN_STA_* */

	ssv_type_u16 capability;
	ssv_type_u16 listen_interval; /* or beacon_int for APs */
	ssv_type_u8 supported_rates[WLAN_SUPP_RATES_MAX];
	int supported_rates_len;


	unsigned int nonerp_set:1;
	unsigned int no_short_slot_time_set:1;
	unsigned int no_short_preamble_set:1;
	unsigned int no_ht_gf_set:1;
	unsigned int no_ht_set:1;
	unsigned int ht_20mhz_set:1;

	ssv_type_u16 auth_alg;
	ssv_type_u8 previous_ap[ETH_ALEN];

	enum timeout_next_e timeout_next;

	/* IEEE 802.1X related data */
	//struct eapol_state_machine *eapol_sm;

	/* IEEE 802.11f (IAPP) related data */
	//struct ieee80211_mgmt *last_assoc_req;

	//u32 acct_session_id_hi;
	//u32 acct_session_id_lo;
	//time_t acct_session_start;
	//int acct_session_started;
	//int acct_terminate_cause; /* Acct-Terminate-Cause */
	//int acct_interim_interval; /* Acct-Interim-Interval */

	//unsigned long last_rx_bytes;
	//unsigned long last_tx_bytes;

	//u32 acct_input_gigawords; /* Acct-Input-Gigawords */
	//u32 acct_output_gigawords; /* Acct-Output-Gigawords */

	ssv_type_u8 *challenge; /* IEEE 802.11 Shared Key Authentication Challenge */

//	struct wpa_state_machine *wpa_sm;
//	struct rsn_preauth_interface *preauth_iface;

	//struct hostapd_ssid *ssid; /* SSID selection based on (Re)AssocReq */
	//struct hostapd_ssid *ssid_probe; /* SSID selection based on ProbeReq */

	//int vlan_id;

	//struct ieee80211_ht_capabilities *ht_capabilities;

#ifdef CONFIG_IEEE80211W
	int sa_query_count; /* number of pending SA Query requests;
			     * 0 = no SA Query in progress */
	int sa_query_timed_out;
	ssv_type_u8 *sa_query_trans_id; /* buffer of WLAN_SA_QUERY_TR_ID_LEN *
				* sa_query_count octets of pending SA Query
				* transaction identifiers */
	struct os_time sa_query_start;
#endif /* CONFIG_IEEE80211W */

//	struct wpabuf *wps_ie; /* WPS IE from (Re)Association Request */


//---------------------------------------------
	/* Updated from RX path only, no locking requirements */
	unsigned long last_rx; //last received time(jiffies)



//---------------------------------------------
	/*
	 * STA powersave frame queues, no more than the internal
	 * locking required.PKT_RxInfo *pPktInfo
	 */
	//struct ap_tx_desp_head ps_tx_buf[IEEE80211_NUM_ACS];
    struct ssv_list_q ps_tx_buf[IEEE80211_NUM_ACS];
    OsMutex       ps_tx_buf_mutex;


	//struct ap_tx_desp_head tx_filtered[IEEE80211_NUM_ACS];
	//unsigned long driver_buffered_tids;

//WMM
	//u32 supp_rates[IEEE80211_NUM_BANDS];
	//u8 addr[ETH_ALEN];
	//u16 aid;
	//struct ieee80211_sta_ht_cap ht_cap;	//802.11n
	//ssv_type_bool wme;							//for power saving (APSD)
	ssv_type_u8 uapsd_queues;					//
	ssv_type_u8 max_sp;							//

//ARP retry count

    ssv_type_u8 arp_retry_count;
//record RCPI value
    ssv_type_u8 rcpi;
    ssv_type_u32 prev_rcpi;

//record FW queue length
    ssv_type_s32 fw_q_len;
    ssv_type_u32 wsid_idx;
//recode reassociate;
#ifndef CONFIG_NO_WPA2
    //struct wpa_state_machine stwpa_sm;
    struct wpa_state_machine wpa_sm;

    ssv_type_u8 reassoc;
    ssv_type_u8 addStaFlag;
    ssv_type_u8 assocRsp;
#endif
//---------------------------------------------
/* use the accessors defined below */
	//unsigned long _flags;
#if(SW_8023TO80211==1)
    ssv_type_u16 seq_ctl[8]; //AP mode used
#endif
    struct ieee80211_ht_capabilities ht_capabilities;


}APStaInfo_st;





static ssv_inline void set_sta_flag(APStaInfo_st *sta,
				ssv_type_u32 flag)
{
	//set_bit(flag, &sta->_flags);
	sta->_flags |= flag;

}

static ssv_inline void clear_sta_flag(APStaInfo_st *sta,
				  ssv_type_u32 flag)
{
	//clear_bit(flag, &sta->_flags);
	sta->_flags &= ~(flag);

}



static ssv_inline int test_sta_flag(APStaInfo_st *sta,
				ssv_type_u32 flag)
{
	return !!(sta->_flags&flag);
}







#if 0
static inline int test_and_clear_sta_flag(struct sta_info *sta,
					  enum ap_sta_info_flags flag)
{
	return test_and_clear_bit(flag, &sta->_flags);
}

static inline int test_and_set_sta_flag(struct sta_info *sta,
					enum ap_sta_info_flags flag)
{
	return test_and_set_bit(flag, &sta->_flags);
}
#endif




struct ApInfo;






void APStaInfo_Init(void);
APStaInfo_st *APStaInfo_FindStaByAddr( ETHER_ADDR *mac );
APStaInfo_st *APStaInfo_FindStaByWsid( ssv_type_u8 target_wsid );
APStaInfo_st *APStaInfo_add(struct ApInfo *pApInfo, const ssv_type_u8 *addr);

void APStaInfo_DrvRemove(struct ApInfo *pApInfo, APStaInfo_st *sta);

void recalc_tim_gen_beacon(APStaInfo_st * sta);

void GenBeacon(void);

void ap_sta_set_authorized(struct ApInfo *pApInfo, APStaInfo_st *sta, int authorized);

ssv_type_bool ap_sta_info_capability(ETHER_ADDR *mac , ssv_type_bool *ht, ssv_type_bool *qos, ssv_type_bool *wds);

//---------------------------------------------------

//PS-POLL/UAPSD

enum frame_release_type {
	FRAME_RELEASE_PSPOLL,
	FRAME_RELEASE_UAPSD
};

void sta_ps_deliver_poll_response(APStaInfo_st *sta);
void sta_ps_deliver_uapsd(APStaInfo_st *sta);




#endif /* AP_STA_INFO_H */
