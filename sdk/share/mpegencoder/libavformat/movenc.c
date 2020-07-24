/*
 * MOV, 3GP, MP4 muxer
 * Copyright (c) 2003 Thomas Raivio
 * Copyright (c) 2004 Gildas Bazin <gbazin at videolan dot org>
 * Copyright (c) 2009 Baptiste Coudurier <baptiste dot coudurier at gmail dot com>
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

#include "mencoder.h"
#include "movenc.h"
#include "avformat.h"
#include "avio_internal.h"
#include "riff.h"
#include "avio.h"
#include "isom.h"
#include "avc.h"
//#include "../libavcodec/get_bits.h"
//#include "libavcodec/put_bits.h"
//#include "libavcodec/vc1.h"
//#include "internal.h"
#include "../libavutil/avstring.h"
#include "../libavutil/bswap.h"
//#include "libavutil/intfloat.h"
#include "../libavutil/mathematics.h"
//#include "../libavutil/opt.h"
//#include "libavutil/dict.h"
//#include "rtpenc.h"
//#include "mov_chan.h"
//
//#undef NDEBUG
//#include <assert.h>
//#define TEST

// do not change the order
enum
{
    CACHE_FILE_VIDEO_STTS = 0,
    CACHE_FILE_VIDEO_STSZ,
    CACHE_FILE_VIDEO_STCO,
    CACHE_FILE_VIDEO_STSC,
    CACHE_FILE_VIDEO_STSS,
    CACHE_FILE_AUDIO_STTS,
    CACHE_FILE_AUDIO_STSZ,
    CACHE_FILE_AUDIO_STCO,
    CACHE_FILE_AUDIO_STSC,
    CACHE_FILE_COUNT
};

#define CACHE_FILE_EXIST(f)     ((f) && (*(f)))
#define ENTRY(track, table_id)  (((track)->entry[(table_id)]) + ((track)->entry_count[(table_id)]) * g_single_entry_size[(table_id)])
#define WRITE_ENTRY(pb, track, table_id)                                            \
{                                                                                   \
    write_entry((pb),                                                               \
                (track)->c_cache[(table_id)],                                       \
                g_single_entry_size[(table_id)] * (track)->entry_count[(table_id)], \
                &((track)->entry[(table_id)]));                                     \
}

static const int g_single_entry_size[MOV_TABLE_COUNT] =
{
    sizeof(MOVStts),
    sizeof(MOVStsz),
    sizeof(MOVStsc),
    sizeof(MOVStco),
    sizeof(MOVStss)
};

static void
generate_file_name(
    MMP_CHAR* out_filename,
    MMP_CHAR* url,
    int index);

static void
write_entry(
    AVIOContext* pb,
    cache_t** ppc,
    uint32_t size,
    unsigned char** ppentry_data)
{
    if (ppc && (*ppc))
    {
        cache_flush(*ppc, MMP_TRUE);
        cache_dump_to_avio_write(*ppc, avio_write, pb);
        free_cache(*ppc);
        *ppc = MMP_NULL;
    }
    else
    {
        avio_write(pb, *ppentry_data, size);

        av_free(*ppentry_data);
        *ppentry_data = MMP_NULL;
    }
}

static void
free_entry(
    cache_t** ppc,
    unsigned char** ppentry_data)
{
    if (ppc && (*ppc))
    {
        free_cache(*ppc);
        *ppc = MMP_NULL;
    }
    if (ppentry_data && (*ppentry_data))
    {
        av_free(*ppentry_data);
        *ppentry_data = MMP_NULL;
    }
}

//FIXME support 64 bit variant with wide placeholders
static int64_t update_size(AVIOContext *pb, int64_t pos)
{
    int64_t curpos = avio_tell(pb);
    avio_seek(pb, pos, SEEK_SET);
    avio_wb32(pb, curpos - pos); /* rewrite size */
    avio_seek(pb, curpos, SEEK_SET);

    return curpos - pos;
}

//static int is_co64_required(const MOVTrack *track)
//{
//    int i;
//
//    for (i = 0; i < track->sample_count; i++) {
//        if (!track->cluster[i].chunkNum)
//            continue;
//        if (track->cluster[i].pos + track->data_offset > UINT32_MAX)
//            return 1;
//    }
//    return 0;
//}

/* Chunk offset atom */
static int mov_write_stco_tag(AVIOContext *pb, MOVTrack *track)
{
    int mode64 = (g_single_entry_size[MOV_STCO] == sizeof(int64_t));
#ifdef TEST
    //int i;
    //int mode64 = 1;//is_co64_required(track); // use 32 bit size variant if possible
    int64_t pos = avio_tell(pb);

    avio_wb32(pb, 0); /* size */
    if (mode64)
        ffio_wfourcc(pb, "co64");
    else
        ffio_wfourcc(pb, "stco");

    avio_wb32(pb, 0); /* version & flags */
    avio_wb32(pb, track->entry_count[MOV_STCO]); /* entry count */
#else
    int32_t size = 16 + g_single_entry_size[MOV_STCO] * track->entry_count[MOV_STCO];
    if (!mode64)
    {
        uint8_t data[] = {
            (size >> 24),       /* size */
            (size >> 16) & 0xFF,
            (size >>  8) & 0xFF,
            (size      ) & 0xFF,
            's', 't', 'c', 'o',
            0,   0,   0,   0,   /* version & flags */
            (track->entry_count[MOV_STCO] >> 24), /* entry count */
            (track->entry_count[MOV_STCO] >> 16) & 0xFF,
            (track->entry_count[MOV_STCO] >>  8) & 0xFF,
            (track->entry_count[MOV_STCO]      ) & 0xFF};
        avio_write(pb, data, sizeof(data));
    }
    else
    {
        uint8_t data[] = {
            (size >> 24),       /* size */
            (size >> 16) & 0xFF,
            (size >>  8) & 0xFF,
            (size      ) & 0xFF,
            'c', 'o', '6', '4',
            0,   0,   0,   0,   /* version & flags */
            (track->entry_count[MOV_STCO] >> 24), /* entry count */
            (track->entry_count[MOV_STCO] >> 16) & 0xFF,
            (track->entry_count[MOV_STCO] >>  8) & 0xFF,
            (track->entry_count[MOV_STCO]      ) & 0xFF};
        avio_write(pb, data, sizeof(data));
    }
#endif
    WRITE_ENTRY(pb, track, MOV_STCO);
    track->stco_chunk_pos   =
    track->stco_chunk_size  =
    track->entry_count[MOV_STCO] = 0;

#ifdef TEST
    return update_size(pb, pos);
#else
    return size;
#endif
}

/* Sample size atom */
static int mov_write_stsz_tag(AVIOContext *pb, MOVTrack *track)
{
#ifdef TEST
    int64_t pos = avio_tell(pb);
    avio_wb32(pb, 0); /* size */
    ffio_wfourcc(pb, "stsz");
    avio_wb32(pb, 0); /* version & flags */

    if (track->stsz_sample_size && track->sample_count)
    {
        avio_wb32(pb, track->stsz_sample_size); // sample size
        avio_wb32(pb, track->sample_count);            // sample count
    }
    else
    {
        avio_wb32(pb, 0);                       // sample size
        avio_wb32(pb, track->sample_count);            // sample count
        if (track->entry_count[MOV_STSZ])
        {
            WRITE_ENTRY(pb, track, MOV_STSZ);
        }
    }
#else
    int32_t size = (track->stsz_sample_size && track->sample_count)
                 ? (12 + 8)
                 : (12 + 8 + sizeof(int) * track->sample_count);
    uint8_t data[] = {
        (size >> 24),       /* size */
        (size >> 16) & 0xFF,
        (size >>  8) & 0xFF,
        (size      ) & 0xFF,
                      's', 't', 's', 'z',
                      0, 0, 0, 0};          /* version & flags */
    avio_write(pb, data, sizeof(data));

    if (track->stsz_sample_size && track->sample_count)
    {
        uint8_t data[] = {
            (track->stsz_sample_size >> 24),         // sample size
            (track->stsz_sample_size >> 16) & 0xFF,
            (track->stsz_sample_size >>  8) & 0xFF,
            (track->stsz_sample_size      ) & 0xFF,
            (track->sample_count     >> 24),         // sample count
            (track->sample_count     >> 16) & 0xFF,
            (track->sample_count     >>  8) & 0xFF,
            (track->sample_count          ) & 0xFF};
        avio_write(pb, data, sizeof(data));
    }
    else
    {
        uint8_t data[] = {
            0, 0, 0, 0,                              // sample size
            (track->sample_count     >> 24),         // sample count
            (track->sample_count     >> 16) & 0xFF,
            (track->sample_count     >>  8) & 0xFF,
            (track->sample_count          ) & 0xFF};
        avio_write(pb, data, sizeof(data));
        if (track->entry_count[MOV_STSZ])
        {
            WRITE_ENTRY(pb, track, MOV_STSZ);
        }
    }
#endif

    free_entry(track->c_cache[MOV_STSZ], &track->entry[MOV_STSZ]);
    track->entry_count[MOV_STSZ] = 0;
#ifdef TEST
    return update_size(pb, pos);
#else
    return size;
#endif
}

/* Sample to chunk atom */
static int mov_write_stsc_tag(AVIOContext *pb, MOVTrack *track)
{
#ifdef TEST
    int64_t pos = avio_tell(pb);

    avio_wb32(pb, 0); /* size */
    ffio_wfourcc(pb, "stsc");
    avio_wb32(pb, 0); // version & flags
    avio_wb32(pb, track->entry_count[MOV_STSC]); // entry count
#else
    int32_t size = 16 + g_single_entry_size[MOV_STSC] * track->entry_count[MOV_STSC];
    uint8_t data[] = {
        (size >> 24),                               /* size */
        (size >> 16) & 0xFF,
        (size >>  8) & 0xFF,
        (size      ) & 0xFF,
        's', 't', 's', 'c',
        0, 0, 0, 0,
        (track->entry_count[MOV_STSC] >> 24),            // sample size
        (track->entry_count[MOV_STSC] >> 16) & 0xFF,
        (track->entry_count[MOV_STSC] >>  8) & 0xFF,
        (track->entry_count[MOV_STSC]      ) & 0xFF};    // version & flags
    avio_write(pb, data, sizeof(data));
#endif
    WRITE_ENTRY(pb, track, MOV_STSC);

    track->entry_count[MOV_STSC] = 0;
#ifdef TEST
    return update_size(pb, pos);
#else
    return size;
#endif
}

