/*
 * Copyright (c) 2015 ITE technology Corp. All Rights Reserved.
 */
/** @file itv.c
 * Used to do H/W video overlay
 *
 * @author I-Chun Lai
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
#include "ite/itv_iptv.h"
#include "isp/mmp_isp.h"
#include "fc_sync.h"
#include "ith/ith_video.h"
#include "capture/capture_9860/mmp_capture.h"
#if (CFG_CHIP_FAMILY != 9850 && CFG_CHIP_FAMILY != 970)
    #include "ite/ite_m2d.h"
    #include "m2d/m2d_graphics.h"
#endif
#include "openrtos/FreeRTOS.h"
#include "castor3player_iptv.h" /* FIXME */
#include "hdmitx/mmp_hdmitx.h"
#include "../../../../sdk/share/encrypt/polarssl/aes.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#if (defined(WIN32) && defined(_DEBUG)) || 0
#   define DEBUG_FLIP
#endif

//#define FRAME_FUNCTION_ENABLE

/* FRAME FUNCTION */
#define ITV_FF_NSET             2
#define ITV_FF_NBUF             2

#define MAX_ROTATE_BUFFER_COUNT 2

#define ITV_MAX_VSBUF           3
//=============================================================================
//                              Macro Definition
//=============================================================================
#define WAIT_UNTIL(expr)          while (!(expr)) { usleep(20000); }
#define MIDDLE_WAIT_UNTIL(expr)   while (!(expr)) { usleep(3000);  }
#define SHORT_WAIT_UNTIL(expr)    while (!(expr)) { usleep(1000);  }
#define QUEUE_IS_FULL(w, r, size) ((((w) + 1) % (size)) == (r))

#define IS_VERTICAL_DISPLAY()     ((gRotation) == ITU_ROT_90 || (gRotation) == ITU_ROT_270)
#define IS_ROTATE_DISPLAY()       ((gRotation) != ITU_ROT_0)

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct
{
    uint8_t         *uibuf[ITV_FF_NBUF];
    ITV_UI_PROPERTY uiprop[ITV_FF_NBUF];
    volatile int    ui_ridx;
    volatile int    ui_widx;
} ITV_STRC_FF;

typedef struct {
    int event;
    int data;
    struct Message *next;
} Message;

typedef struct {
    Message *front, *rear;
    int nb_msgs;
    int abort_request;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} MessageQueue;

typedef struct {
    int init;
    pthread_t msg_t;
    MessageQueue msg_q;
} MTAL_EV_HANDLER;

MTAL_EV_HANDLER *handle = NULL;

typedef struct
{
    // dbuf: display buffer or decoded video buffer
    //       currently we use 3 decoded video buffers
    uint8_t           *dbuf[ITV_MAX_NDBUF];     // decoded video buffer address
    ITV_DBUF_PROPERTY dbufprop[ITV_MAX_NDBUF];  // property of the decoded video buffer
    ITV_DBUF_PROPERTY blank_yuv_frame;
    volatile int disp_buf_ridx;
    volatile int disp_buf_widx;
	bool			  lcd_isr;
	bool			  update_dbuf_rindex;

    // video surface read/write index
    bool              check_video_surf;
    volatile int      vid_surf_cur_idx;
    volatile int      vid_surf_ridx;
    volatile int      vid_surf_widx;

    //FC_STRC_SYNC      itv_sync;
    
    /* postponed command */
    volatile int pcmd_flush_dbuf;
    volatile int pcmd_flush_uibuf[ITV_FF_NSET]; /* FRAME FUNCTION */

    /* MTAL */
    int               mtal_pb_mode_isr;      // indicate whether the player currently is in playback mode or not.
    volatile int mtal_pb_mode_u;
    int mtal_freerun_isr;
    volatile int mtal_freerun_u;
    MTAL_SPEC mtal_spec;
    int mtal_pause_isr;
    volatile int mtal_pause_u;
    unsigned mtal_fake_ir;
    int mtal_error_flag;
    unsigned mtal_lock_hdrl;
    int mtal_pb_livesrc;

	/* media control */
    FF_COMPONENT src_component;
    FF_COMPONENT demux_component;
    FF_COMPONENT decode_component; // not implement

    /* FRAME FUNCTION */
    bool              ui_update;
    bool              video_update;
    ITV_DBUF_PROPERTY *curr_video;
    int ff_mode_isr[ITV_FF_NSET];
    volatile int ff_mode_u[ITV_FF_NSET];
    ITV_STRC_FF ff[ITV_FF_NSET];
    volatile int ff_setbase;
    volatile int ff_setbase_id;
    volatile int ff_setbase_bid;
    volatile uint8_t  *ff_setbase_base;

	MMP_CAP_INPUT_INFO df;
    int                     displayWidth;
    int                     displayHeight;
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
static const uint32_t tv480p60VGAScript[] =
{
#include "../../../target/lcd/lcd_tv_480p60_VGA.inc"
};

static const uint32_t tv800x600p60VGAScript[] =
{
#include "../../../target/lcd/lcd_tv_800x600p60_VGA.inc"
};

static const uint32_t tv1024x768p60VGAScript[] =
{
#include "../../../target/lcd/lcd_tv_1024x768p60_VGA.inc"
};

static const uint32_t tv1280x768p60VGAScript[] =
{
#include "../../../target/lcd/lcd_tv_1280x768p60_VGA.inc"
};

static const uint32_t tv1280x960p60VGAScript[] =
{
#include "../../../target/lcd/lcd_tv_1280x960p60_VGA.inc"
};

static const uint32_t tv1280x1024p60VGAScript[] =
{
#include "../../../target/lcd/lcd_tv_1280x1024p60_VGA.inc"
};

static const uint32_t tv1360x768p60VGAScript[] =
{
#include "../../../target/lcd/lcd_tv_1360x768p60_VGA.inc"
};

static const uint32_t tv1440x900p60VGAScript[] =
{
#include "../../../target/lcd/lcd_tv_1440x900p60_VGA.inc"
};

static const uint32_t tv1600x900p60VGAScript[] =
{
#include "../../../target/lcd/lcd_tv_1600x900p60_VGA.inc"
};

static const uint32_t tv1680x1050p60VGAScript[] =
{
#include "../../../target/lcd/lcd_tv_1680x1050p60_VGA.inc"
};

static const uint32_t tv480p59Script[] =
{
#include "../../../target/lcd/lcd_tv_480p59.inc"
};

static const uint32_t tv480p60Script[] =
{
#include "../../../target/lcd/lcd_tv_480p60.inc"
};

static const uint32_t tv480i59Script[] =
{
#include "../../../target/lcd/lcd_tv_480i59.inc"
};

static const uint32_t tv480i60Script[] =
{
#include "../../../target/lcd/lcd_tv_480i60.inc"
};

static const uint32_t tv576p50Script[] =
{
#include "../../../target/lcd/lcd_tv_576p50.inc"
};

static const uint32_t tv576i50Script[] =
{
#include "../../../target/lcd/lcd_tv_576i50.inc"
};

static const uint32_t tv720p50Script[] =
{
#include "../../../target/lcd/lcd_tv_720p50.inc"
};

static const uint32_t tv720p59Script[] =
{
#include "../../../target/lcd/lcd_tv_720p59.inc"
};

static const uint32_t tv720p60Script[] =
{
#include "../../../target/lcd/lcd_tv_720p60.inc"
};

static const uint32_t tv1080p50Script[] =
{
#include "../../../target/lcd/lcd_tv_1080p50.inc"
};

static const uint32_t tv1080p59Script[] =
{
#include "../../../target/lcd/lcd_tv_1080p59.inc"
};

static const uint32_t tv1080p60Script[] =
{
#include "../../../target/lcd/lcd_tv_1080p60.inc"
};

static const uint32_t tv1080i50Script[] =
{
#include "../../../target/lcd/lcd_tv_1080i50.inc"
};

static const uint32_t tv1080i59Script[] =
{
#include "../../../target/lcd/lcd_tv_1080i59.inc"
};

static const uint32_t tv1080i60Script[] =
{
#include "../../../target/lcd/lcd_tv_1080i60.inc"
};

// when there are no video frames to output, a blank 16 x 16 frame will be output
static ITV_STRC_RESOURCE itv_rcs;
static LCD_FLIP          gtFlipCtrl;
sem_t                    tISPEvent;

#if (CFG_CHIP_FAMILY != 9850 && CFG_CHIP_FAMILY != 970)
// for rotation
static uint32_t        gRotateBuffer[MAX_ROTATE_BUFFER_COUNT];
static uint8_t         gRotateBufIdx = 0;
static MMP_M2D_SURFACE gRoateSurf[MAX_ROTATE_BUFFER_COUNT];
static MMP_M2D_SURFACE *_gRoateSurf;
static MMP_M2D_SURFACE _gLcdSurf[3];
#endif
static ITURotation     gRotation = ITU_ROT_0;
static int             g_lcd_index;
static int             lcd_index;
static                 g_stop_itv_hw_overlay_task;
static ISP_DEVICE      gIspDev;

static uint32_t        g_blank_buf;

static unsigned char *ff0 = NULL;
static unsigned char *ff1 = NULL;
static unsigned char *ffall = NULL;

static bool gbHasDataInput = false;
static bool gbSourceLockMode = false;
static int switch_changed = 0;
static short    gDelayTime = 0;
static bool gbScreenOn = true;

static int gDisplayWidth = 0;
static int gDisplayHeight = 0;
static int gDisplayFrameRate = 60;
static bool gbDisplayFieldMode = false;

static uint8_t gpSessionKey[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
static int gDpuFd = -1;

 //=============================================================================
//                              Private Function Declaration
//=============================================================================
static void itv_lcdISR(void *arg);
static void itv_isp0ISR(void *arg);
static void itv_isp1ISR(void *arg);
static void itv_change_display_format();
static int mtal_x_set_freerun(int freerun);

static int mtal_x_get_freerun(void);

static void msg_queue_init(MessageQueue *q)
{
    memset(q, 0, sizeof(MessageQueue));
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
}

static void msg_queue_flush(MessageQueue *q)
{
    Message *msg, *msg1;
    pthread_mutex_lock(&q->mutex);
    for (msg = q->front; msg!=NULL; msg = msg1)
    {
        msg1 = (Message*) msg->next;
        free(msg);
    }
    q->rear = NULL;
    q->front = NULL;
    q->nb_msgs = 0;
    pthread_mutex_unlock(&q->mutex);
}

static void msg_queue_abort(MessageQueue *q)
{
    pthread_mutex_lock(&q->mutex);
    q->abort_request = 1;
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->mutex);
}

static void msg_queue_deinit(MessageQueue *q)
{
    msg_queue_flush(q);
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->cond);
}

