/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <log.h>
#include <cmd_def.h>
#if (AP_MODE_ENABLE == 1)        
#include <ap_info.h>
#endif
#include <hctrl.h>
#include "host_cmd_engine_priv.h"
#include "txrx_task.h"
#include <ssv_drv.h>
#include <txrx_hdl.h>
#include <ssv_hal.h>
#include <core/recover.h>
#include <ssv_dev.h>
#include <pbuf.h>
#if((CONFIG_CHIP_ID==SSV6051Q)||(CONFIG_CHIP_ID==SSV6051Z)||(CONFIG_CHIP_ID==SSV6030P))
#include <SSV6030/ssv6030_pktdef.h>
#endif
#if((CONFIG_CHIP_ID==SSV6006B)||(CONFIG_CHIP_ID==SSV6006C))
#include <SSV6006/ssv6006_pktdef.h>
#endif


#define TX_FRAME_ARRAY_SIZE 20
ssv_type_u16 tx_frame_len_count=0;
ssv_type_u16 tx_frame_len[TX_FRAME_ARRAY_SIZE];

typedef struct frameList
{
    struct ssv_llist  _list;
    void* frame; //store the address of cmd
} FrmL;

//extern OsMutex  g_dev_info_mutex;
extern ssv_type_s32 ap_handle_ps(void *pPktInfo);
extern struct Host_cfg g_host_cfg;

static OsMutex task_mtx;
static OsMutex tx_mtx;
static ModeType  curr_mode;
static ssv_type_u32 tx_data_frm_num;
static ssv_type_u32 tx_flush_data_frm_num;
static struct ssv_llist_head tx_hwq[PRI_Q_NUM];
static struct ssv_llist_head free_frmQ;

static OsSemaphore tx_frm_sphr;
static OsSemaphore rx_frm_sphr;
static ssv_type_u32 g_TXSEMA_MAX_NUM;
FrmL *g_freeFrmList = NULL;
#define RXSEMA_MAX_NUM 1 //pbuf maximun

#if ONE_TR_THREAD
void TXRXTask_Task(void *args);
static ssv_type_bool Task_Done = false;
#else
void TXRXTask_TxTask(void *args);
void TXRXTask_RxTask(void *args);
static ssv_type_bool TxTask_Done = false;
static ssv_type_bool RxTask_Done = false;
#endif

struct task_info_st g_txrx_task_info[] =
{
#if ONE_TR_THREAD
	{ "txrxtask_task",   (OsMsgQ)0, 0, 	OS_TX_TASK_PRIO, WIFI_TX_STACK_SIZE, NULL, TXRXTask_Task },
#else
    { "txrxtask_txtask",   (OsMsgQ)0, 0, 	OS_TX_TASK_PRIO, WIFI_TX_STACK_SIZE, NULL, TXRXTask_TxTask },
    { "txrxtask_rxtask",   (OsMsgQ)0, 0, 	OS_RX_TASK_PRIO, WIFI_RX_STACK_SIZE, NULL, TXRXTask_RxTask },
#endif
};

void TXRXTask_TxLock (ssv_type_bool lock)
{
    if(lock)
        OS_MutexLock(tx_mtx);
    else
        OS_MutexUnLock(tx_mtx);
}

void _dq_status_handler (void *data)
{
#if (AP_MODE_ENABLE == 1)    
    struct resp_evt_result *dqStatus = (struct resp_evt_result *)data;
    APStaInfo_st *sta = APStaInfo_FindStaByWsid(dqStatus->u.dq_status.wsid);
    if(sta)
    {
        OS_MutexLock(sta->apsta_mutex);
        //LOG_PRINTF("\r\nwsid:%d, queue len:%d\r\n",dqStatus->u.dq_status.wsid,dqStatus->u.dq_status.len);
        sta->fw_q_len = dqStatus->u.dq_status.len;
        OS_MutexUnLock(sta->apsta_mutex);
        if(dqStatus->u.dq_status.len > 0)
        {
            extern void recalc_tim_gen_beacon(APStaInfo_st * sta);
            // indicate tim and generate beacon
            recalc_tim_gen_beacon(sta);
        }
    }
#endif    
    //LOG_PRINTF("wsid:%d, queue len:%d\r\n",dqStatus->u.dq_status.wsid,dqStatus->u.dq_status.len);
}

extern struct task_info_st g_host_task_info[];
void _prepare_tx_frm_list(void)
{
    ssv_type_u32 i;

    llist_head_init(&free_frmQ);
    for(i = 0; i < PRI_Q_NUM; i++)
        llist_head_init(&tx_hwq[i]);

    OS_MemSET((void*)g_freeFrmList, 0 , sizeof(FrmL)*g_TXSEMA_MAX_NUM);
    for(i = 0; i < g_TXSEMA_MAX_NUM; i++)
        llist_push(&free_frmQ, &(g_freeFrmList[i]._list));
}