/* Sync sample atom */
static int mov_write_stss_tag(AVIOContext *pb, MOVTrack *track, uint32_t flag)
{
#ifdef TEST
    int64_t pos = avio_tell(pb);

    avio_wb32(pb, 0); // size
    ffio_wfourcc(pb, flag == MOV_SYNC_SAMPLE ? "stss" : "stps");
    avio_wb32(pb, 0); // version & flags
    avio_wb32(pb, track->entry_count[MOV_STSS]); // entry count
#else
    int32_t size = 16 + sizeof(int) * track->entry_count[MOV_STSS];
    if (flag == MOV_SYNC_SAMPLE)
    {
        uint8_t data[] = {
            (size >> 24),                               /* size */
            (size >> 16) & 0xFF,
            (size >>  8) & 0xFF,
            (size      ) & 0xFF,
            's', 't', 's', 's',
            0, 0, 0, 0,
            (track->entry_count[MOV_STSS] >> 24),            // sample size
            (track->entry_count[MOV_STSS] >> 16) & 0xFF,
            (track->entry_count[MOV_STSS] >>  8) & 0xFF,
            (track->entry_count[MOV_STSS]      ) & 0xFF};    // version & flags
        avio_write(pb, data, sizeof(data));
    }
    else
    {
        uint8_t data[] = {
            (size >> 24),                               /* size */
            (size >> 16) & 0xFF,
            (size >>  8) & 0xFF,
            (size      ) & 0xFF,
            's', 't', 'p', 's',
            0, 0, 0, 0,
            (track->entry_count[MOV_STSS] >> 24),            // sample size
            (track->entry_count[MOV_STSS] >> 16) & 0xFF,
            (track->entry_count[MOV_STSS] >>  8) & 0xFF,
            (track->entry_count[MOV_STSS]      ) & 0xFF};    // version & flags
        avio_write(pb, data, sizeof(data));
    }
#endif
    WRITE_ENTRY(pb, track, MOV_STSS);
    track->entry_count[MOV_STSS] = 0;
#ifdef TEST
    return update_size(pb, pos);
#else
    return size;
#endif
}

static void put_descr(AVIOContext *pb, int tag, unsigned int size)
{
    int i = 3;
    avio_w8(pb, tag);
    for(; i>0; i--)
        avio_w8(pb, (size>>(7*i)) | 0x80);
    avio_w8(pb, size & 0x7F);
}

static unsigned int compute_avg_bitrate(MOVTrack *track)
{
    if (!track->track_duration)
        return 0;
    return (unsigned int)(track->mdat_size * 8 * track->timescale / track->track_duration);
}

static int mov_write_esds_tag(AVIOContext *pb, MOVTrack *track) // Basic
{
    int64_t pos = avio_tell(pb);
    int decoder_specific_info_len = track->vos_len ? 5 + track->vos_len : 0;
    unsigned avg_bitrate;

#ifdef TEST
    avio_wb32(pb, 0); // size
    ffio_wfourcc(pb, "esds");
    avio_wb32(pb, 0); // Version
#else
    {
        uint8_t data[] = {0, 0, 0, 0, 'e', 's', 'd', 's', 0, 0, 0, 0};
        avio_write(pb, data, sizeof(data));
    }
#endif

    // ES descriptor
    put_descr(pb, 0x03, 3 + 5+13 + decoder_specific_info_len + 5+1);
    avio_wb16(pb, track->track_id);
    avio_w8(pb, 0x00); // flags (= no flags)

    // DecoderConfig descriptor
    put_descr(pb, 0x04, 13 + decoder_specific_info_len);

    // Object type indication
    if ((track->enc->codec_id == AV_CODEC_ID_MP2 ||
         track->enc->codec_id == AV_CODEC_ID_MP3) &&
        track->enc->sample_rate > 24000)
        avio_w8(pb, 0x6B); // 11172-3
    else
        avio_w8(pb, ff_codec_get_tag(ff_mp4_obj_type, track->enc->codec_id));

    // the following fields is made of 6 bits to identify the streamtype (4 for video, 5 for audio)
    // plus 1 bit to indicate upstream and 1 bit set to 1 (reserved)
    if (track->enc->codec_type == AVMEDIA_TYPE_AUDIO)
        avio_w8(pb, 0x15); // flags (= Audiostream)
    else
        avio_w8(pb, 0x11); // flags (= Visualstream)

    avio_w8(pb,  track->enc->rc_buffer_size>>(3+16));      // Buffersize DB (24 bits)
    avio_wb16(pb, (track->enc->rc_buffer_size>>3)&0xFFFF); // Buffersize DB

    avg_bitrate = compute_avg_bitrate(track);
    // maxbitrate (FIXME should be max rate in any 1 sec window)
    avio_wb32(pb, FFMAX3(track->enc->bit_rate, track->enc->rc_max_rate, avg_bitrate));
    avio_wb32(pb, avg_bitrate);

    if (track->vos_len) {
        // DecoderSpecific info descriptor
        put_descr(pb, 0x05, track->vos_len);
        avio_write(pb, track->vos_data, track->vos_len);
    }

    // SL descriptor
    put_descr(pb, 0x06, 1);
    avio_w8(pb, 0x02);
    return update_size(pb, pos);
}

static int mov_write_wave_tag(AVIOContext *pb, MOVTrack *track)
{
    int64_t pos = avio_tell(pb);

#ifdef TEST
    avio_wb32(pb, 0);     /* size */
    ffio_wfourcc(pb, "wave");

    avio_wb32(pb, 12);    /* size */
    ffio_wfourcc(pb, "frma");
    avio_wl32(pb, track->tag);
#else
    uint8_t data[] = {0, 0, 0,  0, 'w', 'a', 'v', 'e',
                      0, 0, 0, 12, 'f', 'r', 'm', 'a',
                      (track->tag      ) & 0xFF,
                      (track->tag >>  8) & 0xFF,
                      (track->tag >> 16) & 0xFF,
                      (track->tag >> 24)};
    avio_write(pb, data, sizeof(data));
#endif

    if (track->enc->codec_id == AV_CODEC_ID_AAC) {
        /* useless atom needed by mplayer, ipod, not needed by quicktime */
#ifdef TEST
        avio_wb32(pb, 12); /* size */
        ffio_wfourcc(pb, "mp4a");
        avio_wb32(pb, 0);
#else
        uint8_t data[] = {0, 0, 0, 12, 'm', 'p', '4', 'a', 0, 0, 0, 0};
        avio_write(pb, data, sizeof(data));
#endif
        mov_write_esds_tag(pb, track);
    }

#ifdef TEST
    avio_wb32(pb, 8);     /* size */
    avio_wb32(pb, 0);     /* null tag */
#else
    {
        uint8_t data[] = {0, 0, 0, 8, 0, 0, 0, 0};
        avio_write(pb, data, sizeof(data));
    }
#endif

    return update_size(pb, pos);
}

static int mov_write_audio_tag(AVIOContext *pb, MOVTrack *track)
{
    int64_t pos = avio_tell(pb);
    int version = 0;
    uint32_t tag = track->tag;

    if (track->mode == MODE_MOV) {
        version = 1;
    }

#ifdef TEST
    avio_wb32(pb, 0); /* size */
    avio_wl32(pb, tag); // store it byteswapped
    avio_wb32(pb, 0); /* Reserved */
    avio_wb16(pb, 0); /* Reserved */
    avio_wb16(pb, 1); /* Data-reference index, XXX  == 1 */

    /* SoundDescription */
    avio_wb16(pb, version); /* Version */
    avio_wb16(pb, 0); /* Revision level */
    avio_wb32(pb, 0); /* Reserved */
#else
    {
        uint8_t data[] = {
            0, 0, 0, 0,         /* size */
            (tag      ) & 0xFF, // store it byteswapped
            (tag >>  8) & 0xFF,
            (tag >> 16) & 0xFF,
            (tag >> 24),
            0, 0, 0, 0,         /* Reserved */
            0, 0,               /* Reserved */
            0, 1,               /* Data-reference index, XXX  == 1 */
            /* SoundDescription */
            (version >> 16) & 0xFF, /* Version */
            version         & 0xFF,
            0, 0,                   /* Revision level */
            0, 0, 0, 0};            /* Reserved */
        avio_write(pb, data, sizeof(data));
    }
#endif

    if (track->mode == MODE_MOV) {
        avio_wb16(pb, track->enc->channels);
        avio_wb16(pb, 16);
        avio_wb16(pb, -2); /* compression ID */
    } else { /* reserved for mp4/3gp */
        avio_wb16(pb, 2);
        avio_wb16(pb, 16);
        avio_wb16(pb, 0);
    }

    avio_wb16(pb, 0); /* packet size (= 0) */
    avio_wb16(pb, track->enc->sample_rate <= UINT16_MAX ?
                  track->enc->sample_rate : 0);
    avio_wb16(pb, 0); /* Reserved */

    if(version == 1) { /* SoundDescription V1 extended info */
        avio_wb32(pb, track->enc->frame_size); /* Samples per packet */
        avio_wb32(pb, track->sample_size / track->enc->channels); /* Bytes per packet */
        avio_wb32(pb, track->sample_size); /* Bytes per frame */
        avio_wb32(pb, 2); /* Bytes per sample */
    }

    if(track->mode == MODE_MOV &&
       (track->enc->codec_id == AV_CODEC_ID_AAC))
        mov_write_wave_tag(pb, track);
    else if(track->tag == MKTAG('m','p','4','a'))
        mov_write_esds_tag(pb, track);

    return update_size(pb, pos);
}

int ff_isom_write_avcc(AVIOContext *pb, const uint8_t *data, int len);

static int mov_write_avcc_tag(AVIOContext *pb, MOVTrack *track)
{
#ifdef TEST
    int64_t pos = avio_tell(pb);

    avio_wb32(pb, 0);
    ffio_wfourcc(pb, "avcC");
    ff_isom_write_avcc(pb, track->vos_data, track->vos_len);
    return update_size(pb, pos);
#else
    int64_t pos = avio_tell(pb);
    uint8_t data[] = {
          0,   0,   0,   8,     // size
        'a', 'v', 'c', 'C'};
    avio_write(pb, data, sizeof(data));
    ff_isom_write_avcc(pb, track->vos_data, track->vos_len);

    return update_size(pb, pos);
#endif
}

