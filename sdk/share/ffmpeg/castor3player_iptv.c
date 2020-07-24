/*
 * iTE castor3 media player
 *
 * @file    castor3player.c
 * @author  Evan Chang
 * @version 1.0.0
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
#include "ite/itc.h"

//#include "isp/mmp_isp.h"
// PalGetClock & PalGetDuration

#ifdef __OPENRTOS__
#include "../../../openrtos/include/openrtos/FreeRTOS.h"
#include "../../../openrtos/include/openrtos/task.h"
#endif

#include "ite/itv_iptv.h"
//#include "fc_bm.h"

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
# include "libavfilter/avcodec.h"
# include "libavfilter/avfilter.h"
# include "libavfilter/avfiltergraph.h"
# include "libavfilter/buffersink.h"
#endif

//#include "SDL/SDL.h"
//#include <SDL_thread.h>

#include "cmdutils.h"

#include <unistd.h>
#include <assert.h>

//#include "isp/itv_mmp_isp.h"
#include "fc_external.h"
#include "castor3player_iptv.h"

// FIXME: if exist
//#include "ts_source.h"

///////////////////////////////////////////////////////////////////////////////////////////
// Definitions and type
///////////////////////////////////////////////////////////////////////////////////////////

#define TOTAL_FRAME_BUFFER_CNT  4
#define EACH_CMD_DATA_BUF_H     512 // 250

//#define DEBUG
//#define LCD_OUTPUT
#define ONFLY_MODE
#define MULTI_CHANNEL   1 //remember modify itv.h, replace ITV_MAX_RMI_NINS from 1 to 4
#ifndef WIN32
#define MULTITHREAD_IDO 1
#else
#define MULTITHREAD_IDO 0
#endif

#if MULTITHREAD_IDO

#define MAX_QUEUE_SIZE      (1280 * 1024) // evaluatation under 20 Mbps
#define MIN_AUDIOQ_SIZE     (1 * 16 * 1024)
#define MIN_FRAMES      (18)
#define FRAME_QUEUE_THRESHOLD   (1)

#define AV_RESYNC_GAP       (200 * 90)
#define NULL_PACKET_RETRY_COUNT 10
#define FIRST_VIDEO_WAIT_COUNT  0
#define RESYNC_VIDEO_UPPER_THRESHOLD  2
#define RESYNC_VIDEO_LOWER_THRESHOLD  1
#define VIDEO_WAIT_TIME         240

#define NEED_RESYNC_NULL_STATE          0
#define NEED_RESYNC_START_STATE         1
#define NEED_RESYNC_WAIT_VIDEO_FLUSH    2
#define NEED_RESYNC_VIDEO_FLUSH_DONE    3
#define NEED_RESYNC_DONE_STATE          4

#define VIDEO_DCXO_CHECK_COUNT  10

#define AUDIO_MAX_INIT_CHECK_AV_GAP (800 * 90)
#define AUDIO_MIN_INIT_CHECK_AV_GAP -(100 * 90)
#define AUDIO_CHECK_FRAME_GAP       (3000)
#define AUDIO_ADJUST_COUNT          20
#define AUDIO_JUDGE_COUNT           10
#define AUDIO_JUDGE_RANGE           (20 * 90)

#define VIDEO_JUDGE_COUNT           10
#define VIDEO_ADJUST_COUNT          20
#define VIDEO_ADJUST_CHECK_RANGE    1

//#define VIDEO_ADJUST_CLOCK

typedef struct PacketQueue {
    AVPacketList *first_pkt, *last_pkt;
    int nb_packets;
    int size;
    int abort_request;
    int64_t lastPts;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} PacketQueue;

#endif // MULTITHREAD_IDO

typedef struct PlayerInstance {
    // control
    char filename[256];
    int audio_disable;
    int video_disable;
    volatile int abort_request;
    int paused;
    int last_paused;
    int seek_req;
    int seek_flags;
    int64_t seek_pos;
    int64_t seek_rel;
    int read_pause_return;
    int autoexit;
    int loop;
    unsigned int exclock, frame_count;

    AVFormatContext *ic;

    // audio component
    int audio_stream;
    AVStream *audio_st;
#if MULTITHREAD_IDO
    PacketQueue audioq;
#endif
    int audioq_size;
    pthread_mutex_t audioq_mutex;
    pthread_cond_t audioq_cond;

    AVPacket audio_pkt_temp;
    AVPacket audio_pkt;
    double audio_current_pts;
    double audio_current_pts_drift;
    int frame_drops_early;
    // video component
    int video_stream;
    AVStream *video_st;
#if MULTITHREAD_IDO
    PacketQueue videoq;
#endif

    // subtitle component
    int subtitle_stream;
    AVStream *subtitle_st;
    //PacketQueue subtitleq;

    double frame_timer;
    double frame_last_pts;
    double frame_last_duration;
    double frame_last_dropped_pts;
    double frame_last_returned_time;
    double frame_last_filter_delay;
    int64_t frame_last_dropped_pos;
    double video_clock; ///<pts of last decoded frame / predicted pts of next decoded frame
    double video_current_pts;
    double video_current_pts_drift;
    int64_t video_current_pos;
    float  frameRate;
    // Used in test player
    AVStream *is;
    AVCodecContext *avctx;
    AVCodec *codec;

    /* Evan, to resolve source stream width and height */
    unsigned src_w;
    unsigned src_h;

    int64_t  latest_video_decode_pts;
    int64_t  latest_audio_decode_pts;
    int is_audio_sync;
    int is_video_sync;
    int need_resync_state;
    int int_video_buf_time;
    double init_av_gap;
    pthread_mutex_t resync_mux;
	double video_pts;
} PlayerInstance;

typedef struct PlayerProps {
    pthread_t read_tid;
#if MULTITHREAD_IDO
    pthread_t video_tid;
    pthread_t audio_tid;
#endif

    // control flags
    int show_status;
    int av_sync_type;
    int genpts;
    int seek_by_bytes;
    int64_t start_time;
    int step;
    int decoder_reorder_pts;
    int mute;
    VIDEO_DECODE_MODE mode;

    int instCnt;
    PlayerInstance *inst[MULTI_CHANNEL];
    int frame_buf_cnt;
    int bStreamOpen;
} PlayerProps;

enum {
    AV_SYNC_AUDIO_MASTER, /* default choice */
    AV_SYNC_VIDEO_MASTER,
    AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
};

///////////////////////////////////////////////////////////////////////////////////////////
// global var
//
///////////////////////////////////////////////////////////////////////////////////////////

/*static*/ PlayerProps *global_player_prop = NULL;
#if MULTITHREAD_IDO
AVPacket flush_pkt;
#endif

///////////////////////////////////////////////////////////////////////////////////////////
// Functions decl
//
///////////////////////////////////////////////////////////////////////////////////////////

static int ithMediaPlayer_stop(void);
int ithMediaPlayer_flush(void* prop);

///////////////////////////////////////////////////////////////////////////////////////////
// Private Functions
//
///////////////////////////////////////////////////////////////////////////////////////////

#if MULTITHREAD_IDO
static int packet_queue_put(PacketQueue *q, AVPacket *pkt)
{
    AVPacketList *pkt1;

    /* duplicate the packet */
    if (q->mutex)
    {
        if (pkt != &flush_pkt && av_dup_packet(pkt) < 0)
            return -1;

        pkt1 = av_malloc(sizeof(AVPacketList));
        if (!pkt1) return -1;
        pkt1->pkt = *pkt;
        pkt1->next = NULL;

        pthread_mutex_lock(&q->mutex);

        if (!q->last_pkt)
            q->first_pkt = pkt1;
        else
            q->last_pkt->next = pkt1;
        q->last_pkt = pkt1;
        q->nb_packets++;
        q->size += pkt1->pkt.size + sizeof(*pkt1);
        q->lastPts = pkt->pts;
        /* XXX: should duplicate packet data in DV case */
        pthread_cond_signal(&q->cond);
        pthread_mutex_unlock(&q->mutex);
        return 0;
    }
    else
    {
        return -1;
    }
}

/* packet queue handling */
static void packet_queue_init(PacketQueue *q)
{
    memset(q, 0, sizeof(PacketQueue));
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
    //packet_queue_put(q, &flush_pkt);
}

