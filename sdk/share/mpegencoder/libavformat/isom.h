/*
 * ISO Media common code
 * copyright (c) 2001 Fabrice Bellard
 * copyright (c) 2002 Francois Revol <revol@free.fr>
 * copyright (c) 2006 Baptiste Coudurier <baptiste.coudurier@free.fr>
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

#ifndef AVFORMAT_ISOM_H
#define AVFORMAT_ISOM_H

#include "avio.h"
#include "internal.h"
//#include "dv.h"

/* isom.c */
extern const AVCodecTag ff_mp4_obj_type[];
extern const AVCodecTag ff_codec_movvideo_tags[];
extern const AVCodecTag ff_codec_movaudio_tags[];

int ff_mov_iso639_to_lang(const char lang[4], int mp4);

/* the QuickTime file format is quite convoluted...
 * it has lots of index tables, each indexing something in another one...
 * Here we just use what is needed to read the chunks
 */

typedef struct MOVStts {
    int count;
    int duration;
} MOVStts;

typedef struct MOVStsc {
    uint32_t first_chunk;
    uint32_t samples_per_chunk;
    uint32_t sample_description_index;
} MOVStsc;

typedef int     MOVStss;
typedef int     MOVStsz;
typedef int64_t MOVStco;

typedef struct MOVAtom {
    uint32_t type;
    int64_t size; /* total size (excluding the size and type fields) */
} MOVAtom;

int ff_mp4_read_descr_len(AVIOContext *pb);
int ff_mp4_read_descr(AVFormatContext *fc, AVIOContext *pb, int *tag);
int ff_mp4_read_dec_config_descr(AVFormatContext *fc, AVStream *st, AVIOContext *pb);
void ff_mp4_parse_es_descr(AVIOContext *pb, int *es_id);

#define MP4ODescrTag                    0x01
#define MP4IODescrTag                   0x02
#define MP4ESDescrTag                   0x03
#define MP4DecConfigDescrTag            0x04
#define MP4DecSpecificDescrTag          0x05
#define MP4SLDescrTag                   0x06

#define MOV_TFHD_BASE_DATA_OFFSET       0x01
#define MOV_TFHD_STSD_ID                0x02
#define MOV_TFHD_DEFAULT_DURATION       0x08
#define MOV_TFHD_DEFAULT_SIZE           0x10
#define MOV_TFHD_DEFAULT_FLAGS          0x20
#define MOV_TFHD_DURATION_IS_EMPTY  0x010000

#define MOV_TRUN_DATA_OFFSET            0x01
#define MOV_TRUN_FIRST_SAMPLE_FLAGS     0x04
#define MOV_TRUN_SAMPLE_DURATION       0x100
#define MOV_TRUN_SAMPLE_SIZE           0x200
#define MOV_TRUN_SAMPLE_FLAGS          0x400

#define MOV_FRAG_SAMPLE_FLAG_DEGRADATION_PRIORITY_MASK 0x0000ffff
#define MOV_FRAG_SAMPLE_FLAG_IS_NON_SYNC               0x00010000
#define MOV_FRAG_SAMPLE_FLAG_PADDING_MASK              0x000e0000
#define MOV_FRAG_SAMPLE_FLAG_REDUNDANCY_MASK           0x00300000
#define MOV_FRAG_SAMPLE_FLAG_DEPENDED_MASK             0x00c00000
#define MOV_FRAG_SAMPLE_FLAG_DEPENDS_MASK              0x03000000

#define MOV_FRAG_SAMPLE_FLAG_DEPENDS_NO                0x02000000
#define MOV_FRAG_SAMPLE_FLAG_DEPENDS_YES               0x01000000

int ff_mov_read_esds(AVFormatContext *fc, AVIOContext *pb, MOVAtom atom);
enum AVCodecID ff_mov_get_lpcm_codec_id(int bps, int flags);

void ff_mov_write_chan(AVIOContext *pb, int64_t channel_layout);

#endif /* AVFORMAT_ISOM_H */
