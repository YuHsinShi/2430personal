/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _AP_CONFIG_H_
#define _AP_CONFIG_H_

#include <ssv_types.h>
#include "common/defs.h"
#include <ieee80211.h>
// ------------------------- ap support 80211n ---------------------------
#define AP_SUPPORT_80211N 1

//typedef unsigned int     size_t;

#define DEFAULT_SSID		"SSV"

//-----------------------------------
#define AP_DEFAULT_SNED_PROBE_RESPONSE  1
#define AP_DEFAULT_IGNORE_BROADCAST_SSID  0

//--------------------------------------------------------


#define AP_DEFAULT_HW_MODE		AP_MODE_IEEE80211G
#define AP_DEFAULT_CHANNEL		6
//#define AP_DEFAULT_COUNTRY      "GB "   hard code

#if AP_SUPPORT_80211N
#define AP_DEFAULT_80211N		TRUE
#define AP_DEFAULT_REQUIRE_HT	FALSE
#else
#define AP_DEFAULT_80211N		FALSE
#define AP_DEFAULT_REQUIRE_HT	FALSE

#endif

//-------------------------------------------------------
#define AP_MODE_TO_HW_RATE_FLAGS_MANDATORY_SHIFT 0
#define AP_MODE_TO_HW_RATE_FLAGS_SUPPORT_SHIFT 4


#ifdef _MSC_VER
#pragma pack(push, 1)
#endif /* _MSC_VER */

SSV_PACKED_STRUCT_BEGIN 
struct SSV_PACKED_STRUCT ieee80211_rate {	
	ssv_type_u16 bitrate;
	ssv_type_u16 flags;
	//u16 hw_value, hw_value_short;
}SSV_PACKED_STRUCT_STRUCT;
SSV_PACKED_STRUCT_END

#ifdef _MSC_VER
#pragma pack(pop)
#endif /* _MSC_VER */

/**
 * enum ieee80211_rate_flags - rate flags
 *
 * Hardware/specification flags for rates. These are structured
 * in a way that allows using the same bitrate structure for
 * different bands/PHY modes.
 *
 * @IEEE80211_RATE_SHORT_PREAMBLE: Hardware can send with short
 *	preamble on this bitrate; only relevant in 2.4GHz band and
 *	with CCK rates.
 * @IEEE80211_RATE_MANDATORY_A: This bitrate is a mandatory rate
 *	when used with 802.11a (on the 5 GHz band); filled by the
 *	core code when registering the wiphy.
 * @IEEE80211_RATE_MANDATORY_B: This bitrate is a mandatory rate
 *	when used with 802.11b (on the 2.4 GHz band); filled by the
 *	core code when registering the wiphy.
 * @IEEE80211_RATE_MANDATORY_G: This bitrate is a mandatory rate
 *	when used with 802.11g (on the 2.4 GHz band); filled by the
 *	core code when registering the wiphy.
 * @IEEE80211_RATE_ERP_G: This is an ERP rate in 802.11g mode.
 */
enum ieee80211_rate_flags {
	IEEE80211_RATE_FLAGS_NONE				= 0,
	IEEE80211_RATE_FLAGS_MANDATORY_A		= 1<<1, //----------------------------------------------------->AP_MODE_TO_HW_RATE_FLAGS_MANDATORY_SHIFT
	IEEE80211_RATE_FLAGS_MANDATORY_B		= 1<<2,
	IEEE80211_RATE_FLAGS_MANDATORY_G		= 1<<3,
	IEEE80211_RATE_FLAGS_SUPPORT_A		= 1<<4,		//------------------------------------------------------>AP_MODE_TO_HW_RATE_FLAGS_SUPPORT_SHIFT
	IEEE80211_RATE_FLAGS_SUPPORT_B		= 1<<5,
	IEEE80211_RATE_FLAGS_SUPPORT_G		= 1<<6,
	IEEE80211_RATE_FLAGS_SHORT_PREAMBLE	= 1<<7,
	IEEE80211_RATE_FLAGS_ERP_G			= 1<<8
};

enum ieee80211_rate_num {
	IEEE80211_RATE_NUM_1M 	= 2,
	IEEE80211_RATE_NUM_2M 	= 4,
	IEEE80211_RATE_NUM_5_5M = 11,
	IEEE80211_RATE_NUM_6M 	= 12,
	IEEE80211_RATE_NUM_9M	= 18,
	IEEE80211_RATE_NUM_11M	= 22,
	IEEE80211_RATE_NUM_12M	= 24,
	IEEE80211_RATE_NUM_18M 	= 36,
	IEEE80211_RATE_NUM_22M	= 44,
	IEEE80211_RATE_NUM_24M	= 48,
	IEEE80211_RATE_NUM_33M	= 66,
	IEEE80211_RATE_NUM_36M	= 72,
	IEEE80211_RATE_NUM_48M	= 96,
	IEEE80211_RATE_NUM_54M	= 108
};

struct ssv6xxx_host_ap_config {

	char ssid[IEEE80211_MAX_SSID_LEN + 1];
	ssv_type_size_t ssid_len;

	ssv_type_u8 eApMode;

	ssv_type_bool b80211n;
	ssv_type_bool bRequire_ht;

	ssv_type_u8  nChannel;
	
	ePreamble preamble;

// 	char country[3]; /* first two octets: country code as described in
// 			  * ISO/IEC 3166-1. Third octet:
// 			  * ' ' (ascii 32): all environments			 
// 			  */
	ssv_type_u8  nNumrates;
	struct ieee80211_rate **pRates;
    
    int disable_11n;

	/**
	 * ht_capab - HT (IEEE 802.11n) capabilities
	 */
	ssv_type_u16 ht_capab;

	/**
	 * mcs_set - MCS (IEEE 802.11n) rate parameters
	 */
	ssv_type_u8 mcs_set[16];

	/**
	 * a_mpdu_params - A-MPDU (IEEE 802.11n) parameters
	 */
	ssv_type_u8 a_mpdu_params;

    int secondary_channel;
};

void ssv6xxx_config_init(struct ssv6xxx_host_ap_config *pConfig);

#endif /* _AP_CONFIG_H_ */
