#include <string.h>
#include "mmp_capture.h"
#include "capture/video_device_table.h"
#include "sensor/mmp_sensor.h"
#include "capture_config.h"

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
static SensorDriver md;
static MMP_UINT16   gSensorResolution = 0xFF;
pthread_mutex_t     gDevice_mutex = PTHREAD_MUTEX_INITIALIZER;      //devices mutex protect

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Device Tri-State.
 */
//=============================================================================
void
ithCapDeviceAllDeviceTriState(
    void)
{
    pthread_mutex_lock(&gDevice_mutex);
#ifdef CFG_SENSOR_ENABLE

    //Tri-State Device
    ithSensorOutputPinTriState(md, true);
#endif
    pthread_mutex_unlock(&gDevice_mutex);
    
}

//=============================================================================
/**
 * Device initialization.
 */
//=============================================================================
MMP_RESULT
ithCapDeviceInitialize(
    void)
{

    pthread_mutex_lock(&gDevice_mutex);

    MMP_RESULT    result  = MMP_SUCCESS;
#ifdef CFG_SENSOR_ENABLE
    unsigned char name [] = {CFG_CAPTURE_SENSOR_NAME};
    //printf("device name = %s \n",name);
    md                = ithSensorCreate(name);
    ithSensorPowerDown(md, false);
    ithSensorInit(md, 0);
    gSensorResolution = 0xFF;
#endif
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s error \n", __FUNCTION__);

    pthread_mutex_unlock(&gDevice_mutex);
    return result;
}

//=============================================================================
/**
 * Device terminate.
 */
//=============================================================================
void
ithCapDeviceTerminate(
    void)
{
    pthread_mutex_lock(&gDevice_mutex);
#ifdef CFG_SENSOR_ENABLE    
    ithSensorOutputPinTriState(md, true);
    ithSensorDeInit(md);
    ithSensorPowerDown(md, true);
    ithSensorDestroy(md);
    md = (SensorDriver) NULL;
#endif    
    pthread_mutex_unlock(&gDevice_mutex);

}

//=============================================================================
/**
 * Get Device Signal State.
 */
//=============================================================================
MMP_BOOL
ithCapDeviceIsSignalStable(
    void)
{
    pthread_mutex_lock(&gDevice_mutex);
    MMP_BOOL isSignalStable = MMP_FALSE;
 #ifdef CFG_SENSOR_ENABLE   
    isSignalStable = (MMP_BOOL)ithSensorIsSignalStable(md, 0);
 #endif
    pthread_mutex_unlock(&gDevice_mutex);
    return isSignalStable;
}

//=============================================================================
/**
 * GetDeviceInfo for Sensors
 */
//=============================================================================

