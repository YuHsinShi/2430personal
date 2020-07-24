/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef IEEE802_11_H
#define IEEE802_11_H

#include "ap_info.h"
#include "ap_sta_info.h"
//struct hostapd_iface;
//struct ApInfo_st;
//struct sta_info;
//struct hostapd_frame_info;
//struct ieee80211_ht_capabilities;


//void ieee802_11_mgmt(struct hostapd_data *hapd, const u8 *buf, size_t len,
//		     struct hostapd_frame_info *fi);
//void ieee802_11_mgmt_cb(struct hostapd_data *hapd, const u8 *buf, size_t len,
//			u16 stype, int ok);
void ieee802_11_print_ssid(char *buf, const ssv_type_u8 *ssid, ssv_type_u8 len);

//int ieee802_11_get_mib(struct hostapd_data *hapd, char *buf, size_t buflen);
//int ieee802_11_get_mib_sta(struct hostapd_data *hapd, struct sta_info *sta,
//			   char *buf, size_t buflen);
//
ssv_type_u16 hostapd_own_capab_info(ApInfo_st *pApInfo ,APStaInfo_st *sta, int probe);
//u16 hostapd_own_capab_info(struct hostapd_data *hapd, struct sta_info *sta,
//			   int probe);
ssv_type_u8 * hostapd_eid_supp_rates(ApInfo_st *pApInfo, ssv_type_u8 *eid);
ssv_type_u8 * hostapd_eid_ext_supp_rates(ApInfo_st *pApInfo, ssv_type_u8 *eid);
//u8 * hostapd_eid_ht_capabilities(struct hostapd_data *hapd, u8 *eid);
//u8 * hostapd_eid_ht_operation(struct hostapd_data *hapd, u8 *eid);
//int hostapd_ht_operation_update(struct hostapd_iface *iface);
//void ieee802_11_send_sa_query_req(struct hostapd_data *hapd,
//				  const u8 *addr, const u8 *trans_id);
//void hostapd_get_ht_capab(struct hostapd_data *hapd,
//			  struct ieee80211_ht_capabilities *ht_cap,
//			  struct ieee80211_ht_capabilities *neg_ht_cap);
//u16 copy_sta_ht_capab(struct sta_info *sta, const u8 *ht_capab,
//		      size_t ht_capab_len);
//void update_ht_state(struct hostapd_data *hapd, struct sta_info *sta);
//void hostapd_tx_status(struct hostapd_data *hapd, const u8 *addr,
//		       const u8 *buf, size_t len, int ack);
//void ieee802_11_rx_from_unknown(struct hostapd_data *hapd, const u8 *src,
//				int wds);


int hostapd_get_aid(ApInfo_st *pApInfo, APStaInfo_st *sta);

ssv_type_u16 check_assoc_ies(ApInfo_st *pApInfo, APStaInfo_st *sta,
	const ssv_type_u8 *ies, ssv_type_size_t ies_len, int reassoc);



#endif /* IEEE802_11_H */
