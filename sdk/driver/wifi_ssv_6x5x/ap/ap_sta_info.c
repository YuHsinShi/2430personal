/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#define __SFILE__ "ap_sta_info.c"

#include <ssv_ether.h>
#include <host_apis.h>
#include <log.h>
#include <txrx_hdl.h>
#include <ssv_hal.h>
#include "ap_sta_info.h"
#include "ap_info.h"
#include "ap_def.h"
#include "beacon.h"
#include "common/bitops.h"
#include "common/ap_common.h"
#include "ap_mlme.h"
#include "ap_tx.h"
#include "common/ieee802_11_defs.h"
#include <ieee80211.h>
#include "ap_drv_cmd.h"
#include <ssv_timer.h>
#include <ssv_devinfo.h>
#include "ieee802_11_ht.h"

#if (AP_MODE_ENABLE == 1)
#define IS_HOST_API_AVAILABLE() (HOST_API_ACTIVE==ssv6xxx_wifi_get_host_api_state())

extern struct task_info_st g_host_task_info[];
extern struct Host_cfg g_host_cfg;

void APStaInfo_free(ApInfo_st *pApInfo, APStaInfo_st *sta);
#ifdef __AP_DEBUG__
	void APStaInfo_PrintStaInfo(ssv_type_u32 idx);
#endif//__AP_DEBUG__



#define AP_STA_CONNECTED "AP-STA-CONNECTED "
#define AP_STA_DISCONNECTED "AP-STA-DISCONNECTED "

//--------------------------------------------------------------
//struct list_q;
struct ap_tx_desp;
extern void ap_release_tx_desp_and_frame(struct ap_tx_desp *tx_desp);
extern struct ap_tx_desp *ap_tx_desp_peek(const struct ssv_list_q *list_);

extern ssv_type_u8* ssv6xxx_host_tx_req_get_qos_ptr(struct cfg_host_txreq0 *req0);
extern ssv_type_u8* ssv6xxx_host_tx_req_get_data_ptr(struct cfg_host_txreq0 *req0);
extern void purge_sta_ps_buffers(APStaInfo_st *pAPStaInfo);
//extern void purge_all_bc_buffers();
extern void wpa_free_sta_sm(struct wpa_state_machine *sm);
extern ssv_type_bool ap_sta_info_collect(struct cfg_set_sta *cfg_sta,enum cfg_sta_oper sta_oper, APStaInfo_st *sta, enum cfg_ht_type ht, ssv_type_bool qos);
extern void exec_host_evt_cb(ssv_type_u32 nEvtId, void *data, ssv_type_s32 len);
extern void eapol_auth_free(struct wpa_authenticator *wpa_auth,struct wpa_state_machine *sm);

static ssv_inline ssv_type_bool __bss_tim_get(ssv_type_u16 aid)
{
	/*
	 * This format has been mandated by the IEEE specifications,
	 * so this line may not be changed to use the __set_bit() format.
	 */
	return !!(gDeviceInfo->APInfo->tim[aid / 8] & (1 << (aid % 8)));
}




static ssv_inline void __bss_tim_set(ssv_type_u16 aid)
{
	/*
	 * This format has been mandated by the IEEE specifications,
	 * so this line may not be changed to use the __set_bit() format.
	 */
	gDeviceInfo->APInfo->tim[aid / 8] |= (1 << (aid % 8));
}

static ssv_inline void __bss_tim_clear(ssv_type_u16 aid)
{
	/*
	 * This format has been mandated by the IEEE specifications,
	 * so this line may not be changed to use the __clear_bit() format.
	 */
	gDeviceInfo->APInfo->tim[aid / 8] &= ~(1 << (aid % 8));
}

#if 0
static unsigned long ieee80211_tids_for_ac(int ac)
{
	/* If we ever support TIDs > 7, this obviously needs to be adjusted */
	switch (ac) {
	case IEEE80211_AC_VO:
		return BIT(6) | BIT(7);
	case IEEE80211_AC_VI:
		return BIT(4) | BIT(5);
	case IEEE80211_AC_BE:
		return BIT(0) | BIT(3);
	case IEEE80211_AC_BK:
		return BIT(1) | BIT(2);
	default:
		//WARN_ON(1);
		return 0;
	}
}
#endif

//extern u32 ap_tx_desp_queue_len(const struct list_q *list_);
ssv_type_bool sta_info_recalc_tim(APStaInfo_st *sta)
{
	ssv_type_bool indicate_tim = false;

	//normal case. If there is at least one non-delivery AC, we just need to check those.

	ssv_type_u8 ignore_for_tim = sta->uapsd_queues;
	int ac;

	/*
	 * WMM spec 3.6.1.4
	 * If all ACs are delivery-enabled then we should build
	 * the TIM bit for all ACs anyway; if only some are then
	 * we ignore those and build the TIM bit using only the
	 * non-enabled ones.
	 */
    OS_MutexLock(sta->apsta_mutex);
    //LOG_PRINTF("sta=0x%x,caltim\r\n",(u32)sta);
	if (ignore_for_tim == BIT(IEEE80211_NUM_ACS) - 1)
		ignore_for_tim = 0;



	for (ac = 0; ac < IEEE80211_NUM_ACS; ac++) {
//		unsigned long tids;

		if (ignore_for_tim & BIT(ac))
			continue;

	    indicate_tim |= (list_q_len_safe(&sta->ps_tx_buf[ac], &sta->ps_tx_buf_mutex)!=0);

		if(indicate_tim)
			break;

		//feature for driver could buffer frame
		//tids = ieee80211_tids_for_ac(ac);
		//
		//indicate_tim |=
		//	sta->driver_buffered_tids & tids;
	}
	// check FW queue status
    indicate_tim |= (sta->fw_q_len>0);

	//Reduce to set beacon
	if(indicate_tim ==__bss_tim_get(sta->aid))
    {   
        //LOG_PRINTF("Reduce(%d,%d)\r\n",indicate_tim,__bss_tim_get(sta->aid));
        OS_MutexUnLock(sta->apsta_mutex);
		return FALSE;
    }

	if (indicate_tim)
    {   
		__bss_tim_set(sta->aid);
        //LOG_PRINTF("tim=0x%x wsid=%d,aid=%d\r\n",__bss_tim_get(sta->aid),sta->wsid_idx,sta->aid);
    }
	else
		__bss_tim_clear(sta->aid);

    OS_MutexUnLock(sta->apsta_mutex);
    return TRUE;

	//GenBeacon();

}


void recalc_tim_gen_beacon(APStaInfo_st * sta)
{
    if(!g_host_cfg.AP_TimAllOne)
    {

//#if(AP_MODE_BEACON_VIRT_BMAP_0XFF == 0)
        if(sta_info_recalc_tim(sta))
        {
        #if(MLME_TASK ==1)
            //Send to MLME task GenBeacon
            ap_mlme_handler(NULL,MEVT_BCN_CMD);
        #else
            GenBeacon();
        #endif
        }
//#endif 
    }
}