static void msg_queue_put(MessageQueue *q, Message *msg)
{
    Message *msg_keep = (Message*) malloc(sizeof(Message));
    if (!msg_keep) { return; }

    pthread_mutex_lock(&q->mutex);

    msg_keep->event = msg->event;
    msg_keep->data = msg->data;
    msg_keep->next = NULL;

    if (!q->rear)
        q->front = msg_keep;
    else
    {
        Message *pRearMessage = q->rear;
        if (pRearMessage->event == msg_keep->event)
        {
            switch (pRearMessage->event)
            {
                case MSG_SYS_CMD:
                case MSG_MCTL_CMD:
                    if (pRearMessage->data == msg_keep->data)
                    {
                        free(msg_keep);
                        goto end;
                    }
                    else
                    {
                        q->rear->next = (struct Message*) msg_keep;
                    }
                    break;
                case MSG_CHANGE_DISPLAY_FORMAT:
                    if (q->nb_msgs > 0)
                    {
                        //Remove all msg
                        Message* pCurrentMsg = q->front;
                        Message* pFreeMsg = 0;

                        while (pCurrentMsg)
                        {
                            pFreeMsg = pCurrentMsg;
                            pCurrentMsg = (Message*) pCurrentMsg->next;
                            free(pFreeMsg);
                        }
                        q->front = msg_keep;
                        q->nb_msgs = 0;
                    }
                    break;
                default:
                    q->rear->next = (struct Message*) msg_keep;
                    break;
            }
        }
        else
        {
            q->rear->next = (struct Message*) msg_keep;
        }
    }
    q->rear = msg_keep;
    q->nb_msgs++;
end:
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->mutex);
}

static int msg_queue_get(MessageQueue *q, Message *msg)
{
    Message *msg_out;
    int ret = 0;

    pthread_mutex_lock(&q->mutex);

    //while (1)
    {
        if (q->abort_request)
        {
            ret = -1;
            goto end;
        }

        msg_out = q->front;
        if (msg_out)
        {
            q->front = (Message*) msg_out->next;
            if (!q->front)
                q->rear = NULL;
            q->nb_msgs--;
            msg->event = msg_out->event;
            msg->data = msg_out->data;
            free(msg_out);
            ret = 1;
            goto end;
        }
        //else
        //{
        //    pthread_cond_wait(&q->cond, &q->mutex);
        //}
    }
end:
    pthread_mutex_unlock(&q->mutex);
    return ret;
}


static void update_ui()
{
    int i;
    ITV_STRC_FF *ff;
    int         widx, ridx;
    for (i = 0; i < ITV_FF_NSET; i++)
    {
        ff   = &(itv_rcs.ff[i]);
        widx = ff->ui_widx;
        ridx = ff->ui_ridx;

        if (widx != ridx)
        {
            if (itv_rcs.ff_mode_isr[i] == 1)
    		{
                itv_rcs.ui_update = true;

                switch (i)
                {
                case 0:
                    mmpIspEnable(gIspDev, MMP_ISP_FRAME_FUNCTION_0);
					ithPrintf("x = %d, y = %d, width = %d, height = %d, pitch = %d\n", ff->uiprop[ridx].startX, ff->uiprop[ridx].startY, ff->uiprop[ridx].width, ff->uiprop[ridx].height, ff->uiprop[ridx].pitch);
                mmpIspSetFrameFunction(
                    gIspDev,
                    MMP_ISP_FRAME_FUNCTION_0,
                    ff->uibuf[ridx],
                    ff->uiprop[ridx].startX,
                    ff->uiprop[ridx].startY,
                    ff->uiprop[ridx].width,
                    ff->uiprop[ridx].height,
                    ff->uiprop[ridx].pitch,
                    ff->uiprop[ridx].colorKeyR,
                    ff->uiprop[ridx].colorKeyG,
                    ff->uiprop[ridx].colorKeyB,
                    ff->uiprop[ridx].EnAlphaBlend,
                    ff->uiprop[ridx].constAlpha,
                    MMP_PIXEL_FORMAT_ARGB8888,
                            ff->ui_ridx);
                    break;

                case 1:
                    mmpIspEnable(gIspDev, MMP_ISP_FRAME_FUNCTION_1);
                    mmpIspSetFrameFunction(
                            gIspDev,
                            MMP_ISP_FRAME_FUNCTION_1,
                            ff->uibuf[ridx],
                            ff->uiprop[ridx].startX,
                            ff->uiprop[ridx].startY,
                            ff->uiprop[ridx].width,
                            ff->uiprop[ridx].height,
                            ff->uiprop[ridx].pitch,
                            ff->uiprop[ridx].colorKeyR,
                            ff->uiprop[ridx].colorKeyG,
                            ff->uiprop[ridx].colorKeyB,
                            ff->uiprop[ridx].EnAlphaBlend,
                            ff->uiprop[ridx].constAlpha,
                            MMP_PIXEL_FORMAT_ARGB8888,
                            ff->ui_ridx);
                    break;

                default:
                    break;
    			}
			}
        }
        else
        {
            itv_rcs.pcmd_flush_uibuf[i] = 0;			
            if (itv_rcs.ff_mode_isr[i] == 0)
			{				
                switch (i)
    			{
                case 0:
                    mmpIspDisable(gIspDev, MMP_ISP_FRAME_FUNCTION_0);
                    break;
                case 1:
                    mmpIspDisable(gIspDev, MMP_ISP_FRAME_FUNCTION_1);
                    break;

                default:
                    break;
                }
    		}
    	}
    }
}

static uint32_t next_lcd_index()
{
//#if (CFG_CHIP_PKG_IT9852)
    lcd_index = (lcd_index + 1) % 3;
//#else
//    lcd_index = (lcd_index + 1) % 3;
//#endif
    return lcd_index;
}
int abz = 0;
MMP_ISP_SHARE       isp_share = {0};

