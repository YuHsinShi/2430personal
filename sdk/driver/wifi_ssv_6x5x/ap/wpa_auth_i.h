/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef WPA_AUTH_I_H
#define WPA_AUTH_I_H
#include <ssv_ether.h>
#include "common/defs.h"
#include "common/wpa_common.h"
//#include "wpa_auth.h"



/* max(dot11RSNAConfigGroupUpdateCount,dot11RSNAConfigPairwiseUpdateCount) */
#define RSNA_MAX_EAPOL_RETRIES 4
#define WPA_IE_LEN 22

struct wpa_group;


struct wpa_state_machine {
	struct wpa_authenticator *wpa_auth;
	struct wpa_group *group;

	ssv_type_u8 addr[ETH_ALEN];

	enum {
		WPA_PTK_INITIALIZE, WPA_PTK_DISCONNECT, WPA_PTK_DISCONNECTED,
		WPA_PTK_AUTHENTICATION, WPA_PTK_AUTHENTICATION2,
		WPA_PTK_INITPMK, WPA_PTK_INITPSK, WPA_PTK_PTKSTART,
		WPA_PTK_PTKCALCNEGOTIATING, WPA_PTK_PTKCALCNEGOTIATING2,
		WPA_PTK_PTKINITNEGOTIATING, WPA_PTK_PTKINITDONE
	} wpa_ptk_state;

	ssv_type_boolean Init;
	ssv_type_boolean DeauthenticationRequest;
	ssv_type_boolean AuthenticationRequest;
	ssv_type_boolean ReAuthenticationRequest;
	ssv_type_boolean Disconnect;
	int TimeoutCtr;
	ssv_type_boolean TimeoutEvt;
	ssv_type_boolean EAPOLKeyReceived;
	ssv_type_boolean EAPOLKeyPairwise;
	ssv_type_boolean EAPOLKeyRequest;
	ssv_type_boolean MICVerified;
	ssv_type_u8 ANonce[WPA_NONCE_LEN];
	ssv_type_u8 SNonce[WPA_NONCE_LEN];
	ssv_type_u8 PMK[PMK_LEN];
	struct wpa_ptk PTK;
	ssv_type_boolean PTK_valid;
	ssv_type_boolean pairwise_set;
	int keycount;
	ssv_type_boolean Pair;
	struct {
		ssv_type_u8 counter[WPA_REPLAY_COUNTER_LEN];
		ssv_type_boolean valid;
	} key_replay[RSNA_MAX_EAPOL_RETRIES];
	ssv_type_boolean PTKRequest; /* not in IEEE 802.11i state machine */
	
	ssv_type_u8 *last_rx_eapol_key; /* starting from IEEE 802.1X header */
	ssv_type_size_t last_rx_eapol_key_len;

	unsigned int changed:1;
	unsigned int in_step_loop:1;
	unsigned int pending_deinit:1;
	unsigned int started:1;
	
	unsigned int rx_eapol_key_secure:1;

	ssv_type_u8 req_replay_counter[WPA_REPLAY_COUNTER_LEN];
	int req_replay_counter_used;

	ssv_type_u8 *wpa_ie;
	ssv_type_size_t wpa_ie_len;

	enum {
		WPA_VERSION_NO_WPA = 0 /* WPA not used */,
		WPA_VERSION_WPA = 1 /* WPA / IEEE 802.11i/D3.0 */,
		WPA_VERSION_WPA2 = 2 /* WPA2 / IEEE 802.11i */
	} wpa;
	int pairwise; /* Pairwise cipher suite, WPA_CIPHER_* */
	int wpa_key_mgmt; /* the selected WPA_KEY_MGMT_* */

	ssv_type_u32 dot11RSNAStatsTKIPLocalMICFailures;
	ssv_type_u32 dot11RSNAStatsTKIPRemoteMICFailures;


	int pending_1_of_4_timeout;
};


/* per group key state machine data */
struct wpa_group {
	
	int GTK_len;
	int GN;
	ssv_type_u8 Counter[WPA_NONCE_LEN];

	ssv_type_u8 GMK[WPA_GMK_LEN];
	ssv_type_u8 GTK[2][WPA_GTK_MAX_LEN];
	ssv_type_u8 GNonce[WPA_NONCE_LEN];

	ssv_type_boolean first_sta_seen;
	

};

/* per authenticator data */
struct wpa_authenticator {
	struct wpa_group *group;

	unsigned int dot11RSNAStatsTKIPRemoteMICFailures;
	ssv_type_u32 dot11RSNAAuthenticationSuiteSelected;
	ssv_type_u32 dot11RSNAPairwiseCipherSelected;
	ssv_type_u32 dot11RSNAGroupCipherSelected;
	ssv_type_u8 dot11RSNAPMKIDUsed[PMKID_LEN];
	ssv_type_u32 dot11RSNAAuthenticationSuiteRequested; /* FIX: update */
	ssv_type_u32 dot11RSNAPairwiseCipherRequested; /* FIX: update */
	ssv_type_u32 dot11RSNAGroupCipherRequested; /* FIX: update */
	unsigned int dot11RSNATKIPCounterMeasuresInvoked;
	unsigned int dot11RSNA4WayHandshakeFailures;

    struct wpa_auth_config *conf;
	
	ssv_type_u8 wpa_ie[WPA_IE_LEN];
	ssv_type_size_t wpa_ie_len;
	//u8 addr[ETH_ALEN];

};


int wpa_write_rsn_ie(struct wpa_auth_config *conf, ssv_type_u8 *buf, ssv_type_size_t len,
		     const ssv_type_u8 *pmkid);

void __wpa_send_eapol(struct wpa_authenticator *wpa_auth,
		      struct wpa_state_machine *sm, int key_info,
		      const ssv_type_u8 *key_rsc, const ssv_type_u8 *nonce,
		      const ssv_type_u8 *kde, ssv_type_size_t kde_len,
		      int keyidx, int encr, int force_version);
int wpa_auth_for_each_sta(struct wpa_authenticator *wpa_auth,
			  int (*cb)(struct wpa_state_machine *sm, void *ctx),
			  void *cb_ctx);
int wpa_auth_for_each_auth(struct wpa_authenticator *wpa_auth,
			   int (*cb)(struct wpa_authenticator *a, void *ctx),
			   void *cb_ctx);



#endif /* WPA_AUTH_I_H */