static void packet_queue_flush(PacketQueue *q)
{
    AVPacketList *pkt, *pkt1;
    if (q->mutex)
    {
        pthread_mutex_lock(&q->mutex);
        for (pkt = q->first_pkt; pkt != NULL; pkt = pkt1) {
            pkt1 = pkt->next;
            av_free_packet(&pkt->pkt);
            av_freep(&pkt);
        }
        q->last_pkt = NULL;
        q->first_pkt = NULL;
        q->nb_packets = 0;
        q->size = 0;
        pthread_mutex_unlock(&q->mutex);
    }
}

static void packet_queue_end(PacketQueue *q)
{
    if (q->mutex)
    {
        packet_queue_flush(q);
        pthread_mutex_destroy(&q->mutex);
        pthread_cond_destroy(&q->cond);
        memset(q, 0, sizeof(PacketQueue));
    }
}

static void packet_queue_abort(PacketQueue *q)
{
    if (q->mutex)
    {
        pthread_mutex_lock(&q->mutex);
        q->abort_request = 1;
        pthread_cond_signal(&q->cond);
        pthread_mutex_unlock(&q->mutex);
    }
}

/* return < 0 if aborted, 0 if no packet and > 0 if packet. */
static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block)
{
    AVPacketList *pkt1;
    int ret;
    if (q->mutex)
    {
        pthread_mutex_lock(&q->mutex);

        for (;;)
        {
            if (q->abort_request)
            {
                ret = -1;
                break;
            }

            pkt1 = q->first_pkt;
            if (pkt1)
            {
                q->first_pkt = pkt1->next;
                if (!q->first_pkt)
                    q->last_pkt = NULL;
                q->nb_packets--;
                q->size -= pkt1->pkt.size + sizeof(*pkt1);
                *pkt = pkt1->pkt;
                av_free(pkt1);
                ret = 1;
                break;
            }
            else if (!block)
            {
                ret = 0;
                break;
            }
            else
            {
                av_log(NULL, AV_LOG_DEBUG, "queue empty, condition wait %lld\n", av_gettime());
                pthread_cond_wait(&q->cond, &q->mutex);
                av_log(NULL, AV_LOG_DEBUG, "leave condition wait %lld\n", av_gettime());
            }
        }
        pthread_mutex_unlock(&q->mutex);
        return ret;
    }
    else
    {
        return -1;
    }
}

#endif // MULTITHREAD_IDO

static int decode_interrupt_cb(void *ctx)
{
   return (global_player_prop && global_player_prop->inst[0]->abort_request);
}

#ifdef WIN32
void SaveFrame(AVFrame *pFrame, int width, int height) {
    FILE *pFile;
    char szFilename[64];
    int  y;
    static int iFrame;
    uint8_t* data;

    // Open file
    sprintf(szFilename, "d:\\yuv\\%08d.yuv", iFrame++);
    pFile=fopen(szFilename, "wb");
    if(pFile==NULL)
        return;

    if (height > 1080)
        height = 1080;

#ifdef CFG_FFMPEG_H264_SW
    data = pFrame->data[0];
    for(y=0; y<height; y++)
        fwrite(data+y*pFrame->linesize[0], 1, width, pFile);

    data = pFrame->data[1];
    for(y=0; y<height/2; y++)
        fwrite(data+y*pFrame->linesize[1], 1, width/2, pFile);

    data = pFrame->data[2];
    for(y=0; y<height/2; y++)
        fwrite(data+y*pFrame->linesize[2], 1, width/2, pFile);
#else
    data = ithMapVram(pFrame->data[0], pFrame->linesize[0] * height, ITH_VRAM_READ);
    // Write pixel data
    for(y=0; y<height; y++)
        fwrite(data+y*pFrame->linesize[0], 1, width, pFile);

    data = ithMapVram(pFrame->data[1], pFrame->linesize[1] * height / 2, ITH_VRAM_READ);
    for(y=0; y<height/2; y++)
        fwrite(data+y*pFrame->linesize[1], 1, width / 2, pFile);

    data = ithMapVram(pFrame->data[2], pFrame->linesize[1] * height / 2, ITH_VRAM_READ);
    for(y=0; y<height/2; y++)
        fwrite(data+y*pFrame->linesize[1], 1, width / 2, pFile);
#endif

    // Close file
    fclose(pFile);
}
#endif

void render_frame(AVCodecContext *avctx,
    AVFrame *picture,
    uint64_t pts)
{
    bool bprint = true;
    ITV_DBUF_PROPERTY prop;
    ITV_OPAQUE_DATA *opaque = avctx->opaque;
    VIDEO_DECODE_MODE mode = opaque->mode;
    short sleepTime = 0;
    static uint64_t prevTime = 0; 
    uint64_t gap = pts - prevTime;

    prevTime = pts;
    //return;

#if 0
    if ((sleepTime = mtal_get_delay_time()) >= 0)
    {
        mtal_set_delay_time(0);
        if (sleepTime && sleepTime < 255)
        {
            usleep(sleepTime*1000);
        }
    }
    else //need drop frame
    {
        int oneFrameTime = (int) (gap / 90);
        if (oneFrameTime < 0)
        {
            oneFrameTime = 33;
        }
        sleepTime += oneFrameTime;
        if (sleepTime >= -10)
        {
            sleepTime = 0;
        }
        mtal_set_delay_time(sleepTime);
        return;
    }
#endif    
    memset((void*)&prop, 0, sizeof(ITV_DBUF_PROPERTY));
    prop.target_pts = pts; // -1 for free run

    while(1)
    {
        uint8_t *dbuf = NULL;

        //dbuf = itv_get_dbuf_anchor();
		//if(!dbuf) printf("YC: get dbuf full\n");
        //if(dbuf != NULL)
        if(1)
        {
        	static int test_count = 0;
			test_count++;
            uint32_t rdIndex = picture->opaque ? *(uint32_t *)picture->opaque : 0;
            int bidx; 
#if 1
			//printf("YC: picture->width = %d, picture->height = %d, avctx->width = %d, avctx->height = %d\n", picture->width, picture->height, avctx->width, avctx->height);

			if(picture->width != avctx->width && avctx->width != 0)
                picture->width = avctx->width;
            if(picture->height != avctx->height && avctx->height != 0)
                picture->height = avctx->height;
#endif
	
#if 0
			if(test_count == 100)
			{
				printf("YC: write file start\n");
				FILE *fp = fopen("A:/saved.nv12", "wb");

		        fwrite(picture->data[0], 1, picture->linesize[0] * picture->height, fp);
				fwrite(picture->data[1], 1, picture->linesize[1] * picture->height/2, fp);
				fwrite(picture->data[2], 1, picture->linesize[1] * picture->height/2, fp);

				fclose(fp);

				printf("YC: write file done\n");
			}
#endif			
            bidx = rdIndex;
            prop.src_w    = picture->width;
            prop.src_h    = picture->height;
            prop.ya       = picture->data[0];
            prop.ua       = picture->data[1];
            prop.va       = picture->data[2];
            prop.pitch_y  = picture->linesize[0]; //width; //2048
            prop.pitch_uv = picture->linesize[1]; //width/2; //2048
            prop.bidx     = bidx;
			prop.format   = MMP_ISP_IN_NV12;
			
#ifdef WIN32
            SaveFrame(picture, picture->width, picture->height);
#endif

            if(itv_update_dbuf_anchor (&prop) == -1) printf("YC: update dbuf full\n");

            av_log(NULL, AV_LOG_DEBUG, "disp(%d)pts %lld\n", bidx, prop.target_pts);
            break;
        }
        else
        {
            if (bprint)
            {
                av_log(NULL, AV_LOG_DEBUG, "get no display buf");
                bprint = false;
            }
            av_log(NULL, AV_LOG_DEBUG, ".");
            usleep(1000); // XXX
        }
    }
    av_log(NULL, AV_LOG_DEBUG, "\n");
}
#if 0
static void flush_audio_hw(PlayerInstance *is)
{
    AVFormatContext *ic = is->ic;
    int audio_stream = is->audio_stream;
    int nAudioEngine = 0;

    iteAudioStop();
    switch(ic->streams[audio_stream]->codec->codec_id)
    {
        case CODEC_ID_MP2:
        case CODEC_ID_MP3:
            nAudioEngine = ITE_MP3_DECODE;
            break;
        case CODEC_ID_PCM_S16LE:
        case CODEC_ID_PCM_S16BE:
        case CODEC_ID_PCM_U16LE:
        case CODEC_ID_PCM_U16BE:
        case CODEC_ID_PCM_S8:
        case CODEC_ID_PCM_U8:
        case CODEC_ID_PCM_ALAW:
        case CODEC_ID_PCM_MULAW:
        case CODEC_ID_ADPCM_IMA_WAV:
        case CODEC_ID_ADPCM_SWF:
            nAudioEngine = ITE_WAV_DECODE;
            break;
        case CODEC_ID_AAC:
            nAudioEngine = ITE_AAC_DECODE;
            break;
        case CODEC_ID_AC3:
        case CODEC_ID_EAC3:
            nAudioEngine = ITE_AC3_DECODE;
            break;
        case CODEC_ID_WMAV2:
            nAudioEngine = ITE_WMA_DECODE;
            break;
        case CODEC_ID_AMR_NB:
            nAudioEngine = ITE_AMR_CODEC;
        default:
            break;
    }
    iteAudioOpenEngine(nAudioEngine);
}
#endif
static void flush_audio_hw(PlayerInstance *is)
{
    AVFormatContext *ic          = is->ic;
    int             audio_stream = is->audio_stream;
    int             nAudioEngine = 0;

    switch (ic->streams[audio_stream]->codec->codec_id)
    {
    case CODEC_ID_MP2:
    case CODEC_ID_MP3:
        nAudioEngine = ITE_MP3_DECODE;
        break;

    case CODEC_ID_PCM_S16LE:
    case CODEC_ID_PCM_S16BE:
    case CODEC_ID_PCM_U16LE:
    case CODEC_ID_PCM_U16BE:
    case CODEC_ID_PCM_S8:
    case CODEC_ID_PCM_U8:
    case CODEC_ID_PCM_ALAW:
    case CODEC_ID_PCM_MULAW:
    case CODEC_ID_ADPCM_IMA_WAV:
    case CODEC_ID_ADPCM_SWF:
        nAudioEngine = ITE_WAV_DECODE;
        break;

    case CODEC_ID_AAC:
        nAudioEngine = ITE_AAC_DECODE;
        break;

    case CODEC_ID_AC3:
    case CODEC_ID_EAC3:
        nAudioEngine = ITE_AC3_DECODE;
        break;

    case CODEC_ID_WMAV2:
        nAudioEngine = ITE_WMA_DECODE;
        break;

    case CODEC_ID_AMR_NB:
        nAudioEngine = ITE_AMR_CODEC;

    default:
        break;
    }

	iteAudioStop();
}


/* get the current audio clock value */
static double get_audio_clock(PlayerInstance *is)
{
#if 0
    if (is->paused)
    {
        return is->audio_current_pts;
    }
    else
    {
        return is->audio_current_pts_drift + av_gettime() / 1000000.0;
    }
#else
    return 0;
#endif
}

/* get the current video clock value */
static double get_video_clock(PlayerInstance *is)
{
    if (is->paused)
    {
        return is->video_current_pts;
    }
    else
    {
        return is->video_current_pts_drift + av_gettime() / 1000000.0;
    }
}

/* get the current external clock value */
static double get_external_clock(PlayerInstance *is)
{
#if 0
    int64_t ti;
    ti = av_gettime();
    return is->external_clock + ((ti - is->external_clock_time) * 1e-6);
#else
    return 0;
#endif
}

/* get the current master clock value */
static double get_master_clock(PlayerInstance *is)
{
    double val;

    if (global_player_prop->av_sync_type == AV_SYNC_VIDEO_MASTER)
    {
        if (is->video_st)
            val = get_video_clock(is);
        else
            val = get_audio_clock(is);
    }
    else if (global_player_prop->av_sync_type == AV_SYNC_AUDIO_MASTER)
    {
        if (is->audio_st)
            val = get_audio_clock(is);
        else
            val = get_video_clock(is);
    }
    else
    {
        val = get_external_clock(is);
    }
    return val;
}

/* seek in the stream */
static void stream_seek(PlayerInstance *is, int64_t pos, int64_t rel, int seek_by_bytes)
{
    if (!is->seek_req)
    {
        is->seek_pos = pos;
        is->seek_rel = rel;
        is->seek_flags &= ~AVSEEK_FLAG_BYTE;
        if (seek_by_bytes)
        {
            is->seek_flags |= AVSEEK_FLAG_BYTE;
            is->seek_flags |= AVSEEK_FLAG_ANY;
        }
        is->seek_req = 1;
    }
}

static void set_resync_state(PlayerInstance *is, int state)
{
    if (is->resync_mux)
    {
        pthread_mutex_lock(&is->resync_mux);
    }
    is->need_resync_state = state;
    if (is->resync_mux)
    {
        pthread_mutex_unlock(&is->resync_mux);
    }
}

static int get_reysnc_state(PlayerInstance *is)
{
    return is->need_resync_state;
}


#if MULTITHREAD_IDO
static int get_video_frame(PlayerInstance *is, AVFrame *frame, int64_t *pts, AVPacket *pkt)
{
    int got_picture, i, result = 0;

    result = packet_queue_get(&is->videoq, pkt, 0);
    //if (packet_queue_get(&is->videoq, pkt, 0) < 0)
    if (result < 0)
    {
        return -1;
    }
    else if (result == 0)
    {
        return 0;
    }

    if (pkt->data == flush_pkt.data)
    {
        avcodec_flush_buffers(is->video_st->codec);
        return 0;
    }

	is->video_pts = (double)pkt->pts * av_q2d(is->video_st->time_base);
    avcodec_decode_video2(is->video_st->codec, frame, &got_picture, pkt);

    if (got_picture)
    {
        *pts = *(int64_t*)av_opt_ptr(avcodec_get_frame_class(), frame, "best_effort_timestamp");
        //is->frame_last_returned_time = av_gettime() / 1000000.0;
        return 1;
    }
    else
    {
        av_free_packet(pkt);
    }
    return 0;
}

