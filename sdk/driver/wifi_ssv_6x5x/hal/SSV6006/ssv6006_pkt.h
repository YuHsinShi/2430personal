#ifndef _SSV6006_PKT_H_
#define _SSV6006_PKT_H_

//tx
int ssv6006_hal_dump_txinfo(void *_p);
int ssv6006_hal_get_valid_txinfo_size(void);
int ssv6006_hal_get_txreq0_size(void);
int ssv6006_hal_get_txreq0_ctype(void *p);
int ssv6006_hal_set_txreq0_ctype(void *p,ssv_type_u8 c_type);
int ssv6006_hal_get_txreq0_len(void *p);
int ssv6006_hal_set_txreq0_len(void *p,ssv_type_u32 len);
int ssv6006_hal_get_txreq0_rsvd0(void *p);
int ssv6006_hal_set_txreq0_rsvd0(void *p,ssv_type_u32 val);
int ssv6006_hal_get_txreq0_padding(void *p);
int ssv6006_hal_set_txreq0_padding(void *p, ssv_type_u32 val);
int ssv6006_hal_get_txreq0_qos(void *p);
int ssv6006_hal_get_txreq0_ht(void *p);
int ssv6006_hal_get_txreq0_4addr(void *p);
int ssv6006_hal_get_txreq0_f80211(void *p);
int ssv6006_hal_set_txreq0_f80211(void *p,ssv_type_u8 f80211);
int ssv6006_hal_set_txreq0_bssid_idx(void *p,ssv_type_u8 idx);
int ssv6006_hal_get_txreq0_bssid_idx(void *p);
int ssv6006_hal_get_txreq0_more_data(void *p);
int ssv6006_hal_set_txreq0_more_data(void *p,ssv_type_u8 more_data);
ssv_type_u8 *ssv6006_hal_get_txreq0_qos_ptr(void *_req0);
ssv_type_u8 *ssv6006_hal_get_txreq0_data_ptr(void *_req0);

int ssv6006_hal_tx_8023to80211(void *frame, ssv_type_u32 len, ssv_type_u32 priority,
                                       ssv_type_u16 *qos, ssv_type_u32 *ht, ssv_type_u8 *addr4,
                                       ssv_type_bool f80211, ssv_type_u8 security);

void * ssv6006_hal_fill_txreq0(void *frame, ssv_type_u32 len, ssv_type_u32 priority,
                                            ssv_type_u16 *qos, ssv_type_u32 *ht, ssv_type_u8 *addr4,
                                            ssv_type_bool f80211, ssv_type_u8 security, ssv_type_u8 tx_dscrp_flag, ssv_type_u8 vif_idx);


// rx
int ssv6006_hal_rx_80211to8023(void *_PktRxInfo);
int ssv6006_hal_dump_rxinfo(void *_p);
int ssv6006_hal_get_rxpkt_size(void);
int ssv6006_hal_get_rxpkt_ctype(void *p);
int ssv6006_hal_get_rxpkt_len(void *p);
int ssv6006_hal_get_rxpkt_channel(void *p);
int ssv6006_hal_get_rxpkt_rcpi(void *p);
int ssv6006_hal_set_rxpkt_rcpi(void *p, ssv_type_u32 RCPI);
int ssv6006_hal_get_rxpkt_qos(void *p);
int ssv6006_hal_get_rxpkt_f80211(void *p);
int ssv6006_hal_get_rxpkt_wsid(void *p);
int ssv6006_hal_get_rxpkt_tid(void *p);
int ssv6006_hal_get_rxpkt_seqnum(void *p);
int ssv6006_hal_get_rxpkt_psm(void *p);
ssv_type_u8 *ssv6006_hal_get_rxpkt_qos_ptr(void *_rxpkt);
ssv_type_u8 *ssv6006_hal_get_rxpkt_data_ptr(void *_rxpkt);
int ssv6006_hal_get_rxpkt_data_len(void *rxpkt);
ssv_type_u8  ssv6006_hal_get_rxpkt_bssid_idx(void *rxpkt);
ssv_type_u32 ssv6006_hal_process_hci_rx_aggr(void* pdata, ssv_type_u32 data_length, RxPktHdr cbk_fh);
ssv_type_u32 ssv6006_hal_process_hci_aggr_tx(void* tFrame, void* aggr_buf, ssv_type_u32* aggr_len);
int ssv6006_hal_hci_aggr_en(HCI_AGGR_HW trx, ssv_type_bool en);

#endif //#ifndef _SSV6006_PKT_H_

