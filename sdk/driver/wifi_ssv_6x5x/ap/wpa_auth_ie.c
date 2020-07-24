/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/

#include "common/ieee802_11_defs.h"
#include "common/wpa_common.h"

#include "ap_config.h"
#include "wpa_auth.h"
#include "wpa_auth_i.h"
#include "wpa_auth_ie.h"

#include "log.h"
#include "wpa_debug.h"

#if (AP_MODE_ENABLE == 1)
//#ifdef CONFIG_RSN_TESTING
//int rsn_testing = 0;
//#endif /* CONFIG_RSN_TESTING */

#ifndef CONFIG_NO_WPA2
static int wpa_write_wpa_ie(struct wpa_auth_config *conf, ssv_type_u8 *buf, ssv_type_size_t len)
{
	struct wpa_ie_hdr *hdr;
	int num_suites;
	ssv_type_u8 *pos, *count;

	hdr = (struct wpa_ie_hdr *) buf;
	hdr->elem_id = WLAN_EID_VENDOR_SPECIFIC;
	RSN_SELECTOR_PUT(hdr->oui, WPA_OUI_TYPE);
	WPA_PUT_LE16(hdr->version, WPA_VERSION);
	pos = (ssv_type_u8 *) (hdr + 1);

	if (conf->wpa_group == WPA_CIPHER_CCMP) {
		RSN_SELECTOR_PUT(pos, WPA_CIPHER_SUITE_CCMP);
	} else if (conf->wpa_group == WPA_CIPHER_TKIP) {
		RSN_SELECTOR_PUT(pos, WPA_CIPHER_SUITE_TKIP);
	} else if (conf->wpa_group == WPA_CIPHER_WEP104) {
		RSN_SELECTOR_PUT(pos, WPA_CIPHER_SUITE_WEP104);
	} else if (conf->wpa_group == WPA_CIPHER_WEP40) {
		RSN_SELECTOR_PUT(pos, WPA_CIPHER_SUITE_WEP40);
	} else {
		LOG_DEBUG( "Invalid group cipher (%d).",
			   conf->wpa_group);
		return -1;
	}
	pos += WPA_SELECTOR_LEN;

	num_suites = 0;
	count = pos;
	pos += 2;

	if (conf->wpa_pairwise & WPA_CIPHER_CCMP) {
		RSN_SELECTOR_PUT(pos, WPA_CIPHER_SUITE_CCMP);
		pos += WPA_SELECTOR_LEN;
		num_suites++;
	}
	if (conf->wpa_pairwise & WPA_CIPHER_TKIP) {
		RSN_SELECTOR_PUT(pos, WPA_CIPHER_SUITE_TKIP);
		pos += WPA_SELECTOR_LEN;
		num_suites++;
	}
	if (conf->wpa_pairwise & WPA_CIPHER_NONE) {
		RSN_SELECTOR_PUT(pos, WPA_CIPHER_SUITE_NONE);
		pos += WPA_SELECTOR_LEN;
		num_suites++;
	}

	if (num_suites == 0) {
		LOG_DEBUG( "Invalid pairwise cipher (%d).",
			   conf->wpa_pairwise);
		return -1;
	}
	WPA_PUT_LE16(count, num_suites);

	num_suites = 0;
	count = pos;
	pos += 2;

	if (conf->wpa_key_mgmt & WPA_KEY_MGMT_IEEE8021X) {
		RSN_SELECTOR_PUT(pos, WPA_AUTH_KEY_MGMT_UNSPEC_802_1X);
		pos += WPA_SELECTOR_LEN;
		num_suites++;
	}
	if (conf->wpa_key_mgmt & WPA_KEY_MGMT_PSK) {
		RSN_SELECTOR_PUT(pos, WPA_AUTH_KEY_MGMT_PSK_OVER_802_1X);
		pos += WPA_SELECTOR_LEN;
		num_suites++;
	}

	if (num_suites == 0) {
		LOG_DEBUG ("Invalid key management type (%d).",
			   conf->wpa_key_mgmt);
		return -1;
	}
	WPA_PUT_LE16(count, num_suites);

	/* WPA Capabilities; use defaults, so no need to include it */

	hdr->len = (pos - buf) - 2;

	return pos - buf;
}


