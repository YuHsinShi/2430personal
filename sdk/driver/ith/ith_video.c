/*
 * Copyright (c) 2014 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL Video functions.
 *
 * @author I-Chun Lai
 * @version 1.0
 */
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include "ith_cfg.h"
#include "ite/itp.h"
#include "ith/ith_video.h"
#include "ith_vregs.h"

#ifdef _WIN32
    #define asm(s)
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
enum
{
    Y = 0,
    U,
    V,
    YUV
};

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
static uint8_t  VideoTilingTable[5][32] =
{ {  0,  1,  2,  9, 10, 11, 12,  3,  4,  5, 13,  6, 14,  7,  8, 15,     // pitch = 512
     16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
  {  0,  1,  2, 10, 11, 12, 13,  3,  4,  5, 14,  6, 15,  7,  8,  9,     // pitch = 1024
     16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
  {  0,  1,  2, 11, 12, 13, 14,  3,  4,  5, 15,  6, 16,  7,  8,  9,     // pitch = 2048
     10, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
  {  0,  1,  2, 12, 13, 14, 15,  3,  4,  5, 16,  6,  7,  8,  9,  10,    // pitch = 4096
     11, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
  //{  0,  1,  2, 12, 13, 14, 15,  3,  4,  5, 16,  6, 17,  7,  8,  9,  // pitch = 4096
  //  10, 11, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
  {  0,  1,  2, 13, 14, 15, 16,  3,  4,  5, 17,  6, 18,  7,  8,  9,  // pitch = 8096
     10, 11, 12, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 }};

static uint32_t TILING_MODE_OFF, TILING_BLOCK, FRAME_BUFFER_COUNT;
static uint32_t VIDEO_MAX_WIDTH, VIDEO_MAX_HEIGHT, VIDEO_BUFFER_PITCH, VIDEO_BUFFER_HEIGHT, Y_BUFFER_OFFSET, U_BUFFER_OFFSET, V_BUFFER_OFFSET;
static uint32_t BLANK_BUFFER_HEIGHT, BLANK_U_BUFFER_OFFSET, BLANK_V_BUFFER_OFFSET, DB_BUFFER_HEIGHT, MV_BUFFER_HEIGHT, TC_BUFFER_HEIGHT;
static uint32_t VLD_BUFFER_HEIGHT, COL_BUFFER_HEIGHT, CMD_DATA_BUFFER_COUNT, CMD_DATA_BUFFER_HEIGHT, MEMORY_POOL_HEIGHT, ALIGNMENT_BIT, REMAP_TAB_SEL;

static uint32_t gp_video_buf_vram_addr  = 0;
static uint8_t  *gp_video_buf_sys_addr  = NULL;
static uint8_t  *gp_blank_buf_adr[3];

//=============================================================================
//                              Private Function Declaration
//=============================================================================
static void
_HARDWARE_SetDecodedVideoBufAddr(
    uint32_t    buf_index,
    uint8_t     *y_sys_addr,
    uint8_t     *u_sys_addr,
    uint8_t     *v_sys_addr);

static uint32_t
_HARDWARE_GetDecodedVideoBufVramAddr(
    uint32_t    buf_index,
    uint32_t    yuv_index);

static void
_HARDWARE_SetMVBufAddr(
    uint8_t *mv_buf_sys_addr);

static void
_HARDWARE_SetTCBufAddr(
    uint8_t *tc_buf_sys_addr);

static void
_HARDWARE_SetVLDBufAddr(
    uint8_t *vld_buf_sys_addr);

static void
_HARDWARE_SetDBBufAddr(
    uint8_t *db_buf_sys_addr);

static void
_HARDWARE_TilingReMapTable(
    void);

static void
_HARDWARE_SetCmdDataBufAddr(
    uint8_t *cmd_data_buf_sys_addr);

static void
_Clear_Blank_Buffer(
    uint8_t *py,
    uint8_t *pu,
    uint8_t *pv);

static int
ithVideoBufAlloc(
    uint32_t size);

//=============================================================================
//                              Public Function Definition
//=============================================================================
void
ithVideoEnableClock(
    void)
{
    // enable clock
    ithSetRegBitA(  ITH_HOST_BASE + ITH_VIDEO_CLK1_REG, ITH_EN_W23CLK_BIT);
    ithSetRegBitA(  ITH_HOST_BASE + ITH_VIDEO_CLK1_REG, ITH_EN_A3CLK_BIT);
    ithSetRegBitA(  ITH_HOST_BASE + ITH_VIDEO_CLK1_REG, ITH_EN_N3CLK_BIT);
    ithSetRegBitA(  ITH_HOST_BASE + ITH_VIDEO_CLK1_REG, ITH_EN_M5CLK_BIT);
    ithSetRegBitA(  ITH_HOST_BASE + ITH_VIDEO_CLK1_REG, ITH_EN_XCLK_BIT);
    ithSetRegBitA(  ITH_HOST_BASE + ITH_VIDEO_CLK1_REG, ITH_EN_DIV_XCLK_BIT);
}

void
ithVideoDisableClock(
    void)
{
    // disable clock
    ithClearRegBitA(ITH_HOST_BASE + ITH_VIDEO_CLK1_REG, ITH_EN_W23CLK_BIT);
    ithClearRegBitA(ITH_HOST_BASE + ITH_VIDEO_CLK1_REG, ITH_EN_A3CLK_BIT);
    ithClearRegBitA(ITH_HOST_BASE + ITH_VIDEO_CLK1_REG, ITH_EN_N3CLK_BIT);
    ithClearRegBitA(ITH_HOST_BASE + ITH_VIDEO_CLK1_REG, ITH_EN_M5CLK_BIT);
    ithClearRegBitA(ITH_HOST_BASE + ITH_VIDEO_CLK1_REG, ITH_EN_XCLK_BIT);
    ithClearRegBitA(ITH_HOST_BASE + ITH_VIDEO_CLK1_REG, ITH_EN_DIV_XCLK_BIT);
}

void
ithVideoHostReset(
    void)
{
    uint32_t i;
    ithVideoDisableClock();
    for (i = 0; i < 100; i++) asm ("");
    //ithSetRegBitH(ITH_VIDEO_CLK2_REG, ITH_VIDEO_RST_BIT);
    ithSetRegBitA(ITH_HOST_BASE + ITH_VIDEO_CLK1_REG, ITH_VIDEO_RST_BIT);
    for (i = 0; i < 100; i++) asm ("");
    //ithSetRegBitH(ITH_VIDEO_CLK2_REG, ITH_VIDEO_REG_RST_BIT);
	ithSetRegBitA(ITH_HOST_BASE + ITH_VIDEO_CLK1_REG, ITH_VIDEO_REG_RST_BIT);
    for (i = 0; i < 500; i++) asm ("");
    //ithClearRegBitH(ITH_VIDEO_CLK2_REG, ITH_VIDEO_RST_BIT);
    ithClearRegBitA(ITH_HOST_BASE + ITH_VIDEO_CLK1_REG, ITH_VIDEO_RST_BIT);
    //ithClearRegBitH(ITH_VIDEO_CLK2_REG, ITH_VIDEO_REG_RST_BIT);
    ithClearRegBitA(ITH_HOST_BASE + ITH_VIDEO_CLK1_REG, ITH_VIDEO_REG_RST_BIT);
    for (i = 0; i < 500; i++) asm ("");
    ithVideoEnableClock();
}

VIDEO_HARDWARE_STATUS
ithVideoGetStatus(
    void)
{
    volatile uint32_t       value       = 0;
    VIDEO_HARDWARE_STATUS   hwStatus    = VIDEO_HARDWARE_STATUS_IDLE;

    value       = ithReadRegA(REG_VIDEO_DECODE_STATUS_2_FCH) & VIDEO_MSK_DECODE_STATUS_2_FCH;
    //hwStatus = ((value & VIDEO_MASK_QUEUE_STATUS)==VIDEO_DECODE_STATUS_IDLE) ? VIDEO_HARDWARE_STATUS_IDLE : VIDEO_HARDWARE_STATUS_BUSY;
    hwStatus    = ((value & (VIDEO_MASK_DECODE_STATUS | VIDEO_MASK_QUEUE_STATUS)) == VIDEO_DECODE_STATUS_IDLE) ? VIDEO_HARDWARE_STATUS_IDLE : VIDEO_HARDWARE_STATUS_BUSY;

    return hwStatus;
}

bool
ithVideoGetVLDErr(
    void)
{
    volatile uint32_t value = 0;
    value = (ithReadRegA(REG_VIDEO_VLD_DEBUG_INFO_3_6EH) >> 16) & VIDEO_MSK_VLD_DEBUG_INFO_3_6EH;

    if (value & VIDEO_MASK_VLD_ERROR)
        return true;

    return false;
}

uint8_t *
ithVideoGetBufAddr(
    void)
{
    return gp_video_buf_sys_addr;
}

void
ithVideoBufRelease(
    void)
{
#ifndef CFG_BUILD_MEDIASTREAMER2
    #if !defined (CFG_CHIP_PKG_IT9854)
    if (gp_video_buf_sys_addr)
    {
        itpVmemFree(gp_video_buf_vram_addr);
        gp_video_buf_sys_addr   = NULL;
        gp_video_buf_vram_addr  = 0;
    }
    #endif
#endif
}

void
ithVideoSetBufSize(
    void)
{
// AVC buffer configuration
    TILING_MODE_OFF = 1;
    if (TILING_MODE_OFF)
    {
#if defined(CFG_DOORBELL_INDOOR) || defined(CFG_DOORBELL_ADMIN) || defined(CFG_DOORBELL_LOBBY)
		TILING_BLOCK            = (8);
        FRAME_BUFFER_COUNT      = (4);
        VIDEO_MAX_WIDTH         = (1280);
        VIDEO_MAX_HEIGHT        = (720);
        VIDEO_BUFFER_PITCH      = (VIDEO_MAX_WIDTH);
        Y_BUFFER_OFFSET         = (VIDEO_BUFFER_PITCH * VIDEO_MAX_HEIGHT * 3) / 2;
        U_BUFFER_OFFSET         = (VIDEO_BUFFER_PITCH * VIDEO_MAX_HEIGHT);
        V_BUFFER_OFFSET         = (VIDEO_BUFFER_PITCH * VIDEO_MAX_HEIGHT);
        VIDEO_BUFFER_HEIGHT     = (VIDEO_MAX_HEIGHT * 3 / 2) * FRAME_BUFFER_COUNT;
        BLANK_BUFFER_HEIGHT     = (16);
        BLANK_U_BUFFER_OFFSET   = (16*16);
        BLANK_V_BUFFER_OFFSET   = (16*16+8*8);
        DB_BUFFER_HEIGHT        = (8);                              // (frame_pitch/16) * 2 * 8 * 8
        MV_BUFFER_HEIGHT        = (180);                            // (pitch x height)/2
        TC_BUFFER_HEIGHT        = (4);                              // frame_width/2 * 8
        VLD_BUFFER_HEIGHT       = (32);                             // frame_pitch * 4 * 8 * 2
        COL_BUFFER_HEIGHT       = (0);
        CMD_DATA_BUFFER_COUNT   = (3);
        CMD_DATA_BUFFER_HEIGHT  = (500);
#else
       	TILING_BLOCK            = (8);
    #if defined(CFG_CHIP_PKG_IT9868) || defined(CFG_CHIP_PKG_IT9868_AT) || defined(CFG_CHIP_PKG_IT9869) || defined(CFG_CHIP_PKG_IT9869_AT) // 1080p	        
        FRAME_BUFFER_COUNT      = (4);
        VIDEO_MAX_WIDTH         = (1920);
        VIDEO_MAX_HEIGHT        = (1088);
    #elif defined(CFG_CHIP_PKG_IT9862) || defined(CFG_CHIP_PKG_IT9863) // 480x272
        FRAME_BUFFER_COUNT      = (2);
        VIDEO_MAX_WIDTH         = (480);
        VIDEO_MAX_HEIGHT        = (272);
    #else // 720p
        FRAME_BUFFER_COUNT      = (4);
        VIDEO_MAX_WIDTH         = (1280);
        VIDEO_MAX_HEIGHT        = (720);
    #endif
        VIDEO_BUFFER_PITCH      = (VIDEO_MAX_WIDTH);
        Y_BUFFER_OFFSET         = (VIDEO_BUFFER_PITCH * VIDEO_MAX_HEIGHT * 3) / 2;
        U_BUFFER_OFFSET         = (VIDEO_BUFFER_PITCH * VIDEO_MAX_HEIGHT);
        V_BUFFER_OFFSET         = (VIDEO_BUFFER_PITCH * VIDEO_MAX_HEIGHT);
        VIDEO_BUFFER_HEIGHT     = (VIDEO_MAX_HEIGHT * 3 / 2) * FRAME_BUFFER_COUNT;
        BLANK_BUFFER_HEIGHT     = (16);
        BLANK_U_BUFFER_OFFSET   = (16*16);
        BLANK_V_BUFFER_OFFSET   = (16*16+8*8);
        DB_BUFFER_HEIGHT        = (8);                              // (frame_pitch/16) * 2 * 8 * 8
        MV_BUFFER_HEIGHT        = (180);                            // (pitch x height)/2
        TC_BUFFER_HEIGHT        = (4);                              // frame_width/2 * 8
        VLD_BUFFER_HEIGHT       = (32);                             // frame_pitch * 4 * 8 * 2
        COL_BUFFER_HEIGHT       = (0);
        CMD_DATA_BUFFER_COUNT   = (3);
        CMD_DATA_BUFFER_HEIGHT  = (500);
#endif
    }
}

VIDEO_ERROR_CODE
ithVideoInit(
    VIDEO_DECODER_DESC *p_video_desc)
{
    VIDEO_ERROR_CODE    ret = VIDEO_ERROR_SUCCESS;
    int                 totalMemSize;

    ithVideoSetBufSize();

    //ASSERT(p_video_desc);
    //ASSERT(0 < p_video_desc->video_max_width  && p_video_desc->video_max_width  <= VIDEO_MAX_WIDTH);
    //ASSERT(0 < p_video_desc->video_max_height && p_video_desc->video_max_height <= VIDEO_MAX_HEIGHT);

    // [9070] 1280x720 : 7993344 Byte
    // [9850] 1280x720 : 7759872 Byte
    // [9850] 800x600  : 4668416 Byte
    // [9850] 640x480  : 2723840 Byte
    if (ithGetDeviceId() == 0x9860)
        totalMemSize = (VIDEO_BUFFER_HEIGHT +
                        BLANK_BUFFER_HEIGHT +
                        DB_BUFFER_HEIGHT +
                        MV_BUFFER_HEIGHT +
                        TC_BUFFER_HEIGHT +
                        VLD_BUFFER_HEIGHT +
                        COL_BUFFER_HEIGHT +
                        CMD_DATA_BUFFER_HEIGHT * CMD_DATA_BUFFER_COUNT +
                        1 ) * VIDEO_BUFFER_PITCH;
    else
        totalMemSize = (VIDEO_BUFFER_HEIGHT +
                        BLANK_BUFFER_HEIGHT +
                        DB_BUFFER_HEIGHT +
                        MV_BUFFER_HEIGHT +
                        TC_BUFFER_HEIGHT +
                        VLD_BUFFER_HEIGHT +
                        COL_BUFFER_HEIGHT +
                        CMD_DATA_BUFFER_HEIGHT * CMD_DATA_BUFFER_COUNT +
                        1 ) * VIDEO_BUFFER_PITCH;

    printf("totalMemSize = %d\n", totalMemSize);
    ithVideoBufAlloc(totalMemSize);

    // Blank Buffer
    {
        uint8_t *py, *pu, *pv;

        py  = gp_video_buf_sys_addr + (VIDEO_BUFFER_HEIGHT * VIDEO_BUFFER_PITCH);
        pu  = py + BLANK_U_BUFFER_OFFSET;
        pv  = py + BLANK_V_BUFFER_OFFSET;

        _Clear_Blank_Buffer(py, pu, pv);

        //ithPrintf("BLANK %x %x %x\n", py, pu, pv);
        gp_blank_buf_adr[0] = py;
        gp_blank_buf_adr[1] = pu;
        gp_blank_buf_adr[2] = pv;
    }
    return ret;
}

VIDEO_ERROR_CODE
ithVideoExit(
    void)
{
    VIDEO_ERROR_CODE ret = VIDEO_ERROR_SUCCESS;
    ithVideoBufRelease();
    return ret;
}

VIDEO_ERROR_CODE
ithVideoOpen(
    AVC_DECODER *ptDecoder)
{
    VIDEO_ERROR_CODE    error = VIDEO_ERROR_SUCCESS;
    uint8_t             *pbuf = NULL;
    int                 i = 0, j = 0;
    int                 frame_buf_cnt = 0;
    int                 DB_Buf_Height;

    if (!ptDecoder || !ptDecoder->pAVCDecoderBuf)
        return ERROR_VIDEO_INVALID_ARG;

    ithVideoEnableClock();

    //ASSERT(ptDecoder->pAVCDecoderBuf);
    //ASSERT(0 < ptDecoder->frameBufCount && ptDecoder->frameBufCount <= FRAME_BUFFER_COUNT);

    pbuf            = ptDecoder->pAVCDecoderBuf;
    frame_buf_cnt   = ptDecoder->frameBufCount;

#if 0
    if (ithGetDeviceId() == 9860)
        DB_Buf_Height = 80;
    else
#endif
	DB_Buf_Height = DB_BUFFER_HEIGHT;

    // Frame Buffer
    for (i = 0; i < frame_buf_cnt; i++)
    {
        uint8_t *py, *pu, *pv;

        if (VIDEO_BUFFER_PITCH == 4096)
        {
            if (i == 0)
            {
                py  = pbuf;
                pu  = py + 1280;
                pv  = py + 360 * 4096 + 1280;
            }
            else if (i == 1)
            {
                py  = pbuf + 1280 + 640;
                pu  = py + 1280;
                pv  = py + 360 * 4096 + 1280;
            }
            else if (i == 2)
            {
                py  = pbuf + 720 * 4096;
                pu  = py + 1280;
                pv  = py + 360 * 4096 + 1280;
            }
            else
            {
                py  = pbuf + 720 * 4096 + 1280 + 640;
                pu  = py + 1280;
                pv  = py + 360 * 4096 + 1280;
            }
        }
        else     // pitch = 4096
        {
            py  = pbuf + i * Y_BUFFER_OFFSET;
            pu  = py + U_BUFFER_OFFSET;
            pv  = py + V_BUFFER_OFFSET;
        }

        _HARDWARE_SetDecodedVideoBufAddr(i, py, pu, pv);
        //ithPrintf("VIDEO(%d) %x %x %x\n", i, py, pu, pv);
    }

    // DB Buffer
    {
        int8_t *addr = pbuf + ((VIDEO_BUFFER_HEIGHT + BLANK_BUFFER_HEIGHT) * VIDEO_BUFFER_PITCH);
        _HARDWARE_SetDBBufAddr(addr);
        //ithPrintf("DB %x\n", addr);
    }

    // MV Buffer
    {
        int8_t *addr = pbuf + ((VIDEO_BUFFER_HEIGHT + BLANK_BUFFER_HEIGHT + DB_Buf_Height) * VIDEO_BUFFER_PITCH);
        _HARDWARE_SetMVBufAddr(addr);
        //ithPrintf("MV %x\n", addr);
    }

    // TC Buffer
    {
        int8_t *addr = pbuf + ((VIDEO_BUFFER_HEIGHT + BLANK_BUFFER_HEIGHT + DB_Buf_Height + MV_BUFFER_HEIGHT) * VIDEO_BUFFER_PITCH);
        _HARDWARE_SetTCBufAddr(addr);
        //ithPrintf("TC %x\n", addr);
    }

    // VLD Buffer
    {
        int8_t *addr = pbuf + ((VIDEO_BUFFER_HEIGHT + BLANK_BUFFER_HEIGHT + DB_Buf_Height + MV_BUFFER_HEIGHT + TC_BUFFER_HEIGHT) * VIDEO_BUFFER_PITCH);
        _HARDWARE_SetVLDBufAddr(addr);
        //ithPrintf("VLD %x\n", addr);
    }

    // COL Buffer
    {
        int8_t *addr = pbuf + ((VIDEO_BUFFER_HEIGHT + BLANK_BUFFER_HEIGHT + DB_Buf_Height + MV_BUFFER_HEIGHT + TC_BUFFER_HEIGHT + VLD_BUFFER_HEIGHT) * VIDEO_BUFFER_PITCH);

        ptDecoder->colDataBufAdr[0]                                                                 =
            ptDecoder->colDataBufAdr[1]                                                             =
                ptDecoder->colDataBufAdr[2]                                                         =
                    ptDecoder->colDataBufAdr[3]                                                     =
                        ptDecoder->colDataBufAdr[4]                                                 =
                            ptDecoder->colDataBufAdr[5]                                             =
                                ptDecoder->colDataBufAdr[6]                                         =
                                    ptDecoder->colDataBufAdr[7]                                     =
                                        ptDecoder->colDataBufAdr[8]                                 =
                                            ptDecoder->colDataBufAdr[9]                             =
                                                ptDecoder->colDataBufAdr[10]                        =
                                                    ptDecoder->colDataBufAdr[11]                    =
                                                        ptDecoder->colDataBufAdr[12]                =
                                                            ptDecoder->colDataBufAdr[13]            =
                                                                ptDecoder->colDataBufAdr[14]        =
                                                                    ptDecoder->colDataBufAdr[15]    = ((uint32_t)addr) >> 3;

        //ithPrintf("COL %x\n", addr);
    }

    // CMD Buffer
    for (i = 0; i < CMD_DATA_BUFFER_COUNT; i++)
    {
        int8_t *addr = pbuf + ((CMD_DATA_BUFFER_HEIGHT * i + VIDEO_BUFFER_HEIGHT + BLANK_BUFFER_HEIGHT + DB_Buf_Height + MV_BUFFER_HEIGHT + TC_BUFFER_HEIGHT + VLD_BUFFER_HEIGHT + COL_BUFFER_HEIGHT) * VIDEO_BUFFER_PITCH);

        ptDecoder->ppCmdDataBufAddr[i] = addr;
        //ithPrintf("CMD(%d) %x\n", i, addr);
    }
#ifdef WIN32
    memset(ptDecoder->ppCmdDataBufAddr[0], 0, CMD_DATA_BUFFER_COUNT * CMD_DATA_BUFFER_HEIGHT * VIDEO_BUFFER_PITCH);
#endif

    // Set tiling mode mapping table
    _HARDWARE_TilingReMapTable();

    // MM9070 A1 ECO, add Co-located Data Bottom field Offset Register (0xEEC)
    ithWriteRegA(REG_VIDEO_COL_BOT_OFFSET_ECH, (uint32_t)(VIDEO_BUFFER_PITCH / 16) * (ptDecoder->frameHeight / 4));

    return error;
}

uint32_t gFireCount = 0;
VIDEO_ERROR_CODE
ithVideoFire(
    AVC_DECODER *ptDecoder)
{
    VIDEO_ERROR_CODE    error = VIDEO_ERROR_SUCCESS;
    uint32_t            cmdDataBufSelect, cmdDataBuf_Addr;

    if (!ptDecoder)
        return ERROR_VIDEO_INVALID_ARG;

    cmdDataBufSelect = ptDecoder->cmdDataBufSelect;

	ithWriteRegA(REG_VIDEO_SPECIAL_FUNC_SETTING_F0H, 0xf180);

    if ((cmdDataBufSelect < 0 || cmdDataBufSelect > 2) || (ptDecoder->decodeBufSelect < 0 || ptDecoder->decodeBufSelect > (ptDecoder->frameBufCount) - 1))
        return ERROR_VIDEO_DECODE_BUFFER_INDEX;

    cmdDataBuf_Addr = (uint32_t)ptDecoder->ppCmdDataBufAddr[cmdDataBufSelect];

    _HARDWARE_SetCmdDataBufAddr((uint8_t *)cmdDataBuf_Addr);
    gFireCount++;

    return error;
}

VIDEO_ERROR_CODE
ithVideoWait(
    AVC_DECODER *ptDecoder,
    uint32_t    timeout)
{
    VIDEO_ERROR_CODE        error               = VIDEO_ERROR_SUCCESS;

    uint16_t                xMacroBlock_Curr    = 0;
    uint16_t                yMacroBlock_Curr    = 0;
    uint16_t                xMacroBlock_Last    = 199;
    uint16_t                yMacroBlock_Last    = 199;
    uint32_t                finalCountDown      = timeout;
    VIDEO_HARDWARE_STATUS   hwStatus            = VIDEO_HARDWARE_STATUS_IDLE;
    volatile uint16_t       value0              = 0;
    volatile uint16_t       value1              = 0;
    volatile uint16_t       value2              = 0;
    uint32_t                timeout1            = 0;
    uint16_t                cabErr              = 0;

    hwStatus = ithVideoGetStatus();

    for (; (hwStatus == VIDEO_HARDWARE_STATUS_BUSY) && (finalCountDown > 0); finalCountDown--)
    {
#if 0
        xMacroBlock_Curr    = (ithReadRegA(REG_VIDEO_SETUP_DEBUG_INFO_7CH) & VIDEO_MASK_MACROBLOCK_X_COORDINATE_S) >> 8;
        yMacroBlock_Curr    = (ithReadRegA(REG_VIDEO_SETUP_DEBUG_INFO_7CH) & VIDEO_MASK_MACROBLOCK_Y_COORDINATE_S);

        if (xMacroBlock_Curr == xMacroBlock_Last &&
            yMacroBlock_Curr == yMacroBlock_Last)
        {
            xMacroBlock_Last    = (ithReadRegA(REG_VIDEO_TC_DEBUG_INFO_4_80H) & VIDEO_MASK_MACROBLOCK_X_COORDINATE_T) >> 8;
            yMacroBlock_Last    = (ithReadRegA(REG_VIDEO_TC_DEBUG_INFO_4_80H) & VIDEO_MASK_MACROBLOCK_Y_COORDINATE_T);

            usleep(1000);
            xMacroBlock_Curr    = (ithReadRegA(REG_VIDEO_TC_DEBUG_INFO_4_80H) & VIDEO_MASK_MACROBLOCK_X_COORDINATE_T) >> 8;
            yMacroBlock_Curr    = (ithReadRegA(REG_VIDEO_TC_DEBUG_INFO_4_80H) & VIDEO_MASK_MACROBLOCK_Y_COORDINATE_T);

            if (xMacroBlock_Curr == xMacroBlock_Last &&
                yMacroBlock_Curr == yMacroBlock_Last)
            {
                break;
            }

            xMacroBlock_Last    = (ithReadRegA(REG_VIDEO_SETUP_DEBUG_INFO_7CH) & VIDEO_MASK_MACROBLOCK_X_COORDINATE_S) >> 8;
            yMacroBlock_Last    = (ithReadRegA(REG_VIDEO_SETUP_DEBUG_INFO_7CH) & VIDEO_MASK_MACROBLOCK_Y_COORDINATE_S);
        }
        else
        {
            xMacroBlock_Last    = xMacroBlock_Curr;
            yMacroBlock_Last    = yMacroBlock_Curr;
        }
#endif
        usleep(1000);
        hwStatus = ithVideoGetStatus();
    }

#if 0
    value0  = ithReadRegH(REG_VIDEO_TC_DEBUG_INFO_1_72H);
    value1  = ithReadRegH(REG_VIDEO_DB_DEBUG_INFO_76H);
    value2  = ithReadRegH(REG_VIDEO_ARBITER_DEBUG_INFO_7AH);
    while ((value0 & 0x1000) || !(value1 & 0x1000) || (value2 & 0x0078) || (value1 & 0x2))
    {
        usleep(1);
        value0  = ithReadRegH(REG_VIDEO_TC_DEBUG_INFO_1_72H);
        value1  = ithReadRegH(REG_VIDEO_DB_DEBUG_INFO_76H);
        value2  = ithReadRegH(REG_VIDEO_ARBITER_DEBUG_INFO_7AH);
        timeout1++;
        if (timeout1 > 10)
        {
            break;
        }
        //ithPrintf("timeout %d\n", timeout1);
    }
#endif

    hwStatus = ithVideoGetStatus();

    if (hwStatus == VIDEO_HARDWARE_STATUS_BUSY)
    {
        usleep(10000);
        hwStatus            = ithVideoGetStatus();
        if (hwStatus == VIDEO_HARDWARE_STATUS_BUSY)
            error = ERROR_VIDEO_DECODE_HALT;
        xMacroBlock_Last    = (ithReadRegA(REG_VIDEO_TC_DEBUG_INFO_4_80H) & VIDEO_MASK_MACROBLOCK_X_COORDINATE_T) >> 8;
        yMacroBlock_Last    = ithReadRegA(REG_VIDEO_TC_DEBUG_INFO_4_80H) & VIDEO_MASK_MACROBLOCK_Y_COORDINATE_T;
        cabErr              = ithReadRegA(REG_VIDEO_CABAC_DEBUG_INFO_0_88H) & VIDEO_MSK_CABAC_DEBUG_INFO_0_88H;
        //printf("firecount0: %u, fail hwStatus: 0x%X, x_last: %u, y_last: %u, addr: 0xF88, cabErr: 0x%X\n",
        //    gFireCount, hwStatus, xMacroBlock_Last, yMacroBlock_Last, cabErr);
    }
    else
    {
        xMacroBlock_Last    = (ithReadRegA(REG_VIDEO_TC_DEBUG_INFO_4_80H) & VIDEO_MASK_MACROBLOCK_X_COORDINATE_T) >> 8;
        yMacroBlock_Last    = ithReadRegA(REG_VIDEO_TC_DEBUG_INFO_4_80H) & VIDEO_MASK_MACROBLOCK_Y_COORDINATE_T;
        cabErr              = ithReadRegA(REG_VIDEO_CABAC_DEBUG_INFO_0_88H) & VIDEO_MSK_CABAC_DEBUG_INFO_0_88H;
        if ((cabErr & 0x8000))
        {
            //printf("firecount1: %u, ok hwStatus: 0x%X, x_last: %u, y_last: %u, addr: 0xF88, cabErr: 0x%04X\n",
            //    gFireCount, hwStatus, xMacroBlock_Last, yMacroBlock_Last, cabErr);

            //ithVideoHostReset();
            //ithVideoReset(ptDecoder);
            error = ERROR_VIDEO_DECODE_BUFFER_ERROR;
        }
        //else
        //    printf("decode success\n");
    }

    if (ithVideoGetVLDErr())
    {
        error = ERROR_VIDEO_DECODE_BUFFER_ERROR;
    }

    return error;
}

VIDEO_ERROR_CODE
ithVideoClose(
    AVC_DECODER *ptDecoder)
{
    VIDEO_ERROR_CODE    error = VIDEO_ERROR_SUCCESS;
    uint8_t             *py, *pu, *pv;

    py  = gp_blank_buf_adr[0];
    pu  = gp_blank_buf_adr[1];
    pv  = gp_blank_buf_adr[2];

    _Clear_Blank_Buffer(py, pu, pv);

    if (ithVideoWait(ptDecoder, 1000) != 0)
    {
        ithPrintf("AVC Close : wait avc idle timeout....1\n");
        ithVideoHostReset();
        ithPrintf("AVC Close : wait avc idle timeout....2\n");
        ithVideoReset(ptDecoder);
        ithPrintf("AVC Close : wait avc idle timeout....3\n");
    }

    ithVideoDisableClock();

    if (!ptDecoder)
        return ERROR_VIDEO_INVALID_ARG;

    return error;
}

VIDEO_ERROR_CODE
ithVideoReset(
    AVC_DECODER *ptDecoder)
{
    VIDEO_ERROR_CODE    error               = VIDEO_ERROR_SUCCESS;
    uint32_t            ColDatafieldOffset  = 0;

    if (!ptDecoder)
        return ERROR_VIDEO_INVALID_ARG;

    //if (ptDecoder->vidCodecType != VID_CODEC_AVC) //?
        ithVideoHostReset();

    ptDecoder->vidCodecType = VID_CODEC_AVC;
    //ColDatafieldOffset      = (VIDEO_BUFFER_PITCH / 16) * (ptDecoder->frameHeight / 4);
	//ithWriteRegA(REG_VIDEO_COL_BOT_OFFSET_ECH, (uint32_t)ColDatafieldOffset);

    // Do init
    error = ithVideoOpen(ptDecoder);

    return error;
}

uint32_t ithVideoQuery(VIDEO_QUERY_TYPE queryType, uint32_t value)
{
    uint32_t retValue;

    switch (queryType)
    {
    case VIDEO_ADDRESS_FRAME_BUF_Y:
        retValue = _HARDWARE_GetDecodedVideoBufVramAddr(value, Y);
        break;

    case VIDEO_ADDRESS_FRAME_BUF_U:
        retValue = _HARDWARE_GetDecodedVideoBufVramAddr(value, U);
        break;

    case VIDEO_ADDRESS_FRAME_BUF_V:
        retValue = _HARDWARE_GetDecodedVideoBufVramAddr(value, V);
        break;

    case VIDEO_ADDRESS_BLANK_BUF_Y:
        retValue = (uint32_t) gp_blank_buf_adr[0];
        break;

    case VIDEO_ADDRESS_BLANK_BUF_U:
        retValue = (uint32_t) gp_blank_buf_adr[1];
        break;

    case VIDEO_ADDRESS_BLANK_BUF_V:
        retValue = (uint32_t) gp_blank_buf_adr[2];
        break;

    case VIDEO_FRAME_BUFFER_COUNT:
        retValue = FRAME_BUFFER_COUNT;
        break;

    case VIDEO_FRAME_BUFFER_Y_PITCH:
        retValue = VIDEO_BUFFER_PITCH;
        break;

    case VIDEO_FRAME_BUFFER_UV_PITCH:
        retValue = VIDEO_BUFFER_PITCH;
        break;

    case VIDEO_MAX_VIDEO_WIDTH:
        retValue = VIDEO_MAX_WIDTH;
        break;

    case VIDEO_MAX_VIDEO_HEIGHT:
        retValue = VIDEO_MAX_HEIGHT;
        break;

    case VIDEO_CMD_DATA_BUFFER_MAX_SIZE:
        retValue = CMD_DATA_BUFFER_HEIGHT * VIDEO_BUFFER_PITCH;
        break;

    default:
        break;
    }

    return retValue;
}

static int
ithVideoBufAlloc(
    uint32_t size)
{
    uint32_t rev = 0;
    if (!gp_video_buf_sys_addr)
    {
        gp_video_buf_vram_addr  = itpVmemAlignedAlloc(TILING_BLOCK, size);
        gp_video_buf_sys_addr   = (uint8_t *) ithMapVram(gp_video_buf_vram_addr, size, /*ITH_VRAM_READ |*/ ITH_VRAM_WRITE);
        rev                     = gp_video_buf_sys_addr ? size : -1;
    }
    return rev;
}

//=============================================================================
//                              Private Function Definition
//=============================================================================
void _HARDWARE_SetDecodedVideoBufAddr(
    uint32_t    buf_index,
    uint8_t     *y_sys_addr,
    uint8_t     *u_sys_addr,
    uint8_t     *v_sys_addr)
{
    uint16_t    offset      = buf_index * 4 * 3 + ((buf_index >= 8) ? 16 : 0);
    uint32_t    y_vram_addr = ithSysAddr2VramAddr(y_sys_addr);
    uint32_t    u_vram_addr = ithSysAddr2VramAddr(u_sys_addr);
    uint32_t    v_vram_addr = ithSysAddr2VramAddr(v_sys_addr);

    printf( "video decode buffer Y[%d] [SYS](0x%X) [VRAM](0x%X)\n", buf_index,  y_sys_addr, y_vram_addr);
    printf( "video decode buffer U[%d] [SYS](0x%X) [VRAM](0x%X)\n", buf_index,  u_sys_addr, u_vram_addr);
    printf( "video decode buffer V[%d] [SYS](0x%X) [VRAM](0x%X)\n", buf_index,  v_sys_addr, v_vram_addr);
    y_vram_addr >>= 3;
    u_vram_addr >>= 3;
    v_vram_addr >>= 3;

    ithWriteRegA(   REG_VIDEO_DECODE_FRAME_0_Y_ADDR_L_00H + offset, (uint32_t)y_vram_addr);
    ithWriteRegA(   REG_VIDEO_DECODE_FRAME_0_U_ADDR_L_04H + offset, (uint32_t)u_vram_addr);
    ithWriteRegA(   REG_VIDEO_DECODE_FRAME_0_V_ADDR_L_08H + offset, (uint32_t)v_vram_addr);
}

uint32_t _HARDWARE_GetDecodedVideoBufVramAddr(
    uint32_t    buf_index,
    uint32_t    yuv_index)
{
    uint16_t    offset  = buf_index * 4 * 3 + ((buf_index >= 8) ? 16 : 0) + yuv_index * 4;
    uint32_t    ret     = 0;

    ret = ithReadRegA(REG_VIDEO_DECODE_FRAME_0_Y_ADDR_L_00H + offset);
    ret <<= 3;

    return ret;
}

void _HARDWARE_SetMVBufAddr(
    uint8_t *mv_buf_sys_addr)
{
    uint32_t mv_buf_vram_addr = ithSysAddr2VramAddr(mv_buf_sys_addr);
    //printf( "video mv buffer [SYS](0x%X) [VRAM](0x%X)\n", mv_buf_sys_addr, mv_buf_vram_addr);
    mv_buf_vram_addr >>= 3;

    ithWriteRegA(REG_VIDEO_MV_BUF_0_ADDR_L_60H, (uint32_t)mv_buf_vram_addr);
}

void _HARDWARE_SetTCBufAddr(
    uint8_t *tc_buf_sys_addr)
{
    uint32_t tc_buf_vram_addr = ithSysAddr2VramAddr(tc_buf_sys_addr);
    //printf( "video tc buffer [SYS](0x%X) [VRAM](0x%X)\n", tc_buf_sys_addr, tc_buf_vram_addr);
    tc_buf_vram_addr >>= 3;

    ithWriteRegA(REG_VIDEO_TC_BUF_ADDR_L_68H, (uint32_t)tc_buf_vram_addr);
}

void _HARDWARE_SetVLDBufAddr(
    uint8_t *vld_buf_sys_addr)
{
    uint32_t vld_buf_vram_addr = ithSysAddr2VramAddr(vld_buf_sys_addr);
    //printf( "video vld buffer [SYS](0x%X) [VRAM](0x%X)\n", vld_buf_sys_addr, vld_buf_vram_addr);
    vld_buf_vram_addr >>= 3;

    ithWriteRegA(REG_VIDEO_VLD_BUF_ADDR_L_6CH, (uint32_t)vld_buf_vram_addr);
}

void _HARDWARE_SetDBBufAddr(
    uint8_t *db_buf_sys_addr)
{
    uint32_t db_buf_vram_addr = ithSysAddr2VramAddr(db_buf_sys_addr);
    //printf( "video db buffer [SYS](0x%X) [VRAM](0x%X)\n", db_buf_sys_addr, db_buf_vram_addr);
    db_buf_vram_addr >>= 3;

    ithWriteRegA(REG_VIDEO_DB_BUF_ADDR_L_E8H, (uint32_t)db_buf_vram_addr);
}

void _HARDWARE_SetCmdDataBufAddr(
    uint8_t *cmd_data_buf_sys_addr)
{
    uint32_t cmd_data_buf_vram_addr = ithSysAddr2VramAddr(cmd_data_buf_sys_addr);

    //printf( "video cmd data buffer [SYS](0x%X) [VRAM](0x%X)\n", cmd_data_buf_sys_addr, cmd_data_buf_vram_addr );
    cmd_data_buf_vram_addr >>= 3;
    ithWriteRegA(REG_VIDEO_CMD_DATA_BUF_ADDR_L_F4H, (uint32_t)cmd_data_buf_vram_addr);
}

void _HARDWARE_TilingReMapTable(
    void)
{
    if (TILING_MODE_OFF)
        return;
}

void _Clear_Blank_Buffer(
    uint8_t *py,
    uint8_t *pu,
    uint8_t *pv)
{
    uint8_t i;

    if (TILING_MODE_OFF)
    {
        for (i = 0; i < 16; i++)
            memset((void *)(py + i * VIDEO_BUFFER_PITCH), 0x10, 16);
        for (i = 0; i < 8; i++)
            memset((void *)(pu + i * VIDEO_BUFFER_PITCH), 0x80, 8);
        for (i = 0; i < 8; i++)
            memset((void *)(pv + i * VIDEO_BUFFER_PITCH), 0x80, 8);
    }
}
