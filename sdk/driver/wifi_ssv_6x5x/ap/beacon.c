/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#define __SFILE__ "beacon.c"

#include <log.h>
#include <pbuf.h>
#include <ssv_timer.h>
#include <country_cfg.h>
#include <host_apis.h>
#include <core/recover.h>
#include <core/channel.h>
#include <core/Regulatory.h>
#include "common/ieee802_11_defs.h"
#include "common/ieee802_11_common.h"
#include <ieee80211.h>
#include "common/bitmap.h"

#include "ap_tx.h"
#include "beacon.h"
#include "ap_info.h"
#include "ap_drv_cmd.h"
#include "ap_mlme.h"
#include "ieee802_11_mgmt.h"
#include "ieee802_11_ht.h"
#include "wmm.h"
#include <ssv_devinfo.h>

#if (AP_MODE_ENABLE == 1)

void GenBeacon(void);
extern struct Host_cfg g_host_cfg;


//For 80211G
static ssv_type_u8 ieee802_11_erp_info(ApInfo_st *pApInfo)
{
	ssv_type_u8 erp = 0;

	if (pApInfo->eCurrentApMode != AP_MODE_IEEE80211G)
		return 0;

	if (pApInfo->olbc)
		erp |= ERP_INFO_USE_PROTECTION;

	if (pApInfo->num_sta_non_erp > 0) {
		erp |= ERP_INFO_NON_ERP_PRESENT |
			ERP_INFO_USE_PROTECTION;
	}

	if (pApInfo->num_sta_no_short_preamble > 0 ||
	    pApInfo->config.preamble == LONG_PREAMBLE)
		erp |= ERP_INFO_BARKER_PREAMBLE_MODE;

	return erp;
}


ssv_type_u8 * hostapd_eid_ds_params(ApInfo_st *pApInfo, ssv_type_u8 *eid)
{
	*eid++ = WLAN_EID_DS_PARAMS;
	*eid++ = 1;
	*eid++ = pApInfo->nCurrentChannel;// hapd->iconf->channel;
	return eid;
}


ssv_type_u8 * hostapd_eid_erp_info(ApInfo_st *pApInfo, ssv_type_u8 *eid)
{
	if (pApInfo->eCurrentApMode != AP_MODE_IEEE80211G)
		return eid;

	/* Set NonERP_present and use_protection bits if there
	 * are any associated NonERP stations. */

	/* TODO:
	 *A. use_protection bit can be set to zero even if
	 * there are NonERP stations present. This optimization
	 * might be useful if NonERP stations are "quiet"
	 * See 802.11g/D6 E-1 for recommended practice.

	 * B. In addition, Non ERP present might be set, if AP detects Non ERP
	 * operation on other APs. */

	/* Add ERP Information element */
	*eid++ = WLAN_EID_ERP_INFO;
	*eid++ = 1;
	*eid++ = ieee802_11_erp_info(pApInfo);

	return eid;
}


ssv_type_u8 * hostapd_eid_csa(ApInfo_st *pApInfo, ssv_type_u8 *eid)
{
	//ssv_type_u8 chan;

	*eid++ = WLAN_EID_CHANNEL_SWITCH;
	*eid++ = 3;
	*eid++ = 1;
	*eid++ = pApInfo->nCurrentChannel;
	*eid++ = 0;

	return eid;
}

ssv_type_u8 * hostapd_eid_secondary_channel(ApInfo_st *pApInfo, ssv_type_u8 *eid)
{
	ssv_type_u8 *pos = eid;
	//ssv_type_u8 sec_ch;

	*pos++ = WLAN_EID_SECONDARY_CHANNEL_OFFSET;
	 pos++;  /* length will be set later */
	*pos++ = 0;

	eid[1] = (pos - eid) - 2;

	return pos;
}

static ssv_type_u8 * hostapd_eid_country_add(ssv_type_u8 *pos, ssv_type_u8 *end, int chan_spacing,ssv_type_u8 s,ssv_type_u8 e)
{
	if (end - pos < 3)
		return pos;

	/* first channel number */
	*pos++ = s;
	/* number of channels */
	*pos++ = (e - s) / chan_spacing + 1;
	/* maximum transmit power level */
	*pos++ = 20;

	return pos;
}

