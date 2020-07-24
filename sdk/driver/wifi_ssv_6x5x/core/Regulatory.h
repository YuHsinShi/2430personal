/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _REGULATORY_H_
#define _REGULATORY_H_
#include <ssv_types.h>

#define BW 20

extern int freq_reg_info_regd(ssv_type_u32 center_freq,ssv_type_u32 desired_bw_khz,const struct ieee80211_regdomain *regd);
extern ssv_type_bool freq_need_dfs(ssv_type_u32 center_freq,ssv_type_u32 desired_bw_khz,const struct ieee80211_regdomain *regd);
extern ssv_type_bool freq_40MHZ_Available(ssv_type_u32 center_freq,const struct ieee80211_regdomain *regd);

#endif //_REGULATORY_H_