//TXRX_RxFrameProc Process rx frame
ssv_type_s32 TXRXTask_RxFrameProc(void* frame)
{
    //send to CmdEngine if it is host_event
    //pass to RxHdl if it is data frame
    ssv_type_s32 ret = SSV6XXX_SUCCESS;
    ssv_type_u8 c_type=0;
    CFG_HOST_RXPKT * rxpkt = (CFG_HOST_RXPKT *)OS_FRAME_GET_DATA(frame);
    OS_FRAME_SET_DATA_LEN(frame, ssv_hal_get_rxpkt_len(rxpkt));
    c_type=ssv_hal_get_rxpkt_ctype(rxpkt);
    if(c_type == M0_RXEVENT)
    {
        if(curr_mode == MT_RUNNING)
        {

            ret = RxHdl_FrameProc(frame);
            return ret;
        }
        else
        {
            os_frame_free(frame);
            LOG_DEBUGF(LOG_TXRX|LOG_LEVEL_WARNING,("[TXRXTask]: free rx pkt due to mode = stop\r\n"));
            return SSV6XXX_FAILED;
        }
    }
    else if(c_type == HOST_EVENT)
    {
        struct cfg_host_event *pPktInfo = (struct cfg_host_event *)OS_FRAME_GET_DATA(frame);

        switch(pPktInfo->h_event)
        {
            case SOC_EVT_PS_POLL:
            case SOC_EVT_NULL_DATA:
#if (AP_MODE_ENABLE == 1)                
                ap_handle_ps(frame); //handle null data and ps_poll
#endif
                break;
            case SOC_EVT_DATA_QUEUE_STATUS:
                _dq_status_handler(pPktInfo->dat);
                os_frame_free(frame);
                break;
            case SOC_EVT_TX_LOOPBACK_DONE:
            {
                ssv_hal_tx_loopback_done(pPktInfo->dat);
                os_frame_free(frame);
                break;
            }
            case SOC_EVT_ADD_BA:
            {
                if(g_host_cfg.ampdu_rx_enable)
                    ieee80211_addba_handler(pPktInfo->dat);
                os_frame_free(frame);
                break;
            }
            case SOC_EVT_ACK:
            {
                LOG_PRINTF("Bee!!Bee!!Bee!!\r\n");
                os_frame_free(frame);
                break;
            }
            default:
            {
                void *msg = os_msg_alloc();
                ssv_type_u8 retry=10;
                if (msg)
                {
                    while(os_msg_send(msg, frame)==OS_FAILED)
                    {
                        retry--;
                        OS_TickDelay(1);
                        //LOG_PRINTF("cmdeng evt retry(%d)\r\n",retry);
                        if(!retry)
                        {
                            LOG_PRINTF("drop ap evt(%d) frame\r\n",pPktInfo->h_event);
                            msg_evt_free(msg);
                            os_frame_free(frame);
                            ret = SSV6XXX_NO_MEM;
                            break;
                        }
                    }
                }
                else
                {
                    os_frame_free(frame);
                    ret = SSV6XXX_NO_MEM;
                }
                break;
            }

        }
        return ret;

    }
    else
    {
        LOG_DEBUGF(LOG_TXRX|LOG_LEVEL_WARNING,("[TXRXTask]: unavailable type of rx pkt\r\n"));
        os_frame_free(frame);
        return SSV6XXX_INVA_PARAM;
    }

    return ret;
}

