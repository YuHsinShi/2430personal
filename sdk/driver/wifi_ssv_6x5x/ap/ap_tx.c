/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#define __SFILE__ "ap_tx.c"

#include <ssv_ether.h>
#include <ssv_timer.h>
#include <host_apis.h>
#include <ssv_hal.h>

#include "ap_info.h"
#include "ap_tx.h"
#include "ap_sta_info.h"
#include "common/ieee802_11_defs.h"
#include <ieee80211.h>
#include <ssv_devinfo.h>

#if (AP_MODE_ENABLE == 1)
extern struct Host_cfg g_host_cfg;

extern struct task_info_st g_host_task_info[];
extern ssv_type_u8* ssv6xxx_host_tx_req_get_qos_ptr(struct cfg_host_txreq0 *req0);
extern ssv_type_u8* ssv6xxx_host_tx_req_get_data_ptr(struct cfg_host_txreq0 *req0);



#ifdef __AP_DEBUG__


void ap_check_queue(struct ssv_list_q *head)
{
	//unsigned int i;
	//struct ap_tx_desp *desp=(struct ap_tx_desp *)head;
	
	//for(i=0;i<head->qlen;i++)
	//{
	//	desp = desp->next;
	//}


	//if((struct ap_tx_desp *)desp->list.next!=(struct ap_tx_desp *)head)
	//	ASSERT(FALSE);

	if(head->qlen<=0)
		SSV_ASSERT(FALSE);

}

#endif//__AP_DEBUG__


void ap_print_queue_info(struct ssv_list_q *head)
{
	unsigned int i;
	struct ap_tx_desp *desp= NULL;
	struct ssv_list_q* _list = head->next;
	
	for(i=0;i<head->qlen;i++)
	{
		desp = (struct ap_tx_desp *)_list;
		_list = _list->next;
		LOG_PRINTF("Item[%d] Jiffies:%d ",i, desp->jiffies);		
	}
	LOG_PRINTF("\r\n");
    
	if( desp )
		SSV_ASSERT((struct ap_tx_desp *)desp->list.next==(struct ap_tx_desp *)head);
}












/* 802.1d to AC mapping. Refer pg 57 of WMM-test-plan-v1.2 */
const ssv_type_u8 up_to_ac[] = {
	IEEE80211_AC_BE,
	IEEE80211_AC_BK,
	IEEE80211_AC_BK,
	IEEE80211_AC_BE,
	IEEE80211_AC_VI,
	IEEE80211_AC_VI,
	IEEE80211_AC_VO,
	IEEE80211_AC_VO,
};






struct ap_tx_desp *ap_tx_desp_peek(const struct ssv_list_q *list_)
{
	struct ap_tx_desp *list = (struct ap_tx_desp *)list_->next;
	if (list == (struct ap_tx_desp *)list_)
		list = NULL;
	return list;
}

#if 0
void ap_tx_desp_head_init(struct list_q *list)
{
	list->prev = list->next = list;
	list->qlen = 0;
}


u32 ap_tx_desp_queue_len(const struct ap_tx_desp_head *list_)
{
	return list_->len;
}


struct ap_tx_desp *ap_tx_desp_dequeue(struct ap_tx_desp_head *list)
{
	
	struct ap_tx_desp *data = ((const struct ap_tx_desp *)list)->next;
	

#ifdef __AP_DEBUG__
	ap_check_queue(list);
#endif
	
	do 
	{		
		//Get data		
		if (data == (struct ap_tx_desp *)list)
		{
			data = NULL;
			break;
		}

		if(0 == list->len)
			ASSERT(FALSE);

		
		//Unlink
		{
			struct ap_tx_desp *next, *prev;
			list->len--;
			next	   = data->next;
			prev	   = data->prev;
			data->next  = data->prev = NULL;
			
			next->prev = prev;
			prev->next = next;
		}
	} while (0);

#ifdef __AP_DEBUG__
		ap_check_queue(list);
#endif



	
	return data;
}



