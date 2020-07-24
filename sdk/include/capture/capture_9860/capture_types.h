#ifndef __CAP_TYPES_H__
#define __CAP_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ite/ith.h"
#include "ite/mmp_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

#define CAP_SCALE_TAP           4
#define CAP_SCALE_TAP_SIZE      8

#define CAP_IN_YUYV         0
#define CAP_IN_YVYU         1
#define CAP_IN_UYVY         2
#define CAP_IN_VYUY         3


//=============================================================================
//                Macro Definition
//=============================================================================
/**
 *  Debug message
 */
typedef enum _CAP_MSG_TYPE
{
    CAP_MSG_TYPE_ERR = (0x1 << 0),
} CAP_MSG_TYPE;

#ifdef _MSC_VER // WIN32
    #ifndef trac
        #define trac(string, ...)               do { printf(string, __VA_ARGS__); \
                                                     printf("  %s [#%d]\n", __FILE__, __LINE__); \
} while (0)
    #endif

    #define cap_msg(type, string, ...)          ((void)((type) ? printf(string, __VA_ARGS__) : MMP_NULL))
    #define cap_msg_ex(type, string, ...)       do { if (type) { \
                                                            printf( string,         __VA_ARGS__); \
                                                            printf( "  %s [#%d]\n", __FILE__, __LINE__); } \
} while (0)

#else
    #ifndef trac
        #define trac(string, args ...)          do { printf(string, ## args); \
                                                     printf("  %s [#%d]\n", __FILE__, __LINE__); \
} while (0)
    #endif

    #define cap_msg(type, string, args ...)     ((void)((type) ? printf(string, ## args) : MMP_NULL))
    #define cap_msg_ex(type, string, args ...)  do { if (type) { \
                                                            printf( string,         ## args); \
                                                            printf( "  %s [#%d]\n", __FILE__, __LINE__); } \
} while (0)
#endif

//=============================================================================
//                Type Definition
//=============================================================================

typedef enum CAP_INPUT_VIDEO_FORMAT_TAG
{
    Progressive  = 0,
    Interleaving = 1,
} CAP_INPUT_VIDEO_FORMAT;

typedef enum CAP_INPUT_COLORDEPTH_TAG
{
    COLOR_DEPTH_8_BITS  = 0,
    COLOR_DEPTH_10_BITS = 1,
    COLOR_DEPTH_12_BITS = 2
} CAP_INPUT_COLORDEPTH;

typedef enum CAP_INPUT_YUV_DATA_FORMAT_TAG
{
    YUV422  = 0,
    YUV444  = 1,
    RGB888  = 2,
} CAP_INPUT_YUV_DATA_FORMAT;

typedef enum CAP_INPUT_VIDEO_SYNC_MODE_TAG
{
    BT_601  = 0,
    BT_656  = 1
} CAP_INPUT_VIDEO_SYNC_MODE;

typedef enum CAP_ISP_HANDSHAKING_MODE_TAG
{
    MEMORY_MODE,
    ONFLY_MODE,
    MEMORY_WITH_ONFLY_MODE
} CAP_ISP_HANDSHAKING_MODE;

typedef enum CAP_MEM_BUF_TAG
{
    CAP_MEM_Y0              = 0,
    CAP_MEM_UV0             = 1,
    CAP_MEM_Y1              = 2,
    CAP_MEM_UV1             = 3,
    CAP_MEM_Y2              = 4,
    CAP_MEM_UV2             = 5,
    CAP_MEM_Y3              = 6,
    CAP_MEM_UV3             = 7,
    CAP_MEM_Y4              = 8,
    CAP_MEM_UV4             = 9, 
} CAP_MEM_BUF;

typedef enum CAP_LANE_STATUS_TAG
{
    CAP_LANE0_STATUS,
} CAP_LANE_STATUS;

typedef enum CAP_INPUT_NV12FORMAT_TAG
{
    UV  = 0,
    VU  = 1
} CAP_INPUT_NV12FORMAT;

