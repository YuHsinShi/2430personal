/*
 * iTE castor3 media player for camera capture
 *
 * @file:capdev_player.c
 * @version 2.0.0
 * @How to use:
 *
 *
 *  1.set play window size 
 *     itv_set_video_window(x, y, width, height);
 *
 *  2.init     
 *     static void EventHandler(PLAYER_EVENT nEventID, void *arg){
 *         switch(nEventID)
 *           case PLAYER_EVENT_CAPTURE_DEV: 
 *           //this event is capture error happened, please restart this player.
 *           break;
 *        }
 *     }
 *
 *     mtal_pb_init(EventHandler);
 *     mtal_spec.camera_in = CAPTURE_IN;
 *     mtal_pb_select_file(&mtal_spec);
 *  3.start play
 *     mtal_pb_play();
 *  4.stop
 *     mtal_pb_stop();
 *  5.destory
 *     mtal_pb_exit();
 *
 *
 *  6.deine setting
 *     CAPDEV_SENSOR_MAX_WIDTH , according sensor max Image resolution
 *     CAPDEV_SENSOR_MAX_HEIGHT, according sensor max Image resolution
 *     CAPDEV_MS_PER_FRAME         ,  capture thread update frame pre ms. ex: 60fps => 16ms
 *
 *
 */
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <limits.h>
#include "ite/itp.h"
#include "ith/ith_video.h"
#include "ite/audio.h"
#include "ite/itv.h"
#include "fc_sync.h"

#ifdef __OPENRTOS__
    #include "openrtos/FreeRTOS.h"
    #include "openrtos/queue.h"
#endif
// porting
#include "config.h"
#include "libavutil/avstring.h"
#include "libavutil/colorspace.h"
#include "libavutil/mathematics.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
#include "libavutil/dict.h"
#include "libavutil/parseutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/avassert.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
#include "libavcodec/audioconvert.h"
#include "libavutil/opt.h"
#include "libavcodec/avfft.h"
#include "libswresample/swresample.h"

#if CONFIG_AVFILTER
    #include "libavfilter/avcodec.h"
    #include "libavfilter/avfilter.h"
    #include "libavfilter/avfiltergraph.h"
    #include "libavfilter/buffersink.h"
#endif

#include "cmdutils.h"
#include <unistd.h>
#include <assert.h>
#include "fc_external.h"
#include "file_player.h"
#include "isp/mmp_isp.h"
#include "capture/capture_9860/mmp_capture.h"
#ifdef CFG_SENSOR_ENABLE
    #include "sensor/mmp_sensor.h"
#endif
///////////////////////////////////////////////////////////////////////////////////////////
// Definitions and type
///////////////////////////////////////////////////////////////////////////////////////////

#define CAPDEVQUEUESIZE          10
#define CAPDEV_SENSOR_MAX_WIDTH  1920
#define CAPDEV_SENSOR_MAX_HEIGHT 1080
#define CAPDEV_MS_PER_FRAME      16
#define CAPDEVTIMEOUT            100//100 * 10ms
#define SENSORTIMEOUT            100//100 * 10ms

typedef struct PacketQueue {
    AVPacketList    *first_pkt, *last_pkt;
    int             nb_packets;
    int             size;
    int             abort_request;
    int64_t         lastPts;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
} PacketQueue;

