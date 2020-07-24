/*
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
#include "avformat.h"
#include "avio_internal.h"
//#include "../libavutil/opt.h"

/**
 * @file
 * Options definition for AVFormatContext.
 */

static void avformat_get_context_defaults(AVFormatContext *s)
{
    memset(s, 0, sizeof(AVFormatContext));
}

AVFormatContext *avformat_alloc_context(void)
{
    AVFormatContext *ic;
    ic = av_malloc(sizeof(AVFormatContext));
    if (!ic) return ic;
    avformat_get_context_defaults(ic);
    return ic;
}

int avcodec_get_context_defaults3(AVCodecContext *s, const AVCodec *codec)
{
    memset(s, 0, sizeof(AVCodecContext));

    s->codec_type = codec ? codec->type : AVMEDIA_TYPE_UNKNOWN;

    return 0;
}

AVCodecContext *avcodec_alloc_context3(const AVCodec *codec)
{
    AVCodecContext *avctx = av_malloc(sizeof(AVCodecContext));

    if (avctx == NULL) return NULL;

    if (avcodec_get_context_defaults3(avctx, codec) < 0){
        av_free(avctx);
        return NULL;
    }

    return avctx;
}
