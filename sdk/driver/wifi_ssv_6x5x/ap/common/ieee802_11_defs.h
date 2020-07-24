/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef IEEE802_11_DEFS_H
#define IEEE802_11_DEFS_H


#include <ssv_ether.h>
#include "porting.h"

#define le_to_host16(n) (n)
#define host_to_le16(n) (n)
#define be_to_host16(n) wpa_swap_16(n)
#define host_to_be16(n) wpa_swap_16(n)
#define le_to_host32(n) (n)
#define be_to_host32(n) wpa_swap_32(n)
#define host_to_be32(n) wpa_swap_32(n)


#define AP_GET_BE16(a) ((ssv_type_u16) (((a)[0] << 8) | (a)[1]))
#define AP_PUT_BE16(a, val)			\
	do {					\
	(a)[0] = ((ssv_type_u16) (val)) >> 8;	\
	(a)[1] = ((ssv_type_u16) (val)) & 0xff;	\
	} while (0)

#define AP_GET_LE16(a) ((ssv_type_u16) (((a)[1] << 8) | (a)[0]))
#define AP_PUT_LE16(a, val)			\
	do {					\
	(a)[1] = ((ssv_type_u16) (val)) >> 8;	\
	(a)[0] = ((ssv_type_u16) (val)) & 0xff;	\
	} while (0)

#define AP_GET_BE24(a) ((((ssv_type_u32) (a)[0]) << 16) | (((ssv_type_u32) (a)[1]) << 8) | \
	((ssv_type_u32) (a)[2]))
#define AP_PUT_BE24(a, val)					\
	do {							\
	(a)[0] = (ssv_type_u8) ((((ssv_type_u32) (val)) >> 16) & 0xff);	\
	(a)[1] = (ssv_type_u8) ((((ssv_type_u32) (val)) >> 8) & 0xff);	\
	(a)[2] = (ssv_type_u8) (((ssv_type_u32) (val)) & 0xff);		\
	} while (0)

#define AP_GET_BE32(a) ((((ssv_type_u32) (a)[0]) << 24) | (((ssv_type_u32) (a)[1]) << 16) | \
	(((ssv_type_u32) (a)[2]) << 8) | ((ssv_type_u32) (a)[3]))
#define AP_PUT_BE32(a, val)					\
	do {							\
	(a)[0] = (ssv_type_u8) ((((ssv_type_u32) (val)) >> 24) & 0xff);	\
	(a)[1] = (ssv_type_u8) ((((ssv_type_u32) (val)) >> 16) & 0xff);	\
	(a)[2] = (ssv_type_u8) ((((ssv_type_u32) (val)) >> 8) & 0xff);	\
	(a)[3] = (ssv_type_u8) (((ssv_type_u32) (val)) & 0xff);		\
	} while (0)

#define AP_GET_LE32(a) ((((ssv_type_u32) (a)[3]) << 24) | (((ssv_type_u32) (a)[2]) << 16) | \
	(((ssv_type_u32) (a)[1]) << 8) | ((ssv_type_u32) (a)[0]))
#define AP_PUT_LE32(a, val)					\
	do {							\
	(a)[3] = (ssv_type_u8) ((((ssv_type_u32) (val)) >> 24) & 0xff);	\
	(a)[2] = (ssv_type_u8) ((((ssv_type_u32) (val)) >> 16) & 0xff);	\
	(a)[1] = (ssv_type_u8) ((((ssv_type_u32) (val)) >> 8) & 0xff);	\
	(a)[0] = (ssv_type_u8) (((ssv_type_u32) (val)) & 0xff);		\
	} while (0)

#define AP_GET_BE64(a) ((((ssv_type_u64) (a)[0]) << 56) | (((ssv_type_u64) (a)[1]) << 48) | \
	(((ssv_type_u64) (a)[2]) << 40) | (((ssv_type_u64) (a)[3]) << 32) | \
	(((ssv_type_u64) (a)[4]) << 24) | (((ssv_type_u64) (a)[5]) << 16) | \
	(((ssv_type_u64) (a)[6]) << 8) | ((ssv_type_u64) (a)[7]))
#define AP_PUT_BE64(a, val)				\
	do {						\
	(a)[0] = (ssv_type_u8) (((ssv_type_u64) (val)) >> 56);	\
	(a)[1] = (ssv_type_u8) (((ssv_type_u64) (val)) >> 48);	\
	(a)[2] = (ssv_type_u8) (((ssv_type_u64) (val)) >> 40);	\
	(a)[3] = (ssv_type_u8) (((ssv_type_u64) (val)) >> 32);	\
	(a)[4] = (ssv_type_u8) (((ssv_type_u64) (val)) >> 24);	\
	(a)[5] = (ssv_type_u8) (((ssv_type_u64) (val)) >> 16);	\
	(a)[6] = (ssv_type_u8) (((ssv_type_u64) (val)) >> 8);	\
	(a)[7] = (ssv_type_u8) (((ssv_type_u64) (val)) & 0xff);	\
	} while (0)