//TXRX_Task TXRX main thread
#if ONE_TR_THREAD
void TXRXTask_Task(void *args)
{
    void *rFrame = NULL, *tFrame = NULL;
    FrmQ *outFrm = NULL;
    ssv_type_u8  *msg_data = NULL;
    ssv_type_s32 recv_len = 0;
    signed char i = 0;

    while(curr_mode != MT_EXIT)
    {
        if(rFrame == NULL)
        {
            if((rFrame = (ssv_type_u8 *)os_frame_alloc(MAX_RECV_BUF)) != NULL)
            {
                os_frame_push(rFrame, g_host_cfg.trx_hdr_len);
                msg_data = OS_FRAME_GET_DATA(rFrame);
            }
            else
                LOG_DEBUG("[TxRxTask]: malloc = 0!\r\n");
        }

        if(rFrame != NULL)
        {
            OS_SemWait(rx_frm_sphr, 1);
            recv_len = ssv6xxx_drv_recv(msg_data, OS_FRAME_GET_DATA_LEN(rFrame));
            if(recv_len != -1)
            {
                OS_FRAME_SET_DATA_LEN(rFrame, recv_len);
                TXRXTask_RxFrameProc(rFrame);
                rFrame = NULL;
                msg_data = NULL;
            }
            else
            {
                LOG_DEBUG("[TxRxTask]: Rx semaphore comes in, but not frame receive\r\n");
            }

        }

        for (i = PRI_Q_NUM; i >= 0; i--)
        {
            if((outFrm = (FrmQ *)list_q_deq_safe(&tx_hwq[i], &tx_mtx)) != NULL)
            {
                tFrame = outFrm->frame;
#ifndef __SSV_UNIX_SIM__
                if(ssv6xxx_drv_tx_resource_enough(OS_FRAME_GET_DATA_LEN(tFrame),1) == TRUE)
#endif
                {
                    if (ssv6xxx_drv_send(OS_FRAME_GET_DATA(tFrame), OS_FRAME_GET_DATA_LEN(tFrame)) <0)
                    {
                        LOG_PRINTF("%(): ssv6xxx_drv_send() data failed !!\r\n", __FUNCTION__);
                    }
                    os_frame_free(tFrame);
                    OS_MutexLock(tx_mtx);
                    if(list_q_len(&free_frmQ) > TXSEMA_MAX_NUM)
                        OS_MemFree(outFrm);
                    else
                        list_q_qtail(&free_frmQ,(struct ssv_list_q *)outFrm);
                    OS_MutexUnLock(tx_mtx);
                    OS_SemSignal(tx_frm_sphr);
                    tFrame = NULL;
                    outFrm = NULL;
                }
#ifndef __SSV_UNIX_SIM__
                else
                {
                    list_q_insert_safe(&tx_hwq[i],&tx_hwq[i], (struct ssv_list_q *)outFrm, &tx_mtx);
                    outFrm = NULL;
                    break;
                }
#endif
            }
        }
    }
    Task_Done = true;
}
#else
ssv_type_u8* hciAggTxtempbuf=NULL;
ssv_type_u8* hcirxAggtempbuf=NULL;
extern struct vif_flow_ctrl g_vif_fctl[MAX_VIF_NUM];
extern ssv_type_u32 g_RunTaskCount;
void TXRXTask_TxTask(void *args)
{
    void *tFrame = NULL;
    FrmL *outFrm = NULL;
    ssv_type_u16 i = 0, prc_count = 0;
    ssv_type_bool flush_frm = false;
    ssv_type_u32 aggr_n=0,aggr_len=0,t_len=0;
    //void* aggr_buf=NULL;
    ssv_type_u16 sleep_tick=0;
    TX_STATE tst;
    //u32 start_ticks = 0, account_ticks = 0, wakeup_ticks = 0, wakeup_total = 0, wait_ticks = 0, acc_wait = 0;
    //u32 periodic_pktnum = 0;
    LOG_DEBUGF(LOG_TXRX,("TxTask started(%d)\r\n",g_RunTaskCount));
    g_RunTaskCount++;
    while((curr_mode != MT_EXIT)&&(!OS_TaskShallStop()))
    {
        prc_count = 0;
        
        /*
        account_ticks += (OS_GetSysTick() - start_ticks);
        wakeup_total += (OS_GetSysTick() - wakeup_ticks);
        if(account_ticks > 1000)
        {
            LOG_PRINTF("%s: %d pkt is sent in %d ms, wakeup in %d ms, acc_wait %d ms !!\r\n",
                       __FUNCTION__, periodic_pktnum, account_ticks, wakeup_total, acc_wait);
            periodic_pktnum = 0;
            account_ticks = 0;
            wakeup_total = 0;
            acc_wait = 0;
        }
        start_ticks = OS_GetSysTick();*/
        OS_SemWait(tx_frm_sphr, sleep_tick);
        if(curr_mode != MT_RUNNING)
            continue;
        /*wakeup_ticks = OS_GetSysTick();
        wait_ticks = (wakeup_ticks-start_ticks);
        acc_wait += wait_ticks;*/
        do
        {
            t_len =0;
            for (i = 0; i < PRI_Q_NUM; i++)
            {
                //while((outFrm = (FrmL *)llist_pop_safe(&tx_hwq[i], &tx_mtx)) != NULL)
                //while(llist_l_len(&tx_hwq[i])&&(!(llist_l_is_lockout(&tx_hwq[i], &tx_mtx))))
                if(llist_l_len(&tx_hwq[i]))
                {
                    outFrm = (FrmL *)llist_pop_safe(&tx_hwq[i], &tx_mtx);
                    if(!outFrm)
                        continue;
                    tFrame = outFrm->frame;
                    if(tFrame){
#ifndef __SSV_UNIX_SIM__
                        if(g_host_cfg.hci_aggr_tx !=1)
                        {
                            while(1)
                            {
                                //LOG_PRINTF("C=%d\r\n",i);
                                tst = ssv6xxx_drv_tx_resource_enough(OS_FRAME_GET_DATA_LEN(tFrame),1,i);
                                if(tst==KEEP_TRY){
                                    if(g_host_cfg.tx_sleep)
                                    {
                                        //LOG_PRINTF("+(%d)\r\n",i);
                                        if(i == 0 || i == 1){
                                            OS_TickDelay(g_vif_fctl[i].sleep_tick);
                                        }else{
                                            OS_TickDelay(g_host_cfg.tx_sleep_tick);
                                        }
                                    }
                                }
                                else if(tst==NEXT_ONE){
                                    llist_push_head_safe(&tx_hwq[i], (struct ssv_llist *)outFrm, &tx_mtx);
                                    //LOG_PRINTF("len.%d,%d\r\n",llist_l_len(&tx_hwq[0]),llist_l_len(&tx_hwq[1])); 
                                    //if(llist_l_len(&tx_hwq[1])==0)
                                    //    OS_TickDelay(g_host_cfg.tx_sleep_tick);
                                    goto NEXT_TXQ;
                                }
                                else if(tst == PAUSE_TX){
                                    llist_push_head_safe(&tx_hwq[i], (struct ssv_llist *)outFrm, &tx_mtx);
                                    if(i == 0 || i == 1){
                                        OS_TickDelay(g_vif_fctl[i].sleep_tick);
                                    }else{
                                        OS_TickDelay(g_host_cfg.tx_sleep_tick);
                                    }
                                     goto NEXT_TXQ;
                                }else{
                                    //LOG_PRINTF("Go(%d)\r\n",i);
                                    break;
                                }
        					}
                        }
#endif
                        //LOG_PRINTF("%s:  Send tx frame %08x with FrmQ %08X!!\r\n", __FUNCTION__, tFrame, outFrm);
                        OS_MutexLock(tx_mtx);
                        flush_frm = (tx_flush_data_frm_num == 0)?false:true;
                        tx_data_frm_num --;
                        if(flush_frm == true)
                            tx_flush_data_frm_num--;
                        OS_MutexUnLock(tx_mtx);

                        if (flush_frm == false)
                        {
TX_RESEND:
                            if(g_host_cfg.hci_aggr_tx ==1)
                            {
                                if(hciAggTxtempbuf)
                                {
                                    //aggr_buf = (void*)hciAggTxtempbuf;//OS_MemAlloc(MAX_HCI_AGGR_SIZE);
                                    //OS_MemSET(hciAggTxtempbuf,0,MAX_HCI_AGGR_SIZE);
                                    //LOG_PRINTF("Alloc aggr tx buffer=%x\r\n",(u32)aggr_buf);
                                    if((aggr_len+OS_FRAME_GET_DATA_LEN(tFrame))<MAX_HCI_AGGR_SIZE)
                                    {
                                        aggr_n = ssv_hal_process_hci_aggr_tx(tFrame,hciAggTxtempbuf,&aggr_len);
                                    }
                                    
                                    if(aggr_n>4)
                                    {
                                    
                                        while(1)
                                        {
                                            tst = ssv6xxx_drv_tx_resource_enough(aggr_len,aggr_n,0xFFFF);
                                            if((tst==KEEP_TRY)||(tst == PAUSE_TX)){
                                                if(g_host_cfg.tx_sleep)
                                                {
                                                   OS_TickDelay(g_host_cfg.tx_sleep_tick);
                                                }
                                            }
                                            else if(tst==NEXT_ONE){
                                                //Do not thing
                                            }else{
                                                break;
                                            }
                                        }
                                        if(ssv6xxx_drv_send(hciAggTxtempbuf, aggr_len) <0)
                                        {
                                            LOG_PRINTF("%s: ssv6xxx_drv_send() AGGR data failed !!\r\n", __FUNCTION__);
                                        }
                                        //LOG_PRINTF("A T n=%d, len=%d\r\n",aggr_n,aggr_len);
                                        aggr_n=aggr_len=0;
                                        OS_MemSET(hciAggTxtempbuf,0,MAX_HCI_AGGR_SIZE);
                                    }
                                    else
                                    {
                                        sleep_tick=1;
                                    }
                                }
                                else
                                {
                                    LOG_PRINTF("no hci tx buf. Disable HCI TX Aggregation\r\n");
                                    g_host_cfg.hci_aggr_tx=0;
                                    goto TX_RESEND;
                                }
                                
                            }
                            else
                            {
                                ssv_type_u16 len1=OS_FRAME_GET_DATA_LEN(tFrame);
                                ssv_type_u16 len2=*((ssv_type_u16 *)OS_FRAME_GET_DATA(tFrame));
                                if(len1!=len2){
                                    LOG_PRINTF("\33[31m Wrong TX len (len1=%d,len2=%d) \33[0m\r\n",len1,len2);
                                }
                                tx_frame_len[tx_frame_len_count]=len2;
                                tx_frame_len_count++;
                                tx_frame_len_count=tx_frame_len_count%TX_FRAME_ARRAY_SIZE;

                                    
                                if (ssv6xxx_drv_send(OS_FRAME_GET_DATA(tFrame), OS_FRAME_GET_DATA_LEN(tFrame)) <0)
                                {
                                    LOG_DEBUGF(LOG_TXRX|LOG_LEVEL_SERIOUS, ("%s: ssv6xxx_drv_send() data failed !!\r\n", __FUNCTION__));
                                }
        						//LOG_PRINTF("T\r\n");
                            }
                        }

                        os_frame_free(tFrame);
                        outFrm->frame = NULL;
                        //periodic_pktnum++;
                        prc_count++;
                        llist_push_safe(&free_frmQ,(struct ssv_llist *)outFrm, &tx_mtx);

                        tFrame = NULL;
                        outFrm = NULL;

                        if(prc_count > 1)
                        {
                            OS_SemWait(tx_frm_sphr, sleep_tick); //More frames are sent and resource should be pulled out.
                            //LOG_PRINTF("******************%s: push back due to multi-send:%d !!\r\n", __FUNCTION__, tx_count);                            
                            if(curr_mode != MT_RUNNING){
                                sleep_tick = 1;
                                continue;
                            }
                        }

                    }
                    else
                    {
                        LOG_ERROR("outFrm without pbuf?? \r\n");
                    }
                }
NEXT_TXQ:    
                t_len+=llist_l_len(&tx_hwq[i]);
            }
        }while(t_len);

        if(!prc_count)
        {
            if(aggr_n)
            {
                while(1)
                {
                    tst = ssv6xxx_drv_tx_resource_enough(aggr_len,aggr_n,0xFFFF);
                    if((tst==KEEP_TRY)||(tst == PAUSE_TX)){
                        if(g_host_cfg.tx_sleep)
                        {
                           OS_TickDelay(g_host_cfg.tx_sleep_tick);
                        }
                    }
                    else if(tst==NEXT_ONE){
                        //Do not thing
                    }else{
                        break;
                    }
                }
                if (ssv6xxx_drv_send(hciAggTxtempbuf, aggr_len) <0)
                {
                    LOG_PRINTF("%s: ssv6xxx_drv_send() AGGRT data failed !!\r\n", __FUNCTION__);
                }
                OS_MemSET(hciAggTxtempbuf,0,MAX_HCI_AGGR_SIZE);
                //LOG_PRINTF("A T to,n=%d, len=%d\r\n",aggr_n,aggr_len);
                aggr_n=aggr_len=0;
                sleep_tick=0;
            }
        }
    }
    TxTask_Done = true;
    g_RunTaskCount-=1;
    LOG_PRINTF("TX task exit!!(%d)\r\n",g_RunTaskCount);
}

