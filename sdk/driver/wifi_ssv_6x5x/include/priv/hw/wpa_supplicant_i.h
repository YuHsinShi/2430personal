/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef WPA_SUPPLICANT_I_H
#define WPA_SUPPLICANT_I_H

#include "apps/mac80211/supplicant/common/defs.h"


//struct wpa_sm;
//struct wpa_supplicant;
//struct scan_info;
//struct wpa_bss;
//struct wpa_scan_results;
#define MAX_SSID_LEN 32
#define ETH_ALEN	6


#define DEFAULT_EAP_WORKAROUND ((unsigned int) -1)
#define DEFAULT_EAPOL_FLAGS (EAPOL_FLAG_REQUIRE_KEY_UNICAST | \
			     EAPOL_FLAG_REQUIRE_KEY_BROADCAST)
#define DEFAULT_PROTO (WPA_PROTO_WPA | WPA_PROTO_RSN)
#define DEFAULT_KEY_MGMT (WPA_KEY_MGMT_PSK | WPA_KEY_MGMT_IEEE8021X)
#define DEFAULT_PAIRWISE (WPA_CIPHER_CCMP | WPA_CIPHER_TKIP)
#define DEFAULT_GROUP (WPA_CIPHER_CCMP | WPA_CIPHER_TKIP | \
		       WPA_CIPHER_WEP104 | WPA_CIPHER_WEP40)
#define DEFAULT_FRAGMENT_SIZE 1398

/**
 * struct wpa_ssid - Network configuration data
 *
 * This structure includes all the configuration variables for a network. This
 * data is included in the per-interface configuration data as an element of
 * the network list, struct wpa_config::ssid. Each network block in the
 * configuration is mapped to a struct wpa_ssid instance.
 */
struct wpa_ssid {
	/**
	 * ssid - Service set identifier (network name)
	 *
	 * This is the SSID for the network. For wireless interfaces, this is
	 * used to select which network will be used. If set to %NULL (or
	 * ssid_len=0), any SSID can be used. For wired interfaces, this must
	 * be set to %NULL. Note: SSID may contain any characters, even nul
	 * (ASCII 0) and as such, this should not be assumed to be a nul
	 * terminated string. ssid_len defines how many characters are valid
	 * and the ssid field is not guaranteed to be nul terminated.
	 */
	ssv_type_u8 ssid[32];

	/**
	 * ssid_len - Length of the SSID
	 */
	ssv_type_u8 ssid_len;

	/**
	 * bssid - BSSID
	 *
	 * If set, this network block is used only when associating with the AP
	 * using the configured BSSID
	 *
	 * If this is a persistent P2P group (disabled == 2), this is the GO
	 * Device Address.
	 */
	ssv_type_u8 bssid[ETH_ALEN];

	/**
	 * psk - WPA pre-shared key (256 bits)
	 */
	ssv_type_u8 psk[32];

	/**
	 * passphrase - WPA ASCII passphrase
	 *
	 * If this is set, psk will be generated using the SSID and passphrase
	 * configured for the network. ASCII passphrase must be between 8 and
	 * 63 characters (inclusive).
	 */
	char passphrase[64];

	/**
	 * pairwise_cipher - Bitfield of allowed pairwise ciphers, WPA_CIPHER_*
	 */
	ssv_type_u8 pairwise_cipher;

	/**
	 * group_cipher - Bitfield of allowed group ciphers, WPA_CIPHER_*
	 */
	ssv_type_u8 group_cipher;

	/**
	 * key_mgmt - Bitfield of allowed key management protocols
	 *
	 * WPA_KEY_MGMT_*
	 */
	ssv_type_u8 key_mgmt;

	/**
	 * proto - Bitfield of allowed protocols, WPA_PROTO_*
	 */
	ssv_type_u8 proto;

	/**
	 * auth_alg -  Bitfield of allowed authentication algorithms
	 *
	 * WPA_AUTH_ALG_*
	 */
	ssv_type_u8 auth_alg;

#define NUM_WEP_KEYS 4
#define MAX_WEP_KEY_LEN 16
	/**
	 * wep_key - WEP keys
	 */
	ssv_type_u8 wep_key[NUM_WEP_KEYS][MAX_WEP_KEY_LEN];

