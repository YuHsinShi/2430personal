//=============================================================================
//=============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "iic/mmp_iic.h"
#include "NT99141.h"

//=============================================================================
//                Constant Definition
//=============================================================================
static uint8_t NOVATEK_IICADDR = 0x54 >> 1;
#ifdef CFG_SENSOR_ENABLE
static uint8_t NT_IIC_PORT     = CFG_SENSOR_IIC_PORT; /* please assign IIC PORT      */
#else
static uint8_t NT_IIC_PORT     = IIC_PORT_2;
#endif

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct _REGPAIR
{
    uint8_t  addr;
    uint16_t value;
} REGPAIR;

typedef struct NT99141SensorDriverStruct
{
    SensorDriverStruct base;
} NT99141SensorDriverStruct;

//=============================================================================
//                IIC API FUNCTION START
//=============================================================================
uint8_t NOVATEK_ReadI2C_8Bit(
    uint16_t RegAddr)
{
    static uint8_t dbuf[3]     = {0};
    static char    portname[4] = {0};
    uint8_t        *pdbuf      = dbuf;
    uint8_t        value       = 0;
    int            result      = 0;
    ITPI2cInfo     evt;
    int            gMasterDev  = 0;

    sprintf(portname, ":i2c%d", NT_IIC_PORT);
    gMasterDev         = open(portname, 0);

    *pdbuf++           = (uint8_t)((RegAddr & 0xff00) >> 8);
    *pdbuf++           = (uint8_t)(RegAddr & 0x00ff);

    evt.slaveAddress   = NOVATEK_IICADDR;      //對接裝置salve address
    evt.cmdBuffer      = dbuf;                 //欲傳送給slave裝置的data buffer
    evt.cmdBufferSize  = 2;                    //傳送data size,單位為byte
    evt.dataBuffer     = pdbuf;                //欲接收slave裝置的data buffer
    evt.dataBufferSize = 1;                    //接收data size,單位為byte

    result             = read(gMasterDev, &evt, 1);

    value              = (dbuf[2] & 0xFF);

    printf("0x%04x = 0x%02x %d\n", RegAddr, value, value);

    return value;
}

int NOVATEK_WriteI2C_8Bit(
    uint16_t RegAddr,
    uint8_t  data)
{
    static uint8_t dbuf[3]     = {0};
    static char    portname[4] = {0};
    uint8_t        *pdbuf      = dbuf;
    int            result      = 0;
    ITPI2cInfo     evt;
    int            gMasterDev  = 0;

    sprintf(portname, ":i2c%d", NT_IIC_PORT);
    gMasterDev        = open(portname, 0);

    *pdbuf++          = (uint8_t)((RegAddr & 0xff00) >> 8);
    *pdbuf++          = (uint8_t)(RegAddr & 0x00ff);
    *pdbuf            = (uint8_t)(data);

    evt.slaveAddress  = NOVATEK_IICADDR;       //對接裝置salve address
    evt.cmdBuffer     = dbuf;                  //欲傳送給slave裝置的data buffer
    evt.cmdBufferSize = 3;                     //傳送data size,單位為byte
    if (0 != (result = write(gMasterDev, &evt, 1)))
    {
        printf("NOVATEK_WriteI2C_8Bit I2c Write Error, reg=%04x val=%02x\n", RegAddr, data);
    }

    return result;
}

//=============================================================================
//                IIC API FUNCTION END
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================
static void _NOVATEK_NT99141_SMPTE_30fps_50Hz(
    void)
{
    //[YUYV_1280x720_30.00_30.04_Fps]
    //PCLK=74.25MHz,MCLK=27MHz,50Hz
    NOVATEK_WriteI2C_8Bit(0x320A, 0x00);
    NOVATEK_WriteI2C_8Bit(0x32BF, 0x60);
    NOVATEK_WriteI2C_8Bit(0x32C0, 0x5A);
    NOVATEK_WriteI2C_8Bit(0x32C1, 0x5A);
    NOVATEK_WriteI2C_8Bit(0x32C2, 0x5A);
    NOVATEK_WriteI2C_8Bit(0x32C3, 0x00);
    NOVATEK_WriteI2C_8Bit(0x32C4, 0x20);
    NOVATEK_WriteI2C_8Bit(0x32C5, 0x20);
    NOVATEK_WriteI2C_8Bit(0x32C6, 0x20);
    NOVATEK_WriteI2C_8Bit(0x32C7, 0x00);
    NOVATEK_WriteI2C_8Bit(0x32C8, 0xE0);
    NOVATEK_WriteI2C_8Bit(0x32C9, 0x5A);
    NOVATEK_WriteI2C_8Bit(0x32CA, 0x7A);
    NOVATEK_WriteI2C_8Bit(0x32CB, 0x7A);
    NOVATEK_WriteI2C_8Bit(0x32CC, 0x7A);
    NOVATEK_WriteI2C_8Bit(0x32CD, 0x7A);
    NOVATEK_WriteI2C_8Bit(0x32DB, 0x7B);
    NOVATEK_WriteI2C_8Bit(0x3200, 0x3E);
    NOVATEK_WriteI2C_8Bit(0x3201, 0x0F);
    NOVATEK_WriteI2C_8Bit(0x3028, 0x0A);
    NOVATEK_WriteI2C_8Bit(0x3029, 0x00);
    NOVATEK_WriteI2C_8Bit(0x302A, 0x04);
    NOVATEK_WriteI2C_8Bit(0x3022, 0x27);
    NOVATEK_WriteI2C_8Bit(0x3023, 0x24);
    NOVATEK_WriteI2C_8Bit(0x3002, 0x00);
    NOVATEK_WriteI2C_8Bit(0x3003, 0x04);
    NOVATEK_WriteI2C_8Bit(0x3004, 0x00);
    NOVATEK_WriteI2C_8Bit(0x3005, 0x04);
    NOVATEK_WriteI2C_8Bit(0x3006, 0x05);
    NOVATEK_WriteI2C_8Bit(0x3007, 0x03);
    NOVATEK_WriteI2C_8Bit(0x3008, 0x02);
    NOVATEK_WriteI2C_8Bit(0x3009, 0xD3);
    NOVATEK_WriteI2C_8Bit(0x300A, 0x06);
    NOVATEK_WriteI2C_8Bit(0x300B, 0x7C);
    NOVATEK_WriteI2C_8Bit(0x300C, 0x02);
    NOVATEK_WriteI2C_8Bit(0x300D, 0xE9);
    NOVATEK_WriteI2C_8Bit(0x300E, 0x05);
    NOVATEK_WriteI2C_8Bit(0x300F, 0x00);
    NOVATEK_WriteI2C_8Bit(0x3010, 0x02);
    NOVATEK_WriteI2C_8Bit(0x3011, 0xD0);
    NOVATEK_WriteI2C_8Bit(0x3201, 0x3F);
    NOVATEK_WriteI2C_8Bit(0x3021, 0x06);
    NOVATEK_WriteI2C_8Bit(0x3060, 0x01);
}

static void _NOVATEK_NT99141_SMPTE_25fps_50Hz(
    void)
{
    //[YUYV_1280x720_25.00_25.02_Fps]
    //PCLK=64.125MHz,MCLK=27MHz,50Hz
    //////////////////////////////////////// start up////////////////////////////////////////
    NOVATEK_WriteI2C_8Bit(0x320A, 0x00);
    NOVATEK_WriteI2C_8Bit(0x32BF, 0x60);
    NOVATEK_WriteI2C_8Bit(0x32C0, 0x60);
    NOVATEK_WriteI2C_8Bit(0x32C1, 0x60);
    NOVATEK_WriteI2C_8Bit(0x32C2, 0x60);
    NOVATEK_WriteI2C_8Bit(0x32C3, 0x00);
    NOVATEK_WriteI2C_8Bit(0x32C4, 0x24);
    NOVATEK_WriteI2C_8Bit(0x32C5, 0x20);
    NOVATEK_WriteI2C_8Bit(0x32C6, 0x20);
    NOVATEK_WriteI2C_8Bit(0x32C7, 0x00);
    NOVATEK_WriteI2C_8Bit(0x32C8, 0xC1);
    NOVATEK_WriteI2C_8Bit(0x32C9, 0x60);
    NOVATEK_WriteI2C_8Bit(0x32CA, 0x80);
    NOVATEK_WriteI2C_8Bit(0x32CB, 0x80);
    NOVATEK_WriteI2C_8Bit(0x32CC, 0x80);
    NOVATEK_WriteI2C_8Bit(0x32CD, 0x80);
    NOVATEK_WriteI2C_8Bit(0x32DB, 0x78);
    NOVATEK_WriteI2C_8Bit(0x3200, 0x3E);
    NOVATEK_WriteI2C_8Bit(0x3201, 0x0F);
    NOVATEK_WriteI2C_8Bit(0x3028, 0x12);
    NOVATEK_WriteI2C_8Bit(0x3029, 0x10);
    NOVATEK_WriteI2C_8Bit(0x302A, 0x04);
    NOVATEK_WriteI2C_8Bit(0x3022, 0x27);
    NOVATEK_WriteI2C_8Bit(0x3023, 0x24);
    NOVATEK_WriteI2C_8Bit(0x3002, 0x00);
    NOVATEK_WriteI2C_8Bit(0x3003, 0x04);
    NOVATEK_WriteI2C_8Bit(0x3004, 0x00);
    NOVATEK_WriteI2C_8Bit(0x3005, 0x04);
    NOVATEK_WriteI2C_8Bit(0x3006, 0x05);
    NOVATEK_WriteI2C_8Bit(0x3007, 0x03);
    NOVATEK_WriteI2C_8Bit(0x3008, 0x02);
    NOVATEK_WriteI2C_8Bit(0x3009, 0xD3);
    NOVATEK_WriteI2C_8Bit(0x300A, 0x06);
    NOVATEK_WriteI2C_8Bit(0x300B, 0x7C);
    NOVATEK_WriteI2C_8Bit(0x300C, 0x03);
    NOVATEK_WriteI2C_8Bit(0x300D, 0x04);
    NOVATEK_WriteI2C_8Bit(0x300E, 0x05);
    NOVATEK_WriteI2C_8Bit(0x300F, 0x00);
    NOVATEK_WriteI2C_8Bit(0x3010, 0x02);
    NOVATEK_WriteI2C_8Bit(0x3011, 0xD0);
    NOVATEK_WriteI2C_8Bit(0x3201, 0x3F);
    NOVATEK_WriteI2C_8Bit(0x3021, 0x06);
    NOVATEK_WriteI2C_8Bit(0x3060, 0x01);
}

static void _NOVATEK_NT99141_SMPTE_30fps_60Hz(
    void)
{
    //[YUYV_1280x720_30.00_30.04_Fps]
    //PCLK=74.25MHz,MCLK=27MHz,60Hz
    //////////////////////////////////////// start up////////////////////////////////////////
    NOVATEK_WriteI2C_8Bit(0x320A, 0x00);
    NOVATEK_WriteI2C_8Bit(0x32BF, 0x60);
    NOVATEK_WriteI2C_8Bit(0x32C0, 0x60);
    NOVATEK_WriteI2C_8Bit(0x32C1, 0x5F);
    NOVATEK_WriteI2C_8Bit(0x32C2, 0x5F);
    NOVATEK_WriteI2C_8Bit(0x32C3, 0x00);
    NOVATEK_WriteI2C_8Bit(0x32C4, 0x24);
    NOVATEK_WriteI2C_8Bit(0x32C5, 0x20);
    NOVATEK_WriteI2C_8Bit(0x32C6, 0x20);
    NOVATEK_WriteI2C_8Bit(0x32C7, 0x00);
    NOVATEK_WriteI2C_8Bit(0x32C8, 0xBA);
    NOVATEK_WriteI2C_8Bit(0x32C9, 0x5F);
    NOVATEK_WriteI2C_8Bit(0x32CA, 0x7F);
    NOVATEK_WriteI2C_8Bit(0x32CB, 0x7F);
    NOVATEK_WriteI2C_8Bit(0x32CC, 0x7F);
    NOVATEK_WriteI2C_8Bit(0x32CD, 0x80);
    NOVATEK_WriteI2C_8Bit(0x32DB, 0x77);
    NOVATEK_WriteI2C_8Bit(0x3200, 0x3E);
    NOVATEK_WriteI2C_8Bit(0x3201, 0x0F);
    NOVATEK_WriteI2C_8Bit(0x3028, 0x0A);
    NOVATEK_WriteI2C_8Bit(0x3029, 0x00);
    NOVATEK_WriteI2C_8Bit(0x302A, 0x04);
    NOVATEK_WriteI2C_8Bit(0x3022, 0x27);
    NOVATEK_WriteI2C_8Bit(0x3023, 0x24);
    NOVATEK_WriteI2C_8Bit(0x3002, 0x00);
    NOVATEK_WriteI2C_8Bit(0x3003, 0x04);
    NOVATEK_WriteI2C_8Bit(0x3004, 0x00);
    NOVATEK_WriteI2C_8Bit(0x3005, 0x04);
    NOVATEK_WriteI2C_8Bit(0x3006, 0x05);
    NOVATEK_WriteI2C_8Bit(0x3007, 0x03);
    NOVATEK_WriteI2C_8Bit(0x3008, 0x02);
    NOVATEK_WriteI2C_8Bit(0x3009, 0xD3);
    NOVATEK_WriteI2C_8Bit(0x300A, 0x06);
    NOVATEK_WriteI2C_8Bit(0x300B, 0x7C);
    NOVATEK_WriteI2C_8Bit(0x300C, 0x02);
    NOVATEK_WriteI2C_8Bit(0x300D, 0xE9);
    NOVATEK_WriteI2C_8Bit(0x300E, 0x05);
    NOVATEK_WriteI2C_8Bit(0x300F, 0x00);
    NOVATEK_WriteI2C_8Bit(0x3010, 0x02);
    NOVATEK_WriteI2C_8Bit(0x3011, 0xD0);
    NOVATEK_WriteI2C_8Bit(0x3201, 0x3F);
    NOVATEK_WriteI2C_8Bit(0x3021, 0x06);
    NOVATEK_WriteI2C_8Bit(0x3060, 0x01);
}

