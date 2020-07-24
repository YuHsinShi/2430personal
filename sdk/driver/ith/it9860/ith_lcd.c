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
#include "ite/itp.h"
#include "iic/mmp_iic.h"
#include "ith_cmdq.h"

#define CMD_DELAY         0xFFFFFFFF
#define IIC_PORT_NUM      IIC_PORT_1  //IT6122 or IT6151 using IIC port

#if (CFG_GPIO_LCD_PWR_EN > 0)
static uint32_t lcdA;
static uint32_t lcdB;
static uint32_t lcdC;
#endif

static const uint32_t* lcdScript;
static unsigned int lcdScriptCount, lcdScriptIndex;
static uint32_t mipiHSTable[16];
static uint32_t pinShareTable[8];
static uint32_t mipiDPHYInfo[8];
static ITHLcdPanelType gPanelType = ITH_LCD_RGB;

#ifndef _WIN32
static int errorSum = 0;

static void i2c_write_byte(uint8_t dev_addr, uint8_t addr, uint8_t data)
{
    uint8_t     CmdBuf[2];

    CmdBuf[0] = addr;
    CmdBuf[1] = data;
    mmpIicSendDataEx(IIC_PORT_NUM, IIC_MASTER_MODE, dev_addr, CmdBuf, 2);
}

static void i2c_read_byte(uint8_t dev_addr, uint8_t addr, uint8_t *dataBuffer)
{
    //uint8_t len;

    *dataBuffer = addr;

    mmpIicReceiveData(IIC_PORT_NUM, IIC_MASTER_MODE, dev_addr, &addr, 1, dataBuffer, 1);
}

static void IT6122_DumpReg()
{
    uint32_t i = 0;
    unsigned char value;

    //printf("HMPRX_R Reg\r\n");
    for (i = 0; i < 0xFF; i++){

        if ((i % 0x10) == 0x00){
            //printf("\r\n%02x|\t", i);
        }
        i2c_read_byte(0x6C, i, &value);
        //printf("%02x ", value);
    }
    i2c_read_byte(0x6C, 0xFF, &value);
    //printf("%02x\r\n", value);
}

static bool IT6122_Sys_ini(void)
{
    unsigned char VenID[2], DevID[2], u8RevID;

    i2c_read_byte(MPRxDevAddr, 0x00, &VenID[0]);
    i2c_read_byte(MPRxDevAddr, 0x01, &VenID[1]);
    i2c_read_byte(MPRxDevAddr, 0x02, &DevID[0]);
    i2c_read_byte(MPRxDevAddr, 0x03, &DevID[1]);
    i2c_read_byte(MPRxDevAddr, 0x04, &u8RevID);

    //printf("Current DevID=%02X%02X\r\n ", DevID[1], DevID[0]);
    //printf("Current VenID=%02X%02X\r\n ", VenID[1], VenID[0]);
    //printf("Current g_u8RevID=%02X\r\n \r\n ", u8RevID);

    if (VenID[0] != 0x54 || VenID[1] != 0x49 || DevID[0] != 0x22 || DevID[1] != 0x61 || (u8RevID != 0xA0 && u8RevID != 0xB0 && u8RevID != 0xC0)) {
        //printf(("\r\n Error: Can not find IT6121A0 MP2LV Device !!!\r\n "));
        return false;
    }

    i2c_write_byte(MPRxDevAddr, 0x05, 0x17);
    i2c_write_byte(MPRxDevAddr, 0xC0, (EnSP_S1 << 4) + 0x01);//0x11
    i2c_write_byte(MPRxDevAddr, 0x05, 0x1F);
    i2c_write_byte(MPRxDevAddr, 0x05, 0x00);

    i2c_write_byte(MPRxDevAddr, 0x09, 0xFF);
    i2c_write_byte(MPRxDevAddr, 0x0A, 0xFF);
    i2c_write_byte(MPRxDevAddr, 0x0B, 0xEF);
    if (u8RevID == 0xC0){
        i2c_write_byte(MPRxDevAddr, 0xF8, 0x03); //vidrec, sscfifo	
    }

    i2c_write_byte(MPRxDevAddr, 0x0D, 0x00);
    i2c_write_byte(MPRxDevAddr, 0x0C, (EnLaneSwap << 3) + (EnPNSwap << 2) + MPLaneNum);
    i2c_write_byte(MPRxDevAddr, 0x11, 0x80 + (InvPCLK << 1) + InvMCLK);//0x81 //0x82
    i2c_write_byte(MPRxDevAddr, 0x12, 0x00);
    //iTE_I2C_WriteByte(MPRxDevAddr, 0x18, 0xD5);
    i2c_write_byte(MPRxDevAddr, 0x18, 0x80 + (SkipStg << 4) + HSSetNum);
    i2c_write_byte(MPRxDevAddr, 0x19, 0x03);
    i2c_write_byte(MPRxDevAddr, 0x20, 0x03);
    //iTE_I2C_WriteByte(MPRxDevAddr, 0x44, 0x00);
    i2c_write_byte(MPRxDevAddr, 0x44, (MREC_Update << 5) + (PREC_Update << 4));
    i2c_write_byte(MPRxDevAddr, 0x4B, (EnFReSync << 4) + 0x01);
    i2c_write_byte(MPRxDevAddr, 0x4C, 0x02);
    i2c_write_byte(MPRxDevAddr, 0x4E, (EnVReSync << 3) + (EnHReSync << 2));
    i2c_write_byte(MPRxDevAddr, 0x4F, 0x01);
    //iTE_I2C_WriteByte(MPRxDevAddr, 0x27, 0x3E);
    i2c_write_byte(MPRxDevAddr, 0x27, MPVidType);
    i2c_write_byte(MPRxDevAddr, 0x70, 0x01);
    i2c_write_byte(MPRxDevAddr, 0x72, 0x07);
    i2c_write_byte(MPRxDevAddr, 0x73, 0x03);
    i2c_write_byte(MPRxDevAddr, 0x80, 0x03);
    i2c_write_byte(MPRxDevAddr, 0x84, 0xFF);

    if (EnMBPM) {
        i2c_write_byte(MPRxDevAddr, 0xA1, 0x00);
        i2c_write_byte(MPRxDevAddr, 0xA2, 0x00);

        i2c_write_byte(MPRxDevAddr, 0xA3, 0x60);	// HSW
        i2c_write_byte(MPRxDevAddr, 0xA5, 0x02);	// VSW
    }
    i2c_write_byte(MPRxDevAddr, 0xA0, EnMBPM);
    if (u8RevID == 0xC0){
        i2c_write_byte(MPRxDevAddr, 0xDA, EnLVVidRecInt << 7);//0x80
    }
    else{
        i2c_write_byte(MPRxDevAddr, 0xDF, EnLVVidRecInt << 7);
    }

    i2c_write_byte(MPRxDevAddr, 0x92, 0x1E);

    i2c_write_byte(MPRxDevAddr, 0x80, MPPCLKSel);
    i2c_write_byte(MPRxDevAddr, 0xCB, 0x30 + (EnLVDMode << 3) + (En6bitout << 2) + MAPVESA);//0x3C 3-lanes daul channel //0x38 4-lanes daul channel //0x30 4-lanes single channel //[0] 0:JEIDA 1:VESA
    i2c_write_byte(MPRxDevAddr, 0xC9, 0x00);
    i2c_write_byte(MPRxDevAddr, 0x70, 0x01);
    i2c_write_byte(MPRxDevAddr, 0x05, 0x02);
    //LVTx setting
    if (u8RevID == 0xC0){
        i2c_write_byte(MPRxDevAddr, 0xF2, LVSDM & 0xFF);
        i2c_write_byte(MPRxDevAddr, 0xF3, (LVSDM & 0xFF00) >> 8);
        i2c_write_byte(MPRxDevAddr, 0xF4, LVSDMINC & 0xFF);
        i2c_write_byte(MPRxDevAddr, 0xF5, (EnSSCBufAutoRst << 6) + (EnLVTxSSC << 4) + ((LVSDMINC & 0x700) >> 8));
        i2c_write_byte(MPRxDevAddr, 0xD8, (EnSSCBufConcat << 6) + 0x10);//0x10
        i2c_write_byte(MPRxDevAddr, 0xC5, 0xC0 + (EnSP_S1 << 2) + (EnSSCPLL << 1) + (EnSSCPLL));//0xC4	
    }

    i2c_write_byte(MPRxDevAddr, 0x91, 0xFB);//RCLK using default setting
    i2c_write_byte(MPRxDevAddr, 0x31, 0x00);
    i2c_write_byte(MPRxDevAddr, 0x33, 0x3F);
    i2c_write_byte(MPRxDevAddr, 0x35, 0x3F);
    i2c_write_byte(MPRxDevAddr, 0x37, 0x00);
    i2c_write_byte(MPRxDevAddr, 0x39, 0x00);
    i2c_write_byte(MPRxDevAddr, 0x3A, 0x00);
    i2c_write_byte(MPRxDevAddr, 0x3C, 0x7F);
    i2c_write_byte(MPRxDevAddr, 0x3E, 0x7F);
    i2c_write_byte(MPRxDevAddr, 0x41, 0x00);
    i2c_write_byte(MPRxDevAddr, 0x43, 0x00);
#if (MIPI_EVENT_MODE == 1)
    i2c_write_byte(MPRxDevAddr, 0x33, 0x80 | MIPI_HSYNC_W >> 8);
    i2c_write_byte(MPRxDevAddr, 0x32, MIPI_HSYNC_W);
    i2c_write_byte(MPRxDevAddr, 0x3D, 0x80 | MIPI_VSYNC_W >> 8);
    i2c_write_byte(MPRxDevAddr, 0x3C, MIPI_VSYNC_W);
#endif
    //i2c_write_byte(MPRxDevAddr, 0x4E, 0x03);
    i2c_write_byte(MPRxDevAddr, 0xB0, 0x00);  //0x1 enable TX pattern generator
    //i2c_write_byte(MPRxDevAddr, 0xE2, 0x00);
    //i2c_write_byte(MPRxDevAddr, 0xE3, 0x00);
    //i2c_write_byte(MPRxDevAddr, 0xE4, 0x00);
    //i2c_write_byte(MPRxDevAddr, 0xE5, 0xFF);
    //i2c_write_byte(MPRxDevAddr, 0xE6, 0xFF);
    //i2c_write_byte(MPRxDevAddr, 0xE7, 0xFF);
    //i2c_write_byte(MPRxDevAddr, 0xB6, 0x40); //Reg_PGColBar
    i2c_write_byte(MPRxDevAddr, 0xD0, 0x00); //0x22 //0x32 enable auto bist
    i2c_write_byte(MPRxDevAddr, 0x05, 0x00);
    i2c_write_byte(MPRxDevAddr, 0x06, 0x11);//0x01

    return true;
}