static int video_thread(void *arg)
{
    PlayerProps *pProp = (PlayerProps*) arg;
    AVFrame *frame = avcodec_alloc_frame();
    int64_t pts_int = AV_NOPTS_VALUE, pos = -1;
    double pts;
    int ret;
    int pktSize = 0;
    int64_t curPts = 0;
    frame->opaque = (uint32_t*) malloc(sizeof(uint32_t));
    int retryCount = 0;
    PlayerInstance *is = pProp->inst[0];
    int bufTime = 0;
    int accumulateVideoQueueCount = 0;
    int accumulateVideoCount = 0;
    int dcxoCheckCount = 0;
    uint16_t val = 0;
    int videoGap = 0;
    int videoAbnormalCount = 0;
    struct timeval startT, endT;
    struct timeval startT1, endT1;
    bool bCloseUI = false;
    int videoJudgeCount = 0;
#ifdef VIDEO_ADJUST_CLOCK
    int videoJudgeCounter = 0;
    int videoAdjustCounter = 0;
    int videoAdjustUpperBound = 0;
    int videoAdjustLowerBound = 0;
#endif
    
    //gettimeofday(&startT, NULL);
    //gettimeofday(&startT1, NULL);
    for (;;)
    {
        AVPacket pkt;
        while (is->paused && !is->videoq.abort_request)
            usleep(10000);

#if 1
		while(!itv_get_dbuf_anchor()) 
		{
			//printf("YC: get dbuf full\n");
			usleep(2000);
		}
#endif
#if 0
        if (get_reysnc_state(is) == NEED_RESYNC_WAIT_VIDEO_FLUSH)
        {
            avcodec_flush_buffers(is->video_st->codec);
            set_resync_state(is, NEED_RESYNC_VIDEO_FLUSH_DONE);
            usleep(10 * 1000);
            continue;
        }
#endif
		gettimeofday(&startT, NULL);
        ret = get_video_frame(is, frame, &pts_int, &pkt);
		gettimeofday(&endT, NULL);
        if (ret < 0)
        {
            goto the_end;
        }

        if (ret > 0)
        {
        	static int count = 0;
			count++;
        	//printf(".");
        	//printf("video pts = %lld\n", pkt.pts);
#if 0			
            retryCount = 0;
            pktSize = pkt.size;
            bufTime = (int) (is->videoq.lastPts - curPts);
#endif			
			curPts = pkt.pts;
            av_free_packet(&pkt);
            if (!is->is_video_sync)
            {
#if 0            
                if (!frame->key_frame)
                {
                    usleep(1000);
                    continue;
                }
#endif				
#if 1
                while (is->videoq.nb_packets < FIRST_VIDEO_WAIT_COUNT)
                {
                    if (is->videoq.abort_request)
                    {
                        goto the_end;
                    }
                    usleep(1000);
                }
				printf("wait video count: %d\n", FIRST_VIDEO_WAIT_COUNT);
#else
                usleep(VIDEO_WAIT_TIME * 1000);
#endif
                is->is_video_sync = 1;
                set_resync_state(is, NEED_RESYNC_NULL_STATE);
#if 0
                accumulateVideoQueueCount = 0;
                accumulateVideoCount = 0;
#ifdef VIDEO_ADJUST_CLOCK
                videoJudgeCount = 0;
                videoJudgeCounter = 0;
                videoAdjustCounter = 0;
                videoAdjustUpperBound = 0;
                videoAdjustLowerBound = 0;
#endif
                printf("init video timestamp: %u, q_c: %u\n", (int) pkt.pts, is->videoq.nb_packets);
#endif

            }
			//printf("YC: %s, %d\n", __FUNCTION__, __LINE__);
			videoGap = curPts - is->latest_video_decode_pts;
			#if 0
            if (videoGap > 1600)
            {
                printf("video Gap is %d, old: %u, new: %u\n", videoGap, (int32_t) is->latest_video_decode_pts,
                       (int32_t) curPts);
            }
			#endif
			//printf("YC1: is->videoq.nb_packets = %d, dec time = %d\n", is->videoq.nb_packets, itpTimevalDiff(&startT, &endT));
			is->latest_video_decode_pts = curPts;

#if 0			
			if(count == 100)
			{
				printf("YC: write file start, Y_pitch = %d, UV_pitch = %d\n", frame->linesize[0], frame->linesize[1]);
				FILE *fp = fopen("A:/saved.nv12", "wb");

		        fwrite(frame->data[0], 1, frame->linesize[0] * frame->height, fp);
				fwrite(frame->data[1], 1, frame->linesize[1] * frame->height/2, fp);
				fwrite(frame->data[2], 1, frame->linesize[1] * frame->height/2, fp);

				fclose(fp);

				printf("YC: write file done\n");
			}
#endif
			if(is->videoq.nb_packets > RESYNC_VIDEO_LOWER_THRESHOLD)
			{
				printf("Skip frame\n");
				continue;
			}
            render_frame(is->avctx, frame, pts_int/*(uint64_t)(pts*90000)*/);		
			//usleep(1000);
        }
		else
		{
			//printf("YC: xxx, is->videoq.nb_packets = %d\n", is->videoq.nb_packets);
			//printf("YC: %s, %d\n", __FUNCTION__, __LINE__);
        	usleep(1000);
		}
    }
the_end:
    printf("terminate video thread\n");
    //if (is->videoq.nb_packets)
    {
        printf("terminate video queue\n");
        packet_queue_end(&is->videoq);
        if (is->video_st->codec)
        {
            avcodec_flush_buffers(is->video_st->codec);
        }
    }
    free(frame->opaque);
    av_free(frame);
    //pthread_exit(NULL);
    return 0;
}

static int audio_thread(void *arg)
{
    PlayerProps *pProp = (PlayerProps*) arg;
    AVFrame *frame = avcodec_alloc_frame();
    int64_t pts_int = AV_NOPTS_VALUE, pos = -1;
    int ret;
    int got_frame = 1;
    PlayerInstance *is = pProp->inst[0];

	//audio sync
	int audio_samplerate;
	int dec_buf_size , avail_buf_size, dec_data_size = 0;
	int dac_buf_size, dac_data_size = 0;
	int dac_rptr, dac_wptr;
	int audio_data_size;
	double audio_buf_time;
	static int i2s_frection_value = 0;
	
    printf("castor3player.c(%d), audio_thread is started\n", __LINE__);
    frame->opaque = (uint32_t*) malloc(sizeof(uint32_t));

    for (;;)
    {
        AVPacket pkt;

        while (is->paused && !is->audioq.abort_request)
        {
            usleep(10000);
        }
        ret = packet_queue_get(&is->audioq, &pkt, 0);

        if (ret < 0)
        {
            goto the_end;
        }
        else if (ret > 0)
        {
        	double diff, audio_pts;
			
        	if (!is->is_video_sync)
        	{
        		av_free_packet(&pkt);
        	}
			else
			{
#if 1			
                audio_pts = (double)pkt.pts * av_q2d(is->audio_st->time_base);
				if(is->video_pts)
				{	                
					if(!is->init_av_gap)
						is->init_av_gap = audio_pts - is->video_pts;
					
					itp_codec_get_i2s_sample_rate(&audio_samplerate);
					iteAudioGetAttrib(ITE_AUDIO_DIRVER_DECODE_BUFFER_LENGTH, &dec_buf_size);
					iteAudioGetAvailableBufferLength(ITE_AUDIO_OUTPUT_BUFFER, &avail_buf_size);
					dec_data_size = dec_buf_size - avail_buf_size - 2;
					//printf("dec_buf_size = %d, avail_buf_size = %d\n", dec_buf_size, avail_buf_size);

					dac_buf_size = ithReadRegA(0xD0100000 | 0x90);
					dac_rptr = I2S_DA32_GET_RP();
					dac_wptr = I2S_DA32_GET_WP();
					//printf("dac_buf_size = %d, rptr = %d, wptr = %d\n", dac_buf_size, dac_rptr, dac_wptr);
					if(dac_wptr >= dac_rptr)
					{
						dac_data_size = dac_wptr - dac_rptr;
					}
					else
					{
						dac_data_size = dac_buf_size - (dac_rptr - dac_wptr);
					}

					audio_data_size = dac_data_size + dec_data_size;

					audio_buf_time = (double)audio_data_size / (2 * audio_samplerate);

					diff = (audio_pts - audio_buf_time) - is->video_pts;
					//printf("diff = %f, audio_pts = %f, audio_buf_time = %f, video_pts = %f, init_gap = %f\n", diff, audio_pts, audio_buf_time, is->video_pts, is->init_av_gap);
					
					if(audio_buf_time > 0.1)
					{
						if(i2s_frection_value != 50)
	                    {
	                    	i2s_frection_value = 50;
	                    	i2s_frection_set(i2s_frection_value);
	                    }
					}
					else if(audio_buf_time < 0.05)
					{
						if(i2s_frection_value != 0)
	                    {
	                    	i2s_frection_value = 0;
	                    	i2s_frection_set(i2s_frection_value);
	                    }
					}
				}
				else
				{
					i2s_frection_value = 0;
					i2s_frection_set(i2s_frection_value);
				}
#endif				
				avcodec_decode_audio4(is->audio_st->codec, frame, &got_frame, &pkt);
				//printf("audio pts = %lld\n", pkt.pts);
				av_free_packet(&pkt);
			}
        }
        else
        {
			itp_codec_get_i2s_sample_rate(&audio_samplerate);
			iteAudioGetAttrib(ITE_AUDIO_DIRVER_DECODE_BUFFER_LENGTH, &dec_buf_size);
			iteAudioGetAvailableBufferLength(ITE_AUDIO_OUTPUT_BUFFER, &avail_buf_size);
			dec_data_size = dec_buf_size - avail_buf_size - 2;
			//printf("dec_buf_size = %d, avail_buf_size = %d\n", dec_buf_size, avail_buf_size);

			dac_buf_size = ithReadRegA(0xD0100000 | 0x90);
			dac_rptr = I2S_DA32_GET_RP();
			dac_wptr = I2S_DA32_GET_WP();
			//printf("dac_buf_size = %d, rptr = %d, wptr = %d\n", dac_buf_size, dac_rptr, dac_wptr);
			if(dac_wptr >= dac_rptr)
			{
				dac_data_size = dac_wptr - dac_rptr;
			}
			else
			{
				dac_data_size = dac_buf_size - (dac_rptr - dac_wptr);
			}

			audio_data_size = dac_data_size + dec_data_size;

			audio_buf_time = (double)audio_data_size / (2 * audio_samplerate);
			
			if(audio_buf_time < 0.05)
			{
				if(i2s_frection_value != 0)
                {
                	i2s_frection_value = 0;
                	i2s_frection_set(i2s_frection_value);
                }
			}
            usleep(3000);
        }
    }
the_end:
    printf("terminate audio thread\n");
    //if (is->audioq.nb_packets)
    {
        printf("terminate audio queue\n");
        packet_queue_end(&is->audioq);
    }
    free(frame->opaque);
    av_free(frame);
    //pthread_exit(NULL);
    return 0;
}
#endif // MULTITHREAD_IDO

static int stream_component_open(PlayerInstance *is, int stream_index, void *opaque)
{
    AVFormatContext *ic = is->ic;
    AVCodecContext *avctx;
    AVCodec *codec;

    if (stream_index < 0 || stream_index >= ic->nb_streams)
        return -1;
    avctx = ic->streams[stream_index]->codec;
    avctx->opaque = opaque;
    if (avctx->codec_type == AVMEDIA_TYPE_VIDEO)
    {
        avctx->width = is->src_w; // if any
        avctx->height = is->src_h; // if any
    }
    codec = avcodec_find_decoder(avctx->codec_id);

    if (!codec)
        return -1;

    if (!codec || avcodec_open2(avctx, codec, NULL) < 0)
        return -1;

    ic->streams[stream_index]->discard = AVDISCARD_DEFAULT;
    switch (avctx->codec_type)
    {
        case AVMEDIA_TYPE_AUDIO:
            is->audio_stream = stream_index;
            is->audio_st = ic->streams[stream_index];
#if MULTITHREAD_IDO
            printf("open audio queue\n");
            packet_queue_init(&is->audioq);
#endif
            break;
        case AVMEDIA_TYPE_VIDEO:
            is->video_stream = stream_index;
            is->video_st = ic->streams[stream_index];
            is->avctx = avctx;
#if MULTITHREAD_IDO
            printf("open video queue\n");
            packet_queue_init(&is->videoq);
#endif
            break;
        case AVMEDIA_TYPE_SUBTITLE:
            is->subtitle_stream = stream_index;
            is->subtitle_st = ic->streams[stream_index];
            break;
        default:
            break;
    }
    return 0;
}

static void stream_component_close(PlayerInstance *is, int stream_index)
{
    AVFormatContext *ic = is->ic;
    AVCodecContext *avctx;

    if (stream_index < 0 || stream_index >= ic->nb_streams)
        return;
    avctx = ic->streams[stream_index]->codec;
    if (avctx->opaque)
    {
        av_free(avctx->opaque);
        avctx->opaque = NULL;
    }

#if MULTITHREAD_IDO
    switch(avctx->codec_type)
    {
        case AVMEDIA_TYPE_VIDEO:
            packet_queue_end(&is->videoq);
            break;
        case AVMEDIA_TYPE_AUDIO:
            packet_queue_end(&is->audioq);
            break;
        case AVMEDIA_TYPE_SUBTITLE:
        default:
            break;
    }
#endif

    ic->streams[stream_index]->discard = AVDISCARD_ALL;
    avcodec_close(avctx);
    switch(avctx->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            is->audio_st = NULL;
            is->audio_stream = -1;
            break;
        case AVMEDIA_TYPE_VIDEO:
            is->video_st = NULL;
            is->video_stream = -1;
            break;
        case AVMEDIA_TYPE_SUBTITLE:
            is->subtitle_st = NULL;
            is->subtitle_stream = -1;
            break;
        default:
            break;
    }
}

void* read_thread(void *arg)
{
    PlayerProps *pprop = (PlayerProps*) arg;
    AVFormatContext *ic = NULL;
    int err, ret, i, j, instance = 0;
    int st_index[AVMEDIA_TYPE_NB];
    AVPacket pkt1, *pkt = &pkt1;
    int eof = 0;
    int open_input_success = 1;
    struct timeval startT, endT;
    bool bfind_video_stream = false;

    if (!pprop)
    {

        av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        ret = -1;
        goto fail;
    }

retry:
    for (i = 0; i < pprop->instCnt; i++)
    {
        PlayerInstance *is = pprop->inst[0];
        ITV_OPAQUE_DATA *opaque = (ITV_OPAQUE_DATA*) malloc(sizeof(ITV_OPAQUE_DATA));

        if (!opaque)
        {
            av_log(NULL, AV_LOG_ERROR, "malloc opaque failed\n");
            continue;
        }
        pthread_mutex_init(&is->resync_mux, NULL);
        // player instances initial
        is->video_stream = -1;
        is->audio_stream = -1;
        is->subtitle_stream = -1;
        is->exclock = 0;
        is->frame_count = 0;
		is->video_pts = 0;
		is->init_av_gap = 0;
        opaque->mode = pprop->mode;
        opaque->frame_buf_cnt = pprop->frame_buf_cnt;

        memset(st_index, -1, sizeof(st_index));

        /* init AVFormatContext */
        ic = avformat_alloc_context();
        ic->interrupt_callback.callback = decode_interrupt_cb; // callback when stop avformat
        pprop->bStreamOpen = 0;
        mtal_get_source_path(is->filename);
        err = avformat_open_input(&ic, is->filename, NULL, NULL);

        if (err < 0)
        {
            char rets[256] = {0};
            av_strerror(err, rets, sizeof(rets));
            av_log(NULL, AV_LOG_ERROR, "Open input error \"%s\" : %s\n", is->filename, rets);

			//if(open_input_success)
				//mtal_put_message(MSG_WAIT_CONNECTION, NULL);

            open_input_success = 0;
            if (pprop->inst[0]->abort_request)
            {
                //mtal_set_pb_mode(0x10);
                free(opaque);
                goto fail;
            }
            else
            {
                usleep(1000000);
                free(opaque);
                pthread_mutex_destroy(&is->resync_mux);
                goto retry;
            }
        }
        pprop->bStreamOpen = 1;
        open_input_success = 1;
        is->ic = ic;

        usleep(40000); //XXX

        if(ic->nb_streams == 0)
        {

            bfind_video_stream = false;
            struct timeval waitTimeStartT, waitTimeCurT;
            gettimeofday(&waitTimeStartT, NULL);

            while(ic->nb_streams < 2 || !bfind_video_stream)
            {
                //read one frame to parse av codec
                int readResult = 0;
                if ((readResult = av_read_frame(ic, pkt)) >= 0)
                {
                    av_free_packet(pkt);
                }
                else if (readResult < 0)
                {
                    char rets[256] = {0};
                    av_strerror(readResult, rets, sizeof(rets));
                    av_log(NULL, AV_LOG_ERROR, "Open input error \"%s\" : %s\n", is->filename, rets);
                    usleep(1000000);
                    avformat_close_input(&is->ic);
                    pthread_mutex_destroy(&is->resync_mux);
                    free(opaque);
                    goto retry;
                }

                //av_free_packet(pkt);
                if (ic->nb_streams)
                {
                    for (i = 0; i < ic->nb_streams; i++)
                    {
                        AVStream *st = ic->streams[i];
                        AVCodecContext *avctx = st->codec;
                        if (avctx->codec_type == AVMEDIA_TYPE_VIDEO)
                        {
                            bfind_video_stream = true;
                            break;
                        }
                    }
                    gettimeofday(&waitTimeCurT, NULL);
                    if (itpTimevalDiff(&waitTimeStartT, &waitTimeCurT) > 5000)
                    {
                        printf("castor3player.c(%d), Timeout: read result: %d\n", __LINE__, readResult);
                        usleep(1000000);
                        avformat_close_input(&is->ic);
                        pthread_mutex_destroy(&is->resync_mux);
                        free(opaque);
                        goto retry;
                    }
                }
                else
                {
                    usleep(1000000);
                    avformat_close_input(&is->ic);
                    pthread_mutex_destroy(&is->resync_mux);
                    free(opaque);
                    goto retry;
                }
            }    
        }
        printf("ic->nb_streams = %d\n", ic->nb_streams);

        if (pprop->genpts)
            ic->flags |= AVFMT_FLAG_GENPTS;

        if (ic->pb)
            ic->pb->eof_reached = 0;

        if (pprop->seek_by_bytes < 0)
            pprop->seek_by_bytes = !!(ic->iformat->flags & AVFMT_TS_DISCONT);

        /* if seeking requested, execute it */
        if (pprop->start_time != AV_NOPTS_VALUE)
        {
            int64_t timestamp;

            av_log(NULL, AV_LOG_ERROR, "seek request before start\n");
            timestamp = pprop->start_time;
            /* add the stream start time */
            if (ic->start_time != AV_NOPTS_VALUE)
                timestamp += ic->start_time;
            ret = avformat_seek_file(ic, -1, INT64_MIN, timestamp, INT64_MAX, 0);
            if (ret < 0)
                av_log(NULL, AV_LOG_ERROR, "%s: could not seek to position %0.3f\n", is->filename, (double)timestamp / AV_TIME_BASE);
        }

        /* set for avformat_seek_file */
        for (j = 0; j < ic->nb_streams; j++)
            ic->streams[j]->discard = AVDISCARD_ALL;

        /* find stream id */
        if (!is->video_disable)
            st_index[AVMEDIA_TYPE_VIDEO] = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

        if (!is->audio_disable)
            st_index[AVMEDIA_TYPE_AUDIO] = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, st_index[AVMEDIA_TYPE_VIDEO], NULL, 0);

        if (!is->video_disable)
            st_index[AVMEDIA_TYPE_SUBTITLE] = av_find_best_stream(ic, AVMEDIA_TYPE_SUBTITLE, -1, st_index[AVMEDIA_TYPE_VIDEO], NULL, 0);

        printf("YC: st_index[AVMEDIA_TYPE_VIDEO] = %d, st_index[AVMEDIA_TYPE_AUDIO] = %d\n", st_index[AVMEDIA_TYPE_VIDEO], st_index[AVMEDIA_TYPE_AUDIO]);
        // if (pprop->show_status)
        //     av_dump_format(ic, 0, is->filename, 0);

        /* open the streams */
        if (st_index[AVMEDIA_TYPE_AUDIO] >= 0)
            stream_component_open(is, st_index[AVMEDIA_TYPE_AUDIO], NULL);

        ret = -1;
        if (st_index[AVMEDIA_TYPE_VIDEO] >= 0)
            ret = stream_component_open(is, st_index[AVMEDIA_TYPE_VIDEO], opaque);

        usleep(40000); //XXX

        if (st_index[AVMEDIA_TYPE_SUBTITLE] >= 0)
            stream_component_open(is, st_index[AVMEDIA_TYPE_SUBTITLE], opaque);

        if (is->video_stream < 0 && is->audio_stream < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "%s: could not open codecs\n", is->filename);
            ret = -1;

            mtal_set_pb_mode(0x10);
            goto fail;
        }

        is->exclock = PalGetClock();
    }