static void flip_lcd(int lcd_index)
{
    MMP_ISP_OUTPUT_INFO out_info  = {0};
    uint32_t            out_buffer;
    
    ITURotation         rot;

    //memset(&isp_share, 0x0, sizeof(MMP_ISP_SHARE));

//#ifdef FRAME_FUNCTION_ENABLE
    do
    {
        if (itv_rcs.curr_video != &itv_rcs.blank_yuv_frame)
        {
            if ((NULL == itv_rcs.curr_video)
                || (NULL == itv_rcs.curr_video->ya)
                || (NULL == itv_rcs.curr_video->ua)
                || (NULL == itv_rcs.curr_video->va)
                || (0 == itv_rcs.curr_video->src_w)
                || (0 == itv_rcs.curr_video->src_h)
                || (0 == itv_rcs.curr_video->pitch_y)
                || (0 == itv_rcs.curr_video->pitch_uv))
            {
                itv_rcs.curr_video = &itv_rcs.blank_yuv_frame;
                break;
            }

            if (itv_rcs.curr_video->format == 0)
                itv_rcs.curr_video->format = MMP_ISP_IN_NV12;
    	}
    } while (0);

	if(itv_rcs.curr_video != &itv_rcs.blank_yuv_frame && mtal_get_source_lock_flag())
		itv_rcs.curr_video = &itv_rcs.blank_yuv_frame;
		
//#endif
    isp_share.addrY   = (uint32_t)itv_rcs.curr_video->ya;
    isp_share.addrU   = (uint32_t)itv_rcs.curr_video->ua;
    isp_share.addrV   = (uint32_t)itv_rcs.curr_video->va;
    isp_share.width   = itv_rcs.curr_video->src_w;
    isp_share.height  = itv_rcs.curr_video->src_h;
    isp_share.pitchY  = itv_rcs.curr_video->pitch_y;
    isp_share.pitchUv = itv_rcs.curr_video->pitch_uv;
    isp_share.format  = itv_rcs.curr_video->format;
	/*
    printf("addrY: %x\n", isp_share.addrY);
    printf("addrU: %x\n", isp_share.addrU);
    printf("addrV: %x\n", isp_share.addrU);
    printf("W: %d\n", isp_share.width);
    printf("H: %d\n", isp_share.height);
    printf("PY: %d\n", isp_share.pitchY);
    printf("PU: %d\n", isp_share.pitchUv);
    printf("f: %d\n", isp_share.format);
    abz++;*/

#if (CFG_CHIP_FAMILY != 9850 && CFG_CHIP_FAMILY != 970)
    if (IS_ROTATE_DISPLAY())
        {
        _gRoateSurf      = (_gRoateSurf == &gRoateSurf[0]) ? &gRoateSurf[1] : &gRoateSurf[0];
        out_info.addrRGB = (MMP_UINT32)((M2D_SURFACE *)(*_gRoateSurf))->baseScanPtr;
        lcd_index        = (_gRoateSurf == &gRoateSurf[0]) ? 0 : 1;
        }
        else
#endif
        {
        lcd_index        = next_lcd_index();
        out_buffer       = (uint32_t)itv_rcs.dbuf[lcd_index];
        out_info.addrRGB = out_buffer;
        //g_lcd_index      = lcd_index;
        }

#if (CFG_CHIP_FAMILY != 9850 && CFG_CHIP_FAMILY != 970)
    if (IS_VERTICAL_DISPLAY())
        {
        out_info.width    = ithLcdGetHeight();
        out_info.height   = ithLcdGetWidth();
        out_info.pitchRGB = out_info.width * (ithLcdGetPitch() / ithLcdGetWidth());
    }
    else
#endif
    {
         rot = itv_get_rotation();
    
         switch (rot)
        {
         case ITU_ROT_90:
         case ITU_ROT_270:
             out_info.width  = ithLcdGetHeight();
             out_info.height = ithLcdGetWidth();
             out_info.pitchRGB = ithLcdGetPitch() * ithLcdGetHeight() / ithLcdGetWidth();
             break;
             
         default:
             out_info.width    = ithLcdGetWidth();
             out_info.height   = ithLcdGetHeight();
             out_info.pitchRGB = ithLcdGetPitch();
                        break;
                    }
                }
    out_info.format = MMP_ISP_OUT_RGB888;//MMP_ISP_OUT_DITHER565A;
    /*
    printf("OA:%x\n", out_buffer);
    printf("OW:%d\n", out_info.width);
    printf("OH:%d\n", out_info.height);
    printf("OP:%d\n", out_info.pitchRGB);
    */
    {
    if (abz == 0) {
    #if (CFG_CHIP_FAMILY == 9850 || CFG_CHIP_FAMILY == 970)
        #ifndef CFG_CAPTURE_MODULE_ENABLE
            mmpIspSetMode(gIspDev, MMP_ISP_MODE_TRANSFORM);
        #endif
    #endif
        mmpIspSetOutputWindow(gIspDev, &out_info);
        //mmpIspSetVideoWindow(gIspDev, 0, 0, out_info.width, out_info.height);
    #ifdef _WIN32
        mmpIspDisableInterrupt(gIspDev);
    #else
        mmpIspEnableInterrupt(gIspDev);
        mmpIspEnableLCDOnFlyMode(gIspDev);
    #endif
        abz++;
			printf("YC: gogogogo\n");
           mmpIspPlayImageProcess(gIspDev, &isp_share);

                }
    else{
        ///printf("ppp\n");
        mmpIspPlayImageProcess(gIspDev, &isp_share);
            }
        }

    #ifndef CAP_ONFLY_ENABLE
        #ifdef _WIN32
            //printf("VP busy\n");
            SHORT_WAIT_UNTIL(mmpIspIsEngineIdle(gIspDev));
            //printf("VP idle\n");
        #else
            //printf("VP busy\n");
            //if (isp_share.width != 16)
            //sem_wait(&tISPEvent);
            //if (abz == 20) while(1) sleep(5);
            //printf("VP idle (receive interrupt)\n");
        #endif
    #endif

    g_lcd_index      = lcd_index;
    
    #if (CFG_CHIP_FAMILY != 9850 && CFG_CHIP_FAMILY != 970)
        if (IS_ROTATE_DISPLAY())
        {
            MMP_M2D_SURFACE *m2dtempSurf;
            float           angle;
            MMP_INT         dX;
            MMP_INT         dY;
            MMP_INT         cX;
            MMP_INT         cY;
            int             lcd_index = next_lcd_index();

            switch (gRotation)
            {
            default:
                angle = 0;
                break;

            case ITU_ROT_90:
                angle = 90.0f;
                dX    = ithLcdGetWidth() - 1;
                dY    = 0;
                cX    = 0;
                cY    = 0;
                break;

            case ITU_ROT_180:
                angle = 180.0f;
                dX    = 0; // [TODO]: the value is incorrect. It needs to be fixed.
                dY    = 0;
                cX    = 0;
                cY    = 0;
                break;

            case ITU_ROT_270:
                angle = 270.0f;
                dX    = 0;
                dY    = 0;
                cX    = ithLcdGetHeight();
                cY    = 0;
                break;
            }

            m2dtempSurf = &_gLcdSurf[lcd_index];
            //printf("%s(%d)\n", __FUNCTION__, __LINE__);
        #ifdef CFG_M2D_ENABLE
            //printf("%s(%d)\n", __FUNCTION__, __LINE__);
            mmpM2dTransformations(
                *m2dtempSurf,
                dX,
                dY,
                *_gRoateSurf,
                cX,
                cY,
                angle,
                1.0f);

            //flip
            mmpM2dWaitIdle();
            //printf("%s(%d)\n", __FUNCTION__, __LINE__);
        #endif
            g_lcd_index = lcd_index;
        }
    #endif
    //itv_rcs.ui_update    = false;
    itv_rcs.video_update = false;
}

struct timeval	t1;
unsigned long		nowTime, lastTime, elapse;
int abaaa = 0, abbbb = 0;
static void itv_lcdISR(void *arg __attribute__((__unused__)))
{
	int         widx, ridx;
   //ithPrintf("xTaskGetTickCountFromISR:%lu\n", xTaskGetTickCountFromISR());
//lastTime = ithTimerGetCounter(portTIMER);
    if (gtFlipCtrl.bInited)
        sem_post(&gtFlipCtrl.tScanlineEvent);

    ithLcdIntrClear();
/*nowTime = ithTimerGetCounter(portTIMER);
    if (nowTime > lastTime)
        ithPrintf("itv_lcdISR lcdisrclear:%lu ms\n", (nowTime - lastTime)/1000);
    else
        ithPrintf("itv_lcdISR lcdisrclear:%lu ms\n", (configCPU_CLOCK_HZ / configTICK_RATE_HZ - lastTime + nowTime) / 1000);*/

    //mmpIspPlayImageProcess(gIspDev, &isp_share);
    if (abz) {
		if(!itv_rcs.lcd_isr)
        {
            if ((itv_rcs.disp_buf_ridx != itv_rcs.disp_buf_widx))
			     itv_rcs.disp_buf_ridx = (itv_rcs.disp_buf_widx +ITV_MAX_DISP_BUF-1) % ITV_MAX_DISP_BUF;
			itv_rcs.lcd_isr = true;
            }
	    if ((itv_rcs.disp_buf_ridx != itv_rcs.disp_buf_widx))
	    {
	    	if(itv_rcs.update_dbuf_rindex)
	    	{
	    		//ithPrintf("YC: %s, r = %d, w = %d\n", __FUNCTION__, itv_rcs.disp_buf_ridx, itv_rcs.disp_buf_widx);
				itv_rcs.disp_buf_ridx = (itv_rcs.disp_buf_widx +ITV_MAX_DISP_BUF-1) % ITV_MAX_DISP_BUF;
				itv_rcs.update_dbuf_rindex = false;
	    	}
	        itv_rcs.curr_video    = &itv_rcs.dbufprop[itv_rcs.disp_buf_ridx];
	        itv_rcs.disp_buf_ridx = (itv_rcs.disp_buf_ridx + 1) % ITV_MAX_DISP_BUF;
			//ithPrintf("YC: %s, r = %d\n", __FUNCTION__, itv_rcs.disp_buf_ridx);
	        itv_rcs.video_update  = true;
        }
        else
        {
	        itv_rcs.pcmd_flush_dbuf = 0;    // set this flag to 0 means all queued display buffers have been flushed.
        }

//#ifdef FRAME_FUNCTION_ENABLE		
		update_ui();
//#endif

	    if (itv_rcs.video_update || itv_rcs.ui_update)
	    {
	        flip_lcd(itv_rcs.disp_buf_ridx); 

			if (itv_rcs.ui_update)
	        {
	        	int i;
				
	            for (i = 0; i < ITV_FF_NSET; i++)
	            {
	                ITV_STRC_FF *ff = &(itv_rcs.ff[i]);
	                widx = ff->ui_widx;
	                ridx = ff->ui_ridx;

	                if (itv_rcs.ff_mode_isr[i] == 1)
	                    ff->ui_ridx = (ridx + 1) % ITV_FF_NBUF;
	            }
	        }
			itv_rcs.ui_update    = false;
	    }
    }
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
    /* postponed command */
    if (itv_rcs.mtal_pb_mode_u != -1)
    {
    //#ifndef FRAME_FUNCTION_ENABLE
        //if (itv_rcs.mtal_pb_mode_isr == 0 && itv_rcs.mtal_pb_mode_u == 1)
            //itv_rcs.mtal_pb_mode_u = 2;
    //#endif

        itv_rcs.mtal_pb_mode_isr = itv_rcs.mtal_pb_mode_u;
        itv_rcs.mtal_pb_mode_u   = -1;
    }
    
/*#ifndef FRAME_FUNCTION_ENABLE
    if (itv_rcs.mtal_pb_mode_isr == 2)
    {
        itv_rcs.check_video_surf = true;
        itv_rcs.mtal_pb_mode_isr = 1;
        itv_rcs.disp_buf_ridx = 0;
        itv_rcs.disp_buf_widx = 1;
        memcpy((void *)&itv_rcs.dbufprop[itv_rcs.disp_buf_ridx], (void *)&itv_rcs.blank_yuv_frame, sizeof(ITV_DBUF_PROPERTY));
        itv_rcs.vid_surf_ridx = itv_rcs.vid_surf_widx = 0;
    }
#endif*/

//#if defined(CFG_LCD_ENABLE) && defined(FRAME_FUNCTION_ENABLE)
    if (g_lcd_index < 0)
    {
        //ithLcdSwFlip(g_lcd_index);

        if (gtFlipCtrl.bInited)
            sem_wait(&gtFlipCtrl.tScanlineEvent);
        g_lcd_index = -1;
    }
//#endif
    /* FRAME FUNCTION */
    {
		int i;
		
		for (i = 0; i < ITV_FF_NSET; i++)
		{
			//printf("YC: %s, %d itv_rcs.ff_mode_u[i] = %d\n", __FUNCTION__, __LINE__ , itv_rcs.ff_mode_u[i]);
			if (itv_rcs.ff_mode_u[i] != -1)
			{
				itv_rcs.ff_mode_isr[i] = itv_rcs.ff_mode_u[i];
				itv_rcs.ff_mode_u[i]   = -1;
			}
		}

		if (itv_rcs.ff_setbase == 1)
		{
			const int	id	= itv_rcs.ff_setbase_id;
			const int	bid = itv_rcs.ff_setbase_bid;
			ITV_STRC_FF *ff = &(itv_rcs.ff[id]);

			ff->uibuf[bid]	   = itv_rcs.ff_setbase_base;
			itv_rcs.ff_setbase = 0;
		}
	}

    if (abz == 0) {		
    	itv_rcs.video_update = false;
		update_ui();
//#ifdef FRAME_FUNCTION_ENABLE
    //if (itv_rcs.mtal_pb_mode_isr == 0)
    //{
        // flush display buffer?, TODO: state change (play -> stop, stop -> play)
        // itv_rcs.disp_buf_ridx = itv_rcs.disp_buf_widx;
        //itv_rcs.curr_video = &itv_rcs.blank_yuv_frame;
    //}
//#endif

#if 1
	    if ((itv_rcs.disp_buf_ridx != itv_rcs.disp_buf_widx))
		{
	    	itv_rcs.disp_buf_ridx = (itv_rcs.disp_buf_widx +ITV_MAX_DISP_BUF-1) % ITV_MAX_DISP_BUF;
	        itv_rcs.curr_video    = &itv_rcs.dbufprop[itv_rcs.disp_buf_ridx];
			itv_rcs.disp_buf_ridx = (itv_rcs.disp_buf_ridx + 1) % ITV_MAX_DISP_BUF;
	        itv_rcs.video_update  = true;
		}
	    else
		{
	        /* postponed command */
	        itv_rcs.pcmd_flush_dbuf = 0;    // set this flag to 0 means all queued display buffers have been flushed.
	    }

	    if (itv_rcs.video_update || itv_rcs.ui_update)
		{
	        flip_lcd(itv_rcs.disp_buf_ridx);
		}
#endif

    }
}

