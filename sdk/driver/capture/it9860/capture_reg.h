#ifndef __CAP_REG_H__
#define __CAP_REG_H__

#include "ith/ith_defs.h"
#include "ite/mmp_types.h"
#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
#define REG_CAP0_BASE                                   (ITH_CAP_BASE)          /* CAP0 Base Register Address */
//#define REG_CAP1_BASE                                   (ITH_CAP_BASE | 0x1000) /* CAP1 Base Register Address */
#define DEV_ID_MAX                                      1

//BIT0 - BIT31
#define BIT0                                            0x00000001
#define BIT1                                            0x00000002
#define BIT2                                            0x00000004
#define BIT3                                            0x00000008
#define BIT4                                            0x00000010
#define BIT5                                            0x00000020
#define BIT6                                            0x00000040
#define BIT7                                            0x00000080
#define BIT8                                            0x00000100
#define BIT9                                            0x00000200
#define BIT10                                           0x00000400
#define BIT11                                           0x00000800
#define BIT12                                           0x00001000
#define BIT13                                           0x00002000
#define BIT14                                           0x00004000
#define BIT15                                           0x00008000
#define BIT16                                           0x00010000
#define BIT17                                           0x00020000
#define BIT18                                           0x00040000
#define BIT19                                           0x00080000
#define BIT20                                           0x00100000
#define BIT21                                           0x00200000
#define BIT22                                           0x00400000
#define BIT23                                           0x00800000
#define BIT24                                           0x01000000
#define BIT25                                           0x02000000
#define BIT26                                           0x04000000
#define BIT27                                           0x08000000
#define BIT28                                           0x10000000
#define BIT29                                           0x20000000
#define BIT30                                           0x40000000
#define BIT31                                           0x80000000

//====================================================================
/*
 *
 *    Setting Register Define
 */
//====================================================================
#define CAP_GENERAL_SETTING_REGISTER0                   0x000
#define CAP_MSK_DEEN                                    (N01_BITS_MSK << 3)
#define CAP_MSK_HSEN                                    (N01_BITS_MSK << 2)
#define CAP_MSK_INTERLEAVE                              (N01_BITS_MSK << 1)
#define CAP_MSK_EMBEDDED                                (N01_BITS_MSK << 0)

#define CAP_GENERAL_SETTING_REGISTER1                   0x004
#define CAP_MSK_HEIGHT                                  (N13_BITS_MSK << 16)
#define CAP_MSK_WIDTH                                   (N12_BITS_MSK << 0)

#define CAP_ACTIVE_REGION_SETTING_REGISTER0             0x008
#define CAP_MSK_HNUM1                                   (N13_BITS_MSK << 0)

#define CAP_ACTIVE_REGION_SETTING_REGISTER1             0x00C
#define CAP_MSK_VNUM2                                   (N12_BITS_MSK << 16)
#define CAP_MSK_VNUM1                                   (N12_BITS_MSK << 0)

#define CAP_ACTIVE_REGION_SETTING_REGISTER2             0x010
#define CAP_MSK_VNUM4                                   (N12_BITS_MSK << 16)
#define CAP_MSK_VNUM3                                   (N12_BITS_MSK << 0)

#define CAP_GENERAL_SETTING_REGISTER2                   0x014
#define CAP_MSK_INPUTWIDTH                              (N02_BITS_MSK << 6)
#define CAP_MSK_COLOR_DEPTH                             (N02_BITS_MSK << 4)
#define CAP_MSK_COLOR_ORDER                             (N02_BITS_MSK << 2)
#define CAP_MSK_COLORFORMAT                             (N02_BITS_MSK << 0)

