/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#define __SFILE__ "ap_rx.c"

#include <ssv_ether.h>
#include <host_apis.h>
#include <core/mlme.h>
#include <ssv_dev.h>
#include <ssv_hal.h>
#include "ap_sta_info.h"
#include "ap_info.h"
#include "ap_def.h"
#include "ap_rx.h"
#include "ap_mlme.h"
#include <ssv_devinfo.h>
#include <ssv_timer.h>

#if (AP_MODE_ENABLE == 1)

static rx_result rx_h_check(struct ap_rx_desp *rx);
static rx_result rx_h_sta_process(struct ap_rx_desp *rx);
extern void sta_info_cleanup(void *data1, void *data2);




static rx_result rx_h_drop(struct ap_rx_desp *rx)
{

	if(rx->flags&AP_RX_FLAGS_FRAME_TYPE_PS_POLL
		|| rx->flags&AP_RX_FLAGS_FRAME_TYPE_NULL_DATA)
		return RX_DROP;
	else
		return RX_CONTINUE;
}


//Check data need to be received to upper layer now
//If data is used for power saving mode, trigger frame/ps-poll/start-end ps. It's no need to transmit to upper layer.
ssv_type_bool ssv6xxx_data_need_to_be_received(struct	ap_rx_desp *rx_desp)
{
 	ssv_type_bool bRet = FALSE;
  	rx_result res = RX_DROP;



	#define CALL_RXH(rxh) \
			res = rxh(rx_desp);		\
			if (res != RX_CONTINUE) \
				break
		do
		{

			CALL_RXH(rx_h_check);
			CALL_RXH(rx_h_uapsd_and_pspoll);
			CALL_RXH(rx_h_sta_process);
			CALL_RXH(rx_h_drop);

		} while (0);




		if (res == RX_CONTINUE)
		{
			bRet = TRUE;
		}


 	return bRet;
}

//Filter frames according to frame state(1,2,3)
//Unauthenticated Unassociated, authenticated Unassociated, authenticated associated
static rx_result
rx_h_check(struct ap_rx_desp *rx)
{
// 	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)rx->skb->data;
// 	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(rx->skb);
//
// 	/* Drop duplicate 802.11 retransmissions (IEEE 802.11 Chap. 9.2.9) */
// 	if (rx->sta && !is_multicast_ether_addr(hdr->addr1)) {
// 		if (unlikely(ieee80211_has_retry(hdr->frame_control) &&
// 			     rx->sta->last_seq_ctrl[rx->seqno_idx] ==
// 			     hdr->seq_ctrl)) {
// 			if (status->rx_flags & IEEE80211_RX_RA_MATCH) {
// 				rx->local->dot11FrameDuplicateCount++;
// 				rx->sta->num_duplicates++;
// 			}
// 			return RX_DROP_UNUSABLE;
// 		} else
// 			rx->sta->last_seq_ctrl[rx->seqno_idx] = hdr->seq_ctrl;
// 	}
//
// 	if (unlikely(rx->skb->len < 16)) {
// 		I802_DEBUG_INC(rx->local->rx_handlers_drop_short);
// 		return RX_DROP_MONITOR;
// 	}
//
	/* Drop disallowed frame classes based on STA auth/assoc state;
	 * IEEE 802.11, Chap 5.5.
	 *
	 * mac80211 filters only based on association state, i.e. it drops
	 * Class 3 frames from not associated stations. hostapd sends
	 * deauth/disassoc frames when needed. In addition, hostapd is
	 * responsible for filtering on both auth and assoc states.
	 */

// 	if (ieee80211_vif_is_mesh(&rx->sdata->vif))
// 		return ieee80211_rx_mesh_check(rx);

	if (!rx->sta || !test_sta_flag(rx->sta, WLAN_STA_ASSOC)) {

//#pragma message("===================================================")
//#pragma message("     Need to check EAPOL packets")
//#pragma message("===================================================")

//
// 		EAPOL packets
//		if (rx->sta && rx->sta->dummy &&
//		    ieee80211_is_data_present(hdr->frame_control)) {
//			u16 ethertype;
//			u8 *payload;
//
//			payload = rx->skb->data +
//				ieee80211_hdrlen(hdr->frame_control);
//			ethertype = (payload[6] << 8) | payload[7];
//			if (cpu_to_be16(ethertype) ==
//			    rx->sdata->control_port_protocol)
//				return RX_CONTINUE;
//		}

		//not related to any station

		return RX_DROP;
	}

	//pass through all management and control frames

	return RX_CONTINUE;
}


