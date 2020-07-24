/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#define __SFILE__ "wap_auth.c"


//#include "utils/includes.h"


#include <log.h>
#include <rtos.h>
#include "common/wpa_common.h"
#include "log.h"
//#include "utils/eloop.h"
#include "state_machine.h"
#include "common/ieee802_11_defs.h"
#include "crypto/aes_wrap.h"
#include "crypto/crypto.h"
#include "crypto/sha1.h"
//#include "eapol_auth/eapol_auth_sm.h"
#include "ap_config.h"
//#include "ieee802_11.h"
//#include "pmksa_cache_auth.h"
#include "random.h"
#include "wpa_auth_i.h"
#include "wpa_auth_ie.h"
#include "wpa_debug.h"

#include <ssv_devinfo.h>
#include "ap_info.h"
#include "wpa_debug.h"
#include <txrx_hdl.h>
#include <host_apis.h>
#include "stdarg.h"
#include "wpa_auth.h"
#include "ssv_timer.h"
#include "ap_drv_cmd.h"
#include "ap_mlme.h"

#if (AP_MODE_ENABLE == 1)

#define STATE_MACHINE_DATA struct wpa_state_machine
#define STATE_MACHINE_DEBUG_PREFIX "WPA"
#define STATE_MACHINE_ADDR sm->addr

#ifndef CONFIG_NO_WPA2
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

#define broadcast_ether_addr (const ssv_type_u8 *) "\xff\xff\xff\xff\xff\xff"
//int startCount;
//int endCount =0;
int associateRsp = 0;
int addStaFlg= 0;

extern struct task_info_st g_host_task_info[];
extern void Send_Beacon(ssv_type_bool block);
extern void change_sta_disassoc_to_deauth(ApInfo_st *pApInfo,APStaInfo_st *sta,ssv_type_bool timer_creat);
extern void APStaInfo_free(ApInfo_st *pApInfo, APStaInfo_st *sta);
extern int os_get_random(unsigned char *buf, int len);

static ssv_inline unsigned short wpa_swap_16(unsigned short v)
{
	return ((v & 0xff) << 8) | (v >> 8);
}

static ssv_inline unsigned int wpa_swap_32(unsigned int v)
{
	return ((v & 0xff) << 24) | ((v & 0xff00) << 8) |
		((v & 0xff0000) >> 8) | (v >> 24);
}


//unsigned long ticks_1sec = 1000/OS_TICK2MS(1);


static const ssv_type_u32 dot11RSNAConfigGroupUpdateCount = 4;
static const ssv_type_u32 dot11RSNAConfigPairwiseUpdateCount = 4;
static const ssv_type_u32 eapol_key_timeout_first = 100;//100; /* ms */
static const ssv_type_u32 eapol_key_timeout_subseq = 1000;//1000; /* ms */
static const ssv_type_u32 eapol_key_timeout_error = 100;

/* TODO: make these configurable */
static const int dot11RSNAConfigPMKLifetime = 43200;
static const int dot11RSNAConfigPMKReauthThreshold = 70;
static const int dot11RSNAConfigSATimeout = 60;

static void wpa_send_eapol_timeout(void *eloop_ctx, void *timeout_ctx);
static int wpa_sm_step(struct wpa_state_machine *sm);
static int wpa_verify_key_mic(struct wpa_ptk *PTK, ssv_type_u8 *data, ssv_type_size_t data_len);
//static void wpa_sm_call_step(void *eloop_ctx, void *timeout_ctx);
//static void wpa_group_sm_step(struct wpa_authenticator *wpa_auth,
//			      struct wpa_group *group);
static void wpa_request_new_ptk(struct wpa_state_machine *sm);
static int wpa_gtk_update(struct wpa_authenticator *wpa_auth,
			  struct wpa_group *group);
static int wpa_group_config_group_keys(struct wpa_authenticator *wpa_auth,
				       struct wpa_group *group);

//start: add by angel.chiu for wpa

static const ssv_type_u8 rfc1042_header[6] = { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00 };
#define WPA_STA_WMM BIT(1)


void eapol_auth_free(struct wpa_authenticator *wpa_auth,struct wpa_state_machine *sm)
{	
	os_cancel_timer(wpa_send_eapol_timeout, (ssv_type_u32)wpa_auth, (ssv_type_u32)sm);	
}

extern void set_ap_state(ap_state st);
void enable_beacon(void *data)
{    
               
   struct resp_evt_result *pmk_value = (struct resp_evt_result *)data;
   ApInfo_st *pApInfo= gDeviceInfo->APInfo; 
   LOG_DEBUGF(LOG_L2_AP,("enable_beacon\n\r"));
   wpa_hexdump_key(MSG_DEBUG,"+++++++++++++pmk ",&(pmk_value->u.pmk[0]),PMK_LEN);
   OS_MemCPY(pApInfo->wpa_psk.psk,&(pmk_value->u.pmk[0]),PMK_LEN);
   set_ap_state(AP_STATE_PAUSE);
   Send_Beacon(TRUE);
   set_ap_state(AP_STATE_READY);
   
}

static ssv6xxx_data_result wpa_receive_cb(void *frame,ssv_type_u32 len)
{
    void *msg = os_msg_alloc();
    if(NULL!=msg)
    {
        while(os_msg_send(msg, frame)==OS_FAILED)
        {
            OS_TickDelay(1);
            LOG_PRINTF("wpa_receive_cb retry\r\n");
        }
        return SSV6XXX_DATA_ACPT;
    }
    else
    {
        LOG_PRINTF("%s:msg alloc fail\r\n");
        return SSV6XXX_DATA_CONT;
    }
}


void ssv6xxx_eapol_data_reg_cb(void)
{
    struct eth_flt eth_f;
    eth_f.ethtype = 0x888e;
    eth_f.cb_fn = (data_handler)wpa_receive_cb; 
    RxHdl_SetEthRxFlt(&eth_f,SSV6XXX_CB_ADD);

}

void process_4Ways_start(struct APStaInfo *staInfo)
{
    LOG_DEBUGF(LOG_L2_AP,("assocRsp =%d,addStaflg=%d\n\r",staInfo->assocRsp,staInfo->addStaFlag));
    {
        if(staInfo->assocRsp && staInfo->addStaFlag)
        {
            //enter initial  
            if(staInfo->reassoc) //need to check
            {  
                wpa_auth_sm_event(&(staInfo->wpa_sm), WPA_REAUTH);
            }
            else
            {  
                
                wpa_auth_sta_associated(&(gDeviceInfo->APInfo->wpa_auth), 
                &(staInfo->wpa_sm));
            }
            staInfo->assocRsp = 0 ;
            staInfo->reassoc = 0;
            staInfo->addStaFlag= 0;
        }
    }
}

void start_4way_handshake(void *data)
{
    
    struct APStaInfo *staInfo = NULL;
    struct resp_evt_result *add_sta_res = (struct resp_evt_result *)data;
    struct wpa_auth_config *conf =  &(gDeviceInfo->APInfo->ap_conf);
    ssv_type_u32 i = 0;
    LOG_DEBUGF(LOG_L2_AP,("=====start_4way_handshake 4 way===\n\r "));
    for(i=0;i<gMaxAID;i++)
    {
        staInfo = &(gDeviceInfo->APInfo->StaConInfo[i]);
        if(staInfo->wsid_idx == add_sta_res->u.wsid)
            break;
    }
    if(staInfo == NULL)
        return;
    
    staInfo->addStaFlag = 1;
                
    if(conf->wpa== WPA_PROTO_RSN)
    {
       LOG_DEBUGF(LOG_L2_AP,("+++++process 4 ways++++++\n\r"));
       process_4Ways_start(staInfo);
       
    }
}

static int ssv6xxx_set_key( enum wpa_alg alg, const ssv_type_u8 *addr,
				      int key_idx, int set_tx,
				      const ssv_type_u8 *seq, ssv_type_size_t seq_len,
				      const ssv_type_u8 *key, ssv_type_size_t key_len)
{
    int res = 0;
    ssv_vif* vif = (ssv_vif*)gDeviceInfo->APInfo->vif;
    struct securityEntry  sec_entry ;
    sec_entry.cipher = alg;
    sec_entry.wpaUnicast = 1;
    sec_entry.keyLen = key_len;
    sec_entry.keyIndex = key_idx;
    sec_entry.vif_idx = vif->idx;
    OS_MemCPY(sec_entry.bssid,(void *)addr,6);
    OS_MemCPY(sec_entry.key,(void *)key, key_len); 
    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_SECURITY_ENTRY, &sec_entry, sizeof(struct securityEntry), TRUE, FALSE);     
    return res;
}
    


static int ssv6xxx_send_eapol(
	const ssv_type_u8 *addr, const ssv_type_u8 *data,
	ssv_type_size_t data_len, int encrypt)
{
	
	struct ieee80211_hdr *hdr;
	ssv_type_size_t len;
	ssv_type_u8 *pos;
	int res=0;
	int qos = 1;//0 & WPA_STA_WMM;
    
	
	hdr = (struct ieee80211_hdr *)gDeviceInfo->APInfo->pMgmtPkt;
	
	len = sizeof(*hdr) + (qos ? 2 : 0) + sizeof(rfc1042_header) + 2 +
		data_len;

	hdr->frame_control =
		IEEE80211_FC(WLAN_FC_TYPE_DATA, WLAN_FC_STYPE_DATA);
	hdr->frame_control |= host_to_le16(WLAN_FC_FROMDS);
	if (encrypt)
		hdr->frame_control |= host_to_le16(WLAN_FC_ISWEP);
	if (qos) {
		hdr->frame_control |=
			host_to_le16(WLAN_FC_STYPE_QOS_DATA << 4);
	}

	OS_MemCPY((void *)hdr->IEEE80211_DA_FROMDS, addr, ETH_ALEN);
	OS_MemCPY((void *)hdr->IEEE80211_BSSID_FROMDS, gDeviceInfo->APInfo->own_addr, ETH_ALEN);
	OS_MemCPY((void *)hdr->IEEE80211_SA_FROMDS, gDeviceInfo->APInfo->own_addr, ETH_ALEN);
	pos = (ssv_type_u8 *) ((hdr->IEEE80211_SA_FROMDS)+ETH_ALEN+2);
    
	if (qos) {
		/* Set highest priority in QoS header */
		pos[0] = 0;
		pos[1] = 0;
		pos += 2;
	}

	OS_MemCPY(pos, rfc1042_header, sizeof(rfc1042_header));
	pos += sizeof(rfc1042_header);
	WPA_PUT_BE16(pos, ETH_P_PAE);
	pos += 2;
	OS_MemCPY(pos, data, data_len);
    pos+= data_len;  
    LOG_DEBUGF(LOG_L2_AP,("=====send eapol ===\n\r"));
    res = ap_soc_data_send((void *)gDeviceInfo->APInfo->pOSMgmtframe,pos-gDeviceInfo->APInfo->pMgmtPkt,1,0);
	if (res < 0) {
		wpa_printf(MSG_ERROR, "i802_send_eapol - packet len: %lu - "
			   "failed: %d",(unsigned long) len, res);
	}
	//OS_MemFree(hdr);
	return res;
}

