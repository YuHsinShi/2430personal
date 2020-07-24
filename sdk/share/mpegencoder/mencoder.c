#include "mencoder.h"
#include "libavutil/common.h"
#include <sys/time.h>

#define ALIGN_PTS(mectx, media_type, pts)   \
    (TimeStamp2SampleCount(mectx, media_type, pts, mectx->timeunit[media_type]) * mectx->timeunit[media_type])

//=============================================================================
//                              Global Data Definition
//=============================================================================
//#define MAX_CHUNK_COUNT 420000
#define MAX_CHUNK_COUNT 320000
static MEContext mectx_s;
static MEContext *mectx = &mectx_s;

int index_mode = 1;

//=============================================================================
//                              Private Function Declaration
//=============================================================================
static MMP_RESULT
_MENCODER_Open(
    MEContext*  mectx,
    MMP_BOOL    bSync);

static MMP_RESULT
_MENCODER_Close(
    MEContext*  mectx,
    MMP_BOOL    bSync);

static MMP_RESULT
_MENCODER_SetAudioInfo(
    MEContext *mectx,
    AUDIO_INFO *ptInfo);

static MMP_RESULT
_MENCODER_SetVideoInfo(
    MEContext*  mectx,
    VIDEO_INFO* ptInfo);

static void
_MENCODER_GenerateOutFileName(
    MEContext* mectx);

static int
_MENCODER_GetWriteSize(
    MEContext* mectx);

static MMP_INLINE void
TimeStamp2SampleCount_Init(
    MEContext*  mectx)
{
    mectx->pre_ori_pts[AUDIO] = 0;
    mectx->sample_count[AUDIO] = 0;
    mectx->pre_ori_pts[VIDEO] = 0;
    mectx->sample_count[VIDEO] = 0;
}

static MMP_INLINE int
TimeStamp2SampleCount(
    MEContext*  mectx,
    int         media_type,
    double      pts,
    double      unit)
{
    double  result;
    int     gap;

    gap = (int)(((pts - mectx->pre_ori_pts[media_type]) / unit) + 0.5);

    mectx->pre_ori_pts[media_type] = pts;
    mectx->sample_count[media_type] += gap;
    return mectx->sample_count[media_type];
}

