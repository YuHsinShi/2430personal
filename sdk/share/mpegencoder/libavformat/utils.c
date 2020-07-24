/*
 * various utility functions for use within FFmpeg
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
//#include "libavcodec/raw.h"
//#include "libavcodec/bytestream.h"
//#include "libavutil/avassert.h"
//#include "libavutil/opt.h"
//#include "libavutil/dict.h"
//#include "libavutil/pixdesc.h"
//#include "metadata.h"
//#include "id3v2.h"
//#include "libavutil/avassert.h"
#include "../libavutil/avstring.h"
#include "../libavutil/mathematics.h"
//#include "libavutil/time.h"
#include "../libavutil/timestamp.h"
#include "riff.h"

/**
 * @file
 * various utility functions for use within FFmpeg
 */

/** head of registered output format linked list */
static AVOutputFormat *first_oformat = NULL;

AVOutputFormat *av_oformat_next(AVOutputFormat *f)
{
    if(f) return f->next;
    else  return first_oformat;
}

void av_register_output_format(AVOutputFormat *format)
{
    AVOutputFormat **p;
    p = &first_oformat;
    while (*p != NULL) p = &(*p)->next;
    *p = format;
    format->next = NULL;
}

AVOutputFormat *av_guess_format(const char *short_name, const char *filename,
                                const char *mime_type)
{
    AVOutputFormat *fmt = NULL, *fmt_found;

    /* Find the proper file type. */
    fmt_found = NULL;
    while ((fmt = av_oformat_next(fmt))) {
        fmt_found = fmt;
    }
    return fmt_found;
}

unsigned int ff_codec_get_tag(const AVCodecTag *tags, enum AVCodecID id)
{
    while (tags->id != AV_CODEC_ID_NONE) {
        if (tags->id == id)
            return tags->tag;
        tags++;
    }
    return 0;
}

enum AVCodecID ff_codec_get_id(const AVCodecTag *tags, unsigned int tag)
{
    int i;
    for(i=0; tags[i].id != AV_CODEC_ID_NONE;i++) {
        if(tag == tags[i].tag)
            return tags[i].id;
    }
    for(i=0; tags[i].id != AV_CODEC_ID_NONE; i++) {
        if (avpriv_toupper4(tag) == avpriv_toupper4(tags[i].tag))
            return tags[i].id;
    }
    return AV_CODEC_ID_NONE;
}

unsigned int av_codec_get_tag(const AVCodecTag * const *tags, enum AVCodecID id)
{
    int i;
    for(i=0; tags && tags[i]; i++){
        int tag= ff_codec_get_tag(tags[i], id);
        if(tag) return tag;
    }
    return 0;
}

enum AVCodecID av_codec_get_id(const AVCodecTag * const *tags, unsigned int tag)
{
    int i;
    for(i=0; tags && tags[i]; i++){
        enum AVCodecID id= ff_codec_get_id(tags[i], tag);
        if(id!=AV_CODEC_ID_NONE) return id;
    }
    return AV_CODEC_ID_NONE;
}

void ff_free_stream(AVFormatContext *s, AVStream *st){
    av_assert0(s->nb_streams>0);
    av_assert0(s->streams[ s->nb_streams-1 ] == st);

    av_freep(&st->codec->extradata);
    av_freep(&st->codec);
    av_freep(&st->priv_data);
    av_freep(&s->streams[ --s->nb_streams ]);
}

void avformat_free_context(AVFormatContext *s)
{
    int i;

    if (!s)
        return;

    for(i=s->nb_streams-1; i>=0; i--) {
        ff_free_stream(s, s->streams[i]);
    }
    av_freep(&s->streams);
    av_free(s);
}

void avformat_close_input(AVFormatContext **ps)
{
    AVFormatContext *s = *ps;

    av_freep(&s->pb);
    avformat_free_context(s);

    *ps = NULL;
}

AVStream *avformat_new_stream(AVFormatContext *s, const AVCodec *c)
{
    AVStream *st;
    AVStream **streams;

    if (s->nb_streams >= INT_MAX/sizeof(*streams))
        return NULL;
    streams = av_realloc(s->streams, (s->nb_streams + 1) * sizeof(*streams));
    if (!streams)
        return NULL;
    s->streams = streams;

    st = av_mallocz(sizeof(AVStream));
    if (!st)
        return NULL;

    st->codec = avcodec_alloc_context3(c);
    st->index = s->nb_streams;
    st->duration = AV_NOPTS_VALUE;
        /* we set the current DTS to 0 so that formats without any timestamps
           but durations get some timestamps, formats with some unknown
           timestamps have their first few packets buffered and the
           timestamps corrected before they are returned to the user */
    st->cur_dts = 0;    // s->iformat ? RELATIVE_TS_BASE : 0;

    /* default pts setting is MPEG-like */
    avpriv_set_pts_info(st, 33, 1, 90000);

#ifndef WIN32
    st->sample_aspect_ratio = (AVRational){0,1};
#else
    st->sample_aspect_ratio.num = 0;
    st->sample_aspect_ratio.den = 1;
#endif

    s->streams[s->nb_streams++] = st;
    return st;
}

void avpriv_set_pts_info(AVStream *s, int pts_wrap_bits,
                         unsigned int pts_num, unsigned int pts_den)
{
    AVRational new_tb;
    if(av_reduce(&new_tb.num, &new_tb.den, pts_num, pts_den, INT_MAX)){
        if(new_tb.num != pts_num)
            av_log(NULL, AV_LOG_DEBUG, "st:%d removing common factor %d from timebase\n", s->index, pts_num/new_tb.num);
    }else
        av_log(NULL, AV_LOG_WARNING, "st:%d has too large timebase, reducing\n", s->index);

    if(new_tb.num <= 0 || new_tb.den <= 0) {
        av_log(NULL, AV_LOG_ERROR, "Ignoring attempt to set invalid timebase %d/%d for st:%d\n", new_tb.num, new_tb.den, s->index);
        return;
    }
    s->time_base = new_tb;
}

const struct AVCodecTag *avformat_get_riff_video_tags(void)
{
    return ff_codec_bmp_tags;
}
const struct AVCodecTag *avformat_get_riff_audio_tags(void)
{
    return ff_codec_wav_tags;
}

unsigned int avpriv_toupper4(unsigned int x)
{
    return toupper(x & 0xFF)
           + (toupper((x >> 8) & 0xFF) << 8)
           + (toupper((x >> 16) & 0xFF) << 16)
           + (toupper((x >> 24) & 0xFF) << 24);
}
