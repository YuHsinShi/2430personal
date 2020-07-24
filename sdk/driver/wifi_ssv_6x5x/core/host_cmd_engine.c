/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#define __SFILE__ "host_cmd_engine.c"

#include <host_config.h>
#include <ssv_types.h>
#include <host_apis.h>
#include <ssv_timer.h>
#include <ssv_dev.h>
#include <ssv_hal.h>
#if((CONFIG_CHIP_ID==SSV6051Q)||(CONFIG_CHIP_ID==SSV6051Z)||(CONFIG_CHIP_ID==SSV6030P))
#include <SSV6030/ssv6030_pktdef.h>
#endif
#if((CONFIG_CHIP_ID==SSV6006B)||(CONFIG_CHIP_ID==SSV6006C))
#include <SSV6006/ssv6006_pktdef.h>
#endif
#if (AP_MODE_ENABLE == 1)        
#include <ap/ap.h>
#endif
#include <pbuf.h>
#include <msgevt.h>
#include <cmd_def.h>
#include <drv/ssv_drv.h>		// for ssv6xxx_drv_send()
#include <txrx_hdl.h>
#include <log.h>
#if (defined _WIN32)
#include <wtypes.h>
#include <Dbt.h>
#endif
#include "host_cmd_engine.h"
#include "host_cmd_engine_priv.h"
#include "host_cmd_engine_sm.h"

#define FREE_FRM_NUM 5
extern struct task_info_st g_host_task_info[];

struct CmdEng_st
{
    ModeType mode;
    ssv_type_u32 BlkCmdNum;
    ssv_type_bool BlkCmdIn;
};


HostCmdEngInfo_st *gHCmdEngInfo;
#define STATE_MACHINE_DATA struct HostCmdEngInfo




extern void os_timer_init(os_timer_st *timer);






//-------------------------------------------------------------
extern void CmdEng_RxHdlEvent(struct cfg_host_event *pHostEvt);
extern void CmdEng_TxHdlCmd(struct cfg_host_cmd *pPktInfo);
extern void AP_RxHandleAPMode(CFG_HOST_TXREQ *pPktInfo);





/*
void pendingcmd_expired_handler(void* data1, void* data2)
{
    HostCmdEngInfo_st *info = (HostCmdEngInfo_st *)data1;
    OS_MutexLock(info->CmdEng_mtx);
    if(info->debug != false)
        LOG_DEBUG("[CmdEng]: pending cmd %d timeout!! \n", info->pending_cmd_seqno);

    info->pending_cmd_seqno = 0;
    info->blockcmd_in_q = false;
    OS_MutexUnLock(info->CmdEng_mtx);
}
*/
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

ssv_type_u32 cmdeng_task_st=0;

void CmdEng_HandleQueue(void *frame)
{

	//event/frames(data/ctrl/mgmt)/cmd
	//pPktInfo;
	CFG_HOST_RXPKT * rxpkt = (CFG_HOST_RXPKT *)OS_FRAME_GET_DATA(frame);//(struct cfg_host_rxpkt *)pPktInfo;
	ssv_type_u8 c_type=ssv_hal_get_rxpkt_ctype(rxpkt);
    cmdeng_task_st =__LINE__;
	switch (c_type)
	{
		//-------------
		//RX data
#if(MLME_TASK==0)
	case M0_RXEVENT:
    {
        if ((SSV6XXX_HWM_AP == gDeviceInfo->vif[0].hw_mode)||
            ((MAX_VIF_NUM > 1) && (SSV6XXX_HWM_AP == gDeviceInfo->vif[1].hw_mode)))
        {
#if (AP_MODE_ENABLE == 1)            
            cmdeng_task_st =__LINE__;
            AP_RxHandleAPMode(frame);
#endif
        }
		else
        {
            LOG_DEBUGF(LOG_CMDENG|LOG_LEVEL_WARNING, ("Unhandle M0_RXEVENT \r\n"));
            os_frame_free(frame);
        }
        //    CmdEng_RxHdlData(frame);
		break;
    }
#endif
	case HOST_EVENT:
        cmdeng_task_st =__LINE__;
		CmdEng_RxHdlEvent(frame);
		break;
		//-------------
		//TX
	case HOST_CMD:
    {
        FrmQ *pcmd = NULL;
        struct cfg_host_cmd *hCmd = (struct cfg_host_cmd *)OS_FRAME_GET_DATA(frame);

        if ((gHCmdEngInfo->blockcmd_in_q == true)&&
            (hCmd->h_cmd != SSV6XXX_HOST_CMD_SET_STA_CFG)&&
            (hCmd->h_cmd != SSV6XXX_HOST_CMD_SET_AP_CFG))
        {
            if( (pcmd = (FrmQ *)list_q_deq_safe(&gHCmdEngInfo->free_FrmQ, &gHCmdEngInfo->CmdEng_mtx)) == NULL)
                        pcmd = (FrmQ *)OS_MemAlloc(sizeof(FrmQ));
            if(pcmd!=NULL)
            {
                                pcmd->frame = frame;
                list_q_qtail_safe(&gHCmdEngInfo->pending_cmds, (struct ssv_list_q *)pcmd, &gHCmdEngInfo->CmdEng_mtx);

                LOG_DEBUGF(LOG_CMDENG, ("[CmdEng]: Pending cmd %d\r\n", hCmd->cmd_seq_no));
            }
            else
            {
                LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
            }
            return;
        }
        else
        {
            cmdeng_task_st =__LINE__;
            CmdEng_TxHdlCmd(frame);
        }
		break;
    }
	default:
	    hex_dump(rxpkt,128);
		LOG_FATAL("Unexpect c_type %d appeared\r\n", c_type);
		SSV_ASSERT(0);
	}

}

