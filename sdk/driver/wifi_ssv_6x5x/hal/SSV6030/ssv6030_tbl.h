#ifndef _SSV6030_TBL_H_
#define _SSV6030_TBL_H_

#include <config.h>

#define ssv6200_phy_tbl         phy_setting
#define ssv6200_temp_phy_tbl    temp_phy_setting
#ifdef CONFIG_SSV_CABRIO_E
#define ssv6200_rf_tbl          asic_rf_setting
#else
#define ssv6200_rf_tbl          fpga_rf_setting
#endif

#define _CLK_SEL_MAC_XTAL_  0
#define _CLK_SEL_MAC_PLL_   3

#define _CLK_SEL_PHY_XTAL_  0
#define _CLK_SEL_PHY_PLL_   1

struct ssv6051_dev_table {
    ssv_type_u32 address;
    ssv_type_u32 data;
};


#define ssv6200_phy_tbl         phy_setting
#ifdef CONFIG_SSV_CABRIO_E
#define ssv6200_rf_tbl          asic_rf_setting
#else
#define ssv6200_rf_tbl          fpga_rf_setting
#endif





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

/* 00= 0dB,  01= -1dB,  02= -2dB,  03= -3dB,  04= -4dB   csr_infox<31:26> 6 bit */
static const ssv_type_u8 tx_rf_gain_enhance_table [] =
{
    0x00,     /* 1 M */
    0x00,     /* 2 M LONG PREAMBLE*/
    0x00,     /* 5.5 M LONG PREAMBLE*/
    0x00,     /* 11  M LONG PREAMBLE*/
    0x00,     /* 2 M    SHORT PREAMBLE*/
    0x00,     /* 5.5 M SHORT PREAMBLE*/
    0x00,     /* 11 M  SHORT PREAMBLE*/
    
    0x00,     /* 6 M  */
    0x00,     /* 9 M  */
    0x00,     /* 12 M  */
    0x00,     /* 18 M  */
    0x01,     /* 24 M  */
    0x01,     /* 36 M  */
    0x02,     /* 48 M  */
    0x02,     /* 54 M  */
    
    0x00,     /* 6.5 M LONG GI */
    0x00,     /* 13  M LONG GI */
    0x00,     /* 19.5  M LONG GI */
    0x01,     /* 26  M LONG GI */
    0x01,     /* 39  M LONG GI */
    0x03,     /* 52  M LONG GI */
    0x03,     /* 58  M LONG GI */
    0x03,     /* 65  M LONG GI */
    
    0x00,     /* 7.2 M SHORT GI */
    0x00,     /* 14.4 M SHORT GI */
    0x00,     /* 21.7 M SHORT GI */
    0x01,     /* 28.9 M SHORT GI */
    0x01,     /* 43.3 M SHORT GI */
    0x03,     /* 57.8 M SHORT GI */
    0x03,     /* 65 M SHORT GI */
    0x03,     /* 72.2 M SHORT GI */
    
    0x00,     /* 6.5 M GREEN FILED */
    0x00,     /* 13 M GREEN FILED */
    0x00,     /* 19.5 M GREEN FILED */
    0x01,     /* 26 M GREEN FILED */
    0x01,     /* 39 M GREEN FILED */
    0x03,     /* 52 M GREEN FILED */
    0x03,     /* 58.5 M GREEN FILED */
    0x03,     /* 65 M GREEN FILED */
};

