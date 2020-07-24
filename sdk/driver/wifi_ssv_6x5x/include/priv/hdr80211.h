/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _HDR80211_H_
#define _HDR80211_H_
#include "ssv_common.h"


#define MAC_ADDR_LEN				6

#define FCS_LEN                     4
#define HDR80211_MGMT_LEN           24
#define IEEE80211_QOS_CTL_LEN       2
#define IEEE80211_HT_CTL_LEN        4

/**
 * IEEE 802.11 Address offset position. This defines the offset of Address1,
 * Address2, Address3 and Address4 if present.
 */
#define OFFSET_ADDR1                        4
#define OFFSET_ADDR2                        10
#define OFFSET_ADDR3                        16
#define OFFSET_ADDR4                        24


/* IEEE 802.11 Frame Control: */
#define M_FC_VER		            0x0003
#define M_FC_FTYPE		            0x000c
#define M_FC_STYPE		            0x00f0
#define M_FC_TODS		            0x0100
#define M_FC_FROMDS		            0x0200
#define M_FC_MOREFRAGS	            0x0400
#define M_FC_RETRY		            0x0800
#define M_FC_PWRMGMT	            0x1000
#define M_FC_MOREDATA		        0x2000
#define M_FC_PROTECTED	            0x4000
#define M_FC_ORDER		            0x8000

#define M_SC_FRAG		            0x000F
#define M_SC_SEQ		            0xFFF0

/* Frame Type: */
#define FT_MGMT		                0x0000
#define FT_CTRL		                0x0004
#define FT_DATA		                0x0008



/* Frame SubType: Management Frames */
#define FST_ASSOC_REQ	            0x0000
#define FST_ASSOC_RESP	            0x0010
#define FST_REASSOC_REQ	            0x0020
#define FST_REASSOC_RESP	        0x0030
#define FST_PROBE_REQ	            0x0040
#define FST_PROBE_RESP	            0x0050
#define FST_BEACON		            0x0080
#define FST_ATIM		            0x0090
#define FST_DISASSOC	            0x00A0
#define FST_AUTH		            0x00B0
#define FST_DEAUTH		            0x00C0
#define FST_ACTION		            0x00D0

/* Frame SubType: Control Frames */
#define FST_CONTROL_WRAPPER         0x0070
#define FST_BA_REQ	                0x0080
#define FST_BA	                    0x0090
#define FST_PSPOLL		            0x00A0
#define FST_RTS		                0x00B0
#define FST_CTS		                0x00C0
#define FST_ACK		                0x00D0
#define FST_CFEND		            0x00E0
#define FST_CFENDACK	            0x00F0

/* Frame SubType: Data Frames */
#define FST_DATA			        0x0000
#define FST_DATA_CFACK		        0x0010
#define FST_DATA_CFPOLL		        0x0020
#define FST_DATA_CFACKPOLL		    0x0030
#define FST_NULLFUNC		        0x0040
#define FST_CFACK			        0x0050
#define FST_CFPOLL			        0x0060
#define FST_CFACKPOLL		        0x0070
#define FST_QOS_DATA		        0x0080
#define FST_QOS_DATA_CFACK		    0x0090
#define FST_QOS_DATA_CFPOLL		    0x00A0
#define FST_QOS_DATA_CFACKPOLL	    0x00B0
#define FST_QOS_NULLFUNC		    0x00C0
#define FST_QOS_CFACK		        0x00D0
#define FST_QOS_CFPOLL		        0x00E0
#define FST_QOS_CFACKPOLL		    0x00F0


#define FC_TYPE(fc)           ((fc) & (M_FC_FTYPE|M_FC_STYPE))
#define FC_FTYPE(fc)          ((fc) & M_FC_FTYPE)
#define FC_STYPE(fc)          ((fc) & M_FC_STYPE)