ssv_type_bool sta_info_buffer_expired(APStaInfo_st *sta, struct ap_tx_desp *tx_desp)
{
	int timeout;

	//Null data. no data expired
	if (!tx_desp)
		return false;

	/* Timeout: (2 * listen_interval * beacon_int * 1024 / 1000000) sec */
	timeout = (sta->listen_interval *
		   g_host_cfg.bcn_interval *
		   32 / 15625) * 1000;
	if (timeout < AP_STA_TX_BUFFER_EXPIRE)
		timeout = AP_STA_TX_BUFFER_EXPIRE;
	return time_after((unsigned long)OS_GetSysTick(), (unsigned long)tx_desp->jiffies + (unsigned long)timeout);
}


static ssv_type_bool sta_info_cleanup_expire_buffered_ac(APStaInfo_st *sta, int ac)
{
	struct ap_tx_desp *tx_desp;

	/*
	 * Now also check the normal PS-buffered queue, this will
	 * only find something if the filtered queue was emptied
	 * since the filtered frames are all before the normal PS
	 * buffered frames.
	 */
	for (;;) {
        OS_MutexLock(sta->ps_tx_buf_mutex);
		tx_desp = ap_tx_desp_peek(&sta->ps_tx_buf[ac]);
		if (sta_info_buffer_expired(sta, tx_desp))
			tx_desp =(struct ap_tx_desp *) list_q_deq(&sta->ps_tx_buf[ac]);
		else
			tx_desp = NULL;
        OS_MutexUnLock(sta->ps_tx_buf_mutex);

		/*
		 * frames are queued in order, so if this one
		 * hasn't expired yet (or we reached the end of
		 * the queue) we can stop testing
		 */
		if (!tx_desp)
			break;
        OS_MutexLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
		gDeviceInfo->APInfo->total_ps_buffered--;
        OS_MutexUnLock(gDeviceInfo->APInfo->ap_info_ps_mutex);

		ap_release_tx_desp_and_frame(tx_desp);
	}

	/*
	 * Finally, recalculate the TIM bit for this station -- it might
	 * now be clear because the station was too slow to retrieve its
	 * frames.
	 */
        // indicate tim and generate beacon
        recalc_tim_gen_beacon(sta);


	/*
	 * Return whether there are any frames still buffered, this is
	 * used to check whether the cleanup timer still needs to run,
	 * if there are no frames we don't need to rearm the timer.
	 */
	return (sta->ps_tx_buf[ac].qlen > 0);
}

static ssv_type_bool sta_info_cleanup_expire_buffered(APStaInfo_st *sta)
{
	ssv_type_bool have_buffered = false;
	int ac;

	for (ac = 0; ac < IEEE80211_NUM_ACS; ac++)
		have_buffered |=
			sta_info_cleanup_expire_buffered_ac(sta, ac);

	return have_buffered;
}





void sta_info_cleanup(void *data1, void *data2)
{
	ssv_type_u32 i;
	//APStaInfo_st *sta;
	ssv_type_bool timer_needed = false;

	for(i=0;i<gMaxAID;i++ )
	{

		APStaInfo_st *sta = &gDeviceInfo->APInfo->StaConInfo[i];
		if(!test_sta_flag(sta, WLAN_STA_VALID))
			continue;

		if(sta_info_cleanup_expire_buffered(sta))
			timer_needed = true;
	}
    OS_MutexLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
    gDeviceInfo->APInfo->sta_pkt_cln_timer = timer_needed;
    OS_MutexUnLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
	if (timer_needed)
    {
		os_create_timer(AP_STA_INFO_CLEANUP_INTERVAL, sta_info_cleanup, NULL, NULL, (void*)TIMEOUT_TASK);
    }
}



static ssv_inline void APStaInfo_StaReset(APStaInfo_st *sta)
{

	int i;
    OsMutex ps_tx_buf_mutex = sta->ps_tx_buf_mutex;
    OsMutex apsta_mutex = sta->apsta_mutex;
	OS_MemSET(sta, 0, sizeof(APStaInfo_st));

    //OS_MutexInit(&sta->ps_tx_buf_mutex);
    sta->ps_tx_buf_mutex = ps_tx_buf_mutex;
    sta->apsta_mutex = apsta_mutex;
	for (i = 0; i < IEEE80211_NUM_ACS; i++) {
			//ap_tx_desp_head_init(&sta->ps_tx_buf[i]);
            list_q_init(&sta->ps_tx_buf[i]);
		}


}


void APStaInfo_Init()
{
	ssv_type_s32 tsize;
	ssv_type_u32 j;

	tsize = sizeof(APStaInfo_st) * gMaxAID;
	gDeviceInfo->APInfo->StaConInfo = (struct APStaInfo *)OS_MemAlloc((ssv_type_u32)tsize);
    OS_MemSET((void *)gDeviceInfo->APInfo->StaConInfo,0,tsize);
	SSV_ASSERT(gDeviceInfo->APInfo->StaConInfo != NULL);


	//StaInfo = gStaInfo;

	//----------------------
	//init station ac list
	for (j=0;j<gMaxAID;j++)
    {
		APStaInfo_StaReset(&gDeviceInfo->APInfo->StaConInfo[j]);
        gDeviceInfo->APInfo->StaConInfo[j].wsid_idx = gMaxAID;
        
#ifndef CONFIG_NO_WPA2         
        gDeviceInfo->APInfo->StaConInfo[j].addStaFlag = 0;
        gDeviceInfo->APInfo->StaConInfo[j].assocRsp = 0;
        gDeviceInfo->APInfo->StaConInfo[j].reassoc = 0;
#endif        
        OS_MutexInit(&gDeviceInfo->APInfo->StaConInfo[j].ps_tx_buf_mutex,"ps_tx_buf_mutex");
        OS_MutexInit(&gDeviceInfo->APInfo->StaConInfo[j].apsta_mutex,"apsta_mutex");
    }

	//---------------------------

}









//**************************************************************************************************************************
//**************************************************************************************************************************
//**************************************************************************************************************************

void ap_sta_set_authorized(ApInfo_st *pApInfo, APStaInfo_st *sta,
	int authorized)
{
//	const u8 *dev_addr = NULL;
    struct cfg_set_sta cfg_sta;
    enum cfg_sta_oper sta_oper = CFG_STA_DEL;
    OS_MemSET(&cfg_sta, 0, sizeof(struct cfg_set_sta));

	if (!!authorized == !!(sta->_flags & WLAN_STA_AUTHORIZED))
		return;

#ifdef CONFIG_P2P
	dev_addr = p2p_group_get_dev_addr(hapd->p2p_group, sta->addr);
#endif /* CONFIG_P2P */

	if (authorized) {
        LOG_INFO(AP_STA_CONNECTED MACSTR "\r\n", MAC2STR(sta->addr));
        pApInfo->num_sta_authorized++;
        OS_MutexLock(sta->apsta_mutex);
        sta->_flags |= WLAN_STA_AUTHORIZED;
        OS_MutexUnLock(sta->apsta_mutex);
	} else {
        LOG_INFO( AP_STA_DISCONNECTED MACSTR "\r\n", MAC2STR(sta->addr));
        pApInfo->num_sta_authorized--;
        OS_MutexLock(sta->apsta_mutex);
        sta->_flags &= ~WLAN_STA_AUTHORIZED;
        OS_MutexUnLock(sta->apsta_mutex);
	}
    if(authorized == 1)
        sta_oper = CFG_STA_ADD;

    cfg_sta.aid = sta->aid;
    cfg_sta.vif_idx = ((ssv_vif *)pApInfo->vif)->idx;
    if(ap_sta_info_collect(&cfg_sta,sta_oper,sta,CFG_HT_NONE,test_sta_flag(sta, WLAN_STA_WMM)))
    {
        //Send event to host            
        exec_host_evt_cb(SOC_EVT_STA_STATUS,&cfg_sta, sizeof(struct cfg_set_sta));
    }


}

