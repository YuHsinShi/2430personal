/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef AP_INFO_H
#define AP_INFO_H

#include <ieee80211.h>
#include "common/bitops.h"
#include <pbuf.h>
#include <ssv_ether.h>
#include "ap_config.h"
#include "common/defs.h"
#include "neighbor_ap_list.h"
#include "ap_def.h"
#include "ap_tx.h"
#include "ap.h"
#include <cmd_def.h>
#include "ap_config.h"

#ifndef CONFIG_NO_WPA2
#include "wpa_auth_i.h"
#include "wpa_auth.h"
#define PMK_LEN 32

struct ssv6xxx_wpa_psk {
	ssv_type_u8 psk[PMK_LEN];
	ssv_type_u8 addr[ETH_ALEN];
	};

#endif
//#define AID_WORDS ((AP_MAX_STA + 31) / 32)

//#define AID_WORDS (sizeof(unsigned long) * BITS_TO_LONGS(IEEE80211_MAX_AID + 1))

enum beacon_tim_types {

	tim_type_none,			//None
	tim_type_2bytes,		//AID 	0~15
	tim_type_top_half,		//AID 	0~7
	tim_type_bottom_half	//AID	8~15
};

//IEEE80211_CHAN_FLAGS_NONE			= 0,
enum ieee80211_channel_flags {
	IEEE80211_CHAN_FLAGS_DISABLED		= BIT(0),
	IEEE80211_CHAN_FLAGS_PASSIVE_SCAN	= BIT(1),
	IEEE80211_CHAN_FLAGS_NO_IBSS		= BIT(2),
	IEEE80211_CHAN_FLAGS_RADAR			= BIT(3),
	IEEE80211_CHAN_FLAGS_NO_HT40PLUS	= BIT(4),
	IEEE80211_CHAN_FLAGS_NO_HT40MINUS	= BIT(5)
};

#define IEEE80211_CHAN_NO_HT40 \
	(IEEE80211_CHAN_NO_HT40PLUS | IEEE80211_CHAN_NO_HT40MINUS)

typedef enum ieee80211_security_policy {
	SECURITY_PLAINTEXT = 0,
	SECURITY_STATIC_WEP = 1,
	SECURITY_IEEE_802_1X = 2,
	SECURITY_WPA_PSK = 3,			//PSK
	SECURITY_WPA = 4				//Enterprise  WPA-RADIUS/EAP
} secpolicy;



SSV_PACKED_STRUCT_BEGIN  
struct SSV_PACKED_STRUCT ieee80211_channel_data {
	ssv_type_u8 chan;
	short freq;
	ssv_type_u8 max_tx_power;
	int flag;
}SSV_PACKED_STRUCT_STRUCT;
SSV_PACKED_STRUCT_END


//struct os_timer;

typedef enum{
	AP_READY_SET_AP_MODE	,
	AP_READY_GET_HW_INFO	,
	AP_READY_SET_INFO		
}ap_ready_state;

