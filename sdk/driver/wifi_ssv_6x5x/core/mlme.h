/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _MLME_TASK_H
#define _MLME_TASK_H

#include <host_apis.h>

#if(MLME_TASK==1)
#define MBOX_MLME_TASK       g_mlme_task_info[0].qevt

extern ssv_type_s32 mlme_host_event_handler_(struct cfg_host_event *pPktInfo);

void mlme_task( void *args );
ssv_type_s32 mlme_init(void);
void mlme_sta_mode_init(void);

#endif

#define RSSI_SMOOTHING_SHIFT        5
#define RSSI_DECIMAL_POINT_SHIFT    6

extern void sta_mode_ap_list_handler (void *data,struct ssv6xxx_ieee80211_bss *ap_list);
void mlme_sta_mode_deinit(void);
void mlme_clear_ap_list(void);
ssv_type_bool mlme_remove_ap(struct ssv6xxx_ieee80211_bss *bss);

#endif //_MLME_TASK_H
