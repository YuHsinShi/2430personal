/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _RTOS_H_
#define _RTOS_H_

#include <rtos_def.h>
#include <porting.h>
#include <host_config.h>


#define OS_MemFree(ptr) if((ptr)==NULL){ \
        LOG_PRINTF("Error %s @line %d free null pointer\r\n",__func__,__LINE__); \
    }else{ \
        __OS_MemFree((ptr)); \
    }

/* Define Task Priority: 0 is the lowest priority */
#define OS_CMD_ENG_PRIO         CMD_ENG_PRIORITY
#define OS_RX_TASK_PRIO         WIFI_RX_PRIORITY
#define OS_TX_TASK_PRIO         WIFI_TX_PRIORITY
#define OS_TCP_TASK_PRIO        TCPIP_PRIORITY
#define OS_NETAPP_TASK0_PRIO    NETAPP_PRIORITY
#define OS_NETAPP_TASK1_PRIO    NETAPP_PRIORITY_1
#define OS_NETAPP_TASK2_PRIO    NETAPP_PRIORITY_2
#define OS_NETAPP_TASK3_PRIO    NETAPP_PRIORITY_3
#define OS_NETAPP_TASK4_PRIO    NETAPP_PRIORITY_4
#define OS_DHCPD_TASK_PRIO      DHCPD_PRIORITY
#define OS_NETMGR_TASK_PRIO     NETMGR_PRIORITY
#define OS_TMR_TASK_PRIO        TMR_TASK_PRIORITY
#define OS_MLME_TASK_PRIO       MLME_TASK_PRIORITY
#define OS_RX_ISR_PRIO          RX_ISR_PRIORITY

//#define OS_NETAPP_TASK3_PRIO    NETAPP_PRIORITY+3

//#define OS_TASK_END_PRIO        OS_NETAPP_TASK2_PRIO

#define OS_CLI_PRIO             TASK_END_PRIO

#ifdef FGTEST_STACK_CHECK_VALUE
#define RTOS_STACK_CHECK_VALUE FGTEST_STACK_CHECK_VALUE
#else
#define RTOS_STACK_CHECK_VALUE 0x0
#endif

/* Define OS error values */
#define OS_SUCCESS                  0
#define OS_FAILED                   1

/* Message Commands: */
#define OS_MSG_FRAME_TRAPPED        1

//#define OS_TICKTYPE portTickType //freertos
#define OS_TICKTYPE unsigned long
//#define OS_TICK_RATE_MS (1000/OS_TICKS_PER_SEC) //freertos:portTICK_RATE_MS
#define OS_TICK_RATE_MS         TICK_RATE_MS
#define OS_TICK_HZ              (1000/OS_TICK_RATE_MS)
#define OS_MIN_STK_SIZE         TASK_IDLE_STK_SIZE //freerots:configMINIMAL_STACK_SIZE

#define OS_MS2TICK(_ms)         ( (_ms)/TICK_RATE_MS )
#define OS_TICK2MS(_tick)       ((_tick)*TICK_RATE_MS)


#ifndef time_after
#define time_after(a,b)		((long)(b) - (long)(a) < 0)
#endif

#ifndef time_before
#define time_before(a,b)	time_after(b,a)
#endif

#ifndef time_after_eq
#define time_after_eq(a,b)	((long)(a) - (long)(b) >= 0)
#endif

#ifndef time_before_eq
#define time_before_eq(a,b)	time_after_eq(b,a)
#endif
/*
 * Calculate whether a is in the range of [b, c].
 */
#ifndef time_in_range
#define time_in_range(a,b,c) (time_after_eq(a,b) && time_before_eq(a,c))
#endif
/*
 * Calculate whether a is in the range of [b, c).
 */
#ifndef time_in_range_open
#define time_in_range_open(a,b,c) (time_after_eq(a,b) && time_before(a,c))
#endif

/*
 * These four macros compare jiffies and 'a' for convenience.
 */

/* time_is_before_jiffies(a) return true if a is before jiffies */
#ifndef time_is_before_jiffies
#define time_is_before_jiffies(a) time_after(OS_GetSysTick(), a)
#endif

/* time_is_after_jiffies(a) return true if a is after jiffies */
#ifndef time_is_after_jiffies
#define time_is_after_jiffies(a) time_before(OS_GetSysTick(), a)
#endif

/* time_is_before_eq_jiffies(a) return true if a is before or equal to jiffies*/
#ifndef time_is_before_eq_jiffies
#define time_is_before_eq_jiffies(a) time_after_eq(OS_GetSysTick(), a)
#endif

/* time_is_after_eq_jiffies(a) return true if a is after or equal to jiffies*/
#ifndef time_is_after_eq_jiffies
#define time_is_after_eq_jiffies(a) time_before_eq(OS_GetSysTick(), a)
#endif


typedef struct OsMsgQEntry_st
{
	ssv_type_u32         MsgCmd;
    void        *MsgData;

} OsMsgQEntry, *POsMsgQEntry;


typedef struct task_info_st
{
    const char   *task_name;
    OsMsgQ      qevt;
    ssv_type_u8          qlength;
    ssv_type_u8          prio;
    ssv_type_u8          stack_size;  /* unit: 16 */
    void        *args;
    TASK_FUNC   task_func;

} task_info;


/**
 *  Flag to indicate whether ISR handler is running or not.
 */
extern volatile ssv_type_u8 gOsFromISR;

OS_APIs ssv_type_s32 OS_Init( void );
OS_APIs unsigned long OS_Random(void);
OS_APIs ssv_type_u32 OS_GetSysTick(void);

OS_APIs ssv_type_u32 OS_EnterCritical(void);
OS_APIs void OS_ExitCritical(ssv_type_u32 val);


/* Task: */
OS_APIs ssv_type_s32  OS_TaskCreate( TASK_FUNC task, const char *name, ssv_type_u32 stackSize, void *param, ssv_type_u32 pri, OsTaskHandle *taskHandle );
OS_APIs void OS_TaskDelete( OsTaskHandle taskHandle );
OS_APIs void OS_StartScheduler( void );
OS_APIs void OS_Terminate( void );
OS_APIs ssv_type_bool OS_TaskShallStop(void);



/* Mutex: */
OS_APIs ssv_type_s32 OS_MutexInit( OsMutex *mutex , const char* owenr);
OS_APIs void OS_MutexLock( OsMutex mutex );
OS_APIs void OS_MutexUnLock( OsMutex mutex );
OS_APIs void OS_MutexDelete( OsMutex mutex );

/*semaphore*/
OS_APIs ssv_type_s32 OS_SemInit( OsSemaphore* Sem , ssv_type_u16 maxcnt , ssv_type_u16 cnt);
OS_APIs ssv_type_bool OS_SemWait( OsSemaphore Sem , ssv_type_u16 timeout);
OS_APIs ssv_type_u8 OS_SemSignal( OsSemaphore Sem);
OS_APIs ssv_type_u8 OS_SemSignal_FromISR( OsSemaphore Sem);
OS_APIs void OS_SemDelete(OsSemaphore Sem);
OS_APIs ssv_type_u32 OS_SemCntQuery( OsSemaphore Sem);

/* Delay: */
OS_APIs void OS_MsDelay(ssv_type_u32 ms);
OS_APIs void OS_TickDelay(ssv_type_u32 ticks);


/* Timer: */
OS_APIs ssv_type_s32 OS_TimerCreate( OsTimer *timer, ssv_type_u32 ms, ssv_type_u8 autoReload, void *args, OsTimerHandler timHandler );
OS_APIs ssv_type_s32 OS_TimerSet( OsTimer timer, ssv_type_u32 ms, ssv_type_u8 autoReload, void *args );
OS_APIs ssv_type_s32 OS_TimerStart( OsTimer timer );
OS_APIs ssv_type_s32 OS_TimerStop( OsTimer timer );
OS_APIs void *OS_TimerGetData( OsTimer timer );

//OS_APIs void OS_TimerGetSetting( OsTimer timer, u8 *autoReload, void **args );
//OS_APIs ssv_type_bool OS_TimerIsRunning( OsTimer timer );



/* Message Queue: */
OS_APIs ssv_type_s32 OS_MsgQCreate( OsMsgQ *MsgQ, ssv_type_u32 QLen );
OS_APIs ssv_type_s32 OS_MsgQDelete( OsMsgQ MsgQ);
OS_APIs ssv_type_s32 OS_MsgQEnqueue( OsMsgQ MsgQ, OsMsgQEntry *MsgItem, ssv_type_bool fromISR );
OS_APIs ssv_type_s32 OS_MsgQDequeue( OsMsgQ MsgQ, OsMsgQEntry *MsgItem, ssv_type_u32 timeOut, ssv_type_bool fromISR );
OS_APIs ssv_type_s32 OS_MsgQEnqueueTry( OsMsgQ MsgQ, OsMsgQEntry *MsgItem, ssv_type_bool fromISR );
OS_APIs ssv_type_s32 OS_MsgQWaitingSize( OsMsgQ MsgQ );
#if 0
OS_APIs void *OS_MsgAlloc( void );
OS_APIs void OS_MsgFree( void *Msg );
#endif

/* Profiling: */
#ifdef __SSV_UNIX_SIM__
OS_APIs ssv_type_s32 OS_SysProfiling( void *ptr);
#endif
#endif /* _RTOS_H_ */

