/*
 * SpanDSP - a series of DSP components for telephony
 *
 * spandsp.h - The head guy amongst the headers
 *
 * Written by Steve Underwood <steveu@coppice.org>
 *
 * Copyright (C) 2003 Steve Underwood
 *
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*! \file */

#if !defined(_SPANDSP_H_)
#define _SPANDSP_H_

// #define __inline__ __inline
// #pragma warning(disable:4200)

// #undef SPANDSP_USE_FIXED_POINT
// #undef SPANDSP_MISALIGNED_ACCESS_FAILS

// #define SPANDSP_USE_EXPORT_CAPABILITY 1

#include <stdlib.h>
#include <inttypes.h>

#include <string.h>
#include <limits.h>
// #include <time.h>
#include <math.h>
// @INSERT_STDBOOL_HEADER@
// #include <tiffio.h>

#include <spandsp/telephony.h>
#include <spandsp/alloc.h>
#include <spandsp/bit_operations.h>
#include <spandsp/bitstream.h>
#include <spandsp/g711.h>
#include <spandsp/g726.h>

#endif

// #if defined(SPANDSP_EXPOSE_INTERNAL_STRUCTURES)
// #include <spandsp/expose.h>
// #endif
/*- End of file ------------------------------------------------------------*/
