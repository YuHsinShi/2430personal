#ifndef __VIDEO_DEVICE_TABLE_H__
#define __VIDEO_DEVICE_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "ite/mmp_types.h"
#include "mmp_capture.h"
//=============================================================================
//                Constant Definition
//=============================================================================
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                NORMAL VIDEO TABLE
//=============================================================================
typedef struct CAP_NORMAL_TIMINFO_TABLE_TAG
{
    MMP_UINT16 Index;
    MMP_UINT16 HActive;
    MMP_UINT16 VActive;
    MMP_UINT16 FrameRate;
    MMP_UINT16 ROIPosX;
    MMP_UINT16 ROIPosY;
    MMP_UINT16 ROIWidth;
    MMP_UINT16 ROIHeight;
    MMP_UINT16 HNum1;
    MMP_UINT16 LineNum1;
    MMP_UINT16 LineNum2;
    MMP_UINT16 LineNum3;
    MMP_UINT16 LineNum4;
    MMP_UINT16 ClkDelay;
} CAP_NORMAL_TIMINFO_TABLE;

//Normal Video Table , if add new sensor , you need add your timing in here.
static CAP_NORMAL_TIMINFO_TABLE VIDEO_TABLE [] = {
                                                                                                                                                 //Index, HActive, VActive,    FRate,   ROIX, ROIY, ROIW,   ROIH,    HNum1,   LineNum1,      LineNum2,         LineNum3,   LineNum4,     CLKDELAY
    {0,      720,    487, MMP_CAP_FRAMERATE_59_94HZ,     0,    0,  720,     480,       0,        0,          0,             0,       0,      0}, //480i60 //only for ADV7180
    {1,      720,    480, MMP_CAP_FRAMERATE_59_94HZ,     0,    0,  720,     480,       0,        0,          0,             0,       0,      0}, //480i60
    {2,      720,    576, MMP_CAP_FRAMERATE_50HZ,        0,    0,  720,     576,       0,        0,          0,             0,       0,      0}, //576i50
    {3,     1280,    720, MMP_CAP_FRAMERATE_25HZ,        0,    0, 1280,     720,       0,        0,          0,             0,       0,      0}, //720P25
    {4,     1280,    720, MMP_CAP_FRAMERATE_30HZ,        0,    0, 1280,     720,       0,        0,          0,             0,       0,      0}, //720P30
    {5,     1280,    960, MMP_CAP_FRAMERATE_30HZ,        0,    0, 1280,     960,       0,        0,          0,             0,       0,      0}, //960P30
    {6,     1920,   1080, MMP_CAP_FRAMERATE_30HZ,        0,    0, 1920,    1080,       0,        0,          0,             0,       0,      0}, //1080P30
};

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