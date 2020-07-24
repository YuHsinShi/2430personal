#ifndef __MMP_CAPTURE_H__
#define __MMP_CAPTURE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ite/ith.h"
#include "ite/mmp_types.h"
#include "capture_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define CAP_API     extern

//=============================================================================
//                              Constant Definition
//=============================================================================

#define CAP_HDMI_INPUT_VESA      14
/**
 *  Device Select
 */
typedef enum MMP_CAP_DEVICE_ID_TAG
{
    MMP_CAP_UNKNOW_DEVICE = 0,
    MMP_CAP_DEV_SENSOR,
    MMP_CAP_DEV_MAX,
} MMP_CAP_DEVICE_ID;

typedef enum DEVICESPROPERTYS_TAG
{
    DEVICES_WIDTH = 0,
    DEVICES_HEIGHT,
    DEVICES_FRAMETRATE,
    DEVICES_ISINTERLANCED,
	DEVICES_TABLEINDEX,
} DEVICESPROPERTYS;

typedef enum MMP_CAP_LANE_STATUS_TAG
{
    MMP_CAP_LANE0_STATUS,
} MMP_CAP_LANE_STATUS;

typedef enum MMP_CAP_FRAMERATE_TAG
{
    MMP_CAP_FRAMERATE_UNKNOW = 0,
    MMP_CAP_FRAMERATE_25HZ,
    MMP_CAP_FRAMERATE_50HZ,
    MMP_CAP_FRAMERATE_30HZ,
    MMP_CAP_FRAMERATE_60HZ,
    MMP_CAP_FRAMERATE_29_97HZ,
    MMP_CAP_FRAMERATE_59_94HZ,
    MMP_CAP_FRAMERATE_23_97HZ,
    MMP_CAP_FRAMERATE_24HZ,
    MMP_CAP_FRAMERATE_56HZ,
    MMP_CAP_FRAMERATE_70HZ,
    MMP_CAP_FRAMERATE_72HZ,
    MMP_CAP_FRAMERATE_75HZ,
    MMP_CAP_FRAMERATE_85HZ,
    MMP_CAP_FRAMERATE_VESA_30HZ,
    MMP_CAP_FRAMERATE_VESA_60HZ
} MMP_CAP_FRAMERATE;


typedef enum MMP_CAP_INPUT_INFO_TAG
{
    MMP_CAP_INPUT_INFO_640X480_60P   = 0,
    MMP_CAP_INPUT_INFO_720X480_59I   = 1,
    MMP_CAP_INPUT_INFO_720X480_59P   = 2,
    MMP_CAP_INPUT_INFO_720X480_60I   = 3,
    MMP_CAP_INPUT_INFO_720X480_60P   = 4,
    MMP_CAP_INPUT_INFO_720X576_50I   = 5,
    MMP_CAP_INPUT_INFO_720X576_50P   = 6,
    MMP_CAP_INPUT_INFO_1280X720_50P  = 7,
    MMP_CAP_INPUT_INFO_1280X720_59P  = 8,
    MMP_CAP_INPUT_INFO_1280X720_60P  = 9,
    MMP_CAP_INPUT_INFO_1920X1080_23P = 10,
    MMP_CAP_INPUT_INFO_1920X1080_24P = 11,
    MMP_CAP_INPUT_INFO_1920X1080_25P = 12,
    MMP_CAP_INPUT_INFO_1920X1080_29P = 13,
    MMP_CAP_INPUT_INFO_1920X1080_30P = 14,
    MMP_CAP_INPUT_INFO_1920X1080_50I = 15,
    MMP_CAP_INPUT_INFO_1920X1080_50P = 16,
    MMP_CAP_INPUT_INFO_1920X1080_59I = 17,
    MMP_CAP_INPUT_INFO_1920X1080_59P = 18,
    MMP_CAP_INPUT_INFO_1920X1080_60I = 19,
    MMP_CAP_INPUT_INFO_1920X1080_60P = 20,
    MMP_CAP_INPUT_INFO_800X600_60P   = 21,
    MMP_CAP_INPUT_INFO_1024X768_60P  = 22,
    MMP_CAP_INPUT_INFO_1280X768_60P  = 23,
    MMP_CAP_INPUT_INFO_1280X800_60P  = 24,
    MMP_CAP_INPUT_INFO_1280X960_60P  = 25,
    MMP_CAP_INPUT_INFO_1280X1024_60P = 26,
    MMP_CAP_INPUT_INFO_1360X768_60P  = 27,
    MMP_CAP_INPUT_INFO_1366X768_60P  = 28,
    MMP_CAP_INPUT_INFO_1440X900_60P  = 29,
    MMP_CAP_INPUT_INFO_1400X1050_60P = 30,
    MMP_CAP_INPUT_INFO_1440X1050_60P = 31,
    MMP_CAP_INPUT_INFO_1600X900_60P  = 32,
    MMP_CAP_INPUT_INFO_1600X1200_60P = 33,
    MMP_CAP_INPUT_INFO_1680X1050_60P = 34,
    MMP_CAP_INPUT_INFO_ALL           = 35,
    MMP_CAP_INPUT_INFO_NUM           = 36,
    MMP_CAP_INPUT_INFO_UNKNOWN       = 37,
    MMP_CAP_INPUT_INFO_CAMERA        = 38,
} MMP_CAP_INPUT_INFO;

