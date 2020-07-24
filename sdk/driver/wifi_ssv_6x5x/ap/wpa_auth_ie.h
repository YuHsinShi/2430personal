/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef WPA_AUTH_IE_H
#define WPA_AUTH_IE_H
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

struct wpa_eapol_ie_parse {
	const ssv_type_u8 *wpa_ie;
	ssv_type_size_t wpa_ie_len;
	const ssv_type_u8 *rsn_ie;
	ssv_type_size_t rsn_ie_len;
	const ssv_type_u8 *pmkid;
	const ssv_type_u8 *gtk;
	ssv_type_size_t gtk_len;
	const ssv_type_u8 *mac_addr;
	ssv_type_size_t mac_addr_len;
#ifdef CONFIG_PEERKEY
	const ssv_type_u8 *smk;
	ssv_type_size_t smk_len;
	const ssv_type_u8 *nonce;
	ssv_type_size_t nonce_len;
	const ssv_type_u8 *lifetime;
	ssv_type_size_t lifetime_len;
	const ssv_type_u8 *error;
	ssv_type_size_t error_len;
#endif /* CONFIG_PEERKEY */
#ifdef CONFIG_IEEE80211W
	const ssv_type_u8 *igtk;
	ssv_type_size_t igtk_len;
#endif /* CONFIG_IEEE80211W */
#ifdef CONFIG_IEEE80211R
	const ssv_type_u8 *mdie;
	ssv_type_size_t mdie_len;
	const ssv_type_u8 *ftie;
	ssv_type_size_t ftie_len;
#endif /* CONFIG_IEEE80211R */
};

int wpa_parse_kde_ies(const ssv_type_u8 *buf, ssv_type_size_t len,
		      struct wpa_eapol_ie_parse *ie);
ssv_type_u8 * wpa_add_kde(ssv_type_u8 *pos, ssv_type_u32 kde, const ssv_type_u8 *data, ssv_type_size_t data_len,
		 const ssv_type_u8 *data2, ssv_type_size_t data2_len);
int wpa_auth_gen_wpa_ie(struct wpa_authenticator *wpa_auth);

#endif /* WPA_AUTH_IE_H */
