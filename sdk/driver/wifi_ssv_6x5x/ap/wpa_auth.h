/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef WPA_AUTH_H
#define WPA_AUTH_H

#include "common/defs.h"
#include "common/eapol_common.h"
#include "common/wpa_common.h"
#include "ap_sta_info.h"

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif /* _MSC_VER */

#ifdef _MSC_VER
#pragma pack(pop)
#endif /* _MSC_VER */


/* per STA state machine data */

struct wpa_authenticator;
struct wpa_state_machine;
//struct rsn_pmksa_cache_entry;
struct eapol_state_machine;


struct ft_remote_r0kh {
	struct ft_remote_r0kh *next;
	ssv_type_u8 addr[ETH_ALEN];
	ssv_type_u8 id[FT_R0KH_ID_MAX_LEN];
	ssv_type_size_t id_len;
	ssv_type_u8 key[16];
};


struct ft_remote_r1kh {
	struct ft_remote_r1kh *next;
	ssv_type_u8 addr[ETH_ALEN];
	ssv_type_u8 id[FT_R1KH_ID_LEN];
	ssv_type_u8 key[16];
};


struct wpa_auth_config {
	int wpa;
	int wpa_key_mgmt;
	int wpa_pairwise;
	int wpa_group;

	int rsn_pairwise;
	int rsn_preauth;
	int eapol_version;
	int peerkey;
	int wmm_enabled;

#ifdef CONFIG_IEEE80211W
	enum mfp_options ieee80211w;
#endif /* CONFIG_IEEE80211W */

};

typedef enum {
	LOGGER_DEBUG, LOGGER_INFO, LOGGER_WARNING
} logger_level;

typedef enum {
	WPA_EAPOL_portEnabled, WPA_EAPOL_portValid, WPA_EAPOL_authorized,
	WPA_EAPOL_portControl_Auto, WPA_EAPOL_keyRun, WPA_EAPOL_keyAvailable,
	WPA_EAPOL_keyDone, WPA_EAPOL_inc_EapolFramesTx
} wpa_eapol_variable;


void wpa_init(struct wpa_authenticator *wpa_auth);


void enable_beacon(void *data);
void start_4way_handshake(void *data);

int wpa_init_keys(struct wpa_authenticator *wpa_auth);
void wpa_deinit(struct wpa_authenticator *wpa_auth);

enum {
	WPA_IE_OK, WPA_INVALID_IE, WPA_INVALID_GROUP, WPA_INVALID_PAIRWISE,
	WPA_INVALID_AKMP, WPA_NOT_ENABLED, WPA_ALLOC_FAIL,
	WPA_MGMT_FRAME_PROTECTION_VIOLATION, WPA_INVALID_MGMT_GROUP_CIPHER,
	WPA_INVALID_MDIE, WPA_INVALID_PROTO
};

int wpa_validate_wpa_ie(struct wpa_authenticator *wpa_auth,
			struct wpa_state_machine *sm,
			const ssv_type_u8 *wpa_ie, ssv_type_size_t wpa_ie_len,
			const ssv_type_u8 *mdie, ssv_type_size_t mdie_len);
void wpa_auth_sta_init(struct wpa_authenticator *wpa_auth, const ssv_type_u8 *addr,struct wpa_state_machine *sm);
int wpa_auth_sta_associated(struct wpa_authenticator *wpa_auth,
			    struct wpa_state_machine *sm);
void wpa_auth_sta_no_wpa(struct wpa_state_machine *sm);
void wpa_auth_sta_deinit(struct wpa_state_machine *sm);
void wpa_receive(struct wpa_authenticator *wpa_auth,
		 struct wpa_state_machine *sm,
		 ssv_type_u8 *data, ssv_type_size_t data_len);
typedef enum {
	WPA_AUTH, WPA_ASSOC, WPA_DISASSOC, WPA_DEAUTH, WPA_REAUTH,
	WPA_REAUTH_EAPOL, WPA_ASSOC_FT
} wpa_event;
void wpa_remove_ptk(struct wpa_state_machine *sm);
int wpa_auth_sm_event(struct wpa_state_machine *sm, wpa_event event);

const ssv_type_u8 * wpa_auth_get_wpa_ie(struct wpa_authenticator *wpa_auth,
			       ssv_type_size_t *len);


void wpa_auth_recovery(void);	
void process_4Ways_start(struct APStaInfo *staInfo);
int wpa_parse_wpa_ie_wpa(const ssv_type_u8 *wpa_ie, ssv_type_size_t wpa_ie_len,
			 struct wpa_ie_data *data);


#endif /* WPA_AUTH_H */