#define AP_GET_LE64(a) ((((ssv_type_u64) (a)[7]) << 56) | (((ssv_type_u64) (a)[6]) << 48) | \
	(((ssv_type_u64) (a)[5]) << 40) | (((ssv_type_u64) (a)[4]) << 32) | \
	(((ssv_type_u64) (a)[3]) << 24) | (((ssv_type_u64) (a)[2]) << 16) | \
	(((ssv_type_u64) (a)[1]) << 8) | ((ssv_type_u64) (a)[0]))



#ifdef _MSC_VER
#pragma pack(push, 1)
#endif /* _MSC_VER */




/* IEEE 802.11 defines */

#define WLAN_FC_PVER		0x0003
#define WLAN_FC_TODS		0x0100
#define WLAN_FC_FROMDS		0x0200
#define WLAN_FC_MOREFRAG	0x0400
#define WLAN_FC_RETRY		0x0800
#define WLAN_FC_PWRMGT		0x1000
#define WLAN_FC_MOREDATA	0x2000
#define WLAN_FC_ISWEP		0x4000
#define WLAN_FC_ORDER		0x8000

#define WLAN_FC_GET_TYPE(fc)	(((fc) & 0x000c) >> 2)
#define WLAN_FC_GET_STYPE(fc)	(((fc) & 0x00f0) >> 4)

#define WLAN_GET_SEQ_FRAG(seq) ((seq) & (BIT(3) | BIT(2) | BIT(1) | BIT(0)))
#define WLAN_GET_SEQ_SEQ(seq) \
	(((seq) & (~(BIT(3) | BIT(2) | BIT(1) | BIT(0)))) >> 4)

#define WLAN_FC_TYPE_MGMT		0
#define WLAN_FC_TYPE_CTRL		1
#define WLAN_FC_TYPE_DATA		2

/* management */
#define WLAN_FC_STYPE_ASSOC_REQ		0
#define WLAN_FC_STYPE_ASSOC_RESP	1
#define WLAN_FC_STYPE_REASSOC_REQ	2
#define WLAN_FC_STYPE_REASSOC_RESP	3
#define WLAN_FC_STYPE_PROBE_REQ		4
#define WLAN_FC_STYPE_PROBE_RESP	5
#define WLAN_FC_STYPE_BEACON		8
#define WLAN_FC_STYPE_ATIM		9
#define WLAN_FC_STYPE_DISASSOC		10
#define WLAN_FC_STYPE_AUTH		11
#define WLAN_FC_STYPE_DEAUTH		12
#define WLAN_FC_STYPE_ACTION		13

/* control */
#define WLAN_FC_STYPE_PSPOLL		10
#define WLAN_FC_STYPE_RTS		11
#define WLAN_FC_STYPE_CTS		12
#define WLAN_FC_STYPE_ACK		13
#define WLAN_FC_STYPE_CFEND		14
#define WLAN_FC_STYPE_CFENDACK		15

/* data */
#define WLAN_FC_STYPE_DATA		0
#define WLAN_FC_STYPE_DATA_CFACK	1
#define WLAN_FC_STYPE_DATA_CFPOLL	2
#define WLAN_FC_STYPE_DATA_CFACKPOLL	3
#define WLAN_FC_STYPE_NULLFUNC		4
#define WLAN_FC_STYPE_CFACK		5
#define WLAN_FC_STYPE_CFPOLL		6
#define WLAN_FC_STYPE_CFACKPOLL		7
#define WLAN_FC_STYPE_QOS_DATA		8

#define WLAN_FC_STYPE_QOS_NULL		12



/* Authentication algorithms */
#define WLAN_AUTH_OPEN			0
#define WLAN_AUTH_SHARED_KEY		1
#define WLAN_AUTH_FT			2
#define WLAN_AUTH_LEAP			128



// #define WLAN_CAPABILITY_ESS BIT(0)
// #define WLAN_CAPABILITY_IBSS BIT(1)
// #define WLAN_CAPABILITY_CF_POLLABLE BIT(2)
// #define WLAN_CAPABILITY_CF_POLL_REQUEST BIT(3)
// #define WLAN_CAPABILITY_PRIVACY BIT(4)
// #define WLAN_CAPABILITY_SHORT_PREAMBLE BIT(5)
// #define WLAN_CAPABILITY_PBCC BIT(6)
// #define WLAN_CAPABILITY_CHANNEL_AGILITY BIT(7)
// #define WLAN_CAPABILITY_SPECTRUM_MGMT BIT(8)
// #define WLAN_CAPABILITY_SHORT_SLOT_TIME BIT(10)
// #define WLAN_CAPABILITY_DSSS_OFDM BIT(13)

