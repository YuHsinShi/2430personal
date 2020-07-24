#ifndef _SSV6006_HAL_H_
#define _SSV6006_HAL_H_

#include "regs/ssv6006_hw_regs.h"

/* Hardware Offload Engine ID */
#define M_ENG_CPU                       0x00
#define M_ENG_HWHCI                     0x01
//#define M_ENG_FRAG                    0x02
#define M_ENG_EMPTY                     0x02
#define M_ENG_ENCRYPT                   0x03
#define M_ENG_MACRX                     0x04
#define M_ENG_MIC                       0x05
#define M_ENG_TX_EDCA0                  0x06
#define M_ENG_TX_EDCA1                  0x07
#define M_ENG_TX_EDCA2                  0x08
#define M_ENG_TX_EDCA3                  0x09
#define M_ENG_TX_MNG                    0x0A
#define M_ENG_ENCRYPT_SEC               0x0B
#define M_ENG_MIC_SEC                   0x0C
#define M_ENG_RESERVED_1                0x0D
#define M_ENG_RESERVED_2                0x0E
#define M_ENG_TRASH_CAN                 0x0F
#define M_ENG_MAX                      (M_ENG_TRASH_CAN+1)


/* Software Engine ID: */
#define M_CPU_HWENG                     0x00
#define M_CPU_TXL34CS                   0x01
#define M_CPU_RXL34CS                   0x02
#define M_CPU_DEFRAG                    0x03
#define M_CPU_EDCATX                    0x04
#define M_CPU_RXDATA                    0x05
#define M_CPU_RXMGMT                    0x06
#define M_CPU_RXCTRL                    0x07
#define M_CPU_FRAG                      0x08
#define M_CPU_RXMGMT_ACS                0x09

/*The queue's depth of HW */
#define M_ENG_CPU_INT               16
#define M_ENG_CPU_OUT               16
#define M_ENG_HWHCI_INT             8
#define M_ENG_HWHCI_OUT             16
#define M_ENG_EMPTY_INT             0
#define M_ENG_EMPTY_OUT             0
#define M_ENG_ENCRYPT_INT           4
#define M_ENG_ENCRYPT_OUT           16
#define M_ENG_MACRX_INT             16
#define M_ENG_MACRX_OUT             0
#define M_ENG_MIC_INT               4
#define M_ENG_MIC_OUT               4
#define M_ENG_TX_EDCA0_INT          4
#define M_ENG_TX_EDCA0_OUT          8
#define M_ENG_TX_EDCA1_INT          4
#define M_ENG_TX_EDCA1_OUT          16
#define M_ENG_TX_EDCA2_INT          4
#define M_ENG_TX_EDCA2_OUT          16
#define M_ENG_TX_EDCA3_INT          4
#define M_ENG_TX_EDCA3_OUT          16
#define M_ENG_TX_MNG_INT            4
#define M_ENG_TX_MNG_OUT            8
#define M_ENG_ENCRYPT_SEC_INT       4
#define M_ENG_ENCRYPT_SEC_OUT       16
#define M_ENG_MIC_SEC_INT           4
#define M_ENG_MIC_SEC_OUT           4
#define M_ENG_RESERVED_1_INT        0
#define M_ENG_RESERVED_1_OUT        0
#define M_ENG_RESERVED_2_INT        0
#define M_ENG_RESERVED_2_OUT        0
#define M_ENG_TRASH_CAN_INT         0
#define M_ENG_TRASH_CAN_OUT         32
#ifndef ID_TRAP_SW_TXTPUT
#define ID_TRAP_SW_TXTPUT               50 //(ID_TRAP_SW_START + M_CPU_TXTPUT - 1)
#endif //ID_TRAP_SW_TXTPUT


#define SSV6006_MAC_DECITBL1_SIZE           16
#define SSV6006_MAC_DECITBL2_SIZE           9

#define SSV6006_ADDRESS_OFFSET              16
#define SSV6006_HW_ID_OFFSET                7

#define SSV6006_RX_PB_OFFSET                   80
#define SSV6006_TX_PB_OFFSET                   80

#if(SW_8023TO80211==1)
#define SSV6006_RXINFO_SIZE                 (40+40)//sizeof(PKT_RxInfo)=40,we need more 40 bytes when reuse RX PBUF (ex:ARP Response)
#define SSV6006_TXINFO_SIZE                 80//sizeof(PKT_TxInfo)=80
#else
#define SSV6006_RXINFO_SIZE                 28 //sizeof(chf_host_rxpkt) is 28 bytes
#define SSV6006_TXINFO_SIZE                 12
#endif


#define SSV6006_FW_STATUS_REG               ADR_TX_SEG

#define SSV6006_PBUF_BASE_ADDR	            0x80000000
#define SSV6006_PBUF_ADDR_SHIFT	            16

#define SSV6006_PBUF_MapPkttoID(_PKT)		(((ssv_type_u32)_PKT&0x0FFF0000)>>SSV6006_PBUF_ADDR_SHIFT)
#define SSV6006_PBUF_MapIDtoPkt(_ID)		(SSV6006_PBUF_BASE_ADDR|((_ID)<<SSV6006_PBUF_ADDR_SHIFT))