static int mp4_get_codec_tag(AVFormatContext *s, MOVTrack *track)
{
    int tag = track->enc->codec_tag;

    if (!ff_codec_get_tag(ff_mp4_obj_type, track->enc->codec_id))
        return 0;

    if      (track->enc->codec_id == AV_CODEC_ID_H264)      tag = MKTAG('a','v','c','1');
    else if (track->enc->codec_id == AV_CODEC_ID_AC3)       tag = MKTAG('a','c','-','3');
    else if (track->enc->codec_type == AVMEDIA_TYPE_VIDEO) tag = MKTAG('m','p','4','v');
    else if (track->enc->codec_type == AVMEDIA_TYPE_AUDIO) tag = MKTAG('m','p','4','a');

    return tag;
}

static int mov_find_codec_tag(AVFormatContext *s, MOVTrack *track)
{
    return mp4_get_codec_tag(s, track);
}

static int mov_write_pasp_tag(AVIOContext *pb, MOVTrack *track)
{
    AVRational sar;
    av_reduce(&sar.num, &sar.den, track->enc->sample_aspect_ratio.num,
              track->enc->sample_aspect_ratio.den, INT_MAX);

    avio_wb32(pb, 16);
    ffio_wfourcc(pb, "pasp");
    avio_wb32(pb, sar.num);
    avio_wb32(pb, sar.den);
    return 16;
}

static int mov_write_video_tag(AVIOContext *pb, MOVTrack *track)
{
    int64_t pos = avio_tell(pb);
    char compressor_name[32] = { 0 };

    avio_wb32(pb, 0); /* size */
    avio_wl32(pb, track->tag); // store it byteswapped
#ifdef TEST
    avio_wb32(pb, 0); /* Reserved */
    avio_wb16(pb, 0); /* Reserved */
    avio_wb16(pb, 1); /* Data-reference index */

    avio_wb16(pb, 0); /* Codec stream version */
    avio_wb16(pb, 0); /* Codec stream revision (=0) */

    avio_wb32(pb, 0); /* Reserved */
    avio_wb32(pb, 0); /* Reserved */
    avio_wb32(pb, 0); /* Reserved */
#else
    {
        uint8_t data[] = {0, 0, 0, 0,   /* Reserved */
                          0, 0,         /* Reserved */
                          0, 1,         /* Data-reference index */
                          0, 0,         /* Codec stream version */
                          0, 0,         /* Codec stream revision (=0) */
                          0, 0, 0, 0,   /* Reserved */
                          0, 0, 0, 0,   /* Reserved */
                          0, 0, 0, 0};  /* Reserved */
        avio_write(pb, data, sizeof(data));
    }
#endif

    avio_wb16(pb, track->enc->width); /* Video width */
    avio_wb16(pb, track->height); /* Video height */
#ifdef TEST
    avio_wb32(pb, 0x00480000); /* Horizontal resolution 72dpi */
    avio_wb32(pb, 0x00480000); /* Vertical resolution 72dpi */
    avio_wb32(pb, 0); /* Data size (= 0) */
    avio_wb16(pb, 1); /* Frame count (= 1) */

    /* FIXME not sure, ISO 14496-1 draft where it shall be set to 0 */
    avio_w8(pb, strlen(compressor_name));
    avio_write(pb, compressor_name, 31);

    avio_wb16(pb, 0x18); /* Reserved */
    avio_wb16(pb, 0xffff); /* Reserved */
#else
    {
        uint8_t data[] = {0x00, 0x48, 0x00, 0x00,   /* Horizontal resolution 72dpi */
                          0x00, 0x48, 0x00, 0x00,   /* Vertical resolution 72dpi */
                          0x00, 0x00, 0x00, 0x00,   /* Data size (= 0) */
                          0x00, 0x01,               /* Frame count (= 1) */
                          0x00,                     /* strlen(compressor_name)) */
                          0x00, 0x00, 0x00, 0x00,   /* compressor_name */
                          0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00,
                          0x00, 0x18,               /* Reserved */
                          0xFF, 0xFF};              /* Reserved */
        avio_write(pb, data, sizeof(data));
    }
#endif
    if (track->tag == MKTAG('m','p','4','v'))
        mov_write_esds_tag(pb, track);
    else if(track->enc->codec_id == AV_CODEC_ID_H264) {
        mov_write_avcc_tag(pb, track);
    }

    if (track->enc->sample_aspect_ratio.den && track->enc->sample_aspect_ratio.num &&
        track->enc->sample_aspect_ratio.den != track->enc->sample_aspect_ratio.num) {
        mov_write_pasp_tag(pb, track);
    }
    return update_size(pb, pos);
}

static int mov_write_stsd_tag(AVIOContext *pb, MOVTrack *track)
{
    int64_t pos = avio_tell(pb);
#ifdef TEST
    avio_wb32(pb, 0); /* size */
    ffio_wfourcc(pb, "stsd");
    avio_wb32(pb, 0); /* version & flags */
    avio_wb32(pb, 1); /* entry count */
#else
    {
        uint8_t data[] = {  0,   0,   0,   0,   /* size */
                          's', 't', 's', 'd',
                            0,   0,   0,   0,   /* version & flags */
                            0,   0,   0,   1};  /* entry count */
        avio_write(pb, data, sizeof(data));
    }
#endif
    if (track->enc->codec_type == AVMEDIA_TYPE_VIDEO)
        mov_write_video_tag(pb, track);
    else if (track->enc->codec_type == AVMEDIA_TYPE_AUDIO)
        mov_write_audio_tag(pb, track);
    return update_size(pb, pos);
}

/* Time to sample atom */
static int mov_write_stts_tag(AVIOContext *pb, MOVTrack *track)
{
    uint32_t entries;
    uint32_t atom_size;

    if (track->sample_count >= 1)
    {
        int last_sample_duration = track->track_duration + track->start_dts
                                 - track->cluster[1].dts;
        if (last_sample_duration != track->stts_entry_duration)
        {
            if (track->stts_entry_sample_count > 0)
            {
                if (CACHE_FILE_EXIST(track->c_cache[MOV_STTS]))
                {
                    track->c_stts_entry.count    = av_ne2be32(track->stts_entry_sample_count);
                    track->c_stts_entry.duration = av_ne2be32(track->stts_entry_duration);
                    cache_write(*track->c_cache[MOV_STTS], (unsigned char*)&track->c_stts_entry, sizeof(track->c_stts_entry));
                }
                else
                {
                    MOVStts* ptEntry = (MOVStts*)ENTRY(track, MOV_STTS);
                    ptEntry->count    = av_ne2be32(track->stts_entry_sample_count);
                    ptEntry->duration = av_ne2be32(track->stts_entry_duration);
                }

                track->stts_entry_sample_count = 0;
                track->entry_count[MOV_STTS]++;
            }
            track->stts_entry_duration = last_sample_duration;
        }
        track->stts_entry_sample_count++;
    }
    if (track->stts_entry_sample_count > 0)
    {
        if (CACHE_FILE_EXIST(track->c_cache[MOV_STTS]))
        {
            track->c_stts_entry.count    = av_ne2be32(track->stts_entry_sample_count);
            track->c_stts_entry.duration = av_ne2be32(track->stts_entry_duration);
            cache_write(*track->c_cache[MOV_STTS], (unsigned char*)&track->c_stts_entry, sizeof(track->c_stts_entry));
        }
        else
        {
            MOVStts* ptEntry = (MOVStts*)ENTRY(track, MOV_STTS);
            ptEntry->count    = av_ne2be32(track->stts_entry_sample_count);
            ptEntry->duration = av_ne2be32(track->stts_entry_duration);
        }
        track->stts_entry_sample_count = 0;
        track->entry_count[MOV_STTS]++;
    }
    entries = track->entry_count[MOV_STTS];

    atom_size = 16 + (entries * 8);
#ifdef TEST
    avio_wb32(pb, atom_size); /* size */
    ffio_wfourcc(pb, "stts");
    avio_wb32(pb, 0); /* version & flags */
    avio_wb32(pb, entries); /* entry count */
#else
    {
        uint8_t data[] = {
            (atom_size >> 24),          /* size */
            (atom_size >> 16) & 0xFF,
            (atom_size >>  8) & 0xFF,
            (atom_size      ) & 0xFF,
            's', 't', 't', 's',
              0,   0,   0,   0,         /* version & flags */
            (entries >> 24),            /* entry count */
            (entries >> 16) & 0xFF,
            (entries >>  8) & 0xFF,
            (entries      ) & 0xFF};
        avio_write(pb, data, sizeof(data));
    }
#endif
    WRITE_ENTRY(pb, track, MOV_STTS);
    track->entry_count[MOV_STTS] = 0;
    return atom_size;
}

static int mov_write_dref_tag(AVIOContext *pb)
{
#ifdef TEST
    avio_wb32(pb, 28); /* size */
    ffio_wfourcc(pb, "dref");
    avio_wb32(pb, 0); /* version & flags */
    avio_wb32(pb, 1); /* entry count */

    avio_wb32(pb, 0xc); /* size */
    //FIXME add the alis and rsrc atom
    ffio_wfourcc(pb, "url ");
    avio_wb32(pb, 1); /* version & flags */
#else
    uint8_t data[] = {  0,   0,   0,  28,   /* size */
                      'd', 'r', 'e', 'f',
                        0,   0,   0,   0,   /* version & flags */
                        0,   0,   0,   1,   /* entry count */
                        0,   0,   0, 0xC,   /* size */
                      'u', 'r', 'l', ' ',
                        0,   0,   0,   1};  /* version & flags */
    avio_write(pb, data, sizeof(data));
#endif

    return 28;
}