	/**
	 * wep_key_len - WEP key lengths
	 */
	ssv_type_u8 wep_key_len[NUM_WEP_KEYS];

	/**
	 * wep_tx_keyidx - Default key index for TX frames using WEP
	 */
	ssv_type_u8 wep_tx_keyidx;
};

/**
 * struct wpa_supplicant - Internal data for wpa_supplicant interface
 *
 * This structure contains the internal data for core wpa_supplicant code. This
 * should be only used directly from the core code. However, a pointer to this
 * data is used from other files as an arbitrary context pointer in calls to
 * core functions.
 */
struct wpa_supplicant {
	unsigned char own_addr[ETH_ALEN];

	int countermeasures;
	long last_michael_mic_error;
	ssv_type_u8 bssid[ETH_ALEN];
	//u8 pending_bssid[ETH_ALEN]; /* If wpa_state == WPA_ASSOCIATING, this* field contains the targer BSSID. */
	//int reassociate; /* reassociation requested */
	//int disconnected; /* all connections disabled; i.e., do no reassociate* before this has been cleared */
	struct wpa_ssid *current_ssid;

	/* Selected configuration (based on Beacon/ProbeResp WPA IE) */
	int pairwise_cipher;
	int group_cipher;
	int key_mgmt;
	int wpa_proto;
	int mgmt_group_cipher;

	struct wpa_sm *wpa;

	enum wpa_states wpa_state;
	int eapol_received; /* number of EAPOL packets received after the* previous association event */
    #ifdef ENABLE_BACKGROUND_PMK_CALC
    int (*pmk_calc_task) (void *data);
    void *pmk_calc_data;
    #endif // ENABLE_BACKGROUND_PMK_CALC
	//unsigned int drv_flags;
	//int pending_mic_error_report;
	//int pending_mic_error_pairwise;
	//int mic_errors_seen; /* Michael MIC errors with the current PTK */

	//struct wpabuf *pending_eapol_rx;
	//struct os_time pending_eapol_rx_time;
	//u8 pending_eapol_rx_src[ETH_ALEN];
};


/* wpa_supplicant.c */
const char * wpa_supplicant_state_txt(enum wpa_states state);
int wpa_supplicant_set_suites(struct wpa_supplicant *wpa_s, struct wpa_ssid *ssid,
			      ssv_type_u8 *wpa_ie, ssv_type_size_t *wpa_ie_len);
void wpa_supplicant_associate(struct wpa_supplicant *wpa_s,
			      struct wpa_ssid *ssid);

void wpa_clear_keys(struct wpa_supplicant *wpa_s, const ssv_type_u8 *addr);
void wpa_supplicant_req_auth_timeout(struct wpa_supplicant *wpa_s,
				     ssv_type_u32 sec, ssv_type_u32 usec);

struct wpa_ssid * wpa_supplicant_get_ssid(struct wpa_supplicant *wpa_s);
void wpa_supplicant_cancel_auth_timeout(struct wpa_supplicant *wpa_s);
void wpa_supplicant_deauthenticate(struct wpa_supplicant *wpa_s,
				   int reason_code);
void wpa_supplicant_disassociate(struct wpa_supplicant *wpa_s,
				 int reason_code);

struct wpa_supplicant * wpa_supplicant_init();
void wpa_supplicant_deinit(struct wpa_supplicant *wpa_s);


void wpa_supplicant_rx_eapol(void *ctx, const ssv_type_u8 *src_addr,
			     ssv_type_u8 *buf, ssv_type_size_t len);

void wpa_supplicant_clear_status(struct wpa_supplicant *wpa_s);

/* events.c */
void wpa_supplicant_mark_disassoc(struct wpa_supplicant *wpa_s);

void wpa_supplicant_stop_countermeasures(void *eloop_ctx, void *sock_ctx);

int wpa_supplicant_set_key(void *_wpa_s, enum wpa_alg alg,
				  const ssv_type_u8 *addr, int key_idx, int set_tx,
				  const ssv_type_u8 *seq, ssv_type_size_t seq_len,
				  const ssv_type_u8 *key, ssv_type_size_t key_len);


#endif /* WPA_SUPPLICANT_I_H */
