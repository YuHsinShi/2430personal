#ifndef ITE_ITH_LCD_H
#define ITE_ITH_LCD_H

#include "ite/ith.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup ith_lcd LCD
 *  @{
 */

#define ITH_LCD_SET1_REG                0x0000
#define ITH_LCD_LAYER_SYNC_RQ_BIT       4
#define ITH_LCD_ON_FLY_EN_BIT           7
#define ITH_LCD_CCIR_MODE_BIT           12
#define ITH_LCD_HW_FLIP_BIT             16
#define ITH_LCD_SW_FLIP_MODE_BIT        17
#define ITH_LCD_HSYNC_FLIP_BIT          18
#define ITH_LCD_VIDEO_FLIP_EN_BIT       20
#define ITH_LCD_VIDEO_FLIP_MODE_BIT     21
#define ITH_LCD_FLIP_BUF_CTRL_BIT       22
#define ITH_LCD_ROT_MODE_BIT            28
#define ITH_LCD_ROT_MODE_MASK           (N02_BITS_MSK << ITH_LCD_ROT_MODE_BIT)
#define ITH_LCD_SCAN_TYPE_BIT           30

#define ITH_LCD_SET_MODE_REG            0x0004
#define ITH_LCD_DISPLAY_MODE_BIT        0
#define ITH_LCD_DISPLAY_MODE_MASK       (N12_BITS_MSK << ITH_LCD_DISPLAY_MODE_BIT)

#define ITH_LCD_SWFLIPNUM_REG           0x0004
//#define ITH_LCD_UI_DEC_FLIP_BIT         4
//#define ITH_LCD_UI_DEC_FLIP_MASK        (N02_BITS_MSK << ITH_LCD_UI_DEC_FLIP_BIT)
#define ITH_LCD_SWFLIPNUM_BIT           30
#define ITH_LCD_SWFLIPNUM_MASK          (N02_BITS_MSK << ITH_LCD_SWFLIPNUM_BIT)

#define ITH_LCD_SRCFMT_REG              0x0004
#define ITH_LCD_SRCFMT_BIT              12
#define ITH_LCD_SRCFMT_MASK             (N03_BITS_MSK << ITH_LCD_SRCFMT_BIT)

#define ITH_LCD_BASEB_REG               0x0014
#define ITH_LCD_BASEB_BIT               0
#define ITH_LCD_BASEB_MASK              (0xFFFFFFF8 << ITH_LCD_BASEB_BIT)

#define ITH_LCD_BASEC_REG               0x0018
#define ITH_LCD_BASEC_BIT               0
#define ITH_LCD_BASEC_MASK              (0xFFFFFFF8 << ITH_LCD_BASEC_BIT)

#define ITH_LCD_UPDATE_REG              0x001C
#define ITH_LCD_LAYER1UPDATE_BIT        31
#define ITH_LCD_LAYER1UPDATE_MASK       (N01_BITS_MSK << ITH_LCD_LAYER1UPDATE_BIT)
#define ITH_LCD_DISPEN_BIT              1
#define ITH_LCD_DISPEN_MASK             (N01_BITS_MSK << ITH_LCD_DISPEN_BIT)
#define ITH_LCD_SYNCFIRE_BIT            0
#define ITH_LCD_SYNCFIRE_MASK           (N01_BITS_MSK << ITH_LCD_SYNCFIRE_BIT)

#define ITH_LCD_TEST_COLOR_SET_REG      0x0020
#define ITH_LCD_COLOR_EN_BIT            24
#define ITH_LCD_COLOR_RING_BIT          25
#define ITH_LCD_COLOR_CYCLE_BIT         26
#define ITH_LCD_COLOR_CYCLE_MASK        (N02_BITS_MSK << ITH_LCD_COLOR_CYCLE_BIT)
#define ITH_LCD_COLOR_UPDATE_BIT        31

#define ITH_LCD_HWC_EN_REG              0x0034
#define ITH_LCD_HWC_EN_BIT              30

#define ITH_LCD_HWC_WIDTH_REG           0x0024
#define ITH_LCD_HWC_WIDTH_BIT           0
#define ITH_LCD_HWC_WIDTH_MASK          (N11_BITS_MSK << ITH_LCD_HWC_WIDTH_BIT)

#define ITH_LCD_HWC_HEIGHT_REG          0x0024
#define ITH_LCD_HWC_HEIGHT_BIT          16
#define ITH_LCD_HWC_HEIGHT_MASK         (N11_BITS_MSK << ITH_LCD_HWC_HEIGHT_BIT)

#define ITH_LCD_HWC_PITCH_REG           0x0028
#define ITH_LCD_HWC_PITCH_BIT           0
#define ITH_LCD_HWC_PITCH_MASK          (0x1FFC << ITH_LCD_HWC_PITCH_BIT)

#define ITH_LCD_HWC_POSX_REG            0x002C
#define ITH_LCD_HWC_POSX_BIT            0
#define ITH_LCD_HWC_POSX_MASK           (N11_BITS_MSK << ITH_LCD_HWC_POSX_BIT)

#define ITH_LCD_HWC_POSY_REG            0x002C
#define ITH_LCD_HWC_POSY_BIT            16
#define ITH_LCD_HWC_POSY_MASK           (N11_BITS_MSK << ITH_LCD_HWC_POSY_BIT)

#define ITH_LCD_HWC_BASE_REG            0x0030
#define ITH_LCD_HWC_BASE_BIT            0
#define ITH_LCD_HWC_BASE_MASK           (0xFFFFFFF8 << ITH_LCD_HWC_BASE_BIT)

#define ITH_LCD_HWC_CR_REG              0x0034
#define ITH_LCD_HWC_ABLDEN_BIT          27
#define ITH_LCD_HWC_DEFDSTEN_BIT        29
#define ITH_LCD_HWC_DEFINVDST_BIT       28

#define ITH_LCD_HWC_INVCOLORWEI_REG     0x0034
#define ITH_LCD_HWC_INVCOLORWEI_BIT     16
#define ITH_LCD_HWC_INVCOLORWEI_MASK    (N08_BITS_MSK << ITH_LCD_HWC_INVCOLORWEI_BIT)

#define ITH_LCD_HWC_DEFCOLOR_REG        0x003C
#define ITH_LCD_HWC_DEFCOLOR_BIT        0
#define ITH_LCD_HWC_DEFCOLOR_MASK       (N16_BITS_MSK << ITH_LCD_HWC_DEFCOLOR_BIT)

#define ITH_LCD_HWC_FORECOLOR_REG       0x0038
#define ITH_LCD_HWC_FORECOLOR_BIT       0
#define ITH_LCD_HWC_FORECOLOR_MASK      (N16_BITS_MSK << ITH_LCD_HWC_FORECOLOR_BIT)

#define ITH_LCD_HWC_BACKCOLOR_REG       0x0038
#define ITH_LCD_HWC_BACKCOLOR_BIT       16
#define ITH_LCD_HWC_BACKCOLOR_MASK      (N16_BITS_MSK << ITH_LCD_HWC_BACKCOLOR_BIT)

#define ITH_LCD_HWC_FORECOLORWEI_REG    0x0034
#define ITH_LCD_HWC_FORECOLORWEI_BIT    0
#define ITH_LCD_HWC_FORECOLORWEI_MASK   (N08_BITS_MSK << ITH_LCD_HWC_FORECOLORWEI_BIT)

#define ITH_LCD_HWC_BACKCOLORWEI_REG    0x0034
#define ITH_LCD_HWC_BACKCOLORWEI_BIT    8
#define ITH_LCD_HWC_BACKCOLORWEI_MASK   (N08_BITS_MSK << ITH_LCD_HWC_BACKCOLORWEI_BIT)

#define ITH_LCD_HWC_FIFO_RQ_THD_REG     0x0034
#define ITH_LCD_HWC_FIFO_RQ_THD_BIT     24
#define ITH_LCD_HWC_FIFO_RQ_THD_MASK    (N03_BITS_MSK << ITH_LCD_HWC_FIFO_RQ_THD_BIT)

#define ITH_LCD_HWC_UPDATE_REG          0x0034
#define ITH_LCD_HWC_UPDATE_BIT          31

//#define ITH_LCD_UI_DECPRESS_EN_REG     0x1140
//#define ITH_LCD_UI_DECPRESS_EN_BIT     15
//#define ITH_LCD_UI_DEC_LINEBYTE_REG    0x1140
//#define ITH_LCD_UI_DEC_LINEBYTE_BIT    0
//#define ITH_LCD_UI_DEC_LINEBYTE_MASK  (N15_BITS_MSK << ITH_LCD_UI_DEC_LINEBYTE_BIT)
//
//#define ITH_LCD_UI_DEC_PITCH_REG       0x1142
//#define ITH_LCD_UI_DEC_PITCH_BIT       0
//#define ITH_LCD_UI_DEC_PITCH_MASK      (N15_BITS_MSK << ITH_LCD_UI_DEC_PITCH_BIT)
//
//#define ITH_LCD_UI_DEC_TBYTE_LO_REG    0x1144
//#define ITH_LCD_UI_DEC_TBYTE_HI_REG    0x1146
//
//#define ITH_LCD_UI_DEC_UPDATE_REG      0x1148
//#define ITH_LCD_UI_DEC_UPDATE_BIT      15

#define ITH_LCD_GAMMA_FUN_EN_REG        0x0040
#define ITH_LCD_GAMMA_FUN_EN_BIT        31

#define ITH_LCD_GAMMA_R_PTR_REG         0x0040
#define ITH_LCD_GAMMA_R_PTR_BIT         0
#define ITH_LCD_GAMMA_R_PTR_MASK        (N07_BITS_MSK << ITH_LCD_GAMMA_R_PTR_BIT)

#define ITH_LCD_GAMMA_R_VALUE_REG       0x0040
#define ITH_LCD_GAMMA_R_VALUE_BIT       16
#define ITH_LCD_GAMMA_R_VALUE_MASK      (N10_BITS_MSK << ITH_LCD_GAMMA_R_VALUE_BIT)

#define ITH_LCD_GAMMA_G_PTR_REG         0x0044
#define ITH_LCD_GAMMA_G_PTR_BIT         0
#define ITH_LCD_GAMMA_G_PTR_MASK        (N07_BITS_MSK << ITH_LCD_GAMMA_G_PTR_BIT)

#define ITH_LCD_GAMMA_G_VALUE_REG       0x0044
#define ITH_LCD_GAMMA_G_VALUE_BIT       16
#define ITH_LCD_GAMMA_G_VALUE_MASK      (N10_BITS_MSK << ITH_LCD_GAMMA_G_VALUE_BIT)

#define ITH_LCD_GAMMA_B_PTR_REG         0x0048
#define ITH_LCD_GAMMA_B_PTR_BIT         0
#define ITH_LCD_GAMMA_B_PTR_MASK        (N07_BITS_MSK << ITH_LCD_GAMMA_B_PTR_BIT)

#define ITH_LCD_GAMMA_B_VALUE_REG       0x0048
#define ITH_LCD_GAMMA_B_VALUE_BIT       16
#define ITH_LCD_GAMMA_B_VALUE_MASK      (N10_BITS_MSK << ITH_LCD_GAMMA_B_VALUE_BIT)

#define ITH_LCD_RGB2YUV_REG             0x0050
#define ITH_LCD_RGB2YUV_EN_BIT          0
#define ITH_LCD_YUV_PACKET_BIT          1
#define ITH_LCD_YUV_FORMAT_BIT          2
#define ITH_LCD_YUV_FORMAT_MASK         (N02_BITS_MSK << ITH_LCD_YUV_FORMAT_BIT)

#define ITH_LCD_RGB2YUV11_REG           0x0054
#define ITH_LCD_RGB2YUV11_BIT           0
#define ITH_LCD_RGB2YUV11_MASK          (N10_BITS_MSK << ITH_LCD_RGB2YUV11_BIT)

#define ITH_LCD_RGB2YUV12_REG           0x0054
#define ITH_LCD_RGB2YUV12_BIT           16
#define ITH_LCD_RGB2YUV12_MASK          (N10_BITS_MSK << ITH_LCD_RGB2YUV12_BIT)

#define ITH_LCD_RGB2YUV13_REG           0x0058
#define ITH_LCD_RGB2YUV13_BIT           0
#define ITH_LCD_RGB2YUV13_MASK          (N10_BITS_MSK << ITH_LCD_RGB2YUV13_BIT)

#define ITH_LCD_RGB2YUVC1_REG           0x0058
#define ITH_LCD_RGB2YUVC1_BIT           16
#define ITH_LCD_RGB2YUVC1_MASK          (N09_BITS_MSK << ITH_LCD_RGB2YUVC1_BIT)

#define ITH_LCD_RGB2YUV21_REG           0x005C
#define ITH_LCD_RGB2YUV21_BIT           0
#define ITH_LCD_RGB2YUV21_MASK          (N10_BITS_MSK << ITH_LCD_RGB2YUV21_BIT)

#define ITH_LCD_RGB2YUV22_REG           0x005C
#define ITH_LCD_RGB2YUV22_BIT           16
#define ITH_LCD_RGB2YUV22_MASK          (N10_BITS_MSK << ITH_LCD_RGB2YUV22_BIT)

#define ITH_LCD_RGB2YUV23_REG           0x0060
#define ITH_LCD_RGB2YUV23_BIT           0
#define ITH_LCD_RGB2YUV23_MASK          (N10_BITS_MSK << ITH_LCD_RGB2YUV23_BIT)

#define ITH_LCD_RGB2YUVC2_REG           0x0060
#define ITH_LCD_RGB2YUVC2_BIT           16
#define ITH_LCD_RGB2YUVC2_MASK          (N09_BITS_MSK << ITH_LCD_RGB2YUVC2_BIT)

#define ITH_LCD_RGB2YUV31_REG           0x0064
#define ITH_LCD_RGB2YUV31_BIT           0
#define ITH_LCD_RGB2YUV31_MASK          (N10_BITS_MSK << ITH_LCD_RGB2YUV31_BIT)

#define ITH_LCD_RGB2YUV32_REG           0x0064
#define ITH_LCD_RGB2YUV32_BIT           16
#define ITH_LCD_RGB2YUV32_MASK          (N10_BITS_MSK << ITH_LCD_RGB2YUV32_BIT)

#define ITH_LCD_RGB2YUV33_REG           0x0068
#define ITH_LCD_RGB2YUV33_BIT           0
#define ITH_LCD_RGB2YUV33_MASK          (N10_BITS_MSK << ITH_LCD_RGB2YUV33_BIT)

#define ITH_LCD_RGB2YUVC3_REG           0x0068
#define ITH_LCD_RGB2YUVC3_BIT           16
#define ITH_LCD_RGB2YUVC3_MASK          (N09_BITS_MSK << ITH_LCD_RGB2YUVC3_BIT)

#define ITH_LCD_TCON_CTG_REG            0x0070
#define ITH_LCD_CTG0_EN_BIT             0
#define ITH_LCD_CTG1_EN_BIT             1
#define ITH_LCD_CTG2_EN_BIT             2
#define ITH_LCD_CTG3_EN_BIT             3
#define ITH_LCD_CTG4_EN_BIT             4
#define ITH_LCD_CTG5_EN_BIT             5
#define ITH_LCD_CTG6_EN_BIT             6
#define ITH_LCD_CTG7_EN_BIT             7
#define ITH_LCD_CTG_RST_BIT             8

#define ITH_LCD_CTG_HTOTAL_REG          0x0074
#define ITH_LCD_CTG_HTOTAL_BIT          0
#define ITH_LCD_CTG_HTOTAL_MASK         (N12_BITS_MSK << ITH_LCD_CTG_HTOTAL_BIT)

#define ITH_LCD_CTG_VTOTAL_REG          0x0074
#define ITH_LCD_CTG_VTOTAL_BIT          16
#define ITH_LCD_CTG_VTOTAL_MASK         (N12_BITS_MSK << ITH_LCD_CTG_VTOTAL_BIT)
                                      
#define ITH_LCD_CTG0_SET_REG            0x0078 

#define ITH_LCD_CTG_P1_Y_REG            0x0078
#define ITH_LCD_CTG0_P2_ACT_REG         0x007C
#define ITH_LCD_CTG0_P2_Y_REG           0x007C
#define ITH_LCD_CTG0_P3_ACT_REG         0x0080
#define ITH_LCD_CTG0_P3_Y_REG           0x0080
#define ITH_LCD_CTG0_P4_ACT_REG         0x0084
#define ITH_LCD_CTG0_P4_Y_REG           0x0084

#define ITH_LCD_CTG1_SET_REG            0x0088

#define ITH_LCD_CTG1_P1_Y_REG           0x0088
#define ITH_LCD_CTG1_P2_ACT_REG         0x008C
#define ITH_LCD_CTG1_P2_Y_REG           0x008C
#define ITH_LCD_CTG1_P3_ACT_REG         0x0090
#define ITH_LCD_CTG1_P3_Y_REG           0x0090
#define ITH_LCD_CTG1_P4_ACT_REG         0x0094
#define ITH_LCD_CTG1_P4_Y_REG           0x0094

#define ITH_LCD_CTG2_SET_REG            0x0098

#define ITH_LCD_CTG2_P1_Y_REG           0x0098
#define ITH_LCD_CTG2_P2_ACT_REG         0x009C
#define ITH_LCD_CTG2_P2_Y_REG           0x009C
#define ITH_LCD_CTG2_P3_ACT_REG         0x00A0
#define ITH_LCD_CTG2_P3_Y_REG           0x00A0
#define ITH_LCD_CTG2_P4_ACT_REG         0x00A4
#define ITH_LCD_CTG2_P4_Y_REG           0x00A4

#define ITH_LCD_CTG3_SET_REG            0x00A8

#define ITH_LCD_CTG3_P1_Y_REG           0x00A8
#define ITH_LCD_CTG3_P2_ACT_REG         0x00AC
#define ITH_LCD_CTG3_P2_Y_REG           0x00AC
#define ITH_LCD_CTG3_P3_ACT_REG         0x00B0
#define ITH_LCD_CTG3_P3_Y_REG           0x00B0
#define ITH_LCD_CTG3_P4_ACT_REG         0x00B4
#define ITH_LCD_CTG3_P4_Y_REG           0x00B4

#define ITH_LCD_CTGH_CNT_REG            0x0168
#define ITH_LCD_CTGH_CNT_BIT            0
#define ITH_LCD_CTGH_CNT_MASK           (N12_BITS_MSK << ITH_LCD_CTGH_CNT_BIT)

#define ITH_LCD_CTGV_CNT_BIT            16
#define ITH_LCD_CTGV_CNT_MASK           (N12_BITS_MSK << ITH_LCD_CTGV_CNT_BIT)

#define ITH_LCD_READ_STATUS1_REG        0x017C
#define ITH_LCD_FLIP_NUM_BIT            0
#define ITH_LCD_FLIP_NUM_MASK           (N02_BITS_MSK << ITH_LCD_FLIP_NUM_BIT)

#define ITH_LCD_X_CNT_REG               0x016C
#define ITH_LCD_X_CNT_BIT               0
#define ITH_LCD_X_CNT_MASK              (N12_BITS_MSK << ITH_LCD_X_CNT_BIT)

#define ITH_LCD_CRC_CHECK               0x0180

#define ITH_LCD_INT_CLR_REG             0x004C
#define ITH_LCD_INT_CLR_BIT             31
#define ITH_LCD_INT_CLR_MASK            (N01_BITS_MSK << ITH_LCD_INT_CLR_BIT)

#define ITH_LCD_INT_CTRL_REG            0x004C
#define ITH_LCD_INT_EN_BIT              16
#define ITH_LCD_INT_FIELDMODE1_BIT      17

#define ITH_LCD_INT_LINE1_REG           0x004C
#define ITH_LCD_INT_LINE1_BIT           0
#define ITH_LCD_INT_LINE1_MASK          (N12_BITS_MSK << ITH_LCD_INT_LINE1_BIT)

#define ITH_LCD_IO_EN_REG               0x0110
#define ITH_LCD_IO_EN_BIT               2

#define ITH_LCD_LVDS_SET1_REG           0x01C0

#define ITH_LCD_MIPI_SET1_REG           0x0230

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define IT6122     1
#define IT6151     0
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//for IT6122 defines
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef IT6122

#define MIPI_EVENT_MODE		(1)
#define	MIPI_HSYNC_W		(8)
#define MIPI_VSYNC_W		(2)

#define MPRxDevAddr     (0x6C << 0) //0xD8

#define RGB_24b         (0x3E)
#define RGB_30b         (0x0D)
#define RGB_36b         (0x1D)
#define RGB_18b         (0x1E)
#define RGB_18b_L       (0x2E)
#define YCbCr_16b       (0x2C)
#define YCbCr_20b       (0x0C)
#define YCbCr_24b       (0x1C)

#ifndef MPVidType
#define MPVidType		RGB_24b
#endif

#define MPLaneNum		3
#define EnPNSwap		0
#define EnLaneSwap		0

#ifndef MPMode
#define MPMode			2	//1:Bypass mode 2:Normal mode
#endif
//system control
#define EnMPx1PCLK  	0   // FALSE: 3/4(for 4 Lane) , 3/2(for 2 Lane) , 3(for 1 Lane) PCLK
#define InvMCLK  		1 	//FALSE for solomon, if NonUFO, MCLK max = 140MHz with InvMCLK=TRUE
#define InvPCLK  		0

// PPS option
#if(MPMode == 1)
#define  EnMBPM  		1   // BYPASS Mode
#define	 PREC_Update  	1	// enable P-timing update
#define  MREC_Update  	1
#else 
#define  EnMBPM  		0   // NORMAL Mode
#define	 PREC_Update  	0	
#define  MREC_Update  	0	
#endif

#define EnHReSync  		0
#define EnVReSync  		0
#define EnFReSync  		0

// PPI option
#define HSSetNum  		2   //5
#define SkipStg  		2   //5

#if( MPVidType == RGB_18b )	
#if( MPLaneNum == 3 )
#if( EnMPx1PCLK )
#define MPPCLKSel 0x02  //(1) 4-lane : MCLK = 1/1 PCLK
#else
#define MPPCLKSel 0x02  //(1) 4-lane : MCLK = 1/1 PCLK
#endif	
# elif ( MPLaneNum == 1 ) 				
#if( EnMPx1PCLK )
#define MPPCLKSel 0x05  //(6) 2-lane : MCLK = 1/1 PCLK
#else
#define MPPCLKSel 0x05  //(6) 2-lane : MCLK = 1/1 PCLK
#endif	
# elif( MPLaneNum == 0 ) 
#if( EnMPx1PCLK )
#define MPPCLKSel 0x0b  //(7) 1-lane : MCLK = 1/1 PCLK
#else
#define MPPCLKSel 0x08  //(8) 1-lane : MCLK = 3/4 PCLK
#endif
#else 	
#define MPPCLKSel 0x03	//(0)
#endif

#else
#if( MPLaneNum == 3 )
#if( EnMPx1PCLK )
#define MPPCLKSel 0x03  //(0) 4-lane : MCLK = 1/1 PCLK
#else
#define MPPCLKSel 0x02  //(1) 4-lane : MCLK = 3/4 PCLK
#endif
# elif( MPLaneNum == 1 ) 
#if( EnMPx1PCLK )
#define MPPCLKSel 0x07  //(2) 2-lane : MCLK = 1/1 PCLK
#else
#define MPPCLKSel 0x05  //(3) 2-lane : MCLK = 3/4 PCLK
#endif
# elif( MPLaneNum == 0 ) 
#if( EnMPx1PCLK )
#define MPPCLKSel 0x0F  //(4) 1-lane : MCLK = 1/1 PCLK
#else
#define MPPCLKSel 0x0B  //(5) 1-lane : MCLK = 3/4 PCLK
#endif
#else 	
#define MPPCLKSel 0x03	//(0)			
#endif

#endif

// for LVDS
#define MAPVESA 		true  // (default = FALSE), FALSE:JEIDA(MAP1), TRUE:VESA(MAP3)
#define En6bitout  		false // (default = FALSE)
#define EnLVDMode  		true  //LVDSTx Dual out enable

//LVDS Tx SSC Setting
#define EnLVTxSSC  			false
#define LVSDM 	 			0x3333 //0x147b; 10000ppm    //0x3333; 25000ppm   //0x0a3d; 5000ppm  //0x3333; 25000ppm/162MHz 
#define LVSDMINC 			0x106  //0x0x009;              //0x016;             //0x005;           //0x014;
#define EnSP_S1				true
#define EnSSCBufConcat  	false  //enable sscbufB used when LVDS single Link only
#define EnSSCBufAutoRst 	true
#define EnSSCPLL 			false

#define EnLVVidRecInt 		true   //default: TRUE

#endif //IT6122

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// for it6151 defines start
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef IT6151

#define PANEL_RESOLUTION_1920x1080p60_4LANE_24B

#define MIPI_4_LANE		(3)
#define MIPI_3_LANE		(2)
#define MIPI_2_LANE		(1)
#define MIPI_1_LANE		(0)

// MIPI Packed Pixel Stream
#define RGB_24b         (0x3E)
#define RGB_30b         (0x0D)
#define RGB_36b         (0x1D)
#define RGB_18b_P       (0x1E)
#define RGB_18b_L       (0x2E)
#define YCbCr_16b       (0x2C)
#define YCbCr_20b       (0x0C)
#define YCbCr_24b       (0x1C)

// DPTX reg62[3:0]
#define B_DPTXIN_6Bpp   (0)
#define B_DPTXIN_8Bpp   (1)
#define B_DPTXIN_10Bpp  (2)
#define B_DPTXIN_12Bpp  (3)

#define B_AutoBR		(0x00)
#define B_LBR    		(0x01)
#define B_HBR    		(0x80)

#define DP_4_LANE 		(3)
#define DP_2_LANE 		(1)
#define DP_1_LANE 		(0)

#define B_SSC_ENABLE   	(1)
#define B_SSC_DISABLE   (0)

#define H_Neg			(0)
#define H_Pos			(1)

#define V_Neg			(0)
#define V_Pos			(1)

#define En_UFO			(1)
#define H_ReSync		(0x01)
#define V_ReSync		(0x02)
#define F_ReSync		(0x10)

// CONFIGURE
#define DPTX_SSC_SETTING	(B_SSC_ENABLE)//(B_SSC_DISABLE)
#define MP_MCLK_INV			(1)
#define MP_CONTINUOUS_CLK	(1)
#define MP_LANE_DESKEW		(1)
#define MP_LANE_SWAP		(0)
#define MP_PN_SWAP			(0)

#define DP_PN_SWAP			(0)
#define DP_AUX_PN_SWAP		(0)
#define DP_LANE_SWAP		(0)
#define DP_BPP				(B_DPTXIN_8Bpp)

#define LVDS_LANE_SWAP		(0)
#define LVDS_PN_SWAP		(0)
#define LVDS_DC_BALANCE		(0)

#define LVDS_6BIT			(0) // '0' for 8 bit, '1' for 6 bit
#define VESA_MAP		    (1) // '0' for JEIDA , '1' for VESA MAP

#define INT_MASK			(3)
#define MIPI_RECOVER		(1)

#define MIPI_EVENT_MODE		(1)
#define	MIPI_HSYNC_W		(8)
#define MIPI_VSYNC_W		(2)

#define MIPI_FFRdStg		(0x10)

#define REDUCE_SWING		(0)

#if (REDUCE_SWING == 0)
#define REG_C2				(0x47)
#else
#define REG_C2				(0x41)
#endif

#define AUTO_OUTPUT			(0)
#define TIMER_CNT			(0x0A)

#define DP_I2C_ADDR 	(0x5C << 0) //0xB8
#define MIPI_I2C_ADDR 	(0x6C << 0) //0xD8

struct PanelInfoStr{
    unsigned char	ucVic;
    unsigned short	usPWidth;
    unsigned char	ucDpLanes;
    unsigned char	ucDpBR;
    unsigned char	ucMpLanes;
    unsigned char	ucMpHPol;
    unsigned char	ucMpVPol;
    unsigned char	ucUFO;
    unsigned char	ucMpFmt;
    unsigned char	ucDpReSync;
    unsigned char	ucMpReSync;
    unsigned char	ucMpClkDiv;
    unsigned char	ucHighPclk;
    unsigned char	ucIntMask;
    unsigned char	ucHSSetNum;
};

#if defined PANEL_RESOLUTION_1280x800_4LANE_24B
struct PanelInfoStr sPInfo = { 0, 1280, DP_2_LANE, B_LBR, MIPI_4_LANE, H_Neg, V_Pos, 0, RGB_24b, H_ReSync, F_ReSync, 2, 0, 0, 4 };
#elif defined PANEL_RESOLUTION_1920x1080p60_4LANE_24B
struct PanelInfoStr sPInfo = { 16, 1920, DP_2_LANE, B_HBR, MIPI_4_LANE, H_Pos, V_Pos, 0, RGB_24b, H_ReSync, F_ReSync, 2, 1, 0, 2 };
#elif defined PANEL_RESOLUTION_1920x1080p60_4LANE_18B
struct PanelInfoStr sPInfo = { 16, 1920, DP_2_LANE, B_LBR, MIPI_4_LANE, H_Pos, V_Pos, 0, RGB_18b_L, H_ReSync, F_ReSync, 2, 1, 0, 4 };
#elif defined PANEL_RESOLUTION_1920x1200_4LANE_24B
struct PanelInfoStr sPInfo = { 0, 1920, DP_2_LANE, B_HBR, MIPI_4_LANE, H_Pos, V_Neg, 0, RGB_24b, H_ReSync, F_ReSync, 2, 1, 0, 2 };
#elif defined PANEL_RESOLUTION_2048x1536_4LANE_24B_UFO
struct PanelInfoStr sPInfo = { 0, 2048, DP_4_LANE, B_LBR, MIPI_4_LANE, H_Neg, V_Pos, En_UFO, RGB_24b, 0, 0, 2, 1, 0, 4 };
#elif defined PANEL_RESOLUTION_2048x1536_4LANE_24B
struct PanelInfoStr sPInfo = { 0, 2048, DP_4_LANE, B_LBR, MIPI_4_LANE, H_Neg, V_Pos, 0, RGB_24b, H_ReSync, F_ReSync, 2, 1, 0, 2 };
#elif defined PANEL_RESOLUTION_2048x1536_4LANE_18B
struct PanelInfoStr sPInfo = { 0, 2048, DP_4_LANE, B_LBR, MIPI_4_LANE, H_Neg, V_Pos, 0, RGB_18b_L, H_ReSync, F_ReSync, 2, 1, 0, 4 };
#elif defined PANEL_RESULUTION_1536x2048_4LANE_24B
struct PanelInfoStr sPInfo = { 0, 1536, DP_4_LANE, B_LBR, MIPI_4_LANE, H_Pos, V_Pos, 0, RGB_24b, H_ReSync, F_ReSync, 2, 1, 0, 2 };
#elif defined PANEL_RESULUTION_1536x2048_4LANE_24B_UFO
struct PanelInfoStr sPInfo = { 0, 1536, DP_4_LANE, B_LBR, MIPI_4_LANE, H_Pos, V_Pos, En_UFO, RGB_24b, 0, 0, 2, 1, 0, 4 };
#elif defined PANEL_RESOLUTION_1366x768_4LANE_24B
//	struct PanelInfoStr sPInfo = {  0, 1366, DP_2_LANE, B_LBR, MIPI_4_LANE, H_Neg, V_Neg, 0, RGB_24b, H_ReSync, 0, 3, 0, 0x11};
struct PanelInfoStr sPInfo = { 0, 1366, DP_2_LANE, B_LBR, MIPI_4_LANE, H_Neg, V_Neg, 0, RGB_24b, H_ReSync, F_ReSync, 2, 0, 0, 4 };
#elif defined PANEL_RESOLUTION_1366x768_2LANE_24B
struct PanelInfoStr sPInfo = { 0, 1366, DP_2_LANE, B_LBR, MIPI_2_LANE, H_Neg, V_Neg, 0, RGB_24b, H_ReSync, F_ReSync, 5, 0, 0, 4 };
#elif defined PANEL_RESOLUTION_1368x768_4LANE_24B
struct PanelInfoStr sPInfo = { 0, 1368, DP_2_LANE, B_LBR, MIPI_4_LANE, H_Neg, V_Neg, 0, RGB_24b, H_ReSync, F_ReSync, 2, 0, 0, 4 };
#elif defined PANEL_RESOLUTION_1368x768_4LANE_18B_1DP
struct PanelInfoStr sPInfo = { 0, 1368, DP_1_LANE, B_HBR, MIPI_4_LANE, H_Neg, V_Neg, 0, RGB_18b_L, H_ReSync, F_ReSync, 2, 0, 0, 4 };
#elif defined PANEL_RESOLUTION_1280x720_4LANE_24B
struct PanelInfoStr sPInfo = { 0, 1280, DP_2_LANE, B_LBR, MIPI_4_LANE, H_Neg, V_Neg, 0, RGB_24b, H_ReSync, F_ReSync, 2, 0, 0, 4 };
#endif

#endif //IT6151

/** @} */ // end of ith_lcd

#ifdef __cplusplus
}
#endif

#endif