ssv_type_u8 * hostapd_eid_country(ApInfo_st *pApInfo, ssv_type_u8 *eid,
				int max_len)
{
    ssv_type_u8 *pos = eid;
    ssv_type_u8 *end = eid + max_len;
    ssv_type_u16 s=0,e=0;
    ssv_type_u8 channel_space=0;
    ssv_type_u8 i=0;
    struct ieee80211_regdomain *reg=NULL;

    if (max_len < 6)
        return eid;

    *pos++ = WLAN_EID_COUNTRY;
    pos++; /* length will be set later */


    reg=ssv6xxx_wifi_get_current_regdomain();
    if(reg==NULL)
    {
        return eid;
    }

    pos[0]=reg->alpha2[0];
    pos[1]=reg->alpha2[1];
    pos[2]=reg->alpha2[2];    
    pos += 3;

#if 1
    channel_space=1;
    for(i=0;i<MAX_2G_CHANNEL_NUM;i++)
    {
        
        if(gDeviceInfo->available_2g_channel_mask&(1<<i))
        {
            if(s!=0)
            {
                if((e+channel_space)==ssv6xxx_2ghz_chantable[i].hw_value)
                {
                    e=ssv6xxx_2ghz_chantable[i].hw_value;
                }
                else
                {
                    pos=hostapd_eid_country_add(pos,end,channel_space,s,e);
                    s=e=ssv6xxx_2ghz_chantable[i].hw_value;
                }
            }
            else
            {
                s=e=ssv6xxx_2ghz_chantable[i].hw_value;
            }
        }
        
    }
    pos=hostapd_eid_country_add(pos,end,channel_space,s,e);
    
    if(TRUE==ssv6xxx_wifi_support_5g_band())
    {
        s=0;
        e=0;
        channel_space=4;
        for(i=0;i<MAX_5G_CHANNEL_NUM;i++)
        {
            if(gDeviceInfo->available_5g_channel_mask&(1<<i))
            {
                if(1==g_host_cfg.ap_no_dfs)
                {
                    //if this cannel need to support DFS function, skip it
                    if(TRUE==freq_need_dfs(ssv6xxx_5ghz_chantable[i].center_freq, BW, reg))
                    {
                        continue;
                    }
                }
                
                if(s!=0)
                {
                    if((e+channel_space)==ssv6xxx_5ghz_chantable[i].hw_value)
                    {
                        e=ssv6xxx_5ghz_chantable[i].hw_value;
                    }
                    else
                    {
                        pos=hostapd_eid_country_add(pos,end,channel_space,s,e);
                        s=e=ssv6xxx_5ghz_chantable[i].hw_value;
                    }
                }
                else
                {
                    s=e=ssv6xxx_5ghz_chantable[i].hw_value;
                }
            }        
        }    
    }
    pos=hostapd_eid_country_add(pos,end,channel_space,s,e);
#else
	/* first channel number */
	*pos++ = 1;
	/* number of channels */
	*pos++ = 11;
	/* maximum transmit power level */
	*pos++ = 16;

    if(TRUE==ssv6xxx_wifi_support_5g_band())
    {
        if(IS_AP_IN_5G_BAND())
        {
            // ADD 5G band
            /* first channel number */
        	*pos++ = 52;
        	/* number of channels */
        	*pos++ = 4;
        	/* maximum transmit power level */
        	*pos++ = 16;
        }
    }

#endif
 	if ((pos - eid) & 1) {
		*pos++ = 0; /* pad for 16-bit alignment */
	}



	eid[1] = (pos - eid) - 2;

	return pos;
}