static void _NOVATEK_NT99141_SMPTE_25fps_60Hz(
    void)
{
    //[YUYV_1280x720_25.00_25.02_Fps]
    //PCLK=64.125MHz,MCLK=27MHz,60Hz
    //////////////////////////////////////// start up////////////////////////////////////////
    NOVATEK_WriteI2C_8Bit(0x320A, 0x00);
    NOVATEK_WriteI2C_8Bit(0x32BF, 0x60);
    NOVATEK_WriteI2C_8Bit(0x32C0, 0x63);
    NOVATEK_WriteI2C_8Bit(0x32C1, 0x63);
    NOVATEK_WriteI2C_8Bit(0x32C2, 0x63);
    NOVATEK_WriteI2C_8Bit(0x32C3, 0x00);
    NOVATEK_WriteI2C_8Bit(0x32C4, 0x24);
    NOVATEK_WriteI2C_8Bit(0x32C5, 0x20);
    NOVATEK_WriteI2C_8Bit(0x32C6, 0x20);
    NOVATEK_WriteI2C_8Bit(0x32C7, 0x00);
    NOVATEK_WriteI2C_8Bit(0x32C8, 0xA1);
    NOVATEK_WriteI2C_8Bit(0x32C9, 0x63);
    NOVATEK_WriteI2C_8Bit(0x32CA, 0x83);
    NOVATEK_WriteI2C_8Bit(0x32CB, 0x83);
    NOVATEK_WriteI2C_8Bit(0x32CC, 0x83);
    NOVATEK_WriteI2C_8Bit(0x32CD, 0x83);
    NOVATEK_WriteI2C_8Bit(0x32DB, 0x74);
    NOVATEK_WriteI2C_8Bit(0x3200, 0x3E);
    NOVATEK_WriteI2C_8Bit(0x3201, 0x0F);
    NOVATEK_WriteI2C_8Bit(0x3028, 0x12);
    NOVATEK_WriteI2C_8Bit(0x3029, 0x10);
    NOVATEK_WriteI2C_8Bit(0x302A, 0x04);
    NOVATEK_WriteI2C_8Bit(0x3022, 0x27);
    NOVATEK_WriteI2C_8Bit(0x3023, 0x24);
    NOVATEK_WriteI2C_8Bit(0x3002, 0x00);
    NOVATEK_WriteI2C_8Bit(0x3003, 0x04);
    NOVATEK_WriteI2C_8Bit(0x3004, 0x00);
    NOVATEK_WriteI2C_8Bit(0x3005, 0x04);
    NOVATEK_WriteI2C_8Bit(0x3006, 0x05);
    NOVATEK_WriteI2C_8Bit(0x3007, 0x03);
    NOVATEK_WriteI2C_8Bit(0x3008, 0x02);
    NOVATEK_WriteI2C_8Bit(0x3009, 0xD3);
    NOVATEK_WriteI2C_8Bit(0x300A, 0x06);
    NOVATEK_WriteI2C_8Bit(0x300B, 0x7C);
    NOVATEK_WriteI2C_8Bit(0x300C, 0x03);
    NOVATEK_WriteI2C_8Bit(0x300D, 0x04);
    NOVATEK_WriteI2C_8Bit(0x300E, 0x05);
    NOVATEK_WriteI2C_8Bit(0x300F, 0x00);
    NOVATEK_WriteI2C_8Bit(0x3010, 0x02);
    NOVATEK_WriteI2C_8Bit(0x3011, 0xD0);
    NOVATEK_WriteI2C_8Bit(0x3201, 0x3F);
    NOVATEK_WriteI2C_8Bit(0x3021, 0x06);
    NOVATEK_WriteI2C_8Bit(0x3060, 0x01);
}