static inline void ap_tx_desp_insert(struct ap_tx_desp *new,
struct ap_tx_desp *prev, struct ap_tx_desp *next,
struct ap_tx_desp_head *list)

{
	new->next = next;
	new->prev = prev;
	next->prev  = prev->next = new;
	list->len++;
}

void ap_tx_desp_queue_tail(struct ap_tx_desp_head *list, struct ap_tx_desp *new)
{
#ifdef __AP_DEBUG__
	ap_check_queue(list);
#endif

	ap_tx_desp_insert(new, (struct ap_tx_desp*)list->prev, (struct ap_tx_desp*)list, list);	

#ifdef __AP_DEBUG__
		ap_check_queue(list);
#endif
	
}

#endif







void ap_release_tx_desp(struct ap_tx_desp *tx_desp);

//--------------------------------------------------------------------------------------------------------
//Tx descriptor related function

void APTxDespInit()
{
	gDeviceInfo->APInfo->pApTxData = (struct ap_tx_desp *)OS_MemAlloc(sizeof(struct ap_tx_desp)*AP_TOTAL_MAX_TX_DSP);
	SSV_ASSERT(gDeviceInfo->APInfo->pApTxData);
	OS_MemSET(gDeviceInfo->APInfo->pApTxData, 0, sizeof(struct ap_tx_desp)*AP_TOTAL_MAX_TX_DSP);
}


struct ap_tx_desp* ap_get_tx_desp()
{
	int i;

	for (i=0;i<AP_TOTAL_MAX_TX_DSP;i++)
	{
		struct ap_tx_desp *tx_desp = &gDeviceInfo->APInfo->pApTxData[i];
		if (NULL == tx_desp->host_txreq0)
		{
			return tx_desp;
		}
	}

	return NULL;
	
}

void ap_release_tx_desp(struct ap_tx_desp *tx_desp)
{	
	OS_MemSET(tx_desp,0, sizeof(struct ap_tx_desp));
}

void ap_release_tx_desp_and_frame(struct ap_tx_desp *tx_desp)
{
	os_frame_free(tx_desp->frame);
	ap_release_tx_desp(tx_desp);	
}



//-----------------------------------

/* This function is called whenever the AP is about to exceed the maximum limit
 * of buffered frames for power saving STAs. This situation should not really
 * happen often during normal operation, so dropping the oldest buffered packet
 * from each queue should be OK to make some room for new frames. */
static void purge_old_ps_buffers()
{
	int ac = 0;
    ssv_type_u32 i;
    //int clean_buffer_count =0;
	struct ap_tx_desp *tx_desp = NULL;

	for(i=0;i<gMaxAID;i++ )
	{
		APStaInfo_st *pAPStaInfo = &gDeviceInfo->APInfo->StaConInfo[i];
        //clean_buffer_count =0;
		if( !test_sta_flag(pAPStaInfo, WLAN_STA_VALID) )
			continue;
					

    
        for (ac = IEEE80211_AC_BK; ac >= IEEE80211_AC_VO; ac--) {
            
             /*
                	 * Drop one frame from each station from the lowest-priority
                	 * AC that has frames at all.
                	 */
        	tx_desp =(struct ap_tx_desp *)list_q_deq_safe(&pAPStaInfo->ps_tx_buf[ac],&pAPStaInfo->ps_tx_buf_mutex);
    		if (tx_desp) {
    			ap_release_tx_desp_and_frame(tx_desp);
    			gDeviceInfo->APInfo->total_ps_buffered--;
    			break;
    		}
            
     
		}
	}
	
}

void purge_sta_ps_buffers(APStaInfo_st *pAPStaInfo)
{
	int  ac = 0;
	struct ap_tx_desp *tx_desp = NULL;
					
	/*
	 * Drop frames from each station
	 */
	for (ac = IEEE80211_AC_BK; ac >= IEEE80211_AC_VO; ac--) 
	{		
		while ((tx_desp = (struct ap_tx_desp *)list_q_deq_safe(&pAPStaInfo->ps_tx_buf[ac],&pAPStaInfo->ps_tx_buf_mutex)) != NULL)
            
		{
			ap_release_tx_desp_and_frame(tx_desp);			
            OS_MutexLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
			gDeviceInfo->APInfo->total_ps_buffered--;
            OS_MutexUnLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
		}		
	}	
}