/* Status codes (IEEE 802.11-2007, 7.3.1.9, Table 7-23) */
// #define WLAN_STATUS_SUCCESS 0
// #define WLAN_STATUS_UNSPECIFIED_FAILURE 1
// #define WLAN_STATUS_CAPS_UNSUPPORTED 10
// #define WLAN_STATUS_REASSOC_NO_ASSOC 11
// #define WLAN_STATUS_ASSOC_DENIED_UNSPEC 12
// #define WLAN_STATUS_NOT_SUPPORTED_AUTH_ALG 13
// #define WLAN_STATUS_UNKNOWN_AUTH_TRANSACTION 14
// #define WLAN_STATUS_CHALLENGE_FAIL 15
// #define WLAN_STATUS_AUTH_TIMEOUT 16
// #define WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA 17
// #define WLAN_STATUS_ASSOC_DENIED_RATES 18
// /* IEEE 802.11b */
// #define WLAN_STATUS_ASSOC_DENIED_NOSHORT 19
// #define WLAN_STATUS_ASSOC_DENIED_NOPBCC 20
// #define WLAN_STATUS_ASSOC_DENIED_NOAGILITY 21
// /* IEEE 802.11h */
// #define WLAN_STATUS_SPEC_MGMT_REQUIRED 22
// #define WLAN_STATUS_PWR_CAPABILITY_NOT_VALID 23
// #define WLAN_STATUS_SUPPORTED_CHANNEL_NOT_VALID 24
// /* IEEE 802.11g */
// #define WLAN_STATUS_ASSOC_DENIED_NO_SHORT_SLOT_TIME 25
// #define WLAN_STATUS_ASSOC_DENIED_NO_ER_PBCC 26
// #define WLAN_STATUS_ASSOC_DENIED_NO_DSSS_OFDM 27
// #define WLAN_STATUS_R0KH_UNREACHABLE 28
// /* IEEE 802.11w */
// #define WLAN_STATUS_ASSOC_REJECTED_TEMPORARILY 30
// #define WLAN_STATUS_ROBUST_MGMT_FRAME_POLICY_VIOLATION 31
// #define WLAN_STATUS_UNSPECIFIED_QOS_FAILURE 32
// #define WLAN_STATUS_REQUEST_DECLINED 37
// #define WLAN_STATUS_INVALID_PARAMETERS 38
// /* IEEE 802.11i */
// #define WLAN_STATUS_INVALID_IE 40
// #define WLAN_STATUS_GROUP_CIPHER_NOT_VALID 41
// #define WLAN_STATUS_PAIRWISE_CIPHER_NOT_VALID 42
// #define WLAN_STATUS_AKMP_NOT_VALID 43
// #define WLAN_STATUS_UNSUPPORTED_RSN_IE_VERSION 44
// #define WLAN_STATUS_INVALID_RSN_IE_CAPAB 45
// #define WLAN_STATUS_CIPHER_REJECTED_PER_POLICY 46
// #define WLAN_STATUS_TS_NOT_CREATED 47
// #define WLAN_STATUS_DIRECT_LINK_NOT_ALLOWED 48
// #define WLAN_STATUS_DEST_STA_NOT_PRESENT 49
// #define WLAN_STATUS_DEST_STA_NOT_QOS_STA 50
// #define WLAN_STATUS_ASSOC_DENIED_LISTEN_INT_TOO_LARGE 51
// /* IEEE 802.11r */
// #define WLAN_STATUS_INVALID_FT_ACTION_FRAME_COUNT 52
// #define WLAN_STATUS_INVALID_PMKID 53
// #define WLAN_STATUS_INVALID_MDIE 54
// #define WLAN_STATUS_INVALID_FTIE 55

/* Reason codes (IEEE 802.11-2007, 7.3.1.7, Table 7-22) */
// #define WLAN_REASON_UNSPECIFIED 1
// #define WLAN_REASON_PREV_AUTH_NOT_VALID 2
// #define WLAN_REASON_DEAUTH_LEAVING 3
// #define WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY 4
// #define WLAN_REASON_DISASSOC_AP_BUSY 5
// #define WLAN_REASON_CLASS2_FRAME_FROM_NONAUTH_STA 6
// #define WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA 7
// #define WLAN_REASON_DISASSOC_STA_HAS_LEFT 8
// #define WLAN_REASON_STA_REQ_ASSOC_WITHOUT_AUTH 9
// /* IEEE 802.11h */
// #define WLAN_REASON_PWR_CAPABILITY_NOT_VALID 10
// #define WLAN_REASON_SUPPORTED_CHANNEL_NOT_VALID 11
// /* IEEE 802.11i */
// #define WLAN_REASON_INVALID_IE 13
// #define WLAN_REASON_MICHAEL_MIC_FAILURE 14
// #define WLAN_REASON_4WAY_HANDSHAKE_TIMEOUT 15
// #define WLAN_REASON_GROUP_KEY_UPDATE_TIMEOUT 16
// #define WLAN_REASON_IE_IN_4WAY_DIFFERS 17
// #define WLAN_REASON_GROUP_CIPHER_NOT_VALID 18
// #define WLAN_REASON_PAIRWISE_CIPHER_NOT_VALID 19
// #define WLAN_REASON_AKMP_NOT_VALID 20
// #define WLAN_REASON_UNSUPPORTED_RSN_IE_VERSION 21
// #define WLAN_REASON_INVALID_RSN_IE_CAPAB 22
// #define WLAN_REASON_IEEE_802_1X_AUTH_FAILED 23
// #define WLAN_REASON_CIPHER_SUITE_REJECTED 24


