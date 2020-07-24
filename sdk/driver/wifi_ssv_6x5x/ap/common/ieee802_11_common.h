/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef IEEE802_11_COMMON_H
#define IEEE802_11_COMMON_H

/* Parsed Information Elements */
struct ieee802_11_elems {
	const ssv_type_u8 *ssid;
	const ssv_type_u8 *supp_rates;
	const ssv_type_u8 *fh_params;
	const ssv_type_u8 *ds_params;
	const ssv_type_u8 *cf_params;
	const ssv_type_u8 *tim;
	const ssv_type_u8 *ibss_params;
	const ssv_type_u8 *challenge;
	const ssv_type_u8 *erp_info;
	const ssv_type_u8 *ext_supp_rates;
	const ssv_type_u8 *wpa_ie;
	const ssv_type_u8 *rsn_ie;
	const ssv_type_u8 *wmm; /* WMM Information or Parameter Element */
	const ssv_type_u8 *wmm_tspec;
	const ssv_type_u8 *wps_ie;
	const ssv_type_u8 *power_cap;
	const ssv_type_u8 *supp_channels;
	const ssv_type_u8 *mdie;
	const ssv_type_u8 *ftie;
	const ssv_type_u8 *timeout_int;
	const ssv_type_u8 *ht_capabilities;
	const ssv_type_u8 *ht_operation;
	const ssv_type_u8 *vendor_ht_cap;
	const ssv_type_u8 *p2p;
	const ssv_type_u8 *link_id;
	const ssv_type_u8 *interworking;
    const ssv_type_u8 *ext_capab;

	ssv_type_u8 ssid_len;
	ssv_type_u8 supp_rates_len;
	ssv_type_u8 fh_params_len;
	ssv_type_u8 ds_params_len;
	ssv_type_u8 cf_params_len;
	ssv_type_u8 tim_len;
	ssv_type_u8 ibss_params_len;
	ssv_type_u8 challenge_len;
	ssv_type_u8 erp_info_len;
	ssv_type_u8 ext_supp_rates_len;
	ssv_type_u8 wpa_ie_len;
	ssv_type_u8 rsn_ie_len;
	ssv_type_u8 wmm_len; /* 7 = WMM Information; 24 = WMM Parameter */
	ssv_type_u8 wmm_tspec_len;
	ssv_type_u8 wps_ie_len;
	ssv_type_u8 power_cap_len;
	ssv_type_u8 supp_channels_len;
	ssv_type_u8 mdie_len;
	ssv_type_u8 ftie_len;
	ssv_type_u8 timeout_int_len;
	ssv_type_u8 ht_capabilities_len;
	ssv_type_u8 ht_operation_len;
	ssv_type_u8 vendor_ht_cap_len;
	ssv_type_u8 p2p_len;
	ssv_type_u8 interworking_len;
    ssv_type_u8 ext_capab_len;
};

typedef enum { ParseOK = 0, ParseUnknown = 1, ParseFailed = -1 } ParseRes;

ParseRes ieee802_11_parse_elems(const ssv_type_u8 *start, ssv_type_size_t len,
				struct ieee802_11_elems *elems,
				int show_errors);
int ieee802_11_ie_count(const ssv_type_u8 *ies, ssv_type_size_t ies_len);
struct wpabuf * ieee802_11_vendor_ie_concat(const ssv_type_u8 *ies, ssv_type_size_t ies_len,
					    ssv_type_u32 oui_type);

#endif /* IEEE802_11_COMMON_H */
