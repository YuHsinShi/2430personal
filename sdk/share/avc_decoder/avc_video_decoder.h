/*
 * Copyright (c) 2010 ITE Technology Corp. All Rights Reserved.
 */
/** @file avc_video_decoder.h
 *
 * @author
 */

#ifndef _AVC_VIDEO_DECODER_H_
#define _AVC_VIDEO_DECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================

#include "ite/mmp_types.h"
#include "ite/itp.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define NUM_DATA_POINTERS 4


//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct AVC_FRAME_TAG
{
	uint8_t *data[NUM_DATA_POINTERS];
	int linesize[NUM_DATA_POINTERS];
	int64_t pts;
	int width;
	int height;
} AVC_FRAME;

typedef struct AVC_VIDEO_DECODER_TAG
{
    MMP_UINT8*  pPacketStartAddr;
    MMP_UINT8*  pPacketEndAddr;
    MMP_UINT8*  pBufferStartAddr;
    MMP_UINT8*  pBufferEndAddr;

    MMP_UINT32  timeStamp;
    //MMP_UINT32  mpeg2_flag;

    // sequence_header
    MMP_UINT32  horizontal_size;
    MMP_UINT32  vertical_size;
    MMP_UINT32  aspect_ratio_information;
    MMP_UINT32  frame_rate_code;
    // sequence_extension
    //MMP_UINT32  progressive_sequence;
    // picture_header
    //MMP_UINT32  prev_temporal_reference;
    //MMP_UINT32  curr_temporal_reference;
    //MMP_UINT32  picture_coding_type;
    // picture_display_extension
    //MMP_UINT32  picture_structure;
    //MMP_UINT32  repeat_first_field;
    //MMP_UINT32  top_field_first;
    // picture_coding_extension
    //MMP_UINT32  progressive_frame;
    // user_data - afd_data
    MMP_UINT32  active_format;

    //INFO_VIDEO_RATIO currVideoRatio;
    MMP_UINT32  currAfd;
    MMP_BOOL    bUpdateAspectRatio;
    //MMP_BOOL    bFindSequenceHeader;
    MMP_BOOL    bAVsyncReady;
    MMP_BOOL    bHDVideoFormat;
    MMP_BOOL    bReSync;
    //MMP_UINT32  second_field_flag;
    
    // for video segment decode
    MMP_UINT8*  pSegStartAddr;
    MMP_UINT8*  pSegEndAddr;
    MMP_UINT8*  pPESStartAddr;
    MMP_BOOL    bStartSegment;
    MMP_BOOL    bEndSegment;
    MMP_INT32   remainByte;

    MMP_BOOL    bIsNalFormat;
    MMP_UINT32  NALUnitLength;

    // STC,audio engine time offset
    MMP_INT nOffset;

    MMP_UINT32  gPreTimeStamp;    
} AVC_VIDEO_DECODER;

typedef enum VIDEO_RETURN_ERR_TAG
{
    VIDEO_RETURN_NORMAL                 = (1 << 0),
    VIDEO_RETURN_END_OF_PACKET          = (1 << 1),
    VIDEO_RETURN_RESOLUTION_NOT_SUPPORT = (1 << 2),
    VIDEO_RETURN_SKIP_DECODE            = (1 << 3),
    VIDEO_RETURN_UNEXPECTED_START_CODE  = (1 << 4),
    VIDEO_RETURN_WAIT_FIRE              = (1 << 5),
    VIDEO_RETURN_OUT_REF_FRAME_RANGE    = (1 << 6),
    VIDEO_RETURN_TOOL_NOT_SUPPORT       = (1 << 7),
    VIDEO_RETURN_GET_DEC_HEADER         = (1 << 8),
    VIDEO_RETURN_SKIP_DISPLAY_ONCE      = (1 << 9),
    VIDEO_RETURN_FILL_BLANK             = (1 << 10),
    VIDEO_RETURN_SKIP_PACKET        =  (1 << 11),

    VIDEO_RETURN_UNKNOW                 = (1 << 31),
} VIDEO_RETURN_ERR;

typedef enum VIDEO_FRAME_DROP_STATE_TAG
{
    VIDEO_NO_DROP = 0,
    VIDEO_DROP_B_FRAME,
    VIDEO_DROP_P_FRAME,
    VIDEO_DROP_HALF_I_FRAME,
    VIDEO_DROP_3_4_I_FRAME,
    VIDEO_DROP_7_8_I_FRAME
} VIDEO_FRAME_DROP_STATE;


//=============================================================================
//                              Function Declaration
//=============================================================================

void AVC_Video_Decoder_Release(void);

void AVC_Video_Decoder_Init(void);

MMP_UINT32 AVC_Video_Decoder_Reload(void);

void
AVC_Video_Decoder_Display(
	AVC_FRAME		 *data,
	int 		   *data_size);


void AVC_Video_Decoder_Wait(void);

void AVC_Video_Decoder_Fire(void);

void AVC_Video_Decoder_End(void);

void
AVC_Video_Decoder_ReSync(
    MMP_BOOL bEnable);

int iTE_h264_Dtv_decode_init(void);

int iTE_h264_Dtv_decode_end(void);

int iTE_h264_Dtv_decode_frame(AVC_FRAME *data, int *data_size, MMP_UINT8 *buf, int buf_size);

void iTE_h264_Dtv_decode_flush(void);

#ifdef __cplusplus
}
#endif

#endif
