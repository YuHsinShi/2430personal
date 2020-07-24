/*
 * Copyright (c) 2018 ITE technology Corp. All Rights Reserved.
 */
/** @copy from file itv.c
 *   @just for testcap , test only.
 *
 * @version 0.1
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "test_itv.h"
#include "isp/mmp_isp.h"
#include "ith/ith_video.h"


//=============================================================================
//                              Constant Definition
//=============================================================================
#if (defined(WIN32) && defined(_DEBUG)) || 0
    #define DEBUG_FLIP
#endif

//=============================================================================
//                              Macro Definition
//=============================================================================
#define WAIT_UNTIL(expr)          while (!(expr)) { usleep(20000); }
#define MIDDLE_WAIT_UNTIL(expr)   while (!(expr)) { usleep(3000);  }
#define SHORT_WAIT_UNTIL(expr)    while (!(expr)) { usleep(1000);  }
#define QUEUE_IS_FULL(w, r, size) ((((w) + 1) % (size)) == (r))

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct
{
    uint8_t           *dbuf[ITV_MAX_NDBUF];     // decoded video buffer address
    ITV_DBUF_PROPERTY dbufprop[ITV_MAX_NDBUF];  // property of the decoded video buffer
    ITV_DBUF_PROPERTY blank_yuv_frame;
    volatile int      disp_buf_ridx;
    volatile int      disp_buf_widx;
    /* postponed command */
    volatile int      pcmd_flush_dbuf;
    bool              video_update;
    ITV_DBUF_PROPERTY *curr_video;

} ITV_STRC_RESOURCE;

typedef struct LCD_FLIP_TAG
{
    sem_t     tScanlineEvent;
    uint32_t  bInited;
    pthread_t hw_overlay_tid;
    pthread_t lcd_isr_tid;
} LCD_FLIP;

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
// when there are no video frames to output, a blank 16 x 16 frame will be output
static ITV_STRC_RESOURCE itv_rcs;
static LCD_FLIP          gtFlipCtrl;
sem_t                    tISPEvent;
static int               g_lcd_index;
static int               lcd_index;
static bool              g_stop_itv_hw_overlay_task;
static ISP_DEVICE        gIspDev;
static uint32_t          g_blank_buf;

//=============================================================================
//                              Private Function Declaration
//=============================================================================
static void itv_lcdISR(void *arg);
static void itv_isp0ISR(void *arg);
static void itv_isp1ISR(void *arg);
static void itv_change_display_format();

static uint32_t next_lcd_index()
{
    lcd_index = (lcd_index + 1) % 3;
    return lcd_index;
}

static void flip_lcd(
    int lcd_index)
{
    MMP_ISP_OUTPUT_INFO out_info  = {0};
    MMP_ISP_SHARE       isp_share = { 0 };

    isp_share.addrY   = (uint32_t)itv_rcs.curr_video->ya;
    isp_share.addrU   = (uint32_t)itv_rcs.curr_video->ua;
    isp_share.addrV   = (uint32_t)itv_rcs.curr_video->va;
    isp_share.addrYp  = (uint32_t)itv_rcs.curr_video->old_y;
    isp_share.width   = itv_rcs.curr_video->src_w;
    isp_share.height  = itv_rcs.curr_video->src_h;
    isp_share.pitchY  = itv_rcs.curr_video->pitch_y;
    isp_share.pitchUv = itv_rcs.curr_video->pitch_uv;
    isp_share.format  = itv_rcs.curr_video->format;
    
    lcd_index        = next_lcd_index();
    
    out_info.addrRGB = (uint32_t)itv_rcs.dbuf[lcd_index];
    out_info.width    = ithLcdGetWidth();
    out_info.height   = ithLcdGetHeight();
    out_info.pitchRGB = ithLcdGetPitch();
    out_info.format = MMP_ISP_OUT_DITHER565A;
    
    mmpIspSetMode(gIspDev, MMP_ISP_MODE_PLAY_VIDEO);
    mmpIspSetOutputWindow(gIspDev, &out_info);
    mmpIspSetVideoWindow(gIspDev, 0, 0, out_info.width, out_info.height);
    #ifdef _WIN32
        mmpIspDisableInterrupt(gIspDev);
    #else
        mmpIspEnableInterrupt(gIspDev);
    #endif
    
    mmpIspPlayImageProcess(gIspDev, &isp_share);

    #ifdef _WIN32
        SHORT_WAIT_UNTIL(mmpIspIsEngineIdle(gIspDev));
    #else
        sem_wait(&tISPEvent);
    #endif

    g_lcd_index      = lcd_index;
    itv_rcs.video_update = false;
}