#define CAP_GENERAL_SETTING_REGISTER3                   0x018
#define CAP_CLOCK_SETTING_REGISTER                      0x01C
#define CAP_MSK_UCLK_AUTO_DLYEN                         (N01_BITS_MSK << 31)
#define CAP_MSK_UCLK_PINNUM                             (N06_BITS_MSK << 16)
#define CAP_MSK_UCLK_INV                                (N01_BITS_MSK << 12)
#define CAP_MSK_UCLK_DLY                                (N04_BITS_MSK << 8)
#define CAP_MSK_UCLKEN                                  (N01_BITS_MSK << 3)
#define CAP_MSK_UCLKSRC                                 (N03_BITS_MSK << 0)

#define CAP_GENERAL_SETTING_REGISTER4                   0x020
#define CAP_GENERAL_SETTING_REGISTER5                   0x024
#define CAP_GENERAL_SETTING_REGISTER6                   0x028
#define CAP_MSK_DEPINNUM                                (N06_BITS_MSK << 16)
#define CAP_MSK_VSPINNUM                                (N06_BITS_MSK << 8)
#define CAP_MSK_HSPINNUM                                (N06_BITS_MSK << 0)

#define CAP_Y_DATA_PIN_MUX0                             0x02C
#define CAP_MSK_Y03_PINNUM                              (N06_BITS_MSK << 24)
#define CAP_MSK_Y02_PINNUM                              (N06_BITS_MSK << 16)
#define CAP_MSK_Y01_PINNUM                              (N06_BITS_MSK << 8)
#define CAP_MSK_Y00_PINNUM                              (N06_BITS_MSK << 0)

#define CAP_Y_DATA_PIN_MUX1                             0x030
#define CAP_MSK_Y07_PINNUM                              (N06_BITS_MSK << 24)
#define CAP_MSK_Y06_PINNUM                              (N06_BITS_MSK << 16)
#define CAP_MSK_Y05_PINNUM                              (N06_BITS_MSK << 8)
#define CAP_MSK_Y04_PINNUM                              (N06_BITS_MSK << 0)

#define CAP_Y_DATA_PIN_MUX2                             0x034
#define CAP_MSK_Y11_PINNUM                              (N06_BITS_MSK << 24)
#define CAP_MSK_Y10_PINNUM                              (N06_BITS_MSK << 16)
#define CAP_MSK_Y09_PINNUM                              (N06_BITS_MSK << 8)
#define CAP_MSK_Y08_PINNUM                              (N06_BITS_MSK << 0)

#define CAP_U_DATA_PIN_MUX0                             0x038
#define CAP_MSK_U03_PINNUM                              (N06_BITS_MSK << 24)
#define CAP_MSK_U02_PINNUM                              (N06_BITS_MSK << 16)
#define CAP_MSK_U01_PINNUM                              (N06_BITS_MSK << 8)
#define CAP_MSK_U00_PINNUM                              (N06_BITS_MSK << 0)

#define CAP_U_DATA_PIN_MUX1                             0x03C
#define CAP_MSK_U07_PINNUM                              (N06_BITS_MSK << 24)
#define CAP_MSK_U06_PINNUM                              (N06_BITS_MSK << 16)
#define CAP_MSK_U05_PINNUM                              (N06_BITS_MSK << 8)
#define CAP_MSK_U04_PINNUM                              (N06_BITS_MSK << 0)

#define CAP_U_DATA_PIN_MUX2                             0x040
#define CAP_MSK_U011_PINNUM                             (N06_BITS_MSK << 24)
#define CAP_MSK_U010_PINNUM                             (N06_BITS_MSK << 16)
#define CAP_MSK_U09_PINNUM                              (N06_BITS_MSK << 8)
#define CAP_MSK_U08_PINNUM                              (N06_BITS_MSK << 0)

#define CAP_V_DATA_PIN_MUX0                             0x044
#define CAP_MSK_V03_PINNUM                              (N06_BITS_MSK << 24)
#define CAP_MSK_V02_PINNUM                              (N06_BITS_MSK << 16)
#define CAP_MSK_V01_PINNUM                              (N06_BITS_MSK << 8)
#define CAP_MSK_V00_PINNUM                              (N06_BITS_MSK << 0)