ssv6xxx_result CmdEng_GetStatus(void *stp)
{
    struct CmdEng_st *st = (struct CmdEng_st *)stp;
    if(st == NULL)
        return SSV6XXX_INVA_PARAM;

    OS_MutexLock(gHCmdEngInfo->CmdEng_mtx);
    st->mode = gHCmdEngInfo->curr_mode;
    st->BlkCmdIn = gHCmdEngInfo->blockcmd_in_q;
    st->BlkCmdNum = list_q_len(&gHCmdEngInfo->pending_cmds);
    OS_MutexUnLock(gHCmdEngInfo->CmdEng_mtx);

    return SSV6XXX_SUCCESS;
}








//---------------------------------------------------------------------------------------------
extern ssv_type_s32 AP_Start(void);
extern ssv_type_s32 AP_Stop(ssv_type_bool bFreePbuf);
extern ssv_type_u32 g_RunTaskCount;
void CmdEng_Task( void *args )
{
	MsgEvent *MsgEv;
	ssv_type_s32 res;
    CFG_HOST_RXPKT *rxpkt=NULL;
    ssv_type_u32 msgData=0;

	//LOG_PRINTF("CmdEng_Task started(%d)\r\n",g_RunTaskCount);
	g_RunTaskCount++;

#ifdef __TEST__
	//_Cmd_CreateSocketClient(0, NULL);

#endif

	//SSVHostCmdEng_Start();
	//	SM_ENTER(HCMDE, IDLE, NULL);

	while((gHCmdEngInfo->curr_mode!=MT_EXIT)&&(!OS_TaskShallStop()))
	{
        if ((gHCmdEngInfo->blockcmd_in_q == false) && (list_q_len_safe(&gHCmdEngInfo->pending_cmds, &(gHCmdEngInfo->CmdEng_mtx)) > 0))
        {
            //Proceeding pending cmds
            FrmQ *pcmd = NULL;
            struct cfg_host_cmd *hCmd = NULL;

            pcmd = (FrmQ *)list_q_deq_safe(&gHCmdEngInfo->pending_cmds, &gHCmdEngInfo->CmdEng_mtx);
            hCmd = (struct cfg_host_cmd *)OS_FRAME_GET_DATA(pcmd->frame);
            LOG_DEBUGF(LOG_CMDENG, ("[CmdEng]: Pop pending cmd %d to execute\r\n", hCmd->cmd_seq_no));
            cmdeng_task_st =__LINE__;
            CmdEng_TxHdlCmd(pcmd->frame);
            cmdeng_task_st =__LINE__;
            OS_MutexLock(gHCmdEngInfo->CmdEng_mtx);
            if(list_q_len(&gHCmdEngInfo->free_FrmQ) < FREE_FRM_NUM)
            {    
                list_q_qtail(&gHCmdEngInfo->free_FrmQ, (struct ssv_list_q *)pcmd);
            }
            else
            {
                OS_MemFree(pcmd);
            }
            OS_MutexUnLock(gHCmdEngInfo->CmdEng_mtx);
        }
        else
        {
            /* Wait Message: */
            cmdeng_task_st =__LINE__;
            res = msg_evt_fetch(MBOX_CMD_ENGINE, &MsgEv);
            //ASSERT(res == OS_SUCCESS);
            if(res == OS_SUCCESS)
            {
    		//LOG_TRACE("AP needs to handle msg:%d.\n", MsgEv->MsgType);
            cmdeng_task_st =__LINE__;
            switch(MsgEv->MsgType)
            {
            case MEVT_PKT_BUF:
                rxpkt=(CFG_HOST_RXPKT *)MsgEv->MsgData;
                os_msg_free(MsgEv);
                cmdeng_task_st =__LINE__;
                CmdEng_HandleQueue(rxpkt);
                break;

                /**
                *  Message from software timer timeout event.
                */
            case MEVT_HOST_TIMER:
                cmdeng_task_st =__LINE__;
                os_timer_expired((void *)MsgEv);
                os_msg_free(MsgEv);
                break;


            case MEVT_HOST_CMD:
                msgData=MsgEv->MsgData;
                msg_evt_free(MsgEv);
                cmdeng_task_st =__LINE__;
                switch(msgData)
                {
#if 0// (AP_MODE_ENABLE == 1)
                case AP_CMD_AP_MODE_ON:
                    AP_Start();
                    break;

                case AP_CMD_AP_MODE_OFF:
                    AP_Stop();
                    break;
#endif                    
#ifdef __TEST_DATA__
                case AP_CMD_ADD_STA:
                    TestCase_AddAPSta();
                    break;

                case AP_CMD_PS_POLL_DATA:
                    TestCase_SendPSPoll();
                    break;

                case AP_CMD_PS_TRIGGER_FRAME:
                    TestCase_SendTriggerFrame();
                    break;
#endif//__TEST_DATA__
                case CMD_ENG_TASK_EXIT:
                    LOG_PRINTF("Rev CMD_ENG_TASK_EXIT(mode=%d)\r\n",gHCmdEngInfo->curr_mode);
                    continue;
                    //break;
                default:
                    break;
                }

                break;

            default:
                //SoftMac_DropPkt(MsgEv);
                LOG_DEBUGF(LOG_CMDENG, ("%s(): unknown message type(%02x) !!\r\n",
                    __FUNCTION__, MsgEv->MsgType));
                break;
            };
        }
        }

	}
    g_RunTaskCount-=1;
    LOG_PRINTF("cmd_eng exit(%d)!!\r\n",g_RunTaskCount);
}

