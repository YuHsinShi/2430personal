/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/

//#include "../supplicant_config.h"
#include <rtos.h>
#include <ssv_lib.h>

#include "wpa_common.h"

//#include "../crypto/md5.h"
#include "../crypto/sha1.h"
#include "../crypto/aes_wrap.h"
#include "../crypto/crypto.h"
#include "ieee802_11_defs.h"
#include "defs.h"

#include "../wpa_debug.h"
 
#include "ieee80211.h"

#if (AP_MODE_ENABLE == 1)

#ifndef CONFIG_NO_WPA2

#ifndef MAC2STR
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#endif

#ifndef CONFIG_NO_WPA2
/**
 * wpa_eapol_key_mic - Calculate EAPOL-Key MIC
 * @key: EAPOL-Key Key Confirmation Key (KCK)
 * @ver: Key descriptor version (WPA_KEY_INFO_TYPE_*)
 * @buf: Pointer to the beginning of the EAPOL header (version field)
 * @len: Length of the EAPOL frame (from EAPOL header to the end of the frame)
 * @mic: Pointer to the buffer to which the EAPOL-Key MIC is written
 * Returns: 0 on success, -1 on failure
 *
 * Calculate EAPOL-Key MIC for an EAPOL-Key packet. The EAPOL-Key MIC field has
 * to be cleared (all zeroes) when calling this function.
 *
 * Note: 'IEEE Std 802.11i-2004 - 8.5.2 EAPOL-Key frames' has an error in the
 * description of the Key MIC calculation. It includes packet data from the
 * beginning of the EAPOL-Key header, not EAPOL header. This incorrect change
 * happened during final editing of the standard and the correct behavior is
 * defined in the last draft (IEEE 802.11i/D10).
 */
int wpa_eapol_key_mic(const ssv_type_u8 *key, int ver, const ssv_type_u8 *buf, ssv_type_size_t len,
		      ssv_type_u8 *mic)
{
	ssv_type_u8 hash[SHA1_MAC_LEN];
	switch (ver) {
	case WPA_KEY_INFO_TYPE_HMAC_MD5_RC4:
       
        return -1;
		//return hmac_md5(key, 16, buf, len, mic);
	case WPA_KEY_INFO_TYPE_HMAC_SHA1_AES:        
		if (hmac_sha1(key, 16, buf, len, hash))
		{		  
		    return -1;
        }        
		os_memcpy(mic, hash, 16);
        //wpa_hexdump_key(MSG_DEBUG,"mic:",mic,16);
		break;

	default:
		return -1;
	}

	return 0;
}
#if 0

/**
 * wpa_pmk_to_ptk - Calculate PTK from PMK, addresses, and nonces
 * @pmk: Pairwise master key
 * @pmk_len: Length of PMK
 * @label: Label to use in derivation
 * @addr1: AA or SA
 * @addr2: SA or AA
 * @nonce1: ANonce or SNonce
 * @nonce2: SNonce or ANonce
 * @ptk: Buffer for pairwise transient key
 * @ptk_len: Length of PTK
 * @use_sha256: Whether to use SHA256-based KDF
 *
 * IEEE Std 802.11i-2004 - 8.5.1.2 Pairwise key hierarchy
 * PTK = PRF-X(PMK, "Pairwise key expansion",
 *             Min(AA, SA) || Max(AA, SA) ||
 *             Min(ANonce, SNonce) || Max(ANonce, SNonce))
 *
 * STK = PRF-X(SMK, "Peer key expansion",
 *             Min(MAC_I, MAC_P) || Max(MAC_I, MAC_P) ||
 *             Min(INonce, PNonce) || Max(INonce, PNonce))
 */
