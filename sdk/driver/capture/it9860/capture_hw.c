#include <unistd.h>
#include "capture_config.h"
#include "capture_hw.h"
#include "capture_reg.h"
#include "capture_util.h"

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
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================

//=============================================================================
//                Public Set Function Definition
//=============================================================================
void
ithCapHWICInit(void)
{
   //ithClearRegBitA(ITH_HOST_BASE + ITH_CAP_CLK_REG, ITH_EN_M12CLK_BIT);//turn off M12clk
   //usleep(1000);
   //ithWriteRegMaskA(_GetAddress(0, CAP_SCALING_SETTING_REGISTER0), 0x40, 0x1FFF);//assign nonzero value cap0
   //usleep(1000);
   //ithSetRegBitA(ITH_HOST_BASE + ITH_CAP_CLK_REG, ITH_EN_M12CLK_BIT);//turn on M12clk
}
void
ithCapEnableClock(void)
{
    // enable clock
    ithSetRegBitA(ITH_HOST_BASE + ITH_CAP_CLK_REG, ITH_EN_M12CLK_BIT);
    ithSetRegBitA(ITH_HOST_BASE + ITH_CAP_CLK_REG, ITH_EN_W18CLK_BIT);
    ithSetRegBitA(ITH_HOST_BASE + ITH_CAP_CLK_REG, ITH_EN_DIV_CAPCLK_BIT);
}
 
void
ithCapDisableClock(void)
{
    // disable clock
    ithClearRegBitA(ITH_HOST_BASE + ITH_CAP_CLK_REG, ITH_EN_M12CLK_BIT);
    ithClearRegBitA(ITH_HOST_BASE + ITH_CAP_CLK_REG, ITH_EN_W18CLK_BIT);
    ithClearRegBitA(ITH_HOST_BASE + ITH_CAP_CLK_REG, ITH_EN_DIV_CAPCLK_BIT);
}

void
ithCap_Set_Reset(
    void)
{
    ithWriteRegMaskA(ITH_HOST_BASE + ITH_CAP_CLK_REG, (0x1 << ITH_EN_CAPC_RST_BIT), (0x1 << ITH_EN_CAPC_RST_BIT));
    usleep(1000);
    ithWriteRegMaskA(ITH_HOST_BASE + ITH_CAP_CLK_REG, 0x0, (0x1 << ITH_EN_CAPC_RST_BIT));
    //usleep(1000);
}

void
ithCap_Set_Register_Reset(void)
{
    ithWriteRegMaskA(ITH_HOST_BASE + ITH_CAP_CLK_REG, (0x1 << ITH_EN_CAP_REG_RST_BIT), (0x1 << ITH_EN_CAP_REG_RST_BIT));
    usleep(1000);
    ithWriteRegMaskA(ITH_HOST_BASE + ITH_CAP_CLK_REG, 0x0, (0x1 << ITH_EN_CAP_REG_RST_BIT));
    usleep(1000);
}

void 
ithCap_Set_AutoDelayHWFlow(
    CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 data = 0;
    /*disable Auto Delay*/
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_CLOCK_SETTING_REGISTER), 0x0, 0x1 << 31);
    /*Disable uclken*/
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_CLOCK_SETTING_REGISTER), 0x0, CAP_MSK_UCLKEN);
    
    data = ithReadRegA(_GetAddress(DEV_ID, CAP_ENGINE_STATUS_REGISTER0));
    data = data & (0x0F << 16);
    data = data >> 8;
    /*write Dlystage to UCLKDly*/
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_CLOCK_SETTING_REGISTER), data, CAP_MSK_UCLK_DLY);
    /*Enable uclken*/
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_CLOCK_SETTING_REGISTER), 0x1 << 3, CAP_MSK_UCLKEN);
}

void
ithCap_Set_Fire(
    CAPTURE_DEV_ID DEV_ID)
{
    ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER39), (BIT0 | BIT1));
}

void
ithCap_Set_UnFire(
    CAPTURE_DEV_ID DEV_ID)
{
    ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER39), 0x0);
    /*flush fifo data */
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER8), 0x0, (0x003F << 8));
}

void
ithCap_Set_ErrReset(
    CAPTURE_DEV_ID DEV_ID)
{
    // disable cap
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER39), 0x0, (BIT0 | BIT1));

    // switch to internal colorbar
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_CLOCK_SETTING_REGISTER), BIT1, CAP_MSK_UCLKSRC);

    //reset
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER39), BIT0, BIT0);
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER39), 0x0, BIT0);

    // switch to external IO
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_CLOCK_SETTING_REGISTER), BIT0, CAP_MSK_UCLKSRC);
}

/* Input clk delay enable */
void
ithCap_Set_TurnOnClock_Reg(
    CAPTURE_DEV_ID DEV_ID, MMP_BOOL flag)
{

    if (flag == true)
        ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_CLOCK_SETTING_REGISTER), CAP_MSK_UCLKEN, CAP_MSK_UCLKEN);
    else
        ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_CLOCK_SETTING_REGISTER), 0x0, CAP_MSK_UCLKEN);
}