extern ssv_type_u32 g_hw_enable;
//u32 RxTaskRetryCount[32]={0};
ssv_type_u32 chip_interrupt;
extern HOST_API_STATE active_host_api;
ssv_type_u32 rx_task_st=0;
void TXRXTask_RxTask(void *args)
{
    void * rFrame = NULL;
    ssv_type_u8  *msg_data = NULL;
    ssv_type_s32 recv_len = 0;
    ssv_type_s32 retry = 0;
    ssv_type_size_t next_pkt_len = 0;
    
    LOG_DEBUGF(LOG_TXRX,("Rx Task started(%d)\r\n",g_RunTaskCount));
    g_RunTaskCount++;
    while((curr_mode != MT_EXIT)&&(!OS_TaskShallStop()))
    {
        rx_task_st =__LINE__;
        OS_SemWait(rx_frm_sphr, 0);
        rx_task_st =__LINE__;
        if(curr_mode != MT_RUNNING)
        {
            if(curr_mode != MT_EXIT)
            {
                ssv6xxx_drv_irq_enable(false);
            }
            continue;
        }
        for(retry=0;retry<32;retry++)
        {
            if((g_host_cfg.hci_rx_aggr)&&(hcirxAggtempbuf))
            {
                msg_data = hcirxAggtempbuf;
                recv_len = ssv6xxx_drv_recv(msg_data, next_pkt_len);
            }
            else
            {
                if(rFrame == NULL)
                {
                    rx_task_st =__LINE__;
                    #if(SW_8023TO80211==1)
                    while ((rFrame = (ssv_type_u8 *)os_frame_alloc((g_host_cfg.recv_buf_size-g_host_cfg.trx_hdr_len),TRUE)) == NULL)
                    #else
                    while ((rFrame = (ssv_type_u8 *)os_frame_alloc(g_host_cfg.recv_buf_size,TRUE)) == NULL)
                    #endif
                    {
                        OS_TickDelay(1);
                       	LOG_DEBUGF(LOG_TXRX|LOG_LEVEL_WARNING, ("[RxTask]: wakeup from sleep!\r\n"));
                        //continue;
                    }
                    os_frame_push(rFrame, g_host_cfg.trx_hdr_len);
                    msg_data = OS_FRAME_GET_DATA(rFrame);
                }
                recv_len = ssv6xxx_drv_recv(msg_data, OS_FRAME_GET_DATA_LEN(rFrame));
            }
            rx_task_st =__LINE__;

            if(recv_len > 0)
            {
                if((g_host_cfg.hci_rx_aggr)&&(hcirxAggtempbuf))
                {
                    if(curr_mode != MT_RUNNING)
                    {
                        LOG_PRINTF("ignore rx aggr data\r\n");
                        continue;
                    }
                    next_pkt_len = ssv_hal_process_hci_rx_aggr(msg_data,recv_len,(RxPktHdr)TXRXTask_RxFrameProc);
                }
                else
                {
                    if(curr_mode != MT_RUNNING)
                    {
                        LOG_PRINTF("ignore rx frame\r\n");
                        os_frame_free(rFrame);
                        //continue;
                    }
                    else
                    {
                        rx_task_st =__LINE__;
                        TXRXTask_RxFrameProc(rFrame);
                        rx_task_st =__LINE__;
                    }
                }
                rFrame = NULL;
                msg_data = NULL;
            }
            else if(recv_len == 0)
            {
                LOG_DEBUGF(LOG_TXRX|LOG_LEVEL_SERIOUS, ("[TxRxTask]: recv_len == 0 at frame %p, first 8 bytes content is dumpped as below:\r\n", rFrame));
                hex_dump(msg_data, 8);
            }
            else
            {
                //LOG_DEBUG("[TxRxTask]: Rx semaphore comes in, but not frame receive\r\n");
#if(RECOVER_ENABLE == 1)
#if(RECOVER_MECHANISM == 0)
                ssv_type_u32 i;
                if(retry ==0) // when interrupt always high
                {
                    if(TRUE==ssv_hal_get_diagnosis()) // watchdog wack up & reset
                    {
                        ssv_type_u32 fw_status = 0;
                        ssv_hal_get_fw_status(&fw_status);

                        //LOG_PRINTF("***********************************************g_hw_enable:%d fw_status:%08x\r\n", g_hw_enable, fw_status);
                        if(fw_status == 0x5A5AA5A5 ||g_hw_enable == false){
                            break;
                        }

                        ssv_hal_reset_soc_irq();//  system reset interrupt for host

                        for(i=0;i<MAX_VIF_NUM;i++)
                        {
                            if(gDeviceInfo->vif[i].hw_mode == SSV6XXX_HWM_AP){
                                ssv6xxx_wifi_ap_recover(i);
                            }
                            else if(gDeviceInfo->vif[i].hw_mode == SSV6XXX_HWM_STA)
                            {
                                ssv6xxx_wifi_sta_recover(i);
                            }
                        }
                    }
                }
#elif(RECOVER_MECHANISM == 1)
                if(retry ==0)
                {
                    // handle ipc interrupt for check fw ceash
                    //LOG_PRINTF("is heartbeat\r\n");
                    if(1==ssv_hal_is_heartbeat())
                    {
                        //LOG_PRINTF("%08x: host_isr\r\n",isr_status);
                        gDeviceInfo->fw_timer_interrupt ++;
                        chip_interrupt = OS_TICK2MS(OS_GetSysTick());
                        ssv_hal_reset_heartbeat();
                    }
                }
#endif //#if(RECOVER_MECHANISM == 1)
#endif //#if(RECOVER_ENABLE == 1)
                break;
            }
        }
        //RxTaskRetryCount[retry]++;
        rx_task_st =__LINE__;
        ssv6xxx_drv_irq_enable(false);
    }
    RxTask_Done = true;
    g_RunTaskCount-=1;
    LOG_PRINTF("RX task exit!!(%d)\r\n",g_RunTaskCount);

    if(rFrame != NULL)
    {
        os_frame_free(rFrame);
        rFrame = NULL;
    }
}
#endif

