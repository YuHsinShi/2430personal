/** @file
 * PAL ARMLite CPU functions.
 *
 * @author Kevin Chen
 * @version 1.0
 * @date 2019/10/01
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
#include <errno.h>
#include "openrtos/FreeRTOS.h"
#include "arm_lite_dev/armlite_dev_device.h"


#ifdef CFG_ARMLITE_OPUS_CODEC
extern ITPDevice itpDeviceOpusCodec;
#endif

typedef struct
{
    int         engineMode;
    ITPDevice*  ptDevice;
} ARMLITE_ENGINE;

static pthread_mutex_t gArmLiteMutex  = PTHREAD_MUTEX_INITIALIZER;
static ARMLITE_ENGINE gtCurDevice = { 0 };

static ARMLITE_ENGINE gptArmLiteEngineArray[] =
{
#ifdef CFG_ARMLITE_OPUS_CODEC
    {ARMLITE_OPUS_CODEC, &itpDeviceOpusCodec},
#endif
};

static int armLiteIoctl(int file, unsigned long request, void *ptr, void *info)
{
    int i = 0;
    int newEngineMode = 0;
    ITPDevice *pNewDevice = NULL;
    int ret = 0;
    
    pthread_mutex_lock(&gArmLiteMutex);
    switch (request)
    {
    	case ITP_IOCTL_ARMLITE_SWITCH_ENG:
    	    //Engine is running, therefore, need to reload new engine image.
    	    newEngineMode = *(int*)ptr;
            if (gtCurDevice.engineMode == ARMLITE_UNKNOWN_DEVICE || gtCurDevice.engineMode != newEngineMode)
            {
                iteRiscResetCpu(ARMLITE_CPU);
                for (i = 0; i < sizeof(gptArmLiteEngineArray) / sizeof(ARMLITE_ENGINE); i++)
                {
                    if (newEngineMode == gptArmLiteEngineArray[i].engineMode)
                    {
                        pNewDevice = gptArmLiteEngineArray[i].ptDevice;
                        gtCurDevice.ptDevice = pNewDevice;
                        break;
                    }
                }

                if (i == sizeof(gptArmLiteEngineArray) / sizeof(ARMLITE_ENGINE))
                {
                    ithPrintf("itp_armLiteDev.c(%d), requested ARMLite engine is not exited\n", __LINE__);
                }
            }
    		break;
        default:
            ret = gtCurDevice.ptDevice->ioctl(file, request, ptr, info);
            break;
    }    
    pthread_mutex_unlock(&gArmLiteMutex);
    return ret;
}

static int armLiteRead(int file, char *ptr, int len, void* info)
{
    int ret = 0;
    if (gtCurDevice.ptDevice)
    {
        pthread_mutex_lock(&gArmLiteMutex);
        ret =  gtCurDevice.ptDevice->read(file, ptr, len, info);
        pthread_mutex_unlock(&gArmLiteMutex);
    }
    return ret;
}

static int armLiteWrite(int file, char *ptr, int len, void* info)
{
    int ret = 0;
    if (gtCurDevice.ptDevice)
    {
        pthread_mutex_lock(&gArmLiteMutex);
        ret =  gtCurDevice.ptDevice->write(file, ptr, len, info);
        pthread_mutex_unlock(&gArmLiteMutex);
    }
    return ret;
}

static int armLiteSeek(int file, int ptr, int dir, void *info)
{
    int ret = 0;

    if (gtCurDevice.ptDevice)
    {
        pthread_mutex_lock(&gArmLiteMutex);
        ret =  gtCurDevice.ptDevice->lseek(file, ptr, dir, info);
        pthread_mutex_unlock(&gArmLiteMutex);
    }
    return ret;
}

const ITPDevice itpDeviceArmLite =
{
    ":armLite",
    itpOpenDefault,
    itpCloseDefault,
    armLiteRead,
    armLiteWrite,
    armLiteSeek,
    armLiteIoctl,
    NULL
};