typedef struct ApInfo{
	//--------------

	ap_state current_st;
	//APCmdCb cb;
	ssv_type_u8* pOSMgmtframe;		//pMgmtPkt
	ssv_type_u8* pMgmtPkt;
	ssv_type_u8 eCurrentApMode;
	ssv_type_bool b80211n;
	ssv_type_u8  nCurrentChannel;
	//struct ieee80211_channel_data stChannelData[AP_MAX_CHANNEL];
	ssv_type_u8 own_addr[ETH_ALEN];

	/*
	 * Bitfield for indicating which AIDs are allocated. Only AID values
	 * 1-2007 are used and as such, the bit at index 0 corresponds to AID
	 * 1.
	 */
	ssv_type_u32 sta_aid[1];//[AID_WORDS];
//--------------------------------------------------------------------------------------

	//u8 sta_aid[sizeof(unsigned long) * BITS_TO_LONGS(IEEE80211_MAX_AID + 1)];

	/* yes, this looks ugly, but guarantees that we can later use
	 * bitmap_empty :)
	 * NB: don't touch this bitmap, use sta_info_{set,clear}_tim_bit */

	//u8 tim[sizeof(unsigned long) * BITS_TO_LONGS(IEEE80211_MAX_AID + 1)];
	ssv_type_u8* tim;
	//
	int dtim_count;
	//ssv_type_bool dtim_bc_mc;
//----------------------------------
//station info

	ssv_type_u8 num_sta; /* number of entries in sta_list */

	ssv_type_u8 num_sta_ps;//atomic_t num_sta_ps; /* number of stations in PS mode */
	ssv_type_u8 num_sta_authorized;/* number of authorized stations */

	/* Number of associated Non-ERP stations (i.e., stations using 802.11b
	* in 802.11g BSS) */
	ssv_type_u8 num_sta_non_erp;

	/* Number of associated stations that do not support Short Preamble */
	ssv_type_u8 num_sta_no_short_preamble;

	/* Number of associated stations that do not support Short Slot Time */
	ssv_type_u8 num_sta_no_short_slot_time;

	//secpolicy policy ;

	/* Number of HT associated stations that do not support greenfield */
	int num_sta_ht_no_gf;

	/* Number of associated non-HT stations */
	int num_sta_no_ht;

	/* Number of HT associated stations 20 MHz */
	int num_sta_ht_20mhz;

	//----------------------------
	//Beacon
	//-----------------------------

	struct cfg_bcn_info bcn_info;

    ssv_type_u8  bcn[AP_MGMT_BEACON_LEN];

	ssv_type_u8* pBeaconHead;
	ssv_type_u16 nBHeadLen;

	ssv_type_u8 *pBeaconTail;
	ssv_type_u16 nBTailLen;

	ssv_type_u8 *pBeaconTim;

	//u16 nBLen;
	//----------------------------
	//QoS
	//-----------------------------

	ssv_type_u8 parameter_set_count;


	//----------------------------
	//Detect other AP activities
	//----------------------------
	int olbc; /* Overlapping Legacy BSS Condition */
	int olbc_ht;/* Overlapping BSS information */

	ssv_type_u8 num_neighbor_ap;
	struct neighbor_ap_info neighbor_ap_list[AP_NEIGHBOR_AP_TABLE_MAX_SIZE];

	//------------------------------
	//PS related
	ssv_type_u16 total_ps_buffered;
	ssv_type_bool sta_pkt_cln_timer;             //cleanup station packet in ps mode
    OsMutex  ap_info_ps_mutex;

	//Used to store tx descriptor
	struct ap_tx_desp *pApTxData;

	//-----------------------------
	//security
	//-----------------------------

	int auth_algs; /* bitfield of allowed IEEE 802.11 authentication
			* algorithms, WPA_AUTH_ALG_{OPEN,SHARED,LEAP} */
	int tkip_countermeasures;


	ssv6xxx_sec_type sec_type;
	ssv_type_u8 password[MAX_PASSWD_LEN+1];

#ifndef CONFIG_NO_WPA2

	struct ssv6xxx_wpa_psk wpa_psk;
    struct wpa_authenticator wpa_auth;
    struct wpa_auth_config  ap_conf;
    struct wpa_group ap_group;

    //end:add by angel.chiu for wpa
#endif

	//-----------------------------
	//Timer
	//-----------------------------
	//-----------------------------
	//Configuration
	//----------------------------
	struct ssv6xxx_host_ap_config config;

	/* Overlapping BSS information */
	ssv_type_u16 ht_op_mode;

    /* Auto Channel Selection */
    ssv_type_u32 channel_edcca_count[MAX_2G_CHANNEL_NUM];
    ssv_type_u32 channel_packet_count[MAX_2G_CHANNEL_NUM];
    ssv_type_u32 channel_5g_edcca_count[MAX_5G_CHANNEL_NUM];
    ssv_type_u32 channel_5g_packet_count[MAX_5G_CHANNEL_NUM];
    ssv_type_u8  acs_start;
    /* Beacon */
#if(AUTO_BEACON==0)
    OsMutex g_dev_bcn_mutex;
    ssv_type_u32 beacon_upd_need:1;
    ssv_type_u32 rsvd:7;
#endif
    /* STA inforamction */
    struct APStaInfo *StaConInfo; // AP mode used
    void* vif;
    ssv_type_u8 TimAllOne;

}ApInfo_st;

#endif /* AP_INFO_H */