void change_sta_disassoc_to_deauth(ApInfo_st *pApInfo,APStaInfo_st *sta,ssv_type_bool timer_creat)
{
    ap_sta_set_authorized(pApInfo, sta, 0);
    OS_MutexLock(sta->apsta_mutex);
    clear_sta_flag(sta, WLAN_STA_ASSOC);
    OS_MutexUnLock(sta->apsta_mutex);
    //APStaInfo_DrvRemove(pApInfo, sta); //Don't remove it here. Will remove from outside

#ifndef CONFIG_NO_WPA2	
    eapol_auth_free(&(pApInfo->wpa_auth),&(sta->wpa_sm));
#endif	
    if(timer_creat)
    {
        if (sta->timeout_next == STA_NULLFUNC ||
    	    sta->timeout_next == STA_DISASSOC) {
    		sta->timeout_next = STA_DEAUTH;            
    		os_cancel_timer(ap_handle_timer,  (ssv_type_u32)pApInfo, (ssv_type_u32)sta);
            os_create_timer(AP_DEAUTH_DELAY, ap_handle_timer,pApInfo, sta,(void*)TIMEOUT_TASK);
        }    
    }

}


/**
 * ap_handle_timer - Per STA timer handler
 * @eloop_ctx: struct hostapd_data *
 * @timeout_ctx: struct sta_info *
 *
 * This function is called to check station activity and to remove inactive
 * stations.
 */
void ap_handle_timer(void *data1, void *data2)
{
	ApInfo_st *pApInfo = (ApInfo_st *)data1;
	APStaInfo_st *sta = (APStaInfo_st *)data2;
	unsigned long next_time = 0;

#ifdef __TEST__
	LOG_TRACE( "ap_handle_timer sta->timeout_next %d\r\n", sta->timeout_next);
#endif

	if (!test_sta_flag(sta, WLAN_STA_VALID)) {
		LOG_INFO( "%s, station not VALID \r\n",__func__);
		return;
	}

	if (sta->timeout_next == STA_REMOVE) {
		LOG_INFO( "%s,deauthenticated due to local deauth request \r\n",__func__);
		APStaInfo_free(pApInfo, sta);
		return;
	}

    if(!IS_HOST_API_AVAILABLE())
    {
        //LOG_PRINTF("host api is not available now\r\n");
        os_create_timer(1000, ap_handle_timer, pApInfo, sta, (void*)TIMEOUT_TASK);
        return;
    }

//------------------------------------------------------------------------------

	if (test_sta_flag(sta, WLAN_STA_ASSOC) &&
	    (sta->timeout_next == STA_NULLFUNC ||
	     sta->timeout_next == STA_DISASSOC)) {
		int inactive_sec;
		inactive_sec = OS_TICK2MS(OS_GetSysTick()-sta->last_rx) /* MSEC_PER_SECI*/;

		if (inactive_sec < AP_MAX_INACTIVITY &&
			   test_sta_flag(sta, WLAN_STA_ASSOC)) {
			/* station activity detected; reset timeout state */
        #ifdef __AP_DEBUG__
			LOG_DEBUG(
				"Station " MACSTR " has been active %i ms ago\r\n",
				MAC2STR(sta->addr), inactive_sec);
        #endif
			sta->timeout_next = STA_NULLFUNC;
			next_time = AP_MAX_INACTIVITY - (unsigned long)inactive_sec;
		}
 	}
//------------------------------------------------------------------------------

	if ( test_sta_flag(sta, WLAN_STA_ASSOC) &&
	    sta->timeout_next == STA_DISASSOC &&
	    !test_sta_flag(sta, WLAN_STA_PENDING_POLL)) {
		//LOG_DEBUG( "Station " MACSTR
		//	" has ACKed data poll\r\n", MAC2STR(sta->addr));
		/* data nullfunc frame poll did not produce TX errors; assume
		 * station ACKed it */
		sta->timeout_next = STA_NULLFUNC;
		next_time = AP_MAX_INACTIVITY;
	}

	if (next_time) {
        os_create_timer(next_time, ap_handle_timer, pApInfo, sta, (void*)TIMEOUT_TASK);
		return;																					//---------------------------------------->Return
	}

//------------------------------------------------------------------------------
//Send Frame-Poll, De-authentication, Dis-association.

	if (sta->timeout_next == STA_NULLFUNC &&
	     test_sta_flag(sta, WLAN_STA_ASSOC)) {
		//LOG_DEBUG(  "Polling STA,%s\r\n",__func__);
        OS_MutexLock(sta->apsta_mutex);
		set_sta_flag(sta, WLAN_STA_PENDING_POLL);
        //Send NULL data to poll
 		//nl80211_poll_client(pApInfo->own_addr, sta->addr,										//----------------------------------->implement

        //poll station status arp retry count
        if (sta->arp_retry_count <= STA_TIMEOUT_RETRY_COUNT )
        {
            sta->arp_retry_count ++;
        
            OS_MutexUnLock(sta->apsta_mutex);
            //send host event to net_mgr , for arp request.
            exec_host_evt_cb(SOC_EVT_POLL_STATION,sta->addr,ETH_ALEN);
            os_create_timer(STA_TIMEOUT_RETRY_TIMER, ap_handle_timer, pApInfo, sta, (void*)TIMEOUT_TASK);
            return;
        }
        OS_MutexUnLock(sta->apsta_mutex);
        os_create_timer(STA_TIMEOUT_RETRY_TIMER, ap_handle_timer, pApInfo, sta, (void*)TIMEOUT_TASK);
        return;

	} else if (sta->timeout_next != STA_REMOVE) {
		int deauth = (sta->timeout_next == STA_DEAUTH);

		LOG_DEBUG(  "Sending %s info to STA " MACSTR"\r\n",
			   deauth ? "deauthentication" : "disassociation",
			   MAC2STR(sta->addr));

		if (deauth) {
//Send De-authentication frame																									//----------------------------------->implement
			i802_sta_deauth(pApInfo->own_addr, sta->addr, WLAN_REASON_PREV_AUTH_NOT_VALID);
		} else {
//Send Dis-association frame
			i802_sta_disassoc(pApInfo->own_addr, sta->addr,	WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY);						//----------------------------------->implement
		}
	}


//------------------------------------------------------------------------------



	switch (sta->timeout_next)
	{
		case STA_NULLFUNC:
			sta->timeout_next = STA_DISASSOC;
            os_create_timer(AP_DISASSOC_DELAY, ap_handle_timer,
    					   pApInfo, sta, (void*)TIMEOUT_TASK);
			break;
		case STA_DISASSOC:
//Todo:security
            change_sta_disassoc_to_deauth(pApInfo,sta,FALSE);
			sta->timeout_next = STA_DEAUTH;
            os_create_timer(AP_DEAUTH_DELAY, ap_handle_timer, pApInfo, sta, (void*)TIMEOUT_TASK);
 			APStaInfo_free(pApInfo, sta);

			break;
		case STA_DEAUTH:
		case STA_REMOVE:
			LOG_INFO("STA " MACSTR " deauthenticated due to inactivity \r\n", MAC2STR(sta->addr));
 			APStaInfo_free(pApInfo, sta);

            break;

		default:
			break;
 	}
}

