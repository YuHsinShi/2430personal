/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/
#ifndef _CHANNEL_H_
#define _CHANNEL_H_
#include <porting.h>
//#include <defs.h>
#include <ssv_common.h>


/**
 * struct ieee80211_channel - channel definition
 *
 * This structure describes a single channel for use
 * with cfg80211.
 *
 * @center_freq: center frequency in MHz
 * @hw_value: hardware-specific value for the channel
 * @flags: channel flags from &enum ieee80211_channel_flags.
 * @orig_flags: channel flags at registration time, used by regulatory
 *	code to support devices with additional restrictions
 * @band: band this channel belongs to.
 * @max_antenna_gain: maximum antenna gain in dBi
 * @max_power: maximum transmission power (in dBm)
 * @max_reg_power: maximum regulatory transmission power (in dBm)
 * @beacon_found: helper to regulatory code to indicate when a beacon
 *	has been found on this channel. Use regulatory_hint_found_beacon()
 *	to enable this, this is useful only on 5 GHz band.
 * @orig_mag: internal use
 * @orig_mpwr: internal use
 */
typedef struct ssv_ieee80211_channel {
	/* TODO(aaron): replace this struct wioth u8 */
	//enum ieee80211_band band;
	ssv_type_u8 band;
    ssv_type_u16 center_freq;
    ssv_type_u16 hw_value;
    ssv_type_u32 flags;
    int max_antenna_gain;
    int max_power;
    int max_reg_power;
    ssv_type_bool beacon_found;
    ssv_type_u32 orig_flags;
    int orig_mag, orig_mpwr;
}IEEE80211_CHANNEL;

extern IEEE80211_CHANNEL ssv6xxx_2ghz_chantable[MAX_2G_CHANNEL_NUM];
extern IEEE80211_CHANNEL ssv6xxx_5ghz_chantable[MAX_5G_CHANNEL_NUM];
extern int ssv6xxx_channel_table_init(void);
#endif //_CHANNEL_H_