static void _NOVATEK_NT99141_Internal_Rom_Setting(
    void)
{
    NOVATEK_WriteI2C_8Bit(0x3069, 0x02);
    NOVATEK_WriteI2C_8Bit(0x306A, 0x03);
    NOVATEK_WriteI2C_8Bit(0x320A, 0x00);
    NOVATEK_WriteI2C_8Bit(0x3109, 0x84);
    NOVATEK_WriteI2C_8Bit(0x3040, 0x04);
    NOVATEK_WriteI2C_8Bit(0x3041, 0x02);
    NOVATEK_WriteI2C_8Bit(0x3055, 0x40);
    NOVATEK_WriteI2C_8Bit(0x3054, 0x00);
    NOVATEK_WriteI2C_8Bit(0x3042, 0xFF);
    NOVATEK_WriteI2C_8Bit(0x3043, 0x08);
    NOVATEK_WriteI2C_8Bit(0x3052, 0xE0);
    NOVATEK_WriteI2C_8Bit(0x305F, 0x11);
    NOVATEK_WriteI2C_8Bit(0x3100, 0x07);
    NOVATEK_WriteI2C_8Bit(0x3106, 0x03);
    NOVATEK_WriteI2C_8Bit(0x3105, 0x01);
    NOVATEK_WriteI2C_8Bit(0x3108, 0x05);
    NOVATEK_WriteI2C_8Bit(0x3110, 0x22);
    NOVATEK_WriteI2C_8Bit(0x3111, 0x57);
    NOVATEK_WriteI2C_8Bit(0x3112, 0x22);
    NOVATEK_WriteI2C_8Bit(0x3113, 0x55);
    NOVATEK_WriteI2C_8Bit(0x3114, 0x05);
    NOVATEK_WriteI2C_8Bit(0x3135, 0x00);
    NOVATEK_WriteI2C_8Bit(0x32F0, 0x00);
    NOVATEK_WriteI2C_8Bit(0x3290, 0x01);
    NOVATEK_WriteI2C_8Bit(0x3291, 0x38);
    NOVATEK_WriteI2C_8Bit(0x3296, 0x01);
    NOVATEK_WriteI2C_8Bit(0x3297, 0x68);
    NOVATEK_WriteI2C_8Bit(0x3012, 0x02);
    NOVATEK_WriteI2C_8Bit(0x3013, 0xD0);
    NOVATEK_WriteI2C_8Bit(0x3250, 0xC0);
    NOVATEK_WriteI2C_8Bit(0x3251, 0x00);
    NOVATEK_WriteI2C_8Bit(0x3252, 0xDF);
    NOVATEK_WriteI2C_8Bit(0x3253, 0x95);
    NOVATEK_WriteI2C_8Bit(0x3254, 0x00);
    NOVATEK_WriteI2C_8Bit(0x3255, 0xCB);
    NOVATEK_WriteI2C_8Bit(0x3256, 0x8A);
    NOVATEK_WriteI2C_8Bit(0x3257, 0x38);
    NOVATEK_WriteI2C_8Bit(0x3258, 0x0A);
    NOVATEK_WriteI2C_8Bit(0x329B, 0x01);
    NOVATEK_WriteI2C_8Bit(0x32A1, 0x00);
    NOVATEK_WriteI2C_8Bit(0x32A2, 0xA0);
    NOVATEK_WriteI2C_8Bit(0x32A3, 0x01);
    NOVATEK_WriteI2C_8Bit(0x32A4, 0xC8);
    NOVATEK_WriteI2C_8Bit(0x32A5, 0x01);
    NOVATEK_WriteI2C_8Bit(0x32A6, 0x28);
    NOVATEK_WriteI2C_8Bit(0x32A7, 0x01);
    NOVATEK_WriteI2C_8Bit(0x32A8, 0xFC);
    NOVATEK_WriteI2C_8Bit(0x32A9, 0x11);
    NOVATEK_WriteI2C_8Bit(0x32B0, 0x55);
    NOVATEK_WriteI2C_8Bit(0x32B1, 0x7D);
    NOVATEK_WriteI2C_8Bit(0x32B2, 0x7D);
    NOVATEK_WriteI2C_8Bit(0x32B3, 0x55);
    NOVATEK_WriteI2C_8Bit(0x3210, 0x11);
    NOVATEK_WriteI2C_8Bit(0x3211, 0x11);
    NOVATEK_WriteI2C_8Bit(0x3212, 0x11);
    NOVATEK_WriteI2C_8Bit(0x3213, 0x11);
    NOVATEK_WriteI2C_8Bit(0x3214, 0x10);
    NOVATEK_WriteI2C_8Bit(0x3215, 0x10);
    NOVATEK_WriteI2C_8Bit(0x3216, 0x10);
    NOVATEK_WriteI2C_8Bit(0x3217, 0x10);
    NOVATEK_WriteI2C_8Bit(0x3218, 0x10);
    NOVATEK_WriteI2C_8Bit(0x3219, 0x10);
    NOVATEK_WriteI2C_8Bit(0x321A, 0x10);
    NOVATEK_WriteI2C_8Bit(0x321B, 0x10);
    NOVATEK_WriteI2C_8Bit(0x321C, 0x0f);
    NOVATEK_WriteI2C_8Bit(0x321D, 0x0f);
    NOVATEK_WriteI2C_8Bit(0x321E, 0x0f);
    NOVATEK_WriteI2C_8Bit(0x321F, 0x0f);
    NOVATEK_WriteI2C_8Bit(0x3231, 0x68);
    NOVATEK_WriteI2C_8Bit(0x3232, 0xC4);
    NOVATEK_WriteI2C_8Bit(0x3270, 0x00);
    NOVATEK_WriteI2C_8Bit(0x3271, 0x0C);
    NOVATEK_WriteI2C_8Bit(0x3272, 0x18);
    NOVATEK_WriteI2C_8Bit(0x3273, 0x32);
    NOVATEK_WriteI2C_8Bit(0x3274, 0x44);
    NOVATEK_WriteI2C_8Bit(0x3275, 0x54);
    NOVATEK_WriteI2C_8Bit(0x3276, 0x70);
    NOVATEK_WriteI2C_8Bit(0x3277, 0x88);
    NOVATEK_WriteI2C_8Bit(0x3278, 0x9D);
    NOVATEK_WriteI2C_8Bit(0x3279, 0xB0);
    NOVATEK_WriteI2C_8Bit(0x327A, 0xCF);
    NOVATEK_WriteI2C_8Bit(0x327B, 0xE2);
    NOVATEK_WriteI2C_8Bit(0x327C, 0xEF);
    NOVATEK_WriteI2C_8Bit(0x327D, 0xF7);
    NOVATEK_WriteI2C_8Bit(0x327E, 0xFF);
    NOVATEK_WriteI2C_8Bit(0x3302, 0x00);
    NOVATEK_WriteI2C_8Bit(0x3303, 0x39);
    NOVATEK_WriteI2C_8Bit(0x3304, 0x00);
    NOVATEK_WriteI2C_8Bit(0x3305, 0x9E);
    NOVATEK_WriteI2C_8Bit(0x3306, 0x00);
    NOVATEK_WriteI2C_8Bit(0x3307, 0x28);
    NOVATEK_WriteI2C_8Bit(0x3308, 0x07);
    NOVATEK_WriteI2C_8Bit(0x3309, 0xBC);
    NOVATEK_WriteI2C_8Bit(0x330A, 0x06);
    NOVATEK_WriteI2C_8Bit(0x330B, 0xE4);
    NOVATEK_WriteI2C_8Bit(0x330C, 0x01);
    NOVATEK_WriteI2C_8Bit(0x330D, 0x60);
    NOVATEK_WriteI2C_8Bit(0x330E, 0x01);
    NOVATEK_WriteI2C_8Bit(0x330F, 0x44);
    NOVATEK_WriteI2C_8Bit(0x3310, 0x07);
    NOVATEK_WriteI2C_8Bit(0x3311, 0x06);
    NOVATEK_WriteI2C_8Bit(0x3312, 0x07);
    NOVATEK_WriteI2C_8Bit(0x3313, 0xB7);
    NOVATEK_WriteI2C_8Bit(0x3326, 0x03);
    NOVATEK_WriteI2C_8Bit(0x3327, 0x0A);
    NOVATEK_WriteI2C_8Bit(0x3328, 0x0A);
    NOVATEK_WriteI2C_8Bit(0x3329, 0x06);
    NOVATEK_WriteI2C_8Bit(0x332A, 0x06);
    NOVATEK_WriteI2C_8Bit(0x332B, 0x1C);
    NOVATEK_WriteI2C_8Bit(0x332C, 0x1C);
    NOVATEK_WriteI2C_8Bit(0x332D, 0x00);
    NOVATEK_WriteI2C_8Bit(0x332E, 0x1D);
    NOVATEK_WriteI2C_8Bit(0x332F, 0x1F);
    NOVATEK_WriteI2C_8Bit(0x32F6, 0xCF);
    NOVATEK_WriteI2C_8Bit(0x32F9, 0x21);
    NOVATEK_WriteI2C_8Bit(0x32FA, 0x12);
    NOVATEK_WriteI2C_8Bit(0x3325, 0x5F);
    NOVATEK_WriteI2C_8Bit(0x3330, 0x00);
    NOVATEK_WriteI2C_8Bit(0x3331, 0x08);
    NOVATEK_WriteI2C_8Bit(0x3332, 0xdc);
    NOVATEK_WriteI2C_8Bit(0x3338, 0x08);
    NOVATEK_WriteI2C_8Bit(0x3339, 0x63);
    NOVATEK_WriteI2C_8Bit(0x333A, 0x36);
    NOVATEK_WriteI2C_8Bit(0x333F, 0x07);
    NOVATEK_WriteI2C_8Bit(0x3360, 0x0A);
    NOVATEK_WriteI2C_8Bit(0x3361, 0x14);
    NOVATEK_WriteI2C_8Bit(0x3362, 0x1F);
    NOVATEK_WriteI2C_8Bit(0x3363, 0x37);
    NOVATEK_WriteI2C_8Bit(0x3364, 0x98);
    NOVATEK_WriteI2C_8Bit(0x3365, 0x88);
    NOVATEK_WriteI2C_8Bit(0x3366, 0x78);
    NOVATEK_WriteI2C_8Bit(0x3367, 0x60);
    NOVATEK_WriteI2C_8Bit(0x3368, 0xB8);   //0x90
    NOVATEK_WriteI2C_8Bit(0x3369, 0xA0);   //0x70
    NOVATEK_WriteI2C_8Bit(0x336A, 0x88);   //0x50
    NOVATEK_WriteI2C_8Bit(0x336B, 0x70);   //0x40
    NOVATEK_WriteI2C_8Bit(0x336C, 0x00);
    NOVATEK_WriteI2C_8Bit(0x336D, 0x20);
    NOVATEK_WriteI2C_8Bit(0x336E, 0x1C);
    NOVATEK_WriteI2C_8Bit(0x336F, 0x18);
    NOVATEK_WriteI2C_8Bit(0x3370, 0x10);
    NOVATEK_WriteI2C_8Bit(0x3371, 0x38);
    NOVATEK_WriteI2C_8Bit(0x3372, 0x3C);
    NOVATEK_WriteI2C_8Bit(0x3373, 0x3F);
    NOVATEK_WriteI2C_8Bit(0x3374, 0x3F);
    NOVATEK_WriteI2C_8Bit(0x3375, 0x12);   //0x0A
    NOVATEK_WriteI2C_8Bit(0x3376, 0x16);   //0x0C
    NOVATEK_WriteI2C_8Bit(0x3377, 0x26);   //0x10
    NOVATEK_WriteI2C_8Bit(0x3378, 0x2C);   //0x14
    NOVATEK_WriteI2C_8Bit(0x338A, 0x34);
    NOVATEK_WriteI2C_8Bit(0x338B, 0x7F);
    NOVATEK_WriteI2C_8Bit(0x338C, 0x10);
    NOVATEK_WriteI2C_8Bit(0x338D, 0x23);
    NOVATEK_WriteI2C_8Bit(0x338E, 0x7F);
    NOVATEK_WriteI2C_8Bit(0x338F, 0x14);
    NOVATEK_WriteI2C_8Bit(0x32C4, 0x22);
    NOVATEK_WriteI2C_8Bit(0x3053, 0x4F);
    NOVATEK_WriteI2C_8Bit(0x32F2, 0x80);
    NOVATEK_WriteI2C_8Bit(0x32FC, 0x00);
    NOVATEK_WriteI2C_8Bit(0x32B8, 0x3B);
    NOVATEK_WriteI2C_8Bit(0x32B9, 0x2D);
    NOVATEK_WriteI2C_8Bit(0x32BB, 0x87);
    NOVATEK_WriteI2C_8Bit(0x32BC, 0x34);
    NOVATEK_WriteI2C_8Bit(0x32BD, 0x38);
    NOVATEK_WriteI2C_8Bit(0x32BE, 0x30);
}