APStaInfo_st *APStaInfo_FindStaByAddr( ETHER_ADDR *mac )
{

    ssv_type_u32 idx;

    for(idx=0; idx<gMaxAID; idx++)
    {

		 APStaInfo_st *StaInfo= &gDeviceInfo->APInfo->StaConInfo[idx];
        if (!test_sta_flag(StaInfo, WLAN_STA_VALID))
            continue;

        if (IS_EQUAL_MACADDR((char*)mac, (char*)StaInfo->addr))
            return StaInfo;
    }
    return NULL;
}

APStaInfo_st *APStaInfo_FindStaByWsid( ssv_type_u8 target_wsid )
{

    ssv_type_u32 idx;

    for(idx=0; idx<gMaxAID; idx++)
    {

		 APStaInfo_st *StaInfo= &gDeviceInfo->APInfo->StaConInfo[idx];
        if (!test_sta_flag(StaInfo, WLAN_STA_VALID))
            continue;

        if (StaInfo->wsid_idx == target_wsid)
            return StaInfo;
    }
    return NULL;
}




ssv_type_bool ap_sta_info_capability(ETHER_ADDR *mac , ssv_type_bool *ht, ssv_type_bool *qos, ssv_type_bool *wds)
{

	APStaInfo_st *sta = APStaInfo_FindStaByAddr(mac);

	//mc/bc frame use normal frame case
	if(ssv_is_multicast_ether_addr((const ssv_type_u8*)mac))
	{
		*ht  = *qos = *wds =0;
		return TRUE;
	}


	if(!sta)
	{
		LOG_TRACE( "Could not found STA ::"MACSTR"\r\n", MAC2STR((ssv_type_u8*)mac));
		return FALSE;
	}



	*ht = FALSE;
	*qos = test_sta_flag(sta, WLAN_STA_WMM);
	*wds = test_sta_flag(sta, WLAN_STA_WDS);



	return TRUE;
}











APStaInfo_st *APStaInfo_GetNewSta(void)
{

    ssv_type_u32 idx;

    for(idx=0; idx<gMaxAID; idx++)
    {

		 APStaInfo_st *StaInfo= &gDeviceInfo->APInfo->StaConInfo[idx];
        if (!test_sta_flag(StaInfo, WLAN_STA_VALID))
        {
            OS_MutexLock(StaInfo->apsta_mutex);
            if(MAX_VIF_NUM > 1)
            {
                StaInfo->wsid_idx = ssv_hal_get_ava_wsid(1);
                if(StaInfo->wsid_idx==0xFFFFFFFF){
                    LOG_PRINTF("AP Get ava wsid fail = %d\r\n",StaInfo->wsid_idx);
                    return NULL; //No ava wsid
                }
            }
            else
            {
                StaInfo->wsid_idx = idx;
            }

            set_sta_flag(StaInfo, WLAN_STA_VALID);
            OS_MutexUnLock(StaInfo->apsta_mutex);
			return StaInfo;
		}
    }
    return NULL;
}


void APStaInfo_DrvRemove(ApInfo_st *pApInfo, APStaInfo_st *sta)
{
    OS_MutexLock(sta->apsta_mutex);

	if ( test_sta_flag(sta, WLAN_STA_PS_STA) )
	{
        clear_sta_flag(sta, WLAN_STA_PS_STA);
        OS_MutexUnLock(sta->apsta_mutex);
        OS_MutexLock(pApInfo->ap_info_ps_mutex);
		pApInfo->num_sta_ps--;
		OS_MutexUnLock(pApInfo->ap_info_ps_mutex);
		purge_sta_ps_buffers(sta);
        //reset FW queue
        OS_MutexLock(sta->apsta_mutex);
        sta->fw_q_len = UNKNOWN_FW_QUEUE_LENGTH;
        OS_MutexUnLock(sta->apsta_mutex);
        
        // indicate tim and generate beacon
        recalc_tim_gen_beacon(sta);

	}
    else
    {
        OS_MutexUnLock(sta->apsta_mutex);
    }
    
	ap_soc_cmd_sta_oper(CFG_STA_DEL, sta, CFG_HT_NONE, test_sta_flag(sta, WLAN_STA_WMM));
	//reset wsid
    sta->wsid_idx = gMaxAID;
#ifndef CONFIG_NO_WPA2  
    sta->addStaFlag = 0;
    sta->reassoc = 0;
    sta->assocRsp = 0;
#endif    

}


extern void timer_sta_reorder_release(void* data1, void* data2);
void APStaInfo_free(ApInfo_st *pApInfo, APStaInfo_st *sta)
{
	int set_beacon = 0;
    ssv_type_u32 cnt=0;
    ssv_type_u32 idx;
	if(!test_sta_flag(sta, WLAN_STA_VALID))
		return;


	LOG_TRACE("%s\r\n",__func__);
    clear_sta_flag(sta,WLAN_STA_VALID);
    if(g_host_cfg.ampdu_rx_enable)
    {
        //release ampdu rx session
        ieee80211_delete_ampdu_rx(sta->wsid_idx);           
        //os_cancel_timer(timer_sta_reorder_release,(u32)NULL,(u32)NULL);
    }

	//Todo: SEC
	//accounting_sta_stop(hapd, sta);

	/* just in case */
	ap_sta_set_authorized(pApInfo, sta, 0);

	//if (sta->flags & WLAN_STA_WDS)
	//	hostapd_set_wds_sta(hapd, sta->addr, sta->aid, 0);

	//if (!(sta->flags & WLAN_STA_PREAUTH))
	//	hostapd_drv_sta_remove(hapd, sta->addr);

    idx = sta->wsid_idx;
 	APStaInfo_DrvRemove(pApInfo, sta);
    while(!ssv_hal_check_wsid_free(idx))
    {
        OS_MsDelay(20);
        cnt++;
        if(cnt>200)
            break;
    }
    if(cnt>200)
        LOG_PRINTF("wsid %d free fail\r\n",idx);
    else
        LOG_PRINTF("wsid %d free OK\r\n",idx);        


	//Remove AID
	if (sta->aid > 0)
		pApInfo->sta_aid[(sta->aid - 1) / 32] &=
			~ BIT((sta->aid - 1) % 32);

    if(pApInfo->num_sta>0&&pApInfo->num_sta <= gMaxAID)
    	pApInfo->num_sta--;
    else
        LOG_PRINTF("station number error:%d\n",pApInfo->num_sta);

	//all station are remove.....no need to clean buffer frames
	if(!pApInfo->num_sta)
	{
		os_cancel_timer(sta_info_cleanup, (ssv_type_u32)0, (ssv_type_u32)0);
        OS_MutexLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
        gDeviceInfo->APInfo->sta_pkt_cln_timer = FALSE;
        OS_MutexUnLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
		//purge_all_bc_buffers();
	}

	if (sta->nonerp_set) {
		sta->nonerp_set = 0;
		pApInfo->num_sta_non_erp--;
		if (pApInfo->num_sta_non_erp == 0)
        {
            if(!pApInfo->olbc)
            {
                //Send ERP protect cmd
                ssv_type_bool erp_protect = FALSE;
                _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_ERP_PROTECT, &erp_protect, sizeof(ssv_type_bool),TRUE,FALSE);      
            }
			set_beacon++;
	    }
	}

	if (sta->no_short_slot_time_set) {
		sta->no_short_slot_time_set = 0;
		pApInfo->num_sta_no_short_slot_time--;
		if (pApInfo->eCurrentApMode == AP_MODE_IEEE80211G
		    && pApInfo->num_sta_no_short_slot_time == 0)
        {      
			//Set slot time to register
           ssv_hal_set_short_slot_time(TRUE);
			set_beacon++;
         }
	}

	if (sta->no_short_preamble_set) {
		sta->no_short_preamble_set = 0;
		pApInfo->num_sta_no_short_preamble--;
		if (pApInfo->eCurrentApMode == AP_MODE_IEEE80211G
		    && pApInfo->num_sta_no_short_preamble == 0)
			set_beacon++;
	}

	if (sta->no_ht_gf_set) {
		sta->no_ht_gf_set = 0;
		pApInfo->num_sta_ht_no_gf--;
	}

	if (sta->no_ht_set) {
		sta->no_ht_set = 0;
		pApInfo->num_sta_no_ht--;
	}

	if (sta->ht_20mhz_set) {
		sta->ht_20mhz_set = 0;
		pApInfo->num_sta_ht_20mhz--;
	}
	