static void itv_lcdISR(void *arg __attribute__((__unused__)))
{
    if (gtFlipCtrl.bInited)
        sem_post(&gtFlipCtrl.tScanlineEvent);
#ifdef CFG_LCD_ENABLE
    ithLcdIntrClear();
#endif
}

static void itv_isp0ISR(void *arg __attribute__((__unused__)))
{
    sem_post(&tISPEvent);
    mmpIspClearInterrupt(gIspDev);
}

static void itv_isp1ISR(void *arg __attribute__((__unused__)))
{
    sem_post(&tISPEvent);
    mmpIspClearInterrupt(gIspDev);
}

static void _itv_hw_overlay_task(void)
{

    if (g_lcd_index >= 0)
    {
#ifdef CFG_LCD_ENABLE    
        ithLcdSwFlip(g_lcd_index);
#endif

        if (gtFlipCtrl.bInited)
            sem_wait(&gtFlipCtrl.tScanlineEvent);
        g_lcd_index = -1;
    }

    itv_rcs.video_update = false;

    if ((itv_rcs.disp_buf_ridx != itv_rcs.disp_buf_widx))
    {
        itv_rcs.curr_video    = &itv_rcs.dbufprop[itv_rcs.disp_buf_ridx];
        itv_rcs.video_update  = true;
        itv_rcs.disp_buf_ridx = (itv_rcs.disp_buf_ridx + 1) % ITV_MAX_DISP_BUF;
    }
    else
    {
        /* postponed command */
        itv_rcs.pcmd_flush_dbuf = 0;    // set this flag to 0 means all queued display buffers have been flushed.
    }

    if (itv_rcs.video_update)
    {
        flip_lcd(itv_rcs.disp_buf_ridx);
    }       
}

static void *itv_hw_overlay_task(void *arg)
{
    g_stop_itv_hw_overlay_task = false;
    while (!g_stop_itv_hw_overlay_task)
    {
        _itv_hw_overlay_task();
        usleep(1000);
    }
    pthread_exit(NULL);
    return (void *)0;
}

static void itv_change_display_format()
{
#ifdef CFG_LCD_ENABLE

    MMP_ISP_CORE_INFO   ISPCOREINFO = {0};
    MMP_ISP_OUTPUT_INFO outInfo  = {0};
    int width  = ithLcdGetWidth();
    int height = ithLcdGetHeight();

    mmpIspInitialize(&gIspDev, MMP_ISP_CORE_0);
    ISPCOREINFO.EnPreview = false;
    ISPCOREINFO.PreScaleSel = MMP_ISP_PRESCALE_NORMAL;
    mmpIspSetCore(gIspDev, &ISPCOREINFO);
    mmpIspSetMode(gIspDev, MMP_ISP_MODE_PLAY_VIDEO);
    mmpIspSetOutputFormat(gIspDev, MMP_ISP_OUT_DITHER565A);
    
    ithLcdSwFlip(ithLcdGetFlip());
    ithLcdDisableHwFlip();

    if (!gtFlipCtrl.bInited)
        gtFlipCtrl.bInited = !sem_init(&gtFlipCtrl.tScanlineEvent, 0, 1);

    sem_init(&tISPEvent, 0, 1);
    sem_wait(&tISPEvent);

    if (gtFlipCtrl.bInited)
    {
        g_lcd_index = -1;
        pthread_create(&gtFlipCtrl.hw_overlay_tid, NULL, itv_hw_overlay_task, NULL);
        /* enable lcd ISR */
        {
            uint32_t lcd_height = ithLcdGetHeight();
            ithIntrDisableIrq(ITH_INTR_LCD);
            ithLcdIntrClear();
            ithIntrClearIrq(ITH_INTR_LCD);

            ithIntrRegisterHandlerIrq(ITH_INTR_LCD, itv_lcdISR, NULL);
            ithLcdIntrSetScanLine1(lcd_height);
            ithLcdIntrSetScanLine2(lcd_height);
            ithLcdIntrCtrlDisable(ITH_LCD_INTR_OUTPUT2);
            ithLcdIntrCtrlDisable(ITH_LCD_INTR_FIELDMODE2);
            ithLcdIntrCtrlDisable(ITH_LCD_INTR_OUTPUT1);
            ithLcdIntrCtrlDisable(ITH_LCD_INTR_FIELDMODE1);

            ithLcdIntrCtrlEnable(ITH_LCD_INTR_OUTPUT2);

            ithIntrEnableIrq(ITH_INTR_LCD);
            ithLcdIntrEnable();
        }
        /* enable VP ISR */
        {
            mmpISPRegisterIRQ(gIspDev, itv_isp0ISR);
            //mmpISPRegisterIRQ(gIspDev, itv_isp1ISR);
        }
    }
#endif

}