int wpa_auth_derive_psk(ApInfo_st *pApInfo)
{
    struct ap_calc_pmk_cfg apcfg ;
    
	OS_MemSET((void *)&apcfg,0x0,sizeof( struct ap_calc_pmk_cfg));
	wpa_hexdump_ascii(MSG_DEBUG, "SSID",
			  (ssv_type_u8 *) pApInfo->config.ssid,pApInfo->config.ssid_len);
	wpa_hexdump_ascii_key(MSG_DEBUG, "PSK (ASCII passphrase)",
			      (ssv_type_u8 *) pApInfo->password,
			      ssv6xxx_strlen((void *)pApInfo->password));
	
   
    apcfg.ssid_len = pApInfo->config.ssid_len;
    OS_MemCPY(apcfg.ssid,pApInfo->config.ssid,apcfg.ssid_len);
    OS_MemCPY((void *)apcfg.password,  pApInfo->password, ssv6xxx_strlen((void *)pApInfo->password));
    while(_ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_GET_PMK, &apcfg, sizeof(struct ap_calc_pmk_cfg), FALSE, FALSE) != SSV6XXX_SUCCESS)
    {
        LOG_PRINTF("CMD_GET_PMK retry\r\n");
        OS_MsDelay(10);
    }

	//pApInfo->wpa_psk->group = 1;
	return 0;
}


//end:add by angel.chiu for wpa

//check by angel.chiu
void wpa_get_ntp_timestamp(ssv_type_u8 *buf)
{
	ssv_type_u32 now;
	ssv_type_u32 sec, usec;
	be32_ssv_type tmp;

	/* 64-bit NTP timestamp (time from 1900-01-01 00:00:00) */
	now = OS_TICK2MS(OS_GetSysTick());
	sec = now/1000 + 2208988800U; /* Epoch to 1900 */
	/* Estimate 2^32/10^6 = 4295 - 1/32 - 1/512 */
	usec = now-((now/1000)*1000);
	usec = 4295 * usec - (usec >> 5) - (usec >> 9);
	tmp = host_to_be32(sec);
	os_memcpy(buf, (ssv_type_u8 *) &tmp, 4);
	tmp = host_to_be32(usec);
	os_memcpy(buf + 4, (ssv_type_u8 *) &tmp, 4);
}



static ssv_inline const ssv_type_u8 * wpa_auth_get_psk(struct wpa_authenticator *wpa_auth,
					  const ssv_type_u8 *addr, const ssv_type_u8 *prev_psk)
{
    
	return (gDeviceInfo->APInfo->wpa_psk.psk);
    
}



static ssv_inline int wpa_auth_set_key(struct wpa_authenticator *wpa_auth,
				   //int vlan_id,
				   enum wpa_alg alg, const ssv_type_u8 *addr, int idx,
				   ssv_type_u8 *key, ssv_type_size_t key_len)
{    
    LOG_DEBUGF(LOG_L2_AP,("%s,alg=%d,idx=%d,addr=" MACSTR "\n\r","set key",alg,idx,MAC2STR(addr)));
    ssv6xxx_set_key(alg, addr, idx,1,NULL,0, key, key_len);
    return 0; //WLAN_STATUS_SUCCESS
}


void wpa_auth_recovery()
{
#ifndef CONFIG_NO_WPA2   
    if(gDeviceInfo->APInfo->ap_conf.wpa_key_mgmt == WPA_KEY_MGMT_PSK)
    {
        
        wpa_group_config_group_keys(&(gDeviceInfo->APInfo->wpa_auth),gDeviceInfo->APInfo->wpa_auth.group);              
        
    }
#endif  

}


static ssv_inline int wpa_auth_get_seqnum(struct wpa_authenticator *wpa_auth,
				      const ssv_type_u8 *addr, int idx, ssv_type_u8 *seq)
{
	//to do??
	*seq = 1;
	return 0;
}


static ssv_inline int
wpa_auth_send_eapol(struct wpa_authenticator *wpa_auth, const ssv_type_u8 *addr,
		    const ssv_type_u8 *data, ssv_type_size_t data_len, int encrypt)
{
	
	return ssv6xxx_send_eapol(addr, data, data_len, encrypt);
}




void wpa_auth_logger(struct wpa_authenticator *wpa_auth, const ssv_type_u8 *addr,
		     logger_level level, const char *txt)
{
	wpa_printf(MSG_INFO,"%s,addr:" MACSTR "\n\r",txt,MAC2STR(addr));
}

void wpa_auth_vlogger(struct wpa_authenticator *wpa_auth, const ssv_type_u8 *addr,
		      logger_level level, const char *fmt, ...)
{
	char *format;
	int maxlen;

    
	va_list ap;


	maxlen = ssv6xxx_strlen(fmt) + 100;
	format = OS_MemAlloc(maxlen);
	if (!format)
		return;

	va_start(ap, fmt);
	ssv6xxx_vsnprintf(format, maxlen, fmt, ap);
	va_end(ap);

    wpa_printf(MSG_INFO,"%s,addr:" MACSTR "\n\r",format,MAC2STR(addr));
	
	OS_MemFree(format);

}

//in some error wpa2 ap send deauth to the sta.
static void wpa_sta_disconnect(struct wpa_authenticator *wpa_auth,
			       const ssv_type_u8 *addr)
{
	
    APStaInfo_st *sta =APStaInfo_FindStaByAddr((ETHER_ADDR *)addr);             
    if(sta !=NULL)
    {
         change_sta_disassoc_to_deauth(gDeviceInfo->APInfo, sta,FALSE);
         sta->timeout_next = STA_DEAUTH;
         os_cancel_timer(ap_handle_timer,  (ssv_type_u32)gDeviceInfo->APInfo, (ssv_type_u32)sta);
         //Send De-authentication frame                                                                                                  //----------------------------------->implement
         i802_sta_deauth(gDeviceInfo->APInfo->own_addr, sta->addr, 
         WLAN_REASON_PREV_AUTH_NOT_VALID);
         APStaInfo_free(gDeviceInfo->APInfo, sta);
        
     }
     return;
    
    
}


static int wpa_use_aes_cmac(struct wpa_state_machine *sm)
{
	int ret = 0;

	return ret;
}



static void wpa_rekey_gtk(void *eloop_ctx, void *timeout_ctx)
{

}



static void wpa_group_set_key_len(struct wpa_group *group, int cipher)
{
	switch (cipher) {
	case WPA_CIPHER_CCMP:
		group->GTK_len = 16;
		break;
	case WPA_CIPHER_TKIP:
		group->GTK_len = 32;
		break;
	case WPA_CIPHER_WEP104:
		group->GTK_len = 13;
		break;
	case WPA_CIPHER_WEP40:
		group->GTK_len = 5;
		break;
	}
}


static int wpa_group_init_gmk_and_counter(struct wpa_authenticator *wpa_auth,
					  struct wpa_group *group)
{
	ssv_type_u8 buf[ETH_ALEN + 8 + sizeof(group)];
	ssv_type_u8 rkey[32];

	//if (os_get_random(group->GMK, WPA_GMK_LEN) < 0)
	//	return -1;
	OS_MemCPY(buf,gDeviceInfo->APInfo->own_addr, ETH_ALEN);
    
	wpa_get_ntp_timestamp(buf + ETH_ALEN);
	if (os_get_random(rkey, sizeof(rkey)) < 0)
		return -1;
    if (sha1_prf(rkey, sizeof(rkey), "Init Counter", buf, sizeof(buf),
		     group->GMK, WPA_NONCE_LEN) < 0)
		return -1;
	wpa_hexdump_key(MSG_INFO, "GMK", group->GMK, WPA_GMK_LEN);

	/*
	 * Counter = PRF-256(Random number, "Init Counter",
	 *                   Local MAC Address || Time)
	 */
	OS_MemCPY(buf, gDeviceInfo->APInfo->own_addr, ETH_ALEN);
    
	wpa_get_ntp_timestamp(buf + ETH_ALEN);
	OS_MemCPY(buf + ETH_ALEN + 8, &group, sizeof(group));
	if (os_get_random(rkey, sizeof(rkey)) < 0)
		return -1;

	if (sha1_prf(rkey, sizeof(rkey), "Init Counter", buf, sizeof(buf),
		     group->Counter, WPA_NONCE_LEN) < 0)
		return -1;
	wpa_hexdump_key(MSG_DEBUG, "Key Counter",
			group->Counter, WPA_NONCE_LEN);

	return 0;
}


static void wpa_group_init(struct wpa_authenticator *wpa_auth,
					 int vlan_id, int delay_init)
{
	struct wpa_group *group;
    group = wpa_auth->group;
	
    group->GN=1;
	wpa_group_set_key_len(group, wpa_auth->conf->wpa_group);

   
	if (wpa_group_init_gmk_and_counter(wpa_auth, group) < 0) {
		LOG_DEBUGF(LOG_L2_AP,("Failed to get random data for WPA "
			   "initialization."));
		
	}
    wpa_gtk_update(wpa_auth, group);
	
    wpa_group_config_group_keys(wpa_auth, group);
   
}


/**
 * wpa_init - Initialize WPA authenticator
 * @addr: Authenticator address
 * @conf: Configuration for WPA authenticator
 * @cb: Callback functions for WPA authenticator
 * Returns: Pointer to WPA authenticator data or %NULL on failure
 */