//#pragma message("===================================================")
//#pragma message("       wpa_auth_get_wpa_ie not implement yet")
//#pragma message("===================================================")
ssv_type_u8 * hostapd_eid_wpa(ApInfo_st *pApInfo, ssv_type_u8 *eid, ssv_type_size_t len)
{


#ifndef CONFIG_NO_WPA2

//start :modify by angel.chiu for wpa
    if(pApInfo->ap_conf.wpa == WPA_PROTO_RSN)
    {
        const ssv_type_u8 *ie;
    	ssv_type_size_t ielen;
    	ie = (const ssv_type_u8 *)wpa_auth_get_wpa_ie(&(pApInfo->wpa_auth), &ielen);

    	if (ie == NULL || ielen > len)
    		return eid;

    	OS_MemCPY(eid, ie, ielen);
    	return eid + ielen;
    }
    else
        return eid;
//end: mosify by angel.chiu for wpa2

#else

//*********************
	//not include the field firstly
	return  eid;

#endif
}


void ieee802_11_set_beacon(ApInfo_st *pApInfo, ssv_type_bool post_to_mlme)
{
//--------------------------------
//Head-TIM-Tail
//--------------------------------

	struct ieee80211_mgmt *head;
	ssv_type_u8 *pos, *tailpos;
	ssv_type_u16 capab_info;
    ssv_vif* vif = (ssv_vif*)gDeviceInfo->APInfo->vif;

    if((vif->hw_mode!=SSV6XXX_HWM_AP
       ||(FALSE==ssv_hal_is_beacon_enable()))
       &&post_to_mlme)// check not form start beacon
    {
       LOG_PRINTF("No need to set beacon.(Not in AP mode or Beacon disable)\r\n");
       return;
    }

#ifdef CONFIG_WPS
	if (hapd->conf->wps_state && hapd->wps_beacon_ie)
		tail_len += wpabuf_len(hapd->wps_beacon_ie);
#endif /* CONFIG_WPS */
#ifdef CONFIG_P2P
	if (hapd->p2p_beacon_ie)
		tail_len += wpabuf_len(hapd->p2p_beacon_ie);
#endif /* CONFIG_P2P */



    head = (struct ieee80211_mgmt *)pApInfo->pBeaconHead;


	head->frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
					   WLAN_FC_STYPE_BEACON);
	head->duration = host_to_le16(0);

	OS_MemSET((void*)head->da, 0xff, ETH_ALEN);





	OS_MemCPY((void*)head->sa, (void*)pApInfo->own_addr, ETH_ALEN);
	OS_MemCPY((void*)head->bssid, (void*)pApInfo->own_addr, ETH_ALEN);

	head->u.beacon.beacon_int =
		host_to_le16(g_host_cfg.bcn_interval);

	/* hardware or low-level driver will setup seq_ctrl and timestamp */

	capab_info = hostapd_own_capab_info(pApInfo, NULL, 0);
	head->u.beacon.capab_info = host_to_le16(capab_info);
	pos = (ssv_type_u8 *)&head->u.beacon.variable[0];

	/* SSID */
	*pos++ = WLAN_EID_SSID;
// 	if (hapd->conf->ignore_broadcast_ssid == 2) {
// 		/* clear the data, but keep the correct length of the SSID */
// 		*pos++ = hapd->conf->ssid.ssid_len;
// 		os_memset(pos, 0, hapd->conf->ssid.ssid_len);
// 		pos += hapd->conf->ssid.ssid_len;
// 	} else if (hapd->conf->ignore_broadcast_ssid) {
// 		*pos++ = 0; /* empty SSID */
// 	} else {

		*pos++ = pApInfo->config.ssid_len;
		OS_MemCPY(pos, pApInfo->config.ssid,
			  pApInfo->config.ssid_len);
		pos += pApInfo->config.ssid_len;
//	}


	/* Supported rates */
	pos = hostapd_eid_supp_rates(pApInfo, pos);

	/* DS Params */
	pos = hostapd_eid_ds_params(pApInfo, pos);

	pApInfo->nBHeadLen = pos - (ssv_type_u8 *) head;

//-----------------------------------------------------
//-----------------------------------------------------

//TIM
	pApInfo->pBeaconTim = pos;

