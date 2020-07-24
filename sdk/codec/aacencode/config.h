/*
 ** Copyright 2003-2010, VisualOn, Inc.
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */
/*******************************************************************************
	File:		config.h

	Content:	aac encoder parameter

*******************************************************************************/

#ifndef _AACENC_CONFIG_H_
#define _AACENC_CONFIG_H_

#define ITE_RISC_FLOW

//#define AAC_ENCODE_PERFORMANCE_TEST_BY_TICK


#define AAC_ENABLE_INTERNAL_SD 
#define INTERNAL_SD 0x40000000

//#define WIN32
#if !defined(WIN32) && !defined(__CYGWIN__) && !defined(__OR32__)
#  define __OR32__
#endif

// pre allocate memory
#define STATIC_MEMORY

#define MAX_CHANNELS        2

#define AACENC_BLOCKSIZE    1024   /*! encoder only takes BLOCKSIZE samples at a time */
#define AACENC_TRANS_FAC    8      /*! encoder short long ratio */


#define MAXBITS_COEF		6144
#define MINBITS_COEF		744


/* max compressed frame size */
// temp setting
#define AAC_MAX_CODED_FRAME_SIZE 1500

/******************************
 The Buffer size of input stream
 ******************************/
#define READBUF_SIZE        (4096*12)

/******************************
 The Buffer size of output stream
 ******************************/
 #define OUTPUT_ONE_FRAME 
 #ifdef OUTPUT_ONE_FRAME
 #define OUTFRAME_SIZE 1000
 #else
 #define OUTFRAME_SIZE 3000
 #endif
 
#define OUTBUF_SIZE     (3000*16)  //(1024*16)

#endif