//TX_PKT_RSVD(3) * unit(16)
#define SSV6006_TX_PKT_RSVD_SETTING         0x3
#define SSV6006_TX_PKT_RSVD                 SSV6006_TX_PKT_RSVD_SETTING*16

#define SSV6006_ALLOC_RSVD                   SSV6006_TX_PB_OFFSET+SSV6006_TX_PKT_RSVD

#define SSV6006_HW_MMU_PAGE_SHIFT           0x8
#define SSV6006_HW_MMU_PAGE_MASK            0xff

#define SSV_MAX_WSID                            8

//Aluthough TX Page number is up to 128, TX ID only set 63.
//Because the register of TX_ID_ALL_INFO only reserve 6 bits to show that how many TX IDs are used.
//If we set TX ID over 63, we can't get the real status about how many TX IDs are using
//A TCP/UDP packet occupy 8 pages, the 128 pages only need 16 TX IDs
#define SSV6006_ID_TX_THRESHOLD         63
//Aluthough RX Page number is up to 128, RX ID only set 63.
//Becaue the number of msg_evt/dp of fw is depend on TX/RX ID, if the TX/RX ID is too large, 
//it will cause the fw to reseve a larger memory for msg_evt/dp
#define SSV6006_ID_RX_THRESHOLD         63
#define SSV6006_ID_RX_THRESHOLD_LOW     48
#define SSV6006_PAGE_TX_THRESHOLD       g_host_cfg.tx_res_page//128     
#define SSV6006_PAGE_RX_THRESHOLD       g_host_cfg.rx_res_page//118, 4 pages for secure, 6 pages for beacon
#define SSV6006_PAGE_RX_THRESHOLD_LOW   107     //128

#define SSV6006_TX_LOWTHRESHOLD_PAGE_TRIGGER            45
#define SSV6006_TX_LOWTHRESHOLD_ID_TRIGGER 		2

#define SSV6006_ACS_AGC_GAIN                            0x1f30Cfff

/**
* SSV6006 HW Security Table: 2 BSSID entry + 8 WSID entry (in packet buffer):
*/
#define SSV6006_HW_SEC_TABLE_SIZE       632 //2*BSSID(140 bytes) + 8*WSID(44 bytes)
#define SSV6006_HW_KEY_SIZE              32  //one hw key size, refer to ssv6006_hw_key
#define SSV6006_PAIRWISE_KEY_OFFSET      12  //refer to ssv6006_hw_sta_key
#define SSV6006_GROUP_KEY_OFFSET         12  //refer to ssv6006_bss

/**
* struct ssv6006_hw_sec - ssv6006 hardware security engine data structure.
* This descriptor is shared with ssv6006 hardware and driver.
*/
#define SSV6006_SECURITY_KEY_LEN              (32)

SSV_PACKED_STRUCT_BEGIN 
struct SSV_PACKED_STRUCT ssv6006_hw_key {
    ssv_type_u8          key[SSV6006_SECURITY_KEY_LEN];
}SSV_PACKED_STRUCT_STRUCT;
SSV_PACKED_STRUCT_END

/**
* Key index for each security
* - WEP security: key indexes 0~3 are all stored in pair_key_idx.
* - others security: pairwise key index is stored in pair_key_idx,
*                    group key indexes are stored in group_key_idx.
*/
SSV_PACKED_STRUCT_BEGIN 
struct SSV_PACKED_STRUCT ssv6006_hw_sta_key {
    ssv_type_u8         	pair_key_idx;       /* 0: pairwise key, 1-3: group key */
    ssv_type_u8         	pair_cipher_type;   //WEP,TKIP,CCMP
    ssv_type_u8          valid;              /* 0: invalid entry, 1: valid entry asic hw don't check this field*/
    ssv_type_u8          reserve[1];
    ssv_type_u32         tx_pn_l;
    ssv_type_u32         tx_pn_h;

    struct ssv6006_hw_key pair;
}SSV_PACKED_STRUCT_STRUCT;
SSV_PACKED_STRUCT_END

SSV_PACKED_STRUCT_BEGIN 
struct SSV_PACKED_STRUCT ssv6006_bss {
    ssv_type_u8          group_key_idx;
    ssv_type_u8          group_cipher_type;  //WEP,TKIP,CCMP
    ssv_type_u8          reserve[2];
    ssv_type_u32         tx_pn_l;
    ssv_type_u32         tx_pn_h;

	struct ssv6006_hw_key group_key[4]; //mapping to protocol group key 1-3
}SSV_PACKED_STRUCT_STRUCT;
SSV_PACKED_STRUCT_END

SSV_PACKED_STRUCT_BEGIN 
struct SSV_PACKED_STRUCT ssv6006_hw_sec {
    struct ssv6006_bss bss_group[2];       /* The maximal number of VIFs: 2 */
    struct ssv6006_hw_sta_key sta_key[8];  /* The maximal number of STAs: 8 */
}SSV_PACKED_STRUCT_STRUCT;
SSV_PACKED_STRUCT_END

#endif /* _SSV6006_HAL_H_ */

