/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#define __SFILE__ "ap.mlme.c"

#include <pbuf.h>

#include <ssv_dev.h>
#include <ssv_hal.h>
#include "common/ieee802_11_defs.h"
#include <ieee80211.h>
#include "common/ieee802_11_common.h"
#include "ieee802_11_mgmt.h"
#include "ieee802_11_ht.h"

#include "ap_mlme.h"
#include "ap_config.h"
#include "crypto/crypto.h"

#ifndef CONFIG_NO_WPA2
#include "wpa_auth.h"
#endif
#include "ap_sta_info.h"
#include "ap_info.h"
#include "beacon.h"
#include "ap_def.h"
#include "wmm.h"
#include "ap_rx.h"
#include "ap_drv_cmd.h"
#include <ssv_timer.h>
#include <ssv_devinfo.h>

#include <host_apis.h>

#if (AP_MODE_ENABLE == 1)

#ifdef __AP_DEBUG__
	extern void APStaInfo_PrintStaInfo(ssv_type_u32 idx);
#endif//__AP_DEBUG__

extern void APStaInfo_free(ApInfo_st *pApInfo, APStaInfo_st *sta);
extern struct Host_cfg g_host_cfg;
extern struct task_info_st g_host_task_info[];

#undef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

/*
* Compact form for string representation of MAC address
* To be used, e.g., for constructing dbus paths for P2P Devices
*/
//#define COMPACT_MACSTR "%02x%02x%02x%02x%02x%02x"

#ifndef CONFIG_NO_WPA2

static ssv_inline unsigned short wpa_swap_16(unsigned short v)
{
	return ((v & 0xff) << 8) | (v >> 8);
}
#endif


extern void _HCmdEng_TxHdlData(void *frame ,ssv_type_bool bAPFrame, ssv_type_u32 TxFlags);












#if 0
#define S_SWAP(a,b) do { u8 t = S[a]; S[a] = S[b]; S[b] = t; } while(0)

int rc4_skip(const u8 *key, size_t keylen, size_t skip,
	     u8 *data, size_t data_len)
{
	u32 i, j, k;
	u8 S[256], *pos;
	size_t kpos;

	/* Setup RC4 state */
	for (i = 0; i < 256; i++)
		S[i] = i;
	j = 0;
	kpos = 0;
	for (i = 0; i < 256; i++) {
		j = (j + S[i] + key[kpos]) & 0xff;
		kpos++;
		if (kpos >= keylen)
			kpos = 0;
		S_SWAP(i, j);
	}

	/* Skip the start of the stream */
	i = j = 0;
	for (k = 0; k < skip; k++) {
		i = (i + 1) & 0xff;
		j = (j + S[i]) & 0xff;
		S_SWAP(i, j);
	}

	/* Apply RC4 to data */
	pos = data;
	for (k = 0; k < data_len; k++) {
		i = (i + 1) & 0xff;
		j = (j + S[i]) & 0xff;
		S_SWAP(i, j);
		*pos++ ^= S[(S[i] + S[j]) & 0xff];
	}

	return 0;
}
#endif

//------------------

static ssv_type_u16 auth_shared_key(struct ApInfo *pApInfo, APStaInfo_st *sta,
			   ssv_type_u16 auth_transaction, const ssv_type_u8 *challenge,
			   int iswep)
{
#ifdef __AP_DEBUG__
	LOG_TRACE("station authentication (shared key, transaction %d)\n", auth_transaction);
#endif

	if (auth_transaction == 1) {
		if (!sta->challenge) {
			/* Generate a pseudo-random challenge */
			ssv_type_u8 key[8];
			
			ssv_type_u32 tick;
            unsigned long r;
			sta->challenge = (ssv_type_u8*)OS_MemAlloc(WLAN_AUTH_CHALLENGE_LEN);
			if (sta->challenge == NULL)
				return WLAN_STATUS_UNSPECIFIED_FAILURE;
			
			tick = OS_GetSysTick();
			r = OS_Random();
			OS_MemCPY(key, &tick, 4);
			OS_MemCPY(key + 4, &r, 4);
			rc4_skip(key, sizeof(key), 0,
				 sta->challenge, WLAN_AUTH_CHALLENGE_LEN);
		}
		return 0;
	}

	if (auth_transaction != 3)
		return WLAN_STATUS_UNSPECIFIED_FAILURE;

	/* Transaction 3 */
	if (!iswep || !sta->challenge || !challenge ||
	    ssv6xxx_memcmp(sta->challenge, challenge, WLAN_AUTH_CHALLENGE_LEN)) {
		LOG_INFO("shared key authentication - invalid challenge-response. \n");
		return WLAN_STATUS_CHALLENGE_FAIL;
	}
#ifdef __AP_DEBUG__
	LOG_DEBUG("authentication OK (shared key). \n");
#endif
#ifdef IEEE80211_REQUIRE_AUTH_ACK
	/* Station will be marked authenticated if it ACKs the
	 * authentication reply. */
#else
	OS_MutexLock(sta->apsta_mutex);
	set_sta_flag(sta, WLAN_STA_AUTH);
    OS_MutexUnLock(sta->apsta_mutex);

	//wpa_auth_sm_event(sta->wpa_sm, WPA_AUTH);
#endif
	OS_MemFree(sta->challenge);
	sta->challenge = NULL;

	return 0;
}





//-------------------------------------------------------------------------------------------------------------------




static int wpa_driver_nl80211_send_mlme(const ssv_type_u8 *data,
					ssv_type_size_t data_len, ssv_type_bool bMgmtCopy)
{
	struct ieee80211_mgmt *mgmt;
    //lint -e550
	int encrypt = 1;
	ssv_type_u16 fc;

	mgmt = (struct ieee80211_mgmt *) data;
	fc = le_to_host16(mgmt->frame_control);
    
    #ifdef __AP_DEBUG__
	LOG_DEBUG("Send MLME Length %d\r\n", data_len);
    #endif

	if(data_len > AP_MGMT_PKT_LEN)
	{
		LOG_FATAL("Need to resize pack buffer size,%d\r\n",data_len);
	}
	else
	{
	    #ifdef __AP_DEBUG__
		LOG_TRACE("Send MLME Type->%02x SubType->%04x data len %d\r\n", 
			WLAN_FC_GET_TYPE(fc), WLAN_FC_GET_STYPE(fc), data_len);
        #endif
	}



	if (WLAN_FC_GET_TYPE(fc) == WLAN_FC_TYPE_MGMT &&
	    WLAN_FC_GET_STYPE(fc) == WLAN_FC_STYPE_AUTH) {
		/*
		 * Only one of the authentication frame types is encrypted.
		 * In order for static WEP encryption to work properly (i.e.,
		 * to not encrypt the frame), we need to tell mac80211 about
		 * the frames that must not be encrypted.
		 */
		ssv_type_u16 auth_alg = le_to_host16(mgmt->u.auth.auth_alg);
		ssv_type_u16 auth_trans = le_to_host16(mgmt->u.auth.auth_transaction);
		if (auth_alg != WLAN_AUTH_SHARED_KEY || auth_trans != 3)
			encrypt = 0;
	}

	//return wpa_driver_nl80211_send_frame(drv, data, data_len, encrypt);


	


	if(bMgmtCopy)
		OS_MemCPY(gDeviceInfo->APInfo->pMgmtPkt, data, data_len);

	return ap_soc_data_send(gDeviceInfo->APInfo->pOSMgmtframe, data_len, TRUE, 0);
	
	
}