static int mov_write_stbl_tag(AVIOContext *pb, MOVTrack *track)
{
    int64_t pos = avio_tell(pb);
#ifdef TEST
    avio_wb32(pb, 0); /* size */
    ffio_wfourcc(pb, "stbl");
#else
    {
        uint8_t data[] = {  0,   0,   0,   0,   /* size */
                          's', 't', 'b', 'l'};
        avio_write(pb, data, sizeof(data));
    }
#endif
    mov_write_stsd_tag(pb, track);
    mov_write_stts_tag(pb, track);
    if ((track->enc->codec_type == AVMEDIA_TYPE_VIDEO) &&
        track->entry_count[MOV_STSS] && track->entry_count[MOV_STSS] < track->sample_count)
    {
        mov_write_stss_tag(pb, track, MOV_SYNC_SAMPLE);
    }
    else if (CACHE_FILE_EXIST(track->c_cache[MOV_STSS]))
    {
        free_cache(*track->c_cache[MOV_STSS]);
        *track->c_cache[MOV_STSS] = NULL;
    }
    else if (track->entry[MOV_STSS])
    {
        av_free(track->entry[MOV_STSS]);
        track->entry[MOV_STSS] = NULL;
    }

    if (track->stco_chunk_pos)
    {
        int64_t pos = track->data_offset + track->stco_chunk_pos;
        if (CACHE_FILE_EXIST(track->c_cache[MOV_STCO]))
        {
            track->c_stco_entry = av_ne2be64(pos);
            cache_write(*track->c_cache[MOV_STCO], (unsigned char*)&track->c_stco_entry, sizeof(track->c_stco_entry));
        }
        else
        {
            MOVStco* ptEntry = (MOVStco*)ENTRY(track, MOV_STCO);
            *ptEntry = av_ne2be64(pos);
        }
        track->entry_count[MOV_STCO]++;
    }

    if (track->stsc_samples_per_chunk != track->stsc_chunk_samples_per_chunk)
    {
        if (CACHE_FILE_EXIST(track->c_cache[MOV_STSC]))
        {
            MOVStsc* ptEntry                    = &track->c_stsc_entry;
            track->stsc_first_chunk             = track->entry_count[MOV_STCO];
            ptEntry->first_chunk                = av_ne2be32(track->stsc_first_chunk);
            ptEntry->samples_per_chunk          = av_ne2be32(track->stsc_chunk_samples_per_chunk);
            ptEntry->sample_description_index   = av_ne2be32(1);
            cache_write(*track->c_cache[MOV_STSC], (unsigned char*)&track->c_stsc_entry, sizeof(track->c_stsc_entry));
        }
        else
        {
            MOVStsc* ptEntry                    = (MOVStsc*)ENTRY(track, MOV_STSC);
            track->stsc_first_chunk             = track->entry_count[MOV_STCO];
            ptEntry->first_chunk                = av_ne2be32(track->stsc_first_chunk);
            ptEntry->samples_per_chunk          = av_ne2be32(track->stsc_chunk_samples_per_chunk);
            ptEntry->sample_description_index   = av_ne2be32(1);
        }
        track->entry_count[MOV_STSC]++;
        track->stsc_samples_per_chunk       = track->stsc_chunk_samples_per_chunk;
    }

    mov_write_stsc_tag(pb, track);
    mov_write_stsz_tag(pb, track);
    mov_write_stco_tag(pb, track);
    return update_size(pb, pos);
}

static int mov_write_dinf_tag(AVIOContext *pb)
{
    int64_t pos = avio_tell(pb);
#ifdef TEST
    avio_wb32(pb, 0); /* size */
    ffio_wfourcc(pb, "dinf");
#else
    {
        uint8_t data[] = {
              0,   0,   0,   0,   /* size */
            'd', 'i', 'n', 'f'};
        avio_write(pb, data, sizeof(data));
    }
#endif
    mov_write_dref_tag(pb);
    return update_size(pb, pos);
}

static int mov_write_smhd_tag(AVIOContext *pb)
{
#ifdef TEST
    avio_wb32(pb, 16); /* size */
    ffio_wfourcc(pb, "smhd");
    avio_wb32(pb, 0); /* version & flags */
    avio_wb16(pb, 0); /* reserved (balance, normally = 0) */
    avio_wb16(pb, 0); /* reserved */
#else
    uint8_t data[] = {  0,   0,   0,  16,   /* size */
                      's', 'm', 'h', 'd',
                        0,   0,   0,   0,   /* version & flags */
                        0,   0,             /* reserved (balance, normally = 0) */
                        0,   0};            /* reserved */
    avio_write(pb, data, sizeof(data));
#endif
    return 16;
}

static int mov_write_vmhd_tag(AVIOContext *pb)
{
#ifdef TEST
    avio_wb32(pb, 0x14); /* size (always 0x14) */
    ffio_wfourcc(pb, "vmhd");
    avio_wb32(pb, 0x01); /* version & flags */
    avio_wb64(pb, 0); /* reserved (graphics mode = copy) */
#else
    uint8_t data[] = {0x0, 0x0, 0x0, 0x14,  /* size (always 0x14) */
                      'v', 'm', 'h',  'd',
                        0,   0,   0,    1,  /* version & flags */
                        0,   0,   0,    0,  /* reserved (graphics mode = copy) */
                        0,   0,   0,    0};
    avio_write(pb, data, sizeof(data));
#endif
    return 0x14;
}

static int mov_write_hdlr_tag(AVIOContext *pb, MOVTrack *track)
{
    const char *hdlr, *descr = NULL, *hdlr_type = NULL;
    int64_t pos = avio_tell(pb);

    if (!track) { /* no media --> data handler */
        hdlr = "dhlr";
        hdlr_type = "url ";
        descr = "DataHandler";
    } else {
        hdlr = (track->mode == MODE_MOV) ? "mhlr" : "\0\0\0\0";
        if (track->enc->codec_type == AVMEDIA_TYPE_VIDEO) {
            hdlr_type = "vide";
            descr = "VideoHandler";
        } else if (track->enc->codec_type == AVMEDIA_TYPE_AUDIO) {
            hdlr_type = "soun";
            descr = "SoundHandler";
        } else {
            hdlr = "dhlr";
            hdlr_type = "url ";
            descr = "DataHandler";
        }
    }

#ifdef TEST
    avio_wb32(pb, 0); /* size */
    ffio_wfourcc(pb, "hdlr");
    avio_wb32(pb, 0); /* Version & flags */
#else
    {
        uint8_t data[] = {0x0, 0x0, 0x0, 0x0,   /* size */
                          'h', 'd', 'l', 'r',
                            0,   0,   0,  0};   /* Version & flags */
        avio_write(pb, data, sizeof(data));
    }
#endif
    avio_write(pb, hdlr, 4); /* handler */
    ffio_wfourcc(pb, hdlr_type); /* handler type */
#ifdef TEST
    avio_wb32(pb ,0); /* reserved */
    avio_wb32(pb ,0); /* reserved */
    avio_wb32(pb ,0); /* reserved */
#else
    {
        uint8_t data[] = {  0,   0,   0,   0,   /* reserved */
                            0,   0,   0,   0,   /* reserved */
                            0,   0,   0,   0};  /* reserved */
        avio_write(pb, data, sizeof(data));
    }
#endif
    if (!track || track->mode == MODE_MOV)
        avio_w8(pb, strlen(descr)); /* pascal string */
    avio_write(pb, descr, strlen(descr)); /* handler description */
    if (track && track->mode != MODE_MOV)
        avio_w8(pb, 0); /* c string */
    return update_size(pb, pos);
}

static int mov_write_minf_tag(AVIOContext *pb, MOVTrack *track)
{
    int64_t pos = avio_tell(pb);
#ifdef TEST
    avio_wb32(pb, 0); /* size */
    ffio_wfourcc(pb, "minf");
#else
    {
        uint8_t data[] = {0x0, 0x0, 0x0, 0x0,   /* size */
                          'm', 'i', 'n', 'f'};
        avio_write(pb, data, sizeof(data));
    }
#endif
    if(track->enc->codec_type == AVMEDIA_TYPE_VIDEO)
        mov_write_vmhd_tag(pb);
    else if (track->enc->codec_type == AVMEDIA_TYPE_AUDIO)
        mov_write_smhd_tag(pb);
    mov_write_dinf_tag(pb);
    mov_write_stbl_tag(pb, track);
    return update_size(pb, pos);
}

static int mov_write_mdhd_tag(AVIOContext *pb, MOVTrack *track)
{
    int version = track->track_duration < INT32_MAX ? 0 : 1;

    (version == 1) ? avio_wb32(pb, 44) : avio_wb32(pb, 32); /* size */
#ifdef TEST
    ffio_wfourcc(pb, "mdhd");
    avio_w8(pb, version);
    avio_wb24(pb, 0); /* flags */
#else
    {
        uint8_t data[] = {'m', 'd', 'h', 'd',
                          version,
                          0, 0, 0};
        avio_write(pb, data, sizeof(data));
    }
#endif
    if (version == 1) {
        avio_wb64(pb, track->time);
        avio_wb64(pb, track->time);
    } else {
        avio_wb32(pb, track->time); /* creation time */
        avio_wb32(pb, track->time); /* modification time */
    }
    avio_wb32(pb, track->timescale); /* time scale (sample rate for audio) */
    if (!track->sample_count)
        (version == 1) ? avio_wb64(pb, UINT64_C(0xffffffffffffffff)) : avio_wb32(pb, 0xffffffff);
    else
        (version == 1) ? avio_wb64(pb, track->track_duration) : avio_wb32(pb, track->track_duration); /* duration */
    avio_wb16(pb, track->language); /* language */
    avio_wb16(pb, 0); /* reserved (quality) */

    return 32;
}

static int mov_write_mdia_tag(AVIOContext *pb, MOVTrack *track)
{
    int64_t pos = avio_tell(pb);
#ifdef TEST
    avio_wb32(pb, 0); /* size */
    ffio_wfourcc(pb, "mdia");
#else
    {
        uint8_t data[] = {0x0, 0x0, 0x0, 0x0,   /* size */
                          'm', 'd', 'i', 'a'};
        avio_write(pb, data, sizeof(data));
    }
#endif

    mov_write_mdhd_tag(pb, track);
    mov_write_hdlr_tag(pb, track);
    mov_write_minf_tag(pb, track);
    return update_size(pb, pos);
}

/* transformation matrix
     |a  b  u|
     |c  d  v|
     |tx ty w| */
