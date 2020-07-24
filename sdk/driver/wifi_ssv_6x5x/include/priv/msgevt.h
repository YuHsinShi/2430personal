/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _MSGEVT_H_
#define _MSGEVT_H_
#include <rtos.h>
//#include <rtos_def.h>
#include <ssv_types.h>



/**
 *  Define Message Type for MsgType:
 *
 *  Note that we assign ID from 10 due to that ID 0-9 are reserved for 
 *  lwip-tcpip message type.
 *
 *  @ MEVT_MSG_PKTBUFF
 *  @ MEVT_MSG_SOFT_TIMER
 *  @ MEVT_MSG_HOST_TIMER
 */
typedef enum msgevt_type_en
{
    MEVT_PKT_BUF                        = 10,
    MEVT_SOFT_TIMER                         ,
    MEVT_HOST_TIMER                         ,
    MEVT_HOST_CMD                           ,
    MEVT_TX_DONE                            ,
    MEVT_NET_MGR_EVENT                      ,
    MEVT_NET_APP_REQ                        ,
    MEVT_EAPOL                              ,
    MEVT_BKG_TASK                           , // Background task
    MEVT_BCN_CMD							,
//	MEVT_DTIM_EXPIRED						,
    MEVT_BCN_MAX							
} msgevt_type;


struct MsgEvent_st
{
    msgevt_type     MsgType;
    ssv_type_u32             MsgData;
    ssv_type_u32             MsgData1;    
    ssv_type_u32             MsgData2;
    ssv_type_u32             MsgData3;
};

typedef struct MsgEvent_st MsgEvent;
typedef struct MsgEvent_st *PMsgEvent;

/**
* Define Message Event Queue:
* 
* @ MBOX_SOFT_MAC
* @ MBOX_MLME
* @ MBOX_CMD_ENGINE
* @ MBOX_TCPIP
*/

//#define MBOX_CMD_ENGINE         g_soc_task_info[2].qevt

#define MBOX_CMD_ENGINE         g_host_task_info[0].qevt
#define MBOX_SIM_RX_DRIVER      g_host_task_info[1].qevt
#define MBOX_SIM_TX_DRIVER      g_host_task_info[2].qevt
#define MBOX_TIMER              g_timer_task_info[0].qevt
#define MBOX_NETMGR             st_netmgr_task[0].qevt

#define MBOX_TCPIP 


void *msg_evt_alloc_0(void);
void *msg_evt_alloc(void);
void  msg_evt_free(MsgEvent *msgev);
ssv_type_s32   msg_evt_post(OsMsgQ msgevq, MsgEvent *msgev);
ssv_type_s32   msg_evt_fetch(OsMsgQ msgevq, MsgEvent **msgev);
ssv_type_s32   msg_evt_fetch_timeout(OsMsgQ msgevq, MsgEvent **msgev, ssv_type_u32 msTimout);
ssv_type_s32   msg_evt_init(ssv_type_u32 max_evt_cnt);
ssv_type_s32	  msg_evt_post_data1(OsMsgQ msgevq, msgevt_type msg_type, ssv_type_u32 msg_data);
ssv_type_s32   msg_evt_deinit(void);

#endif /* _MSGEVT_H_ */