/**
* SSV6200 PHY Info Table for both AP & STA mode.
*
*/
static const ssv_type_u32 phy_info_tbl[] =
    {
#if (CONFIG_CHIP_ID == SSV6051Z)
        /* PHY Infor Table: */
		0x18000000, 0x18000100, 0x18000200, 0x18000300, 0x18000140,
		0x18000240, 0x18000340, 0x0C000001, 0x0C000101, 0x0C000201,
		0x0C000301, 0x18000401, 0x18000501, 0x18000601, 0x18000701,
		0x0C030002, 0x0C030102, 0x0C030202, 0x18030302, 0x18030402,
		0x18030502, 0x18030602, 0x1C030702, 0x0C030082, 0x0C030182,
		0x0C030282, 0x18030382, 0x18030482, 0x18030582, 0x18030682,
		0x1C030782, 0x0C030042, 0x0C030142, 0x0C030242, 0x18030342,
		0x18030442, 0x18030542, 0x18030642, 0x1C030742,
#else // 6051Q & 6030P
        /* PHY Infor Table: */
        0x0C000000,     /* 1 M */
        0x0C000100,     /* 2 M LONG PREAMBLE*/
        0x0C000200,     /* 5.5 M LONG PREAMBLE*/
        0x0C000300,     /* 11  M LONG PREAMBLE*/
        0x0C000140,     /* 2 M    SHORT PREAMBLE*/
        0x0C000240,     /* 5.5 M SHORT PREAMBLE*/
        0x0C000340,     /* 11 M  SHORT PREAMBLE*/

        0x00000001,     /* 6 M  */
        0x00000101,     /* 9 M  */
        0x00000201,     /* 12 M  */
        0x00000301,     /* 18 M  */
        0x0C000401,     /* 24 M  */
        0x0C000501,     /* 36 M  */
        0x0C000601,     /* 48 M  */
        0x0C000701,     /* 54 M  */

        0x00030002,     /* 6.5 M LONG GI */
        0x00030102,     /* 13  M LONG GI */
        0x00030202,     /* 19.5  M LONG GI */
        0x0C030302,     /* 26  M LONG GI */
        0x0C030402,     /* 39  M LONG GI */
        0x0C030502,     /* 52  M LONG GI */
        0x0C030602,     /* 58  M LONG GI */
        0x10030702,     /* 65  M LONG GI */

        0x00030082,     /* 7.2 M SHORT GI */
        0x00030182,     /* 14.4 M SHORT GI */
        0x00030282,     /* 21.7 M SHORT GI */
        0x0C030382,     /* 28.9 M SHORT GI */
        0x0C030482,     /* 43.3 M SHORT GI */
        0x0C030582,     /* 57.8 M SHORT GI */
        0x0C030682,     /* 65 M SHORT GI */
        0x10030782,     /* 72.2 M SHORT GI */

        0x00030042,     /* 6.5 M GREEN FILED */
        0x00030142,     /* 13 M GREEN FILED */
        0x00030242,     /* 19.5 M GREEN FILED */
        0x0C030342,     /* 26 M GREEN FILED */
        0x0C030442,     /* 39 M GREEN FILED */
        0x0C030542,     /* 52 M GREEN FILED */
        0x0C030642,     /* 58.5 M GREEN FILED */
        0x10030742,     /* 65 M GREEN FILED */
#endif

        /* PHY Index Table: */

        //B
        SET_PHY_INFO(314, 0, 0, 0, 0),  //0x013A0000,
        SET_PHY_INFO(258, 0, 1, 0, 0),  //0x01020010,
        SET_PHY_INFO(223, 0, 1, 0, 0),  //0x00df0010,
        SET_PHY_INFO(213, 0, 1, 0, 0),  //0x00d50010,

        SET_PHY_INFO(162, 0, 4, 0, 0),  //0x00a20040,
        SET_PHY_INFO(127, 0, 4, 0, 0),  //0x007f0040,
        SET_PHY_INFO(117, 0, 4, 0, 0),  //0x00750040,

        //G
        SET_PHY_INFO(60, 7, 7, 0, 0),  //0x003c0070,
        SET_PHY_INFO(60, 7, 7, 0, 0),  //0x00340070,
        SET_PHY_INFO(48, 9, 9, 0, 0),  //0x00300090,
        SET_PHY_INFO(48, 9, 9, 0, 0),  //0x002c0090,

        SET_PHY_INFO(44, 11, 11, 0, 0),  //0x002c00b0,
        SET_PHY_INFO(44, 11, 11, 0, 0),  //0x002800b0,
        SET_PHY_INFO(44, 11, 11, 0, 0),  //0x002800b0,
        SET_PHY_INFO(44, 11, 11, 0, 0),  //0x002800b0,


        //N
        SET_PHY_INFO(60,  7,  7, 0, 1),  //0x004c1c71,
        SET_PHY_INFO(48,  9,  9, 1, 1),  //0x00402493,
        SET_PHY_INFO(48,  9,  9, 2, 1),  //0x003c2495,
        SET_PHY_INFO(44, 11, 11, 3, 1),  //0x003c2cb7,
        SET_PHY_INFO(44, 11, 11, 4, 1),  //0x00382cb9,
        SET_PHY_INFO(44, 11, 11, 5, 1),  //0x00382cbb,
        SET_PHY_INFO(44, 11, 11, 5, 1),  //0x00382cbb,
        SET_PHY_INFO(44, 11, 11, 5, 1),  //0x00382cbb,


        SET_PHY_INFO(60,  7,  7, 6, 1),  //0x004c1c7d,
        SET_PHY_INFO(48,  9,  9, 1, 1),  //0x00402493,
        SET_PHY_INFO(48,  9,  9, 2, 1),  //0x003c2495,
        SET_PHY_INFO(44, 11, 11, 3, 1),  //0x003c2cb7,
        SET_PHY_INFO(44, 11, 11, 4, 1),  //0x00382cb9,
        SET_PHY_INFO(44, 11, 11, 5, 1),  //0x00382cbb,
        SET_PHY_INFO(44, 11, 11, 5, 1),  //0x00382cbb,
        SET_PHY_INFO(44, 11, 11, 5, 1),  //0x00382cbb,


        SET_PHY_INFO(60,  7,  7, 0, 0),  //0x00401c70,
        SET_PHY_INFO(48,  9,  9, 0, 0),  //0x00342490,
        SET_PHY_INFO(48,  9,  9, 0, 0),  //0x00302490,
        SET_PHY_INFO(44, 11, 11, 0, 0),  //0x00302cb0,
        SET_PHY_INFO(44, 11, 11, 0, 0),  //0x002c2cb0,
        SET_PHY_INFO(44, 11, 11, 0, 0),  //0x002c2cb0,
        SET_PHY_INFO(44, 11, 11, 0, 0),  //0x002c2cb0,
        SET_PHY_INFO(44, 11, 11, 0, 0),  //0x002c2cb0,

        /* PHY LL-Length Table: */
        SET_PHY_L_LENGTH(50, 38, 35),//0x000329a3
        SET_PHY_L_LENGTH(35, 29, 26),//0x0002375a
        SET_PHY_L_LENGTH(29, 26, 23),//0x0001d697
        SET_PHY_L_LENGTH(26, 23, 23),//0x0001a5d7
        SET_PHY_L_LENGTH(23, 23, 20),//0x000175d4

        SET_PHY_L_LENGTH(23, 20, 20),//0x00017514
        SET_PHY_L_LENGTH(47, 38, 35),//0x0002f9a3
        SET_PHY_L_LENGTH( 0,  0,  0),//0x00000000
    };
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

