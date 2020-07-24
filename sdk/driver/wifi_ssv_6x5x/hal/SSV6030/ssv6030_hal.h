#ifndef _SSV6030_HAL_H_
#define _SSV6030_HAL_H_

#include <hal/SSV6030/regs/ssv6051_reg.h>

//#define CONFIG_SSV_CABRIO_A			1
#define CONFIG_SSV_CABRIO_E			1

/*-------------------------HW RF setting-------------------------------*/
//For IC with IPD: ID_SSV6051Z/6030P, Otherwise set to 0

#if (CONFIG_CHIP_ID==SSV6051Z || CONFIG_CHIP_ID==SSV6030P)
    #define SSV_IPD 1
    //Internal LDO setting([MP4-4.2V]=0 or [ON BOARD IC-3.3V]=1)
    //If IPD=1, INTERNAL_LDO MUST to 1
    #define SSV_INTERNAL_LDO    1
#else//CONFIG_ID_SSV6051Z
    #define SSV_IPD 0
    #define SSV_INTERNAL_LDO    0

#endif//CONFIG_ID_SSV6051Z

#define DO_IQ_CALIBRATION 1

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

/*SW reset bit*/
#define MAC_SW_RST                      BIT(1)
#define MCU_SW_RST                      BIT(2)
#define SDIO_SW_RST                     BIT(3)
#define SPI_SLV_SW_RST                  BIT(4)
#define SYS_ALL_RST                     BIT(21)


#define SSV6051_MAC_DECITBL1_SIZE           16
#define SSV6051_MAC_DECITBL2_SIZE           9

#define SSV6051_ADDRESS_OFFSET              16
#define SSV6051_HW_ID_OFFSET                7

#define SSV6051_RXINFO_SIZE                 0x18

#define SSV6051_FW_STATUS_REG               ADR_TX_SEG

#define SSV6051_RX_PB_OFFSET                   80
#define SSV6051_TX_PB_OFFSET                   80

#define SSV6051_PBUF_BASE_ADDR	            0x80000000
#define SSV6051_PBUF_ADDR_SHIFT	            16

#define SSV6051_PBUF_MapPkttoID(_PKT)		(((ssv_type_u32)_PKT&0x0FFF0000)>>SSV6051_PBUF_ADDR_SHIFT)
#define SSV6051_PBUF_MapIDtoPkt(_ID)		(SSV6051_PBUF_BASE_ADDR|((_ID)<<SSV6051_PBUF_ADDR_SHIFT))

//TX_PKT_RSVD(3) * unit(16)
#define SSV6051_TX_PKT_RSVD_SETTING         0x5 //From 3 to 5, it's for RB-450,we need more 22 bytes  to move the ampdu frame
#define SSV6051_TX_PKT_RSVD                 SSV6051_TX_PKT_RSVD_SETTING*16
#define SSV6051_ALLOC_RSVD                  SSV6051_TX_PB_OFFSET+SSV6051_TX_PKT_RSVD

#define SSV6051_HW_MMU_PAGE_SHIFT           0x8
#define SSV6051_HW_MMU_PAGE_MASK            0xff

#define SSV6051_ID_TX_THRESHOLD         30    //64
/*Avoide MCU input queue full into deadlock (HCI output queue full, MACRX output queue full)
  Add MICRX after MCU in Rx DATA flow. (HCI:16 + MICRX:4+4) */
#define SSV6051_ID_RX_THRESHOLD         31    //64
#define SSV6051_ID_RX_THRESHOLD_LOW         28    //64
#define SSV6051_PAGE_TX_THRESHOLD       g_host_cfg.tx_res_page //59    //128
#define SSV6051_PAGE_RX_THRESHOLD       g_host_cfg.rx_res_page //61    //128
#define SSV6051_PAGE_RX_THRESHOLD_LOW       43    //128

#define SSV6051_TX_LOWTHRESHOLD_PAGE_TRIGGER            45
#define SSV6051_TX_LOWTHRESHOLD_ID_TRIGGER 		2

#define SSV6051_ACS_AGC_GAIN                            0x1f30Cfff

typedef enum _edca_tx_queue_type {
	EDCA_AC_BK	= 0	,
	EDCA_AC_BE		,
	EDCA_AC_VI		,
	EDCA_AC_VO 		,						
	EDCA_AC_MAX		,
	EDCA_AC_ALL		= EDCA_AC_MAX,
	EDCA_AC_BC      = EDCA_AC_MAX,
	EDCA_AC_BCN     ,	
	EDCA_QUE_MAX	
	
}edca_tx_queue_type;

#define SECURITY_KEY_LEN              (32)

STRUCT_PACKED struct ssv6xxx_hw_key {
    ssv_type_u8          key[SECURITY_KEY_LEN];
    ssv_type_u32			tx_pn_l;
    ssv_type_u32         tx_pn_h;
    ssv_type_u32        	rx_pn_l;
    ssv_type_u32         rx_pn_h;
};

STRUCT_PACKED struct ssv6xxx_hw_sta_key {
    ssv_type_u8         	pair_key_idx:4;		/* 0: pairwise key, 1-3: group key */
    ssv_type_u8         	group_key_idx:4;	/* 0: pairwise key, 1-3: group key */
    ssv_type_u8          valid;              /* 0: invalid entry, 1: valid entry asic hw don't check this field*/
    ssv_type_u8			reserve[2];
    struct ssv6xxx_hw_key	pair;
};

STRUCT_PACKED struct ssv6xxx_hw_sec {
    struct ssv6xxx_hw_key group_key[3];	//mapping to protocol group key 1-3
    struct ssv6xxx_hw_sta_key sta_key[8];
};

#endif /* _SSV6030_HAL_H_ */