#if MULTITHREAD_IDO
    /* Create Video thread */
	pthread_attr_t attr;
	struct sched_param param;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 102400);
	param.sched_priority = sched_get_priority_max(SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
    printf("init video thread\n");
    pthread_create(&pprop->video_tid, &attr, video_thread, (void*)pprop);
    //pthread_create(&pprop->video_tid, NULL, video_thread, (void*)pprop);
    /* Create Audio thread */
#if 1	
    printf("init audio thread\n");
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 102400);
	param.sched_priority = sched_get_priority_max(SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
    pthread_create(&pprop->audio_tid, &attr, audio_thread, (void*)pprop);
#endif	
    //pthread_create(&pprop->audio_tid, NULL, audio_thread, (void*)pprop);
#endif

    // TODO: live source stream
#if 0    
    if (mtal_pb_is_livesrc())
    {
        if (mtal_get_pb_mode() != 0x11)
        {
            mtal_put_message(MSG_MCTL_CMD, MCTL_CMD_FLUSH);
        }
        // guarantee message has been handled
        while (mtal_get_pb_mode() != 0x11)
        {
             usleep(20000);
        }
    }
#endif
    for (;;)
    {
        PlayerInstance *is = pprop->inst[0];
        AVFormatContext *ic = is->ic;

        if (is->abort_request)
        {
            printf("Player get abort_request\n");
            break;
        }

#if MULTITHREAD_IDO
        /* if the queue are full, no need to read more */
        if (is->videoq.size > MAX_QUEUE_SIZE
        || (is->videoq.nb_packets > MIN_FRAMES
        || is->video_stream < 0))
        {
            /* wait 1 ms */
            usleep(500*1000);
            printf("castor3player.c(%d), %d, %d, %d\n", __LINE__, 
                   is->videoq.size, is->videoq.nb_packets,
                   is->video_stream);
            continue;
        }

#if 0
		if(is->videoq.nb_packets >= FRAME_QUEUE_THRESHOLD)
		{
			usleep(1000);
			continue;
		}
#endif		
#endif

        if (get_reysnc_state(is) == NEED_RESYNC_START_STATE)
        {
            if (is->video_stream >= 0)
            {
                //int timeoutCount = 0;
                //itv_set_update_dbuf_rindex_flag();
                packet_queue_flush(&is->videoq);
				
#if 0				
                set_resync_state(is, NEED_RESYNC_WAIT_VIDEO_FLUSH);
                usleep(20 * 1000);
                while (timeoutCount++ < 5)
                {
                    if (get_reysnc_state(is) == NEED_RESYNC_VIDEO_FLUSH_DONE)
                    {
                        printf("video thread flush codec is done\n");
                        break;
                    }
                    usleep(100 * 1000);
                }
#endif				
            }

            if (is->audio_stream >= 0)
            {
                packet_queue_flush(&is->audioq);
            }
            printf("%s(%d) flush out buffer\n", __FILE__, __LINE__);
            set_resync_state(is, NEED_RESYNC_DONE_STATE);
            is->is_video_sync = 0;
            is->is_audio_sync = 0;
            is->latest_audio_decode_pts = 0;
            is->latest_video_decode_pts = 0;
        }

#if 0
        if (eof)
        {
            unsigned long dur = PalGetDuration(is->exclock);
            if (get_reysnc_state(is) == NEED_RESYNC_NULL_STATE)
            {
                printf("%s(%d) EOF need resync\n", __FILE__, __LINE__);
                set_resync_state(is, NEED_RESYNC_START_STATE);
            }
            eof = 0;
            usleep(1000);
            continue;
        }
#endif		
     	gettimeofday(&startT, NULL);
        ret = av_read_frame(ic, pkt);
		gettimeofday(&endT, NULL);
        if (mtal_get_switch_flag())
        {
            printf("swtich mode change\n");
            if (ret >= 0)
            {
                av_free_packet(pkt);
            }
            goto fail;
        }

        if (ret < 0)
        {
            if (ret == AVERROR_EOF || url_feof(ic->pb))
            {
#if 1
                if (ret == AVERROR_EOF)
                {
                    printf("%s(%d), AVERROR_EOF\n", __FILE__, __LINE__);
                }
                if (url_feof(ic->pb))
                {
                    printf("%s(%d), url_feof(ic->pb)\n", __FILE__, __LINE__);

					//if(eof == 0)
						//mtal_put_message(MSG_NO_SIGNAL, NULL);
				
                	eof = 1;
                }
#endif
				continue;
            }
			
            if (ic->pb && ic->pb->error)
            {
                break;
            }

            //Unicast player is teardown by some reason, therefore, need to restart player.
#if 0            
            if (mtal_get_screen_mode_flag())
            {
                mtal_set_fake_ir(0x10);
            }
            else
            {
                mtal_set_fake_ir(0x32);
            }
#endif			
            goto fail;
        }
		else
		{
			//if(eof == 1)
				//mtal_put_message(MSG_CLEAN_OSD, NULL);
			eof = 0;
		}
        //printf("read_frame-\n");

        if (pkt->pts != AV_NOPTS_VALUE)
            av_log(NULL, AV_LOG_DEBUG, "Frame %d: pts=%lld, dts=%lld, size=%d, data=%x\n", pkt->stream_index, pkt->pts, pkt->dts, pkt->size, pkt->data);

        if (pkt->stream_index == is->video_stream)
        {
#if MULTITHREAD_IDO			
			static int need_resync_count = 0;
            if (is->videoq.nb_packets > RESYNC_VIDEO_UPPER_THRESHOLD)
            {
            	need_resync_count++;
				if(need_resync_count >= 10)
				{
                	printf("videoq is %d, need to resync again\n", is->videoq.nb_packets);
                	set_resync_state(is, NEED_RESYNC_START_STATE);
					need_resync_count = 0;
				}
            }
			else
			{
				need_resync_count = 0;
			}
			
            packet_queue_put(&is->videoq, pkt);
#if 0			
            //{
	            int waitTime = 0;   
	            gettimeofday(&endT, NULL);
	            if ((waitTime = itpTimevalDiff(&startT, &endT)) > 100)
	            {
	                printf("video process time: %d, size: %d\n", waitTime, pkt->size);
	            }
	            gettimeofday(&startT, NULL);
            //}
#endif				
			//printf("YC: video nb_packets = %d, parse time = %d\n", is->videoq.nb_packets, itpTimevalDiff(&startT, &endT));
			if(itpTimevalDiff(&startT, &endT) > 30)
				printf("video recv time waste %d ms, v_size = %d\n", itpTimevalDiff(&startT, &endT), pkt->size);
#else // MULTITHREAD_IDO
            int got_picture = 0, rev = -1;
            AVFrame *frame = NULL;

            unsigned char *p = (unsigned char*)&pkt->destruct;
            int i;

            frame = avcodec_alloc_frame();
            avcodec_get_frame_defaults(frame);
            frame->opaque = malloc(sizeof(uint32_t));

            rev = avcodec_decode_video2(is->avctx, frame, &got_picture, pkt);
            if (rev < 0)
                av_log(NULL, AV_LOG_ERROR, "video decode error\n");

            av_free_packet(pkt);

            if (got_picture)
            {
                render_frame(is->avctx, frame, /*(uint64_t)(pts * 90000)*/0);
            }

            free(frame->opaque);
            av_free(frame);

#endif // MULTITHREAD_IDO
        }
        else if (pkt->stream_index == is->audio_stream)
        {
            //av_init_packet(pkt);
            //pkt->data = NULL;
            //pkt->size = 0;
            //pkt->stream_index = is->audio_stream;
#if MULTITHREAD_IDO
            if (is->need_resync_state)
            {
                av_free_packet(pkt);
            }
            else
            {
                packet_queue_put(&is->audioq, pkt);
            }
#endif
        }
        else
        {
            av_free_packet(pkt);
            usleep(1000);
            continue;
        }
        usleep(1000);
    }

fail:
    itv_flush_dbuf();

    mtal_set_pb_mode(0x10);

    /* wait until the end */
    while(!pprop->inst[0]->abort_request)
    {
        usleep(100000);
    }
    ret = 0;

#if MULTITHREAD_IDO
    pprop->inst[0]->is_video_sync = 0;
    pprop->inst[0]->is_audio_sync = 0;
    pprop->inst[0]->latest_audio_decode_pts = 0;
    pprop->inst[0]->latest_video_decode_pts = 0;
    printf("terminate video\n");
    /* close video thread */
    packet_queue_abort(&pprop->inst[0]->videoq);
    if (pprop->video_tid)
    {
        printf("wait video thread end\n");
        pthread_join(pprop->video_tid, NULL);
        usleep(10*1000);
        pprop->video_tid = 0;
    }
    printf("terminate audio\n");
    /* close audio thread */
    packet_queue_abort(&pprop->inst[0]->audioq);
    if (pprop->audio_tid)
    {
        printf("wait audio thread end\n");
        pthread_join(pprop->audio_tid, NULL);
        usleep(10*1000);
        pprop->audio_tid = 0;
    }
#endif

    /* close each stream */
    for (i = 0; i < pprop->instCnt; i++)
    {
        PlayerInstance *is = pprop->inst[0];
        pthread_mutex_destroy(&is->resync_mux);

        if (!is) continue;

        if(st_index[AVMEDIA_TYPE_AUDIO] >= 0)
            stream_component_close(is, st_index[AVMEDIA_TYPE_AUDIO]);
        if (st_index[AVMEDIA_TYPE_VIDEO] >= 0)
            stream_component_close(is, st_index[AVMEDIA_TYPE_VIDEO]);
        if (st_index[AVMEDIA_TYPE_SUBTITLE] >= 0)
            stream_component_close(is, st_index[AVMEDIA_TYPE_SUBTITLE]);

        if(open_input_success && (is->ic))
        {
            avformat_close_input(&is->ic);
            is->ic = NULL; /* safety */
        }
    }
    avio_set_interrupt_cb(NULL);

    if (ret != 0)
    {
        av_log(NULL, AV_LOG_ERROR, "read_thread failed, ret=%d\n", ret);
        //mtal_set_fake_ir(0x10);
    }
    else
        av_log(NULL, AV_LOG_ERROR, "read thread %x done\n", pprop->read_tid);

    //pthread_exit(NULL);
    return 0;
}