static void write_matrix(AVIOContext *pb)
{
#ifdef TEST
    avio_wb32(pb, 1 << 16); /* 16.16 format */
    avio_wb32(pb, 0 << 16); /* 16.16 format */
    avio_wb32(pb, 0);        /* u in 2.30 format */
    avio_wb32(pb, 0 << 16); /* 16.16 format */
    avio_wb32(pb, 1 << 16); /* 16.16 format */
    avio_wb32(pb, 0);        /* v in 2.30 format */
    avio_wb32(pb, 0 << 16); /* 16.16 format */
    avio_wb32(pb, 0 << 16); /* 16.16 format */
    avio_wb32(pb, 1 << 30);  /* w in 2.30 format */
#else
    uint8_t data[] = {0,    1, 0, 0,    /* 16.16 format */
                      0,    0, 0, 0,    /* 16.16 format */
                      0,    0, 0, 0,    /* u in 2.30 format */
                      0,    0, 0, 0,    /* 16.16 format */
                      0,    1, 0, 0,    /* 16.16 format */
                      0,    0, 0, 0,    /* v in 2.30 format */
                      0,    0, 0, 0,    /* 16.16 format */
                      0,    0, 0, 0,    /* 16.16 format */
                      0x40, 0, 0, 0};   /* w in 2.30 format */
    avio_write(pb, data, sizeof(data));
#endif
}

static int mov_write_tkhd_tag(AVIOContext *pb, MOVTrack *track, AVStream *st)
{
    int64_t duration = av_rescale_rnd(track->track_duration, MOV_TIMESCALE,
                                      track->timescale, AV_ROUND_UP);
    int version = duration < INT32_MAX ? 0 : 1;

    (version == 1) ? avio_wb32(pb, 104) : avio_wb32(pb, 92); /* size */
#ifdef TEST
    ffio_wfourcc(pb, "tkhd");
    avio_w8(pb, version);
    avio_wb24(pb, 0xf); /* flags (track enabled) */
#else
    {
        uint8_t data[] = {'t', 'k', 'h', 'd',
                          version,
                          0, 0, 0xF};
        avio_write(pb, data, sizeof(data));
    }
#endif
    if (version == 1) {
        avio_wb64(pb, track->time);
        avio_wb64(pb, track->time);
    } else {
        avio_wb32(pb, track->time); /* creation time */
        avio_wb32(pb, track->time); /* modification time */
    }
    avio_wb32(pb, track->track_id); /* track-id */
    avio_wb32(pb, 0); /* reserved */
    if (!track->sample_count)
        (version == 1) ? avio_wb64(pb, UINT64_C(0xffffffffffffffff)) : avio_wb32(pb, 0xffffffff);
    else
        (version == 1) ? avio_wb64(pb, duration) : avio_wb32(pb, duration);

#ifdef TEST
    avio_wb32(pb, 0); /* reserved */
    avio_wb32(pb, 0); /* reserved */
    avio_wb16(pb, 0); /* layer */
#else
    {
        uint8_t data[] = {0, 0, 0, 0,   /* reserved */
                          0, 0, 0, 0,   /* reserved */
                          0, 0};        /* layer */
        avio_write(pb, data, sizeof(data));
    }
#endif
    avio_wb16(pb, st ? st->codec->codec_type : 0); /* alternate group) */
    /* Volume, only for audio */
    if(track->enc->codec_type == AVMEDIA_TYPE_AUDIO)
        avio_wb16(pb, 0x0100);
    else
        avio_wb16(pb, 0);
    avio_wb16(pb, 0); /* reserved */

    write_matrix(pb);
    /* Track width and height, for visual only */
    if(st && (track->enc->codec_type == AVMEDIA_TYPE_VIDEO))
    {
        double sample_aspect_ratio = av_q2d(st->sample_aspect_ratio);
        if(!sample_aspect_ratio || track->height != track->enc->height)
            sample_aspect_ratio = 1;
        avio_wb32(pb, sample_aspect_ratio * track->enc->width*0x10000);
        avio_wb32(pb, track->height*0x10000);
    }
    else {
#ifdef TEST
        avio_wb32(pb, 0);
        avio_wb32(pb, 0);
#else
        uint8_t data[] = {0, 0, 0, 0,
                          0, 0, 0, 0};
        avio_write(pb, data, sizeof(data));
#endif
    }
    return 0x5c;
}

static int mov_write_trak_tag(AVIOContext *pb, MOVMuxContext *mov,
                              MOVTrack *track, AVStream *st)
{
    int64_t pos = avio_tell(pb);
#ifdef TEST
    avio_wb32(pb, 0); /* size */
    ffio_wfourcc(pb, "trak");
#else
    {
        uint8_t data[] = {0x0, 0x0, 0x0, 0x0,   /* size */
                          't', 'r', 'a', 'k'};
        avio_write(pb, data, sizeof(data));
    }
#endif
    mov_write_tkhd_tag(pb, track, st);
    mov_write_mdia_tag(pb, track);

    return update_size(pb, pos);
}

static int mov_write_iods_tag(AVIOContext *pb, MOVMuxContext *mov)
{
    int i, has_audio = 0, has_video = 0;
    int64_t pos = avio_tell(pb);
    int audio_profile = mov->iods_audio_profile;
    int video_profile = mov->iods_video_profile;
    for (i = 0; i < mov->nb_streams; i++) {
        if(mov->tracks[i].sample_count > 0) {
            has_audio |= mov->tracks[i].enc->codec_type == AVMEDIA_TYPE_AUDIO;
            has_video |= mov->tracks[i].enc->codec_type == AVMEDIA_TYPE_VIDEO;
        }
    }
    if (audio_profile < 0)
        audio_profile = 0xFF - has_audio;
    if (video_profile < 0)
        video_profile = 0xFF - has_video;
#ifdef TEST
    avio_wb32(pb, 0x0); /* size */
    ffio_wfourcc(pb, "iods");
    avio_wb32(pb, 0);    /* version & flags */
#else
    {
        uint8_t data[] = {0x0, 0x0, 0x0, 0x0,   /* size */
                          'i', 'o', 'd', 's',
                          0x0, 0x0, 0x0, 0x0};  /* version & flags */
        avio_write(pb, data, sizeof(data));
    }
#endif
    put_descr(pb, 0x10, 7);
#ifdef TEST
    avio_wb16(pb, 0x004f);
    avio_w8(pb, 0xff);
    avio_w8(pb, 0xff);
    avio_w8(pb, audio_profile);
    avio_w8(pb, video_profile);
    avio_w8(pb, 0xff);
#else
    {
        uint8_t data[] = {0x00, 0x4f, 0xff, 0xff,
                          audio_profile,
                          video_profile,
                          0xff};
        avio_write(pb, data, sizeof(data));
    }
#endif
    return update_size(pb, pos);
}

static int mov_write_mvhd_tag(AVIOContext *pb, MOVMuxContext *mov)
{
    int max_track_id = 1, i;
    int64_t max_track_len_temp, max_track_len = 0;
    int version;

    for (i=0; i<mov->nb_streams; i++) {
        if(mov->tracks[i].sample_count > 0) {
            max_track_len_temp = av_rescale_rnd(mov->tracks[i].track_duration,
                                                MOV_TIMESCALE,
                                                mov->tracks[i].timescale,
                                                AV_ROUND_UP);
            if (max_track_len < max_track_len_temp)
                max_track_len = max_track_len_temp;
            if (max_track_id < mov->tracks[i].track_id)
                max_track_id = mov->tracks[i].track_id;
        }
    }

    version = max_track_len < UINT32_MAX ? 0 : 1;
    (version == 1) ? avio_wb32(pb, 120) : avio_wb32(pb, 108); /* size */
#ifdef TEST
    ffio_wfourcc(pb, "mvhd");
    avio_w8(pb, version);
    avio_wb24(pb, 0); /* flags */
#else
    {
        uint8_t data[] = {'m', 'v', 'h', 'd',
                          version,
                          0, 0, 0};  /* flags */
        avio_write(pb, data, sizeof(data));
    }
#endif
    if (version == 1) {
        avio_wb64(pb, mov->time);
        avio_wb64(pb, mov->time);
    } else {
        avio_wb32(pb, mov->time); /* creation time */
        avio_wb32(pb, mov->time); /* modification time */
    }
    avio_wb32(pb, MOV_TIMESCALE);
    (version == 1) ? avio_wb64(pb, max_track_len) : avio_wb32(pb, max_track_len); /* duration of longest track */

#ifdef TEST
    avio_wb32(pb, 0x00010000); /* reserved (preferred rate) 1.0 = normal */
    avio_wb16(pb, 0x0100); /* reserved (preferred volume) 1.0 = normal */
    avio_wb16(pb, 0); /* reserved */
    avio_wb32(pb, 0); /* reserved */
    avio_wb32(pb, 0); /* reserved */
#else
    {
        uint8_t data[] = {0x00, 0x01, 0x00, 0x00,   /* reserved (preferred rate) 1.0 = normal */
                          0x01, 0x00,               /* reserved (preferred volume) 1.0 = normal */
                          0x00, 0x00,               /* reserved */
                          0x00, 0x00, 0x00, 0x00,   /* reserved */
                          0x00, 0x00, 0x00, 0x00};  /* reserved */
        avio_write(pb, data, sizeof(data));
    }
#endif

    /* Matrix structure */
    write_matrix(pb);

#ifdef TEST
    avio_wb32(pb, 0); /* reserved (preview time) */
    avio_wb32(pb, 0); /* reserved (preview duration) */
    avio_wb32(pb, 0); /* reserved (poster time) */
    avio_wb32(pb, 0); /* reserved (selection time) */
    avio_wb32(pb, 0); /* reserved (selection duration) */
    avio_wb32(pb, 0); /* reserved (current time) */
#else
    {
        uint8_t data[24] = {0};
        avio_write(pb, data, sizeof(data));
    }
#endif
    avio_wb32(pb, max_track_id + 1); /* Next track id */
    return 0x6c;
}

static int mov_write_moov_tag(AVIOContext *pb, MOVMuxContext *mov,
                              AVFormatContext *s)
{
    int i;
    int64_t pos = avio_tell(pb);
#ifdef TEST
    avio_wb32(pb, 0); /* size placeholder*/
    ffio_wfourcc(pb, "moov");
#else
    uint8_t data[8] = {0, 0, 0, 0, 'm', 'o', 'o', 'v'};
    avio_write(pb, data, sizeof(data));
#endif

    for (i=0; i<mov->nb_streams; i++) {
        if (mov->tracks[i].sample_count <= 0)
            continue;

        mov->tracks[i].time = mov->time;
        mov->tracks[i].track_id = i+1;
    }

    mov_write_mvhd_tag(pb, mov);
    if (mov->mode != MODE_MOV && !mov->iods_skip)
        mov_write_iods_tag(pb, mov);
    for (i=0; i<mov->nb_streams; i++) {
        if (mov->tracks[i].sample_count > 0) {
            mov_write_trak_tag(pb, mov, &(mov->tracks[i]), i < s->nb_streams ? s->streams[i] : NULL);
        }
    }

    return update_size(pb, pos);
}