void TXRXTask_DeInit(void)
{
    OS_MutexDelete(task_mtx);
    OS_MutexDelete(tx_mtx);
    
    OS_SemDelete(tx_frm_sphr);
    OS_SemDelete(rx_frm_sphr);

    if(g_freeFrmList)
    {
        OS_MemFree(g_freeFrmList);
    }
}
//TXRX_Init Initialize
ssv_type_s32 TXRXTask_Init(void)
{
    ssv_type_u32 i, size, res=OS_SUCCESS;

	curr_mode = MT_STOP;
    tx_flush_data_frm_num = 0;
    tx_data_frm_num = 0;

    g_TXSEMA_MAX_NUM = g_host_cfg.pool_size+g_host_cfg.pool_sec_size+10;
    g_freeFrmList = (FrmL *)OS_MemAlloc(sizeof(FrmL)*g_TXSEMA_MAX_NUM);    
    ssv_assert(g_freeFrmList != NULL);
    
    OS_MemSET((void*)g_freeFrmList, 0, sizeof(FrmL)*g_TXSEMA_MAX_NUM);
    OS_MemSET(&tx_rcs, 0, sizeof(tx_rcs));
    OS_MutexInit(&task_mtx,"task_mtx");
    OS_MutexInit(&tx_mtx,"tx_mtx");

    //if(g_host_cfg.hci_rx_aggr)
    //{        
    //    hcirxAggtempbuf = (u8*)OS_MemAlloc(MAX_HCI_AGGR_SIZE);
    //    assert(hcirxAggtempbuf != NULL);
    //}
    LOG_DEBUGF(LOG_TXRX,("init TX sem_max=%d, RX sem_max=%d\r\n",g_TXSEMA_MAX_NUM,RXSEMA_MAX_NUM));
    ssv_assert(OS_SemInit(&tx_frm_sphr, g_TXSEMA_MAX_NUM, 0)==SSV6XXX_SUCCESS);
    ssv_assert(OS_SemInit(&rx_frm_sphr, RXSEMA_MAX_NUM, 0)==SSV6XXX_SUCCESS);

    _prepare_tx_frm_list();

	size = sizeof(g_txrx_task_info)/sizeof(struct task_info_st);
	for (i = 0; i < size; i++)
    {
		/* Create Registered Task: */
		OS_TaskCreate(g_txrx_task_info[i].task_func,
			g_txrx_task_info[i].task_name,
			g_txrx_task_info[i].stack_size<<4,
			g_txrx_task_info[i].args,
			g_txrx_task_info[i].prio,
			NULL);
	}

    LOG_DEBUGF(LOG_TXRX,("%s(): end !!\r\n", __FUNCTION__));
    return res;
}
ssv_type_s32 TRRXTask_hic_aggr_en(HCI_AGGR_HW trx,ssv_type_bool en)
{
    //LOG_PRINTF("HCI TRX(%d)en(%d)\r\n",trx,en);
#if((CONFIG_CHIP_ID==SSV6006B)||(CONFIG_CHIP_ID==SSV6006C))
    if(trx == SSV_HCI_RX)
    {
        if(en)
        {
            hcirxAggtempbuf = (ssv_type_u8*)OS_MemAlloc(MAX_HCI_AGGR_SIZE);
            if(hcirxAggtempbuf==NULL)
            {
                LOG_PRINTF("RX hci aggr allocate buf fail\r\n");
                return false;
            }
            OS_MemSET((void*)hcirxAggtempbuf,0,MAX_HCI_AGGR_SIZE);
        }
        else
        {
            if(hcirxAggtempbuf){
                OS_MemFree(hcirxAggtempbuf);
                hcirxAggtempbuf = NULL;
            }
        }
        return true;
    }
    else if(trx == SSV_HCI_TX)
    {
        if(en)
        {
            hciAggTxtempbuf = (ssv_type_u8*)OS_MemAlloc(MAX_HCI_AGGR_SIZE);
            if(hciAggTxtempbuf==NULL)
            {
                LOG_PRINTF("TX hci aggr allocate buf fail\r\n");
                return false;
            }
            //LOG_PRINTF("hciAggTxtempbuf=0x%x\r\n",(u32)hciAggTxtempbuf);
            //if((((u32)hciAggTxtempbuf)%4)!=0)
            //    assert(0);
            OS_MemSET((void*)hciAggTxtempbuf,0,MAX_HCI_AGGR_SIZE);
        }
        else
        {
            if(hciAggTxtempbuf){
                //LOG_PRINTF("free hciAggTxtempbuf=0x%x\r\n",(u32)hciAggTxtempbuf);
                OS_MemFree(hciAggTxtempbuf);
                hciAggTxtempbuf = NULL;
            }
        }
        return true;
    }
#endif
    
    return false;
}