typedef struct PlayerInstance {
    pthread_t       video_tid;
    pthread_t       audio_tid;
    FC_STRC_SYNC    stc;
    // control
    char            filename[256];
    int             audio_disable;
    int             video_disable;
    int             abort_request;
    int             paused;
    int             last_paused;
    int             seek_req;
    int             seek_flags;
    int64_t         seek_pos;
    int64_t         seek_rel;
    int             read_pause_return;
    int             autoexit;
    int             loop;
    unsigned int    exclock, frame_count;
    bool            is_seekto_case;

    AVFormatContext *ic;

    // audio component
    int             audio_stream;
    AVStream        *audio_st;
    PacketQueue     audioq;
    int             audioq_size;
    pthread_mutex_t audioq_mutex;
    pthread_cond_t  audioq_cond;

    AVPacket        audio_pkt_temp;
    AVPacket        audio_pkt;
    double          audio_current_pts;
    double          audio_current_pts_drift;
    int             frame_drops_early;
    int             vol_level;
    // video component
    int             video_stream;
    AVStream        *video_st;
    PacketQueue     videoq;
    float           video_speed;

    // subtitle component
    int             subtitle_stream;
    AVStream        *subtitle_st;
    //PacketQueue subtitleq;

    double          frame_timer;
    double          frame_last_pts;
    double          frame_last_delay;
    double          frame_last_duration;
    double          frame_last_dropped_pts;
    double          frame_last_returned_time;
    double          frame_last_filter_delay;
    int64_t         frame_last_dropped_pos;
    double          video_clock; ///<pts of last decoded frame / predicted pts of next decoded frame
    double          video_current_pts;
    double          video_current_pts_drift;
    int64_t         video_current_pos;
    double          video_start_pos;
    bool            get_first_pkt;
#ifdef AV_SYNC_STC
    bool            get_first_I_frame;
    bool            audio_is_sync;
    bool            first_pkt_is_audio;
#endif
    // Used in test player
    int64_t         total_duration;
    AVStream        *is;
    AVCodecContext  *avctx;
    AVCodecContext  *videoctx;
    AVCodecContext  *audioctx;
    AVCodec         *codec;
} PlayerInstance;

typedef struct PlayerProps {
    pthread_t      read_tid;
    cb_handler_t   callback;

    // control flags
    int            show_status;
    int            av_sync_type;
    int            genpts;
    int            seek_by_bytes;
    int64_t        start_time;
    int            step;
    int            decoder_reorder_pts;
    int            mute;

    int            instCnt;
    PlayerInstance *inst;
} PlayerProps;

///////////////////////////////////////////////////////////////////////////////////////////
// Global Value
//
///////////////////////////////////////////////////////////////////////////////////////////
static pthread_mutex_t player_mutex;
static bool            is_thread_create    = false;
static PlayerProps     *global_player_prop = NULL;
static CAPTURE_HANDLE  gCapDev0;
static bool            gCaptureError = false;

#ifdef __OPENRTOS__
QueueHandle_t          gCapInfoQueue; // Queue use to interrupt
#endif
///////////////////////////////////////////////////////////////////////////////////////////
// Private Functions
//
///////////////////////////////////////////////////////////////////////////////////////////
static void
_CAP_ISR0(
    void *arg)
{
#ifdef __OPENRTOS__
    uint32_t       capture0state = 0, interrupt_status = 0;
    BaseType_t     gHigherPriorityTaskWoken = (BaseType_t) 0;
    CAPTURE_HANDLE *ptDev                   = (CAPTURE_HANDLE *) arg;

    capture0state = ithCapGetEngineErrorStatus(&gCapDev0, MMP_CAP_LANE0_STATUS);

    if (capture0state >> 31)
    {
        if ((capture0state >> 8) & 0xF)
        {
            //ithPrintf("cap0_isr err\n");
            ithPrintf("_CAP_ISR0=0x%x\n", capture0state);
            interrupt_status = 1;
            xQueueSendToBackFromISR(gCapInfoQueue, (void *)&interrupt_status, &gHigherPriorityTaskWoken);
            //clear cap0 interrupt and reset error status
            ithCapClearInterrupt(&gCapDev0, MMP_TRUE);
        }
        else
        {
            //ithPrintf("cap0_isr frame end\n");
            interrupt_status = 0;
            xQueueSendToBackFromISR(gCapInfoQueue, (void *)&interrupt_status, &gHigherPriorityTaskWoken);
            //clear cap0 interrupt
            ithCapClearInterrupt(&gCapDev0, MMP_FALSE);
        }
    }
    portYIELD_FROM_ISR(gHigherPriorityTaskWoken);
#endif
    return;
}