/* Information Element IDs */
// #define WLAN_EID_SSID 0
// #define WLAN_EID_SUPP_RATES 1
// #define WLAN_EID_FH_PARAMS 2
// #define WLAN_EID_DS_PARAMS 3
// #define WLAN_EID_CF_PARAMS 4
// #define WLAN_EID_TIM 5
// #define WLAN_EID_IBSS_PARAMS 6
// #define WLAN_EID_COUNTRY 7
// #define WLAN_EID_CHALLENGE 16
// /* EIDs defined by IEEE 802.11h - START */
// #define WLAN_EID_PWR_CONSTRAINT 32
// #define WLAN_EID_PWR_CAPABILITY 33
// #define WLAN_EID_TPC_REQUEST 34
// #define WLAN_EID_TPC_REPORT 35
// #define WLAN_EID_SUPPORTED_CHANNELS 36
// #define WLAN_EID_CHANNEL_SWITCH 37
// #define WLAN_EID_MEASURE_REQUEST 38
// #define WLAN_EID_MEASURE_REPORT 39
// #define WLAN_EID_QUITE 40
// #define WLAN_EID_IBSS_DFS 41
// /* EIDs defined by IEEE 802.11h - END */
// #define WLAN_EID_ERP_INFO 42
// #define WLAN_EID_HT_CAP 45
// #define WLAN_EID_RSN 48
// #define WLAN_EID_EXT_SUPP_RATES 50
// #define WLAN_EID_MOBILITY_DOMAIN 54
// #define WLAN_EID_FAST_BSS_TRANSITION 55
// #define WLAN_EID_TIMEOUT_INTERVAL 56
// #define WLAN_EID_RIC_DATA 57
// #define WLAN_EID_HT_OPERATION 61
// #define WLAN_EID_SECONDARY_CHANNEL_OFFSET 62
// #define WLAN_EID_20_40_BSS_COEXISTENCE 72
// #define WLAN_EID_20_40_BSS_INTOLERANT 73
// #define WLAN_EID_OVERLAPPING_BSS_SCAN_PARAMS 74
// #define WLAN_EID_MMIE 76
// #define WLAN_EID_VENDOR_SPECIFIC 221


/* Action frame categories (IEEE 802.11-2007, 7.3.1.11, Table 7-24) */
#define WLAN_ACTION_SPECTRUM_MGMT 0
#define WLAN_ACTION_QOS 1
#define WLAN_ACTION_DLS 2
#define WLAN_ACTION_BLOCK_ACK 3
#define WLAN_ACTION_PUBLIC 4
#define WLAN_ACTION_RADIO_MEASUREMENT 5
#define WLAN_ACTION_FT 6
#define WLAN_ACTION_HT 7
#define WLAN_ACTION_SA_QUERY 8
#define WLAN_ACTION_WMM 17 /* WMM Specification 1.1 */

/* SA Query Action frame (IEEE 802.11w/D8.0, 7.4.9) */
#define WLAN_SA_QUERY_REQUEST 0
#define WLAN_SA_QUERY_RESPONSE 1

#define WLAN_SA_QUERY_TR_ID_LEN 2


// /* Timeout Interval Type */
// #define WLAN_TIMEOUT_REASSOC_DEADLINE 1
// #define WLAN_TIMEOUT_KEY_LIFETIME 2
// #define WLAN_TIMEOUT_ASSOC_COMEBACK 3




//#undef ieee80211_hdr;

SSV_PACKED_STRUCT_BEGIN  
struct SSV_PACKED_STRUCT ieee80211_hdr {
 	le16_ssv_type frame_control;
 	le16_ssv_type duration_id;
 	ssv_type_u8 addr1[6];
 	ssv_type_u8 addr2[6];
 	ssv_type_u8 addr3[6];
 	le16_ssv_type seq_ctrl;
 	/* followed by 'u8 addr4[6];' if ToDS and FromDS is set in data frame
 	 */
}SSV_PACKED_STRUCT_STRUCT;
SSV_PACKED_STRUCT_END

#define IEEE80211_DA_FROMDS addr1
#define IEEE80211_BSSID_FROMDS addr2
#define IEEE80211_SA_FROMDS addr3

#define IEEE80211_HDRLEN (sizeof(struct ieee80211_hdr))

#define IEEE80211_FC(type, stype) host_to_le16((type << 2) | (stype << 4))