void wpa_init(struct wpa_authenticator *wpa_auth)
{  
	//OS_MemCPY(wpa_auth->addr, gDeviceInfo->APInfo->own_addr, ETH_ALEN);
	wpa_group_init(wpa_auth, 0, 1);
}


/**
 * wpa_deinit - Deinitialize WPA authenticator
 * @wpa_auth: Pointer to WPA authenticator data from wpa_init()
 */
void wpa_deinit(struct wpa_authenticator *wpa_auth)
{
    //if(wpa_auth->wpa_ie!=NULL)
    //	OS_MemFree(wpa_auth->wpa_ie);
}

void wpa_auth_sta_init(struct wpa_authenticator *wpa_auth, const ssv_type_u8 *addr,struct wpa_state_machine *sm)
{
    if (sm)
    {
        if(sm->last_rx_eapol_key !=NULL)
        {
            OS_MemFree(sm->last_rx_eapol_key);
        }
        if(sm->wpa_ie)
        {
            OS_MemFree(sm->wpa_ie);
        }   

        OS_MemSET(sm,0x0,sizeof(struct wpa_state_machine));
	    OS_MemCPY(sm->addr, addr, ETH_ALEN);
	    sm->wpa_auth = wpa_auth;
    
	    sm->group = wpa_auth->group;
        //sm->group->first_sta_seen = FALSE;
    }
}


int wpa_auth_sta_associated(struct wpa_authenticator *wpa_auth,
			    struct wpa_state_machine *sm)
{
	if (wpa_auth == NULL || !wpa_auth->conf->wpa)
		return -1;

	if (sm->started) {
		OS_MemSET(&sm->key_replay, 0, sizeof(sm->key_replay));
		sm->ReAuthenticationRequest = TRUE;
		return wpa_sm_step(sm);
	}

	sm->started = 1;

	sm->Init = TRUE;
	if (wpa_sm_step(sm) == 1)
		return 1; /* should not really happen */
	sm->Init = FALSE;
	sm->AuthenticationRequest = TRUE;
	return wpa_sm_step(sm);
}


void wpa_auth_sta_no_wpa(struct wpa_state_machine *sm)
{
	/* WPA/RSN was not used - clear WPA state. This is needed if the STA
	 * reassociates back to the same AP while the previous entry for the
	 * STA has not yet been removed. */
	if (sm->started == 0)
		return;

	sm->wpa_key_mgmt = 0;
}


void wpa_free_sta_sm(struct wpa_state_machine *sm)
{
    if(sm->last_rx_eapol_key !=NULL){
	    OS_MemFree(sm->last_rx_eapol_key);
        sm->last_rx_eapol_key = NULL;
    }
    if(sm->wpa_ie){
	    OS_MemFree(sm->wpa_ie);
        sm->wpa_ie = NULL;
    }
   
}


void wpa_auth_sta_deinit(struct wpa_state_machine *sm)
{
	if (sm->started == 0)
		return;
    
	os_cancel_timer(wpa_send_eapol_timeout, (ssv_type_u32)sm->wpa_auth, (ssv_type_u32)sm);
	sm->pending_1_of_4_timeout = 0;
	if (sm->in_step_loop) {
		/* Must not free state machine while wpa_sm_step() is running.
		 * Freeing will be completed in the end of wpa_sm_step(). */
		wpa_printf(MSG_DEBUG, "WPA: Registering pending STA state "
			   "machine deinit for " MACSTR, MAC2STR(sm->addr));
		sm->pending_deinit = 1;
        sm->started = 0;
	} 
    //else
    //{   
    //    if(sm != NULL)
    //    {
	//	    wpa_free_sta_sm(sm);
    //    }
    //}
}


static void wpa_request_new_ptk(struct wpa_state_machine *sm)
{
	if (sm ->started == 0)
		return;

	sm->PTKRequest = TRUE;
    
	sm->PTK_valid = 0;
}


static int wpa_replay_counter_valid(struct wpa_state_machine *sm,
				    const ssv_type_u8 *replay_counter)
{
	int i;
	for (i = 0; i < RSNA_MAX_EAPOL_RETRIES; i++) {
		if (!sm->key_replay[i].valid)
			break;
		if (os_memcmp(replay_counter, sm->key_replay[i].counter,
			      WPA_REPLAY_COUNTER_LEN) == 0)
			return 1;
	}
	return 0;
}





static void wpa_receive_error_report(struct wpa_authenticator *wpa_auth,
				     struct wpa_state_machine *sm, int group)
{
	/* Supplicant reported a Michael MIC error */
	wpa_auth_vlogger(wpa_auth, sm->addr, LOGGER_INFO,
			 "received EAPOL-Key Error Request "
			 "(STA detected Michael MIC failure (group=%d))",
			 group);

	if (group && wpa_auth->conf->wpa_group != WPA_CIPHER_TKIP) {
		wpa_auth_logger(wpa_auth, sm->addr, LOGGER_INFO,
				"ignore Michael MIC failure report since "
				"group cipher is not TKIP");
	} else if (!group && sm->pairwise != WPA_CIPHER_TKIP) {
		wpa_auth_logger(wpa_auth, sm->addr, LOGGER_INFO,
				"ignore Michael MIC failure report since "
				"pairwise cipher is not TKIP");
	} else {
		//wpa_auth_mic_failure_report(wpa_auth, sm->addr);
		sm->dot11RSNAStatsTKIPRemoteMICFailures++;
		wpa_auth->dot11RSNAStatsTKIPRemoteMICFailures++;
	}

	/*
	 * Error report is not a request for a new key handshake, but since
	 * Authenticator may do it, let's change the keys now anyway.
	 */
	wpa_request_new_ptk(sm);
}


void wpa_receive(struct wpa_authenticator *wpa_auth,
		 struct wpa_state_machine *sm,
		 ssv_type_u8 *data, ssv_type_size_t data_len)
{
	struct ieee802_1x_hdr *hdr;
	struct wpa_eapol_key *key;
	ssv_type_u16 key_info, key_data_length;
	enum { PAIRWISE_2, PAIRWISE_4, GROUP_2, REQUEST,
	       SMK_M1, SMK_M3, SMK_ERROR } msg;
	char *msgtxt;
	struct wpa_eapol_ie_parse kde;
	int ft;
	const ssv_type_u8 *eapol_key_ie;
	ssv_type_size_t eapol_key_ie_len;
	if (wpa_auth == NULL || !wpa_auth->conf->wpa || sm ->started == 0)
		return;

	if (data_len < sizeof(*hdr) + sizeof(*key))
		return;
    //add by angel.chiu
    os_cancel_timer(wpa_send_eapol_timeout, (ssv_type_u32)wpa_auth, (ssv_type_u32)sm);
	hdr = (struct ieee802_1x_hdr *) data;
	key = (struct wpa_eapol_key *) (hdr + 1);
	key_info = WPA_GET_BE16(key->key_info);
	key_data_length = WPA_GET_BE16(key->key_data_length);
	wpa_printf(MSG_DEBUG, "WPA: Received EAPOL-Key from " MACSTR
		   " key_info=0x%x type=%u key_data_length=%u sizeof(*hdr)=%d sizeof(*key)=%d",
		   MAC2STR(sm->addr), key_info, key->type, key_data_length,sizeof(*hdr),sizeof(*key));
	if (key_data_length > data_len - sizeof(*hdr) - sizeof(*key)) {
		wpa_printf(MSG_INFO, "WPA: Invalid EAPOL-Key frame - "
			   "key_data overflow (%d > %d)",
			   key_data_length,
			   (data_len - sizeof(*hdr) -sizeof(*key)));
		return;
	}

	if (sm->wpa == WPA_VERSION_WPA2) {
		if (key->type == EAPOL_KEY_TYPE_WPA) {
			/*
			 * Some deployed station implementations seem to send
			 * msg 4/4 with incorrect type value in WPA2 mode.
			 */
			LOG_DEBUGF(LOG_L2_AP,("Workaround: Allow EAPOL-Key "
				   "with unexpected WPA type in RSN mode"));
		} else if (key->type != EAPOL_KEY_TYPE_RSN) {
			wpa_printf(MSG_DEBUG, "Ignore EAPOL-Key with "
				   "unexpected type %d in RSN mode",
				   key->type);
			return;
		}
	} else {
		if (key->type != EAPOL_KEY_TYPE_WPA) {
			wpa_printf(MSG_DEBUG, "Ignore EAPOL-Key with "
				   "unexpected type %d in WPA mode\n\r",
				   key->type);
			return;
		}
	}

	wpa_hexdump(MSG_DEBUG, "WPA: Received Key Nonce",(void *) key->key_nonce,
		    WPA_NONCE_LEN);
	wpa_hexdump(MSG_DEBUG, "WPA: Received Replay Counter",
		    (void *)key->replay_counter, WPA_REPLAY_COUNTER_LEN);

	/* FIX: verify that the EAPOL-Key frame was encrypted if pairwise keys
	 * are set */

	if ((key_info & (WPA_KEY_INFO_SMK_MESSAGE | WPA_KEY_INFO_REQUEST)) ==
	    (WPA_KEY_INFO_SMK_MESSAGE | WPA_KEY_INFO_REQUEST)) {
		if (key_info & WPA_KEY_INFO_ERROR) {
			msg = SMK_ERROR;
			msgtxt = "SMK Error";
		} else {
			msg = SMK_M1;
			msgtxt = "SMK M1";
		}
	} else if (key_info & WPA_KEY_INFO_SMK_MESSAGE) {
		msg = SMK_M3;
		msgtxt = "SMK M3";
	} else if (key_info & WPA_KEY_INFO_REQUEST) {
		msg = REQUEST;
		msgtxt = "Request";
	} else if (!(key_info & WPA_KEY_INFO_KEY_TYPE)) {
		msg = GROUP_2;
		msgtxt = "2/2 Group";
	} else if (key_data_length == 0) {
		msg = PAIRWISE_4;
		msgtxt = "4/4 Pairwise";
	} else {
		msg = PAIRWISE_2;
		msgtxt = "2/4 Pairwise";
	}

	/* TODO: key_info type validation for PeerKey */
	if (msg == REQUEST || msg == PAIRWISE_2 || msg == PAIRWISE_4 ||
	    msg == GROUP_2) {
		ssv_type_u16 ver = key_info & WPA_KEY_INFO_TYPE_MASK;
		if (sm->pairwise == WPA_CIPHER_CCMP) {
			if (wpa_use_aes_cmac(sm) &&
			    ver != WPA_KEY_INFO_TYPE_AES_128_CMAC) {
				wpa_auth_logger(wpa_auth, sm->addr,
						LOGGER_WARNING,
						"advertised support for "
						"AES-128-CMAC, but did not "
						"use it");
				return;
			}

			if (!wpa_use_aes_cmac(sm) &&
			    ver != WPA_KEY_INFO_TYPE_HMAC_SHA1_AES) {
				wpa_auth_logger(wpa_auth, sm->addr,
						LOGGER_WARNING,
						"did not use HMAC-SHA1-AES "
						"with CCMP");
				return;
			}
		}
	}

    
    wpa_hexdump_key(MSG_DEBUG,"key->replay_counter",(void *)key->replay_counter,WPA_REPLAY_COUNTER_LEN);
    wpa_hexdump_key(MSG_DEBUG,"sm->req_replay_counter",sm->req_replay_counter,WPA_REPLAY_COUNTER_LEN);
	if (key_info & WPA_KEY_INFO_REQUEST) {
		if (sm->req_replay_counter_used &&
		    os_memcmp((void *)key->replay_counter,(void *) sm->req_replay_counter,
			      WPA_REPLAY_COUNTER_LEN) <= 0) {
			wpa_auth_logger(wpa_auth, sm->addr, LOGGER_WARNING,
					"received EAPOL-Key request with "
					"replayed counter");
			return;
		}
	}

	if (!(key_info & WPA_KEY_INFO_REQUEST) &&
	    !wpa_replay_counter_valid(sm, (void *)key->replay_counter)) {
		int i;
		wpa_auth_vlogger(wpa_auth, sm->addr, LOGGER_DEBUG,
				 "received EAPOL-Key %s with unexpected "
				 "replay counter", msgtxt);
		for (i = 0; i < RSNA_MAX_EAPOL_RETRIES; i++) {
			if (!sm->key_replay[i].valid)
				break;
			wpa_hexdump(MSG_DEBUG, "pending replay counter",
				    sm->key_replay[i].counter,
				    WPA_REPLAY_COUNTER_LEN);
		}
		wpa_hexdump(MSG_DEBUG, "received replay counter",
			    (void *)key->replay_counter, WPA_REPLAY_COUNTER_LEN);
		return;
	}

	switch (msg) {
	case PAIRWISE_2:
		if (sm->wpa_ptk_state != WPA_PTK_PTKSTART &&
		    sm->wpa_ptk_state != WPA_PTK_PTKCALCNEGOTIATING) {
			wpa_auth_vlogger(wpa_auth, sm->addr, LOGGER_INFO,
					 "received EAPOL-Key msg 2/4 in "
					 "invalid state (%d) - dropped",
					 sm->wpa_ptk_state);
			return;
		}
		//random_add_randomness(key->key_nonce, WPA_NONCE_LEN);
        #if 0
		if (sm->group->reject_4way_hs_for_entropy) {
			/*
			 * The system did not have enough entropy to generate
			 * strong random numbers. Reject the first 4-way
			 * handshake(s) and collect some entropy based on the
			 * information from it. Once enough entropy is
			 * available, the next atempt will trigger GMK/Key
			 * Counter update and the station will be allowed to
			 * continue.
			 */
			wpa_printf(MSG_DEBUG, "WPA: Reject 4-way handshake to "
				   "collect more entropy for random number "
				   "generation\n\r");
			sm->group->reject_4way_hs_for_entropy = FALSE;
			//random_mark_pool_ready();
			sm->group->first_sta_seen = FALSE;
			wpa_sta_disconnect(wpa_auth, sm->addr);
			return;
		}
        #endif
		if (wpa_parse_kde_ies((ssv_type_u8 *) (key + 1), key_data_length,
				      &kde) < 0) {
			wpa_auth_vlogger(wpa_auth, sm->addr, LOGGER_INFO,
					 "received EAPOL-Key msg 2/4 with "
					 "invalid Key Data contents");
			return;
		}
		if (kde.rsn_ie) {
			eapol_key_ie = kde.rsn_ie;
			eapol_key_ie_len = kde.rsn_ie_len;
		} else {
			eapol_key_ie = kde.wpa_ie;
			eapol_key_ie_len = kde.wpa_ie_len;
		}
		ft = sm->wpa == WPA_VERSION_WPA2 &&
			wpa_key_mgmt_ft(sm->wpa_key_mgmt);
		if (sm->wpa_ie == NULL ||
		    wpa_compare_rsn_ie(ft,
				       sm->wpa_ie, sm->wpa_ie_len,
				       eapol_key_ie, eapol_key_ie_len)) {
			//wpa_auth_logger(wpa_auth, sm->addr, LOGGER_INFO,
			//		"WPA IE from (Re)AssocReq did not "
			//		"match with msg 2/4");
			if (sm->wpa_ie) {
				wpa_hexdump(MSG_DEBUG, "WPA IE in AssocReq",
					    sm->wpa_ie, sm->wpa_ie_len);
			}
			wpa_hexdump(MSG_DEBUG, "WPA IE in msg 2/4",
				    eapol_key_ie, eapol_key_ie_len);
			/* MLME-DEAUTHENTICATE.request */
			wpa_sta_disconnect(wpa_auth, sm->addr);
			return;
		}

		break;
	case PAIRWISE_4:
		if (sm->wpa_ptk_state != WPA_PTK_PTKINITNEGOTIATING ||
		    !sm->PTK_valid) {
			wpa_auth_vlogger(wpa_auth, sm->addr, LOGGER_INFO,
					 "received EAPOL-Key msg 4/4 in "
					 "invalid state (%d) - dropped",
					 sm->wpa_ptk_state);
			return;
		}
		break;
	case GROUP_2:
	/*	if (sm->wpa_ptk_group_state != WPA_PTK_GROUP_REKEYNEGOTIATING
		    || !sm->PTK_valid) {
			wpa_auth_vlogger(wpa_auth, sm->addr, LOGGER_INFO,
					 "received EAPOL-Key msg 2/2 in "
					 "invalid state (%d) - dropped",
					 sm->wpa_ptk_group_state);
			return;
		}*/
		break;

	case SMK_M1:
	case SMK_M3:
	case SMK_ERROR:
		return; /* STSL disabled - ignore SMK messages */

	case REQUEST:
		break;
	}
     
	//wpa_auth_vlogger(wpa_auth, sm->addr, LOGGER_DEBUG,
	//		 "received EAPOL-Key frame (%s)");
	if (key_info & WPA_KEY_INFO_ACK) {
		//wpa_auth_logger(wpa_auth, sm->addr, LOGGER_INFO,
		//		"received invalid EAPOL-Key: Key Ack set");
		return;
	}   
	if (!(key_info & WPA_KEY_INFO_MIC)) {
		//wpa_auth_logger(wpa_auth, sm->addr, LOGGER_INFO,
		//		"received invalid EAPOL-Key: Key MIC not set");
        
		return;
	}

	sm->MICVerified = FALSE;
	if (sm->PTK_valid) {
		if (wpa_verify_key_mic(&(sm->PTK), data, data_len)) {
			wpa_auth_logger(wpa_auth, sm->addr, LOGGER_INFO,
					"received EAPOL-Key with invalid MIC");
			return;
		}
        
		sm->MICVerified = TRUE;
        //LOG_DEBUG("++++++++++++++cancel timer\n\r");
		os_cancel_timer(wpa_send_eapol_timeout, (ssv_type_u32)wpa_auth, (ssv_type_u32)sm);
		sm->pending_1_of_4_timeout = 0;
	}

	if (key_info & WPA_KEY_INFO_REQUEST) {
		if (sm->MICVerified) {
			sm->req_replay_counter_used = 1;
			OS_MemCPY((void *)sm->req_replay_counter,(void *) key->replay_counter,
				  WPA_REPLAY_COUNTER_LEN);
		} else {
			wpa_auth_logger(wpa_auth, sm->addr, LOGGER_INFO,
					"received EAPOL-Key request with "
					"invalid MIC");
            os_create_timer(eapol_key_timeout_error,wpa_send_eapol_timeout, wpa_auth, sm,(void*)TIMEOUT_TASK);
			return;
		}

		/*
		 * TODO: should decrypt key data field if encryption was used;
		 * even though MAC address KDE is not normally encrypted,
		 * supplicant is allowed to encrypt it.
		 */
		if (msg == SMK_ERROR) {
			return;
		} else if (key_info & WPA_KEY_INFO_ERROR) {
			wpa_receive_error_report(
				wpa_auth, sm,
				!(key_info & WPA_KEY_INFO_KEY_TYPE));
		} else if (key_info & WPA_KEY_INFO_KEY_TYPE) {
			wpa_auth_logger(wpa_auth, sm->addr, LOGGER_INFO,
					"received EAPOL-Key Request for new "
					"4-Way Handshake");
			wpa_request_new_ptk(sm);

		} else if (key_data_length > 0 &&
			   wpa_parse_kde_ies((const ssv_type_u8 *) (key + 1),
					     key_data_length, &kde) == 0 &&
			   kde.mac_addr) {
		} else {
			wpa_auth_logger(wpa_auth, sm->addr, LOGGER_INFO,
					"received EAPOL-Key Request for GTK "
					"rekeying");
            //LOG_DEBUG("++++++++++++++cancel timer\n\r");
			os_cancel_timer(wpa_rekey_gtk, (ssv_type_u32)wpa_auth, (ssv_type_u32)NULL);
			wpa_rekey_gtk(wpa_auth, NULL);
		}
	} else {
		/* Do not allow the same key replay counter to be reused. This
		 * does also invalidate all other pending replay counters if
		 * retransmissions were used, i.e., we will only process one of
		 * the pending replies and ignore rest if more than one is
		 * received. */
		sm->key_replay[0].valid = FALSE;
	}


    if(sm->last_rx_eapol_key != NULL)     
    {   
	    OS_MemFree(sm->last_rx_eapol_key);
	}
	sm->last_rx_eapol_key = OS_MemAlloc(data_len);
	if (sm->last_rx_eapol_key == NULL)
    {
        os_create_timer(eapol_key_timeout_error,wpa_send_eapol_timeout, wpa_auth, sm,(void*)TIMEOUT_TASK);
		return;
    }
	OS_MemCPY(sm->last_rx_eapol_key, data, data_len);
	sm->last_rx_eapol_key_len = data_len;

	sm->rx_eapol_key_secure = !!(key_info & WPA_KEY_INFO_SECURE);
	sm->EAPOLKeyReceived = TRUE;
	sm->EAPOLKeyPairwise = !!(key_info & WPA_KEY_INFO_KEY_TYPE);
	sm->EAPOLKeyRequest = !!(key_info & WPA_KEY_INFO_REQUEST);
	OS_MemCPY((void *)sm->SNonce, (void *)key->key_nonce, WPA_NONCE_LEN);
	wpa_sm_step(sm);
}