//TXRX_FrameEnqueue Enqueue tx frames
ssv_type_s32 TXRXTask_FrameEnqueue(void* frame, ssv_type_u32 priority)
{
    FrmL *dataFrm = NULL;

    CFG_HOST_TXREQ *req = (CFG_HOST_TXREQ *)OS_FRAME_GET_DATA(frame);;
#if(SW_8023TO80211==1)
    if ((curr_mode == MT_STOP) && (M2_TXREQ==ssv_hal_get_txreq0_ctype(req)))
#else
    if ((curr_mode == MT_STOP) && (M0_TXREQ==ssv_hal_get_txreq0_ctype(req)))
#endif
    {
        LOG_DEBUGF(LOG_TXRX|LOG_LEVEL_WARNING, ("[TxRx_task]: No enqueue frame due to mode = stop and frame type = data\r\n"));
        return false;
    }

    OS_MutexLock(tx_mtx);
    dataFrm = (FrmL *)llist_pop(&free_frmQ);
    if (dataFrm == NULL)
    {
        LOG_DEBUGF(LOG_TXRX|LOG_LEVEL_SERIOUS, ("[TxRx_task]:can not get empty llist for dataFrm\r\n"));
        OS_MutexUnLock(tx_mtx);
        return false;
    }


    os_frame_set_debug_flag(frame,SSV_PBUF_DBG_FLAG_L2_TX_DRIVER);

    dataFrm->frame = frame;
    //LOG_DEBUG("[TxRx_task]: Enqueue %d for incoming frame %08x with FrmQ %08x\r\n", priority, frame, dataFrm);
    //LOG_PRINTF("Enqueue vif=%d\r\n",priority);
    tx_data_frm_num++;

    llist_push(&tx_hwq[priority], &dataFrm->_list);
    //LOG_DEBUG("****************[TxRx_task]: size of tx_hwq[priority] = %d\r\n", tx_hwq[priority].qlen);
    OS_MutexUnLock(tx_mtx);

    while(OS_SemSignal(tx_frm_sphr) != 0)
        OS_TickDelay(1);

    return true;
}