int wpa_write_rsn_ie(struct wpa_auth_config *conf, ssv_type_u8 *buf, ssv_type_size_t len,
		     const ssv_type_u8 *pmkid)
{
	struct rsn_ie_hdr *hdr;
	int num_suites;
	ssv_type_u8 *pos, *count;
	ssv_type_u16 capab;

	hdr = (struct rsn_ie_hdr *) buf;
	hdr->elem_id = WLAN_EID_RSN;
	WPA_PUT_LE16(hdr->version, RSN_VERSION);
	pos = (ssv_type_u8 *) (hdr + 1);
	if (conf->wpa_group == WPA_CIPHER_CCMP) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_CCMP);
	} else if (conf->wpa_group == WPA_CIPHER_TKIP) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_TKIP);
	} else if (conf->wpa_group == WPA_CIPHER_WEP104) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_WEP104);
	} else if (conf->wpa_group == WPA_CIPHER_WEP40) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_WEP40);
	} else {
		LOG_DEBUG( "Invalid group cipher (%d).",
			   conf->wpa_group);
		return -1;
	}
	pos += RSN_SELECTOR_LEN;
	num_suites = 0;
	count = pos;
	pos += 2;
	
	if (conf->rsn_pairwise & WPA_CIPHER_CCMP) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_CCMP);
		pos += RSN_SELECTOR_LEN;
		num_suites++;
	}
	if (conf->rsn_pairwise & WPA_CIPHER_TKIP) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_TKIP);
		pos += RSN_SELECTOR_LEN;
		num_suites++;
	}
	if (conf->rsn_pairwise & WPA_CIPHER_NONE) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_NONE);
		pos += RSN_SELECTOR_LEN;
		num_suites++;
	}


	if (num_suites == 0) {
		LOG_DEBUG( "Invalid pairwise cipher (%d).",
			   conf->rsn_pairwise);
		return -1;
	}
	WPA_PUT_LE16(count, num_suites);

	num_suites = 0;
	count = pos;
	pos += 2;



	if (conf->wpa_key_mgmt & WPA_KEY_MGMT_IEEE8021X) {
		RSN_SELECTOR_PUT(pos, RSN_AUTH_KEY_MGMT_UNSPEC_802_1X);
		pos += RSN_SELECTOR_LEN;
		num_suites++;
	}
	if (conf->wpa_key_mgmt & WPA_KEY_MGMT_PSK) {
		RSN_SELECTOR_PUT(pos, RSN_AUTH_KEY_MGMT_PSK_OVER_802_1X);
		pos += RSN_SELECTOR_LEN;
		num_suites++;
	}

	if (num_suites == 0) {
		LOG_DEBUG("Invalid key management type (%d).",
			   conf->wpa_key_mgmt);
		return -1;
	}
	WPA_PUT_LE16(count, num_suites);

	/* RSN Capabilities */
	capab = 0;
	if (conf->rsn_preauth)
		capab |= WPA_CAPABILITY_PREAUTH;
	if (conf->peerkey)
		capab |= WPA_CAPABILITY_PEERKEY_ENABLED;
	if (conf->wmm_enabled) {
		/* 4 PTKSA replay counters when using WMM */
		capab |= (RSN_NUM_REPLAY_COUNTERS_16 << 2);
	}

	WPA_PUT_LE16(pos, capab);
	pos += 2;

	if (pmkid) {
		if (pos + 2 + PMKID_LEN > buf + len)
			return -1;
		/* PMKID Count */
		WPA_PUT_LE16(pos, 1);
		pos += 2;
		OS_MemCPY(pos, pmkid, PMKID_LEN);
		pos += PMKID_LEN;
	}


	hdr->len = (pos - buf) - 2;
	return pos - buf;
}


int wpa_auth_gen_wpa_ie(struct wpa_authenticator *wpa_auth)
{
	ssv_type_u8 *pos, buf[128];
	int res;
	pos = buf;
	
	
	if (wpa_auth->conf->wpa & WPA_PROTO_RSN)
	{
		res = wpa_write_rsn_ie(wpa_auth->conf,
				       pos, buf + sizeof(buf) - pos, NULL);
		if (res < 0)
			return res;
		pos += res;
	}

	if (wpa_auth->conf->wpa & WPA_PROTO_WPA) {
        
		res = wpa_write_wpa_ie(wpa_auth->conf,
				       pos, buf + sizeof(buf) - pos);
		if (res < 0)
			return res;
		pos += res;
	}
    //if(wpa_auth->wpa_ie == NULL)
	//    wpa_auth->wpa_ie = (u8 *)OS_MemAlloc(pos - buf);
	//if (wpa_auth->wpa_ie == NULL)
	//	return -1;
	
	OS_MemCPY(wpa_auth->wpa_ie, buf, pos - buf);
	//wpa_auth->wpa_ie_len = pos - buf;
    if(pos - buf != WPA_IE_LEN)
        LOG_FATAL("===gen wpa ie length=%d\n\r",wpa_auth->wpa_ie_len );
	return 0;
}