// struct ieee80211_mgmt {
// 	le16 frame_control;
// 	le16 duration;
// 	u8 da[6];
// 	u8 sa[6];
// 	u8 bssid[6];
// 	le16 seq_ctrl;
// 	union {
// 		struct {
// 			le16 auth_alg;
// 			le16 auth_transaction;
// 			le16 status_code;
// 			/* possibly followed by Challenge text */
// 			u8 variable[0];
// 		} SSV_PACKED_STRUCT_BEGIN  auth;
// 		struct {
// 			le16 reason_code;
// 		} SSV_PACKED_STRUCT_BEGIN  deauth;
// 		struct {
// 			le16 capab_info;
// 			le16 listen_interval;
// 			/* followed by SSID and Supported rates */
// 			u8 variable[0];
// 		} SSV_PACKED_STRUCT_BEGIN  assoc_req;
// 		struct {
// 			le16 capab_info;
// 			le16 status_code;
// 			le16 aid;
// 			/* followed by Supported rates */
// 			u8 variable[0];
// 		} SSV_PACKED_STRUCT_BEGIN  assoc_resp, reassoc_resp;
// 		struct {
// 			le16 capab_info;
// 			le16 listen_interval;
// 			u8 current_ap[6];
// 			/* followed by SSID and Supported rates */
// 			u8 variable[0];
// 		} SSV_PACKED_STRUCT_BEGIN  reassoc_req;
// 		struct {
// 			le16 reason_code;
// 		} SSV_PACKED_STRUCT_BEGIN  disassoc;
// 		struct {
// 			u8 timestamp[8];
// 			le16 beacon_int;
// 			le16 capab_info;
// 			/* followed by some of SSID, Supported rates,
// 			 * FH Params, DS Params, CF Params, IBSS Params, TIM */
// 			u8 variable[0];
// 		} SSV_PACKED_STRUCT_BEGIN  beacon;
// 		struct {
// 			/* only variable items: SSID, Supported rates */
// 			u8 variable[0];
// 		} SSV_PACKED_STRUCT_BEGIN  probe_req;
// 		struct {
// 			u8 timestamp[8];
// 			le16 beacon_int;
// 			le16 capab_info;
// 			/* followed by some of SSID, Supported rates,
// 			 * FH Params, DS Params, CF Params, IBSS Params */
// 			u8 variable[0];
// 		} SSV_PACKED_STRUCT_BEGIN  probe_resp;
// 		struct {
// 			u8 category;
// 			union {
// 				struct {
// 					u8 action_code;
// 					u8 dialog_token;
// 					u8 status_code;
// 					u8 variable[0];
// 				} SSV_PACKED_STRUCT_BEGIN  wmm_action;
// 				struct{
// 					u8 action_code;
// 					u8 element_id;
// 					u8 length;
// 					u8 switch_mode;
// 					u8 new_chan;
// 					u8 switch_count;
// 				} SSV_PACKED_STRUCT_BEGIN  chan_switch;
// 				struct {
// 					u8 action;
// 					u8 sta_addr[ETH_ALEN];
// 					u8 target_ap_addr[ETH_ALEN];
// 					u8 variable[0]; /* FT Request */
// 				} SSV_PACKED_STRUCT_BEGIN  ft_action_req;
// 				struct {
// 					u8 action;
// 					u8 sta_addr[ETH_ALEN];
// 					u8 target_ap_addr[ETH_ALEN];
// 					le16 status_code;
// 					u8 variable[0]; /* FT Request */
// 				} SSV_PACKED_STRUCT_BEGIN  ft_action_resp;
// 				struct {
// 					u8 action;
// 					u8 trans_id[WLAN_SA_QUERY_TR_ID_LEN];
// 				} SSV_PACKED_STRUCT_BEGIN  sa_query_req;
// 				struct {
// 					u8 action; /* */
// 					u8 trans_id[WLAN_SA_QUERY_TR_ID_LEN];
// 				} SSV_PACKED_STRUCT_BEGIN  sa_query_resp;
// 			} u;
// 		} SSV_PACKED_STRUCT_BEGIN  action;
// 	} u;
// } SSV_PACKED_STRUCT_BEGIN ;


SSV_PACKED_STRUCT_BEGIN  
struct SSV_PACKED_STRUCT ieee80211_ht_capabilities {
	le16_ssv_type ht_capabilities_info; // 2
	ssv_type_u8 a_mpdu_params; /* Maximum A-MPDU Length Exponent B0..B1
			   * Minimum MPDU Start Spacing B2..B4
			   * Reserved B5..B7 */
	ssv_type_u8 supported_mcs_set[16]; // 16
	le16_ssv_type ht_extended_capabilities; // 2
	le32_ssv_type tx_bf_capability_info; // 4
	ssv_type_u8 asel_capabilities; // 1
}SSV_PACKED_STRUCT_STRUCT;
SSV_PACKED_STRUCT_END
//2+1+16+2+4+1 = 26
#define HT_CAP_SIZE 26

SSV_PACKED_STRUCT_BEGIN  
struct SSV_PACKED_STRUCT ieee80211_ht_operation {
	ssv_type_u8 primary_chan;
	/* Five octets of HT Operation Information */
	ssv_type_u8 ht_param; /* B0..B7 */
	le16_ssv_type operation_mode; /* B8..B23 */
	le16_ssv_type param; /* B24..B39 */
	ssv_type_u8 basic_mcs_set[16];

}SSV_PACKED_STRUCT_STRUCT;
SSV_PACKED_STRUCT_END



#define ERP_INFO_NON_ERP_PRESENT BIT(0)
#define ERP_INFO_USE_PROTECTION BIT(1)
#define ERP_INFO_BARKER_PREAMBLE_MODE BIT(2)


