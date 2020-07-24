//=============================================================================
//=============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "iic/mmp_iic.h"
#include "userdefine.h"

//=============================================================================
//                Constant Definition
//=============================================================================
static uint8_t IICADDR = 0x00 >> 1;           /* please assign IIC ADDRESS */
#ifdef CFG_SENSOR_ENABLE
static uint8_t IICPORT = CFG_SENSOR_IIC_PORT; /* please assign IIC PORT      */
#else
static uint8_t IICPORT = IIC_PORT_2;
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

typedef struct UserSensorDriverStruct
{
    SensorDriverStruct base;
} UserSensorDriverStruct;

//=============================================================================
//                IIC API FUNCTION START
//=============================================================================
uint8_t ReadI2c_Byte(uint8_t RegAddr)
{
    static uint8_t dbuf[2]     = {0};
    static char    portname[4] = {0};
    int            result      = 0;
    uint8_t        *pdbuf      = dbuf;
    ITPI2cInfo     evt;
    int            gMasterDev  = 0;

    sprintf(portname, ":i2c%d", IICPORT);
    gMasterDev         = open(portname, 0);

    dbuf[0]            = (uint8_t)(RegAddr);
    pdbuf++;

    evt.slaveAddress   = IICADDR;
    evt.cmdBuffer      = dbuf;
    evt.cmdBufferSize  = 1;
    evt.dataBuffer     = pdbuf;
    evt.dataBufferSize = 1;

    result             = read(gMasterDev, &evt, 1);
    if (result != 0)
    {
        ithPrintf("ReadI2c_Byte read address 0x%02x error!\n", RegAddr);
    }
    //printf("Reg = %x, value[0] = %x ,value[1] = %x\n",RegAddr, dbuf[0],dbuf[1]);
    return dbuf[1];
}

uint32_t WriteI2c_Byte(uint8_t RegAddr, uint8_t data)
{
    static uint8_t dbuf[2]     = {0};
    static char    portname[4] = {0};
    int            result      = 0;
    uint8_t        *pdbuf      = dbuf;
    ITPI2cInfo     evt;
    int            gMasterDev  = 0;

    sprintf(portname, ":i2c%d", IICPORT);
    gMasterDev        = open(portname, 0);

    *pdbuf++          = (uint8_t)(RegAddr & 0xff);
    *pdbuf            = (uint8_t)(data);

    evt.slaveAddress  = IICADDR;     //對接裝置salve address
    evt.cmdBuffer     = dbuf;        //欲傳送給slave data buffer
    evt.cmdBufferSize = 2;           //傳送data size,單位為byte

    if (0 != (result = write(gMasterDev, &evt, 1)))
    {
        printf("WriteI2c_Byte Write Error, reg=%02x val=%02x\n", RegAddr, data);
    }
    return result;
}

uint32_t WriteI2c_ByteMask(uint8_t RegAddr, uint8_t data, uint8_t mask)
{
    uint8_t  value;
    uint32_t flag;

    value = ReadI2c_Byte(RegAddr);
    value = ((value & ~mask) | (data & mask));
    flag  = WriteI2c_Byte(RegAddr, value);

    return flag;
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

//=============================================================================
//                USER IMPLEMENT FUNCTION START
//=============================================================================
////////////////////////////////////////////////////////////////////////////////////////////////////////

//X10LightDriver_t1.c
void UserInitialize(uint16_t Mode)
{
    /* Please implement initial code here */
}

void UserTerminate(void)
{
    /* Please implement terminate code here */
}

void UserOutputPinTriState(uint8_t flag)
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

uint8_t UserIsSignalStable(uint16_t Mode)
{
    bool isStable = false;
    /* Please implement checking signal stable code here */
    /* stable return true else return false                       */
    return isStable;
}

uint16_t UserGetProperty(MODULE_GETPROPERTY property)
{
    /* Please implement get information from device code here */
    switch (property)
    {
    case GetTopFieldPolarity:
    case GetHeight:
    case GetWidth:
    //frame rate
    case Rate:
    case GetModuleIsInterlace:
    case matchResolution:
    default:
        return 0;
        break;
    }
}

uint8_t UserGetStatus(MODULE_GETSTATUS Status)
{
    /* Please implement get status from device code here */
    switch (Status)
    {
    case IsPowerDown:
    case IsSVideoInput:
    default:
        return 0;
        break;
    }
}

void UserSetProperty(MODULE_SETPROPERTY Property, uint16_t Value)
{
    /* Please implement set property to device code here */
}

void UserPowerDown(uint8_t enable)
{
    /* Please implement power down code here */
}

//=============================================================================
//                USER IMPLEMENT FUNCTION END
//=============================================================================
static void UserSensorDriver_Destory(SensorDriver base)
{
    SensorDriver self = (SensorDriver)base;
    if (self)
        free(self);
}

/* assign callback funciton */
static SensorDriverInterfaceStruct interface =
{
    UserInitialize,
    UserTerminate,
    UserOutputPinTriState,
    UserIsSignalStable,
    UserGetProperty,
    UserGetStatus,
    UserSetProperty,
    UserPowerDown,
    UserSensorDriver_Destory
};

SensorDriver UserSensorDriver_Create()
{
    UserSensorDriver self = calloc(1, sizeof(UserSensorDriverStruct));
    self->base.vtable = &interface;
    self->base.type   = "userdefine";
    return (SensorDriver)self;
}

//end of X10LightDriver_t1.c