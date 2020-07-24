#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "iic/mmp_iic.h"
#include "TVP5150AM1.h"
#ifdef CFG_SENSOR_ENABLE
    #define TVP5150_IIC_PORT CFG_SENSOR_IIC_PORT
#else
    #define TVP5150_IIC_PORT IIC_PORT_2
#endif
static uint8_t TVP5150_IICWRADDR = 0xBA >> 1;
static uint8_t TVP5150_IICREADDR = 0xBB >> 1;
typedef struct SensorDriverTVP5150Struct
{
    SensorDriverStruct base;
} SensorDriverTVP5150Struct;

//=============================================================================
//                IIC API FUNCTION START
//=============================================================================
uint8_t _TVP5150_ReadI2c_Byte(uint8_t RegAddr)
{
    static uint8_t dbuf[2]     = {0};
    static char    portname[4] = {0};
    int            result      = 0;
    uint8_t        *pdbuf      = dbuf;
    ITPI2cInfo     evt;
    int            gMasterDev  = 0;

    sprintf(portname, ":i2c%d", TVP5150_IIC_PORT);
    gMasterDev         = open(portname, 0);

    dbuf[0]            = (uint8_t)(RegAddr);
    pdbuf++;

    evt.slaveAddress   = TVP5150_IICREADDR;
    evt.cmdBuffer      = dbuf;
    evt.cmdBufferSize  = 1;
    evt.dataBuffer     = pdbuf;
    evt.dataBufferSize = 1;

    result             = read(gMasterDev, &evt, 1);
    if (result != 0)
    {
        ithPrintf("_TVP5150_ReadI2c_Byte read address 0x%02x error!\n", RegAddr);
    }
    //printf("Reg = %x, value[0] = %x ,value[1] = %x\n",RegAddr, dbuf[0],dbuf[1]);
    return dbuf[1];
}

uint32_t _TVP5150_WriteI2c_Byte(uint8_t RegAddr, uint8_t data)
{
    static uint8_t dbuf[2]     = {0};
    static char    portname[4] = {0};
    int            result      = 0;
    uint8_t        *pdbuf      = dbuf;
    ITPI2cInfo     evt;
    int            gMasterDev  = 0;

    sprintf(portname, ":i2c%d", TVP5150_IIC_PORT);
    gMasterDev        = open(portname, 0);

    *pdbuf++          = (uint8_t)(RegAddr & 0xff);
    *pdbuf            = (uint8_t)(data);

    evt.slaveAddress  = TVP5150_IICWRADDR;   //對接裝置salve address
    evt.cmdBuffer     = dbuf;                //欲傳送給slave data buffer
    evt.cmdBufferSize = 2;                   //傳送data size,單位為byte

    if (0 != (result = write(gMasterDev, &evt, 1)))
    {
        printf("_TVP5150_WriteI2c_Byte Write Error, reg=%02x val=%02x\n", RegAddr, data);
    }
    return result;
}

void TVP5150Initialize(uint16_t Mode)
{
    //ithPrintf("TVP5150Initialize\n");
#ifdef CFG_SENSOR_RESETPIN_ENABLE
    usleep(1000 * 20);
    ithGpioSetMode(CFG_SN1_GPIO_RST, ITH_GPIO_MODE0);
    ithGpioSetOut(CFG_SN1_GPIO_RST);
    ithGpioClear(CFG_SN1_GPIO_RST);
    usleep(1000);
    ithGpioSet(CFG_SN1_GPIO_RST);
    usleep(1000);
    _TVP5150_WriteI2c_Byte(0x00, 0x00);
    _TVP5150_WriteI2c_Byte(0x03, 0x69);
#endif
}

void TVP5150Terminate(void)
{
    //ithPrintf("TVP5150Terminate\n");
    _TVP5150_WriteI2c_Byte(0x03, 0x01);
}

void TVP5150OutputPinTriState(uint8_t flag)
{}

uint8_t TVP5150IsSignalStable(uint16_t Mode)
{
    //printf("0x03 = %x \n",_TVP5150_ReadI2c_Byte(0x03));
    if (_TVP5150_ReadI2c_Byte(0x88) & 0x6)
        return MMP_TRUE;
    else
        return MMP_FALSE;
}

uint16_t TVP5150GetProperty(MODULE_GETPROPERTY property)
{
    switch (property)
    {
    case GetWidth:
        return 720;
        break;
    case GetHeight:
        return 576;
    case GetModuleIsInterlace:
        return 1;
        break;
    case Rate:
        return 5000;
        break;
    default:
        ithPrintf("error property id =%d\n", property);
        return 0;
        break;
    }
}

uint8_t TVP5150GetStatus(MODULE_GETSTATUS Status)
{
    switch (Status)
    {
    default:
        ithPrintf("error status id =%d \n", Status);
        return 0;
        break;
    }
}

void TVP5150SetProperty(MODULE_SETPROPERTY Property, uint16_t Value)
{}

void TVP5150PowerDown(uint8_t enable)
{}

static void TVP5150SensorDriver_Destory(SensorDriver base)
{
    TVP5150SensorDriver self = (TVP5150SensorDriver)base;
    if (self)
        free(self);
}

static SensorDriverInterfaceStruct interface =
{
    TVP5150Initialize,
    TVP5150Terminate,
    TVP5150OutputPinTriState,
    TVP5150IsSignalStable,
    TVP5150GetProperty,
    TVP5150GetStatus,
    TVP5150SetProperty,
    TVP5150PowerDown,
    TVP5150SensorDriver_Destory
};

SensorDriver TVP5150SensorDriver_Create()
{
    TVP5150SensorDriver self = calloc(1, sizeof(SensorDriverTVP5150Struct));
    self->base.vtable = &interface;
    self->base.type   = "TVP5150";
    return (SensorDriver)self;
}