/*
 * Copyright (c) 2005-2012 Michael Niedermayer <michaelni@gmx.at>
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
 * miscellaneous math routines and tables
 */

//#include <stdint.h>
//#include <limits.h>

#include "mathematics.h"
#include "../libavutil/common.h"
//#include "avassert.h"
//#include "version.h"

int64_t av_gcd(int64_t a, int64_t b){
    if(b) return av_gcd(b, a%b);
    else  return a;
}

int64_t av_rescale_rnd(int64_t a, int64_t b, int64_t c, enum AVRounding rnd){
    int64_t r=0;
    av_assert2(c > 0);
    av_assert2(b >=0);
    av_assert2((unsigned)(rnd&~AV_ROUND_PASS_MINMAX)<=5 && (rnd&~AV_ROUND_PASS_MINMAX)!=4);

    if (rnd & AV_ROUND_PASS_MINMAX) {
        if (a == INT64_MIN || a == INT64_MAX)
            return a;
        rnd -= AV_ROUND_PASS_MINMAX;
    }

    if(a<0 && a != INT64_MIN) return -av_rescale_rnd(-a, b, c, rnd ^ ((rnd>>1)&1));

    if(rnd==AV_ROUND_NEAR_INF) r= c/2;
    else if(rnd&1)             r= c-1;

    if(b<=INT_MAX && c<=INT_MAX){
        if(a<=INT_MAX)
            return (a * b + r)/c;
        else
            return a/c*b + (a%c*b + r)/c;
    }else{
#if 1
        uint64_t a0= a&0xFFFFFFFF;
        uint64_t a1= a>>32;
        uint64_t b0= b&0xFFFFFFFF;
        uint64_t b1= b>>32;
        uint64_t t1= a0*b1 + a1*b0;
        uint64_t t1a= t1<<32;
        int i;

        a0 = a0*b0 + t1a;
        a1 = a1*b1 + (t1>>32) + (a0<t1a);
        a0 += r;
        a1 += a0<r;

        for(i=63; i>=0; i--){
//            int o= a1 & 0x8000000000000000ULL;
            a1+= a1 + ((a0>>i)&1);
            t1+=t1;
            if(/*o || */c <= a1){
                a1 -= c;
                t1++;
            }
        }
        return t1;
    }
#else
        AVInteger ai;
        ai= av_mul_i(av_int2i(a), av_int2i(b));
        ai= av_add_i(ai, av_int2i(r));

        return av_i2int(av_div_i(ai, av_int2i(c)));
    }
#endif
}

int64_t av_rescale(int64_t a, int64_t b, int64_t c){
    return av_rescale_rnd(a, b, c, AV_ROUND_NEAR_INF);
}