static int wpa_gmk_to_gtk(const ssv_type_u8 *gmk, const char *label, const ssv_type_u8 *addr,
			  const ssv_type_u8 *gnonce, ssv_type_u8 *gtk, ssv_type_size_t gtk_len)
{
	ssv_type_u8 data[ETH_ALEN + WPA_NONCE_LEN + 8 + 16];
	ssv_type_u8 *pos;
	int ret = 0;

	/* GTK = PRF-X(GMK, "Group key expansion",
	 *	AA || GNonce || Time || random data)
	 * The example described in the IEEE 802.11 standard uses only AA and
	 * GNonce as inputs here. Add some more entropy since this derivation
	 * is done only at the Authenticator and as such, does not need to be
	 * exactly same.
	 */
	OS_MemCPY(data, addr, ETH_ALEN);
	OS_MemCPY(data + ETH_ALEN, gnonce, WPA_NONCE_LEN);
	pos = data + ETH_ALEN + WPA_NONCE_LEN;
	wpa_get_ntp_timestamp(pos);
    wpa_hexdump_key(MSG_DEBUG, "timestamp",
			pos, 8);
	pos += 8;
	os_get_random(pos, 16);
    wpa_hexdump_key(MSG_DEBUG, "random",
			pos, 16);

	if (sha1_prf(gmk, WPA_GMK_LEN, label, data, sizeof(data), gtk, gtk_len)
	    < 0)
		ret = -1;


	return ret;
}


static void wpa_send_eapol_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct wpa_authenticator *wpa_auth = eloop_ctx;
	struct wpa_state_machine *sm = timeout_ctx;
    
    if(sm->started == 0)
        return;   
	sm->pending_1_of_4_timeout = 0;
	wpa_auth_logger(wpa_auth, sm->addr, LOGGER_DEBUG, "EAPOL-Key timeout");
	sm->TimeoutEvt = TRUE;
	wpa_sm_step(sm);
}


void __wpa_send_eapol(struct wpa_authenticator *wpa_auth,
		      struct wpa_state_machine *sm, int key_info,
		      const ssv_type_u8 *key_rsc, const ssv_type_u8 *nonce,
		      const ssv_type_u8 *kde, ssv_type_size_t kde_len,
		      int keyidx, int encr, int force_version)
{
	struct ieee802_1x_hdr *hdr;
	struct wpa_eapol_key *key;
	ssv_type_size_t len;
	int alg;
	int key_data_len, pad_len = 0;
	ssv_type_u8 *buf, *pos;
	int version, pairwise;
	int i;

	len = sizeof(struct ieee802_1x_hdr) + sizeof(struct wpa_eapol_key);

	if (force_version)
		version = force_version;
	else if (wpa_use_aes_cmac(sm))
		version = WPA_KEY_INFO_TYPE_AES_128_CMAC;
	else if (sm->pairwise == WPA_CIPHER_CCMP)
		version = WPA_KEY_INFO_TYPE_HMAC_SHA1_AES;
	else
		version = WPA_KEY_INFO_TYPE_HMAC_MD5_RC4;

	pairwise = key_info & WPA_KEY_INFO_KEY_TYPE;

	wpa_printf(MSG_DEBUG, "WPA: Send EAPOL(version=%d secure=%d mic=%d "
		   "ack=%d install=%d pairwise=%d kde_len=%d keyidx=%d "
		   "encr=%d)\n\r",
		   version,
		   (key_info & WPA_KEY_INFO_SECURE) ? 1 : 0,
		   (key_info & WPA_KEY_INFO_MIC) ? 1 : 0,
		   (key_info & WPA_KEY_INFO_ACK) ? 1 : 0,
		   (key_info & WPA_KEY_INFO_INSTALL) ? 1 : 0,
		   pairwise, kde_len, keyidx, encr);

	key_data_len = kde_len;

	if ((version == WPA_KEY_INFO_TYPE_HMAC_SHA1_AES ||
	     version == WPA_KEY_INFO_TYPE_AES_128_CMAC) && encr) {
		pad_len = key_data_len % 8;
		if (pad_len)
			pad_len = 8 - pad_len;
		key_data_len += pad_len + 8;
	}

	len += key_data_len;

	hdr = OS_MemAlloc(len);

	if (hdr == NULL)
		return;
    OS_MemSET((void *)hdr,0x0,len);
	hdr->version = 1;  //for checck 1 or 2
	hdr->type = IEEE802_1X_TYPE_EAPOL_KEY;
	hdr->length = host_to_be16(len  - sizeof(*hdr));
	key = (struct wpa_eapol_key *) (hdr + 1);

	key->type = sm->wpa == WPA_VERSION_WPA2 ?
		EAPOL_KEY_TYPE_RSN : EAPOL_KEY_TYPE_WPA;
	key_info |= version;
	if (encr && sm->wpa == WPA_VERSION_WPA2)
		key_info |= WPA_KEY_INFO_ENCR_KEY_DATA;
	if (sm->wpa != WPA_VERSION_WPA2)
		key_info |= keyidx << WPA_KEY_INFO_KEY_INDEX_SHIFT;
	WPA_PUT_BE16(key->key_info, key_info);

	alg = pairwise ? sm->pairwise : wpa_auth->conf->wpa_group;
	switch (alg) {
	case WPA_CIPHER_CCMP:
		WPA_PUT_BE16(key->key_length, 16);
		break;
	case WPA_CIPHER_TKIP:
		WPA_PUT_BE16(key->key_length, 32);
		break;
	case WPA_CIPHER_WEP40:
		WPA_PUT_BE16(key->key_length, 5);
		break;
	case WPA_CIPHER_WEP104:
		WPA_PUT_BE16(key->key_length, 13);
		break;
	}
	if (key_info & WPA_KEY_INFO_SMK_MESSAGE)
		WPA_PUT_BE16(key->key_length, 0);

   
	/* FIX: STSL: what to use as key_replay_counter? */
	for (i = RSNA_MAX_EAPOL_RETRIES - 1; i > 0; i--) {
		sm->key_replay[i].valid = sm->key_replay[i - 1].valid;
		OS_MemCPY(sm->key_replay[i].counter,
			  sm->key_replay[i - 1].counter,
			  WPA_REPLAY_COUNTER_LEN);
	}
	inc_byte_array(sm->key_replay[0].counter, WPA_REPLAY_COUNTER_LEN);
	OS_MemCPY((void *)key->replay_counter,(void *)sm->key_replay[0].counter,
		  WPA_REPLAY_COUNTER_LEN);
	sm->key_replay[0].valid = TRUE;

	if (nonce)
    {   
		OS_MemCPY((void *)key->key_nonce, nonce, WPA_NONCE_LEN);
        wpa_hexdump_key(MSG_DEBUG,"ANONCE:",nonce,WPA_NONCE_LEN);
    }
	if (key_rsc)
		OS_MemCPY((void *)key->key_rsc, key_rsc, WPA_KEY_RSC_LEN);
    
	if (kde && !encr) {
		OS_MemCPY((void *)(key + 1),(void *) kde, kde_len);
		WPA_PUT_BE16(key->key_data_length, kde_len);
	} else if (encr && kde) {
		buf = OS_MemAlloc(key_data_len);

		if (buf == NULL) {
			OS_MemFree((void *)hdr);
			return;
		}
        OS_MemSET(buf,0x0,key_data_len);
		pos = buf;
		OS_MemCPY(pos, kde, kde_len);
		pos += kde_len;

		if (pad_len)
			*pos++ = 0xdd;

		wpa_hexdump_key(MSG_DEBUG, "Plaintext EAPOL-Key Key Data",
				buf, key_data_len);
		if (version == WPA_KEY_INFO_TYPE_HMAC_SHA1_AES ||
		    version == WPA_KEY_INFO_TYPE_AES_128_CMAC) {
			if (aes_wrap((void *)sm->PTK.kek, (key_data_len - 8) / 8, buf,
				     (ssv_type_u8 *) (key + 1))) {
				OS_MemFree((void *)hdr);
				OS_MemFree((void *)buf);
				return;
			}
			WPA_PUT_BE16(key->key_data_length, key_data_len);
		} else {
			ssv_type_u8 ek[32];
			OS_MemCPY((void *)key->key_iv,
				  sm->group->Counter + WPA_NONCE_LEN - 16, 16);
			inc_byte_array(sm->group->Counter, WPA_NONCE_LEN);
			OS_MemCPY((void *)ek, (void *)key->key_iv, 16);
			OS_MemCPY((void *)(ek + 16),(void *)sm->PTK.kek, 16);
			OS_MemCPY((void *)(key + 1), (void *)buf, key_data_len);
			rc4_skip((void *)ek, 32, 256, (ssv_type_u8 *) (key + 1), key_data_len);
			WPA_PUT_BE16(key->key_data_length, key_data_len);
		}
		OS_MemFree(buf);
	}
    
	if (key_info & WPA_KEY_INFO_MIC) {
		if (!sm->PTK_valid) {
			//wpa_auth_logger(wpa_auth, sm->addr, LOGGER_DEBUG,
			//		"PTK not valid when sending EAPOL-Key "
			//		"frame");
			OS_MemFree((void *)hdr);
			return;
		}
		wpa_eapol_key_mic((void *)sm->PTK.kck, version, (ssv_type_u8 *) hdr, len,
				  (void *)key->key_mic);
	}
    
	wpa_auth_send_eapol(wpa_auth, sm->addr, (ssv_type_u8 *) hdr, len,
			    sm->pairwise_set);

    
	OS_MemFree((void *)hdr);
}


