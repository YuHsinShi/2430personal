#include "ite/ith.h"
#include "jpg_defs.h"
#include "jpg_hw.h"
#include "jpg_extern_link.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
//#define JPG_REGS_USE_CMDQ 0
bool b_JPG_REGS_USE_CMDQ = false;

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

#define JPG_HW_REG uint32_t

typedef struct _JPG_HW_REGS
{
    JPG_HW_REG  reg0;       ///< 0x00
    JPG_HW_REG  reg1;       ///< 0x04
    JPG_HW_REG  reg2;       ///< 0x08
    JPG_HW_REG  reg3;       ///< 0x0C
    JPG_HW_REG  reg4;       ///< 0x10
    JPG_HW_REG  reg5;       ///< 0x14
    JPG_HW_REG  reg6;       ///< 0x18
    JPG_HW_REG  reg7;       ///< 0x1C
    JPG_HW_REG  reg8;       ///< 0x20
    JPG_HW_REG  reg9;       ///< 0x24
    JPG_HW_REG  reg10;      ///< 0x28
    JPG_HW_REG  reg11;      ///< 0x2C
    JPG_HW_REG  reg12;      ///< 0x6C
    JPG_HW_REG  reg13;      ///< 0xAC
    JPG_HW_REG  reg14;      ///< 0xEC
    JPG_HW_REG  reg15;      ///< 0xFO
    JPG_HW_REG  reg16;      ///< 0xF4
    JPG_HW_REG  reg17;      ///< 0xF8
    JPG_HW_REG  reg18;      ///< 0xFC
    JPG_HW_REG  reg19;      ///< 0x100
    JPG_HW_REG  reg20;      ///< 0x104
    JPG_HW_REG  reg21;      ///< 0x108
    JPG_HW_REG  reg22;      ///< 0x10C
    JPG_HW_REG  reg23;      ///< 0x110
    JPG_HW_REG  reg24;      ///< 0x1000
    JPG_HW_REG  reg25;      ///< 0x1004
    JPG_HW_REG  reg26;      ///< 0x1008
    JPG_HW_REG  reg27;      ///< 0x100C
    JPG_HW_REG  reg28;      ///< 0x1010
    JPG_HW_REG  reg29;      ///< 0x1014
    JPG_HW_REG  reg30;      ///< 0x1018
    JPG_HW_REG  reg31;      ///< 0x101C
    JPG_HW_REG  reg32;      ///< 0x1020
}JPG_HW_REGS;

//=============================================================================
//                  Global Data Definition
//=============================================================================

JPG_HW_REGS jpg_regs = {0};

uint32_t jpgCompCtrl[5] =
{
    0,
    JPG_MSK_LINE_BUF_COMPONENT_1_VALID,
    (JPG_MSK_LINE_BUF_COMPONENT_1_VALID | JPG_MSK_LINE_BUF_COMPONENT_2_VALID),
    (JPG_MSK_LINE_BUF_COMPONENT_1_VALID | JPG_MSK_LINE_BUF_COMPONENT_2_VALID | JPG_MSK_LINE_BUF_COMPONENT_3_VALID),
    (JPG_MSK_LINE_BUF_COMPONENT_1_VALID | JPG_MSK_LINE_BUF_COMPONENT_2_VALID | JPG_MSK_LINE_BUF_COMPONENT_3_VALID | JPG_MSK_LINE_BUF_COMPONENT_4_VALID)
};

JPG_YUV_TO_RGB yuv2RgbMatrix =
{
	0x0100, // _11
	0x015f, // _13
	0x0100, // _21
	0x07aa, // _22
	0x074d, // _23
	0x0100, // _31
	0x01bb, // _32
	0x0351, // ConstR
	0x0084, // ConstG
	0x0322, // ConstB
	0,      // Reserved
	0,	    // Vip mode = RGB565
};