static int _SIGNALCHECK_FIRE(CAPTURE_HANDLE *ptDev)
{
    int         timeout  = 0;
    CAP_CONTEXT *Capctxt = &ptDev->cap_info;

    while ((ithCapGetEngineErrorStatus(ptDev, MMP_CAP_LANE0_STATUS) & 0x3) != 0x3)
    {
        if (++timeout > CAPDEVTIMEOUT)
            return 1;
        printf("Hsync or Vsync not stable!\n");
        usleep(10 * 1000);
    }

    ithCapFire(ptDev, MMP_TRUE);
    printf("Capture Fire! (%d)\n", ptDev->cap_id);

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
static void CaptureGetNewFrame(CAPTURE_HANDLE *ptDev, ITE_CAP_VIDEO_INFO *Outdata)
{
    int         cap_idx;
    CAP_CONTEXT *Capctxt = &ptDev->cap_info;

    Outdata->OutHeight    = Capctxt->outinfo.OutHeight;
    Outdata->OutWidth     = Capctxt->outinfo.OutWidth;
    Outdata->IsInterlaced = Capctxt->ininfo.Interleave;
    Outdata->PitchY       = Capctxt->ininfo.PitchY;
    Outdata->PitchUV      = Capctxt->ininfo.PitchUV;
    Outdata->OutMemFormat = Capctxt->outinfo.OutMemFormat;

    cap_idx               = ithCapReturnWrBufIndex(ptDev);

    switch (cap_idx)
    {
    case 0:
        Outdata->DisplayAddrY = (uint8_t *)Capctxt->OutAddrY[0];
        Outdata->DisplayAddrU = (uint8_t *)Capctxt->OutAddrUV[0];
        Outdata->DisplayAddrV = (uint8_t *)Capctxt->OutAddrUV[0];
        break;

    case 1:
        Outdata->DisplayAddrY = (uint8_t *)Capctxt->OutAddrY[1];
        Outdata->DisplayAddrU = (uint8_t *)Capctxt->OutAddrUV[1];
        Outdata->DisplayAddrV = (uint8_t *)Capctxt->OutAddrUV[1];
        break;

    case 2:
        Outdata->DisplayAddrY = (uint8_t *)Capctxt->OutAddrY[2];
        Outdata->DisplayAddrU = (uint8_t *)Capctxt->OutAddrUV[2];
        Outdata->DisplayAddrV = (uint8_t *)Capctxt->OutAddrUV[2];
        break;
    }

    //printf("[%d] AddrY = %x ,ADDU = %x\n",cap_idx,Outdata->DisplayAddrY,Outdata->DisplayAddrU);
}

static void ithCapdevPlayer_FlipLCD(CAPTURE_HANDLE *ptDev)
{
    uint8_t            *dbuf    = NULL;
    ITV_DBUF_PROPERTY  dbufprop = {0};
    ITE_CAP_VIDEO_INFO outdata  = {0};

#ifdef CFG_BUILD_ITV
    dbuf = itv_get_dbuf_anchor();
#endif
    if (dbuf == NULL)
    {
        printf("itv buffer full \n");
        return;
    }

    CaptureGetNewFrame(ptDev, &outdata);

    if (outdata.IsInterlaced)
    {
        itv_enable_isp_feature(MMP_ISP_DEINTERLACE);
    }
    dbufprop.src_w    = outdata.OutWidth;
    dbufprop.src_h    = outdata.OutHeight;
    dbufprop.pitch_y  = outdata.PitchY;
    dbufprop.pitch_uv = outdata.PitchUV;

    dbufprop.format   = MMP_ISP_IN_NV12;
    dbufprop.ya       = outdata.DisplayAddrY;
    dbufprop.ua       = outdata.DisplayAddrU;
    dbufprop.va       = outdata.DisplayAddrV;
#ifdef CFG_BUILD_ITV
    itv_update_dbuf_anchor(&dbufprop);
#endif
}

void *read_thread_cap(void *arg)
{
    PlayerProps    *pprop           = (PlayerProps *) arg;
    PlayerInstance *is;
    int            ret              = 0;
    int            interrupt_status = 0;
    unsigned int   first_tick       = 0;
    unsigned int   diff_tick        = 0;
    int            delay_time       = 0;

    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        ret = -1;
        goto fail;
    }

    is                  = pprop->inst;

    // player instances initial
    is->video_stream    = -1;
    is->audio_stream    = -1;
    is->subtitle_stream = -1;
    is->exclock         = 0;
    is->frame_count     = 0;
    is->total_duration  = 0;

    for (;;)
    {
        PlayerInstance *is = pprop->inst;
        if (is->abort_request)
        {
            printf("should not run here!!!\n");
            break;
        }
#ifdef __OPENRTOS__
        first_tick = itpGetTickCount();
        if (xQueueReceive(gCapInfoQueue, &interrupt_status, 0))
        {
            //printf("get queue = %d\n", intrrupt_status);
            if (interrupt_status == 0)
            {
                ithCapdevPlayer_FlipLCD(&gCapDev0);
            }
            else
            {
                gCaptureError = true;
                break;
            }
        }
        if (ithCapIsFire(&gCapDev0) == false)
        {
            if (ithCapDeviceIsSignalStable())
            {
                ithCapGetDeviceInfo(&gCapDev0);
                ithCapParameterSetting(&gCapDev0);
                _SIGNALCHECK_FIRE(&gCapDev0);
                first_tick = itpGetTickCount();
            }
        }
        
        diff_tick = itpGetTickDuration(first_tick);
        delay_time = CAPDEV_MS_PER_FRAME - diff_tick;
        if(delay_time < 1) delay_time = 1;

#endif
        //printf("CAP sleep ms = %d \n", delay_time);
        usleep(delay_time * 1000);
    }

fail:
    printf("terminate video\n");

    if (ret != 0)
        av_log(NULL, AV_LOG_ERROR, "read_thread failed, ret=%d\n", ret);
    else
        av_log(NULL, AV_LOG_ERROR, "read thread %x done\n", pprop->read_tid);

    if(gCaptureError)
    {
         pprop->callback(PLAYER_EVENT_CAPTURE_DEV, (void*)NULL);
         gCaptureError = false;
    }

    pthread_exit(NULL);
    return 0;
}