#if 0

void purge_all_bc_buffers()
{	
	struct ap_tx_desp *tx_desp = NULL;
					
	/*
	 * Drop All BC frames 
	 */	
	while( tx_desp = (struct ap_tx_desp *)list_q_deq_safe(&gDeviceInfo->APInfo->ps_bc_buf,&gDeviceInfo->APInfo->ps_bc_buf_mutex) )
	{
		ap_release_tx_desp_and_frame(tx_desp);	
        OS_MutexLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
		gDeviceInfo->APInfo->total_ps_buffered--;	
        OS_MutexUnLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
	}		
		
}


#endif


//---------------------
extern void sta_info_cleanup(void *data1, void *data2);
extern ssv_type_bool sta_info_recalc_tim(APStaInfo_st *sta);


tx_result tx_h_unicast_ps_buf(struct ap_tx_desp *tx_desp)
{
	APStaInfo_st *sta = tx_desp->sta;	
    tx_result res = TX_CONTINUE;
    ssv_type_bool needtimer = FALSE;
    int ac = 0;

	/* only deauth, disassoc and action are bufferable MMPDUs */
 	if (!sta)
		return res;
    
    ac = up_to_ac[tx_desp->priority];

    OS_MutexLock(sta->apsta_mutex);
    if ( ((test_sta_flag(sta, WLAN_STA_PS_STA)) || (list_q_len_safe(&sta->ps_tx_buf[ac],&sta->ps_tx_buf_mutex) != 0)) && !(tx_desp->flags & AP_TX_NATIVE_AP_FRAME))
    {					
 		needtimer = TRUE;
        OS_MutexLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
		if (gDeviceInfo->APInfo->total_ps_buffered >= AP_TOTAL_MAX_TX_BUFFER)    
            purge_old_ps_buffers();
        OS_MutexUnLock(gDeviceInfo->APInfo->ap_info_ps_mutex);

    
		if (list_q_len_safe(&sta->ps_tx_buf[ac],&sta->ps_tx_buf_mutex) >= AP_STA_MAX_TX_BUFFER) {
			struct ap_tx_desp *old = (struct ap_tx_desp *)list_q_deq_safe(&sta->ps_tx_buf[ac],&sta->ps_tx_buf_mutex);			
			ap_release_tx_desp_and_frame(old);
		} 
        else
        {
            OS_MutexLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
			gDeviceInfo->APInfo->total_ps_buffered++;
            OS_MutexUnLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
        }
		tx_desp->jiffies= OS_GetSysTick();
		
		list_q_qtail_safe(&sta->ps_tx_buf[ac], (struct ssv_list_q *)tx_desp,&sta->ps_tx_buf_mutex);

		/*
		* We queued up some frames, so the TIM bit might
		* need to be set, recalculate it.
		*/
		if(sta->fw_q_len == UNKNOWN_FW_QUEUE_LENGTH)
		    sta->fw_q_len = 1;
        else
            sta->fw_q_len ++;
        
		OS_MutexUnLock(sta->apsta_mutex);


        //#if(AP_MODE_BEACON_VIRT_BMAP_0XFF == 0)		
        if(!g_host_cfg.AP_TimAllOne)
        {
            if(sta_info_recalc_tim(sta))
            {
                //Send one data frame to FW.
                sta_ps_deliver_poll_response(sta);

                #if(MLME_TASK ==1)
            	ap_mlme_handler(NULL,MEVT_BCN_CMD);
                #else
            	GenBeacon();
                #endif
            }
        }
        //#endif
        
        #ifdef __AP_DEBUG__ 

		LOG_TRACE("STA %x:%x:%x:%x:%x:%x in PS mode, Queue Frame in AC[%d]\r\n",
			sta->addr[0],sta->addr[1],sta->addr[2],sta->addr[3],sta->addr[4],sta->addr[5], ac);
		#endif
        res = TX_QUEUED;

	}
    else
    {
        OS_MutexUnLock(sta->apsta_mutex);
    }
       


    OS_MutexLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
    if(needtimer&&!gDeviceInfo->APInfo->sta_pkt_cln_timer)
    {
        gDeviceInfo->APInfo->sta_pkt_cln_timer = TRUE;
        OS_MutexUnLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
        os_create_timer(AP_STA_INFO_CLEANUP_INTERVAL, sta_info_cleanup, NULL, NULL, (void*)TIMEOUT_TASK); 
    }
    else
    {
        OS_MutexUnLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
    }


	return res;	
}

