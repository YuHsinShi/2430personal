#include <sys/ioctl.h>
#include "stdio.h"
#include "ite/itp.h"
#include "ite/itv.h"
#include "mediastreamer2/msfilewriter.h"
#include "mediastreamer2/msvideo.h"
#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msticker.h"
#include "libavcodec/avcodec.h"
#include "ortp/str_utils.h"
#include "openrtos/FreeRTOS.h"
#include "openrtos/queue.h"

#undef CFG_DUAL_STREAM

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

#define QUEUESIZE      20
#define ISPBUFFERCOUNT 3
#define CAPTIMEOUT     100//100 * 10ms
#define SENSORTIMEOUT  100//100 * 10ms
#define PVWIDTH        (CFG_LCD_WIDTH / 2)
#define PVHEIGHT       (CFG_LCD_HEIGHT / 2)
#define PVPITCH        (PVWIDTH * 2)
#define PVSTARTX       0
#define PVSTARTY       0
#define MAX_UI_BUF_CNT 3

typedef enum CAP_STATUS_TAG
{
    CAP_ERROR,
    CAP_FRAME_END,
} CAP_STATUS;

typedef struct {
    int          CapDualStreamIndex;           /* dual stream index */
    int          Capinterlanced;
    unsigned int CapOutWidth;
    unsigned int CapOutHeight;
    unsigned int CapPitchY;
    unsigned int CapPitchUV;
    unsigned int CapaddrY;
    unsigned int CapaddrUV;
} CAPQUEUEINFO;

typedef struct {
    int FinishBufferIdx[2];                    /*finish buffer index*/
    int CurrentBufferIdx[2];                   /*current  write buffer index*/
    int StreamCount;                           /*Count stream id*/
} ISPINDEXINFO;

typedef struct {
    ms_mutex_t      mutex;
    MSVideoSize     gInputSize;
    CAPTURE_HANDLE  gCapDev0;
    QueueHandle_t   gCapQueue;                 /*capture  queue*/
    bool            gCapError;                 /*check capture error */
    pthread_t       ISPpthread;
    bool            ISPthreadRun;
    ISP_DEVICE      gIspDev1;
    QueueHandle_t   gISPQueue;                 /*ISP queue */
    ISPINDEXINFO    gISPINDEX;
    ISP_BUFFER_INFO *ISPBUFFERINFO;
    ISP_BUFFER_INFO ISPPVBUFFERINFO;
    uint32_t        gISRStop;
} CAPFILTER;

static void _UPDATE_INDEX(ISPINDEXINFO *ISPINDEX, int streamID)
{
    ISPINDEX->FinishBufferIdx[streamID]   = ISPINDEX->CurrentBufferIdx[streamID];
    ISPINDEX->CurrentBufferIdx[streamID]++;
    ISPINDEX->CurrentBufferIdx[streamID] %= ISPBUFFERCOUNT;
}

static void _CAP0ISR(void *arg)
{
    uint32_t     capture0state            = 0;
    uint16_t     capindex                 = 0;
    CAPQUEUEINFO capqueueinfo             = {0};
    CAPFILTER    *c_ptr                   = (CAPFILTER *) arg;
    BaseType_t   gHigherPriorityTaskWoken = (BaseType_t)0;

    capture0state = ithCapGetEngineErrorStatus(&c_ptr->gCapDev0, MMP_CAP_LANE0_STATUS);

    if (capture0state >> 31)
    {
        if ((capture0state >> 8) & 0xF)
        {
            //clear cap0 interrupt and reset error status
            c_ptr->gCapError = true;
            ithCapClearInterrupt(&c_ptr->gCapDev0, MMP_TRUE);
        }
        else
        {
            capindex                        = ithCapReturnWrBufIndex(&c_ptr->gCapDev0);
            capqueueinfo.CapOutWidth        = (unsigned int)c_ptr->gCapDev0.cap_info.outinfo.OutWidth;
            capqueueinfo.CapOutHeight       = (unsigned int)c_ptr->gCapDev0.cap_info.outinfo.OutHeight;
            capqueueinfo.CapPitchY          = (unsigned int)c_ptr->gCapDev0.cap_info.ininfo.PitchY;
            capqueueinfo.CapPitchUV         = (unsigned int)c_ptr->gCapDev0.cap_info.ininfo.PitchUV;
            capqueueinfo.CapaddrY           = (unsigned int)c_ptr->gCapDev0.cap_info.OutAddrY[capindex];
            capqueueinfo.CapaddrUV          = (unsigned int)c_ptr->gCapDev0.cap_info.OutAddrUV[capindex];
            capqueueinfo.Capinterlanced     = c_ptr->gCapDev0.cap_info.ininfo.Interleave;

            capqueueinfo.CapDualStreamIndex = 0;
            xQueueSendToBackFromISR(c_ptr->gCapQueue, &capqueueinfo, &gHigherPriorityTaskWoken);
#if defined(CFG_DUAL_STREAM)
            capqueueinfo.CapDualStreamIndex = 1;
            xQueueSendToBackFromISR(c_ptr->gCapQueue, &capqueueinfo, &gHigherPriorityTaskWoken);
#endif
            //clear cap0 interrupt
            ithCapClearInterrupt(&c_ptr->gCapDev0, MMP_FALSE);
        }
    }
    portYIELD_FROM_ISR(gHigherPriorityTaskWoken);
}