static void wpa_send_eapol(struct wpa_authenticator *wpa_auth,
			   struct wpa_state_machine *sm, int key_info,
			   const ssv_type_u8 *key_rsc, const ssv_type_u8 *nonce,
			   const ssv_type_u8 *kde, ssv_type_size_t kde_len,
			   int keyidx, int encr)
{
	int timeout_ms;
	int pairwise = key_info & WPA_KEY_INFO_KEY_TYPE;
	int ctr;

	if (sm ->started == 0)
		return;

	__wpa_send_eapol(wpa_auth, sm, key_info, key_rsc, nonce, kde, kde_len,
			 keyidx, encr, 0);

	ctr = sm->TimeoutCtr ;
	if (ctr == 1 /*&& wpa_auth->conf->tx_status*/)
		timeout_ms = eapol_key_timeout_first;
	else
		timeout_ms = eapol_key_timeout_subseq;
	if (pairwise && ctr == 1 && !(key_info & WPA_KEY_INFO_MIC))
		sm->pending_1_of_4_timeout = 1;
	wpa_printf(MSG_DEBUG, "WPA: Use EAPOL-Key timeout of %u ms (retry "
		   "counter %d)\n\r", timeout_ms, ctr);  
    os_cancel_timer(wpa_send_eapol_timeout, (ssv_type_u32)wpa_auth, (ssv_type_u32)NULL);
	os_create_timer(timeout_ms,wpa_send_eapol_timeout, wpa_auth, sm,(void*)TIMEOUT_TASK);
}


static int wpa_verify_key_mic(struct wpa_ptk *PTK, ssv_type_u8 *data, ssv_type_size_t data_len)
{
	struct ieee802_1x_hdr *hdr;
	struct wpa_eapol_key *key;
	ssv_type_u16 key_info;
	int ret = 0;
	ssv_type_u8 mic[16];

	if (data_len < sizeof(*hdr) + sizeof(*key))
		return -1;

	hdr = (struct ieee802_1x_hdr *) data;
	key = (struct wpa_eapol_key *)( hdr+1);
	key_info = WPA_GET_BE16(key->key_info);
    
	OS_MemCPY((void *)mic, (void *)key->key_mic, 16);
	OS_MemSET((void *)key->key_mic, 0, 16);
    //startCount = os_getSysTime;
	if (wpa_eapol_key_mic((ssv_type_u8 *)PTK->kck, key_info & WPA_KEY_INFO_TYPE_MASK,
			      data, data_len, (ssv_type_u8 *)key->key_mic) ||
	    os_memcmp((void *)mic,(void *)key->key_mic, 16) != 0)
	{		   
	    ret = -1;
        
    }
    //endCount = os_getSysTime;
	OS_MemCPY((void *)key->key_mic, (void *)mic, 16);
	return ret;
}


void wpa_remove_ptk(struct wpa_state_machine *sm)
{
    if(sm->pairwise_set == TRUE) 
	{	     
	    OS_MemSET((void *)(&sm->PTK), 0, sizeof(sm->PTK));
         LOG_DEBUGF(LOG_L2_AP,( "%s\n\r", "wpa_remove_ptk"));
	    wpa_auth_set_key(sm->wpa_auth,WPA_ALG_NONE, sm->addr, 0, NULL, 0);
	    sm->pairwise_set = FALSE;
    }
	
}


int wpa_auth_sm_event(struct wpa_state_machine *sm, wpa_event event)
{
	int remove_ptk = 1;

	if (sm->started == 0)
		return -1;

	wpa_auth_vlogger(sm->wpa_auth, sm->addr, LOGGER_DEBUG,
			 "event %d notification", event);
    LOG_DEBUGF(LOG_L2_AP,( "event %d notification", event));

	switch (event) {
	case WPA_AUTH:
	case WPA_ASSOC:
		break;
	case WPA_DEAUTH:
	case WPA_DISASSOC:
		sm->DeauthenticationRequest = TRUE;
		break;
	case WPA_REAUTH:
	case WPA_REAUTH_EAPOL:
		if (!sm->started) {
			/*
			 * When using WPS, we may end up here if the STA
			 * manages to re-associate without the previous STA
			 * entry getting removed. Consequently, we need to make
			 * sure that the WPA state machines gets initialized
			 * properly at this point.
			 */
			LOG_DEBUGF(LOG_L2_AP,( "WPA state machine had not been "
				   "started - initialize now"));
			sm->started = 1;
			sm->Init = TRUE;
			if (wpa_sm_step(sm) == 1)
				return 1; /* should not really happen */
			sm->Init = FALSE;
			sm->AuthenticationRequest = TRUE;
			break;
		}
		
		sm->ReAuthenticationRequest = TRUE;
		break;
	case WPA_ASSOC_FT:

		break;
	}



	if (remove_ptk) {
		sm->PTK_valid = FALSE;       
		OS_MemSET((void *)(&sm->PTK), 0, sizeof(sm->PTK));

		if (event != WPA_REAUTH_EAPOL)
			wpa_remove_ptk(sm);
	}

	return wpa_sm_step(sm);
}


static enum wpa_alg wpa_alg_enum(int alg)
{
	switch (alg) {
	case WPA_CIPHER_CCMP:
		return WPA_ALG_CCMP;
	case WPA_CIPHER_TKIP:
		return WPA_ALG_TKIP;
	case WPA_CIPHER_WEP104:
	case WPA_CIPHER_WEP40:
		return WPA_ALG_WEP;
	default:
		return WPA_ALG_NONE;
	}
}


SM_STATE(WPA_PTK, INITIALIZE)
{
	SM_ENTRY_MA(WPA_PTK, INITIALIZE, wpa_ptk);
	if (sm->Init) {
		/* Init flag is not cleared here, so avoid busy
		 * loop by claiming nothing changed. */
		sm->changed = FALSE;
	}

	sm->keycount = 0;
	
	if (1 /* Unicast cipher supported AND (ESS OR ((IBSS or WDS) and
	       * Local AA > Remote AA)) */) {
		sm->Pair = TRUE;
	}
	
	wpa_remove_ptk(sm);
	sm->TimeoutCtr = 0;
}


SM_STATE(WPA_PTK, DISCONNECT)
{
	SM_ENTRY_MA(WPA_PTK, DISCONNECT, wpa_ptk);
	sm->Disconnect = FALSE;
	wpa_sta_disconnect(sm->wpa_auth, sm->addr);
}


SM_STATE(WPA_PTK, DISCONNECTED)
{
	SM_ENTRY_MA(WPA_PTK, DISCONNECTED, wpa_ptk);
	sm->DeauthenticationRequest = FALSE;
}


SM_STATE(WPA_PTK, AUTHENTICATION)
{
	SM_ENTRY_MA(WPA_PTK, AUTHENTICATION, wpa_ptk);
	OS_MemSET((void *)(&sm->PTK), 0, sizeof(sm->PTK));
	sm->PTK_valid = FALSE;
	LOG_DEBUG("%s\n\r",__FUNCTION__);
	sm->AuthenticationRequest = FALSE;
}


SM_STATE(WPA_PTK, AUTHENTICATION2)
{
	SM_ENTRY_MA(WPA_PTK, AUTHENTICATION2, wpa_ptk);

	//if (!sm->group->first_sta_seen) {
	//	wpa_group_first_station(sm->wpa_auth, sm->group);
	//	sm->group->first_sta_seen = TRUE;
	//}
	OS_MemCPY(sm->ANonce, sm->group->Counter, WPA_NONCE_LEN);
	wpa_hexdump(MSG_DEBUG, "WPA: Assign ANonce", sm->ANonce,
		    WPA_NONCE_LEN);
	inc_byte_array(sm->group->Counter, WPA_NONCE_LEN);
	sm->ReAuthenticationRequest = FALSE;
	/* IEEE 802.11i does not clear TimeoutCtr here, but this is more
	 * logical place than INITIALIZE since AUTHENTICATION2 can be
	 * re-entered on ReAuthenticationRequest without going through
	 * INITIALIZE. */
	sm->TimeoutCtr = 0;
}


SM_STATE(WPA_PTK, INITPMK)
{
	//u8 msk[2 * PMK_LEN];
	//size_t len = 2 * PMK_LEN;

	SM_ENTRY_MA(WPA_PTK, INITPMK, wpa_ptk);

    //if (wpa_auth_get_msk(sm->wpa_auth, sm->addr, msk, &len) == 0) {
	//	wpa_printf(MSG_DEBUG, "WPA: PMK from EAPOL state machine "
	//		   "(len=%d)", len);
	//	OS_MemCPY(sm->PMK, msk, PMK_LEN);

	//} else {
	//	wpa_printf(MSG_DEBUG, "WPA: Could not get PMK");
	//}

	sm->req_replay_counter_used = 0;
	/* IEEE 802.11i does not set keyRun to FALSE, but not doing this
	 * will break reauthentication since EAPOL state machines may not be
	 * get into AUTHENTICATING state that clears keyRun before WPA state
	 * machine enters AUTHENTICATION2 state and goes immediately to INITPMK
	 * state and takes PMK from the previously used AAA Key. This will
	 * eventually fail in 4-Way Handshake because Supplicant uses PMK
	 * derived from the new AAA Key. Setting keyRun = FALSE here seems to
	 * be good workaround for this issue. */
	
}


SM_STATE(WPA_PTK, INITPSK)
{
	const ssv_type_u8 *psk;
	SM_ENTRY_MA(WPA_PTK, INITPSK, wpa_ptk);
	psk = wpa_auth_get_psk(sm->wpa_auth, sm->addr, NULL);
	if (psk) {
		OS_MemCPY(sm->PMK, psk, PMK_LEN);
	}
	sm->req_replay_counter_used = 0;
}


SM_STATE(WPA_PTK, PTKSTART)
{
	ssv_type_u8 /*buf[2 + RSN_SELECTOR_LEN + PMKID_LEN],*/ *pmkid = NULL;
	ssv_type_size_t pmkid_len = 0;

	SM_ENTRY_MA(WPA_PTK, PTKSTART, wpa_ptk);
	sm->PTKRequest = FALSE;
	sm->TimeoutEvt = FALSE;

	sm->TimeoutCtr++;
	if (sm->TimeoutCtr > (int) dot11RSNAConfigPairwiseUpdateCount) {
		/* No point in sending the EAPOL-Key - we will disconnect
		 * immediately following this. */
		return;
	}
    
	wpa_send_eapol(sm->wpa_auth, sm,
		       WPA_KEY_INFO_ACK | WPA_KEY_INFO_KEY_TYPE, NULL,
		       sm->ANonce, pmkid, pmkid_len, 0, 0);
}


static int wpa_derive_ptk(struct wpa_state_machine *sm, const ssv_type_u8 *pmk,
			  struct wpa_ptk *ptk)
{
	ssv_type_size_t ptk_len = sm->pairwise == WPA_CIPHER_CCMP ? 48 : 64;

    //wpa_pmk_to_ptk(pmk, PMK_LEN, "Pairwise key expansion",
	//	       gDeviceInfo->APInfo->own_addr, sm->addr, sm->ANonce, sm->SNonce,
	//	       (u8 *)ptk, ptk_len,
	//	       wpa_key_mgmt_sha256(sm->wpa_key_mgmt));
	
    
	ssv_type_u8 data[2 * ETH_ALEN + 2 * WPA_NONCE_LEN];
   
	if (ssv6xxx_memcmp(gDeviceInfo->APInfo->own_addr,sm->addr , ETH_ALEN) < 0) {
		OS_MemCPY(data,gDeviceInfo->APInfo->own_addr, ETH_ALEN);
		OS_MemCPY(data + ETH_ALEN, sm->addr, ETH_ALEN);
	} else {
		OS_MemCPY(data, sm->addr, ETH_ALEN);
		OS_MemCPY(data + ETH_ALEN, gDeviceInfo->APInfo->own_addr, ETH_ALEN);
	}

	if (ssv6xxx_memcmp(sm->ANonce, sm->SNonce, WPA_NONCE_LEN) < 0) {
		OS_MemCPY(data + 2 * ETH_ALEN, sm->ANonce, WPA_NONCE_LEN);
		OS_MemCPY(data + 2 * ETH_ALEN + WPA_NONCE_LEN, sm->SNonce,
			  WPA_NONCE_LEN);
	} else {
		OS_MemCPY(data + 2 * ETH_ALEN, sm->SNonce, WPA_NONCE_LEN);
		OS_MemCPY(data + 2 * ETH_ALEN + WPA_NONCE_LEN, sm->ANonce,
			  WPA_NONCE_LEN);
	}
    

    sha1_prf(pmk, PMK_LEN,  "Pairwise key expansion", data, sizeof(data), (ssv_type_u8 *)ptk,
        			 ptk_len);

	wpa_printf(MSG_DEBUG, "\nWPA: PTK derivation - A1=" MACSTR " A2=" MACSTR,
               MAC2STR(gDeviceInfo->APInfo->own_addr), MAC2STR(sm->addr));
	wpa_hexdump(MSG_DEBUG, "\nWPA: Nonce1", sm->ANonce, WPA_NONCE_LEN);
	wpa_hexdump(MSG_DEBUG, "\nWPA: Nonce2", sm->SNonce, WPA_NONCE_LEN);
	wpa_hexdump_key(MSG_DEBUG, "\nWPA: PMK", pmk, PMK_LEN);
	wpa_hexdump_key(MSG_DEBUG, "\nWPA: PTK",(ssv_type_u8 *) ptk, ptk_len);


	return 0;
}


SM_STATE(WPA_PTK, PTKCALCNEGOTIATING)
{
	struct wpa_ptk PTK;
	int ok = 0;
	const ssv_type_u8 *pmk = NULL;

	SM_ENTRY_MA(WPA_PTK, PTKCALCNEGOTIATING, wpa_ptk);
	sm->EAPOLKeyReceived = FALSE;

	/* WPA with IEEE 802.1X: use the derived PMK from EAP
	 * WPA-PSK: iterate through possible PSKs and select the one matching
	 * the packet */
	for (;;) {
		if (wpa_key_mgmt_wpa_psk(sm->wpa_key_mgmt)) {
			pmk = wpa_auth_get_psk(sm->wpa_auth, sm->addr, pmk);
			if (pmk == NULL)
				break;
		} else
			pmk = sm->PMK;

		wpa_derive_ptk(sm, pmk, &PTK);
       

		if (wpa_verify_key_mic((struct wpa_ptk *)&PTK, (ssv_type_u8 *)sm->last_rx_eapol_key,
				       sm->last_rx_eapol_key_len) == 0) {
			ok = 1;
            
			break;
		}
        break;
		if (!wpa_key_mgmt_wpa_psk(sm->wpa_key_mgmt))
			break;
	}
    
	if (!ok) {
		wpa_auth_logger(sm->wpa_auth, sm->addr, LOGGER_DEBUG,
				"invalid MIC in msg 2/4 of 4-Way Handshake");
		return;
	}

	sm->pending_1_of_4_timeout = 0;
	os_cancel_timer(wpa_send_eapol_timeout, (ssv_type_u32)sm->wpa_auth, (ssv_type_u32)sm);

	if (wpa_key_mgmt_wpa_psk(sm->wpa_key_mgmt)) {
		/* PSK may have changed from the previous choice, so update
		 * state machine data based on whatever PSK was selected here.
		 */
		OS_MemCPY(sm->PMK, pmk, PMK_LEN);
	}

	sm->MICVerified = TRUE;

	OS_MemCPY((void *)(&sm->PTK), (void *)&PTK, sizeof(PTK));
	sm->PTK_valid = TRUE;
    //LOG_DEBUG("+++++++++++++++++%s:%d,ptk_vaid=%d\n\r",__FUNCTION__,__LINE__,sm->PTK_valid);
}


SM_STATE(WPA_PTK, PTKCALCNEGOTIATING2)
{
	SM_ENTRY_MA(WPA_PTK, PTKCALCNEGOTIATING2, wpa_ptk);
	sm->TimeoutCtr = 0;
}

SM_STATE(WPA_PTK, PTKINITNEGOTIATING)
{
	ssv_type_u8 rsc[WPA_KEY_RSC_LEN], *_rsc, *gtk, *kde, *pos;
	ssv_type_size_t gtk_len, kde_len;
	struct wpa_group *gsm = sm->group;
	ssv_type_u8 *wpa_ie;
	int wpa_ie_len, secure, keyidx, encr = 0;

	SM_ENTRY_MA(WPA_PTK, PTKINITNEGOTIATING, wpa_ptk);
	sm->TimeoutEvt = FALSE;

	sm->TimeoutCtr++;
	if (sm->TimeoutCtr > (int) dot11RSNAConfigPairwiseUpdateCount) {
		/* No point in sending the EAPOL-Key - we will disconnect
		 * immediately following this. */
		return;
	}

	/* Send EAPOL(1, 1, 1, Pair, P, RSC, ANonce, MIC(PTK), RSNIE, [MDIE],
	   GTK[GN], IGTK, [FTIE], [TIE * 2])
	 */
	OS_MemSET(rsc, 0, WPA_KEY_RSC_LEN);
	wpa_auth_get_seqnum(sm->wpa_auth, NULL, gsm->GN, rsc);
	/* If FT is used, wpa_auth->wpa_ie includes both RSNIE and MDIE */
	wpa_ie = sm->wpa_auth->wpa_ie;
	//wpa_ie_len = sm->wpa_auth->wpa_ie_len;
	wpa_ie_len = WPA_IE_LEN;
	if (sm->wpa == WPA_VERSION_WPA &&
	    (sm->wpa_auth->conf->wpa & WPA_PROTO_RSN) &&
	    wpa_ie_len > wpa_ie[1] + 2 && wpa_ie[0] == WLAN_EID_RSN) {
		/* WPA-only STA, remove RSN IE */
		wpa_ie = wpa_ie + wpa_ie[1] + 2;
		wpa_ie_len = wpa_ie[1] + 2;
	}
	wpa_auth_logger(sm->wpa_auth, sm->addr, LOGGER_DEBUG,
			"sending 3/4 msg of 4-Way Handshake");
	if (sm->wpa == WPA_VERSION_WPA2) {
		/* WPA2 send GTK in the 4-way handshake */
		secure = 1;
		gtk = gsm->GTK[gsm->GN - 1];
		gtk_len = gsm->GTK_len;
		keyidx = gsm->GN;
		_rsc = rsc;
		encr = 1;
	} else {
		/* WPA does not include GTK in msg 3/4 */
		secure = 0;
		gtk = NULL;
		gtk_len = 0;
		keyidx = 0;
		_rsc = NULL;
		if (sm->rx_eapol_key_secure) {
			/*
			 * It looks like Windows 7 supplicant tries to use
			 * Secure bit in msg 2/4 after having reported Michael
			 * MIC failure and it then rejects the 4-way handshake
			 * if msg 3/4 does not set Secure bit. Work around this
			 * by setting the Secure bit here even in the case of
			 * WPA if the supplicant used it first.
			 */
			wpa_auth_logger(sm->wpa_auth, sm->addr, LOGGER_DEBUG,
					"STA used Secure bit in WPA msg 2/4 - "
					"set Secure for 3/4 as workaround");
			secure = 1;
		}
	}
    
	kde_len = wpa_ie_len ;
	if (gtk)
		kde_len += 2 + RSN_SELECTOR_LEN + 2 + gtk_len;

	kde = OS_MemAlloc(kde_len);

	if (kde == NULL)
	{	
	    os_create_timer(eapol_key_timeout_error,wpa_send_eapol_timeout, sm->wpa_auth, sm,(void*)TIMEOUT_TASK);
	    return;
    }
    OS_MemSET(kde,0x0,kde_len);
	pos = kde;
	OS_MemCPY(pos, wpa_ie, wpa_ie_len);
	pos += wpa_ie_len;

	if (gtk) {
        wpa_hexdump_key(MSG_INFO,"Send messeage gtk",gtk,gtk_len);
		{ssv_type_u8 hdr[2];
		hdr[0] = keyidx & 0x03;
		hdr[1] = 0;
		pos = wpa_add_kde(pos, RSN_KEY_DATA_GROUPKEY, hdr, 2,
				  gtk, gtk_len);
		}
	}
	//pos = ieee80211w_kde_add(sm, pos);
	wpa_send_eapol(sm->wpa_auth, sm,
		       (secure ? WPA_KEY_INFO_SECURE : 0) | WPA_KEY_INFO_MIC |
		       WPA_KEY_INFO_ACK | WPA_KEY_INFO_INSTALL |
		       WPA_KEY_INFO_KEY_TYPE,
		       _rsc, sm->ANonce, kde, pos - kde, keyidx, encr);
	OS_MemFree(kde);
}


