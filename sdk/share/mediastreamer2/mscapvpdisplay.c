#include <sys/ioctl.h>
#include "stdio.h"
#include "assert.h"
#include "ite/itp.h"
#include "ite/ith.h"
#include "ite/itv.h"
#include "mediastreamer2/msfilewriter.h"
#include "mediastreamer2/msvideo.h"
#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msticker.h"
#include "libavcodec/avcodec.h"
#include "ortp/str_utils.h"
#include "openrtos/FreeRTOS.h"
#include "openrtos/queue.h"

#if (CFG_CHIP_FAMILY == 970)
    #include "capture/capture_970/mmp_capture.h"
    #include "video_encoder/video_encoder_it970.h"
#endif

#if (CFG_CHIP_FAMILY == 9860)
    #include "capture/capture_9860/mmp_capture.h"
#endif

#ifdef CFG_SENSOR_ENABLE
    #include "sensor/mmp_sensor.h"
#endif

#ifndef FALSE
    #define FALSE      0
#endif
#ifndef TRUE
    #define TRUE       1
#endif

#define ISPBUFFERCOUNT 3
#define ISPQUEUESIZE   20
#define CAPTIMEOUT     100//100 * 10ms
#define SENSORTIMEOUT  100//100 * 10ms
#define PVWIDTH        (CFG_LCD_WIDTH / 2)
#define PVHEIGHT       (CFG_LCD_HEIGHT / 2)
#define PVPITCH        (PVWIDTH * 2)
#define PVSTARTX       0
#define PVSTARTY       0
#define MAX_UI_BUF_CNT 3

typedef struct {
    int FinishBufferIdx;             /*finish buffer index                      */
    int CurrentBufferIdx;            /*current  write buffer index           */
} ISPQUEUEINFO;

typedef struct {
    ms_mutex_t      mutex;
    CAPTURE_HANDLE  gCapDev0;
    ISP_DEVICE      gIspDev1;
    QueueHandle_t   gISPQueue;
    ISPQUEUEINFO    gISPQueueINFO;
    ISP_BUFFER_INFO *ISPBUFFERINFO;
    ISP_BUFFER_INFO ISPPVBUFFERINFO;
    uint32_t        gISRStop;
    uint32_t        gISRFlag;
    uint32_t        gISRIndex;
} CAPFILTER;