#ifdef _WIN32
static g_stop_lcd_isr_task;

static void *lcd_isr_task(void *arg)
{
    #ifdef DEBUG_FLIP
        printf("[LCD ISR] Enter\n");
#endif

    g_stop_lcd_isr_task = false;
    while (!g_stop_lcd_isr_task)
    {
        itv_lcdISR(arg);
        usleep(16000);
    }

    #ifdef DEBUG_FLIP
        printf("[LCD ISR] Leave\n");
#endif
    pthread_exit(NULL);
    return (void *)0;
}
#endif // _WIN32

static void *itv_hw_overlay_task(void *arg)
    {
    g_stop_itv_hw_overlay_task = false;
    while (!g_stop_itv_hw_overlay_task || itv_rcs.mtal_pb_mode_isr != 0)
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
	{
        MMP_ISP_CORE_INFO   ISPCOREINFO = {0};
        MMP_ISP_OUTPUT_INFO outInfo  = {0};
        int width  = ithLcdGetWidth();
        int height = ithLcdGetHeight();
        ITURotation     rot;

        gDisplayWidth = width;
        gDisplayHeight = height;

        rot = itv_get_rotation();

        switch (rot)
    	{
        case ITU_ROT_90:
        case ITU_ROT_270:
            width  = ithLcdGetHeight();
            height = ithLcdGetWidth();
            break;

        default:
            width = ithLcdGetWidth();
            height = ithLcdGetHeight();
            break;
        }

        mmpIspInitialize(&gIspDev, MMP_ISP_CORE_0);
        ISPCOREINFO.EnPreview = false;
        ISPCOREINFO.PreScaleSel = MMP_ISP_PRESCALE_NORMAL;
        mmpIspSetCore(gIspDev, &ISPCOREINFO);
        mmpIspSetMode(gIspDev, MMP_ISP_MODE_TRANSFORM);
        mmpIspSetDisplayWindow(gIspDev, 0, 0, width, height);

		if(gbDisplayFieldMode)
		{
			itv_enable_isp_feature(MMP_ISP_FIELD_SCALE_MODE);
			itv_enable_isp_feature(MMP_ISP_KEEP_LAST_FIELD_MODE);
		}

#ifdef CAP_ONFLY_ENABLE
        mmpIspEnableCAPOnFlyMode(gIspDev);
#endif
        mmpIspSetOutputFormat(gIspDev, MMP_ISP_OUT_DITHER565A);
    //#ifdef FRAME_FUNCTION_ENABLE
        //ithLcdSwFlip(ithLcdGetFlip());
        //ithLcdDisableHwFlip();
    //#endif
    }
#endif

    if (!gtFlipCtrl.bInited)
        gtFlipCtrl.bInited = !sem_init(&gtFlipCtrl.tScanlineEvent, 0, 1);

    sem_init(&tISPEvent, 0, 1);
    sem_wait(&tISPEvent);

    if (gtFlipCtrl.bInited)
    {
        g_lcd_index = -1;

#ifndef WIN32
        //#ifdef FRAME_FUNCTION_ENABLE
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
        //#endif
        /* enable VP ISR */
    	{
            mmpISPRegisterIRQ(gIspDev, itv_isp0ISR);
            //mmpISPRegisterIRQ(gIspDev, itv_isp1ISR);
    	}
        pthread_create(&gtFlipCtrl.hw_overlay_tid, NULL, itv_hw_overlay_task, NULL);
    #else
        pthread_create(&gtFlipCtrl.lcd_isr_tid, NULL, lcd_isr_task, NULL);
#endif
	}
}

int itv_init(void)
{
    int i;
    uint32_t tmp;

    printf("ITV# init !\n");

    /* sanity check */
#if 0
    if (ITV_MAX_NDBUF > 3)
    {
        printf("ERROR# invalid ITV_MAX_NDBUF (%d), %s:%d !\n", ITV_MAX_NDBUF, __FILE__, __LINE__);
        return -1;
    }
#endif
    g_blank_buf = itpVmemAlloc(16*16*2);
    
    // initialize the blank YUV frame's value
    memset((void *)&itv_rcs, 0, sizeof(ITV_STRC_RESOURCE));

	itv_rcs.lcd_isr = false;
	itv_rcs.update_dbuf_rindex = false;
	itv_rcs.disp_buf_ridx = itv_rcs.disp_buf_widx = 0;
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
    itv_rcs.blank_yuv_frame.format   = MMP_ISP_IN_NV12;
    itv_rcs.curr_video               = &itv_rcs.blank_yuv_frame;
#ifdef CFG_LCD_ENABLE
    itv_rcs.dbuf[0]                  = (uint8_t *)ithLcdGetBaseAddrA();
    itv_rcs.dbuf[1]                  = (uint8_t *)ithLcdGetBaseAddrB();
    itv_rcs.dbuf[2]                  = (uint8_t *)ithLcdGetBaseAddrC();
    printf("initia 0:%x\n", itv_rcs.dbuf[0]);
    printf("initia 1:%x\n", itv_rcs.dbuf[1]);
    printf("initia 2:%x\n", itv_rcs.dbuf[2]);
    lcd_index                        = ithLcdGetFlip();
#endif
    /* MTAL */
        itv_rcs.mtal_pb_mode_u = -1;
    //-
    itv_rcs.vid_surf_cur_idx         = -1;

    /* FRAME FUNCTION */
        for(i=0; i<ITV_FF_NSET; i++)
            itv_rcs.ff_mode_u[i] = -1;

    itv_change_display_format();

	{
        gDpuFd = open(":dpu:aes", 0);
        if (gDpuFd < 0)
        {
            printf("DPU init is failed\n");
            while(1);
        }
        else
        {
            int keyLen = 128;
            uint8_t cipherMode = ITP_DPU_CIPHER_ECB;
            uint8_t decryMode = 1;
            printf("DPU init is success: %d\n", gDpuFd);
            {
                int i = 0;
                printf("RX Session Key is:.......\n");
                for (i = 0; i < 16; i++)
                {
                    printf("0x%02X, ", gpSessionKey[i]);
                }
                printf("\n");
            }
    		ioctl(gDpuFd, ITP_IOCTL_SET_KEY_LENGTH, &keyLen); //key.len, key.ptr
    		ioctl(gDpuFd, ITP_IOCTL_SET_KEY_BUFFER, gpSessionKey); //key.len, key.ptr
    		ioctl(gDpuFd, ITP_IOCTL_SET_CIPHER, &cipherMode); //key.len, key.ptr
    		ioctl(gDpuFd, ITP_IOCTL_SET_DESCRYPT, &decryMode); //key.len, key.ptr
        }
    }

    /* sync module */
    //fc_init_sync(&itv_rcs.itv_sync);    // [ToDO] inin sync module should be done in the player, not here

    return 0;
}