/* Input Related Reg Setting */
MMP_RESULT
ithCap_Set_Input_Pin_Mux_Reg(
    CAPTURE_DEV_ID DEV_ID, CAP_INPUT_MUX_INFO *pininfo)
{
    MMP_RESULT  result = MMP_SUCCESS;
    MMP_UINT32  data = 0, mask = 0;
    
    //Disable uclken
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_CLOCK_SETTING_REGISTER), 0x0, CAP_MSK_UCLKEN);
    
    // Setting Y Pin mux
    data    = pininfo->Y_Pin_Num[0];
    data    |= (pininfo->Y_Pin_Num[1] << 8);
    data    |= (pininfo->Y_Pin_Num[2] << 16);
    data    |= (pininfo->Y_Pin_Num[3] << 24);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_Y_DATA_PIN_MUX0), data);

    data    = pininfo->Y_Pin_Num[4];
    data    |= (pininfo->Y_Pin_Num[5] << 8);
    data    |= (pininfo->Y_Pin_Num[6] << 16);
    data    |= (pininfo->Y_Pin_Num[7] << 24);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_Y_DATA_PIN_MUX1), data);

    data    = pininfo->Y_Pin_Num[8];
    data    |= (pininfo->Y_Pin_Num[9] << 8);
    data    |= (pininfo->Y_Pin_Num[10] << 16);
    data    |= (pininfo->Y_Pin_Num[11] << 24);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_Y_DATA_PIN_MUX2), data);

    // Setting U Pin mux
    data    = pininfo->U_Pin_Num[0];
    data    |= (pininfo->U_Pin_Num[1] << 8);
    data    |= (pininfo->U_Pin_Num[2] << 16);
    data    |= (pininfo->U_Pin_Num[3] << 24);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_U_DATA_PIN_MUX0), data);

    data    = pininfo->U_Pin_Num[4];
    data    |= (pininfo->U_Pin_Num[5] << 8);
    data    |= (pininfo->U_Pin_Num[6] << 16);
    data    |= (pininfo->U_Pin_Num[7] << 24);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_U_DATA_PIN_MUX1), data);

    data    = pininfo->U_Pin_Num[8];
    data    |= (pininfo->U_Pin_Num[9] << 8);
    data    |= (pininfo->U_Pin_Num[10] << 16);
    data    |= (pininfo->U_Pin_Num[11] << 24);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_U_DATA_PIN_MUX2), data);

    // Setting V Pin mux
    data    = pininfo->V_Pin_Num[0];
    data    |= (pininfo->V_Pin_Num[1] << 8);
    data    |= (pininfo->V_Pin_Num[2] << 16);
    data    |= (pininfo->V_Pin_Num[3] << 24);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_V_DATA_PIN_MUX0), data);

    data    = pininfo->V_Pin_Num[4];
    data    |= (pininfo->V_Pin_Num[5] << 8);
    data    |= (pininfo->V_Pin_Num[6] << 16);
    data    |= (pininfo->V_Pin_Num[7] << 24);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_V_DATA_PIN_MUX1), data);

    data    = pininfo->V_Pin_Num[8];
    data    |= (pininfo->V_Pin_Num[9] << 8);
    data    |= (pininfo->V_Pin_Num[10] << 16);
    data    |= (pininfo->V_Pin_Num[11] << 24);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_V_DATA_PIN_MUX2), data);

    //Set HS,VS,DE, Pin mux //Benson
    data    = pininfo->HS_Pin_Num;
    data    |= (pininfo->VS_Pin_Num << 8);
    data    |= (pininfo->DE_Pin_Num << 16);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER6), data);

    //Set Input clk mode
    data = (MMP_UINT32)(pininfo->UCLKSrc & 0x07) |
        ((MMP_UINT32)(pininfo->EnUCLK & 0x1) << 3) |
        ((MMP_UINT32)(pininfo->UCLKRatio & 0x0F) << 4) |
        ((MMP_UINT32)(pininfo->UCLKDly & 0x0F) << 8) |
        ((MMP_UINT32)(pininfo->UCLKInv & 0x1) << 12) |
        ((MMP_UINT32)(pininfo->UCLKPINNUM & 0x3F) << 16) |
        ((MMP_UINT32)(pininfo->UCLKAutoDlyDir & 0x1) << 28)|
        ((MMP_UINT32)(0x1) << 29)|
        ((MMP_UINT32)(0x0) << 30)|
        ((MMP_UINT32)(pininfo->UCLKAutoDlyEn & 0x1) << 31);

    mask = (0x07)   |
        (0x1 << 3)  |
        (0xF << 4)  |
        (0xF << 8)  |
        (0x1 << 12) |
        (0x3F << 16)|
        (0x1 << 28) |
        (0x1 << 29) |
        (0x1 << 30) |
        (0x1 << 31);
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_CLOCK_SETTING_REGISTER), data, mask);

    return result;
}

void
ithCap_Set_Color_Format_Reg(
    CAPTURE_DEV_ID DEV_ID, CAP_YUV_INFO *pYUVinfo)
{
    MMP_UINT32 Value;

    Value = (pYUVinfo->InputMode & 0x3) |
        ((pYUVinfo->ColorOrder & 0x3) << 2) |
        ((pYUVinfo->ColorDepth & 0x3) << 4) |
        ((pYUVinfo->InputWidth & 0x3) << 6);

    ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER2), Value);
}

