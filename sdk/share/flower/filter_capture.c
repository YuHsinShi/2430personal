#include <stdio.h>
#include "flower.h"
#include "capture/capture_9860/mmp_capture.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define SIGNALCHECK_TIMEOUT 100//10ms

CAPTURE_HANDLE   gCapDev0;
static int       gMaxWidth  = 0;
static int       gMaxHeight = 0;
static IteFilter *f_backup  = NULL;

//=============================================================================
//                              Private Function Declaration
//=============================================================================
static void _CAP_ISR0(void *arg)
{
    uint32_t       capture0state = 0, bufindex = 0;
    BaseType_t     HigherPriorityTaskWoken = (BaseType_t)0;
    CAPTURE_HANDLE *ptDev                   = (CAPTURE_HANDLE *) arg;
    IteQueueblk    blk                      = {0};

    capture0state = ithCapGetEngineErrorStatus(ptDev, MMP_CAP_LANE0_STATUS);

    if (capture0state >> 31)            //interrupt flag
    {
        if ((capture0state >> 8) & 0xF) //error flag
        {
            ithPrintf("_CAP_ISR0=0x%x\n", capture0state);
            //clear cap0 interrupt and reset error status
            ithCapClearInterrupt(ptDev, MMP_TRUE);
        }
        else
        {
            //ithPrintf("cap0_isr frame end\n");
            bufindex              = ithCapReturnWrBufIndex(ptDev);
            blk.VInfo.Width       = (unsigned int)ptDev->cap_info.outinfo.OutWidth;
            blk.VInfo.Height      = (unsigned int)ptDev->cap_info.outinfo.OutHeight;
            blk.VInfo.PitchY      = (unsigned int)ptDev->cap_info.ininfo.PitchY;
            blk.VInfo.PitchU      = (unsigned int)ptDev->cap_info.ininfo.PitchUV;
            blk.VInfo.PitchV      = blk.VInfo.PitchU;
            blk.VInfo.DataAddrY   = (unsigned int)ptDev->cap_info.OutAddrY[bufindex];
            blk.VInfo.DataAddrU   = (unsigned int)ptDev->cap_info.OutAddrUV[bufindex];
            blk.VInfo.DataAddrV   = blk.VInfo.DataAddrU;
            blk.VInfo.Interlanced = ptDev->cap_info.ininfo.Interleave;
            
            ite_queue_put_fromISR(f_backup->output[0].Qhandle, &blk, HigherPriorityTaskWoken);

            //clear cap0 interrupt
            ithCapClearInterrupt(ptDev, MMP_FALSE);
        }
        portYIELD_FROM_ISR(HigherPriorityTaskWoken);
    }
}

static int _SIGNALCHECK_FIRE(CAPTURE_HANDLE *ptDev)
{
    int         timeout  = 0;
    while ((ithCapGetEngineErrorStatus(ptDev, MMP_CAP_LANE0_STATUS) & 0x3) != 0x3)
    {
        if (++timeout > SIGNALCHECK_TIMEOUT)
            return 1;
        DEBUG_PRINT("Hsync or Vsync not stable!\n");
        usleep(10000);
    }

    ithCapFire(ptDev, MMP_TRUE);
    DEBUG_PRINT("Capture Fire! (%d)\n", ptDev->cap_id);

    return 0;
}

static void f_capture_init(IteFilter *f)
{
    DEBUG_PRINT("[%s] Filter(%d)\n", __FUNCTION__, f->filterDes.id);

    /*capture clk on*/
    ithCapPowerUp();
    /*capture init*/
    ithCapInitialize();
    /*Sensor init*/
    ithCapDeviceInitialize();
}

static void f_capture_uninit(IteFilter *f)
{
    DEBUG_PRINT("[%s] Filter(%d)\n", __FUNCTION__, f->filterDes.id);
}

static void f_capture_preprocess(IteFilter *f)
{
    DEBUG_PRINT("[%s] Filter(%d)\n", __FUNCTION__, f->filterDes.id);
    if (gMaxWidth != 0 && gMaxHeight != 0)
    {
        CAPTURE_SETTING modeset = {MMP_CAP_DEV_SENSOR, MMP_FALSE,  MMP_TRUE, gMaxWidth, gMaxHeight};
        ithCapConnect(&gCapDev0, modeset);
        ithCapRegisterIRQ(_CAP_ISR0, &gCapDev0);
    }
    else
    {
        DEBUG_PRINT("[%s] Filter(%d) Width or Height Error \n", __FUNCTION__, f->filterDes.id);
    }
}

static void f_capture_process(IteFilter *f)
{
    DEBUG_PRINT("[%s] Filter(%d)\n", __FUNCTION__, f->filterDes.id);
    f_backup = f;

    while (1)
    {
        if (ithCapIsFire(&gCapDev0) == false)
        {
            if (ithCapDeviceIsSignalStable())
            {
                ithCapGetDeviceInfo(&gCapDev0);
                ithCapParameterSetting(&gCapDev0);
                _SIGNALCHECK_FIRE(&gCapDev0);
            }
        }
        usleep(500*1000);
    }

}

static void f_capture_postprocess(IteFilter *f)
{
    DEBUG_PRINT("[%s] Filter(%d)\n", __FUNCTION__, f->filterDes.id);
    ithCapTerminate();
    ithCapDisableIRQ();
    ithCapDisConnect(&gCapDev0);
}

static void f_capture_setwidth(IteFilter *f, void *arg)
{
    DEBUG_PRINT("[%s] Filter(%d) width = (%d)\n", __FUNCTION__, f->filterDes.id, (int)arg);
    gMaxWidth = (int)arg;
}

static void f_capture_setheight(IteFilter *f, void *arg)
{
    DEBUG_PRINT("[%s] Filter(%d) height = (%d)\n", __FUNCTION__, f->filterDes.id, (int)arg);
    gMaxHeight = (int)arg;
}

static IteMethodDes Filter_CAP_methods[] = {
    {ITE_FILTER_CAP_SetWidth,  f_capture_setwidth   },
    {ITE_FILTER_CAP_SetHeight, f_capture_setheight  },
    {                       0, NULL                 }
};

IteFilterDes        FilterCapture = {
    ITE_FILTER_CAP_ID,
    f_capture_init,
    f_capture_uninit,
    f_capture_preprocess,
    f_capture_process,
    f_capture_postprocess,
    Filter_CAP_methods
};