int itv_deinit(void)
{
    printf("ITV# de-init !\n");

    if (gtFlipCtrl.bInited)
	{
        g_stop_itv_hw_overlay_task = true;
        pthread_join(gtFlipCtrl.hw_overlay_tid, NULL);

    #ifndef WIN32
        //#ifdef FRAME_FUNCTION_ENABLE
            ithLcdIntrDisable();
        //#endif
    #else
        g_stop_lcd_isr_task = true;
        pthread_join(gtFlipCtrl.lcd_isr_tid, NULL);
#endif

        sem_destroy(&(gtFlipCtrl.tScanlineEvent));
        memset(&gtFlipCtrl, 0, sizeof(gtFlipCtrl));

        sem_destroy(&tISPEvent);
        memset(&tISPEvent, 0, sizeof(tISPEvent));
	}

#ifdef CFG_LCD_ENABLE
    mmpIspTerminate(&gIspDev);
#endif

    /* sync module */
    //fc_deinit_sync(&itv_rcs.itv_sync);

    memset((void*)&itv_rcs, 0, sizeof(ITV_STRC_RESOURCE));
    
    itpVmemFree(g_blank_buf);
	//ithIspDisableClock();
	abz = 0;
    return 0;
}

void itv_cleanup_screen()
{
	itv_rcs.curr_video = &itv_rcs.blank_yuv_frame;
}

uint8_t *itv_get_dbuf_anchor(void)
{
    //if (itv_rcs.disp_buf_ridx == 0)
    //{
    //  if (QUEUE_IS_FULL(itv_rcs.disp_buf_widx, ITV_MAX_DISP_BUF, ITV_MAX_DISP_BUF))
    //    return NULL;
    //}
    //else
    {
     if (QUEUE_IS_FULL(itv_rcs.disp_buf_widx, itv_rcs.disp_buf_ridx, ITV_MAX_DISP_BUF))
     {
        return NULL;
     }
    }

    return 1;
}

int itv_update_dbuf_anchor(ITV_DBUF_PROPERTY *prop)
{
    //if (itv_rcs.disp_buf_ridx == 0)
    //{
    // if (QUEUE_IS_FULL(itv_rcs.disp_buf_widx, ITV_MAX_DISP_BUF, ITV_MAX_DISP_BUF))
    //    return -1;
    //}
    //else
    //printf("YC: r = %d, w = %d\n", itv_rcs.disp_buf_ridx , itv_rcs.disp_buf_widx);
	
    {
     if (QUEUE_IS_FULL(itv_rcs.disp_buf_widx, itv_rcs.disp_buf_ridx , ITV_MAX_DISP_BUF))
        return -1;
    }

    memcpy((void *)&itv_rcs.dbufprop[itv_rcs.disp_buf_widx], (void *)prop, sizeof(ITV_DBUF_PROPERTY));
    itv_rcs.disp_buf_widx = (itv_rcs.disp_buf_widx + 1) % ITV_MAX_DISP_BUF;

    return 0;
}

bool itv_get_lcd_isr_flag()
{       
	return itv_rcs.lcd_isr;
}

bool itv_set_update_dbuf_rindex_flag()
{
	itv_rcs.update_dbuf_rindex = true;
}

void itv_flush_dbuf(void)
    {
    itv_rcs.pcmd_flush_dbuf = 1;

    WAIT_UNTIL(itv_rcs.pcmd_flush_dbuf == 0);
}

/* video surface */
void itv_set_vidSurf_buf(uint8_t* addr, uint8_t index)
{
    //itv_rcs.dbuf[index] = addr;
}

bool itv_get_new_video(void)
{
    if (itv_rcs.vid_surf_ridx != itv_rcs.vid_surf_widx)
        return true;
    else
        return false;
}

int itv_get_vidSurf_index(void)
{
    if (itv_rcs.vid_surf_ridx != itv_rcs.vid_surf_widx)
{
      itv_rcs.vid_surf_ridx = (itv_rcs.vid_surf_ridx + 1) % ITV_MAX_VSBUF;
      itv_rcs.vid_surf_cur_idx = g_lcd_index;
}

    if (itv_rcs.mtal_pb_mode_isr != 1)
        return -1;
    else if((itv_rcs.curr_video)->src_w == 16)
        return -2;

    return itv_rcs.vid_surf_cur_idx;
}

int itv_check_vidSurf_anchor(void)
{
//#ifndef FRAME_FUNCTION_ENABLE
  //  if (QUEUE_IS_FULL(itv_rcs.vid_surf_widx, itv_rcs.vid_surf_ridx, ITV_MAX_VSBUF) && itv_rcs.check_video_surf)
    //    return -1;
//#endif
    return 1;
}

int itv_update_vidSurf_anchor(void)
{
//#ifndef FRAME_FUNCTION_ENABLE
  //  itv_rcs.vid_surf_widx = (itv_rcs.vid_surf_widx + 1) % ITV_MAX_VSBUF;
//#endif
    return 0;
}

void itv_stop_vidSurf_anchor(void)
{
//#ifndef FRAME_FUNCTION_ENABLE
  //  itv_rcs.check_video_surf = false;
//#endif
}

/* FRAME FUNCTION */
int itv_ff_setup_base(int id, int bid, uint8_t *base)
    {
    itv_rcs.ff_setbase_id   = id;
    itv_rcs.ff_setbase_bid  = bid;
    itv_rcs.ff_setbase_base = base;
    itv_rcs.ff_setbase      = 1;

    WAIT_UNTIL(itv_rcs.ff_setbase == 0);

    return 0;
}

int itv_ff_enable(int id, int enable)
{
    printf("ITV# %s, id(%d):%d +\n", __func__, id, enable);

    itv_flush_uibuf(id);
    itv_rcs.ff_mode_u[id] = enable;
	
    WAIT_UNTIL(itv_rcs.ff_mode_u[id] == -1);

    printf("ITV# %s, id(%d):%d -\n", __func__, id, enable);
    return 0;
}

uint8_t *itv_get_uibuf_anchor(int id)
{
    ITV_STRC_FF *ff  = &(itv_rcs.ff[id]);
    const int   widx = ff->ui_widx;
    const int   ridx = ff->ui_ridx;

    if (QUEUE_IS_FULL(widx, ridx, ITV_FF_NBUF))
        return NULL;

    return ff->uibuf[widx];
}

int itv_update_uibuf_anchor(int id, ITV_UI_PROPERTY *uiprop)
{
    ITV_STRC_FF *ff  = &(itv_rcs.ff[id]);
    const int   widx = ff->ui_widx;
    const int   ridx = ff->ui_ridx;

    if (QUEUE_IS_FULL(widx, ridx, ITV_FF_NBUF))
        return -1;

    memcpy((void *)&(ff->uiprop[widx]), (void *)uiprop, sizeof(ITV_UI_PROPERTY));
    ff->ui_widx = (widx + 1) % ITV_FF_NBUF;
    return 0;
}

void itv_flush_uibuf(int id)
{
    itv_rcs.pcmd_flush_uibuf[id] = 1;

    WAIT_UNTIL(itv_rcs.pcmd_flush_uibuf[id] == 0);
}

// pb_mode: a boolean value,
//          1 means in playback mode, 0 means not in playback mode
int itv_set_pb_mode(int pb_mode)
{
    printf("%s(%d) pb_mode(%d)\n", __FUNCTION__, __LINE__, pb_mode);

    WAIT_UNTIL(itv_rcs.mtal_pb_mode_u == -1);
    itv_rcs.mtal_pb_mode_u = pb_mode;
    SHORT_WAIT_UNTIL(itv_rcs.mtal_pb_mode_u == -1);

    printf("%s(%d)\n", __FUNCTION__, __LINE__);
    return itv_rcs.mtal_pb_mode_u;
}

ITURotation itv_get_rotation(void)
{
    return gRotation;
}

void itv_set_rotation(ITURotation rot)
{
    gRotation = rot;

#if (CFG_CHIP_FAMILY != 9850 && CFG_CHIP_FAMILY != 970)
    // create rotation buffer and sruface
    if (IS_ROTATE_DISPLAY() && (!gRotateBuffer[0]))
    {
        int i;
        int size = ithLcdGetPitch() * ithLcdGetHeight() * 2;
        gRotateBuffer[0] = itpVmemAlloc(size);

        //assert(gRotateBuffer[0]);

        for (i = 1; i < MAX_ROTATE_BUFFER_COUNT; ++i)
        {
            gRotateBuffer[i] = gRotateBuffer[i - 1] + ithLcdGetPitch() * ithLcdGetHeight();
        }

        printf("Rotate tmpbut %x %x\n", gRotateBuffer[0], gRotateBuffer[1]);
    #ifdef CFG_M2D_ENABLE
        for (i = 0; i < MAX_ROTATE_BUFFER_COUNT; ++i)
       	{
            mmpM2dCreateVirtualSurface(
                ithLcdGetHeight(),
                ithLcdGetWidth(),
                MMP_M2D_IMAGE_FORMAT_RGB565,
                (MMP_UINT8 *)gRotateBuffer[i],
                &gRoateSurf[i]);
            printf("%X\n", (void *)((M2D_SURFACE *)gRoateSurf[i])->baseScanPtr);
        }
        for (i = 0; i < 3; ++i)
        {
            mmpM2dCreateVirtualSurface(
                ithLcdGetWidth(),
                ithLcdGetHeight(),
                MMP_M2D_IMAGE_FORMAT_RGB565,
                itv_rcs.dbuf[i],
                &_gLcdSurf[i]);
        }
#endif
	}
#endif
}

void
itv_set_video_window(
    uint32_t startX,
    uint32_t startY,
    uint32_t width,
    uint32_t height)
{
    mmpIspSetVideoWindow(gIspDev, startX, startY, width, height);
}

ISP_RESULT
itv_enable_isp_feature(
    MMP_ISP_CAPS cap)
{
    return mmpIspEnable(gIspDev, cap);
}

