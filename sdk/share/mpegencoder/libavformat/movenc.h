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

#ifndef AVFORMAT_MOVENC_H
#define AVFORMAT_MOVENC_H

#include "avformat.h"
#include "isom.h"
#include "cache/cache_file.h"

#define MOV_STTS_CLUSTER_SIZE       16
#define MOV_STTS_CLUSTER_EXTRA_SIZE 2   // reserved space for mov_write_stts_tag()
#define MOV_STSZ_CLUSTER_SIZE       98304
#define MOV_STSS_CLUSTER_SIZE       (MOV_STSZ_CLUSTER_SIZE / 16)
#define MOV_STCO_CLUSTER_SIZE       MOV_STSZ_CLUSTER_SIZE
#define MOV_STSC_CLUSTER_SIZE       MOV_STSZ_CLUSTER_SIZE
#define MOV_TABLE_RESERVED_SIZE    64
#define MOV_TIMESCALE 1000

#define RTP_MAX_PACKET_SIZE 1450

#define MODE_MP4  0x01
#define MODE_MOV  0x02
#define MODE_3GP  0x04

// do not change the order
enum
{
    MOV_STTS = 0,
    MOV_STSZ,
    MOV_STSC,
    MOV_STCO,
    MOV_STSS,
    MOV_TABLE_COUNT,
};

typedef struct MOVIentry {
    uint64_t        pos;
    int64_t         dts;
    unsigned int    size;
    unsigned int    samples_in_chunk;
    unsigned int    chunkNum;           ///< Chunk number if the current entry is a chunk start otherwise 0
    unsigned int    entries;
    //int             cts;
#define MOV_SYNC_SAMPLE         0x0001
#define MOV_PARTIAL_SYNC_SAMPLE 0x0002
    uint32_t        flags;
} MOVIentry;

typedef struct MOVIndex {
    int             mode;
    unsigned        timescale;
    uint64_t        time;
    int64_t         track_duration;
    uint64_t        mdat_size;

    int             sample_count;
    long            sample_size;
#define MOV_TRACK_CTTS         0x0001
#define MOV_TRACK_STPS         0x0002
    uint32_t        flags;
    int             language;
    int             track_id;
    int             tag; ///< stsd fourcc
    AVCodecContext  *enc;

    int             vos_len;
    uint8_t         *vos_data;
    MOVIentry       cluster[2];
    int             audio_vbr;
    int             height; ///< active picture (w/o VBI) height for D-10/IMX
    int64_t         start_dts;

    unsigned char*  entry[MOV_TABLE_COUNT];
    int             entry_allocated[MOV_TABLE_COUNT];   // allocated count of entries 
    int             entry_count[MOV_TABLE_COUNT];       // actually stored count of entries
    cache_t**       c_cache[MOV_TABLE_COUNT];
    // stts
    int             stts_entry_duration;
    int             stts_entry_sample_count;
    MOVStts         c_stts_entry;
    // stss
    int             c_stss_entry;
    // stsz
    int             stsz_sample_size;
    int             c_stsz_entry;
    // stco
    int64_t         stco_chunk_pos;
    int64_t         stco_chunk_size;
    int64_t         c_stco_entry;
    // stsc (sample to chunk)
    uint32_t        stsc_first_chunk;
    uint32_t        stsc_samples_per_chunk;
    uint32_t        stsc_chunk_samples_per_chunk;
    MOVStsc         c_stsc_entry;

    uint32_t        max_packet_size;

    int64_t         data_offset;
} MOVTrack;

typedef struct MOVMuxContext {
    int             mode;
    int64_t         time;
    int             nb_streams;
    int64_t         mdat_pos;
    uint64_t        mdat_size;
    MOVTrack        *tracks;

    int             flags;

    int             iods_skip;
    int             iods_video_profile;
    int             iods_audio_profile;

    AVIOContext     *mdat_buf;
} MOVMuxContext;

#define FF_MOV_FLAG_RTP_HINT 1
#define FF_MOV_FLAG_FRAGMENT 2
#define FF_MOV_FLAG_EMPTY_MOOV 4
#define FF_MOV_FLAG_FRAG_KEYFRAME 8
#define FF_MOV_FLAG_SEPARATE_MOOF 16
#define FF_MOV_FLAG_FRAG_CUSTOM 32
#define FF_MOV_FLAG_ISML 64
#define FF_MOV_FLAG_FASTSTART 128

#endif /* AVFORMAT_MOVENC_H */
