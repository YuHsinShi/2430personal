/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#define __SFILE__ "host_cmd_engine_rx.c"

#include <log.h>
#include <ssv_types.h>
#include <hdr80211.h>

#include <host_apis.h>
#include "host_cmd_engine.h"
#include "host_cmd_engine_priv.h"


#if (AP_MODE_ENABLE == 1)        
#include <ap/ap.h>
#include <ap_info.h>
#endif
#include <ssv_hal.h>

#include <pbuf.h>
#include <msgevt.h>
#include <cmd_def.h>
#include <ssv_devinfo.h>
#define EVT_PREPROCESS


extern vif_state get_current_vif_state(void);
extern void AP_RxHandleEvent(struct cfg_host_event *pHostEvt);
//extern void pendingcmd_expired_handler(void* data1, void* data2);
ssv_type_s32 leave_host_event_handler_(struct cfg_host_event *pPktInfo);
ssv_type_s32 join_host_event_handler_(struct cfg_host_event *pPktInfo);
ssv_type_s32 scan_host_event_handler_(struct cfg_host_event *pPktInfo);
ssv_type_s32 scan_done_host_event_handler_(struct cfg_host_event *pPktInfo);
void exec_host_evt_cb(ssv_type_u32 nEvtId, void *data, ssv_type_s32 len);

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//																						Rx Event
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


typedef ssv_type_s32 (*CmdEng_RxEvtHandler)(struct cfg_host_event *pPktInfo);


ssv_type_s32 CmdEng_evt_drop(struct cfg_host_event *pPktInfo)
{
	ssv6xxx_result ret = SSV6XXX_SUCCESS;

	LOG_TRACE("Drop Host Evt\n");

	return ret;
}

ssv_type_s32 CmdEng_evt_get_reg_handle(struct cfg_host_event *pEvent)
{

    LOG_PRINTF("%s(): len=%d, val=0x%08x\r\n",
        __FUNCTION__, pEvent->len, *(ssv_type_u32 *)pEvent->dat);

    return SSV6XXX_SUCCESS;
}

#ifdef EVT_PREPROCESS
ssv_type_s32 CmdEng_evt_test_fn(struct cfg_host_event *pEvent)
{

    LOG_PRINTF("[CmdEng]: evtid= %d , len=%d, val=0x%08x\r\n" , pEvent->h_event ,pEvent->len, *(ssv_type_u32 *)pEvent->dat);

    return SSV6XXX_FAILED;
}
#endif
//
// s32 CmdEng_evt_pspoll(struct cfg_host_event *pPktInfo)
// {
// 	//ssv6xxx_result ret = SSV6XXX_SUCCESS;
// 	if (SSV6XXX_HWM_AP==gHCmdEngInfo->hw_mode)
// 		AP_RxHandleEvent(pPktInfo);
// 	else
// 		FREEIF(pPktInfo);
//
// 	return SSV6XXX_SUCCESS;
// }
//
//
// s32 CmdEng_evt_nulldata(struct cfg_host_event *pPktInfo)
// {
// 	if (SSV6XXX_HWM_AP==gHCmdEngInfo->hw_mode)
// 		AP_RxHandleEvent(pPktInfo);
// 	else
// 		FREEIF(pPktInfo);
//
// 	return SSV6XXX_SUCCESS;
// }


/*HOST_EVT_SCAN_RESULT*/





const CmdEng_RxEvtHandler PrivateRxEvtHandler[] =
{
	CmdEng_evt_drop,		//SOC_EVT_CONFIG_HW_RESP
	NULL,					//SOC_EVT_SET_BSS_PARAM_RESP
	NULL,	                //SOC_EVT_PS_POLL      ,handle on rx task
	NULL,	                //SOC_EVT_NULL_DATA ,handle on rx task
	NULL,					//SOC_EVT_REG_RESULT
};

struct evt_prefn
{
    ssv6xxx_soc_event evt;
    CmdEng_RxEvtHandler pre_fn;
};

