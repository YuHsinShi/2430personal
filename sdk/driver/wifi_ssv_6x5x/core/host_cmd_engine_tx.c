/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#define __SFILE__ "host_cmd_engine_tx.c"

#include <log.h>
#include <ssv_types.h>
#include <ssv_hal.h>
#include <pbuf.h>
#include <msgevt.h>
#include <cmd_def.h>
#if (AP_MODE_ENABLE == 1)        
#include <ap/ap.h>
#include <ap_info.h>
#include <ap_sta_info.h>
#endif
#include <host_apis.h>
#include "host_cmd_engine.h"
#include "host_cmd_engine_priv.h"
#include "host_cmd_engine_sm.h"
#include "host_cmd_engine_tx.h"

#include <drv/ssv_drv.h>
#include <ssv_devinfo.h>
#include <txrx_hdl.h>
//extern void pendingcmd_expired_handler(void* data1, void* data2);

#if 0
static s32  ssv6xxx_wifi_cb(s32 arraylen, u32 **array, u32 *handler, ssv6xxx_cb_action action)
{
	ssv6xxx_result ret = SSV6XXX_SUCCESS;
	int i, nEmptyIdx=-1;
	ssv_type_bool bFound = FALSE;

	do
	{
		for (i=0;i<arraylen;i++)
		{
			if (NULL == array[i])
			{
				nEmptyIdx = i;
			}
			else if(handler == array[i])
			{
				nEmptyIdx = i;
				bFound = TRUE;
				break;
			}
			else
			{}
		}

		if(action == SSV6XXX_CB_ADD)
		{
			//add
			if (-1 == nEmptyIdx)
			{
				LOG_ERROR("queue is not enough\n");
				ret = SSV6XXX_QUEUE_FULL;
#ifdef __AP_DEBUG__
				ASSERT(FALSE);
#endif//__AP_DEBUG__
				break;
			}
			array[nEmptyIdx] = handler;
		}
		else
		{
			//remove
			if(bFound)
				array[nEmptyIdx] = NULL;
		}

	} while (0);


	return ret;


}


extern s32 AP_Start( void );
extern s32 AP_Config(struct cfg_set_ap_cfg *ap_cfg);
extern s32 AP_Stop( void );


// u8 *str[] = { "STA", "AP", "IBSS", "WDS" };
static s32 HCmdEng_set_opmode(struct cfg_host_cmd *hCmd)
{
	ssv6xxx_result ret = SSV6XXX_SUCCESS;
	u32 *op_mode = hCmd->un.dat32;


	gDeviceInfo->hw_mode = *op_mode;
	if(*op_mode != 1)//AP mode
		AP_Stop();




	return ret;
}



static s32 HCmdEng_set_config(struct cfg_host_cmd *hCmd)
{
	ssv6xxx_result ret = SSV6XXX_SUCCESS;
	struct cfg_host_cmd *hSubCmd = (struct cfg_host_cmd *)hCmd->un.dat8;
	u16 len = HOST_CMD_HDR_LEN;


	do{

		if(hSubCmd->h_cmd == SSV6XXX_HOST_CMD_SET_OPMODE)
			HCmdEng_set_opmode(hSubCmd);


		len += hSubCmd->len;
		hSubCmd = (struct cfg_host_cmd *)((u8*)hSubCmd+hSubCmd->len);

	}while(len<hCmd->len);




	return ret;
}




static s32 HCmdEng_set_ap_config(struct cfg_host_cmd *hCmd)
{
	struct cfg_set_ap_cfg *ap_cfg = (struct cfg_set_ap_cfg *)hCmd->un.dat8;

	AP_Config(ap_cfg);
	AP_Start();

	return 0;
}







