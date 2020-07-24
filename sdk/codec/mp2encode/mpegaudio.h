/*
 * copyright (c) 2001 Fabrice Bellard
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

/**
 * @file
 * mpeg audio declarations for both encoder and decoder.
 */

#ifndef AVCODEC_MPEGAUDIO_H
#define AVCODEC_MPEGAUDIO_H

#ifndef CONFIG_FLOAT
#   define CONFIG_FLOAT 0
#endif

#include "put_bits.h"

/* max frame size, in samples */
#define MPA_FRAME_SIZE 1152

#define SAMPLES_BUF_SIZE 4096

/* max compressed frame size */
#define MPA_MAX_CODED_FRAME_SIZE 1792

#define MPA_MAX_CHANNELS 2

#define SBLIMIT 32 /* number of subbands */

#define MPA_STEREO  0
#define MPA_JSTEREO 1
#define MPA_DUAL    2
#define MPA_MONO    3

#ifndef FRAC_BITS
#define FRAC_BITS   23   /* fractional bits for sb_samples and dct */
#define WFRAC_BITS  16   /* fractional bits for window */
#endif

#define FRAC_ONE    (1 << FRAC_BITS)

#define FIX(a)   ((int)((a) * FRAC_ONE))

#if CONFIG_FLOAT
#   define INTFLOAT float
typedef float MPA_INT;
typedef float OUT_INT;
#elif FRAC_BITS <= 15
#   define INTFLOAT int
typedef int16_t MPA_INT;
typedef int16_t OUT_INT;
#else
#   define INTFLOAT int
typedef int32_t MPA_INT;
typedef int16_t OUT_INT;
#endif

#ifndef av_log2
#   define av_log2       av_log2_c
#endif

typedef struct MpegAudioContext {
    PutBitContext pb;
    int nb_channels;
    int lsf;           /* 1 if mpeg2 low bitrate selected */
    int bitrate_index; /* bit rate */
    int freq_index;
    int frame_size; /* frame size, in bits, without padding */
    /* padding computation */
    int frame_frac, frame_frac_incr, do_padding;
    short* samples_buf[MPA_MAX_CHANNELS]; /* buffer for filter */
    short samples_buf_temp[MPA_MAX_CHANNELS][SAMPLES_BUF_SIZE]; /* buffer for filter */    
    int samples_offset[MPA_MAX_CHANNELS];       /* offset in samples_buf */
    int sb_samples[MPA_MAX_CHANNELS][3][12][SBLIMIT];
    unsigned char scale_factors[MPA_MAX_CHANNELS][SBLIMIT][3]; /* scale factors */
    /* code to group 3 scale factors */
    unsigned char scale_code[MPA_MAX_CHANNELS][SBLIMIT];
    int sblimit; /* number of used subbands */
    const unsigned char *alloc_table;
    int sample_rate;
    int bit_rate;

} MpegAudioContext;

static unsigned char ff_log2_tab[256]={
        0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
        5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
};
static int av_log2_c(unsigned int v)
{
    int n = 0;
    if (v & 0xffff0000) {
        v >>= 16;
        n += 16;
    }
    if (v & 0xff00) {
        v >>= 8;
        n += 8;
    }
    n += ff_log2_tab[v];

    return n;
}

int ff_mpa_l2_select_table(int bitrate, int nb_channels, int freq, int lsf);
int MPA_encode_init(MpegAudioContext *s);
int MPA_encode_frame(MpegAudioContext *s,unsigned char *frame, int buf_size, void *data);

#endif /* AVCODEC_MPEGAUDIO_H */