void ithNovaTekSetAntiFlicker60Hz(
    void)
{
    printf("set flick 60Hz\n");

#ifdef CFG_SENSOR_RESETPIN_ENABLE
    //Set GPIO29 to Low
    ithGpioClear(CFG_SN1_GPIO_RST);
    //Set GPIO29 Output Mode
    ithGpioSetOut(CFG_SN1_GPIO_RST);
    //Set GPIO29 Mode0
    ithGpioSetMode(CFG_SN1_GPIO_RST, ITH_GPIO_MODE0);
    usleep(1000);
    ithGpioSet(CFG_SN1_GPIO_RST);
    usleep(1000);
#endif
    _NOVATEK_NT99141_Internal_Rom_Setting();
    _NOVATEK_NT99141_SMPTE_30fps_60Hz();
}

void ithNovaTekSetAntiFlicker50Hz(
    void)
{
    printf("set flick 50Hz\n");
#ifdef CFG_SENSOR_RESETPIN_ENABLE
    //Set to Low for reset
    ithGpioClear(CFG_SN1_GPIO_RST);
    //Set Output Mode
    ithGpioSetOut(CFG_SN1_GPIO_RST);
    //Set Mode0
    ithGpioSetMode(CFG_SN1_GPIO_RST, ITH_GPIO_MODE0);
    usleep(1000);
    ithGpioSet(CFG_SN1_GPIO_RST);
    usleep(1000);
#endif
    _NOVATEK_NT99141_Internal_Rom_Setting();
    _NOVATEK_NT99141_SMPTE_25fps_50Hz();
};