extern const ssv_type_u8 up_to_ac[];



//Ps-poll frame and trigger frame
rx_result rx_h_uapsd_and_pspoll(struct ap_rx_desp *rx)
{
 	int tid, ac;


	/*
	 * Don't do anything if the station isn't already asleep. In
	 * the uAPSD case, the station will probably be marked asleep,
	 * in the PS-Poll case the station must be confused ...
	 */
	if (!test_sta_flag(rx->sta, WLAN_STA_PS_STA))
		return RX_CONTINUE;


	if (rx->flags&AP_RX_FLAGS_FRAME_TYPE_PS_POLL)
	{
		sta_ps_deliver_poll_response(rx->sta);
		return RX_DROP;

	}
 	else if ((rx->flags&AP_RX_FLAGS_PM_BIT) &&
		((rx->flags& (AP_RX_FLAGS_QOS_BIT|AP_RX_FLAGS_FRAME_TYPE_DATA)) || (rx->flags& (AP_RX_FLAGS_QOS_BIT|AP_RX_FLAGS_FRAME_TYPE_NULL_DATA))) )
	{

		#ifdef __AP_DEBUG__
		LOG_TRACE("Trigger Frame from STA: "MACSTR "\r\n", MAC2STR(rx->sta->addr));
        #endif
		//WMM 1.0 spec  3.6.0.7
		tid = rx->UP;
		ac = up_to_ac[tid & 7];

		/*
		 * If this AC is not trigger-enabled do nothing.
		 *
		 * NB: This could/should check a separate bitmap of trigger-
		 * enabled queues, but for now we only implement uAPSD w/o
		 * TSPEC changes to the ACs, so they're always the same.
		 */
		if (!(rx->sta->uapsd_queues & BIT(ac)))
			return RX_CONTINUE;

		/* if we are in a service period, do nothing */
		if (test_sta_flag(rx->sta, WLAN_STA_SP))
			return RX_CONTINUE;

		//if (!test_sta_flag(rx->sta, WLAN_STA_PS_DRIVER))
			sta_ps_deliver_uapsd(rx->sta);
		//else
		//	set_sta_flag(rx->sta, WLAN_STA_UAPSD);
	}

 	return RX_CONTINUE;
}


//-------------

static void ap_sta_ps_start(struct APStaInfo *sta)
{
	//struct ieee80211_sub_if_data *sdata = sta->sdata;
	//struct ieee80211_local *local = sdata->local;
    OS_MutexLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
	gDeviceInfo->APInfo->num_sta_ps++;
    OS_MutexUnLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
    OS_MutexLock(sta->apsta_mutex);
	set_sta_flag(sta, WLAN_STA_PS_STA);
    OS_MutexUnLock(sta->apsta_mutex);


//	if (!(local->hw.flags & IEEE80211_HW_AP_LINK_PS))
//		drv_sta_notify(local, sdata, STA_NOTIFY_SLEEP, &sta->sta);
//#ifdef CONFIG_MAC80211_VERBOSE_PS_DEBUG
//	printk(KERN_DEBUG "%s: STA %pM aid %d enters power save mode\n",
//	       sdata->name, sta->sta.addr, sta->sta.aid);
//#endif /* CONFIG_MAC80211_VERBOSE_PS_DEBUG */
}
extern void sta_ps_deliver_wakeup(APStaInfo_st *sta);
static void ap_sta_ps_end(struct APStaInfo *sta)
{
//	struct ieee80211_sub_if_data *sdata = sta->sdata;
//

	if (gDeviceInfo->APInfo->num_sta_ps)
    {
        OS_MutexLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
		gDeviceInfo->APInfo->num_sta_ps--;
        OS_MutexUnLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
    }
	else
		return;

//
//#ifdef CONFIG_MAC80211_VERBOSE_PS_DEBUG
//	printk(KERN_DEBUG "%s: STA %pM aid %d exits power save mode\n",
//	       sdata->name, sta->sta.addr, sta->sta.aid);
//#endif /* CONFIG_MAC80211_VERBOSE_PS_DEBUG */
//
//	if (test_sta_flag(sta, WLAN_STA_PS_DRIVER)) {
//#ifdef CONFIG_MAC80211_VERBOSE_PS_DEBUG
//		printk(KERN_DEBUG "%s: STA %pM aid %d driver-ps-blocked\n",
//		       sdata->name, sta->sta.addr, sta->sta.aid);
//#endif /* CONFIG_MAC80211_VERBOSE_PS_DEBUG */
//		return;
//	}

	sta_ps_deliver_wakeup(sta);


	if (!gDeviceInfo->APInfo->num_sta_ps)
    {
		os_cancel_timer(sta_info_cleanup, (ssv_type_u32)0, (ssv_type_u32)0);
	    OS_MutexLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
        gDeviceInfo->APInfo->sta_pkt_cln_timer = FALSE;
	    OS_MutexUnLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
    }



}




