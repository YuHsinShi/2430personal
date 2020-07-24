/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <rtos.h>
#include <log.h>
#include <ssv_lib.h>
#include "ssv_timer.h"

OsMutex g_tmr_mutex;
struct ssv_list_q tmr_hd;
struct ssv_list_q free_tmr_hd;
struct ssv_list_q expired_tmr_hd;
extern struct task_info_st g_host_task_info[];
struct os_timer g_ssv_timer[SSV_TMR_MAX];
static ModeType  tmr_mode;

void SSV_Timer_Task( void *args );

struct task_info_st g_timer_task_info[] =
{
    { "ssv_tmr_task",    (OsMsgQ)0, 16,   OS_TMR_TASK_PRIO,   TMR_TASK_STACK_SIZE, NULL, SSV_Timer_Task},
};
#define MBOX_TMR_TASK        g_timer_task_info[0].qevt

ssv_type_s32 os_timer_task_exit(void);

void os_timer_deinit(void)
{
    //Let tmr task exit right here
    os_timer_task_exit();
    OS_MsgQDelete(g_timer_task_info[0].qevt);
    OS_MutexDelete(g_tmr_mutex);
}
void os_timer_init(void)
{
    int i;
    struct os_timer *pOSTimer;

    OS_MutexInit( &g_tmr_mutex ,"g_tmr_mutex");
    list_q_init((struct ssv_list_q *)&tmr_hd);
    list_q_init((struct ssv_list_q *)&free_tmr_hd);
    list_q_init((struct ssv_list_q *)&expired_tmr_hd);

    if (g_timer_task_info[0].qlength> 0) {
        SSV_ASSERT(OS_MsgQCreate(&g_timer_task_info[0].qevt,
        (ssv_type_u32)g_timer_task_info[0].qlength)==OS_SUCCESS);
    }

    /* Create Registered Task: */
    OS_TaskCreate(g_timer_task_info[0].task_func,
    g_timer_task_info[0].task_name,
    g_timer_task_info[0].stack_size<<4,
    g_timer_task_info[0].args,
    g_timer_task_info[0].prio,
    NULL);

    OS_MemSET((void*)&g_ssv_timer,0,sizeof(g_ssv_timer));
    for(i=0;i<SSV_TMR_MAX;i++)
    {
        pOSTimer = g_ssv_timer+i;//(struct os_timer *)OS_MemAlloc(sizeof(struct os_timer));
        list_q_qtail(&free_tmr_hd,(struct ssv_list_q *)pOSTimer);
    }
}

void _create_timer(struct os_timer *pOSTimer, ssv_type_u32 xElapsed)
{
    struct ssv_list_q *qhd = &tmr_hd;
    OS_MutexLock(g_tmr_mutex);

    pOSTimer->en_state = EN_TMR_CNT_DWN;
    if(qhd->qlen > 0)
    {
        struct ssv_list_q *next = qhd->next;
        struct os_timer* tmr_ptr;

        while(next != qhd)
        {
            tmr_ptr = (struct os_timer*)next;
            if(tmr_ptr->msRemian >= pOSTimer->msRemian)
            {
                list_q_insert(qhd,next->prev,(struct ssv_list_q *)pOSTimer);

                //update all timer remain time;
                next = ((struct ssv_list_q *)pOSTimer)->next;				
                while(next != qhd)
                {
                    tmr_ptr = (struct os_timer*)next;
                    if(tmr_ptr->msRemian >= pOSTimer->msRemian)
                        tmr_ptr->msRemian -= pOSTimer->msRemian;

                    next = next->next;
                }
                goto DONE;
            }
            else
            {
                pOSTimer->msRemian -= tmr_ptr->msRemian;
            }
            next = next->next;
        }

        list_q_qtail(qhd,(struct ssv_list_q *)pOSTimer);
    }
    else
    {
        list_q_qtail(qhd,(struct ssv_list_q *)pOSTimer);
    }
DONE:
    //LOG_PRINTF("add TMR(%x):%d,%d,%d,tick=%d\r\n",(u32)pOSTimer,pOSTimer->msRemian,pOSTimer->msTimeout,xElapsed,OS_MS2TICK(pOSTimer->msRemian));
    //LOG_PRINTF("%s %d,%d,%d,%d\r\n",__func__,tmr_hd.qlen,expired_tmr_hd.qlen,free_tmr_hd.qlen,pOSTimer->msRemian);
    OS_MutexUnLock(g_tmr_mutex);

}