#define HT_CAP_INFO_LDPC_CODING_CAP		((ssv_type_u16) BIT(0))
#define HT_CAP_INFO_SUPP_CHANNEL_WIDTH_SET	((ssv_type_u16) BIT(1))
#define HT_CAP_INFO_SMPS_MASK			((ssv_type_u16) (BIT(2) | BIT(3)))
#define HT_CAP_INFO_SMPS_STATIC			((ssv_type_u16) 0)
#define HT_CAP_INFO_SMPS_DYNAMIC		((ssv_type_u16) BIT(2))
#define HT_CAP_INFO_SMPS_DISABLED		((ssv_type_u16) (BIT(2) | BIT(3)))
#define HT_CAP_INFO_GREEN_FIELD			((ssv_type_u16) BIT(4))
#define HT_CAP_INFO_SHORT_GI20MHZ		((ssv_type_u16) BIT(5))
#define HT_CAP_INFO_SHORT_GI40MHZ		((ssv_type_u16) BIT(6))
#define HT_CAP_INFO_TX_STBC			((ssv_type_u16) BIT(7))
#define HT_CAP_INFO_RX_STBC_MASK		((ssv_type_u16) (BIT(8) | BIT(9)))
#define HT_CAP_INFO_RX_STBC_1			((ssv_type_u16) BIT(8))
#define HT_CAP_INFO_RX_STBC_12			((ssv_type_u16) BIT(9))
#define HT_CAP_INFO_RX_STBC_123			((ssv_type_u16) (BIT(8) | BIT(9)))
#define HT_CAP_INFO_DELAYED_BA			((ssv_type_u16) BIT(10))
#define HT_CAP_INFO_MAX_AMSDU_SIZE		((ssv_type_u16) BIT(11))
#define HT_CAP_INFO_DSSS_CCK40MHZ		((ssv_type_u16) BIT(12))
#define HT_CAP_INFO_PSMP_SUPP			((ssv_type_u16) BIT(13))
#define HT_CAP_INFO_40MHZ_INTOLERANT		((ssv_type_u16) BIT(14))
#define HT_CAP_INFO_LSIG_TXOP_PROTECT_SUPPORT	((ssv_type_u16) BIT(15))


#define EXT_HT_CAP_INFO_PCO			((ssv_type_u16) BIT(0))
#define EXT_HT_CAP_INFO_TRANS_TIME_OFFSET	1
#define EXT_HT_CAP_INFO_MCS_FEEDBACK_OFFSET	8
#define EXT_HT_CAP_INFO_HTC_SUPPORTED		((ssv_type_u16) BIT(10))
#define EXT_HT_CAP_INFO_RD_RESPONDER		((ssv_type_u16) BIT(11))


#define TX_BEAMFORM_CAP_TXBF_CAP ((ssv_type_u32) BIT(0))
#define TX_BEAMFORM_CAP_RX_STAGGERED_SOUNDING_CAP ((ssv_type_u32) BIT(1))
#define TX_BEAMFORM_CAP_TX_STAGGERED_SOUNDING_CAP ((ssv_type_u32) BIT(2))
#define TX_BEAMFORM_CAP_RX_ZLF_CAP ((ssv_type_u32) BIT(3))
#define TX_BEAMFORM_CAP_TX_ZLF_CAP ((ssv_type_u32) BIT(4))
#define TX_BEAMFORM_CAP_IMPLICIT_ZLF_CAP ((ssv_type_u32) BIT(5))
#define TX_BEAMFORM_CAP_CALIB_OFFSET 6
#define TX_BEAMFORM_CAP_EXPLICIT_CSI_TXBF_CAP ((ssv_type_u32) BIT(8))
#define TX_BEAMFORM_CAP_EXPLICIT_UNCOMPR_STEERING_MATRIX_CAP ((ssv_type_u32) BIT(9))
#define TX_BEAMFORM_CAP_EXPLICIT_BF_CSI_FEEDBACK_CAP ((ssv_type_u32) BIT(10))
#define TX_BEAMFORM_CAP_EXPLICIT_BF_CSI_FEEDBACK_OFFSET 11
#define TX_BEAMFORM_CAP_EXPLICIT_UNCOMPR_STEERING_MATRIX_FEEDBACK_OFFSET 13
#define TX_BEAMFORM_CAP_EXPLICIT_COMPRESSED_STEERING_MATRIX_FEEDBACK_OFFSET 15
#define TX_BEAMFORM_CAP_MINIMAL_GROUPING_OFFSET 17
#define TX_BEAMFORM_CAP_CSI_NUM_BEAMFORMER_ANT_OFFSET 19
#define TX_BEAMFORM_CAP_UNCOMPRESSED_STEERING_MATRIX_BEAMFORMER_ANT_OFFSET 21
#define TX_BEAMFORM_CAP_COMPRESSED_STEERING_MATRIX_BEAMFORMER_ANT_OFFSET 23
#define TX_BEAMFORM_CAP_SCI_MAX_OF_ROWS_BEANFORMER_SUPPORTED_OFFSET 25


