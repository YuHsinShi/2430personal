/*
 * muxing functions for use within FFmpeg
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* #define DEBUG */

#include "avformat.h"
#include "avio_internal.h"
#include "internal.h"
#include "../libavcodec/internal.h"
//#include "libavcodec/bytestream.h"
//#include "libavutil/opt.h"
#include "../libavutil/dict.h"
//#include "libavutil/pixdesc.h"
#include "../libavutil/timestamp.h"
//#include "metadata.h"
//#include "id3v2.h"
//#include "libavutil/avassert.h"
//#include "libavutil/avstring.h"
#include "../libavutil/mathematics.h"
//#include "libavutil/time.h"
//#include "riff.h"
//#include "audiointerleave.h"
//#include "url.h"
//#include <stdarg.h>
//#if CONFIG_NETWORK
//#include "network.h"
//#endif
//
//#undef NDEBUG
//#include <assert.h>

/**
 * @file
 * muxing functions for use within libavformat
 */

/* fraction handling */

/**
 * f = val + (num / den) + 0.5.
 *
 * 'num' is normalized so that it is such as 0 <= num < den.
 *
 * @param f fractional number
 * @param val integer value
 * @param num must be >= 0
 * @param den must be >= 1
 */
static void frac_init(AVFrac *f, int64_t val, int64_t num, int64_t den)
{
    num += (den >> 1);
    if (num >= den) {
        val += num / den;
        num  = num % den;
    }
    f->val = val;
    f->num = num;
    f->den = den;
}

/**
 * Fractional addition to f: f = f + (incr / f->den).
 *
 * @param f fractional number
 * @param incr increment, can be positive or negative
 */
//static void frac_add(AVFrac *f, int64_t incr)
//{
//    int64_t num, den;
//
//    num = f->num + incr;
//    den = f->den;
//    if (num < 0) {
//        f->val += num / den;
//        num     = num % den;
//        if (num < 0) {
//            num += den;
//            f->val--;
//        }
//    } else if (num >= den) {
//        f->val += num / den;
//        num     = num % den;
//    }
//    f->num = num;
//}

static int init_muxer(AVFormatContext *s)
{
    int ret = 0, i;
    AVStream *st;
    AVCodecContext *codec = NULL;
    AVOutputFormat *of;

    if (!s)    
        return -1;
    of = s->oformat;
    // some sanity checks
    if (s->nb_streams == 0 && !(of->flags & AVFMT_NOSTREAMS)) {
        av_log(s, AV_LOG_ERROR, "no streams\n");
        ret = AVERROR(EINVAL);
        goto fail;
    }

    for (i = 0; i < s->nb_streams; i++) {
        st    = s->streams[i];
        codec = st->codec;

        switch (codec->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            if (codec->sample_rate <= 0) {
                av_log(s, AV_LOG_ERROR, "sample rate not set\n");
                ret = AVERROR(EINVAL);
                goto fail;
            }
            break;
        case AVMEDIA_TYPE_VIDEO:
            if (codec->time_base.num <= 0 ||
                codec->time_base.den <= 0) { //FIXME audio too?
                av_log(s, AV_LOG_ERROR, "time base not set\n");
                ret = AVERROR(EINVAL);
                goto fail;
            }

            if ((codec->width <= 0 || codec->height <= 0) &&
                !(of->flags & AVFMT_NODIMENSIONS)) {
                av_log(s, AV_LOG_ERROR, "dimensions not set\n");
                ret = AVERROR(EINVAL);
                goto fail;
            }
            if (av_cmp_q(st->sample_aspect_ratio, codec->sample_aspect_ratio)
                && FFABS(av_q2d(st->sample_aspect_ratio) - av_q2d(codec->sample_aspect_ratio)) > 0.004*av_q2d(st->sample_aspect_ratio)
            ) {
                av_log(s, AV_LOG_ERROR, "Aspect ratio mismatch between muxer "
                                        "(%d/%d) and encoder layer (%d/%d)\n",
                       st->sample_aspect_ratio.num, st->sample_aspect_ratio.den,
                       codec->sample_aspect_ratio.num,
                       codec->sample_aspect_ratio.den);
                ret = AVERROR(EINVAL);
                goto fail;
            }
            break;
        }

        if (of->codec_tag) {
            codec->codec_tag = av_codec_get_tag(of->codec_tag, codec->codec_id);
        }

        if (of->flags & AVFMT_GLOBALHEADER &&
            !(codec->flags & CODEC_FLAG_GLOBAL_HEADER))
            av_log(s, AV_LOG_WARNING,
                   "Codec for stream %d does not use global headers "
                   "but container format requires global headers\n", i);
    }

    if (!s->priv_data && of->priv_data_size > 0) {
        s->priv_data = av_mallocz(of->priv_data_size);
        if (!s->priv_data) {
            trac("");
            ret = AVERROR(ENOMEM);
            goto fail;
        }
    }

    return 0;

fail:
    return ret;
}

static int init_pts(AVFormatContext *s)
{
    int i;
    AVStream *st;

    /* init PTS generation */
    for (i = 0; i < s->nb_streams; i++) {
        int64_t den = AV_NOPTS_VALUE;
        st = s->streams[i];

        switch (st->codec->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            den = (int64_t)st->time_base.num * st->codec->sample_rate;
            break;
        case AVMEDIA_TYPE_VIDEO:
            den = (int64_t)st->time_base.num * st->codec->time_base.den;
            break;
        default:
            break;
        }
        if (den != AV_NOPTS_VALUE) {
            if (den <= 0)
                return AVERROR_INVALIDDATA;

            frac_init(&st->pts, 0, 0, den);
        }
    }

    return 0;
}

int avformat_write_header(AVFormatContext *s)
{
    int ret = 0;

    if (ret = init_muxer(s))
        return ret;

    if (s->oformat->write_header) {
        ret = s->oformat->write_header(s);
        if (ret >= 0 && s->pb && s->pb->error < 0)
            ret = s->pb->error;
        if (ret < 0)
            return ret;
    }

    if ((ret = init_pts(s)) < 0)
        return ret;

    return 0;
}

int avformat_init_format(AVFormatContext *s)
{
    int ret = 0;
    if (s->oformat->init_format) {
        ret = s->oformat->init_format(s);
    }
    return ret;
}

/**
 * Move side data from payload to internal struct, call muxer, and restore
 * original packet.
 */
static inline int split_write_packet(AVFormatContext *s, AVPacket *pkt)
{
    return s->oformat->write_packet(s, pkt);
}

#define CHUNK_START 0x1000

int av_interleaved_write_frame(AVFormatContext *s, AVPacket *pkt)
{
    int ret, flush = 0;

    for (;; ) {
        AVPacket* opkt = pkt;
        if (!opkt)
            return 0;

        ret = split_write_packet(s, opkt);

        //av_free_packet(opkt);
        pkt = NULL;

        if (ret != 0)
            return ret;
        if (s->pb && s->pb->error)
            return s->pb->error;
    }
}

int av_write_trailer(AVFormatContext *s)
{
    int ret = 0, i;

    if (s->oformat->write_trailer)
        ret = s->oformat->write_trailer(s);

    if (s->pb)
       avio_flush(s->pb);
    if (ret == 0)
       ret = s->pb ? s->pb->error : 0;
    for (i = 0; i < s->nb_streams; i++) {
        av_freep(&s->streams[i]->priv_data);
    }
    av_freep(&s->priv_data);
    return ret;
}

uint64_t av_write_size(AVFormatContext *s)
{
    uint64_t ret = 0;

    if (s->oformat->write_size)
        ret = s->oformat->write_size(s);
    return ret;
}
