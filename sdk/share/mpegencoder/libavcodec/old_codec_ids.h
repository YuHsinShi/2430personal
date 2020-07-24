/*
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

#ifndef AVCODEC_OLD_CODEC_IDS_H
#define AVCODEC_OLD_CODEC_IDS_H

#include "../libavutil/common.h"

/*
 * This header exists to prevent new codec IDs from being accidentally added to
 * the deprecated list.
 * Do not include it directly. It will be removed on next major bump
 *
 * Do not add new items to this list. Use the AVCodecID enum instead.
 */

    CODEC_ID_NONE = AV_CODEC_ID_NONE,

    /* video codecs */
    CODEC_ID_RAWVIDEO,
    CODEC_ID_H264,

    /* audio codecs */
    CODEC_ID_MP2 = 0x15000,
    CODEC_ID_MP3, ///< preferred ID for decoding MPEG audio layer 1, 2 or 3
    CODEC_ID_AAC,
    CODEC_ID_MP3ADU,
    CODEC_ID_MP3ON4,
    CODEC_ID_MP1,
    CODEC_ID_AAC_LATM,

    /* other specific kind of codecs (generally used for attachments) */
    CODEC_ID_FIRST_UNKNOWN = 0x18000,           ///< A dummy ID pointing at the start of various fake codecs.

    CODEC_ID_PROBE = 0x19000, ///< codec_id is not known (like CODEC_ID_NONE) but lavf should attempt to identify it

    CODEC_ID_MPEG2TS = 0x20000, /**< _FAKE_ codec to indicate a raw MPEG-2 TS
                                * stream (only used by libavformat) */
    CODEC_ID_MPEG4SYSTEMS = 0x20001, /**< _FAKE_ codec to indicate a MPEG-4 Systems
                                * stream (only used by libavformat) */
    CODEC_ID_FFMETADATA = 0x21000,   ///< Dummy codec for streams containing only metadata information.

#endif /* AVCODEC_OLD_CODEC_IDS_H */