int i802_sta_deauth(const ssv_type_u8 *own_addr, const ssv_type_u8 *addr,
	int reason)
{
	//struct i802_bss *bss = priv;
	struct ieee80211_mgmt mgmt;

	OS_MemSET((void*)&mgmt, 0, sizeof(mgmt));
	mgmt.frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
		WLAN_FC_STYPE_DEAUTH);
	OS_MemCPY((void*)mgmt.da, (void*)addr, ETH_ALEN);
	OS_MemCPY((void*)mgmt.sa, (void*)own_addr, ETH_ALEN);
	OS_MemCPY((void*)mgmt.bssid, (void*)own_addr, ETH_ALEN);
	mgmt.u.deauth.reason_code = host_to_le16(reason);
	return wpa_driver_nl80211_send_mlme((ssv_type_u8 *) &mgmt,
		IEEE80211_HDRLEN + sizeof(mgmt.u.deauth),
		TRUE);
}


int i802_sta_disassoc(const ssv_type_u8 *own_addr, const ssv_type_u8 *addr,
	int reason)
{
	//struct i802_bss *bss = priv;
	struct ieee80211_mgmt mgmt;

	OS_MemSET((void*)&mgmt, 0, sizeof(mgmt));
	mgmt.frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
		WLAN_FC_STYPE_DISASSOC);
	OS_MemCPY((void*)mgmt.da, (void*)addr, ETH_ALEN);
	OS_MemCPY((void*)mgmt.sa, (void*)own_addr, ETH_ALEN);
	OS_MemCPY((void*)mgmt.bssid, (void*)own_addr, ETH_ALEN);
	mgmt.u.disassoc.reason_code = host_to_le16(reason);
	return wpa_driver_nl80211_send_mlme((ssv_type_u8 *) &mgmt,
		IEEE80211_HDRLEN + sizeof(mgmt.u.disassoc),
		TRUE);
}




int  nl80211_poll_client(const ssv_type_u8 *own_addr, const ssv_type_u8 *addr,
	int qos)
{
	//struct i802_bss *bss = priv;
	SSV_PACKED_STRUCT_BEGIN 
	struct SSV_PACKED_STRUCT{
		struct ieee80211_hdr hdr;
		ssv_type_u16 qos_ctl;
	}SSV_PACKED_STRUCT_STRUCT nulldata;
    SSV_PACKED_STRUCT_END
	ssv_type_size_t size;

	/* Send data frame to poll STA and check whether this frame is ACKed */

	OS_MemSET((void*)&nulldata, 0, sizeof(nulldata));

	if (qos) {
		nulldata.hdr.frame_control =
			IEEE80211_FC(WLAN_FC_TYPE_DATA,
			WLAN_FC_STYPE_QOS_NULL);
		size = sizeof(nulldata);
	} else {
		nulldata.hdr.frame_control =
			IEEE80211_FC(WLAN_FC_TYPE_DATA,
			WLAN_FC_STYPE_NULLFUNC);
		size = sizeof(struct ieee80211_hdr);
	}

	nulldata.hdr.frame_control |= host_to_le16(WLAN_FC_FROMDS);
	OS_MemCPY((void*)nulldata.hdr.IEEE80211_DA_FROMDS, (void*)addr, ETH_ALEN);
	OS_MemCPY((void*)nulldata.hdr.IEEE80211_BSSID_FROMDS, (void*)own_addr, ETH_ALEN);
	OS_MemCPY((void*)nulldata.hdr.IEEE80211_SA_FROMDS, (void*)own_addr, ETH_ALEN);

	return wpa_driver_nl80211_send_mlme((ssv_type_u8 *) &nulldata, size, TRUE);
	
}






//=--------------------------------------------------------------------------------

static void send_assoc_resp(struct ApInfo *pApInfo, APStaInfo_st *sta,
			    ssv_type_u16 status_code, int reassoc, const ssv_type_u8 *ies,
			    ssv_type_size_t ies_len)
{
	ssv_type_u32 send_len;
	//u8 *buf = //[sizeof(struct ieee80211_mgmt) + 1024];
	struct ieee80211_mgmt *reply = (struct ieee80211_mgmt *)pApInfo->pMgmtPkt;
	ssv_type_u8 *p;

	reply->frame_control =
		IEEE80211_FC(WLAN_FC_TYPE_MGMT,
			     (reassoc ? WLAN_FC_STYPE_REASSOC_RESP :
			      WLAN_FC_STYPE_ASSOC_RESP));
	OS_MemCPY((void*)reply->da, (void*)sta->addr, ETH_ALEN);
	OS_MemCPY((void*)reply->sa, (void*)pApInfo->own_addr, ETH_ALEN);
	OS_MemCPY((void*)reply->bssid, (void*)pApInfo->own_addr, ETH_ALEN);
   reply->seq_ctrl = 0x0010;
	send_len = IEEE80211_HDRLEN;
	send_len += sizeof(reply->u.assoc_resp);
	reply->u.assoc_resp.capab_info =
		host_to_le16(hostapd_own_capab_info(pApInfo, sta, 0));
	reply->u.assoc_resp.status_code = host_to_le16(status_code);
	reply->u.assoc_resp.aid = host_to_le16((sta ? sta->aid : 0)
					       | BIT(14) | BIT(15));

//-----------------------
//Information element
	/* Supported rates */
	p = hostapd_eid_supp_rates(pApInfo, (ssv_type_u8*)reply->u.assoc_resp.variable);
	/* Extended supported rates */
	p = hostapd_eid_ext_supp_rates(pApInfo, p);
 
 #if AP_SUPPORT_80211N
 	p = hostapd_eid_ht_capabilities(pApInfo, p);
 	p = hostapd_eid_ht_operation(pApInfo, p);
 #endif /* CONFIG_IEEE80211N */


	if (test_sta_flag(sta, WLAN_STA_WMM))
		p = hostapd_eid_wmm(pApInfo, p);



	send_len += (ssv_type_u32)(p - reply->u.assoc_resp.variable);



	if (wpa_driver_nl80211_send_mlme( (ssv_type_u8 *) reply, send_len, FALSE) < 0)
		LOG_ERROR("Failed to send assoc resp:len=%d\r\n",send_len);
}







