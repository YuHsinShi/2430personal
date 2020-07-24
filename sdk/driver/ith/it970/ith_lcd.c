/*
 * Copyright (c) 2016 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL LCD functions.
 *
 * @author Irene Wang
 * @version 1.0
 */
#include <unistd.h>
#include "../ith_cfg.h"
#include "ith_lcd.h"

#define CMD_DELAY   0xFFFFFFFF

static const uint32_t* lcdScript;
static unsigned int lcdScriptCount, lcdScriptIndex;
static uint32_t mipiHSTable[16];
static uint32_t mipiHSPLLFrange[4];
static uint32_t pinShareTable[8];

void ithLcdReset(void)
{
    ithWriteRegMaskA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, 0xFFFFFFFF, (0x1 << ITH_LCD_REG_RST_BIT) | (0x1 << ITH_LCD_RST_BIT));
    ithWriteRegMaskA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, 0x0, (0x1 << ITH_LCD_REG_RST_BIT) | (0x1 << ITH_LCD_RST_BIT));
}

void ithLcdEnable(void)
{
    // enable clock
    //if (ithLcdGetPanelType() != ITH_LCD_RGB)
    //    ithSetRegBitA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, ITH_EN_DCLK_BIT);
    ithSetRegBitA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, ITH_EN_M3CLK_BIT);
    ithSetRegBitA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, ITH_EN_W12CLK_BIT);

    if (ithLcdGetPanelType() == ITH_LCD_MIPI)
    {
        ithWriteRegA(0xD800004C, 0x0002c001);  //MIPI controller enable
        usleep(1);
        ithWriteRegA(0xD8000048, 0x8002c003);  //DPHY Enable
        usleep(100);                           //100us
        ithWriteRegA(0xD8000044, 0x400ac081);  //DPHY PORn rst normal
        usleep(75);                            //75us
        ithWriteRegA(0xD8000044, 0x000ac081);  //DPHY ResetN rst normal
        usleep(200);                           //200us

        ithLcdResetMipiHSInfo();

        //LCD
        ithWriteRegA(0xD0000070, 0x00000307);    // ctg_reset_off
        usleep(1);
    }

    // disable LCD pin input mode
    if (ithLcdGetPanelType() == ITH_LCD_RGB)
    {
        ithLcdResetPinShareInfo();
    }

    //set reg 0x0020 as 0x81000000 for test mode
    // wait for 0x81000000 become 0x01000000
    ithWriteRegA(ITH_LCD_BASE + ITH_LCD_TEST_COLOR_SET_REG, 0x800000FF);
    while (ithReadRegA(ITH_LCD_BASE + ITH_LCD_TEST_COLOR_SET_REG) & 0x80000000)
    {
        ithDelay(1000);
    }
}

void ithLcdDisable(void)
{
    //set reg 0x0020 as 0x81000000 for test mode
    // wait for 0x81000000 become 0x01000000
    ithWriteRegA(ITH_LCD_BASE + ITH_LCD_TEST_COLOR_SET_REG, 0x810000FF);
    while (ithReadRegA(ITH_LCD_BASE + ITH_LCD_TEST_COLOR_SET_REG) & 0x80000000)
    {
        ithDelay(1000);
    }

    if (ithLcdGetPanelType() == ITH_LCD_MIPI)
    {
        ithLcdSaveMipiHSInfo();

        //disable MIPI CLK
        ithWriteRegA(0xD8000048, 0x0002c003);    //[31]=0 DPHY disable, KESCCLK 
        ithWriteRegA(0xD8000044, 0x800ac081);    //[31]=1 DPHY PORn, KCLK
        usleep(1);                          // > 500ns
        ithWriteRegA(0xD8000044, 0xC00ac081);    //[30]=1 DPHY ResetN, KCLK
        usleep(1);                          // > 500ns
        ithWriteRegA(0xD800004C, 0xC002c001);    //[31]:Reset MIPI power controller,[30]:Reset MIPI system controller,  KDSICLK

        //LCD
        ithWriteRegA(0xD0000070, 0x00010307);    // ctg_reset_on
        usleep(1);
    }
    

    // set LCD pin input mode
    if (ithLcdGetPanelType() == ITH_LCD_RGB)
    {
        ithLcdSavePinShareInfo();
        ithWriteRegA(0xD10000E4, 0);
        ithWriteRegA(0xD10000E8, 0);
        ithWriteRegA(0xD10000EC, 0);
        ithWriteRegA(0xD1000160, 0);
        ithWriteRegA(0xD1000164, 0);
        ithWriteRegA(0xD1000168, 0);
        ithWriteRegA(0xD100016C, 0);
    }
        
    // disable clock
    ithClearRegBitA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, ITH_EN_M3CLK_BIT);
    ithClearRegBitA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, ITH_EN_W12CLK_BIT);
    //if (ithLcdGetPanelType() != ITH_LCD_RGB)
    //    ithClearRegBitA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, ITH_EN_DCLK_BIT);
}

void ithLcdSetBaseAddrA(uint32_t addr)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_BASEA_REG, addr, ITH_LCD_BASEA_MASK);
}

void ithLcdSetBaseAddrB(uint32_t addr)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_BASEB_REG, addr, ITH_LCD_BASEB_MASK);
}

uint32_t ithLcdGetBaseAddrB(void)
{
    return (ithReadRegA(ITH_LCD_BASE + ITH_LCD_BASEB_REG) & ITH_LCD_BASEB_MASK);
}

void ithLcdSetBaseAddrC(uint32_t addr)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_BASEC_REG, addr, ITH_LCD_BASEC_MASK);
}

uint32_t ithLcdGetBaseAddrC(void)
{
    return (ithReadRegA(ITH_LCD_BASE + ITH_LCD_BASEC_REG) & ITH_LCD_BASEC_MASK);
}

void ithLcdEnableHwFlip(void)
{
    ithSetRegBitA(ITH_LCD_BASE + ITH_LCD_SET1_REG, ITH_LCD_HW_FLIP_BIT);
}

void ithLcdDisableHwFlip(void)
{
    ithClearRegBitA(ITH_LCD_BASE + ITH_LCD_SET1_REG, ITH_LCD_HW_FLIP_BIT);
}

void ithLcdEnableVideoFlip(void)
{
    ithLcdDisableHwFlip();
    ithSetRegBitA(ITH_LCD_BASE + ITH_LCD_SET1_REG, ITH_LCD_VIDEO_FLIP_EN_BIT);
}

void ithLcdDisableVideoFlip(void)
{
    ithLcdDisableHwFlip();
    ithClearRegBitA(ITH_LCD_BASE + ITH_LCD_SET1_REG, ITH_LCD_VIDEO_FLIP_EN_BIT);
}

void ithLcdLoadScriptFirst(const uint32_t* script, unsigned int count)
{
    unsigned int i;

    lcdScript = script;
    lcdScriptCount = count;

    // Run script until fire    
    for (i = 0; i < count; i += 2)
    {
        unsigned int reg    = script[i];
        unsigned int val    = script[i + 1];
        
        if (reg == CMD_DELAY)
            ithDelay(val);
        else
		{
            ithWriteRegA(reg, val);
			if (reg == (ITH_LCD_BASE + ITH_LCD_UPDATE_REG) && (val & ITH_LCD_SYNCFIRE_MASK))
				break;
		}
    }
    lcdScriptIndex = i;
}

void ithLcdLoadScriptNext(void)
{
    unsigned int i;
    
    for (i = lcdScriptIndex; i < lcdScriptCount; i += 2)
    {
        unsigned int reg    = lcdScript[i];
        unsigned int val    = lcdScript[i + 1];
        
        if (reg != CMD_DELAY)
            ithWriteRegA(reg, val);
    }
}

void ithLcdCursorSetBaseAddr(uint32_t addr)
{
    ithWriteRegA(ITH_LCD_BASE + ITH_LCD_HWC_BASE_REG, addr);
}

void ithLcdCursorSetColorWeight(ITHLcdCursorColor color, uint8_t value)
{
    switch (color)
    {
    case ITH_LCD_CURSOR_DEF_COLOR:
        ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HWC_INVCOLORWEI_REG, value << ITH_LCD_HWC_INVCOLORWEI_BIT, ITH_LCD_HWC_INVCOLORWEI_MASK);
        break;

    case ITH_LCD_CURSOR_FG_COLOR:
        ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HWC_FORECOLORWEI_REG, value << ITH_LCD_HWC_FORECOLORWEI_BIT, ITH_LCD_HWC_FORECOLORWEI_MASK);
        break;

    case ITH_LCD_CURSOR_BG_COLOR:
        ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HWC_BACKCOLORWEI_REG, value << ITH_LCD_HWC_BACKCOLORWEI_BIT, ITH_LCD_HWC_BACKCOLORWEI_MASK);
        break;
    }
}

ITHLcdFormat ithLcdGetFormat(void)
{
    return (ITHLcdFormat)((ithReadRegA(ITH_LCD_BASE + ITH_LCD_SRCFMT_REG) & ITH_LCD_SRCFMT_MASK) >> ITH_LCD_SRCFMT_BIT);
}

void ithLcdSetWidth(uint32_t width)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_WIDTH_REG, width << ITH_LCD_WIDTH_BIT, ITH_LCD_WIDTH_MASK);
}


void ithLcdSetHeight(uint32_t height)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HEIGHT_REG, height << ITH_LCD_HEIGHT_BIT, ITH_LCD_HEIGHT_MASK);
}

void ithLcdSetPitch(uint32_t pitch)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_PITCH_REG, pitch << ITH_LCD_PITCH_BIT, ITH_LCD_PITCH_MASK);
}

unsigned int ithLcdGetXCounter(void)
{
    return (ithReadRegA(ITH_LCD_BASE + ITH_LCD_CTGH_CNT_REG) & ITH_LCD_CTGH_CNT_MASK) >> ITH_LCD_CTGH_CNT_BIT;
}

unsigned int ithLcdGetYCounter(void)
{
    return (ithReadRegA(ITH_LCD_BASE + ITH_LCD_READ_STATUS1_REG) & ITH_LCD_CTGV_CNT_MASK) >> ITH_LCD_CTGV_CNT_BIT;
}

void ithLcdSyncFire(void)
{
    ithSetRegBitA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG, ITH_LCD_SYNCFIRE_BIT);
}

 bool ithLcdIsSyncFired(void)
{
     return (ithReadRegA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG) & ITH_LCD_SYNCFIRE_MASK) ? true : false;
}

bool ithLcdIsEnabled(void)
{
    return ((ithReadRegA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG) & (ITH_LCD_DISPEN_MASK | ITH_LCD_SYNCFIRE_MASK)) == (ITH_LCD_DISPEN_MASK | ITH_LCD_SYNCFIRE_MASK)) ? true : false;
}

unsigned int ithLcdGetFlip(void)
{
    return (ithReadRegA(ITH_LCD_BASE + ITH_LCD_READ_STATUS1_REG) & ITH_LCD_FLIP_NUM_MASK) >> ITH_LCD_FLIP_NUM_BIT;
}

unsigned int ithLcdGetMaxLcdBufCount(void)
{
#if CFG_VIDEO_ENABLE
    return 3;
#else
    return 2;
#endif
}

void ithLcdSwFlip(unsigned int index)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_SWFLIPNUM_REG, index << ITH_LCD_SWFLIPNUM_BIT, ITH_LCD_SWFLIPNUM_MASK);
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG, 1 << ITH_LCD_LAYER1UPDATE_BIT, ITH_LCD_LAYER1UPDATE_MASK);
}

void ithLcdCursorEnable(void)
{
    ithSetRegBitA(ITH_LCD_BASE + ITH_LCD_HWC_EN_REG, ITH_LCD_HWC_EN_BIT);
}

void ithLcdCursorDisable(void)
{
    ithClearRegBitA(ITH_LCD_BASE + ITH_LCD_HWC_EN_REG, ITH_LCD_HWC_EN_BIT);
}