#define CAP_V_DATA_PIN_MUX1                             0x048
#define CAP_MSK_V07_PINNUM                              (N06_BITS_MSK << 24)
#define CAP_MSK_V06_PINNUM                              (N06_BITS_MSK << 16)
#define CAP_MSK_V05_PINNUM                              (N06_BITS_MSK << 8)
#define CAP_MSK_V04_PINNUM                              (N06_BITS_MSK << 0)

#define CAP_V_DATA_PIN_MUX2                             0x04C
#define CAP_MSK_V011_PINNUM                             (N06_BITS_MSK << 24)
#define CAP_MSK_V010_PINNUM                             (N06_BITS_MSK << 16)
#define CAP_MSK_V09_PINNUM                              (N06_BITS_MSK << 8)
#define CAP_MSK_V08_PINNUM                              (N06_BITS_MSK << 0)

#define CAP_CAPTURE_INTERNAL_SETTING                    0x050
#define CAP_MSK_DUMPMODE                                (N01_BITS_MSK << 31)
#define CAP_MSK_CHECK_DE                                (N01_BITS_MSK << 30)
#define CAP_MSK_CHECK_VS                                (N01_BITS_MSK << 29)
#define CAP_MSK_CHECK_HS                                (N01_BITS_MSK << 28)
#define CAP_MSK_SAMPLE_VS                               (N01_BITS_MSK << 27)
#define CAP_MSK_HSYNC_SKIP                              (N06_BITS_MSK << 12)
#define CAP_MSK_VSYNC_SKIP                              (N03_BITS_MSK << 8)
#define CAP_MSK_VSPOL_DET                               (N01_BITS_MSK << 7)
#define CAP_MSK_HSPOL_DET                               (N01_BITS_MSK << 6)
#define CAP_MSK_VHS                                     (N01_BITS_MSK << 1)
#define CAP_MSK_PHS                                     (N01_BITS_MSK << 0)