//Assoc
static void handle_assoc(struct ApInfo *pApInfo,
			 const struct ieee80211_mgmt *mgmt, ssv_type_size_t len,
			 int reassoc)
{
	ssv_type_u16 capab_info, listen_interval;
	ssv_type_u16 resp = WLAN_STATUS_SUCCESS;
	const ssv_type_u8 *pos;
	int left, i;
	APStaInfo_st *sta;

	if (len < IEEE80211_HDRLEN + (reassoc ? sizeof(mgmt->u.reassoc_req) :
		sizeof(mgmt->u.assoc_req))) {
			LOG_PRINTF("handle_assoc(reassoc=%d) - too short payload (len=%lu)"
				"\r\n", reassoc, (unsigned long) len);
			return;
	}

	if (reassoc) {
		capab_info = le_to_host16(mgmt->u.reassoc_req.capab_info);
		listen_interval = le_to_host16(
			mgmt->u.reassoc_req.listen_interval);
		LOG_DEBUG("reassociation request: STA=" MACSTR
			   " capab_info=0x%02x listen_interval=%d current_ap="
			   MACSTR "\r\n",
			   MAC2STR(mgmt->sa), capab_info, listen_interval,
			   MAC2STR(mgmt->u.reassoc_req.current_ap));
		left = len - (IEEE80211_HDRLEN + sizeof(mgmt->u.reassoc_req));
		pos = (ssv_type_u8 *)mgmt->u.reassoc_req.variable;
	} else {
		capab_info = le_to_host16(mgmt->u.assoc_req.capab_info);
		listen_interval = le_to_host16(
			mgmt->u.assoc_req.listen_interval);
        #ifdef __AP_DEBUG__
			LOG_DEBUG("association request: STA=" MACSTR
			   " capab_info=0x%02x listen_interval=%d\r\n",
			   MAC2STR(mgmt->sa), capab_info, listen_interval);
        #endif
            
		left = len - (IEEE80211_HDRLEN + sizeof(mgmt->u.assoc_req));
		pos = (ssv_type_u8 *)mgmt->u.assoc_req.variable;
	}

	sta = APStaInfo_FindStaByAddr((ETHER_ADDR *)mgmt->sa);

	if (sta == NULL || test_sta_flag(sta, WLAN_STA_AUTH) == 0) {
		LOG_INFO("Station tried to "
			"associate before authentication "
			"mac="MACSTR"(aid=%d flags=0x%x)\r\n",MAC2STR(mgmt->sa),
			sta ? sta->aid : -1,
			sta ? sta->_flags : 0);

        if(ssv_is_valid_ether_addr((const ssv_type_u8*)mgmt->sa))
		    i802_sta_deauth((ssv_type_u8 *)pApInfo->own_addr, (ssv_type_u8 *)mgmt->sa, WLAN_REASON_CLASS2_FRAME_FROM_NONAUTH_STA);
        else
            LOG_PRINTF("invalid mac addr\r\n");
        return;
	}

	if (pApInfo->tkip_countermeasures) {
		resp = WLAN_REASON_MIC_FAILURE;
		goto fail;
	}

	if (listen_interval > AP_MAX_LISTEN_INTERVAL) {
		LOG_DEBUG("Too large Listen Interval (%d)\r\n",
			       listen_interval);
		resp = WLAN_STATUS_ASSOC_DENIED_LISTEN_INT_TOO_LARGE;
		goto fail;
	}

	/* followed by SSID and Supported rates; and HT capabilities if 802.11n
	 * is used */
	resp = check_assoc_ies(pApInfo, sta, pos,(ssv_type_size_t) left, reassoc);
	if (resp != WLAN_STATUS_SUCCESS)
		goto fail;
//-------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------
//finish check. start to add station in this AP

 	if (hostapd_get_aid(pApInfo, sta) < 0) {									//---------->get aid------------------------------------------------------------------------>
		LOG_INFO("&s,No room for more AIDs\r\n",__func__);
		resp = WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA;
		goto fail;
 	}

	sta->capability = capab_info;
	sta->listen_interval = listen_interval;

	if (pApInfo->eCurrentApMode == AP_MODE_IEEE80211G)
    {
        OS_MutexLock(sta->apsta_mutex);
		set_sta_flag(sta, WLAN_STA_NONERP);
        OS_MutexUnLock(sta->apsta_mutex);
    }
	for (i = 0; i < sta->supported_rates_len; i++) {
		if ((sta->supported_rates[i] & 0x7f) > 22) {
            OS_MutexLock(sta->apsta_mutex);
			clear_sta_flag(sta, WLAN_STA_NONERP);
            OS_MutexUnLock(sta->apsta_mutex);
			break;
		}
	}

//No WMM
//	if (!test_sta_flag(sta, WLAN_STA_WMM))
		


	


    //Non ERP	
    if(g_host_cfg.erp==1)
    {
        if (test_sta_flag(sta, WLAN_STA_NONERP)&& !sta->nonerp_set) {
            sta->nonerp_set = 1;
            pApInfo->num_sta_non_erp++;
            if (pApInfo->num_sta_non_erp == 1)
            {      
                //Send ERP protect cmd
                ssv_type_bool erp_protect = TRUE;
                _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_ERP_PROTECT, &erp_protect, sizeof(ssv_type_bool),TRUE,FALSE);      

                ieee802_11_set_beacon(pApInfo,TRUE);
            }
        }  
    }
//Short Slot Time
	if (!(sta->capability & WLAN_CAPABILITY_SHORT_SLOT_TIME) &&
	    !sta->no_short_slot_time_set) {
		sta->no_short_slot_time_set = 1;
		pApInfo->num_sta_no_short_slot_time++;
		if (pApInfo->eCurrentApMode ==
		    AP_MODE_IEEE80211G &&
		    pApInfo->num_sta_no_short_slot_time == 1)
			//set slot time to register
		   ssv_hal_set_short_slot_time(FALSE);
			ieee802_11_set_beacon(pApInfo,TRUE);
	}

	if (sta->capability & WLAN_CAPABILITY_SHORT_PREAMBLE)
    {   
        OS_MutexLock(sta->apsta_mutex);
		set_sta_flag(sta, WLAN_STA_SHORT_PREAMBLE);
        OS_MutexUnLock(sta->apsta_mutex);
    }
	else
    {  
        
        OS_MutexLock(sta->apsta_mutex);
		clear_sta_flag(sta, WLAN_STA_SHORT_PREAMBLE);
        OS_MutexUnLock(sta->apsta_mutex);
    }

	if (!(sta->capability & WLAN_CAPABILITY_SHORT_PREAMBLE) &&
		!sta->no_short_preamble_set) {
			sta->no_short_preamble_set = 1;
			pApInfo->num_sta_no_short_preamble++;
			if (pApInfo->eCurrentApMode == AP_MODE_IEEE80211G
				&& pApInfo->num_sta_no_short_preamble == 1)
				ieee802_11_set_beacon(pApInfo,TRUE);
	}
 
 #if AP_SUPPORT_80211N
 	update_ht_state(pApInfo, sta);
 #endif /* CONFIG_IEEE80211N */

	LOG_DEBUG(MACSTR " association OK (aid %d)\r\n", MAC2STR(sta->addr), sta->aid);
//------------------------------------------------------------------------------------------------****************************************************************==>??	
	/* Station will be marked associated, after it acknowledges AssocResp
	 */
	OS_MutexLock(sta->apsta_mutex); 
	set_sta_flag(sta, WLAN_STA_ASSOC);
    OS_MutexUnLock(sta->apsta_mutex);


//	if ((!hapd->conf->ieee802_1x && !hapd->conf->wpa) ||
//   sta->auth_alg == WLAN_AUTH_FT) {
	/*
	 * Open, static WEP, or FT protocol; no separate authorization
	 * step.
	 */
    #ifndef CONFIG_NO_WPA2 
    if(gDeviceInfo->APInfo->wpa_auth.conf->wpa_key_mgmt != WPA_KEY_MGMT_PSK) 
    #endif	
    {
        ap_sta_set_authorized(pApInfo, sta, 1);
    }
	