static s32 HCmdEng_cmd_set_cb(struct cfg_host_cmd *pPktInfo)
{
	ssv6xxx_result ret = SSV6XXX_SUCCESS;
	struct cfg_set_cb_request *cbreq =(struct cfg_set_cb_request *) ((u8 *)pPktInfo->un.dat8);
	ssv6xxx_cb_action action = SSV6XXX_CB_ADD;
    u32 Lcmd=pPktInfo->h_cmd|(pPktInfo->RSVD0<<8);

	if(SSV6XXX_HOST_CMD_SET_DATA_CB == Lcmd)
		return ssv6xxx_wifi_cb(HOST_DATA_CB_NUM, (u32 **)&gHCmdEngInfo->data_cb, (u32 *)cbreq->cb, SSV6XXX_CB_ADD);
	else{

    	switch (Lcmd)
    	{
    	case SSV6XXX_HOST_CMD_SET_EVT_CB:
    	case SSV6XXX_HOST_CMD_SET_DATA_CB:

    		break;
    	case SSV6XXX_HOST_CMD_UNSET_EVT_CB:
    	case SSV6XXX_HOST_CMD_UNSET_DATA_CB:
    		action = SSV6XXX_CB_REMOVE;
    		break;

    	default:
    		ASSERT(FALSE);
    		break;
    	}
    }
	ret = ssv6xxx_wifi_cb(HOST_EVT_CB_NUM, (u32 **)&gHCmdEngInfo->evt_cb, (u32 *)cbreq->cb, action);


	//FREEIF(pPktInfo);
	return ret;
}





static s32 HCmdEng_cmd_set_addr4(struct cfg_host_cmd *pPktInfo)
{
    /**
        * The Format of SSV6XXX_HOST_CMD_SET_ADDR4 command:
        * @ { Address (6) }
        *      @ if Address is zero, imply no WDS function.
        */
    ssv6xxx_result ret = SSV6XXX_SUCCESS;
    u8 *pos=(u8 *)pPktInfo->un.dat8;

    /* Forbid BC/MC set to address 4 */
    if (pos[0] & 0x01) {
        return SSV6XXX_FAILED;
    }

    if (pos[0]==0x00 && pos[1]==0x00 && pos[2]==0x00 &&
        pos[3]==0x00 && pos[4]==0x00 && pos[5]==0x00)
    {
        /* Disable WDS function */
        SET_TXREQ_WITH_ADDR4(gDeviceInfo, 0);
    }
    else
    {
        /* Enable WDS function */
        SET_TXREQ_WITH_ADDR4(gDeviceInfo, 1);
        OS_MemCPY((void*)gDeviceInfo->addr4.addr, (void*)pos, ETHER_ADDR_LEN);
    }
    return ret;
}



static s32 HCmdEng_cmd_set_ht(struct cfg_host_cmd *pPktInfo)
{
    /**
        * The Format of SSV6XXX_HOST_CMD_SET_HT command:
        * @ { Enable/Disable (2) + HT Control Field (4) }
        */
    ssv6xxx_result ret = SSV6XXX_SUCCESS;

    if (pPktInfo->un.dat16[0] == 0)
    {
        /* Disable HT */
        SET_TXREQ_WITH_HT(gDeviceInfo, 0);
        gDeviceInfo->ht_ctrl = 0x00;
    }
    else
    {
        /* Enable HT */
        SET_TXREQ_WITH_HT(gDeviceInfo, 1);
        gDeviceInfo->ht_ctrl = *(u32 *)&pPktInfo->un.dat16[1];
    }
    return ret;
}



static s32 HCmdEng_cmd_set_protect(struct cfg_host_cmd *pPktInfo)
{
    ssv6xxx_result ret = SSV6XXX_SUCCESS;


    return ret;
}



static s32 HCmdEng_cmd_set_qos(struct cfg_host_cmd *pPktInfo)
{
    /**
        * The Format of the SSV6XXX_HOST_CMD_SET_QOS command:
        * @ { Enable/Disable (2)  + QoS Control Field (2) }
        */
    ssv6xxx_result ret = SSV6XXX_SUCCESS;
    if (pPktInfo->un.dat16[0] != 0) {
        gDeviceInfo->qos_ctrl = pPktInfo->un.dat16[1];
        SET_TXREQ_WITH_QOS(gDeviceInfo, 1);
    }
    else
    {
        gDeviceInfo->qos_ctrl = 0;
        SET_TXREQ_WITH_QOS(gDeviceInfo, 0);
    }
    return ret;
}
#endif












static const HCmdEng_TrapHandler sgTrapHandler[] =
{
	//{ SSV6XXX_HOST_CMD_SET_CONFIG, HCmdEng_set_config },
	//{ SSV6XXX_HOST_CMD_SET_OPMODE, HCmdEng_set_opmode },
	{ SSV6XXX_HOST_CMD_SET_CONFIG, NULL },
	{ SSV6XXX_HOST_CMD_SET_OPMODE, NULL },


};