#define CAP_CAPTURE_OUTPUT_SETTING                      0x054
#define CAP_FRAMERATE_SETTING_REGISTER                  0x058
#define CAP_ERROR_DETECT_SETTING_REGISTER               0x05C
#define CAP_GENERAL_SETTING_REGISTER7                   0x060
#define CAP_DITHERING_SETTING                           0x064
#define CAP_ROI_SETTING_REGISTER0                       0x068
#define CAP_ROI_SETTING_REGISTER1                       0x06C
#define CAP_COLOR_SPACE_CONVERSION_SETTING_REGISTER0    0x070
#define CAP_COLOR_SPACE_CONVERSION_SETTING_REGISTER1    0x074
#define CAP_COLOR_SPACE_CONVERSION_SETTING_REGISTER2    0x078
#define CAP_COLOR_SPACE_CONVERSION_SETTING_REGISTER3    0x07C
#define CAP_COLOR_SPACE_CONVERSION_SETTING_REGISTER4    0x080
#define CAP_COLOR_SPACE_CONVERSION_SETTING_REGISTER5    0x084
#define CAP_COLOR_SPACE_CONVERSION_SETTING_REGISTER6    0x088
#define CAP_SCALING_SETTING_REGISTER0                   0x08C
#define CAP_SCALING_SETTING_REGISTER1                   0x090
#define CAP_SCALING_SETTING_REGISTER2                   0x094
#define CAP_SCALING_SETTING_REGISTER3                   0x098
#define CAP_SCALING_SETTING_REGISTER4                   0x09C
#define CAP_SCALING_SETTING_REGISTER5                   0x0A0
#define CAP_SCALING_SETTING_REGISTER6                   0x0A4
#define CAP_GENERAL_SETTING_REGISTER8                   0x0A8
#define CAP_GENERAL_SETTING_REGISTER9                   0x0AC
#define CAP_GENERAL_SETTING_REGISTER10                  0x0B0
#define CAP_GENERAL_SETTING_REGISTER11                  0x0B4
#define CAP_GENERAL_SETTING_REGISTER12                  0x0B8
#define CAP_GENERAL_SETTING_REGISTER13                  0x0BC
#define CAP_GENERAL_SETTING_REGISTER14                  0x0C0
#define CAP_GENERAL_SETTING_REGISTER15                  0x0C4
#define CAP_GENERAL_SETTING_REGISTER16                  0x0C8
#define CAP_GENERAL_SETTING_REGISTER17                  0x0CC
#define CAP_GENERAL_SETTING_REGISTER18                  0x0D0
#define CAP_GENERAL_SETTING_REGISTER19                  0x0D4
#define CAP_GENERAL_SETTING_REGISTER20                  0x0D8
#define CAP_GENERAL_SETTING_REGISTER21                  0x0DC
#define CAP_INTERRUPT_SETTING_REGISTER                  0x0E0
#define CAP_GENERAL_SETTING_REGISTER22                  0x0E4
#define CAP_GENERAL_SETTING_REGISTER23                  0x0E8
#define CAP_GENERAL_SETTING_REGISTER24                  0x0EC
#define CAP_GENERAL_SETTING_REGISTER25                  0x0F0
#define CAP_GENERAL_SETTING_REGISTER26                  0x0F4
#define CAP_GENERAL_SETTING_REGISTER27                  0x0F8
#define CAP_GENERAL_SETTING_REGISTER28                  0x0FC
#define CAP_GENERAL_SETTING_REGISTER29                  0x100
#define CAP_GENERAL_SETTING_REGISTER30                  0x104
#define CAP_GENERAL_SETTING_REGISTER31                  0x108
#define CAP_GENERAL_SETTING_REGISTER32                  0x10C
#define CAP_GENERAL_SETTING_REGISTER33                  0x110
#define CAP_GENERAL_SETTING_REGISTER34                  0x114
#define CAP_GENERAL_SETTING_REGISTER35                  0x118
#define CAP_GENERAL_SETTING_REGISTER36                  0x11C
#define CAP_GENERAL_SETTING_REGISTER37                  0x120
#define CAP_GENERAL_SETTING_REGISTER38                  0x124
#define CAP_GENERAL_SETTING_REGISTER39                  0x128
#define CAP_AV_SYNC_SETTING_REGISTER0                   0x12C
#define CAP_AV_SYNC_SETTING_REGISTER1                   0x130
#define CAP_GENERAL_SETTING_REGISTER40                  0x134
//====================================================================
/*
 *
 *    Status Register Define
 */
//====================================================================
#define CAP_ENGINE_STATUS_REGISTER0                     0x200
#define CAP_ENGINE_STATUS_REGISTER1                     0x204
#define CAP_ENGINE_STATUS_REGISTER2                     0x208
#define CAP_ENGINE_STATUS_REGISTER3                     0x20C
#define CAP_AV_SYNC_STATUS_REGISTER0                    0x210
#define CAP_AV_SYNC_STATUS_REGISTER1                    0x214
#define CAP_AV_SYNC_STATUS_REGISTER2                    0x218
#define CAP_AV_SYNC_STATUS_REGISTER3                    0x21C
#define CAP_SRAM_BIST_STATUS_REGISTER                   0x220
#define CAP_DEBUG_STATUS_REGISTER                       0x224
#define CAP_FRAME_CRC_STATUS_REGISTER0                  0x228
#define CAP_FRAME_CRC_STATUS_REGISTER1                  0x22C
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================
static inline MMP_UINT32 _GetAddress(MMP_UINT32 id, MMP_UINT32 offset)
{
    if (id >= 0 && id < DEV_ID_MAX)
        return ((REG_CAP0_BASE | offset) | id << 12 );
    else
        ithPrintf("[Error]device id error.\n");

    return REG_CAP0_BASE;
}

//=============================================================================
//                Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif