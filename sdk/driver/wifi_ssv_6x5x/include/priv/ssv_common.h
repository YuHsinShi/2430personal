/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _COMMON_H_
#define _COMMON_H_

#include <porting.h>
#include <host_config.h>

#define CHIP_ID_SSV6051Q_OLD 0x70000000
#define CHIP_ID_SSV6051Z 0x71000000
#define CHIP_ID_SSV6052Q 0x72000000
#define CHIP_ID_SSV6051Q 0x73000000
#define CHIP_ID_SSV6060P 0x74000000
#define CHIP_ID_SSV6051P 0x75000000
#define CHIP_ID_SSV6030P 0x76000000

#define CHIP_ID_SV6155P 0x70000000
#define CHIP_ID_SV6156P 0x71000000
#define CHIP_ID_SV6166P 0x72000000
#define CHIP_ID_SV6167Q 0x74000000
#define CHIP_ID_SV6166F 0x75000000
#define CHIP_ID_SV6166FS 0x77000000
#define CHIP_ID_SV6166M 0x76000000
#define CHIP_ID_SV6255P 0x78000000
#define CHIP_ID_SV6256P 0x79000000
#define CHIP_ID_SV6266P 0x7a000000
#define CHIP_ID_SV6267Q 0x7C000000
#define CHIP_ID_SV6266F 0x7D000000
#define CHIP_ID_SV6266M 0x7E000000
#define CHIP_ID_SV6151P 0x73800000
#define CHIP_ID_SV6152P 0x73800000

typedef enum{
    SSV6XXX_HWM_STA		,
    SSV6XXX_HWM_AP		,
    SSV6XXX_HWM_IBSS	,
    SSV6XXX_HWM_WDS	    ,
    SSV6XXX_HWM_SCONFIG ,
    SSV6XXX_HWM_AP_VLAN     ,
    SSV6XXX_HWM_MONITOR     ,
    SSV6XXX_HWM_MESH_POINT  ,
    SSV6XXX_HWM_P2P_CLIENT  ,
    SSV6XXX_HWM_P2P_GO      ,
    SSV6XXX_HWM_P2P_DEVICE  ,
    SSV6XXX_HWM_INVALID	
}ssv6xxx_hw_mode;

typedef enum{
	AP_STATE_INIT			,
	AP_STATE_IDLE			,	//Off
	AP_STATE_READY			,	//Init
	//AP_STATE_READY_SET_AP_MODE	,
	AP_STATE_RUNNING		,	//Running
	AP_STATE_PAUSE			
}ap_state;

enum ssv_80211_channel_type {
	SSV_80211_CHAN_NO_HT,
	SSV_80211_CHAN_HT20,
	SSV_80211_CHAN_HT40MINUS,
	SSV_80211_CHAN_HT40PLUS
};

//custom temperture setting
#ifndef EAPOL_ETHER_TYPPE
#define EAPOL_ETHER_TYPPE	0x888E
#endif

enum ssv_ieee80211_band {
SSV_IEEE80211_BAND_2GHZ ,
SSV_IEEE80211_BAND_5GHZ ,
/* keep last */
SSV_IEEE80211_NUM_BANDS
};

typedef enum {
	LONG_PREAMBLE = 0,
	SHORT_PREAMBLE = 1
} ePreamble;

typedef enum{
	SSV6XXX_SEC_NONE,
	SSV6XXX_SEC_WEP_40,			//5		ASCII
	SSV6XXX_SEC_WEP_104,		//13	ASCII
	SSV6XXX_SEC_WPA_PSK,		//8~63	ASCII
	SSV6XXX_SEC_WPA2_PSK,		//8~63	ASCII
	SSV6XXX_SEC_WPS,
	SSV6XXX_SEC_MAX
}ssv6xxx_sec_type;

enum ssv6xxx_tx_type {
    SSV6XXX_TX_REQ0,
    SSV6XXX_TX_REQ1,
    SSV6XXX_TX_REQ2,
    SSV6XXX_TX_MAX
};

typedef struct ssv_rf_temperature_st {
    ssv_type_u32 address;
    ssv_type_u32 high_temp;
    ssv_type_u32 regular_temp;
    ssv_type_u32 low_temp;
} ssv_rf_temperature;

typedef struct cali_result_info_st
{
    void* cali_res;
    ssv_type_u32 cali_size;
}cali_result_info;

#if (CONFIG_CHIP_ID==SSV6006C)
#define MAX_VIF_NUM 2
#else
#define MAX_VIF_NUM 1
#endif
//MAC: 60 11 22 33 44 55 = byte 0:1:2:3:4:5
#define MAC_MASK 0xef //For byte3

#define WSID_NOT_FOUND                0xf

#define RF_TEMPER_ARRARY_SIZE 5
#define RF_TEMPER_SETTING_SIZE RF_TEMPER_ARRARY_SIZE*sizeof(struct ssv_rf_temperature_st)

#define MAX_CCI_SENSITIVE 128
#define MIN_CCI_SENSITIVE 0

/* 30 byte 4 addr hdr, 2 byte QoS, 2304 byte MSDU, 12 byte crypt, 4 byte FCS, 80 byte rx_desc */
#define MAX_FRAME_SIZE                      2432

//HCI RX AGG
#define MAX_RX_PKT_RSVD                     512
#define HCI_AGGR_SIZE                   0x1b00 //6912
#define MAX_HCI_AGGR_SIZE                (HCI_AGGR_SIZE+MAX_FRAME_SIZE)  //AGGR_SIZE+MPDU

#define WPA_CIPHER_NONE BIT(0)
#define WPA_CIPHER_WEP40 BIT(1)
#define WPA_CIPHER_WEP104 BIT(2)
#define WPA_CIPHER_TKIP BIT(3)
#define WPA_CIPHER_CCMP BIT(4)
#ifdef CONFIG_IEEE80211W
#define WPA_CIPHER_AES_128_CMAC BIT(5)
#endif /* CONFIG_IEEE80211W */

#define WPA_KEY_MGMT_IEEE8021X BIT(0)
#define WPA_KEY_MGMT_PSK BIT(1)
#define WPA_KEY_MGMT_NONE BIT(2)
#define WPA_KEY_MGMT_IEEE8021X_NO_WPA BIT(3)
#define WPA_KEY_MGMT_WPA_NONE BIT(4)
#define WPA_KEY_MGMT_FT_IEEE8021X BIT(5)
#define WPA_KEY_MGMT_FT_PSK BIT(6)
#define WPA_KEY_MGMT_IEEE8021X_SHA256 BIT(7)
#define WPA_KEY_MGMT_PSK_SHA256 BIT(8)
#define WPA_KEY_MGMT_WPS BIT(9)

#define WPA_PROTO_WPA BIT(0)
#define WPA_PROTO_RSN BIT(1)

#define WPA_AUTH_ALG_OPEN BIT(0)
#define WPA_AUTH_ALG_SHARED BIT(1)
//#define WPA_AUTH_ALG_LEAP BIT(2)
//#define WPA_AUTH_ALG_FT BIT(3)


#define MAX_SSID_LEN 32
#define MAX_PASSWD_LEN 63
#define MAX_2G_CHANNEL_NUM 16
#define MAX_5G_CHANNEL_NUM 25
#define MAX_CHANNEL_NUM (MAX_2G_CHANNEL_NUM+MAX_5G_CHANNEL_NUM)
#define IS_5G_BAND(CH) ((CH)>MAX_2G_CHANNEL_NUM)

#define MAX_WEP_PASSWD_LEN (13+1)

#define MAX_ETHTYPE_TRAP 8

/*station timeout ARP request count*/

#define STA_TIMEOUT_RETRY_COUNT (5)
#define STA_TIMEOUT_RETRY_TIMER (3 * 1000) //msec


#define PHY_INFO_TBL1_SIZE          39
#define PHY_INFO_TBL2_SIZE          39
#define PHY_INFO_TBL3_SIZE          8

#define CHAN_SWITCH_TIMER_TIME      260 //260ms


//------------------------------------------------

/**
 *  struct cfg_sta_info - STA structure description
 *
*/
#if 0
struct cfg_sta_info {
    ETHER_ADDR      addr;
#if 1
		u32 			bit_rates; /* The first eight rates are the basic rate set */

		u8				listen_interval;

		u8				key_id;
		u8				key[16];
		u8				mic_key[8];

		/* TKIP IV */
		u16 			iv16;
		u32 			iv32;
#endif

} ;//__attribute__ ((packed));

#endif



/**
 *  struct cfg_bss_info - BSS/IBSS structure description
 *
 */
#if 0
struct cfg_bss_info {
    ETHER_ADDR          bssid;

};// __attribute__ ((packed));

#endif
SSV_PACKED_STRUCT_BEGIN 
struct SSV_PACKED_STRUCT ETHER_ADDR_st
{
    ssv_type_u8      addr[ETHER_ADDR_LEN];
}SSV_PACKED_STRUCT_STRUCT;
SSV_PACKED_STRUCT_END

typedef struct ETHER_ADDR_st        ETHER_ADDR;

#ifndef BIT
#define BIT(x) (1 << (x))
#endif

#define DIV_ROUND_UP(n, d)	(((n) + (d) - 1) / (d))

typedef enum
{
    MT_STOP,
    MT_RUNNING,
    MT_EXIT
}  ModeType; //CmdEngine/TXRX_Task mode type

/**
 *  struct cfg_80211_ssid - SSID description, the max of the length of 32-byte.
 *
 *  @ ssid: the SSID
 *  @ ssid_len: length of the SSID
 */
SSV_PACKED_STRUCT_BEGIN 
struct SSV_PACKED_STRUCT cfg_80211_ssid {
    ssv_type_u8              ssid[MAX_SSID_LEN];//ssid[] didnot include '\0' at end of matrix
    ssv_type_u8              ssid_len;
}SSV_PACKED_STRUCT_STRUCT;
SSV_PACKED_STRUCT_END


//=========================================
//                Public Command Data
//=========================================

//-------------------------------------------------------------------------------------------------------------------------------------------------
//HOST_CMD_SCAN

#define IEEE80211_MAX_SSID_LEN		32
#define IEEE80211_MAX_SUPP_RATES    32
#define IEEE80211_HT_MCS_MASK_LEN   10

/**
 * struct ssv_mcs_info - MCS information
 * @rx_mask: RX mask
 * @rx_highest: highest supported RX rate. If set represents
 *	the highest supported RX data rate in units of 1 Mbps.
 *	If this field is 0 this value should not be used to
 *	consider the highest RX data rate supported.
 * @tx_params: TX parameters
 */
SSV_PACKED_STRUCT_BEGIN 
struct SSV_PACKED_STRUCT ssv_mcs_info {
	ssv_type_u8 rx_mask[IEEE80211_HT_MCS_MASK_LEN];
	ssv_type_u16 rx_highest;
	ssv_type_u8 tx_params;
	ssv_type_u8 reserved[3];
}SSV_PACKED_STRUCT_STRUCT;
SSV_PACKED_STRUCT_END


/**
 * struct ssv_ht_cap - HT capabilities IE
 *
 */
SSV_PACKED_STRUCT_BEGIN 
struct SSV_PACKED_STRUCT ssv_ht_cap {
    ssv_type_u16 cap_info;
    ssv_type_u8 ampdu_params_info;

    /* 16 bytes MCS information */
    struct ssv_mcs_info mcs;

    ssv_type_u16 extended_ht_cap_info;
    ssv_type_u32 tx_BF_cap_info;
    ssv_type_u8 antenna_selection_info;
}SSV_PACKED_STRUCT_STRUCT;
SSV_PACKED_STRUCT_END


/**
 * struct ssv_ht_info - HT operation IE
 *
 */
SSV_PACKED_STRUCT_BEGIN 
struct SSV_PACKED_STRUCT ssv_ht_info {
    ssv_type_u8 control_chan;
    ssv_type_u8 ht_param;
    ssv_type_u16 operation_mode;
    ssv_type_u16 stbc_param;
    ssv_type_u8 basic_set[16];
}SSV_PACKED_STRUCT_STRUCT;
SSV_PACKED_STRUCT_END



SSV_PACKED_STRUCT_BEGIN 
struct SSV_PACKED_STRUCT ssv6xxx_rxphy_info_padding {
    /* WORD 1: for B, G, N mode */
    ssv_type_u32             rpci:8;     /* RSSI */
    ssv_type_u32             snr:8;
    ssv_type_u32             RSVD:16;
}SSV_PACKED_STRUCT_STRUCT;
SSV_PACKED_STRUCT_END


SSV_PACKED_STRUCT_BEGIN  
struct SSV_PACKED_STRUCT ssv6xxx_ieee80211_bss {
    struct cfg_80211_ssid ssid;
    ssv_type_u16 capab_info;
    ssv_type_u16 parameter_set_count;

    ETHER_ADDR	bssid;

    ssv_type_u8 dtim_period;
    ssv_type_u8 wmm_used;
    ssv_type_u8 uapsd_supported;
    ssv_type_u8 channel_id;
    ssv_type_u8 channel_offset;

