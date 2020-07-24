#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "test_itv.h"       //for VideoInit()
#include "mmp_capture.h"
#ifdef __OPENRTOS__
#include "openrtos/FreeRTOS.h"
#include "openrtos/queue.h"
#endif

typedef enum state_control
{
    NO_SIGNAL                  = 0,
    NO_SIGNAL_TO_SIGNAL_STABLE = 1,
    SIGNAL_STABLE              = 2,
} state_control;

typedef struct {
    int          Capinterlanced;
    unsigned int CapOutWidth;
    unsigned int CapOutHeight;
    unsigned int CapPitchY;
    unsigned int CapPitchUV;
    unsigned int CapaddrY;
    unsigned int CapaddrUV;
} CAPQUEUEINFO;

static CAPTURE_HANDLE gCapDev0;
pthread_t             display_task;
static bool           cap0_get_err = false;
state_control         state        = NO_SIGNAL;
static struct timeval pollS_time1;
#ifdef __OPENRTOS__
QueueHandle_t         CaptoDisplayQueue;
#endif

//=====================================
static uint32_t
_GET_DURATION(
    struct timeval *startT)
{
    struct timeval currT         = {0};
    uint64_t       duration_time = 0;

    gettimeofday(&currT, NULL);
    duration_time  = (currT.tv_sec - startT->tv_sec) * 1000;     // sec to ms
    duration_time += ((currT.tv_usec - startT->tv_usec) / 1000); // us to ms
    return (uint32_t)duration_time;
}

static void
FSM_STATE(bool signal)
{
    if (signal)
    {
        if (state == NO_SIGNAL)
            state = NO_SIGNAL_TO_SIGNAL_STABLE;
        else if (state == NO_SIGNAL_TO_SIGNAL_STABLE)
            state = SIGNAL_STABLE;
        else if (state == SIGNAL_STABLE)
            state = SIGNAL_STABLE;
    }
    else
    {
        state = NO_SIGNAL;
    }
}

static void
_CAP_ISR0(
    void *arg)
{
#ifdef __OPENRTOS__
    uint32_t       capture0state = 0, capindex = 0;
    CAPQUEUEINFO   capqueueinfo;
    BaseType_t     gHigherPriorityTaskWoken = (BaseType_t)0;
    CAPTURE_HANDLE *ptDev                   = (CAPTURE_HANDLE *) arg;

    capture0state = ithCapGetEngineErrorStatus(&gCapDev0, MMP_CAP_LANE0_STATUS);
    //ithPrintf("cap0_isr 0x%x\n", capture0state);

    if (capture0state >> 31)
    {
        if ((capture0state >> 8) & 0xF)
        {
            ithPrintf("_CAP_ISR0=0x%x\n", capture0state);
            cap0_get_err = true;
            //clear cap0 interrupt and reset error status
            ithCapClearInterrupt(&gCapDev0, MMP_TRUE);
        }
        else
        {
            //ithPrintf("cap0_isr frame end\n");
            capindex                    = ithCapReturnWrBufIndex(&gCapDev0);
            capqueueinfo.CapOutWidth    = (unsigned int)gCapDev0.cap_info.outinfo.OutWidth;
            capqueueinfo.CapOutHeight   = (unsigned int)gCapDev0.cap_info.outinfo.OutHeight;
            capqueueinfo.CapPitchY      = (unsigned int)gCapDev0.cap_info.ininfo.PitchY;
            capqueueinfo.CapPitchUV     = (unsigned int)gCapDev0.cap_info.ininfo.PitchUV;
            capqueueinfo.CapaddrY       = (unsigned int)gCapDev0.cap_info.OutAddrY[capindex];
            capqueueinfo.CapaddrUV      = (unsigned int)gCapDev0.cap_info.OutAddrUV[capindex];
            capqueueinfo.Capinterlanced = gCapDev0.cap_info.ininfo.Interleave;

            xQueueSendToBackFromISR(CaptoDisplayQueue, &capqueueinfo, &gHigherPriorityTaskWoken);

            //clear cap0 interrupt
            ithCapClearInterrupt(&gCapDev0, MMP_FALSE);
        }
        portYIELD_FROM_ISR(gHigherPriorityTaskWoken);
    }
#endif
    return;
}