uint8_t JPGTilingTable[5][32] =
{ 
  //{  0,  1,  2,  3, 4, 5, 6,  7,  8,  9, 10,  11, 12,  13,  14, 15,  // pitch = 512
  //   16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
  {  0,  1,  2,  9, 10, 11, 12,  3,  4,  5, 13,  6, 14,  7,  8, 15,  // pitch = 512
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
  {  0,  1,  2, 10, 11, 12, 13,  3,  4,  5, 14,  6, 15,  7,  8,  9,  // pitch = 1024
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
  {  0,  1,  2, 11, 12, 13, 14,  3,  4,  5, 15,  6, 16,  7,  8,  9,  // pitch = 2048
    10, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
  {  0,  1,  2, 12, 13, 14, 15,  3,  4,  5, 16,  6, 7,  8,  9,  10,  // pitch = 4096
     11, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
  //{  0,  1,  2, 12, 13, 14, 15,  3,  4,  5, 16,  6, 17,  7,  8,  9,  // pitch = 4096
  //  10, 11, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
  {  0,  1,  2, 13, 14, 15, 16,  3,  4,  5, 17,  6, 18,  7,  8,  9,  // pitch = 8096
    10, 11, 12, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 } };

//=============================================================================
//                  Private Function Definition
//=============================================================================
static void jpgWriteRegUsingCmdQ(uint32_t addr, uint32_t data)
{
#ifdef CFG_CMDQ_ENABLE
    uint32_t        requestCmdQLen = 0;
    uint32_t*       cmdqAddr = NULL;

    ithCmdQLock(ITH_CMDQ0_OFFSET);

    cmdqAddr = ithCmdQWaitSize(8, ITH_CMDQ0_OFFSET);
    ITH_CMDQ_SINGLE_CMD(cmdqAddr, addr, data);

    ithCmdQFlush(cmdqAddr, ITH_CMDQ0_OFFSET);
    ithCmdQUnlock(ITH_CMDQ0_OFFSET);
#endif    
}

static uint32_t jpgWriteRegMaskUsingCmdQ(uint32_t addr, uint32_t data, uint32_t mask, uint32_t ori_data)
{
#ifdef CFG_CMDQ_ENABLE
    uint32_t        requestCmdQLen = 0;
    uint32_t*       cmdqAddr = NULL;
    uint32_t        w_data;

    ithCmdQLock(ITH_CMDQ0_OFFSET);
    
    cmdqAddr = ithCmdQWaitSize(8, ITH_CMDQ0_OFFSET);

    //w_data = (ithReadRegA(addr) & ~mask) | (data & mask);
    w_data = (ori_data & ~mask) | (data & mask);
    ITH_CMDQ_SINGLE_CMD(cmdqAddr, addr, w_data);

    ithCmdQFlush(cmdqAddr, ITH_CMDQ0_OFFSET);
    ithCmdQUnlock(ITH_CMDQ0_OFFSET);

    return w_data;
#endif    
}

static uint8_t RemapTableSel(uint16_t pitch)
{
    uint8_t index;
    
    switch (pitch)
    {
    case 512:
        index = 0;
        break;
    case 1024:
    	  index = 1;
        break;
    case 2048:
    	  index = 2;
        break;
    case 4096:
        index = 3;
        break;
    case 8192:
        index = 4;
        break;
    default:
    	  index = 0;
        break;
    }
    
    return index;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
/////////////////////////////////////////////////
// Base Engine API
////////////////////////////////////////////////
void
_LogReg(
    bool    bMsgOn)
{
    JPG_REG   reg, p;
    uint32_t  i, j = 0, count = 0;

    if( bMsgOn == false )       return;

    reg     = REG_JPG_BASE;
    count   = (0xD0200112 - REG_JPG_BASE) / sizeof(JPG_REG);
    p = reg;

    jpg_msg(1, "\n\t   2    0    6    4    A    8    E    C\r\n");
    for(i = 0; i < count; ++i)
    {
        JPG_REG   value = 0;

        jpgReadReg(p, &value);
        if( j == 0 )    jpg_msg(1, "0x%08X:", p);

        jpg_msg(1, " %08X", value);
        if( j >= 3 )
        {
            jpg_msg(1, "\r\n");
            j = 0;
        }
        else
            j++;

        p += 4;
    }

    if( j > 0 )
        jpg_msg(1, "\r\n");

    return;
}

void
_LogRGBModeReg(
    bool    bMsgOn)
{
    JPG_REG   reg, p;
    uint32_t  i, j = 0, count = 0;

    if( bMsgOn == false )       return;

    reg     = 0xD0201000;
    count   = (0xD0201024 - 0xD0201000) / sizeof(JPG_REG);
    p = reg;

    jpg_msg(1, "\n\t   2    0    6    4    A    8    E    C\r\n");
    for(i = 0; i < count; ++i)
    {
        JPG_REG   value = 0;

        jpgReadReg(p, &value);
        if( j == 0 )    jpg_msg(1, "0x%08X:", p);

        jpg_msg(1, " %08X", value);
        if( j >= 3 )
        {
            jpg_msg(1, "\r\n");
            j = 0;
        }
        else
            j++;

        p += 4;
    }

    if( j > 0 )
        jpg_msg(1, "\r\n");

    return;
}


void
_LogRemapReg(
    bool    bMsgOn)
{
    JPG_REG   reg, p;
    uint32_t  i, j = 0, count = 0;

    if( bMsgOn == false )       return;

    reg     = REG_JPG_BASE_EXT;
    count   = (0x0F44 - REG_JPG_BASE_EXT) / sizeof(JPG_REG);
    p = reg;

    jpg_msg(1, "\n\t   0    2    4    6    8    A    C    E\r\n");
    for(i = 0; i < count; ++i)
    {
        JPG_REG   value = 0;

        jpgReadReg(p, &value);
        if( j == 0 )    jpg_msg(1, "0x%04X:", p);

        jpg_msg(1, " %04X", value);
        if( j >= 7 )
        {
            jpg_msg(1, "\r\n");
            j = 0;
        }
        else
            j++;

        p += 2;
    }

    if( j > 0 )
        jpg_msg(1, "\r\n");

    return;
}


void
JPG_RegReset(
    void)
{
    if (b_JPG_REGS_USE_CMDQ)
        memset(&jpg_regs, 0, sizeof(JPG_HW_REGS));

    jpgResetHwReg();
    jpgResetHwEngine();
}

void
JPG_PowerUp(
    void)
{
    jpgEnableClock();

    JPG_RegReset();
}

//=============================================================================
/**
 * JPG encoder Power down (No isp)
 *
 * @param void
 * @return void
 */
//=============================================================================
void
JPG_EncPowerDown(
    void)
{
    volatile JPG_REG   hwStatus = 0;
    uint16_t           timeOut = 0;

    hwStatus = JPG_GetEngineStatusReg();
    if( (hwStatus&0xF800) == 0x3000 )
    {
        jpg_sleep(2);
        hwStatus = JPG_GetEngineStatusReg();
        if( (hwStatus&0xF800) == 0x3000 )
        {
            JPG_RegReset();
            jpg_msg_ex(JPG_MSG_TYPE_ERR, "time out !!");
            goto end;
        }
    }

    hwStatus = JPG_GetProcStatusReg();
    while( !(hwStatus & JPG_STATUS_ENCODE_COMPLETE) )
    {
        jpg_sleep(10);
        timeOut++;
        if( timeOut > 100 )
        {
            // 1 sec timeOut
            //JPG_RegReset();
            jpg_msg_ex(JPG_MSG_TYPE_ERR, "time out (status= 0x%x)!!", hwStatus);
            break;
        }
        hwStatus = 0;
        hwStatus = JPG_GetProcStatusReg();
    }
	
end :
    // ------------------------------
    // for power consumption
    timeOut = 0;
    do
    {
        // Wait JPG streaming buffer empty, avoid cpu hang
        hwStatus = JPG_GetEngineStatusReg();
        if( (hwStatus & 0xc000) != 0 && timeOut < 1000 )
           jpg_sleep(1);
        else
            break;
    } while(timeOut++);
    JPG_RegReset();      // clear regist value
    jpgDisableClock();   // disable clock
    // -------------------------------------
    return;

}


//=============================================================================
/**
 * JPG decoder Power down
 *
 * @param void
 * @return void
 */
//=============================================================================
void
JPG_DecPowerDown(
    void)
{
    volatile JPG_REG   hwStatus = 0;
    uint16_t           timeOut = 0;

#ifdef CFG_CMDQ_ENABLE
    ithCmdQWaitEmpty(ITH_CMDQ0_OFFSET);
#endif

	hwStatus = JPG_GetProcStatusReg();
    while( !(hwStatus & JPG_STATUS_DECODE_COMPLETE) )
    {
        jpg_sleep(1);
       
        timeOut++;
        if( timeOut > 500 )
        {
            // 1 sec timeOut
            JPG_RegReset();
            jpg_sleep(500);
            //Jpg_Ext_Link_Ctrl(JEL_CTRL_ISP_HW_RESET, 0, 0);
            jpg_msg_ex(JPG_MSG_TYPE_ERR, "time out (status[0xB00]= 0x%x)!!", hwStatus);
            break;
        }
        hwStatus = 0;
        hwStatus = JPG_GetProcStatusReg();
    }

    hwStatus = JPG_GetEngineStatusReg();
    if( (hwStatus&0xF800) == 0x3000 )
    {
        jpg_sleep(1);
        hwStatus = JPG_GetEngineStatusReg();
        if( (hwStatus&0xF800) == 0x3000 )
        {
            JPG_RegReset();
            jpg_sleep(500);
            //Jpg_Ext_Link_Ctrl(JEL_CTRL_ISP_HW_RESET, 0, 0);
            jpg_msg_ex(JPG_MSG_TYPE_ERR, "time out !!");
            goto end;
        }
    }

end :
    // ------------------------------
    // for power consumption
    // maybe have side effect (When enter suspend, jpg no return ack and then isp busy ??)
    timeOut = 0;
    do
    {
        // Wait JPG streaming buffer empty, avoid cpu hang
        hwStatus = JPG_GetEngineStatusReg();
        if ((hwStatus & 0xc000) != 0 && timeOut < 1000)
           jpg_sleep(1);
        else
            break;
    } while(timeOut++);
    JPG_RegReset();      // clear regist value
    jpgDisableClock();   // disable clock
    // -------------------------------------
    return;
}

/////////////////////////////////////////////////
// Access Register API
////////////////////////////////////////////////
// 0x00
void
JPG_SetCodecCtrlReg(
    JPG_REG   data)
{
	//doesn`t need to modity.
	if(b_JPG_REGS_USE_CMDQ)
    {   
        uint32_t w_data;
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_CODEC_CTRL, (JPG_REG)data, (JPG_REG)(~JPG_MSK_BITSTREAM_READ_BYTE_POS), jpg_regs.reg0);
        jpg_regs.reg0 = w_data;
    }
    else
    {
        jpgWriteRegMask(REG_JPG_CODEC_CTRL, (JPG_REG)data, (JPG_REG)(~JPG_MSK_BITSTREAM_READ_BYTE_POS));
    }    
}

void
JPG_SetBitstreamReadBytePosReg(
    JPG_REG   data)
{
	//doesn`t need to modity.
	if(b_JPG_REGS_USE_CMDQ)
    {   
        uint32_t w_data;
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_CODEC_CTRL,
                   (JPG_REG)(data << JPG_SHT_BITSTREAM_READ_BYTE_POS),
                   JPG_MSK_BITSTREAM_READ_BYTE_POS,
                   jpg_regs.reg0);
        jpg_regs.reg0 = w_data;
    }
    else
    {
        jpgWriteRegMask(REG_JPG_CODEC_CTRL,
                   (JPG_REG)(data << JPG_SHT_BITSTREAM_READ_BYTE_POS),
                   JPG_MSK_BITSTREAM_READ_BYTE_POS);
    }
}

// 0x02
void
JPG_SetDriReg(
    JPG_REG   data)
{
    if(b_JPG_REGS_USE_CMDQ)
    {
        uint32_t w_data;
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_DRI_SETTING, (JPG_REG)data << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000, jpg_regs.reg0);
        jpg_regs.reg0 = w_data;
    }
    else
    {
	    jpgWriteRegMask(REG_JPG_DRI_SETTING, (JPG_REG)data << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000);
    }
}

// 0x04
void
JPG_SetTableSpecifyReg(
    const JPG_FRM_COMP      *frmComp)
{
    uint32_t w_data;
    JPG_REG     data = 0;
    // Hw issue
    uint16_t    qTableSel_Y  = 0x0;  // 00
    uint16_t    qTableSel_UV = 0x1;  // 01

    data = (frmComp->jFrmInfo[0].acHuffTableSel << JPG_SHT_COMPONENT_A_AC_HUFFMAN_TABLE) |
           (frmComp->jFrmInfo[0].dcHuffTableSel << JPG_SHT_COMPONENT_A_DC_HUFFMAN_TABLE) |
           (qTableSel_Y << JPG_SHT_COMPONENT_A_Q_TABLE) |
           (frmComp->jFrmInfo[1].acHuffTableSel << JPG_SHT_COMPONENT_B_AC_HUFFMAN_TABLE) |
           (frmComp->jFrmInfo[1].dcHuffTableSel << JPG_SHT_COMPONENT_B_DC_HUFFMAN_TABLE) |
           (qTableSel_UV << JPG_SHT_COMPONENT_B_Q_TABLE) |
           (frmComp->jFrmInfo[2].acHuffTableSel << JPG_SHT_COMPONENT_C_AC_HUFFMAN_TABLE) |
           (frmComp->jFrmInfo[2].dcHuffTableSel << JPG_SHT_COMPONENT_C_DC_HUFFMAN_TABLE) |
           (qTableSel_UV << JPG_SHT_COMPONENT_C_Q_TABLE) |
           (frmComp->jFrmInfo[3].acHuffTableSel << JPG_SHT_COMPONENT_D_AC_HUFFMAN_TABLE) |
           (frmComp->jFrmInfo[3].dcHuffTableSel << JPG_SHT_COMPONENT_D_DC_HUFFMAN_TABLE) |
           (frmComp->jFrmInfo[3].qTableSel << JPG_SHT_COMPONENT_D_Q_TABLE);
    if(b_JPG_REGS_USE_CMDQ)
    {
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_TABLE_SPECIFY, data , 0xFFFF, jpg_regs.reg1);
        jpg_regs.reg1 = w_data;
    }
    else
    {
	    jpgWriteRegMask(REG_JPG_TABLE_SPECIFY, data , 0xFFFF);
    }
}

// 0x06, 0x08, 0xF0~0xFA
void
JPG_SetFrmSizeInfoReg(
    const JPG_FRM_SIZE_INFO     *sizeInfo)
{
    if(b_JPG_REGS_USE_CMDQ)
    {
        uint32_t w_data;
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_DISPLAY_MCU_WIDTH_Y, (JPG_REG)sizeInfo->mcuDispWidth << JPG_SHT_TO_HIGH_WORD, JPG_MSK_MCU << JPG_SHT_TO_HIGH_WORD, jpg_regs.reg1);
        jpg_regs.reg1 = w_data;

        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_DISPLAY_MCU_HEIGHT_Y, sizeInfo->mcuDispHeight, JPG_MSK_MCU, jpg_regs.reg2);
        jpg_regs.reg2 = w_data;

        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_ORIGINAL_MCU_WIDTH, sizeInfo->mcuRealWidth, JPG_MSK_MCU, jpg_regs.reg15);
        jpg_regs.reg15 = w_data;

        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_ORIGINAL_MCU_HEIGHT, (JPG_REG)sizeInfo->mcuRealHeight << JPG_SHT_TO_HIGH_WORD , JPG_MSK_MCU << JPG_SHT_TO_HIGH_WORD, jpg_regs.reg15);
        jpg_regs.reg15 = w_data;

        // set processed MCU range
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_LEFT_MCU_OFFSET, (JPG_REG)(sizeInfo->mcuDispLeft + 1), JPG_MSK_MCU, jpg_regs.reg16);
        jpg_regs.reg16 = w_data;

        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_RIGHT_MCU_OFFSET, (JPG_REG)sizeInfo->mcuDispRight << JPG_SHT_TO_HIGH_WORD, JPG_MSK_MCU << JPG_SHT_TO_HIGH_WORD, jpg_regs.reg16);
        jpg_regs.reg16 = w_data;

        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_UP_MCU_OFFSET, (JPG_REG)(sizeInfo->mcuDispUp + 1), JPG_MSK_MCU, jpg_regs.reg17);
        jpg_regs.reg17 = w_data;

        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_DOWN_MCU_OFFSET, sizeInfo->mcuDispDown << JPG_SHT_TO_HIGH_WORD , JPG_MSK_MCU << JPG_SHT_TO_HIGH_WORD, jpg_regs.reg17);
        jpg_regs.reg17 = w_data;
    }
    else
    {
        jpgWriteRegMask(REG_JPG_DISPLAY_MCU_WIDTH_Y, (JPG_REG)sizeInfo->mcuDispWidth << JPG_SHT_TO_HIGH_WORD, JPG_MSK_MCU << JPG_SHT_TO_HIGH_WORD);
    	
        jpgWriteRegMask(REG_JPG_DISPLAY_MCU_HEIGHT_Y, sizeInfo->mcuDispHeight, JPG_MSK_MCU);
        jpgWriteRegMask(REG_JPG_ORIGINAL_MCU_WIDTH, sizeInfo->mcuRealWidth, JPG_MSK_MCU);
    	jpgWriteRegMask(REG_JPG_ORIGINAL_MCU_HEIGHT, (JPG_REG)sizeInfo->mcuRealHeight << JPG_SHT_TO_HIGH_WORD , JPG_MSK_MCU << JPG_SHT_TO_HIGH_WORD);

        // set processed MCU range
        jpgWriteRegMask(REG_JPG_LEFT_MCU_OFFSET, (JPG_REG)(sizeInfo->mcuDispLeft + 1), JPG_MSK_MCU);   
        jpgWriteRegMask(REG_JPG_RIGHT_MCU_OFFSET, (JPG_REG)sizeInfo->mcuDispRight << JPG_SHT_TO_HIGH_WORD, JPG_MSK_MCU << JPG_SHT_TO_HIGH_WORD);
        
        jpgWriteRegMask(REG_JPG_UP_MCU_OFFSET, (JPG_REG)(sizeInfo->mcuDispUp + 1), JPG_MSK_MCU);
    	jpgWriteRegMask(REG_JPG_DOWN_MCU_OFFSET, sizeInfo->mcuDispDown << JPG_SHT_TO_HIGH_WORD , JPG_MSK_MCU << JPG_SHT_TO_HIGH_WORD);
    }
}