#define ASEL_CAPABILITY_ASEL_CAPABLE ((ssv_type_u8) BIT(0))
#define ASEL_CAPABILITY_EXPLICIT_CSI_FEEDBACK_BASED_TX_AS_CAP ((ssv_type_u8) BIT(1))
#define ASEL_CAPABILITY_ANT_INDICES_FEEDBACK_BASED_TX_AS_CAP ((ssv_type_u8) BIT(2))
#define ASEL_CAPABILITY_EXPLICIT_CSI_FEEDBACK_CAP ((ssv_type_u8) BIT(3))
#define ASEL_CAPABILITY_ANT_INDICES_FEEDBACK_CAP ((ssv_type_u8) BIT(4))
#define ASEL_CAPABILITY_RX_AS_CAP ((ssv_type_u8) BIT(5))
#define ASEL_CAPABILITY_TX_SOUND_PPDUS_CAP ((ssv_type_u8) BIT(6))

#define HT_INFO_HT_PARAM_SECONDARY_CHNL_OFF_MASK	((ssv_type_u8) BIT(0) | BIT(1))
#define HT_INFO_HT_PARAM_SECONDARY_CHNL_ABOVE		((ssv_type_u8) BIT(0))
#define HT_INFO_HT_PARAM_SECONDARY_CHNL_BELOW		((ssv_type_u8) BIT(0) | BIT(1))
#define HT_INFO_HT_PARAM_REC_TRANS_CHNL_WIDTH		((ssv_type_u8) BIT(2))
#define HT_INFO_HT_PARAM_RIFS_MODE			((ssv_type_u8) BIT(3))
#define HT_INFO_HT_PARAM_CTRL_ACCESS_ONLY		((ssv_type_u8) BIT(4))
#define HT_INFO_HT_PARAM_SRV_INTERVAL_GRANULARITY	((ssv_type_u8) BIT(5))


/* HT Protection (B8..B9 of HT Operation Information) */
#define HT_PROT_NO_PROTECTION           0
#define HT_PROT_NONMEMBER_PROTECTION    1
#define HT_PROT_20MHZ_PROTECTION        2
#define HT_PROT_NON_HT_MIXED            3
/* Bits within ieee80211_ht_operation::operation_mode (BIT(0) maps to B8 in
 * HT Operation Information) */
#define HT_OPER_OP_MODE_HT_PROT_MASK ((ssv_type_u16) (BIT(0) | BIT(1))) /* B8..B9 */
#define HT_OPER_OP_MODE_NON_GF_HT_STAS_PRESENT	((ssv_type_u16) BIT(2)) /* B10 */
/* BIT(3), i.e., B11 in HT Operation Information field - Reserved */
#define HT_OPER_OP_MODE_OBSS_NON_HT_STAS_PRESENT	((ssv_type_u16) BIT(4)) /* B12 */
/* BIT(5)..BIT(15), i.e., B13..B23 - Reserved */

/* Last two octets of HT Operation Information (BIT(0) = B24) */
/* B24..B29 - Reserved */
#define HT_OPER_PARAM_DUAL_BEACON			((ssv_type_u16) BIT(6))
#define HT_OPER_PARAM_DUAL_CTS_PROTECTION		((ssv_type_u16) BIT(7))
#define HT_OPER_PARAM_STBC_BEACON			((ssv_type_u16) BIT(8))
#define HT_OPER_PARAM_LSIG_TXOP_PROT_FULL_SUPP		((ssv_type_u16) BIT(9))
#define HT_OPER_PARAM_PCO_ACTIVE			((ssv_type_u16) BIT(10))
#define HT_OPER_PARAM_PCO_PHASE				((ssv_type_u16) BIT(11))
/* B36..B39 - Reserved */



#define OUI_MICROSOFT 0x0050f2 /* Microsoft (also used in Wi-Fi specs)
				* 00:50:F2 */
#define WPA_IE_VENDOR_TYPE 0x0050f201
#define WPS_IE_VENDOR_TYPE 0x0050f204

#define WMM_OUI_TYPE 2
#define WMM_OUI_SUBTYPE_INFORMATION_ELEMENT 0
#define WMM_OUI_SUBTYPE_PARAMETER_ELEMENT 1
#define WMM_OUI_SUBTYPE_TSPEC_ELEMENT 2
#define WMM_VERSION 1

#define WMM_ACTION_CODE_ADDTS_REQ 0
#define WMM_ACTION_CODE_ADDTS_RESP 1
#define WMM_ACTION_CODE_DELTS 2

#define WMM_ADDTS_STATUS_ADMISSION_ACCEPTED 0
#define WMM_ADDTS_STATUS_INVALID_PARAMETERS 1
/* 2 - Reserved */
#define WMM_ADDTS_STATUS_REFUSED 3
/* 4-255 - Reserved */

/* WMM TSPEC Direction Field Values */
#define WMM_TSPEC_DIRECTION_UPLINK 0
#define WMM_TSPEC_DIRECTION_DOWNLINK 1
/* 2 - Reserved */
#define WMM_TSPEC_DIRECTION_BI_DIRECTIONAL 3








/*
 * WMM Information Element (used in (Re)Association Request frames; may also be
 * used in Beacon frames)
 */