void wpa_pmk_to_ptk(const u8 *pmk, size_t pmk_len, const char *label,
		    const u8 *addr1, const u8 *addr2,
		    const u8 *nonce1, const u8 *nonce2,
		    u8 *ptk, size_t ptk_len, int use_sha256)
{
	u8 data[2 * ETH_ALEN + 2 * WPA_NONCE_LEN];
   
	if (ssv6xxx_memcmp(addr1, addr2, ETH_ALEN) < 0) {
		OS_MemCPY(data, addr1, ETH_ALEN);
		OS_MemCPY(data + ETH_ALEN, addr2, ETH_ALEN);
	} else {
		OS_MemCPY(data, addr2, ETH_ALEN);
		OS_MemCPY(data + ETH_ALEN, addr1, ETH_ALEN);
	}

	if (ssv6xxx_memcmp(nonce1, nonce2, WPA_NONCE_LEN) < 0) {
		OS_MemCPY(data + 2 * ETH_ALEN, nonce1, WPA_NONCE_LEN);
		OS_MemCPY(data + 2 * ETH_ALEN + WPA_NONCE_LEN, nonce2,
			  WPA_NONCE_LEN);
	} else {
		OS_MemCPY(data + 2 * ETH_ALEN, nonce2, WPA_NONCE_LEN);
		OS_MemCPY(data + 2 * ETH_ALEN + WPA_NONCE_LEN, nonce1,
			  WPA_NONCE_LEN);
	}


	sha1_prf(pmk, pmk_len, label, data, sizeof(data), ptk,
			 ptk_len);

	wpa_printf(MSG_DEBUG, "\nWPA: PTK derivation - A1=" MACSTR " A2=" MACSTR,
               MAC2STR(addr1), MAC2STR(addr2));
	wpa_hexdump(MSG_DEBUG, "\nWPA: Nonce1", nonce1, WPA_NONCE_LEN);
	wpa_hexdump(MSG_DEBUG, "\nWPA: Nonce2", nonce2, WPA_NONCE_LEN);
	wpa_hexdump_key(MSG_DEBUG, "\nWPA: PMK", pmk, pmk_len);
	wpa_hexdump_key(MSG_DEBUG, "\nWPA: PTK", ptk, ptk_len);
}

#endif

static int rsn_selector_to_bitfield(const ssv_type_u8 *s)
{
	if (RSN_SELECTOR_GET(s) == RSN_CIPHER_SUITE_NONE)
		return WPA_CIPHER_NONE;
	if (RSN_SELECTOR_GET(s) == RSN_CIPHER_SUITE_WEP40)
		return WPA_CIPHER_WEP40;
	if (RSN_SELECTOR_GET(s) == RSN_CIPHER_SUITE_TKIP)
		return WPA_CIPHER_TKIP;
	if (RSN_SELECTOR_GET(s) == RSN_CIPHER_SUITE_CCMP)
		return WPA_CIPHER_CCMP;
	if (RSN_SELECTOR_GET(s) == RSN_CIPHER_SUITE_WEP104)
		return WPA_CIPHER_WEP104;

	return 0;
}


static int rsn_key_mgmt_to_bitfield(const ssv_type_u8 *s)
{
	if (RSN_SELECTOR_GET(s) == RSN_AUTH_KEY_MGMT_UNSPEC_802_1X)
		return WPA_KEY_MGMT_IEEE8021X;
	if (RSN_SELECTOR_GET(s) == RSN_AUTH_KEY_MGMT_PSK_OVER_802_1X)
		return WPA_KEY_MGMT_PSK;

	return 0;
}
#endif /* CONFIG_NO_WPA2 */


/**
 * wpa_parse_wpa_ie_rsn - Parse RSN IE
 * @rsn_ie: Buffer containing RSN IE
 * @rsn_ie_len: RSN IE buffer length (including IE number and length octets)
 * @data: Pointer to structure that will be filled in with parsed data
 * Returns: 0 on success, <0 on failure
 */
int wpa_parse_wpa_ie_rsn(const ssv_type_u8 *rsn_ie, ssv_type_size_t rsn_ie_len,
			 struct wpa_ie_data *data)
{
#ifndef CONFIG_NO_WPA2
	const struct rsn_ie_hdr *hdr;
	const ssv_type_u8 *pos;
	int left;
	int i, count;

	os_memset(data, 0, sizeof(*data));
	data->proto = WPA_PROTO_RSN;
	data->pairwise_cipher = WPA_CIPHER_CCMP;
	data->group_cipher = WPA_CIPHER_CCMP;
	data->key_mgmt = WPA_KEY_MGMT_IEEE8021X;
	data->capabilities = 0;
	data->pmkid = (const ssv_type_u8 *)NULL;
	data->num_pmkid = 0;

	data->mgmt_group_cipher = 0;

	if (rsn_ie_len == 0) {
		/* No RSN IE - fail silently */
		return -1;
	}