//	wpa_msg(hapd->msg_ctx, MSG_INFO,
//		AP_STA_CONNECTED MACSTR, MAC2STR(sta->addr));
//}




	if (reassoc) {
		OS_MemCPY((void*)sta->previous_ap, (void*)mgmt->u.reassoc_req.current_ap,
			  ETH_ALEN);
	}
//Todo: check if no need
// 	if (sta->last_assoc_req)
// 		os_free(sta->last_assoc_req);
// 
// 	sta->last_assoc_req = os_malloc(len);
// 	if (sta->last_assoc_req)
// 		os_memcpy(sta->last_assoc_req, mgmt, len);

	/* Make sure that the previously registered inactivity timer will not
	 * remove the STA immediately. */
	sta->timeout_next = STA_NULLFUNC;

	//Set to driver
#ifndef CONFIG_NO_WPA2 
	sta->addStaFlag = 0;
	sta->assocRsp = 0;
	sta->reassoc = 0;
#endif	
	ap_soc_cmd_sta_oper(CFG_STA_ADD, sta, CFG_HT_NONE, test_sta_flag(sta, WLAN_STA_WMM));

    if((gDeviceInfo->APInfo->sec_type==SSV6XXX_SEC_WEP_40)||(gDeviceInfo->APInfo->sec_type==SSV6XXX_SEC_WEP_104))
    {
        
        ssv_hal_ap_wep_setting(gDeviceInfo->APInfo->sec_type,gDeviceInfo->APInfo->password,
                               ((ssv_vif*)gDeviceInfo->APInfo->vif)->idx, (ssv_type_u8*)&sta->addr[0]);
    }

fail:
    if((resp!=WLAN_STATUS_SUCCESS)&&(sta!=NULL))
		APStaInfo_free(pApInfo, sta);
    
	send_assoc_resp(pApInfo, sta, resp, reassoc, pos, (ssv_type_size_t)left);
#ifndef CONFIG_NO_WPA2
    if(resp == WLAN_STATUS_SUCCESS) 
    {
        struct wpa_auth_config *conf= (struct wpa_auth_config 
        *)(gDeviceInfo->APInfo->wpa_auth.conf);
        if(conf->wpa_key_mgmt == WPA_KEY_MGMT_PSK) 
        {
            process_4Ways_start(sta);                
            sta->assocRsp = 1;            
        }
            
    }
#endif    
}



static ssv_type_s32 ap_mlme_rx_assoc_request(struct ApInfo*pApInfo, struct ieee80211_mgmt *mgmt, ssv_type_u16 len, ssv_type_u8 bssid_idx)
{
	handle_assoc(pApInfo, mgmt, (ssv_type_size_t)len, 0);		
	return AP_MLME_OK;
}


// static s32 ap_mlme_rx_assoc_response(struct ApInfo*pApInfo, struct ieee80211_mgmt *mgmt, u16 len)
// {
// 	//handle_assoc(pApInfo, mgmt, len, 1);
// 	return AP_MLME_OK;
// }


static ssv_type_s32 ap_mlme_rx_reassoc_request(struct ApInfo*pApInfo, struct ieee80211_mgmt *mgmt, ssv_type_u16 len, ssv_type_u8 bssid_idx)
{
	handle_assoc(pApInfo, mgmt, (ssv_type_size_t)len, 1);
	return AP_MLME_OK;
}


// static s32 ap_mlme_rx_reassoc_response(struct ApInfo*pApInfo, struct ieee80211_mgmt *mgmt, u16 len)
// {
// 
// 	return AP_MLME_OK;
// }