typedef enum CAP_INPUT_DITHER_MODE_TAG
{
    DITHER_OL   = 0,
    DITHER_1L   = 1,
    DITHER_2L   = 2,
    DITHER_3L   = 3
} CAP_INPUT_DITHER_MODE;

typedef enum CAP_INPUT_DATA_WIDTH_TAG
{
    PIN_24_30_36BITS    = 0,
    PIN_16_20_24BITS    = 1,
    PIN_8_10_12BITS     = 2
} CAP_INPUT_DATA_WIDTH;

typedef enum CAP_OUTPUT_MEMORY_FOTMAT_TAG
{
    SEMI_PLANAR_420 = 0,
    SEMI_PLANAR_422 = 1,
    PACKET_MODE_422 = 2
} CAP_OUTPUT_MEMORY_FOTMAT;

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct CAP_INPUT_CLK_INFO_TAG
{
    MMP_BOOL    EnUCLK;
    MMP_UINT8   UCLKRatio;
    MMP_UINT8   UCLKDly;
    MMP_BOOL    UCLKInv;
    MMP_UINT8   UCLKSrc;
    MMP_UINT8   UCLKPINNUM;
} CAP_INPUT_CLK_INFO;

typedef struct CAP_INPUT_PIN_SELECT_TAG
{
    MMP_UINT8   Y_Pin_Num[12];
    MMP_UINT8   U_Pin_Num[12];
    MMP_UINT8   V_Pin_Num[12];
} CAP_INPUT_PIN_SELECT;

typedef struct CAP_INPUT_DITHER_INFO_TAG
{
    MMP_BOOL                EnDither;
    CAP_INPUT_DITHER_MODE   DitherMode;
} CAP_INPUT_DITHER_INFO;

/* Input Format Info */
typedef struct CAP_INPUT_INFO_TAG
{
    MMP_BOOL                    VSyncPol;       /* '0' 0 stands for blanking '1' 1 stands for blanking */
    MMP_BOOL                    HSyncPol;
    MMP_UINT16                  VSyncSkip;
    MMP_UINT16                  HSyncSkip;
    MMP_BOOL                    HSnapV;
    MMP_BOOL                    CheckHS;        /* begin capture after HSYNC stable */
    MMP_BOOL                    CheckVS;        /* begin capture after VSYNC stable */
    MMP_BOOL                    CheckDE;        /* begin capture after DE stable */
    
    MMP_UINT16                  WrMergeThresld; /* threshold to begin mem write */  /*range:(0-31)*/
    CAP_INPUT_NV12FORMAT        NV12Format;
    MMP_UINT32                  MemUpBound;     /* up bound of address limit */
    MMP_UINT32                  MemLoBound;     /* lower bound of address limit */

    CAP_INPUT_VIDEO_SYNC_MODE   EmbeddedSync;
    CAP_INPUT_VIDEO_FORMAT      Interleave;     /* 0: Progressive , 1 :interleaving */

    MMP_UINT16                  PitchY;
    MMP_UINT16                  PitchUV;
    MMP_UINT16                  framerate;

    /* Active Region  Info */
    MMP_UINT16                  capwidth;
    MMP_UINT16                  capheight;
    /*For no Data enable Use */
    MMP_UINT16                  HNum1;      /* Input HSync active area start numner [12:0] */
    MMP_UINT16                  LineNum1;   /* Input active area start line number[11:0] of top field */
    MMP_UINT16                  LineNum2;   /* Input active area line number[11:0] of top field */
    MMP_UINT16                  LineNum3;   /* Input active area start line number[11:0] of bottom field */
    MMP_UINT16                  LineNum4;   /* Input active area line number[11:0] of bottom field */
    /* ROI Info */
    MMP_UINT16                  ROIPosX;
    MMP_UINT16                  ROIPosY;
    MMP_UINT16                  ROIWidth;
    MMP_UINT16                  ROIHeight;
} CAP_INPUT_INFO;