//TXRX_SetOpMode Set the operation mode
ssv6xxx_result TXRXTask_SetOpMode(ModeType mode)
{
    ssv6xxx_result ret = SSV6XXX_SUCCESS;

    if(mode > MT_EXIT)
        return SSV6XXX_INVA_PARAM;

    OS_MutexLock(tx_mtx);

    switch (curr_mode)
    {
        case MT_STOP:
        {
            if(mode == MT_EXIT)
            {
                tx_flush_data_frm_num = tx_data_frm_num;
            }
        }
            break;
        case MT_RUNNING:
            if((mode == MT_STOP)||(mode == MT_EXIT))
            {
                //To stop
                tx_flush_data_frm_num = tx_data_frm_num;
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
        curr_mode = mode;

    OS_MutexUnLock(tx_mtx);
    LOG_DEBUGF(LOG_TXRX, ("[TxRx_task]: curr_mode = %d\r\n", curr_mode));

    if(curr_mode == MT_EXIT)
    {
        ssv_type_u32 cur_task_cnt = g_RunTaskCount;
        while(OS_SemSignal(tx_frm_sphr) != 0)//txtask
        {
            OS_TickDelay(1);
            LOG_PRINTF("signal tx task fail!!\r\n");
        }

        OS_TickDelay(1);
        while(g_RunTaskCount!=(cur_task_cnt-1))
        {
            LOG_PRINTF("wait txtsk exit(%d/%d)\r\n",cur_task_cnt,g_RunTaskCount);
            OS_TickDelay(1);
        }
        cur_task_cnt = g_RunTaskCount;
        OS_SemSignal(rx_frm_sphr); //Rxtask
        
        OS_TickDelay(1);
        while(g_RunTaskCount!=(cur_task_cnt-1))
        {
            LOG_PRINTF("wait Rxtsk exit(%d/%d)\r\n",cur_task_cnt,g_RunTaskCount);
            OS_TickDelay(1);
        }
#if 0
        
#if ONE_TR_THREAD
        while(TxTask_Done != true)
#else
        while ((TxTask_Done != true) || (RxTask_Done != true))
#endif
        {
            OS_TickDelay(1);
            LOG_PRINTF("Wait TRX task exit(%d/%d)\r\n",TxTask_Done,RxTask_Done);
        }
#endif    

    }
    return SSV6XXX_SUCCESS;
}

void TXRXTask_Isr(ssv_type_u32 signo,ssv_type_bool isfromIsr)
{
    //LOG_DEBUGF(LOG_TXRX|LOG_LEVEL_WARNING, ("TXRXTask_Isr\r\n"));
    //LOG_PRINTF("isr\r\n");
    if(signo &INT_RX)
    {
        ssv6xxx_drv_irq_disable(isfromIsr);
        if(isfromIsr == TRUE)
        {
            if(OS_SemSignal_FromISR(rx_frm_sphr) !=0)
            {
                //LOG_DEBUGF(LOG_TXRX|LOG_LEVEL_SERIOUS, ("OS_SemSignal_FromISR fail\r\n"));
                LOG_PRINTF("1 RX sem cnt=%d\r\n",OS_SemCntQuery(rx_frm_sphr));
                //ssv6xxx_drv_irq_enable(true);
            }
        }    
        else
        {
            if(OS_SemSignal(rx_frm_sphr) !=0)
            {
                //LOG_DEBUGF(LOG_TXRX|LOG_LEVEL_SERIOUS, ("OS_SemSignal fail\r\n"));
                LOG_PRINTF("2 RX sem cnt=%d\r\n",OS_SemCntQuery(rx_frm_sphr));
                //ssv6xxx_drv_irq_enable(false);
            }
        }
    }
}

extern ssv_type_bool ssv6xxx_drv_irq_status(void);
extern ssv_type_u32 cmdeng_task_st;
void TXRXTask_ShowSt(void)
{
    ssv_type_u8 i=0;
    LOG_DEBUG("[TxRx_task]: curr_mode = %d\r\n %d tx frames in q\r\n %d tx frames need to be flushed\r\n",
              curr_mode, tx_data_frm_num, tx_flush_data_frm_num);

    for(i=0;i<TX_FRAME_ARRAY_SIZE;i++){
        LOG_PRINTF("tx frame %d: len=%d\r\n",i,tx_frame_len[i]);
    }
    LOG_PRINTF("TX sem cnt=%d, RX sem cnt=%d,irq_st=%d,rx_st=%d,cmdeng_task_st=%d\r\n",OS_SemCntQuery(tx_frm_sphr),OS_SemCntQuery(rx_frm_sphr),ssv6xxx_drv_irq_status(),rx_task_st,cmdeng_task_st);
}
ssv_type_u32 ssv6xxx_wifi_get_fw_interrupt_cnt(void)
{
    return gDeviceInfo->fw_timer_interrupt;
}