int itv_set_output_format(MMP_CAP_INPUT_INFO df, uint32_t audio_sample_rate)
{
	int newDisplayWidth = 0;
	int newDisplayHeight = 0;
	bool bChangeDisplay = true;
	itv_rcs.df = df;
	mtal_cal_display_area(&newDisplayWidth, &newDisplayHeight);
	if (newDisplayWidth != itv_rcs.displayWidth || newDisplayHeight != itv_rcs.displayHeight)
	{
        printf("new width: %u, new height: %u\n", newDisplayWidth, newDisplayHeight);
        itv_rcs.displayWidth = newDisplayWidth;
        itv_rcs.displayHeight = newDisplayHeight;

        gDisplayWidth = newDisplayWidth;
        gDisplayHeight = newDisplayHeight;
	}

	//itv_rcs.audio_sample_rate = audio_sample_rate;

	if (bChangeDisplay)
	{
		//ithHDMITXDisable();
		//itv_deinit();
		ituFrameFuncExit();
		ithLcdReset();
		gbDisplayFieldMode = false;	
		switch(df)
		{
			case MMP_CAP_INPUT_INFO_640X480_60P:
				ithLcdLoadScriptFirst(tv480p60VGAScript, ITH_COUNT_OF(tv480p60VGAScript));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_VESA, HDMITX_VESA_640x480p60, TRUE, FALSE);
                gDisplayFrameRate = 60;
				break;
			case MMP_CAP_INPUT_INFO_800X600_60P:
				ithLcdLoadScriptFirst(tv800x600p60VGAScript, ITH_COUNT_OF(tv800x600p60VGAScript));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_VESA, HDMITX_VESA_800x600p60, TRUE, FALSE);
                gDisplayFrameRate = 60;
				break;
			case MMP_CAP_INPUT_INFO_1024X768_60P:
				ithLcdLoadScriptFirst(tv1024x768p60VGAScript, ITH_COUNT_OF(tv1024x768p60VGAScript));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_VESA, HDMITX_VESA_1024x768p60, TRUE, FALSE);
                gDisplayFrameRate = 60;
				break;
			case MMP_CAP_INPUT_INFO_1280X768_60P:
				ithLcdLoadScriptFirst(tv1280x768p60VGAScript, ITH_COUNT_OF(tv1280x768p60VGAScript));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_VESA, HDMITX_VESA_1280x768p60, TRUE, FALSE);
                gDisplayFrameRate = 60;
				break;
			case MMP_CAP_INPUT_INFO_1280X960_60P:
				ithLcdLoadScriptFirst(tv1280x960p60VGAScript, ITH_COUNT_OF(tv1280x960p60VGAScript));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_VESA, HDMITX_VESA_1280x960p60, TRUE, FALSE);
                gDisplayFrameRate = 60;
				break;
			case MMP_CAP_INPUT_INFO_1280X1024_60P:
				ithLcdLoadScriptFirst(tv1280x1024p60VGAScript, ITH_COUNT_OF(tv1280x1024p60VGAScript));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_VESA, HDMITX_VESA_1280x1024p60, TRUE, FALSE);
                gDisplayFrameRate = 60;
				break;
			case MMP_CAP_INPUT_INFO_1360X768_60P:
				ithLcdLoadScriptFirst(tv1360x768p60VGAScript, ITH_COUNT_OF(tv1360x768p60VGAScript));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_VESA, HDMITX_VESA_1360X768p60, TRUE, FALSE);
                gDisplayFrameRate = 60;
				break;
			case MMP_CAP_INPUT_INFO_1440X900_60P:
				ithLcdLoadScriptFirst(tv1440x900p60VGAScript, ITH_COUNT_OF(tv1440x900p60VGAScript));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_VESA, HDMITX_VESA_1440x900p60, TRUE, FALSE);
                gDisplayFrameRate = 60;
				break;
			case MMP_CAP_INPUT_INFO_1600X900_60P:
				ithLcdLoadScriptFirst(tv1600x900p60VGAScript, ITH_COUNT_OF(tv1600x900p60VGAScript));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_VESA, HDMITX_VESA_1600x900p60, TRUE, FALSE);
                gDisplayFrameRate = 60;
				break;
			case MMP_CAP_INPUT_INFO_1680X1050_60P:
				ithLcdLoadScriptFirst(tv1680x1050p60VGAScript, ITH_COUNT_OF(tv1680x1050p60VGAScript));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_VESA, HDMITX_VESA_1680x1050p60, TRUE, FALSE);
                gDisplayFrameRate = 60;
				break;	
			case MMP_CAP_INPUT_INFO_720X480_59P:
				ithLcdLoadScriptFirst(tv480p59Script, ITH_COUNT_OF(tv480p59Script));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_480p60, HDMITX_CEA_720x480p60, TRUE, FALSE);
                gDisplayFrameRate = 59;
				break;
			case MMP_CAP_INPUT_INFO_720X480_60P:
				ithLcdLoadScriptFirst(tv480p60Script, ITH_COUNT_OF(tv480p60Script));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_480p60, HDMITX_CEA_720x480p60, TRUE, FALSE);
                gDisplayFrameRate = 60;
				break;
			case MMP_CAP_INPUT_INFO_720X480_59I:
				ithLcdLoadScriptFirst(tv480i59Script, ITH_COUNT_OF(tv480i59Script));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_480i60, HDMITX_CEA_720x480i60, TRUE, FALSE);
                gDisplayFrameRate = 59;
				gbDisplayFieldMode = true;
				break;
			case MMP_CAP_INPUT_INFO_720X480_60I:
				ithLcdLoadScriptFirst(tv480i60Script, ITH_COUNT_OF(tv480i60Script));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_480i60, HDMITX_CEA_720x480i60, TRUE, FALSE);
                gDisplayFrameRate = 60;
				gbDisplayFieldMode = true;
				break;
			case MMP_CAP_INPUT_INFO_720X576_50P:
				ithLcdLoadScriptFirst(tv576p50Script, ITH_COUNT_OF(tv576p50Script));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_576p50, HDMITX_CEA_720x576p50, TRUE, FALSE);
                gDisplayFrameRate = 50;
				break;	
			case MMP_CAP_INPUT_INFO_720X576_50I:
				ithLcdLoadScriptFirst(tv576i50Script, ITH_COUNT_OF(tv576i50Script));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_576i50, HDMITX_CEA_720x576i50, TRUE, FALSE);
                gDisplayFrameRate = 50;
				gbDisplayFieldMode = true;
				break;
			case MMP_CAP_INPUT_INFO_1280X720_50P:
				ithLcdLoadScriptFirst(tv720p50Script, ITH_COUNT_OF(tv720p50Script));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_720p50, HDMITX_CEA_1280x720p50, TRUE, FALSE);
                gDisplayFrameRate = 50;
                break;
			case MMP_CAP_INPUT_INFO_1280X720_59P:
				ithLcdLoadScriptFirst(tv720p59Script, ITH_COUNT_OF(tv720p59Script));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_720p60, HDMITX_CEA_1280x720p60, TRUE, FALSE);
                gDisplayFrameRate = 59;
                break;	
			case MMP_CAP_INPUT_INFO_1280X720_60P:
				ithLcdLoadScriptFirst(tv720p60Script, ITH_COUNT_OF(tv720p60Script));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
			    ithHDMITXSetDisplayOption(HDMITX_720p60, HDMITX_CEA_1280x720p60, TRUE, FALSE);
                gDisplayFrameRate = 60;
				break;
			case MMP_CAP_INPUT_INFO_1920X1080_50P:
				ithLcdLoadScriptFirst(tv1080p50Script, ITH_COUNT_OF(tv1080p50Script));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_1080p50, HDMITX_CEA_1920x1080p50, TRUE, FALSE);
                gDisplayFrameRate = 50;
				break;	
			case MMP_CAP_INPUT_INFO_1920X1080_59P:
				ithLcdLoadScriptFirst(tv1080p59Script, ITH_COUNT_OF(tv1080p59Script));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_1080p60, HDMITX_CEA_1920x1080p60, TRUE, FALSE);
                gDisplayFrameRate = 59;
				break;	
			case MMP_CAP_INPUT_INFO_1920X1080_60P:
				ithLcdLoadScriptFirst(tv1080p60Script, ITH_COUNT_OF(tv1080p60Script));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_1080p60, HDMITX_CEA_1920x1080p60, TRUE, FALSE);
                gDisplayFrameRate = 60;
				break;
			case MMP_CAP_INPUT_INFO_1920X1080_50I:
				ithLcdLoadScriptFirst(tv1080i50Script, ITH_COUNT_OF(tv1080i50Script));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_1080i50, HDMITX_CEA_1920x1080i50, TRUE, FALSE);
                gDisplayFrameRate = 50;
				gbDisplayFieldMode = true;
				break;	
			case MMP_CAP_INPUT_INFO_1920X1080_59I:
				ithLcdLoadScriptFirst(tv1080i59Script, ITH_COUNT_OF(tv1080i59Script));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_1080i60, HDMITX_CEA_1920x1080i60, TRUE, FALSE);
                gDisplayFrameRate = 59;
				gbDisplayFieldMode = true;
				break;
			case MMP_CAP_INPUT_INFO_1920X1080_60I:
				ithLcdLoadScriptFirst(tv1080i60Script, ITH_COUNT_OF(tv1080i60Script));
				ithWriteRegMaskA(0xD1000160, 0x110, 0x110);  //reset i2c gpio (65, 66)
				ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
				ithHDMITXSetDisplayOption(HDMITX_1080i60, HDMITX_CEA_1920x1080i60, TRUE, FALSE);
                gDisplayFrameRate = 60;
				gbDisplayFieldMode = true;
				break;				
		}
		usleep(3000);
		ithIspResetAllReg();
		ithIspResetReg(ITH_ISP_CORE0);
		//itv_init();
		ituFrameFuncInit();
	}
	return 0;
}

