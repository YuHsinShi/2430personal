#include <string.h>
#include "math.h"
#include "capture_config.h"
#include "capture.h"
#include "capture_hw.h"
#include "capture_util.h"
#include "capture_reg.h"
#include "mmp_capture.h"
//=============================================================================
//                Constant Definition
//=============================================================================
#define CAP_DATA_BUS_WIDTH 12
#define CAP_VDPIN_DEFAULT  100
#define CAP_VDPIN_BASE     38

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================
/*
Input   Output  Ratio(In/Out)
1088    1080    1.007407407
1920    1920    1
1280    1920    0.666666667
576     1080    0.533333333
480     1080    0.444444444
720     1920    0.375
640     1920    0.333333333
288     1080    0.266666667
352     1920    0.183333333
*/
#define     WEIGHT_NUM 9
static MMP_FLOAT   ScaleRatio[WEIGHT_NUM] = {(1088.0f/1080), (1920.0f/1920), (1280.0f/1920), (576.0f/1080), (480.0f/1080), (720.0f/1920), (640.0f/1920), (288.0f/1080), (352.0f/1920)};
static MMP_UINT32  WeightMatInt[WEIGHT_NUM][CAP_SCALE_TAP_SIZE][CAP_SCALE_TAP];


//=============================================================================
//                Private Function Definition
//=============================================================================
static void
_Cap_SetGpioIN(MMP_UINT32 pins)
{
    ithGpioSetIn(pins);
    ithGpioSetFFIEn(pins);
    ithGpioSetMode(pins, ITH_GPIO_MODE2);
    //printf("GPIO_IN=%d",pins);
}
//=============================================================================
/**
* Calculate Scale Factor
*/
//=============================================================================
static MMP_FLOAT
_Cap_ScaleFactor(
    MMP_UINT16  Input,
    MMP_UINT16  Output)
{
    return (MMP_FLOAT) (((MMP_INT) (16384.0f*Input/(MMP_FLOAT)Output))/16384.0f);
}

//=============================================================================
/**
* Select Color Matrix
*/
//=============================================================================
static void
_Cap_ColorMatrix(
    CAP_CONTEXT *Capctxt,MMP_BOOL videomode)
{
    if (videomode == MMP_TRUE)
    { 
        /* RGB to YUV videomode */
        Capctxt->RGBtoYUVFun._11 = 0x004D;
        Capctxt->RGBtoYUVFun._12 = 0x0096;
        Capctxt->RGBtoYUVFun._13 = 0x001D;
        Capctxt->RGBtoYUVFun._21 = 0x07D4;
        Capctxt->RGBtoYUVFun._22 = 0x07A9;
        Capctxt->RGBtoYUVFun._23 = 0x0083;
        Capctxt->RGBtoYUVFun._31 = 0x0083;
        Capctxt->RGBtoYUVFun._32 = 0x0792;
        Capctxt->RGBtoYUVFun._33 = 0x07EB;
        Capctxt->RGBtoYUVFun.ConstY = 0x0000;
        Capctxt->RGBtoYUVFun.ConstU = 0x0080;
        Capctxt->RGBtoYUVFun.ConstV = 0x0080;

    }
    else
    {     
        /* RGB to YUV REG  non videomode*/
        Capctxt->RGBtoYUVFun._11 = 0x0042;
        Capctxt->RGBtoYUVFun._12 = 0x0081;
        Capctxt->RGBtoYUVFun._13 = 0x0019;
        Capctxt->RGBtoYUVFun._21 = 0x07DA;
        Capctxt->RGBtoYUVFun._22 = 0x07B6;
        Capctxt->RGBtoYUVFun._23 = 0x0070;
        Capctxt->RGBtoYUVFun._31 = 0x0070;
        Capctxt->RGBtoYUVFun._32 = 0x07A2;
        Capctxt->RGBtoYUVFun._33 = 0x07EE;
        Capctxt->RGBtoYUVFun.ConstY = 0x0010;
        Capctxt->RGBtoYUVFun.ConstU = 0x0080;
        Capctxt->RGBtoYUVFun.ConstV = 0x0080;
    }
}