void _update_all_timer(ssv_type_u32 xElapsed)
{
    struct ssv_list_q *qhd = &tmr_hd;
    struct ssv_list_q *next = NULL;
    struct os_timer* tmr_ptr;
    MsgEvent *pMsgEv=NULL;
    ssv_type_u32 evt_retry = 10;

    OS_MutexLock(g_tmr_mutex);

    //LOG_PRINTF("\r\nTMR update :%d\r\n",xElapsed);

    next = qhd->next;

    if(qhd->qlen > 0)//while(next != qhd)
    {
        tmr_ptr = (struct os_timer*)next;
        if(tmr_ptr->msRemian > xElapsed)
        {
            if(xElapsed>0)
                tmr_ptr->msRemian -= xElapsed;
            //    LOG_PRINTF("%x:%d,%x\r\n",(u32)tmr_ptr,tmr_ptr->msRemian,tmr_ptr->nMTData0);
            //break; //First one is the smallest one. if the smallest one > elapse ,no one will expired. so break!!
        }
        else
        {
            //LOG_PRINTF("Time's up:%,%d,%d,%x\r\n",tmr_ptr->msTimeout,tmr_ptr->msRemian,tmr_ptr->nMTData0);
            do
            {
                //next = next->next;
                tmr_ptr = (struct os_timer*)list_q_deq(qhd);
                
                //    LOG_PRINTF("Time's up:%x,next->remain=%d,data0=%d\r\n",(u32)tmr_ptr,((struct os_timer*)next)->msRemian,tmr_ptr->nMTData0);
                if(tmr_ptr->handler)
                {
EVT_AGN:
                    pMsgEv=msg_evt_alloc();
                    if(pMsgEv)
                    {
                        pMsgEv->MsgType=tmr_ptr->nMsgType;
                        pMsgEv->MsgData=(ssv_type_u32)tmr_ptr->handler;
                        pMsgEv->MsgData1=tmr_ptr->nMTData0;
                        pMsgEv->MsgData2=tmr_ptr->nMTData1;
                        pMsgEv->MsgData3=(ssv_type_u32)tmr_ptr;
                        if(tmr_ptr->infombx)
                        {
                            tmr_ptr->en_state = EN_TMR_TICK_POST;
                            //OS_MutexUnLock(g_tmr_mutex);
                            while((msg_evt_post((OsMsgQ)tmr_ptr->infombx, pMsgEv)==OS_FAILED)&&(evt_retry))
                            {
                                OS_TickDelay(1);
                                evt_retry--;
                                //LOG_PRINTF("tmr info retry\r\n");
                            }
                            if(evt_retry==0)
                            {
                                tmr_ptr->en_state = EN_TMR_FREE;
                                list_q_qtail(&free_tmr_hd,(struct ssv_list_q *)tmr_ptr);
                                os_msg_free(pMsgEv);
                                LOG_PRINTF("give up tmr info\r\n");
                            }
                            else
                            {                                
                            list_q_qtail(&expired_tmr_hd,(struct ssv_list_q *)tmr_ptr);
                            }
                            //OS_MutexLock(g_tmr_mutex);
                        }
                        else
                        {
                            //OS_MemFree((void*)tmr_ptr);
                            tmr_ptr->en_state = EN_TMR_FREE;
                            list_q_qtail(&free_tmr_hd,(struct ssv_list_q *)tmr_ptr);
                            os_msg_free(pMsgEv);
                            LOG_PRINTF("infombx error\r\n");
                        }
                    }
                    else
                    {
                        //OS_MemFree((void*)tmr_ptr);
                        while(evt_retry--)
                        {
                            OS_TickDelay(1);
                            LOG_PRINTF("tmr alloc evt retry %d\r\n",evt_retry);
                            goto EVT_AGN;
                        }
                        tmr_ptr->en_state = EN_TMR_FREE;
                        list_q_qtail(&free_tmr_hd,(struct ssv_list_q *)tmr_ptr);
                        LOG_PRINTF("tmr alloc evt fail %d\r\n",evt_retry);
                    }
                }
                else
                {
                    tmr_ptr->en_state = EN_TMR_FREE;
                    list_q_qtail(&free_tmr_hd,(struct ssv_list_q *)tmr_ptr);
                    LOG_PRINTF("invalid tmr\r\n");
                    //OS_MemFree((void*)tmr_ptr);
                }
                next = qhd->next;
            }while((next!=qhd)&&(((struct os_timer*)next)->msRemian==0));
        }
    }

    OS_MutexUnLock(g_tmr_mutex);

    //LOG_PRINTF("%s %d,%d,%d\r\n",__func__,tmr_hd.qlen,expired_tmr_hd.qlen,free_tmr_hd.qlen);
}

void _cancel_timer(timer_handler handler, ssv_type_u32 data1, ssv_type_u32 data2)
{
    struct ssv_list_q *qhd = &tmr_hd;
    OS_MutexLock(g_tmr_mutex);

    if(qhd->qlen > 0)
    {
        struct ssv_list_q *next = qhd->next;
        struct os_timer* tmr_ptr;        
        ssv_type_u32   msRemain=0;
        
        while(next != qhd)
        {
            tmr_ptr = (struct os_timer*)next;
            next = next->next;
            if (handler == tmr_ptr->handler &&
                tmr_ptr->nMTData0 == data1 &&
                tmr_ptr->nMTData1 == data2 )
            {
                msRemain = tmr_ptr->msRemian;
                tmr_ptr->handler = NULL;
                tmr_ptr->en_state = EN_TMR_FREE;
                list_q_remove(qhd,(struct ssv_list_q*)tmr_ptr);
                list_q_qtail(&free_tmr_hd,(struct ssv_list_q*)tmr_ptr);

                //re-add remain time for reset timer
                if(msRemain > 0)
                {
                    while(next != qhd)
                    {
                        tmr_ptr = (struct os_timer*)next;
                        tmr_ptr->msRemian += msRemain;                
                        next = next->next;
                    }
                }
                //LOG_PRINTF("cancel_timer:%x\r\n",(u32)tmr_ptr);
                //break;
            }
        }
    }

    qhd = &expired_tmr_hd;

    if(qhd->qlen > 0)
    {
        struct ssv_list_q* next = qhd->next;
        struct os_timer* tmr_ptr;

        while(next != qhd)
        {
            tmr_ptr = (struct os_timer*)next;
            next = next->next;
            if (handler == tmr_ptr->handler &&
                tmr_ptr->nMTData0 == data1 &&
                tmr_ptr->nMTData1 == data2 )
            {
                tmr_ptr->en_state = EN_TMR_CANCEL;
                //LOG_PRINTF("cancel expired timer:%x\r\n",(u32)tmr_ptr);
            }
        }
    }
    OS_MutexUnLock(g_tmr_mutex);
    //LOG_PRINTF("%s %d,%d,%d\r\n",__func__,tmr_hd.qlen,expired_tmr_hd.qlen,free_tmr_hd.qlen);
}