tx_result tx_h_multicast_ps_buf(struct ap_tx_desp *tx_desp)
{
#if 1
    //Skip multicast packet, send it
    return TX_CONTINUE;

#else

    //if any one station is in ps, need to queue it.	
	if (0 == gDeviceInfo->APInfo->num_sta_ps)
		return TX_CONTINUE;
	
	if (gDeviceInfo->APInfo->total_ps_buffered >= AP_TOTAL_MAX_TX_BUFFER)
 		purge_old_ps_buffers();

	if (ap_tx_desp_queue_len(&gDeviceInfo->APInfo->ps_bc_buf) >= AP_MAX_BC_BUFFER) 
		ap_release_tx_desp_and_frame(ap_tx_desp_dequeue(&gDeviceInfo->APInfo->ps_bc_buf));	
	else
		gDeviceInfo->APInfo->total_ps_buffered++;
	
	ap_tx_desp_queue_tail(&gDeviceInfo->APInfo->ps_bc_buf, tx_desp);

	return TX_QUEUED;	
#endif
}



tx_result ssv6xxx_data_need_queue(struct ap_tx_desp *tx_desp)
{	
	if (tx_desp->flags & AP_TX_POLL_RESPONSE || tx_desp->flags & AP_TX_NATIVE_AP_FRAME)
		return TX_CONTINUE;
	
	if (tx_desp->flags & AP_TX_UNICAST)
		return tx_h_unicast_ps_buf(tx_desp);
	else
		return tx_h_multicast_ps_buf(tx_desp);

	return TX_CONTINUE;	
}


tx_result ssv6xxx_data_check_authed(struct ap_tx_desp *tx_desp)
{
	//ieee80211_tx_result ret = TX_CONTINUE;
	ssv_type_bool bAuthed = false;
	
	if (tx_desp->sta)
			bAuthed = test_sta_flag(tx_desp->sta, WLAN_STA_AUTHORIZED);


	if (tx_desp->flags & AP_TX_UNICAST)
	{
        if(tx_desp->flags & AP_TX_NATIVE_AP_FRAME)
            return TX_CONTINUE;
		//Unicast
		if (!bAuthed && (tx_desp->flags & AP_TX_DATA)) 		
		    return TX_DROP;		
	} 
	else if (
	(tx_desp->flags & AP_TX_DATA) && 
			    !gDeviceInfo->APInfo->num_sta_authorized) 
	{		

		/*
		 * No associated STAs - no need to send multicast
		 * frames.
		 */

		LOG_TRACE("%s No Authorized Sta. Drop multicat frame.\r\n",__func__);
		return TX_DROP;

	}

	return TX_CONTINUE;
}

void ssv6xxx_data_prepare_tx_data(struct ap_tx_desp *tx_desp, ssv_type_u32 *frame, CFG_HOST_TXREQ *host_txreq0, ssv_type_bool bAPFrame, ssv_type_u32 TxFlags)
{
	//int nHeaderLen;
//	u8 *dat = (u8*)host_txreq0;	
    ETHER_ADDR *pDaMAC = NULL; 
    APStaInfo_st *pStaInfo = NULL;
    ssv_type_u8 rsvd0=0;
    OS_MemSET(tx_desp, 0, sizeof(struct ap_tx_desp *));


    tx_desp->flags |= TxFlags;


    if(bAPFrame)
        tx_desp->flags |= AP_TX_NATIVE_AP_FRAME;

    rsvd0=ssv_hal_get_txreq0_rsvd0((CFG_HOST_TXREQ *)host_txreq0);
    if(rsvd0&AP_PS_FRAME)
    {
        tx_desp->flags |= AP_TX_POLL_RESPONSE;        
        ssv_hal_set_txreq0_rsvd0((CFG_HOST_TXREQ *)host_txreq0,0);
    }
	


    tx_desp->frame = frame;
    tx_desp->host_txreq0 = host_txreq0;
    tx_desp->data = ssv6xxx_host_tx_req_get_data_ptr(host_txreq0);
    tx_desp->nDataLen = (ssv_type_u16)ssv_hal_get_txreq0_len(host_txreq0)-(tx_desp->data-(ssv_type_u8*)host_txreq0);
	

    if(1==ssv_hal_get_txreq0_f80211((CFG_HOST_TXREQ *)host_txreq0))
    {
        //80211 frame
        struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)tx_desp->data;
        pDaMAC = (ETHER_ADDR *) (ieee80211_get_DA((struct ieee80211_hdr_4addr *)tx_desp->data));
        (ieee80211_is_data(hdr->frame_control)) ? (tx_desp->flags |= AP_TX_DATA):(tx_desp->flags &= ~AP_TX_DATA);
		
    }
    else
    {
        //ether frame
        ethhdr *pDaEthHdr =  (ethhdr *)tx_desp->data;
        pDaMAC = (ETHER_ADDR *)&pDaEthHdr->h_dest;		
        tx_desp->flags |= AP_TX_DATA;
    }
		
    pStaInfo = APStaInfo_FindStaByAddr(pDaMAC); 	
    tx_desp->sta = pStaInfo;


    if (ssv_is_multicast_ether_addr((const ssv_type_u8*)pDaMAC)) {
        tx_desp->flags &= ~AP_TX_UNICAST;		
    } else
        tx_desp->flags |= AP_TX_UNICAST;

	
    {
        ssv_type_u16 *qoshdr = (ssv_type_u16*)ssv6xxx_host_tx_req_get_qos_ptr(host_txreq0);

        if(qoshdr)
            tx_desp->priority = (*qoshdr&IEEE80211_QOS_CTL_TAG1D_MASK);
        else
            tx_desp->priority = 0;
    }

	

	
	
}



tx_result ssv6xxx_data_could_be_send(void *frame, ssv_type_bool bAPFrame, ssv_type_u32 TxFlags)
{
	//ssv_type_bool bRet = FALSE;
	tx_result res = TX_DROP;
	struct cfg_host_txreq0 *host_txreq0 = (struct cfg_host_txreq0 *)OS_FRAME_GET_DATA(frame);	
	struct ap_tx_desp *tx_desp = NULL;

	#define CALL_TXH(txh) \
		res = txh(tx_desp);		\
		if (res != TX_CONTINUE)	\
			break

		


	do 
	{
        OS_MutexLock(gDeviceInfo->APInfo->ap_info_ps_mutex);

		tx_desp = ap_get_tx_desp();
		SSV_ASSERT(NULL != tx_desp);
        ssv6xxx_data_prepare_tx_data(tx_desp, frame, (CFG_HOST_TXREQ *)host_txreq0, bAPFrame, TxFlags);

        OS_MutexUnLock(gDeviceInfo->APInfo->ap_info_ps_mutex);

		CALL_TXH(ssv6xxx_data_check_authed);
		CALL_TXH(ssv6xxx_data_need_queue);
		
	} while (0);

	if (TX_DROP == res || TX_CONTINUE == res)				
		ap_release_tx_desp(tx_desp);//Just need memory set description, frame will be sent or drop by parent function.
	else
	{;}


	return res;
}


#if 0