MMP_RESULT
ithCap_Set_IO_Mode_Reg(
    CAPTURE_DEV_ID DEV_ID, CAP_IO_MODE_INFO *io_config)
{
    // ask H.C
    MMP_RESULT  result = MMP_SUCCESS;
    //CAP IO FF
    ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER4), io_config->CAPIOFFEn_VD_00_31);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER5), (io_config->CAPIOFFEn_VD_35_32 & 0xF));
    return result;
}

void
ithCap_Set_Input_Data_Info_Reg(
    CAPTURE_DEV_ID DEV_ID, CAP_INPUT_INFO *pIninfo)
{
    MMP_UINT32 data = 0, mask = 0;

    /* Set Interlace or Progressive */
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER0), (pIninfo->Interleave << 1), BIT1);

    /* Set Hsync & Vsync Porlarity */
    data    = (pIninfo->HSyncPol & 0x1);
    data    |= ((pIninfo->VSyncPol & 0x1) << 1);

    /* Set VsyncSkip & HsyncSkip */
    data    |= ((pIninfo->VSyncSkip & 0x7) << 8);
    data    |= ((pIninfo->HSyncSkip & 0x3F) << 12);

    /* Set sample Vsync by Hsync */
    data    |= ((pIninfo->HSnapV & 0x1) << 27);

    /* Set CheckHsync &  CheckVsync & CheckDE enable */
    data    |= ((pIninfo->CheckHS & 0x1) << 28);
    data    |= ((pIninfo->CheckVS & 0x1) << 29);
    data    |= ((pIninfo->CheckDE & 0x1) << 30);

    mask    = (0x1) |
        (0x1 << 1) |
        (0x7 << 8) |
        (0x3F << 12) |
        (0x1 << 27) |
        (0x1 << 28) |
        (0x1 << 29) |
        (0x1 << 30);
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_CAPTURE_INTERNAL_SETTING), data, mask);

    /* Set memory write threshould & NV12Format */
    data    = ((pIninfo->NV12Format & 0x1) << 5);
    data    |= ((pIninfo->WrMergeThresld & 0x3F) << 8);
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER8), data, ((0x003F << 8) | (0x1 << 5)));

    /* Set YUV pitch */
    data    = ((pIninfo->PitchY >> 3 & 0x3FF) << 3);
    data    |= ((pIninfo->PitchUV >> 3 & 0x3FF) << 19);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER21), data);

    /*  Set Active Region  Set CapWidth & Cap Height  */
    data    = (pIninfo->capwidth & 0x1FFF);
    data    |= ((pIninfo->capheight & 0x1FFF) << 16);
    ithWriteRegA(   _GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER1),         data);
    ithWriteRegA(   _GetAddress(DEV_ID, CAP_ACTIVE_REGION_SETTING_REGISTER0),   (pIninfo->HNum1 & 0x1FFF));

    data    = (pIninfo->LineNum1 & 0xFFF);
    data    |= ((pIninfo->LineNum2 & 0xFFF) << 16);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_ACTIVE_REGION_SETTING_REGISTER1), data);

    data    = (pIninfo->LineNum3 & 0xFFF);
    data    |= ((pIninfo->LineNum4 & 0xFFF) << 16);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_ACTIVE_REGION_SETTING_REGISTER2), data);

    /* Set ROI */ /* The source frame start X position and start Y position */
    data    = (pIninfo->ROIPosX & 0x1FFF);
    data    |= ((pIninfo->ROIPosY & 0x1FFF) << 16);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_ROI_SETTING_REGISTER0), data);

    /* The width size and height size of ROI image */
    data    = (pIninfo->ROIWidth & 0x1FFF);
    data    |= ((pIninfo->ROIHeight & 0x1FFF) << 16);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_ROI_SETTING_REGISTER1), data);
}

void
ithCap_Set_HorScale_Width_Reg(
    CAPTURE_DEV_ID DEV_ID, CAP_OUTPUT_INFO *pOutInfo)
{
    /*Set scale Width*/
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_SCALING_SETTING_REGISTER0), (MMP_UINT32)(pOutInfo->OutWidth), 0x1FFF);
}

/* Frame rate control */
void
ithCap_Set_Skip_Pattern_Reg(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT16 pattern, MMP_UINT16 period)
{
    MMP_UINT32 data = 0;
    data    = (pattern & 0xFFFF);
    data    |= ((period & 0xF) << 16);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_FRAMERATE_SETTING_REGISTER), data);
}

void
ithCap_Set_Hsync_Polarity(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT16 Hsync)
{
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_CAPTURE_INTERNAL_SETTING), Hsync, CAP_MSK_PHS);
}

void
ithCap_Set_Vsync_Polarity(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT16 Vsync)
{
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_CAPTURE_INTERNAL_SETTING), (Vsync << 1), CAP_MSK_VHS);
}

void
ithCap_Set_Interrupt_Mode(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT16 Intr_Mode, MMP_BOOL flag)
{
    MMP_UINT32 data = 0;

    if (flag == true)
    {
        if (Intr_Mode & CAP_INT_MODE_FRAME_END)
            data |= 0x1;
        if (Intr_Mode & CAP_INT_MODE_SYNC_ERR)
            data |= (0x1 << 1);
        if (Intr_Mode & CAP_INT_MODE_DCLK_PHASE_DRIFTED)
            data |= (0x1 << 2);
        if (Intr_Mode & CAP_INT_MODE_MUTE_DETECT)
            data |= (0x1 << 3);

        ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_INTERRUPT_SETTING_REGISTER), data, 0x0000000F);
    }
}

void
ithCap_Set_Color_Bar(
    CAPTURE_DEV_ID DEV_ID, CAP_COLOR_BAR_CONFIG color_info)
{
    MMP_UINT32 data = 0;

    if (color_info.Enable_colorbar)
        data |= B_CAP_COLOR_BAR_ROLLING_EN;

    data    |= (color_info.VS_act_start_line & 0x0FFF);
    data    |= ((color_info.VS_act_line & 0x0FFF) << 16);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER22), data);

    data    = (color_info.blank_line1 & 0x0FFF);
    data    |= ((color_info.act_line & 0x0FFF) << 16);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER23), data);

    data    = (color_info.blank_line2 & 0x0FFF);
    data    |= ((color_info.Hs_act & 0x0FFF) << 16);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER24), data);

    data    = (color_info.blank_pix1 & 0x0FFF);
    data    |= ((color_info.act_pix & 0x0FFF) << 16);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER25), data);

    data    = (color_info.blank_pix2 & 0x0FFF);

    if (color_info.Hsync_pol)
        data |= B_CAP_COLOR_BAR_HSPOL;

    if (color_info.Vsync_pol)
        data |= B_CAP_COLOR_BAR_VSPOL;

    ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER26), data);
}

void
ithCap_Set_Interleave(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT32 Interleave)
{
    /*	Set Interleave or Progressive   */
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER0), (Interleave << 1), 0x0002);
}

void
ithCap_Set_Width_Height(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT32 width, MMP_UINT32 height)
{
    MMP_UINT32  data    = 0;

    /*	Set Active Region  Set CapWidth & Cap Height   */
    data    = (width & 0x1FFF);
    data    |= ((height & 0x1FFF) << 16);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER1), data);
}

void
ithCap_Set_ROI_Width_Height(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT32 width, MMP_UINT32 height)
{
    /* The width & height size of ROI image */
    MMP_UINT32 data = 0;

    data    = (width & 0x1FFF);
    data    |= ((height & 0x1FFF) << 16);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_ROI_SETTING_REGISTER1), data);
}

void
ithCap_Set_Clean_Intr(
    CAPTURE_DEV_ID DEV_ID)
{
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_INTERRUPT_SETTING_REGISTER), (0x1 << 4), (0x1 << 4));
    //ithPrintf("clear cap %d int\n", DEV_ID);
}

void
ithCap_Set_Enable_Reg(
    CAPTURE_DEV_ID DEV_ID, CAP_ENFUN_INFO *pFunEn)
{
    MMP_UINT32 data = 0;

    //Enable Hsync &  BT601 or BT656
    data    = (pFunEn->EnInBT656);
    data    |= ((pFunEn->EnHSync & 0x1) << 2);

    // Data Enable mode in BT601
    data    |= ((pFunEn->EnDEMode & 0x1 ) << 3);
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER0),                 data,                                           0xD);

    // Enable CS fun
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_COLOR_SPACE_CONVERSION_SETTING_REGISTER0),  ((pFunEn->EnCSFun) << 24),    (0x1 << 24));

    //AutoDected Hsync & AutoDected Vsync or not
    data    = ((pFunEn->EnAutoDetHSPol & 0x1) << 6);
    data    |= ((pFunEn->EnAutoDetVSPol & 0x1) << 7);
    data    |= ((pFunEn->EnDumpMode & 0x1) << 31);
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_CAPTURE_INTERNAL_SETTING), data, (CAP_MSK_HSPOL_DET | CAP_MSK_VSPOL_DET | CAP_MSK_DUMPMODE));

    data    = ((pFunEn->EnMemContinousDump & 0x1) << 7);
    data    |= ((pFunEn->EnSramNap & 0x1) << 8);
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_CAPTURE_OUTPUT_SETTING), data, ((0x1 << 7) | (0x1 << 8)));

    data    = ((pFunEn->EnPort1UV2LineDS & 0x1) << 6);
    data    |= ((pFunEn->EnMemLimit & 0x1) << 31);
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER8), data, ((0x1 << 6) | (0x1 << 31)));
}

void
ithCap_Set_Enable_Dither(
    CAPTURE_DEV_ID DEV_ID, CAP_INPUT_DITHER_INFO *pDitherinfo)
{
    MMP_UINT32 data = 0;

    data    = (pDitherinfo->DitherMode & 0x3);
    data    |= ((pDitherinfo->EnDither & 0x1) << 3);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_DITHERING_SETTING), data);
}