#if (defined(_WIN32)) && (_WIN32 == 1 && CONFIG_RX_POLL == 0)
#define INITGUID
#include <guiddef.h>
WNDPROC wpOrigProc;
DEFINE_GUID(GUID_DEVINTERFACE_ssvsdio_intevent,
			0x76c8ffb9, 0xf552, 0x4398, 0xa3, 0xd6, 0x52, 0x73, 0xfe, 0xc5, 0x5b, 0xe2);

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_DEVICECHANGE:
		{
			if ( wParam == DBT_CUSTOMEVENT )
			{
				DEV_BROADCAST_HANDLE* handle = (DEV_BROADCAST_HANDLE*)lParam;
				if ( handle->dbch_devicetype == DBT_DEVTYP_HANDLE && IsEqualGUID(&handle->dbch_eventguid, &GUID_DEVINTERFACE_ssvsdio_intevent ))
				{
					SSV6XXX_Drv_Rx_Task(NULL);
				}
			}
			break;
		}
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

HWND CreateMessageOnlyWindow()
{
	WNDCLASSEX wx;
	DEV_BROADCAST_HANDLE handle;
	HWND hwnd;
	ZeroMemory(&wx, sizeof(WNDCLASSEX));
	ZeroMemory(&handle, sizeof(DEV_BROADCAST_HANDLE));

	wx.cbSize = sizeof(WNDCLASSEX);
	wx.lpfnWndProc = WndProc;
	wx.lpszClassName = L"Message-Only Window";

	if ( RegisterClassEx(&wx) )
		hwnd = CreateWindow(L"Message-Only Window", NULL, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, NULL, NULL);
	else
		return NULL;

	handle.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
	handle.dbch_handle = (HANDLE)ssv6xxx_drv_get_handle();
	handle.dbch_devicetype = DBT_DEVTYP_HANDLE;
	handle.dbch_eventguid = GUID_DEVINTERFACE_ssvsdio_intevent;
	RegisterDeviceNotification(hwnd,&handle,DEVICE_NOTIFY_WINDOW_HANDLE);
}