// 0x0A~0x1A
void
JPG_SetLineBufInfoReg(
    const JPG_LINE_BUF_INFO     *lineBufInfo)
{
    uint32_t w_data;
    uint32_t addr = 0;

    // component 1 starting address
    addr = (uint32_t)lineBufInfo->comp1Addr;
    addr >>= 2;
    if(b_JPG_REGS_USE_CMDQ)
    {
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_LINE_BUF_ADDR_A_COMPONENT_L, (JPG_REG)(addr & JPG_MSK_LINE_BUF_ADDR_L) << JPG_SHT_TO_HIGH_WORD, JPG_MSK_LINE_BUF_ADDR_L << JPG_SHT_TO_HIGH_WORD, jpg_regs.reg2);
        jpg_regs.reg2 = w_data;
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_LINE_BUF_ADDR_A_COMPONENT_H, (JPG_REG)((addr >> 16) & JPG_MSK_LINE_BUF_ADDR_H), JPG_MSK_LINE_BUF_ADDR_H, jpg_regs.reg3);
        jpg_regs.reg3 = w_data;
    }
    else
    {
    	jpgWriteRegMask(REG_JPG_LINE_BUF_ADDR_A_COMPONENT_L, (JPG_REG)(addr & JPG_MSK_LINE_BUF_ADDR_L) << JPG_SHT_TO_HIGH_WORD, JPG_MSK_LINE_BUF_ADDR_L << JPG_SHT_TO_HIGH_WORD);
        jpgWriteRegMask(REG_JPG_LINE_BUF_ADDR_A_COMPONENT_H, (JPG_REG)((addr >> 16) & JPG_MSK_LINE_BUF_ADDR_H), JPG_MSK_LINE_BUF_ADDR_H);
    }

    // component 2 starting address
    addr = (uint32_t)lineBufInfo->comp2Addr;
    addr >>= 2;

    if(b_JPG_REGS_USE_CMDQ)
    {
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_LINE_BUF_ADDR_B_COMPONENT_L, (JPG_REG)(addr & JPG_MSK_LINE_BUF_ADDR_L) << JPG_SHT_TO_HIGH_WORD , JPG_MSK_LINE_BUF_ADDR_L << JPG_SHT_TO_HIGH_WORD, jpg_regs.reg3);
        jpg_regs.reg3 = w_data;
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_LINE_BUF_ADDR_B_COMPONENT_H, (JPG_REG)((addr >> 16) & JPG_MSK_LINE_BUF_ADDR_H), JPG_MSK_LINE_BUF_ADDR_H, jpg_regs.reg4);
        jpg_regs.reg4 = w_data;
    }
    else
    {
    	jpgWriteRegMask(REG_JPG_LINE_BUF_ADDR_B_COMPONENT_L, (JPG_REG)(addr & JPG_MSK_LINE_BUF_ADDR_L) << JPG_SHT_TO_HIGH_WORD , JPG_MSK_LINE_BUF_ADDR_L << JPG_SHT_TO_HIGH_WORD);
        jpgWriteRegMask(REG_JPG_LINE_BUF_ADDR_B_COMPONENT_H, (JPG_REG)((addr >> 16) & JPG_MSK_LINE_BUF_ADDR_H), JPG_MSK_LINE_BUF_ADDR_H);
    }
    // component 3 starting address
    addr = (uint32_t)lineBufInfo->comp3Addr;
    addr >>= 2;

    if(b_JPG_REGS_USE_CMDQ)
    {
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_LINE_BUF_ADDR_C_COMPONENT_L, (JPG_REG)(addr & JPG_MSK_LINE_BUF_ADDR_L) << JPG_SHT_TO_HIGH_WORD, JPG_MSK_LINE_BUF_ADDR_L << JPG_SHT_TO_HIGH_WORD, jpg_regs.reg4);
        jpg_regs.reg4 = w_data;
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_LINE_BUF_ADDR_C_COMPONENT_H, (JPG_REG)((addr >> 16) & JPG_MSK_LINE_BUF_ADDR_H), JPG_MSK_LINE_BUF_ADDR_H, jpg_regs.reg5);
        jpg_regs.reg5 = w_data;
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_LINE_BUF_PITCH_COMPONENT_A,
                       (JPG_REG)((lineBufInfo->comp1Pitch >> 2) & JPG_MSK_LINE_BUF_PITCH),
                       JPG_MSK_LINE_BUF_PITCH,
                       jpg_regs.reg6);
        jpg_regs.reg6 = w_data;
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_LINE_BUF_PITCH_COMPONENT_BC,
                   (JPG_REG)((lineBufInfo->comp23Pitch >> 2) & JPG_MSK_LINE_BUF_PITCH) << JPG_SHT_TO_HIGH_WORD,
                   JPG_MSK_LINE_BUF_PITCH << JPG_SHT_TO_HIGH_WORD,
                   jpg_regs.reg6);
        jpg_regs.reg6 = w_data;
    }
    else
    {
    	jpgWriteRegMask(REG_JPG_LINE_BUF_ADDR_C_COMPONENT_L, (JPG_REG)(addr & JPG_MSK_LINE_BUF_ADDR_L) << JPG_SHT_TO_HIGH_WORD, JPG_MSK_LINE_BUF_ADDR_L << JPG_SHT_TO_HIGH_WORD);
        jpgWriteRegMask(REG_JPG_LINE_BUF_ADDR_C_COMPONENT_H, (JPG_REG)((addr >> 16) & JPG_MSK_LINE_BUF_ADDR_H), JPG_MSK_LINE_BUF_ADDR_H);

        // component 1 pitch
        jpgWriteRegMask(REG_JPG_LINE_BUF_PITCH_COMPONENT_A,
                       (JPG_REG)((lineBufInfo->comp1Pitch >> 2) & JPG_MSK_LINE_BUF_PITCH),
                       JPG_MSK_LINE_BUF_PITCH);
        // component 2/3 pitch
        jpgWriteRegMask(REG_JPG_LINE_BUF_PITCH_COMPONENT_BC,
                   (JPG_REG)((lineBufInfo->comp23Pitch >> 2) & JPG_MSK_LINE_BUF_PITCH) << JPG_SHT_TO_HIGH_WORD,
                   JPG_MSK_LINE_BUF_PITCH << JPG_SHT_TO_HIGH_WORD);
    }    
}

// 0x16
void
JPG_SetLineBufSliceUnitReg(
    JPG_REG   data,
    JPG_REG   yVerticalSamp)
{
    if(b_JPG_REGS_USE_CMDQ)
    {
        uint32_t w_data;
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_LINE_BUF_SLICE_NUM,
                   (JPG_REG)((data / yVerticalSamp) & JPG_MSK_LINE_BUF_SLICE_NUM) << JPG_SHT_TO_HIGH_WORD,
                   JPG_MSK_LINE_BUF_SLICE_NUM << JPG_SHT_TO_HIGH_WORD,
                   jpg_regs.reg5);
        jpg_regs.reg5 = w_data;
    }
    else
    {
        jpgWriteRegMask(REG_JPG_LINE_BUF_SLICE_NUM,
                   (JPG_REG)((data / yVerticalSamp) & JPG_MSK_LINE_BUF_SLICE_NUM) << JPG_SHT_TO_HIGH_WORD,
                   JPG_MSK_LINE_BUF_SLICE_NUM << JPG_SHT_TO_HIGH_WORD);
    }
}

// 0x1C
void
JPG_SetLineBufSliceWriteNumReg(
    JPG_REG   data)
{
    // for encode, set how many slice is ready to encode.
    if(b_JPG_REGS_USE_CMDQ)
    {
        uint32_t w_data;
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_LINE_BUF_SLICE_WRITE,
                   (JPG_REG)(data & JPG_MSK_LINE_BUF_SLICE_WRITE),
                   JPG_MSK_LINE_BUF_SLICE_WRITE,
                   jpg_regs.reg7);
        jpg_regs.reg7 = w_data;
    }
    else
    {
        jpgWriteRegMask(REG_JPG_LINE_BUF_SLICE_WRITE,
                   (JPG_REG)(data & JPG_MSK_LINE_BUF_SLICE_WRITE),
                   JPG_MSK_LINE_BUF_SLICE_WRITE);
    }
}

// 0x1E, 0x20, 0x22, 0x24
void
JPG_SetBitStreamBufInfoReg(
    const JPG_HW_BS_CTRL   *bsBufInfo)
{
    uint32_t    w_data;
    uint32_t    addr = 0;
    uint32_t    size = 0;

    // Bit-Stream buffer starting address
    addr = (uint32_t)bsBufInfo->addr;
    addr >>= 2;

    if(b_JPG_REGS_USE_CMDQ)
    {
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_BITSTREAM_BUF_ADDR_L, (JPG_REG)(addr & JPG_MSK_BITSTREAM_BUF_ADDR_L) << JPG_SHT_TO_HIGH_WORD, JPG_MSK_BITSTREAM_BUF_ADDR_L << JPG_SHT_TO_HIGH_WORD, jpg_regs.reg7);
        jpg_regs.reg7 = w_data;
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_BITSTREAM_BUF_ADDR_H, (JPG_REG)((addr >> 16) & JPG_MSK_BITSTREAM_BUF_ADDR_H), JPG_MSK_BITSTREAM_BUF_ADDR_H, jpg_regs.reg8);
        jpg_regs.reg8 = w_data;
    }
    else
    {
        jpgWriteRegMask(REG_JPG_BITSTREAM_BUF_ADDR_L, (JPG_REG)(addr & JPG_MSK_BITSTREAM_BUF_ADDR_L) << JPG_SHT_TO_HIGH_WORD, JPG_MSK_BITSTREAM_BUF_ADDR_L << JPG_SHT_TO_HIGH_WORD);
        jpgWriteRegMask(REG_JPG_BITSTREAM_BUF_ADDR_H, (JPG_REG)((addr >> 16) & JPG_MSK_BITSTREAM_BUF_ADDR_H), JPG_MSK_BITSTREAM_BUF_ADDR_H);
    }    
    
    //Bit-Stream buffer size
    size = bsBufInfo->size >> 2;

    if(b_JPG_REGS_USE_CMDQ)
    {
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_BITSTREAM_BUF_SIZE_L, (JPG_REG)(size & JPG_MSK_BITSTREAM_BUF_SIZE_L) << JPG_SHT_TO_HIGH_WORD, JPG_MSK_BITSTREAM_BUF_SIZE_L << JPG_SHT_TO_HIGH_WORD, jpg_regs.reg8);
        jpg_regs.reg8 = w_data;
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_BITSTREAM_BUF_SIZE_H, (JPG_REG)((size >> 16) & JPG_MSK_BITSTREAM_BUF_SIZE_H), JPG_MSK_BITSTREAM_BUF_SIZE_H, jpg_regs.reg9);
        jpg_regs.reg9 = w_data;
    }
    else
    {
	    jpgWriteRegMask(REG_JPG_BITSTREAM_BUF_SIZE_L, (JPG_REG)(size & JPG_MSK_BITSTREAM_BUF_SIZE_L) << JPG_SHT_TO_HIGH_WORD, JPG_MSK_BITSTREAM_BUF_SIZE_L << JPG_SHT_TO_HIGH_WORD);
        jpgWriteRegMask(REG_JPG_BITSTREAM_BUF_SIZE_H, (JPG_REG)((size >> 16) & JPG_MSK_BITSTREAM_BUF_SIZE_H), JPG_MSK_BITSTREAM_BUF_SIZE_H);
    }  
}

// 0x26, 0x28
void
JPG_SetBitstreamBufRwSizeReg(
    uint32_t    data)
{
    uint32_t    wrSize = data >> 2;

    // Use Ring buf, Read: H/W remain buf size (endoder), Write: S/W move out data length (decoder)
    if(b_JPG_REGS_USE_CMDQ)
    {
        uint32_t w_data;
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_BITSTREAM_RW_SIZE_L, (JPG_REG)(wrSize & JPG_MSK_BITSTREAM_RW_SIZE_L) << JPG_SHT_TO_HIGH_WORD , JPG_MSK_BITSTREAM_RW_SIZE_L << JPG_SHT_TO_HIGH_WORD, jpg_regs.reg9);
        jpg_regs.reg9 = w_data;
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_BITSTREAM_RW_SIZE_H, (JPG_REG)((wrSize >> 16) & JPG_MSK_BITSTREAM_RW_SIZE_H), JPG_MSK_BITSTREAM_RW_SIZE_H, jpg_regs.reg10);
        jpg_regs.reg10 = w_data;
    }
    else
    {
        jpgWriteRegMask(REG_JPG_BITSTREAM_RW_SIZE_L, (JPG_REG)(wrSize & JPG_MSK_BITSTREAM_RW_SIZE_L) << JPG_SHT_TO_HIGH_WORD , JPG_MSK_BITSTREAM_RW_SIZE_L << JPG_SHT_TO_HIGH_WORD);
        jpgWriteRegMask(REG_JPG_BITSTREAM_RW_SIZE_H, (JPG_REG)((wrSize >> 16) & JPG_MSK_BITSTREAM_RW_SIZE_H), JPG_MSK_BITSTREAM_RW_SIZE_H);
    }  
}

// 0x2A, 0x2D
void
JPG_SetSamplingFactorReg(
    const JPG_FRM_COMP      *frmComp)
{
    uint32_t w_data;
    JPG_REG   data = 0;

    data = (frmComp->jFrmInfo[0].horizonSamp  << JPG_SHT_SAMPLING_FACTOR_A_H) |
           (frmComp->jFrmInfo[0].verticalSamp << JPG_SHT_SAMPLING_FACTOR_A_V) |
           (frmComp->jFrmInfo[1].horizonSamp  << JPG_SHT_SAMPLING_FACTOR_B_H) |
           (frmComp->jFrmInfo[1].verticalSamp << JPG_SHT_SAMPLING_FACTOR_B_V);

    if(b_JPG_REGS_USE_CMDQ)
    {
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_SAMPLING_FACTOR_AB, data << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000, jpg_regs.reg10);
        jpg_regs.reg10 = w_data;
    }
    else
    {
	    jpgWriteRegMask(REG_JPG_SAMPLING_FACTOR_AB, data << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000);
    }
    data = 0;
    data = (frmComp->jFrmInfo[2].horizonSamp  << JPG_SHT_SAMPLING_FACTOR_C_H) |
           (frmComp->jFrmInfo[2].verticalSamp << JPG_SHT_SAMPLING_FACTOR_C_V) |
           (frmComp->jFrmInfo[3].horizonSamp  << JPG_SHT_SAMPLING_FACTOR_D_H) |
           (frmComp->jFrmInfo[3].verticalSamp << JPG_SHT_SAMPLING_FACTOR_D_V);

    if(b_JPG_REGS_USE_CMDQ)
    {
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_SAMPLING_FACTOR_CD, data , 0xFFFF, jpg_regs.reg11);
        jpg_regs.reg11 = w_data;
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_LEFT_MCU_OFFSET,
                   (JPG_REG)(frmComp->jFrmInfo[0].verticalSamp << JPG_SHT_MCU_HEIGHT_BLOCK),
                   JPG_MSK_MCU_HEIGHT_BLOCK,
                   jpg_regs.reg16);
        jpg_regs.reg16 = w_data;
    }
    else
    {
	    jpgWriteRegMask(REG_JPG_SAMPLING_FACTOR_CD, data , 0xFFFF);

        // Set MCU block height
        jpgWriteRegMask(REG_JPG_LEFT_MCU_OFFSET,
                   (JPG_REG)(frmComp->jFrmInfo[0].verticalSamp << JPG_SHT_MCU_HEIGHT_BLOCK),
                   JPG_MSK_MCU_HEIGHT_BLOCK);
    }
    //Set MCU block number
    data = (frmComp->jFrmInfo[0].horizonSamp * frmComp->jFrmInfo[0].verticalSamp) +
           (frmComp->jFrmInfo[1].horizonSamp * frmComp->jFrmInfo[1].verticalSamp) +
           (frmComp->jFrmInfo[2].horizonSamp * frmComp->jFrmInfo[2].verticalSamp) - 1;
    if(b_JPG_REGS_USE_CMDQ)
    {
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_RIGHT_MCU_OFFSET,
	       		   (JPG_REG)(data << JPG_SHT_BLOCK_MCU_NUM) << JPG_SHT_TO_HIGH_WORD,
	       		   JPG_MSK_BLOCK_MCU_NUM << JPG_SHT_TO_HIGH_WORD,
	       		   jpg_regs.reg16);
        jpg_regs.reg16 = w_data;
    }
    else
    {
	    jpgWriteRegMask(REG_JPG_RIGHT_MCU_OFFSET,
	       		   (JPG_REG)(data << JPG_SHT_BLOCK_MCU_NUM) << JPG_SHT_TO_HIGH_WORD,
	       		   JPG_MSK_BLOCK_MCU_NUM << JPG_SHT_TO_HIGH_WORD);
    }
}

// 0x2E~0xED
// input the Zig-zag order
void
JPG_SetQtableReg(
    JPG_FRM_COMP    *frmComp)
{
    uint32_t w_data;
    uint32_t data = 0;
    JPG_Q_TABLE   *qTable = &frmComp->qTable;

    uint8_t       zz = 0, i = 0 , j=0;
    uint8_t       *curTable = 0;
    JPG_REG       reg = REG_JPG_INDEX0_QTABLE;

    for(i = 0; i < qTable->tableCnt; i++)
    {
        curTable = qTable->table[frmComp->jFrmInfo[i].qTableSel];

        if(b_JPG_REGS_USE_CMDQ)
        {
            switch( i )
            {
                case 0:
                {
                    reg = REG_JPG_INDEX0_QTABLE;
                    w_data = jpgWriteRegMaskUsingCmdQ((JPG_REG)(reg), (JPG_REG)(curTable[0] | (curTable[1] << 8)) << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000, jpg_regs.reg11);
                    jpg_regs.reg11 = w_data;
                    break;
                }
                case 1:
                {
                    reg = REG_JPG_INDEX1_QTABLE;
                    w_data = jpgWriteRegMaskUsingCmdQ((JPG_REG)(reg), (JPG_REG)(curTable[0] | (curTable[1] << 8)) << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000, jpg_regs.reg12);
                    jpg_regs.reg12 = w_data;
                    break;
                }    
                case 2:
                {
                    reg = REG_JPG_INDEX2_QTABLE;
                    w_data = jpgWriteRegMaskUsingCmdQ((JPG_REG)(reg), (JPG_REG)(curTable[0] | (curTable[1] << 8)) << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000, jpg_regs.reg13);
                    jpg_regs.reg13 = w_data;
                    break;
                }    
            }
        }
        else
        {
            switch( i )
            {
                case 0:     reg = REG_JPG_INDEX0_QTABLE;    break;
                case 1:     reg = REG_JPG_INDEX1_QTABLE;    break;
                case 2:     reg = REG_JPG_INDEX2_QTABLE;    break;
            }
    		
    	    //change to write 4Bytes Register. 
    		jpgWriteRegMask((JPG_REG)(reg), (JPG_REG)(curTable[0] | (curTable[1] << 8)) << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000);
        }
        
		j=1;
	 	for(zz = 2; zz < JPG_Q_TABLE_SIZE; zz += 4)
    	{
			data = 0;
			if(zz !=(JPG_Q_TABLE_SIZE-2))
			{
    			data = (curTable[zz] | (curTable[zz+1] << 8) |  (curTable[zz+2] << 16) |  (curTable[zz+3] << 24));
                if(b_JPG_REGS_USE_CMDQ)
                    jpgWriteRegUsingCmdQ((JPG_REG)(reg + 4*j), data);                
                else
				    jpgWriteReg((JPG_REG)(reg + 4*j), data);
			}
            else
			{
                if(b_JPG_REGS_USE_CMDQ)
                {
                    if(reg == REG_JPG_INDEX0_QTABLE)
                    {
                        w_data = jpgWriteRegMaskUsingCmdQ((JPG_REG)(reg+4*j), (JPG_REG)(curTable[zz] | (curTable[zz+1] << 8)), 0xFFFF, jpg_regs.reg12);
                        jpg_regs.reg12 = w_data;
                    }
                    else if(reg == REG_JPG_INDEX1_QTABLE)
                    {
                        w_data = jpgWriteRegMaskUsingCmdQ((JPG_REG)(reg+4*j), (JPG_REG)(curTable[zz] | (curTable[zz+1] << 8)), 0xFFFF, jpg_regs.reg13);
                        jpg_regs.reg13 = w_data;
                    }
                    else if(reg == REG_JPG_INDEX2_QTABLE)
                    {
                        w_data = jpgWriteRegMaskUsingCmdQ((JPG_REG)(reg+4*j), (JPG_REG)(curTable[zz] | (curTable[zz+1] << 8)), 0xFFFF, jpg_regs.reg14);
                        jpg_regs.reg14 = w_data;
                    }
                }
                else
                {
				    jpgWriteRegMask((JPG_REG)(reg+4*j), (JPG_REG)(curTable[zz] | (curTable[zz+1] << 8)), 0xFFFF);
                }
			}
			j++;	
    	}
    }
}

// 0xEE
void
JPG_DropHv(
    JPG_REG   data)
{
    // set jpg engine down/up sample output (ex. 444 in -> 422 out)
    if(b_JPG_REGS_USE_CMDQ)
    {
        uint32_t w_data;
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_DROP_DUPLICATE, (JPG_REG)(data & JPG_MSK_DUPLICATE_H_V) << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000, jpg_regs.reg14);
        jpg_regs.reg14 = w_data;
    }
    else
    {
	    jpgWriteRegMask(REG_JPG_DROP_DUPLICATE, (JPG_REG)(data & JPG_MSK_DUPLICATE_H_V) << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000);
    }
}

// 0xFC
void
JPG_StartReg(
    void)
{
#if 0 // for write back case
    ithFlushDCache();
    ithInvalidateDCache();
#endif
    // fire jpg engine
    if(b_JPG_REGS_USE_CMDQ)
    {
        ithPrintf("YC: %s, %d\n", __FUNCTION__, __LINE__);
        uint32_t w_data;
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_CODEC_FIRE, (JPG_REG)JPG_MSK_START_CODEC ,0xFFFF, jpg_regs.reg18);
        jpg_regs.reg18 = w_data;
    }
    else
    {    
	    jpgWriteRegMask(REG_JPG_CODEC_FIRE, (JPG_REG)JPG_MSK_START_CODEC ,0xFFFF);
    }
}

// 0xFE
JPG_REG
JPG_GetEngineStatusReg(
    void)
{
    JPG_REG value = 0;

    // H/W debug register
    jpgReadReg(REG_JPG_ENGINE_STATUS_0, &value);
	value >>= 16; //right shift 16. 

    return value;
}

// 0x100
JPG_REG
JPG_GetProcStatusReg( //JPG_GetEngineStatus1Reg(
    void)
{
    JPG_REG value = 0;

    // decode/encode status (line buf and stream buf status)
    jpgReadReg(REG_JPG_ENGINE_STATUS_1,  &value);

    return value;
}

// 0x102
void
JPG_SetLineBufCtrlReg(
    JPG_REG   data)
{
    if(b_JPG_REGS_USE_CMDQ)
    {
        uint32_t w_data;
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_LINE_BUF_CTRL, (JPG_REG)(data & JPG_MSK_LINE_BUF_CTRL) << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000, jpg_regs.reg19);
        jpg_regs.reg19 = w_data;
    }
    else
    {
	    jpgWriteRegMask(REG_JPG_LINE_BUF_CTRL, (JPG_REG)(data & JPG_MSK_LINE_BUF_CTRL) << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000);
    }
}

// 0x104
JPG_REG
JPG_GetLineBufValidSliceReg(
    void)
{
    JPG_REG value = 0;

    // H/W spec is not discrepted
    jpgReadReg(REG_JPG_LINE_BUF_VALID_SLICE, &value);

    return (JPG_REG)(value & JPG_MSK_LINE_BUF_VALID_SLICE);
}

// 0x106
void
JPG_SetBitstreamBufCtrlReg(
    JPG_REG   data)
{
    if(b_JPG_REGS_USE_CMDQ)
    {
        uint32_t w_data;
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_BITSTREAM_BUF_CTRL,
                   (JPG_REG)(data & JPG_MSK_BITSTREAM_BUF_CTRL) << JPG_SHT_TO_HIGH_WORD,
                   JPG_MSK_BITSTREAM_BUF_CTRL << JPG_SHT_TO_HIGH_WORD,
                   jpg_regs.reg20);
        jpg_regs.reg20 = w_data;
    }
    else
    {
        jpgWriteRegMask(REG_JPG_BITSTREAM_BUF_CTRL,
                   (JPG_REG)(data & JPG_MSK_BITSTREAM_BUF_CTRL) << JPG_SHT_TO_HIGH_WORD,
                   JPG_MSK_BITSTREAM_BUF_CTRL << JPG_SHT_TO_HIGH_WORD);
    }
}

// 0x108
uint32_t
JPG_GetBitStreamValidSizeReg(
    void)
{
    volatile JPG_REG  low = 0;

    jpgReadReg(REG_JPG_BITSTREAM_VALID_SIZE_L, (JPG_REG*)&low);
    return (low);
}

// 0x10C
void
JPG_SetHuffmanCodeCtrlReg(
    JPG_HUFFMAN_TAB_SEL   tableSel,
    uint8_t               *pCodeLength)
{
    uint32_t    w_data;
    JPG_REG     Selection = 0x0000;
    JPG_REG     data = 0x0000;
    uint16_t    i = 0;

    switch( tableSel )
    {
        case JPG_HUUFFMAN_Y_DC:
            Selection = (JPG_HUFFMAN_DC_TABLE | JPG_HUFFMAN_LUMINANCE);
            break;

        case JPG_HUUFFMAN_UV_DC:
            Selection = (JPG_HUFFMAN_DC_TABLE | JPG_HUFFMAN_CHROMINANCE);
            break;

        case JPG_HUUFFMAN_Y_AC:
            Selection = (JPG_HUFFMAN_AC_TABLE | JPG_HUFFMAN_LUMINANCE);
            break;

        case JPG_HUUFFMAN_UV_AC:
            Selection = (JPG_HUFFMAN_AC_TABLE | JPG_HUFFMAN_CHROMINANCE);
            break;

        default:
            jpg_msg_ex(JPG_MSG_TYPE_ERR, "\nWrong parameters !! ");
            return;
    }

    // Needs to add new flag for 970 , Benson .
    
    // Reset
    if(b_JPG_REGS_USE_CMDQ)
    {
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_TABLE_SPECIFY , 0x3<<30, 0x3<<30, jpg_regs.reg1);
        jpg_regs.reg1 = w_data;
        jpgWriteRegUsingCmdQ(REG_JPG_HUFFMAN_CTRL,  (JPG_REG)(Selection));
    }
    else
    {    
        jpgWriteRegMask(REG_JPG_TABLE_SPECIFY , 0x3<<30, 0x3<<30); // low bit. 
	    jpgWriteRegMask(REG_JPG_HUFFMAN_CTRL,  (JPG_REG)(Selection),0xFFFF);
    }
    // Write Length number
    for(i = 1; i < 16; i++)
    {
        data = (Selection | (i << 8) | (*(pCodeLength + i - 1)));
        if(b_JPG_REGS_USE_CMDQ)
            jpgWriteRegUsingCmdQ(REG_JPG_HUFFMAN_CTRL, (JPG_REG)(data));
        else    
		    jpgWriteRegMask(REG_JPG_HUFFMAN_CTRL, (JPG_REG)(data) , 0xFFFF);
    }

    if(b_JPG_REGS_USE_CMDQ)
    {
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_TABLE_SPECIFY , 0x0<<30, 0x3<<30, jpg_regs.reg1);
        jpg_regs.reg1 = w_data;
    }
    else
	    jpgWriteRegMask(REG_JPG_TABLE_SPECIFY , 0x0<<30, 0x3<<30); // close the bit.
}

// 0xB0E
void
JPG_SetDcHuffmanValueReg(
    JPG_HUFFMAN_TAB_SEL     tableSel,
    uint8_t                 *pCodeValue,
    uint16_t                totalLength)
{
    uint32_t    w_data;
    JPG_REG     data = 0x0000;
    uint16_t    wTotalHTNum = 0x0000;
    uint16_t    i = 0;

    switch( tableSel )
    {
        case JPG_HUUFFMAN_Y_DC:
            wTotalHTNum = (totalLength + 1) >> 1;

            for(i = 0; i < wTotalHTNum; i++)
            {
                // Encode and Decode are the same
                if( (totalLength & 0x1) && (i == (wTotalHTNum - 1)) )
                {
                    data = (JPG_HUFFMAN_DC_LUMINANCE_TABLE | (i << 8) | (*(pCodeValue + 2 * i)));
                }
                else
                {
                    data = (JPG_HUFFMAN_DC_LUMINANCE_TABLE | (i << 8) | ((*(pCodeValue + 2 * i + 1)) << 4) | (*(pCodeValue + 2 * i)));
                }
                if(b_JPG_REGS_USE_CMDQ)
                {
                    w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_TABLE_SPECIFY , 0x1<<30, 0x1<<30, jpg_regs.reg1);
                    jpg_regs.reg1 = w_data;
                    jpgWriteRegUsingCmdQ(REG_JPG_HUFFMAN_DC_CTRL,  (JPG_REG)data << JPG_SHT_TO_HIGH_WORD);
                    w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_TABLE_SPECIFY , 0x0<<30, 0x1<<30, jpg_regs.reg1);
                    jpg_regs.reg1 = w_data;
                }
                else
                {
                    jpgWriteRegMask(REG_JPG_TABLE_SPECIFY , 0x1<<30, 0x1<<30); // high bit.
    				jpgWriteRegMask(REG_JPG_HUFFMAN_DC_CTRL,  (JPG_REG)data << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000);
    				jpgWriteRegMask(REG_JPG_TABLE_SPECIFY , 0x0<<30, 0x1<<30); // high bit.
                }               

                /*{
                    if (!(i % 10) && i != 0)
                        jpg_msg(1, " \n");
                    jpg_msg(1, "0x%04X ", data);
                    if( i== (wTotalHTNum -1))
                        jpg_msg(1, "\n");
                }
                //*/
            }
            break;

        case JPG_HUUFFMAN_UV_DC:
            wTotalHTNum = ((totalLength + 1) >> 1);

            for(i = 0; i < wTotalHTNum; i++)
            {
                // Encode and Decode are the same
                if ( (totalLength & 0x1) && (i == (wTotalHTNum - 1)) )
                {
                    data = (JPG_HUFFMAN_DC_CHROMINANCE_TABLE | (i << 8) | (*(pCodeValue + 2 * i)));
                }
                else
                {
                    data = (JPG_HUFFMAN_DC_CHROMINANCE_TABLE | (i << 8) | ((*(pCodeValue + 2 * i + 1)) << 4) | (*(pCodeValue + 2 * i)));
                }
                if(b_JPG_REGS_USE_CMDQ)
                {
                    w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_TABLE_SPECIFY , 0x1<<30, 0x1<<30, jpg_regs.reg1);
                    jpg_regs.reg1 = w_data;
                    jpgWriteRegUsingCmdQ(REG_JPG_HUFFMAN_DC_CTRL,  (JPG_REG)data  << JPG_SHT_TO_HIGH_WORD);
                    w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_TABLE_SPECIFY , 0x0<<30, 0x1<<30, jpg_regs.reg1);
                    jpg_regs.reg1 = w_data;
                }
                else
                {
                    jpgWriteRegMask(REG_JPG_TABLE_SPECIFY , 0x1<<30, 0x1<<30); // high bit.
    				jpgWriteRegMask(REG_JPG_HUFFMAN_DC_CTRL,  (JPG_REG)data  << JPG_SHT_TO_HIGH_WORD, 0xFFFF0000);
    				jpgWriteRegMask(REG_JPG_TABLE_SPECIFY , 0x0<<30, 0x1<<30); // high bit.
                }
                
                /*{
                    if (!(i % 10) && i != 0)
                        jpg_msg(1, " \n");
                    jpg_msg(1, "0x%04X ", data);

                    if( i== (wTotalHTNum -1))
                        jpg_msg(1, "\n");
                }
                //*/
            }
            break;

        default:
            jpg_msg_ex(JPG_MSG_TYPE_ERR, "\nWrong parameters !!");
            break;
    }
}

// 0x110
// 0x112
void
JPG_SetEncodeAcHuffmanValueReg(
    JPG_HUFFMAN_TAB_SEL     tableSel,
    uint8_t                 *pCodeValue,
    uint16_t                totalLength)
{
    uint32_t    w_data;
    JPG_REG     data = 0x0000;
    uint16_t    wTotalHTNum = 0x0000;
    uint16_t    i = 0;

    switch( tableSel )
    {
        case JPG_HUUFFMAN_Y_AC:
            wTotalHTNum = totalLength;

            if(b_JPG_REGS_USE_CMDQ)
            {
                w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_TABLE_SPECIFY , 0x3<<30, 0x3<<30, jpg_regs.reg1);
                jpg_regs.reg1 = w_data;
            }
            else
			    jpgWriteRegMask(REG_JPG_TABLE_SPECIFY , 0x3<<30, 0x3<<30);	//WJPG_FPGA_BEFlg(0x14)(bit30),0xWJPG_FPGA_LBen(0x15)(bit31)
			
            for(i = 0; i < wTotalHTNum; i++)
            {
                // Encode and Decode are different
                data = ((*(pCodeValue + i) << 8) | i);
                if(b_JPG_REGS_USE_CMDQ)
                    jpgWriteRegUsingCmdQ(REG_JPG_HUFFMAN_AC_LUMINANCE_CTRL, (JPG_REG)data);
                else
                    jpgWriteRegMask(REG_JPG_HUFFMAN_AC_LUMINANCE_CTRL, (JPG_REG)data , 0xFFFF);
            }

            if(b_JPG_REGS_USE_CMDQ)
            {
                w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_TABLE_SPECIFY , 0x0<<30, 0x3<<30, jpg_regs.reg1);
                jpg_regs.reg1 = w_data;
            }
            else            
			    jpgWriteRegMask(REG_JPG_TABLE_SPECIFY , 0x0<<30, 0x3<<30);
            
            break;

        case JPG_HUUFFMAN_UV_AC:
            wTotalHTNum = totalLength;
            if(b_JPG_REGS_USE_CMDQ)
            {
                w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_TABLE_SPECIFY , 0x1<<30, 0x1<<30, jpg_regs.reg1);
                jpg_regs.reg1 = w_data;
            }
            else
			    jpgWriteRegMask(REG_JPG_TABLE_SPECIFY , 0x1<<30, 0x1<<30);
            
            for(i = 0; i < wTotalHTNum; i++)
            {
                // Encode and Decode are different
                data = ((*(pCodeValue + i) << 8) | i );
                if(b_JPG_REGS_USE_CMDQ)
                    jpgWriteRegUsingCmdQ(REG_JPG_HUFFMAN_AC_CHROMINANCE_CTRL, (JPG_REG)data << JPG_SHT_TO_HIGH_WORD);
                else
				    jpgWriteRegMask(REG_JPG_HUFFMAN_AC_CHROMINANCE_CTRL, (JPG_REG)data << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000);
            }
            
            if(b_JPG_REGS_USE_CMDQ)
            {
                w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_TABLE_SPECIFY , 0x0<<30, 0x1<<30, jpg_regs.reg1);
                jpg_regs.reg1 = w_data;
            }
            else 
			    jpgWriteRegMask(REG_JPG_TABLE_SPECIFY , 0x0<<30, 0x1<<30);
            
            break;

        default:
            jpg_msg_ex(JPG_MSG_TYPE_ERR, "\nWrong parameters !!");
            break;
    }
}

// 0x110
// 0x112
void
JPG_SetDecodeAcHuffmanValueReg(
    JPG_HUFFMAN_TAB_SEL     tableSel,
    uint8_t                 *pCodeValue,
    uint16_t                totalLength)
{
	uint32_t    w_data;
    JPG_REG     data = 0x0000;
    uint16_t    wTotalHTNum = 0x0000;
    uint16_t    i = 0;

    switch( tableSel )
    {
        case JPG_HUUFFMAN_Y_AC:
            wTotalHTNum = totalLength;
            if(b_JPG_REGS_USE_CMDQ)
            {
                w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_TABLE_SPECIFY , 0x3<<30, 0x3<<30, jpg_regs.reg1);
                jpg_regs.reg1 = w_data;
            }
            else			
			    jpgWriteRegMask(REG_JPG_TABLE_SPECIFY , 0x3<<30, 0x3<<30);

            for(i = 0; i < wTotalHTNum; i++)
            {
                // Encode and Decode are different
                data =  ((i << 8) | (*(pCodeValue + i)));
                if(b_JPG_REGS_USE_CMDQ)
                    jpgWriteRegUsingCmdQ(REG_JPG_HUFFMAN_AC_LUMINANCE_CTRL, (JPG_REG)data);
                else
				    jpgWriteRegMask(REG_JPG_HUFFMAN_AC_LUMINANCE_CTRL, (JPG_REG)data , 0xFFFF);
            }

            if(b_JPG_REGS_USE_CMDQ)
            {
                w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_TABLE_SPECIFY , 0x0<<30, 0x3<<30, jpg_regs.reg1);
                jpg_regs.reg1 = w_data;
            }
            else
			    jpgWriteRegMask(REG_JPG_TABLE_SPECIFY , 0x0<<30, 0x3<<30);

            break;

        case JPG_HUUFFMAN_UV_AC:
            wTotalHTNum = totalLength;
            if(b_JPG_REGS_USE_CMDQ)
            {
                w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_TABLE_SPECIFY , 0x1<<30, 0x1<<30, jpg_regs.reg1);
                jpg_regs.reg1 = w_data;
            }
            else
			    jpgWriteRegMask(REG_JPG_TABLE_SPECIFY , 0x1<<30, 0x1<<30);
            
            for(i = 0; i < wTotalHTNum; i++)
            {
                // Encode and Decode are different
                data =  ((i << 8) | (*(pCodeValue + i)));
                if(b_JPG_REGS_USE_CMDQ)
                    jpgWriteRegUsingCmdQ(REG_JPG_HUFFMAN_AC_CHROMINANCE_CTRL, (JPG_REG)data << JPG_SHT_TO_HIGH_WORD);
                else		
				    jpgWriteRegMask(REG_JPG_HUFFMAN_AC_CHROMINANCE_CTRL, (JPG_REG)data << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000);
            }

            if(b_JPG_REGS_USE_CMDQ)
            {
                w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_TABLE_SPECIFY , 0x0<<30, 0x1<<30, jpg_regs.reg1);
                jpg_regs.reg1 = w_data;
            }
            else
			    jpgWriteRegMask(REG_JPG_TABLE_SPECIFY , 0x0<<30, 0x1<<30);
            
            break;

        default:
            break;
    }
}

void
JPG_SetYuv2RgbMatrix(
    JPG_YUV_TO_RGB      *matrix)
{
    // MM970 support it.
    if(b_JPG_REGS_USE_CMDQ)
    {
    	uint32_t    w_data;
        w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_RGB_MODE_CTRL, matrix->VIPMode, 0xFFFF, jpg_regs.reg24);
        jpg_regs.reg24 = w_data;
    	w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_YUV_TO_RGB_11, matrix->_11, 0xFFFF, jpg_regs.reg26);
    	jpg_regs.reg26 = w_data;
    	w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_YUV_TO_RGB_13, matrix->_13 << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000, jpg_regs.reg26);
    	jpg_regs.reg26 = w_data;
    	w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_YUV_TO_RGB_21, matrix->_21 , 0xFFFF, jpg_regs.reg27);
    	jpg_regs.reg27 = w_data;
    	w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_YUV_TO_RGB_22, matrix->_22 << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000, jpg_regs.reg27);
    	jpg_regs.reg27 = w_data;
    	w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_YUV_TO_RGB_23, matrix->_23 , 0xFFFF, jpg_regs.reg28);
    	jpg_regs.reg28 = w_data;
    	w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_YUV_TO_RGB_31, matrix->_31 << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000, jpg_regs.reg28);
    	jpg_regs.reg28 = w_data;
    	w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_YUV_TO_RGB_32, matrix->_32 , 0xFFFF, jpg_regs.reg29);
    	jpg_regs.reg29 = w_data;
    	w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_YUV_TO_RGB_CONST_R, matrix->ConstR << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000, jpg_regs.reg29);
    	jpg_regs.reg29 = w_data;
    	w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_YUV_TO_RGB_CONST_G, matrix->ConstG , 0xFFFF, jpg_regs.reg30);
    	jpg_regs.reg30 = w_data;
    	w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_YUV_TO_RGB_CONST_B, matrix->ConstB << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000, jpg_regs.reg30);
    	jpg_regs.reg30 = w_data;
    }
    else
    {    
        //jpgWriteRegMask(REG_JPG_RGB_MODE_CTRL, matrix->VIPMode, 0xFFFF);
    	//jpgWriteRegMask(REG_JPG_YUV_TO_RGB_11, matrix->_11, 0xFFFF);
    	//jpgWriteRegMask(REG_JPG_YUV_TO_RGB_13, matrix->_13 << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000);
    	//jpgWriteRegMask(REG_JPG_YUV_TO_RGB_21, matrix->_21 , 0xFFFF);
    	//jpgWriteRegMask(REG_JPG_YUV_TO_RGB_22, matrix->_22 << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000);
    	//jpgWriteRegMask(REG_JPG_YUV_TO_RGB_23, matrix->_23 , 0xFFFF);
    	//jpgWriteRegMask(REG_JPG_YUV_TO_RGB_31, matrix->_31 << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000);
    	//jpgWriteRegMask(REG_JPG_YUV_TO_RGB_32, matrix->_32 , 0xFFFF);
    	//jpgWriteRegMask(REG_JPG_YUV_TO_RGB_CONST_R, matrix->ConstR << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000);
    	//jpgWriteRegMask(REG_JPG_YUV_TO_RGB_CONST_G, matrix->ConstG , 0xFFFF);
    	//jpgWriteRegMask(REG_JPG_YUV_TO_RGB_CONST_B, matrix->ConstB << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000);
    	jpg_regs.reg24 = matrix->VIPMode;
    	jpgWriteReg(REG_JPG_RGB_MODE_CTRL, matrix->VIPMode);
    	jpgWriteReg(REG_JPG_YUV_TO_RGB_11, matrix->_11 |  (matrix->_13 << JPG_SHT_TO_HIGH_WORD));
    	jpgWriteReg(REG_JPG_YUV_TO_RGB_21, matrix->_21 |  (matrix->_22 << JPG_SHT_TO_HIGH_WORD));
    	jpgWriteReg(REG_JPG_YUV_TO_RGB_23, matrix->_23 |  (matrix->_31 << JPG_SHT_TO_HIGH_WORD));
    	jpgWriteReg(REG_JPG_YUV_TO_RGB_32, matrix->_32 |  (matrix->ConstR << JPG_SHT_TO_HIGH_WORD));
    	jpgWriteReg(REG_JPG_YUV_TO_RGB_CONST_G, matrix->ConstG |  (matrix->ConstB << JPG_SHT_TO_HIGH_WORD));
    }	
}

void
JPG_SetRgb565DitherKey(
    JPG_DITHER_KEY     *ditherKeyInfo)
{
	uint32_t    w_data;
    // MM970 support it.
    if( !ditherKeyInfo->bDisableDitherKey ) 
    {
        if(b_JPG_REGS_USE_CMDQ)
        {
    		w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_DISABLE_DITHER_KEY, ditherKeyInfo->bDisableDitherKey, 0xFFFF, jpg_regs.reg31);
    		jpg_regs.reg31 = w_data;
    		w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_SET_DITHER_KEY_MODE, ditherKeyInfo->ditherKeyMode << JPG_SHT_TO_HIGH_WORD, 0xFFFF0000, jpg_regs.reg31);
    		jpg_regs.reg31 = w_data;
        }
        else
        {
		    //jpgWriteRegMask(REG_JPG_DISABLE_DITHER_KEY, ditherKeyInfo->bDisableDitherKey << JPG_SHT_TO_HIGH_WORD , 0xFFFF0000);
		    //jpgWriteRegMask(REG_JPG_SET_DITHER_KEY_MODE, ditherKeyInfo->ditherKeyMode, 0xFFFF);
		    jpgWriteReg(REG_JPG_DISABLE_DITHER_KEY, (ditherKeyInfo->ditherKeyMode << JPG_SHT_TO_HIGH_WORD) | ditherKeyInfo->bDisableDitherKey);
        }	
    }
    else
    {
        if(b_JPG_REGS_USE_CMDQ)
        {
		    w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_DISABLE_DITHER_KEY, ditherKeyInfo->bDisableDitherKey, 0xFFFF, jpg_regs.reg31);
		    jpg_regs.reg31 = w_data;
        }
        else
        {
		    //jpgWriteRegMask(REG_JPG_DISABLE_DITHER_KEY, ditherKeyInfo->bDisableDitherKey << JPG_SHT_TO_HIGH_WORD  , 0xFFFF0000);
		    jpgWriteReg(REG_JPG_DISABLE_DITHER_KEY, ditherKeyInfo->bDisableDitherKey);
        }	
    }
}

void
JPG_SetAlphaPlaneReg(
    JPG_ALPHA_PLANE     *alphaPlaneInfo)
{
	uint32_t    w_data;
    uint32_t Alpha_vram_addr;

    if (alphaPlaneInfo->bEnConstAlpha)
    {
        if(b_JPG_REGS_USE_CMDQ)
        {
		    w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_EN_CONST_ALPHA, (JPG_REG)(alphaPlaneInfo->bEnConstAlpha), JPG_MSK_EN_CONST_APLHA, jpg_regs.reg32);
		    jpg_regs.reg32 = w_data;
		    w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_SET_CONST_ALPHA, ((JPG_REG)(alphaPlaneInfo->ConstAlpha & JPG_MSK_CONST_ALPHA) << JPG_SHT_TO_HIGH_WORD), JPG_MSK_CONST_ALPHA << JPG_SHT_TO_HIGH_WORD, jpg_regs.reg32);
		    jpg_regs.reg32 = w_data;
        }
        else
        {
            //jpgWriteRegMask(REG_JPG_EN_CONST_ALPHA, (JPG_REG)(alphaPlaneInfo->bEnConstAlpha), JPG_MSK_EN_CONST_APLHA);
            //jpgWriteRegMask(REG_JPG_SET_CONST_ALPHA, ((JPG_REG)(alphaPlaneInfo->ConstAlpha & JPG_MSK_CONST_ALPHA) << JPG_SHT_TO_HIGH_WORD), JPG_MSK_CONST_ALPHA << JPG_SHT_TO_HIGH_WORD);
            
            jpgWriteReg(REG_JPG_EN_CONST_ALPHA, (JPG_REG)(alphaPlaneInfo->bEnConstAlpha) | ((JPG_REG)(alphaPlaneInfo->ConstAlpha & JPG_MSK_CONST_ALPHA) << JPG_SHT_TO_HIGH_WORD));
        }
	
    }
    else
    {

        Alpha_vram_addr = ithSysAddr2VramAddr(alphaPlaneInfo->AlphaPlaneAddr);
        Alpha_vram_addr >>= 2;
        if(b_JPG_REGS_USE_CMDQ)
        {
		    w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_ALPHA_PLANE_ADDR_L, (JPG_REG)(Alpha_vram_addr & JPG_MSK_ALPHA_PLANE_ADDR_L) << JPG_SHT_TO_HIGH_WORD , JPG_MSK_ALPHA_PLANE_ADDR_L << JPG_SHT_TO_HIGH_WORD, jpg_regs.reg24);
		    jpg_regs.reg24 = w_data;
		    w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_APLHA_PLANE_ADDR_H, (JPG_REG)((Alpha_vram_addr >> 16) & JPG_MSK_ALPHA_PLANE_ADDR_H), JPG_MSK_BITSTREAM_BUF_ADDR_H, jpg_regs.reg25);
		    jpg_regs.reg25 = w_data;
		    w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_ALPHA_PLANE_PITCH,
		               (JPG_REG)((alphaPlaneInfo->AlphaPlanePitch >> 3) & JPG_MSK_APLHA_PLANE_PITCH) << JPG_SHT_TO_HIGH_WORD ,JPG_MSK_APLHA_PLANE_PITCH << JPG_SHT_TO_HIGH_WORD,
		               jpg_regs.reg25);
		    jpg_regs.reg25 = w_data;
        }
        else
        {
            //jpgWriteRegMask(REG_JPG_ALPHA_PLANE_ADDR_L, (JPG_REG)(Alpha_vram_addr & JPG_MSK_ALPHA_PLANE_ADDR_L) << JPG_SHT_TO_HIGH_WORD, JPG_MSK_ALPHA_PLANE_ADDR_L << JPG_SHT_TO_HIGH_WORD);
            //jpgWriteRegMask(REG_JPG_APLHA_PLANE_ADDR_H, (JPG_REG)((Alpha_vram_addr >> 16) & JPG_MSK_ALPHA_PLANE_ADDR_H), JPG_MSK_BITSTREAM_BUF_ADDR_H);
            //jpgWriteRegMask(REG_JPG_ALPHA_PLANE_PITCH,
            //    (JPG_REG)((alphaPlaneInfo->AlphaPlanePitch >> 3) & JPG_MSK_APLHA_PLANE_PITCH) << JPG_SHT_TO_HIGH_WORD, JPG_MSK_APLHA_PLANE_PITCH << JPG_SHT_TO_HIGH_WORD);
            
            jpgWriteReg(REG_JPG_ALPHA_PLANE_ADDR_L, ((JPG_REG)(Alpha_vram_addr & JPG_MSK_ALPHA_PLANE_ADDR_L) << JPG_SHT_TO_HIGH_WORD) | jpg_regs.reg24);
            jpgWriteReg(REG_JPG_APLHA_PLANE_ADDR_H, (JPG_REG)((Alpha_vram_addr >> 16) & JPG_MSK_ALPHA_PLANE_ADDR_H) |
                                                    ((JPG_REG)((alphaPlaneInfo->AlphaPlanePitch >> 3) & JPG_MSK_APLHA_PLANE_PITCH) << JPG_SHT_TO_HIGH_WORD));                                                                            
        }    
    }
}

void
JPG_EnableInterrupt(void)
{
	//jpgWriteRegMask(REG_JPG_SAMPLING_FACTOR_CD, (JPG_MSK_INTERRUPT_ENCODE_FULL | JPG_MSK_INTERRUPT_ENCODE_END),
	//                       (JPG_MSK_INTERRUPT_ENCODE_FULL | JPG_MSK_INTERRUPT_ENCODE_END));
	//jpgWriteRegMask(REG_JPG_SAMPLING_FACTOR_CD, JPG_MSK_INTERRUPT_DECODE_END,
	//                       JPG_MSK_INTERRUPT_DECODE_END);
    if(b_JPG_REGS_USE_CMDQ)
    {
	    uint32_t w_data;
	    w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_SAMPLING_FACTOR_CD, (JPG_MSK_INTERRUPT_ENCODE_END | JPG_MSK_INTERRUPT_DECODE_END | JPG_MSK_INTERRUPT_BITSTREAM_BUF_FULL),
	                       (JPG_MSK_INTERRUPT_ENCODE_END | JPG_MSK_INTERRUPT_DECODE_END | JPG_MSK_INTERRUPT_BITSTREAM_BUF_FULL),
	                       jpg_regs.reg11);
	    jpg_regs.reg11 = w_data;                     
    }
    else
        jpgWriteRegMask(REG_JPG_SAMPLING_FACTOR_CD, (JPG_MSK_INTERRUPT_ENCODE_END | JPG_MSK_INTERRUPT_DECODE_END | JPG_MSK_INTERRUPT_BITSTREAM_BUF_FULL),
	                       (JPG_MSK_INTERRUPT_ENCODE_END | JPG_MSK_INTERRUPT_DECODE_END | JPG_MSK_INTERRUPT_BITSTREAM_BUF_FULL));
}

void
JPG_DisableInterrupt(void)
{
	//jpgWriteRegMask(REG_JPG_SAMPLING_FACTOR_CD, 0,
	//                       (JPG_MSK_INTERRUPT_ENCODE_FULL | JPG_MSK_INTERRUPT_ENCODE_END));
    if(b_JPG_REGS_USE_CMDQ)
    {
	    uint32_t w_data;
	    w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_SAMPLING_FACTOR_CD, 0,
	                       (JPG_MSK_INTERRUPT_ENCODE_END | JPG_MSK_INTERRUPT_DECODE_END | JPG_MSK_INTERRUPT_BITSTREAM_BUF_FULL),
	                       jpg_regs.reg11);
	    jpg_regs.reg11 = w_data;
    }
    else
	    jpgWriteRegMask(REG_JPG_SAMPLING_FACTOR_CD, 0,
	                       (JPG_MSK_INTERRUPT_ENCODE_END | JPG_MSK_INTERRUPT_DECODE_END | JPG_MSK_INTERRUPT_BITSTREAM_BUF_FULL));
}

void
JPG_ClearInterrupt(void)
{
    if(b_JPG_REGS_USE_CMDQ)
	    jpgWriteRegUsingCmdQ(REG_JPG_CODEC_FIRE, JPG_MSK_INTERRUPT_CLEAR);				 
    else
	//printf("YC: data = 0x%x\n", (ithReadRegA(REG_JPG_CODEC_FIRE) & ~JPG_MSK_INTERRUPT_CLEAR) | (JPG_MSK_INTERRUPT_CLEAR & JPG_MSK_INTERRUPT_CLEAR));
	//jpgWriteRegMask(REG_JPG_CODEC_FIRE, JPG_MSK_INTERRUPT_CLEAR , JPG_MSK_INTERRUPT_CLEAR);
	    jpgWriteReg(REG_JPG_CODEC_FIRE, JPG_MSK_INTERRUPT_CLEAR);
}

void JPG_SetNV12Enable(void)
{
    if(b_JPG_REGS_USE_CMDQ)
    {
    	uint32_t w_data;
    	w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_DISPLAY_MCU_HEIGHT_Y, JPG_MSK_NV12_ENABLE, JPG_MSK_NV12_ENABLE, jpg_regs.reg2);
    	jpg_regs.reg2 = w_data;
    }
    else
	    jpgWriteRegMask(REG_JPG_DISPLAY_MCU_HEIGHT_Y, JPG_MSK_NV12_ENABLE, JPG_MSK_NV12_ENABLE);
}

void JPG_SetNV21Enable(void)
{
    if(b_JPG_REGS_USE_CMDQ)
    {
	    uint32_t w_data;
	    w_data = jpgWriteRegMaskUsingCmdQ(REG_JPG_DISPLAY_MCU_HEIGHT_Y, JPG_MSK_NV21_ENABLE, JPG_MSK_NV21_ENABLE, jpg_regs.reg2);
	    jpg_regs.reg2 = w_data;
    }
    else
	    jpgWriteRegMask(REG_JPG_DISPLAY_MCU_HEIGHT_Y, JPG_MSK_NV21_ENABLE, JPG_MSK_NV21_ENABLE);
}

