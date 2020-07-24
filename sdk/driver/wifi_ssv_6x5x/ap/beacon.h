/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef BEACON_H
#define BEACON_H

#include "ap_info.h"

struct ieee80211_mgmt;

void ieee802_11_set_beacon(ApInfo_st *pApInfo, ssv_type_bool post_to_mlme);

ssv_type_u8 * hostapd_eid_ds_params(ApInfo_st *pApInfo, ssv_type_u8 *eid);
ssv_type_u8 * hostapd_eid_country(ApInfo_st *pApInfo, ssv_type_u8 *eid,int max_len);
ssv_type_u8 * hostapd_eid_secondary_channel(ApInfo_st *pApInfo, ssv_type_u8 *eid);
ssv_type_u8 * hostapd_eid_csa(ApInfo_st *pApInfo, ssv_type_u8 *eid);
ssv_type_u8 * hostapd_eid_erp_info(ApInfo_st *pApInfo, ssv_type_u8 *eid);
ssv_type_u8 * hostapd_eid_wpa(ApInfo_st *pApInfo, ssv_type_u8 *eid, ssv_type_size_t len);
#endif /* BEACON_H */