int itp_itv_init(void)
{

    printf("ITV# init !\n");

    /* sanity check */
    if (ITV_MAX_NDBUF > 3)
    {
        printf("ERROR# invalid ITV_MAX_NDBUF (%d), %s:%d !\n", ITV_MAX_NDBUF, __FILE__, __LINE__);
        return -1;
    }

    g_blank_buf = itpVmemAlloc(16*16*2);
            
    // initialize the blank YUV frame's value
    memset((void *)&itv_rcs, 0, sizeof(ITV_STRC_RESOURCE));
    
    itv_rcs.blank_yuv_frame.ya       = (uint8_t *)g_blank_buf;
    itv_rcs.blank_yuv_frame.ua       = (uint8_t *)g_blank_buf + 256;
    itv_rcs.blank_yuv_frame.va       = (uint8_t *)g_blank_buf + 256 + 64;
    
    memset((void *) itv_rcs.blank_yuv_frame.ya, 0x10, 256);
    memset((void *) itv_rcs.blank_yuv_frame.ua, 0x80, 64);
    memset((void *) itv_rcs.blank_yuv_frame.va, 0x80, 64);
    
    itv_rcs.blank_yuv_frame.src_w    = 16;
    itv_rcs.blank_yuv_frame.src_h    = 16;
    itv_rcs.blank_yuv_frame.pitch_y  = 16;
    itv_rcs.blank_yuv_frame.pitch_uv = 16;
    itv_rcs.blank_yuv_frame.format   = MMP_ISP_IN_YUV420;
    itv_rcs.curr_video               = &itv_rcs.blank_yuv_frame;
#ifdef CFG_LCD_ENABLE
    itv_rcs.dbuf[0]                  = (uint8_t *)ithLcdGetBaseAddrA();
    itv_rcs.dbuf[1]                  = (uint8_t *)ithLcdGetBaseAddrB();
    itv_rcs.dbuf[2]                  = (uint8_t *)ithLcdGetBaseAddrC();
    lcd_index                        = ithLcdGetFlip();
#endif

    itv_change_display_format();

    return 0;
}

int itp_itv_deinit(void)
{
    printf("ITV# de-init !\n");

    if (gtFlipCtrl.bInited)
    {
        g_stop_itv_hw_overlay_task = true;
        pthread_join(gtFlipCtrl.hw_overlay_tid, NULL);

        #ifdef CFG_LCD_ENABLE
        ithLcdIntrDisable();
        #endif

        sem_destroy(&(gtFlipCtrl.tScanlineEvent));
        memset(&gtFlipCtrl, 0, sizeof(gtFlipCtrl));

        sem_destroy(&tISPEvent);
        memset(&tISPEvent, 0, sizeof(tISPEvent));
    }

    memset((void *)&itv_rcs, 0, sizeof(ITV_STRC_RESOURCE));
    
    itpVmemFree(g_blank_buf);
    return 0;
}

uint8_t *itp_itv_get_dbuf_anchor(void)
{    
    if (QUEUE_IS_FULL(itv_rcs.disp_buf_widx, itv_rcs.disp_buf_ridx, ITV_MAX_DISP_BUF))
        return NULL;

    return itv_rcs.dbuf[itv_rcs.disp_buf_widx];
}

int itp_itv_update_dbuf_anchor(ITV_DBUF_PROPERTY *prop)
{
    if (QUEUE_IS_FULL(itv_rcs.disp_buf_widx, itv_rcs.disp_buf_ridx, ITV_MAX_DISP_BUF))
        return -1;

    memcpy((void *)&itv_rcs.dbufprop[itv_rcs.disp_buf_widx], (void *)prop, sizeof(ITV_DBUF_PROPERTY));
    itv_rcs.disp_buf_widx = (itv_rcs.disp_buf_widx + 1) % ITV_MAX_DISP_BUF;
    
    return 0;
}

void itp_itv_flush_dbuf(void)
{
    itv_rcs.pcmd_flush_dbuf = 1;

    WAIT_UNTIL(itv_rcs.pcmd_flush_dbuf == 0);
}

int itp_itv_set_pb_mode(int pb_mode)
{
    return 0;
}

void itp_itv_set_video_window(uint32_t startX, uint32_t startY, uint32_t width, uint32_t height)
{
    mmpIspSetVideoWindow(gIspDev, startX, startY, width, height);
}

ISP_RESULT itp_itv_enable_isp_feature(MMP_ISP_CAPS cap)
{
    return mmpIspEnable(gIspDev, cap);
}

ISP_RESULT itp_itv_enable_3d_deinterlance(bool flag)
{
    if(flag)
      return mmpIspSetDeinterlaceType(gIspDev,MMP_ISP_3D_DEINTERLACE);
    else
      return mmpIspSetDeinterlaceType(gIspDev,MMP_ISP_2D_DEINTERLACE);
}

