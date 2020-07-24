#ifndef __OPUS_CODEC__
#define __OPUS_CODEC__

#ifdef __cplusplus
extern "C" {
#endif

#include "arm_lite_dev/armlite_dev_device.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define OPUS_CREATE                 1
#define OPUS_ENCODE                 2
#define OPUS_DECODE                 3

#define ITP_IOCTL_OPUS_CREATE            ITP_IOCTL_CUSTOM_CTL_ID1
#define ITP_IOCTL_OPUS_ENCODE            ITP_IOCTL_CUSTOM_CTL_ID2
#define ITP_IOCTL_OPUS_DECODE            ITP_IOCTL_CUSTOM_CTL_ID3

#define MAX_CMD_DATA_BUFFER_SIZE    2560
#define CMD_DATA_BUFFER_OFFSET      (450 * 1024)
#define CONTEXT_BUFFER_SIZE  1024

#define OPUS_ENCODE_INIT  0
#define OPUS_DECODE_INIT  1

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct
{
    int       err;
    uint32_t  sampleRate;
    uint32_t  application;
    uint32_t  channels;
    uint32_t  mode;
    uint32_t  codecAddr;
} OPUS_INIT_DATA;

typedef struct
{
    int       err;
    uint32_t  frameSize;
    uint32_t  codecAddr;
    uint32_t  ctxtLen;
    int       fecDec;
    uint16_t  ctxtBuffer[CONTEXT_BUFFER_SIZE];
} OPUS_CONTEXT;

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================


//=============================================================================
//                Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif





