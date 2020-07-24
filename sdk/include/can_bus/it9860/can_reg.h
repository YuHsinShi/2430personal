#ifndef __CAN_REG_H__
#define __CAN_REG_H__

#include "ith/ith_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//Constant Definition
//=============================================================================
//HW Parameter Description
#define RAM_MEMTYPE               0x4
#define CFG_RBUF_SLOTS_NUMMBER    0x10
#define CFG_ACF_NUMBER            0x10
#define CFG_STB_ENABLE            0x1
#define CFG_STB_SLOTS_NUMBER      0x10
#define CFG_CAN_FD                0x1
#define CFG_UPWARD_COMPATIBILITY  0x1
#define CFG_TTCAN                 1
#define CFG_CIA603                1
#define CFG_CIA603_CLK            1

//=============================================================================
//Setting Register Define
//=============================================================================
// (*) :  register can only be written if bit RESET in register CFG_STAT is set.

//Receive Buffer Registers // 80byte
#define CAN_RB_IDENTIFIER_REG             0x00
//
#define CAN_RB_CONTROL_REG                0x04
#define CAN_RB_DLC                          (N04_BITS_MSK<< 0)
#define CAN_RB_DLC_SHIFT                     0
#define CAN_RB_BRS                          (N01_BITS_MSK<< 4)
#define CAN_RB_BRS_SHIFT                     4
#define CAN_RB_EDL                          (N01_BITS_MSK<< 5)
#define CAN_RB_EDL_SHIFT                     5
#define CAN_RB_RTR                          (N01_BITS_MSK<< 6)
#define CAN_RB_RTR_SHIFT                     6
#define CAN_RB_IDE                          (N01_BITS_MSK<< 7)
#define CAN_RB_IDE_SHIFT                     7
#define CAN_RB_TX                           (N01_BITS_MSK<<12)
#define CAN_RB_TX_SHIFT                      12
#define CAN_RB_KOER                         (N03_BITS_MSK<<13)
#define CAN_RB_KOER_SHIFT                    13
//
#define CAN_RB_DATA0_REG                  0x08  //if TTCAN = 1 , bit (15:0) is CYCLE TIME
#define CAN_RB_DATA1_REG                  0x0c
#define CAN_RB_DATA2_REG                  0x10
#define CAN_RB_DATA3_REG                  0x14
#define CAN_RB_DATA4_REG                  0x18
#define CAN_RB_DATA5_REG                  0x1c
#define CAN_RB_DATA6_REG                  0x20
#define CAN_RB_DATA7_REG                  0x24
#define CAN_RB_DATA8_REG                  0x28
#define CAN_RB_DATA9_REG                  0x2c
#define CAN_RB_DATA10_REG                 0x30
#define CAN_RB_DATA11_REG                 0x34
#define CAN_RB_DATA12_REG                 0x38
#define CAN_RB_DATA13_REG                 0x3c
#define CAN_RB_DATA14_REG                 0x40
#define CAN_RB_DATA15_REG                 0x44
//
#define CAN_RB_RTS0_REG                   0x48
//
#define CAN_RB_RTS1_REG                   0x4c

//Transmit Buffer Registers // 72byte
#define CAN_TB_IDENTIFIER_REG             0x50
#define CAN_TB_CONTROL_REG                0x54
#define CAN_TB_DLC                          (N04_BITS_MSK<< 0)
#define CAN_TB_DLC_SHIFT                     0
#define CAN_TB_BRS                          (N01_BITS_MSK<< 4)
#define CAN_TB_BRS_SHIFT                     4
#define CAN_TB_EDL                          (N01_BITS_MSK<< 5)
#define CAN_TB_EDL_SHIFT                     5
#define CAN_TB_RTR                          (N01_BITS_MSK<< 6)
#define CAN_TB_RTR_SHIFT                     6
#define CAN_TB_IDE                          (N01_BITS_MSK<< 7)
#define CAN_TB_IDE_SHIFT                     7
#define CAN_TB_DATA0_REG                  0x58
#define CAN_TB_DATA1_REG                  0x5c
#define CAN_TB_DATA2_REG                  0x60
#define CAN_TB_DATA3_REG                  0x64
#define CAN_TB_DATA4_REG                  0x68
#define CAN_TB_DATA5_REG                  0x6c
#define CAN_TB_DATA6_REG                  0x70
#define CAN_TB_DATA7_REG                  0x74
#define CAN_TB_DATA8_REG                  0x78
#define CAN_TB_DATA9_REG                  0x7c
#define CAN_TB_DATA10_REG                 0x80
#define CAN_TB_DATA11_REG                 0x84
#define CAN_TB_DATA12_REG                 0x88
#define CAN_TB_DATA13_REG                 0x8c
#define CAN_TB_DATA14_REG                 0x90
#define CAN_TB_DATA15_REG                 0x94

//Transmission Time Stamp TTS (0x98 to 0x9f)
#define CAN_TT_STAMP0_REG                 0x98
#define CAN_TT_STAMP1_REG                 0x9c
//
#define CAN_CFG_STAT_REG                  0xa0
#define CAN_BUSOFF                          (N01_BITS_MSK<< 0)//Bus Off
#define CAN_TACTIVE                         (N01_BITS_MSK<< 1)//Transmission ACTIVE
#define CAN_RACTIVE                         (N01_BITS_MSK<< 2)//Reception ACTIVE
#define CAN_TSSS                            (N01_BITS_MSK<< 3)//Transmission Secondary Single Shot mode for STB
#define CAN_TPSS                            (N01_BITS_MSK<< 4)//Transmission Primary Single Shot mode for PTB
#define CAN_LBMI                            (N01_BITS_MSK<< 5)//Loop Back Mode, Internal
#define CAN_LBME                            (N01_BITS_MSK<< 6)//Loop Back Mode, External
#define CAN_RESET                           (N01_BITS_MSK<< 7)//RESET request bit  
#define CAN_TCMD_REG                      0xa1
#define CAN_TSA                             (N01_BITS_MSK<< 8)//Transmit Secondary Abort
#define CAN_TSA_SHIFT                        8
#define CAN_TSALL                           (N01_BITS_MSK<< 9)//Transmit Secondary ALL frames
#define CAN_TSALL_SHIFT                      9
#define CAN_TSONE                           (N01_BITS_MSK<<10)//Transmit Secondary ONE frame
#define CAN_TSONE_SHIFT                      10
#define CAN_TPA                             (N01_BITS_MSK<<11)//Transmit Primary Abort
#define CAN_TPA_SHIFT                        11
#define CAN_TPE                             (N01_BITS_MSK<<12)//Transmit Primary Enable
#define CAN_TPE_SHIFT                        12
#define CAN_STBY                            (N01_BITS_MSK<<13)//Transceiver Standby Mode
#define CAN_STBY_SHIFT                       13
#define CAN_LOM                             (N01_BITS_MSK<<14)//Listen Only Mode
#define CAN_LOM_SHIFT                        14
#define CAN_TBSEL                           (N01_BITS_MSK<<15)//Transmit Buffer Select
#define CAN_TBSEL_SHIFT                      15
#define CAN_TCTRL_REG                     0xa2
#define CAN_TSSTAT                          (N02_BITS_MSK<<16)//Transmission Secondary STATus bits
#define CAN_TSSTAT_SHIFT                     16
#define CAN_TTTBM                           (N01_BITS_MSK<<20)//TTCAN Transmit Buffer Mode
#define CAN_TTTBM_SHIFT                      20
#define CAN_TSMODE                          (N01_BITS_MSK<<21)//Transmit buffer Secondary operation MODE
#define CAN_TSMODE_SHIFT                     21
#define CAN_TSNEXT                          (N01_BITS_MSK<<22)//Transmit buffer Secondary NEXT
#define CAN_TSNEXT_SHIFT                     22
#define CAN_FD_ISO                          (N01_BITS_MSK<<23)//CAN FD ISO mode //(*)
#define CAN_FD_ISO_SHIFT                     23
#define CAN_RCTRL_REG                     0xa3
#define CAN_RSTAT                           (N02_BITS_MSK<<24)//Receive buffer STATus
#define CAN_RSTAT_SHIFT                      24
#define CAN_RBALL                           (N01_BITS_MSK<<27)//Receive Buffer stores ALL data frames
#define CAN_RBALL_SHIFT                      27
#define CAN_RREL                            (N01_BITS_MSK<<28)//Receive buffer RELease
#define CAN_RREL_SHIFT                       28
#define CAN_ROV                             (N01_BITS_MSK<<29)//Receive buffer OVerflow
#define CAN_ROV_SHIFT                        29
#define CAN_ROM                             (N01_BITS_MSK<<30)//Receive buffer Overflow Mode
#define CAN_ROM_SHIFT                        30
#define CAN_SACK                            (N01_BITS_MSK<<31)//Self-ACKnowledge
#define CAN_SACK_SHIFT                       31  
//
#define CAN_RTIE_REG                      0xa4
#define CAN_TSFF                            (N01_BITS_MSK<< 0)//Transmit Secondary buffer Full Flag
#define CAN_EIE                             (N01_BITS_MSK<< 1)//Error Interrupt Enable
#define CAN_TSIE                            (N01_BITS_MSK<< 2)//Transmission Secondary Interrupt Enable
#define CAN_TPIE                            (N01_BITS_MSK<< 3)//Transmission Primary Interrupt Enable
#define CAN_RAFIE                           (N01_BITS_MSK<< 4)//RB Almost Full Interrupt Enable
#define CAN_RFIE                            (N01_BITS_MSK<< 5)//RB Full Interrupt Enable
#define CAN_ROIE                            (N01_BITS_MSK<< 6)//RB Overrun Interrupt Enable
#define CAN_RIE                             (N01_BITS_MSK<< 7)//Receive Interrupt Enable
#define CAN_RTIF_REG                      0xa5
#define CAN_AIF                             (N01_BITS_MSK<< 8)//Abort Interrupt Flag
#define CAN_EIF                             (N01_BITS_MSK<< 9)//Error Interrupt Flag
#define CAN_TSIF                            (N01_BITS_MSK<<10)//Transmission Secondary Interrupt Flag
#define CAN_TPIF                            (N01_BITS_MSK<<11)//Transmission Primary Interrupt Flag
#define CAN_RAFIF                           (N01_BITS_MSK<<12)//RB Almost Full Interrupt Flag
#define CAN_RFIF                            (N01_BITS_MSK<<13)//RB Full Interrupt Flag
#define CAN_ROIF                            (N01_BITS_MSK<<14)//RB Overrun Interrupt Flag
#define CAN_RIF                             (N01_BITS_MSK<<15)//Receive Interrupt Flag
#define CAN_ERRINT_REG                    0xa6
#define CAN_BEIF                            (N01_BITS_MSK<<16)//Bus Error Interrupt Flag
#define CAN_BEIE                            (N01_BITS_MSK<<17)//Bus Error Interrupt Enable
#define CAN_ALIF                            (N01_BITS_MSK<<18)//Arbitration Lost Interrupt Flag
#define CAN_ALIE                            (N01_BITS_MSK<<19)//Arbitration Lost Interrupt Enable
#define CAN_EPIF                            (N01_BITS_MSK<<20)//Error Passive Interrupt Flag
#define CAN_EPIE                            (N01_BITS_MSK<<21)//Error Passive Interrupt Enable
#define CAN_EPASS                           (N01_BITS_MSK<<22)//Error Passive mode active
#define CAN_EWARN                           (N01_BITS_MSK<<23)//Error WARNing limit reached
#define CAN_LIMIT_REG                     0xa7 //(*)
#define CAN_EWL                             (N04_BITS_MSK<<24)//Programmable Error Warning Limit
#define CAN_AFWL                            (N04_BITS_MSK<<28)//receive buffer Almost Full Warning Limit
//
#define CAN_S_Seg_1_REG                   0xa8 //(*)
#define CAN_S_Seg_1                         (N08_BITS_MSK<< 0)//Bit Timing Segment 1(slow speed)
#define CAN_S_Seg_1_SHIFT                    0
#define CAN_S_Seg_2_REG                   0xa9 //(*)
#define CAN_S_Seg_2                         (N07_BITS_MSK<< 8)//Bit Timing Segment 2(slow speed)
#define CAN_S_Seg_2_SHIFT                    8
#define CAN_S_SJW_REG                     0xaa //(*)
#define CAN_S_SJW                           (N07_BITS_MSK<<16)//Synchronization Jump Width(slow speed)
#define CAN_S_SJW_SHIFT                      16
#define CAN_S_PRESC_REG                   0xab //(*)
#define CAN_S_PRESC                         (N08_BITS_MSK<<24)//Prescaler  slow speed
#define CAN_S_PRESC_SHIFT                    24
//
#define CAN_F_Seg_1_REG                   0xac //(*)
#define CAN_F_Seg_1                         (N05_BITS_MSK<< 0)//Bit Timing Segment 1 (fast speed)
#define CAN_F_Seg_1_SHIFT                    0
#define CAN_F_Seg_2_REG                   0xad //(*)
#define CAN_F_Seg_2                         (N04_BITS_MSK<< 8)//Bit Timing Segment 2 (fast speed)
#define CAN_F_Seg_2_SHIFT                    8
#define CAN_F_SJW_REG                     0xae //(*)
#define CAN_F_SJW                           (N04_BITS_MSK<<16)//Synchronization Jump Width (fast speed)
#define CAN_F_SJW_SHIFT                      16
#define CAN_F_PRESC_REG                   0xaf //(*)
#define CAN_F_PRESC                         (N08_BITS_MSK<<24)//Prescaler fast speed
#define CAN_F_PRESC_SHIFT                    24
//
#define CAN_EALCAP_REG                    0xb0
#define CAN_ALC                             (N05_BITS_MSK<< 0)//Arbitration Lost Capture
#define CAN_ALC_SHIFT                        0
#define CAN_KOER                            (N03_BITS_MSK<< 5)//Kind Of ERror
#define CAN_KOER_SHIFT                       5
#define CAN_TDC_REG                       0xb1 //(*)
#define CAN_SSPOFF                          (N07_BITS_MSK<< 8)//Secondary Sample Point OFFset
#define CAN_SSPOFF_SHIFT                     8
#define CAN_TDCEN                           (N01_BITS_MSK<<15)//Transmitter Delay Compensation ENable
#define CAN_TDCEN_SHIFT                      15
#define CAN_RECNT_REG                     0xb2
#define CAN_RECNT                           (N08_BITS_MSK<<16)//Receive Error CouNT (number of errors during reception)
#define CAN_RECNT_SHIFT                     16
#define CAN_TECNT_REG                     0xb3
#define CAN_TECNT                           (N08_BITS_MSK<<24)//Transmit Error CouNT (number of errors during transmission)
#define CAN_TECNT_SHIFT                     24
//
#define CAN_ACFCTRL_REG                   0xb4
#define CAN_ACFADR                          (N04_BITS_MSK<< 0)//acceptance filter address
#define CAN_SELMASK                         (N01_BITS_MSK<< 5)//acceptance filter address
#define CAN_TIMECFG_REG                   0xb5
#define CAN_TIMEEN                          (N01_BITS_MSK<< 8)//TIME-stamping ENable
#define CAN_TIMEEN_SHIFT                     8
#define CAN_TIMEPOS                         (N01_BITS_MSK<< 9)//TIME-stamping POSition
#define CAN_TIMEPOS_SHIFT                    9
#define CAN_ACF_EN_0_REG                  0xb6
#define CAN_AE0_x                           (N08_BITS_MSK<<16)//Acceptance filter Enable AE7 - AE0
#define CAN_AE0_x_SHIFT                      16
#define CAN_ACF_EN_1_REG                  0xb7
#define CAN_AE1_x                           (N08_BITS_MSK<<24)//Acceptance filter Enable AE15 -AE8
#define CAN_AE1_x_SHIFT                      24
//
#define CAN_ACF_0_REG                     0xb8 //(*)
#define CAN_ACODE_MASK_0                    (N08_BITS_MSK<< 0)//Acceptance CODE or MASK
#define CAN_ACF_1_REG                     0xb9 //(*)
#define CAN_ACODE_MASK_1                    (N08_BITS_MSK<< 8)//Acceptance CODE or MASK
#define CAN_ACF_2_REG                     0xba //(*)
#define CAN_ACODE_MASK_2                    (N08_BITS_MSK<<16)//Acceptance CODE or MASK
#define CAN_ACF_3_REG                     0xbb //(*)
#define CAN_ACODE_MASK_3                    (N08_BITS_MSK<<24)//Acceptance CODE or MASK
//
#define CAN_VER_0_REG                     0xbc
#define CAN_VER_0                           (N08_BITS_MSK<< 0)//Version of CAN-CTRL,VER_0 the minor version
#define CAN_VER_1_REG                     0xbd
#define CAN_VER_1                           (N08_BITS_MSK<< 8)//Version of CAN-CTRL,VER_1 holds the major version
#define CAN_TBSLOT_REG                    0xbe
#define CAN_TBPTR                           (N06_BITS_MSK<<16)//Pointer to a TB message slot
#define CAN_TBPTR_SHIFT                      16
#define CAN_TBF                             (N01_BITS_MSK<<22)//set TB slot to "Filled"
#define CAN_TBF_SHIFT                        22
#define CAN_TBE                             (N01_BITS_MSK<<23)//set TB slot to "Empty"
#define CAN_TBE_SHIFT                        23
#define CAN_TTCFG_REG                     0xbf
#define CAN_TTEN                            (N01_BITS_MSK<<24)//Time Trigger Enable
#define CAN_TTEN_SHIFT                       24
#define CAN_T_PRESC                         (N02_BITS_MSK<<25)//TTCAN Timer PRESCaler
#define CAN_T_PRESC_SHIFT                    25
#define CAN_TTIF                            (N01_BITS_MSK<<27)//Time Trigger Interrupt Flag
#define CAN_TTIE                            (N01_BITS_MSK<<28)//Time Trigger Interrupt Enable
#define CAN_TEIF                            (N01_BITS_MSK<<29)//Trigger Error Interrupt Flag
#define CAN_WTIF                            (N01_BITS_MSK<<30)//Watch Trigger Interrupt Flag
#define CAN_WTIE                            (N01_BITS_MSK<<31)//Watch Trigger Interrupt Enable
//
#define CAN_REF_MSG_0_REG                 0xc0
#define CAN_REF_ID                          (0xFFFFFFF   << 0)//REFerence message IDentifier
#define CAN_REF_IDE                         (N01_BITS_MSK<<31)//REFerence message IDentifier  1:REF_ID(28:0) 0:REF_ID(10:0)
//
#define CAN_TRIG_CFG_0_REG                0xc4
#define CAN_TTPTR                           (N06_BITS_MSK<< 0)//Transmit Trigger TB slot Pointer
#define CAN_TRIG_CFG_1_REG                0xc5
#define CAN_TTYPE                           (N03_BITS_MSK<< 8)//Trigger Type
#define CAN_TTYPE_SHIFT                      8
#define CAN_TEW                             (N04_BITS_MSK<<12)//Transmit Enable Window
#define CAN_TEW_SHIFT                        12
#define CAN_TT_TRIG_0_REG                 0xc6
#define CAN_TT_TRIG0                        (N08_BITS_MSK<<16)//Trigger Time
#define CAN_TT_TRIG0_SHIFT                   16
#define CAN_TT_TRIG_1_REG                 0xc7
#define CAN_TT_TRIG1                        (N08_BITS_MSK<<24)//Trigger Time
#define CAN_TT_TRIG1_SHIFT                   24
//
#define CAN_TT_WTRIG_0_REG                0xc8
#define CAN_TT_WTRIG0                       (N08_BITS_MSK<< 0)//Watch Trigger Time
#define CAN_TT_WTRIG0_SHIFT                  0
#define CAN_TT_WTRIG_1_REG                0xc9
#define CAN_TT_WTRIG1                       (N08_BITS_MSK<< 8)//Watch Trigger Time
#define CAN_TT_WTRIG1_SHIFT                  8

#define CAN_TIMESTAMP_CLK_REG             0xf0

//=============================================================================
//Status REGister Define
//=============================================================================

//=============================================================================
//Struct Definition
//=============================================================================

//=============================================================================
//Private Function Definition
//=============================================================================

//=============================================================================
//Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif