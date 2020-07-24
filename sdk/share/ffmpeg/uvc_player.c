/*
 * iTE castor3 media player for usb camera
 *
 * @file:uvc_player.c
 * @version 1.0.0
 * @How to use:
 *
 *  1.open Kconfig (USB0 Enable ||USB1 Enable)  && USB Video Class(UVC_ENABLE)
 *
 *  2.set play window size
 *     itv_set_video_window(x, y, width, height);
 *  3.init
 *     mtal_pb_init(EventHandler);
 *     mtal_spec.camera_in = USB_IN;
 *     mtal_pb_select_file(&mtal_spec);
 *
 *  4.mtal_set_camera_parameter(w,h,fps);
 *
 *  5.start play
 *     mtal_pb_play();
 *
 *  6.stop
 *     mtal_pb_stop();
 *
 *  7.destory
 *     mtal_pb_exit();
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
#include <unistd.h>
#include <assert.h>

#include "ite/itp.h"
#include "ith/ith_video.h"
#include "ite/itv.h"
#include "config.h"
#include "cmdutils.h"
#include "fc_external.h"
#include "file_player.h"
#include "libuvc/libuvc.h"
///////////////////////////////////////////////////////////////////////////////////////////
// Definitions and type
///////////////////////////////////////////////////////////////////////////////////////////

typedef struct YuyvPkt {
    void     *data;
    size_t   data_bytes;
    uint32_t width;
    uint32_t height;
} YuyvPkt;

///////////////////////////////////////////////////////////////////////////////////////////
// Global Value
//
///////////////////////////////////////////////////////////////////////////////////////////
static pthread_mutex_t     player_mutex;
static uvc_device_t        *dev;
static uvc_context_t       *ctx;
static uvc_device_handle_t *devh;
static uvc_stream_ctrl_t   ctrl;
static uvc_error_t         device_res;
static uvc_error_t         streaming_res;
static int                 UVC_width  = 640;
static int                 UVC_height = 480;
static int                 UVC_fps    = 25;
///////////////////////////////////////////////////////////////////////////////////////////
// Private Functions
//
///////////////////////////////////////////////////////////////////////////////////////////

/* This callback function runs once per frame. Use it to perform any
 * quick processing you need, or have it put the frame into your application's
 * input queue. If this function takes too long, you'll start losing frames. */
static void
_uvcFrameCb(
    uvc_frame_t *frame,
    void        *ptr)
{
    uint8_t           *dbuf    = NULL;
    ITV_DBUF_PROPERTY dbufprop = {0};

    if (frame->data_bytes && frame->data)
    {
        switch (frame->frame_format)
        {
        case UVC_FRAME_FORMAT_MJPEG:
#if 0
            {
                JpegInputPkt tJpegPkt = { 0 };
                tJpegPkt.pInputBuffer = frame->data;
                tJpegPkt.bufferSize   = frame->data_bytes;
                packetQueuePut(&gtJpegInputQueue, &tJpegPkt);
                break;
            }
#endif
            break;
        case UVC_FRAME_FORMAT_YUYV:
            {
                int expected_size = frame->width * frame->height * 2;
                // The received data may be wrong, so the amount of data received
                // needed to be checked to confirm whether  the complete video frame
                // have been received.
                if (expected_size == frame->data_bytes)
                {
                    // When cpu write back mode is enabled, cache must be flushed
                    // before the h/w ISP engine starting to do color conversion.
                    // Otherwise, the h/w ISP engine may read old memory data
                    // and there will be some noise seen on the screen.
                    ithFlushDCacheRange(frame->data, frame->data_bytes);

#ifdef CFG_BUILD_ITV
                    dbuf = itv_get_dbuf_anchor();
                    if (dbuf == NULL)
                    {
                        printf("itv buffer full \n");
                        break;
                    }

                    dbufprop.src_w    = frame->width;
                    dbufprop.src_h    = frame->height;
                    dbufprop.pitch_y  = frame->width * 2;
                    dbufprop.pitch_uv = frame->width;

                    dbufprop.format   = MMP_ISP_IN_YUYV;
                    dbufprop.ya       = frame->data;
                    dbufprop.ua       = frame->data;
                    dbufprop.va       = frame->data;

                    itv_update_dbuf_anchor(&dbufprop);
#endif
                }

                break;
            }
        }
    }
}

void _yuyvPktRelease(
    void *pkt)
{
    YuyvPkt *ptPkt = (YuyvPkt *)pkt;
    if (ptPkt && ptPkt->data && ptPkt->data_bytes)
    {
        free(ptPkt->data);
    }
    free(pkt);
}

///////////////////////////////////////////////////////////////////////////////////////////
// Public Functions
//
///////////////////////////////////////////////////////////////////////////////////////////