static void _ISP1ISR(void *arg)
{
    CAPFILTER  *c_ptr                   = (CAPFILTER *) arg;
    BaseType_t gHigherPriorityTaskWoken = (BaseType_t) 0;

    if (c_ptr->gISRStop) return;

#if !defined(CFG_DUAL_STREAM)
    _UPDATE_INDEX(&c_ptr->gISPINDEX, 0);
    VideoEncoder_SetEncodeIdx(0);
    xQueueSendToBackFromISR(c_ptr->gISPQueue, &c_ptr->gISPINDEX.FinishBufferIdx[0], &gHigherPriorityTaskWoken);
#else

    if ((c_ptr->gISPINDEX.StreamCount % 2) == 0)
    {
        _UPDATE_INDEX(&c_ptr->gISPINDEX, 0);
        VideoEncoder_SetEncodeIdx(0);
        xQueueSendToBackFromISR(c_ptr->gISPQueue, &c_ptr->gISPINDEX.FinishBufferIdx[0], &gHigherPriorityTaskWoken);
    }
    else
    {
        VideoEncoder_SetEncodeIdx(1);
        _UPDATE_INDEX(&c_ptr->gISPINDEX, 1);
    }

    c_ptr->gISPINDEX.StreamCount++;
    c_ptr->gISPINDEX.StreamCount %= 2;
#endif

    mmpIspClearInterrupt(c_ptr->gIspDev1);
    portYIELD_FROM_ISR(gHigherPriorityTaskWoken);
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
static void *_DOISP(void *arg)
{
    MMP_ISP_CORE_INFO   ISPCOREINFO  = {0};
    CAPQUEUEINFO        CAPQUEUEINFO = {0};
    MMP_ISP_OUTPUT_INFO ISPOUTINFO   = {0};
    MMP_ISP_SHARE       ISPSHARE     = {0};

    CAPFILTER           *c_ptr       = (CAPFILTER *)arg;

    /* ISP init core 1 */
    mmpIspInitialize(&c_ptr->gIspDev1, MMP_ISP_CORE_1);

    while (1)
    {
        if (mmpIspIsEngineIdle(c_ptr->gIspDev1))
        {
            if (xQueueReceive(c_ptr->gCapQueue, &CAPQUEUEINFO, 0))
            {
                /* ISP 1 SET CORE */
                ISPCOREINFO.EnPreview   = true;
                ISPCOREINFO.PreScaleSel = MMP_ISP_PRESCALE_PREVIEW;
                mmpIspSetCore(c_ptr->gIspDev1, &ISPCOREINFO);
                mmpIspSetMode(c_ptr->gIspDev1, MMP_ISP_MODE_PLAY_VIDEO);
                mmpIspSetOutputFormat(c_ptr->gIspDev1, MMP_ISP_OUT_NV12);
                /* ENABLE ISP ISR */
                ithIntrDisableIrq(ITH_INTR_ISP_CORE1);
                ithIntrRegisterHandlerIrq(ITH_INTR_ISP_CORE1, _ISP1ISR, (void *)c_ptr);
                ithIntrSetTriggerModeIrq(ITH_INTR_ISP_CORE1, ITH_INTR_EDGE);
                ithIntrSetTriggerLevelIrq(ITH_INTR_ISP_CORE1, ITH_INTR_HIGH_RISING);
                ithIntrEnableIrq(ITH_INTR_ISP_CORE1);
                mmpIspEnableInterrupt(c_ptr->gIspDev1);

                if (CAPQUEUEINFO.Capinterlanced)
                    mmpIspEnable(c_ptr->gIspDev1, MMP_ISP_DEINTERLACE);
                else
                    mmpIspDisable(c_ptr->gIspDev1, MMP_ISP_DEINTERLACE);

                mmpIspSetDeinterlaceType(c_ptr->gIspDev1, MMP_ISP_2D_DEINTERLACE);
                if (CAPQUEUEINFO.CapDualStreamIndex == 0)
                {
                    ISPOUTINFO.addrY   = (MMP_UINT32)c_ptr->ISPBUFFERINFO->pISPBufAdrY[c_ptr->gISPINDEX.CurrentBufferIdx[0]];
                    ISPOUTINFO.addrU   = (MMP_UINT32)c_ptr->ISPBUFFERINFO->pISPBufAdrU[c_ptr->gISPINDEX.CurrentBufferIdx[0]];
                    ISPOUTINFO.addrV   = (MMP_UINT32)c_ptr->ISPBUFFERINFO->pISPBufAdrU[c_ptr->gISPINDEX.CurrentBufferIdx[0]];
#if defined(CFG_SENSOR_ENABLE)
                    ISPOUTINFO.width   = SENSOR_WIDTH;
                    ISPOUTINFO.height  = SENSOR_HEIGHT;
                    ISPOUTINFO.pitchY  = SENSOR_WIDTH;
                    ISPOUTINFO.pitchUv = SENSOR_WIDTH;
#endif
                    ISPOUTINFO.format  = MMP_ISP_OUT_NV12;
                    mmpIspSetOutputWindow(c_ptr->gIspDev1, &ISPOUTINFO);
                    mmpIspSetVideoWindow(c_ptr->gIspDev1, 0, 0, ISPOUTINFO.width, ISPOUTINFO.height);

                    ISPOUTINFO.addrRGB  = c_ptr->ISPPVBUFFERINFO.pISPBufAdrY[c_ptr->gISPINDEX.CurrentBufferIdx[0]];
                    ISPOUTINFO.width    = PVWIDTH;
                    ISPOUTINFO.height   = PVHEIGHT;
                    ISPOUTINFO.pitchRGB = PVPITCH;
                    ISPOUTINFO.format   = MMP_ISP_OUT_DITHER565A;
                    mmpIspSetPVOutputWindow(c_ptr->gIspDev1, &ISPOUTINFO);
                    mmpIspSetPVVideoWindow(c_ptr->gIspDev1, 0, 0, ISPOUTINFO.width, ISPOUTINFO.height);
                }
                else
                {
#if defined(CFG_DUAL_STREAM)
                    /* ISP 1 SET CORE */
                    ISPCOREINFO.EnPreview   = false;
                    ISPCOREINFO.PreScaleSel = MMP_ISP_PRESCALE_NORMAL;
                    mmpIspSetCore(c_ptr->gIspDev1, &ISPCOREINFO);
                    mmpIspSetMode(c_ptr->gIspDev1, MMP_ISP_MODE_PLAY_VIDEO);
                    mmpIspSetOutputFormat(c_ptr->gIspDev1, MMP_ISP_OUT_NV12);
                    /* ENABLE ISP ISR */
                    ithIntrDisableIrq(ITH_INTR_ISP_CORE1);
                    ithIntrRegisterHandlerIrq(ITH_INTR_ISP_CORE1, _ISP1ISR, (void *)c_ptr);
                    ithIntrSetTriggerModeIrq(ITH_INTR_ISP_CORE1, ITH_INTR_EDGE);
                    ithIntrSetTriggerLevelIrq(ITH_INTR_ISP_CORE1, ITH_INTR_HIGH_RISING);
                    ithIntrEnableIrq(ITH_INTR_ISP_CORE1);
                    mmpIspEnableInterrupt(c_ptr->gIspDev1);

                    if (CAPQUEUEINFO.Capinterlanced)
                        mmpIspEnable(c_ptr->gIspDev1, MMP_ISP_DEINTERLACE);
                    else
                        mmpIspDisable(c_ptr->gIspDev1, MMP_ISP_DEINTERLACE);

                    mmpIspSetDeinterlaceType(c_ptr->gIspDev1, MMP_ISP_2D_DEINTERLACE);

                    ISPOUTINFO.addrY   = (MMP_UINT32)c_ptr->ISPBUFFERINFO->pISPBufAdrY_2[c_ptr->gISPINDEX.CurrentBufferIdx[1]];
                    ISPOUTINFO.addrU   = (MMP_UINT32)c_ptr->ISPBUFFERINFO->pISPBufAdrU_2[c_ptr->gISPINDEX.CurrentBufferIdx[1]];
                    ISPOUTINFO.addrV   = (MMP_UINT32)c_ptr->ISPBUFFERINFO->pISPBufAdrU_2[c_ptr->gISPINDEX.CurrentBufferIdx[1]];
                    ISPOUTINFO.width   = SENSOR_WIDTH_2;
                    ISPOUTINFO.height  = SENSOR_HEIGHT_2;
                    ISPOUTINFO.pitchY  = SENSOR_WIDTH_2;
                    ISPOUTINFO.pitchUv = SENSOR_WIDTH_2;
                    ISPOUTINFO.format  = MMP_ISP_OUT_NV12;
                    mmpIspSetOutputWindow(c_ptr->gIspDev1, &ISPOUTINFO);
                    mmpIspSetVideoWindow(c_ptr->gIspDev1, 0, 0, ISPOUTINFO.width, ISPOUTINFO.height);
#endif
                }
                /* input info*/
                ISPSHARE.width   = CAPQUEUEINFO.CapOutWidth;
                ISPSHARE.height  = CAPQUEUEINFO.CapOutHeight;
                ISPSHARE.pitchY  = CAPQUEUEINFO.CapPitchY;
                ISPSHARE.pitchUv = CAPQUEUEINFO.CapPitchUV;
                ISPSHARE.addrY   = CAPQUEUEINFO.CapaddrY;
                ISPSHARE.addrU   = CAPQUEUEINFO.CapaddrUV;
                ISPSHARE.addrV   = CAPQUEUEINFO.CapaddrUV;
                ISPSHARE.format = MMP_ISP_IN_NV12;
                mmpIspPlayImageProcess(c_ptr->gIspDev1, &ISPSHARE);
            }
            else
            {
                if (c_ptr->ISPthreadRun == false) break;
            }
        }
        usleep(1000);
    }

    mmpIspResetEngine(c_ptr->gIspDev1);
    mmpIspTerminate(&c_ptr->gIspDev1);

    printf("_DOISP Closed\n");
}

static void capvpmem_init(MSFilter *f)
{
    CAPFILTER      *c_ptr = ms_new0(CAPFILTER, 1);
    pthread_attr_t attr;
    int            i;
    ms_mutex_init(&c_ptr->mutex, NULL);

    c_ptr->gCapQueue                     = xQueueCreate(QUEUESIZE, (unsigned portBASE_TYPE) sizeof(CAPQUEUEINFO));
    c_ptr->gISPQueue                     = xQueueCreate(QUEUESIZE, (unsigned portBASE_TYPE) sizeof(int));
    c_ptr->gISPINDEX.CurrentBufferIdx[0] = 0;
    c_ptr->gISPINDEX.FinishBufferIdx[0]  = -1;
    c_ptr->gISPINDEX.CurrentBufferIdx[1] = 0;
    c_ptr->gISPINDEX.FinishBufferIdx[1]  = -1;
    c_ptr->gISPINDEX.StreamCount         = 0;
    c_ptr->ISPthreadRun                  = true;
    c_ptr->gCapError                     = false;
    f->data                              = c_ptr;

    for (i = 0; i < ISPBUFFERCOUNT; i++)
        c_ptr->ISPPVBUFFERINFO.pISPBufAdrY[i] = ms_malloc0(PVPITCH * PVHEIGHT);

    itv_ff_enable(0, true);
    for (i = 0; i < MAX_UI_BUF_CNT; i++)
        itv_ff_setup_base(0, i, (uint8_t *)c_ptr->ISPPVBUFFERINFO.pISPBufAdrY[i]);

    pthread_attr_init(&attr);
    ms_thread_create(&(c_ptr->ISPpthread), &attr, _DOISP, (void *) c_ptr);

    /* Power On */
    ithCapPowerUp();

    /* capture init */
    ithCapInitialize();

#if defined(CFG_SENSOR_ENABLE)
    ithCapDeviceInitialize();
    ithCapDeviceLEDON(1);
#endif
}

static void capvpmem_uninit(MSFilter *f)
{
    CAPFILTER *c_ptr = (CAPFILTER *)f->data;
    int       i = 0;

    c_ptr->gISRStop = 1;

    itv_ff_enable(0, false);

    c_ptr->ISPthreadRun = false;
    ms_thread_join(c_ptr->ISPpthread, NULL);

    for (i = 0; i < ISPBUFFERCOUNT; i++)
        ms_free(c_ptr->ISPPVBUFFERINFO.pISPBufAdrY[i]);

    ithCapTerminate();
    ithCapDisConnect(&c_ptr->gCapDev0);
    ithCapPowerDown();

#if defined(CFG_SENSOR_ENABLE)
    ithCapDeviceLEDON(0);
    ithCapDeviceTerminate();
#endif
    ms_mutex_destroy(&c_ptr->mutex);
    vQueueDelete(c_ptr->gCapQueue);
    vQueueDelete(c_ptr->gISPQueue);

    ms_free(c_ptr);
    return;
}

static void capvpmem_preprocess(MSFilter *f)
{
    CAPFILTER *c_ptr = (CAPFILTER *)f->data;

    /*IT970 memory mode max use 1280*720P , otherwise memory bandwidth may be not enough.*/
    printf("input vsize w = %d, h =%d \n", c_ptr->gInputSize.width, c_ptr->gInputSize.height);
    CAPTURE_SETTING mem_modeset = {MMP_CAP_DEV_SENSOR, MMP_FALSE,  MMP_TRUE, c_ptr->gInputSize.width, c_ptr->gInputSize.height};
    ithCapConnect(&c_ptr->gCapDev0, mem_modeset);
    ithCapRegisterIRQ(_CAP0ISR, (void *)c_ptr);
    _SENSOR_SIGNALCHECK();
    ithCapGetDeviceInfo(&c_ptr->gCapDev0);
    ithCapParameterSetting(&c_ptr->gCapDev0);
    c_ptr->ISPBUFFERINFO = VideoEncoder_GetISPBUFFERINFO();

    _SIGNALCHECK_FIRE(&c_ptr->gCapDev0);
    //itv_set_video_window(0, 0, ithLcdGetWidth(), ithLcdGetHeight());
    //itv_set_pb_mode(1);
}

static void capvpmem_process(MSFilter *f)
{
    CAPFILTER   *c_ptr     = (CAPFILTER *)f->data;
    CAP_CONTEXT *CAPINFO   = &(c_ptr->gCapDev0.cap_info);
    int         ISPWRINDEX = 0;

    if (c_ptr->gCapError)
    {
        printf("CAPTURE ERROR (%s)(%d)\n", __FUNCTION__, __LINE__);
        ithCapWaitEngineIdle(&c_ptr->gCapDev0);
        ithCapEngineReset();
        ithCapGetDeviceInfo(&c_ptr->gCapDev0);
        ithCapParameterSetting(&c_ptr->gCapDev0);
        if (_SIGNALCHECK_FIRE(&c_ptr->gCapDev0) == 0)
            c_ptr->gCapError = false;
    }
    if (xQueueReceive(c_ptr->gISPQueue, &ISPWRINDEX, 0))
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

static void capvpmem_postprocess(MSFilter *f)
{
    //itv_set_pb_mode(0);
}

static int capvpmem_set_vsize(MSFilter *f, void *data)
{
    CAPFILTER *c_ptr = (CAPFILTER *)f->data;
    c_ptr->gInputSize = *(MSVideoSize *)data;
    return 0;
}

static MSFilterMethod methods[] = {
    {MS_FILTER_SET_VIDEO_SIZE, capvpmem_set_vsize  },
};

#ifdef _MSC_VER

MSFilterDesc ms_capvpmemdisplay_desc = {
    MS_CAPVPMEM_DISPLAY_ID,
    "MS_CAPVPMEM FOR DISPLAY",
    "Capture IN",
    MS_FILTER_OTHER,
    NULL,
    0,
    0,
    capvpmem_init,
    capvpmem_preprocess,
    capvpmem_process,
    capvpmem_postprocess,
    capvpmem_uninit,
    methods
};

#else

MSFilterDesc ms_capvpmemdisplay_desc = {
    .id          = MS_CAPVPMEM_DISPLAY_ID,
    .name        = "MS_CAPVPMEM FOR DISPLAY",
    .text        = "Capture IN",
    .category    = MS_FILTER_OTHER,
    .enc_fmt     = NULL,
    .ninputs     =                    0,
    .noutputs    =                    0,
    .init        = capvpmem_init,
    .preprocess  = capvpmem_preprocess,
    .process     = capvpmem_process,
    .postprocess = capvpmem_postprocess,
    .uninit      = capvpmem_uninit,
    .methods     = methods
};

#endif

MS_FILTER_DESC_EXPORT(ms_capvpmemdisplay_desc)