const struct evt_prefn PreFnTbl[] =
{
#if (MLME_TASK==1)
    { SOC_EVT_SCAN_RESULT, mlme_host_event_handler_},
#else
    { SOC_EVT_SCAN_RESULT, scan_host_event_handler_},
#endif
    { SOC_EVT_JOIN_RESULT, join_host_event_handler_},
    { SOC_EVT_SCAN_DONE, scan_done_host_event_handler_},
    { SOC_EVT_LEAVE_RESULT,leave_host_event_handler_},
};
extern void sta_mode_ap_list_handler (void *data,struct ssv6xxx_ieee80211_bss *ap_list);
extern struct Host_cfg g_host_cfg;
ssv_type_s32 scan_host_event_handler_(struct cfg_host_event *pPktInfo)
{
	void* pPktData = NULL;
    if(pPktInfo==NULL)
    {
         LOG_PRINTF("Error %s @line %d free null pointer\r\n",__FUNCTION__,__LINE__);
         return 0;
    }
	pPktData = (void*)((ssv_type_u32)(pPktInfo->dat));
	sta_mode_ap_list_handler(pPktData,gDeviceInfo->ap_list);

    if(g_host_cfg.info_scan_result)
        exec_host_evt_cb(pPktInfo->h_event,pPktInfo->dat,pPktInfo->len-sizeof(HDR_HostEvent));
	return 0;
}