//***************************************************************************
//***************************************************************************
	// Tail
	tailpos = pApInfo->pBeaconTail;


	//Plus 2 for
	//pApInfo->pBeaconTail = tailpos = pos;

	tailpos = hostapd_eid_country(pApInfo, tailpos,
		AP_MGMT_BEACON_LEN - pApInfo->nBHeadLen);

	/* ERP Information element */
    if(!IS_AP_IN_5G_BAND())
    {
	    tailpos = hostapd_eid_erp_info(pApInfo, tailpos);
    }
 	/* Extended supported rates */
 	tailpos = hostapd_eid_ext_supp_rates(pApInfo, tailpos);

 	/* RSN, MDIE, WPA */
 	tailpos = hostapd_eid_wpa(pApInfo, tailpos,(ssv_type_size_t) AP_BEACON_TAIL_BUF_SIZE - (tailpos-(ssv_type_u8*)pApInfo->pBeaconTail) );

#if AP_SUPPORT_80211N
	tailpos = hostapd_eid_ht_capabilities(pApInfo, tailpos);
	tailpos = hostapd_eid_ht_operation(pApInfo, tailpos);
#endif /* AP_SUPPORT_80211N */

#if 0
	tailpos = hostapd_eid_ext_capab(hapd, tailpos);

	/*
	 * TODO: Time Advertisement element should only be included in some
	 * DTIM Beacon frames.
	 */
	tailpos = hostapd_eid_time_adv(hapd, tailpos);

	tailpos = hostapd_eid_interworking(hapd, tailpos);
	tailpos = hostapd_eid_adv_proto(hapd, tailpos);
	tailpos = hostapd_eid_roaming_consortium(hapd, tailpos);
#endif
	/* Wi-Fi Alliance WMM */
	tailpos = hostapd_eid_wmm(pApInfo, tailpos);

#ifdef CONFIG_WPS
	if (hapd->conf->wps_state && hapd->wps_beacon_ie) {
		os_memcpy(tailpos, wpabuf_head(hapd->wps_beacon_ie),
			  wpabuf_len(hapd->wps_beacon_ie));
		tailpos += wpabuf_len(hapd->wps_beacon_ie);
	}
#endif /* CONFIG_WPS */

#ifdef CONFIG_P2P
	if ((hapd->conf->p2p & P2P_ENABLED) && hapd->p2p_beacon_ie) {
		os_memcpy(tailpos, wpabuf_head(hapd->p2p_beacon_ie),
			  wpabuf_len(hapd->p2p_beacon_ie));
		tailpos += wpabuf_len(hapd->p2p_beacon_ie);
	}
#endif /* CONFIG_P2P */
#ifdef CONFIG_P2P_MANAGER
	if ((hapd->conf->p2p & (P2P_MANAGE | P2P_ENABLED | P2P_GROUP_OWNER)) ==
	    P2P_MANAGE)
		tailpos = hostapd_eid_p2p_manage(hapd, tailpos);
#endif /* CONFIG_P2P_MANAGER */



#if (BEACON_DBG == 1)
//Add this element info for beacon release test
#define WPA_PUT_BE24(a, val)					\
	do {							\
	(a)[0] = (ssv_type_u8) ((((ssv_type_u32) (val)) >> 16) & 0xff);	\
	(a)[1] = (ssv_type_u8) ((((ssv_type_u32) (val)) >> 8) & 0xff);	\
	(a)[2] = (ssv_type_u8) (((ssv_type_u32) (val)) & 0xff);		\
	} while (0)

#define OUI_WFA 0x506f9a

	*tailpos++ = WLAN_EID_VENDOR_SPECIFIC;
	*tailpos++ = 7;
	WPA_PUT_BE24(tailpos, OUI_WFA);
	tailpos += 3;

	/* Hotspot Configuration: DGAF Enabled */
	//u32 length
	*tailpos++ = 16;
	*tailpos++ = 16;
	*tailpos++ = 16;
	*tailpos++ = 16;
#endif//#if (BEACON_DBG == 1)



	pApInfo->nBTailLen = tailpos > pApInfo->pBeaconTail ? tailpos - pApInfo->pBeaconTail : 0;

//----------------------------------------------------------------------