#define IS_TODS_SET(fc)       ((fc) & M_FC_TODS)
#define IS_FROMDS_SET(fc)     ((fc) & M_FC_FROMDS)
#define IS_MOREFLAG_SET(fc)   ((fc) & M_FC_MOREFRAGS)
#define IS_RETRY_SET(fc)      ((fc) & M_FC_RETRY)
#define IS_PM_SET(fc)         ((fc) & M_FC_PWRMGMT)
#define IS_MOREDATA_SET(fc)   ((fc) & M_FC_MOREDATA)
#define IS_PROTECT_SET(fc)    ((fc) & M_FC_PROTECTED)
#define IS_ORDER_SET(fc)      ((fc) & M_FC_ORDER)
#define IS_4ADDR_FORMAT(fc)   SSV_IS_EQUAL(((fc)&(M_FC_TODS|M_FC_FROMDS)), (M_FC_TODS|M_FC_FROMDS))
#define IS_MGMT_FRAME(fc)     SSV_IS_EQUAL(FC_FTYPE(fc), FT_MGMT)
#define IS_CTRL_FRAME(fc)     SSV_IS_EQUAL(FC_FTYPE(fc), FT_CTRL)
#define IS_DATA_FRAME(fc)     SSV_IS_EQUAL(FC_FTYPE(fc), FT_DATA)
#define IS_QOS_DATA(fc)       SSV_IS_EQUAL(((fc)&(M_FC_FTYPE|FST_QOS_DATA)), (FT_DATA|FST_QOS_DATA))
#define IS_NULL_DATA(fc)      SSV_IS_EQUAL((fc)&(M_FC_FTYPE|FST_NULLFUNC), (FT_DATA|FST_NULLFUNC))
#define IS_QOS_NULL_DATA(fc)  SSV_IS_EQUAL((fc)&(M_FC_FTYPE|FST_NULLFUNC|FST_QOS_DATA), (FT_DATA|FST_NULLFUNC|FST_QOS_DATA)) 



#define IS_ASSOC_REQ(fc)      SSV_IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_ASSOC_REQ))
#define IS_ASSOC_RESP(fc)     SSV_IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_ASSOC_RESP))
#define IS_REASSOC_REQ(fc)    SSV_IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_REASSOC_REQ))
#define IS_REASSOC_RESP(fc)   SSV_IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_REASSOC_RESP))
#define IS_PROBE_REQ(fc)      SSV_IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_PROBE_REQ))
#define IS_PROBE_RESP(fc)     SSV_IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_PROBE_RESP))
#define IS_BEACON(fc)         SSV_IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_BEACON))
#define IS_ATIM(fc)           SSV_IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_ATIM))
#define IS_DISASSOC(fc)       SSV_IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_DISASSOC))
#define IS_AUTH(fc)           SSV_IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_AUTH))
#define IS_DEAUTH(fc)         SSV_IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_DEAUTH))
#define IS_ACTION(fc)         SSV_IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_ACTION))
#define IS_PSPOLL(fc)         SSV_IS_EQUAL(FC_TYPE(fc), (FT_CTRL|FST_PSPOLL))
#define IS_RTS(fc)            SSV_IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_RTS))
#define IS_CTS(fc)            SSV_IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_CTS))
#define IS_ACK(fc)            SSV_IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_ACK))
#define IS_BAR(fc)            SSV_IS_EQUAL(FC_TYPE(fc), (FT_CTRL|FST_BA_REQ))
#define IS_BA(fc)             SSV_IS_EQUAL(FC_TYPE(fc), (FT_CTRL|FST_BA))

#define GET_FC_MOREFRAG(fc)	  (((fc) & 0x0400) >> 10)

SSV_PACKED_STRUCT_BEGIN  
struct SSV_PACKED_STRUCT FC_Field_st {
    ssv_type_u32                 ver:2;
    ssv_type_u32                 type:2;
    ssv_type_u32                 subtype:4;
    ssv_type_u32                 toDS:1;
    ssv_type_u32                 fromDS:1;
    ssv_type_u32                 MoreFlag:1;
    ssv_type_u32                 Retry:1;
    ssv_type_u32                 PwrMgmt:1;
    ssv_type_u32                 MoreData:1;
    ssv_type_u32                 Protected:1;
    ssv_type_u32                 order:1;    

}SSV_PACKED_STRUCT_STRUCT;
SSV_PACKED_STRUCT_END
    
