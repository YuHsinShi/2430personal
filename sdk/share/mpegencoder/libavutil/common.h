/*
 * copyright (c) 2006 Michael Niedermayer <michaelni@gmx.at>
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
 * common internal and external API header
 */

#ifndef AVUTIL_COMMON_H
#define AVUTIL_COMMON_H

//#include <ctype.h>
//#include <errno.h>
//#include <inttypes.h>
//#include <limits.h>
//#include <math.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

#include "attributes.h"
//#include "version.h"
#include "../libavutil/avconfig.h"

#if AV_HAVE_BIGENDIAN
#   define AV_NE(be, le) (be)
#else
#   define AV_NE(be, le) (le)
#endif

#ifndef INT64_C 
#define INT64_C(c) (c ## LL) 
#define UINT64_C(c) (c ## ULL) 
#endif 

//rounded division & shift
#define RSHIFT(a,b) ((a) > 0 ? ((a) + ((1<<(b))>>1))>>(b) : ((a) + ((1<<(b))>>1)-1)>>(b))
/* assume b>0 */
#define ROUNDED_DIV(a,b) (((a)>0 ? (a) + ((b)>>1) : (a) - ((b)>>1))/(b))
#define FFUDIV(a,b) (((a)>0 ?(a):(a)-(b)+1) / (b))
#define FFUMOD(a,b) ((a)-(b)*FFUDIV(a,b))
#define FFABS(a) ((a) >= 0 ? (a) : (-(a)))
#define FFSIGN(a) ((a) > 0 ? 1 : -1)

#define FFMAX(a,b) ((a) > (b) ? (a) : (b))
#define FFMAX3(a,b,c) FFMAX(FFMAX(a,b),c)
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
#define FFMIN3(a,b,c) FFMIN(FFMIN(a,b),c)

#define FFSWAP(type,a,b) do{type SWAP_tmp= b; b= a; a= SWAP_tmp;}while(0)
#define FF_ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))
#define FFALIGN(x, a) (((x)+(a)-1)&~((a)-1))

/* Pull in unguarded fallback defines at the end of this file. */
#include "common.h"

/**
 * Clip a signed integer value into the amin-amax range.
 * @param a value to clip
 * @param amin minimum value of the clip range
 * @param amax maximum value of the clip range
 * @return clipped value
 */
static av_always_inline av_const int av_clip_c(int a, int amin, int amax)
{
#if defined(HAVE_AV_CONFIG_H) && defined(ASSERT_LEVEL) && ASSERT_LEVEL >= 2
    if (amin > amax) abort();
#endif
    if      (a < amin) return amin;
    else if (a > amax) return amax;
    else               return a;
}

#define MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define MKBETAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))

#include "mem.h"

/*
 * The following definitions are outside the multiple inclusion guard
 * to ensure they are immediately available in intmath.h.
 */

#ifndef av_ceil_log2
#   define av_ceil_log2     av_ceil_log2_c
#endif
#ifndef av_clip
#   define av_clip          av_clip_c
#endif
#ifndef av_clip64
#   define av_clip64        av_clip64_c
#endif
#ifndef av_clip_uint8
#   define av_clip_uint8    av_clip_uint8_c
#endif
#ifndef av_clip_int8
#   define av_clip_int8     av_clip_int8_c
#endif
#ifndef av_clip_uint16
#   define av_clip_uint16   av_clip_uint16_c
#endif
#ifndef av_clip_int16
#   define av_clip_int16    av_clip_int16_c
#endif
#ifndef av_clipl_int32
#   define av_clipl_int32   av_clipl_int32_c
#endif
#ifndef av_clip_uintp2
#   define av_clip_uintp2   av_clip_uintp2_c
#endif
#ifndef av_sat_add32
#   define av_sat_add32     av_sat_add32_c
#endif
#ifndef av_sat_dadd32
#   define av_sat_dadd32    av_sat_dadd32_c
#endif
#ifndef av_clipf
#   define av_clipf         av_clipf_c
#endif
#ifndef av_popcount
#   define av_popcount      av_popcount_c
#endif
#ifndef av_popcount64
#   define av_popcount64    av_popcount64_c
#endif
#endif