//short_slot_time, preamble, cts_protect, ht_opmode,

//#pragma message("===================================================")
//#pragma message("Todo Set preamble, protection to HW")
//#pragma message("===================================================")

	//GenBeacon();
#if(MLME_TASK ==1)
    if(post_to_mlme)
        ap_mlme_handler(NULL,MEVT_BCN_CMD);
    else
        GenBeacon();

#else
    GenBeacon();
#endif


}

ssv_type_u8 *ieee80211_beacon_add_tim(ApInfo_st *pApInfo,ssv_type_u8 *eid)
{
    ssv_type_u8 *pos, *tim;
    int aid0 = 0;
    int i, have_bits = 0, n1, n2;
    ssv_type_u32 j;
    /* Generate bitmap for TIM only if there are any STAs in power save
    * mode. */
    //OS_MutexLock(pApInfo->ap_info_ps_mutex);
    if (pApInfo->num_sta_ps > 0)
        /* in the hope that this is faster than
             * checking byte-for-byte */
        have_bits = !bitmap_empty((unsigned long*)pApInfo->tim,gMaxAID+1);
    //OS_MutexUnLock(pApInfo->ap_info_ps_mutex);

    //if (pApInfo->dtim_count == 0)
    //	pApInfo->dtim_count = AP_DEFAULT_DTIM_PERIOD - 1;
    //else
    //	pApInfo->dtim_count--;

    tim = pos = eid;//= (u8 *) skb_put(skb, 6);

    *pos++ = WLAN_EID_TIM;
    *pos++ = 4;

    gDeviceInfo->APInfo->bcn_info.tim_cnt_oft = pos - pApInfo->pBeaconHead;

    *pos++ = pApInfo->dtim_count;
    *pos++ = AP_DEFAULT_DTIM_PERIOD;

    //if (pApInfo->dtim_count == 0 && !ap_tx_desp_queue_empty(&pApInfo->ps_bc_buf))
    //	aid0 = 1;

    //pApInfo->dtim_bc_mc = aid0 == 1;

    if (have_bits) 
    {
    /* Find largest even number N1 so that bits numbered 1 through
        * (N1 x 8) - 1 in the bitmap are 0 and number N2 so that bits
        * (N2 + 1) x 8 through 2007 are 0. */
        n1 = 0;
        for (j = 0; j < gMaxTimLen; j++) {
            if (pApInfo->tim[j]) {
                n1 = j & 0xfe;
                break;
            }
        }
        n2 = n1;
        for (i = gMaxTimLen - 1; i >= n1; i--) {
            if (pApInfo->tim[i]) {
                n2 = i;
                break;
            }
        }

        /* Bitmap control */
        *pos++ = n1 | aid0;
        /* Part Virt Bitmap */
        //skb_put(skb, n2 - n1);
        OS_MemCPY(pos, pApInfo->tim + n1, (ssv_type_u32)n2 - n1 + 1);

        tim[1] = n2 - n1 + 4;							//length
        pos+=(n2-n1+1);
    } 
    else 
    {
        *pos++ = aid0; /* Bitmap control */
        //#if(AP_MODE_BEACON_VIRT_BMAP_0XFF == 0)
        if(!gDeviceInfo->APInfo->TimAllOne)
        {
            *pos++ = 0; /* Part Virt Bitmap */
        }
        else //#else
        {   
            *pos++ = 0xff; /* Part Virt Bitmap */
        }
        //#endif
    }

    return pos;
}


//extern H_APIs s32 _ssv_wifi_send(void *data, s32 len, struct cfg_host_txreq *txreq, ssv_type_bool bAPFrame, u32 TxFlags);


extern ssv_type_s32 ssv6xxx_drv_send(void *dat, ssv_type_size_t len);


extern struct ap_tx_desp *
ap_get_buffered_bc(ApInfo_st *pAPInfo);




//u32 g_bcount = 0;