SSV_PACKED_STRUCT_BEGIN  
struct SSV_PACKED_STRUCT wmm_information_element {
	/* Element ID: 221 (0xdd); Length: 7 */
	/* required fields for WMM version 1 */
	ssv_type_u8 oui[3]; /* 00:50:f2 */
	ssv_type_u8 oui_type; /* 2 */
	ssv_type_u8 oui_subtype; /* 0 */
	ssv_type_u8 version; /* 1 for WMM version 1.0 */
	ssv_type_u8 qos_info; /* AP/STA specific QoS info */

}SSV_PACKED_STRUCT_STRUCT;
SSV_PACKED_STRUCT_END

#define WMM_QOSINFO_STA_AC_MASK 0x0f
#define WMM_QOSINFO_STA_SP_MASK 0x03
#define WMM_QOSINFO_STA_SP_SHIFT 5


#define WMM_AC_AIFSN_MASK 0x0f
#define WMM_AC_AIFNS_SHIFT 0
#define WMM_AC_ACM 0x10
#define WMM_AC_ACI_MASK 0x60
#define WMM_AC_ACI_SHIFT 5

#define WMM_AC_ECWMIN_MASK 0x0f
#define WMM_AC_ECWMIN_SHIFT 0
#define WMM_AC_ECWMAX_MASK 0xf0
#define WMM_AC_ECWMAX_SHIFT 4

SSV_PACKED_STRUCT_BEGIN  
struct SSV_PACKED_STRUCT wmm_ac_parameter {
	ssv_type_u8 aci_aifsn; /* AIFSN, ACM, ACI */
	ssv_type_u8 cw; /* ECWmin, ECWmax (CW = 2^ECW - 1) */
	le16_ssv_type txop_limit;
}SSV_PACKED_STRUCT_STRUCT;
SSV_PACKED_STRUCT_END

/*
 * WMM Parameter Element (used in Beacon, Probe Response, and (Re)Association
 * Response frmaes)
 */
SSV_PACKED_STRUCT_BEGIN  
struct SSV_PACKED_STRUCT wmm_parameter_element {
	/* Element ID: 221 (0xdd); Length: 24 */
	/* required fields for WMM version 1 */
	ssv_type_u8 oui[3]; /* 00:50:f2 */
	ssv_type_u8 oui_type; /* 2 */
	ssv_type_u8 oui_subtype; /* 1 */
	ssv_type_u8 version; /* 1 for WMM version 1.0 */
	ssv_type_u8 qos_info; /* AP/STA specif QoS info */
	ssv_type_u8 reserved; /* 0 */
	struct wmm_ac_parameter ac[4]; /* AC_BE, AC_BK, AC_VI, AC_VO */

}SSV_PACKED_STRUCT_STRUCT;
SSV_PACKED_STRUCT_END

/* WMM TSPEC Element */
SSV_PACKED_STRUCT_BEGIN  
struct SSV_PACKED_STRUCT wmm_tspec_element {
	ssv_type_u8 eid; /* 221 = 0xdd */
	ssv_type_u8 length; /* 6 + 55 = 61 */
	ssv_type_u8 oui[3]; /* 00:50:f2 */
	ssv_type_u8 oui_type; /* 2 */
	ssv_type_u8 oui_subtype; /* 2 */
	ssv_type_u8 version; /* 1 */
	/* WMM TSPEC body (55 octets): */
	ssv_type_u8 ts_info[3];
	le16_ssv_type nominal_msdu_size;
	le16_ssv_type maximum_msdu_size;
	le32_ssv_type minimum_service_interval;
	le32_ssv_type maximum_service_interval;
	le32_ssv_type inactivity_interval;
	le32_ssv_type suspension_interval;
	le32_ssv_type service_start_time;
	le32_ssv_type minimum_data_rate;
	le32_ssv_type mean_data_rate;
	le32_ssv_type peak_data_rate;
	le32_ssv_type maximum_burst_size;
	le32_ssv_type delay_bound;
	le32_ssv_type minimum_phy_rate;
	le16_ssv_type surplus_bandwidth_allowance;
	le16_ssv_type medium_time;
}SSV_PACKED_STRUCT_STRUCT;
SSV_PACKED_STRUCT_END


#define OUI_BROADCOM 0x00904c /* Broadcom (Epigram) */

#define VENDOR_HT_CAPAB_OUI_TYPE 0x33 /* 00-90-4c:0x33 */

/* cipher suite selectors */
#define WLAN_CIPHER_SUITE_USE_GROUP	0x000FAC00
#define WLAN_CIPHER_SUITE_WEP40		0x000FAC01
#define WLAN_CIPHER_SUITE_TKIP		0x000FAC02
/* reserved: 				0x000FAC03 */
#define WLAN_CIPHER_SUITE_CCMP		0x000FAC04
#define WLAN_CIPHER_SUITE_WEP104	0x000FAC05
#define WLAN_CIPHER_SUITE_AES_CMAC	0x000FAC06

/* AKM suite selectors */
#define WLAN_AKM_SUITE_8021X		0x000FAC01
#define WLAN_AKM_SUITE_PSK		0x000FAC02



#ifdef _MSC_VER
#pragma pack(pop)
#endif /* _MSC_VER */



#endif /* IEEE802_11_DEFS_H */
