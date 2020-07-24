/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _AP_MLME_H_
#define _AP_MLME_H_

#include <pbuf.h>

/**
 *  Define MLME error numbers:
 */
#define AP_MLME_OK                                  0
#define AP_MLME_FAILED                             -1
#define AP_MLME_TIMEOUT                            -2

#define IS_AP_IN_5G_BAND() IS_5G_BAND(gDeviceInfo->APInfo->nCurrentChannel)
#define IS_40MHZ_LEGAL_IN_THIS_COUNTRY() (TRUE==ssv6xxx_wifi_is_40MHZ_legal(gDeviceInfo->APInfo->nCurrentChannel))
#define IS_AP_HT20_40() (g_host_cfg.ap_bw==AP_HT2040)
#define IS_AP_HT40_ONLY() (g_host_cfg.ap_bw==AP_HT40_ONLY)
#define IS_40MHZ_AVAILABLE() ((IS_AP_HT40_ONLY())||(IS_AP_IN_5G_BAND()&&IS_40MHZ_LEGAL_IN_THIS_COUNTRY()&&IS_AP_HT20_40()))

struct ApInfo;

typedef ssv_type_s32 (*AP_MGMT80211_RxHandler)(struct ApInfo*, struct ieee80211_mgmt *mgmt, ssv_type_u16 len, ssv_type_u8 bssid_idx);

extern AP_MGMT80211_RxHandler AP_RxHandler[];

int i802_sta_deauth(const ssv_type_u8 *own_addr, const ssv_type_u8 *addr, int reason);
int i802_sta_disassoc(const ssv_type_u8 *own_addr, const ssv_type_u8 *addr, int reason);
int nl80211_poll_client(const ssv_type_u8 *own_addr, const ssv_type_u8 *addr, int qos);
ssv_type_s32 send_deauth_and_remove_sta(ssv_type_u8 *hwaddr, ssv_type_bool deauth);


#endif /* _AP_MLME_H_ */