void ithNovaTekLedOn(
    void)
{
#ifdef CFG_SENSOR_LEDPIN_ENABLE
    //LED_EN
    //Set to High
    ithGpioSet(CFG_SN1_GPIO_LED);
    //Set Output Mode
    ithGpioSetOut(CFG_SN1_GPIO_LED);
    //Set Mode0
    ithGpioSetMode(CFG_SN1_GPIO_LED, ITH_GPIO_MODE0);
#endif
}

void ithNovaTekLedOff(
    void)
{
#ifdef CFG_SENSOR_LEDPIN_ENABLE
    ithGpioClear(CFG_SN1_GPIO_LED);
#endif
}

//=============================================================================
//                NT99141 IMPLEMENT FUNCTION START
//=============================================================================
////////////////////////////////////////////////////////////////////////////////////////////////////////

void NT99141Initialize(uint16_t Mode)
{
#if defined(CFG_SENSOR_FLICK_60HZ)
    ithNovaTekSetAntiFlicker60Hz();
#elif defined(CFG_SENSOR_FLICK_50HZ)
    ithNovaTekSetAntiFlicker50Hz();
#endif
}

void NT99141Terminate(void)
{
    /* Please implement terminate code here */
}

void NT99141OutputPinTriState(uint8_t flag)
{
    if (flag == true)
    {
        /* Please implement outputpintristate code here */
    }
    else
    {
        /* Please implement disable outputpintristate code here */
    }
}