ssv_type_u8 * wpa_add_kde(ssv_type_u8 *pos, ssv_type_u32 kde, const ssv_type_u8 *data, ssv_type_size_t data_len,
		 const ssv_type_u8 *data2, ssv_type_size_t data2_len)
{
	*pos++ = WLAN_EID_VENDOR_SPECIFIC;
	*pos++ = RSN_SELECTOR_LEN + data_len + data2_len;
	RSN_SELECTOR_PUT(pos, kde);
	pos += RSN_SELECTOR_LEN;
	os_memcpy(pos, data, data_len);
	pos += data_len;
	if (data2) {
		os_memcpy(pos, data2, data2_len);
		pos += data2_len;
	}
	return pos;
}

int wpa_validate_wpa_ie(struct wpa_authenticator *wpa_auth,
			struct wpa_state_machine *sm,
			const ssv_type_u8 *wpa_ie, ssv_type_size_t wpa_ie_len,
			const ssv_type_u8 *mdie, ssv_type_size_t mdie_len)
{
	struct wpa_ie_data data;
	int ciphers, key_mgmt, res, version;
	ssv_type_u32 selector;
	//size_t i;
	//const u8 *pmkid = NULL;
	
	if (wpa_auth == NULL || sm == NULL)
		return WPA_NOT_ENABLED;

	if (wpa_ie == NULL || wpa_ie_len < 1)
		return WPA_INVALID_IE;

	if (wpa_ie[0] == WLAN_EID_RSN)
		version = WPA_PROTO_RSN;
	else
		version = WPA_PROTO_WPA;
    
	if (!(wpa_auth->conf->wpa & version)) {
		wpa_printf(MSG_DEBUG, "Invalid WPA proto (%d) from " MACSTR,
			   version, MAC2STR(sm->addr));
		return WPA_INVALID_PROTO;
	}

	if (version == WPA_PROTO_RSN) {
		res = wpa_parse_wpa_ie_rsn(wpa_ie, wpa_ie_len, &data);

		selector = RSN_AUTH_KEY_MGMT_UNSPEC_802_1X;
		if (0) {
		}

		else if (data.key_mgmt & WPA_KEY_MGMT_IEEE8021X)
			selector = RSN_AUTH_KEY_MGMT_UNSPEC_802_1X;
		else if (data.key_mgmt & WPA_KEY_MGMT_PSK)
			selector = RSN_AUTH_KEY_MGMT_PSK_OVER_802_1X;
		wpa_auth->dot11RSNAAuthenticationSuiteSelected = selector;

		selector = RSN_CIPHER_SUITE_CCMP;
		if (data.pairwise_cipher & WPA_CIPHER_CCMP)
			selector = RSN_CIPHER_SUITE_CCMP;
		else if (data.pairwise_cipher & WPA_CIPHER_TKIP)
			selector = RSN_CIPHER_SUITE_TKIP;
		else if (data.pairwise_cipher & WPA_CIPHER_WEP104)
			selector = RSN_CIPHER_SUITE_WEP104;
		else if (data.pairwise_cipher & WPA_CIPHER_WEP40)
			selector = RSN_CIPHER_SUITE_WEP40;
		else if (data.pairwise_cipher & WPA_CIPHER_NONE)
			selector = RSN_CIPHER_SUITE_NONE;
		wpa_auth->dot11RSNAPairwiseCipherSelected = selector;

		selector = RSN_CIPHER_SUITE_CCMP;
		if (data.group_cipher & WPA_CIPHER_CCMP)
			selector = RSN_CIPHER_SUITE_CCMP;
		else if (data.group_cipher & WPA_CIPHER_TKIP)
			selector = RSN_CIPHER_SUITE_TKIP;
		else if (data.group_cipher & WPA_CIPHER_WEP104)
			selector = RSN_CIPHER_SUITE_WEP104;
		else if (data.group_cipher & WPA_CIPHER_WEP40)
			selector = RSN_CIPHER_SUITE_WEP40;
		else if (data.group_cipher & WPA_CIPHER_NONE)
			selector = RSN_CIPHER_SUITE_NONE;
		wpa_auth->dot11RSNAGroupCipherSelected = selector;
	} else {
		res = wpa_parse_wpa_ie_wpa(wpa_ie, wpa_ie_len, &data);

		selector = WPA_AUTH_KEY_MGMT_UNSPEC_802_1X;
		if (data.key_mgmt & WPA_KEY_MGMT_IEEE8021X)
			selector = WPA_AUTH_KEY_MGMT_UNSPEC_802_1X;
		else if (data.key_mgmt & WPA_KEY_MGMT_PSK)
			selector = WPA_AUTH_KEY_MGMT_PSK_OVER_802_1X;
		wpa_auth->dot11RSNAAuthenticationSuiteSelected = selector;

		selector = WPA_CIPHER_SUITE_TKIP;
		if (data.pairwise_cipher & WPA_CIPHER_CCMP)
			selector = WPA_CIPHER_SUITE_CCMP;
		else if (data.pairwise_cipher & WPA_CIPHER_TKIP)
			selector = WPA_CIPHER_SUITE_TKIP;
		else if (data.pairwise_cipher & WPA_CIPHER_WEP104)
			selector = WPA_CIPHER_SUITE_WEP104;
		else if (data.pairwise_cipher & WPA_CIPHER_WEP40)
			selector = WPA_CIPHER_SUITE_WEP40;
		else if (data.pairwise_cipher & WPA_CIPHER_NONE)
			selector = WPA_CIPHER_SUITE_NONE;
		wpa_auth->dot11RSNAPairwiseCipherSelected = selector;

		selector = WPA_CIPHER_SUITE_TKIP;
		if (data.group_cipher & WPA_CIPHER_CCMP)
			selector = WPA_CIPHER_SUITE_CCMP;
		else if (data.group_cipher & WPA_CIPHER_TKIP)
			selector = WPA_CIPHER_SUITE_TKIP;
		else if (data.group_cipher & WPA_CIPHER_WEP104)
			selector = WPA_CIPHER_SUITE_WEP104;
		else if (data.group_cipher & WPA_CIPHER_WEP40)
			selector = WPA_CIPHER_SUITE_WEP40;
		else if (data.group_cipher & WPA_CIPHER_NONE)
			selector = WPA_CIPHER_SUITE_NONE;
		wpa_auth->dot11RSNAGroupCipherSelected = selector;
	}
	if (res) {
		wpa_printf(MSG_DEBUG, "Failed to parse WPA/RSN IE from "
			   MACSTR " (res=%d)", MAC2STR(sm->addr), res);
		wpa_hexdump(MSG_DEBUG, "WPA/RSN IE", wpa_ie, wpa_ie_len);
		return WPA_INVALID_IE;
	}

	if (data.group_cipher != wpa_auth->conf->wpa_group) {
		wpa_printf(MSG_DEBUG, "Invalid WPA group cipher (0x%x) from "
			   MACSTR, data.group_cipher, MAC2STR(sm->addr));
		return WPA_INVALID_GROUP;
	}

	key_mgmt = data.key_mgmt & wpa_auth->conf->wpa_key_mgmt;
	if (!key_mgmt) {
		wpa_printf(MSG_DEBUG, "Invalid WPA key mgmt (0x%x) from "
			   MACSTR, data.key_mgmt, MAC2STR(sm->addr));
		return WPA_INVALID_AKMP;
	}
	if (0) {
	}
	else if (key_mgmt & WPA_KEY_MGMT_IEEE8021X)
		sm->wpa_key_mgmt = WPA_KEY_MGMT_IEEE8021X;
	else
		sm->wpa_key_mgmt = WPA_KEY_MGMT_PSK;

	if (version == WPA_PROTO_RSN)
		ciphers = data.pairwise_cipher & wpa_auth->conf->rsn_pairwise;
	else
		ciphers = data.pairwise_cipher & wpa_auth->conf->wpa_pairwise;
	if (!ciphers) {
		wpa_printf(MSG_DEBUG, "Invalid %s pairwise cipher (0x%x) "
			   "from " MACSTR,
			   version == WPA_PROTO_RSN ? "RSN" : "WPA",
			   data.pairwise_cipher, MAC2STR(sm->addr));
		return WPA_INVALID_PAIRWISE;
	}

	if (ciphers & WPA_CIPHER_CCMP)
		sm->pairwise = WPA_CIPHER_CCMP;
	else
		sm->pairwise = WPA_CIPHER_TKIP;

	/* TODO: clear WPA/WPA2 state if STA changes from one to another */
	if (wpa_ie[0] == WLAN_EID_RSN)
		sm->wpa = WPA_VERSION_WPA2;
	else
		sm->wpa = WPA_VERSION_WPA;
	if (sm->wpa_ie == NULL || sm->wpa_ie_len < wpa_ie_len) {
        if(sm->wpa_ie)
        {
		    os_free(sm->wpa_ie);
        }
		sm->wpa_ie = (ssv_type_u8 *)OS_MemAlloc(wpa_ie_len);
		if (sm->wpa_ie == NULL)
			return WPA_ALLOC_FAIL;
	}
	os_memcpy(sm->wpa_ie, wpa_ie, wpa_ie_len);
	sm->wpa_ie_len = wpa_ie_len;

	return WPA_IE_OK;
}