#if 0
static const HCmdEng_TxCmdHandler CustomTxLocalCmdHandler[] =
{
	//---------------------------------------------
    //Command use in host
	HCmdEng_cmd_set_cb,	//HOST_CMD_SET_EVT_CB	,
	HCmdEng_cmd_set_cb,	//HOST_CMD_SET_DATA_CB	,
	HCmdEng_cmd_set_cb, //HOST_CMD_UNSET_DATA_CB	,
	HCmdEng_cmd_set_cb, //HOST_CMD_UNSET_DATA_CB	,
	NULL,
	HCmdEng_cmd_set_addr4,
	HCmdEng_cmd_set_ht,
	HCmdEng_cmd_set_protect,
	HCmdEng_cmd_set_qos,
	HCmdEng_set_ap_config,          //SSV6XXX_HOST_CMD_SET_AP_CFG
};
#endif



#if 0
s32 HCmdEng_cmd_send(ssv6xxx_host_cmd_id CmdId, void *pCmdData, u32 nCmdLen)
{
	ssv6xxx_result ret = SSV6XXX_SUCCESS;
	int size = HOST_CMD_HDR_LEN+ nCmdLen;
	struct cfg_host_cmd *pHostCmd = OS_MemAlloc(HOST_CMD_HDR_LEN+ nCmdLen);
	OS_MemSET(pHostCmd, 0, size);

	pHostCmd->c_type = HOST_CMD;
	pHostCmd->h_cmd = CmdId;
	pHostCmd->len = size;
	OS_MemCPY(((u8 *)pHostCmd+HOST_CMD_HDR_LEN), pCmdData, nCmdLen);

	if (ssv6xxx_drv_send(pHostCmd, pHostCmd->len) < 0) {
		PBUF_MFree(pHostCmd);
		ret = SSV6XXX_FAILED;
	}
	return ret;
}
#endif

