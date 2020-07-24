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

#include "../config.h"

//#include <stdint.h>
//#include "mp_msg.h"
#include "mp_taglists.h"
#include "../libavutil/common.h"
#include "../libavformat/avformat.h"
// for AVCodecTag
#include "../libavformat/internal.h"

static const struct AVCodecTag mp_wav_tags[] = {
    { CODEC_ID_MP1,               0x50},
    { 0, 0 },
};

static const struct AVCodecTag mp_codecid_override_tags[] = {
    { CODEC_ID_AAC,               MKTAG('M', 'P', '4', 'A')},
    { CODEC_ID_AAC_LATM,          MKTAG('M', 'P', '4', 'L')},
    { CODEC_ID_H264,              MKTAG('H', '2', '6', '4')},
    { CODEC_ID_MP3,               0x55},
    { CODEC_ID_MP2,               0x50},
    { 0, 0 },
};

static const struct AVCodecTag * const mp_codecid_override_taglists[] =
                        {mp_codecid_override_tags, 0};

static const struct AVCodecTag mp_bmp_tags[] = {
    { 0, 0 },
};

static void get_taglists(const struct AVCodecTag *dst[3], int audio)
{
    dst[0] = audio ? mp_wav_tags : mp_bmp_tags;
    dst[1] = audio ? avformat_get_riff_audio_tags() : avformat_get_riff_video_tags();
    dst[2] = NULL;
}

enum CodecID mp_tag2codec_id(uint32_t tag, int audio)
{
    const struct AVCodecTag *taglists[3];
    get_taglists(taglists, audio);
    return av_codec_get_id(taglists, tag);
}