//=============================================================================
/**
 * Create weighting for the matrix of scaling.
*/
//=============================================================================
static void
_CAP_CreateWeighting(
    MMP_FLOAT   scale,
    MMP_UINT8   taps,
    MMP_UINT8   tapSize,
    MMP_FLOAT   weightMatrix[][CAP_SCALE_TAP])
{
    MMP_UINT8   i, j;
    MMP_FLOAT   WW;
    MMP_FLOAT   W[32];
    MMP_INT16   point;
    MMP_UINT8   adjust;
    MMP_INT16   sharp;
    MMP_UINT8   method;
    MMP_FLOAT   precision = 64.0f;
    MMP_FLOAT   fscale = 1.0f;

    //Kevin:TODO temp solution
    adjust = 0;
    method = 11;
    sharp = 0;

    switch(sharp)
    {
        case 4:     fscale = 0.6f;  break;
        case 3:     fscale = 0.7f;  break;
        case 2:     fscale = 0.8f;  break;
        case 1:     fscale = 0.9f;  break;
        case 0:     fscale = 1.0f;  break;
        case -1:    fscale = 1.1f;  break;
        case -2:    fscale = 1.2f;  break;
        case -3:    fscale = 1.3f;  break;
        case -4:    fscale = 1.4f;  break;
        default:    fscale = 1.5f;  break;
    };

    if (adjust == 0)
    {
        if (scale < 1.0f)
        {
            scale = fscale;
        }
        else
        {
            scale *= fscale;
        }
    }
    else if (adjust == 1)
    {
        //Last update (2002/04/24) by WKLIN]
        // For Low Pass
        if (scale < 1.0f)
        {
            scale = 1.2f;
        }
        else if (scale > 1.0f)
        {
            scale *= 1.1f;
        }
    }
    else if (adjust == 2)
    {
        //Last update (2003/08/17) by WKLIN in Taipei
        //For including more high frequency details
        if (scale < 1.0f)
        {
            scale = 0.9f;
        }
        else if (scale > 1.0f)
        {
            scale *= 0.9f;
        }
    }
    else if (adjust == 3)
    {
        //Last update (2003/09/10) by WKLIN in Hsin-Chu
        //For excluding more high frequency details
        if (scale < 1.0f)
        {
            scale = 1.2f;
        }
        else if (scale >= 1.0f)
        {
            scale *= 1.3f;
        }
    }

    if (method == 10)
    {
        //Sinc
        for (i=0; i <= (tapSize>>1); i++)
        {
            WW = 0.0f;
            point = (MMP_INT)(taps>>1) - 1;
            for (j = 0; j < taps; j++)
            {
                W[j] = capSinc( ((MMP_FLOAT)point + i/((MMP_FLOAT)tapSize)) / (MMP_FLOAT)scale);
                point--;
                WW += W[j];
            }

            //for (j=0; j< taps; j++)
            //  WeightMat[i][j] = ((int)(W[j]/WW*precision + 0.5 ))/precision;

            //Changed: 2004/02/24
            weightMatrix[i][taps-1] = 1.0;
            for (j = 0; j < taps-1; j++)
            {
                weightMatrix[i][j] = ((MMP_INT)(W[j]/WW*precision + 0.5)) / precision;
                weightMatrix[i][taps-1] -= weightMatrix[i][j];
            }
        }

        for (i = ((tapSize>>1)+1); i < tapSize; i++)
        {
            for (j = 0; j < taps; j++)
            {
                weightMatrix[i][j] = weightMatrix[tapSize-i][taps-1-j];
            }
        }
    }
    else if (method == 11)
    {
        //rcos
        for (i = 0; i <= (tapSize>>1); i++)
        {
            WW =0.0;
            point = (MMP_INT) (taps>>1)-1;
            for (j=0; j < taps; j++)
            {
                W[j] = capRcos( ((MMP_FLOAT)point + i/((MMP_FLOAT)tapSize)) / (MMP_FLOAT)scale);
                point--;
                WW += W[j];
            }

            //for (j=0; j< taps; j++)
            //  weightMatrix[i][j] = ((int)(W[j]/WW*precision + 0.5 ))/precision;

            //Changed: 2004/02/24
            weightMatrix[i][taps-1] = 1.0;
            for (j = 0; j < taps-1; j++)
            {
                weightMatrix[i][j] = ((MMP_INT)(W[j]/WW*precision + 0.5)) / precision;
                weightMatrix[i][taps-1] -= weightMatrix[i][j];
            }

        }
        for (i = ((tapSize>>1)+1); i < tapSize; i++)
        {
            for (j = 0; j < taps; j++)
            {
                weightMatrix[i][j] = weightMatrix[tapSize-i][taps-1-j];
            }
        }
    }
    else if (method == 12)
    {
        // Catmull-Rom Cubic interpolation
        for (i = 0; i <= (tapSize>>1); i++)
        {
            WW = 0.0f;
            point = (MMP_INT)(taps>>1)-1;
            for (j = 0; j < taps; j++)
            {
                W[j] = capCubic01( ((MMP_FLOAT)point + i/((MMP_FLOAT)tapSize)) / (MMP_FLOAT)scale);
                point--;
                WW += W[j];

                //cap_msg_ex(CAP_MSG_TYPE_ERR, "i:%2d   W=%6.3f   point=%2d    WW=%6.3f\n",
                //  i, W[j], point, WW);
            }


            //Changed: 2004/02/24
            weightMatrix[i][taps-1] = 1.0;
            for (j = 0; j < taps-1; j++)
            {
                weightMatrix[i][j] = ((MMP_INT)(W[j]/WW*precision + 0.5)) / precision;
                weightMatrix[i][taps-1] -= weightMatrix[i][j];
            }
        }
        for (i = ((tapSize>>1)+1); i < tapSize; i++)
        {
            for (j = 0; j < taps; j++)
            {
                weightMatrix[i][j] = weightMatrix[tapSize-i][taps-1-j];
            }
        }
    }
    else
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, " %s() unknow error !\n", __FUNCTION__);
    }
}

//=============================================================================
//                Public Function Definition
//=============================================================================
//=============================================================================
/**
 * CAP memory Initialize.
 */
//=============================================================================

//   CAP_MEM_BUF_PITCH
//     ----------
// 720 |   Y0     |
//     ----------
// 360 |   UV0   |
//     ----------
// 720 |   Y1     |
//     ----------
// 360 |   UV1   |
//     ----------
// 720 |   Y2     |
//     ----------
// 360 |   UV2   |
//     ----------

MMP_RESULT
Cap_Memory_Initialize(
    CAPTURE_HANDLE *ptDev, CAPTURE_SETTING info)
{
    MMP_RESULT result = MMP_SUCCESS;
	CAP_CONTEXT *pCapctxt = (CAP_CONTEXT *)ptDev;
    MMP_UINT32 CapMemBuffer[CAPTURE_MEM_BUF_COUNT] = { 0 };
    MMP_UINT32 i;
    MMP_UINT32 offset = 0;
    MMP_UINT32  Y_BUFFER_BASE = 0;
    MMP_UINT32  UV_BUFFER_BASE = info.Max_Height;    
	
    MMP_UINT32 size = ((info.Max_Height >> 1) * info.Max_Width) * (CAPTURE_MEM_BUF_COUNT + (CAPTURE_MEM_BUF_COUNT / 2));;

    if (pCapctxt->video_sys_addr)
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, " Init Conflict %s (%d) error\n", __FUNCTION__, __LINE__);
        goto end;
    }

	pCapctxt->video_vram_addr = (MMP_UINT32)itpVmemAlignedAlloc(64, size); //64 bit aligen
	pCapctxt->video_sys_addr = (uint8_t*) ithMapVram(pCapctxt->video_vram_addr, size, ITH_VRAM_WRITE);
	if (pCapctxt->video_vram_addr == MMP_NULL)
	{
		cap_msg_ex(CAP_MSG_TYPE_ERR, " create memory pool fail %s (%d) error\n", __FUNCTION__, __LINE__);
		result = MMP_RESULT_ERROR;
		goto end;
	}
	CapMemBuffer[0] = (MMP_UINT32)pCapctxt->video_sys_addr;

    for (i = 0; i < CAPTURE_MEM_BUF_COUNT; i++)
    {
        if(i % 2 == 0)
        {
            offset    = Y_BUFFER_BASE * info.Max_Width;
            Y_BUFFER_BASE += (info.Max_Height + (info.Max_Height/2)) ;
        }
        else
        {
            offset    = UV_BUFFER_BASE * info.Max_Width;
            UV_BUFFER_BASE += (info.Max_Height + (info.Max_Height/2));
        }
        CapMemBuffer[i] = offset + CapMemBuffer[0];

        if(i % 2 == 0)
        {
            if(i == 0)
                pCapctxt->OutAddrY[i]   = (MMP_UINT32)ithSysAddr2VramAddr((void*)CapMemBuffer[i]);
            else
                pCapctxt->OutAddrY[i/2] = (MMP_UINT32)ithSysAddr2VramAddr((void*)CapMemBuffer[i]);
        }
        else
            pCapctxt->OutAddrUV[((i+1) / 2) - 1] = (MMP_UINT32)ithSysAddr2VramAddr((void*)CapMemBuffer[i]);
       

        printf("CapMemBuffer[%d] addr = 0x%x\n", i , CapMemBuffer[i]);
    }
	
end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) error\n", __FUNCTION__, __LINE__);

    return (MMP_RESULT)result;

}
//=============================================================================
/**
 * CAP memory clear.
 */
//=============================================================================
MMP_RESULT
Cap_Memory_Clear(
    CAPTURE_HANDLE *ptDev)
{
    MMP_RESULT result = MMP_SUCCESS;
	CAP_CONTEXT *pCapctxt = &ptDev->cap_info;

	if (pCapctxt->video_sys_addr)
    {
        itpVmemFree(pCapctxt->video_vram_addr);
        pCapctxt->video_sys_addr  = NULL;
        pCapctxt->video_vram_addr = 0;
		pCapctxt->video_sys_addr = 0;
    }
    else
    {
        result = MMP_RESULT_ERROR;
        goto end;
    }

end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) error\n", __FUNCTION__, __LINE__);
	
    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * Update CAP device.
 */
//=============================================================================
MMP_RESULT
Cap_Update_Reg(
    CAPTURE_HANDLE *ptDev)
{
    MMP_RESULT  result = MMP_SUCCESS;
    MMP_UINT32 index;

    CAP_CONTEXT *Capctxt = &ptDev->cap_info; 
    
    if(Capctxt == MMP_NULL)
    {
        result = MMP_RESULT_ERROR;
        goto end;
    }

    #if 0
    // Update Onfly or Memeory mode
    if (Capctxt->EnableOnflyMode == MMP_TRUE && Capctxt->EnableInterrupt == MMP_TRUE)
    {
        ithCap_Set_ISP_HandShaking(ptDev->cap_id,MEMORY_WITH_ONFLY_MODE, &Capctxt->outinfo);
        ithCap_Set_Interrupt_Mode(ptDev->cap_id,(CAP_INT_MODE_FRAME_END | CAP_INT_MODE_SYNC_ERR), MMP_TRUE);
    }
    #endif
    if (Capctxt->EnableOnflyMode == MMP_TRUE)
    {
        ithCap_Set_ISP_HandShaking(ptDev->cap_id,ONFLY_MODE, &Capctxt->outinfo);
        if(Capctxt->EnableInterrupt == MMP_TRUE)
		    ithCap_Set_Interrupt_Mode(ptDev->cap_id,CAP_INT_MODE_SYNC_ERR, MMP_TRUE);
    }
    else
    {
        ithCap_Set_ISP_HandShaking(ptDev->cap_id,MEMORY_MODE, &Capctxt->outinfo);
        if(Capctxt->EnableInterrupt == MMP_TRUE)
            ithCap_Set_Interrupt_Mode(ptDev->cap_id,(CAP_INT_MODE_FRAME_END | CAP_INT_MODE_SYNC_ERR), MMP_TRUE);
    }

	// Update Dither Setting
	if (Capctxt->Ditherinfo.EnDither == MMP_TRUE)
		ithCap_Set_Enable_Dither(ptDev->cap_id, &Capctxt->Ditherinfo);

#if defined (COLOR_MATRIX_VIDEOMODE) 
    _Cap_ColorMatrix(Capctxt,MMP_TRUE);//VideoMode
#else
    _Cap_ColorMatrix(Capctxt,MMP_FALSE);//Non VideoMode
#endif
    // Update Capture Write Buffer Address
    ithCap_Set_Buffer_addr_Reg(ptDev->cap_id,Capctxt->OutAddrY, Capctxt->OutAddrUV, Capctxt->outinfo.OutAddrOffset);

    // Update Input Pin mux
    ithCap_Set_Input_Pin_Mux_Reg(ptDev->cap_id,&Capctxt->inmux_info);

    // Update IO Parameter
    ithCap_Set_IO_Mode_Reg(ptDev->cap_id,&Capctxt->iomode_info);

    // Update Input Parameter
    ithCap_Set_Input_Data_Info_Reg(ptDev->cap_id,&Capctxt->ininfo);

    // Update Hor Scale
    ithCap_Set_HorScale_Width_Reg(ptDev->cap_id,&Capctxt->outinfo);

    ithCap_Set_Enable_Reg(ptDev->cap_id,&Capctxt->funen);

    // Update RGB to YUV Matrix
    if (Capctxt->funen.EnCSFun)
        ithCap_Set_RGBtoYUVMatrix_Reg(ptDev->cap_id,&Capctxt->RGBtoYUVFun);

    // Update Color Format
    ithCap_Set_Color_Format_Reg(ptDev->cap_id,&Capctxt->YUVinfo);

    // Update Scale Matrix
    Capctxt->ScaleFun.HCI = _Cap_ScaleFactor(Capctxt->ininfo.ROIWidth, Capctxt->outinfo.OutWidth);
    ithCap_Set_ScaleParam_Reg(ptDev->cap_id,&Capctxt->ScaleFun);

    for(index = 0; index < WEIGHT_NUM; index++)
    {
        if(Capctxt->ScaleFun.HCI >= ScaleRatio[index])
        {
            ithCap_Set_IntScaleMatrixH_Reg(ptDev->cap_id,WeightMatInt[index]);
            break;
        }
        else if(index == WEIGHT_NUM - 1)
            ithCap_Set_IntScaleMatrixH_Reg(ptDev->cap_id,WeightMatInt[index]);
    }

    // Update Skip Pattern
    ithCap_Set_Skip_Pattern_Reg(ptDev->cap_id,Capctxt->Skippattern, Capctxt->SkipPeriod);

end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) error\n", __FUNCTION__, __LINE__);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * CAP default value initialization.
 */
//=============================================================================
MMP_RESULT
Cap_Initialize(
    CAP_CONTEXT *Capctxt)
{
    MMP_RESULT result = MMP_SUCCESS;

    MMP_UINT32  i, j, index;
    MMP_FLOAT   WeightMat[CAP_SCALE_TAP_SIZE][CAP_SCALE_TAP];

    if (Capctxt == MMP_NULL)
    {
        result = MMP_RESULT_ERROR;
        goto end;
    }

    //clear flag
    Capctxt->YUVinfo.ColorDepth = COLOR_DEPTH_8_BITS;
	Capctxt->ininfo.NV12Format = UV;

    Capctxt->funen.EnDEMode = MMP_FALSE;
    Capctxt->funen.EnCSFun = MMP_FALSE;
    Capctxt->funen.EnInBT656 = MMP_FALSE;
	Capctxt->Ditherinfo.EnDither = MMP_FALSE;    
	Capctxt->funen.EnHSync = MMP_FALSE;
	Capctxt->funen.EnPort1UV2LineDS = MMP_TRUE; // ask H.C.
	Capctxt->funen.EnAutoDetHSPol = MMP_FALSE;
	Capctxt->funen.EnAutoDetVSPol = MMP_FALSE;
	Capctxt->funen.EnDumpMode = MMP_FALSE;
	Capctxt->funen.EnMemContinousDump = MMP_FALSE;
	Capctxt->funen.EnSramNap = MMP_FALSE;
	Capctxt->funen.EnMemLimit = MMP_FALSE;
	Capctxt->ininfo.CheckHS = MMP_FALSE;
	Capctxt->ininfo.CheckDE = MMP_FALSE;
	Capctxt->ininfo.CheckVS = MMP_TRUE; //always turn on.
	Capctxt->ininfo.WrMergeThresld = 0x10;
    Capctxt->ininfo.HSyncSkip = 0x2;

    /* Scale */
    Capctxt->ScaleFun.HCI = 0.0f;

    //Initial Scale Weight Matrix
    for(index = 0; index < WEIGHT_NUM; index++)
    {
        _CAP_CreateWeighting(ScaleRatio[index], CAP_SCALE_TAP, CAP_SCALE_TAP_SIZE, WeightMat);

          for(j = 0; j < CAP_SCALE_TAP_SIZE; j++)
              for(i = 0; i < CAP_SCALE_TAP; i++)
                WeightMatInt[index][j][i] = (MMP_UINT16)CAP_FLOATToFix(WeightMat[j][i], 1, 6);
    }
    
end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) Capture Initialize Fail\n", __FUNCTION__, __LINE__);

    return result;
}
void Cap_SetUserConfig(CAPTURE_HANDLE *ptDev, MMP_UINT32* Cptr)
{
    MMP_RESULT  result      = MMP_SUCCESS;
    CAP_CONTEXT *Capctxt    = &ptDev->cap_info;
    MMP_UINT16 i = 0;
    MMP_UINT8   Y_16Pin[CAP_DATA_BUS_WIDTH]; 
    MMP_UINT8   U_16Pin[CAP_DATA_BUS_WIDTH];
    MMP_UINT8   V_16Pin[CAP_DATA_BUS_WIDTH];
    
    if (Cptr == NULL)
    {
        result = MMP_RESULT_ERROR;
        goto end;
    }

    for(i = 0;i < CAP_DATA_BUS_WIDTH; i++)
    {
        if(Cptr[i] != CAP_VDPIN_DEFAULT)
        {
            Y_16Pin[i] = (MMP_UINT8)Cptr[i];
            _Cap_SetGpioIN(Cptr[i]+CAP_VDPIN_BASE);
        }
        else
            Y_16Pin[i] = 0;
    }
    
    for(i = 0;i < CAP_DATA_BUS_WIDTH; i++)
    {
        if(Cptr[i + CAP_DATA_BUS_WIDTH] != CAP_VDPIN_DEFAULT)
        {
            U_16Pin[i] = (MMP_UINT8)Cptr[i + CAP_DATA_BUS_WIDTH];
            _Cap_SetGpioIN(Cptr[i+ CAP_DATA_BUS_WIDTH]+CAP_VDPIN_BASE);         
        }
        else
            U_16Pin[i] = 0;
    }
    
    for(i = 0;i < CAP_DATA_BUS_WIDTH; i++)
    {
        if(Cptr[i + CAP_DATA_BUS_WIDTH * 2] != CAP_VDPIN_DEFAULT)
        {
            V_16Pin[i] = (MMP_UINT8)Cptr[i + CAP_DATA_BUS_WIDTH * 2];
            _Cap_SetGpioIN(Cptr[i + CAP_DATA_BUS_WIDTH * 2]+CAP_VDPIN_BASE); 
        }
        else
            V_16Pin[i] = 0;
    }

    /* Input  pin mux setting */
    memcpy(&Capctxt->inmux_info.Y_Pin_Num, &Y_16Pin,   CAP_DATA_BUS_WIDTH);
    memcpy(&Capctxt->inmux_info.U_Pin_Num, &U_16Pin,   CAP_DATA_BUS_WIDTH);
    memcpy(&Capctxt->inmux_info.V_Pin_Num, &V_16Pin,   CAP_DATA_BUS_WIDTH);

    for(i = 36; i < 40; i++)
    {
        if(Cptr[i] != CAP_VDPIN_DEFAULT)
            _Cap_SetGpioIN(Cptr[i]+CAP_VDPIN_BASE); 
    }
    
    if(Cptr[36] != CAP_VDPIN_DEFAULT)
        Capctxt->inmux_info.HS_Pin_Num = Cptr[36];
    else
        Capctxt->inmux_info.HS_Pin_Num = 0;

    if(Cptr[37] != CAP_VDPIN_DEFAULT)
        Capctxt->inmux_info.VS_Pin_Num = Cptr[37];
    else
        Capctxt->inmux_info.VS_Pin_Num = 0;
    
    if(Cptr[38] != CAP_VDPIN_DEFAULT)
        Capctxt->inmux_info.DE_Pin_Num = Cptr[38]; 
    else
        Capctxt->inmux_info.DE_Pin_Num = 0;

    /* Clk */
    if(Cptr[39] != CAP_VDPIN_DEFAULT)
        Capctxt->inmux_info.UCLKPINNUM      = Cptr[39];
    else
        Capctxt->inmux_info.UCLKPINNUM = 0;
    
    Capctxt->inmux_info.UCLKSrc             = Cptr[40];//0x1;
    Capctxt->inmux_info.UCLKInv             = Cptr[41];//0x0;
    Capctxt->inmux_info.UCLKDly             = Cptr[42];//0x0;
    Capctxt->inmux_info.UCLKRatio           = Cptr[43];//0x0;
    Capctxt->inmux_info.EnUCLK              = Cptr[44];//0x1;
    Capctxt->inmux_info.UCLKAutoDlyDir      = Cptr[62];
    Capctxt->inmux_info.UCLKAutoDlyEn       = Cptr[63];
    
    /* I/O Setting */
    Capctxt->iomode_info.CAPIOFFEn_VD_00_31 = Cptr[45];//0x00000fbf;
    Capctxt->iomode_info.CAPIOFFEn_VD_35_32 = Cptr[46];//0x0; 
    
    /* Input Data Format Setting */
    Capctxt->YUVinfo.InputMode              = Cptr[47];//YUV422;
    Capctxt->YUVinfo.ColorOrder             = Cptr[48];//CAP_IN_UYVY;
    Capctxt->YUVinfo.InputWidth             = Cptr[49];//PIN_8_10_12BITS;

    Capctxt->funen.EnDEMode                 = Cptr[50];//MMP_TRUE;
    Capctxt->funen.EnInBT656                = Cptr[51];//MMP_FALSE; //BT601
	Capctxt->funen.EnHSync                  = Cptr[52];//MMP_FALSE;
	Capctxt->funen.EnAutoDetHSPol           = Cptr[53];//MMP_TRUE;
	Capctxt->funen.EnAutoDetVSPol           = Cptr[54];//MMP_FALSE; 
    
 	/* Input Format default Setting */
    Capctxt->ininfo.EmbeddedSync            = Capctxt->funen.EnInBT656;
    Capctxt->ininfo.Interleave              = Cptr[55];//Progressive;
    Capctxt->ininfo.WrMergeThresld          = Cptr[56];//0x10
    Capctxt->ininfo.CheckDE                 = Cptr[57];//MMP_TRUE;
	Capctxt->ininfo.CheckHS                 = Cptr[58];//MMP_FALSE;
    Capctxt->ininfo.CheckVS                 = Cptr[59];//MMP_FALSE;//if connect to sensor, suggest to turn off. just turn on CheckHS to induce frame check time. 
    
    /* Skippattern init */
    Capctxt->Skippattern                    = Cptr[60];//0xf
    Capctxt->SkipPeriod                     = Cptr[61];//0x3


    /* Video source from external IO */
    ithWriteRegMaskA(_GetAddress(ptDev->cap_id, CAP_CLOCK_SETTING_REGISTER), 0x1, CAP_MSK_UCLKSRC);     

end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) Setting Fail, because null pointer\n", __FUNCTION__, __LINE__);
}