static rx_result rx_h_sta_process(struct ap_rx_desp *rx)
{
 	struct APStaInfo *sta = rx->sta;
 	if (!sta)
 		return RX_CONTINUE;

	sta->last_rx = OS_GetSysTick();


	/*
	 * Change STA power saving mode only at the end of a frame
	 * exchange sequence.
	 */

		if (test_sta_flag(sta, WLAN_STA_PS_STA)) {
			/*
			 * Ignore doze->wake transitions that are
			 * indicated by non-data frames, the standard
			 * is unclear here, but for example going to
			 * PS mode and then scanning would cause a
			 * doze->wake transition for the probe request,
			 * and that is clearly undesirable.
			 */
			if (((rx->flags&AP_RX_FLAGS_FRAME_TYPE_DATA)||(rx->flags&AP_RX_FLAGS_FRAME_TYPE_NULL_DATA))&&
				!(rx->flags&AP_RX_FLAGS_PM_BIT))
			{
				//LOG_TRACE("STA: "MACSTR " leave from PS mode\r\n", MAC2STR(rx->sta->addr));
				//LOG_PRINTF("wsid(%d) PS leave\r\n", rx->sta->wsid_idx);
				ap_sta_ps_end(sta);
			}
		}
		else
		{
			if ((rx->flags&AP_RX_FLAGS_PM_BIT))
			{
				//LOG_PRINTF("STA: "MACSTR " enter in PS mode\r\n", MAC2STR(rx->sta->addr));
				//LOG_PRINTF("wsid(%d),aid=%d enter PS\r\n", rx->sta->wsid_idx,rx->sta->aid);
				ap_sta_ps_start(sta);
                //it need to check FW queue status. if n_frame < 0 send host cmd to FW
                if(sta->fw_q_len>0)
                {
                    // indicate tim and generate beacon
                    recalc_tim_gen_beacon(sta);

                }
                else
                {
                    struct cfg_dq_status dqStatus;
                    dqStatus.wsid = sta->wsid_idx;
                    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_DQ_STATUS, &dqStatus, sizeof(struct cfg_dq_status),FALSE,FALSE);
                }
			}
		}

 	return RX_CONTINUE;
}



ssv_type_u8* ssv6xxx_host_rx_data_get_qos_ptr(CFG_HOST_RXPKT *rxpkt)
{
    return (ssv_type_u8 *)ssv_hal_get_rxpkt_qos_ptr(rxpkt);
}

//extern s32 _ssv6xxx_wifi_send_ethernet(void *frame, s32 len, enum ssv6xxx_data_priority priority, u8 tx_dscrp_flag,const ssv_type_bool mutexLock,const ssv_type_bool g_dev_mutex );
extern ssv_type_u16 HCmdEng_GetRawRxDataOffset(CFG_HOST_RXPKT *p);
ssv_inline static ssv_type_u32 ap_mode_fine_tune_rcpi(struct ap_rx_desp *rx_desp)
{
    //struct cfg_host_rxpkt *pPktInfoUpLayer=(struct cfg_host_rxpkt *)rx_desp->data;
    CFG_HOST_RXPKT *pPktInfoUpLayer=(CFG_HOST_RXPKT *)rx_desp->data;
    ssv_type_u32 rcpi=0;

    rcpi=ssv_hal_get_rxpkt_rcpi(pPktInfoUpLayer);

    if(rx_desp->sta->prev_rcpi)
    {
        rx_desp->sta->prev_rcpi = ((rcpi<< RSSI_DECIMAL_POINT_SHIFT)
            + (((rx_desp->sta->prev_rcpi)<<RSSI_SMOOTHING_SHIFT) - (rx_desp->sta->prev_rcpi))) >> RSSI_SMOOTHING_SHIFT;
        //pPktInfoUpLayer->RCPI= ((rx_desp->sta->prev_rcpi) >> RSSI_DECIMAL_POINT_SHIFT);
        rcpi= ((rx_desp->sta->prev_rcpi) >> RSSI_DECIMAL_POINT_SHIFT);
       ssv_hal_set_rxpkt_rcpi(pPktInfoUpLayer,rcpi);
    }
    else
        //rx_desp->sta->prev_rcpi = (pPktInfoUpLayer->RCPI<< RSSI_DECIMAL_POINT_SHIFT);
        rx_desp->sta->prev_rcpi = (rcpi<< RSSI_DECIMAL_POINT_SHIFT);

    //return (pPktInfoUpLayer->RCPI);
    return rcpi;
}