MMP_RESULT
ithCap_Set_ISP_HandShaking(
    CAPTURE_DEV_ID DEV_ID, CAP_ISP_HANDSHAKING_MODE mode, CAP_OUTPUT_INFO *pOutInfo)
{
    MMP_UINT32  data        = 0;
    MMP_BOOL    MMP_ERROR   = 1;

    if (mode == ONFLY_MODE)//onfly mode
    {
        data    |= B_CAP_ONFLY_MODE;
        ithWriteRegA(_GetAddress(DEV_ID, CAP_CAPTURE_OUTPUT_SETTING), data);

        data    = ithReadRegA(_GetAddress(DEV_ID, CAP_CAPTURE_OUTPUT_SETTING));
        return (data & B_CAP_ONFLY_MODE) ? MMP_SUCCESS : MMP_ERROR;
    }
    else if (mode == MEMORY_MODE)//memory mode
    {
        data    |= B_CAP_MEM_MODE;
        data    |= ((pOutInfo->OutMemFormat & 0x3) << 4);
        ithWriteRegA(_GetAddress(DEV_ID, CAP_CAPTURE_OUTPUT_SETTING), data);

        // Set PreLoadNum
        data = ithReadRegA(_GetAddress(DEV_ID, CAP_CAPTURE_OUTPUT_SETTING));
        return (data & B_CAP_MEM_MODE) ? MMP_SUCCESS : MMP_ERROR;
    }
    else if (mode == MEMORY_WITH_ONFLY_MODE)
    {
        data    |= (B_CAP_ONFLY_MODE | B_CAP_MEM_MODE);
        data    |= ((pOutInfo->OutMemFormat & 0x3) << 4);
        ithWriteRegA(_GetAddress(DEV_ID, CAP_CAPTURE_OUTPUT_SETTING), data);

        // Set PreLoadNum
        data = ithReadRegA(_GetAddress(DEV_ID, CAP_CAPTURE_OUTPUT_SETTING));
        return (data & (B_CAP_ONFLY_MODE | B_CAP_MEM_MODE)) == (B_CAP_ONFLY_MODE | B_CAP_MEM_MODE) ? MMP_SUCCESS : MMP_ERROR;
    }
    else
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, "ISP HandShaking error\n");
        return MMP_ERROR;
    }
}

MMP_RESULT
ithCap_Set_Error_Handleing(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT32 errDetectEn)
{
    MMP_RESULT result = MMP_SUCCESS;

    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_ERROR_DETECT_SETTING_REGISTER), errDetectEn, 0x0000FFFF);
    return result;
}

MMP_RESULT
ithCap_Set_Wait_Error_Reset(
    CAPTURE_DEV_ID DEV_ID)
{
    MMP_RESULT  result = MMP_SUCCESS;
    MMP_UINT32  data;
    data = 0xFFFF0000;// & (!B_CAP_ERR_RST_STOP_ENGINE);
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_ERROR_DETECT_SETTING_REGISTER), data, 0xFFFF0000);

    return result;
}

void
ithCap_Set_Memory_AddressLimit_Reg(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT32 memUpBound, MMP_UINT32 memLoBound)
{
    ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER9), memUpBound);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER10), memLoBound);
}

void
ithCap_Set_Buffer_addr_Reg(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT32 *pYAddr, MMP_UINT32 *pUVAddr, MMP_UINT32 addrOffset)
{
    uint32_t vram_addr = 0;

    //Y0
    vram_addr = ithSysAddr2VramAddr((void*)pYAddr[0]);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER11), (vram_addr + addrOffset));

    //UV0
    vram_addr = ithSysAddr2VramAddr((void*)pUVAddr[0]);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER12), (vram_addr + addrOffset));

    //Y1
    vram_addr = ithSysAddr2VramAddr((void*)pYAddr[1]);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER13), (vram_addr + addrOffset));

    //UV1
    vram_addr = ithSysAddr2VramAddr((void*)pUVAddr[1]);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER14), (vram_addr + addrOffset));

    //Y2
    vram_addr = ithSysAddr2VramAddr((void*)pYAddr[2]);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER15), (vram_addr + addrOffset));

    //UV2
    vram_addr = ithSysAddr2VramAddr((void*)pUVAddr[2]);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER16), (vram_addr + addrOffset));

    switch(CAPTURE_MEM_BUF_COUNT)
    {
        case 6:
         //Y3
        vram_addr = ithSysAddr2VramAddr((void*)pYAddr[0]);
        ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER17), (vram_addr + addrOffset));

        //UV3
        vram_addr = ithSysAddr2VramAddr((void*)pUVAddr[0]);
        ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER18), (vram_addr + addrOffset));

        //Y4
        vram_addr = ithSysAddr2VramAddr((void*)pYAddr[0]);
        ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER19), (vram_addr + addrOffset));

        //UV4
        vram_addr = ithSysAddr2VramAddr((void*)pUVAddr[0]);
        ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER20), (vram_addr + addrOffset));            
        break;
        case 8:
         //Y3
        vram_addr = ithSysAddr2VramAddr((void*)pYAddr[3]);
        ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER17), (vram_addr + addrOffset));

        //UV3
        vram_addr = ithSysAddr2VramAddr((void*)pUVAddr[3]);
        ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER18), (vram_addr + addrOffset));

        //Y4
        vram_addr = ithSysAddr2VramAddr((void*)pYAddr[0]);
        ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER19), (vram_addr + addrOffset));

        //UV4
        vram_addr = ithSysAddr2VramAddr((void*)pUVAddr[0]);
        ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER20), (vram_addr + addrOffset)); 
        break;
        case 10:
        //Y3
        vram_addr = ithSysAddr2VramAddr((void*)pYAddr[3]);
        ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER17), (vram_addr + addrOffset));

        //UV3
        vram_addr = ithSysAddr2VramAddr((void*)pUVAddr[3]);
        ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER18), (vram_addr + addrOffset));

        //Y4
        vram_addr = ithSysAddr2VramAddr((void*)pYAddr[4]);
        ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER19), (vram_addr + addrOffset));

        //UV4
        vram_addr = ithSysAddr2VramAddr((void*)pUVAddr[4]);
        ithWriteRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER20), (vram_addr + addrOffset)); 
        break;
    }

    // Set framebuf num , memory mode
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER8), (CAPTURE_MEM_BUF_COUNT >> 1) - 1, 0x0007);
}

void
ithCap_Set_ScaleParam_Reg(
    CAPTURE_DEV_ID DEV_ID, const CAP_SCALE_CTRL *pScaleFun)
{
    MMP_UINT32  Value = 0;
    MMP_UINT32  HCI;

    HCI = CAP_FLOATToFix(pScaleFun->HCI, 6, 14);

    ithWriteRegA(_GetAddress(DEV_ID, CAP_SCALING_SETTING_REGISTER1), (MMP_UINT32)(HCI & 0x001FFFFF));
}

void
ithCap_Set_IntScaleMatrixH_Reg(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT32 WeightMatX[][CAP_SCALE_TAP])
{
    MMP_UINT32 Value;

    Value = (MMP_UINT32)((WeightMatX[0][0] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_L) |
        ((WeightMatX[0][1] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_H) |
        ((WeightMatX[0][2] & CAP_BIT_SCALEWEIGHT) << 16) |
        ((WeightMatX[0][3] & CAP_BIT_SCALEWEIGHT) << 24);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_SCALING_SETTING_REGISTER2), (MMP_UINT32)Value);

    Value = (MMP_UINT32)((WeightMatX[1][0] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_L) |
        ((WeightMatX[1][1] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_H) |
        ((WeightMatX[1][2] & CAP_BIT_SCALEWEIGHT) << 16) |
        ((WeightMatX[1][3] & CAP_BIT_SCALEWEIGHT) << 24);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_SCALING_SETTING_REGISTER3), (MMP_UINT32)Value);

    Value = (MMP_UINT32)((WeightMatX[2][0] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_L) |
        ((WeightMatX[2][1] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_H) |
        ((WeightMatX[2][2] & CAP_BIT_SCALEWEIGHT) << 16) |
        ((WeightMatX[2][3] & CAP_BIT_SCALEWEIGHT) << 24);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_SCALING_SETTING_REGISTER4), (MMP_UINT32)Value);

    Value = (MMP_UINT32)((WeightMatX[3][0] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_L) |
        ((WeightMatX[3][1] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_H) |
        ((WeightMatX[3][2] & CAP_BIT_SCALEWEIGHT) << 16) |
        ((WeightMatX[3][3] & CAP_BIT_SCALEWEIGHT) << 24);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_SCALING_SETTING_REGISTER5), (MMP_UINT32)Value);

    Value = (MMP_UINT32)((WeightMatX[4][0] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_L) |
        ((WeightMatX[4][1] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_H) |
        ((WeightMatX[4][2] & CAP_BIT_SCALEWEIGHT) << 16) |
        ((WeightMatX[4][3] & CAP_BIT_SCALEWEIGHT) << 24);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_SCALING_SETTING_REGISTER6), (MMP_UINT32)Value);
}

//=============================================================================
/**
 * RGB to YUV transfer matrix.
 */
//=============================================================================
void
ithCap_Set_RGBtoYUVMatrix_Reg(
    CAPTURE_DEV_ID DEV_ID, const CAP_RGB_TO_YUV *pRGBtoYUV)
{
    MMP_UINT32 data = 0;

    //CSOffset 1~3 all setting to zero, why?
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_COLOR_SPACE_CONVERSION_SETTING_REGISTER0), 0x0, 0x00FFFFFF);

    data    = (MMP_UINT32)(pRGBtoYUV->_11 & CAP_BIT_RGB_TO_YUV);
    data    |= (MMP_UINT32)((pRGBtoYUV->_12 & CAP_BIT_RGB_TO_YUV) << 16);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_COLOR_SPACE_CONVERSION_SETTING_REGISTER1), data);

    data    = (MMP_UINT32)(pRGBtoYUV->_13 & CAP_BIT_RGB_TO_YUV);
    data    |= (MMP_UINT32)((pRGBtoYUV->_21 & CAP_BIT_RGB_TO_YUV) << 16);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_COLOR_SPACE_CONVERSION_SETTING_REGISTER2), data);

    data    = (MMP_UINT32)(pRGBtoYUV->_22 & CAP_BIT_RGB_TO_YUV);
    data    |= (MMP_UINT32)((pRGBtoYUV->_23 & CAP_BIT_RGB_TO_YUV) << 16);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_COLOR_SPACE_CONVERSION_SETTING_REGISTER3), data);

    data    = (MMP_UINT32)(pRGBtoYUV->_31 & CAP_BIT_RGB_TO_YUV);
    data    |= (MMP_UINT32)((pRGBtoYUV->_32 & CAP_BIT_RGB_TO_YUV) << 16);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_COLOR_SPACE_CONVERSION_SETTING_REGISTER4), data);

    data    = (MMP_UINT32)(pRGBtoYUV->_33 & CAP_BIT_RGB_TO_YUV);
    data    |= (MMP_UINT32)((pRGBtoYUV->ConstY & CAP_BIT_RGB_TO_YUV_CONST) << 16);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_COLOR_SPACE_CONVERSION_SETTING_REGISTER5), data);

    data    = (MMP_UINT32)(pRGBtoYUV->ConstU & CAP_BIT_RGB_TO_YUV_CONST);
    data    |= (MMP_UINT32)((pRGBtoYUV->ConstV & CAP_BIT_RGB_TO_YUV_CONST) << 16);
    ithWriteRegA(_GetAddress(DEV_ID, CAP_COLOR_SPACE_CONVERSION_SETTING_REGISTER6), data);
}