static int
_SIGNALCHECK_FIRE(
    CAPTURE_HANDLE *ptDev)
{
    int         timeout  = 0;
    CAP_CONTEXT *Capctxt = &ptDev->cap_info;

    while ((ithCapGetEngineErrorStatus(ptDev, MMP_CAP_LANE0_STATUS) & 0x3) != 0x3)
    {
        if (++timeout > 100)
            return 1;
        printf("Hsync or Vsync not stable!\n");
        usleep(10 * 1000);
    }

    ithCapFire(ptDev, MMP_TRUE);
    printf("Capture Fire! (%d)\n", ptDev->cap_id);

    return 0;
}

static void *_DisplayThread()
{
#ifdef __OPENRTOS__
    uint8_t           *dbuf        = NULL;
    ITV_DBUF_PROPERTY dbufprop     = {0};
    CAPQUEUEINFO      CAPQUEUEINFO = {0};
    //gettimeofday(&pollS_time1, NULL);
    while (1)
    {
        if (xQueueReceive(CaptoDisplayQueue, &CAPQUEUEINFO, 0))
        {
            dbuf = itp_itv_get_dbuf_anchor();
            if (dbuf != NULL)
            {
                if (CAPQUEUEINFO.Capinterlanced)
                    itp_itv_enable_isp_feature(MMP_ISP_DEINTERLACE);

                dbufprop.src_w    = CAPQUEUEINFO.CapOutWidth;
                dbufprop.src_h    = CAPQUEUEINFO.CapOutHeight;
                dbufprop.pitch_y  = CAPQUEUEINFO.CapPitchY;
                dbufprop.pitch_uv = CAPQUEUEINFO.CapPitchUV;

                dbufprop.format   = MMP_ISP_IN_NV12;
                dbufprop.ya       = (uint8_t  *)CAPQUEUEINFO.CapaddrY;
                dbufprop.ua       = (uint8_t  *)CAPQUEUEINFO.CapaddrUV;
                dbufprop.va       = (uint8_t  *)CAPQUEUEINFO.CapaddrUV;
                //printf("frame gap: %d\n",_GET_DURATION(&pollS_time1));
                //gettimeofday(&pollS_time1, NULL);
                itp_itv_update_dbuf_anchor(&dbufprop);
            }
        }
        usleep(1000);
    }
#endif
}


//Modules test Run(Sensor) peripheral->sensor enable.
void *TestFunc(void *arg)
{
    printf("modules test cap start\n");
    itpInit();

    CaptoDisplayQueue = xQueueCreate(10, (unsigned portBASE_TYPE) sizeof(CAPQUEUEINFO));
    pthread_create(&display_task, NULL, _DisplayThread, NULL);

#ifdef CFG_LCD_ENABLE
    /*lcd panel int*/
    ioctl(ITP_DEVICE_SCREEN,    ITP_IOCTL_POST_RESET, NULL);
    #ifdef CFG_BACKLIGHT_ENABLE
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_RESET,      NULL);
    #endif
    /*itv init*/
    itp_itv_init();
#endif

    ithCapPowerUp();
    /*capture init*/
    ithCapInitialize();
    /*modules init*/
    ithCapDeviceInitialize();

    CAPTURE_SETTING mem_modeset = {MMP_CAP_DEV_SENSOR, MMP_FALSE,  MMP_TRUE, 1280, 720};
    ithCapConnect(&gCapDev0, mem_modeset);
    ithCapRegisterIRQ(_CAP_ISR0, &gCapDev0);

    /*main loop*/
    for (;;)
    {
        FSM_STATE(ithCapDeviceIsSignalStable());

        switch (state)
        {
        case NO_SIGNAL:
            if (cap0_get_err)
            {
                printf("###CAPTURE ERROR###\n");
                ithCapTerminate();
                cap0_get_err = false;
            }
            break;
        case NO_SIGNAL_TO_SIGNAL_STABLE:
            printf("###NO_SIGNAL_TO_SIGNAL_STABLE###\n");
            ithCapGetDeviceInfo(&gCapDev0);
            ithCapParameterSetting(&gCapDev0);
            if (_SIGNALCHECK_FIRE(&gCapDev0))
                FSM_STATE(false);

            break;
        case SIGNAL_STABLE:
            if (cap0_get_err)
            {
                printf("###CAPTURE ERROR###\n");
                ithCapTerminate();
                cap0_get_err = false;
                FSM_STATE(false);
            }
            break;
        default:
            break;
        }
        usleep(200 * 1000);
    }
}