/**
 * wpa_parse_generic - Parse EAPOL-Key Key Data Generic IEs
 * @pos: Pointer to the IE header
 * @end: Pointer to the end of the Key Data buffer
 * @ie: Pointer to parsed IE data
 * Returns: 0 on success, 1 if end mark is found, -1 on failure
 */
static int wpa_parse_generic(const ssv_type_u8 *pos, const ssv_type_u8 *end,
			     struct wpa_eapol_ie_parse *ie)
{
	if (pos[1] == 0)
		return 1;

	if (pos[1] >= 6 &&
	    RSN_SELECTOR_GET(pos + 2) == WPA_OUI_TYPE &&
	    pos[2 + WPA_SELECTOR_LEN] == 1 &&
	    pos[2 + WPA_SELECTOR_LEN + 1] == 0) {
		ie->wpa_ie = pos;
		ie->wpa_ie_len = pos[1] + 2;
		return 0;
	}

	if (pos + 1 + RSN_SELECTOR_LEN < end &&
	    pos[1] >= RSN_SELECTOR_LEN + PMKID_LEN &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_PMKID) {
		ie->pmkid = pos + 2 + RSN_SELECTOR_LEN;
		return 0;
	}

	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_GROUPKEY) {
		ie->gtk = pos + 2 + RSN_SELECTOR_LEN;
		ie->gtk_len = pos[1] - RSN_SELECTOR_LEN;
		return 0;
	}

	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_MAC_ADDR) {
		ie->mac_addr = pos + 2 + RSN_SELECTOR_LEN;
		ie->mac_addr_len = pos[1] - RSN_SELECTOR_LEN;
		return 0;
	}
	return 0;
}