struct ap_tx_desp *
ap_get_buffered_bc(ApInfo_st *pAPInfo)
{

	struct ap_tx_desp *tx_desp = NULL;
	//struct cfg_host_txreq0 *txreq0 = NULL; 
	//sdata = vif_to_sdata(vif);
	//bss = &sdata->u.ap;

	//rcu_read_lock();
	//beacon = rcu_dereference(bss->beacon);

	//if (sdata->vif.type != NL80211_IFTYPE_AP || !beacon || !beacon->head)
	//	goto out;

	if (pAPInfo->dtim_count != 0 || !pAPInfo->dtim_bc_mc)
		goto out; /* send buffered bc/mc only after DTIM beacon */

	//while (1) 
	{
		tx_desp =(struct ap_tx_desp *)list_q_deq_safe(&pAPInfo->ps_bc_buf,&pAPInfo->ps_bc_buf_mutex);

		if (!tx_desp)
			goto out;
        OS_MutexLock(gDeviceInfo->APInfo->ap_info_ps_mutex);
		pAPInfo->total_ps_buffered--;
        OS_MutexUnLock(gDeviceInfo->APInfo->ap_info_ps_mutex);

		if (!tx_desp && tx_desp->nDataLen >= 2) {
			struct ieee80211_hdr *hdr =
				(struct ieee80211_hdr *) tx_desp->data;
			/* more buffered multicast/broadcast frames ==> set
			 * MoreData flag in IEEE 802.11 header to inform PS
			 * STAs */
			hdr->frame_control |=
				cpu_to_le16(IEEE80211_FCTL_MOREDATA);
		}

		//if (!ieee80211_tx_prepare(sdata, &tx, skb))
		//	break;
		//dev_kfree_skb_any(skb);
	}

	//info = IEEE80211_SKB_CB(skb);

	//tx.flags |= IEEE80211_TX_PS_BUFFERED;
	//tx.channel = local->hw.conf.channel;
	//info->band = tx.channel->band;

	//if (invoke_tx_handlers(&tx))
	//	skb = NULL;

	//txreq0 = tx_desp->host_txreq0;
	//-------------------------------------------
	//tx_desp->data = NULL;
	//ap_release_tx_desp(tx_desp);
	
 out:

	return tx_desp;
}


//TO DO-------------------------------------------------------------------------

#pragma message("===================================================")
#pragma message("Todo Need to discuss flow....")
#pragma message("===================================================")


void ap_tx_status(ApInfo_st *pAPInfo, const u8 *addr,
		      /* const u8 *buf, size_t len, */int ack)
{
	APStaInfo  *sta;


	sta=STAInfo_FindSTAByAddr((ETHER_ADDR *)addr);
	
	if (sta == NULL || !test_sta_flag(sta, WLAN_STA_ASSOC))
		return;
	if (sta->flags & WLAN_STA_PENDING_POLL) {
		LOG_DEBUG("STA " MACSTR " %s pending "
			   "activity poll", MAC2STR(sta->addr),
			   ack ? "ACKed" : "did not ACK");
		if (ack)
			sta->flags &= ~WLAN_STA_PENDING_POLL;
	}

//	ieee802_1x_tx_status(hapd, sta, buf, len, ack);
}




extern s32 ssv6xxx_drv_send(void *dat, size_t len);
void ap_tx_dtimexpiry_event()
{	
	
	{	
		struct ap_tx_desp *tx_desp = NULL; 
		tx_desp = ap_get_buffered_bc(gDeviceInfo->APInfo);
	
		while (tx_desp) {
	
			//ssv6xxx_drv_send(tx_desp->host_txreq0, tx_desp->host_txreq0->len);
			ssv6xxx_drv_send(OS_FRAME_GET_DATA(tx_desp->frame), OS_FRAME_GET_DATA_LEN(tx_desp->frame));	//??
	
			//Todo:  need to check if queue have size to queue
	
			//ath5k_tx_queue(ah->hw, skb, ah->cabq);
			//if (ah->cabq->txq_len >= ah->cabq->txq_max)
			//	break;
	
			//tx_desp->data = NULL;
			ap_release_tx_desp(tx_desp);
	
			tx_desp = ap_get_buffered_bc(gDeviceInfo->APInfo);
		}
	}



	
}

#endif
#endif