int mtal_pb_open(MTAL_SPEC *spec) /* open for later playback */
{
    printf("MTAL# %s +\n", __func__);

    /* mtal routine */
    {
        const int pb_mode = mtal_get_pb_mode();

        if(((pb_mode >> 4) & 0x1) == 1)
        {
            printf("ERROR# MTAL IS ALREADY IN PB_MODE !\n");
            printf("MTAL# %s -\n", __func__);
            return -1;
        }

        itv_flush_dbuf();

        mtal_pb_set_livesrc(0);
		//mtal_x_set_freerun(0);
		//itv_reset_property();
    }

    /* TODO: add playback routine here */
    if (media_player && media_player->init)
    {
        // ...
		int retv = media_player->init(NORMAL_PLAYER);
        if (retv < 0) SERR();
    }

    /* mtal routine */
    if (spec != NULL)
        memcpy((void*)&itv_rcs.mtal_spec, (void*)spec, sizeof(MTAL_SPEC));
    //-

    printf("MTAL# %s -\n", __func__);
    return 0;
}

int mtal_pb_close(void) /* close for termination of playback */
{
    printf("MTAL# %s +\n", __func__);

    /* TODO: add playback routine here */
    if (0) {
        // ...
        int retv = media_player->deinit();
        if (retv < 0) SERR();
    }

    /* mtal routine */
    itv_flush_dbuf();
    //mtal_set_fake_ir(0x30);
    //-

    printf("MTAL# %s -\n", __func__);
    return 0;
}

int mtal_pb_play(void)
{
    printf("MTAL# %s +\n", __func__);

    /* mtal routine */
    itv_flush_dbuf();
    mtal_set_pb_mode(0x11);

	/* TODO: add playback routine here */
    {
        int i, retv;
        for (i=0; i<itv_rcs.mtal_spec.n_strm; i++) {
            char *src_name = itv_rcs.mtal_spec.strm_spec[i].srcname;
            unsigned src_w = itv_rcs.mtal_spec.strm_spec[i].src_w;
            unsigned src_h = itv_rcs.mtal_spec.strm_spec[i].src_h;
            if (src_w && src_h)
                media_player->select(src_name, src_w, src_h);
            else
                media_player->select(src_name, 0, 0);
            usleep(8000); //XXX
        }

		retv = media_player->play();
        if (retv < 0) SERR();
    }

    /* mtal routine */
    //mtal_set_fake_ir(0x30);

    printf("MTAL# %s -\n", __func__);
    return 0;
}

int mtal_pb_pause(int pause_flag)
{
    printf("ITV# %s(%d) +\n", __func__, pause_flag);
    while(itv_rcs.mtal_pause_u != -1) { usleep(40000); }
    itv_rcs.mtal_pause_u = pause_flag;
    while(itv_rcs.mtal_pause_u != -1) { usleep(40000); }
	//itv_reset_property();
    printf("ITV# %s(%d) -\n", __func__, pause_flag);
    return 0;
}

int mtal_pb_stop(void)
{
    printf("MTAL# %s +\n", __func__);

    /* mtal routine */
	#if 0
    {
        const int pb_mode = mtal_get_pb_mode();
        if(((pb_mode >> 4) & 0x1) == 0)
        {
            printf("ERROR# MTAL IS NOT IN PB_MODE !\n");
            printf("MTAL# %s -\n", __func__);
            return -1;
        }

		mtal_pb_pause(0);
    }
	#endif
    //-
    //mtal_set_switch_flag(1);

    /* TODO: add playback routine here */
    {
		int retv = media_player->stop();
		if (retv < 0) SERR();
    }

    /* mtal routine */
    itv_flush_dbuf();
    mtal_set_pb_mode(0x00);
    //-

    printf("MTAL# %s -\n", __func__);
    return 0;
}

int mtal_pb_set_livesrc(int livesrc)
{
    itv_rcs.mtal_pb_livesrc = livesrc;
    return 0;
}

int mtal_pb_is_livesrc(void)
{
    return itv_rcs.mtal_pb_livesrc;
}

void mtal_set_pb_mode(int pb_mode)
{
#ifndef WIN32
    while(itv_rcs.mtal_pb_mode_u != -1) { usleep(20000); }
#endif
    itv_rcs.mtal_pb_mode_u = pb_mode;
#ifndef WIN32
    while(itv_rcs.mtal_pb_mode_u != -1) { usleep(20000); }
#endif
}

unsigned mtal_get_pb_mode(void)
{
    return itv_rcs.mtal_pb_mode_isr;
}

static int mtal_x_set_freerun(int freerun)
{
    while(itv_rcs.mtal_freerun_u != -1) { usleep(20000); }
    itv_rcs.mtal_freerun_u = freerun;
#ifndef WIN32
    while(itv_rcs.mtal_freerun_u != -1) { usleep(20000); }
#endif
    return 0;
}

static int mtal_x_get_freerun(void)
{
    return itv_rcs.mtal_freerun_isr;
}

int mtal_set_component(ITV_COMPONENT_TYPE type, FF_COMPONENT *comp)
{
    int ret = 0;

    switch (type)
    {
        case ITV_SRC_COMPONENT:
            memcpy(&itv_rcs.src_component, comp, sizeof(FF_COMPONENT));
            break;
        case ITV_DEMUX_COMPONENT:
            memcpy(&itv_rcs.demux_component, comp, sizeof(FF_COMPONENT));
            break;
        case ITV_DECODER_COMPONENT:
            memcpy(&itv_rcs.decode_component, comp, sizeof(FF_COMPONENT));
            break;
        default:
            ret = -1;
            break;
    }
    return ret;
}

int mtal_set_dataInputStatus(bool bHasData)
{
    gbHasDataInput = bHasData;
}

bool mtal_get_dataInputStatus()
{
    return gbHasDataInput;
}

void mtal_get_source_path(char* pOutputPath)
{
    strcpy(pOutputPath, itv_rcs.mtal_spec.strm_spec[0].srcname);
}

void mtal_set_source_path(char* pSrcPath)
{
    strcpy(itv_rcs.mtal_spec.strm_spec[0].srcname, pSrcPath);
}

int mtal_ev_handler_init(void)
{
    if (handle) { return 1; }

    handle = (MTAL_EV_HANDLER*) calloc(sizeof(char), sizeof(MTAL_EV_HANDLER));
    if (!handle)
	{
        printf("MTAL# handler init failed\n");
    	return -1;
	}

    msg_queue_init(&handle->msg_q);
    //pthread_create(&handle->msg_t, NULL, message_handler, (void*)handle);
    handle->init = 1;

    return 0;                                                          
}                                                                      

int mtal_ev_handler_deinit(void)
{
    if (!handle) { return -1; }

    msg_queue_abort(&handle->msg_q);
    pthread_join(handle->msg_t, NULL);
    msg_queue_deinit(&handle->msg_q);
    handle->init = 0;
    free(handle);

    return 0;
}

int mtal_put_message(ITV_EVENT_MESSAGE event, int data)
{
    Message msg1, *msg = &msg1;

    if (!handle) { return -1; }

    msg->event = event;
    msg->data = data;
    msg_queue_put(&handle->msg_q, msg);

    return 0;
}

void mtal_set_switch_flag(int flag)
{
    switch_changed = flag;
}

int mtal_get_switch_flag(void)
{
    return switch_changed;
}

void mtal_set_delay_time(short delayTime)
{
    gDelayTime = delayTime;
}

short mtal_get_delay_time()
{
    return gDelayTime;
}

bool mtal_get_screen_mode_flag(void)
{
    return gbScreenOn;
}

int mtal_set_screen_mode_flag(bool bEnableScreen)
{
    gbScreenOn = bEnableScreen;
}

void mtal_cal_display_area(int *width, int *height)
{
    switch (itv_rcs.df)
    {
        case ITV_1080i50        :
        case ITV_1080i59        :
        case ITV_1080i60        :
        case ITV_1080p23        :
        case ITV_1080p24        :
        case ITV_1080p25        :
        case ITV_1080p29        :
        case ITV_1080p30        :
        case ITV_1080p50        :
        case ITV_1080p59        :
        case ITV_1080p60        :
            *width  = 1920;
            *height = 1080;
            break;
        case ITV_720p50         :
            *width  = 1280;
            *height = 720;
            break;
        case ITV_720p59         :
        case ITV_720p60         :
            *width  = 1280;
            *height = 720;
            break;
        case ITV_480p59         :
        case ITV_480i59         :
        case ITV_480p60         :
        case ITV_480i60         :
            *width  = 720;
            *height = 480;
            break;
        case ITV_576p50         :
        case ITV_576i50         :
            *width  = 720;
            *height = 576;
            break;
        case ITV_640x480p60     :
            *width  = 640;
            *height = 480;
            break;
        case ITV_800x600p60     :
            *width  = 800;
            *height = 600;
            break;
        case ITV_1024x768p60     :
            *width  = 1024;
            *height = 768;
            break;
        case ITV_1280x768p60     :
            *width  = 1280;
            *height = 768;
            break;
        case ITV_1280x800p60     :
            *width  = 1280;
            *height = 800;
            break;
        case ITV_1280x960p60     :
            *width  = 1280;
            *height = 960;
            break;
        case ITV_1280x1024p60    :
            *width  = 1280;
            *height = 1024;
            break;
        case ITV_1360x768p60    :
            *width  = 1360;
            *height = 768;
            break;
        case ITV_1366x768p60    :
            *width  = 1366;
            *height = 768;
            break;
        case ITV_1440x900p60    :
            *width  = 1440;
            *height = 900;
            break;
        case ITV_1400x1050p60    :
            *width  = 1400;
            *height = 1050;
            break;
        case ITV_1440x1050p60    :
            *width  = 1440;
            *height = 1050;
            break;
        case ITV_1600x900p60    :
            *width  = 1600;
            *height = 900;
            break;
        case ITV_1600x1200p60    :
            *width  = 1600;
            *height = 1200;
            break;
        case ITV_1680x1050p60    :
            *width  = 1680;
            *height = 1050;
            break;
        default:
            *width = 720;
            *height = 480;
            break;
    }
}