typedef struct CAP_ENFUN_INFO_TAG
{
    MMP_BOOL    EnDEMode;
    MMP_BOOL    EnCSFun;
    MMP_BOOL    EnInBT656;

    MMP_BOOL    EnHSync;
    MMP_BOOL    EnAutoDetHSPol;
    MMP_BOOL    EnAutoDetVSPol;
    MMP_BOOL    EnDumpMode;
    MMP_BOOL    EnMemContinousDump;
    MMP_BOOL    EnSramNap;
    MMP_BOOL    EnMemLimit;
	
    MMP_BOOL    EnProgressiveToField;
    MMP_BOOL    EnCrossLineDE;
    MMP_BOOL    EnYPbPrTopVSMode;
    MMP_BOOL    EnDlyVS;
    MMP_BOOL    EnHSPosEdge;
    MMP_BOOL    EnPort1UV2LineDS;
} CAP_ENFUN_INFO;

typedef struct CAP_YUV_INFO_TAG
{
    MMP_UINT8                   ColorOrder; /* 0:YUYV 1:YVYU 2: UYVY 3 :VYUY */
    CAP_INPUT_COLORDEPTH        ColorDepth; /* 0: 8-bit 1: 10 -bit 2 :12-bit */
    CAP_INPUT_DATA_WIDTH        InputWidth; 
    CAP_INPUT_YUV_DATA_FORMAT   InputMode;  /* ColorFormat 0:YUV422 1:YUV444 2:RGB888 */
} CAP_YUV_INFO;

/* Pin I/O  Related Define */
typedef struct CAP_INPUT_MUX_INFO_TAG
{
    MMP_UINT8   Y_Pin_Num[12];
    MMP_UINT8   U_Pin_Num[12];
    MMP_UINT8   V_Pin_Num[12];

    //CAP_INPUT_CLK_INFO
    MMP_BOOL    EnUCLK;       /* enable capture clock */
    MMP_UINT8   UCLKRatio;    /* reserved */
    MMP_UINT8   UCLKDly;      /* use uclk to sample input video signal with delay. */
    MMP_BOOL    UCLKInv;      /* set 1 to sample input video signal with inverted uclk */
    MMP_UINT8   UCLKSrc;      /* uclk source :  1:external IO 2: internal colorbar 3: internal LCD */

    MMP_UINT8   UCLKPINNUM;   /* VD number of input clock */
    MMP_BOOL    UCLKAutoDlyDir; /* 0: bi-direction 1: uni-direction*/
    MMP_BOOL    UCLKAutoDlyEn;/* enable auto delay for clock */
    MMP_UINT8   HS_Pin_Num;   /* VD number of HSYNC */
    MMP_UINT8   VS_Pin_Num;   /* VD number of VSYNC */
    MMP_UINT8   DE_Pin_Num;   /* VD number of DE */
} CAP_INPUT_MUX_INFO;

typedef struct CAP_IO_MODE_INFO_TAG
{
    MMP_UINT32  CAPIOFFEn_VD_00_31;
    MMP_UINT32  CAPIOFFEn_VD_35_32;

} CAP_IO_MODE_INFO;

typedef struct CAP_OUTPUT_INFO_TAG
{
    MMP_UINT16                  OutWidth;
    MMP_UINT16                  OutHeight;
    MMP_UINT32                  OutAddrOffset;
    CAP_OUTPUT_MEMORY_FOTMAT    OutMemFormat;

} CAP_OUTPUT_INFO;

typedef struct CAP_COLOR_BAR_CONFIG_TAG
{
    MMP_BOOL    Enable_colorbar;
    MMP_BOOL    Vsync_pol;
    MMP_BOOL    Hsync_pol;
    MMP_UINT16  VS_act_start_line;
    MMP_UINT16  VS_act_line;
    MMP_UINT16  act_line;
    MMP_UINT16  blank_line1;
    MMP_UINT16  blank_line2;
    MMP_UINT16  Hs_act;
    MMP_UINT16  blank_pix1;
    MMP_UINT16  blank_pix2;
    MMP_UINT16  act_pix;
} CAP_COLOR_BAR_CONFIG;