void
ithCapGetDeviceInfo(
    CAPTURE_HANDLE *ptDev)
{
    pthread_mutex_lock(&gDevice_mutex);
#ifdef CFG_SENSOR_ENABLE      
    MMP_BOOL    matchResolution = MMP_FALSE;
    MMP_UINT32  i               = 0;
    CAP_CONTEXT *Capctxt        = &ptDev->cap_info;

    Capctxt->ininfo.capwidth   = ithSensorGetProperty(md,  GetWidth);
    Capctxt->ininfo.capheight  = ithSensorGetProperty(md,  GetHeight);
    Capctxt->ininfo.Interleave = ithSensorGetProperty(md,  GetModuleIsInterlace);

    switch (ithSensorGetProperty(md, Rate))
    {
    case 2397:
        Capctxt->ininfo.framerate = MMP_CAP_FRAMERATE_23_97HZ;
        break;
    case 2400:
        Capctxt->ininfo.framerate = MMP_CAP_FRAMERATE_24HZ;
        break;
    case 2500:
        Capctxt->ininfo.framerate = MMP_CAP_FRAMERATE_25HZ;
        break;
    case 2997:
        Capctxt->ininfo.framerate = MMP_CAP_FRAMERATE_29_97HZ;
        break;
    case 3000:
        Capctxt->ininfo.framerate = MMP_CAP_FRAMERATE_30HZ;
        break;
    case 5000:
        Capctxt->ininfo.framerate = MMP_CAP_FRAMERATE_50HZ;
        break;
    case 5994:
        Capctxt->ininfo.framerate = MMP_CAP_FRAMERATE_59_94HZ;
        break;
    case 6000:
        Capctxt->ininfo.framerate = MMP_CAP_FRAMERATE_60HZ;
        break;
    default:
        Capctxt->ininfo.framerate = MMP_CAP_FRAMERATE_UNKNOW;
        break;
    }

    for (i = 0; i < ITH_COUNT_OF(VIDEO_TABLE); i++)
    {
        if ((Capctxt->ininfo.capwidth == VIDEO_TABLE[i].HActive) &&
            (Capctxt->ininfo.capheight == VIDEO_TABLE[i].VActive) &&
            (Capctxt->ininfo.framerate == VIDEO_TABLE[i].FrameRate))
        {
            /* Set input ROI info */
            Capctxt->ininfo.ROIPosX     = VIDEO_TABLE[i].ROIPosX;
            Capctxt->ininfo.ROIPosY     = VIDEO_TABLE[i].ROIPosY;
            Capctxt->ininfo.ROIWidth    = VIDEO_TABLE[i].ROIWidth;
            Capctxt->ininfo.ROIHeight   = VIDEO_TABLE[i].ROIHeight;
            Capctxt->inmux_info.UCLKDly = VIDEO_TABLE[i].ClkDelay;
            matchResolution             = MMP_TRUE;
            gSensorResolution           = i;
            break;
        }
    }

    /* Default color depth 8 bits */
    Capctxt->YUVinfo.ColorDepth    = COLOR_DEPTH_8_BITS;
    
    /* RGBTOYUV  FUNCTION */
    Capctxt->funen.EnCSFun         = MMP_FALSE;

    /* YUV Pitch */
    Capctxt->ininfo.PitchY         = Capctxt->ininfo.capwidth;
    Capctxt->ininfo.PitchUV        = Capctxt->ininfo.capwidth;

    /* Set output info */
    Capctxt->outinfo.OutWidth      = Capctxt->ininfo.ROIWidth;
    Capctxt->outinfo.OutHeight     = Capctxt->ininfo.ROIHeight;
    Capctxt->outinfo.OutAddrOffset = 0;
    Capctxt->outinfo.OutMemFormat  = SEMI_PLANAR_420;

    Capctxt->bMatchResolution      = matchResolution;
#endif    
    pthread_mutex_unlock(&gDevice_mutex);

}

//=============================================================================
/**
 * Get devices Property
 */
//=============================================================================

MMP_UINT16 
ithCapDeviceGetProperty(DEVICESPROPERTYS option)
{
    MMP_UINT16 info = 0;
    pthread_mutex_lock(&gDevice_mutex);
    switch (option)
    {
        case DEVICES_TABLEINDEX:
            info = gSensorResolution;
            break;
#ifdef CFG_SENSOR_ENABLE              
        case DEVICES_WIDTH:
            info = ithSensorGetProperty(md,  GetWidth);
            break;
        case DEVICES_HEIGHT:
            info = ithSensorGetProperty(md,  GetHeight);
            break;
        case DEVICES_ISINTERLANCED:
            info = ithSensorGetProperty(md,  GetModuleIsInterlace);
            break;
        case DEVICES_FRAMETRATE:
            info = ithSensorGetProperty(md, Rate);
            break;
#endif
        default:
            break;
    } 
    pthread_mutex_unlock(&gDevice_mutex);

    return info;
    
}


//=============================================================================
/**
 * Device LED ON/OFF
 */
//=============================================================================

void
ithCapDeviceLEDON(
    MMP_BOOL enable)
{
    pthread_mutex_lock(&gDevice_mutex);
#ifdef CFG_SENSOR_ENABLE
    if (enable)
        ithSensorSetProperty(md, LEDON, 0);
    else
        ithSensorSetProperty(md, LEDOFF, 0);
#endif
    pthread_mutex_unlock(&gDevice_mutex);
}