/**
 * wpa_parse_kde_ies - Parse EAPOL-Key Key Data IEs
 * @buf: Pointer to the Key Data buffer
 * @len: Key Data Length
 * @ie: Pointer to parsed IE data
 * Returns: 0 on success, -1 on failure
 */
int wpa_parse_kde_ies(const ssv_type_u8 *buf, ssv_type_size_t len, struct wpa_eapol_ie_parse *ie)
{
	const ssv_type_u8 *pos, *end;
	int ret = 0;
   
	os_memset(ie, 0, sizeof(*ie));
	for (pos = buf, end = pos + len; pos + 1 < end; pos += 2 + pos[1]) {
		if (pos[0] == 0xdd &&
		    ((pos == buf + len - 1) || pos[1] == 0)) {
			/* Ignore padding */
			break;
		}
		if (pos + 2 + pos[1] > end) {
			wpa_printf(MSG_DEBUG, "WPA: EAPOL-Key Key Data "
				   "underflow (ie=%d len=%d pos=%d)",
				   pos[0], pos[1], (int) (pos - buf));
			wpa_hexdump_key(MSG_DEBUG, "WPA: Key Data",
					buf, len);
			ret = -1;
			break;
		}
		if (*pos == WLAN_EID_RSN) {
			ie->rsn_ie = pos;
			ie->rsn_ie_len = pos[1] + 2;

		} else if (*pos == WLAN_EID_VENDOR_SPECIFIC) {
			ret = wpa_parse_generic(pos, end, ie);
			if (ret < 0)
				break;
			if (ret > 0) {
				ret = 0;
				break;
			}
		} else {
			wpa_hexdump(MSG_DEBUG, "WPA: Unrecognized EAPOL-Key "
				    "Key Data IE", pos, 2 + pos[1]);
		}
	}
    

	return ret;
}

#endif//CONFIG_NO_WPA2
#endif