//#ifdef CONFIG_P2P
//	if (sta->no_p2p_set) {
//		sta->no_p2p_set = 0;
//		hapd->num_sta_no_p2p--;
//		if (hapd->num_sta_no_p2p == 0)
//			hostapd_p2p_non_p2p_sta_disconnected(hapd);
//	}
//#endif /* CONFIG_P2P */

//Todo: HT
#if AP_SUPPORT_80211N
	if (hostapd_ht_operation_update(pApInfo) > 0)
    {   
        ssv_type_u16 operation_mode = host_to_le16(pApInfo->ht_op_mode);
        _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_HT_PROTECT, &operation_mode, sizeof(ssv_type_bool),TRUE,FALSE);
		set_beacon++;
    }
#endif /* AP_SUPPORT_80211N */

	if (set_beacon)
		ieee802_11_set_beacon(pApInfo,TRUE);

	os_cancel_timer(ap_handle_timer,  (ssv_type_u32)pApInfo, (ssv_type_u32)sta);












//Todo:
//	os_cancel_timer(ap_handle_session_timer);





//Todo: SEC
//	os_free(sta->last_assoc_req);
//	os_free(sta->challenge);




//Todo: SEC
	//wpabuf_free(sta->wps_ie);
	//wpabuf_free(sta->p2p_ie);

//Todo: HT
	//os_free(sta->ht_capabilities);

	OS_MutexLock(sta->ps_tx_buf_mutex);
    
#ifndef CONFIG_NO_WPA2	
       wpa_auth_sta_deinit(&(sta->wpa_sm));
       wpa_free_sta_sm(&(sta->wpa_sm));
      //memset (sta->wpa_sm) by  APStaInfo_StaReset(sta);  :
#endif	
    
   

	APStaInfo_StaReset(sta);
	OS_MutexUnLock(sta->ps_tx_buf_mutex);

#ifdef __AP_DEBUG__
    APStaInfo_PrintStaInfo(0);
    APStaInfo_PrintStaInfo(1);  
#endif//__AP_DEBUG__

}





APStaInfo_st *APStaInfo_add(ApInfo_st *pApInfo, const ssv_type_u8 *addr)
{
	APStaInfo_st *sta = NULL;

	sta = APStaInfo_FindStaByAddr((ETHER_ADDR*)addr);
	if (sta)
		return sta;

    //Keep WSID0 always active. Rate control needed to use WSID0 to statistics.    
    sta = &gDeviceInfo->APInfo->StaConInfo[0];
    if(sta->arp_retry_count>STA_TIMEOUT_RETRY_COUNT)// means no response over 15 sec
    {
        //remove inactive station 
        change_sta_disassoc_to_deauth(pApInfo,sta,FALSE);
        sta->timeout_next = STA_DEAUTH;
        os_cancel_timer(ap_handle_timer,  (ssv_type_u32)pApInfo, (ssv_type_u32)sta);
        //Send De-authentication frame                                                                                                 
        i802_sta_deauth(pApInfo->own_addr, sta->addr, WLAN_REASON_PREV_AUTH_NOT_VALID);
        APStaInfo_free(pApInfo, sta);
        LOG_PRINTF("APStaInfo_free\r\n");
    }
    sta = NULL;
    
	if (pApInfo->num_sta >= gMaxAID) {
        ssv_type_u32 idx=0,del_idx=gMaxAID;
        ssv_type_u8 max_arp_retry_count=0;
        APStaInfo_st *StaInfo;
		/* FIX: might try to remove some old STAs first? */
		LOG_DEBUG( "no more room for new STAs (%d/%d), need to remove timeout station\r\n",
			   pApInfo->num_sta, gMaxAID);

        //find max timeout count
        for(idx=0; idx<gMaxAID; idx++)
        {
             StaInfo= &gDeviceInfo->APInfo->StaConInfo[idx];
             if(StaInfo->arp_retry_count>max_arp_retry_count 
                && StaInfo->arp_retry_count>STA_TIMEOUT_RETRY_COUNT) //means no response over 15 sec
             {
                del_idx = idx;
                max_arp_retry_count = StaInfo->arp_retry_count;
             }
            
        }
        if(del_idx<gMaxAID)
        {
            //LOG_PRINTF("del_idx:%d\r\n",del_idx);
            StaInfo= &gDeviceInfo->APInfo->StaConInfo[del_idx];
    
            //Todo: SEC
            change_sta_disassoc_to_deauth(pApInfo, StaInfo,FALSE);
            StaInfo->timeout_next = STA_DEAUTH;
            os_cancel_timer(ap_handle_timer,  (ssv_type_u32)pApInfo, (ssv_type_u32)StaInfo);
            //Send De-authentication frame                                                                                                  //----------------------------------->implement
            i802_sta_deauth(pApInfo->own_addr, StaInfo->addr, WLAN_REASON_PREV_AUTH_NOT_VALID);
            APStaInfo_free(pApInfo, StaInfo);
        }        
      
	}

	sta = APStaInfo_GetNewSta();
	if (sta == NULL) {
		LOG_ERROR( "%s can't get new sta\r\n",__func__);
		return NULL;
	}
    LOG_PRINTF("STA info Add wsid=%d\r\n",sta->wsid_idx);
	//sta->acct_interim_interval = hapd->conf->acct_interim_interval;

	/* initialize STA info data */
	os_create_timer(AP_MAX_INACTIVITY, ap_handle_timer, pApInfo, sta, (void*)TIMEOUT_TASK);


	OS_MemCPY(sta->addr, addr, ETH_ALEN);

	gDeviceInfo->APInfo->num_sta++;


	//Add new sta in soc after finishing association.


	return sta;
}



