static int ithUVCPlayer_init(cb_handler_t callback)
{
    uvc_error_t res;

    printf("call = %s\n", __FUNCTION__);

    /* Initialize a UVC service context. Libuvc will set up its own libusb
     * context. Replace NULL with a libusb_context pointer to run libuvc
     * from an existing libusb context.                                               */
    res = uvc_init(&ctx);

    if (res < 0)
    {
        uvc_perror(res, "uvc_init");
        return res;
    }
    pthread_mutex_init(&player_mutex, NULL);
    printf("UVC initialized\n");

    return 0;
}

static int ithUVCPlayer_select_file(const char *filename, int level)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithUVCPlayer_play(void)
{
    printf("call = %s\n", __FUNCTION__);
    pthread_mutex_lock(&player_mutex);

    /* Locates the first attached UVC device, stores in dev */
    device_res = uvc_find_device(ctx, &dev, 0, 0, NULL); /* filter devices: vendor_id, product_id, "serial_num" */

    if (device_res < 0)
    {
        uvc_perror(device_res, "uvc_find_device"); /* no devices found */
    }
    else
    {
        printf("Device found");

        /* Try to open the device: requires exclusive access */
        device_res = uvc_open(dev, &devh);
        if (device_res < 0)
        {
            uvc_perror(device_res, "uvc_open"); /* unable to open device */
        }
        else
        {
            printf("Device opened");
            /* Print out a message containing all the information that libuvc knows about the device */
            uvc_print_diag(devh, stderr);

            const uvc_format_desc_t *fmt_desc = uvc_get_format_descs(devh);
            switch (fmt_desc->bDescriptorSubtype)
            {
            case UVC_VS_FORMAT_UNCOMPRESSED:
                // fmt_desc->guidFormat
                break;

            case UVC_VS_FORMAT_MJPEG:
                break;
            }

            streaming_res = uvc_get_stream_ctrl_format_size(
                devh, &ctrl,                     /* result stored in ctrl */
                UVC_FRAME_FORMAT_YUYV,           /* YUV 422, aka YUV 4:2:2. try _COMPRESSED */
                UVC_width, UVC_height, UVC_fps); /* width, height, fps */

            /* Print out the result */
            uvc_print_stream_ctrl(&ctrl, stderr);

            if (streaming_res < 0)
            {
                uvc_perror(streaming_res, "get_mode"); /* device doesn't provide a matching stream */
            }
            else
            {
                //packetQueueInit(&gtYuyvQueue, _yuyvPktRelease, 4, sizeof(YuyvPkt));
                /* Start the video stream. The library will call user function cb:  * cb(frame, (void*) 12345) */

                streaming_res = uvc_start_streaming(devh, &ctrl, _uvcFrameCb, NULL, 0);

                if (streaming_res < 0)
                {
                    uvc_perror(streaming_res, "start_streaming"); /* unable to start stream */
                }
                else
                {
                    printf("Streaming...");

                    uvc_set_ae_mode(devh, 1);  /* e.g., turn on auto exposure */

                    //sleep(TOTAL_TEST_SECONDS); /* stream for (TOTAL_TEST_SECONDS) seconds */
                }
            }
        }
    }

    pthread_mutex_unlock(&player_mutex);
    return 0;
}

static int ithUVCPlayer_stop(void)
{
    printf("call = %s\n", __FUNCTION__);
    pthread_mutex_lock(&player_mutex);

    if (device_res >= 0 && streaming_res >= 0)
    {
        printf("Done streaming.");
        /* End the stream. Blocks until last callback is serviced */
        uvc_stop_streaming(devh);
    }
    if (device_res >= 0)
    {
        /* Release our handle on the device */
        uvc_close(devh);
        printf("Device closed");
    }

    /* default set RMI active index to 1 (single instance) */
#ifdef CFG_BUILD_ITV
    itv_flush_dbuf();
#endif

    pthread_mutex_unlock(&player_mutex);
    return 0;
}

static int ithUVCPlayer_deinit()
{
    printf("call = %s\n", __FUNCTION__);

    uvc_exit(ctx);
    printf("UVC exited");
    //puts("Wait free queue allocated resource done");
    //packetQueueEnd(&gtYuyvQueue);
    //puts("Free queue allocated resource is done");

    /* Release video request memory buffers */
    pthread_mutex_destroy(&player_mutex);
    return 0;
}

/* */
static void ithUVCPlayer_SetCameraParameter(int width, int height, int fps)
{
    pthread_mutex_lock(&player_mutex);
    UVC_width  = width;
    UVC_height = height;
    UVC_fps    = fps;
    pthread_mutex_unlock(&player_mutex);
    printf("width = %d, height = %d, fps =%d\n", width, height, fps);
}