static int mov_write_mdat_tag(AVIOContext *pb, MOVMuxContext *mov)
{
    avio_wb32(pb, 8);    // placeholder for extended size field (64 bit)
    ffio_wfourcc(pb, mov->mode == MODE_MOV ? "wide" : "free");

    mov->mdat_pos = avio_tell(pb);
#ifdef TEST
    avio_wb32(pb, 0); /* size placeholder*/
    ffio_wfourcc(pb, "mdat");
#else
    {
        uint8_t data[] = {0, 0, 0, 0, 'm', 'd', 'a', 't'};
        avio_write(pb, data, sizeof(data));
    }
#endif
    return 0;
}

/* TODO: This needs to be more general */
static int mov_write_ftyp_tag(AVIOContext *pb, AVFormatContext *s)
{
    MOVMuxContext *mov = s->priv_data;
    int64_t pos = avio_tell(pb);
    int has_h264 = 0, has_video = 0;
    int i;

    for (i = 0; i < s->nb_streams; i++) {
        AVStream *st = s->streams[i];
        if (st->codec->codec_type == AVMEDIA_TYPE_VIDEO)
            has_video = 1;
        if (st->codec->codec_id == AV_CODEC_ID_H264)
            has_h264 = 1;
    }

#ifdef TEST
    avio_wb32(pb, 0); /* size */
    ffio_wfourcc(pb, "ftyp");
    ffio_wfourcc(pb, "isom");
    avio_wb32(pb, 0x200);
    if(mov->mode == MODE_MOV)
        ffio_wfourcc(pb, "qt  ");
    else {
        ffio_wfourcc(pb, "isom");
        ffio_wfourcc(pb, "iso2");
        if (has_h264)
            ffio_wfourcc(pb, "avc1");
    }
    if (mov->mode == MODE_MP4)
        ffio_wfourcc(pb, "mp41");
#else
    if(mov->mode == MODE_MOV)
    {
        uint8_t data[] = {   0,    0,    0,    0,
                           'f',  't',  'y',  'p',
                           'i',  's',  'o',  'm',
                          0x00, 0x00, 0x02, 0x00,
                           'q',  't',  ' ',  ' '};
        avio_write(pb, data, sizeof(data));
    }
    else
    {
        uint8_t data[] = {   0,    0,    0,    0,
                           'f',  't',  'y',  'p',
                           'i',  's',  'o',  'm',
                          0x00, 0x00, 0x02, 0x00,
                           'i',  's',  'o',  'm',
                           'i',  's',  'o',  '2',};
        avio_write(pb, data, sizeof(data));
        if (has_h264)
        {
            uint8_t data[] = {'a', 'v', 'c', '1'};
            avio_write(pb, data, sizeof(data));
        }
        {
            uint8_t data[] = {'m', 'p', '4', '1'};
            avio_write(pb, data, sizeof(data));
        }
    }
#endif
    return update_size(pb, pos);
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
    MMP_CHAR* string);

static void
generate_file_name(
    MMP_CHAR* out_filename,
    MMP_CHAR* url,
    int index)
{
    MMP_CHAR numStr[10];
    MMP_CHAR* extension;
    PalStrcpy(out_filename, url);

    extension = PalStrrchr(out_filename, '.');
    if (extension != MMP_NULL)
        *extension = 0;
    smtkItow(index, numStr);
    PalStrcat(out_filename, ".");
    PalStrcat(out_filename, numStr);

    //PalWcscat(out_filename, append);
}

int realloc_entry(MOVTrack *trk, int table_id)
{
    static const int entry_table_increase_size[MOV_TABLE_COUNT] =
    {
        MOV_STTS_CLUSTER_SIZE,
        MOV_STSZ_CLUSTER_SIZE,
        MOV_STSC_CLUSTER_SIZE,
        MOV_STCO_CLUSTER_SIZE,
        MOV_STSS_CLUSTER_SIZE
    };
    int result = 0;

    if ((trk->entry_count[table_id] + MOV_TABLE_RESERVED_SIZE) >= trk->entry_allocated[table_id])   // if IS_ENTRY_SPACE_NOT_ENOUGH
    {
        int new_count = trk->entry_count[table_id] + entry_table_increase_size[table_id] + MOV_TABLE_RESERVED_SIZE;
        int new_size = g_single_entry_size[table_id] * new_count;
#if 1
        void* new_space = av_realloc(trk->entry[table_id], new_size);
#else
        // for test
        void* new_space;

        if (trk->entry_allocated[table_id] == 0 || table_id == 0)
            new_space = av_realloc(trk->entry[table_id], new_size);
        else
            new_space = 0;
#endif

        if (new_space)
        {
            trk->entry[table_id]            = (unsigned char*)new_space;
            trk->entry_allocated[table_id]  = new_count;
        }
        else
        {
            trac("no enough memory! id(%d) size(%d)", table_id, new_size);
            // set the error flag to info upper layer about the no enough memory problem
            result = 1;
        }
        if (trk->entry_count[table_id] + 4 >= trk->entry_allocated[table_id])
        {
            result = -1;
        }
    }

    return result;
}

int ff_mov_write_packet(AVFormatContext *s, AVPacket *pkt)
{
    MOVMuxContext *mov = s->priv_data;
    AVIOContext *pb = s->pb;
    MOVTrack *trk = &mov->tracks[pkt->stream_index];
    unsigned int samples_in_chunk = 0;
    int size= pkt->size;
    int result = 0;

    if (trk->sample_size)
        samples_in_chunk = size / trk->sample_size;
    else
        samples_in_chunk = 1;

    if (s->flags & AVFMT_FLAG_CACHE_DATA_IN_FILE)
    {
        if (trk->entry_count[MOV_STTS] == 0 && (!trk->c_cache[MOV_STTS]))
        {
            trk->c_cache[MOV_STTS] = (trk->enc->codec_type == AVMEDIA_TYPE_VIDEO)
                              ? &s->caches[CACHE_FILE_VIDEO_STTS]
                              : &s->caches[CACHE_FILE_AUDIO_STTS];
            if (!CACHE_FILE_EXIST(trk->c_cache[MOV_STTS]))
            {
                trac("");
                return -1;
            }
        }

        if ((trk->enc->codec_type == AVMEDIA_TYPE_VIDEO)
        && ((trk->entry_count[MOV_STSS] == 0 && (!trk->c_cache[MOV_STSS]))))
        {
            trk->c_cache[MOV_STSS] = &s->caches[CACHE_FILE_VIDEO_STSS];
            if (!CACHE_FILE_EXIST(trk->c_cache[MOV_STSS]))
            {
                trac("");
                return -1;
            }
        }

        if ((trk->entry_count[MOV_STSZ] == 0 && (!trk->c_cache[MOV_STSZ])))
        {
            trk->c_cache[MOV_STSZ] = (trk->enc->codec_type == AVMEDIA_TYPE_VIDEO)
                              ? &s->caches[CACHE_FILE_VIDEO_STSZ]
                              : &s->caches[CACHE_FILE_AUDIO_STSZ];
            if (!CACHE_FILE_EXIST(trk->c_cache[MOV_STSZ]))
            {
                trac("");
                return -1;
            }
        }

        if ((trk->entry_count[MOV_STCO] == 0 && (!trk->c_cache[MOV_STCO])))
        {
            trk->c_cache[MOV_STCO] = (trk->enc->codec_type == AVMEDIA_TYPE_VIDEO)
                              ? &s->caches[CACHE_FILE_VIDEO_STCO]
                              : &s->caches[CACHE_FILE_AUDIO_STCO];
            if (!CACHE_FILE_EXIST(trk->c_cache[MOV_STCO]))
            {
                trac("");
                return -1;
            }
        }

        if ((trk->entry_count[MOV_STSC] == 0 && (!trk->c_cache[MOV_STSC])))
        {
            trk->c_cache[MOV_STSC] = (trk->enc->codec_type == AVMEDIA_TYPE_VIDEO)
                              ? &s->caches[CACHE_FILE_VIDEO_STSC]
                              : &s->caches[CACHE_FILE_AUDIO_STSC];
            if (!CACHE_FILE_EXIST(trk->c_cache[MOV_STSC]))
            {
                trac("");
                return -1;
            }
        }

        avio_write(pb, pkt->data, size);
        memcpy((void*)&trk->cluster[0], (void*)&trk->cluster[1], sizeof(trk->cluster[0]));
        trk->cluster[1].pos = avio_tell(pb) - size;
        trk->cluster[1].samples_in_chunk = samples_in_chunk;
        trk->cluster[1].chunkNum = 0;
        trk->cluster[1].size = size;
        trk->cluster[1].entries = samples_in_chunk;
        trk->cluster[1].dts = pkt->dts;
        if (!trk->sample_count && trk->start_dts != AV_NOPTS_VALUE) {
            /* First packet of a new fragment. We already wrote the duration
            * of the last packet of the previous fragment based on track_duration,
            * which might not exactly match our dts. Therefore adjust the dts
            * of this packet to be what the previous packets duration implies. */
            trk->cluster[1].dts = trk->start_dts + trk->track_duration;
        }
        if (!trk->sample_count && trk->start_dts == AV_NOPTS_VALUE) {
            trk->cluster[1].dts = trk->start_dts = 0;
        }
        if (trk->start_dts == AV_NOPTS_VALUE)
            trk->start_dts = pkt->dts;
        trk->track_duration = pkt->dts - trk->start_dts + pkt->duration;
        if (trk->sample_count >= 1)
        {
            // stts
            int pre_sample_duration  = trk->cluster[1].dts
                                     - trk->cluster[0].dts;
            if (pre_sample_duration != trk->stts_entry_duration)
            {
                if (trk->stts_entry_sample_count > 0)
                {
                    trk->c_stts_entry.count    = av_ne2be32(trk->stts_entry_sample_count);
                    trk->c_stts_entry.duration = av_ne2be32(trk->stts_entry_duration);
                    cache_write(*trk->c_cache[MOV_STTS], (unsigned char*)&trk->c_stts_entry, sizeof(trk->c_stts_entry));

                    trk->stts_entry_sample_count = 0;
                    trk->entry_count[MOV_STTS]++;
                }
                trk->stts_entry_duration = pre_sample_duration;
            }
            trk->stts_entry_sample_count++;

            // stco, stsc
            if (((trk->stco_chunk_pos + trk->stco_chunk_size) == trk->cluster[1].pos)
              && (trk->stco_chunk_size + trk->cluster[1].size < (1 << 20)))
            {
                // same chunk
                trk->stco_chunk_size += trk->cluster[1].size;
                trk->stsc_chunk_samples_per_chunk += trk->cluster[1].entries;
            }
            else
            {
                int64_t pos = trk->data_offset + trk->stco_chunk_pos;
                trk->c_stco_entry = av_ne2be64(pos);
                cache_write(*trk->c_cache[MOV_STCO], (unsigned char*)&trk->c_stco_entry, sizeof(trk->c_stco_entry));
                trk->stco_chunk_pos     = trk->cluster[1].pos;
                trk->stco_chunk_size    = trk->cluster[1].size;
                trk->entry_count[MOV_STCO]++;

                if (trk->entry_count[MOV_STSC] > 0)
                {
                    if (trk->stsc_samples_per_chunk != trk->stsc_chunk_samples_per_chunk)
                    {
                        MOVStsc* ptEntry                    = &trk->c_stsc_entry;
                        trk->stsc_first_chunk               = trk->entry_count[MOV_STCO];
                        ptEntry->first_chunk                = av_ne2be32(trk->stsc_first_chunk);
                        ptEntry->samples_per_chunk          = av_ne2be32(trk->stsc_chunk_samples_per_chunk);
                        ptEntry->sample_description_index   = av_ne2be32(1);
                        cache_write(*trk->c_cache[MOV_STSC], (unsigned char*)&trk->c_stsc_entry, sizeof(trk->c_stsc_entry));
                        trk->entry_count[MOV_STSC]++;
                        trk->stsc_samples_per_chunk         = trk->stsc_chunk_samples_per_chunk;
                    }
                }
                else
                {
                    MOVStsc* ptEntry                    = &trk->c_stsc_entry;
                    ptEntry->first_chunk                = av_ne2be32(trk->stsc_first_chunk);
                    ptEntry->samples_per_chunk          = av_ne2be32(trk->stsc_chunk_samples_per_chunk);
                    ptEntry->sample_description_index   = av_ne2be32(1);
                    cache_write(*trk->c_cache[MOV_STSC], (unsigned char*)&trk->c_stsc_entry, sizeof(trk->c_stsc_entry));
                    trk->entry_count[MOV_STSC]               = 1;
                    trk->stsc_samples_per_chunk         = trk->stsc_chunk_samples_per_chunk;
                }
                trk->stsc_chunk_samples_per_chunk = trk->cluster[1].entries;
            }
        }
        else
        {
            // stsz
            //trk->stsz_sample_size   = trk->cluster[1].size;
            // stco
            trk->stco_chunk_pos     = trk->cluster[1].pos;
            trk->stco_chunk_size    = trk->cluster[1].size;
            // stsc
            trk->stsc_first_chunk               = 1;
            trk->stsc_chunk_samples_per_chunk   = trk->cluster[1].entries;
        }

        // stsz
        if (CACHE_FILE_EXIST(trk->c_cache[MOV_STSZ]))
        {
            trk->c_stsz_entry = av_ne2be32(trk->cluster[1].size);
            cache_write(*trk->c_cache[MOV_STSZ], (unsigned char*)&trk->c_stsz_entry, sizeof(trk->c_stsz_entry));
            trk->entry_count[MOV_STSZ]++;
        }

        if (pkt->pts == AV_NOPTS_VALUE) {
            av_log(s, AV_LOG_WARNING, "pts has no value\n");
            pkt->pts = pkt->dts;
        }

        trk->cluster[1].flags = 0;
        if (pkt->flags & AV_PKT_FLAG_KEY) {
            if (trk->c_cache[MOV_STSS])
            {
                trk->c_stss_entry = av_ne2be32(trk->sample_count + 1);
                cache_write(*trk->c_cache[MOV_STSS], (unsigned char*)&trk->c_stss_entry, sizeof(trk->c_stss_entry));
                trk->entry_count[MOV_STSS]++;
            }
        }
    }
    else
    {
        result = realloc_entry(trk, MOV_STTS);
        if (result < 0)
            return result;

        result |= realloc_entry(trk, MOV_STSZ);
        if (result < 0)
            return result;

        result |= realloc_entry(trk, MOV_STSC);
        if (result < 0)
            return result;

        result |= realloc_entry(trk, MOV_STCO);
        if (result < 0)
            return result;

        if (trk->enc->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            result |= realloc_entry(trk, MOV_STSS);
            if (result < 0)
                return result;
        }

        avio_write(pb, pkt->data, size);
        memcpy((void*)&trk->cluster[0], (void*)&trk->cluster[1], sizeof(trk->cluster[0]));
        trk->cluster[1].pos = avio_tell(pb) - size;
        trk->cluster[1].samples_in_chunk = samples_in_chunk;
        trk->cluster[1].chunkNum = 0;
        trk->cluster[1].size = size;
        trk->cluster[1].entries = samples_in_chunk;
        trk->cluster[1].dts = pkt->dts;
        if (!trk->sample_count && trk->start_dts != AV_NOPTS_VALUE) {
            /* First packet of a new fragment. We already wrote the duration
            * of the last packet of the previous fragment based on track_duration,
            * which might not exactly match our dts. Therefore adjust the dts
            * of this packet to be what the previous packets duration implies. */
            trk->cluster[1].dts = trk->start_dts + trk->track_duration;
        }
        if (!trk->sample_count && trk->start_dts == AV_NOPTS_VALUE) {
            trk->cluster[1].dts = trk->start_dts = 0;
        }
        if (trk->start_dts == AV_NOPTS_VALUE)
            trk->start_dts = pkt->dts;
        trk->track_duration = pkt->dts - trk->start_dts + pkt->duration;
        if (trk->sample_count >= 1)
        {
            // stts
            int pre_sample_duration  = trk->cluster[1].dts
                                     - trk->cluster[0].dts;
            if (pre_sample_duration != trk->stts_entry_duration)
            {
                if (trk->stts_entry_sample_count > 0)
                {
                    MOVStts* ptEntry = (MOVStts*)ENTRY(trk, MOV_STTS);
                    ptEntry->count    = av_ne2be32(trk->stts_entry_sample_count);
                    ptEntry->duration = av_ne2be32(trk->stts_entry_duration);

                    trk->stts_entry_sample_count = 0;
                    trk->entry_count[MOV_STTS]++;
                }
                trk->stts_entry_duration = pre_sample_duration;
            }
            trk->stts_entry_sample_count++;

            // stsz
            //if (!trk->stsz_sample_size)
            //{
            //    trk->stsz_entry[trk->sample_count] = av_ne2be32(trk->cluster[1].size);
            //    trk->entry_count[MOV_STSZ]++;
            //}
            //else
            //{
            //    if (trk->cluster[1].size != trk->stsz_sample_size)
            //    {
            //        int i;
            //        trk->entry_count[MOV_STSZ] = trk->sample_count + 1;
            //        for (i = 0; i < trk->sample_count; ++i)
            //        {
            //            trk->stsz_entry[i] = av_ne2be32(trk->stsz_sample_size);
            //        }
            //        trk->stsz_entry[trk->sample_count] = av_ne2be32(trk->cluster[1].size);
            //        trk->stsz_sample_size  = 0;
            //    }
            //}

            // stco, stsc
            if (((trk->stco_chunk_pos + trk->stco_chunk_size) == trk->cluster[1].pos)
                && (trk->stco_chunk_size + trk->cluster[1].size < (1 << 20)))
            {
                // same chunk
                trk->stco_chunk_size += trk->cluster[1].size;
                trk->stsc_chunk_samples_per_chunk += trk->cluster[1].entries;
            }
            else
            {
                int64_t pos = trk->data_offset + trk->stco_chunk_pos;
                MOVStco* ptEntry        = (MOVStco*)ENTRY(trk, MOV_STCO);
                *ptEntry                = av_ne2be64(pos);
                trk->stco_chunk_pos     = trk->cluster[1].pos;
                trk->stco_chunk_size    = trk->cluster[1].size;
                trk->entry_count[MOV_STCO]++;

                if (trk->entry_count[MOV_STSC] > 0)
                {
                    if (trk->stsc_samples_per_chunk != trk->stsc_chunk_samples_per_chunk)
                    {
                        MOVStsc* ptEntry                    = (MOVStsc*)ENTRY(trk, MOV_STSC);
                        trk->stsc_first_chunk               = trk->entry_count[MOV_STCO];
                        ptEntry->first_chunk                = av_ne2be32(trk->stsc_first_chunk);
                        ptEntry->samples_per_chunk          = av_ne2be32(trk->stsc_chunk_samples_per_chunk);
                        ptEntry->sample_description_index   = av_ne2be32(1);

                        trk->entry_count[MOV_STSC]++;
                        trk->stsc_samples_per_chunk         = trk->stsc_chunk_samples_per_chunk;
                    }
                }
                else
                {
                    MOVStsc* ptEntry                    = (MOVStsc*)ENTRY(trk, MOV_STSC);
                    ptEntry->first_chunk                = av_ne2be32(trk->stsc_first_chunk);
                    ptEntry->samples_per_chunk          = av_ne2be32(trk->stsc_chunk_samples_per_chunk);
                    ptEntry->sample_description_index   = av_ne2be32(1);
                    trk->entry_count[MOV_STSC]               = 1;
                    trk->stsc_samples_per_chunk         = trk->stsc_chunk_samples_per_chunk;
                }
                trk->stsc_chunk_samples_per_chunk = trk->cluster[1].entries;
            }
        }
        else
        {
            // stsz
            //trk->stsz_sample_size   = trk->cluster[1].size;
            // stco
            trk->stco_chunk_pos     = trk->cluster[1].pos;
            trk->stco_chunk_size    = trk->cluster[1].size;
            // stsc
            trk->stsc_first_chunk               = 1;
            trk->stsc_chunk_samples_per_chunk   = trk->cluster[1].entries;
        }

        // stsz
        if (trk->entry[MOV_STSZ])
        {
            MOVStsz* ptEntry = (MOVStsz*)ENTRY(trk, MOV_STSZ);
            //trk->stsz_entry[trk->sample_count] = av_ne2be32(trk->cluster[1].size);
            *ptEntry = av_ne2be32(trk->cluster[1].size);
            trk->entry_count[MOV_STSZ]++;
        }

        if (pkt->pts == AV_NOPTS_VALUE) {
            av_log(s, AV_LOG_WARNING, "pts has no value\n");
            pkt->pts = pkt->dts;
        }
        trk->cluster[1].flags = 0;
        if (pkt->flags & AV_PKT_FLAG_KEY) {
            if (trk->entry[MOV_STSS])
            {
                MOVStss* ptEntry = (MOVStss*)ENTRY(trk, MOV_STSS);
                *ptEntry = av_ne2be32(trk->sample_count + 1);
                trk->entry_count[MOV_STSS]++;
            }
        }
    }
    trk->sample_count++;
    trk->mdat_size += size;
    mov->mdat_size += size;

    avio_flush(pb);

    return result;
}