typedef MMP_UINT32 AV_SYNC_COUNTER_CTRL;

#define AUDIO_COUNTER_CLEAR 0x0001
#define VIDEO_COUNTER_CLEAR 0x0002
#define MUTE_COUNTER_CLEAR  0x0004
#define AUDIO_COUNTER_LATCH 0x0008
#define VIDEO_COUNTER_LATCH 0x0008
#define MUTE_COUNTER_LATCH  0x0010
#define AUDIO_COUNTER_SEL   0x0020
#define VIDEO_COUNTER_SEL   0x0040
#define MUTE_COUNTER_SEL    0x0080
#define I2S_SOURCE_SEL      0x0100
#define SPDIF_SOURCE_SEL    0x0200
#define MUTEPRE_COUNTER_SEL 0x0400

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct ITE_CAP_VIDEO_INFO_TAG
{
    MMP_UINT16          OutWidth;
    MMP_UINT16          OutHeight;
    MMP_UINT8           IsInterlaced;
    MMP_CAP_FRAMERATE FrameRate;
    MMP_UINT8           *DisplayAddrY;
    MMP_UINT8           *DisplayAddrU;
    MMP_UINT8           *DisplayAddrV;
    MMP_UINT8           *DisplayAddrOldY;
    MMP_UINT16          PitchY;
    MMP_UINT16          PitchUV;
	MMP_UINT16          OutMemFormat;
} ITE_CAP_VIDEO_INFO;

typedef enum CAPTURE_DEV_ID_TAG
{
    CAP_DEV_ID0     = 0,
} CAPTURE_DEV_ID;


typedef struct CAPTURE_HANDLE_TAG
{
    CAP_CONTEXT         cap_info;   //cap device info
    CAPTURE_DEV_ID      cap_id;     //cap 0
    MMP_CAP_DEVICE_ID   source_id;  // front source id
} CAPTURE_HANDLE;

typedef struct CAPTURE_SETTING_TAG
{
	MMP_CAP_DEVICE_ID inputsource;
	MMP_BOOL OnflyMode_en;
	MMP_BOOL Interrupt_en;
	MMP_UINT32 Max_Width;
	MMP_UINT32 Max_Height;
	
} CAPTURE_SETTING;

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================

//=============================================================================
/**
 * ithCapDeviceAllDeviceTriState
 */
//=============================================================================
CAP_API void
ithCapDeviceAllDeviceTriState(
    void);

//=============================================================================
/**
 * ithCapDeviceInitialize
 */
//=============================================================================
CAP_API MMP_RESULT
ithCapDeviceInitialize(
    void);

//=============================================================================
/**
 * ithCapDeviceTerminate
 */
//=============================================================================
CAP_API void
ithCapDeviceTerminate(
    void);

//=============================================================================
/**
 * ithCapDeviceIsSignalStable
 */
//=============================================================================
CAP_API MMP_BOOL
ithCapDeviceIsSignalStable(
    void);

//=============================================================================
/**
 * ithCapGetDeviceInfo.
 */
//=============================================================================
CAP_API void
ithCapGetDeviceInfo(
    CAPTURE_HANDLE *ptDev);

//=============================================================================
/**
 * ithCapDeviceGetProperty
 */
//=============================================================================
CAP_API MMP_UINT16 
ithCapDeviceGetProperty(
    DEVICESPROPERTYS option);

//=============================================================================
/**
 * ithCapDeviceLEDON
 */
//=============================================================================
CAP_API void 
ithCapDeviceLEDON(
    MMP_BOOL enable);


//=============================================================================
/**
 * ithCapInitialize
 */
//=============================================================================
CAP_API MMP_RESULT
ithCapInitialize(
    void);


//=============================================================================
/**
 * ithCapConnect.
 */
//=============================================================================
CAP_API MMP_RESULT
ithCapConnect(
    CAPTURE_HANDLE *cap_handle, CAPTURE_SETTING info);

//=============================================================================
/**
 * ithCapDisConnect
 */