static void _ISP1ISR(void *arg)
{
    CAPFILTER  *c_ptr                   = (CAPFILTER *) arg;
    BaseType_t gHigherPriorityTaskWoken = (BaseType_t) 0;
    MMP_ISP_OUTPUT_INFO out_info = {0};
    MMP_ISP_OUTPUT_INFO PV_out_info = {0};

    if (c_ptr->gISRStop) return;

    mmpIspClearInterrupt(c_ptr->gIspDev1);

    out_info.format    = MMP_ISP_OUT_NV12;
    PV_out_info.format = MMP_ISP_OUT_DITHER565A;

    out_info.addrY      = (MMP_UINT32)c_ptr->ISPBUFFERINFO->pISPBufAdrY[c_ptr->gISRIndex];
    out_info.addrU      = (MMP_UINT32)c_ptr->ISPBUFFERINFO->pISPBufAdrU[c_ptr->gISRIndex];
    PV_out_info.addrRGB = (MMP_UINT32)c_ptr->ISPPVBUFFERINFO.pISPBufAdrY[c_ptr->gISRIndex];

    if (!c_ptr->gISRFlag && c_ptr->gISRIndex == 2) c_ptr->gISRFlag = 1;

    //while (mmpIspIsEngineIdle(c_ptr->gIspDev1));
    /*ithPrintf("ISR0:*%x*\n", ithReadRegA(0xD030029C));
    ithPrintf("ISR1:*%x*\n", ithReadRegA(0xD03002A0));
    ithPrintf("ISR2:*%x*\n", ithReadRegA(0xD03002A4));*/
    mmpIspSetBlankOutputWindow(c_ptr->gIspDev1, &out_info, &PV_out_info);

    if (c_ptr->gISRFlag)
    {
        c_ptr->gISPQueueINFO.FinishBufferIdx   = c_ptr->gISPQueueINFO.CurrentBufferIdx;
        c_ptr->gISPQueueINFO.CurrentBufferIdx++;
        c_ptr->gISPQueueINFO.CurrentBufferIdx %= ISPBUFFERCOUNT;

        VideoEncoder_SetEncodeIdx(0);
        xQueueSendToBackFromISR(c_ptr->gISPQueue, &c_ptr->gISPQueueINFO.FinishBufferIdx, &gHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(gHigherPriorityTaskWoken);
    }

    c_ptr->gISRIndex = (c_ptr->gISRIndex + 1) % ISPBUFFERCOUNT;
}

static int _SIGNALCHECK_FIRE(CAPTURE_HANDLE *ptDev)
{
    int timeout = 0;
    while ((ithCapGetEngineErrorStatus(ptDev, MMP_CAP_LANE0_STATUS) & 0x3) != 0x3)
    {
        //printf("Hsync or Vsync not stable!\n");
        if (++timeout > CAPTIMEOUT)
        {
            return 1;
        }
        usleep(1000 * 10);
    }

    ithCapFire(ptDev, MMP_TRUE);
    printf("Capture FIRE(0)\n");
    return 0;
}

static int _SENSOR_SIGNALCHECK(void)
{
    int timeout = 0;
    while (ithCapDeviceIsSignalStable() != true)
    {
        //printf("Sensor not stable!\n");
        if (++timeout > SENSORTIMEOUT)
        {
            printf("Wait Sensor stable timeout\n");
            return 1;
        }
        usleep(1000 * 10);
    }
    return 0;
}
static void capvponfly_init(MSFilter *f)
{
    CAPFILTER      *c_ptr = ms_new0(CAPFILTER, 1);
    struct mq_attr attr;
    int            i;

    ms_mutex_init(&c_ptr->mutex, NULL);
    c_ptr->gISPQueue                      = xQueueCreate(ISPQUEUESIZE, (unsigned portBASE_TYPE) sizeof(int));
    c_ptr->gISPQueueINFO.CurrentBufferIdx = 0;
    c_ptr->gISPQueueINFO.FinishBufferIdx  = -1;
    f->data                               = c_ptr;

    /* Power On */
    ithCapPowerUp();

    /* capture init */
    ithCapInitialize();

    /* ISP init core 1 */
    for (i = 0; i < ISPBUFFERCOUNT; i++)
        c_ptr->ISPPVBUFFERINFO.pISPBufAdrY[i] = ms_malloc0(PVPITCH * PVHEIGHT);

    mmpIspInitialize(&c_ptr->gIspDev1, MMP_ISP_CORE_1);

    itv_ff_enable(0, true);
    for (i = 0; i < MAX_UI_BUF_CNT; i++)
        itv_ff_setup_base(0, i, (uint8_t *)c_ptr->ISPPVBUFFERINFO.pISPBufAdrY[i]);

#if defined(CFG_SENSOR_ENABLE)
    ithCapDeviceInitialize();
    ithCapDeviceLEDON(1);
#endif
}

static void capvponfly_uninit(MSFilter *f)
{
    CAPFILTER *c_ptr = (CAPFILTER *)f->data;
    int       i;

    c_ptr->gISRStop = 1;

    itv_ff_enable(0, false);

#ifdef CFG_CMDQ_ENABLE
    ithCmdQWaitEmpty(ITH_CMDQ1_OFFSET);
#endif

    mmpIspResetEngine(c_ptr->gIspDev1);
    ithIspResetReg(ITH_ISP_CORE1);
    while (!mmpIspIsEngineIdle(c_ptr->gIspDev1));
    mmpIspTerminate(&c_ptr->gIspDev1);

    for (i = 0; i < ISPBUFFERCOUNT; i++)
        ms_free(c_ptr->ISPPVBUFFERINFO.pISPBufAdrY[i]);

    ithCapDisConnect(&c_ptr->gCapDev0);
    ithCapTerminate();

    ithCapPowerDown();

#if defined(CFG_SENSOR_ENABLE)
    ithCapDeviceLEDON(0);
    ithCapDeviceTerminate();
#endif

    ms_mutex_destroy(&c_ptr->mutex);
    vQueueDelete(c_ptr->gISPQueue);

    ms_free(c_ptr);

    return;
}

static void capvponfly_preprocess(MSFilter *f)
{
    CAPFILTER                 *c_ptr        = (CAPFILTER *)f->data;
    CAP_CONTEXT               *CAPINFO;
    MMP_ISP_SHARE             ISPSHARE;
    MMP_ISP_CORE_INFO         ISPCOREINFO   = {0};
    MMP_ISP_WRITE_BUFFER_INFO ISPWBINFO     = {0};
    MMP_ISP_OUTPUT_INFO       ISPOUTINFO    = {0};
    int                       i             = 0;

    /*CAPTURE SETTING*/
    CAPTURE_SETTING           onfly_modeset = {MMP_CAP_DEV_SENSOR, MMP_TRUE,  MMP_FALSE, 1920, 1080};
    ithCapConnect(&c_ptr->gCapDev0, onfly_modeset);
    _SENSOR_SIGNALCHECK();
    ithCapGetDeviceInfo(&c_ptr->gCapDev0);
    CAPINFO                 = &(c_ptr->gCapDev0.cap_info);
    ithCapParameterSetting(&c_ptr->gCapDev0);

    c_ptr->ISPBUFFERINFO    = VideoEncoder_GetISPBUFFERINFO();
    printf("cap w = %d, h =%d, p = %d\n", CAPINFO->ininfo.capwidth, CAPINFO->ininfo.capheight, CAPINFO->ininfo.PitchY);

    ISPCOREINFO.EnPreview   = true;
    ISPCOREINFO.PreScaleSel = MMP_ISP_PRESCALE_PREVIEW;
    mmpIspSetCore(c_ptr->gIspDev1, &ISPCOREINFO);
    mmpIspSetMode(c_ptr->gIspDev1, MMP_ISP_MODE_PLAY_VIDEO);
    mmpIspEnableCAPOnFlyMode(c_ptr->gIspDev1);
    mmpIspSetOutputFormat(c_ptr->gIspDev1, MMP_ISP_OUT_NV12);
    /* ENABLE ISP ISR */
    ithIntrDisableIrq(ITH_INTR_ISP_CORE1);
    ithIntrRegisterHandlerIrq(ITH_INTR_ISP_CORE1, _ISP1ISR, c_ptr);
    ithIntrSetTriggerModeIrq(ITH_INTR_ISP_CORE1, ITH_INTR_EDGE);
    ithIntrSetTriggerLevelIrq(ITH_INTR_ISP_CORE1, ITH_INTR_HIGH_RISING);
    ithIntrEnableIrq(ITH_INTR_ISP_CORE1);

    /* ISP OUTPUT ADDRESS */
    for (i = 0; i < ISPBUFFERCOUNT; ++i)
    {
        switch (i)
        {
        case 0:
            ISPOUTINFO.addrY = (MMP_UINT32) c_ptr->ISPBUFFERINFO->pISPBufAdrY[i];
            ISPOUTINFO.addrU = (MMP_UINT32) c_ptr->ISPBUFFERINFO->pISPBufAdrU[i];
            break;

        case 1:
            ISPWBINFO.addrYRGB_1 = (MMP_UINT32) c_ptr->ISPBUFFERINFO->pISPBufAdrY[i];
            ISPWBINFO.addrU_1    = (MMP_UINT32) c_ptr->ISPBUFFERINFO->pISPBufAdrU[i];
            break;

        case 2:
            ISPWBINFO.addrYRGB_2 = (MMP_UINT32) c_ptr->ISPBUFFERINFO->pISPBufAdrY[i];
            ISPWBINFO.addrU_2    = (MMP_UINT32) c_ptr->ISPBUFFERINFO->pISPBufAdrU[i];
            break;

        case 3:
            ISPWBINFO.addrYRGB_3 = (MMP_UINT32) c_ptr->ISPBUFFERINFO->pISPBufAdrY[i];
            ISPWBINFO.addrU_3    = (MMP_UINT32) c_ptr->ISPBUFFERINFO->pISPBufAdrU[i];
            break;

        case 4:
            ISPWBINFO.addrYRGB_4 = (MMP_UINT32) c_ptr->ISPBUFFERINFO->pISPBufAdrY[i];
            ISPWBINFO.addrU_4    = (MMP_UINT32) c_ptr->ISPBUFFERINFO->pISPBufAdrU[i];
            break;

        default:
            break;
        }
    }

    //mmpIspSetWriteBufferInfo(c_ptr->gIspDev1, &ISPWBINFO);
    //mmpIspSetWriteBufferNum(c_ptr->gIspDev1, ISPBUFFERCOUNT - 1);
#if defined(CFG_SENSOR_ENABLE)
    ISPOUTINFO.width   = SENSOR_WIDTH;
    ISPOUTINFO.height  = SENSOR_HEIGHT;
    ISPOUTINFO.pitchY  = SENSOR_WIDTH;
    ISPOUTINFO.pitchUv = SENSOR_WIDTH;
    ISPOUTINFO.format  = MMP_ISP_OUT_NV12;
#endif
    mmpIspSetOutputWindow(c_ptr->gIspDev1, &ISPOUTINFO);
    mmpIspSetVideoWindow(c_ptr->gIspDev1, 0, 0, ISPOUTINFO.width, ISPOUTINFO.height);

    ISPOUTINFO.addrRGB  = c_ptr->ISPPVBUFFERINFO.pISPBufAdrY[0];
    ISPOUTINFO.width    = PVWIDTH;
    ISPOUTINFO.height   = PVHEIGHT;
    ISPOUTINFO.pitchRGB = PVPITCH;
    ISPOUTINFO.format   = MMP_ISP_OUT_DITHER565A;
    mmpIspSetPVOutputWindow(c_ptr->gIspDev1, &ISPOUTINFO);
    mmpIspSetPVVideoWindow(c_ptr->gIspDev1, 0, 0, ISPOUTINFO.width, ISPOUTINFO.height);
    mmpIspEnableInterrupt(c_ptr->gIspDev1);

    if (_SIGNALCHECK_FIRE(&c_ptr->gCapDev0) == 0)
    {
        /* ISP INPUT SETTING*/
        ISPSHARE.width   = CAPINFO->outinfo.OutWidth;
        ISPSHARE.height  = CAPINFO->outinfo.OutHeight;
        ISPSHARE.pitchY  = CAPINFO->ininfo.PitchY;
        ISPSHARE.pitchUv = CAPINFO->ininfo.PitchUV;

        if (CAPINFO->YUVinfo.InputMode == 0)
            ISPSHARE.format = MMP_ISP_IN_YUV422;
        else if (CAPINFO->YUVinfo.InputMode == 1)
            ISPSHARE.format = MMP_ISP_IN_YUV444;
        else
            ISPSHARE.format = MMP_ISP_IN_RGB888;

        mmpIspPlayImageProcess(c_ptr->gIspDev1, &ISPSHARE);
    }
    //itv_set_video_window(0, 0, ithLcdGetWidth(), ithLcdGetHeight());
    //itv_set_pb_mode(1);
}

static void capvponfly_process(MSFilter *f)
{
    CAPFILTER     *c_ptr   = (CAPFILTER *)f->data;
    CAP_CONTEXT   *CAPINFO = &(c_ptr->gCapDev0.cap_info);
    unsigned int  CAPError = ithCapGetEngineErrorStatus(&c_ptr->gCapDev0, MMP_CAP_LANE0_STATUS);
    MMP_ISP_SHARE ISPSHARE;
    int           index    = 0;

    if (xQueueReceive(c_ptr->gISPQueue, &index, 0))
    {
        if ((CAPError & 0x0F00) == 0x0)
        {
            ITV_UI_PROPERTY ui_prop = {0};
            uint32_t        ui_buf_ptr = (uint32_t)itv_get_uibuf_anchor(0);

            if (ui_buf_ptr != NULL)
            {
                ui_prop.startX       = PVSTARTX;
                ui_prop.startY       = PVSTARTY;
                ui_prop.width        = PVWIDTH;
                ui_prop.height       = PVHEIGHT;
                ui_prop.pitch        = PVPITCH;
                ui_prop.colorKeyR    = 0x00;
                ui_prop.colorKeyG    = 0x00;
                ui_prop.colorKeyB    = 0x00;
                ui_prop.EnAlphaBlend = false;
                ui_prop.constAlpha   = 0xFF;

                itv_update_uibuf_anchor(0, &ui_prop);
            }
        }
    }

    if (CAPError & 0x0F00)
    {
        /* Reset engine */
        printf("CAPTURE ERROR = %x\n",CAPError);
        ithCapWaitEngineIdle(&c_ptr->gCapDev0);
        ithCapEngineReset();
        mmpIspResetEngine(c_ptr->gIspDev1);

        c_ptr->gISPQueueINFO.CurrentBufferIdx = 0;
        c_ptr->gISPQueueINFO.FinishBufferIdx  = -1;

        ithCapGetDeviceInfo(&c_ptr->gCapDev0);
        ithCapParameterSetting(&c_ptr->gCapDev0);

        if (_SIGNALCHECK_FIRE(&c_ptr->gCapDev0) == 0)
        {
            /* ISP INPUT SETTING */
            ISPSHARE.width   = CAPINFO->outinfo.OutWidth;
            ISPSHARE.height  = CAPINFO->outinfo.OutHeight;
            ISPSHARE.pitchY  = CAPINFO->ininfo.PitchY;
            ISPSHARE.pitchUv = CAPINFO->ininfo.PitchUV;

            if (CAPINFO->YUVinfo.InputMode == 0)
                ISPSHARE.format = MMP_ISP_IN_YUV422;
            else if (CAPINFO->YUVinfo.InputMode == 1)
                ISPSHARE.format = MMP_ISP_IN_YUV444;
            else
                ISPSHARE.format = MMP_ISP_IN_RGB888;
            //mmpIspPlayImageProcess(c_ptr->gIspDev1, &ISPSHARE);
        }
    }
}

static void capvponfly_postprocess(MSFilter *f)
{
    //itv_set_pb_mode(0);
}

static MSFilterMethod methods[] = {
    {0,                        NULL             },
};

#ifdef _MSC_VER

MSFilterDesc ms_capvponflydisplay_desc = {
    MS_CAPVPONFLY_DISPLAY_ID,
    "MS_CAPVPONFLY FOR DISPLAY",
    "Capture IN",
    MS_FILTER_OTHER,
    NULL,
    0,
    0,
    capvponfly_init,
    capvponfly_preprocess,
    capvponfly_process,
    capvponfly_postprocess,
    capvponfly_uninit,
    methods
};

#else

MSFilterDesc ms_capvponflydisplay_desc = {
    .id          = MS_CAPVPONFLY_DISPLAY_ID,
    .name        = "MS_CAPVPONFLY FOR DISPLAY",
    .text        = "Capture IN",
    .category    = MS_FILTER_OTHER,
    .enc_fmt     = NULL,
    .ninputs     =                      0,
    .noutputs    =                      0,
    .init        = capvponfly_init,
    .preprocess  = capvponfly_preprocess,
    .process     = capvponfly_process,
    .postprocess = capvponfly_postprocess,
    .uninit      = capvponfly_uninit,
    .methods     = methods
};

#endif

MS_FILTER_DESC_EXPORT(ms_capvponflydisplay_desc)