#ifdef __DUMP_TX__
extern ssv_type_u16 fpga_dumptx(struct PKT_Info_st *ppkt,ssv_type_u8 *string,ssv_type_u8 accumulating, ssv_type_bool bdumponefile);
#endif
extern OsSemaphore ap_sta_on_off_sphr;
extern ssv_type_s32 TXRXTask_FrameEnqueue(void* frame, ssv_type_u32 priority);
void  CmdEng_TxHdlCmd(void *frame)
{
    struct cfg_host_cmd *hCmd = (struct cfg_host_cmd *)OS_FRAME_GET_DATA(frame);
    const HCmdEng_TrapHandler  * trap_entry = sgTrapHandler;
    ssv_type_u32 i;// max_idx;

    cmdeng_task_st =__LINE__;
    if(hCmd->h_cmd < SSV6XXX_HOST_SOC_CMD_MAXID)
    {
        trap_entry = sgTrapHandler;
        for(i=0; i<sizeof(sgTrapHandler)/sizeof(HCmdEng_TrapHandler); i++) {
            if (sgTrapHandler[i].hCmdID != hCmd->h_cmd)
                continue;

            if(sgTrapHandler[i].hTrapHandler)
                sgTrapHandler[i].hTrapHandler(hCmd);
            break;

        }
#ifdef __DUMP_TX__
        fpga_dumptx((struct PKT_Info_st*)hCmd, NULL, 5, 1);
#endif

        if ((hCmd->h_cmd == SSV6XXX_HOST_CMD_SCAN) || (hCmd->h_cmd == SSV6XXX_HOST_CMD_JOIN) || (hCmd->h_cmd == SSV6XXX_HOST_CMD_LEAVE) )
        {
        //u32 timeout = (hCmd->h_cmd == SSV6XXX_HOST_CMD_SCAN)? SCAN_CMD_TIMEOUT:PENDING_CMD_TIMEOUT;

        cmdeng_task_st =__LINE__;
            OS_MutexLock(gHCmdEngInfo->CmdEng_mtx);
            gHCmdEngInfo->pending_cmd_seqno = hCmd->cmd_seq_no;
            gHCmdEngInfo->blockcmd_in_q = true;
            LOG_DEBUGF(LOG_CMDENG, ("[CmdEng]: Got Block cmd %d, start to pending cmd\r\n", gHCmdEngInfo->pending_cmd_seqno));
            //if (hCmd->h_cmd == SSV6XXX_HOST_CMD_SCAN)
            //os_create_timer(SCAN_CMD_TIMEOUT, pendingcmd_expired_handler, gHCmdEngInfo, NULL, MBOX_CMD_ENGINE);
            OS_MutexUnLock(gHCmdEngInfo->CmdEng_mtx);
        }
        //==============================================================
        {
            //Send to tx driver task
            cmdeng_task_st =__LINE__;
            if(false == TXRXTask_FrameEnqueue(frame, 0))
            {
                LOG_PRINTF("[CmdEng]snd cmd %d fail\r\n",hCmd->h_cmd);
                os_frame_free(frame);
            }
        }
        //==============================================================
        return;
    }
    else
    {
        ssv6xxx_result res;
        cmdeng_task_st =__LINE__;
        if(hCmd->h_cmd == SSV6XXX_HOST_CMD_SET_STA_CFG)
        {
            Sta_setting * sta_mod = NULL;
            sta_mod = (Sta_setting *)hCmd->un.dat8;
            if(SSV6XXX_SUCCESS == (res=ssv6xxx_wifi_station(sta_mod))){
                OS_SemSignal(ap_sta_on_off_sphr);
            }else{
                LOG_PRINTF("sta start error=%d\r\n",res);
            }
            os_frame_free(frame);
            return;
        }
        else if (hCmd->h_cmd == SSV6XXX_HOST_CMD_SET_AP_CFG)
        {
            Ap_setting * ap_mod = NULL;
            ap_mod = (Ap_setting *)hCmd->un.dat8;
            if(SSV6XXX_SUCCESS == (res=ssv6xxx_wifi_ap(ap_mod,ap_mod->step))){
                OS_SemSignal(ap_sta_on_off_sphr);
            }else{
                LOG_PRINTF("ap start error=%d\r\n",res);
            }
            os_frame_free(frame);
            return;

        }

    }
#if (AP_MODE_ENABLE == 1)        
#if (AUTO_BEACON != 0)
    if (hCmd->h_cmd > SSV6XXX_HOST_SOC_CMD_MAXID)
#else
    if (hCmd->h_cmd == SSV6XXX_HOST_CMD_UPDATE_BEACON)
    {
        ssv_type_u8 update_bcn = 0;

        OS_MutexLock(gDeviceInfo->APInfo->g_dev_bcn_mutex);
        update_bcn = gDeviceInfo->APInfo->beacon_upd_need;
        OS_MutexUnLock(gDeviceInfo->APInfo->g_dev_bcn_mutex);

        if (update_bcn == 1)
        {
            cmdeng_task_st =__LINE__;
            if (0==ssv_hal_beacon_set(gDeviceInfo->APInfo->bcn, gDeviceInfo->APInfo->bcn_info.tim_cnt_oft, TRUE))
            {
                cmdeng_task_st =__LINE__;
                OS_MutexLock(gDeviceInfo->APInfo->g_dev_bcn_mutex);
                gDeviceInfo->APInfo->beacon_upd_need = 0;
                OS_MutexUnLock(gDeviceInfo->APInfo->g_dev_bcn_mutex);

            }
            else
            {
                //post message to cmd_engine
                //LOG_PRINTF("Send beacon_upd_need again\n\r");
                cmdeng_task_st =__LINE__;
                if(ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_UPDATE_BEACON, NULL, 0) != SSV6XXX_SUCCESS)
                    LOG_PRINTF("Failed to UPDATE_BEACON\n\r");
            }
        }       
    }
    else if (hCmd->h_cmd > SSV6XXX_HOST_SOC_CMD_MAXID)
#endif
#endif
    {
        LOG_DEBUGF(LOG_CMDENG|LOG_LEVEL_WARNING, ("%s(): invalid host cmd: %d\r\n", __FUNCTION__, hCmd->h_cmd));
        os_frame_free(frame);
        return;
    }

    os_frame_free(frame);
}




extern tx_result ssv6xxx_data_could_be_send(void *frame, ssv_type_bool bAPFrame, ssv_type_u32 TxFlags);