	if (rsn_ie_len < sizeof(struct rsn_ie_hdr)) {
		wpa_printf(MSG_DEBUG, "%s: ie len too short %d",
			   __FUNCTION__, rsn_ie_len);
		return -1;
	}

	hdr = (const struct rsn_ie_hdr *) rsn_ie;

	if (hdr->elem_id != WLAN_EID_RSN ||
	    hdr->len != rsn_ie_len - 2 ||
	    WPA_GET_LE16(hdr->version) != RSN_VERSION) {
		wpa_printf(MSG_DEBUG, "%s: malformed ie or unknown version",
			   __FUNCTION__);
		return -2;
	}

	pos = (const ssv_type_u8 *) (hdr + 1);
	left = rsn_ie_len - sizeof(*hdr);

	//pos = rsn_ie+2;
	//left = rsn_ie_len - 2;
	if (left >= RSN_SELECTOR_LEN) {
		data->group_cipher = rsn_selector_to_bitfield(pos);

		pos += RSN_SELECTOR_LEN;
		left -= RSN_SELECTOR_LEN;
	} else if (left > 0) {
		wpa_printf(MSG_DEBUG, "%s: ie length mismatch, %d too much",
			   __FUNCTION__, left);
		return -3;
	}

	if (left >= 2) {
		data->pairwise_cipher = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * RSN_SELECTOR_LEN) {
			wpa_printf(MSG_DEBUG, "%s: ie count botch (pairwise), "
				   "count %d left %d", __FUNCTION__, count, left);
			return -4;
		}
		for (i = 0; i < count; i++) {
			data->pairwise_cipher |= rsn_selector_to_bitfield(pos);
			pos += RSN_SELECTOR_LEN;
			left -= RSN_SELECTOR_LEN;
		}

	} else if (left == 1) {
		wpa_printf(MSG_DEBUG, "%s: ie too short (for key mgmt)",
			   __FUNCTION__);
		return -5;
	}

	if (left >= 2) {
		data->key_mgmt = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * RSN_SELECTOR_LEN) {
			wpa_printf(MSG_DEBUG, "%s: ie count botch (key mgmt), "
				   "count %d left %d", __FUNCTION__, count, left);
			return -6;
		}
		for (i = 0; i < count; i++) {
			data->key_mgmt |= rsn_key_mgmt_to_bitfield(pos);
			pos += RSN_SELECTOR_LEN;
			left -= RSN_SELECTOR_LEN;
		}
	} else if (left == 1) {
		wpa_printf(MSG_DEBUG, "%s: ie too short (for capabilities)",
			   __FUNCTION__);
		return -7;
	}

	if (left >= 2) {
		data->capabilities = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
	}

	if (left >= 2) {
		data->num_pmkid = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (left < (int) data->num_pmkid * PMKID_LEN) {
			wpa_printf(MSG_DEBUG, "%s: PMKID underflow "
				   "(num_pmkid=%d, left=%d)",
				   __FUNCTION__,data->num_pmkid,
				   left);
			data->num_pmkid = 0;
			return -9;
		} else {
			data->pmkid = pos;
			pos += data->num_pmkid * PMKID_LEN;
			left -= data->num_pmkid * PMKID_LEN;
		}
	}

	if (left > 0) {
		wpa_printf(MSG_DEBUG, "%s: ie has %d trailing bytes - ignored",
			   __FUNCTION__, left);
	}

	return 0;
#else /* CONFIG_NO_WPA2 */
	return -1;
#endif /* CONFIG_NO_WPA2 */
}


static int wpa_selector_to_bitfield(const ssv_type_u8 *s)
{
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_NONE)
		return WPA_CIPHER_NONE;
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_WEP40)
		return WPA_CIPHER_WEP40;
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_TKIP)
		return WPA_CIPHER_TKIP;
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_CCMP)
		return WPA_CIPHER_CCMP;
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_WEP104)
		return WPA_CIPHER_WEP104;
	return 0;
}


static int wpa_key_mgmt_to_bitfield(const ssv_type_u8 *s)
{
	if (RSN_SELECTOR_GET(s) == WPA_AUTH_KEY_MGMT_UNSPEC_802_1X)
		return WPA_KEY_MGMT_IEEE8021X;
	if (RSN_SELECTOR_GET(s) == WPA_AUTH_KEY_MGMT_PSK_OVER_802_1X)
		return WPA_KEY_MGMT_PSK;
	if (RSN_SELECTOR_GET(s) == WPA_AUTH_KEY_MGMT_NONE)
		return WPA_KEY_MGMT_WPA_NONE;
	return 0;
}