//****************************************************************************************************************************************
//PS-poll, USPAD
//****************************************************************************************************************************************

void ap_send_buffered_frame(struct ap_tx_desp *tx_desp)
{
    ssv_type_u32 rsvd0=ssv_hal_get_txreq0_rsvd0((CFG_HOST_TXREQ *)tx_desp->host_txreq0);
    rsvd0|=AP_PS_FRAME;
    ssv_hal_set_txreq0_rsvd0((CFG_HOST_TXREQ *)tx_desp->host_txreq0,rsvd0);
    //tx_desp->host_txreq0->RSVD0 |= AP_PS_FRAME;
    if ( (TxHdl_FrameProc(tx_desp->frame, false, 0, 0, (ssv_vif*)gDeviceInfo->APInfo->vif)) == FALSE)
    {
        os_frame_free(tx_desp->frame);
    }
        
    ap_release_tx_desp(tx_desp);
}



//**************************************************************
//							RX
//**************************************************************

//Should send through normal path->host apis tx->hcmd queue->hcmd engine->drv.
static void ieee80211_send_null_response(APStaInfo_st *sta, int tid,
					 enum frame_release_type reason)
{

	struct ieee80211_qos_hdr *nullfunc;

	int size = sizeof(*nullfunc);
	__le16_ssv_type fc;
	ssv_type_bool qos = test_sta_flag(sta, WLAN_STA_WMM);


	if (qos) {
		fc = cpu_to_le16(IEEE80211_FTYPE_DATA |
				 IEEE80211_STYPE_QOS_NULLFUNC |
				 IEEE80211_FCTL_FROMDS);
	} else {
		size -= 2;
		fc = cpu_to_le16(IEEE80211_FTYPE_DATA |
				 IEEE80211_STYPE_NULLFUNC |
				 IEEE80211_FCTL_FROMDS);
	}

	nullfunc = (struct ieee80211_qos_hdr *)gDeviceInfo->APInfo->pMgmtPkt;


	nullfunc->frame_control = fc;
	nullfunc->duration_id = 0;
	OS_MemCPY((void*)nullfunc->addr1, (void*)sta->addr, ETH_ALEN);
	OS_MemCPY((void*)nullfunc->addr2, (void*)gDeviceInfo->APInfo->own_addr, ETH_ALEN);
	OS_MemCPY((void*)nullfunc->addr3, (void*)gDeviceInfo->APInfo->own_addr, ETH_ALEN);


	if (qos) {
		nullfunc->qos_ctrl = cpu_to_le16(tid);

		if (reason == FRAME_RELEASE_UAPSD)
			nullfunc->qos_ctrl |=
			cpu_to_le16(IEEE80211_QOS_CTL_EOSP);
	}


	/*
	 * Tell TX path to send this frame even though the
	 * STA may still remain is PS mode after this frame
	 * exchange. Also set EOSP to indicate this packet
	 * ends the poll/service period.
	 */

// 	info->flags |= IEEE80211_TX_CTL_POLL_RESPONSE |
// 		       IEEE80211_TX_STATUS_EOSP |
// 		       IEEE80211_TX_CTL_REQ_TX_STATUS;





	ap_soc_data_send(gDeviceInfo->APInfo->pOSMgmtframe, size, TRUE, AP_TX_POLL_RESPONSE);


}




static void
ieee80211_sta_ps_deliver_response(APStaInfo_st *sta,			//PS poll UAPSD
				  int n_frames, ssv_type_u8 ignored_acs,
				  enum frame_release_type reason)
{
	ssv_type_bool found = false;
	ssv_type_bool more_data = false;
	int ac;
	unsigned long driver_release_tids = 0;
	struct ssv_list_q frames;


	do{


		/* Service or PS-Poll period starts */
		if(FRAME_RELEASE_UAPSD == reason)
        {
            OS_MutexLock(sta->apsta_mutex);
			set_sta_flag(sta, WLAN_STA_SP);		//clear bit in ieee80211_tx_status
			OS_MutexUnLock(sta->apsta_mutex);
        }
		//ap_tx_desp_head_init(&frames);
        list_q_init(&frames);

		/*
		 * Get response frame(s) and more data bit for it.
		 */
		for (ac = 0; ac < IEEE80211_NUM_ACS; ac++)
		{


			if (ignored_acs & BIT(ac))
				continue;

    		while (n_frames > 0) {										
				struct ap_tx_desp *tx_desp;

                tx_desp = (struct ap_tx_desp *)list_q_deq_safe(&sta->ps_tx_buf[ac],&sta->ps_tx_buf_mutex);


				if (tx_desp)
                {
                    OS_MutexLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
					gDeviceInfo->APInfo->total_ps_buffered--;
                    OS_MutexUnLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
                }
			
				if (!tx_desp)
					break;
				n_frames--;
				found = true;
                list_q_qtail_safe(&frames, (struct ssv_list_q *)tx_desp, &sta->ps_tx_buf_mutex);
			}
				
			if (!ap_tx_desp_queue_empty(&sta->ps_tx_buf[ac],&sta->ps_tx_buf_mutex)) {
				more_data = true;
				break;
			}
	 	}

		if (!found) {

            // No data frame in FW and host, it would send null data to FW             
            OS_MutexLock(sta->apsta_mutex);
            if (sta->fw_q_len < 0 && sta->fw_q_len != UNKNOWN_FW_QUEUE_LENGTH)
            {

    			int tid;
                OS_MutexUnLock(sta->apsta_mutex);

    			/*
        			 * For PS-Poll, this can only happen due to a race condition
        			 * when we set the TIM bit and the station notices it, but
        			 * before it can poll for the frame we expire it.
        			 *
        			 * For uAPSD, this is said in the standard (11.2.1.5 h):
        			 *	At each unscheduled SP for a non-AP STA, the AP shall
        			 *	attempt to transmit at least one MSDU or MMPDU, but no
        			 *	more than the value specified in the Max SP Length field
        			 *	in the QoS Capability element from delivery-enabled ACs,
        			 *	that are destined for the non-AP STA.
        			 *
        			 * Since we have no other MSDU/MMPDU, transmit a QoS null frame.
        			 */

    			/* This will evaluate to 1, 3, 5 or 7. */
    			tid = 7 - ((ffs(~ignored_acs) - 1) << 1);

    			ieee80211_send_null_response(sta, tid, reason);
            }
            else
            {
                OS_MutexUnLock(sta->apsta_mutex);
            }
            

            // indicate tim and generate beacon
            recalc_tim_gen_beacon(sta);            


			break;
		}

		if (!driver_release_tids) {

			struct ap_tx_desp *tx_desp;


			while ((tx_desp = (struct ap_tx_desp *)list_q_deq_safe(&frames,&sta->ps_tx_buf_mutex)) != NULL) {


				ssv_type_u8 *qoshdr = NULL;


				/*
				 * Tell TX path to send this frame even though the
				 * STA may still remain is PS mode after this frame
				 * exchange.
				 */

				/*
				 * Use MoreData flag to indicate whether there are
				 * more buffered frames for this STA
				 */

				if (more_data || !ap_tx_desp_queue_empty(&frames,&sta->ps_tx_buf_mutex))
					ssv_hal_set_txreq0_more_data((CFG_HOST_TXREQ *)tx_desp->host_txreq0,1);
				else
					ssv_hal_set_txreq0_more_data((CFG_HOST_TXREQ *)tx_desp->host_txreq0,0);


				//it's 80211 packet, need to add bit by hand
				if(ssv_hal_get_txreq0_f80211((CFG_HOST_TXREQ *)tx_desp->host_txreq0))
				{
					struct ieee80211_hdr *hdr = (void*) tx_desp->data;

					if (ssv_hal_get_txreq0_more_data((CFG_HOST_TXREQ *)tx_desp->host_txreq0))
						hdr->frame_control |=
							cpu_to_le16(IEEE80211_FCTL_MOREDATA);
					else
						hdr->frame_control &=
							cpu_to_le16(~IEEE80211_FCTL_MOREDATA);
				}



				//Set QoS Info only in data
				/* set EOSP for the frame */
				if (reason == FRAME_RELEASE_UAPSD &&
				     ap_tx_desp_queue_empty(&frames,&sta->ps_tx_buf_mutex))//Only set in Data frame ??? non data frame is last in queue
				{

					if(1==ssv_hal_get_txreq0_qos((CFG_HOST_TXREQ *)tx_desp->host_txreq0)){

						//802.11 frame
						if(1==ssv_hal_get_txreq0_f80211((CFG_HOST_TXREQ *)tx_desp->host_txreq0))
							qoshdr = ieee80211_get_qos_ctl((struct ieee80211_hdr_4addr *)tx_desp->data);
						else//802.3 frame
							qoshdr = ssv6xxx_host_tx_req_get_qos_ptr((CFG_HOST_TXREQ *)tx_desp->host_txreq0);

						*qoshdr |= IEEE80211_QOS_CTL_EOSP;

					}
					else{

						//LOG_WARN("buffered fame is not qos frame sub_type:%d \r\n", tx_desp->host_txreq0->sub_type);
					}

				}
				ap_send_buffered_frame(tx_desp);
                
                OS_MutexLock(sta->apsta_mutex);
                if(sta->fw_q_len>= 0 && FRAME_RELEASE_PSPOLL == reason)                       
                    sta->fw_q_len ++;
                OS_MutexUnLock(sta->apsta_mutex);

			}


            // indicate tim and generate beacon
            recalc_tim_gen_beacon(sta);

		}

	}while(0);



	//Actually it should be clear when EOSP frame has already sent.
	OS_MutexLock(sta->apsta_mutex);
	clear_sta_flag(sta, WLAN_STA_SP);
    OS_MutexUnLock(sta->apsta_mutex);



}


void sta_ps_deliver_poll_response(APStaInfo_st *sta)
{
	ssv_type_u8 ignore_for_response = sta->uapsd_queues;

	/*
	 * If all ACs are delivery-enabled then we should reply
	 * from any of them, if only some are enabled we reply
	 * only from the non-enabled ones.
	 */
	if (ignore_for_response == BIT(IEEE80211_NUM_ACS) - 1)
		ignore_for_response = 0;

	ieee80211_sta_ps_deliver_response(sta, 1, ignore_for_response,
					  FRAME_RELEASE_PSPOLL);
}

void sta_ps_deliver_uapsd(APStaInfo_st *sta)
{
	int n_frames = sta->max_sp;
	ssv_type_u8 delivery_enabled = sta->uapsd_queues;

	/*
	 * If we ever grow support for TSPEC this might happen if
	 * the TSPEC update from hostapd comes in between a trigger
	 * frame setting WLAN_STA_UAPSD in the RX path and this
	 * actually getting called.
	 */
	if (!delivery_enabled)
		return;

	switch (sta->max_sp) {
	case 1:
		n_frames = 2;
		break;
	case 2:
		n_frames = 4;
		break;
	case 3:
		n_frames = 6;
		break;
	case 0:
		/* XXX: what is a good value? */
		n_frames = 8;
		break;
	}

	ieee80211_sta_ps_deliver_response(sta, n_frames, ~delivery_enabled,
					  FRAME_RELEASE_UAPSD);
}





void sta_ps_deliver_wakeup(APStaInfo_st *sta)//Station wake up, so deliver so buffer and recal TIM
{
	//struct ieee80211_sub_if_data *sdata = sta->sdata;
	//struct ieee80211_local *local = sdata->local;
	//struct sk_buff_head pending;
	int buffered = 0, ac;
	struct ap_tx_desp *tx_desp;

	//BUILD_BUG_ON(BITS_TO_LONGS(STA_TID_NUM) > 1);
	//sta->driver_buffered_tids = 0;

//	if (!(local->hw.flags & IEEE80211_HW_AP_LINK_PS))
//		drv_sta_notify(local, sdata, STA_NOTIFY_AWAKE, &sta->sta);
//	skb_queue_head_init(&pending);

	/* Send all buffered frames to the station */
	for (ac = 0; ac < IEEE80211_NUM_ACS; ac++) {
		//int count = skb_queue_len(&pending), tmp;
			//while (tx_desp = ap_tx_desp_dequeue(&sta->ps_tx_buf[ac])) {
            while ((tx_desp = (struct ap_tx_desp *)list_q_deq_safe(&sta->ps_tx_buf[ac],&sta->ps_tx_buf_mutex)) != NULL) {
#if 1
				ap_send_buffered_frame(tx_desp);
#else
				ssv6xxx_drv_send(OS_FRAME_GET_DATA(tx_desp->frame), OS_FRAME_GET_DATA_LEN(tx_desp->frame));
		    	ap_release_tx_desp(tx_desp);
#endif

				buffered++;
			}
//		skb_queue_splice_tail_init(&sta->ps_tx_buf[ac], &pending);
//		tmp = skb_queue_len(&pending);
//		buffered += tmp - count;
	}

    OS_MutexLock(sta->apsta_mutex);
	clear_sta_flag(sta, WLAN_STA_SP);
	clear_sta_flag(sta, WLAN_STA_PS_STA);
    OS_MutexUnLock(sta->apsta_mutex);

	OS_MutexLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
	gDeviceInfo->APInfo->total_ps_buffered -= buffered;
    OS_MutexUnLock(gDeviceInfo->APInfo->ap_info_ps_mutex);

    //reset fw queue length
    OS_MutexLock(sta->apsta_mutex);
    sta->fw_q_len = UNKNOWN_FW_QUEUE_LENGTH;
    OS_MutexUnLock(sta->apsta_mutex);
    
    // indicate tim and generate beacon
    recalc_tim_gen_beacon(sta);


//#ifdef CONFIG_MAC80211_VERBOSE_PS_DEBUG
//	printk(KERN_DEBUG "%s: STA %pM aid %d sending %d filtered/%d PS frames "
//	       "since STA not sleeping anymore\n", sdata->name,
//	       sta->sta.addr, sta->sta.aid, filtered, buffered);
//#endif /* CONFIG_MAC80211_VERBOSE_PS_DEBUG */
}






void APStaInfo_Release(void)
{
	ssv_type_u32 idx=0;

	do
	{
		APStaInfo_free(gDeviceInfo->APInfo, &gDeviceInfo->APInfo->StaConInfo[idx]);
		idx++;
	}while(idx<gMaxAID);

}

struct ap_sta_info_tbl
{
	const char *string;
};

static const struct ap_sta_info_tbl ap_station_state[32]=
{
	{"VALID"},
	{"AUTH"},
	{"ASSOC"},
	{""},
	{""},					//.4

	{""},
	{"AUTHORIZED"},
	{"PENDING_POLL"},
	{"SHORT_PREAMBLE"},
	{""},					//9

	{"WMM"},
	{""},
	{"HT"},
	{""},
	{""},					//14

	{"WDS"},
	{"NONERP"},
	{"PS_STA"},
	{""},
	{""},					//19

	{""},
	{""},
	{"PSPOLL"},
	{""},
	{"SP"},					//24

	{"ASSOC_REQ_OK"},
	{""},
	{""},
	{""},
	{""},					//29


	{""},
	{""},					//31

};



extern void ap_print_queue_info(struct ssv_list_q *head);

extern H_APIs ssv_type_s32 ssv6xxx_get_rssi_by_mac(ssv_type_u8 *macaddr, ssv_type_u8 vif_idx);


void APStaInfo_PrintStaInfo(ssv_type_u32 idx)
{
    int i;
    int timeout;
        
    APStaInfo_st *StaInfo= &gDeviceInfo->APInfo->StaConInfo[idx];

    if(idx >= g_host_cfg.ap_supported_sta_num)
    {
        LOG_PRINTF("idx number is invalid \r\n");
        return ;
    }

    //LOG_TRACE("PS STA:%d BUF BC:%d Jiffies:%d\r\n", gDeviceInfo->APInfo->num_sta_ps, list_q_len_safe(&(gDeviceInfo->APInfo->ps_bc_buf), &gDeviceInfo->APInfo->ps_bc_buf_mutex), OS_GetSysTick());

    LOG_PRINTF("STA:%d ,wsid=%d\r\n",idx,StaInfo->wsid_idx);

    if (!test_sta_flag(StaInfo, WLAN_STA_VALID))
    {
        LOG_PRINTF("=>Invalid\r\n");
        return;
    }

    LOG_PRINTF("_flag:%08x :", StaInfo->_flags);

    for(i=0;i<32;i++)
    {
        if(!!( BIT(i)& StaInfo->_flags ) == TRUE)
        LOG_PRINTF(" %s", ap_station_state[i]);
    }
    LOG_PRINTF("\r\n");

    timeout = (StaInfo->listen_interval *
                g_host_cfg.bcn_interval *
                32 / 15625) * 1000;

    LOG_PRINTF("idle:%d buf timeout:%d\r\n", OS_TICK2MS(OS_GetSysTick()-StaInfo->last_rx), timeout);

    for(i=0;i<IEEE80211_NUM_ACS;i++)
    {
        LOG_PRINTF("BUF AC[%d]:%d \r\n", i, list_q_len_safe(&StaInfo->ps_tx_buf[i], &StaInfo->ps_tx_buf_mutex));
        //ap_print_queue_info(&StaInfo->ps_tx_buf[i]);
    }
    
}
void reset_ap_info(void* vif_ptr)
{
    ssv_type_u8* pOSMgmtframe;
    ssv_type_u8* pMgmtPkt;
    ssv_type_u8* tim;
    ssv_type_u8* pBeaconHead;
    ssv_type_u8 *pBeaconTail;
    ssv_type_u8 *pBeaconTim;
    struct APStaInfo *pStaConInfo=NULL;
    struct ap_tx_desp *pApTxData;
    struct wpa_authenticator wpa_auth;
	struct ssv6xxx_host_ap_config config;
    OsMutex       ap_info_ps_mutex;
#if(AUTO_BEACON==0)
    OsMutex       g_dev_bcn_mutex;
#endif
    ssv_type_u32 idx =0;
    ssv_vif* vif = (ssv_vif*)vif_ptr;

    //backup
    pOSMgmtframe=gDeviceInfo->APInfo->pOSMgmtframe;
    pMgmtPkt=gDeviceInfo->APInfo->pMgmtPkt;
    tim=gDeviceInfo->APInfo->tim;
    pBeaconHead=gDeviceInfo->APInfo->pBeaconHead;
    pBeaconTail=gDeviceInfo->APInfo->pBeaconTail;
    pBeaconTim=gDeviceInfo->APInfo->pBeaconTim;
    pApTxData=gDeviceInfo->APInfo->pApTxData;
    ap_info_ps_mutex=gDeviceInfo->APInfo->ap_info_ps_mutex;
#if(AUTO_BEACON==0)
    g_dev_bcn_mutex=gDeviceInfo->APInfo->g_dev_bcn_mutex;
#endif
    wpa_auth=gDeviceInfo->APInfo->wpa_auth;
    config=gDeviceInfo->APInfo->config;
    pStaConInfo=gDeviceInfo->APInfo->StaConInfo;
    //reset
    OS_MemSET(gDeviceInfo->APInfo, 0, sizeof(struct ApInfo));    
    gDeviceInfo->APInfo->StaConInfo=pStaConInfo;
	do
	{
		APStaInfo_StaReset(&gDeviceInfo->APInfo->StaConInfo[idx]);
		idx++;
	}while(idx<gMaxAID);

    //restore
    gDeviceInfo->APInfo->pBeaconHead = (ssv_type_u8 *)gDeviceInfo->APInfo->bcn + pb_offset;
    gDeviceInfo->APInfo->pOSMgmtframe=pOSMgmtframe;
    gDeviceInfo->APInfo->pMgmtPkt=pMgmtPkt;
    gDeviceInfo->APInfo->tim=tim;
    gDeviceInfo->APInfo->pBeaconHead=pBeaconHead;
    gDeviceInfo->APInfo->pBeaconTail=pBeaconTail;
    gDeviceInfo->APInfo->pBeaconTim=pBeaconTim;
    gDeviceInfo->APInfo->pApTxData=pApTxData;
    gDeviceInfo->APInfo->ap_info_ps_mutex=ap_info_ps_mutex;
#if(AUTO_BEACON==0)
    gDeviceInfo->APInfo->g_dev_bcn_mutex=g_dev_bcn_mutex;
#endif
    gDeviceInfo->APInfo->wpa_auth=wpa_auth;
    gDeviceInfo->APInfo->config=config;
    ssv6xxx_config_init(&gDeviceInfo->APInfo->config);
	gDeviceInfo->APInfo->eCurrentApMode	= gDeviceInfo->APInfo->config.eApMode;
	gDeviceInfo->APInfo->b80211n			= gDeviceInfo->APInfo->config.b80211n;
	gDeviceInfo->APInfo->nCurrentChannel	= gDeviceInfo->APInfo->config.nChannel;
	gDeviceInfo->APInfo->auth_algs = WPA_AUTH_ALG_OPEN;
	//gDeviceInfo->APInfo->auth_algs = WPA_AUTH_ALG_OPEN|WPA_AUTH_ALG_SHARED;
    OS_MemCPY(gDeviceInfo->APInfo->own_addr,vif->self_mac,6);
    gDeviceInfo->APInfo->TimAllOne = g_host_cfg.AP_TimAllOne;

}
#endif


