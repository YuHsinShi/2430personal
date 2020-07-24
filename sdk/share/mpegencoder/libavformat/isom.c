/*
 * ISO Media common code
 * Copyright (c) 2001 Fabrice Bellard
 * Copyright (c) 2002 Francois Revol <revol@free.fr>
 * Copyright (c) 2006 Baptiste Coudurier <baptiste.coudurier@free.fr>
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

//#define DEBUG

#include "avformat.h"
#include "internal.h"
#include "isom.h"
//#include "../libavcodec/mpeg4audio.h"
//#include "../libavcodec/mpegaudiodata.h"

/* http://www.mp4ra.org */
/* ordered by muxing preference */
const AVCodecTag ff_mp4_obj_type[] = {
    { AV_CODEC_ID_H264        , 0x21 },
    { AV_CODEC_ID_AAC         , 0x40 },
    { AV_CODEC_ID_AAC         , 0x66 }, /* MPEG2 AAC Main */
    { AV_CODEC_ID_AAC         , 0x67 }, /* MPEG2 AAC Low */
    { AV_CODEC_ID_AAC         , 0x68 }, /* MPEG2 AAC SSR */
    { AV_CODEC_ID_MP3         , 0x69 }, /* 13818-3 */
    { AV_CODEC_ID_MP2         , 0x69 }, /* 11172-3 */
    { AV_CODEC_ID_MP3         , 0x6B }, /* 11172-3 */
    { AV_CODEC_ID_AC3         , 0xA5 },
    { AV_CODEC_ID_MPEG4SYSTEMS, 0x01 },
    { AV_CODEC_ID_MPEG4SYSTEMS, 0x02 },
    { AV_CODEC_ID_NONE        ,    0 },
};

const AVCodecTag ff_codec_movvideo_tags[] = {
    { AV_CODEC_ID_H264, MKTAG('a', 'v', 'c', '1') }, /* AVC-1/H.264 */
    { AV_CODEC_ID_H264, MKTAG('a', 'i', '5', 'p') }, /* AVC-Intra  50M 720p24/30/60 */
    { AV_CODEC_ID_H264, MKTAG('a', 'i', '5', 'q') }, /* AVC-Intra  50M 720p25/50 */
    { AV_CODEC_ID_H264, MKTAG('a', 'i', '5', '2') }, /* AVC-Intra  50M 1080p25/50 */
    { AV_CODEC_ID_H264, MKTAG('a', 'i', '5', '3') }, /* AVC-Intra  50M 1080p24/30/60 */
    { AV_CODEC_ID_H264, MKTAG('a', 'i', '5', '5') }, /* AVC-Intra  50M 1080i50 */
    { AV_CODEC_ID_H264, MKTAG('a', 'i', '5', '6') }, /* AVC-Intra  50M 1080i60 */
    { AV_CODEC_ID_H264, MKTAG('a', 'i', '1', 'p') }, /* AVC-Intra 100M 720p24/30/60 */
    { AV_CODEC_ID_H264, MKTAG('a', 'i', '1', 'q') }, /* AVC-Intra 100M 720p25/50 */
    { AV_CODEC_ID_H264, MKTAG('a', 'i', '1', '2') }, /* AVC-Intra 100M 1080p25/50 */
    { AV_CODEC_ID_H264, MKTAG('a', 'i', '1', '3') }, /* AVC-Intra 100M 1080p24/30/60 */
    { AV_CODEC_ID_H264, MKTAG('a', 'i', '1', '5') }, /* AVC-Intra 100M 1080i50 */
    { AV_CODEC_ID_H264, MKTAG('a', 'i', '1', '6') }, /* AVC-Intra 100M 1080i60 */
    { AV_CODEC_ID_H264, MKTAG('A', 'V', 'i', 'n') }, /* AVC-Intra with implicit SPS/PPS */
    { AV_CODEC_ID_NONE, 0 },
};

const AVCodecTag ff_codec_movaudio_tags[] = {
    { AV_CODEC_ID_AAC,             MKTAG('m', 'p', '4', 'a') },
    { AV_CODEC_ID_AC3,             MKTAG('a', 'c', '-', '3') }, /* ETSI TS 102 366 Annex F */
    { AV_CODEC_ID_AC3,             MKTAG('s', 'a', 'c', '3') }, /* Nero Recode */
    { AV_CODEC_ID_MP2,             MKTAG('.', 'm', 'p', '2') },
    { AV_CODEC_ID_MP3,             MKTAG('.', 'm', 'p', '3') },
    { AV_CODEC_ID_MP3,             0x6D730055                },
    { AV_CODEC_ID_NONE, 0 },
};

/* map numeric codes from mdhd atom to ISO 639 */
/* cf. QTFileFormat.pdf p253, qtff.pdf p205 */
/* http://developer.apple.com/documentation/mac/Text/Text-368.html */
/* deprecated by putting the code as 3*5bit ascii */
static const char mov_mdhd_language_map[][4] = {
    /* 0-9 */
    "eng", "fra", "ger", "ita", "dut", "sve", "spa", "dan", "por", "nor",
    "heb", "jpn", "ara", "fin", "gre", "ice", "mlt", "tur", "hr "/*scr*/, "chi"/*ace?*/,
    "urd", "hin", "tha", "kor", "lit", "pol", "hun", "est", "lav",    "",
    "fo ",    "", "rus", "chi",    "", "iri", "alb", "ron", "ces", "slk",
    "slv", "yid", "sr ", "mac", "bul", "ukr", "bel", "uzb", "kaz", "aze",
    /*?*/
    "aze", "arm", "geo", "mol", "kir", "tgk", "tuk", "mon",    "", "pus",
    "kur", "kas", "snd", "tib", "nep", "san", "mar", "ben", "asm", "guj",
    "pa ", "ori", "mal", "kan", "tam", "tel",    "", "bur", "khm", "lao",
    /*                   roman? arabic? */
    "vie", "ind", "tgl", "may", "may", "amh", "tir", "orm", "som", "swa",
    /*==rundi?*/
       "", "run",    "", "mlg", "epo",    "",    "",    "",    "",    "",
    /* 100 */
       "",    "",    "",    "",    "",    "",    "",    "",    "",    "",
       "",    "",    "",    "",    "",    "",    "",    "",    "",    "",
       "",    "",    "",    "",    "",    "",    "",    "", "wel", "baq",
    "cat", "lat", "que", "grn", "aym", "tat", "uig", "dzo", "jav"
};

int ff_mov_iso639_to_lang(const char lang[4], int mp4)
{
    int i, code = 0;

    /* old way, only for QT? */
    for (i = 0; lang[0] && !mp4 && i < FF_ARRAY_ELEMS(mov_mdhd_language_map); i++) {
        if (!strcmp(lang, mov_mdhd_language_map[i]))
            return i;
    }
    /* XXX:can we do that in mov too? */
    if (!mp4)
        return -1;
    /* handle undefined as such */
    if (lang[0] == '\0')
        lang = "und";
    /* 5bit ascii */
    for (i = 0; i < 3; i++) {
        uint8_t c = lang[i];
        c -= 0x60;
        if (c > 0x1f)
            return -1;
        code <<= 5;
        code |= c;
    }
    return code;
}