void GenBeacon(void)
{

    ssv_type_u8 nBlen=0;
    ssv_type_u8 *pos = NULL;

    OS_MutexLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
    pos = ieee80211_beacon_add_tim(gDeviceInfo->APInfo, gDeviceInfo->APInfo->pBeaconTim);

    //Add beacon rail
    OS_MemCPY(pos, gDeviceInfo->APInfo->pBeaconTail, gDeviceInfo->APInfo->nBTailLen);
    nBlen = (pos+gDeviceInfo->APInfo->nBTailLen) - gDeviceInfo->APInfo->pBeaconHead;
    gDeviceInfo->APInfo->bcn_info.bcn_len = nBlen;
    OS_MutexUnLock(gDeviceInfo->APInfo->ap_info_ps_mutex);

    ap_soc_set_bcn(SSV6XXX_SET_BEACON, gDeviceInfo->APInfo->bcn, &gDeviceInfo->APInfo->bcn_info, AP_DEFAULT_DTIM_PERIOD-1, g_host_cfg.bcn_interval,TRUE);

}








void StartBeacon(void)
{
    LOG_DEBUGF(LOG_L2_AP,("StartBeacon\n\r"));
    if(gDeviceInfo->recovering != TRUE)
        LOG_TRACE("%s Set Beacon and relative params to soc.\r\n",__func__);
        ieee802_11_set_beacon(gDeviceInfo->APInfo,FALSE);
}


//------------------------------------------------------------------------------------------------


#if 0



{
	os_create_timer(AP_DEFAULT_BEACON_INT*10, timertest1, NULL, NULL);
	os_create_timer(AP_DEFAULT_BEACON_INT*20, timertest2, NULL, NULL);
	os_create_timer(AP_DEFAULT_BEACON_INT*30, timertest3, NULL, NULL);
	os_create_timer(AP_DEFAULT_BEACON_INT*40, timertest4, NULL, NULL);
	os_create_timer(AP_DEFAULT_BEACON_INT*50, timertest5, NULL, NULL);



	os_create_timer(AP_DEFAULT_BEACON_INT*60, timertest6, (void*)6, NULL);
	os_create_timer(AP_DEFAULT_BEACON_INT*70, timertest6, (void*)7, NULL);
	os_create_timer(AP_DEFAULT_BEACON_INT*80, timertest6, (void*)8, NULL);
	os_create_timer(AP_DEFAULT_BEACON_INT*90, timertest6, (void*)9, NULL);
}

void timertest1(void *data1, void *data2)
{
	LOG_TRACE( "timertest1 %d MS \n", os_tick2ms(os_sys_jiffies()));
	os_create_timer(AP_DEFAULT_BEACON_INT*10, timertest1, NULL, NULL);
}


void timertest2(void *data1, void *data2)
{
	LOG_TRACE( "timertest2 %d MS \n", os_tick2ms(os_sys_jiffies()));
	os_create_timer(AP_DEFAULT_BEACON_INT*20, timertest2, NULL, NULL);
}

void timertest3(void *data1, void *data2)
{
	LOG_TRACE( "timertest3 %d MS \n", os_tick2ms(os_sys_jiffies()));
	os_create_timer(AP_DEFAULT_BEACON_INT*30, timertest3, NULL, NULL);
}


void timertest4(void *data1, void *data2)
{
	LOG_TRACE( "timertest4 %d MS \n", os_tick2ms(os_sys_jiffies()));
	os_create_timer(AP_DEFAULT_BEACON_INT*40, timertest4, NULL, NULL);
}




void timertest5(void *data1, void *data2)
{
	LOG_TRACE( "timertest5 %d MS \n", os_tick2ms(os_sys_jiffies()));
	os_create_timer(AP_DEFAULT_BEACON_INT*50, timertest5, NULL, NULL);
}

void timertest6(void *data1, void *data2)
{
	u32 itval = AP_DEFAULT_BEACON_INT* (u32)data1 * 10;
	LOG_TRACE( "timertest6 (u32)data1:%d--> %d MS \n",(u32)data1, os_tick2ms(os_sys_jiffies()));
	os_create_timer(itval, timertest6, data1, NULL);
}

#endif
//#endif /* CONFIG_NATIVE_WINDOWS */
#endif