int mtal_message_handler(void)
{
	Message msg1, *msg = &msg1;
	int result = 0;
	//for (;;)
	{
		if (!handle)
		{
			//printf("MTAL# fatal, handler failed\n");
			return -1;
		}

		if (msg_queue_get(&handle->msg_q, msg) > 0)
		{
			//TODO: user defined message here
			printf("Msg Event = %d\n", msg->event);
			switch (msg->event)
			{
			case MSG_CHANGE_DISPLAY_FORMAT:
				{					
					// TODO: pseudo code, skipflag => half_display, ISR should be
					// ((input_source_frame_rate)/(skipflag+1)) in progressive,
					// input_source_frame_rate in interlace
					//
					unsigned short skipflag = ((msg->data >> 8) & 0x03);
					//gCurDisplaySetup = msg->data;
					//half_display = !!skipflag;

					printf("Display format = %d\n", msg->data & 0xFF);
					//if (itv_rcs.df != (msg->data & 0x1F))
					{
						#if 1
						if (mtal_pb_stop() != -1)
						{
							MTAL_SPEC mtal_spec;
							memcpy(&mtal_spec, &itv_rcs.mtal_spec, sizeof(mtal_spec));
							switch(msg->data & 0xFF)
							{
								case MMP_CAP_INPUT_INFO_640X480_60P:
									mtal_spec.strm_spec[0].src_w = 640;
									mtal_spec.strm_spec[0].src_h = 480;
									break;
								case MMP_CAP_INPUT_INFO_800X600_60P:
									mtal_spec.strm_spec[0].src_w = 800;
									mtal_spec.strm_spec[0].src_h = 600;
									break; 
								case MMP_CAP_INPUT_INFO_1024X768_60P:
									mtal_spec.strm_spec[0].src_w = 1024;
									mtal_spec.strm_spec[0].src_h = 768;
									break;
								case MMP_CAP_INPUT_INFO_1280X768_60P:
									mtal_spec.strm_spec[0].src_w = 1280;
									mtal_spec.strm_spec[0].src_h = 768;
									break;
								case MMP_CAP_INPUT_INFO_1280X960_60P:
									mtal_spec.strm_spec[0].src_w = 1280;
									mtal_spec.strm_spec[0].src_h = 960;
									break;
								case MMP_CAP_INPUT_INFO_1280X1024_60P:
									mtal_spec.strm_spec[0].src_w = 1280;
									mtal_spec.strm_spec[0].src_h = 1024;
									break;
								case MMP_CAP_INPUT_INFO_1360X768_60P:
									mtal_spec.strm_spec[0].src_w = 1360;
									mtal_spec.strm_spec[0].src_h = 768;
									break;
								case MMP_CAP_INPUT_INFO_1440X900_60P:
									mtal_spec.strm_spec[0].src_w = 1440;
									mtal_spec.strm_spec[0].src_h = 900;
									break;
								case MMP_CAP_INPUT_INFO_1600X900_60P:
									mtal_spec.strm_spec[0].src_w = 1600;
									mtal_spec.strm_spec[0].src_h = 900;
									break;
								case MMP_CAP_INPUT_INFO_1680X1050_60P:
									mtal_spec.strm_spec[0].src_w = 1680;
									mtal_spec.strm_spec[0].src_h = 1050;
									break;	
								case MMP_CAP_INPUT_INFO_720X480_60P:
								case MMP_CAP_INPUT_INFO_720X480_59P:
								case MMP_CAP_INPUT_INFO_720X480_59I:
								case MMP_CAP_INPUT_INFO_720X480_60I:
									mtal_spec.strm_spec[0].src_w = 720;
									mtal_spec.strm_spec[0].src_h = 480;
									break;
								case MMP_CAP_INPUT_INFO_720X576_50P:	
								case MMP_CAP_INPUT_INFO_720X576_50I:
									mtal_spec.strm_spec[0].src_w = 720;
									mtal_spec.strm_spec[0].src_h = 576;
									break;
								case MMP_CAP_INPUT_INFO_1280X720_60P:
								case MMP_CAP_INPUT_INFO_1280X720_59P:	
								case MMP_CAP_INPUT_INFO_1280X720_50P:	
									mtal_spec.strm_spec[0].src_w = 1280;
									mtal_spec.strm_spec[0].src_h = 720;
									break;
								case MMP_CAP_INPUT_INFO_1920X1080_60P:
                                case MMP_CAP_INPUT_INFO_1920X1080_59P:
								case MMP_CAP_INPUT_INFO_1920X1080_50P:
								case MMP_CAP_INPUT_INFO_1920X1080_50I:	
								case MMP_CAP_INPUT_INFO_1920X1080_60I:
								case MMP_CAP_INPUT_INFO_1920X1080_59I:	
									mtal_spec.strm_spec[0].src_w = 1920;
									mtal_spec.strm_spec[0].src_h = 1080;
									break;
							}
							//mtal_spec.strm_spec[0].src_w = 1280;
						    //mtal_spec.strm_spec[0].src_h = 720;
						    mtal_spec.n_strm = 1;

							// terminate playback
							mtal_pb_close();

							// release resource

							printf("ITV# restarting...\n");

							// re-init resource
							//itv_set_output_format(msg->data & 0xFF, gCurAudioSamplingRate);
							itv_set_output_format(msg->data & 0xFF, 48000);
							//itv_rmi_set_active_ids(0x01);

							// re-do itv_IspEnable
							{
								//gbReEnableISP = true;
							}

							// playback
							if (mtal_pb_open(&mtal_spec) != -1) { 								
								mtal_pb_play();
							}
							//usleep(1000000);
							result = MSG_CHANGE_DISPLAY_FORMAT;
						}
						#endif
					}
				}
				break;
			case MSG_WAIT_CONNECTION:
				result = MSG_WAIT_CONNECTION;
				break;
			case MSG_CONNECT_SUCCESS:
				result = MSG_CONNECT_SUCCESS;
				break;
			case MSG_NO_SIGNAL:
				result = MSG_NO_SIGNAL;
				break;
			case MSG_RESOLUTION_NOT_SUPPORTED:
				result = MSG_RESOLUTION_NOT_SUPPORTED;
				break;
			case MSG_CLEAN_OSD:
				result = MSG_CLEAN_OSD;
				break;
			default:
				printf("HANDLER# undefined msg %d\n", msg->event);
				result = -1;
				break;
			}
		}
	}
	return result;
	//pthread_exit(NULL);
}

void mtal_get_display_setting(int *width, int *height, int* framerate)
{
    *width = gDisplayWidth;
    *height = gDisplayHeight;
    *framerate = gDisplayFrameRate;
}

void mtal_set_security_session_key(uint8_t* pSessionKey)
{
    memcpy(gpSessionKey, pSessionKey, 16);
	//ioctl(gDpuFd, ITP_IOCTL_SET_KEY_BUFFER, gpSessionKey); //key.len, key.ptr
    {
        gDpuFd = open(":dpu:aes", 0);
        if (gDpuFd < 0)
        {
            printf("DPU init is failed\n");
            while(1);
        }
        else
        {
            int keyLen = 128;
            uint8_t cipherMode = ITP_DPU_CIPHER_ECB;
            uint8_t decryMode = 1;
            printf("DPU init is success: %d\n", gDpuFd);
            {
                int i = 0;
                printf("RX Session Key is:.......\n");
                for (i = 0; i < 16; i++)
                {
                    printf("0x%02X, ", gpSessionKey[i]);
                }
                printf("\n");
            }
            ioctl(gDpuFd, ITP_IOCTL_SET_KEY_LENGTH, &keyLen); //key.len, key.ptr
            ioctl(gDpuFd, ITP_IOCTL_SET_KEY_BUFFER, gpSessionKey); //key.len, key.ptr
            ioctl(gDpuFd, ITP_IOCTL_SET_CIPHER, &cipherMode); //key.len, key.ptr
            ioctl(gDpuFd, ITP_IOCTL_SET_DESCRYPT, &decryMode); //key.len, key.ptr
        }
    }
}

void mtal_get_security_session_key(uint8_t* pSessionKey)
{
    memcpy(pSessionKey, gpSessionKey, 16);
}

void mtal_decrypt_data(uint8_t* pData)
{
	aes_context ctx = { 0 };
	uint8_t pInitVector[16] = { 0 };

    aes_setkey_dec(&ctx, gpSessionKey, 128);
    aes_crypt_cbc(&ctx, AES_DECRYPT, 16, pInitVector, pData, pData);
}

int mtal_decrypt_data_by_size(uint8_t* pData, int size)
{
    if ((size & 0xF) || gDpuFd < 0)
    {
        return -1;
    }
    write(gDpuFd, pData, size);
    read(gDpuFd, pData, size);
	
    return 0;
}

bool mtal_get_source_lock_flag(void)
{
    if (gbSourceLockMode)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int mtal_set_source_lock_flag(bool bSourceLock)
{
    gbSourceLockMode = bSourceLock;
}