typedef struct FC_Field_st FC_Field, *PFC_Field;

typedef struct QOS_Ctrl_st {
    ssv_type_u32                 tid:4;
    ssv_type_u32                 bit4:1;
    ssv_type_u32                 ack_policy:2;
    ssv_type_u32                 rsvd:1;
    ssv_type_u32                 bit8_15:8;

} QOSCtrl_Field, *PQOSCtrl_Field;

typedef struct HDR80211_Data_st {
	FC_Field            fc;
	ssv_type_u16                 dur;
	ETHER_ADDR          addr1;
	ETHER_ADDR          addr2;
	ETHER_ADDR          addr3;
	ssv_type_u16                 seq_ctrl;    
    
} HDR80211_Data, *PHDR80211_Data;

typedef struct HDR80211_Data_4addr_st {
	FC_Field            fc;
	ssv_type_u16                 dur;
	ETHER_ADDR          addr1;
	ETHER_ADDR          addr2;
	ETHER_ADDR          addr3;
	ssv_type_u16                 seq_ctrl;        
    ETHER_ADDR          addr4;

} HDR80211_Data_4addr, *PHDR80211_Data_4addr;

#define BASE_LEN(PTR2variable,PTR)			(PTR2variable - (ssv_type_u8 *) PTR)

typedef SSV_PACKED_STRUCT_BEGIN  struct HDR80211_Mgmt_st {
    FC_Field            fc;
    ssv_type_u16                 dur;
    ETHER_ADDR          da;
    ETHER_ADDR          sa;
    ETHER_ADDR          bssid;
    ssv_type_u16                 seq_ctrl;

    SSV_PACKED_STRUCT_BEGIN  
    union {
    
        SSV_PACKED_STRUCT_BEGIN   
        struct SSV_PACKED_STRUCT{
            ssv_type_u16         auth_algo;
            ssv_type_u16         trans_id;
            ssv_type_u16         status_code;
            /* possibly followed by Challenge text */
            ssv_type_u8          variable[0];    
        }SSV_PACKED_STRUCT_STRUCT auth;
        SSV_PACKED_STRUCT_END
            
        SSV_PACKED_STRUCT_BEGIN  
        struct SSV_PACKED_STRUCT{
            ssv_type_u16         reason_code;
        }SSV_PACKED_STRUCT_STRUCT deauth;
        SSV_PACKED_STRUCT_END
            
        SSV_PACKED_STRUCT_BEGIN  
        struct SSV_PACKED_STRUCT{
            ssv_type_u16         capab_info;
            ssv_type_u16         listen_interval;
            /* followed by SSID and Supported rates */
            ssv_type_u8          variable[0];
        }SSV_PACKED_STRUCT_STRUCT assoc_req;
        SSV_PACKED_STRUCT_END
            
        SSV_PACKED_STRUCT_BEGIN  
        struct SSV_PACKED_STRUCT{
			ssv_type_u16         capab_info;
			ssv_type_u16         status_code;
			ssv_type_u16         aid;
			/* followed by Supported rates */
			ssv_type_u8 variable[0];
		}SSV_PACKED_STRUCT_STRUCT assoc_resp, reassoc_resp;
        SSV_PACKED_STRUCT_END
            
        SSV_PACKED_STRUCT_BEGIN  
        struct SSV_PACKED_STRUCT{
			ssv_type_u16         capab_info;
			ssv_type_u16         listen_interval;
			ssv_type_u8          current_ap[6];
			/* followed by SSID and Supported rates */
			ssv_type_u8 variable[0];
		}SSV_PACKED_STRUCT_STRUCT reassoc_req;
        SSV_PACKED_STRUCT_END
            
        SSV_PACKED_STRUCT_BEGIN  
        struct SSV_PACKED_STRUCT{
			ssv_type_u16         reason_code;
		}SSV_PACKED_STRUCT_STRUCT disassoc;
        SSV_PACKED_STRUCT_END
            
        SSV_PACKED_STRUCT_BEGIN  
        struct SSV_PACKED_STRUCT{
			ssv_type_u64         timestamp;
			ssv_type_u16         beacon_int;
			ssv_type_u16         capab_info;
			/* followed by some of SSID, Supported rates,
			 * FH Params, DS Params, CF Params, IBSS Params, TIM */
			ssv_type_u8 variable[0];
		}SSV_PACKED_STRUCT_STRUCT beacon;
        SSV_PACKED_STRUCT_END
            
		SSV_PACKED_STRUCT_BEGIN  
        struct SSV_PACKED_STRUCT{ /*lint -save -e43 */
			/* only variable items: SSID, Supported rates */
			ssv_type_u8 variable[0];
		}SSV_PACKED_STRUCT_STRUCT probe_req; /*lint -restore */
        SSV_PACKED_STRUCT_END
            
		SSV_PACKED_STRUCT_BEGIN  
        struct SSV_PACKED_STRUCT{
			ssv_type_u64         timestamp;
			ssv_type_u16         beacon_int;
			ssv_type_u16         capab_info;
			/* followed by some of SSID, Supported rates,
			 * FH Params, DS Params, CF Params, IBSS Params */
			ssv_type_u8 variable[0];
		}SSV_PACKED_STRUCT_STRUCT  probe_resp;
        SSV_PACKED_STRUCT_END
            
        SSV_PACKED_STRUCT_BEGIN  
        struct SSV_PACKED_STRUCT{
            ssv_type_u8          category;
            SSV_PACKED_STRUCT_BEGIN  
            union {
                SSV_PACKED_STRUCT_BEGIN  
                struct SSV_PACKED_STRUCT{
					ssv_type_u8  action_code;
					ssv_type_u8  dialog_token;
					ssv_type_u8  status_code;
					ssv_type_u8  variable[0];
				}SSV_PACKED_STRUCT_STRUCT  wme_action;
                SSV_PACKED_STRUCT_END
                    
				SSV_PACKED_STRUCT_BEGIN  
                struct SSV_PACKED_STRUCT{
					ssv_type_u8  action_code;
					ssv_type_u8  element_id;
					ssv_type_u8  length;
//					struct ieee80211_channel_sw_ie sw_elem;
				}SSV_PACKED_STRUCT_STRUCT chan_switch;
                SSV_PACKED_STRUCT_END
                    
                SSV_PACKED_STRUCT_BEGIN  
                struct SSV_PACKED_STRUCT{
					ssv_type_u8  action_code;
					ssv_type_u8  dialog_token;
					ssv_type_u8  element_id;
					ssv_type_u8  length;
//					struct ieee80211_msrment_ie msr_elem;
				}SSV_PACKED_STRUCT_STRUCT measurement;
                SSV_PACKED_STRUCT_END
                    
				SSV_PACKED_STRUCT_BEGIN  
                struct SSV_PACKED_STRUCT{
					ssv_type_u8  action_code;
					ssv_type_u8  dialog_token;
					ssv_type_u16 capab;
					ssv_type_u16 timeout;
					ssv_type_u16 start_seq_num;
				}SSV_PACKED_STRUCT_STRUCT addba_req;
                SSV_PACKED_STRUCT_END
                    
				SSV_PACKED_STRUCT_BEGIN  
                struct SSV_PACKED_STRUCT{
					ssv_type_u8  action_code;
					ssv_type_u8  dialog_token;
					ssv_type_u16 status;
					ssv_type_u16 capab;
					ssv_type_u16 timeout;
				}SSV_PACKED_STRUCT_STRUCT addba_resp;
                SSV_PACKED_STRUCT_END
                    
                SSV_PACKED_STRUCT_BEGIN  
                struct SSV_PACKED_STRUCT{
					ssv_type_u8  action_code;
					ssv_type_u16 params;
					ssv_type_u16 reason_code;
				}SSV_PACKED_STRUCT_STRUCT delba;
                SSV_PACKED_STRUCT_END
                    
				SSV_PACKED_STRUCT_BEGIN  
                struct SSV_PACKED_STRUCT{
					ssv_type_u8  action_code;
					/* capab_info for open and confirm,
					 * reason for close
					 */
					ssv_type_u16 aux;
					/* Followed in plink_confirm by status
					 * code, AID and supported rates,
					 * and directly by supported rates in
					 * plink_open and plink_close
					 */
					ssv_type_u8  variable[0];
				}SSV_PACKED_STRUCT_STRUCT plink_action;
                SSV_PACKED_STRUCT_END
                    
                SSV_PACKED_STRUCT_BEGIN  
                struct SSV_PACKED_STRUCT{
					ssv_type_u8  action_code;
					ssv_type_u8  variable[0];
				}SSV_PACKED_STRUCT_STRUCT mesh_action;
                SSV_PACKED_STRUCT_END
                    
				SSV_PACKED_STRUCT_BEGIN  
                struct SSV_PACKED_STRUCT{
					ssv_type_u8  action;
//					u8  trans_id[WLAN_SA_QUERY_TR_ID_LEN];
				}SSV_PACKED_STRUCT_STRUCT sa_query;
                SSV_PACKED_STRUCT_END
                    
				SSV_PACKED_STRUCT_BEGIN  
                struct SSV_PACKED_STRUCT{
					ssv_type_u8  action;
					ssv_type_u8  smps_control;
				}SSV_PACKED_STRUCT_STRUCT ht_smps;
                SSV_PACKED_STRUCT_END
                    
            }SSV_PACKED_STRUCT_STRUCT u;
            SSV_PACKED_STRUCT_END
        }SSV_PACKED_STRUCT_STRUCT action;
        SSV_PACKED_STRUCT_END
            
    }SSV_PACKED_STRUCT_STRUCT u;
    SSV_PACKED_STRUCT_END
    
}SSV_PACKED_STRUCT_STRUCT HDR80211_Mgmt, *PHDR80211_Mgmt;
SSV_PACKED_STRUCT_END

typedef struct HDR80211_Ctrl_st 
{
    FC_Field            fc;
    ssv_type_u16                 dur;
    ETHER_ADDR          ra;
    ETHER_ADDR          ta;

} HDR80211_Ctrl, *PHDR80211_Ctrl;


typedef struct HDR8023_Data_st {
	ETHER_ADDR			dest;
	ETHER_ADDR			src;
	ssv_type_u16					protocol;
} HDR8023_Data, *PHDR8023_Data;


#define HDR80211_MGMT(x)    /*lint -save -e740 */ (HDR80211_Mgmt *)((ssv_type_u8 *)(x)+((PKT_Info *)(x))->hdr_offset) /*lint -restore */
#define HDR80211_CTRL(x)
#define HDR80211_DATA(x)





/* Define WLAN Cipher Suite */
#define CIPHER_SUITE_NONE                   0
#define CIPHER_SUITE_WEP40                  1
#define CIPHER_SUITE_WEP104                 2
#define CIPHER_SUITE_TKIP                   3
#define CIPHER_SUITE_CCMP                   4



/**
 * WLAN Operation Mode Definition (for wlan_mode field):
 *
 * @ WLAN_STA: operate as STA (infrastructure) mode
 * @ WLAN_AP: operate as AP (infrastructure) mode
 * @ WLAN_IBSS: operate as IBSS (ad-hoc) mode
 * @ WLAN_WDS: Wireless Distribution System mode (Wireless Bridge)
 */
#define WLAN_STA                            0
#define WLAN_AP                             1
#define WLAN_IBSS                           2
#define WLAN_WDS                            3




/**
 * HT Mode Definition (for ht_mode field):
 * 
 * @ HT_NONE
 * @ HT_MF
 * @ HT_GF
 */
#define HT_NONE                         0
#define HT_MF                           1
#define HT_GF                           2


#if 0
static u8 *GET_QOS_CTRL(HDR80211_Data_4addr *hdr)
{
	if (hdr->fc.fromDS && hdr->fc.toDS)
		return (u8 *)hdr + 30;
	else
		return (u8 *)hdr + 24;
}
#endif

#endif /* _HDR80211_H_ */



