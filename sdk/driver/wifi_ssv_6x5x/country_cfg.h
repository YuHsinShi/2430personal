/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/

#ifndef _COUNTRY_CFG_H_
#define _COUNTRY_CFG_H_

#define MAX_COUNTRY_NUM 10
#define MAX_REG_RULES 10

#define MHZ_TO_KHZ(freq) ((freq) * 1000)
#define KHZ_TO_MHZ(freq) ((freq) / 1000)
#define DBI_TO_MBI(gain) ((gain) * 100)
#define MBI_TO_DBI(gain) ((gain) / 100)
#define DBM_TO_MBM(gain) ((gain) * 100)
#define MBM_TO_DBM(gain) ((gain) / 100)

/**
 * enum nl80211_reg_rule_flags - regulatory rule flags
 *
 * @NL80211_RRF_NO_OFDM: OFDM modulation not allowed
 * @NL80211_RRF_NO_CCK: CCK modulation not allowed
 * @NL80211_RRF_NO_INDOOR: indoor operation not allowed
 * @NL80211_RRF_NO_OUTDOOR: outdoor operation not allowed
 * @NL80211_RRF_DFS: DFS support is required to be used
 * @NL80211_RRF_PTP_ONLY: this is only for Point To Point links
 * @NL80211_RRF_PTMP_ONLY: this is only for Point To Multi Point links
 * @NL80211_RRF_PASSIVE_SCAN: passive scan is required
 * @NL80211_RRF_NO_IBSS: no IBSS is allowed
 */
enum nl80211_reg_rule_flags {
	NL80211_RRF_NO_OFDM		= 1<<0,
	NL80211_RRF_NO_CCK		= 1<<1,
	NL80211_RRF_NO_INDOOR		= 1<<2,
	NL80211_RRF_NO_OUTDOOR		= 1<<3,
	NL80211_RRF_DFS			= 1<<4,
	NL80211_RRF_PTP_ONLY		= 1<<5,
	NL80211_RRF_PTMP_ONLY		= 1<<6,
	NL80211_RRF_PASSIVE_SCAN	= 1<<7,
	NL80211_RRF_NO_IBSS		= 1<<8
};

struct ieee80211_freq_range {
	ssv_type_u32 start_freq_khz;
	ssv_type_u32 end_freq_khz;
	ssv_type_u32 max_bandwidth_khz;
};

struct ieee80211_power_rule {
	ssv_type_u32 max_antenna_gain;
	ssv_type_u32 max_eirp;
};

struct ieee80211_reg_rule {
	struct ieee80211_freq_range freq_range;
	struct ieee80211_power_rule power_rule;
	ssv_type_u32 flags;
};

struct ieee80211_regdomain {
	ssv_type_u32 n_reg_rules;
	char alpha2[3];
	ssv_type_u8 dfs_region;
	struct ieee80211_reg_rule reg_rules[MAX_REG_RULES];
};

extern struct ieee80211_regdomain regdom[MAX_COUNTRY_NUM];
extern int ssv6xxx_wifi_init_regdom(void);
#endif