static void stream_open(PlayerInstance *is)
{
    /* start video display */
    //pthread_mutex_init(&is->pictq_mutex, NULL);
    //pthread_cond_init(&is->pictq_cond, NULL);

    /* start audio display */
    //pthread_mutex_init(&is->audioq_mutex, NULL);
    //pthread_cond_init(&is->audioq_cond, NULL);
}

static void stream_close(PlayerProps *pprop)
{
    int i;

    //av_log(NULL, AV_LOG_DEBUG, "stream close\n");
    printf("castor3player.c(%d), stream close+\n", __LINE__);
    //malloc_stats();
    /* make sure one complete frame is done */
    //mtal_set_switch_flag(1);
    pprop->inst[0]->abort_request = 1;
    usleep(30000); //XXX
    if (pprop->read_tid)
    {
        pthread_join(pprop->read_tid, NULL);
    }

    usleep(10*1000);
end:
    /* free all pictures */
    for (i = 0; i < pprop->instCnt; i++)
    {
        PlayerInstance *is = pprop->inst[0];

        if (!is)
           continue;

        is->abort_request = 0;
        free(pprop->inst[0]);
        pprop->inst[0] = NULL;
    }
    pprop->instCnt = 0;
    mtal_set_dataInputStatus(false);

    printf("castor3player.c(%d), stream close-\n", __LINE__);
#ifdef MALLOC_DEBUG
    Rmalloc_stat();
#else
    malloc_stats();
#endif
    //av_free(is); // implement stop, don't free PlayerProps here
}

///////////////////////////////////////////////////////////////////////////////////////////
// Public Functions
//
///////////////////////////////////////////////////////////////////////////////////////////

int ithMediaPlayer_init(PLAYER_MODE mode)
{
    PlayerProps *pprop = NULL;

    if (global_player_prop) return 1;

    /* register all codecs, demux and protocols */
    av_register_all();
    avformat_network_init();

#ifdef DEBUG
    av_log_set_level(AV_LOG_DEBUG);
#endif

    pprop = (PlayerProps*) calloc(sizeof(char), sizeof(PlayerProps));
    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Initialize player failed\n");
        return -1;
    }

    // player properties initial, TODO use API later
    pprop->show_status = 1;
    pprop->seek_by_bytes = -1;
    pprop->start_time = AV_NOPTS_VALUE;
    pprop->decoder_reorder_pts = -1;
    pprop->av_sync_type = AV_SYNC_VIDEO_MASTER;
    pprop->mode = mode; // NORMAL_MODE(0) or RMI_MODE(1);
    pprop->frame_buf_cnt = (mode == NORMAL_PLAYER)? 8: TOTAL_FRAME_BUFFER_CNT;

    //hwInit(pprop);

    // set ffmpeg compoent to itv
    {
        FF_COMPONENT comp1, *comp = &comp1;
        comp->component_prop = (void*) pprop;
        comp->flush = ithMediaPlayer_flush;
        mtal_set_component(ITV_DEMUX_COMPONENT, comp);
    }

    global_player_prop = pprop;
}