ssv_type_u32 g_probe_req_cnt=0;
ssv_type_u32 B_tick=0;
ssv_type_u32 A_tick=0;
static void recover_probe_req(void *data1, void *data2)
{
    ssv_hal_drop_probe_request(FALSE);
    g_probe_req_cnt=0;
    //LOG_PRINTF("   R\r\n");

}
static ssv_type_s32 ap_mlme_rx_probe_request(struct ApInfo*pApInfo, struct ieee80211_mgmt *mgmt, ssv_type_u16 len, ssv_type_u8 bssid_idx)
{
	//PKT_TxInfo *pPktTxInfo;
	struct ieee80211_mgmt *resp_mgmt;
	struct ieee802_11_elems elems;
	char *ssid;
	ssv_type_u8 *pos, *epos;
	const ssv_type_u8 *ie;
	ssv_type_size_t ssid_len, ie_len;
	APStaInfo_st *sta = NULL;
//	size_t buflen;
//	size_t i;


	//size_t len = pPktInfo->len;

	ie = (ssv_type_u8 *)mgmt->u.probe_req.variable;
	ie_len = len - (IEEE80211_HDRLEN) ;//+ sizeof(mgmt->u.probe_req));


	

#if 0
	for (i = 0; hapd->probereq_cb && i < hapd->num_probereq_cb; i++)
		if (hapd->probereq_cb[i].cb(hapd->probereq_cb[i].ctx,
			mgmt->sa, ie, ie_len) > 0)
			return;
#endif

	if (!AP_DEFAULT_SNED_PROBE_RESPONSE)
		return AP_MLME_FAILED;


#if 1
    sta = APStaInfo_FindStaByAddr((ETHER_ADDR *)mgmt->sa);
    //LOG_PRINTF("Probe DA = "MACSTR"\r\n",MAC2STR(mgmt->da));
    //size_t len = pPktInfo->len;
    //LOG_PRINTF("P=%d\r\n",g_probe_req_cnt);
    if(sta == NULL)
    {   
        if(g_probe_req_cnt == 0xffff)
        {   
            //LOG_PRINTF("I\r\n");
            return AP_MLME_FAILED;
        }
        
        if(g_probe_req_cnt ==0)
        {
            B_tick = OS_GetSysTick();
        }

        g_probe_req_cnt++;
        if(g_probe_req_cnt>2)
        {
            A_tick = OS_GetSysTick();
            if(OS_TICK2MS(A_tick-B_tick) <= 50) //50 ms
            {
                g_probe_req_cnt=0xFFFF;
                ssv_hal_drop_probe_request(TRUE);
                os_create_timer(1000, recover_probe_req, NULL, NULL, (void*)TIMEOUT_TASK);
                //LOG_PRINTF("N\r\n");
                    return AP_MLME_FAILED;
            }
            else
            {
                g_probe_req_cnt=0;
            }
        }
    }
    else
    {
        //LOG_PRINTF("STA  = "MACSTR"\r\n",MAC2STR(sta->addr));
    }
#endif

	if (ieee802_11_parse_elems(ie, ie_len, &elems, 0) == ParseFailed) {
		LOG_ERROR("Could not parse ProbeReq from " MACSTR "\r\n",
			MAC2STR(mgmt->sa));
		return AP_MLME_FAILED;
	}

	ssid = NULL;
	ssid_len = 0;

	if ((!elems.ssid || !elems.supp_rates)) {
		LOG_ERROR("STA " MACSTR " sent probe request "
			"without SSID or supported rates element\r\n",
			MAC2STR(mgmt->sa));
		return AP_MLME_FAILED;
	}



	if (AP_DEFAULT_IGNORE_BROADCAST_SSID && elems.ssid_len == 0) {
		LOG_ERROR("Probe Request from " MACSTR " for "
			"broadcast SSID ignored\r\n", MAC2STR(mgmt->sa));
		return AP_MLME_FAILED;
	}

    if(!sta)
    {
        sta = APStaInfo_FindStaByAddr((ETHER_ADDR *)mgmt->sa);
    }
	

	if (elems.ssid_len == 0 ||
		(elems.ssid_len == gDeviceInfo->APInfo->config.ssid_len &&
		ssv6xxx_memcmp((void*)elems.ssid, (void*)gDeviceInfo->APInfo->config.ssid, elems.ssid_len) == 0)) {
			ssid =  gDeviceInfo->APInfo->config.ssid;
			ssid_len = gDeviceInfo->APInfo->config.ssid_len;
			
			//Todo: SEC
			//if (sta)
			//	sta->ssid_probe = &gAPInfo->ssid;
	}

	if (!ssid) {
		/*if (!(mgmt->da[0] & 0x01)) {*/
			char ssid_txt[33];
			ieee802_11_print_ssid(ssid_txt, elems.ssid,
				elems.ssid_len);
#ifdef __LOG_DEGUG__
			LOG_ERROR("Probe Request from " MACSTR	" for foreign SSID '%s'\n",
				MAC2STR(mgmt->sa), ssid_txt);
#endif			
		//}		

		return AP_MLME_FAILED;
	}

#ifdef CONFIG_INTERWORKING
	if (elems.interworking && elems.interworking_len >= 1) {
		ssv_type_u8 ant = elems.interworking[0] & 0x0f;
		if (ant != INTERWORKING_ANT_WILDCARD &&
			ant != hapd->conf->access_network_type) {
				LOG_DEBUGF(LOG_L2_AP,("Probe Request from " MACSTR
					" for mismatching ANT %u ignored",
					MAC2STR(mgmt->sa), ant));
				return;
		}
	}

	if (elems.interworking &&
		(elems.interworking_len == 7 || elems.interworking_len == 9)) {
			const ssv_type_u8 *hessid;
			if (elems.interworking_len == 7)
				hessid = elems.interworking + 1;
			else
				hessid = elems.interworking + 1 + 2;
			if (!ssv_is_broadcast_ether_addr(hessid) &&
				os_memcmp(hessid, hapd->conf->hessid, ETH_ALEN) != 0) {
					LOG_DEBUGF(LOG_L2_AP,("Probe Request from " MACSTR
						" for mismatching HESSID " MACSTR
						" ignored",
						MAC2STR(mgmt->sa), MAC2STR(hessid)));
					return;
			}
	}
#endif /* CONFIG_INTERWORKING */
//***********************************************************************************************************************************	
//***********************************************************************************************************************************
//***********************************************************************************************************************************


	/* TODO: verify that supp_rates contains at least one matching rate
	* with AP configuration */
//#define MAX_PROBERESP_LEN 768
//	buflen = AP_MGMT_PKT_LEN;
#ifdef CONFIG_WPS
	if (hapd->wps_probe_resp_ie)
		buflen += wpabuf_len(hapd->wps_probe_resp_ie);
#endif /* CONFIG_WPS */
#ifdef CONFIG_P2P
	if (hapd->p2p_probe_resp_ie)
		buflen += wpabuf_len(hapd->p2p_probe_resp_ie);
#endif /* CONFIG_P2P */


	/* IEEE 802.11 MGMT frame header */

	resp_mgmt = (struct ieee80211_mgmt *)gDeviceInfo->APInfo->pMgmtPkt;
	epos = ((ssv_type_u8 *) resp_mgmt) + AP_MGMT_PKT_LEN;

	resp_mgmt->frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
		WLAN_FC_STYPE_PROBE_RESP);
	OS_MemCPY((void*)resp_mgmt->da, (void*)mgmt->sa, ETH_ALEN);
	OS_MemCPY((void*)resp_mgmt->sa, (void*)gDeviceInfo->APInfo->own_addr, ETH_ALEN);

	OS_MemCPY((void*)resp_mgmt->bssid, (void*)gDeviceInfo->APInfo->own_addr, ETH_ALEN);

	/* IEEE 802.11 MGMT frame body: information elements: */
	resp_mgmt->u.probe_resp.beacon_int =
		host_to_le16(g_host_cfg.bcn_interval);

	/* hardware or low-level driver will setup seq_ctrl and timestamp */
	resp_mgmt->u.probe_resp.capab_info =
		host_to_le16(hostapd_own_capab_info(gDeviceInfo->APInfo, sta, 1));

	pos = (ssv_type_u8 *)resp_mgmt->u.probe_resp.variable;
	*pos++ = WLAN_EID_SSID;
	*pos++ = ssid_len;
	OS_MemCPY(pos, ssid, ssid_len);
	pos += ssid_len;

	/* Supported rates */
	pos = hostapd_eid_supp_rates(gDeviceInfo->APInfo,pos);

	/* DS Params */
	pos = hostapd_eid_ds_params(gDeviceInfo->APInfo, pos);

	pos = hostapd_eid_country(gDeviceInfo->APInfo, pos, epos - pos);

	/* ERP Information element */
    if(!IS_AP_IN_5G_BAND())
    {
	    pos = hostapd_eid_erp_info(gDeviceInfo->APInfo, pos);
    }
	/* Extended supported rates */
	pos = hostapd_eid_ext_supp_rates(gDeviceInfo->APInfo, pos);

//#pragma message("===================================================")
//#pragma message("     hostapd_eid_wpa not implement yet")
//#pragma message("===================================================")
	/* RSN, MDIE, WPA */
	pos = hostapd_eid_wpa(gDeviceInfo->APInfo, pos, (ssv_type_size_t)(epos - pos));



#if AP_SUPPORT_80211N
	pos = hostapd_eid_ht_capabilities(gDeviceInfo->APInfo, pos);
	pos = hostapd_eid_ht_operation(gDeviceInfo->APInfo, pos);
    //pos = hostapd_eid_secondary_channel(gDeviceInfo->APInfo, pos);    
    //pos = hostapd_eid_csa(gDeviceInfo->APInfo, pos);        
#endif /* CONFIG_IEEE80211N */

	/* Wi-Fi Alliance WMM */
	pos = hostapd_eid_wmm(pApInfo, pos);






	ap_soc_data_send((void *)gDeviceInfo->APInfo->pOSMgmtframe, pos-gDeviceInfo->APInfo->pMgmtPkt, TRUE, 0);


	return AP_MLME_OK;
}



static ssv_type_s32 ap_mlme_rx_beacon(struct ApInfo*pApInfo, struct ieee80211_mgmt *mgmt, ssv_type_u16 len, ssv_type_u8 bssid_idx)
{

	struct ieee802_11_elems elems;
	const ssv_type_u8 *ie;
	ssv_type_size_t ie_len;
	

	ie = (ssv_type_u8 *)mgmt->u.beacon.variable;
	ie_len = len - (IEEE80211_HDRLEN + sizeof(mgmt->u.beacon));

	
	if (len < IEEE80211_HDRLEN + sizeof(mgmt->u.beacon)) {
		LOG_ERROR("handle_beacon - too short payload (len=%lu)\r\n",
		       (unsigned long) len);
		return AP_MLME_FAILED;
	}

	(void) ieee802_11_parse_elems(ie,
				      ie_len, &elems,
				      0);

	neighbor_ap_list_process_beacon(gDeviceInfo->APInfo, mgmt, &elems);
	



	return AP_MLME_OK;
}