static unsigned int GetClock(void)
{
    struct timeval tv;

    if (gettimeofday(&tv, NULL) != 0)
        printf("gettimeofday failed!\n");
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static unsigned long GetDuration(unsigned int clock)
{
    struct timeval tv;

    if (gettimeofday(&tv, NULL) != 0)
        printf("gettimeofday failed!\n");
    return (unsigned int)(tv.tv_sec*1000+tv.tv_usec/1000) - clock;
}

#ifdef DEBUG_WRITE_FAIL
static void
_PrintFileName(
    MMP_CHAR* filename)
{
    printf("filename: %s\n", filename);
}
#else
#define _PrintFileName(f)
#endif

//=============================================================================
//                              Public Function Definition
//=============================================================================
MEContext*
mencoder_Init(
    void)
{
    memset(mectx, 0, sizeof(MEContext));
    mectx->out_file_format  = MUXER_TYPE_LAVF;
    //mectx->out_file_format = MUXER_TYPE_AVI;
    mectx->max_mux_f_size   = 2000; // 2000 MBytes
    return mectx;
}

void
mencoder_Terminate(
    MEContext*  mectx)
{}

MMP_RESULT
mencoder_Open(
    MEContext*  mectx,
    MMP_CHAR*      filename,
    RECORD_MODE*    ptRecMode)
{
    do
    {
        MMP_CHAR* extension;

        if (!filename)
            break;
        mectx->acc_mux_f_size   = 0;
        mectx->ori_filename     = PalStrdup(filename);
        mectx->fragment_count   = 0;
        mectx->a_pts            =
        mectx->v_pts            = -1;
        mectx->timeunit[AUDIO]  =
        mectx->timeunit[VIDEO]  = 0;
        extension = PalStrrchr(mectx->ori_filename, '.');
        if (extension != MMP_NULL)
            *extension = 0;
        mectx->do_not_fragment  = ptRecMode->bDisableFileSplitter;
        //printf("mectx->do_not_fragment(%d)\n", mectx->do_not_fragment);

        mectx->osbc             = new_stream_buffer_ctrl();
        if (!mectx->osbc)
        {
            trac("");
            break;
        }

        mectx->osi                  = 0;
#ifndef OPEN_ONE_FILE_ONCE
        if (!mectx->do_not_fragment)
        {
            _MENCODER_GenerateOutFileName(mectx);
            _PrintFileName(mectx->out_filename);
            mectx->ostreams[mectx->osi] = open_output_stream(mectx->out_filename, MMP_TRUE);
            if (!mectx->ostreams[mectx->osi])
            {
                mp_msg(MSGT_MENCODER, MSGL_FATAL, MSGTR_CannotOpenOutputFile, filename);
                break;
            }
            mectx->osi      = !mectx->osi;
        }
#endif

        if (MMP_SUCCESS != _MENCODER_Open(mectx, mectx->do_not_fragment))
        {
            break;
        }
        return MMP_SUCCESS;
    } while(0);

    if (mectx->ori_filename)
    {
        free(mectx->ori_filename);
        mectx->ori_filename = MMP_NULL;
    }

    if (mectx->osbc)
    {
        free_stream_buffer_ctrl(mectx->osbc);
        mectx->osbc = MMP_NULL;
    }
    return MMP_RESULT_ERROR;
}

MMP_RESULT
mencoder_Close(
    MEContext*  mectx)
{
    _MENCODER_Close(mectx, MMP_TRUE);
#ifndef OPEN_ONE_FILE_ONCE
    if (!mectx->do_not_fragment)
    {
        mectx->osi      = !mectx->osi;
        mectx->ostream  = mectx->ostreams[mectx->osi];
        free_stream(mectx->ostream, MMP_TRUE);
        // delete file
        //_PrintFileName(mectx->out_filename);
        //PalWFileDelete(mectx->out_filename, MMP_NULL, MMP_NULL);
    }
#endif
    mectx->ostream  = MMP_NULL;

    free_stream_buffer_ctrl(mectx->osbc);
    mectx->osbc = MMP_NULL;
    free(mectx->ori_filename);
    mectx->ori_filename = MMP_NULL;

    return MMP_SUCCESS;
}

MMP_RESULT
mencoder_SetVideoInfo(
    MEContext*  mectx,
    VIDEO_INFO* ptInfo)
{
    if (!ptInfo || !mectx || !mectx->muxer)
    {
        printf("%s(%d)\n", __FUNCTION__, __LINE__);
        return MMP_RESULT_ERROR;
    }

    if (mectx->sh_video)
    {
        printf("%s(%d)\n", __FUNCTION__, __LINE__);
        return MMP_RESULT_ERROR;
    }

    // ============= VIDEO ===============
    memcpy(&mectx->tVideoInfo, ptInfo, sizeof(VIDEO_INFO));
    _MENCODER_SetVideoInfo(mectx, &mectx->tVideoInfo);

    //printf("%s(%d)\n", __FUNCTION__, __LINE__);
    return MMP_SUCCESS;
}

MMP_RESULT
mencoder_SetAudioInfo(
    MEContext *mectx,
    AUDIO_INFO *ptInfo)
{
    if (!ptInfo || !mectx)
    {
        printf("%s(%d)\n", __FUNCTION__, __LINE__);
        return MMP_RESULT_ERROR;
    }

    // ============= AUDIO ===============
    memcpy(&mectx->tAudioInfo, ptInfo, sizeof(AUDIO_INFO));

    //printf("%s(%d)\n", __FUNCTION__, __LINE__);
    return MMP_SUCCESS;
}

MMP_RESULT
mencoder_WriteAudioChunk(
    MEContext   *mectx,
    char        *data,
    int         size,
    double      pts)
{
    if (!mectx->sh_audio)
    {
        printf("%s (%d)\n", __FUNCTION__, __LINE__);
        return MMP_RESULT_ERROR;
    }
    if (size <= 0)
    {
        printf("%s (%d)\n", __FUNCTION__, __LINE__);
        return MMP_RESULT_ERROR;    // EOF?
    }
    //if (stream_write_size(mectx->ostream) > mectx->max_mux_f_size)
    //{
    //    printf("%s (%d)\n", __FUNCTION__, __LINE__);
    //}
    if (mectx->base_pts < 0)
    {
        printf("%s (%d)\n", __FUNCTION__, __LINE__);
        return MMP_RESULT_ERROR;
    }
    pts = ALIGN_PTS(mectx, AUDIO, pts);
    if (pts > mectx->base_pts
     && pts > (mectx->a_pts + mectx->base_pts))
    {
        mectx->a_pts            = pts - mectx->base_pts;
		#if 0
        if (mectx->tAudioInfo.audioEncoderType == ITE_MP2_DECODE)
        {
            mectx->mux_a->buffer         = data;
            mectx->is_memory_not_enough |= !!muxer_write_chunk(mectx->mux_a, size, AVIIF_KEYFRAME, mectx->a_pts, mectx->a_pts);
        }
        else
		#endif
        {
            mectx->mux_a->buffer         = data + 7;
            mectx->is_memory_not_enough |= !!muxer_write_chunk(mectx->mux_a, size - 7, AVIIF_KEYFRAME, mectx->a_pts, mectx->a_pts);
        }
    }
    return MMP_SUCCESS;
}

MMP_RESULT
mencoder_WriteVideoChunk(
    MEContext*  mectx,
    char        *data,
    int         size,
    double      pts,
    MMP_BOOL    isKeyFrame)
{
    unsigned int clock = GetClock();
    if (!mectx->sh_video)
    {
        printf("%s (%d)\n", __FUNCTION__, __LINE__);
        return MMP_RESULT_ERROR;
    }
    if (size <= 0)
    {
        printf("%s (%d)\n", __FUNCTION__, __LINE__);
        return MMP_RESULT_ERROR;    // EOF?
    }
    
    if ((!mectx->do_not_fragment)
     && (mectx->is_memory_not_enough
      || (_MENCODER_GetWriteSize(mectx) > mectx->max_mux_f_size)))
    {
       // printf("isKeyFrame = %d , mectx->a_pts = %f, mectx->v_pts = %f\n", isKeyFrame, mectx->a_pts,
        //mectx->v_pts);
        if (isKeyFrame)
        {
            //if ((mectx->a_pts == 0) || (mectx->a_pts >= mectx->v_pts))
            //{   // ready to switch to the next file
                _MENCODER_Close(mectx, MMP_FALSE);
                _MENCODER_Open(mectx, MMP_FALSE);
                _MENCODER_SetVideoInfo(mectx, &mectx->tVideoInfo);
            //}
            //else
            //{
            //    printf("%s (%d)\n", __FUNCTION__, __LINE__);
            //    return MMP_RESULT_ERROR;
            //}
        }
    }
    if (mectx->base_pts < 0)
    {
        if (isKeyFrame)
        {
            if (GetDuration(clock) <= 180)
            {
                TimeStamp2SampleCount_Init(mectx);
                pts = ALIGN_PTS(mectx, VIDEO, pts);
                mectx->base_pts = pts;
                mectx->v_pts = -1;
                mectx->a_pts = -1;
            }
            else
                return MMP_SUCCESS;
        }
        else
            return MMP_SUCCESS;
    }
    else
        pts = ALIGN_PTS(mectx, VIDEO, pts);

    if (pts > (mectx->v_pts + mectx->base_pts))
    {
        mectx->v_pts            = pts - mectx->base_pts;
        mectx->mux_v->buffer    = data;
        mectx->is_memory_not_enough |= !!muxer_write_chunk(mectx->mux_v, size, (isKeyFrame ? AVIIF_KEYFRAME : 0), mectx->v_pts, mectx->v_pts);
    }
    return MMP_SUCCESS;
}

stream_buffer_ctrl_t*
mencoder_GetStreamBufferCtrl(
    void)
{
    return mectx->osbc;
}

int
mencoder_GetWriteSize(
    void)
{
    return mectx->acc_mux_f_size + _MENCODER_GetWriteSize(mectx);
}

//=============================================================================
//                              Private Function Definition
//=============================================================================
static MMP_RESULT
_MENCODER_Open(
    MEContext*  mectx,
    MMP_BOOL    bSync)
{
    stream_t **ostream = &mectx->ostreams[mectx->osi];
#ifdef OPEN_ONE_FILE_ONCE
    bSync = MMP_TRUE;
#endif
    do
    {
        //mectx->out_file_format = MUXER_TYPE_AVI;
        _MENCODER_GenerateOutFileName(mectx);
        _PrintFileName(mectx->out_filename);
        mectx->ostreams[mectx->osi] = open_output_stream(mectx->out_filename, bSync);
        if (!mectx->ostreams[mectx->osi])
        {
            mp_msg(MSGT_MENCODER, MSGL_FATAL, MSGTR_CannotOpenOutputFile, filename);
            break;
        }
#ifndef OPEN_ONE_FILE_ONCE
        if (!mectx->do_not_fragment)
            mectx->osi      = !mectx->osi;
#endif
        mectx->ostream  = mectx->ostreams[mectx->osi];

        mectx->muxer = muxer_new_muxer(mectx->out_file_format, mectx->ostream, mectx->do_not_fragment);
        if (!mectx->muxer)
        {
            mp_msg(MSGT_MENCODER, MSGL_FATAL, MSGTR_CannotInitializeMuxer);
            break;
        }

        _MENCODER_SetAudioInfo(mectx, &mectx->tAudioInfo);
        mectx->base_pts = -1;
        mectx->is_memory_not_enough  = 0;
        return MMP_SUCCESS;
    } while (0);

    if (*ostream)
    {
        free_stream(*ostream, MMP_TRUE);
        *ostream = MMP_NULL;
    }
    return MMP_RESULT_ERROR;
}

static MMP_RESULT
_MENCODER_Close(
    MEContext*  mectx,
    MMP_BOOL    bSync)
{
#ifdef OPEN_ONE_FILE_ONCE
    bSync = MMP_TRUE;
#endif
    /* flush muxer just in case, this is a no-op unless
     * we created a stream but never wrote frames to it... */
    muxer_flush(mectx->muxer);
    if (mectx->muxer->cont_write_index)
        muxer_write_index(mectx->muxer);
    stream_seek(mectx->muxer->stream,0);
    if (mectx->muxer->cont_write_header)
        muxer_write_header(mectx->muxer);                                     // update header

    muxer_free_muxer(mectx->muxer);
    mectx->sh_audio = MMP_NULL;
    mectx->sh_video = MMP_NULL;
    mectx->mux_a = MMP_NULL;
    mectx->mux_v = MMP_NULL;
    mectx->muxer = MMP_NULL;
    mectx->acc_mux_f_size += _MENCODER_GetWriteSize(mectx);
    free_stream(mectx->ostream, bSync);
    mectx->ostream = MMP_NULL;

    return MMP_SUCCESS;
}

MMP_RESULT
_MENCODER_SetAudioInfo(
    MEContext *mectx,
    AUDIO_INFO *ptInfo)
{
    if (!mectx || !mectx->muxer)
    {
        printf("%s(%d)\n", __FUNCTION__, __LINE__);
        return MMP_RESULT_ERROR;
    }

    if (mectx->sh_audio)
    {
        printf("%s(%d)\n", __FUNCTION__, __LINE__);
        return MMP_RESULT_ERROR;
    }
	
    // ============= AUDIO ===============
    mectx->_sh_audio.format     = /*(ptInfo->audioEncoderType == ITE_MP2_DECODE)
                                ? 0x00000055
                                : */MKTAG('M', 'P', '4', 'A');
    mectx->_sh_audio.channels   = 2;
    mectx->_sh_audio.samplerate = 48000;
    mectx->_sh_audio.i_bps      = mectx->_sh_audio.samplerate
                                * mectx->_sh_audio.channels
                                * 2 / 8;
    mectx->sh_audio             = &mectx->_sh_audio;
    if (mectx->sh_audio)
    {
        mectx->mux_a = muxer_new_stream(mectx->muxer,MUXER_TYPE_AUDIO);

        //////////////////////////////////////
        //mectx->mux_a->buffer_size   = 0x100000; //16384;
        //mectx->mux_a->buffer        = malloc(mectx->mux_a->buffer_size);
        //if (!mectx->mux_a->buffer)
        //    break;

        mectx->mux_a->source=mectx->sh_audio;
        //mux_a->codec=out_audio_codec;

        //if (mectx->sh_audio->wf)
        //{
        //    mectx->mux_a->wf=malloc(sizeof(*mectx->mux_a->wf) + mectx->sh_audio->wf->cbSize);
        //    memcpy(mectx->mux_a->wf, mectx->sh_audio->wf, sizeof(*mectx->mux_a->wf) + mectx->sh_audio->wf->cbSize);
        //    if(!mectx->sh_audio->i_bps) mectx->sh_audio->i_bps=mectx->mux_a->wf->nAvgBytesPerSec;
        //}
        //else

		#if 0
        if (ptInfo->audioEncoderType == MMP_MP2_ENCODE)
        {
            mectx->mux_a->wf                  = malloc(sizeof(*mectx->mux_a->wf));
            mectx->mux_a->wf->nBlockAlign     = 1; //mux_a->h.dwSampleSize;
            mectx->mux_a->wf->wFormatTag      = mectx->sh_audio->format;
            mectx->mux_a->wf->nChannels       = mectx->sh_audio->channels;
            mectx->mux_a->wf->nSamplesPerSec  = mectx->sh_audio->samplerate;
            mectx->mux_a->wf->nAvgBytesPerSec = mectx->sh_audio->i_bps; //mux_a->h.dwSampleSize*mux_a->wf->nSamplesPerSec;
            mectx->mux_a->wf->wBitsPerSample  = 16;                     // FIXME
            mectx->mux_a->wf->cbSize          = 0;                      // FIXME for l3codeca.acm
        }
        else
		#endif
        {
            MMP_UINT8 codec_data[] = {0x11, 0x90};
            mectx->mux_a->wf                  = malloc(sizeof(*mectx->mux_a->wf) + sizeof(codec_data));
            mectx->mux_a->wf->cbSize          = sizeof(codec_data);
            mectx->mux_a->wf->nBlockAlign     = 1; //mux_a->h.dwSampleSize;
            mectx->mux_a->wf->wFormatTag      = mectx->sh_audio->format;
            mectx->mux_a->wf->nChannels       = mectx->sh_audio->channels;
            mectx->mux_a->wf->nSamplesPerSec  = mectx->sh_audio->samplerate;
            mectx->mux_a->wf->nAvgBytesPerSec = mectx->sh_audio->i_bps; //mux_a->h.dwSampleSize*mux_a->wf->nSamplesPerSec;
            mectx->mux_a->wf->wBitsPerSample  = 16;                     // FIXME
            memcpy(mectx->mux_a->wf + 1, codec_data, sizeof(codec_data));
            mectx->timeunit[AUDIO]            = (double)1024 / mectx->_sh_audio.samplerate;
        }
        if(mectx->sh_audio->audio.dwScale)
        {
            mectx->mux_a->h.dwSampleSize=mectx->sh_audio->audio.dwSampleSize;
            mectx->mux_a->h.dwScale=mectx->sh_audio->audio.dwScale;
            mectx->mux_a->h.dwRate=mectx->sh_audio->audio.dwRate;
        }
        else
        {
            mectx->mux_a->h.dwSampleSize=mectx->mux_a->wf->nBlockAlign;
            mectx->mux_a->h.dwScale=mectx->mux_a->h.dwSampleSize;
            mectx->mux_a->h.dwRate=mectx->mux_a->wf->nAvgBytesPerSec;
        }
    }

    return MMP_SUCCESS;
}

static MMP_RESULT
_MENCODER_SetVideoInfo(
    MEContext*  mectx,
    VIDEO_INFO* ptInfo)
{
    mectx->_sh_video.disp_w = ptInfo->width;
    mectx->_sh_video.disp_h = ptInfo->height;

    switch (ptInfo->frameRate)
    {
    case AVC_FRAME_RATE_25HZ:
    	//mectx->_sh_video.fps = (float)ptInfo->EnFrameRate;
        mectx->_sh_video.fps = 2500;
        break;

    case AVC_FRAME_RATE_50HZ:
        mectx->_sh_video.fps = 5000;
        break;

    case AVC_FRAME_RATE_30HZ:
    case AVC_FRAME_RATE_VESA_30HZ:
        //mectx->_sh_video.fps = (float)ptInfo->EnFrameRate;
        mectx->_sh_video.fps = 3000;
        break;

    case AVC_FRAME_RATE_60HZ:
    case AVC_FRAME_RATE_VESA_60HZ:
        mectx->_sh_video.fps = 6000;
        break;

    case AVC_FRAME_RATE_29_97HZ:
    	//mectx->_sh_video.fps = ((float)ptInfo->EnFrameRate/30)*((float)29.97);
    	mectx->_sh_video.fps = 2997;
    	break;

    case AVC_FRAME_RATE_59_94HZ:
    	//mectx->_sh_video.fps = ((float)ptInfo->EnFrameRate/30)*((float)59.94);
        mectx->_sh_video.fps = 5994;
        break;

    case AVC_FRAME_RATE_23_97HZ:
    	//mectx->_sh_video.fps = ((float)ptInfo->EnFrameRate/24)*((float)23.976);
    	mectx->_sh_video.fps = 2397;
        break;

    case AVC_FRAME_RATE_24HZ:
    	//mectx->_sh_video.fps = (float)ptInfo->EnFrameRate;
        mectx->_sh_video.fps = 2400;
        break;

    default:
        printf("%s(%d)\n", __FUNCTION__, __LINE__);
        return  MMP_RESULT_ERROR;
    }

    mectx->_sh_video.format = mmioFOURCC('h','2','6','4');//0x10000005;  // H264
    mectx->sh_video         = &mectx->_sh_video;

    mectx->mux_v                   = muxer_new_stream(mectx->muxer, MUXER_TYPE_VIDEO);
    mectx->mux_v->source           = mectx->sh_video;
    mectx->mux_v->h.dwSampleSize   = 0;        // VBR
    switch (mectx->_sh_video.fps)
    {
    case 2500:
        mectx->mux_v->h.dwScale        = 1;
        mectx->mux_v->h.dwRate         = 25;
        break;

    case 5000:
        mectx->mux_v->h.dwScale        = 1;
        mectx->mux_v->h.dwRate         = 50;
        break;

    case 3000:
        mectx->mux_v->h.dwScale        = 1;
        mectx->mux_v->h.dwRate         = 30;
        break;

    case 6000:
        mectx->mux_v->h.dwScale        = 1;
        mectx->mux_v->h.dwRate         = 60;
        break;

    case 2997:
        mectx->mux_v->h.dwScale        = 1001;
        mectx->mux_v->h.dwRate         = 30000;
        break;

    case 5994:
        mectx->mux_v->h.dwScale        = 1001;
        mectx->mux_v->h.dwRate         = 60000;
        break;

    case 2397:
        mectx->mux_v->h.dwScale        = 1001;
        mectx->mux_v->h.dwRate         = 24000;
        break;

    case 2400:
        mectx->mux_v->h.dwScale        = 1;
        mectx->mux_v->h.dwRate         = 24;
        break;

    default:
        mectx->mux_v->h.dwScale        = 100;
        mectx->mux_v->h.dwRate         = mectx->mux_v->h.dwScale * mectx->sh_video->fps / 100;
        break;
    }
    mectx->timeunit[VIDEO]             = (double)mectx->mux_v->h.dwScale / mectx->mux_v->h.dwRate;
    mectx->mux_v->codec            = 0;
    mectx->mux_v->bih              = MMP_NULL;
    mectx->sh_video->codec         = NULL;
    //sh_video->vfilter       = NULL;

    {
        mectx->mux_v->bih=calloc(1, sizeof(*mectx->mux_v->bih) + ptInfo->AVCDecoderConfigurationRecordSize);
        mectx->mux_v->bih->biSize           = sizeof(*mectx->mux_v->bih) + ptInfo->AVCDecoderConfigurationRecordSize;
        mectx->mux_v->bih->biWidth          = mectx->sh_video->disp_w;
        mectx->mux_v->bih->biHeight         = mectx->sh_video->disp_h;
        mectx->mux_v->bih->biCompression    = mectx->sh_video->format;
        mectx->mux_v->bih->biPlanes         = 1;
        mectx->mux_v->bih->biBitCount       = 24; // FIXME!!!
        mectx->mux_v->bih->biSizeImage      = mectx->mux_v->bih->biWidth
                                            * mectx->mux_v->bih->biHeight
                                            * (mectx->mux_v->bih->biBitCount / 8);
        //mectx->mux_v->aspect                = 16.0/9.0;
        if (ptInfo->AVCDecoderConfigurationRecordSize > 0)
            memcpy(mectx->mux_v->bih + 1,
                   ptInfo->AVCDecoderConfigurationRecord,
                   ptInfo->AVCDecoderConfigurationRecordSize);
    }
    return MMP_SUCCESS;
}

//=============================================================================
/**
 * Convert a decimal integer to an Unicode string.
 *
 * @param value  The input decimal integer,
 *               its range is between -999999 and 999999.
 * @param string Pointer to the output Unicode string.
 * @return Pointer to the output Unicode string.
 */
//=============================================================================
MMP_CHAR*
smtkItow(
    MMP_INT    value,
    MMP_CHAR* string)
{
    MMP_INT i, d;
    MMP_INT flag = 0;
    MMP_CHAR* ptr = string;

    if (!value)
    {
        *ptr++ = 0x30;
        *ptr = 0;

        return string;
    }

    /* if this is a negative value insert the minus sign. */
    if (value < 0)
    {
        *ptr++ = PAL_T('-');

        /* Make the value positive. */
        value *= -1;
    }

    for (i = 100000; i > 0; i /= 10)
    {
        d = value / i;

        if (d || flag)
        {
            *ptr++ = (MMP_CHAR)(d + 0x30);
            value -= (d * i);
            flag = 1;
        }
    }

    /* Null terminate the string. */
    *ptr = 0;

    return string;
}

static void
_MENCODER_GenerateOutFileName(
    MEContext* mectx)
{
    MMP_CHAR numStr[10];
    PalStrcpy(mectx->out_filename, mectx->ori_filename);
    if (mectx->fragment_count > 0)
    {
        smtkItow(mectx->fragment_count, numStr);
        PalStrcat(mectx->out_filename, "_");
        PalStrcat(mectx->out_filename, numStr);
    }
    PalStrcat(mectx->out_filename, ".mp4");
    mectx->fragment_count++;
}

static int
_MENCODER_GetWriteSize(
    MEContext* mectx)
{
    if (mectx->muxer && mectx->muxer->cont_write_size)
        return (int)((muxer_write_size(mectx->muxer) + 0x80000) / 0x100000);
    else if (mectx->ostream)
        return (int)((stream_write_size(mectx->ostream) + 0x80000) / 0x100000);
    else
        return 0;
}