static void stream_close(PlayerProps *pprop)
{
    void           *status;
    PlayerInstance *is;

    av_log(NULL, AV_LOG_DEBUG, "stream close\n");

    if ((!pprop) || (!pprop->inst))
        return;

    /* make sure one complete frame is done */
    pprop->inst->abort_request = 1;
    pthread_join(pprop->read_tid, &status);

    /* free all pictures */
    is = pprop->inst;

    if (!is)
        return;

    is->abort_request = 0;
#ifdef CFG_BUILD_ITV
    fc_deinit_sync(&is->stc);
#endif
    free(pprop->inst);
    pprop->inst    = NULL;
    pprop->instCnt = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////
// Public Functions
//
///////////////////////////////////////////////////////////////////////////////////////////

static int ithCapdevPlayer_init(cb_handler_t callback)
{
    PlayerProps    *pprop        = NULL;
    PlayerInstance *inst         = NULL;
    uint16_t       bSignalStable = 0;
    MMP_ISP_SHARE   isp_share = {0};
    printf("call = %s\n", __FUNCTION__);

    if (global_player_prop)
        return 1;

#ifdef DEBUG
    av_log_set_level(AV_LOG_DEBUG);
#endif

    pprop = (PlayerProps *) calloc(sizeof(char), sizeof(PlayerProps));
    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Initialize player failed\n");
        return -1;
    }

    // player properties initial, TODO use API later
    pthread_mutex_init(&player_mutex, NULL);
    pprop->callback      = callback;
    pprop->show_status   = 1;
    pprop->seek_by_bytes = -1;
    pprop->start_time    = AV_NOPTS_VALUE;

    inst                 = (PlayerInstance *) calloc(1, sizeof(PlayerInstance));

    if (!inst)
        av_log(NULL, AV_LOG_ERROR, "not enough memory\n");

    if (pprop->inst)
        free(pprop->inst);
    pprop->inst        = inst;
    pprop->instCnt     = 1;

    global_player_prop = pprop;
    is_thread_create   = false;
#ifdef __OPENRTOS__
    gCapInfoQueue      = xQueueCreate(CAPDEVQUEUESIZE, (unsigned portBASE_TYPE) sizeof(int));
#endif

    /*power on*/
    ithCapPowerUp();

#if defined(CFG_SENSOR_ENABLE)
    /*Sensor init */
    ithCapDeviceInitialize();
#endif

    /*capture init*/
    ithCapInitialize();
    CAPTURE_SETTING mem_modeset = {MMP_CAP_DEV_SENSOR, MMP_FALSE,  MMP_TRUE, CAPDEV_SENSOR_MAX_WIDTH, CAPDEV_SENSOR_MAX_HEIGHT};
    ithCapConnect(&gCapDev0, mem_modeset);
    ithCapRegisterIRQ(_CAP_ISR0, &gCapDev0);
    
    return 0;
}