uint8_t NT99141IsSignalStable(uint16_t Mode)
{
    bool isStable = false;
    /* Please implement checking signal stable code here */
    /* stable return true else return false                       */
    isStable = true;
    return isStable;
}

uint16_t NT99141GetProperty(MODULE_GETPROPERTY property)
{
    /* Please implement get information from device code here */
    switch (property)
    {
    case GetHeight:
        return 720;
        break;
    case GetWidth:
        return 1280;
        break;
    //frame rate
    case Rate:
#if defined(CFG_SENSOR_FLICK_60HZ)
        return 3000;
#else
        return 2500;
#endif
        break;
    case GetModuleIsInterlace:
        return 0;
        break;
    case matchResolution:
        return true;
        break;
    default:
        return 0;
        break;
    }
}

uint8_t NT99141GetStatus(MODULE_GETSTATUS Status)
{
    /* Please implement get status from device code here */
    switch (Status)
    {
    default:
        return 0;
        break;
    }
}

void NT99141SetProperty(MODULE_SETPROPERTY property, uint16_t Value)
{
    /* Please implement set property to device code here */
    switch (property)
    {
    case LEDON:
        ithNovaTekLedOn();
        break;
    case LEDOFF:
        ithNovaTekLedOff();
        break;
    default:
        break;
    }
}

void NT99141PowerDown(uint8_t enable)
{
#ifdef CFG_SENSOR_POWERDOWNPIN_ENABLE

    /* Please implement power down code here */
    if (enable)
    {
        //Power off
        ithGpioSet(CFG_SN1_GPIO_PWN);
        usleep(10000);
        //Set GPIO30 to Low after power down
        ithGpioClear(CFG_SN1_GPIO_PWN);
    }
    else
    {
    #ifdef CFG_SENSOR_MCLKPIN_ENABLE
        ithGpioSetOut(CFG_SN1_GPIO_MCLK);
        ithGpioSetMode(CFG_SN1_GPIO_MCLK, ITH_GPIO_MODE2);
    #endif
        //Power ON
        //Set GPIO30 to Low
        ithGpioClear(CFG_SN1_GPIO_PWN);
        //Set GPIO30 Output Mode
        ithGpioSetOut(CFG_SN1_GPIO_PWN);
        //Set GPIO30 Mode0
        ithGpioSetMode(CFG_SN1_GPIO_PWN, ITH_GPIO_MODE0);
    }
#endif
}

//=============================================================================
//                NT99141 IMPLEMENT FUNCTION END
//=============================================================================
static void NT99141SensorDriver_Destory(SensorDriver base)
{
    NT99141SensorDriver self = (NT99141SensorDriver)base;
    if (self)
        free(self);
}

/* assign callback funciton */
static SensorDriverInterfaceStruct interface =
{
    NT99141Initialize,
    NT99141Terminate,
    NT99141OutputPinTriState,
    NT99141IsSignalStable,
    NT99141GetProperty,
    NT99141GetStatus,
    NT99141SetProperty,
    NT99141PowerDown,
    NT99141SensorDriver_Destory
};

SensorDriver NT99141SensorDriver_Create()
{
    NT99141SensorDriver self = calloc(1, sizeof(NT99141SensorDriverStruct));
    self->base.vtable = &interface;
    self->base.type   = "NT99141";
    return (SensorDriver)self;
}

//end of X10LightDriver_t1.c