extern OsSemaphore scanning_sphr;
ssv_type_s32 scan_done_host_event_handler_(struct cfg_host_event *pPktInfo)
{
    struct resp_evt_result *scan_done = (struct resp_evt_result *)pPktInfo->dat;
    int i=0;
    //#if(ENABLE_DYNAMIC_RX_SENSITIVE==1)
    //ssv_hal_recover_phy_cca_bits(); //Only use this function on SmartConfig mode
    //#endif
#if (AP_MODE_ENABLE == 1)        
    if(scan_done->u.scan_done.result_code==0){

        for(i=0;i<MAX_2G_CHANNEL_NUM;i++){
            gDeviceInfo->APInfo->channel_edcca_count[i]+=scan_done->u.scan_done.channel_edcca_count[i];
            gDeviceInfo->APInfo->channel_packet_count[i]+=scan_done->u.scan_done.channel_packet_count[i];
            //LOG_PRINTF("2G Band Channel %d: edcca count is %d, packet count is %d \r\n",i,scan_done->u.scan_done.channel_edcca_count[i],scan_done->u.scan_done.channel_packet_count[i]);
        }
        
        for(i=0;i<MAX_5G_CHANNEL_NUM;i++){
            gDeviceInfo->APInfo->channel_5g_edcca_count[i]+=scan_done->u.scan_done.channel_5g_edcca_count[i];
            gDeviceInfo->APInfo->channel_5g_packet_count[i]+=scan_done->u.scan_done.channel_5g_packet_count[i];
            //LOG_PRINTF("5G Band Channel %d: edcca count is %d, packet count is %d \r\n",i,scan_done->u.scan_done.channel_5g_edcca_count[i],scan_done->u.scan_done.channel_5g_packet_count[i]);
        }

    }

    if(scanning_sphr!=0){
        OS_SemSignal(scanning_sphr);
    }

#endif
    if(get_current_vif_state()==VIF_ALL_DEACTIVE)
        ssv_hal_sta_reject_bcn();
    else
        ssv_hal_sta_rcv_specific_bcn();

    //This function is not only used in STA mode but also in AP mode, because of auto channel selection
    //_ssv6xxx_wifi_auto_channel_selection is a blocking function, it blocks the netmgr_task when netmgr_task want to do the auto channel selection.
    //Users can configure that how many time of channel scannings in one auto channel selection,
    //if users decide to do 10 channel scanning in one auto channel selection,we will send 10 msg events to netmgr_task,
    // and netmgr_task can't free the  msg in a short time, it will cause the msg event allocat fail
#if (AP_MODE_ENABLE == 1)        
    //if(gDeviceInfo->hw_mode != SSV6XXX_HWM_STA){
    if((gDeviceInfo->APInfo)&&(gDeviceInfo->APInfo->acs_start == TRUE))
    {
        return 0;
    }
#endif

    exec_host_evt_cb(pPktInfo->h_event,pPktInfo->dat,pPktInfo->len-sizeof(HDR_HostEvent));
    return 0;
}
extern struct rx_ba_session_desc g_ba_rx_session_desc[RX_AGG_RX_BA_MAX_STATION][RX_AGG_RX_BA_MAX_SESSIONS];
extern struct task_info_st g_host_task_info[];
extern void timer_sta_reorder_release(void* data1, void* data2);
extern ssv_type_bool mlme_remove_ap(struct ssv6xxx_ieee80211_bss *bss);
extern void ieee80211_delete_ampdu_rx(ssv_type_u8 wsid);
ssv_type_s32 leave_host_event_handler_(struct cfg_host_event *pPktInfo)
{
    struct resp_evt_result *leave_res = NULL;    
    ssv_vif* vif =NULL;
    leave_res = (struct resp_evt_result *)pPktInfo->dat;
    
    LOG_PRINTF("leave_res->u.leave.bssid_idx=%d\r\n",leave_res->u.leave.bssid_idx);
    if(leave_res->u.leave.bssid_idx<MAX_VIF_NUM)
    {
        vif = &gDeviceInfo->vif[leave_res->u.leave.bssid_idx];
    }

    if((vif)&&(vif->hw_mode == SSV6XXX_HWM_STA))
    {
        //clean sta mode global variable
        if(leave_res->u.leave.reason_code!=0){ // leave inactive
            mlme_remove_ap(&vif->m_info.StaInfo->joincfg->bss);
        }
        #if(ENABLE_DYNAMIC_RX_SENSITIVE==0)
        OS_MutexLock(gDeviceInfo->g_dev_info_mutex);
        vif->m_info.StaInfo->status= DISCONNECT;
        OS_MemSET(vif->m_info.StaInfo->joincfg, 0, sizeof(struct cfg_join_request));
        if(get_current_vif_state()==VIF_ALL_DEACTIVE)
            ssv_hal_sta_reject_bcn();
        OS_MutexUnLock(gDeviceInfo->g_dev_info_mutex);
        #else
        OS_MutexLock(gDeviceInfo->g_dev_info_mutex);
        if(get_current_vif_state()==VIF_ALL_DEACTIVE)
            ssv_hal_sta_reject_bcn();
        OS_MutexUnLock(gDeviceInfo->g_dev_info_mutex);
        ssv6xxx_sta_mode_disconnect((void*)vif->m_info.StaInfo);
        #endif

        if(g_host_cfg.ampdu_rx_enable)
        {
            ieee80211_delete_ampdu_rx(vif->m_info.StaInfo->wsid);
        }
        
        gDeviceInfo->bMCC = FALSE;
        exec_host_evt_cb(pPktInfo->h_event,pPktInfo->dat,pPktInfo->len-sizeof(HDR_HostEvent));
    }else{
        if(vif)
            LOG_PRINTF("mode=%d not SSV6XXX_HWM_STA\r\n",vif->hw_mode);
    }
    return 0;
}
extern void checkNsetup_mcc(ssv_type_u8 vif_idx);
ssv_type_s32 join_host_event_handler_(struct cfg_host_event *pPktInfo)
{
    struct resp_evt_result *join_res = NULL;
#if 1
    struct resp_evt_result *leave_res = NULL;
#else
    struct cfg_leave_request LeaveReq;
#endif
    ssv_type_bool pass_to_up_layer=TRUE;
    join_res = (struct resp_evt_result *)pPktInfo->dat;
    LOG_DEBUGF(LOG_CMDENG,("join_host_event_handler,vif_idx=%d,status_code=%d,wsid=%d\r\n",
        join_res->u.join.bssid_idx,join_res->u.join.status_code,join_res->u.join.wsid));
    if(gDeviceInfo->recovering==TRUE){
        if (join_res->u.join.status_code != 0){
            mlme_remove_ap(&gDeviceInfo->vif[join_res->u.join.bssid_idx].m_info.StaInfo->joincfg->bss);
            #if(ENABLE_DYNAMIC_RX_SENSITIVE==0)
            OS_MutexLock(gDeviceInfo->g_dev_info_mutex);
            gDeviceInfo->vif[join_res->u.join.bssid_idx].m_info.StaInfo->status = DISCONNECT;
            if(get_current_vif_state()==VIF_ALL_DEACTIVE)
                ssv_hal_sta_reject_bcn();
            OS_MemSET(gDeviceInfo->vif[join_res->u.join.bssid_idx].m_info.StaInfo->joincfg, 0, sizeof(struct cfg_join_request));
            OS_MutexUnLock(gDeviceInfo->g_dev_info_mutex);
            #else
            OS_MutexLock(gDeviceInfo->g_dev_info_mutex);
            if(get_current_vif_state()==VIF_ALL_DEACTIVE)
                ssv_hal_sta_reject_bcn();
            OS_MutexUnLock(gDeviceInfo->g_dev_info_mutex);
            ssv6xxx_sta_mode_disconnect((void*)gDeviceInfo->vif[join_res->u.join.bssid_idx].m_info.StaInfo);
            #endif
#if 1
            //Transform JOIN_RESULT_EVT to LEAVE_EVT because we mulst info the netmgr that STA disconnect with AP.
            leave_res=(struct resp_evt_result *)pPktInfo->dat;
            leave_res->u.leave.reason_code=4;
            pPktInfo->h_event=SOC_EVT_LEAVE_RESULT;
            pPktInfo->len=sizeof(HDR_HostEvent)+LEAVE_RESP_EVT_HEADER_SIZE;
#else
            LeaveReq.reason = 1;
            ssv6xxx_wifi_leave(&LeaveReq);
            pass_to_up_layer=FALSE;
#endif
        }
        else{
            //Connect sucessulfy with AP, and we don't need to info the netmgr
            #if(ENABLE_DYNAMIC_RX_SENSITIVE==0)
            OS_MutexLock(gDeviceInfo->g_dev_info_mutex);
            gDeviceInfo->vif[join_res->u.join.bssid_idx].m_info.StaInfo->status = CONNECT;
            ssv_hal_sta_rcv_specific_bcn();
            OS_MutexUnLock(gDeviceInfo->g_dev_info_mutex);
            #else
            OS_MutexLock(gDeviceInfo->g_dev_info_mutex);
            ssv_hal_sta_rcv_specific_bcn();
            OS_MutexUnLock(gDeviceInfo->g_dev_info_mutex);
            ssv6xxx_sta_mode_connect((void*)gDeviceInfo->vif[join_res->u.join.bssid_idx].m_info.StaInfo);
            #endif

            pass_to_up_layer=FALSE;
            gDeviceInfo->vif[join_res->u.join.bssid_idx].m_info.StaInfo->wsid = join_res->u.join.wsid;
            gDeviceInfo->vif[join_res->u.join.bssid_idx].freq.sec_channel_offset = join_res->u.join.ch_offset;            
            checkNsetup_mcc(join_res->u.join.bssid_idx);
        }
        gDeviceInfo->recovering=FALSE;
    }else{
        if (join_res->u.join.status_code != 0){
            #if(ENABLE_DYNAMIC_RX_SENSITIVE==0)
            OS_MutexLock(gDeviceInfo->g_dev_info_mutex);
            gDeviceInfo->vif[join_res->u.join.bssid_idx].m_info.StaInfo->status = DISCONNECT;
            if(get_current_vif_state()==VIF_ALL_DEACTIVE)
                ssv_hal_sta_reject_bcn();
            OS_MutexUnLock(gDeviceInfo->g_dev_info_mutex);
            #else
            OS_MutexLock(gDeviceInfo->g_dev_info_mutex);
            if(get_current_vif_state()==VIF_ALL_DEACTIVE)
                ssv_hal_sta_reject_bcn();
            OS_MutexUnLock(gDeviceInfo->g_dev_info_mutex);
            ssv6xxx_sta_mode_disconnect((void*)gDeviceInfo->vif[join_res->u.join.bssid_idx].m_info.StaInfo);
            #endif

        }
        else{
            #if(ENABLE_DYNAMIC_RX_SENSITIVE==0)
            OS_MutexLock(gDeviceInfo->g_dev_info_mutex);
            gDeviceInfo->vif[join_res->u.join.bssid_idx].m_info.StaInfo->status = CONNECT;
            ssv_hal_sta_rcv_specific_bcn();
            OS_MemCPY(gDeviceInfo->vif[join_res->u.join.bssid_idx].m_info.StaInfo->joincfg, 
                   gDeviceInfo->vif[join_res->u.join.bssid_idx].m_info.StaInfo->joincfg_backup, 
                   sizeof(struct cfg_join_request));
            OS_MutexUnLock(gDeviceInfo->g_dev_info_mutex);
            #else
            OS_MutexLock(gDeviceInfo->g_dev_info_mutex);
            ssv_hal_sta_rcv_specific_bcn();
            OS_MutexUnLock(gDeviceInfo->g_dev_info_mutex);
            ssv6xxx_sta_mode_connect((void*)gDeviceInfo->vif[join_res->u.join.bssid_idx].m_info.StaInfo);
            #endif
            gDeviceInfo->vif[join_res->u.join.bssid_idx].m_info.StaInfo->wsid = join_res->u.join.wsid;
            gDeviceInfo->vif[join_res->u.join.bssid_idx].freq.sec_channel_offset = join_res->u.join.ch_offset;
            checkNsetup_mcc(join_res->u.join.bssid_idx);
        }
    }

    if(gDeviceInfo->vif[join_res->u.join.bssid_idx].m_info.StaInfo->joincfg->bss.wmm_used)
        SET_TXREQ_WITH_QOS(gDeviceInfo, 1);
    else
        SET_TXREQ_WITH_QOS(gDeviceInfo, 0);

    if(pass_to_up_layer==TRUE){
        exec_host_evt_cb(pPktInfo->h_event,pPktInfo->dat,pPktInfo->len-sizeof(HDR_HostEvent));
    }
	return 0;
}