static int ithCapdevPlayer_select_file(const char *filename, int level)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithCapdevPlayer_play(void)
{
    PlayerProps    *pprop = global_player_prop;
    PlayerInstance *is    = NULL;
    int            rc;
    printf("call = %s\n", __FUNCTION__);
    pthread_mutex_lock(&player_mutex);

    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        pthread_mutex_unlock(&player_mutex);
        return -1;
    }

    is = pprop->inst;
    if (!is)
    {
        av_log(NULL, AV_LOG_ERROR, "No assigned stream in player\n");
        pthread_mutex_unlock(&player_mutex);
        return -1;
    }

    if (!is_thread_create)
    {
        is->get_first_pkt      = false;
#ifdef AV_SYNC_STC
        is->get_first_I_frame  = false;
        is->audio_is_sync      = false;
        is->first_pkt_is_audio = false;
#endif
        is->video_start_pos    = 0;
        is->frame_timer        = 0;
        is->frame_last_delay   = 0;
        is->frame_last_pts     = 0;

        rc                     = pthread_create(&pprop->read_tid, NULL, read_thread_cap, (void *)pprop);
        if (rc)
        {
            av_log(NULL, AV_LOG_ERROR, "create thread failed %d\n", rc);
        }
    }

    is_thread_create = true;
    pthread_mutex_unlock(&player_mutex);
    return 0;
}

static int ithCapdevPlayer_stop(void)
{
    PlayerProps    *pprop = global_player_prop;
    PlayerInstance *is    = NULL;

    printf("call = %s\n", __FUNCTION__);
    pthread_mutex_lock(&player_mutex);

    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        pthread_mutex_unlock(&player_mutex);
        return -1;
    }

    is = pprop->inst;
    stream_close(pprop);
    ithCapTerminate();
    /* Release video request memory buffers */
    ithCapDisConnect(&gCapDev0);    
    ithCapPowerDown();
#if defined(CFG_SENSOR_ENABLE)
    ithCapDeviceTerminate();
#endif
    is_thread_create = false;

    /* default set RMI active index to 1 (single instance) */
#ifdef CFG_BUILD_ITV
    itv_flush_dbuf();
#endif

    pthread_mutex_unlock(&player_mutex);
    return 0;
}

static int ithCapdevPlayer_deinit()
{
    PlayerProps *pprop = global_player_prop;
    printf("call = %s\n", __FUNCTION__);

    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        return -1;
    }

    global_player_prop = NULL;

    /* Release PlayerProps if any */
    if (pprop && pprop->show_status)
    {
        if (pprop->inst)
            free(pprop->inst);
        free(pprop);
        pprop = NULL;
        printf("\n");
    }

#ifdef __OPENRTOS__
    vQueueDelete(gCapInfoQueue);
#endif
    pthread_mutex_destroy(&player_mutex);
    return 0;
}

/* pause or resume the video */
static int ithCapdevPlayer_pause(void)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithCapdevPlayer_play_videoloop(void)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithCapdevPlayer_get_total_duration(int64_t *total_time)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithCapdevPlayer_get_total_duration_ext(int64_t *total_time, char *filepath)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithCapdevPlayer_get_current_time(int64_t *current_time)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithCapdevPlayer_seekto(int pos)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithCapdevPlayer_slow_fast_play(float speed)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithCapdevPlayer_get_file_pos(double *pos)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithCapdevPlayer_volume_up(void)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithCapdevPlayer_volume_down(void)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithCapdevPlayer_mute(void)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

