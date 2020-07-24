/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _SSV_TIMER_H_
#define _SSV_TIMER_H_
#include "ssv_lib.h"

typedef void (*timer_handler)(void *, void *);


typedef enum
{
    EN_TMR_FREE,
    EN_TMR_CNT_DWN,
    EN_TMR_TICK_POST,
    EN_TMR_CANCEL     
}EN_TMR_STATE;


typedef struct os_timer
{
    struct ssv_list_q tmr_list;
    ssv_type_u32             nMsgType;
    timer_handler   handler;
    void*           infombx;
    ssv_type_u32             nMTData0;
    ssv_type_u32             nMTData1;
    ssv_type_u32             msTimeout;
    ssv_type_u32             msRemian;
    EN_TMR_STATE    en_state;
} os_timer_st;

enum msgtype_tmr
{
    TMR_EVT_CREATE=0,
    TMR_EVT_CANCEL,
    TMR_EVT_FREE,
    TMR_EVT_FLUSH,
    TMR_EVT_TASK_EXIT
};

//Timer related
ssv_type_s32 os_create_timer(ssv_type_u32 ms, timer_handler handler, void *data1, void *data2, void* mbx);
ssv_type_s32 os_cancel_timer(timer_handler handler, ssv_type_u32 data1, ssv_type_u32 data2);
ssv_type_s32 os_free_timer(struct os_timer* free_tmr);
void os_timer_expired(void* evt);
ssv_type_s32 os_timer_task_exit(void);

#endif
