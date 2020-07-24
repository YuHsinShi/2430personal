/*
 * RIFF common functions and data
 * Copyright (c) 2000 Fabrice Bellard
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

#include "../libavutil/mathematics.h"
#include "../libavcodec/avcodec.h"
#include "avformat.h"
#include "avio_internal.h"
#include "riff.h"
//#include "libavcodec/bytestream.h"
//#include "libavutil/avassert.h"

/* Note: when encoding, the first matching tag is used, so order is
   important if multiple tags possible for a given codec.
   Note also that this list is used for more than just riff, other
   files use it as well.
*/
const AVCodecTag ff_codec_bmp_tags[] = {
    { AV_CODEC_ID_H264,         MKTAG('H', '2', '6', '4') },
    { AV_CODEC_ID_H264,         MKTAG('h', '2', '6', '4') },
    { AV_CODEC_ID_H264,         MKTAG('X', '2', '6', '4') },
    { AV_CODEC_ID_H264,         MKTAG('x', '2', '6', '4') },
    { AV_CODEC_ID_H264,         MKTAG('a', 'v', 'c', '1') },
    { AV_CODEC_ID_H264,         MKTAG('D', 'A', 'V', 'C') },
    { AV_CODEC_ID_H264,         MKTAG('S', 'M', 'V', '2') },
    { AV_CODEC_ID_H264,         MKTAG('V', 'S', 'S', 'H') },
    { AV_CODEC_ID_H264,         MKTAG('Q', '2', '6', '4') }, /* QNAP surveillance system */
    { AV_CODEC_ID_H264,         MKTAG('V', '2', '6', '4') },
    { AV_CODEC_ID_NONE,         0 }
};

const AVCodecTag ff_codec_wav_tags[] = {
    { AV_CODEC_ID_MP2,             0x0050 },
    { AV_CODEC_ID_MP3,             0x0055 },
    { AV_CODEC_ID_AAC,             0x00ff },
    { AV_CODEC_ID_AAC,             0x1600 }, /* ADTS AAC */
    { AV_CODEC_ID_AAC_LATM,        0x1602 },
    { AV_CODEC_ID_AC3,             0x2000 },
    { AV_CODEC_ID_AAC,             0x706d },
    { AV_CODEC_ID_AAC,             0x4143 },
    { AV_CODEC_ID_AAC,             0xA106 },
    { AV_CODEC_ID_NONE,      0 },
};
