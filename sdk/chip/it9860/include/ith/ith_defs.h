/*
 * Copyright (c) 2011 ITE Tech.Inc.All Rights Reserved.
 */
/** @file
 * Castor hardware abstraction layer definitions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef ITH_DEFS_H
#define ITH_DEFS_H

#include "ith/ith_platform.h"

// Base address
#define ITH_MMIO_BASE                   0xA0000000

#define ITH_SRAM_AUDIO_BASE         	0xA0000000
#define ITH_SRAM_AUDIO_SIZE         	8192

#define ITH_AHB0_BASE                   0xB0000000
#define ITH_RISC_BASE                   0xB0200000
#define ITH_USB0_BASE                   0xB0400000
#define ITH_USB1_BASE                   0xB0500000
#define ITH_CMDQ_BASE                   0xB0600000
#define ITH_OPENVG_BASE                 0xB0700000 //GFX
#define ITH_NAND_BASE                   0xB0800000
#define ITH_SD0_BASE                    0xB0A00000
#define ITH_SD1_BASE                    0xB0B00000
#define ITH_AXISPI_BASE                 0xC0000000
#define ITH_LCD_BASE                    0xD0000000
#define ITH_CAP_BASE                    0xD0700000
#define ITH_SARADC_BASE                 0xD0800000
#define ITH_DPU_BASE                    0xD0900000
#define ITH_DMA_BASE                    0xD0A00000
#define ITH_UIENC_BASE                  0xD0B00000
#define ITH_MIPI_BASE                   0xD0C00000
#define ITH_MIPI_DPHY_BASE              0xD0D00000
#define ITH_GMAC_BASE                   0xD0E00000
#define ITH_GPIO_BASE                   0xD1000000
#define ITH_IIC_BASE                    0xD1100000
#define ITH_SSP0_BASE                   0xD1700000
#define ITH_SSP1_BASE                   0xD1800000

#ifdef __SM32__
    #define ITH_INTR_BASE               0xD1200040
#else
    #define ITH_INTR_BASE               0xD1200000
#endif

#define ITH_HOST_BASE                   0xD8000000

#define ITH_RTC_BASE                    0xD1500000
#define ITH_UART0_BASE                  0xD1600000
#define ITH_UART1_BASE                  0xD1600100
#define ITH_UART2_BASE                  0xD1600200
#define ITH_UART3_BASE                  0xD1600300
#define ITH_UART4_BASE                  0xD1600400
#define ITH_UART5_BASE                  0xD1600500
#define ITH_TIMER_BASE                  0xD1900000
#define ITH_WD_BASE                     0xD1A00000
#define ITH_IR0_BASE                    0xD1B00000
#define ITH_IR1_BASE					0xD1C00000
#define ITH_IR2_BASE					0xD1D00000
#define ITH_IR3_BASE					0xD1E00000
#define ITH_TIMER2_BASE					0xD1F00000
#define ITH_MEM_BASE                    0xD8100000

// Chip ID
#define ITH_HTRAP_REG                   0x0000
#define ITH_HTRAP_BIT                   16
#define ITH_HTRAP_MASK                  (N08_BITS_MSK << ITH_HTRAP_BIT)

#define ITH_REVISION_ID_REG             0x0004

// ARM Cache Type
#define ITH_ARM_CACHE_TYPE              0x0F192192
#define ITH_ARMLITE_CACHE_TYPE          0x0F152152

// CLOCK
#define ITH_PLL_CFG_REG                 0x0000
#define ITH_PLL_CFG_BIT                 7
#define ITH_PLL_CFG_MASK                (N02_BITS_MSK << ITH_PLL_CFG_BIT)

#define ITH_HOST_CLK1_REG               0x0010
#define ITH_BCLK_UPD_BIT                14
#define ITH_BCLK_UPD_MASK               (N01_BITS_MSK << ITH_BCLK_UPD_BIT)
#define ITH_BCLK_SRC_SEL_BIT            11
#define ITH_BCLK_SRC_SEL_MASK           (N03_BITS_MSK << ITH_BCLK_SRC_SEL_BIT)
#define ITH_BCLK_RATIO_BIT              0
#define ITH_BCLK_RATIO_MASK             (N10_BITS_MSK << ITH_BCLK_RATIO_BIT)

// AXI CLOCK
#define ITH_AXI_CLK_REG                	(ITH_HOST_BASE + 0x000C)
#define ITH_AXICLK_A1CLK_BIT			21
#define ITH_AXICLK_A0CLK_BIT			19
#define ITH_EN_AXICLK_BIT				17
#define ITH_EN_DIV_AXICLK_BIT			15
#define ITH_AXICLK_SRC_SEL_BIT          12
#define ITH_AXICLK_SRC_SEL_MASK         (N03_BITS_MSK << ITH_AXICLK_SRC_SEL_BIT)
#define ITH_AXICLK_UPD_BIT              11
#define ITH_AXICLK_UPD_MASK             (N01_BITS_MSK << ITH_AXICLK_UPD_BIT)
#define ITH_AXICLK_RATIO_BIT            0
#define ITH_AXICLK_RATIO_MASK           (N10_BITS_MSK << ITH_AXICLK_RATIO_BIT)

// MEM PHY_CTRL CLOCK
#define ITH_MEM_CLK1_REG                (ITH_HOST_BASE + 0x0010)
#define ITH_EN_MCLK_BIT					17
#define ITH_EN_MCLK_MASK                (N01_BITS_MSK << ITH_EN_MCLK_BIT)
#define ITH_EN_DIV_MCLK_BIT             15
#define ITH_EN_DIV_MCLK_MASK            (N01_BITS_MSK << ITH_EN_DIV_MCLK_BIT)
#define ITH_MCLK_SRC_SEL_BIT            12
#define ITH_MCLK_SRC_SEL_MASK           (N03_BITS_MSK << ITH_MCLK_SRC_SEL_BIT)
#define ITH_MCLK_UPD_BIT                11
#define ITH_MCLK_UPD_MASK 	            (N01_BITS_MSK << ITH_MCLK_UPD_BIT)
#define ITH_MCLK_RATIO_BIT              0
#define ITH_MCLK_RATIO_MASK             (N10_BITS_MSK << ITH_MCLK_RATIO_BIT)

// MEM WRAPPER CLOCK
#define ITH_MEM_CLK2_REG                (ITH_HOST_BASE + 0x0014)
#define ITH_MCLK2_W11CLK_BIT			21
#define ITH_MCLK2_M15CLK_BIT			19
#define ITH_EN_MCLK2_BIT				17
#define ITH_EN_MCLK2_MASK               (N01_BITS_MSK << ITH_EN_MCLK2_BIT)
#define ITH_EN_DIV_MCLK2_BIT            15
#define ITH_EN_DIV_MCLK2_MASK           (N01_BITS_MSK << ITH_EN_DIV_MCLK2_BIT)
#define ITH_MCLK2_SRC_SEL_BIT           12
#define ITH_MCLK2_SRC_SEL_MASK          (N03_BITS_MSK << ITH_MCLK2_SRC_SEL_BIT)
#define ITH_MCLK2_UPD_BIT               11
#define ITH_MCLK2_UPD_MASK 	            (N01_BITS_MSK << ITH_MCLK2_UPD_BIT)
#define ITH_MCLK2_RATIO_BIT             0
#define ITH_MCLK2_RATIO_MASK            (N10_BITS_MSK << ITH_MCLK2_RATIO_BIT)

// AHB CLOCK
#define ITH_AHB_CLK_REG                 (ITH_HOST_BASE + 0x0018)
#define ITH_NCLK_SRC_SEL_BIT            12
#define ITH_NCLK_SRC_SEL_MASK           (N03_BITS_MSK << ITH_NCLK_SRC_SEL_BIT)
#define ITH_NCLK_UPD_BIT                11
#define ITH_NCLK_UPD_MASK               (N01_BITS_MSK << ITH_NCLK_UPD_BIT)
#define ITH_NCLK_RATIO_BIT              0
#define ITH_NCLK_RATIO_MASK             (N10_BITS_MSK << ITH_NCLK_RATIO_BIT)
#define ITH_EN_N11CLK_BIT				21
#define ITH_EN_N10CLK_BIT				19
#define ITH_EN_N0CLK_BIT				17

// APB CLOCK 1
#define ITH_APB_CLK1_REG                (ITH_HOST_BASE + 0x001C)
#define ITH_SARADC_RST_BIT              24
#define ITH_SARADC_RST_MASK             (N01_BITS_MSK << ITH_SARADC_RST_BIT)
#define ITH_WCLK_SRC_SEL_BIT            12
#define ITH_WCLK_SRC_SEL_MASK           (N03_BITS_MSK << ITH_WCLK_SRC_SEL_BIT)
#define ITH_WCLK_UPD_BIT                11
#define ITH_WCLK_UPD_MASK               (N01_BITS_MSK << ITH_WCLK_UPD_BIT)
#define ITH_WCLK_RATIO_BIT              0
#define ITH_WCLK_RATIO_MASK             (N10_BITS_MSK << ITH_WCLK_RATIO_BIT)

// APB CLOCK 2
#define ITH_APB_CLK2_REG                (ITH_HOST_BASE + 0x0020)
#define ITH_IR3_RESET_BIT				31
#define ITH_IR2_RESET_BIT				30
#define ITH_IR1_RESET_BIT				29
#define ITH_IR0_RESET_BIT				28
#define ITH_EN_W25CLK_BIT				27
#define ITH_EN_W17CLK_BIT               25
#define ITH_EN_W16CLK_BIT               23
#define ITH_EN_W10CLK_BIT				21
#define ITH_EN_W9CLK_BIT				19
#define ITH_EN_W8CLK_BIT				17
#define ITH_EN_W7CLK_BIT				15
#define ITH_EN_W6CLK_BIT				13
#define ITH_EN_W5CLK_BIT                11
#define ITH_EN_W4CLK_BIT                9
#define ITH_EN_W3CLK_BIT                7
#define ITH_EN_W2CLK_BIT                5
#define ITH_EN_W1CLK_BIT                3
#define ITH_EN_W0CLK_BIT                1

// CQ 2D CLOCK
#define ITH_CQ_CLK_REG                  0x0024
#define ITH_CQ_RST_BIT                  31
#define ITH_EN_N2CLK_BIT                25
#define ITH_EN_M2CLK_BIT                21 //9850 ITH_EN_M3CLK_BIT

// LCD CLOCK
#define ITH_LCD_CLK1_REG                0x0028

#define ITH_LCD_REG_RST_BIT             31
#define ITH_LCD_RST_BIT                 30
#define ITH_EN_W12CLK_BIT               21
#define ITH_EN_M3CLK_BIT                19
#define ITH_EN_DCLK_BIT                 17

// DPU CLOCK
#define ITH_DPU_CLK_REG                 (ITH_HOST_BASE + 0x002C)
#define ITH_EN_W19CLK_BIT               23
#define ITH_EN_M14CLK_BIT               21
#define ITH_EN_DPCLK_BIT                19
#define ITH_EN_DIV_DPCLK_BIT			15
#define ITH_DPCLK_SRC_SEL_BIT           12
#define ITH_DPCLK_UPD_BIT               11
#define ITH_DPCLK_UPD_MASK              (N01_BITS_MSK << ITH_DPCLK_UPD_BIT)
#define ITH_DPCLK_RATIO_BIT             0
#define ITH_DPCLK_RATIO_MASK            (N10_BITS_MSK << ITH_DPCLK_RATIO_BIT)

// TVE
#define ITH_TVE_CLK2_REG                0x002E
#define ITH_TVE_YRST_BIT                13
#define ITH_TVE_ERST_BIT                12
#define ITH_EN_YCLK_BIT                 3
#define ITH_EN_ECLK_BIT                 1

// ISP CLOCK
#define ITH_ISP_CLK_REG                 0x0030
#define ITH_EN_DIV_ICLK_BIT             15
#define ITH_ICLK_SRC_SEL_BIT            12
#define ITH_ICLK_SRC_SEL_MASK           (N03_BITS_MSK << ITH_ICLK_SRC_SEL_BIT)
#define ITH_ISPQ_RST_BIT                31
#define ITH_ISP0_REG_RST_BIT            30
#define ITH_ISP0_RST_BIT                29
#define ITH_ISP_ALLREG_RST_BIT          28
#define ITH_ISP1_REG_RST_BIT            27
#define ITH_ISP1_RST_BIT                26
#define ITH_EN_W13CLK_BIT               23
#define ITH_EN_M4CLK_BIT                21
#define ITH_EN_I1CLK_BIT                19
#define ITH_EN_I0CLK_BIT                17

// VIDEO CLOCK
#define ITH_EN_DIV_XCLK_BIT             15              //should remove it ,Benson
#define ITH_VIDEO_CLK2_REG              0x0036          //should remove it ,Benson
#define ITH_EN_M7CLK_BIT                11              //should remove it ,Benson

// VIDEO CLOCK
#define ITH_VIDEO_CLK1_REG              0x0034
#define ITH_VIDEO_REG_RST_BIT           31
#define ITH_VIDEO_RST_BIT               30
#define ITH_EN_W23CLK_BIT               27
#define ITH_EN_A3CLK_BIT                25
#define ITH_EN_N3CLK_BIT                23
#define ITH_EN_M5CLK_BIT                21
#define ITH_EN_XCLK_BIT                 17
#define ITH_EN_DIV_XCLK_BIT             15

// JPEG CLOCK
#define ITH_JPEG_CLK_REG                0x0038
#define ITH_JPEG_REG_RST_BIT            31
#define ITH_JPEG_RST_BIT                30
#define ITH_EN_W14CLK_BIT               23
#define ITH_EN_M6CLK_BIT                19
#define ITH_EN_JCLK_BIT                 17
#define ITH_EN_DIV_JCLK_BIT             15

// USB CLOCK
#define ITH_USB_CLK_REG                 0x64

// UIENC CLOCK
#define ITH_UIENC_CLK_REG               0x0070
#define ITH_EN_UIENC_RST_BIT            31
#define ITH_EN_M16CLK_BIT               1

//// LCD CLOCK REG4
//#define ITH_LCD_CLK_REG4                0x0054
//#define ITH_EN_DHCLK_DIV_BIT            15

// CAP CLOCK
#define ITH_CAP_CLK_REG                 0x0060
#define ITH_EN_CAPC_RST_BIT             31
#define ITH_EN_CAP_REG_RST_BIT          30
#define ITH_EN_W18CLK_BIT               19
#define ITH_EN_DIV_CAPCLK_BIT           15
#define ITH_EN_M12CLK_BIT               17

// RISC CLOCK
#define ITH_RISC_CLK_REG				(ITH_HOST_BASE + 0x0050)
#define ITH_EN_N8CLK_BIT				9
#define ITH_EN_N4CLK_BIT				1

// SENSOR CLOCK
#define ITH_SENSOR_CLK_REG              0x0064
#define ITH_SENSOR_CLK_DIV_EN_BIT       11
#define ITH_SENSOR_CLK_DIV_EN_MASK      (N01_BITS_MSK << ITH_SENSOR_CLK_DIV_EN_BIT)
#define ITH_SENSOR_CLK_SEL_BIT          8
#define ITH_SENSOR_CLK_SEL_MASK         (N03_BITS_MSK << ITH_SENSOR_CLK_SEL_BIT)
#define ITH_SENSOR_CLK_RATIO_BIT        0
#define ITH_SENSOR_CLK_RATIO_MASK       (N04_BITS_MSK << ITH_SENSOR_CLK_RATIO_BIT)

//// OPENVG CLOCK
//#define ITH_OPVG_CLK1_REG               0x0056
//#define ITH_OPVG_RST_BIT                15
//#define ITH_EN_N9CLK_BIT                7
//#define ITH_EN_M16CLK_BIT               5
//#define ITH_EN_DG_M16CLK_BIT            4
//#define ITH_EN_VCLK_BIT                 3
//#define ITH_EN_DG_VCLK_BIT              2
//#define ITH_EN_TCLK_BIT                 1
//#define ITH_EN_DG_TCLK_BIT              0
//
//#define ITH_OPVG_CLK2_REG               0x0058
//#define ITH_EN_DIV_TCLK_BIT             15
//
//#define ITH_OPVG_CLK3_REG               0x005A
//#define ITH_EN_DIV_VCLK_BIT             15

// DFT
#define ITH_GEN_DFT1_REG                0x0072

// PLL
#define ITH_PLL1_1_SET1_REG				(ITH_HOST_BASE + 0x0100)
#define ITH_PLL1_1_SET2_REG				(ITH_HOST_BASE + 0x0104)

#define ITH_PLL2_1_SET1_REG				(ITH_HOST_BASE + 0x0110)
#define ITH_PLL2_1_SET2_REG				(ITH_HOST_BASE + 0x0114)
#define ITH_PLL2_2_SET1_REG				(ITH_HOST_BASE + 0x0118)
#define ITH_PLL2_2_SET2_REG				(ITH_HOST_BASE + 0x011C)

#define ITH_PLL3_1_SET1_REG				(ITH_HOST_BASE + 0x0120)
#define ITH_PLL3_1_SET2_REG				(ITH_HOST_BASE + 0x0124)

#define ITH_PLL_SET1_MODE_BIT			31
#define ITH_PLL_SET1_MUX_BIT			30
#define ITH_PLL_SET1_BW_BIT				28
#define ITH_PLL_SET1_BW_MSK				(N02_BITS_MSK << ITH_PLL_SET1_BW_BIT)
#define ITH_PLL_SET1_NUMERATOR_BIT		16
#define ITH_PLL_SET1_NUMERATOR_MSK		(N07_BITS_MSK << ITH_PLL_SET1_NUMERATOR_BIT)
#define ITH_PLL_SET1_POSTDIV_BIT		8
#define ITH_PLL_SET1_POSTDIV_MSK		(N07_BITS_MSK << ITH_PLL_SET1_POSTDIV_BIT)
#define ITH_PLL_SET1_PREDIV_BIT			0
#define ITH_PLL_SET1_PREDIV_MSK			(N05_BITS_MSK << ITH_PLL_SET1_PREDIV_BIT)

#define ITH_PLL_SET2_EN_BIT				31
#define ITH_PLL_SET2_EN_MSK				(N01_BITS_MSK << ITH_PLL_SET2_EN_BIT)
#define ITH_PLL_SET2_UPD_BIT			30
#define ITH_PLL_SET2_UPD_MSK			(N01_BITS_MSK << ITH_PLL_SET2_UPD_BIT)
#define ITH_PLL_SET2_SDM_PH_BIT			28
#define ITH_PLL_SET2_SDM_DR_BIT			20
#define ITH_PLL_SET2_SDM_DR_MSK			(N08_BITS_MSK << ITH_PLL_SET2_SDM_DR_BIT)
#define ITH_PLL_SET2_SDM_FR_BIT			0
#define ITH_PLL_SET2_SDM_FR_MSK			(N20_BITS_MSK << ITH_PLL_SET2_SDM_FR_BIT)

#define ITH_PPL_DCXO_SET_REG			(ITH_HOST_BASE + 0x0138)

// FixMe: OLD PLL regsiter address, should remove them
#define ITH_PLL1_SET1_REG               0x00A0
#define ITH_PLL1_SET2_REG               0x00A2

#define ITH_PLL1_SET3_REG               0x00A4
#define ITH_PLL1_PWR_STATE_POS          15
#define ITH_PLL1_PWR_STATE_MSK          (N01_BITS_MSK << ITH_PLL1_PWR_STATE_POS)
#define ITH_PLL1_BYPASS_BIT             14
#define ITH_PLL1_START_BIT              13
#define ITH_PLL1_UPDATE_BIT             12
#define ITH_PLL1_UPDATE_MASK            (N01_BITS_MSK << ITH_PLL1_UPDATE_BIT)

#define ITH_PLL1_SET4_REG               0x00A6

#define ITH_PLL1_SET5_REG               0x00A8
#define ITH_PLL1_PD_LEVEL_BIT           0
#define ITH_PLL1_PD_LEVEL_MASK          (N09_BITS_MSK << ITH_PLL1_PD_LEVEL_BIT)

#define ITH_PLL1_SET6_REG               0x00AA
#define ITH_PLL1_SP_EN_BIT              15
#define ITH_PLL1_SP_STATE_BIT           14
#define ITH_PLL1_SP_MODE_BIT            12
#define ITH_PLL1_SP_MODE_MASK           (N02_BITS_MSK << ITH_PLL1_SP_MODE_BIT)
#define ITH_PLL1_SP_STEPX_BIT           6
#define ITH_PLL1_SP_STEPX_MASK          (N06_BITS_MSK << ITH_PLL1_SP_STEPX_BIT)
#define ITH_PLL1_SP_STEPY_BIT           0
#define ITH_PLL1_SP_STEPY_MASK          (N06_BITS_MSK << ITH_PLL1_SP_STEPY_BIT)

#define ITH_PLL2_SET1_REG               0x00B0
#define ITH_PLL2_SET2_REG               0x00B2
#define ITH_PLL2_SET3_REG               0x00B4
#define ITH_PLL2_PWR_STATE_POS          15
#define ITH_PLL2_PWR_STATE_MSK          (N01_BITS_MSK << ITH_PLL2_PWR_STATE_POS)
#define ITH_PLL2_BYPASS_BIT             14
#define ITH_PLL2_START_BIT              13
#define ITH_PLL2_UPDATE_BIT             12
#define ITH_PLL2_UPDATE_MASK            (N01_BITS_MSK << ITH_PLL2_UPDATE_BIT)

#define ITH_PLL3_SET1_REG               0x00C0
#define ITH_PLL3_SET2_REG               0x00C2
#define ITH_PLL3_SET3_REG               0x00C4
#define ITH_PLL3_PWR_STATE_POS          15
#define ITH_PLL3_PWR_STATE_MSK          (N01_BITS_MSK << ITH_PLL3_PWR_STATE_POS)
#define ITH_PLL3_BYPASS_BIT             14
#define ITH_PLL3_START_BIT              13
#define ITH_PLL3_UPDATE_BIT             12
#define ITH_PLL3_UPDATE_MASK            (N01_BITS_MSK << ITH_PLL3_UPDATE_BIT)

#define ITH_PLL_GEN_SET_REG             0x00D2

// HOST
#define ITH_HOST_BUS_CTRL1_REG          0x0200

#define ITH_EN_MMIO_REG                 0x0202
#define ITH_EN_MMIO_BIT                 0
#define ITH_EN_MMIO_MASK                (N15_BITS_MSK << ITH_EN_MMIO_BIT)

#define ITH_EN_I2S_MMIO_BIT             0
#define ITH_EN_JPEG_MMIO_BIT            1
#define ITH_EN_MPEG_MMIO_BIT            2
#define ITH_EN_VIDEO_MMIO_BIT           3
#define ITH_EN_TVE_MMIO_BIT             4
#define ITH_EN_TSI0_MMIO_BIT            5
#define ITH_EN_TSI1_MMIO_BIT            6
#define ITH_EN_CQ_MMIO_BIT              7
#define ITH_EN_RISC0_MMIO_BIT           8
#define ITH_EN_2D_MMIO_BIT              9
#define ITH_EN_FPC_MMIO_BIT             10
#define ITH_EN_NFC_MMIO_BIT             11
#define ITH_EN_USB_MMIO_BIT             12
#define ITH_EN_ISP_MMIO_BIT             13
#define ITH_EN_LCD_MMIO_BIT             14
#define ITH_EN_CAP_MMIO_BIT             15

#define ITH_HOST_BUS_CTRL18_REG         0x0222

// AHB0
#define ITH_AHB0_CTRL_REG               0x88
#define ITH_AHB0_CTRL_REG_DEFAULT       0x00100F01

// MEM Control
#define ITH_MEM_STATE_CTL_REG           (ITH_MEM_BASE + 0x04)
#define ITH_CHK_SELF_REFRESH_BIT		10
#define ITH_EXIT_SELF_REFRESH_BIT		3
#define ITH_EXIT_SELF_REFRESH_MSK		(N01_BITS_MSK << ITH_EXIT_SELF_REFRESH_BIT)
#define ITH_EN_SELF_REFRESH_BIT         2
#define ITH_EN_SELF_REFRESH_MSK			(N01_BITS_MSK << ITH_EN_SELF_REFRESH_BIT)

#define ITH_MEM_PD_CTL_REG              (ITH_MEM_BASE + 0x2C)
#define ITH_AUTO_SELF_REFRESH_BIT       28

// MEM DEBUG
#define ITH_MEMDBG0_BASE_REG            0x0360
#define ITH_MEMDBG1_BASE_REG            0x0380

#define ITH_MEMDBG_ADDR_LO_REG          0x00
#define ITH_MEMDBG_ADDR_HI_REG          0x04

#define ITH_MEMDBG_EN_REG               0x08
#define ITH_MEMDBG_MODE_BIT             0
#define ITH_MEMDBG_MODE_MASK            (N02_BITS_MSK << ITH_MEMDBG_MODE_BIT)
#define ITH_MEMDBG_AXI0INT_S_BIT        8
#define ITH_MEMDBG_AXI0INT_S_MASK       (N01_BITS_MSK << ITH_MEMDBG_AXI0INT_S_BIT)
#define ITH_MEMDBG_AXI1INT_S_BIT        9
#define ITH_MEMDBG_AXI1INT_S_MASK       (N01_BITS_MSK << ITH_MEMDBG_AXI1INT_S_BIT)
#define ITH_MEMDBG_AXI2INT_S_BIT        10
#define ITH_MEMDBG_AXI2INT_S_MASK       (N01_BITS_MSK << ITH_MEMDBG_AXI2INT_S_BIT)
#define ITH_MEMDBG_AXI3INT_S_BIT        11
#define ITH_MEMDBG_AXI3INT_S_MASK       (N01_BITS_MSK << ITH_MEMDBG_AXI3INT_S_BIT)
#define ITH_MEMDBG_AXI0INT_EN_BIT       16
#define ITH_MEMDBG_AXI0INT_EN_MASK      (N01_BITS_MSK << ITH_MEMDBG_AXI0INT_EN_BIT)
#define ITH_MEMDBG_AXI1INT_EN_BIT       17
#define ITH_MEMDBG_AXI1INT_EN_MASK      (N01_BITS_MSK << ITH_MEMDBG_AXI1INT_EN_BIT)
#define ITH_MEMDBG_AXI2INT_EN_BIT       18
#define ITH_MEMDBG_AXI2INT_EN_MASK      (N01_BITS_MSK << ITH_MEMDBG_AXI2INT_EN_BIT)
#define ITH_MEMDBG_AXI3INT_EN_BIT       19
#define ITH_MEMDBG_AXI3INT_EN_MASK      (N01_BITS_MSK << ITH_MEMDBG_AXI3INT_EN_BIT)
#define ITH_MEMDBG_CLR_BIT              27
#define ITH_MEMDBG_CLR_MASK             (N01_BITS_MSK << ITH_MEMDBG_CLR_BIT)
#define ITH_MEMDBG_EN_BIT               31
#define ITH_MEMDBG_EN_MASK              (N01_BITS_MSK << ITH_MEMDBG_EN_BIT)

#define ITH_MEMDBG_INT_STATUS_MASK      (N04_BITS_MSK << ITH_MEMDBG_AXI0INT_S_BIT)

#define ITH_MEMDBG_PASSID0_REG          0x0C
#define ITH_MEMDBG_PASSID1_REG          0x10
#define ITH_MEMDBG_PASSID_AXI0_BIT      0
#define ITH_MEMDBG_PASSID_AXI0_MASK     (N12_BITS_MSK << ITH_MEMDBG_PASSID_AXI0_BIT)
#define ITH_MEMDBG_PASSID_AXI1_BIT      12
#define ITH_MEMDBG_PASSID_AXI1_MASK     (N12_BITS_MSK << ITH_MEMDBG_PASSID_AXI1_BIT)
#define ITH_MEMDBG_PASSID_AXI2_BIT      24
#define ITH_MEMDBG_PASSID_AXI2_MASK     (N04_BITS_MSK << ITH_MEMDBG_PASSID_AXI2_BIT)
#define ITH_MEMDBG_PASSID_AXI3_BIT      28
#define ITH_MEMDBG_PASSID_AXI3_MASK     (N04_BITS_MSK << ITH_MEMDBG_PASSID_AXI3_BIT)

#define ITH_MEMDBG_PASSID_AXI0_NONE     (0xFFF)
#define ITH_MEMDBG_PASSID_AXI1_NONE     (0x0F0)
#define ITH_MEMDBG_PASSID_AXI2_NONE     (0xF)
#define ITH_MEMDBG_PASSID_AXI3_NONE     (0xF)

#define ITH_MEMDBG_STATUS_REG           0x14
#define ITH_MEMDBG_AXI0ID_BIT           0
#define ITH_MEMDBG_AXI0ID_MASK          (N12_BITS_MSK << ITH_MEMDBG_AXI0ID_BIT)
#define ITH_MEMDBG_AXI1ID_BIT           12
#define ITH_MEMDBG_AXI1ID_MASK          (N12_BITS_MSK << ITH_MEMDBG_AXI1ID_BIT)
#define ITH_MEMDBG_AXI2ID_BIT           24
#define ITH_MEMDBG_AXI2ID_MASK          (N04_BITS_MSK << ITH_MEMDBG_AXI2ID_BIT)
#define ITH_MEMDBG_AXI3ID_BIT           28
#define ITH_MEMDBG_AXI3ID_MASK          (N04_BITS_MSK << ITH_MEMDBG_AXI3ID_BIT)

// MEM STAT
#define ITH_MEMSTAT_SERV_PERIOD_REG     0x03E0

#define ITH_MEMSTAT_SERV_CR_REG         0x03E2
#define ITH_MEMSTAT_SERV_EN_BIT         15
#define ITH_MEMSTAT_SERV_DONE_BIT       12
#define ITH_MEMSTAT_SERV1_RQ_BIT        5
#define ITH_MEMSTAT_SERV1_RQ_MASK       (N05_BITS_MSK << ITH_MEMSTAT_SERV1_RQ_BIT)
#define ITH_MEMSTAT_SERV0_RQ_BIT        0
#define ITH_MEMSTAT_SERV0_RQ_MASK       (N05_BITS_MSK << ITH_MEMSTAT_SERV0_RQ_BIT)

#define ITH_MEMSTAT_SERVALL_NUM_LO_REG  0x03E4
#define ITH_MEMSTAT_SERVALL_NUM_HI_REG  0x03E6
#define ITH_MEMSTAT_SERV0_NUM_LO_REG    0x03E8
#define ITH_MEMSTAT_SERV0_NUM_HI_REG    0x03EA
#define ITH_MEMSTAT_SERV1_NUM_LO_REG    0x03EC
#define ITH_MEMSTAT_SERV1_NUM_HI_REG    0x03EE
#define ITH_MEMSTAT_SERVALL_CNT_LO_REG  0x03F0
#define ITH_MEMSTAT_SERVALL_CNT_HI_REG  0x03F2
#define ITH_MEMSTAT_SERVTIME_CNT_LO_REG 0x03F4
#define ITH_MEMSTAT_SERVTIME_CNT_HI_REG 0x03F6

#define ITH_MEMSTAT_INTR_REG            0x03F8
#define ITH_MEMSTAT_INTR_WP1_BIT        10
#define ITH_MEMSTAT_INTR_WP0_BIT        9
#define ITH_MEMSTAT_INTR_EN_BIT         8
#define ITH_MEMSTAT_INTR_WC_BIT         0

// MEM WRAP
#define ITH_MEMWRAP_AXI0_REG            0x208
#define ITH_MEMWRAP_AXI1_REG            0x248
#define ITH_MEMWRAP_AXI2_REG            0x2A0
#define ITH_MEMWRAP_AXI3_REG            0x2DC

#define ITH_MEMWRAP_FLUSH_BIT           0
#define ITH_MEMWRAP_FLUSH_MASK          (N01_BITS_MSK << ITH_MEMWRAP_FLUSH_BIT)

// MEM ISP TILING
#define ITH_MEM_ISP_TILING_READ_REG     (ITH_MEM_BASE + 0x290)
#define ITH_ISP_TILING_READ_EN_BIT      15
#define ITH_ISP_TILING_READ_EN_MASK     (N01_BITS_MSK << ITH_ISP_TILING_READ_EN_BIT)
#define ITH_ISP_TILING_READ_WIDTH_BIT   11
#define ITH_ISP_TILING_READ_WIDTH_MASK  (N01_BITS_MSK << ITH_ISP_TILING_READ_WIDTH_BIT)
#define ITH_ISP_TILING_READ_PITCH_BIT   8
#define ITH_ISP_TILING_READ_PITCH_MASK  (N03_BITS_MSK << ITH_ISP_TILING_READ_PITCH_BIT)

#define ITH_MEM_ISP_TILING_WRITE_REG    (ITH_MEM_BASE + 0x2D0)
#define ITH_ISP_TILING_WRITE_EN_BIT     15
#define ITH_ISP_TILING_WRITE_EN_MASK    (N01_BITS_MSK << ITH_ISP_TILING_WRITE_EN_BIT)
#define ITH_ISP_TILING_WRITE_WIDTH_BIT  11
#define ITH_ISP_TILING_WRITE_WIDTH_MASK (N01_BITS_MSK << ITH_ISP_TILING_WRITE_WIDTH_BIT)
#define ITH_ISP_TILING_WRITE_PITCH_BIT  8
#define ITH_ISP_TILING_WRITE_PITCH_MASK (N03_BITS_MSK << ITH_ISP_TILING_WRITE_PITCH_BIT)

// USB
#define ITH_USB_HC_MISC_REG             0x40
#define ITH_USB_HOSTPHY_SUSPEND_BIT     6

// TV ENCODER
#define ITH_TVE_NTSC_PAL_SYS_REG        (0x1A00)

#define ITH_TVE_DAC_OUT_REG             (0x1A02)
#define ITH_TVE_B0DAC1OUT_BIT           0
#define ITH_TVE_B0DAC1OUT_MASK          (N04_BITS_MSK << ITH_TVE_B0DAC1OUT_BIT)
#define ITH_TVE_B0DAC2OUT_BIT           4
#define ITH_TVE_B0DAC2OUT_MASK          (N04_BITS_MSK << ITH_TVE_B0DAC2OUT_BIT)
#define ITH_TVE_B0DAC3OUT_BIT           8
#define ITH_TVE_B0DAC3OUT_MASK          (N04_BITS_MSK << ITH_TVE_B0DAC3OUT_BIT)
#define ITH_TVE_B0DAC4OUT_BIT           12
#define ITH_TVE_B0DAC4OUT_MASK          (N04_BITS_MSK << ITH_TVE_B0DAC4OUT_BIT)

#define ITH_TVE_B0TCYCSTEP_REG          (0x1CEC)

// FPC & STC
#define ITH_PCR_CLOCK_REG1              (0x40)
#define ITH_PCR_CLOCK_REG2              (0x42)
#define ITH_PCR_SETTING_REG             (0x80)
#define ITH_PCR_BASECNT_LO_REG          (0x82)
#define ITH_PCR_BASECNT_HI_REG          (0x84)
#define ITH_PCR_COUNT_REG               (0x86)

// LCD
#define ITH_LCD_BASEA_REG               0x0010
#define ITH_LCD_BASEA_BIT               0
#define ITH_LCD_BASEA_MASK              (0xFFFFFFF8 << ITH_LCD_BASEA_BIT)

#define ITH_LCD_WIDTH_REG               0x0008
#define ITH_LCD_WIDTH_BIT               0
#define ITH_LCD_WIDTH_MASK              (0x07FF << ITH_LCD_WIDTH_BIT)

#define ITH_LCD_HEIGHT_REG              0x0008
#define ITH_LCD_HEIGHT_BIT              16
#define ITH_LCD_HEIGHT_MASK             (0x07FF << ITH_LCD_HEIGHT_BIT)

#define ITH_LCD_PITCH_REG               0x000C
#define ITH_LCD_PITCH_BIT               0
#define ITH_LCD_PITCH_MASK              (0x1FF8 << ITH_LCD_PITCH_BIT)

// Command queue
#define ITH_CMDQ_BASE_OFFSET            0x8000

#define ITH_CMDQ_BASE_REG               0x0000
#define ITH_CMDQ_BASE_BIT               0
#define ITH_CMDQ_BASE_MASK              (0xFFFFFFF8 << ITH_CMDQ_BASE_BIT)

#define ITH_CMDQ_LEN_REG                0x0004
#define ITH_CMDQ_LEN_BIT                0
#define ITH_CMDQ_LEN_MASK               (N09_BITS_MSK << ITH_CMDQ_LEN_BIT)

#define ITH_CMDQ_WR_REG                 0x0008
#define ITH_CMDQ_WR_BIT                 0
#define ITH_CMDQ_WR_MASK                (0xFFFFFFF8 << ITH_CMDQ_WR_BIT)

#define ITH_CMDQ_RD_REG                 0x000C
#define ITH_CMDQ_RD_BIT                 0
#define ITH_CMDQ_RD_MASK                (0xFFFFFFF8 << ITH_CMDQ_RD_BIT)

#define ITH_CMDQ_ENWAITISPBUSYINAHB_BIT 15
#define ITH_CMDQ_ENISPCMDTOAHB_BIT      14
#define ITH_CMDQ_ENINTR_BIT             13
#define ITH_CMDQ_ENBIGENDIAN_BIT        12
#define ITH_CMDQ_ENCMDERRSTOP_BIT       3
#define ITH_CMDQ_TURBOFLIP_BIT          2
#define ITH_CMDQ_FLIPBUFMODE_BIT        1

#define ITH_CMDQ_CR_REG                 0x0010
#define ITH_CMDQ_CR_BIT                 0
#define ITH_CMDQ_CR_MASK                (N13_BITS_MSK << ITH_CMDQ_CR_BIT)

#define ITH_CMDQ_FLIPIDX_REG            0x0014
#define ITH_CMDQ_FLIPIDX_BIT            0
#define ITH_CMDQ_FLIPIDX_MASK           (N02_BITS_MSK << ITH_CMDQ_FLIPIDX_BIT)

#define ITH_CMDQ_IR1_REG                0x0018

#define ITH_CMDQ_INTCLR_BIT             15
#define ITH_CMDQ_INT_MODE_BIT           4
#define ITH_CMDQ_INT_EN_BIT             0

#define ITH_CMDQ_IR2_REG                0x001C

#define ITH_CMDQ_INTSTATUS_BIT          12
#define ITH_CMDQ_AHBEMTPY_BIT           11
#define ITH_CMDQ_FLIPCMDLOAD_BIT        10
#define ITH_CMDQ_OVGCMDLOAD_BIT         9
#define ITH_CMDQ_ISPCMDLOAD_BIT         8
#define ITH_CMDQ_FLIPEMPTY_BIT          7
#define ITH_CMDQ_ISPBUSY_BIT            6
#define ITH_CMDQ_DPUBUSY_BIT            5
#define ITH_CMDQ_OVGBUSY_BIT            4
#define ITH_CMDQ_CMQFAIL_BIT            3
#define ITH_CMDQ_ALLIDLE_BIT            2
#define ITH_CMDQ_HQEMPTY_BIT            1
#define ITH_CMDQ_SQEMPTY_BIT            0

#define ITH_CMDQ_SR1_REG                0x0020
#define ITH_CMDQ_SR2_REG                0x0024
#define ITH_CMDQ_SR3_REG                0x0028
#define ITH_CMDQ_SR4_REG                0x002C

#define ITH_CMDQ_REMAINSIZE_REG         0x0030

// OPENVG
#define ITH_OPVG_BID1_REG               0x1FC
#define ITH_OPVG_BID2_REG               0x200
#define ITH_OPVG_BID3_REG               0x204

// NAND
#define ITH_NAND_AUTOBOOTCFG_REG        0x38
#define ITH_NAND_AUTOBOOTCFG_BIT        8
#define ITH_NAND_AUTOBOOTCFG_MASK       (N02_BITS_MSK << ITH_NAND_AUTOBOOTCFG_BIT)

// GPIO
#define ITH_GPIO1_DATAOUT_REG           0x00
#define ITH_GPIO1_DATAIN_REG            0x04
#define ITH_GPIO1_PINDIR_REG            0x08
#define ITH_GPIO1_DATASET_REG           0x0C
#define ITH_GPIO1_DATACLR_REG           0x10
#define ITH_GPIO1_PULLEN_REG            0x14
#define ITH_GPIO1_PULLTYPE_REG          0x18
#define ITH_GPIO1_INTREN_REG            0x1C
#define ITH_GPIO1_INTRRAWSTATE_REG      0x20
#define ITH_GPIO1_INTRMASKSTATE_REG     0x24
#define ITH_GPIO1_INTRMASK_REG          0x28
#define ITH_GPIO1_INTRCLR_REG           0x2C
#define ITH_GPIO1_INTRTRIG_REG          0x30
#define ITH_GPIO1_INTRBOTH_REG          0x34
#define ITH_GPIO1_INTRRISENEG_REG       0x38
#define ITH_GPIO1_BOUNCEEN_REG          0x3C
#define ITH_GPIO1_SMT_SET_REG           0x40
#define ITH_GPIO1_SR_SET_REG            0x44
#define ITH_GPIO1_FFIEN_REG             0x48
#define ITH_GPIO1_FFOEN_REG             0x4C
#define ITH_GPIO1_DRIVING_SET_L_REG     0x58
#define ITH_GPIO1_DRIVING_SET_H_REG     0x5C
#define ITH_GPIO1_MODESEL_G1_REG        0x60
#define ITH_GPIO1_MODESEL_G2_REG        0x64
#define ITH_GPIO1_MODESEL_G3_REG        0x68
#define ITH_GPIO1_MODESEL_G4_REG        0x6C

#define ITH_GPIO2_DATAOUT_REG           0x80
#define ITH_GPIO2_DATAIN_REG            0x84
#define ITH_GPIO2_PINDIR_REG            0x88
#define ITH_GPIO2_DATASET_REG           0x8C
#define ITH_GPIO2_DATACLR_REG           0x90
#define ITH_GPIO2_PULLEN_REG            0x94
#define ITH_GPIO2_PULLTYPE_REG          0x98
#define ITH_GPIO2_INTREN_REG            0x9C
#define ITH_GPIO2_INTRRAWSTATE_REG      0xA0
#define ITH_GPIO2_INTRMASKSTATE_REG     0xA4
#define ITH_GPIO2_INTRMASK_REG          0xA8
#define ITH_GPIO2_INTRCLR_REG           0xAC
#define ITH_GPIO2_INTRTRIG_REG          0xB0
#define ITH_GPIO2_INTRBOTH_REG          0xB4
#define ITH_GPIO2_INTRRISENEG_REG       0xB8
#define ITH_GPIO2_BOUNCEEN_REG          0xBC
#define ITH_GPIO2_SMT_SET_REG           0xC0
#define ITH_GPIO2_SR_SET_REG            0xC4
#define ITH_GPIO2_FFIEN_REG             0xC8
#define ITH_GPIO2_FFOEN_REG             0xCC
#define ITH_GPIO2_DRIVING_SET_L_REG     0xD8
#define ITH_GPIO2_DRIVING_SET_H_REG     0xDC
#define ITH_GPIO2_MODESEL_G1_REG        0xE0
#define ITH_GPIO2_MODESEL_G2_REG        0xE4
#define ITH_GPIO2_MODESEL_G3_REG        0xE8
#define ITH_GPIO2_MODESEL_G4_REG        0xEC

#define ITH_GPIO3_DATAOUT_REG           0x100
#define ITH_GPIO3_DATAIN_REG            0x104
#define ITH_GPIO3_PINDIR_REG            0x108
#define ITH_GPIO3_DATASET_REG           0x10C
#define ITH_GPIO3_DATACLR_REG           0x110
#define ITH_GPIO3_PULLEN_REG            0x114
#define ITH_GPIO3_PULLTYPE_REG          0x118
#define ITH_GPIO3_INTREN_REG            0x11C
#define ITH_GPIO3_INTRRAWSTATE_REG      0x120
#define ITH_GPIO3_INTRMASKSTATE_REG     0x124
#define ITH_GPIO3_INTRMASK_REG          0x128
#define ITH_GPIO3_INTRCLR_REG           0x12C
#define ITH_GPIO3_INTRTRIG_REG          0x130
#define ITH_GPIO3_INTRBOTH_REG          0x134
#define ITH_GPIO3_INTRRISENEG_REG       0x138
#define ITH_GPIO3_BOUNCEEN_REG          0x13C
#define ITH_GPIO3_SMT_SET_REG           0x140
#define ITH_GPIO3_SR_SET_REG            0x144
#define ITH_GPIO3_FFIEN_REG             0x148
#define ITH_GPIO3_FFOEN_REG             0x14C
#define ITH_GPIO3_DRIVING_SET_L_REG     0x158
#define ITH_GPIO3_DRIVING_SET_H_REG     0x15C
#define ITH_GPIO3_MODESEL_G1_REG        0x160
#define ITH_GPIO3_MODESEL_G2_REG        0x164
#define ITH_GPIO3_MODESEL_G3_REG        0x168
#define ITH_GPIO3_MODESEL_G4_REG        0x16C

#define ITH_GPIO4_DATAOUT_REG           0x180
#define ITH_GPIO4_DATAIN_REG            0x184
#define ITH_GPIO4_PINDIR_REG            0x188
#define ITH_GPIO4_DATASET_REG           0x18C
#define ITH_GPIO4_DATACLR_REG           0x190
#define ITH_GPIO4_PULLEN_REG            0x194
#define ITH_GPIO4_PULLTYPE_REG          0x198
#define ITH_GPIO4_INTREN_REG            0x19C
#define ITH_GPIO4_INTRRAWSTATE_REG      0x1A0
#define ITH_GPIO4_INTRMASKSTATE_REG     0x1A4
#define ITH_GPIO4_INTRMASK_REG          0x1A8
#define ITH_GPIO4_INTRCLR_REG           0x1AC
#define ITH_GPIO4_INTRTRIG_REG          0x1B0
#define ITH_GPIO4_INTRBOTH_REG          0x1B4
#define ITH_GPIO4_INTRRISENEG_REG       0x1B8
#define ITH_GPIO4_BOUNCEEN_REG          0x1BC
#define ITH_GPIO4_SMT_SET_REG           0x1C0
#define ITH_GPIO4_SR_SET_REG            0x1C4
#define ITH_GPIO4_FFIEN_REG             0x1C8
#define ITH_GPIO4_FFOEN_REG             0x1CC
#define ITH_GPIO4_DRIVING_SET_REG       0x1D8
#define ITH_GPIO4_MODESEL_G1_REG        0x1E0
#define ITH_GPIO4_MODESEL_G2_REG        0x1E4
#define ITH_GPIO4_MODESEL_G3_REG        0x1E8
#define ITH_GPIO4_MODESEL_G4_REG        0x1EC

//Extend Setting
#define ITH_GPIO_MISC0_REG              0x204
#define ITH_GPIO_MISC1_REG              0x208
#define ITH_GPIO_URSEL1_REG             0x210
#define ITH_GPIO_URSEL2_REG             0x214
#define ITH_GPIO_URSEL3_REG             0x218
#define ITH_GPIO_URSEL4_REG             0x21C
#define ITH_GPIO_URSEL5_REG             0x220
#define ITH_GPIO_URSEL6_REG             0x224
#define ITH_GPIO_GPIO_BPSPISEL1         0x230
#define ITH_GPIO_GPIO_BPSPISEL2         0x234
#define ITH_GPIO_PWMSEL01_REG			0x248
#define ITH_GPIO_PWMSEL23_REG			0x24C
#define ITH_GPIO_PWMSEL45_REG			0x250
#define ITH_GPIO_PWMSEL67_REG			0x254
#define ITH_GPIO_CANBUS0_REG			0x258
#define ITH_GPIO_CANBUS1_REG			0x25C


//Common Setting
#define ITH_GPIO_BOUNCEPRESCALE_REG     0x200
#define ITH_GPIO_FEATURE_REG            0x238
#define ITH_GPIO_REV_REG                0x23C

#define ITH_GPIO_URTX_EN_BIT            15
#define ITH_GPIO_URTX_BIT               0
#define ITH_GPIO_URTX_MASK              (0x7F << ITH_GPIO_URTX_BIT)
#define ITH_GPIO_URRX_EN_BIT            31
#define ITH_GPIO_URRX_BIT               16
#define ITH_GPIO_URRX_MASK              (0x7F << ITH_GPIO_URRX_BIT)

#define ITH_GPIO_MISC_SET_REG           0x204
#define ITH_GPIO_EN_AHB_SRAM_POS        (24u)   // 0: disable; 1: enable AHBSRAM wishbone path
#define ITH_GPIO_HOST_SEL_POS           4
#define ITH_GPIO_HOST_SEL_MSK           (0x7 << ITH_GPIO_HOST_SEL_POS)

#define ITH_GPIO_HOSTSEL_HOSTCFG        0x0
#define ITH_GPIO_HOSTSEL_GPIO           0x4
#define ITH_GPIO_HOSTSEL_ARMJTAG        0x5
#define ITH_GPIO_HOSTSEL_RISCJTAG       0x6
#define ITH_GPIO_HOSTSEL_ARMRISCJTAG    0x7

#define ITH_GPIO_AHBSRAM_ENDIAN_BIT     3
#define ITH_GPIO_ND_DIDLYEN_BIT         22
#define ITH_GPIO_AHBTOMEM_ENDIAN_BIT    23
#define ITH_GPIO_AHBSRAM_WBEN_BIT       24
#define ITH_GPIO_SPI_BYPASS_BIT         25
#define ITH_GPIO_UR2_RXSRC_BIT          30
#define ITH_GPIO_UR3_RXSRC_BIT          31

#define ITH_GPIO_PWM_LOW_EN_BIT			15
#define ITH_GPIO_PWM_LOW_BIT			0
#define ITH_GPIO_PWM_LOW_MASK			(0x7F << ITH_GPIO_PWM_LOW_BIT)
#define ITH_GPIO_PWM_HIGH_EN_BIT		31
#define ITH_GPIO_PWM_HIGH_BIT			16
#define ITH_GPIO_PWM_HIGH_MASK			(0x7F << ITH_GPIO_PWM_HIGH_BIT)

// Interrupt
#define ITH_INTR_IRQ1_SRC_REG           0x00
#define ITH_INTR_IRQ1_EN_REG            0x04
#define ITH_INTR_IRQ1_CLR_REG           0x08
#define ITH_INTR_IRQ1_TRIGMODE_REG      0x0C
#define ITH_INTR_IRQ1_TRIGLEVEL_REG     0x10
#define ITH_INTR_IRQ1_STATUS_REG        0x14
#define ITH_INTR_IRQ1_SWINTR_REG        0x18
#define ITH_INTR_FIQ1_SRC_REG           0x20
#define ITH_INTR_FIQ1_EN_REG            0x24
#define ITH_INTR_FIQ1_CLR_REG           0x28
#define ITH_INTR_FIQ1_TRIGMODE_REG      0x2C
#define ITH_INTR_FIQ1_TRIGLEVEL_REG     0x30
#define ITH_INTR_FIQ1_STATUS_REG        0x34
#define ITH_INTR_FIQ1_SWINTR_REG        0x38
#define ITH_INTR_IRQ2_SRC_REG           0x100
#define ITH_INTR_IRQ2_EN_REG            0x104
#define ITH_INTR_IRQ2_CLR_REG           0x108
#define ITH_INTR_IRQ2_TRIGMODE_REG      0x10C
#define ITH_INTR_IRQ2_TRIGLEVEL_REG     0x110
#define ITH_INTR_IRQ2_STATUS_REG        0x114
#define ITH_INTR_IRQ2_SWINTR_REG        0x118
#define ITH_INTR_FIQ2_SRC_REG           0x120
#define ITH_INTR_FIQ2_EN_REG            0x124
#define ITH_INTR_FIQ2_CLR_REG           0x128
#define ITH_INTR_FIQ2_TRIGMODE_REG      0x12C
#define ITH_INTR_FIQ2_TRIGLEVEL_REG     0x130
#define ITH_INTR_FIQ2_STATUS_REG        0x134
#define ITH_INTR_FIQ2_SWINTR_REG        0x138

#define ITH_INTR_SWINT_BIT              0

// RTC
#define ITH_RTC_SEC_REG                 0x00
#define ITH_RTC_SEC_MASK                0x3F

#define ITH_RTC_MIN_REG                 0x04
#define ITH_RTC_MIN_MASK                0x3F

#define ITH_RTC_HOUR_REG                0x08
#define ITH_RTC_HOUR_MASK               0x1F

#define ITH_RTC_DAY_REG                 0x0C
#define ITH_RTC_DAY_MASK                0xFFFF

#define ITH_RTC_WEEK_REG                0x10
#define ITH_RTC_WEEK_MASK               0x7

#define ITH_RTC_ASEC_REG                0x14
#define ITH_RTC_ASEC_MASK               0x3F

#define ITH_RTC_AMIN_REG                0x18
#define ITH_RTC_AMIN_MASK               0x3F

#define ITH_RTC_AHOUR_REG               0x1c
#define ITH_RTC_AHOUR_MASK              0x1F

#define ITH_RTC_ADAY_REG                0x20
#define ITH_RTC_ADAY_MASK               0xFFFF

#define ITH_RTC_AWEEK_REG               0x24
#define ITH_RTC_AWEEK_MASK              0x7

#define ITH_RTC_REC_REG                 0x28
#define ITH_RTC_CR_REG                  0x2C

#define ITH_RTC_PWREN_IOSEL_REG         0x2C
#define ITH_RTC_PWREN_IOSEL_BIT         22
#define ITH_RTC_PWREN_IOSEL_MASK        (N02_BITS_MSK << ITH_RTC_PWREN_IOSEL_BIT)

#define ITH_RTC_STATE_REG               0x2C
#define ITH_RTC_STATE_BIT               25
#define ITH_RTC_STATE_MASK              (N06_BITS_MSK << ITH_RTC_STATE_BIT)

#define ITH_RTC_WSEC_REG                0x30
#define ITH_RTC_WSEC_MASK               0x3F

#define ITH_RTC_WMIN_REG                0x34
#define ITH_RTC_WMIN_MASK               0x3F

#define ITH_RTC_WHOUR_REG               0x38
#define ITH_RTC_WHOUR_MASK              0x1F

#define ITH_RTC_WDAY_REG                0x3C
#define ITH_RTC_WDAY_MASK               0xFFFF

#define ITH_RTC_WWEEK_REG				0x40
#define ITH_RTC_WWEEK_MASK				0x7

#define ITH_RTC_INTRSTATE_REG           0x44

#define ITH_RTC_DIV_REG                 0x48
#define ITH_RTC_DIV_EN_BIT              31
#define ITH_RTC_DIV_EN_MASK             (N01_BITS_MSK << ITH_RTC_DIV_EN_BIT)
#define ITH_RTC_DIV_SRC_BIT             30
#define ITH_RTC_DIV_SRC_MASK            (N01_BITS_MSK << ITH_RTC_DIV_SRC_BIT)
#define ITH_RTC_DIV_CYCLE_BIT           0
#define ITH_RTC_DIV_CYCLE_MASK          (0x3FFFFFFF << ITH_RTC_DIV_CYCLE_BIT)

// UART
#define ITH_UART_DLL_REG                0x0                     // Divisor Register LSB
#define ITH_UART_THR_REG                0x00                    // Transmitter Holding Register(Write)
#define ITH_UART_RBR_REG                0x00                    // Receive Buffer register (Read)

#define ITH_UART_IER_REG                0x04                    // Interrupt Enable Register(Write)
#define ITH_UART_DLM_REG                0x4                     // Divisor Register MSB

#define ITH_UART_RECV_READY             0x04
#define ITH_UART_THR_EMPTY              0x02

#define ITH_UART_IIR_REG                0x08                    // Interrupt Identification Register (Read)
#define ITH_UART_IIR_TXFIFOFULL         0x10                    // This bit is set as 1 when TX FIFO is full

#define ITH_UART_FCR_REG                0x08                    // FIFO control register(Write)
#define ITH_UART_FCR_FIFO_EN_BIT        0                       // FIFO Enable
#define ITH_UART_FCR_RXFIFO_RESET_BIT   1                       // Rx FIFO Reset
#define ITH_UART_FCR_TXFIFO_RESET_BIT   2                       // Tx FIFO Reset
#define ITH_UART_FCR_TXFIFO_TRGL_BIT    4
#define ITH_UART_FCR_TXFIFO_TRGL_MASK   (N02_BITS_MSK << ITH_UART_FCR_TXFIFO_TRGL_BIT)
#define ITH_UART_FCR_RXFIFO_TRGL_BIT    6
#define ITH_UART_FCR_RXFIFO_TRGL_MASK   (N02_BITS_MSK << ITH_UART_FCR_RXFIFO_TRGL_BIT)

#define ITH_UART_LCR_REG                0x0C                    // Line Control register
#define ITH_UART_LCR_STOP               0x4
#define ITH_UART_LCR_EVEN               0x18                    // Even Parity
#define ITH_UART_LCR_ODD                0x8                     // Odd Parity
#define ITH_UART_LCR_PE                 0x8                     // Parity Enable
#define ITH_UART_LCR_SETBREAK           0x40                    // Set Break condition
#define ITH_UART_LCR_STICKPARITY        0x20                    // Stick Parity Enable
#define ITH_UART_LCR_DLAB               0x80                    // Divisor Latch Access Bit

#define ITH_UART_DLH_REG                0x10                    // Fraction Divisor Register
#define ITH_UART_MCR_REG                0x10                    // Modem Control Register
#define ITH_UART_MCR_DMAMODE2           0x20                    // DMA mode2

#define ITH_UART_LSR_REG                0x14                    // Line status register(Read)
#define ITH_UART_LSR_DR                 0x1                     // Data Ready
#define ITH_UART_LSR_OVERRUN			0x2						// Overrun Error
#define ITH_UART_LSR_PARITY				0x4						// Parity Error
#define ITH_UART_LSR_FRAMING			0x8						// Framing Error
#define ITH_UART_LSR_BREAK				0x10					// Break interrupt
#define ITH_UART_LSR_THRE               0x20                    // THR Empty
#define ITH_UART_LSR_TRANS_EMPTY        0x40

#define ITH_UART_MDR_REG                0x20
#define ITH_UART_MDR_MODE_SEL_MASK      0x03

#define ITH_UART_CLK_REG                0x22
#define ITH_UART_CLK_BIT                15
#define ITH_UART_CLK_SRC_BIT            14

#define ITH_UART_FEATURE_REG            0x98
#define ITH_UART_FIFO_DEPTH_BIT         0
#define ITH_UART_FIFO_DEPTH_MASK        (N04_BITS_MSK << ITH_UART_FIFO_DEPTH_BIT)

// Timer
#define ITH_TIMER1_CNT_REG              0x0
#define ITH_TIMER1_LOAD_REG             0x4
#define ITH_TIMER1_MATCH1_REG           0x8
#define ITH_TIMER1_MATCH2_REG           0xC

#define ITH_TIMER1_CR_REG               0x80         // 9850

#define ITH_TIMER_EN_BIT                0
#define ITH_TIMER_CLK_BIT               1
#define ITH_TIMER_UPDOWN_BIT            2
#define ITH_TIMER_ONESHOT_BIT           3
#define ITH_TIMER_MODE_BIT              4
#define ITH_TIMER_PWMEN_BIT             5
#define ITH_TIMER_EN64_BIT              6

#define ITH_TIMER_INTRRAWSTATE_REG      0xA0
#define ITH_TIMER_INTRSTATE_REG         0xA4
#define ITH_TIMER_INTRMASK_REG          0xA8
#define ITH_TIMER_TMUS_EN_REG			0xB0
#define ITH_TIMER_TMUS_COUNTER_REG		0xB4
#define ITH_TIMER_BASE_1TO2_GAP			(ITH_TIMER2_BASE - ITH_TIMER_BASE)


// Watch Dog
#define ITH_WD_COUNTER_REG              0x00
#define ITH_WD_LOAD_REG                 0x04
#define ITH_WD_RESTART_REG              0x08
#define ITH_WD_CR_REG                   0x0C

#define ITH_WD_AUTORELOAD               0x5AB9

// Remote IR
#define ITH_IR_RX_CTRL_REG				0x00
#define ITH_IR_RX_CAPEDGE_BIT           6
#define ITH_IR_RX_CAPEDGE_MASK          (N02_BITS_MSK << ITH_IR_RX_CAPEDGE_BIT)
#define ITH_IR_RX_CAPMODE_BIT			12
#define ITH_IR_RX_CAPMODE_MASK			(N02_BITS_MSK << ITH_IR_RX_CAPMODE_BIT)

#define ITH_IR_RX_STATUS_REG			0x04
#define ITH_IR_DATAREADY_BIT            0
#define ITH_IR_OE_BIT                   3

#define ITH_IR_CLEAR_BIT                9

#define ITH_IR_RX_PRESCALE_REG			0x08
#define ITH_IR_RX_DATA_REG				0x0C

#define ITH_IR_TX_CTRL_REG				0x10

#define ITH_IR_TX_STATUS_REG			0x14
#define ITH_IR_TX_FIFO_FULL_BIT         1
#define ITH_IR_TX_FIFO_EMPTY_BIT        2

#define ITH_IR_TX_PRESCALE_REG			0x18
#define ITH_IR_TX_MOD_REG               0x1C
#define ITH_IR_TX_DATA_REG				0x20
#define ITH_IR_RX_MOD_FILTER_REG        0x24
#define ITH_IR_RX_MOD_FILTER_RST_BIT	31
#define ITH_IR_FS1_REG					0x28
#define ITH_IR_FS2_REG					0x2C
#define ITH_IR_FDC1_REG					0x30
#define ITH_IR_FDC2_REG					0x34
#define ITH_IR_FDC3_REG					0x38
#define ITH_IR_FDC4_REG					0x3C
#define ITH_IR_GPIO_SEL_REG             0x40
#define ITH_IR_HWCFG_REG                0x44

#define ITH_IR_WIDTH_BIT                8
#define ITH_IR_WIDTH_MASK               (N08_BITS_MSK << ITH_IR_WIDTH_BIT)

// DMA
#if defined(CFG_AXI_DMA)

    #include "ith/ith_defs_dma_axi.h"

#else     // #if defined(CFG_AXI_DMA)

    #define ITH_DMA_INT_REG             0x00
    #define ITH_DMA_INT_TC_REG          0x04
    #define ITH_DMA_INT_TC_CLR_REG      0x08

    #define ITH_DMA_INT_ERRABT_REG      0x0C
    #define ITH_DMA_INT_ERRABT_CLR_REG  0x10

    #define ITH_DMA_TC_REG              0x14
    #define ITH_DMA_ERRABT_REG          0x18
    #define ITH_DMA_CH_EN_REG           0x1C
    #define ITH_DMA_CH_BUSY_REG         0x20

    #define ITH_DMA_CSR_REG             0x24                // main configuration status register
    #define ITH_DMA_M1_BIG_ENDIAN_BIT   2
    #define ITH_DMA_M0_BIG_ENDIAN_BIT   1
    #define ITH_DMA_EN_BIT              0

    #define ITH_DMA_SYNC_REG            0x28
    #define ITH_DMA_REVISION_REG        0x30

    #define ITH_DMA_FEATURE_REG         0x34
    #define ITH_DMA_MAX_CHNO_N_BIT      12
    #define ITH_DMA_MAX_CHNO_N_MASK     (N04_BITS_MSK << ITH_DMA_MAX_CHNO_N_BIT)

    #define ITH_DMA_C0_CSR_REG          0x100               // channel control register
    #define ITH_DMA_CUR_TC_MSK_BIT      31
    #define ITH_DMA_CUR_TC_MASK         (N01_BITS_MSK << ITH_DMA_CUR_TC_MSK_BIT)

    #define ITH_DMA_FF_TH_BIT           24
    #define ITH_DMA_FF_TH_MASK          (N03_BITS_MSK << ITH_DMA_FF_TH_BIT)
enum
{
    ITH_DMA_FF_TH_1     = 0x0,
    ITH_DMA_FF_TH_2     = 0x1,
    ITH_DMA_FF_TH_4     = 0x2,
    ITH_DMA_FF_TH_8     = 0x3,
    ITH_DMA_FF_TH_16    = 0x4
};

    #define ITH_DMA_CH_PRIO_BIT     22
    #define ITH_DMA_CH_PRIO_MASK    (N02_BITS_MSK << ITH_DMA_CH_PRIO_BIT)
typedef enum
{
    ITH_DMA_CH_PRIO_LOWEST  = 0x0,
    ITH_DMA_CH_PRIO_HIGH_3  = 0x1,
    ITH_DMA_CH_PRIO_HIGH_2  = 0x2,
    ITH_DMA_CH_PRIO_HIGHEST = 0x3
} ITHDmaPriority;

    #define ITH_DMA_BURST_SIZE_BIT  16
    #define ITH_DMA_BURST_SIZE_MASK (N03_BITS_MSK << ITH_DMA_BURSTC_SIZE_BIT)
typedef enum
{
    ITH_DMA_BURST_1     = 0,
    ITH_DMA_BURST_4     = 1,
    ITH_DMA_BURST_8     = 2,
    ITH_DMA_BURST_16    = 3,
    ITH_DMA_BURST_32    = 4,
    ITH_DMA_BURST_64    = 5,
    ITH_DMA_BURST_128   = 6,
    ITH_DMA_BURST_256   = 7
} ITHDmaBurst;

    #define ITH_DMA_ABT_BIT         15
    #define ITH_DMA_ABT_MASK        (N01_BITS_MSK << ITH_DMA_ABT_BIT)

    #define ITH_DMA_SRC_WIDTH_BIT   11
    #define ITH_DMA_SRC_WIDTH_MASK  (N03_BITS_MSK << ITH_DMA_SRC_WIDTH_BIT)
    #define ITH_DMA_DST_WIDTH_BIT   8
    #define ITH_DMA_DST_WIDTH_MASK  (N03_BITS_MSK << ITH_DMA_DST_WIDTH_BIT)
typedef enum
{
    ITH_DMA_WIDTH_8     = 0,
    ITH_DMA_WIDTH_16    = 1,
    ITH_DMA_WIDTH_32    = 2
} ITHDmaWidth;

    #define ITH_DMA_MODE_BIT    7
    #define ITH_DMA_MODE_MASK   (N01_BITS_MSK << ITH_DMA_MODE_BIT)
typedef enum
{
    ITH_DMA_NORMAL_MODE         = 0,
    ITH_DMA_HW_HANDSHAKE_MODE   = 1
} ITHDmaMode;

    #define ITH_DMA_SRCAD_CTRL_BIT  5
    #define ITH_DMA_SRCAD_CTRL_MASK (N02_BITS_MSK << ITH_DMA_SRCAD_CTRL_BIT)
    #define ITH_DMA_DSTAD_CTRL_BIT  3
    #define ITH_DMA_DSTAD_CTRL_MASK (N02_BITS_MSK << ITH_DMA_DSTAD_CTRL_BIT)
typedef enum
{
    ITH_DMA_CTRL_INC    = 0,
    ITH_DMA_CTRL_DEC    = 1,
    ITH_DMA_CTRL_FIX    = 2
} ITHDmaAddrCtl;

    #define ITH_DMA_SRC_SEL_BIT 2
    #define ITH_DMA_DST_SEL_BIT 1
typedef enum
{
    ITH_DMA_MASTER_0    = 0,
    ITH_DMA_MASTER_1    = 1
} ITHDmaDataMaster;

    #define ITH_DMA_CH_EN_MASK  0x1

    #define ITH_DMA_C0_CFG_REG  0x104                       // channel configuration register
    #define ITH_DST_HE_BIT      13
    #define ITH_DST_HE_MASK     (N01_BITS_MSK << ITH_DST_HE_BIT)
    #define ITH_DST_RS_BIT      9
    #define ITH_DST_RS_MASK     (N04_BITS_MSK << ITH_DST_RS_BIT)
    #define ITH_SRC_HE_BIT      7
    #define ITH_SRC_HE_MASK     (N01_BITS_MSK << ITH_SRC_HE_BIT)
    #define ITH_SRC_RS_BIT      3
    #define ITH_SRC_RS_MASK     (N04_BITS_MSK << ITH_SRC_RS_BIT)
typedef enum
{
    ITH_DMA_MEM             = 0,
    ITH_DMA_HW_IR_Cap_Tx    = 0,
    ITH_DMA_HW_IR_Cap_Rx    = 1,
    ITH_DMA_HW_SSP_TX       = 2,
    ITH_DMA_HW_SSP_RX       = 3,
    ITH_DMA_HW_SSP2_TX      = 4,
    ITH_DMA_HW_SPDIF        = ITH_DMA_HW_SSP2_TX,
    ITH_DMA_HW_SSP2_RX      = 5,
    ITH_DMA_HW_UART0_TX     = 6,
    ITH_DMA_HW_UART0_RX     = 7,
    ITH_DMA_HW_UART1_TX     = 8,
    ITH_DMA_HW_UART1_RX     = 9,
    ITH_DMA_HW_IIC0_TX      = 10,
    ITH_DMA_HW_IIC0_RX      = 11,
    ITH_DMA_HW_IIC1_TX      = 12,
    ITH_DMA_HW_IIC1_RX      = 13,
    ITH_DMA_HW_SD           = 14
} ITHDmaRequest;

    #define ITH_DMA_INT_MASK        0x7         // D[2]:abort, D[1]:error, D[0]:tc

    #define ITH_DMA_C0_SRCADR_REG   0x108       // channel source address
    #define ITH_DMA_C0_DSTADR_REG   0x10C       // channel destination address
    #define ITH_DMA_C0_LLP_REG      0x110       // linked list descriptor pointer
    #define ITH_DMA_C0_TX_SIZE_REG  0x114       // transfer size

    #define ITH_DMA_CH_OFFSET       0x20

#endif      // #if defined(CFG_AXI_DMA)

#endif      // ITH_DEFS_H