void ithLcdCursorCtrlEnable(ITHLcdCursorCtrl ctrl)
{
    switch(ctrl)
    {
    case ITH_LCD_CURSOR_ALPHABLEND_ENABLE:
        ithSetRegBitA(ITH_LCD_BASE + ITH_LCD_HWC_CR_REG, ITH_LCD_HWC_ABLDEN_BIT);
        break;   
    case ITH_LCD_CURSOR_DEFDEST_ENABLE:  
        ithSetRegBitA(ITH_LCD_BASE + ITH_LCD_HWC_CR_REG, ITH_LCD_HWC_DEFDSTEN_BIT);
        break;
    case ITH_LCD_CURSOR_INVDEST_ENABLE:
        ithSetRegBitA(ITH_LCD_BASE + ITH_LCD_HWC_CR_REG, ITH_LCD_HWC_DEFINVDST_BIT);
        break;  
    }
}

void ithLcdCursorCtrlDisable(ITHLcdCursorCtrl ctrl)
{
    switch(ctrl)
    {
    case ITH_LCD_CURSOR_ALPHABLEND_ENABLE:
        ithClearRegBitA(ITH_LCD_BASE + ITH_LCD_HWC_CR_REG, ITH_LCD_HWC_ABLDEN_BIT);
        break;   
    case ITH_LCD_CURSOR_DEFDEST_ENABLE:  
        ithClearRegBitA(ITH_LCD_BASE + ITH_LCD_HWC_CR_REG, ITH_LCD_HWC_DEFDSTEN_BIT);
        break;
    case ITH_LCD_CURSOR_INVDEST_ENABLE:
        ithClearRegBitA(ITH_LCD_BASE + ITH_LCD_HWC_CR_REG, ITH_LCD_HWC_DEFINVDST_BIT);
        break;  
    }
}

void ithLcdCursorSetWidth(unsigned int width)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HWC_WIDTH_REG, width << ITH_LCD_HWC_WIDTH_BIT, ITH_LCD_HWC_WIDTH_MASK);
}

void ithLcdCursorSetHeight(unsigned int height)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HWC_HEIGHT_REG, height << ITH_LCD_HWC_HEIGHT_BIT, ITH_LCD_HWC_HEIGHT_MASK);
}

void ithLcdCursorSetPitch(unsigned int pitch)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HWC_PITCH_REG, pitch << ITH_LCD_HWC_PITCH_BIT, ITH_LCD_HWC_PITCH_MASK);
}

void ithLcdCursorSetX(unsigned int x)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HWC_POSX_REG, x << ITH_LCD_HWC_POSX_BIT, ITH_LCD_HWC_POSX_MASK);
}

void ithLcdCursorSetY(unsigned int y)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HWC_POSY_REG, y << ITH_LCD_HWC_POSY_BIT, ITH_LCD_HWC_POSY_MASK);
}

void ithLcdCursorSetColor(ITHLcdCursorColor color, uint16_t value)
{
    switch(color)
    {
        case ITH_LCD_CURSOR_DEF_COLOR:   
            ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HWC_DEFCOLOR_REG, value << ITH_LCD_HWC_DEFCOLOR_BIT, ITH_LCD_HWC_DEFCOLOR_MASK);
            break;
        case ITH_LCD_CURSOR_FG_COLOR:  
            ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HWC_FORECOLOR_REG, value << ITH_LCD_HWC_FORECOLOR_BIT, ITH_LCD_HWC_FORECOLOR_MASK);
            break;
        case ITH_LCD_CURSOR_BG_COLOR:
            ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HWC_BACKCOLOR_REG, value << ITH_LCD_HWC_BACKCOLOR_BIT, ITH_LCD_HWC_BACKCOLOR_MASK);
            break;      
    }
}

void ithLcdCursorUpdate(void)
{
    ithSetRegBitA(ITH_LCD_BASE + ITH_LCD_HWC_UPDATE_REG, ITH_LCD_HWC_UPDATE_BIT);
}

 bool ithLcdCursorIsUpdateDone(void)
{
     return ithReadRegA(ITH_LCD_BASE + ITH_LCD_HWC_UPDATE_REG) & (0x1 << ITH_LCD_HWC_UPDATE_BIT);
}

void ithLcdIntrCtrlEnable(ITHLcdIntrCtrl ctrl)
{
    switch (ctrl)
    {
    case ITH_LCD_INTR_ENABLE:
        ithSetRegBitA(ITH_LCD_BASE + ITH_LCD_INT_CTRL_REG, ITH_LCD_INT_EN_BIT);
        break;
    case ITH_LCD_INTR_FIELDMODE1:
        ithSetRegBitA(ITH_LCD_BASE + ITH_LCD_INT_CTRL_REG, ITH_LCD_INT_FIELDMODE1_BIT);
        break;
    case ITH_LCD_INTR_OUTPUT2:
    case ITH_LCD_INTR_FIELDMODE2:
    case ITH_LCD_INTR_OUTPUT1:
        break;
    }
}

void ithLcdIntrCtrlDisable(ITHLcdIntrCtrl ctrl)
{
    switch (ctrl)
    {
    case ITH_LCD_INTR_ENABLE:
        ithClearRegBitA(ITH_LCD_BASE + ITH_LCD_INT_CTRL_REG, ITH_LCD_INT_EN_BIT);
        break;

    case ITH_LCD_INTR_FIELDMODE1:
        ithClearRegBitA(ITH_LCD_BASE + ITH_LCD_INT_CTRL_REG, ITH_LCD_INT_FIELDMODE1_BIT);
        break;
    case ITH_LCD_INTR_OUTPUT2:
    case ITH_LCD_INTR_FIELDMODE2:
    case ITH_LCD_INTR_OUTPUT1:
        break;

    }
}

void ithLcdIntrEnable(void)
{
    ithLcdIntrCtrlEnable(ITH_LCD_INTR_ENABLE);
}

void ithLcdIntrDisable(void)
{
    ithLcdIntrCtrlDisable(ITH_LCD_INTR_ENABLE);
}

void ithLcdIntrClear(void)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_INT_CLR_REG, 0x1 << ITH_LCD_INT_CLR_BIT, ITH_LCD_INT_CLR_MASK);
}

void ithLcdIntrSetScanLine1(unsigned int line)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_INT_LINE1_REG, line << ITH_LCD_INT_LINE1_BIT, ITH_LCD_INT_LINE1_MASK);
}

void ithLcdIntrSetScanLine2(unsigned int line)
{

}

void ithLcdSetRotMode(ITHLcdScanType type, ITHLcdRotMode mode)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_SET1_REG, type << ITH_LCD_SCAN_TYPE_BIT, 0x1 << ITH_LCD_SCAN_TYPE_BIT);
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_SET1_REG, mode << ITH_LCD_ROT_MODE_BIT, ITH_LCD_ROT_MODE_MASK);

    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG, 1 << ITH_LCD_LAYER1UPDATE_BIT, ITH_LCD_LAYER1UPDATE_MASK);

    while (ithReadRegA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG) & (0x1 << ITH_LCD_LAYER1UPDATE_BIT))
    {
        ithDelay(1000);
    }
}

ITHLcdPanelType ithLcdGetPanelType(void)
{
    ITHLcdPanelType panelType = ITH_LCD_RGB;

    if (ithReadRegA(ITH_LCD_BASE + ITH_LCD_MIPI_SET1_REG) & 0x1)
        panelType = ITH_LCD_MIPI;
    else if (ithReadRegA(ITH_LCD_BASE + ITH_LCD_LVDS_SET1_REG) & 0x1)
        panelType = ITH_LCD_LVDS;

    return panelType;

}

void ithLcdSaveMipiHSInfo(void)
{
    //save MIPI HS registers
    mipiHSTable[0] = ithReadRegA(0xd0c00004);
    mipiHSTable[1] = ithReadRegA(0xd0c00008);
    mipiHSTable[2] = ithReadRegA(0xd0c00010);
    mipiHSTable[3] = ithReadRegA(0xd0c00014);
    mipiHSTable[4] = ithReadRegA(0xd0c00018);
    mipiHSTable[5] = ithReadRegA(0xd0c0001c);
    mipiHSTable[6] = ithReadRegA(0xd0c00020);
    mipiHSTable[7] = ithReadRegA(0xd0c00028);
    mipiHSTable[8] = ithReadRegA(0xd0c00048);
    mipiHSTable[9] = ithReadRegA(0xd0c00054);
    mipiHSTable[10] = ithReadRegA(0xd0c00058);
    mipiHSTable[11] = ithReadRegA(0xd0c00080);
    mipiHSTable[12] = ithReadRegA(0xd0c00084);
    mipiHSTable[13] = ithReadRegA(0xd0c00088);
    mipiHSTable[14] = ithReadRegA(0xd0c0008c);
    mipiHSTable[15] = ithReadRegA(0xd0c00050);

    mipiHSPLLFrange[0] = ithReadRegA(0xDC10001C);
    mipiHSPLLFrange[1] = ithReadRegA(0xDC100020);
    mipiHSPLLFrange[2] = ithReadRegA(0xDC10002C); 
}

void ithLcdResetMipiHSInfo(void)
{
    ithWriteRegA(0xDC10001C, mipiHSPLLFrange[0]);
    ithWriteRegA(0xDC100020, mipiHSPLLFrange[1]);
    ithWriteRegA(0xDC10002C, mipiHSPLLFrange[2]);
    usleep(150);

    //resetting MIPI HS registers
    ithWriteRegA(0xd0c00004, mipiHSTable[0]); //+0x6[7]=BLLP, +0x04[0]=EOTPGE
    ithWriteRegA(0xd0c00008, mipiHSTable[1]);
    ithWriteRegA(0xd0c00010, mipiHSTable[2]);
    ithWriteRegA(0xd0c00014, mipiHSTable[3]);  //RGB666(0x1E),RGB888(0x3E)
    ithWriteRegA(0xd0c00018, mipiHSTable[4]);  //+0x18[5:0]=CLWR
    ithWriteRegA(0xd0c0001c, mipiHSTable[5]);  //24-bit pixel
    ithWriteRegA(0xd0c00020, mipiHSTable[6]);  //HACT=1024(0x0400)
    ithWriteRegA(0xd0c00028, mipiHSTable[7]);  //1024*3(0x0c00)
    //WRITE d0c0003c 000000ff f
    ithWriteRegA(0xd0c00048, mipiHSTable[8]);  //+0x48[6]=HSE Pkt
    //WRITE d0c00050 000006a0 f
    ithWriteRegA(0xd0c00054, mipiHSTable[9]);  // HSS(4)+HSA*3+HSE(4)+HBP*3+4+HACT*3+2+HFP*3
    ithWriteRegA(0xd0c00058, mipiHSTable[10]);
    ithWriteRegA(0xd0c00080, mipiHSTable[11]);  // VBP=23(0x17),VSA=2(0x02)
    ithWriteRegA(0xd0c00084, mipiHSTable[12]);  // VACT=600(0x0258),VFP=12(0x0c)
    ithWriteRegA(0xd0c00088, mipiHSTable[13]);  // HBP=140*3(0x01a4),HSA=20*3(0x3c)
    ithWriteRegA(0xd0c0008c, mipiHSTable[14]);  // HFP=160*3(0x01e0)

    ithWriteRegA(0xd0c00050, mipiHSTable[15]);  // pixel fifo threshold
    usleep(150);
}

void ithLcdSavePinShareInfo(void)
{
    //save LCD pin share registers
    pinShareTable[0] = ithReadRegA(0xD10000E4);
    pinShareTable[1] = ithReadRegA(0xD10000E8);
    pinShareTable[2] = ithReadRegA(0xD10000EC);
    pinShareTable[3] = ithReadRegA(0xD1000160);
    pinShareTable[4] = ithReadRegA(0xD1000164);
    pinShareTable[5] = ithReadRegA(0xD1000168);
    pinShareTable[6] = ithReadRegA(0xD100016C);
}

void ithLcdResetPinShareInfo(void)
{
    //resetting LCD pin share registers
    ithWriteRegA(0xD10000E4, pinShareTable[0]); 
    ithWriteRegA(0xD10000E8, pinShareTable[1]);
    ithWriteRegA(0xD10000EC, pinShareTable[2]);
    ithWriteRegA(0xD1000160, pinShareTable[3]);  
    ithWriteRegA(0xD1000164, pinShareTable[4]); 
    ithWriteRegA(0xD1000168, pinShareTable[5]);  
    ithWriteRegA(0xD100016C, pinShareTable[6]); 
}



