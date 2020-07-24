/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef PRIVSEP_COMMANDS_H
#define PRIVSEP_COMMANDS_H

enum privsep_cmd {
	PRIVSEP_CMD_REGISTER,
	PRIVSEP_CMD_UNREGISTER,
	PRIVSEP_CMD_SCAN,
	PRIVSEP_CMD_GET_SCAN_RESULTS,
	PRIVSEP_CMD_ASSOCIATE,
	PRIVSEP_CMD_GET_BSSID,
	PRIVSEP_CMD_GET_SSID,
	PRIVSEP_CMD_SET_KEY,
	PRIVSEP_CMD_GET_CAPA,
	PRIVSEP_CMD_L2_REGISTER,
	PRIVSEP_CMD_L2_UNREGISTER,
	PRIVSEP_CMD_L2_NOTIFY_AUTH_START,
	PRIVSEP_CMD_L2_SEND,
	PRIVSEP_CMD_SET_COUNTRY
};

struct privsep_cmd_associate
{
	ssv_type_u8 bssid[ETH_ALEN];
	ssv_type_u8 ssid[32];
	ssv_type_size_t ssid_len;
	int freq;
	int pairwise_suite;
	int group_suite;
	int key_mgmt_suite;
	int auth_alg;
	int mode;
	ssv_type_size_t wpa_ie_len;
	/* followed by wpa_ie_len bytes of wpa_ie */
};

struct privsep_cmd_set_key
{
	int alg;
	ssv_type_u8 addr[ETH_ALEN];
	int key_idx;
	int set_tx;
	ssv_type_u8 seq[8];
	ssv_type_size_t seq_len;
	ssv_type_u8 key[32];
	ssv_type_size_t key_len;
};

enum privsep_event {
	PRIVSEP_EVENT_SCAN_RESULTS,
	PRIVSEP_EVENT_ASSOC,
	PRIVSEP_EVENT_DISASSOC,
	PRIVSEP_EVENT_ASSOCINFO,
	PRIVSEP_EVENT_MICHAEL_MIC_FAILURE,
	PRIVSEP_EVENT_INTERFACE_STATUS,
	PRIVSEP_EVENT_PMKID_CANDIDATE,
	PRIVSEP_EVENT_STKSTART,
	PRIVSEP_EVENT_FT_RESPONSE,
	PRIVSEP_EVENT_RX_EAPOL
};

#endif /* PRIVSEP_COMMANDS_H */
