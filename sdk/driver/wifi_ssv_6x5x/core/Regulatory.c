/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/
#include <porting.h>
#include <country_cfg.h>
#include <host_apis.h>
#include <log.h>
#include "channel.h"
#include "Regulatory.h"


/*
 * abs() handles unsigned and signed longs, ints, shorts and chars.  For all
 * input types abs() returns a signed long.
 * abs() should not be used for 64-bit types (s64, u64, long long) - use abs64()
 * for those.
 */
#if 1
#define abs(x) (((int)(x)<0)?-(x):x)
#else
#define abs(x) ({						\
		long ret;					\
		if (sizeof(x) == sizeof(long)) {		\
			long __x = (x);				\
			ret = (__x < 0) ? -__x : __x;		\
		} else {					\
			int __x = (x);				\
			ret = (__x < 0) ? -__x : __x;		\
		}						\
		ret;						\
	})
#endif
/**
 * freq_in_rule_band - tells us if a frequency is in a frequency band
 * @freq_range: frequency rule we want to query
 * @freq_khz: frequency we are inquiring about
 *
 * This lets us know if a specific frequency rule is or is not relevant to
 * a specific frequency's band. Bands are device specific and artificial
 * definitions (the "2.4 GHz band" and the "5 GHz band"), however it is
 * safe for now to assume that a frequency rule should not be part of a
 * frequency's band if the start freq or end freq are off by more than 2 GHz.
 * This resolution can be lowered and should be considered as we add
 * regulatory rule support for other "bands".
 **/
static ssv_type_bool freq_in_rule_band(const struct ieee80211_freq_range *freq_range,
	ssv_type_u32 freq_khz)
{
#define ONE_GHZ_IN_KHZ	1000000
	if (abs(freq_khz - freq_range->start_freq_khz) <= (2 * ONE_GHZ_IN_KHZ))
		return true;
	if (abs(freq_khz - freq_range->end_freq_khz) <= (2 * ONE_GHZ_IN_KHZ))
		return true;
	return false;
#undef ONE_GHZ_IN_KHZ
}	

static ssv_type_bool reg_does_bw_fit(const struct ieee80211_freq_range *freq_range,
			    ssv_type_u32 center_freq_khz,
			    ssv_type_u32 bw_khz)
{
	ssv_type_u32 start_freq_khz, end_freq_khz;
    
	start_freq_khz = center_freq_khz - (bw_khz/2);
	end_freq_khz = center_freq_khz + (bw_khz/2);
    
	if (start_freq_khz >= freq_range->start_freq_khz &&
	    end_freq_khz <= freq_range->end_freq_khz)
		return true;

	return false;
}

int freq_reg_info_regd(ssv_type_u32 center_freq,ssv_type_u32 desired_bw_khz,const struct ieee80211_regdomain *regd)
{
    ssv_type_u32 i=0;
	ssv_type_bool band_rule_found = false; 
	ssv_type_bool bw_fits = false;
    
	for (i = 0; i < regd->n_reg_rules; i++) {
		const struct ieee80211_reg_rule *rr;
		const struct ieee80211_freq_range *fr = NULL;

		rr = &regd->reg_rules[i];
		fr = &rr->freq_range;
    
        /*
         * We only need to know if one frequency rule was
         * was in center_freq's band, that's enough, so lets
         * not overwrite it once found
         */
        if (!band_rule_found)                
    	    band_rule_found = freq_in_rule_band(fr, MHZ_TO_KHZ(center_freq));

		bw_fits = reg_does_bw_fit(fr,
					  MHZ_TO_KHZ(center_freq),
					  MHZ_TO_KHZ(desired_bw_khz));
        
		if (band_rule_found && bw_fits) {
			return 0;
		}
        
	}
    return -1;
    
}

ssv_type_bool freq_need_dfs(ssv_type_u32 center_freq,ssv_type_u32 desired_bw_khz,const struct ieee80211_regdomain *regd)
{
    ssv_type_u32 i=0;
	ssv_type_bool band_rule_found = false; 
	ssv_type_bool bw_fits = false;
    
	for (i = 0; i < regd->n_reg_rules; i++) {
		const struct ieee80211_reg_rule *rr;
		const struct ieee80211_freq_range *fr = NULL;

		rr = &regd->reg_rules[i];
		fr = &rr->freq_range;

        /*
         * We only need to know if one frequency rule was
         * was in center_freq's band, that's enough, so lets
         * not overwrite it once found
         */
        if (!band_rule_found)                
    	    band_rule_found = freq_in_rule_band(fr, MHZ_TO_KHZ(center_freq));

		bw_fits = reg_does_bw_fit(fr,
					  MHZ_TO_KHZ(center_freq),
					  MHZ_TO_KHZ(desired_bw_khz));
        
		if (band_rule_found && bw_fits) {
            if(rr->flags&NL80211_RRF_DFS)
            {                
			    return TRUE;
            }
		}
        
	}
    return FALSE;
    
}

ssv_type_bool freq_40MHZ_legal(ssv_type_u32 center_freq,const struct ieee80211_regdomain *regd)
{
    ssv_type_u32 i=0;
	ssv_type_bool band_rule_found = false; 
	ssv_type_bool bw_fits = false;
    
	for (i = 0; i < regd->n_reg_rules; i++) {
		const struct ieee80211_reg_rule *rr;
		const struct ieee80211_freq_range *fr = NULL;

		rr = &regd->reg_rules[i];
		fr = &rr->freq_range;

        /*
         * We only need to know if one frequency rule was
         * was in center_freq's band, that's enough, so lets
         * not overwrite it once found
         */
        if (!band_rule_found)                
    	    band_rule_found = freq_in_rule_band(fr, MHZ_TO_KHZ(center_freq));

		bw_fits = reg_does_bw_fit(fr,
					  MHZ_TO_KHZ(center_freq),
					  MHZ_TO_KHZ(20));
        
		if (band_rule_found && bw_fits) {
            if(fr->max_bandwidth_khz>=MHZ_TO_KHZ(40))
            {           
			    return TRUE;
            }
		}
        
	}
    return FALSE;
    
}


        