static ssv_type_s32 ap_mlme_rx_atim(struct ApInfo*pApInfo, struct ieee80211_mgmt *mgmt, ssv_type_u16 len,ssv_type_u8 bssid_idx)
{

	return AP_MLME_OK;
}

extern void change_sta_disassoc_to_deauth(ApInfo_st *pApInfo,APStaInfo_st *sta,ssv_type_bool timer_creat);
static ssv_type_s32 ap_mlme_rx_disassoc(struct ApInfo*pApInfo, struct ieee80211_mgmt *mgmt, ssv_type_u16 len, ssv_type_u8 bssid_idx)
{


	APStaInfo_st *sta;

	if (len < IEEE80211_HDRLEN + sizeof(mgmt->u.disassoc)) {
		LOG_PRINTF("handle_disassoc - too short payload (len=%lu)\r\n",
		       (unsigned long) len);
		return AP_MLME_FAILED;
	}

	LOG_DEBUG("disassocation: STA=" MACSTR " reason_code=%d\r\n",
		   MAC2STR(mgmt->sa),
		   le_to_host16(mgmt->u.disassoc.reason_code));

	sta = APStaInfo_FindStaByAddr((ETHER_ADDR*)mgmt->sa);
	if (sta == NULL) {
		LOG_DEBUG("Station " MACSTR " trying to disassociate, but it "
		       "is not associated.\r\n", MAC2STR(mgmt->sa));
		return AP_MLME_FAILED;
	}

	//Todo: SEC
	change_sta_disassoc_to_deauth(pApInfo, sta,TRUE);
	LOG_INFO("Station " MACSTR " disassociated\r\n", MAC2STR(mgmt->sa));
    APStaInfo_free(pApInfo, sta);
#ifdef __AP_DEBUG__
    APStaInfo_PrintStaInfo(0);
	APStaInfo_PrintStaInfo(1);	
#endif//__AP_DEBUG__

	return AP_MLME_OK;
}


//-----------------------------------------------------------------
//Authentication

static void send_auth_reply(struct ApInfo*pApInfo, const ssv_type_u8 *dst, const ssv_type_u8 *bssid,
	ssv_type_u16 auth_alg, ssv_type_u16 auth_transaction, ssv_type_u16 resp,
	const ssv_type_u8 *ies, ssv_type_size_t ies_len)
{
	struct ieee80211_mgmt *reply;
	ssv_type_u8 *buf;
	ssv_type_size_t rlen;

	rlen = IEEE80211_HDRLEN + sizeof(reply->u.auth) + ies_len;
	buf = pApInfo->pMgmtPkt;   //os_zalloc(rlen);
// 	if (buf == NULL)
// 		return;

	reply = (struct ieee80211_mgmt *) buf;
	reply->frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
		WLAN_FC_STYPE_AUTH);
	OS_MemCPY((void*)reply->da, (void*)dst, ETH_ALEN);
	OS_MemCPY((void*)reply->sa, (void*)gDeviceInfo->APInfo->own_addr, ETH_ALEN);
	OS_MemCPY((void*)reply->bssid, (void*)bssid, ETH_ALEN);

	reply->u.auth.auth_alg = host_to_le16(auth_alg);
	reply->u.auth.auth_transaction = host_to_le16(auth_transaction);
	reply->u.auth.status_code = host_to_le16(resp);

	if (ies && ies_len)
		OS_MemCPY((void*)reply->u.auth.variable, (void*)ies, ies_len);

    #ifdef __AP_DEBUG__
	LOG_DEBUG("authentication reply: STA=" MACSTR
		" auth_alg=%d auth_transaction=%d resp=%d (IE len=%lu)\n",
		MAC2STR(dst), auth_alg, auth_transaction,
		resp, (unsigned long) ies_len);
    #endif
    
	if (wpa_driver_nl80211_send_mlme((const ssv_type_u8*)reply, rlen, TRUE) < 0)
		LOG_DEBUG("send_auth_reply: send,%s",__func__);
	
	//os_free(buf);
}




