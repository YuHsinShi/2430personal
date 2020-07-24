/*
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <sys/types.h>
//#include <inttypes.h>
//#include <limits.h>
#include "../config.h"
//#include "mp_msg.h"
//#include "help_mp.h"
//#include "mencoder.h"
#include "aviheader.h"
#include "ms_hdr.h"
//#include "av_opts.h"
#include "../av_helpers.h"

#include "../stream/stream.h"
#include "muxer.h"
//#include "demuxer.h"
#include "stheader.h"
//#include "m_option.h"
#include "../libavformat/avformat.h"
//#include "libavutil/avstring.h"
#include "../libavutil/dict.h"

#include "mp_taglists.h"

enum PixelFormat imgfmt2pixfmt(int fmt);

#define BIO_BUFFER_SIZE 32

typedef struct {
    AVFormatContext *oc;
    int audio_streams;
    int video_streams;
    int64_t last_pts;
    uint8_t buffer[BIO_BUFFER_SIZE];
} muxer_priv_t;

typedef struct {
    int64_t last_pts;
    AVStream *avstream;
} muxer_stream_priv_t;

static char *conf_format = "mp4";

static int mp_write(void *opaque, uint8_t *buf, int size)
{
    muxer_t *muxer = opaque;
    return stream_write_buffer(muxer->stream, buf, size);
}

static int64_t mp_seek(void *opaque, int64_t pos, int whence)
{
    muxer_t *muxer = opaque;
    if(whence == SEEK_CUR)
    {
        offset_t cur = stream_tell(muxer->stream);
        if(cur == -1)
            return -1;
        pos += cur;
    }
    mp_msg(MSGT_MUXER, MSGL_DBG2, "SEEK %"PRIu64"\n", (int64_t)pos);
    if(!stream_seek(muxer->stream, pos))
        return -1;
    return 0;
}

static uint64_t mp_write_size(void *opaque)
{
    muxer_t *muxer = opaque;
    return stream_write_size(muxer->stream);
}

static muxer_stream_t* lavf_new_stream(muxer_t *muxer, int type)
{
    muxer_priv_t *priv = muxer->priv;
    muxer_stream_t *stream;
    muxer_stream_priv_t *spriv;
    AVCodecContext *ctx;

    if (type != MUXER_TYPE_VIDEO && type != MUXER_TYPE_AUDIO)
    {
        mp_msg(MSGT_MUXER, MSGL_ERR, "UNKNOWN TYPE %d\n", type);
        return NULL;
    }

    stream = calloc(1, sizeof(muxer_stream_t));
    if (!stream)
    {
        trac("");
        mp_msg(MSGT_MUXER, MSGL_ERR, "Could not allocate muxer_stream, EXIT.\n");
        return NULL;
    }
    muxer->streams[muxer->avih.dwStreams] = stream;

    spriv = calloc(1, sizeof(muxer_stream_priv_t));
    if (!spriv)
    {
        free(stream);
        trac("");
        return NULL;
    }
    stream->priv = spriv;

    spriv->avstream = avformat_new_stream(priv->oc, NULL);
    if (!spriv->avstream)
    {
        mp_msg(MSGT_MUXER, MSGL_ERR, "Could not allocate avstream, EXIT.\n");
        trac("");
        return NULL;
    }
    spriv->avstream->id = 1;

    ctx = spriv->avstream->codec;
    ctx->codec_id = CODEC_ID_NONE;
    switch(type)
    {
    case MUXER_TYPE_VIDEO:
        ctx->codec_type = AVMEDIA_TYPE_VIDEO;
        break;

    case MUXER_TYPE_AUDIO:
        ctx->codec_type = AVMEDIA_TYPE_AUDIO;
        break;
    }

    muxer->avih.dwStreams++;
    stream->muxer = muxer;
    stream->type = type;
    mp_msg(MSGT_MUXER, MSGL_V, "ALLOCATED STREAM N. %d, type=%d\n", muxer->avih.dwStreams, type);
    return stream;
}

static void lavf_free_stream(muxer_stream_t* ms)
{
    if (ms)
    {
        if (ms->priv)
            free(ms->priv);
        if (ms->bih)
            free(ms->bih);
        if (ms->wf)
            free(ms->wf);
        free(ms);
    }
}

static void lavf_free_muxer(muxer_t *muxer)
{
    uint32_t num;

    if (!muxer)
        return;

    for (num = 0; num < muxer->avih.dwStreams; ++num)
    {
        muxer_stream_t *str = muxer->streams[num];
        lavf_free_stream(str);
    }
    if (muxer->idx)
        free(muxer->idx);
    free(muxer);
}

static void fix_parameters(muxer_stream_t *stream)
{
    muxer_stream_priv_t *spriv = stream->priv;
    AVCodecContext *ctx = spriv->avstream->codec;

    ctx->bit_rate= stream->avg_rate;
    ctx->rc_buffer_size= stream->vbv_size;
    ctx->rc_max_rate= stream->max_rate;

    if (stream->type == MUXER_TYPE_AUDIO)
    {
        if (!ctx->bit_rate)
            ctx->bit_rate = stream->wf->nAvgBytesPerSec * 8;
        ctx->codec_id = mp_tag2codec_id(stream->wf->wFormatTag, 1);
        ctx->sample_rate = stream->wf->nSamplesPerSec;
        ctx->channels = stream->wf->nChannels;
        if (stream->h.dwRate && (stream->h.dwScale * (int64_t)ctx->sample_rate) % stream->h.dwRate == 0)
            ctx->frame_size= (stream->h.dwScale * (int64_t)ctx->sample_rate) / stream->h.dwRate;
        if (stream->wf+1 && stream->wf->cbSize)
        {
            ctx->extradata = av_malloc(stream->wf->cbSize);
            if (ctx->extradata != NULL)
            {
                ctx->extradata_size = stream->wf->cbSize;
                memcpy(ctx->extradata, stream->wf+1, ctx->extradata_size);
            }
        }
    }
    else if(stream->type == MUXER_TYPE_VIDEO)
    {
        ctx->codec_id = mp_tag2codec_id(stream->bih->biCompression, 0);
        if (ctx->codec_id <= 0)
            ctx->codec_tag= stream->bih->biCompression;
        ctx->width = stream->bih->biWidth;
        ctx->height = stream->bih->biHeight;
        ctx->bit_rate = 800000;
        ctx->time_base.den = stream->h.dwRate;
        ctx->time_base.num = stream->h.dwScale;
        if (stream->aspect)
            ctx->sample_aspect_ratio =
            spriv->avstream->sample_aspect_ratio = av_d2q(stream->aspect * ctx->height / ctx->width, 255);

        if (stream->bih->biSize > sizeof(*stream->bih))
        {
            ctx->extradata_size = stream->bih->biSize - sizeof(*stream->bih);
            ctx->extradata = av_malloc(ctx->extradata_size);
            if (ctx->extradata != NULL)
                memcpy(ctx->extradata, stream->bih+1, ctx->extradata_size);
            else
            {
                trac("");
                mp_msg(MSGT_MUXER, MSGL_ERR, "MUXER_LAVF(video stream) error! Could not allocate %d bytes for extradata.\n",
                    ctx->extradata_size);
                ctx->extradata_size = 0;
            }
        }
    }
}

static int write_chunk(muxer_stream_t *stream, size_t len, unsigned int flags, double dts, double pts)
{
    muxer_t *muxer = stream->muxer;
    muxer_priv_t *priv = muxer->priv;
    muxer_stream_priv_t *spriv = stream->priv;
    AVPacket pkt;
    int result = 0;

    if (len)
    {
        av_init_packet(&pkt);
        pkt.size = len;
        pkt.stream_index= spriv->avstream->index;
        pkt.data = stream->buffer;

        if(flags & AVIIF_KEYFRAME)
            pkt.flags |= AV_PKT_FLAG_KEY;
        else
            pkt.flags = 0;

        pkt.dts = (dts / av_q2d(priv->oc->streams[pkt.stream_index]->time_base) + 0.5);
        pkt.pts = (pts / av_q2d(priv->oc->streams[pkt.stream_index]->time_base) + 0.5);
        ////fprintf(stderr, "%Ld %Ld id:%d tb:%f %f\n", pkt.dts, pkt.pts, pkt.stream_index, av_q2d(priv->oc->streams[pkt.stream_index]->time_base), stream->timer);

        if ((result = av_interleaved_write_frame(priv->oc, &pkt)) != 0)
        {
            mp_msg(MSGT_MUXER, MSGL_ERR, "Error while writing frame.\n");
        }
    }

    return result;
}

static void write_header(muxer_t *muxer)
{
    muxer_priv_t *priv = muxer->priv;

    mp_msg(MSGT_MUXER, MSGL_INFO, MSGTR_WritingHeader);
    avformat_write_header(priv->oc);
    muxer->cont_write_header = NULL;
}

static void write_trailer(muxer_t *muxer)
{
    muxer_priv_t *priv = muxer->priv;

    mp_msg(MSGT_MUXER, MSGL_INFO, MSGTR_WritingTrailer);
    av_write_trailer(priv->oc);

    avformat_close_input(&priv->oc);
}

static uint64_t write_size(muxer_t *muxer)
{
    muxer_priv_t *priv = muxer->priv;
    return av_write_size(priv->oc);
}

static int init_format(muxer_t *muxer)
{
    muxer_priv_t *priv = muxer->priv;
    return avformat_init_format(priv->oc);
}

int muxer_init_muxer_lavf(muxer_t *muxer)
{
    muxer_priv_t *priv;
    AVOutputFormat *fmt = NULL;

    init_avformat();

    priv = calloc(1, sizeof(muxer_priv_t));
    if (priv == NULL)
    {
        trac("");
        return 0;
    }

    priv->oc = avformat_alloc_context();
    if (!priv->oc)
    {
        trac("");
        mp_msg(MSGT_MUXER, MSGL_FATAL, "Could not get format context.\n");
        goto fail;
    }

    if (conf_format)
        fmt = av_guess_format(conf_format, NULL, NULL);
    priv->oc->oformat = fmt;
    if (muxer->cache_data_in_file)
        priv->oc->flags |= AVFMT_FLAG_CACHE_DATA_IN_FILE;

    priv->oc->pb = avio_alloc_context(priv->buffer, BIO_BUFFER_SIZE, muxer, mp_write, mp_seek, mp_write_size);
    //if ((muxer->stream->flags & MP_STREAM_SEEK) != MP_STREAM_SEEK)
    //    priv->oc->pb->seekable = 0;
    priv->oc->pb->url = muxer->stream->url;

    muxer->priv = priv;
    muxer->cont_new_stream = &lavf_new_stream;
    //muxer->cont_free_muxer = &lavf_free_muxer;
    muxer->cont_free_stream = &lavf_free_stream;
    muxer->cont_init_format = &init_format;
    muxer->cont_write_chunk = &write_chunk;
    muxer->cont_write_header = &write_header;
    muxer->cont_write_index = &write_trailer;
    muxer->cont_write_size = &write_size;
    muxer->fix_stream_parameters = &fix_parameters;
    mp_msg(MSGT_MUXER, MSGL_INFO, "OK, exit.\n");
    if (0 != muxer_init_format(muxer))
        goto fail;
    return 1;

fail:
    if (priv->oc)
    {
        avformat_close_input(&priv->oc);
    }
    free(priv);
    return 0;
}