void
ithCap_Set_MemThreshold(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT16 threshold)
{
    MMP_UINT32 data = 0;
    data  = ((threshold & 0x3F) << 8);
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER8), data, (0x003F << 8));
}


//=============================================================================
//
// Audio/Video/Mute Counter control function
//
//=============================================================================
void
ithAVSync_CounterCtrl(
    CAPTURE_DEV_ID DEV_ID, AV_SYNC_COUNTER_CTRL mode, MMP_UINT16 divider)
{
    if (mode & AUDIO_COUNTER_SEL)
    {
        ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_AV_SYNC_SETTING_REGISTER1), divider, 0x000007FF);
    }
    else if (mode & VIDEO_COUNTER_SEL)
    {
        ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_AV_SYNC_SETTING_REGISTER1), ((divider & 0x000007FF) << 16), 0x000007FF << 16);
    }
    else if (mode & MUTE_COUNTER_SEL)
    {
        ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_AV_SYNC_SETTING_REGISTER0), ((divider & 0x000007FF) << 16), 0x000007FF << 16);
    }
}

void
ithAVSync_CounterReset(
    CAPTURE_DEV_ID DEV_ID, AV_SYNC_COUNTER_CTRL mode)
{
    ithWriteRegMaskA(_GetAddress(DEV_ID, CAP_AV_SYNC_SETTING_REGISTER0), (mode & 0x001F), 0x001F);
}


MMP_UINT32
ithAVSync_CounterRead(
    CAPTURE_DEV_ID DEV_ID, AV_SYNC_COUNTER_CTRL mode)
{
    MMP_UINT32 value = 0;

    if (mode == AUDIO_COUNTER_SEL)
    {
        value = ithReadRegA(_GetAddress(DEV_ID, CAP_AV_SYNC_STATUS_REGISTER0));
    }
    else if (mode == VIDEO_COUNTER_SEL)
    {
        value = ithReadRegA(_GetAddress(DEV_ID, CAP_AV_SYNC_STATUS_REGISTER1));
    }
    else if (mode == MUTE_COUNTER_SEL)
    {
        value = ithReadRegA(_GetAddress(DEV_ID, CAP_AV_SYNC_STATUS_REGISTER2));
    }
    else if (mode == MUTEPRE_COUNTER_SEL)
    {
        value = ithReadRegA(_GetAddress(DEV_ID, CAP_AV_SYNC_STATUS_REGISTER3));
    }

    return value;
}

MMP_BOOL
ithAVSync_MuteDetect(
    CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 value;

    value = ithReadRegA(_GetAddress(DEV_ID, CAP_ENGINE_STATUS_REGISTER0));

    if (value & 0x01000000)
        return true;
    else
        return false;
}

//=============================================================================
//                Public Get Function Definition
//=============================================================================

MMP_BOOL
ithCap_Get_IsFire(
    CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 data;
    data = ithReadRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER39));

    if ((data & 0x0003) == 0x0003)
        return true;
    else
        return false;
}

MMP_RESULT
ithCap_Get_WaitEngineIdle(
    CAPTURE_DEV_ID DEV_ID)
{
    MMP_RESULT  result  = MMP_SUCCESS;
    MMP_UINT32  status  = 0;
    MMP_UINT32  timeOut = 0;

    //change to look the engine busy bit.
    status = ithReadRegA(_GetAddress(DEV_ID, CAP_ENGINE_STATUS_REGISTER0));
    while (!(status & 0x1000))
    {
        usleep(1000);
        if (++timeOut > 2000)
        {
            cap_msg_ex(CAP_MSG_TYPE_ERR, "Capture still busy !!!!!\n");
            result = MMP_RESULT_ERROR;
            goto end;
        }
        status = ithReadRegA(_GetAddress(DEV_ID, CAP_ENGINE_STATUS_REGISTER0));
    }

end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) ERROR !!!!!\n", __FUNCTION__, __LINE__);

    return (MMP_RESULT)result;
}

/* Get Capture Lane error status */
MMP_UINT32
ithCap_Get_Lane_status(
    CAPTURE_DEV_ID DEV_ID, CAP_LANE_STATUS lanenum)
{
    MMP_UINT32 data = 0;

    switch (lanenum)
    {
    case CAP_LANE0_STATUS:
        data    = ithReadRegA(_GetAddress(DEV_ID, CAP_ENGINE_STATUS_REGISTER0));
        break;
    default:
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) ERROR\n", __FUNCTION__, __LINE__);
    }

    return (MMP_UINT32)data;
}

MMP_UINT32
ithCap_Get_Hsync_Polarity(
    CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 data;
    data    = ithReadRegA(_GetAddress(DEV_ID, CAP_CAPTURE_INTERNAL_SETTING));
    data    = ((data & 0x01) >> 0);
    return data;
}