//=============================================================================
CAP_API MMP_RESULT
ithCapDisConnect(
    CAPTURE_HANDLE *ptDev);

//=============================================================================
/**
 * Cap terminate
 */
//=============================================================================
CAP_API MMP_RESULT
ithCapTerminate(
    void);

//=============================================================================
/**
 * ithCapIsFire
 */
//=============================================================================
CAP_API MMP_BOOL
ithCapIsFire(
    CAPTURE_HANDLE *ptDev);

//=============================================================================
/**
 * ithCapGetEngineErrorStatus
 */
//=============================================================================
CAP_API MMP_UINT32
ithCapGetEngineErrorStatus(
    CAPTURE_HANDLE *ptDev,MMP_CAP_LANE_STATUS lanenum);

//=============================================================================
/**
 * ithCapParameterSetting
 */
//=============================================================================
CAP_API MMP_RESULT
ithCapParameterSetting(
    CAPTURE_HANDLE *ptDev);

//=============================================================================
/**
 * ithCapFire
 */
//=============================================================================
CAP_API void
ithCapFire(
    CAPTURE_HANDLE *ptDev, MMP_BOOL enable);

//=============================================================================
/**
 * ithCapRegisterIRQ
 */
//=============================================================================
CAP_API void
ithCapRegisterIRQ(
    ITHIntrHandler caphandler,CAPTURE_HANDLE *ptDev);

//=============================================================================
/**
 * ithCapDisableIRQ
 */
//=============================================================================
CAP_API void
ithCapDisableIRQ(
    void);

//=============================================================================
/**
 * ithCapClearInterrupt
 */
//=============================================================================
CAP_API MMP_UINT16
ithCapClearInterrupt(
    CAPTURE_HANDLE *ptDev, MMP_BOOL get_err);

//=============================================================================
/**
 * ithCapReturnWrBufIndex
 */
//=============================================================================
CAP_API MMP_UINT16
ithCapReturnWrBufIndex(
    CAPTURE_HANDLE *ptDev);

//=============================================================================
/**
 * ithCapGetInputFrameRate
 */
//=============================================================================
CAP_API MMP_CAP_FRAMERATE
ithCapGetInputFrameRate(
    CAPTURE_HANDLE *ptDev);

//=============================================================================
/**
 * ithAVSyncCounterInit
 */
//=============================================================================
CAP_API void
ithAVSyncCounterCtrl(
    CAPTURE_HANDLE *ptDev, AV_SYNC_COUNTER_CTRL mode, MMP_UINT16 divider);

//=============================================================================
/**
 * ithAVSyncCounterReset
 */
//=============================================================================
CAP_API void
ithAVSyncCounterReset(
    CAPTURE_HANDLE *ptDev, AV_SYNC_COUNTER_CTRL mode);

//=============================================================================
/**
 * ithAVSyncCounterRead
 */
//=============================================================================
CAP_API MMP_UINT32
ithAVSyncCounterRead(
    CAPTURE_HANDLE *ptDev, AV_SYNC_COUNTER_CTRL mode);

//=============================================================================
/**
 * ithAVSyncMuteDetect
 */
//=============================================================================
CAP_API MMP_BOOL
ithAVSyncMuteDetect(
    CAPTURE_HANDLE *ptDev);

//=============================================================================
/**
 * ithCapGetInputSrcInfo
 */
//=============================================================================
CAP_API MMP_CAP_INPUT_INFO
ithCapGetInputSrcInfo(
    CAPTURE_HANDLE *ptDev);

//=============================================================================
/**
 * ithCapPowerUp
 */
//=============================================================================
CAP_API void
ithCapPowerUp(
    void);

//=============================================================================
/**
 * ithCapPowerDown
 */
//=============================================================================
CAP_API void
ithCapPowerDown(
    void);

//=============================================================================
/**
 * ithCapGetDetectedWidth
 */
//=============================================================================
CAP_API MMP_UINT32 
ithCapGetDetectedWidth(
	CAPTURE_HANDLE *ptDev);

//=============================================================================
/**
 * ithCapGetDetectedHeight
 */
//=============================================================================
CAP_API MMP_UINT32 
ithCapGetDetectedHeight(
	CAPTURE_HANDLE *ptDev);

//=============================================================================
/**
 * ithCapGetDetectedInterleave
 */
//=============================================================================
CAP_API MMP_UINT32 
ithCapGetDetectedInterleave(
	CAPTURE_HANDLE *ptDev);

//=============================================================================
/**
 * ithCapSetInterleave
 */
//=============================================================================
CAP_API void 
ithCapSetInterleave(
	CAPTURE_HANDLE *ptDev,
	MMP_UINT32 interleave);

#ifdef __cplusplus
}
#endif

#endif