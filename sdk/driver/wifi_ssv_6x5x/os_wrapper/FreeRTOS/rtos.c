/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <rtos.h>

#include <log.h>


volatile ssv_type_u8 gOsFromISR;
#if 0
struct mtx_tbl{
 void* mutex_p;
 char* owenr;
 bool used;
};
#define MAX_MTX_TBL 50
struct mtx_tbl mutextbl[MAX_MTX_TBL];
u32 mtx_cnt=0;
#endif
OS_APIs ssv_type_s32  OS_Init( void )
{
    gOsFromISR = 0;
    //OS_MemSET((void*)mutextbl,0,sizeof(mutextbl));
    return OS_SUCCESS;
}
OS_APIs unsigned long OS_Random(void)
{
	return xTaskGetTickCount()%65536+54*18;
}


OS_APIs void OS_Terminate( void )
{
    vTaskEndScheduler();
}

OS_APIs ssv_type_u32 OS_EnterCritical(void)
{
    vPortEnterCritical();
    return 0;
}

OS_APIs void OS_ExitCritical(ssv_type_u32 val)
{
   vPortExitCritical();
   return;
}

/* Task: */
OS_APIs ssv_type_s32 OS_TaskCreate( OsTask task, const char *name, ssv_type_u32 stackSize, void *param, ssv_type_u32 pri, OsTaskHandle *taskHandle )
{
    long Result;

    Result = xTaskCreate(
            task,                   /* The task to create */
            (const signed char * const)name,     /* Task name */
            stackSize,              /* Stack Size (in WORD (4bytes)) */
            param,                  /* Parameter for Task */
            pri,                    /* Priority: 0 (low) */
            taskHandle);


    return (Result == pdPASS) ? OS_SUCCESS : OS_FAILED;           /* Task Handle */
}


OS_APIs void OS_TaskDelete(OsTaskHandle taskHandle)
{
    vTaskDelete(taskHandle);
}

OS_APIs ssv_type_bool OS_TaskShallStop(void)
{
    return 0;
}

OS_APIs void OS_StartScheduler( void )
{
    vTaskStartScheduler();
}

OS_APIs ssv_type_u32 OS_GetSysTick(void)
{

    return xTaskGetTickCount();
}


/* Mutex APIs: */
OS_APIs ssv_type_s32 OS_MutexInit( OsMutex *mutex , const char* owenr)
{
    *mutex = xSemaphoreCreateMutex();
    if ( NULL == *mutex )
        return OS_FAILED;

#if 0
    {
        u8 i;
        vPortEnterCritical();
        mtx_cnt++;
        ASSERT(mtx_cnt<MAX_MTX_TBL);

        for(i=0;i<MAX_MTX_TBL;i++)
        {
            if(!mutextbl[i].mutex_p)
            {
                mutextbl[i].mutex_p = (void*)(*mutex);
                mutextbl[i].owenr = owenr;
                break;
            }
        }
        if(i>=MAX_MTX_TBL)
            LOG_PRINTF("init mtx_cnt= %d\r\n",mtx_cnt);
        vPortExitCritical();
    }
#endif
    return OS_SUCCESS;
}


OS_APIs void OS_MutexDelete( OsMutex mutex )
{
    vSemaphoreDelete(mutex);
#if 0
    {
        u8 i;
        vPortEnterCritical();
        mtx_cnt--;
        for(i=0;i<MAX_MTX_TBL;i++)
        {
            if(mutextbl[i].mutex_p == (void*)mutex)
            {
                mutextbl[i].mutex_p = NULL;
                mutextbl[i].owenr = NULL;
                break;
            }
        }
        if(i>=MAX_MTX_TBL)
            LOG_PRINTF("del mtx_cnt= %d\r\n",mtx_cnt);
        vPortExitCritical();
    }
#endif
}


OS_APIs void OS_MutexLock( OsMutex mutex )
{
    if(mutex)
        xSemaphoreTake( mutex, portMAX_DELAY);
    else
        ASSERT(0);
#if 0
    {
        u8 retry=0;
    MTX_LOCK:
        if(xSemaphoreTake( mutex, 1) != pdPASS)
        {
            retry++;
            if(retry > 30)
            {
                u32 i;
                for(i=0;i<MAX_MTX_TBL;i++)
                {
                    if(mutextbl[i].mutex_p == (void*)mutex)
                    {
                        LOG_PRINTF("lockmtx %s fail\r\n",mutextbl[i].owenr);
                        break;
                    }
                }
                if(i>=MAX_MTX_TBL)
                    LOG_PRINTF("lock mtx fail but can't not found\r\n");
                retry=0;
            }
            goto MTX_LOCK;
        }
    }
#endif
}



OS_APIs void OS_MutexUnLock( OsMutex mutex )
{
    xSemaphoreGive( mutex );
}


OS_APIs void OS_MsDelay(ssv_type_u32 ms)
{
    ssv_type_u32  ticks = ms / portTICK_RATE_MS;
    if (ticks == 0)
        ticks = 1;
    vTaskDelay(ticks);
}

OS_APIs void OS_TickDelay(ssv_type_u32 ticks)
{
        vTaskDelay(ticks);
}



/* Message Queue: */
OS_APIs ssv_type_s32 OS_MsgQCreate( OsMsgQ *MsgQ, ssv_type_u32 QLen )
{
    *MsgQ = xQueueCreate( QLen, sizeof( OsMsgQEntry ) );
    if ( NULL == *MsgQ )
        return OS_FAILED;
    return OS_SUCCESS;
}


OS_APIs ssv_type_s32 OS_MsgQEnqueue( OsMsgQ MsgQ, OsMsgQEntry *MsgItem, ssv_type_bool fromISR )
{
    ssv_type_s32 err;
    ssv_type_s32_t val;
    ssv_type_u8 rcnt=10;

postQ:
    if (fromISR == false)
        err = xQueueSendToBack( MsgQ, (void *)MsgItem, 0 );
    else
        err = xQueueSendToBackFromISR( MsgQ, (void *)MsgItem, &val );

    if(pdPASS!=err){
        if(rcnt){
            OS_MsDelay(1);
            rcnt--;
            goto postQ;
        }else{
            //LOG_PRINTF("eq fail(%dsgQ=0x%x,rcnt=%d!!\r\n",err,MsgQ,rcnt);
        }
    }


    return ( pdPASS!=err )? OS_FAILED: OS_SUCCESS;
}

OS_APIs ssv_type_s32 OS_MsgQEnqueueTry( OsMsgQ MsgQ, OsMsgQEntry *MsgItem, ssv_type_bool fromISR )
{
    ssv_type_s32 err;
    ssv_type_s32_t val;
    if (fromISR == false)
        err = xQueueSendToBack( MsgQ, (void *)MsgItem, 0UL);
    else
        err = xQueueSendToBackFromISR( MsgQ, (void *)MsgItem, &val );
    return ( pdPASS!=err )? OS_FAILED: OS_SUCCESS;
}


OS_APIs ssv_type_s32 OS_MsgQDequeue( OsMsgQ MsgQ, OsMsgQEntry *MsgItem, ssv_type_u32 timeOut, ssv_type_bool fromISR )
{
    ssv_type_s32 err;
    ssv_type_s32_t val;
	ssv_type_u32 BlockTick;

	//BlockTick = timeOut;
	BlockTick= (timeOut == 0)?portMAX_DELAY : timeOut;

    if (fromISR == false)
        err = xQueueReceive( MsgQ, (void *)MsgItem, BlockTick );
    else
        err = xQueueReceiveFromISR( MsgQ, (void *)MsgItem, &val );
    return ( pdPASS!=err )? OS_FAILED: OS_SUCCESS;
}


OS_APIs ssv_type_s32 OS_MsgQWaitingSize( OsMsgQ MsgQ )
{
    return ( uxQueueMessagesWaiting( MsgQ ) );
}



/* Timer: */
OS_APIs ssv_type_s32 OS_TimerCreate( OsTimer *timer, ssv_type_u32 ms, ssv_type_u8 autoReload, void *args, OsTimerHandler timHandler )
{
#if ( configUSE_TIMERS == 1 )
    ms = ( 0 == ms )? 1: ms;
    *timer = xTimerCreate( NULL, OS_MS2TICK(ms), autoReload, args, timHandler);
    if ( NULL == *timer )
        return OS_FAILED;
#endif

    return OS_SUCCESS;
}

OS_APIs ssv_type_s32 OS_TimerSet( OsTimer timer, ssv_type_u32 ms, ssv_type_u8 autoReload, void *args )
{
#if 0 //( configUSE_TIMERS == 1 )
    if ( pdFAIL == xTimerChangeSetting( timer, OS_MS2TICK(ms), autoReload, args) )
        return OS_FAILED;
#endif
    return OS_SUCCESS;
}

OS_APIs ssv_type_s32 OS_TimerStart( OsTimer timer )
{
#if ( configUSE_TIMERS == 1 )
    return xTimerStart( timer, 0 );
#endif
    return OS_SUCCESS;
}

OS_APIs ssv_type_s32 OS_TimerStop( OsTimer timer )
{
#if ( configUSE_TIMERS == 1 )
    return xTimerStop( timer, 0 );
#endif
    return OS_SUCCESS;
}

OS_APIs void *OS_TimerGetData( OsTimer timer )
{
#if ( configUSE_TIMERS == 1 )
    return pvTimerGetTimerID(timer);
#endif
    return OS_SUCCESS;
}

OS_APIs ssv_type_s32 OS_SemInit( OsSemaphore* Sem , ssv_type_u16 maxcnt , ssv_type_u16 cnt)
{
//	 u8 xReturn = OS_FAILED;
    if ((cnt > maxcnt) || (maxcnt == 0))
        return OS_FAILED;

	(*Sem) = xSemaphoreCreateCounting(maxcnt, cnt);

	if ( NULL != *Sem )
		return OS_SUCCESS;
	else
		return OS_FAILED;
}

OS_APIs ssv_type_bool OS_SemWait( OsSemaphore Sem , ssv_type_u16 timeout)
{
    if(timeout)
        return ((xSemaphoreTake( Sem, timeout)== pdPASS)?OS_SUCCESS:OS_FAILED);

    else
        return ((xSemaphoreTake( Sem, portMAX_DELAY)== pdPASS)?OS_SUCCESS:OS_FAILED);

}

OS_APIs ssv_type_u8 OS_SemSignal( OsSemaphore Sem)
{
	return ((xSemaphoreGive( Sem ) == pdPASS) ? OS_SUCCESS:OS_FAILED);

}

OS_APIs ssv_type_u32 OS_SemCntQuery( OsSemaphore Sem)
{
    return 0;
}

OS_APIs ssv_type_u8 OS_SemSignal_FromISR( OsSemaphore Sem)
{
	return ((xSemaphoreGiveFromISR( Sem , pdFALSE) == pdPASS) ? OS_SUCCESS:OS_FAILED);
}

OS_APIs void OS_SemDelete(OsSemaphore Sem)
{
	vSemaphoreDelete( Sem );
}

OS_APIs ssv_type_s32 OS_MsgQDelete( OsMsgQ MsgQ)
{
    vQueueDelete(MsgQ);
    return OS_SUCCESS;
}

OS_APIs ssv_type_s32 OS_SysProfiling(void *pTextBuf)
{
#if configGENERATE_RUN_TIME_STATS
#ifndef OS_NO_SUPPORT_PROFILING
    if(pTextBuf == NULL)
        return OS_FAILED;

    vTaskGetRunTimeStats(pTextBuf);
#endif
#endif
    return OS_SUCCESS;
}