MMP_UINT32
ithCap_Get_Vsync_Polarity(
    CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 data;
    data    = ithReadRegA(_GetAddress(DEV_ID, CAP_CAPTURE_INTERNAL_SETTING));
    data    = ((data & 0x02 ) >> 1);
    return data;
}

MMP_UINT32
ithCap_Get_Hsync_Polarity_Status(
    CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 data; //change to check Hsync Stable
    data    = ithReadRegA(_GetAddress(DEV_ID, CAP_ENGINE_STATUS_REGISTER0));
    data    &= 0x01;
    cap_msg_ex(CAP_MSG_TYPE_ERR, "%s data = 0x%x\n", data);
    return data;
}

MMP_UINT32
ithCap_Get_Vsync_Polarity_Status(
    CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 data; //change to check Vsync Stable.
    data    = ithReadRegA(_GetAddress(DEV_ID, CAP_ENGINE_STATUS_REGISTER0));
    data    &= 0x02;
    cap_msg_ex(CAP_MSG_TYPE_ERR, "%s data = 0x%x\n", data);
    return data;
}

MMP_UINT32
ithCap_Get_Revision(
    CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 data;
    data    = ithReadRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER3));
    data    &= 0x001F;
    cap_msg_ex(CAP_MSG_TYPE_ERR, "%s data = 0x%x\n", data);
    return data;
}

MMP_UINT32
ithCap_Get_MRawVTotal(CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 data;
    data = ithReadRegA(_GetAddress(DEV_ID, CAP_ENGINE_STATUS_REGISTER3));
    return ((data & 0xFFFF0000) >> 16);
}

MMP_UINT32
ithCap_Get_Detectd_Hsync_Polarity(
    CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 data;
    data    = ithReadRegA(_GetAddress(DEV_ID, CAP_CAPTURE_INTERNAL_SETTING));
    data    = ((data & 0x00000040) >> 6);
    ithPrintf("detected Hsync Polarity = 0x%x\n", data);
    return data;
}

MMP_UINT32
ithCap_Get_Detectd_Vsync_Polarity(
    CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 data;
    data    = ithReadRegA(_GetAddress(DEV_ID, CAP_CAPTURE_INTERNAL_SETTING));
    data    = ((data & 0x00000080) >> 7);
    ithPrintf("detected Vsync Polarity = 0x%x\n", data);
    return data;
}


MMP_UINT32
ithCap_Get_Detected_Width(
    CAPTURE_DEV_ID DEV_ID)
{
    MMP_RESULT  result = MMP_SUCCESS;
    MMP_UINT32  data, InputBitWidth;

    data            = ithReadRegA(_GetAddress(DEV_ID, CAP_ENGINE_STATUS_REGISTER2));
    InputBitWidth   = ithReadRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER2));
    InputBitWidth   = (InputBitWidth & 0xC0) >> 6;

    if (InputBitWidth)
        data /= InputBitWidth;

    return ((data & 0x0FFF));
}

MMP_UINT32
ithCap_Get_Detected_Height(
    CAPTURE_DEV_ID DEV_ID)
{
    MMP_RESULT  result = MMP_SUCCESS;
    MMP_UINT32  data;

    data = ithReadRegA(_GetAddress(DEV_ID, CAP_ENGINE_STATUS_REGISTER2));
    return ((data & 0x1FFF0000) >> 16);
}

MMP_UINT32
ithCap_Get_Detected_Interleave(
    CAPTURE_DEV_ID DEV_ID)
{
    MMP_RESULT  result = MMP_SUCCESS;
    MMP_UINT32  data;

    data    = ithReadRegA(_GetAddress(DEV_ID, CAP_ENGINE_STATUS_REGISTER0));
    data    = (data & 0x08000000) >> 27;
    return data;
}

MMP_UINT32
ithCap_Get_Error_Status(
    CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 data = 0;
    data = ithReadRegA(_GetAddress(DEV_ID, CAP_ENGINE_STATUS_REGISTER0));
    if (data & 0xF00)
    {
        ithPrintf("Capture Get Error!! , data=0x%x\n", data);
        return (data & 0xF00) >> 8;
    }
    else
        return 0;
}

MMP_UINT32 
ithCap_Get_FIFOMAX(
    CAPTURE_DEV_ID DEV_ID, MMP_BOOL ISFIFO_Y)
{
   /* Debug Sel = 0 */
   ithWriteRegMaskA (_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER40), 0x0,  0x3F);
   
   MMP_UINT32 data = 0;
   /* Read FIFO MAX Y & UV */
   data = ithReadRegA(_GetAddress(DEV_ID, CAP_DEBUG_STATUS_REGISTER));
   
   if(ISFIFO_Y)
     data = data & 0x1FF;//Y FIFO MAX
   else
     data = (data & 0x1FF0000) >> 16;//UV FIFO MAX

   return data;
}
    
void
ithCap_Dump_Reg(
    CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 i, data;

    for (i = 0; i <= 544; i += 4)
    {
        data = ithReadRegA(_GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER0 + i));
        ithPrintf("reg=0x%x ,val=0x%08x\n", _GetAddress(DEV_ID, CAP_GENERAL_SETTING_REGISTER0 + i), data);
    }
    ithPrintf("\n");
}