static int mov_write_single_packet(AVFormatContext *s, AVPacket *pkt)
{
    if (!pkt->size) return 0; /* Discard 0 sized packets */

    return ff_mov_write_packet(s, pkt);
}

static int mov_write_packet(AVFormatContext *s, AVPacket *pkt)
{
    if (!pkt) {
        return 1;
    } else {
        if (!pkt->size) return 0; /* Discard 0 sized packets */

        return mov_write_single_packet(s, pkt);
    }
}

static int mov_write_header(AVFormatContext *s)
{
    AVIOContext *pb = s->pb;
    MOVMuxContext *mov = s->priv_data;
    int i;

    /* Default mode == MP4 */
    mov->mode = MODE_MOV;

    if (s->oformat != NULL) {
        mov_write_ftyp_tag(pb,s);
    }

    mov->nb_streams = s->nb_streams;

    mov->tracks = av_mallocz(mov->nb_streams*sizeof(*mov->tracks));
    if (!mov->tracks)
    {
        trac("");
        return AVERROR(ENOMEM);
    }

    for(i=0; i<s->nb_streams; i++){
        AVStream *st= s->streams[i];
        MOVTrack *track= &mov->tracks[i];
//        AVDictionaryEntry *lang = av_dict_get(st->metadata, "language", NULL,0);

        track->enc = st->codec;
//        track->language = ff_mov_iso639_to_lang(lang?lang->value:"und", mov->mode!=MODE_MOV);
//        if (track->language < 0)
//            track->language = 0;
        track->mode = mov->mode;
        track->tag = mov_find_codec_tag(s, track);
        if (!track->tag) {
            av_log(s, AV_LOG_ERROR, "track %d: could not find tag, "
                   "codec not currently supported in container\n", i);
            goto error;
        }
        /* If hinting of this track is enabled by a later hint track,
         * this is updated. */
        track->start_dts = AV_NOPTS_VALUE;
        if(st->codec->codec_type == AVMEDIA_TYPE_VIDEO){
            track->timescale = st->codec->time_base.den;
            //while(track->timescale < 10000)
            //    track->timescale *= 2;
        }else if(st->codec->codec_type == AVMEDIA_TYPE_AUDIO){
            track->timescale = st->codec->sample_rate / 16;
            if(!st->codec->frame_size) {
                av_log(s, AV_LOG_WARNING, "track %d: codec frame size is not set\n", i);
                track->audio_vbr = 1;
            } else if(st->codec->frame_size > 1){ /* assume compressed audio */
                track->audio_vbr = 1;
            }else{
                track->sample_size = 0;
            }
            if (track->mode != MODE_MOV &&
                track->enc->codec_id == AV_CODEC_ID_MP3 && track->timescale < 16000) {
                av_log(s, AV_LOG_ERROR, "track %d: muxing mp3 at %dhz is not supported\n",
                       i, track->enc->sample_rate);
                goto error;
            }
        }else{
            track->timescale = MOV_TIMESCALE;
        }
        if (!track->height)
            track->height = st->codec->height;

        avpriv_set_pts_info(st, 64, 1, track->timescale);

        /* copy extradata if it exists */
        if (st->codec->extradata_size) {
            track->vos_len  = st->codec->extradata_size;
            track->vos_data = av_malloc(track->vos_len);
            memcpy(track->vos_data, st->codec->extradata, track->vos_len);
        }
    }

    mov_write_mdat_tag(pb, mov);
    avio_flush(pb);

    return 0;
 error:
    av_freep(&mov->tracks);
    return -1;
}

static int mov_write_trailer(AVFormatContext *s)
{
    MOVMuxContext *mov = s->priv_data;
    AVIOContext *pb = s->pb;
    int64_t moov_pos;
    int res = 0;
    int i;

    if (!mov)
        goto end;
    moov_pos = avio_tell(pb);

    /* Write size of mdat tag */
    if (mov->mdat_size + 8 <= UINT32_MAX) {
        avio_seek(pb, mov->mdat_pos, SEEK_SET);
        avio_wb32(pb, mov->mdat_size + 8);
    } else {
        /* overwrite 'wide' placeholder atom */
        avio_seek(pb, mov->mdat_pos - 8, SEEK_SET);
        /* special value: real atom size will be 64 bit value after
        * tag field */
#ifdef TEST
        avio_wb32(pb, 1);
        ffio_wfourcc(pb, "mdat");
#else
        {
            uint8_t data[] = {0, 0, 0, 1, 'm', 'd', 'a', 't'};
            avio_write(pb, data, sizeof(data));
        }
#endif
        avio_wb64(pb, mov->mdat_size + 16);
    }
    avio_seek(pb, moov_pos, SEEK_SET);
    mov_write_moov_tag(pb, mov, s);

    for (i=0; i<mov->nb_streams; i++) {
        MOVTrack *track = &mov->tracks[i];
        if (track->vos_len)
            av_free(track->vos_data);
        free_entry(track->c_cache[MOV_STTS], &track->entry[MOV_STTS]);
        free_entry(track->c_cache[MOV_STSS], &track->entry[MOV_STSS]);
        free_entry(track->c_cache[MOV_STSZ], &track->entry[MOV_STSZ]);
        free_entry(track->c_cache[MOV_STCO], &track->entry[MOV_STCO]);
        free_entry(track->c_cache[MOV_STSC], &track->entry[MOV_STSC]);
    }
    av_freep(&mov->tracks);

end:
    if (s->caches)
    {
        for (i = 0; i < CACHE_FILE_COUNT; i++)
        {
            free_entry(&s->caches[i], NULL);
        }
        av_freep(&s->caches);
    }
    return res;
}

static uint64_t mov_write_size(AVFormatContext *s)
{
    MOVMuxContext *mov = s->priv_data;
    AVIOContext *pb = s->pb;
    uint64_t write_size;
    uint32_t max_entry_size;
    uint32_t entry_size;
    uint32_t header_size;
    int i;

    if (!mov)
        return 0;

    write_size = avio_write_size(pb);
    max_entry_size = 0;
    header_size = 0;

#define _PROCESS_ENTRY_(header_size, entry_size, max_entry_size, table_id)                  \
    {                                                                                       \
        entry_size = (g_single_entry_size[(table_id)] * track->entry_count[(table_id)]);    \
        if (max_entry_size < entry_size)                                                    \
            max_entry_size = entry_size;                                                    \
        header_size += entry_size;                                                          \
    }

    for (i=0; i<mov->nb_streams; i++) {
        MOVTrack *track = &mov->tracks[i];
        _PROCESS_ENTRY_(header_size, entry_size, max_entry_size, MOV_STTS);
        _PROCESS_ENTRY_(header_size, entry_size, max_entry_size, MOV_STSS);
        _PROCESS_ENTRY_(header_size, entry_size, max_entry_size, MOV_STSZ);
        _PROCESS_ENTRY_(header_size, entry_size, max_entry_size, MOV_STCO);
        _PROCESS_ENTRY_(header_size, entry_size, max_entry_size, MOV_STSC);
    }
    write_size += header_size;
    write_size += max_entry_size;
    return write_size;
}

static int mov_init_format(AVFormatContext *s)
{
    int i;

    if (s->flags & AVFMT_FLAG_CACHE_DATA_IN_FILE)
    {
        MMP_CHAR   out_filename[MAX_FILE_PATH];

        s->caches = (cache_t**)av_mallocz(sizeof(*s->caches) * CACHE_FILE_COUNT);
        if (!s->caches)
            goto fail;

        for (i = 0; i < CACHE_FILE_COUNT; ++i)
        {
            generate_file_name(out_filename, s->pb->url, i);
            s->caches[i] =  new_cache(out_filename, MMP_TRUE);
            if (!s->caches[i])
            {
                trac("");
                goto fail;
            }
        }
    }

    return 0;

fail:
    if (s->caches)
    {
        for (i = 0; i < CACHE_FILE_COUNT; i++)
        {
            free_entry(&s->caches[i], NULL);
        }
        av_freep(&s->caches);
    }
    return -1;
}

#if CONFIG_MP4_MUXER
//MOV_CLASS(mp4)
#if WIN32
const AVCodecTag* const ff_mp4_codec_tag[] = {
    ff_mp4_obj_type, 0
};

AVOutputFormat ff_mp4_muxer = {
    //"mp4",
    AVFMT_GLOBALHEADER | AVFMT_ALLOW_FLUSH,
    ff_mp4_codec_tag,
    NULL,
    sizeof(MOVMuxContext),
    mov_init_format,
    mov_write_header,
    mov_write_packet,
    mov_write_trailer,
    mov_write_size
};
#else
AVOutputFormat ff_mp4_muxer = {
    //.name              = "mp4",
    //.long_name         = NULL_IF_CONFIG_SMALL("MP4 (MPEG-4 Part 14)"),
    //.mime_type         = "application/mp4",
    //.extensions        = "mp4",
    .priv_data_size    = sizeof(MOVMuxContext),
    .init_format        = mov_init_format,
    .write_header      = mov_write_header,
    .write_packet      = mov_write_packet,
    .write_trailer     = mov_write_trailer,
    .write_size        = mov_write_size,
    .flags             = AVFMT_GLOBALHEADER | AVFMT_ALLOW_FLUSH,
    .codec_tag         = (const AVCodecTag* const []){ ff_mp4_obj_type, 0 },
    //.priv_class        = &mp4_muxer_class,
};
#endif
#endif
