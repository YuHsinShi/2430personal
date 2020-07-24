#ifndef _SSV6006_TBL_H_
#define _SSV6006_TBL_H_


#if 0
#define ssv6006_phy_tbl         ssv6006_geminiA_phy_setting
#define ssv6006_rf_tbl          ssv6006_geminiA_rf_setting
#else
#define ssv6006_phy_tbl         ssv6006_turismoB_phy_setting
#define ssv6006_rf_tbl          ssv6006_turismoB_rf_setting
#endif

#define _CLK_SEL_MAC_XTAL_  0
#define _CLK_SEL_MAC_PLL_   3

#define _CLK_SEL_PHY_XTAL_  0
#define _CLK_SEL_PHY_PLL_   1

struct ssv6051_dev_table {
    ssv_type_u32 address;
    ssv_type_u32 data;
};




#define SET_PHY_INFO( _ctsdur, _ba_rate_idx,                \
                        _ack_rate_idx, _llength_idx,        \
                        _llength_enable)                    \
                                                            \
                                    (_ctsdur<<16|           \
                                    _ba_rate_idx <<10|      \
                                    _ack_rate_idx<<4|       \
                                    _llength_idx<<1|        \
                                    _llength_enable)

//#define UPDATE_PHY_INFO_ACK_RATE(_phy_info, _ack_rate_idx) ( _phy_info = (_phy_info&0xfffffc0f)|(_ack_rate_idx<<4))



#define SET_PHY_L_LENGTH( _l_ba, _l_rts, _l_cts_ack )        (_l_ba<<12|_l_rts<<6 |_l_cts_ack)

/**
* SSV6200 PHY Info Table for both AP & STA mode.
*
*/

//static u32 phy_info_tbl[] =
//{
//    /* PHY Infor Table: */
//    0x00000000, 0x00000100, 0x00000200, 0x00000300, 0x00000140,
//    0x00000240, 0x00000340, 0x00000001, 0x00000101, 0x00000201,
//    0x00000301, 0x00000401, 0x00000501, 0x00000601, 0x00000701,
//    0x00030002, 0x00030102, 0x00030202, 0x00030302, 0x00030402,
//    0x00030502, 0x00030602, 0x00030702, 0x00030082, 0x00030182,
//    0x00030282, 0x00030382, 0x00030482, 0x00030582, 0x00030682,
//    0x00030782, 0x00030042, 0x00030142, 0x00030242, 0x00030342,
//    0x00030442, 0x00030542, 0x00030642, 0x00030742,
//
//    /* PHY Index Table: */
//
////B
//    0x013A0000, 0x01020010, 0x00df0010, 0x00d50010,
//  0x00a20040, 0x007f0040, 0x00750040,
////G
//  0x003c0070, 0x00340070, 0x00300090, 0x002c0090, 0x002c00b0, 0x002800b0, 0x002800b0, 0x002800b0,
////N
//  0x004c1c71, 0x00402493, 0x003c2495, 0x003c2cb7, 0x00382cb9, 0x00382cbb, 0x00382cbb, 0x00382cbb,
//  0x004c1c7d, 0x00402493, 0x003c2495, 0x003c2cb7, 0x00382cb9, 0x00382cbb, 0x00382cbb, 0x00382cbb,
//  0x00401c70, 0x00342490, 0x00302490, 0x00302cb0, 0x002c2cb0, 0x002c2cb0, 0x002c2cb0, 0x002c2cb0,
//
//    /* PHY LL-Length Table: */
//    0x000329a3, 0x0002375a, 0x0001d697, 0x0001a5d7, 0x000175d4,
//    0x00017514, 0x0002f9a3, 0x00000000
//};



#endif /* _SSV6030_TBL_H_ */