void SSV6XXX_drv_msg_only(void *args)
{
	MSG msg;
	HWND m_hWnd;
	m_hWnd = CreateMessageOnlyWindow();



	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if(msg.message == WM_QUIT)
		{
			break;
		}
	}
}
#endif

struct task_info_st g_host_task_info[] =
{
	{ "host_cmd",   (OsMsgQ)0, 24, 	OS_CMD_ENG_PRIO, CMD_ENG_STACK_SIZE, NULL, CmdEng_Task },

};

#ifdef RXFLT_ENABLE
ssv6xxx_data_result test_cb(void * data, ssv_type_u32 len)
{
    LOG_DEBUG("[CmdEng]: RXFLT test, data = %x, len = %d\r\n", (ssv_type_u32)data, len);
    return SSV6XXX_DATA_CONT;
}
#endif
extern void TxRxHdl_DeInit(void);
void CmdEng_Deinit(void)
{
    TxRxHdl_DeInit();
    
    OS_MutexDelete(gHCmdEngInfo->CmdEng_mtx);
    OS_MemFree(gHCmdEngInfo);
    OS_MsgQDelete(g_host_task_info[0].qevt);
}
ssv_type_s32 CmdEng_Init(void)
{
	ssv_type_u32 i, size, res=OS_SUCCESS;

	size = sizeof(HostCmdEngInfo_st);
	gHCmdEngInfo = (HostCmdEngInfo_st *)OS_MemAlloc(size);
    if(NULL==gHCmdEngInfo)
    {
        LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
        return OS_FAILED;
    }

	OS_MemSET(gHCmdEngInfo, 0, size);
    OS_MutexInit(&(gHCmdEngInfo->CmdEng_mtx),"CmdEng_mtx");
    list_q_init(&gHCmdEngInfo->pending_cmds);
    list_q_init(&gHCmdEngInfo->free_FrmQ);
    gHCmdEngInfo->curr_mode = MT_STOP;
    gHCmdEngInfo->blockcmd_in_q = false;
    gHCmdEngInfo->pending_cmd_seqno = 0;


    TxRxHdl_Init();

	size = sizeof(g_host_task_info)/sizeof(struct task_info_st);
	for(i = 0; i < size; i++)
    {
		if (g_host_task_info[i].qlength> 0)
        {
			SSV_ASSERT(OS_MsgQCreate(&g_host_task_info[i].qevt,
				(ssv_type_u32)g_host_task_info[i].qlength)==OS_SUCCESS);
		}

		/* Create Registered Task: */
		OS_TaskCreate(g_host_task_info[i].task_func,
			g_host_task_info[i].task_name,
			g_host_task_info[i].stack_size<<4,
			g_host_task_info[i].args,
			g_host_task_info[i].prio,
			NULL);
	}
#if 0 //RXFLT_ENABLE demo
    struct wifi_flt test_flt_w;
    test_flt_w.b7b2mask = 0x03;
    test_flt_w.fc_b7b2 = 0x00;
    test_flt_w.cb_fn = test_cb;
    RxHdl_SetWifiRxFlt(&test_flt_w, SSV6XXX_CB_ADD);
    struct eth_flt test_flt_e;
    test_flt_e.ethtype = 0x886E;
    test_flt_e.cb_fn = test_cb;
    RxHdl_SetEthRxFlt(&test_flt_e, SSV6XXX_CB_ADD);
#endif
	return res;
}

