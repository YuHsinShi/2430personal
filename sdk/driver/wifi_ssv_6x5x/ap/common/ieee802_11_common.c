/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <log.h>
#include "ieee802_11_defs.h"
#include "ieee802_11_common.h"
#include "ieee80211.h"
#include "defs.h"

#if (AP_MODE_ENABLE == 1)

//-------------------------------------






//------------------------

static int ieee802_11_parse_vendor_specific(const ssv_type_u8 *pos, ssv_type_size_t elen,
					    struct ieee802_11_elems *elems,
					    int show_errors)
{
	unsigned int oui;

	/* first 3 bytes in vendor specific information element are the IEEE
	 * OUI of the vendor. The following byte is used a vendor specific
	 * sub-type. */
	if (elen < 4) {
		if (show_errors) {
			LOG_ERROR("short vendor specific "
				   "information element ignored (len=%lu)\r\n",
				   (unsigned long) elen);
		}
		return -1;
	}

	oui = AP_GET_BE24(pos);
	switch (oui) {
	case OUI_MICROSOFT:
		/* Microsoft/Wi-Fi information elements are further typed and
		 * subtyped */
		switch (pos[3]) {
		case 1:
			/* Microsoft OUI (00:50:F2) with OUI Type 1:
			 * real WPA information element */
			elems->wpa_ie = pos;
			elems->wpa_ie_len = elen;
			break;
		case WMM_OUI_TYPE:
			/* WMM information element */
			if (elen < 5) {
				LOG_ERROR( "short WMM "
					   "information element ignored "
					   "(len=%lu)\r\n",
					   (unsigned long) elen);
				return -1;
			}
			switch (pos[4]) {
			case WMM_OUI_SUBTYPE_INFORMATION_ELEMENT:
			case WMM_OUI_SUBTYPE_PARAMETER_ELEMENT:
				/*
				 * Share same pointer since only one of these
				 * is used and they start with same data.
				 * Length field can be used to distinguish the
				 * IEs.
				 */
				elems->wmm = pos;
				elems->wmm_len = elen;
				break;
			case WMM_OUI_SUBTYPE_TSPEC_ELEMENT:
				elems->wmm_tspec = pos;
				elems->wmm_tspec_len = elen;
				break;
			default:
				LOG_WARN(  "unknown WMM "
					   "information element ignored "
					   "(subtype=%d len=%lu)\r\n",
					   pos[4], (unsigned long) elen);
				return -1;
			}
			break;
		case 4:
			/* Wi-Fi Protected Setup (WPS) IE */
			elems->wps_ie = pos;
			elems->wps_ie_len = elen;
			break;
		default:
#if 0			
			LOG_WARN(  "Unknown Microsoft "
				   "information element ignored "
				   "(type=%d len=%lu)\n",
				   pos[3], (unsigned long) elen);
#endif
			return -1;
		}
		break;

	case OUI_BROADCOM:
		switch (pos[3]) {
		case VENDOR_HT_CAPAB_OUI_TYPE:
			elems->vendor_ht_cap = pos;
			elems->vendor_ht_cap_len = elen;
			break;
		default:
//			LOG_WARN( "Unknown Broadcom "
//				   "information element ignored "
//				   "(type=%d len=%lu)\r\n",
//				   pos[3], (unsigned long) elen);
			return -1;
		}
		break;

	default:
#if 0
		LOG_WARN( "unknown vendor specific information "
			   "element ignored (vendor OUI %02x:%02x:%02x "
			   "len=%lu)\n",
			   pos[0], pos[1], pos[2], (unsigned long) elen);
#endif
		return -1;
	}

	return 0;
}


/**
 * ieee802_11_parse_elems - Parse information elements in management frames
 * @start: Pointer to the start of IEs
 * @len: Length of IE buffer in octets
 * @elems: Data structure for parsed elements
 * @show_errors: Whether to show parsing errors in debug log
 * Returns: Parsing result
 */