    //RSN
    ssv_type_u16 proto;
    ssv_type_u32	group_cipher;
    ssv_type_u32	pairwise_cipher[2];

    ssv_type_u32	last_probe_resp;

    ssv_type_u8 supp_rates[IEEE80211_MAX_SUPP_RATES];
    ssv_type_u8 supp_rates_len;

    /*
     * During association, we save an ERP value from a probe response so
     * that we can feed ERP info to the driver when handling the
     * association completes. these fields probably won't be up-to-date
     * otherwise, you probably don't want to use them.
     */
    ssv_type_u8 has_erp_value;
    ssv_type_u8 erp_value;

    ssv_type_u8 ht_used;
    struct ssv_ht_cap ht_cap;
    struct ssv6xxx_rxphy_info_padding rxphypad;
    ssv_type_u32 prev_rcpi;
}SSV_PACKED_STRUCT_STRUCT;
SSV_PACKED_STRUCT_END

SSV_PACKED_STRUCT_BEGIN  
struct SSV_PACKED_STRUCT ssv_pack_check{
	le16_ssv_type elm1_2; // 2
	ssv_type_u8 elm2_1;
	ssv_type_u8 elm3[16];
	le16_ssv_type elm4_2; // 2
	le32_ssv_type elm5_4; // 4
	ssv_type_u8 elm6_1; // 1
}SSV_PACKED_STRUCT_STRUCT;
SSV_PACKED_STRUCT_END

struct st_tempe_table 
{
    ssv_type_u8 band_gain[7];   //0:ch1~ch2, 1:ch2~ch3, ..., 6:ch13~ch14
   
    ssv_type_u8 freq_xi;
    ssv_type_u8 freq_xo;
    
    ssv_type_u8 ldo_rxafe;
    ssv_type_u8 ldo_dcdcv;
    ssv_type_u8 ldo_dldov;
    
    ssv_type_u8 pa_vcas1;
    ssv_type_u8 pa_vcas2;
    ssv_type_u8 pa_vcas3;
    ssv_type_u8 pa_bias;
    ssv_type_u8 pa_cap;
    
    ssv_type_u8 padpd_cali;
};

struct st_rate_gain 
{
    ssv_type_u8 rate1;
    ssv_type_u8 rate2;
    ssv_type_u8 rate3;
    ssv_type_u8 rate4;
};

struct st_tempe_5g_table 
{
    ssv_type_u8 bbscale_band0;   //Band0 in 0xccb0ada8(31-24)
    ssv_type_u8 bbscale_band1;   //Band1 in 0xccb0ada8(23-16)
    ssv_type_u8 bbscale_band2;   //Band2 in 0xccb0ada8(15- 8)
    ssv_type_u8 bbscale_band3;   //Band3 in 0xccb0ada8( 7- 0)
    ssv_type_u32 bias1;                  // Band0 in 0xccb0a62c(15-0), Band1 in 0xccb0a62c(31-16)
    ssv_type_u32 bias2;                  // Band2 in 0xccb0a630(15-0), Band3 in 0xccb0a630(31-16)   
};

struct st_rf_table 
{
    struct st_tempe_table rt_config;
    struct st_tempe_table ht_config;
    struct st_tempe_table lt_config; 
  
    ssv_type_u8 rf_gain;
    ssv_type_u8 rate_gain_b;

    struct st_rate_gain rate_config_g;

    struct st_rate_gain rate_config_20n;

    struct st_rate_gain rate_config_40n;
    char low_boundary;
    char high_boundary;
    /*0xFF: EN_FIRST_BOOT, 0:EN_NOT_FIST_BOOT*/
    ssv_type_u8 boot_flag;
    /*0:EN_WORK_NOMAL, 1:EN_WORK_ENGINEER*/
    ssv_type_u8 work_mode; 
    struct st_tempe_5g_table rt_5g_config;
    struct st_tempe_5g_table ht_5g_config;
    struct st_tempe_5g_table lt_5g_config;
    ssv_type_u16 band_f0_threshold;  
    ssv_type_u16 band_f1_threshold;
    ssv_type_u16 band_f2_threshold; 
};

enum{
    EN_FIRST_BOOT=0xFF,
    EN_NOT_FIRST_BOOT=0
};

enum{
    EN_WORK_NOMAL=0,
    EN_WORK_ENGINEER
};

#endif /* _COMMON_H_ */