typedef struct CAP_HORSCALE_INFO_TAG
{
    MMP_UINT32  HorScaleHCI;
    MMP_UINT16  HorScaleWidth;

    MMP_UINT8   HorScaleWX_00;
    MMP_UINT8   HorScaleWX_01;
    MMP_UINT8   HorScaleWX_02;
    MMP_UINT8   HorScaleWX_03;

    MMP_UINT8   HorScaleWX_10;
    MMP_UINT8   HorScaleWX_11;
    MMP_UINT8   HorScaleWX_12;
    MMP_UINT8   HorScaleWX_13;

    MMP_UINT8   HorScaleWX_20;
    MMP_UINT8   HorScaleWX_21;
    MMP_UINT8   HorScaleWX_22;
    MMP_UINT8   HorScaleWX_23;

    MMP_UINT8   HorScaleWX_30;
    MMP_UINT8   HorScaleWX_31;
    MMP_UINT8   HorScaleWX_32;
    MMP_UINT8   HorScaleWX_33;

    MMP_UINT8   HorScaleWX_40;
    MMP_UINT8   HorScaleWX_41;
    MMP_UINT8   HorScaleWX_42;
    MMP_UINT8   HorScaleWX_43;
} CAP_HORSCALE_INFO;

//Transfer Matrix RGB to YUV
typedef struct CAP_RGB_TO_YUV_TAG
{
    MMP_UINT16  _11;
    MMP_UINT16  _12;
    MMP_UINT16  _13;
    MMP_UINT16  _21;
    MMP_UINT16  _22;
    MMP_UINT16  _23;
    MMP_UINT16  _31;
    MMP_UINT16  _32;
    MMP_UINT16  _33;
    MMP_UINT16  ConstY;
    MMP_UINT16  ConstU;
    MMP_UINT16  ConstV;
} CAP_RGB_TO_YUV;

/* Capture Scale Function */
typedef struct CAP_SCALE_CTRL_TAG
{
    MMP_FLOAT   HCI;
    MMP_FLOAT   WeightMatX[CAP_SCALE_TAP_SIZE][CAP_SCALE_TAP];
} CAP_SCALE_CTRL;

typedef struct CAP_CONTEXT_TAG
{

    // OnflyMode option 1: onfly mode 0: memory mode
    MMP_BOOL            EnableOnflyMode;
    
    // Interrupt option 1: on, 0: off
    MMP_BOOL            EnableInterrupt;
    
    // Skip fun setting
    MMP_UINT16          Skippattern;
    MMP_UINT16          SkipPeriod;
    
    // Resolution match flag
    MMP_BOOL            bMatchResolution;
    
    // Output Memory header
    MMP_UINT8           *video_sys_addr;
    MMP_UINT32          video_vram_addr;
    
    // Output Memory address
    MMP_UINT32          OutAddrY[5];
    MMP_UINT32          OutAddrUV[5];   

    // Input format Info
    CAP_INPUT_INFO          ininfo;

    // Input data format Info
    CAP_YUV_INFO            YUVinfo;

    // Input data format Info
    CAP_INPUT_DITHER_INFO   Ditherinfo;

    // I/O pin Info
    CAP_INPUT_MUX_INFO      inmux_info;
    
    // I/O FIFO mode
    CAP_IO_MODE_INFO        iomode_info;
    
    // Output info.
    CAP_OUTPUT_INFO         outinfo;

    // Enable function
    CAP_ENFUN_INFO          funen;

    // Scale Fun.
    CAP_SCALE_CTRL          ScaleFun;

    // RGB to YUV Fun.
    CAP_RGB_TO_YUV          RGBtoYUVFun;

} CAP_CONTEXT;

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif