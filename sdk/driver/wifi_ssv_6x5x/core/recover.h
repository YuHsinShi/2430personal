/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _RECOVER_H_
#define _RECOVER_H_
#define MAX_RECOVER_COUNT 9999
void ssv6xxx_wifi_ap_recover(ssv_type_u8 vif_idx);
void ssv6xxx_wifi_sta_recover(ssv_type_u8 vif_idx);
void check_watchdog_timer(void *data1, void *data2);

#endif