static int ithMediaPlayer_select_file(const char* filename, unsigned src_w, unsigned src_h)
{
    PlayerProps *pprop = global_player_prop;

    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Null player properties %s\n", __func__);
        return -1;
    }
    if (pprop->instCnt >= 1)
    {
        av_log(NULL, AV_LOG_ERROR, "Maximum stream reach\n");
        //return -1;
        stream_close(pprop);
        printf("Maximum stream: stream close is done\n");
    }

    if (filename[0] != '\0')
    {
        PlayerInstance *inst = NULL;

        av_log(NULL, AV_LOG_INFO, "Select %s\n", filename);

        inst = (PlayerInstance*) malloc(sizeof(PlayerInstance));
        if (!inst)
            av_log(NULL, AV_LOG_ERROR, "not enough memory\n");
        memset(inst, 0, sizeof(PlayerInstance));

        av_strlcpy(inst->filename, filename, strlen(filename)+1);

        // disable audio
        inst->audio_disable = 0;

        // source width and height if any
        inst->src_w = src_w;
        inst->src_h = src_h;

        //FIXME:
        inst->loop = 0;
        inst->autoexit = 1;

        stream_open(inst);
        pprop->inst[pprop->instCnt++] = inst;
    }

    return 0;
}

static int ithMediaPlayer_play(void)
{
    PlayerProps *pprop = global_player_prop;
    int rc;
    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        return -1;
    }
    else if (!pprop->inst[0])
    {
        av_log(NULL, AV_LOG_ERROR, "No assigned stream in player\n");
        return -1;
    }

#if MULTITHREAD_IDO
    //av_init_packet(&flush_pkt);
    //flush_pkt.data = "FLUSH";
#endif
    {
    /* Create read thread */
	pthread_attr_t attr;
	struct sched_param param;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 102400);
	param.sched_priority = sched_get_priority_max(SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);

    rc = pthread_create(&pprop->read_tid, &attr, read_thread, (void*)pprop);
    }
    if (rc)
    {
        av_log(NULL, AV_LOG_ERROR, "create thread failed %d\n", rc);
        ithMediaPlayer_stop();
    }

    return 0;
}

/* pause or resume the video */
static int ithMediaPlayer_pause(void)
{
    PlayerProps *pprop = global_player_prop;
    PlayerInstance *is = NULL;

    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        return -1;
    }

    is = pprop->inst[0];
    if (!is)
    {
        av_log(NULL, AV_LOG_ERROR, "No assigned stream in player\n");
        return -1;
    }

    if (is->paused)
    {
        is->frame_timer += av_gettime() / 1000000.0 + is->video_current_pts_drift - is->video_current_pts;
        if(is->read_pause_return != AVERROR(ENOSYS))
        {
            is->video_current_pts = is->video_current_pts_drift + av_gettime() / 1000000.0;
        }
        is->video_current_pts_drift = is->video_current_pts - av_gettime() / 1000000.0;
    }
    return is->paused = !is->paused;
}

static int ithMediaPlayer_stop(void)
{
    PlayerProps *pprop = global_player_prop;
    VIDEO_DECODE_MODE mode;
    if (!pprop)
    {
        printf("Player not exist....................................\n");
        //av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
    return -1;
    }

    mode = pprop->mode;
    stream_close(pprop);

    /* default set RMI active index to 1 (single instance) */
    if (mode == RMI_MODE)
    {
        itv_flush_dbuf();
        //itv_rmi_set_active_ids(0x1);
    }

    return 0;
}

static void do_seek(PlayerInstance *is, unsigned short key)
{
    double incr, pos;
    PlayerInstance *cur_stream = is;
    unsigned char direction[6] = {0};

    switch(key)
    {
    case 0:
        incr = 60.0;
        sprintf(direction, "UP");
        break;
    case 1:
        incr = -60.0;
        sprintf(direction, "DOWN");
        break;
    case 2:
        incr = 10.0;
        sprintf(direction, "RIGHT");
        break;
    case 3:
        incr = -10.0;
        sprintf(direction, "LEFT");
        break;
    }

    if (global_player_prop->seek_by_bytes) //there is some problems when using seek by byte, not every demuxer support this.
    {
        int av = -1;
        if (cur_stream->video_stream >=0 && cur_stream->video_current_pos >= 0)
        {
            pos = cur_stream->video_current_pos;
            av = 1;
        }
        else if (cur_stream->audio_stream >= 0 && cur_stream->audio_pkt.pos >=0)
        {
            pos = cur_stream->audio_pkt.pos;
            av = 2;
        }
        else
        {
            pos = avio_tell(cur_stream->ic->pb);
            av = 3;
        }

        if (cur_stream->ic->bit_rate)
            incr *= cur_stream->ic->bit_rate / 8.0;
        else
            incr *= 180000.0;

        pos += incr;

        {
        int64_t seek_target = pos + incr;
        int64_t seek_min = incr > 0 ? seek_target - incr + 2 : INT64_MIN;
        int64_t seek_max = incr < 0 ? seek_target - incr - 2 : INT64_MAX;
        av_log(NULL, AV_LOG_DEBUG, "seek(%s,%d) bitrate=%d incr=%0.2f pos=%0.2f target=%lld seek_min=%lld seek_max=%lld backrewind=%d\n",
            direction, av, cur_stream->ic->bit_rate,
            incr, pos, seek_target, seek_min, seek_max,
            (seek_target-seek_min>(uint64_t)(seek_max-seek_target)?AVSEEK_FLAG_BACKWARD:0));
        }

        stream_seek(cur_stream, pos, incr, 1);
    }
    else
    {
        pos = get_master_clock(cur_stream);
        pos += incr;
        stream_seek(cur_stream, (int64_t)(pos * AV_TIME_BASE), (int64_t)(incr * AV_TIME_BASE), 0);
    }
}

static int ithMediaPlayer_deinit()
{
    PlayerProps *pprop = global_player_prop;

    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        return -1;
    }
    global_player_prop = NULL;

    /* De-init network */
    avformat_network_deinit();

    /* Release PlayerProps if any */
    if (pprop->show_status)
    {
        free(pprop);
        pprop = NULL;
    }

    /* Release video request memory buffers */
    ithVideoBufRelease();
}

static int ithMediaPlayer_volume_up(void)
{
//    i2s_volume_up();
    return 0;
}

static int ithMediaPlayer_volume_down(void)
{
//    i2s_volume_down();
    return 0;
}

static int ithMediaPlayer_mute(void)
{
    PlayerProps *pprop = global_player_prop;
    int mute = -1;

    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        return -1;
    }
    mute = !pprop->mute;
    pprop->mute = mute;

//    i2s_mute_DAC(mute);

    return mute;
}

int ithMediaPlayer_flush(void* arg)
{
    PlayerProps *prop = (PlayerProps*) arg;
    int ret = 0, i;

#if MULTITHREAD_IDO
    // TODO: determine instance
    for (i = 0; i < prop->instCnt; i++)
    {
        PlayerInstance *is = prop->inst[0];
        packet_queue_flush(&is->videoq);
    }

    for (i = 0; i < prop->instCnt; i++)
    {
        PlayerInstance *is = prop->inst[0];
        packet_queue_flush(&is->audioq);
    }
#endif
    return ret;
}

#if defined(_MSC_VER)
ithMediaPlayer player1 = {
    ithMediaPlayer_init,
    ithMediaPlayer_select_file,
    ithMediaPlayer_play,
    ithMediaPlayer_pause,
    ithMediaPlayer_stop,
    ithMediaPlayer_deinit,
    ithMediaPlayer_volume_up,
    ithMediaPlayer_volume_down,
    ithMediaPlayer_mute
};
#else // no defined _MSC_VER
ithMediaPlayer player1 = {
    .init = ithMediaPlayer_init,
    .select = ithMediaPlayer_select_file,
    .play = ithMediaPlayer_play,
    .pause = ithMediaPlayer_pause,
    .stop = ithMediaPlayer_stop,
    .deinit = ithMediaPlayer_deinit,
    .vol_up = ithMediaPlayer_volume_up,
    .vol_down = ithMediaPlayer_volume_down,
    .mute = ithMediaPlayer_mute
};
#endif
ithMediaPlayer *media_player=&player1;

void pi_set_sync_type(PlayerProps *pprop)
{
    pprop->av_sync_type = AV_SYNC_VIDEO_MASTER;
}

void pi_set_playback_mode(PlayerInstance *is)
{
    // disable loop mode
    is->loop = 1;
    is->autoexit = 1;
}

void pi_video_disable(PlayerInstance *is)
{
    is->video_disable = 1;
}

void pi_audio_disable(PlayerInstance *is)
{
    is->audio_disable = 1;
}