SM_STATE(WPA_PTK, PTKINITDONE)
{
	SM_ENTRY_MA(WPA_PTK, PTKINITDONE, wpa_ptk);
	sm->EAPOLKeyReceived = FALSE;
	if (sm->Pair) {
		enum wpa_alg alg;
		int klen;
		if (sm->pairwise == WPA_CIPHER_TKIP) {
			alg = WPA_ALG_TKIP;
			klen = 32;
		} else {
			alg = WPA_ALG_CCMP;
			klen = 16;
		}
		wpa_auth_set_key(sm->wpa_auth, alg, sm->addr, 0,
				     (void *)sm->PTK.tk1, klen);
		/* FIX: MLME-SetProtection.Request(TA, Tx_Rx) */
		sm->pairwise_set = TRUE;

	}

    {
        APStaInfo_st *sta;
        sta = APStaInfo_FindStaByAddr((ETHER_ADDR *)sm->addr);
        if(sta!=NULL)
            ap_sta_set_authorized(gDeviceInfo->APInfo, sta, 1);
    }
	//sm->has_GTK = TRUE;
	wpa_auth_vlogger(sm->wpa_auth, sm->addr, LOGGER_INFO,
			 "pairwise key handshake completed (%s)",
			 sm->wpa == WPA_VERSION_WPA ? "WPA" : "RSN");


}


SM_STEP(WPA_PTK)
{
	struct wpa_authenticator *wpa_auth = sm->wpa_auth;

	if (sm->Init)
		SM_ENTER(WPA_PTK, INITIALIZE);
	else if (sm->Disconnect
		 /* || FIX: dot11RSNAConfigSALifetime timeout */) {
		wpa_auth_logger(wpa_auth, sm->addr, LOGGER_DEBUG,
				"WPA_PTK: sm->Disconnect");
		SM_ENTER(WPA_PTK, DISCONNECT);
	}
	else if (sm->DeauthenticationRequest)
		SM_ENTER(WPA_PTK, DISCONNECTED);
	else if (sm->AuthenticationRequest)
		SM_ENTER(WPA_PTK, AUTHENTICATION);
	else if (sm->ReAuthenticationRequest)
		SM_ENTER(WPA_PTK, AUTHENTICATION2);
	else if (sm->PTKRequest)
		SM_ENTER(WPA_PTK, PTKSTART);
	else switch (sm->wpa_ptk_state) {
	case WPA_PTK_INITIALIZE:
		break;
	case WPA_PTK_DISCONNECT:
		SM_ENTER(WPA_PTK, DISCONNECTED);
		break;
	case WPA_PTK_DISCONNECTED:
		SM_ENTER(WPA_PTK, INITIALIZE);
		break;
	case WPA_PTK_AUTHENTICATION:
		SM_ENTER(WPA_PTK, AUTHENTICATION2);
		break;
	case WPA_PTK_AUTHENTICATION2:
		if (wpa_key_mgmt_wpa_ieee8021x(sm->wpa_key_mgmt)/* &&
		    wpa_auth_get_eapol(sm->wpa_auth, sm->addr,
				       WPA_EAPOL_keyRun) > 0*/)
			SM_ENTER(WPA_PTK, INITPMK);
		else if (wpa_key_mgmt_wpa_psk(sm->wpa_key_mgmt)
			 /* FIX: && 802.1X::keyRun */)
			SM_ENTER(WPA_PTK, INITPSK);
		break;
	case WPA_PTK_INITPMK:
		
			wpa_auth->dot11RSNA4WayHandshakeFailures++;
			wpa_auth_logger(sm->wpa_auth, sm->addr, LOGGER_INFO,
					"INITPMK - keyAvailable = false");
			SM_ENTER(WPA_PTK, DISCONNECT);
		
		break;
	case WPA_PTK_INITPSK:
		if (wpa_auth_get_psk(sm->wpa_auth, sm->addr, NULL))
			SM_ENTER(WPA_PTK, PTKSTART);
		else {
			wpa_auth_logger(sm->wpa_auth, sm->addr, LOGGER_INFO,
					"no PSK configured for the STA");
			wpa_auth->dot11RSNA4WayHandshakeFailures++;
			SM_ENTER(WPA_PTK, DISCONNECT);
		}
		break;
	case WPA_PTK_PTKSTART:
		if (sm->EAPOLKeyReceived && !sm->EAPOLKeyRequest &&
		    sm->EAPOLKeyPairwise)
			SM_ENTER(WPA_PTK, PTKCALCNEGOTIATING);
		else if (sm->TimeoutCtr >
			 (int) dot11RSNAConfigPairwiseUpdateCount) {
			wpa_auth->dot11RSNA4WayHandshakeFailures++;
			wpa_auth_vlogger(sm->wpa_auth, sm->addr, LOGGER_DEBUG,
					 "PTKSTART: Retry limit %d reached",
					 dot11RSNAConfigPairwiseUpdateCount);
			SM_ENTER(WPA_PTK, DISCONNECT);
		} else if (sm->TimeoutEvt)
			SM_ENTER(WPA_PTK, PTKSTART);
		break;
	case WPA_PTK_PTKCALCNEGOTIATING:
		if (sm->MICVerified)
			SM_ENTER(WPA_PTK, PTKCALCNEGOTIATING2);
		else if (sm->EAPOLKeyReceived && !sm->EAPOLKeyRequest &&
			 sm->EAPOLKeyPairwise)
			SM_ENTER(WPA_PTK, PTKCALCNEGOTIATING);
		else if (sm->TimeoutEvt)
			SM_ENTER(WPA_PTK, PTKSTART);
		break;
	case WPA_PTK_PTKCALCNEGOTIATING2:
		SM_ENTER(WPA_PTK, PTKINITNEGOTIATING);
		break;
	case WPA_PTK_PTKINITNEGOTIATING:
		if (sm->EAPOLKeyReceived && !sm->EAPOLKeyRequest &&
		    sm->EAPOLKeyPairwise && sm->MICVerified)
			SM_ENTER(WPA_PTK, PTKINITDONE);
		else if (sm->TimeoutCtr >
			 (int) dot11RSNAConfigPairwiseUpdateCount) {
			wpa_auth->dot11RSNA4WayHandshakeFailures++;
			wpa_auth_vlogger(sm->wpa_auth, sm->addr, LOGGER_DEBUG,
					 "PTKINITNEGOTIATING: Retry limit %d "
					 "reached",
					 dot11RSNAConfigPairwiseUpdateCount);
			SM_ENTER(WPA_PTK, DISCONNECT);
		} else if (sm->TimeoutEvt)
			SM_ENTER(WPA_PTK, PTKINITNEGOTIATING);
		break;
	case WPA_PTK_PTKINITDONE:
		break;
	}
}

static int wpa_gtk_update(struct wpa_authenticator *wpa_auth,
			  struct wpa_group *group)
{
	int ret = 0;

	OS_MemCPY(group->GNonce, group->Counter, WPA_NONCE_LEN);
	inc_byte_array(group->Counter, WPA_NONCE_LEN);
	if (wpa_gmk_to_gtk(group->GMK, "Group key expansion",
			    gDeviceInfo->APInfo->own_addr, group->GNonce,
			   group->GTK[group->GN - 1], group->GTK_len) < 0)
		ret = -1;
	wpa_hexdump_key(MSG_INFO, "GTK",
			group->GTK[group->GN - 1], group->GTK_len);


	return ret;
}



static int wpa_group_config_group_keys(struct wpa_authenticator *wpa_auth,
				       struct wpa_group *group)
{
	int ret = 0;
    wpa_hexdump_key(MSG_INFO,"CONFIG GTK:",group->GTK[group->GN - 1],group->GTK_len);
	wpa_auth_set_key(wpa_auth, 
			     wpa_alg_enum(wpa_auth->conf->wpa_group),
			     broadcast_ether_addr, group->GN,
			     group->GTK[group->GN - 1], group->GTK_len);


	return ret;
}


static int wpa_sm_step(struct wpa_state_machine *sm)
{
	if( sm->started == 0)
		return 0;

	if (sm->in_step_loop) {
		/* This should not happen, but if it does, make sure we do not
		 * end up freeing the state machine too early by exiting the
		 * recursive call. */
		LOG_DEBUGF(LOG_L2_AP,( "WPA: wpa_sm_step() called recursively"));
		return 0;
	}

	sm->in_step_loop = 1;
	do {
		if (sm->started == 0)
			break;
		sm->changed = FALSE;
		SM_STEP_RUN(WPA_PTK);
	} while (sm->changed );
	sm->in_step_loop = 0;

	if (sm->pending_deinit) {
		wpa_printf(MSG_DEBUG, "WPA: Completing pending STA state "
			   "machine deinit for " MACSTR, MAC2STR(sm->addr));
   
		//wpa_free_sta_sm(sm);
		
		return 1;
	}
	return 0;
}


const ssv_type_u8 * wpa_auth_get_wpa_ie(struct wpa_authenticator *wpa_auth, ssv_type_size_t *len)
{
	if (wpa_auth == NULL)
		return NULL;
	//*len = wpa_auth->wpa_ie_len;
	*len =  WPA_IE_LEN;
	return wpa_auth->wpa_ie;
}


#endif
#endif
