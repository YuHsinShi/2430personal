/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/
#include "channel.h"


#define CHAN2G(ch, _freq, _idx, _ch_type){\
    (ch).band = SSV_IEEE80211_BAND_2GHZ;\
    (ch).center_freq = (_freq);\
    (ch).hw_value = (_idx);\
    (ch).max_power = 20;\
    (ch).flags = _ch_type;     \
}

#define CHAN5G(ch, _freq, _idx,_ch_type)  {      \
    (ch).band = SSV_IEEE80211_BAND_5GHZ;   \
    (ch).center_freq = (_freq);         \
    (ch).hw_value = (_idx);             \
    (ch).max_power = 20;                \
    (ch).flags = _ch_type;     \
}

/**
* Note that this table maybe modified at run-time. We shall make a copy of 
* this table before using it.
*/
IEEE80211_CHANNEL ssv6xxx_2ghz_chantable[MAX_2G_CHANNEL_NUM];
IEEE80211_CHANNEL ssv6xxx_5ghz_chantable[MAX_5G_CHANNEL_NUM];

int ssv6xxx_channel_table_init(void)
{
	OS_MemSET((void*)&ssv6xxx_2ghz_chantable, 0x0, sizeof(ssv6xxx_2ghz_chantable));
	OS_MemSET((void*)&ssv6xxx_5ghz_chantable, 0x0, sizeof(ssv6xxx_5ghz_chantable));
    /*
    CHAN2G(2412, 1 ), // Channel 1 
    CHAN2G(2417, 2 ), // Channel 2 
    CHAN2G(2422, 3 ), // Channel 3 
    CHAN2G(2427, 4 ), // Channel 4 
    CHAN2G(2432, 5 ), // Channel 5 
    CHAN2G(2437, 6 ), // Channel 6 
    CHAN2G(2442, 7 ), // Channel 7 
    CHAN2G(2447, 8 ), // Channel 8 
    CHAN2G(2452, 9 ), // Channel 9 
    CHAN2G(2457, 10), // Channel 10 
    CHAN2G(2462, 11), // Channel 11 
    CHAN2G(2467, 12), // Channel 12 
    CHAN2G(2472, 13), // Channel 13 
    CHAN2G(2484, 14), // Channel 14     
    */
    CHAN2G(ssv6xxx_2ghz_chantable[1],2412,1,SSV_80211_CHAN_HT40PLUS);
    CHAN2G(ssv6xxx_2ghz_chantable[2],2417,2,SSV_80211_CHAN_HT40PLUS);
    CHAN2G(ssv6xxx_2ghz_chantable[3],2422,3,SSV_80211_CHAN_HT40PLUS);
    CHAN2G(ssv6xxx_2ghz_chantable[4],2427,4,SSV_80211_CHAN_HT40PLUS);
    CHAN2G(ssv6xxx_2ghz_chantable[5],2432,5,SSV_80211_CHAN_HT40PLUS);
    CHAN2G(ssv6xxx_2ghz_chantable[6],2437,6,SSV_80211_CHAN_HT40PLUS);
    CHAN2G(ssv6xxx_2ghz_chantable[7],2442,7,SSV_80211_CHAN_HT40PLUS);
    CHAN2G(ssv6xxx_2ghz_chantable[8],2447,8,SSV_80211_CHAN_HT40PLUS);
    CHAN2G(ssv6xxx_2ghz_chantable[9],2452,9,SSV_80211_CHAN_HT40PLUS);
    CHAN2G(ssv6xxx_2ghz_chantable[10],2457,10,SSV_80211_CHAN_HT40MINUS);
    CHAN2G(ssv6xxx_2ghz_chantable[11],2462,11,SSV_80211_CHAN_HT40MINUS);
    CHAN2G(ssv6xxx_2ghz_chantable[12],2467,12,SSV_80211_CHAN_HT40MINUS);
    CHAN2G(ssv6xxx_2ghz_chantable[13],2472,13,SSV_80211_CHAN_HT40MINUS);
    CHAN2G(ssv6xxx_2ghz_chantable[14],2484,14,SSV_80211_CHAN_HT40MINUS);

    /*
    // _We_ call this UNII 1 
	CHAN5G(5180, 36), // Channel 36 
	CHAN5G(5200, 40), // Channel 40 
	CHAN5G(5220, 44), // Channel 44 
	CHAN5G(5240, 48), // Channel 48 
	// _We_ call this UNII 2 
	CHAN5G(5260, 52), // Channel 52 
	CHAN5G(5280, 56), // Channel 56 
	CHAN5G(5300, 60), // Channel 60 
	CHAN5G(5320, 64), // Channel 64 
	// _We_ call this "Middle band" 
	CHAN5G(5500, 100), // Channel 100 
	CHAN5G(5520, 104), // Channel 104 
	CHAN5G(5540, 108), // Channel 108 
	CHAN5G(5560, 112), // Channel 112 
	CHAN5G(5580, 116), // Channel 116 
	CHAN5G(5600, 120), // Channel 120 
	CHAN5G(5620, 124), // Channel 124 
	CHAN5G(5640, 128), // Channel 128 
	CHAN5G(5660, 132), // Channel 132 
	CHAN5G(5680, 136), // Channel 136 
	CHAN5G(5700, 140), // Channel 140 
	// _We_ call this UNII 3 
	CHAN5G(5745, 149), // Channel 149 
	CHAN5G(5765, 153), // Channel 153 
	CHAN5G(5785, 157), // Channel 157 
	CHAN5G(5805, 161), // Channel 161 
	CHAN5G(5825, 165), // Channel 165 
	*/
    CHAN5G(ssv6xxx_5ghz_chantable[0],5180,36,SSV_80211_CHAN_HT40PLUS);
    CHAN5G(ssv6xxx_5ghz_chantable[1],5200,40,SSV_80211_CHAN_HT40MINUS);
    CHAN5G(ssv6xxx_5ghz_chantable[2],5220,44,SSV_80211_CHAN_HT40PLUS);
    CHAN5G(ssv6xxx_5ghz_chantable[3],5240,48,SSV_80211_CHAN_HT40MINUS);
    
    CHAN5G(ssv6xxx_5ghz_chantable[4],5260,52,SSV_80211_CHAN_HT40PLUS);
    CHAN5G(ssv6xxx_5ghz_chantable[5],5280,56,SSV_80211_CHAN_HT40MINUS);
    CHAN5G(ssv6xxx_5ghz_chantable[6],5300,60,SSV_80211_CHAN_HT40PLUS);
    CHAN5G(ssv6xxx_5ghz_chantable[7],5320,64,SSV_80211_CHAN_HT40MINUS);
    
    CHAN5G(ssv6xxx_5ghz_chantable[8],5500,100,SSV_80211_CHAN_HT40PLUS);
    CHAN5G(ssv6xxx_5ghz_chantable[9],5520,104,SSV_80211_CHAN_HT40MINUS);
    CHAN5G(ssv6xxx_5ghz_chantable[10],5540,108,SSV_80211_CHAN_HT40PLUS);
    CHAN5G(ssv6xxx_5ghz_chantable[11],5560,112,SSV_80211_CHAN_HT40MINUS);
    
    CHAN5G(ssv6xxx_5ghz_chantable[12],5580,116,SSV_80211_CHAN_HT40PLUS);
    CHAN5G(ssv6xxx_5ghz_chantable[13],5600,120,SSV_80211_CHAN_HT40MINUS);
    CHAN5G(ssv6xxx_5ghz_chantable[14],5620,124,SSV_80211_CHAN_HT40PLUS);
    CHAN5G(ssv6xxx_5ghz_chantable[15],5640,128,SSV_80211_CHAN_HT40MINUS);

    CHAN5G(ssv6xxx_5ghz_chantable[16],5660,132,SSV_80211_CHAN_HT40PLUS);
    CHAN5G(ssv6xxx_5ghz_chantable[17],5680,136,SSV_80211_CHAN_HT40MINUS);
    CHAN5G(ssv6xxx_5ghz_chantable[18],5700,140,SSV_80211_CHAN_HT40PLUS);
    CHAN5G(ssv6xxx_5ghz_chantable[19],5720,144,SSV_80211_CHAN_HT40MINUS);
    
    CHAN5G(ssv6xxx_5ghz_chantable[20],5745,149,SSV_80211_CHAN_HT40PLUS);
    CHAN5G(ssv6xxx_5ghz_chantable[21],5765,153,SSV_80211_CHAN_HT40MINUS);
    CHAN5G(ssv6xxx_5ghz_chantable[22],5785,157,SSV_80211_CHAN_HT40PLUS);
    CHAN5G(ssv6xxx_5ghz_chantable[23],5805,161,SSV_80211_CHAN_HT40MINUS);

    CHAN5G(ssv6xxx_5ghz_chantable[24],5825,165,SSV_80211_CHAN_HT20);

    return 0;
}