int wpa_parse_wpa_ie_wpa(const ssv_type_u8 *wpa_ie, ssv_type_size_t wpa_ie_len,
			 struct wpa_ie_data *data)
{
#if 0
	const struct wpa_ie_hdr *hdr;
#endif
	const ssv_type_u8 *pos;
	int left;
	int i, count;
	os_memset(data, 0, sizeof(*data));
	data->proto = WPA_PROTO_WPA;
	data->pairwise_cipher = WPA_CIPHER_TKIP;
	data->group_cipher = WPA_CIPHER_TKIP;
	data->key_mgmt = WPA_KEY_MGMT_IEEE8021X;
	data->capabilities = 0;
	data->pmkid = (const ssv_type_u8 *)NULL;
	data->num_pmkid = 0;
	data->mgmt_group_cipher = 0;

#if 0
	if (wpa_ie_len == 0) {
		/* No WPA IE - fail silently */
		return -1;
	}

	if (wpa_ie_len < sizeof(struct wpa_ie_hdr)) {
		wpa_printf(MSG_DEBUG, "%s: ie len too short %d",
			   __FUNCTION__,wpa_ie_len);
		return -1;
	}

	hdr = (const struct wpa_ie_hdr *) wpa_ie;
	if (hdr->elem_id != WLAN_EID_VENDOR_SPECIFIC ||
	    hdr->len != wpa_ie_len - 2 ||
	    RSN_SELECTOR_GET(hdr->oui) != WPA_OUI_TYPE ||
	    WPA_GET_LE16(hdr->version) != WPA_VERSION) {
		wpa_printf(MSG_DEBUG, "%s: malformed ie or unknown version",
			   __FUNCTION__);
		return -2;
	}
	pos = (const u8 *) (hdr + 1);
	left = wpa_ie_len - sizeof(*hdr);
#else
    if (wpa_ie_len == 0) {
		/* No WPA IE - fail silently */
		return -1;
	}
/*
struct wpa_ie_hdr {
	u8 elem_id;
	u8 len;
	u8 oui[4]; 
	u8 version[2]; 
} struct_packed;
*/

    pos = (const ssv_type_u8 *)wpa_ie;
    left=wpa_ie_len;
	pos +=6; // 6=sizeof(oui)+sizeof(version)
	left -=6;// 6=sizeof(oui)+sizeof(version)

#endif

	if (left >= WPA_SELECTOR_LEN) {
		data->group_cipher = wpa_selector_to_bitfield(pos);
		pos += WPA_SELECTOR_LEN;
		left -= WPA_SELECTOR_LEN;
	} else if (left > 0) {
		wpa_printf(MSG_DEBUG, "%s: ie length mismatch, %d too much",
			   __FUNCTION__, left);
		return -3;
	}

	if (left >= 2) {
		data->pairwise_cipher = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * WPA_SELECTOR_LEN) {
			wpa_printf(MSG_DEBUG, "%s: ie count botch (pairwise), "
				   "count %d left %d", __FUNCTION__, count, left);
			return -4;
		}
		for (i = 0; i < count; i++) {
			data->pairwise_cipher |= wpa_selector_to_bitfield(pos);
			pos += WPA_SELECTOR_LEN;
			left -= WPA_SELECTOR_LEN;
		}
	} else if (left == 1) {
		wpa_printf(MSG_DEBUG, "%s: ie too short (for key mgmt)",
			   __FUNCTION__);
		return -5;
	}

	if (left >= 2) {
		data->key_mgmt = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * WPA_SELECTOR_LEN) {
			wpa_printf(MSG_DEBUG, "%s: ie count botch (key mgmt), "
				   "count %d left %d", __FUNCTION__, count, left);
			return -6;
		}
		for (i = 0; i < count; i++) {
			data->key_mgmt |= wpa_key_mgmt_to_bitfield(pos);
			pos += WPA_SELECTOR_LEN;
			left -= WPA_SELECTOR_LEN;
		}
	} else if (left == 1) {
		wpa_printf(MSG_DEBUG, "%s: ie too short (for capabilities)",
			   __FUNCTION__);
		return -7;
	}

	if (left >= 2) {
		data->capabilities = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
	}

	if (left > 0) {
		wpa_printf(MSG_DEBUG, "%s: ie has %d trailing bytes - ignored",
			   __FUNCTION__, left);
	}

	return 0;
}