static ssv_type_s32 ap_mlme_rx_auth(struct ApInfo*pApInfo, struct ieee80211_mgmt *mgmt, ssv_type_u16 len, ssv_type_u8 bssid_idx)
{
	
    ssv_type_u16 auth_alg, auth_transaction, status_code;
    ssv_type_u16 resp = WLAN_STATUS_SUCCESS;
    APStaInfo_st *sta = NULL;

    ssv_type_u16 fc;
    const ssv_type_u8 *challenge = NULL;
    ssv_type_u8 *variable = NULL;

    //	int vlan_id = 0;
    ssv_type_u8 resp_ies[2 + WLAN_AUTH_CHALLENGE_LEN];
    ssv_type_size_t resp_ies_len = 0;

    if (len < IEEE80211_HDRLEN + sizeof(mgmt->u.auth)) {
        LOG_ERROR("handle_auth - too short payload (len=%lu)\r\n",
        (unsigned long) len);
        return AP_MLME_FAILED;
    }

    auth_alg = le_to_host16(mgmt->u.auth.auth_alg);
    auth_transaction = le_to_host16(mgmt->u.auth.auth_transaction);
    status_code = le_to_host16(mgmt->u.auth.status_code);
    fc = le_to_host16(mgmt->frame_control);
    variable = mgmt->u.auth.variable;
    if (len >= IEEE80211_HDRLEN + sizeof(mgmt->u.auth) +
        2 + WLAN_AUTH_CHALLENGE_LEN &&
        *(variable + 0) == WLAN_EID_CHALLENGE &&
        *(variable + 1) == WLAN_AUTH_CHALLENGE_LEN)
    challenge = (ssv_type_u8 *)(variable + 2);
#ifdef __AP_DEBUG__

    LOG_TRACE("authentication: STA=" MACSTR " auth_alg=%d "
    "auth_transaction=%d status_code=%d wep=%d%s\r\n",
    MAC2STR(mgmt->sa), auth_alg, auth_transaction,
    status_code, !!(fc & WLAN_FC_ISWEP),
    challenge ? " challenge" : "");
#endif
    if (pApInfo->tkip_countermeasures) {
        resp = WLAN_REASON_MIC_FAILURE;
        goto fail;
    }

    if (!(
        (((ssv_type_u32)pApInfo->auth_algs & WPA_AUTH_ALG_OPEN) &&
        (ssv_type_u32)auth_alg == WLAN_AUTH_OPEN) ||
        (((ssv_type_u32)pApInfo->auth_algs & WPA_AUTH_ALG_SHARED) &&
        (ssv_type_u32)auth_alg == WLAN_AUTH_SHARED_KEY))) {
        LOG_DEBUG("Unsupported authentication algorithm (%d)\r\n",
        auth_alg);
        resp = WLAN_STATUS_NOT_SUPPORTED_AUTH_ALG;
        goto fail;
    }

    if (!(auth_transaction == 1 ||
        (auth_alg == WLAN_AUTH_SHARED_KEY && auth_transaction == 3))) {
        LOG_DEBUG("Unknown authentication transaction number (%d)\r\n",
        auth_transaction);
        resp = WLAN_STATUS_UNKNOWN_AUTH_TRANSACTION;
        goto fail;
    }

    if (ssv6xxx_memcmp((void*)mgmt->sa, (void*)pApInfo->own_addr, ETH_ALEN) == 0) {
        LOG_DEBUG("Station " MACSTR " not allowed to authenticate.\r\n",
        MAC2STR(mgmt->sa));
        resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
        goto fail;
    }

    /* if found old station, clear old data and re-add */
    if (APStaInfo_FindStaByAddr((ETHER_ADDR*)(ssv_type_u8 *)mgmt->sa) != NULL)
    {
        send_deauth_and_remove_sta((ssv_type_u8 *)mgmt->sa, false);
        //OS_MsDelay(20); //Wait fw to clear old info
    }

    sta = APStaInfo_add(pApInfo, (ssv_type_u8 *)mgmt->sa);

    if (!sta) {
        resp = WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA;//WLAN_STATUS_UNSPECIFIED_FAILURE;
        goto fail;
    }


// 	if (vlan_id > 0) {
// 		if (hostapd_get_vlan_id_ifname(hapd->conf->vlan,
// 					       vlan_id) == NULL) {
// 			hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_RADIUS,
// 				       HOSTAPD_LEVEL_INFO, "Invalid VLAN ID "
// 				       "%d received from RADIUS server",
// 				       vlan_id);
// 			resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
// 			goto fail;
// 		}
// 		sta->vlan_id = vlan_id;
// 		hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_RADIUS,
// 			       HOSTAPD_LEVEL_INFO, "VLAN ID %d", sta->vlan_id);
// 	}
// 
// 	sta->flags &= ~WLAN_STA_PREAUTH;
// 	ieee802_1x_notify_pre_auth(sta->eapol_sm, 0);
// 
// 	if (hapd->conf->acct_interim_interval == 0 && acct_interim_interval)
// 		sta->acct_interim_interval = acct_interim_interval;
// 	if (res == HOSTAPD_ACL_ACCEPT_TIMEOUT)
// 		ap_sta_session_timeout(hapd, sta, session_timeout);
// 	else
// 		ap_sta_no_session_timeout(hapd, sta);
// 
    if(gDeviceInfo->bMCC)
    {
        ssv6xxx_wifi_set_mcc_time_slice(((ssv_vif*)gDeviceInfo->APInfo->vif)->idx,CHAN_SWITCH_TIMER_TIME*2+100);
        os_cancel_timer(tmr_set_vif_mcc_slice,(ssv_type_u32)gDeviceInfo->APInfo->vif,(ssv_type_u32)0);
        os_create_timer(10000, tmr_set_vif_mcc_slice, gDeviceInfo->APInfo->vif, NULL, (void*)TIMEOUT_TASK);
    }

    switch (auth_alg) {
        case WLAN_AUTH_OPEN:
#ifdef __AP_DEBUG__
        LOG_DEBUG("authentication OK (open system),%s\n",__func__);
#endif
#ifdef IEEE80211_REQUIRE_AUTH_ACK
        /* Station will be marked authenticated if it ACKs the
        * authentication reply. */
#else
        OS_MutexLock(sta->apsta_mutex);
        set_sta_flag(sta, WLAN_STA_AUTH);
        OS_MutexUnLock(sta->apsta_mutex);

        sta->auth_alg = WLAN_AUTH_OPEN;
        //mlme_authenticate_indication(hapd, sta);
#endif
        break;
        case WLAN_AUTH_SHARED_KEY:
        resp = auth_shared_key(pApInfo, sta, auth_transaction, challenge,
        fc & WLAN_FC_ISWEP);
        sta->auth_alg = WLAN_AUTH_SHARED_KEY;
        //mlme_authenticate_indication(hapd, sta);
        if (sta->challenge && auth_transaction == 1) {
            resp_ies[0] = WLAN_EID_CHALLENGE;
            resp_ies[1] = WLAN_AUTH_CHALLENGE_LEN;
            OS_MemCPY(resp_ies + 2, sta->challenge,
            WLAN_AUTH_CHALLENGE_LEN);
            resp_ies_len = 2 + WLAN_AUTH_CHALLENGE_LEN;
        }
        break;

    }

fail:
    if((resp!=WLAN_STATUS_SUCCESS)&&(sta!=NULL))
        APStaInfo_free(pApInfo, sta);

    send_auth_reply(pApInfo, (ssv_type_u8 *)mgmt->sa, (ssv_type_u8 *)mgmt->bssid, auth_alg,
    auth_transaction + 1, resp, resp_ies, resp_ies_len);

    return AP_MLME_OK;
}


static ssv_type_s32 ap_mlme_rx_deauth(struct ApInfo*pApInfo, struct ieee80211_mgmt *mgmt, ssv_type_u16 len, ssv_type_u8 bssid_idx)
{
	APStaInfo_st *sta;

	if (len < IEEE80211_HDRLEN + sizeof(mgmt->u.deauth)) {

		LOG_DEBUG("handle_deauth - too short "
			"payload (len=%lu)\r\n", (unsigned long) len);
		return AP_MLME_FAILED;
	}

	LOG_DEBUG("deauthentication: STA=" MACSTR
		" reason_code=%d\r\n",
		MAC2STR(mgmt->sa), le_to_host16(mgmt->u.deauth.reason_code));

	sta = APStaInfo_FindStaByAddr((ETHER_ADDR*)mgmt->sa);
	if (sta == NULL) {
		LOG_DEBUG("Station " MACSTR " trying "
			"to deauthenticate, but it is not authenticated\r\n",
			MAC2STR(mgmt->sa));
		return AP_MLME_FAILED;
	}

	ap_sta_set_authorized(pApInfo, sta, 0);

	
	sta->_flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC |
		WLAN_STA_ASSOC_REQ_OK);

#ifndef CONFIG_NO_WPA2
{
    struct wpa_auth_config *conf= (struct wpa_auth_config 
        *)(gDeviceInfo->APInfo->wpa_auth.conf);
    
    if(conf->wpa_key_mgmt == WPA_KEY_MGMT_PSK) 
    {
        wpa_auth_sm_event(&(sta->wpa_sm), WPA_DEAUTH);
    }
}
#endif	
// 	mlme_deauthenticate_indication(
// 		hapd, sta, le_to_host16(mgmt->u.deauth.reason_code));
// 	sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST;
// 	ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
	
	APStaInfo_free(pApInfo, sta);

	return AP_MLME_OK;
}


static ssv_type_s32 ap_mlme_rx_action(struct ApInfo*pApInfo, struct ieee80211_mgmt *mgmt, ssv_type_u16 len, ssv_type_u8 bssid_idx)
{
	return AP_MLME_OK;
}

/**
*  IEEE 802.11 Management Frame Handler:
*/