/* pause or resume the video */
static int ithUVCPlayer_pause(void)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithUVCPlayer_play_videoloop(void)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithUVCPlayer_get_total_duration(int64_t *total_time)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithUVCPlayer_get_total_duration_ext(int64_t *total_time, char *filepath)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithUVCPlayer_get_current_time(int64_t *current_time)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithUVCPlayer_seekto(int pos)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithUVCPlayer_slow_fast_play(float speed)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithUVCPlayer_get_file_pos(double *pos)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithUVCPlayer_volume_up(void)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithUVCPlayer_volume_down(void)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithUVCPlayer_mute(void)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

int ithUVCPlayer_drop_all_input_streams(void)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static void ithUVCPlayer_InitAVDecodeEnv()
{
    printf("call = %s\n", __FUNCTION__);
}

static void ithUVCPlayer_InitH264DecodeEnv()
{
    printf("call = %s\n", __FUNCTION__);
}

static void ithUVCPlayer_InitAudioDecodeEnv(int samplerate, int num_channels, RTSPCLIENT_AUDIO_CODEC codec_id)
{
    printf("call = %s\n", __FUNCTION__);
}

static int ithUVCPlayer_h264_decode_from_rtsp(unsigned char *inputbuf, int inputbuf_size, double timestamp)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static int ithUVCPlayer_audio_decode_from_rtsp(unsigned char *inputbuf, int inputbuf_size, double timestamp)
{
    printf("call = %s\n", __FUNCTION__);
    return 0;
}

static void ithUVCPlayer_DeinitAVDecodeEnv()
{
    printf("call = %s\n", __FUNCTION__);
    return;
}

#if defined(_MSC_VER)
ithMediaPlayer uvcplayer = {
    ithUVCPlayer_init,
    ithUVCPlayer_select_file,
    ithUVCPlayer_play,
    ithUVCPlayer_pause,
    ithUVCPlayer_stop,
    ithUVCPlayer_play_videoloop,
    ithUVCPlayer_deinit,
    ithUVCPlayer_get_total_duration,
    ithUVCPlayer_get_total_duration_ext,
    ithUVCPlayer_get_current_time,
    ithUVCPlayer_seekto,
    ithUVCPlayer_slow_fast_play,
    ithUVCPlayer_get_file_pos,
    ithUVCPlayer_volume_up,
    ithUVCPlayer_volume_down,
    ithUVCPlayer_mute,
    ithUVCPlayer_drop_all_input_streams,
    ithUVCPlayer_InitAVDecodeEnv,
    ithUVCPlayer_InitH264DecodeEnv,
    ithUVCPlayer_InitAudioDecodeEnv,
    ithUVCPlayer_h264_decode_from_rtsp,
    ithUVCPlayer_audio_decode_from_rtsp,
    ithUVCPlayer_DeinitAVDecodeEnv,
    ithUVCPlayer_SetCameraParameter
};
#else // no defined _MSC_VER
ithMediaPlayer uvcplayer = {
    .init                   = ithUVCPlayer_init,
    .select                 = ithUVCPlayer_select_file,
    .play                   = ithUVCPlayer_play,
    .pause                  = ithUVCPlayer_pause,
    .stop                   = ithUVCPlayer_stop,
    .play_videoloop         = ithUVCPlayer_play_videoloop,
    .deinit                 = ithUVCPlayer_deinit,
    .gettotaltime           = ithUVCPlayer_get_total_duration,
    .gettotaltime_ext       = ithUVCPlayer_get_total_duration_ext,
    .getcurrenttime         = ithUVCPlayer_get_current_time,
    .seekto                 = ithUVCPlayer_seekto,
    .slow_fast_play         = ithUVCPlayer_slow_fast_play,
    .getfilepos             = ithUVCPlayer_get_file_pos,
    .vol_up                 = ithUVCPlayer_volume_up,
    .vol_down               = ithUVCPlayer_volume_down,
    .mute                   = ithUVCPlayer_mute,
    .drop_all_input_streams = ithUVCPlayer_drop_all_input_streams,
    .InitAVDecodeEnv        = ithUVCPlayer_InitAVDecodeEnv,
    .InitH264DecodeEnv      = ithUVCPlayer_InitH264DecodeEnv,
    .InitAudioDecodeEnv     = ithUVCPlayer_InitAudioDecodeEnv,
    .h264_decode_from_rtsp  = ithUVCPlayer_h264_decode_from_rtsp,
    .audio_decode_from_rtsp = ithUVCPlayer_audio_decode_from_rtsp,
    .DeinitAVDecodeEnv      = ithUVCPlayer_DeinitAVDecodeEnv,
    .SetCameraParameter     = ithUVCPlayer_SetCameraParameter,
};
#endif