static void IT6151_DPTX_init(void)
{
    //printf(" IT6151_DPTX_init !!!\n");

    i2c_write_byte(DP_I2C_ADDR, 0x05, 0x29);
    i2c_write_byte(DP_I2C_ADDR, 0x05, 0x00);
    i2c_write_byte(DP_I2C_ADDR, 0x09, INT_MASK);// Enable HPD_IRQ,HPD_CHG,VIDSTABLE
    i2c_write_byte(DP_I2C_ADDR, 0x0A, 0x00);
    i2c_write_byte(DP_I2C_ADDR, 0x0B, 0x00);
    i2c_write_byte(DP_I2C_ADDR, 0xC5, 0xC1);
    i2c_write_byte(DP_I2C_ADDR, 0xB5, 0x00);
    i2c_write_byte(DP_I2C_ADDR, 0xB7, 0x80);
    i2c_write_byte(DP_I2C_ADDR, 0xC4, 0xF0);
    i2c_write_byte(DP_I2C_ADDR, 0x06, 0xFF);// Clear all interrupt
    i2c_write_byte(DP_I2C_ADDR, 0x07, 0xFF);// Clear all interrupt
    i2c_write_byte(DP_I2C_ADDR, 0x08, 0xFF);// Clear all interrupt
    i2c_write_byte(DP_I2C_ADDR, 0x05, 0x00);
    i2c_write_byte(DP_I2C_ADDR, 0x0c, 0x08);
    i2c_write_byte(DP_I2C_ADDR, 0x21, 0x05);
    i2c_write_byte(DP_I2C_ADDR, 0x3a, 0x04);
    i2c_write_byte(DP_I2C_ADDR, 0x5f, 0x06);
    i2c_write_byte(DP_I2C_ADDR, 0xc9, 0xf5);
    i2c_write_byte(DP_I2C_ADDR, 0xca, 0x4c);
    i2c_write_byte(DP_I2C_ADDR, 0xcb, 0x37);
    i2c_write_byte(DP_I2C_ADDR, 0xce, (sPInfo.ucDpBR & 0x80));
    i2c_write_byte(DP_I2C_ADDR, 0xd3, 0x03);
    i2c_write_byte(DP_I2C_ADDR, 0xd4, 0x60);
    i2c_write_byte(DP_I2C_ADDR, 0xe8, 0x11);
    i2c_write_byte(DP_I2C_ADDR, 0xec, sPInfo.ucVic);
    usleep(5000);
    i2c_write_byte(DP_I2C_ADDR, 0x62, DP_BPP);
    i2c_write_byte(DP_I2C_ADDR, 0x23, 0x42);
    i2c_write_byte(DP_I2C_ADDR, 0x24, 0x07);
    i2c_write_byte(DP_I2C_ADDR, 0x25, 0x01);
    i2c_write_byte(DP_I2C_ADDR, 0x26, 0x00);
    i2c_write_byte(DP_I2C_ADDR, 0x27, 0x10);
    i2c_write_byte(DP_I2C_ADDR, 0x2B, 0x05);
    i2c_write_byte(DP_I2C_ADDR, 0x23, 0x40);
    i2c_write_byte(DP_I2C_ADDR, 0x22, (DP_AUX_PN_SWAP << 3) | (DP_PN_SWAP << 2) | 0x03);
    i2c_write_byte(DP_I2C_ADDR, 0x16, (DPTX_SSC_SETTING << 4) | (DP_LANE_SWAP << 3) | (sPInfo.ucDpLanes << 1) | (sPInfo.ucDpBR & 0x01));
    i2c_write_byte(DP_I2C_ADDR, 0x0f, 0x01);
    i2c_write_byte(DP_I2C_ADDR, 0x76, 0xa7);
    i2c_write_byte(DP_I2C_ADDR, 0x77, 0xaf);
    //i2c_write_byte(DP_I2C_ADDR, 0x78, 0x7c);	//added for LG 1080PRB flick test for TCL
    //i2c_write_byte(DP_I2C_ADDR, 0x79, 0x34);	//added for LG 1080PRB flick test for TCL
    i2c_write_byte(DP_I2C_ADDR, 0x7e, 0x8f);
    i2c_write_byte(DP_I2C_ADDR, 0x7f, 0x07);
    i2c_write_byte(DP_I2C_ADDR, 0x80, 0xef);
    i2c_write_byte(DP_I2C_ADDR, 0x81, 0x5f);
    i2c_write_byte(DP_I2C_ADDR, 0x82, 0xef);
    i2c_write_byte(DP_I2C_ADDR, 0x83, 0x07);
    i2c_write_byte(DP_I2C_ADDR, 0x88, 0x38);
    i2c_write_byte(DP_I2C_ADDR, 0x89, 0x1f);
    i2c_write_byte(DP_I2C_ADDR, 0x8a, 0x48);
    i2c_write_byte(DP_I2C_ADDR, 0x0f, 0x00);
    i2c_write_byte(DP_I2C_ADDR, 0x5c, 0xf3);
    i2c_write_byte(DP_I2C_ADDR, 0x17, 0x04);
    i2c_write_byte(DP_I2C_ADDR, 0x17, 0x01);
    usleep(5000);
    i2c_write_byte(DP_I2C_ADDR, 0xD3, 0x03);
    i2c_write_byte(DP_I2C_ADDR, 0x22, 0xC3);
}

static void IT6151_MIPI_Init(void)
{
    //printf(" IT6151_MIPI_init !!!\n");

    //i2c_write_byte(MIPI_I2C_ADDR,0x05,0x33);
    i2c_write_byte(MIPI_I2C_ADDR, 0x05, 0x00);
    i2c_write_byte(MIPI_I2C_ADDR, 0x0c, (MP_LANE_SWAP << 7) | (MP_PN_SWAP << 6) | (sPInfo.ucMpLanes << 4));
    i2c_write_byte(MIPI_I2C_ADDR, 0x11, MP_MCLK_INV | 0x10);
    i2c_write_byte(MIPI_I2C_ADDR, 0x19, (MP_CONTINUOUS_CLK << 1) | MP_LANE_DESKEW);
    i2c_write_byte(MIPI_I2C_ADDR, 0x4B, (sPInfo.ucMpReSync & 0x18));
    i2c_write_byte(MIPI_I2C_ADDR, 0x4C, MIPI_FFRdStg & 0xFF);
    i2c_write_byte(MIPI_I2C_ADDR, 0x4D, (MIPI_FFRdStg >> 8) & 0x01);
    i2c_write_byte(MIPI_I2C_ADDR, 0x4E, ((sPInfo.ucMpReSync & 0x03) << 2) | (sPInfo.ucMpVPol << 1) | (sPInfo.ucMpHPol));
    i2c_write_byte(MIPI_I2C_ADDR, 0x4F, 0x01);
    i2c_write_byte(MIPI_I2C_ADDR, 0x72, 0x01);
    i2c_write_byte(MIPI_I2C_ADDR, 0x73, 0x03);
    i2c_write_byte(MIPI_I2C_ADDR, 0x80, sPInfo.ucMpClkDiv);

    i2c_write_byte(MIPI_I2C_ADDR, 0xC0, (sPInfo.ucHighPclk << 4) | 0x07);//0x1F);//0x13);
#if (AUTO_OUTPUT == 1)
    {
        i2c_write_byte(MIPI_I2C_ADDR, 0xC1, 0x01);
    }

#else
    i2c_write_byte(MIPI_I2C_ADDR, 0xC1, 0x71);
#endif

    i2c_write_byte(MIPI_I2C_ADDR, 0xC2, REG_C2);//0x25); //0x47
    i2c_write_byte(MIPI_I2C_ADDR, 0xC3, 0x67);//0x37);//0x67);
    i2c_write_byte(MIPI_I2C_ADDR, 0xC4, 0x03);//0x03);//0x04);
    i2c_write_byte(MIPI_I2C_ADDR, 0xCB, (LVDS_PN_SWAP << 5) | (LVDS_LANE_SWAP << 4) | (LVDS_6BIT << 2) | (LVDS_DC_BALANCE << 1) | VESA_MAP);
#if (MIPI_EVENT_MODE == 1)
    i2c_write_byte(MIPI_I2C_ADDR, 0x33, 0x80 | MIPI_HSYNC_W >> 8);
    i2c_write_byte(MIPI_I2C_ADDR, 0x32, MIPI_HSYNC_W);
    i2c_write_byte(MIPI_I2C_ADDR, 0x3D, 0x80 | MIPI_VSYNC_W >> 8);
    i2c_write_byte(MIPI_I2C_ADDR, 0x3C, MIPI_VSYNC_W);
#endif

    i2c_write_byte(MIPI_I2C_ADDR, 0x06, 0xFF);
    i2c_write_byte(MIPI_I2C_ADDR, 0x07, 0xFF);

    i2c_write_byte(MIPI_I2C_ADDR, 0x09, sPInfo.ucIntMask);
    i2c_write_byte(MIPI_I2C_ADDR, 0x0A, 0xC0);

    i2c_write_byte(MIPI_I2C_ADDR, 0x92, 0x14);
    i2c_write_byte(MIPI_I2C_ADDR, 0x90, 0x01);

    i2c_write_byte(MIPI_I2C_ADDR, 0x07, 0xFF);
    i2c_write_byte(MIPI_I2C_ADDR, 0x08, 0xFF);
    i2c_write_byte(MIPI_I2C_ADDR, 0x0A, 0x3F);
    i2c_write_byte(MIPI_I2C_ADDR, 0x0B, 0x7F);

    i2c_write_byte(MIPI_I2C_ADDR, 0x05, 0x32);
    i2c_write_byte(MIPI_I2C_ADDR, 0x05, 0x30);
    i2c_write_byte(MIPI_I2C_ADDR, 0x05, 0x00);
}

static int IT6151_init(void)
{
    bool		result = 0;
    unsigned char VenID[2], DevID[2], RevID;

    i2c_read_byte(DP_I2C_ADDR, 0x00, &VenID[0]);
    i2c_read_byte(DP_I2C_ADDR, 0x01, &VenID[1]);
    i2c_read_byte(DP_I2C_ADDR, 0x02, &DevID[0]);
    i2c_read_byte(DP_I2C_ADDR, 0x03, &DevID[1]);
    i2c_read_byte(DP_I2C_ADDR, 0x04, &RevID);

    //printf("Current DPDevID=%02X%02X\n", DevID[1], DevID[0]);
    //printf("Current DPVenID=%02X%02X\n", VenID[1], VenID[0]);
    //printf("Current DPRevID=%02X\n\n", RevID);
    //printf(" IT6151_init !!!\n");

    if (VenID[0] == 0x54 && VenID[1] == 0x49 && DevID[0] == 0x51 && DevID[1] == 0x61){

        //printf(" Test 1 DP_I2C_ADDR=0x%x, MIPI_I2C_ADDR=0x%x\n", DP_I2C_ADDR, MIPI_I2C_ADDR);

        i2c_write_byte(DP_I2C_ADDR, 0x05, 0x04);// DP SW Reset
        i2c_write_byte(DP_I2C_ADDR, 0xfd, (MIPI_I2C_ADDR << 1) | 1);
        i2c_write_byte(MIPI_I2C_ADDR, 0x05, 0x00);
        i2c_write_byte(MIPI_I2C_ADDR, 0x0c, (MP_LANE_SWAP << 7) | (MP_PN_SWAP << 6) | (sPInfo.ucMpLanes << 4) | sPInfo.ucUFO);
        i2c_write_byte(MIPI_I2C_ADDR, 0x11, MP_MCLK_INV);
        if (RevID == 0xA1){
            i2c_write_byte(MIPI_I2C_ADDR, 0x19, MP_LANE_DESKEW);
        }
        else{
            i2c_write_byte(MIPI_I2C_ADDR, 0x19, (MP_CONTINUOUS_CLK << 1) | MP_LANE_DESKEW);
        }

        i2c_write_byte(MIPI_I2C_ADDR, 0x27, sPInfo.ucMpFmt);
        i2c_write_byte(MIPI_I2C_ADDR, 0x28, ((sPInfo.usPWidth / 4 - 1) >> 2) & 0xC0);
        i2c_write_byte(MIPI_I2C_ADDR, 0x29, (sPInfo.usPWidth / 4 - 1) & 0xFF);

        i2c_write_byte(MIPI_I2C_ADDR, 0x2e, 0x34);
        i2c_write_byte(MIPI_I2C_ADDR, 0x2f, 0x01);

        i2c_write_byte(MIPI_I2C_ADDR, 0x4e, ((sPInfo.ucDpReSync & 0x03) << 2) | (sPInfo.ucMpVPol << 1) | (sPInfo.ucMpHPol));
        i2c_write_byte(MIPI_I2C_ADDR, 0x80, (sPInfo.ucUFO << 5) | sPInfo.ucMpClkDiv);

        i2c_write_byte(MIPI_I2C_ADDR, 0x09, MIPI_RECOVER);
        i2c_write_byte(MIPI_I2C_ADDR, 0x92, TIMER_CNT);
#if (MIPI_EVENT_MODE == 1)
        i2c_write_byte(MIPI_I2C_ADDR, 0x33, 0x80 | MIPI_HSYNC_W >> 8);
        i2c_write_byte(MIPI_I2C_ADDR, 0x32, MIPI_HSYNC_W & 0xFF);
        i2c_write_byte(MIPI_I2C_ADDR, 0x3D, 0x80 | MIPI_VSYNC_W >> 8);
        i2c_write_byte(MIPI_I2C_ADDR, 0x3C, MIPI_VSYNC_W & 0xFF);
#endif
        i2c_write_byte(MIPI_I2C_ADDR, 0x18, 0x20 | sPInfo.ucHSSetNum); //SkipStg 0x30

        IT6151_DPTX_init();

        return 0;
    }

    //printf(" Test 2 DP_I2C_ADDR=0x%x, MIPI_I2C_ADDR=0x%x\n", DP_I2C_ADDR, MIPI_I2C_ADDR);

    i2c_read_byte(MIPI_I2C_ADDR, 0x00, &VenID[0]);
    i2c_read_byte(MIPI_I2C_ADDR, 0x01, &VenID[1]);
    i2c_read_byte(MIPI_I2C_ADDR, 0x02, &DevID[0]);
    i2c_read_byte(MIPI_I2C_ADDR, 0x03, &DevID[1]);
    i2c_read_byte(MIPI_I2C_ADDR, 0x04, &RevID);

    //printf("Current MPDevID=%02X%02X\n", DevID[1], DevID[0]);
    //printf("Current MPVenID=%02X%02X\n", VenID[1], VenID[0]);
    //printf("Current MPRevID=%02X\n\n", RevID);

    if (VenID[0] == 0x54 && VenID[1] == 0x49 && DevID[0] == 0x21 && DevID[1] == 0x61){
        IT6151_MIPI_Init();
        return 1;
    }
    return -1;
}

static bool frist = 0;

static void IT6151_Debug(void)
{
    unsigned char Reg;
    uint32_t i = 0;

    i2c_read_byte(MIPI_I2C_ADDR, 0xD, &Reg);
    if (Reg == 0x31 && frist)
        return;
    frist = 1;
    //printf("--------------Read MIPI RX--------------------\n");
    for (i = 0; i < 0xFF; i++)
    {
        i2c_read_byte(DP_I2C_ADDR, i, &Reg);
        //printf("Read DP TX read_data[0x%X] = 0x%X\n", i, Reg);
    }
    for (i = 0; i < 0xFF; i++)
    {
        i2c_read_byte(MIPI_I2C_ADDR, i, &Reg);
        //printf("Read MIPI RX read_data[0x%X] = 0x%X\n", i, Reg);
    }

}

static int IT6151_ESD_Check(void)
{
    static 	unsigned char ucIsIT6151 = 0xFF;
    unsigned char ucReg, ucStat;

    if (ucIsIT6151 == 0xFF){
        unsigned char VenID[2], DevID[2];

        //printf("\nIT6151 1st IRQ !!!\n");
        i2c_read_byte(DP_I2C_ADDR, 0x00, &VenID[0]);
        i2c_read_byte(DP_I2C_ADDR, 0x01, &VenID[1]);
        i2c_read_byte(DP_I2C_ADDR, 0x02, &DevID[0]);
        i2c_read_byte(DP_I2C_ADDR, 0x03, &DevID[1]);

        //printf("Current DevID=%02X%02X\n", (int)DevID[1], (int)DevID[0]);
        //printf("Current VenID=%02X%02X\n", (int)VenID[1], (int)VenID[0]);

        if (VenID[0] == 0x54 && VenID[1] == 0x49 && DevID[0] == 0x51 && DevID[1] == 0x61){
            ucIsIT6151 = 1;
        }
        else{
            ucIsIT6151 = 0;
        }
    }

    if (ucIsIT6151 == 1){
        i2c_read_byte(DP_I2C_ADDR, 0x05, &ucReg);
        if (ucReg & 0x01){
            return 1;
        }
        i2c_read_byte(DP_I2C_ADDR, 0x0D, &ucReg);


        if (ucReg & 0x89){
            printf("\nIT6151 Reg0x0D=0x%x !!!\n", ucReg);
            i2c_read_byte(DP_I2C_ADDR, 0x06, &ucReg);
            printf("\nIT6151 DReg0x06=0x%x !!!\n", ucReg);
            i2c_read_byte(DP_I2C_ADDR, 0x07, &ucReg);
            printf("\nIT6151 DReg0x07=0x%x !!!\n", ucReg);
            i2c_read_byte(DP_I2C_ADDR, 0x08, &ucReg);
            printf("\nIT6151 DReg0x08=0x%x !!!\n", ucReg);

            i2c_read_byte(DP_I2C_ADDR, 0xb6, &ucReg);
            printf("\nIT6151 DReg0xb6=0x%x !!!\n", ucReg);
            i2c_read_byte(DP_I2C_ADDR, 0xc4, &ucReg);
            printf("\nIT6151 DReg0xc4=0x%x !!!\n", ucReg);
            i2c_read_byte(DP_I2C_ADDR, 0xb7, &ucReg);
            printf("\nIT6151 DReg0xb7=0x%x !!!\n", ucReg);
            i2c_read_byte(DP_I2C_ADDR, 0x21, &ucReg);
            printf("\nIT6151 DReg0x21=0x%x !!!\n", ucReg);
        }

        i2c_read_byte(DP_I2C_ADDR, 0x0D, &ucReg);
        //printf("\nIT6151 Reg0x0D=0x%x !!!\n", ucReg);

        if (ucReg & 0x80){	//MIPI_IRQ
            i2c_read_byte(MIPI_I2C_ADDR, 0x06, &ucStat);
            if (ucStat & 0x01){
                i2c_write_byte(MIPI_I2C_ADDR, 0x06, ucStat);

                i2c_read_byte(MIPI_I2C_ADDR, 0x0D, &ucStat);
                if (ucStat & 0x10){
                    //disable timer
                    i2c_write_byte(MIPI_I2C_ADDR, 0x0B, 0x00);
                    i2c_write_byte(MIPI_I2C_ADDR, 0x08, 0x40);
                }
                else{
                    //enable timer
                    i2c_write_byte(MIPI_I2C_ADDR, 0x0B, 0x40);
                }
            }
            i2c_read_byte(MIPI_I2C_ADDR, 0x08, &ucStat);
            if (ucStat & 0x40){
                if (ucStat & 0x20){
                    //disable timer
                    i2c_write_byte(MIPI_I2C_ADDR, 0x0B, 0x00);
                    i2c_write_byte(MIPI_I2C_ADDR, 0x08, 0x40);
                }
                else{
                    return 1;
                }
            }
        }

        if (ucReg & 0x01){	//DP_IRQ
            i2c_read_byte(DP_I2C_ADDR, 0x21, &ucStat);
            if (ucStat & 0x02){
                i2c_write_byte(DP_I2C_ADDR, 0x21, ucStat);
            }
            i2c_read_byte(DP_I2C_ADDR, 0x06, &ucReg);
            i2c_read_byte(DP_I2C_ADDR, 0x0D, &ucStat);
            if (ucReg & 0x03){
                if (ucStat & 0x02){
                    return 1;
                }
            }
        }
    }
    else if (ucIsIT6151 == 0){
        i2c_read_byte(MIPI_I2C_ADDR, 0x05, &ucReg);
        if (ucReg & 0x33){
            IT6151_MIPI_Init();
        }
        else{
            i2c_read_byte(MIPI_I2C_ADDR, 0x06, &ucReg);
            if ((ucReg & 0x11) == 0x11){

                //printf("\nIT6151 Reg0x06=0x%x !!!\n", ucReg);
                i2c_write_byte(MIPI_I2C_ADDR, 0x06, ucReg);
                i2c_read_byte(MIPI_I2C_ADDR, 0x0D, &ucReg);
                if ((ucReg & 0x38) != 0x30){
#if 1
                    IT6151_MIPI_Init();
#else
                    i2c_read_byte(MIPI_I2C_ADDR, 0x05, &ucReg);
                    i2c_write_byte(MIPI_I2C_ADDR, 0x05, ucReg | 0x20);
                    i2c_read_byte(MIPI_I2C_ADDR, 0x37, &ucReg);
                    i2c_write_byte(MIPI_I2C_ADDR, 0x37, ucReg & 0x7F);
#endif			
                }
                else{
                    unsigned short usTemp;

                    i2c_read_byte(MIPI_I2C_ADDR, 0x04, &ucReg);
                    if (ucReg == 0xA0){
                        i2c_read_byte(MIPI_I2C_ADDR, 0x37, &ucReg);
                        if (ucReg & 0x80){
                            i2c_write_byte(MIPI_I2C_ADDR, 0x37, ucReg & 0x7F);
                            i2c_read_byte(MIPI_I2C_ADDR, 0x37, &ucReg);
                        }

                        usTemp = ucReg;
                        usTemp <<= 8;
                        i2c_read_byte(MIPI_I2C_ADDR, 0x36, &ucReg);
                        usTemp += ucReg;
                        usTemp++;

                        i2c_write_byte(MIPI_I2C_ADDR, 0x36, usTemp & 0xFF);
                        i2c_write_byte(MIPI_I2C_ADDR, 0x37, 0x80 | (usTemp >> 8));
                    }
                    i2c_read_byte(MIPI_I2C_ADDR, 0x05, &ucReg);
                    i2c_write_byte(MIPI_I2C_ADDR, 0x05, ucReg & 0xDF);
                }
            }
        }
    }
    return 0;
}

static void IT6151_ESD_Recover(void)
{
    unsigned char ucStat;

    i2c_read_byte(MIPI_I2C_ADDR, 0x08, &ucStat);
    if (ucStat & 0x40){
        i2c_write_byte(MIPI_I2C_ADDR, 0x0B, 0x00);
        i2c_write_byte(MIPI_I2C_ADDR, 0x08, 0x40);
        IT6151_init();
    }
    else{
        IT6151_DPTX_init();
    }
}

static void* IT6151_check_Task(void* arg)
{
    for (;;)
    {
        unsigned char Reg, RxReg;
        int result = 0;

        {
            i2c_read_byte(DP_I2C_ADDR, 0xD, &Reg);
            i2c_read_byte(MIPI_I2C_ADDR, 0xD, &RxReg);

            result = IT6151_ESD_Check();
            if (((Reg & 0x80) && ((RxReg & 0x30) != 0x30)) || (RxReg & 0x8) || (RxReg == 0))
            {
                //printf("main Read DP TX read_data[0xD] = 0x%X\n", Reg);
                //printf("main Read MIPI RX read_data[0xD] = 0x%X\n", RxReg);
                if (errorSum > 0)
                {

                    uint32_t lcdBaseA = ithLcdGetBaseAddrA();
                    uint32_t lcdBaseB = ithLcdGetBaseAddrB();
#if defined(CFG_VIDEO_ENABLE) || defined(CFG_LCD_TRIPLE_BUFFER)
                    uint32_t lcdBaseC = ithLcdGetBaseAddrC();
#endif

                    //printf("main+++++++++++++++++++++++++++++++result:%d\n", result);
                    //IT6151_Debug();

                    ithCmdQWaitEmpty(ITH_CMDQ0_OFFSET);

                    ithLcdReset();

                    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_RESET, NULL);

                    usleep(1000);
                    ithLcdEnableHwFlip();
                    usleep(1000);

                    //memset((void*)&__lcd_base_a, 0, CFG_LCD_PITCH * CFG_LCD_HEIGHT);
                    //memset((void*)&__lcd_base_b, 0, CFG_LCD_PITCH * CFG_LCD_HEIGHT);
                    //memset((void*)&__lcd_base_c, 0, CFG_LCD_PITCH * CFG_LCD_HEIGHT);

                    ithLcdSetBaseAddrA(lcdBaseA);
                    ithLcdSetBaseAddrB(lcdBaseB);
#if defined(CFG_VIDEO_ENABLE) || defined(CFG_LCD_TRIPLE_BUFFER)
                    ithLcdSetBaseAddrC(lcdBaseC);
#endif
                    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);

                    usleep(100000);
                    IT6151_ESD_Recover();

                    errorSum = 0;
                }
                else
                    errorSum++;
            }
        }
        usleep(1000000);
    }
}
#endif //#ifndef _WIN32

/**
* Save MIPI HS information settings
*/
static void ithLcdSaveMipiHSInfo(void)
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
}

/**
* Reset MIPI HS information settings
*/
static void ithLcdResetMipiHSInfo(void)
{
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

/**
* Save RGB pin share information settings
*/
static void ithLcdSavePinShareInfo(void)
{
    //save LCD pin share registers
    //pinShareTable[0] = ithReadRegA(0xD10000E4);
    pinShareTable[1] = ithReadRegA(0xD10000E8);
    pinShareTable[2] = ithReadRegA(0xD10000EC);
    pinShareTable[3] = ithReadRegA(0xD1000160);
    pinShareTable[4] = ithReadRegA(0xD1000164);
    //pinShareTable[5] = ithReadRegA(0xD1000168);
    //pinShareTable[6] = ithReadRegA(0xD100016C);
}

/**
* Reset RGB pin share information settings
*/
static void ithLcdResetPinShareInfo(void)
{
    //resetting LCD pin share registers
    //ithWriteRegA(0xD10000E4, pinShareTable[0]); 
    ithWriteRegA(0xD10000E8, pinShareTable[1]);
    ithWriteRegA(0xD10000EC, pinShareTable[2]);
    ithWriteRegA(0xD1000160, pinShareTable[3]);
    ithWriteRegA(0xD1000164, pinShareTable[4]);
    //ithWriteRegA(0xD1000168, pinShareTable[5]);  
    //ithWriteRegA(0xD100016C, pinShareTable[6]); 
}

/**
* Save MIPI DPHY information settings
*/
static void ithLcdSaveMipiDPHYInfo(void)
{
    mipiDPHYInfo[0] = ithReadRegA(0xD0D00000);
    mipiDPHYInfo[1] = ithReadRegA(0xD0D0001C);

    mipiDPHYInfo[2] = ithReadRegA(0xD0D00004);
    mipiDPHYInfo[3] = ithReadRegA(0xD0D00008);
    mipiDPHYInfo[4] = ithReadRegA(0xD0D0000C);
    mipiDPHYInfo[5] = ithReadRegA(0xD0D00010);
    mipiDPHYInfo[6] = ithReadRegA(0xD0D00014);
    mipiDPHYInfo[7] = ithReadRegA(0xD0D00018);
}

/**
* Reset MIPI DPHY information settings
*/
static void ithLcdResetMipiDPHYInfo(void)
{
    /* ************************************************* */
    /*          MIPI DPHY reg base: 0xD0D00000           */
    /* ************************************************* */
    ithWriteRegA(0xD0D00000, (mipiDPHYInfo[0] & ~(0x1 << 24)) | (0 & (0x1 << 24)));    // PLLNS=48, Pad Type=MIPI, [21:17]P/N SWAP
    ithWriteRegA(0xD0D00004, mipiDPHYInfo[2]);    // PLLMS=1, PLLF=1/8 (First, datarateclk change to slow)
    ithWriteRegA(0xD0D0001C, mipiDPHYInfo[1]);    // ESCCLK = BYTECLK/3
    ithWriteRegA(0xD0D00000, mipiDPHYInfo[0]);    // PLL ENABLE
    usleep(200);

    ithWriteRegA(0xD0D00008, mipiDPHYInfo[3]);
    ithWriteRegA(0xD0D0000C, mipiDPHYInfo[4]);
    ithWriteRegA(0xD0D00010, mipiDPHYInfo[5]);
    ithWriteRegA(0xD0D00014, mipiDPHYInfo[6]);
    ithWriteRegA(0xD0D00018, mipiDPHYInfo[7]);

    ithWriteRegA(0xD0D00004, 0x055e8001);  //CLKEN,DATAEN
    ithWriteRegA(0xD0D00004, 0x055f8001);  //RESET
    usleep(1);
    ithWriteRegA(0xD0D00004, 0x055e8041);  //normal
    usleep(200);
}

void ithLcdReset(void)
{
    ithWriteRegMaskA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, 0xFFFFFFFF, (0x1 << ITH_LCD_REG_RST_BIT) | (0x1 << ITH_LCD_RST_BIT));
    ithWriteRegMaskA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, 0x0, (0x1 << ITH_LCD_REG_RST_BIT) | (0x1 << ITH_LCD_RST_BIT));
}

void ithLcdEnable(void)
{
    // enable clock
    if (gPanelType != ITH_LCD_RGB)
        ithSetRegBitA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, ITH_EN_DCLK_BIT);
    ithSetRegBitA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, ITH_EN_M3CLK_BIT);
    ithSetRegBitA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, ITH_EN_W12CLK_BIT);

#if (CFG_GPIO_LCD_PWR_EN > 0)
#ifdef CFG_GPIO_LCD_PWR_EN_ACTIVE_LOW
    ithGpioClear(CFG_GPIO_LCD_PWR_EN);
#else
    ithGpioSet(CFG_GPIO_LCD_PWR_EN);
#endif
#endif

    // disable LCD pin input mode
    if (gPanelType == ITH_LCD_RGB)
    {
        usleep(100);
        ithWriteRegA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG, 0x00000001);
        usleep(10);
        ithWriteRegA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG, 0x00000003);
        usleep(10000);

        ithLcdResetPinShareInfo();
    }
    else if (gPanelType == ITH_LCD_MIPI)
    {
#if (CFG_GPIO_LCD_PWR_EN > 0)
        ithLcdReset();
        usleep(1000);

        ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_RESET, NULL);

        ithLcdEnableHwFlip();

        ithLcdSetBaseAddrA(lcdA);
        ithLcdSetBaseAddrB(lcdB);
        #if defined(CFG_VIDEO_ENABLE) || defined(CFG_LCD_TRIPLE_BUFFER)
        ithLcdSetBaseAddrC(lcdC);
        #endif
        ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
#else
        //MIPI PHY normal
        ithWriteRegA(0xD8000044, 0x00280001);  //[31] DPHY PORn rst normal
        usleep(200);                           //200us

        // reset mipi DPHY
        ithLcdResetMipiDPHYInfo();

        // MIPI controller
        ithWriteRegA(0xD800004C, 0x0002C001);    // MIPI controller normal
        usleep(200);

        // LCD Setting (CPUIF FOR DBI), CPUIF mode
        ithWriteRegA(ITH_LCD_BASE + ITH_LCD_SET_MODE_REG, 0x0F7F0410);    // CPUIF

        // MIPI reg base: 0xd0c00000 (LP) 
        ithWriteRegA(0xD0C00004, 0x004F028F);    // $6[7]=BLLP, +$04[0]=EOTPGE
        ithWriteRegA(0xD0C00010, 0x000F0000);
        ithWriteRegA(0xD0C00014, 0x0000001B);
        usleep(200000);                          //200ms 

        // LCD Setting, normal RGB mode
        ithWriteRegA(ITH_LCD_BASE + ITH_LCD_SET_MODE_REG, 0x0F7F0A60);    // SRC:RGB565, dst 24-bits
        ithWriteRegA(ITH_LCD_BASE + ITH_LCD_TEST_COLOR_SET_REG, 0x00000000);    // test color mode=0, None

        // CTG Setting
        ithWriteRegA(ITH_LCD_BASE + ITH_LCD_TCON_CTG_REG, 0x00010300);    // ctg_reset_on
        usleep(1);                                                        // 1 μs
        ithWriteRegA(ITH_LCD_BASE + ITH_LCD_TCON_CTG_REG, 0x00000307);    // enable ctg 0 1 2

        // reset HS
        ithLcdResetMipiHSInfo();

        // Enable LCD
        ithWriteRegA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG, 0x00000001);
        usleep(10);
        ithWriteRegA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG, 0x00000003);
        usleep(10000);
#endif
    }
    else if (gPanelType == ITH_LCD_LVDS)
    {
        // LVDS Enable
        ithSetRegBitA(ITH_MIPI_DPHY_BASE, 24);                     // [24]PLL enable
        usleep(200);
        ithSetRegBitA(ITH_LCD_BASE + ITH_LCD_LVDS_SET1_REG, 0);    // [0] Disable LVDS
 
        ithWriteRegA(ITH_LCD_BASE + ITH_LCD_TEST_COLOR_SET_REG, 0x00000000);    // test color mode=0, None

        // CTG Setting                    
        ithWriteRegA(ITH_LCD_BASE + ITH_LCD_TCON_CTG_REG, 0x00010300);    // ctg_reset_on
        usleep(1);                          // 1 μs
        ithWriteRegA(ITH_LCD_BASE + ITH_LCD_TCON_CTG_REG, 0x00000307);    // enable ctg 0 1 2

        // Enable LCD
        ithWriteRegA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG, 0x00000001);
        usleep(10);
        ithWriteRegA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG, 0x00000003);
        usleep(10000);
    }

    if (gPanelType == ITH_LCD_RGB)
    {
        //set reg 0x0020 as 0x80000000 for leave test mode
        // wait for 0x80000000 become 0x00000000
        ithWriteRegA(ITH_LCD_BASE + ITH_LCD_TEST_COLOR_SET_REG, 0x80000000);
        while (ithReadRegA(ITH_LCD_BASE + ITH_LCD_TEST_COLOR_SET_REG) & 0x80000000)
        {
            ithDelay(1000);
        }
    }
}

void ithLcdDisable(void)
{
    //set reg 0x0020 as 0x81000000 for test mode
    // wait for 0x81000000 become 0x01000000
    ithWriteRegA(ITH_LCD_BASE + ITH_LCD_TEST_COLOR_SET_REG, 0x81000000);
    while (ithReadRegA(ITH_LCD_BASE + ITH_LCD_TEST_COLOR_SET_REG) & 0x80000000)
    {
        ithDelay(1000);
    }

    gPanelType = ithLcdGetPanelType();

    // set LCD pin input mode
    if (gPanelType == ITH_LCD_RGB)
    {
        ithLcdSavePinShareInfo();
        //ithWriteRegA(0xD10000E4, 0);
        ithWriteRegA(0xD10000E8, 0);
        ithWriteRegA(0xD10000EC, 0);
        ithWriteRegA(0xD1000160, 0);
        ithWriteRegA(0xD1000164, 0);
        //ithWriteRegA(0xD1000168, 0);
        //ithWriteRegA(0xD100016C, 0);

        usleep(10000);
        ithWriteRegA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG, 0x00000002);
        usleep(10);
        ithWriteRegA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG, 0x00000000);
        usleep(100);
    }
    else if (gPanelType == ITH_LCD_MIPI)
    {
        // save HS and DPHY Info
        ithLcdSaveMipiHSInfo();
        ithLcdSaveMipiDPHYInfo();

        //LCD controller disable, CTG reset
        ithWriteRegA(ITH_LCD_BASE + ITH_LCD_TCON_CTG_REG, 0x00010307);    // for Hsync=1, Vsync=1
        usleep(1);

        ithWriteRegA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG, 0x00000002);
        usleep(10);
        ithWriteRegA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG, 0x00000000);

        //MIPI PHY PORn
        ithWriteRegA(0xD8000044, 0x80280001);    //[31]=1 DPHY PORn, En_W20CLK(mipi ctrl),En_W21CLK(mipi phy)
        usleep(1);

        //MIPI controller reset
        ithWriteRegA(0xD800004C, 0xC002C001);    //[31]:Reset MIPI power controller,[30]:Reset MIPI system controller,  KDSICLK
    }
    else if (gPanelType == ITH_LCD_LVDS)
    {
        // LCD disable
        ithWriteRegA(ITH_LCD_BASE + ITH_LCD_TCON_CTG_REG, 0x00010307);  //for Hsync=1, Vsync=1
        usleep(1);

        ithWriteRegA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG, 0x00000002);    //Display disable
        usleep(10);
        ithWriteRegA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG, 0x00000000);    //Sync disable
        usleep(100);

        // LVDS disable
        ithClearRegBitA(ITH_MIPI_DPHY_BASE, 24);                        // [24]PLL disable
        ithClearRegBitA(ITH_LCD_BASE + ITH_LCD_LVDS_SET1_REG, 0);       // [0] Disable LVDS
    }

#if (CFG_GPIO_LCD_PWR_EN > 0)
    lcdA = ithLcdGetBaseAddrA();
    lcdB = ithLcdGetBaseAddrB();
#if defined(CFG_VIDEO_ENABLE) || defined(CFG_LCD_TRIPLE_BUFFER)
    lcdC = ithLcdGetBaseAddrC();
#endif
    ithLcdReset();
#else
    //reset LCD controller
    ithWriteRegMaskA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, 0x1 << ITH_LCD_RST_BIT, 0x1 << ITH_LCD_RST_BIT);
    usleep(2);
    ithWriteRegMaskA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, 0x0, 0x1 << ITH_LCD_RST_BIT);
#endif

    // disable clock
    ithClearRegBitA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, ITH_EN_M3CLK_BIT);
    ithClearRegBitA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, ITH_EN_W12CLK_BIT);
    if (gPanelType != ITH_LCD_RGB)
        ithClearRegBitA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, ITH_EN_DCLK_BIT);
    
#if (CFG_GPIO_LCD_PWR_EN > 0)
#ifdef CFG_GPIO_LCD_PWR_EN_ACTIVE_LOW
    ithGpioSet(CFG_GPIO_LCD_PWR_EN);
#else
    ithGpioClear(CFG_GPIO_LCD_PWR_EN);
#endif
#endif
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
    if (ithReadRegA(ITH_LCD_BASE + ITH_LCD_MIPI_SET1_REG) & 0x1)
        gPanelType = ITH_LCD_MIPI;
    else if (ithReadRegA(ITH_LCD_BASE + ITH_LCD_LVDS_SET1_REG) & 0x1)
        gPanelType = ITH_LCD_LVDS;

    return gPanelType;

}

#ifndef _WIN32
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//for IT6122 API
////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ithLcdInitIT6122(void)
{
    //IT6122 MIPI to LVDS
    return IT6122_Sys_ini();
}

void ithLcdDumpRegIT6122(void)
{
    IT6122_DumpReg();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//for IT6151 API
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ithLcdInitIT6151(void)
{
    //IT6151 MIPI to EDP
    IT6151_init();
}

void ithLcdDumpRegIT6151(void)
{
    IT6151_Debug();
}

void ithLcdInitCheckTaskIT6151(void)
{
    pthread_t       task;
    pthread_create(&task, NULL, IT6151_check_Task, NULL);
}
#endif //#ifndef _WIN32



