#ifndef MENCODER_H
#define MENCODER_H

#include "config.h"
#include "stream/stream.h"
#include "libmpdemux/muxer.h"
#include "libmpdemux/stheader.h"
#include "encoder/mmp_encoder.h"

//#include "../core_interface.h"
//#include "../queue_mgr.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define MAX_FILE_PATH   255

enum 
{
    AUDIO = 0,
    VIDEO,
    MEDIA_TYPE_COUNT
};

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct VIDEO_INFO_TAG
{
    AVC_FRAME_RATE  frameRate;
    MMP_UINT32      EnFrameRate;
    MMP_UINT32      width;
    MMP_UINT32      height;
#ifdef ENABLE_MENCODER
    MMP_UINT8*      AVCDecoderConfigurationRecord;
    MMP_UINT32      AVCDecoderConfigurationRecordSize;
#endif
} VIDEO_INFO;

typedef struct AUDIO_INFO_TAG
{
    MMP_AUDIO_ENGINE audioEncoderType;
} AUDIO_INFO;

typedef struct RECORD_MODE_TAG
{
    MMP_BOOL bDisableFileSplitter;
} RECORD_MODE;


typedef struct MEContext
{
    int                     out_file_format;    // default to AVI
    MMP_CHAR               *ori_filename;
    MMP_CHAR                out_filename[MAX_FILE_PATH];
    int                     do_not_fragment;
    int                     is_memory_not_enough;
    int                     fragment_count;
    unsigned int            max_mux_f_size;     // in MBytes
    unsigned int            acc_mux_f_size;
    double                  base_pts;
    double                  a_pts;  // shifted pts, relative to 0
    double                  v_pts;  // shifted pts, relative to 0
    double                  pre_ori_pts[MEDIA_TYPE_COUNT];
    double                  sample_count[MEDIA_TYPE_COUNT];
    double                  timeunit[MEDIA_TYPE_COUNT];
    VIDEO_INFO              tVideoInfo;
    AUDIO_INFO              tAudioInfo;

    int                     osi;
    stream_buffer_ctrl_t    *osbc;
    stream_t                *ostreams[2];
    stream_t                *ostream;
    muxer_t                 *muxer;
    muxer_stream_t          *mux_a;
    muxer_stream_t          *mux_v;
    sh_video_t              _sh_video;
    sh_video_t              *sh_video;
    sh_audio_t              _sh_audio;
    sh_audio_t              *sh_audio;
} MEContext;

//=============================================================================
//                              Function Declaration
//=============================================================================
MEContext*
mencoder_Init(
    void);

void
mencoder_Terminate(
    MEContext*  mectx);

MMP_RESULT
mencoder_Open(
    MEContext*  mectx,
    MMP_CHAR*      filename,
    RECORD_MODE*    ptRecMode);

MMP_RESULT
mencoder_Close(
    MEContext*  mectx);

MMP_RESULT
mencoder_SetVideoInfo(
    MEContext*  mectx,
    VIDEO_INFO* ptVideoInfo);

MMP_RESULT
mencoder_SetAudioInfo(
    MEContext *mectx,
    AUDIO_INFO *ptInfo);

MMP_RESULT
mencoder_WriteAudioChunk(
    MEContext   *mectx,
    char        *data,
    int         size,
    double      pts);

MMP_RESULT
mencoder_WriteVideoChunk(
    MEContext*  mectx,
    char        *data,
    int         size,
    double      pts,
    MMP_BOOL    isKeyFrame);

stream_buffer_ctrl_t*
mencoder_GetStreamBufferCtrl(
    void);

int
mencoder_GetWriteSize(
    void);

#endif /* MENCODER_H */