void _free_timer(struct os_timer* free_tmr)
{
    struct ssv_list_q *qhd = &expired_tmr_hd;
    OS_MutexLock(g_tmr_mutex);
    if(qhd->qlen > 0)
    {
        struct ssv_list_q* next = qhd->next;
        struct os_timer* tmr_ptr;

        while(next != qhd)
        {
            tmr_ptr = (struct os_timer*)next;
            next = next->next;
            if(tmr_ptr == free_tmr)
            {
                tmr_ptr->handler = NULL;
                tmr_ptr->en_state = EN_TMR_FREE;
                list_q_remove(qhd,(struct ssv_list_q*)tmr_ptr);
                list_q_qtail(&free_tmr_hd,(struct ssv_list_q*)tmr_ptr);
            }
        }
    }

    OS_MutexUnLock(g_tmr_mutex);
    //LOG_PRINTF("%s,%d,%d,%d\r\n",__func__,tmr_hd.qlen,expired_tmr_hd.qlen,free_tmr_hd.qlen);
}
void SSV_Timer_Task( void *args )
{
    ssv_type_u32 xStartTime, xEndTime, xElapsed;
    MsgEvent *MsgEv = NULL;
    struct os_timer* cur_tmr = NULL;

    //LOG_PRINTF("SSV_Timer_Task Start\r\n");
    tmr_mode = MT_RUNNING;
    while((tmr_mode==MT_RUNNING)&&(!OS_TaskShallStop()))
    {
        xElapsed = 0;
        xStartTime = OS_GetSysTick();
        cur_tmr = (struct os_timer*)tmr_hd.next;
        if(cur_tmr != (struct os_timer*)&tmr_hd)
        {
            //LOG_PRINTF("cur_tmr=%x,%d,%d,xStartTime=%d\r\n",(u32)cur_tmr,cur_tmr->msTimeout,cur_tmr->msRemian,xStartTime);
            if( OS_SUCCESS == msg_evt_fetch_timeout( MBOX_TMR_TASK, &MsgEv,cur_tmr->msRemian))
            {
                xEndTime = OS_GetSysTick();
                xElapsed = ( xEndTime - xStartTime ) * OS_TICK_RATE_MS;
                //LOG_PRINTF("Not timeout wakup,cur_tmr=%x,%d,xElapsed=%d,xEndTime=%d,msgtype=%d\r\n",(u32)cur_tmr,cur_tmr->msRemian,xElapsed,xEndTime,MsgEv->MsgType);
            }
            else //Time out; expire timer
            {
                xElapsed = cur_tmr->msRemian;//( xEndTime - xStartTime ) * OS_TICK_RATE_MS;
            }
        }
        else
        {
            //LOG_PRINTF("\r\nNO TMR\r\n");
            msg_evt_fetch(MBOX_TMR_TASK, &MsgEv); //There's no TMR.Block till get msg.
        }

        xStartTime = OS_GetSysTick();
        //if(xElapsed>0)
        {
            _update_all_timer(xElapsed);
        }

        if(MsgEv)
        {
            //LOG_PRINTF("MsgEv->MsgType=%d\r\n",MsgEv->MsgType);
            switch((enum msgtype_tmr)MsgEv->MsgType)
            {
                case TMR_EVT_CREATE:
                {
                    struct os_timer *pOSTimer = (struct os_timer *)MsgEv->MsgData;
                    _create_timer(pOSTimer,xElapsed);
                }
                break;
                case TMR_EVT_CANCEL:
                {
                    _cancel_timer((timer_handler)MsgEv->MsgData, MsgEv->MsgData1, MsgEv->MsgData2);
                }
                break;
                case TMR_EVT_FREE:
                {
                    _free_timer((struct os_timer*) MsgEv->MsgData);
                }
                break;
                case TMR_EVT_TASK_EXIT:
                {
                    //goto SSV_TMR_EXIT;
                }
                default:
                break;
            }
            os_msg_free(MsgEv);
            MsgEv = NULL;
        }
        xEndTime = OS_GetSysTick();
        xElapsed = ( xEndTime - xStartTime ) * OS_TICK_RATE_MS;
		
        //if(xElapsed>0)
        {
            _update_all_timer(xElapsed);
        }
    }
SSV_TMR_EXIT:    
    //g_RunTaskCount-=1;
    tmr_mode = MT_EXIT;
    LOG_PRINTF("TMR task exit\r\n");                    
}

ssv_type_s32 os_create_timer(ssv_type_u32 ms, timer_handler handler, void *data1, void *data2, void* mbx)
{
    ssv_type_s32 ret = 0;


    struct os_timer *pOSTimer;
    MsgEvent *pMsgEv=NULL;

    //pOSTimer = (struct os_timer *)OS_MemAlloc(sizeof(struct os_timer));
    OS_MutexLock(g_tmr_mutex);
    //LOG_PRINTF("free_tmr_hd len=%d\r\n",free_tmr_hd.qlen);
    pOSTimer = (struct os_timer*)list_q_deq(&free_tmr_hd);
    OS_MutexUnLock(g_tmr_mutex);
    //LOG_PRINTF("create TMR=%x\r\n",(u32)pOSTimer);
    if(pOSTimer)
    {
        pOSTimer->nMsgType = MEVT_HOST_TIMER;
        pOSTimer->handler = handler;
        pOSTimer->nMTData0 = (ssv_type_u32)data1;
        pOSTimer->nMTData1 = (ssv_type_u32)data2;
        pOSTimer->msTimeout = pOSTimer->msRemian = ms;
        pOSTimer->infombx = mbx;

        pMsgEv=msg_evt_alloc();
        if(pMsgEv)
        {
            pMsgEv->MsgType=TMR_EVT_CREATE;
            pMsgEv->MsgData=(ssv_type_u32)pOSTimer;
            pMsgEv->MsgData1=0;
            pMsgEv->MsgData2=0;
            pMsgEv->MsgData3=0;
            while((ret = msg_evt_post(MBOX_TMR_TASK, pMsgEv))==OS_FAILED)
            {
                OS_MsDelay(10);
                LOG_PRINTF("create_timer retry\r\n");
            }
            return ret;
        }
    }
    ret = OS_FAILED;

    return ret;
}

ssv_type_s32 os_cancel_timer(timer_handler handler, ssv_type_u32 data1, ssv_type_u32 data2)
{
    ssv_type_s32 ret = 0;
    MsgEvent *pMsgEv=NULL;

    pMsgEv=msg_evt_alloc();
    if(pMsgEv)
    {
        pMsgEv->MsgType=TMR_EVT_CANCEL;
        pMsgEv->MsgData=(ssv_type_u32)handler;
        pMsgEv->MsgData1=data1;
        pMsgEv->MsgData2=data2;
        pMsgEv->MsgData3=0;
        while((ret = msg_evt_post(MBOX_TMR_TASK, pMsgEv))==OS_FAILED)
        {
            OS_MsDelay(10);
            LOG_PRINTF("cancel_timer retry\r\n");
        }
        return ret;
    }
    else
    {
        return OS_FAILED;
    }
}

ssv_type_s32 os_free_timer(struct os_timer* free_tmr)
{
    //_free_timer(free_tmr);

    ssv_type_s32 ret = 0;
    MsgEvent *pMsgEv=NULL;
    //LOG_PRINTF("%s,%x\r\n",__func__,(u32)free_tmr);
    pMsgEv=msg_evt_alloc();
    if(pMsgEv)
    {
        pMsgEv->MsgType=TMR_EVT_FREE;
        pMsgEv->MsgData=(ssv_type_u32)free_tmr;
        while((ret = msg_evt_post(MBOX_TMR_TASK, pMsgEv))==OS_FAILED)
        {
            OS_MsDelay(10);
            LOG_PRINTF("free_timer retry\r\n");
        }
        return ret;
    }
    else
    {
        LOG_PRINTF("2post evt fail\r\n");
        return OS_FAILED;
    }
}

void os_timer_expired(void* evt)
{
    MsgEvent* MsgEv = (MsgEvent*)evt;
    if(MsgEv)
    {
        struct os_timer* ptmr = (struct os_timer*)MsgEv->MsgData3;
        if(ptmr->en_state == EN_TMR_TICK_POST)
        {
            timer_handler thdr = (timer_handler)MsgEv->MsgData;
            thdr((void*)MsgEv->MsgData1, (void*)MsgEv->MsgData2);
        }
        else
        {
            LOG_PRINTF("invalid tmr state=%d\r\n",ptmr->en_state);
        }
        os_free_timer(ptmr);
    }
}

ssv_type_s32 os_timer_task_exit(void)
{
    //_free_timer(free_tmr);

    ssv_type_s32 ret = 0;
    MsgEvent *pMsgEv=NULL;
    //LOG_PRINTF("%s,%x\r\n",__func__,(u32)free_tmr);
    pMsgEv=msg_evt_alloc();
    if(pMsgEv)
    {
        tmr_mode = MT_STOP;
        pMsgEv->MsgType=TMR_EVT_TASK_EXIT;
        //pMsgEv->MsgData=(u32)free_tmr;
        while((ret = msg_evt_post(MBOX_TMR_TASK, pMsgEv))==OS_FAILED)
        {
            OS_MsDelay(10);
            LOG_PRINTF("timer_task_exit retry");
        }
        while(tmr_mode != MT_EXIT)
        {
            LOG_PRINTF("wait tmr exit..\r\n");
            OS_TickDelay(1);
        }
        return ret;
    }
    else
    {
        LOG_PRINTF("2post evt fail\r\n");
        return OS_FAILED;
    }
}

void test_expired_handler(void* data1, void* data2)
{
    LOG_PRINTF("test_expired_handler %x\r\n",(ssv_type_u32)data1);
}
ssv_type_u32 tmrData1;
void cmd_tmr(ssv_type_s32 argc, char *argv[])
{
    ssv_type_u16 timeout;
    ssv_type_u32 cmd = ssv6xxx_atoi(argv[1]);;
    //u32 data1;
    if(argc > 1)
    {
        switch(cmd)
        {
            case 1: //create timer
                timeout = (ssv_type_u16)ssv6xxx_atoi(argv[2]);
                tmrData1 = OS_Random();
                //LOG_PRINTF("test TMR = %dms,data1=%x\r\n",timeout,tmrData1);
                os_create_timer(timeout,test_expired_handler,(void*)tmrData1,NULL, (void*)MBOX_CMD_ENGINE);
                break;
            case 2: //cancel timer
                LOG_PRINTF("Cancel timer\r\n");
                os_cancel_timer(test_expired_handler,(ssv_type_u32)tmrData1, (ssv_type_u32)0);
                break;
            case 3:
                //LOG_PRINTF("free timer\r\n");
                //os_free_timer(test_expired_handler,tmrData1, 0);
                LOG_PRINTF("%s %d,%d,%d\r\n",__func__,tmr_hd.qlen,expired_tmr_hd.qlen,free_tmr_hd.qlen);
                break;
        }
    }
}

