/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


//#define __SFILE__ "ap_drv_cmd.c"
#include "ap_drv_cmd.h"
#include "ap_config.h"

#include <ssv_ether.h>
#include <cmd_def.h>
#include <host_apis.h>
#include <ssv_devinfo.h>
#include <txrx_hdl.h>
#include <ssv_hal.h>

#include "ap_info.h"

#if (AP_MODE_ENABLE == 1)

void ap_soc_set_bcn(enum ssv6xxx_tx_extra_type extra_type, void *frame, struct cfg_bcn_info *bcn_info, ssv_type_u8 dtim_cnt, ssv_type_u16 bcn_itv, ssv_type_bool block)
{
   if(0==ssv_hal_soc_set_bcn(extra_type,frame,bcn_info,dtim_cnt,bcn_itv))
   {
#if (AUTO_BEACON != 0)
        ssv_hal_beacon_set(frame, bcn_info->tim_cnt_oft,block);
#else
        if (0==ssv_hal_beacon_set(frame, bcn_info->tim_cnt_oft,block))
        {
            OS_MutexLock(gDeviceInfo->APInfo->g_dev_bcn_mutex);
            gDeviceInfo->APInfo->beacon_upd_need = 0;
            OS_MutexUnLock(gDeviceInfo->APInfo->g_dev_bcn_mutex);
        }
        else
        {
            ssv_type_bool post_msg = false;//post message to cmd_engine
            OS_MutexLock(gDeviceInfo->APInfo->g_dev_bcn_mutex);
            if(gDeviceInfo->APInfo->beacon_upd_need == 0)
            {
                gDeviceInfo->APInfo->beacon_upd_need = 1;
                post_msg = true;
            }
            OS_MutexUnLock(gDeviceInfo->APInfo->g_dev_bcn_mutex);
            if (post_msg == true)
                _ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_UPDATE_BEACON, NULL, 0,FALSE);
        }
#endif
    }
}


#ifdef __AP_DEBUG__
extern void APStaInfo_PrintStaInfo(ssv_type_u32 idx);
#endif
ssv_type_bool ap_sta_info_collect(struct cfg_set_sta *cfg_sta,enum cfg_sta_oper sta_oper, APStaInfo_st *sta, enum cfg_ht_type ht, ssv_type_bool qos)
{
    struct cfg_wsid_info *wsid_info = &cfg_sta->wsid_info;
    ssv_type_bool ret = FALSE;
    cfg_sta->sta_oper = sta_oper;

    if(CFG_STA_ADD == sta_oper)
        SET_STA_INFO_VALID(wsid_info, TRUE);
    else
        SET_STA_INFO_VALID(wsid_info, FALSE);

    SET_STA_INFO_OP_MODE(wsid_info, CFG_OP_MODE_STA);
    SET_STA_INFO_QOS_EN(wsid_info, qos);
    SET_STA_INFO_HT_MODE(wsid_info, ht);

    if(sta->addr&&sta->wsid_idx<gMaxAID)
    {
        int i = 0;
        ret = TRUE;
        OS_MemCPY((void*)&wsid_info->addr, (void*)sta->addr, ETH_ALEN);
        wsid_info->wsid_idx = sta->wsid_idx;
        //short preamble
        cfg_sta->short_preamble = !(sta->no_short_preamble_set) ;
        //ht_capabilities
        cfg_sta->ht_capabilities.ht_capabilities_info = sta->ht_capabilities.ht_capabilities_info;
        //mcs
        OS_MemCPY((void*)cfg_sta->ht_capabilities.supported_mcs_set,(void*)sta->ht_capabilities.supported_mcs_set,16);
        //non-ht-support rate
        for(i=0;i<sta->supported_rates_len;i++)
        {
            switch(sta->supported_rates[i]&0x7f)
            {
                case 2: // 1
                    cfg_sta->non_ht_supp_rates |= 1<<0;
                    break;
                case 4: // 2
                    cfg_sta->non_ht_supp_rates |= 1<<1;
                    break;
                case 11: // 5.5
                    cfg_sta->non_ht_supp_rates |= 1<<2;
                    break;
                case 22: // 11
                    cfg_sta->non_ht_supp_rates |= 1<<3;
                    break;
                case 12: // 6
                    cfg_sta->non_ht_supp_rates |= 1<<4;
                    break;
                case 18: // 9
                    cfg_sta->non_ht_supp_rates |= 1<<5;
                    break;
                case 24: // 12
                    cfg_sta->non_ht_supp_rates |= 1<<6;
                    break;
                case 36: // 18
                    cfg_sta->non_ht_supp_rates |= 1<<7;
                    break;
                case 48: // 24
                    cfg_sta->non_ht_supp_rates |= 1<<8;
                    break;
                case 72: // 36
                    cfg_sta->non_ht_supp_rates |= 1<<9;
                    break;
                case 96: // 48
                    cfg_sta->non_ht_supp_rates |= 1<<10;
                    break;
                case 108: // 54
                    cfg_sta->non_ht_supp_rates |= 1<<11;
                    break;
                default:
                    break;
            }

        }
    }
    return ret;
}

void ap_soc_cmd_sta_oper(enum cfg_sta_oper sta_oper, APStaInfo_st *sta, enum cfg_ht_type ht, ssv_type_bool qos)
{

	struct cfg_set_sta cfg_sta;
    OS_MemSET(&cfg_sta, 0, sizeof(struct cfg_set_sta));
    cfg_sta.vif_idx = ((ssv_vif*)gDeviceInfo->APInfo->vif)->idx;
    if(ap_sta_info_collect(&cfg_sta,sta_oper,sta,ht,qos))
    {
        //Add sta to soc
        _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_STA, &cfg_sta, sizeof(struct cfg_set_sta),TRUE,FALSE);
    }
#ifdef __AP_DEBUG__
	APStaInfo_PrintStaInfo(0);
    APStaInfo_PrintStaInfo(1);
#endif//__AP_DEBUG__

}




//Send frame directly.
ssv_type_s32 ap_soc_data_send(void *frame, ssv_type_s32 len, ssv_type_bool bAPFrame, ssv_type_u32 TxFlags)
{
	CFG_HOST_TXREQ *req = (CFG_HOST_TXREQ *)OS_FRAME_GET_DATA(frame);
	len+=ssv_hal_get_txreq0_size();

#ifdef __AP_DEBUG__
//check if write buffer too much
	if(frame == gDeviceInfo->APInfo->pOSMgmtframe && len > AP_MGMT_PKT_LEN)
		SSV_ASSERT(FALSE);
#endif

	ssv_hal_set_txreq0_len(req,len);
	OS_FRAME_SET_DATA_LEN(frame, len);
    #if(SW_8023TO80211==1)
    ssv_hal_fill_txreq0(frame,len,0,NULL,NULL,NULL,1,0,0,((ssv_vif*)gDeviceInfo->APInfo->vif)->idx);
    #endif
    if(TRUE == TxHdl_FrameProc(frame, bAPFrame, 0, TxFlags, (ssv_vif*)gDeviceInfo->APInfo->vif))
	    return 0;
    else
        return -1;
}
#endif