int ithCapdevPlayer_drop_all_input_streams(void)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int video_thread_for_rtsp_client(void *arg)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int audio_thread_for_rtsp_client(void *arg)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static void ithCapdevPlayer_InitAVDecodeEnv()
{
    printf("call = %s\n", __FUNCTION__);
}

static void ithCapdevPlayer_InitH264DecodeEnv()
{
    printf("call = %s\n", __FUNCTION__);
}

static void ithCapdevPlayer_InitAudioDecodeEnv(int samplerate, int num_channels, RTSPCLIENT_AUDIO_CODEC codec_id)
{
    printf("call = %s\n", __FUNCTION__);
}

static int ithCapdevPlayer_h264_decode_from_rtsp(unsigned char *inputbuf, int inputbuf_size, double timestamp)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithCapdevPlayer_audio_decode_from_rtsp(unsigned char *inputbuf, int inputbuf_size, double timestamp)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static void ithCapdevPlayer_DeinitAVDecodeEnv()
{
    printf("call = %s\n", __FUNCTION__);
    return;
}

#if defined(_MSC_VER)
ithMediaPlayer captureplayer = {
    ithCapdevPlayer_init,
    ithCapdevPlayer_select_file,
    ithCapdevPlayer_play,
    ithCapdevPlayer_pause,
    ithCapdevPlayer_stop,
    ithCapdevPlayer_play_videoloop,
    ithCapdevPlayer_deinit,
    ithCapdevPlayer_get_total_duration,
    ithCapdevPlayer_get_total_duration_ext,
    ithCapdevPlayer_get_current_time,
    ithCapdevPlayer_seekto,
    ithCapdevPlayer_slow_fast_play,
    ithCapdevPlayer_get_file_pos,
    ithCapdevPlayer_volume_up,
    ithCapdevPlayer_volume_down,
    ithCapdevPlayer_mute,
    ithCapdevPlayer_drop_all_input_streams,
    ithCapdevPlayer_InitAVDecodeEnv,
    ithCapdevPlayer_InitH264DecodeEnv,
    ithCapdevPlayer_InitAudioDecodeEnv,
    ithCapdevPlayer_h264_decode_from_rtsp,
    ithCapdevPlayer_audio_decode_from_rtsp,
    ithCapdevPlayer_DeinitAVDecodeEnv
};
#else // no defined _MSC_VER
ithMediaPlayer captureplayer = {
    .init                   = ithCapdevPlayer_init,
    .select                 = ithCapdevPlayer_select_file,
    .play                   = ithCapdevPlayer_play,
    .pause                  = ithCapdevPlayer_pause,
    .stop                   = ithCapdevPlayer_stop,
    .play_videoloop         = ithCapdevPlayer_play_videoloop,
    .deinit                 = ithCapdevPlayer_deinit,
    .gettotaltime           = ithCapdevPlayer_get_total_duration,
    .gettotaltime_ext       = ithCapdevPlayer_get_total_duration_ext,
    .getcurrenttime         = ithCapdevPlayer_get_current_time,
    .seekto                 = ithCapdevPlayer_seekto,
    .slow_fast_play         = ithCapdevPlayer_slow_fast_play,
    .getfilepos             = ithCapdevPlayer_get_file_pos,
    .vol_up                 = ithCapdevPlayer_volume_up,
    .vol_down               = ithCapdevPlayer_volume_down,
    .mute                   = ithCapdevPlayer_mute,
    .drop_all_input_streams = ithCapdevPlayer_drop_all_input_streams,
    .InitAVDecodeEnv        = ithCapdevPlayer_InitAVDecodeEnv,
    .InitH264DecodeEnv      = ithCapdevPlayer_InitH264DecodeEnv,
    .InitAudioDecodeEnv     = ithCapdevPlayer_InitAudioDecodeEnv,
    .h264_decode_from_rtsp  = ithCapdevPlayer_h264_decode_from_rtsp,
    .audio_decode_from_rtsp = ithCapdevPlayer_audio_decode_from_rtsp,
    .DeinitAVDecodeEnv      = ithCapdevPlayer_DeinitAVDecodeEnv
};
#endif
//ithMediaPlayer *media_player = &captureplayer;
