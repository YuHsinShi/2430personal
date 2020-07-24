/***********************************************************************
Copyright (c) 2011, Skype Limited. All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
- Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
- Neither the name of Internet Society, IETF or IETF Trust, nor the
names of specific contributors, may be used to endorse or promote
products derived from this software without specific prior written
permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

/* Use C99 variable-size arrays */
#define VAR_ARRAYS      1

/* This is a build of OPUS */
#define OPUS_BUILD      1

/* Compile as fixed-point (for machines without a fast enough FPU) */
#define FIXED_POINT     1

/* Do not build the float API */
#define DISABLE_FLOAT_API

/* Define to 1 if you have the `lrintf' function. */
#define HAVE_LRINTF

/* Make use of ARM asm optimization */
#define OPUS_ARM_ASM

/* Use generic ARMv4 inline asm optimizations */
// #define OPUS_ARM_INLINE_ASM

/* Use ARMv5E inline asm optimizations */
#define OPUS_ARM_INLINE_EDSP 1

/* Define if assembler supports EDSP instructions */
#define OPUS_ARM_MAY_HAVE_EDSP 1

#define OVERRIDE_OPUS_ALLOC
#define OVERRIDE_OPUS_FREE

#include "version.h"

#endif /* CONFIG_H */