AP_MGMT80211_RxHandler AP_RxHandler[] = 
{
	ap_mlme_rx_assoc_request, //Association Request Handler (subtype=00)
	NULL, //Association Response Handler (subtype=01)
	ap_mlme_rx_reassoc_request,// Re-association Request Handler (subtype=02)
	NULL,//Re-association Response Handler (subtype=03)
	ap_mlme_rx_probe_request,//Probe Request Handler (subtype=04)
	NULL,//Probe Response Handler (subtype=05)
	NULL,//Reserved (subtype=06)
	NULL,//Reserved (subtype=07)
	ap_mlme_rx_beacon,//Beacon Handler (subtype=08)
	ap_mlme_rx_atim,//ATIM Handler (subtype=09)
	
	ap_mlme_rx_disassoc,//Disassociation Handler (subtype=10)
	ap_mlme_rx_auth,//Authentication Handler (subtype=11)
	ap_mlme_rx_deauth,//De-authentication Handler (subtype=12)


	ap_mlme_rx_action,//Action Handler (subtype=13)
};


extern ssv_type_u32 cmdeng_task_st;
void AP_RxHandleAPMode(void *frame)
{
    CFG_HOST_RXPKT *pPktInfo = (CFG_HOST_RXPKT *)OS_FRAME_GET_DATA(frame);
    ssv_type_u8 bFreePacket = TRUE;
    cmdeng_task_st =__LINE__;
    if(1==ssv_hal_get_rxpkt_f80211(pPktInfo))
    {
        ssv_type_u8 *raw = ssv6xxx_host_rx_data_get_data_ptr(pPktInfo);
        ssv_type_u16 fc = (raw[1]<<8) | raw[0];
        ssv_type_u32 nDataLen =ssv_hal_get_rxpkt_data_len(pPktInfo);
        ssv_type_u8 bssid_idx = ssv_hal_get_rxpkt_bssid_idx(pPktInfo);
        if ( WLAN_FC_GET_TYPE(fc) == WLAN_FC_TYPE_MGMT )
        {
            ssv_type_u8 stype;
            stype = WLAN_FC_GET_STYPE(fc);

            if ( stype < ARRAY_SIZE(AP_RxHandler) )
            {

                struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)(raw);
                APStaInfo_st * sta = APStaInfo_FindStaByAddr((ETHER_ADDR *)mgmt->sa);
                if(sta)
                sta->last_rx = OS_GetSysTick();

                OS_MemSET((void*)gDeviceInfo->APInfo->pMgmtPkt, 0, AP_MGMT_PKT_LEN);

                //LOG_TRACE("AP get frame. Frame type %d\n",stype);

                cmdeng_task_st =__LINE__;
                if ( AP_RxHandler[stype] !=NULL )
                {
                    AP_RxHandler[stype](gDeviceInfo->APInfo, mgmt, nDataLen,bssid_idx);
                }
                else
                {
                    LOG_TRACE("not implement manager func [sub type:%x]\r\n",stype);
                }
            }
            else
            {
                LOG_TRACE("error! out of func bound [sub type:%x]\r\n",stype);			
            }

        }
        else
        {
            LOG_TRACE("not accept type [type:%x]\r\n",WLAN_FC_GET_TYPE(fc));		
        }
    }

#ifndef CONFIG_NO_WPA2	
    else
    {
        ethhdr *pEthHdr =  (ethhdr *)(ssv6xxx_host_rx_data_get_data_ptr(pPktInfo));
        //u8 *raw = ssv6xxx_host_rx_data_get_data_ptr(pPktInfo);
        APStaInfo_st * sta = APStaInfo_FindStaByAddr((ETHER_ADDR *)pEthHdr->h_source);
        ssv_type_u8 *eapol_data = (ssv_type_u8 *)((ssv_type_u8 *)pEthHdr+sizeof(struct ethhdr_st));          
        ssv_type_size_t eapol_len= be_to_host16(*((ssv_type_u16 *)(eapol_data+2)));
        //LOG_DEBUG("====eapol_len==%d\n\r",eapol_len);
        if(sta != NULL)
        {
            cmdeng_task_st =__LINE__;
            wpa_receive(&(gDeviceInfo->APInfo->wpa_auth),&(sta->wpa_sm),eapol_data,eapol_len+sizeof(struct ieee802_1x_hdr));
        }
        //hex_dump(pPktInfo,nDataLen);
    }
#endif
	if (bFreePacket)
	{
		os_frame_free(frame);
	}
}
void send_deauth_and_remove_all()
{
    ssv_type_u32 idx;

    gDeviceInfo->APInfo->TimAllOne = 1;
    GenBeacon();
    OS_MsDelay(g_host_cfg.bcn_interval*3);

    for(idx=0; idx<gMaxAID; idx++)
    {
        APStaInfo_st *sta= &gDeviceInfo->APInfo->StaConInfo[idx];
        if (!test_sta_flag(sta, WLAN_STA_VALID))
            continue;
        else
        {           
            change_sta_disassoc_to_deauth(gDeviceInfo->APInfo, sta,FALSE);
            sta->timeout_next = STA_DEAUTH;
            os_cancel_timer(ap_handle_timer,  (ssv_type_u32)gDeviceInfo->APInfo, (ssv_type_u32)sta);
            //Send De-authentication frame //
            i802_sta_deauth(gDeviceInfo->APInfo->own_addr, sta->addr, WLAN_REASON_PREV_AUTH_NOT_VALID);
            OS_MsDelay(g_host_cfg.bcn_interval);
            APStaInfo_free(gDeviceInfo->APInfo, sta);
        }     
    }

}

ssv_type_s32 send_deauth_and_remove_sta(ssv_type_u8 *hwaddr, ssv_type_bool deauth)
{
    ssv_type_u32 idx=0;
    APStaInfo_st *sta= NULL;

    for(idx=0; idx<gMaxAID; idx++)
    {
        sta = &gDeviceInfo->APInfo->StaConInfo[idx];
        if (!test_sta_flag(sta, WLAN_STA_VALID))
        {
            sta = NULL;
            continue;
        }
        else
        {           
            if ((sta->addr[0]== hwaddr[0])&&
                (sta->addr[1]== hwaddr[1])&&
                (sta->addr[2]== hwaddr[2])&&
                (sta->addr[3]== hwaddr[3])&&
                (sta->addr[4]== hwaddr[4])&&
                (sta->addr[5]== hwaddr[5]))
                break;
       }
       sta = NULL;
    }

    if(sta)
    {
        LOG_PRINTF("REM STA:%x:%x:%x:%x:%x:%x\r\n",
            sta->addr[0],sta->addr[1],sta->addr[2],sta->addr[3],sta->addr[4],sta->addr[5]);
        change_sta_disassoc_to_deauth(gDeviceInfo->APInfo, sta,FALSE);
        sta->timeout_next = STA_DEAUTH;
        os_cancel_timer(ap_handle_timer,  (ssv_type_u32)gDeviceInfo->APInfo, (ssv_type_u32)sta);
        //Send De-authentication frame //
        if (deauth)
        {
            i802_sta_deauth(gDeviceInfo->APInfo->own_addr, sta->addr, WLAN_REASON_PREV_AUTH_NOT_VALID);
        }
        APStaInfo_free(gDeviceInfo->APInfo, sta);
        return SSV6XXX_SUCCESS;
    }
    return SSV6XXX_FAILED;    
}

#endif