void exec_host_evt_cb(ssv_type_u32 nEvtId, void *data, ssv_type_s32 len)
{
	ssv_type_u32 i;
	for (i=0;i<HOST_EVT_CB_NUM;i++)
	{
		evt_handler handler = gDeviceInfo->evt_cb[i];
        cmdeng_task_st =__LINE__;
		if (handler)
			handler(nEvtId, data, len, gDeviceInfo->evt_cb_priv[i]);
    }
}

void CmdEng_RxHdlEvent(void *frame)
{
    struct cfg_host_event *pPktInfo = (struct cfg_host_event *)OS_FRAME_GET_DATA(frame);

#ifndef CONFIG_NO_WPA2
    struct resp_evt_result *res = NULL;
#endif
#ifdef EVT_PREPROCESS
	int i;
    ssv_type_s32 PreFnTblSize = sizeof(PreFnTbl)/sizeof(struct evt_prefn);
#endif

    cmdeng_task_st =__LINE__;
	if (pPktInfo->h_event >= SOC_EVT_PRIVE_CMD_START)
    {       
        cmdeng_task_st =__LINE__;
		PrivateRxEvtHandler[pPktInfo->h_event-SOC_EVT_PRIVE_CMD_START](pPktInfo);
    }
	else
	{
		//Customer event need to free packet info
		/*if(pPktInfo->h_event==HOST_EVT_HW_MODE_RESP)
		{

		}*/
        cmdeng_task_st =__LINE__;
        OS_MutexLock(gHCmdEngInfo->CmdEng_mtx);
        if ((gHCmdEngInfo->blockcmd_in_q == true) && (pPktInfo->h_event == SOC_EVT_MLME_CMD_DONE) && (pPktInfo->evt_seq_no == gHCmdEngInfo->pending_cmd_seqno))
        {
            LOG_DEBUGF(LOG_CMDENG, ("[CmdEng]: Got SOC_EVT_MLME_CMD_DONE %d for block cmd\r\n", gHCmdEngInfo->pending_cmd_seqno));
            gHCmdEngInfo->blockcmd_in_q = false;
            gHCmdEngInfo->pending_cmd_seqno = 0;
            //os_cancel_timer(pendingcmd_expired_handler, gHCmdEngInfo, NULL);
        }
        OS_MutexUnLock(gHCmdEngInfo->CmdEng_mtx);

        if (pPktInfo->h_event == SOC_EVT_MLME_CMD_DONE)
        {
            os_frame_free(frame);
            return;
        }
        //handle station mode fw event
        cmdeng_task_st =__LINE__;

        switch (pPktInfo->h_event) {
#if (AP_MODE_ENABLE == 1)
#ifndef CONFIG_NO_WPA2
        case SOC_EVT_GET_PMK:
            res = (struct resp_evt_result *)pPktInfo->dat;
            enable_beacon(res);
            LOG_PRINTF("SOC_EVT_GET_PMK\r\n");
			goto eventdone;
        case SOC_EVT_ADD_STA_DONE:
            res = (struct resp_evt_result *)pPktInfo->dat;
            cmdeng_task_st =__LINE__;
            if(gDeviceInfo->APInfo->ap_conf.wpa_key_mgmt == WPA_KEY_MGMT_PSK)
                start_4way_handshake(res);
            goto eventdone;
#endif
#endif
         default:
            break;
        }

#ifdef EVT_PREPROCESS

        for(i=0; i< PreFnTblSize;i++)
        {
            if(pPktInfo->h_event == PreFnTbl[i].evt)
            {
                cmdeng_task_st =__LINE__;
                if(PreFnTbl[i].pre_fn(pPktInfo) == SSV6XXX_SUCCESS)
                {
                    os_frame_free(frame);
                    return;
                }
            }
        }
#endif
        cmdeng_task_st =__LINE__;
        exec_host_evt_cb(pPktInfo->h_event,pPktInfo->dat,pPktInfo->len-sizeof(HDR_HostEvent));
	}
eventdone:
	os_frame_free(frame);
}


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//																						Rx Data
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

ssv_type_u16 CmdEng_GetRawRxDataOffset(CFG_HOST_RXPKT *p)
{
    ssv_type_u16 offset=0;
    offset=(ssv_type_u32)ssv_hal_get_rxpkt_data_ptr(p)-(ssv_type_u32)p;
    return offset;
}

void  CmdEng_RxHdlData(void *frame)
{
	//struct cfg_host_rxpkt *pPktInfo = ;
	ssv6xxx_data_result data_ret = SSV6XXX_DATA_CONT;

	//Give it to AP handle firstly.
	//LOG_PRINTF("gHCmdEngInfo->hw_mode: %d \n",gHCmdEngInfo->hw_mode);
#if (AP_MODE_ENABLE == 1)
    if ((SSV6XXX_HWM_AP == gDeviceInfo->vif[0].hw_mode)||((MAX_VIF_NUM > 1) && (SSV6XXX_HWM_AP == gDeviceInfo->vif[1].hw_mode)))
    	data_ret = AP_RxHandleFrame(frame);
#endif

    if (SSV6XXX_DATA_CONT == data_ret)
    {
        int i;
        CFG_HOST_RXPKT *pkt_id = (CFG_HOST_RXPKT *)OS_FRAME_GET_DATA(frame);
        ssv_type_u8 bssid_idx= ssv_hal_get_rxpkt_bssid_idx(pkt_id);
        if(bssid_idx>=MAX_VIF_NUM)
        {
            LOG_PRINTF("CmdEng_RxHdlData vif invalid =%d\r\n",bssid_idx);
            SSV_ASSERT(0);
        }
        //remove ssv descriptor(RxInfo), just leave raw data.
        os_frame_pull(frame, CmdEng_GetRawRxDataOffset((struct cfg_host_rxpkt *)OS_FRAME_GET_DATA(frame)));

        for (i=0;i<HOST_DATA_CB_NUM;i++)
        {
            data_handler handler = gDeviceInfo->data_cb[i];
            if (handler)
            {
                data_ret = handler(frame, OS_FRAME_GET_DATA_LEN(frame),bssid_idx);
                if (SSV6XXX_DATA_ACPT==data_ret)
                break;
            }
        }//-----------------
    }

    if(SSV6XXX_DATA_ACPT != data_ret)
    {
        os_frame_free(frame);
    }
}