/**
 * wpa_cipher_txt - Convert cipher suite to a text string
 * @cipher: Cipher suite (WPA_CIPHER_* enum)
 * Returns: Pointer to a text string of the cipher suite name
 */
const char * wpa_cipher_txt(int cipher)
{
	switch (cipher) {
	case WPA_CIPHER_NONE:
		return "NONE";
	case WPA_CIPHER_WEP40:
		return "WEP-40";
	case WPA_CIPHER_WEP104:
		return "WEP-104";
	case WPA_CIPHER_TKIP:
		return "TKIP";
	case WPA_CIPHER_CCMP:
		return "CCMP";
	case WPA_CIPHER_CCMP | WPA_CIPHER_TKIP:
		return "CCMP+TKIP";
	default:
		return "UNKNOWN";
	}
}

static void wpa_fixup_wpa_ie_rsn(ssv_type_u8 *assoc_ie, const ssv_type_u8 *wpa_msg_ie,
				 ssv_type_size_t rsn_ie_len)
{
	int pos, count;

	pos = sizeof(struct rsn_ie_hdr) + RSN_SELECTOR_LEN;
	if (rsn_ie_len < (ssv_type_size_t)(pos + 2))
		return;

	count = WPA_GET_LE16(wpa_msg_ie + pos);
	pos += 2 + count * RSN_SELECTOR_LEN;
	if (rsn_ie_len < (ssv_type_size_t)(pos + 2))
		return;

	count = WPA_GET_LE16(wpa_msg_ie + pos);
	pos += 2 + count * RSN_SELECTOR_LEN;
	if (rsn_ie_len < (ssv_type_size_t)(pos + 2))
		return;

	if (!assoc_ie[pos] && !assoc_ie[pos + 1] &&
	    (wpa_msg_ie[pos] || wpa_msg_ie[pos + 1]))
		os_memcpy(&assoc_ie[pos], &wpa_msg_ie[pos], 2);
}


int wpa_compare_rsn_ie(int ft_initial_assoc,
		       const ssv_type_u8 *ie1, ssv_type_size_t ie1len,
		       const ssv_type_u8 *ie2, ssv_type_size_t ie2len)
{
   
	if (ie1 == NULL || ie2 == NULL)
	{	
        return -1;
    }    
	if (ie1len == ie2len) {
		ssv_type_u8 *ie_tmp;
		if (os_memcmp(ie1, ie2, ie1len) == 0)
		{	
		    return 0; /* identical IEs */
          }
        
		ie_tmp = (ssv_type_u8 *)os_malloc(ie1len);
        if(NULL==ie_tmp)
        {
            LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
            return -1;
        }
		os_memcpy(ie_tmp, ie1, ie1len);
		wpa_fixup_wpa_ie_rsn(ie_tmp, ie2, ie1len);
		if (os_memcmp(ie_tmp, ie2, ie1len) == 0)
			return 0; /* only mismatch in RSN capabilties */
	}
	return -1;
}


/**
 * inc_byte_array - Increment arbitrary length byte array by one
 * @counter: Pointer to byte array
 * @len: Length of the counter in bytes
 *
 * This function increments the last byte of the counter by one and continues
 * rolling over to more significant bytes if the byte was incremented from
 * 0xff to 0x00.
 */
void inc_byte_array(ssv_type_u8 *counter, ssv_type_size_t len)
{
	int pos = len - 1;
	while (pos >= 0) {
		counter[pos]++;
		if (counter[pos] != 0)
			break;
		pos--;
	}
}

int wpa_cipher_key_len(int cipher)
{
	switch (cipher) {
	case WPA_CIPHER_CCMP:
	//case WPA_CIPHER_GCMP:
		return 16;
	case WPA_CIPHER_TKIP:
		return 32;
	case WPA_CIPHER_WEP104:
		return 13;
	case WPA_CIPHER_WEP40:
		return 5;
	}

	return 0;
}



#endif//CONFIG_NO_WPA2
#endif