extern ssv_type_u16 CmdEng_GetRawRxDataOffset(CFG_HOST_RXPKT *p);

//get data from command queue
//it could be cmd/mgmt/ctrl/data
ssv6xxx_data_result AP_RxHandleData(void *frame)
{
    ssv6xxx_data_result data_ret = SSV6XXX_DATA_ACPT;
    ssv_type_bool bDropPktInfo = FALSE;
    CFG_HOST_RXPKT *pPktInfoUpLayer = (CFG_HOST_RXPKT *)OS_FRAME_GET_DATA(frame);

    void *pRelayFrame = NULL;
    struct ap_rx_desp rx_desp;

    OS_MemSET(&rx_desp, 0, sizeof(struct ap_rx_desp));

	do
	{
        //static const u8 pae_group_addr[ETH_ALEN]
        //	= { 0x01, 0x80, 0xC2, 0x00, 0x00, 0x03 };

        ethhdr *pEthHdr =  (ethhdr *)(ssv6xxx_host_rx_data_get_data_ptr(pPktInfoUpLayer));
        APStaInfo_st *pStaInfo = NULL;
        //u8 *raw =  (u8*)&(pEthHdr->h_proto);
        //le16 h_proto = raw[0]<<8|raw[1];

#ifdef __TEST__
        if(ETH_P_IP == h_proto)
        {
            int i;
            i=7;
        }
#endif
//--------------------------------------------------------------------------------------
//RX Check.

//Step1. Fill SA info in rx despriptor

        //Frame type, PM/QoS bit/UP

        if(ssv_hal_get_rxpkt_len(pPktInfoUpLayer)==ssv_hal_get_rxpkt_size())
            rx_desp.flags |= AP_RX_FLAGS_FRAME_TYPE_NULL_DATA;
        else
            rx_desp.flags |= AP_RX_FLAGS_FRAME_TYPE_DATA;


        rx_desp.flags |= (ssv_hal_get_rxpkt_psm(pPktInfoUpLayer))?AP_RX_FLAGS_PM_BIT:0;

        //Source address
        rx_desp.sta = APStaInfo_FindStaByAddr((ETHER_ADDR *)&pEthHdr->h_source);
        if(rx_desp.sta)
        {
            OS_MutexLock(rx_desp.sta->apsta_mutex);
            rx_desp.sta->arp_retry_count =0;
            OS_MutexUnLock(rx_desp.sta->apsta_mutex);
            rx_desp.flags |= test_sta_flag(rx_desp.sta, WLAN_STA_WMM)?AP_RX_FLAGS_QOS_BIT:0;
        }
        rx_desp.data = (void*)pPktInfoUpLayer;

        if(ssv_hal_get_rxpkt_qos(pPktInfoUpLayer))
        {
            ssv_type_u8 *qoshdr = ssv6xxx_host_rx_data_get_qos_ptr(pPktInfoUpLayer);
            rx_desp.UP = *qoshdr & IEEE80211_QOS_CTL_TID_MASK;
        }
        else
            rx_desp.UP = 0;

        //smooth rcpi value
        if(rx_desp.sta)
        {
            OS_MutexLock(rx_desp.sta->apsta_mutex);
            rx_desp.sta->rcpi= ap_mode_fine_tune_rcpi(&rx_desp);
            OS_MutexUnLock(rx_desp.sta->apsta_mutex);
        }
        if (!rx_desp.sta || !test_sta_flag(rx_desp.sta, WLAN_STA_ASSOC)) {
            i802_sta_deauth((ssv_type_u8*)gDeviceInfo->APInfo->own_addr, (ssv_type_u8*)&pEthHdr->h_source, WLAN_REASON_CLASS2_FRAME_FROM_NONAUTH_STA);
        }
//Step2. give it to check function
        if(FALSE == ssv6xxx_data_need_to_be_received(&rx_desp))
        {
            //LOG_PRINTF("****************Data drop\r\n");
            bDropPktInfo = TRUE;
            break;
        }
//---------------------------------------------------------------------------------------
//Relay.
        pStaInfo = NULL;
        {
            ssv_type_bool b_multicast = ssv_is_multicast_ether_addr(pEthHdr->h_dest);
            if ( b_multicast ||
            //Check if DA in our station list.
            ((pStaInfo = APStaInfo_FindStaByAddr((ETHER_ADDR*)pEthHdr->h_dest)) != NULL))//ieee80211_deliver_skb
            {
                //Need send packet to station.

                /*
                          * send multi-cast frames both to higher layers in
                         * local net stack and back to the wireless medium
                         */
                //u8 *qoshdr = ssv6xxx_host_rx_data_get_qos_ptr(pPktInfoUpLayer);
                //
                //if ( qoshdr && pStaInfo && test_sta_flag(pStaInfo, WLAN_STA_WMM))
                //	nRelayPriority = *qoshdr & IEEE80211_QOS_CTL_TID_MASK;

                if (pStaInfo)
                {
                    //1.Relay
                    //set flag to disable upper layer sent.
                    pPktInfoUpLayer = NULL;
                    //bDropPktInfo = TRUE;
                    data_ret = SSV6XXX_DATA_ACPT;
                    pRelayFrame = frame;
                    //LOG_PRINTF("****************Relay unicast frame to sta " ETH_ADDR_FORMAT " .\n", ETH_ADDR(pEthHdr->h_dest));
                }
                else
                {
                    //1.Relay 2.Upper layer
                    //broadcast frame->need to relay and send it to upper layer
                    //data_ret = SSV6XXX_DATA_CONT;
                    //LOG_PRINTF("****************Relay broadcast frame.\n");
                    pRelayFrame = os_frame_dup(frame);
                }

                if(pRelayFrame)
                {
                    ssv_vif* vif = (ssv_vif*)gDeviceInfo->APInfo->vif;
                    os_frame_pull(pRelayFrame,(ssv_type_u32)CmdEng_GetRawRxDataOffset((CFG_HOST_RXPKT *)OS_FRAME_GET_DATA(pRelayFrame)));
                    //LOG_PRINTF("AP_RxHandleData\r\n");
                    ssv6xxx_wifi_send_ethernet(vif->idx, pRelayFrame, OS_FRAME_GET_DATA_LEN(pRelayFrame), rx_desp.UP);
                }
                else
                    LOG_ERROR("%s can't duplicated frame to relay\r\n",__func__);

            }
        }
//---------------------------------------------------------------------------------------
//Relay to station

//---------------------------------------------------------------------------------------
//Forward to upper layer
        if(pPktInfoUpLayer)
        {
            //Others, send data to upper layer.
            // just return SSV6XXX_DATA_CONT, call back function will help us send to upper layer
            data_ret = SSV6XXX_DATA_CONT;
            bDropPktInfo = FALSE;
#if 0
            LOG_TRACE("Send this packet to upper layer\n");
            LOG_TRACE("DA:%x:%x:%x:%x:%x:%x SA:%x:%x:%x:%x:%x:%x\n",
            pEthHdr->h_dest[0], pEthHdr->h_dest[1], pEthHdr->h_dest[2], pEthHdr->h_dest[3], pEthHdr->h_dest[4], pEthHdr->h_dest[5],
            pEthHdr->h_source[0], pEthHdr->h_source[1], pEthHdr->h_source[2], pEthHdr->h_source[3], pEthHdr->h_source[4], pEthHdr->h_source[5]);
#endif
        }

	} while (0);

    if (bDropPktInfo)
    {
        os_frame_free(frame);
    }

	return data_ret;
}
#endif