ParseRes ieee802_11_parse_elems(const ssv_type_u8 *start, ssv_type_size_t len,
				struct ieee802_11_elems *elems,
				int show_errors)
{
	ssv_type_size_t left = len;
	const ssv_type_u8 *pos = start;
	int unknown = 0;

	OS_MemSET(elems, 0, sizeof(*elems));

	while (left >= 2) {
		ssv_type_u8 id, elen;

		id = *pos++;
		elen = *pos++;
		left -= 2;

		if (elen > left) {
			if (show_errors) {
				LOG_WARN( "IEEE 802.11 element "
					   "parse failed (id=%d elen=%d "
					   "left=%lu)\n",
					   id, elen, (unsigned long) left);
				//wpa_hexdump(MSG_MSGDUMP, "IEs", start, len);
			}
			return ParseFailed;
		}

		switch (id) {
		case WLAN_EID_SSID:
			elems->ssid = pos;
			elems->ssid_len = elen;
			break;
		case WLAN_EID_SUPP_RATES:
			elems->supp_rates = pos;
			elems->supp_rates_len = elen;
			break;
		case WLAN_EID_FH_PARAMS:
			elems->fh_params = pos;
			elems->fh_params_len = elen;
			break;
		case WLAN_EID_DS_PARAMS:
			elems->ds_params = pos;
			elems->ds_params_len = elen;
			break;
		case WLAN_EID_CF_PARAMS:
			elems->cf_params = pos;
			elems->cf_params_len = elen;
			break;
		case WLAN_EID_TIM:
			elems->tim = pos;
			elems->tim_len = elen;
			break;
		case WLAN_EID_IBSS_PARAMS:
			elems->ibss_params = pos;
			elems->ibss_params_len = elen;
			break;
		case WLAN_EID_CHALLENGE:
			elems->challenge = pos;
			elems->challenge_len = elen;
			break;
		case WLAN_EID_ERP_INFO:
			elems->erp_info = pos;
			elems->erp_info_len = elen;
			break;
		case WLAN_EID_EXT_SUPP_RATES:
			elems->ext_supp_rates = pos;
			elems->ext_supp_rates_len = elen;
			break;
		case WLAN_EID_VENDOR_SPECIFIC:
			if (ieee802_11_parse_vendor_specific(pos, elen,
							     elems,
							     show_errors))
				unknown++;
			break;
		case WLAN_EID_RSN:
			elems->rsn_ie = pos;
			elems->rsn_ie_len = elen;
			break;
		case WLAN_EID_PWR_CAPABILITY:
			elems->power_cap = pos;
			elems->power_cap_len = elen;
			break;
		case WLAN_EID_SUPPORTED_CHANNELS:
			elems->supp_channels = pos;
			elems->supp_channels_len = elen;
			break;
		case WLAN_EID_MOBILITY_DOMAIN:
			elems->mdie = pos;
			elems->mdie_len = elen;
			break;
		case WLAN_EID_FAST_BSS_TRANSITION:
			elems->ftie = pos;
			elems->ftie_len = elen;
			break;
		case WLAN_EID_TIMEOUT_INTERVAL:
			elems->timeout_int = pos;
			elems->timeout_int_len = elen;
			break;
		case WLAN_EID_HT_CAPABILITY:
			elems->ht_capabilities = pos;
			elems->ht_capabilities_len = elen;
			break;
		case WLAN_EID_HT_OPERATION:
			elems->ht_operation = pos;
			elems->ht_operation_len = elen;
			break;
		case WLAN_EID_LINK_ID:
			if (elen < 18)
				break;
			elems->link_id = pos;
			break;
		case WLAN_EID_INTERWORKING:
			elems->interworking = pos;
			elems->interworking_len = elen;
			break;
        case WLAN_EID_EXT_CAPABILITY:
			elems->ext_capab = pos;
			elems->ext_capab_len = elen;
            break;
		default:
			unknown++;
			if (!show_errors)
				break;
			LOG_WARN( "IEEE 802.11 element parse "
				   "ignored unknown element (id=%d elen=%d)\r\n",
				   id, elen);
			break;
		}

		left -= elen;
		pos += elen;
	}

	if (left)
		return ParseFailed;

	return unknown ? ParseUnknown : ParseOK;
}


int ieee802_11_ie_count(const ssv_type_u8 *ies, ssv_type_size_t ies_len)
{
	int count = 0;
	const ssv_type_u8 *pos, *end;

	if (ies == NULL)
		return 0;

	pos = ies;
	end = ies + ies_len;

	while (pos + 2 <= end) {
		if (pos + 2 + pos[1] > end)
			break;
		count++;
		pos += 2 + pos[1];
	}

	return count;
}


// struct wpabuf * ieee802_11_vendor_ie_concat(const u8 *ies, size_t ies_len,
// 					    u32 oui_type)
// {
// 	struct wpabuf *buf;
// 	const u8 *end, *pos, *ie;
// 
// 	pos = ies;
// 	end = ies + ies_len;
// 	ie = NULL;
// 
// 	while (pos + 1 < end) {
// 		if (pos + 2 + pos[1] > end)
// 			return NULL;
// 		if (pos[0] == WLAN_EID_VENDOR_SPECIFIC && pos[1] >= 4 &&
// 		    WPA_GET_BE32(&pos[2]) == oui_type) {
// 			ie = pos;
// 			break;
// 		}
// 		pos += 2 + pos[1];
// 	}
// 
// 	if (ie == NULL)
// 		return NULL; /* No specified vendor IE found */
// 
// 	buf = OS_MemAlloc(ies_len);???
// 	if (buf == NULL)
// 		return NULL;
// 
// 	/*
// 	 * There may be multiple vendor IEs in the message, so need to
// 	 * concatenate their data fields.
// 	 */
// 	while (pos + 1 < end) {
// 		if (pos + 2 + pos[1] > end)
// 			break;
// 		if (pos[0] == WLAN_EID_VENDOR_SPECIFIC && pos[1] >= 4 &&
// 		    WPA_GET_BE32(&pos[2]) == oui_type)
// 			OS_MemCPY(buf, pos + 6, pos[1] - 4);
// 		pos += 2 + pos[1];
// 	}
// 
// 	return buf;
// }
//-----------------------------


int ieee80211_channel_to_frequency(int chan, enum ssv_ieee80211_band band)
{
	/* see 802.11 17.3.8.3.2 and Annex J
	 * there are overlapping channel numbers in 5GHz and 2GHz bands */
	if (band == SSV_IEEE80211_BAND_5GHZ) {
		if (chan >= 182 && chan <= 196)
			return 4000 + chan * 5;
		else
			return 5000 + chan * 5;
	} else { /* IEEE80211_BAND_2GHZ */
		if (chan == 14)
			return 2484;
		else if (chan < 14)
			return 2407 + chan * 5;
		else
			return 0; /* not supported */
	}
}

int ieee80211_frequency_to_channel(int freq)
{
	/* see 802.11 17.3.8.3.2 and Annex J */
	if (freq == 2484)
		return 14;
	else if (freq < 2484)
		return (freq - 2407) / 5;
	else if (freq >= 4910 && freq <= 4980)
		return (freq - 4000) / 5;
	else
		return (freq - 5000) / 5;
}
#endif
