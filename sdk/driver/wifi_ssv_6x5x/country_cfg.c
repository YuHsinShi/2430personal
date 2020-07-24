/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/
#include <porting.h>
#include <ssv_types.h>
#include <country_cfg.h>


#define REG_COUNTRY(reg,c) \
{   \
    (reg).alpha2[0]=c[0]; \
    (reg).alpha2[1]=c[1]; \
}

#define REG_RULES_NUM(reg,num) \
{   \
    (reg).n_reg_rules=num; \
}

#define REG_RULE(reg, rul_num, start, end, bw, gain, eirp, reg_flags) \
{							\
	(reg).reg_rules[rul_num].freq_range.start_freq_khz = MHZ_TO_KHZ(start);	\
    (reg).reg_rules[rul_num].freq_range.end_freq_khz = MHZ_TO_KHZ(end);    \
    (reg).reg_rules[rul_num].freq_range.max_bandwidth_khz = MHZ_TO_KHZ(bw);    \
    (reg).reg_rules[rul_num].power_rule.max_antenna_gain = DBI_TO_MBI(gain);\
    (reg).reg_rules[rul_num].power_rule.max_eirp = DBM_TO_MBM(eirp);    \
    (reg).reg_rules[rul_num].flags = reg_flags;                \
}




struct ieee80211_regdomain regdom[MAX_COUNTRY_NUM];

int ssv6xxx_wifi_init_regdom(void)
{
    OS_MemSET((void *)regdom, 0, sizeof(regdom));

    /* counter code 00 */
    REG_COUNTRY(regdom[0],"00");
    REG_RULES_NUM(regdom[0],7);
    // IEEE 802.11b/g, channels 1..11 
	REG_RULE(regdom[0],0,2402, 2472, 40, 0, 20, 0);
	// IEEE 802.11b/g, channels 12..13. 
	REG_RULE(regdom[0],1,2457, 2482, 40, 0, 20,0);
	//IEEE 802.11 channel 14 - Only JP enables this and for 802.11b only 
	REG_RULE(regdom[0],2,2474, 2494, 20, 0, 20, 0);
	// channel 36..48 
	REG_RULE(regdom[0],3,5170, 5250, 80, 0, 20, 0);
    // channel 52..64 
    REG_RULE(regdom[0],4,5250, 5330, 80, 0, 20, NL80211_RRF_DFS);
    // channel 100..144 
    REG_RULE(regdom[0],5,5490, 5730, 160, 0, 20, NL80211_RRF_DFS); 
    // channel 149..165 
    REG_RULE(regdom[0],6,5735, 5835, 80, 0, 20, 0);     

    /* counter code CN */
    REG_COUNTRY(regdom[1],"CN");
    REG_RULES_NUM(regdom[1],4);
	REG_RULE(regdom[1],0,2402, 2482, 40, 0, 20, 0);
	REG_RULE(regdom[1],1,5170, 5250, 80, 0, 23, 0); 
	REG_RULE(regdom[1],2,5250, 5330, 80, 0, 23, NL80211_RRF_DFS);     
	REG_RULE(regdom[1],3,5735, 5835, 80, 0, 30, 0); 

    /* counter code TW */
    REG_COUNTRY(regdom[2],"TW");
    REG_RULES_NUM(regdom[2],5);
	REG_RULE(regdom[2],0,2402, 2472, 40, 0, 30, 0);
	REG_RULE(regdom[2],1,5270, 5330, 40, 0, 17,NL80211_RRF_DFS);
	REG_RULE(regdom[2],2,5490, 5590, 80, 0, 30,NL80211_RRF_DFS);    
	REG_RULE(regdom[2],3,5650, 5710, 40, 0, 30,NL80211_RRF_DFS);    
	REG_RULE(regdom[2],4,5735, 5815, 80, 0, 30,0);    

    /* counter code TW */
    REG_COUNTRY(regdom[3],"HK");
    REG_RULES_NUM(regdom[3],5);
	REG_RULE(regdom[3],0,2402, 2482, 40, 0, 20, 0);
	REG_RULE(regdom[3],1,5170, 5250, 80, 0, 17, 0);
	REG_RULE(regdom[3],2,5250, 5330, 80, 0, 24, NL80211_RRF_DFS);    
	REG_RULE(regdom[3],3,5490, 5710, 160, 0, 24,NL80211_RRF_DFS);    
	REG_RULE(regdom[3],4,5735, 5835, 80, 0, 30,0);   
	
	return 0;   
}