void CmdEng_FlushPendingCmds(void)
{
    //os_cancel_timer(pendingcmd_expired_handler, (u32)gHCmdEngInfo, (u32)NULL);
    gHCmdEngInfo->blockcmd_in_q = false;
    gHCmdEngInfo->pending_cmd_seqno = 0;
    while(list_q_len(&gHCmdEngInfo->pending_cmds) > 0)
    {
        FrmQ *pcmd = (FrmQ *)list_q_deq(&gHCmdEngInfo->pending_cmds);
        os_frame_free(pcmd->frame);
        pcmd->frame = NULL;
        if(list_q_len(&gHCmdEngInfo->free_FrmQ) > FREE_FRM_NUM)
        {
            OS_MemFree(pcmd);
        }
        else
        {
            list_q_qtail(&gHCmdEngInfo->free_FrmQ, (struct ssv_list_q *)pcmd);
        }
    }
    //LOG_DEBUG("[CmdEng]: CmdEng_FlushPendingCmds\n");
}

ssv6xxx_result CmdEng_SetOpMode(ModeType mode)
{
    ssv6xxx_result ret = SSV6XXX_SUCCESS;

    if(mode > MT_EXIT)
        return SSV6XXX_INVA_PARAM;

    OS_MutexLock(gHCmdEngInfo->CmdEng_mtx);

    switch (gHCmdEngInfo->curr_mode)
    {
        case MT_STOP:
        {
            switch (mode)
            {
                case MT_RUNNING:
                    // To run
                    break;
                case MT_EXIT:
                    // To end
                    break;
                default:
                    ret = SSV6XXX_INVA_PARAM;
                    break;
            }
        }
            break;
        case MT_RUNNING:
            if((mode == MT_STOP)||(mode == MT_EXIT))
            {
                //To stop
                CmdEng_FlushPendingCmds();
            }
            else
            {
                //error handling
                ret = SSV6XXX_INVA_PARAM;
            }
            break;
        case MT_EXIT:
            break;
        default:
            //error handling
            ret = SSV6XXX_FAILED;
            break;
    }

    if(ret == SSV6XXX_SUCCESS)
        gHCmdEngInfo->curr_mode = mode;

    if(mode == MT_EXIT)
    {
        MsgEvent *msg_evt = NULL;
        msg_evt = msg_evt_alloc();
        if(NULL!=msg_evt)
        {
            ssv_type_u32 cur_task_cnt = g_RunTaskCount;
            msg_evt->MsgType = MEVT_HOST_CMD;
            msg_evt->MsgData = CMD_ENG_TASK_EXIT;
            while((ret = msg_evt_post(MBOX_CMD_ENGINE, msg_evt))==OS_FAILED)
            {
                LOG_PRINTF("cmd_eng exit retry\r\n");
                OS_MsDelay(10);                
            }
            //LOG_PRINTF("send CMD_ENG_TASK_EXIT = %d\r\n",ret);
            while(g_RunTaskCount!=(cur_task_cnt-1))
            {
                LOG_PRINTF("wait cmdeng exit(%d/%d)\r\n",cur_task_cnt,g_RunTaskCount);
                OS_TickDelay(1);
            }
        }
    }

    OS_MutexUnLock(gHCmdEngInfo->CmdEng_mtx);

    return ret;
}


#if 0
void RX_public_host_event(u32 nEvtId, void *data)
{

	HDR_HostEvent *HostEvent;
	HostEvent= (HDR_HostEvent *)data;

	switch(nEvtId){
	case HOST_EVT_HW_MODE_RESP:
		SM_ENTER(HCMDE, RUNNING, NULL);

		//LOG_TRACE(SSV_STA,"set hw mode sucessful\n");
		break;
	case HOST_EVT_SCAN_RESULT:
		//LOG_TRACE(SSV_STA, "scan sucessful\n");
		break;
	case HOST_EVT_JOIN_RESULT:
		{
			struct resp_evt_result *join_resp ;

			join_resp=(struct resp_evt_result *)HostEvent->dat;
			//struct netif *netif;
			//char *name="wlan0";
			if(join_resp->u.status_code==0)
			{
				/*netif = netif_find(name);
				if(netif!=NULL)
				{
				netifapi_dhcp_start(netif);
				}*/
				//LOG_TRACE(SSV_STA, "join sucessful\n");

			}

			break;
		}

	case HOST_EVT_LEAVE_RESULT:
		break;

	}

}

#endif

// ssv6xxx_result SSVHostCmdEng_Start()
// {
//
//
//
// 	//gHCmdEngInfo->state = SSV_STATE_IDLE;
// 	return SSV6XXX_SUCCESS;
// }
// ssv6xxx_result SSVHostCmdEng_Stop()
// {
//
//
//
//
// 	return SSV6XXX_SUCCESS;
// }






