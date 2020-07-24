#include <string.h> 
#include "capture_config.h"
#include "capture_hw.h"
#include "capture.h"
#include "mmp_capture.h"


//=============================================================================
//                Constant Definition
//=============================================================================
#define CAPCONFIGNUM 64
#define CEA_TIMING_RANG 2
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================
MMP_UINT32 CAPConfigTable[] =
{
#include "defcap_config.h"
};

MMP_UINT16        gCapinitNum;                                    //record how many device opened
pthread_mutex_t   gCap_mutex = PTHREAD_MUTEX_INITIALIZER;      //for gCapinitNum mutex protect
MMP_UINT32*       gCap0Config = CAPConfigTable;                   //cap0 config


//=============================================================================
//                Private Function Definition
//=============================================================================
MMP_INLINE MMP_BOOL _CHECK_RANGE(MMP_UINT32 IN, MMP_UINT32 STD, MMP_UINT32 range)
{
    if(IN > (STD + range) || IN < (STD - range))
        return MMP_FALSE; 
    else
        return MMP_TRUE;
}

//=============================================================================
//                Public Function Definition
//=============================================================================
//=============================================================================
/**
 * @brief Cap context initialization.
 * @param  none.
 * @return  MMP_RESULT,init success or fail
 */
//=============================================================================

MMP_RESULT
ithCapInitialize(
    void)
{
    MMP_RESULT  result   = MMP_SUCCESS;
    pthread_mutex_lock(&gCap_mutex);

    ithCap_Set_Reset();
    //ithCap_Set_Register_Reset();
    //ithCapHWICInit();

    gCapinitNum = 0;  
    
    pthread_mutex_unlock(&gCap_mutex);
    return result;
}

//=============================================================================
/**
 * @brief Cap connect source and capinfo default init.
 * @param *ptDev,points to capture_handle structure.
 * @param info, onfly mode ,memory mode, max width, max height param. 
 * @return MMP_RESULT,connect success or fail.
 */
//=============================================================================
MMP_RESULT ithCapConnect(
    CAPTURE_HANDLE *ptDev, CAPTURE_SETTING info)
{
    MMP_RESULT result = MMP_SUCCESS;
 
    pthread_mutex_lock(&gCap_mutex);

 
    if (ptDev == NULL || gCapinitNum >= CAP_DEVICE_ID_MAX)
    {
        result = MMP_RESULT_ERROR;
        goto end;
    }
 
    /* reset caphandle mem */
    memset((void *)ptDev, 0, sizeof(CAPTURE_HANDLE));
 
    /* count capture be init num */
    gCapinitNum++;
    
    /* cap id setting */
    ptDev->cap_id = gCapinitNum - 1;
 
    /* default capinfo setting */
    Cap_Initialize(&ptDev->cap_info);
    
    /* capture set user config*/
    if(ptDev->cap_id == CAP_DEV_ID0)
        Cap_SetUserConfig(ptDev, gCap0Config);
    /* frontend source id setting */
    ptDev->source_id = info.inputsource;
    
    /* capture onflymode flag setting */
    if (info.OnflyMode_en)
    {   
        ptDev->cap_info.EnableOnflyMode = MMP_TRUE;//onfly mode
    }
    else
    {
        ptDev->cap_info.EnableOnflyMode = MMP_FALSE;
        Cap_Memory_Initialize(ptDev, info);//memory mode
    }
 
    /* capture interrupt flag setting */
    if (info.Interrupt_en)
        ptDev->cap_info.EnableInterrupt = MMP_TRUE;
    else
        ptDev->cap_info.EnableInterrupt = MMP_FALSE;
    
    /* setting mem pitchY pitchUV */
    ptDev->cap_info.ininfo.PitchY = info.Max_Width;
    ptDev->cap_info.ininfo.PitchUV = info.Max_Width;
    
end:
    pthread_mutex_unlock(&gCap_mutex);
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s error %d", __FUNCTION__, __LINE__);
    return result;
}

//=============================================================================
/**
 * @brief Cap disconnect source include disable engine(unfire) ,free memory ,and reset cap_handle .
 * @param *ptDev,points to capture_handle structure.
 * @return  MMP_RESULT,disconnect success or fail.
 */
//=============================================================================
MMP_RESULT ithCapDisConnect(CAPTURE_HANDLE *ptDev)
{
    MMP_RESULT result = MMP_SUCCESS;
    pthread_mutex_lock(&gCap_mutex);
    if (ptDev == NULL)
    {
        result = MMP_RESULT_ERROR;
        goto end;
    }
    
    if(ithCap_Get_IsFire(ptDev->cap_id))
    {
         ithCap_Set_UnFire(ptDev->cap_id);
         result = ithCap_Get_WaitEngineIdle(ptDev->cap_id);
         if (result)
         {
             cap_msg_ex(CAP_MSG_TYPE_ERR, " err 0x%x !\n", result);
             goto end;
         }
         ithCap_Set_Clean_Intr(ptDev->cap_id);
    }
    /*memory mode*/ 
    if(!ptDev->cap_info.EnableOnflyMode && ptDev->cap_info.EnableInterrupt)
    {
        Cap_Memory_Clear(ptDev);
    }
    
    /* reset caphandle mem */
    memset((void *)ptDev, 0, sizeof(CAPTURE_HANDLE));
    
    if (gCapinitNum > 0)
        gCapinitNum--;
end:
    pthread_mutex_unlock(&gCap_mutex);
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s error %d", __FUNCTION__, __LINE__);
    
    return result;
}

//=============================================================================
/**
 * @brief Cap terminate all engine,include disable engine(unfire)  and reset engine.
 * @param  none.
 * @return  MMP_RESULT,1 => fail 0 => success
 */
//=============================================================================

MMP_RESULT
ithCapTerminate(
    void)
{
    MMP_RESULT  result   = MMP_SUCCESS;
    MMP_UINT16  index   = 0;
    pthread_mutex_lock(&gCap_mutex);
    /*Disable Cap all engine*/
    for (index = 0; index < CAP_DEVICE_ID_MAX; index++)
    {
        if(ithCap_Get_IsFire(index))
        {
            ithCap_Set_UnFire(index);
            result = ithCap_Get_WaitEngineIdle(index);
            if (result)
            {
                cap_msg_ex(CAP_MSG_TYPE_ERR, " err 0x%x !\n", result);
                goto end;
            }
        }
        ithCap_Set_Clean_Intr(index);
    }
    
    ithCap_Set_Reset();
end:
    pthread_mutex_unlock(&gCap_mutex);
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return result;
}

//=============================================================================
/**
 * @brief Cap current fire status.
 * @param *ptDev,points to capture_handle structure.
 * @return true => fire(running) , false => unfire(stop)
 */
//=============================================================================
MMP_BOOL
ithCapIsFire(
    CAPTURE_HANDLE *ptDev)
{
    MMP_BOOL status = 0;
    pthread_mutex_lock(&gCap_mutex);
    status = ithCap_Get_IsFire(ptDev->cap_id);
    pthread_mutex_unlock(&gCap_mutex);
    return status;
}

//=============================================================================
/**
 * @brief Cap Get EngineErrorStatus [this function can be called in ISR].
 * @param *ptDev,points to capture_handle structure.
 * @param lanenum,LANE0 => reg[0x200].
 * @return
 * bit[3:0]Stable status:
 * [0]:Hsync stable
 * [1]:Vsync stable
 * [2]:DE stable(X)
 * [3]:DE stable(Y)
 * bit[11:8] Error status:
 * [1]:Hsync loss
 * [2]:Vsync loss
 * [3]:DE loss
 * [4]:frame end error
 * [5]:capture overflow
 * [7]:frame rate change
 * [8]:time out
 */
//=============================================================================
MMP_UINT32
ithCapGetEngineErrorStatus(
    CAPTURE_HANDLE *ptDev, MMP_CAP_LANE_STATUS lanenum)
{
    MMP_UINT32 err_status = 0;

    err_status = ithCap_Get_Lane_status(ptDev->cap_id, lanenum);

    return err_status;
}

//=============================================================================
/**
 * @brief Cap Set parameter to hw registers. 
 * @param *ptDev,points to capture_handle structure.
 * @return  MMP_RESULT,1 => fail 0 => success.
 */
//=============================================================================
MMP_RESULT
ithCapParameterSetting(
    CAPTURE_HANDLE *ptDev)
{
    MMP_RESULT  result   = MMP_SUCCESS;
    pthread_mutex_lock(&gCap_mutex);
    if (ptDev == MMP_NULL)
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, "Capture not initialize\n");
        result = MMP_RESULT_ERROR;
        goto end;
    }

    //Update parameter
    result = Cap_Update_Reg(ptDev);

end:
    pthread_mutex_unlock(&gCap_mutex);
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) ERROR !!!!\n", __FUNCTION__, __LINE__);

    return result;
}

//=============================================================================
/**
 * @brief Set Cap fire or not.
 * @param *ptDev,points to capture_handle structure.
 * @param enable,true => fire, false => unfire.
 */
//=============================================================================
void
ithCapFire(
    CAPTURE_HANDLE *ptDev, MMP_BOOL enable)
{
    pthread_mutex_lock(&gCap_mutex);
    if(enable)
    {
        ithCap_Set_MemThreshold(ptDev->cap_id, ptDev->cap_info.ininfo.WrMergeThresld);
        // Update Error Handle mode
#ifdef DIGITAL_SENSOR_DEV
        ithCap_Set_Error_Handleing(ptDev->cap_id,0xFFFF);
#endif
#ifdef ANALOG_SENSOR_DEV
        ithCap_Set_Error_Handleing(ptDev->cap_id,0xFFFC);
#endif

        ithCap_Set_Wait_Error_Reset(ptDev->cap_id);

        ithCap_Set_Fire(ptDev->cap_id);
    }
    else
    {
        ithCap_Set_UnFire(ptDev->cap_id);
    }
    pthread_mutex_unlock(&gCap_mutex);
}
//=============================================================================
/**
 * @brief Register  an interrupt handler[Only OPENRTOS]
 * @param caphandler,user define caphandler
 * @param *ptDev,points to capture_handle structure.
 * @return none.
 */
//=============================================================================
void
ithCapRegisterIRQ(
    ITHIntrHandler caphandler, CAPTURE_HANDLE *ptDev)
{
    // Initialize Capture IRQ
    ithIntrDisableIrq(ITH_INTR_CAPTURE);
    ithIntrClearIrq(ITH_INTR_CAPTURE);
#if defined (__OPENRTOS__)
    // register NAND Handler to IRQ
    ithIntrRegisterHandlerIrq(ITH_INTR_CAPTURE, caphandler, (void *)ptDev);
#endif     // defined (__OPENRTOS__)

    // set IRQ to edge trigger
    ithIntrSetTriggerModeIrq(ITH_INTR_CAPTURE, ITH_INTR_EDGE);

    // set IRQ to detect rising edge
    ithIntrSetTriggerLevelIrq(ITH_INTR_CAPTURE, ITH_INTR_HIGH_RISING);

    // Enable IRQ
    ithIntrEnableIrq(ITH_INTR_CAPTURE);
}

//=============================================================================
/**
 * @brief Cap disable IRQ[Only OPENRTOS]
 * @param none.
 * @return none.
 */
//=============================================================================
void
ithCapDisableIRQ(
    void)
{
    // Initialize Capture IRQ
    ithIntrDisableIrq(ITH_INTR_CAPTURE);
    ithIntrClearIrq(ITH_INTR_CAPTURE);
}

//=============================================================================
/**
 * @brief Clear Interrupt [this function can be called in ISR]
 * @param *ptDev,points to capture_handle structure.
 * @return 0.
 */
//=============================================================================

MMP_UINT16
ithCapClearInterrupt(
    CAPTURE_HANDLE *ptDev, MMP_BOOL get_err)
{
    if (get_err)
    {
        ithCap_Set_ErrReset(ptDev->cap_id);
        //ithCap_Set_Error_Handleing(ptDev->cap_id, 0x0);
    }

    ithCap_Set_Clean_Intr(ptDev->cap_id);
    return 0;
}

//=============================================================================
/**
 * @brief Get current number of memory ring buffer [this function can be called in ISR]
 * @param *ptDev,points to capture_handle structure.
 * @return current buffer number.
 */
//=============================================================================

MMP_UINT16
ithCapReturnWrBufIndex(
    CAPTURE_HANDLE *ptDev)
{
    MMP_UINT16  CapWrBufIndex = 0;
    CapWrBufIndex = ((ithCap_Get_Lane_status(ptDev->cap_id, CAP_LANE0_STATUS) & 0x70) >> 4);
    return CapWrBufIndex;
}

//=============================================================================
/**
 * @brief Get current input Source Frame Rate(Note: need wait capture 1 frame )
 * @param *ptDev,points to capture_handle structure.     
 * @return MMP_CAP_FRAMERATE, hw dectected current frame rate.
 */
//=============================================================================
MMP_CAP_FRAMERATE
ithCapGetInputFrameRate(
    CAPTURE_HANDLE *ptDev)
{
    MMP_UINT32  RawVTotal = 0;
    MMP_UINT16  FrameRate_mode = 0;
    MMP_UINT32  MCLK_Freq = 0;
    MMP_UINT32  framerate = 0;
    MMP_UINT32  div = 0;
    MMP_UINT32  interlaced = 0;
    pthread_mutex_lock(&gCap_mutex);
    RawVTotal = ithCap_Get_MRawVTotal(ptDev->cap_id);
    interlaced = ithCap_Get_Detected_Interleave(ptDev->cap_id);
    if(RawVTotal > 0)
    {
        if(interlaced)
            RawVTotal = RawVTotal / 2;
        
        div    = ithReadRegA(0xD8000014) & 0xF;
        MCLK_Freq = 792 / div;
        framerate = ((3906.25 * MCLK_Freq * 1000) /RawVTotal);
        //printf("RawVTotal = %d MCLK_Freq = %d framerate = %d\n", RawVTotal, MCLK_Freq, framerate);
    }

    if ((23988 > framerate) && (framerate > 23946))       // 23.976fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_23_97HZ;
    }
    else if ((24030 > framerate) && (framerate > 23987))  // 24fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_24HZ;
    }
    else if ((25030 > framerate) && (framerate > 24970))  // 25fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_25HZ;
    }
    else if ((29985 > framerate) && (framerate > 29940))  // 29.97fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_29_97HZ;
    }
    else if ((30030 > framerate) && (framerate > 29984))  // 30fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_30HZ;
    }
    else if ((50030 > framerate) && (framerate > 49970))  // 50fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_50HZ;
    }
    else if ((57000 > framerate) && (framerate > 55000))  // 56fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_56HZ;
    }
    else if ((59970 > framerate) && (framerate > 57001))  // 59.94fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_59_94HZ;
    }
    else if ((62030 > framerate) && (framerate > 59969))  // 60fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_60HZ;
    }
    else if ((70999 > framerate) && (framerate > 69000))  // 70fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_70HZ;
    }
    else if ((73000 > framerate) && (framerate > 71000))  // 72fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_72HZ;
    }
    else if ((76000 > framerate) && (framerate > 74000))  // 75fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_75HZ;
    }
    else if ((86000 > framerate) && (framerate > 84000))  // 85fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_85HZ;
    }
    else
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_UNKNOW;
    }
    //printf("frame rate mode = %d \n",FrameRate_mode);
    pthread_mutex_unlock(&gCap_mutex);
    return FrameRate_mode;
}

//=============================================================================
/**
 * @brief Get input source info.
 * @param *ptDev,points to capture_handle structure.
 * @return MMP_CAP_INPUT_INFO.
 */
//=============================================================================
MMP_CAP_INPUT_INFO
ithCapGetInputSrcInfo(
    CAPTURE_HANDLE *ptDev)
{
    MMP_CAP_INPUT_INFO info;
    MMP_CAP_FRAMERATE  frameRateMode;
    MMP_UINT32 width, height, interlanced;
    CAP_CONTEXT        *Capctxt = &ptDev->cap_info;
    frameRateMode = ithCapGetInputFrameRate(ptDev);
    pthread_mutex_lock(&gCap_mutex);
    width         = ithCap_Get_Detected_Width(ptDev->cap_id);
    height        = ithCap_Get_Detected_Height(ptDev->cap_id);
    interlanced   = ithCap_Get_Detected_Interleave(ptDev->cap_id);
        
    switch (frameRateMode)
    {
    case MMP_CAP_FRAMERATE_23_97HZ:
        if (_CHECK_RANGE(width,1920,CEA_TIMING_RANG) && _CHECK_RANGE(height,1080,CEA_TIMING_RANG))
            info = MMP_CAP_INPUT_INFO_1920X1080_23P;
        else
            info = MMP_CAP_INPUT_INFO_UNKNOWN;
        break;

    case MMP_CAP_FRAMERATE_24HZ:
        if (_CHECK_RANGE(width,1920,CEA_TIMING_RANG) && _CHECK_RANGE(height,1080,CEA_TIMING_RANG))
            info = MMP_CAP_INPUT_INFO_1920X1080_24P;
        else
            info = MMP_CAP_INPUT_INFO_UNKNOWN;
        break;

    case MMP_CAP_FRAMERATE_25HZ:
        if (_CHECK_RANGE(width,1920,CEA_TIMING_RANG) && _CHECK_RANGE(height,1080,CEA_TIMING_RANG))
            info = MMP_CAP_INPUT_INFO_1920X1080_25P;
        else
            info = MMP_CAP_INPUT_INFO_UNKNOWN;
        break;

    case MMP_CAP_FRAMERATE_29_97HZ:
        if (_CHECK_RANGE(width,1920,CEA_TIMING_RANG) && _CHECK_RANGE(height,1080,CEA_TIMING_RANG))
            info = MMP_CAP_INPUT_INFO_1920X1080_29P;
        else
            info = MMP_CAP_INPUT_INFO_UNKNOWN;
        break;

    case MMP_CAP_FRAMERATE_30HZ:
        if (_CHECK_RANGE(width,1920,CEA_TIMING_RANG) && _CHECK_RANGE(height,1080,CEA_TIMING_RANG))
            info = MMP_CAP_INPUT_INFO_1920X1080_30P;
        else
            info = MMP_CAP_INPUT_INFO_UNKNOWN;
        break;

    case MMP_CAP_FRAMERATE_50HZ:
        if (_CHECK_RANGE(width,720,CEA_TIMING_RANG) && _CHECK_RANGE(height,576,CEA_TIMING_RANG))
        {
            if (interlanced)
                info = MMP_CAP_INPUT_INFO_720X576_50I;
            else
                info = MMP_CAP_INPUT_INFO_720X576_50P;
        }
        else if (_CHECK_RANGE(width,1280,CEA_TIMING_RANG) && _CHECK_RANGE(height,720,CEA_TIMING_RANG))
        {
            info = MMP_CAP_INPUT_INFO_1280X720_50P;
        }
        else if (_CHECK_RANGE(width,1920,CEA_TIMING_RANG) && _CHECK_RANGE(height,1080,CEA_TIMING_RANG))
        {
            if (interlanced)
                info = MMP_CAP_INPUT_INFO_1920X1080_50I;
            else
                info = MMP_CAP_INPUT_INFO_1920X1080_50P;
        }
        else
        {
            info = MMP_CAP_INPUT_INFO_UNKNOWN;
        }
        break;

    case MMP_CAP_FRAMERATE_59_94HZ:
       
        if (_CHECK_RANGE(width,720,CEA_TIMING_RANG) && _CHECK_RANGE(height,480,CEA_TIMING_RANG))
        {
            if (interlanced)
                info = MMP_CAP_INPUT_INFO_720X480_59I;
            else
                info = MMP_CAP_INPUT_INFO_720X480_59P;
        }
        else if (_CHECK_RANGE(width,1280,CEA_TIMING_RANG) && _CHECK_RANGE(height,720,CEA_TIMING_RANG))
        {
            info = MMP_CAP_INPUT_INFO_1280X720_59P;
        }
        else if (_CHECK_RANGE(width,1920,CEA_TIMING_RANG) && _CHECK_RANGE(height,1080,CEA_TIMING_RANG))
        {
            if (interlanced)
                info = MMP_CAP_INPUT_INFO_1920X1080_59I;
            else
                info = MMP_CAP_INPUT_INFO_1920X1080_59P;
        }
        else
        {
           info = MMP_CAP_INPUT_INFO_UNKNOWN;
        }
        break;

    case MMP_CAP_FRAMERATE_60HZ:
        if (_CHECK_RANGE(width,720,CEA_TIMING_RANG) && _CHECK_RANGE(height,480,CEA_TIMING_RANG))
        {
            if (interlanced)
                info = MMP_CAP_INPUT_INFO_720X480_60I;
            else
                info = MMP_CAP_INPUT_INFO_720X480_60P;
        }
        else if (_CHECK_RANGE(width,1280,CEA_TIMING_RANG) && _CHECK_RANGE(height,720,CEA_TIMING_RANG))
        {
            info = MMP_CAP_INPUT_INFO_1280X720_60P;
        }     
        else if (_CHECK_RANGE(width,1920,CEA_TIMING_RANG) && _CHECK_RANGE(height,1080,CEA_TIMING_RANG))
        {
            if (interlanced)
                info = MMP_CAP_INPUT_INFO_1920X1080_60I;
            else
                info = MMP_CAP_INPUT_INFO_1920X1080_60P;
        }
        else
        {
            info = MMP_CAP_INPUT_INFO_UNKNOWN;
        }
        break;

    default:
        info = MMP_CAP_INPUT_INFO_UNKNOWN;
        break;
    }
    pthread_mutex_unlock(&gCap_mutex);
    return info;
}

//=============================================================================
/**
 * @brief ithAVSyncCounterInit
 */
//=============================================================================
void
ithAVSyncCounterCtrl(CAPTURE_HANDLE *ptDev, AV_SYNC_COUNTER_CTRL mode, MMP_UINT16 divider)
{
    pthread_mutex_lock(&gCap_mutex);
    ithAVSync_CounterCtrl(ptDev->cap_id, mode, divider);
    pthread_mutex_unlock(&gCap_mutex);
}

//=============================================================================
/**
 * @brief ithAVSyncCounterReset
 */
//=============================================================================
void
ithAVSyncCounterReset(CAPTURE_HANDLE *ptDev, AV_SYNC_COUNTER_CTRL mode)
{
    pthread_mutex_lock(&gCap_mutex);
    ithAVSync_CounterReset(ptDev->cap_id, mode);
    pthread_mutex_unlock(&gCap_mutex);
}

//=============================================================================
/**
 * @brief ithAVSyncCounterRead
 */
//=============================================================================
MMP_UINT32
ithAVSyncCounterRead(CAPTURE_HANDLE *ptDev, AV_SYNC_COUNTER_CTRL mode)
{
    MMP_UINT32 counter_read = 0;

    pthread_mutex_lock(&gCap_mutex);
    counter_read = ithAVSync_CounterRead(ptDev->cap_id, mode);
    pthread_mutex_unlock(&gCap_mutex);

    return counter_read;
}

//=============================================================================
/**
 * @brief ithAVSyncMuteDetect
 */
//=============================================================================
MMP_BOOL
ithAVSyncMuteDetect(CAPTURE_HANDLE *ptDev)
{
    MMP_BOOL mute = 0;
    pthread_mutex_lock(&gCap_mutex);
    mute = ithAVSync_MuteDetect(ptDev->cap_id);
    pthread_mutex_unlock(&gCap_mutex);
    return mute;
}

//=============================================================================
/**
 * @brief power up cap controler.
 * @param void.
 * @return void.
 */
//=============================================================================
void
ithCapPowerUp(
    void)
{
    pthread_mutex_lock(&gCap_mutex);
    ithCap_Set_Reset();
    ithCapEnableClock();
    pthread_mutex_unlock(&gCap_mutex);
}

//=============================================================================
/**
 * @brief power down cap controler.
 * @param void.
 * @return void.
 */
//=============================================================================
void
ithCapPowerDown(
    void)
{
    MMP_UINT16  index   = 0;
    MMP_RESULT  result   = MMP_SUCCESS;
    pthread_mutex_lock(&gCap_mutex);
    for (index = 0; index < CAP_DEVICE_ID_MAX; index++)
    {
        if(ithCap_Get_IsFire(index))
        {
            ithCap_Set_UnFire(index);
            result = ithCap_Get_WaitEngineIdle(index);
            if (result)
            {
                cap_msg_ex(CAP_MSG_TYPE_ERR, " err 0x%x !\n", result);
                goto end;
            }
        }
        ithCap_Set_Clean_Intr(index);
    }
    ithCapDisableClock();
end:
    pthread_mutex_unlock(&gCap_mutex);
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);    
}

//=============================================================================
/**
 * @brief Cap Get Detected Width
 * @param *ptDev,points to capture_handle structure.
 * @return Detected Width.
 */
//=============================================================================
MMP_UINT32
ithCapGetDetectedWidth(
    CAPTURE_HANDLE *ptDev)
{
    MMP_UINT32 width = 0;
    pthread_mutex_lock(&gCap_mutex);
    width = ithCap_Get_Detected_Width(ptDev->cap_id);
    pthread_mutex_unlock(&gCap_mutex);
    return width;
}

//=============================================================================
/**
 * @brief Cap Get Detected Height
 * @param *ptDev,points to capture_handle structure.
 * @return Detected Height.
 */
//=============================================================================
MMP_UINT32
ithCapGetDetectedHeight(
    CAPTURE_HANDLE *ptDev)
{
    MMP_UINT32 height = 0;
    pthread_mutex_lock(&gCap_mutex);
    height = ithCap_Get_Detected_Height(ptDev->cap_id);
    pthread_mutex_unlock(&gCap_mutex);
    return height;
}

//=============================================================================
/**
 * @brief Cap Get Detected Interleave
 * @param *ptDev,points to capture_handle structure.
 * @return Detected Interleave.
 */
//=============================================================================
MMP_UINT32
ithCapGetDetectedInterleave(
    CAPTURE_HANDLE *ptDev)
{
    MMP_UINT32 interlaced = 0;
    pthread_mutex_lock(&gCap_mutex);
    interlaced = ithCap_Get_Detected_Interleave(ptDev->cap_id);
    pthread_mutex_unlock(&gCap_mutex);
    return interlaced;
}

//=============================================================================
/**
 * @brief Set cap interleave mode.
 * @param *ptDev,points to capture_handle structure.
 * @return void.
 */
//=============================================================================
void
ithCapSetInterleave(
    CAPTURE_HANDLE *ptDev, MMP_UINT32 interleave)
{
    pthread_mutex_lock(&gCap_mutex);
    ithCap_Set_Interleave(ptDev->cap_id, interleave);
    pthread_mutex_unlock(&gCap_mutex);
}

