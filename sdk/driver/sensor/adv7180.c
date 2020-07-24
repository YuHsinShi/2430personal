#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "iic/mmp_iic.h"
#include "adv7180.h"

//=============================================================================
//                Constant Definition
//=============================================================================
//static uint8_t ADV7180_IICADDR = 0x42 >> 1;
static uint8_t ADV7180_IICADDR = 0x40 >> 1;
#ifdef CFG_SENSOR_ENABLE
    #define ADV7180_IIC_PORT CFG_SENSOR_IIC_PORT
#else
    #define ADV7180_IIC_PORT IIC_PORT_2
#endif

#define POWER_MANAGEMENT     0x0F
#define REG_STATUS1          0x10
#define REG_IDENT            0x11
#define REG_STATUS2          0x12
#define REG_STATUS3          0x13

#define RESET_MASK           (1 << 7)
#define TRI_STATE_ENABLE     (1 << 7)

//#define AUTO_DETECT_INPUT
//#define BT601_ENABLE 1
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

typedef struct ADV7180SensorDriverStruct
{
    SensorDriverStruct base;
} ADV7180SensorDriverStruct;

//=============================================================================
//                Global Data Definition
//=============================================================================
static uint16_t gtADV7180CurMode   = 0xFF;
static uint16_t gtADV7180PreMode   = 0xFF;
static uint16_t gtADV7180CurDev    = 0xFF;
static bool     gtADV7180InitDone  = MMP_FALSE;
static bool     gtADV7180PowerDown = MMP_FALSE;
static uint16_t matchRes           = 0;

/* 32-Lead LFCSP , page 108*/
static REGPAIR  CVBS_INPUT[]       =
{
    {0x00, 0x04}, //AIN3
    {0x04, 0x54},
    {0x17, 0x41},
    {0x31, 0x02},
    {0x3D, 0xA2},
    {0x3E, 0x6A},
    {0x3F, 0xA0},
    {0x58, 0x01},
    {0x0E, 0x80},
    {0x55, 0x81},
    {0x0E, 0x00},

    //Autodetect enable PAL_B/NTSC/N443
    {0x07, 0x23}, // Benson

#if BT601_ENABLE  // BT601
    //    // Figure35 and Figure 40 for BT601 NTSC and PAL
    {0x31, 0x1A},
    {0x32, 0x81},
    {0x33, 0x84},
    {0x34, 0x00},
    {0x35, 0x00},
    {0x36, 0x7D},
    {0x37, 0xA1},
    //NTSC
    {0xE5, 0x41},
    {0xE6, 0x84},
    {0xE7, 0x06},
    //PAL
    {0xE8, 0x41},
    {0xE9, 0x84},
    {0xEA, 0x06},
#endif
};

static REGPAIR  SVIDEO_INPUT [] =
{
    {0x00, 0x06}, //AIN1 AIN2
    {0x04, 0x54},
    {0x31, 0x02},
    {0x3D, 0xA2},
    {0x3E, 0x6A},
    {0x3F, 0xA0},
    {0x58, 0x05},
    {0x0E, 0x80},
    {0x55, 0x81},
    {0x0E, 0x00},

    //Autodetect enable PAL_B/NTSC/N443
    {0x07, 0x23},
#if BT601_ENABLE
    // Figure35 and Figure 40 for BT601 NTSC and PAL
    {0x31, 0x1A},
    {0x32, 0x81},
    {0x33, 0x84},
    {0x34, 0x00},
    {0x35, 0x00},
    {0x36, 0x7D},
    {0x37, 0xA1},
    //NTSC
    {0xE5, 0x41},
    {0xE6, 0x84},
    {0xE7, 0x06},
    //PAL
    {0xE8, 0x41},
    {0xE9, 0x84},
    {0xEA, 0x06},
#endif
};

static REGPAIR  YPrPb_INPUT [] =
{
    {0x00, 0x09},
    {0x31, 0x02},
    {0x3D, 0xA2},
    {0x3E, 0x6A},
    {0x3F, 0xA0},
    {0x58, 0x01},
    {0x0E, 0x80},
    {0x55, 0x81},
    {0x0E, 0x00},

    //Autodetect enable PAL_B/NTSC/N443
    {0x07, 0x23},
#if BT601_ENABLE
    // Figure35 and Figure 40 for BT601 NTSC and PAL
    {0x31, 0x1A},
    {0x32, 0x81},
    {0x33, 0x84},
    {0x34, 0x00},
    {0x35, 0x00},
    {0x36, 0x7D},
    {0x37, 0xA1},
    //NTSC
    {0xE5, 0x41},
    {0xE6, 0x84},
    {0xE7, 0x06},
    //PAL
    {0xE8, 0x41},
    {0xE9, 0x84},
    {0xEA, 0x06},
#endif
};

//=============================================================================
//                Private Function Definition
//=============================================================================
//=============================================================================
//                IIC API FUNCTION START
//=============================================================================
uint8_t _ADV7180_ReadI2c_Byte(uint8_t RegAddr)
{
    static uint8_t dbuf[2]     = {0};
    static char    portname[4] = {0};
    int            result      = 0;
    uint8_t        *pdbuf      = dbuf;
    ITPI2cInfo     evt;
    int            gMasterDev  = 0;

    sprintf(portname, ":i2c%d", ADV7180_IIC_PORT);
    gMasterDev         = open(portname, 0);

    dbuf[0]            = (uint8_t)(RegAddr);
    pdbuf++;

    evt.slaveAddress   = ADV7180_IICADDR;
    evt.cmdBuffer      = dbuf;
    evt.cmdBufferSize  = 1;
    evt.dataBuffer     = pdbuf;
    evt.dataBufferSize = 1;

    result             = read(gMasterDev, &evt, 1);
    if (result != 0)
    {
        ithPrintf("_ADV7180_ReadI2c_Byte read address 0x%02x error!\n", RegAddr);
    }
    //printf("Reg = %x, value[0] = %x ,value[1] = %x\n",RegAddr, dbuf[0],dbuf[1]);
    return dbuf[1];
}

uint32_t _ADV7180_WriteI2c_Byte(uint8_t RegAddr, uint8_t data)
{
    static uint8_t dbuf[2]     = {0};
    static char    portname[4] = {0};
    int            result      = 0;
    uint8_t        *pdbuf      = dbuf;
    ITPI2cInfo     evt;
    int            gMasterDev  = 0;

    sprintf(portname, ":i2c%d", ADV7180_IIC_PORT);
    gMasterDev        = open(portname, 0);

    *pdbuf++          = (uint8_t)(RegAddr & 0xff);
    *pdbuf            = (uint8_t)(data);

    evt.slaveAddress  = ADV7180_IICADDR;     //對接裝置salve address
    evt.cmdBuffer     = dbuf;                //欲傳送給slave data buffer
    evt.cmdBufferSize = 2;                   //傳送data size,單位為byte

    if (0 != (result = write(gMasterDev, &evt, 1)))
    {
        printf("_ADV7180_WriteI2c_Byte Write Error, reg=%02x val=%02x\n", RegAddr, data);
    }
    return result;
}

uint32_t _ADV7180_WriteI2c_ByteMask(uint8_t RegAddr, uint8_t data, uint8_t mask)
{
    uint8_t  value;
    uint32_t flag;

    value = _ADV7180_ReadI2c_Byte(RegAddr);
    value = ((value & ~mask) | (data & mask));
    flag  = _ADV7180_WriteI2c_Byte(RegAddr, value);

    return flag;
}

//=============================================================================
//                IIC API FUNCTION END
//=============================================================================

static void _ADV7180_SWReset()
{
    _ADV7180_WriteI2c_ByteMask(POWER_MANAGEMENT, RESET_MASK, RESET_MASK);
}

static void _Set_ADV7180_Input_CVBS(void)
{
    uint16_t i;

    for (i = 0; i < (sizeof(CVBS_INPUT) / sizeof(REGPAIR)); i++)
    {
        _ADV7180_WriteI2c_Byte(CVBS_INPUT[i].addr, CVBS_INPUT[i].value);
    }
    _ADV7180_WriteI2c_ByteMask(0x4, 0x80, 0x80);
}

static void _Set_ADV7180_Input_SVIDEO(void)
{
    uint16_t i;

    for (i = 0; i < (sizeof(SVIDEO_INPUT) / sizeof(REGPAIR)); i++)
        _ADV7180_WriteI2c_Byte(SVIDEO_INPUT[i].addr, SVIDEO_INPUT[i].value);
}

static void _Set_ADV7180_Input_YPrPb(void)
{
    uint16_t i;

    for (i = 0; i < (sizeof(YPrPb_INPUT) / sizeof(REGPAIR)); i++)
        _ADV7180_WriteI2c_Byte(YPrPb_INPUT[i].addr, YPrPb_INPUT[i].value);
}

//=============================================================================
//                Public Function Definition
//=============================================================================
void Set_ADV7180_Tri_State_Enable()
{
    //LLC pin Tri-State
    _ADV7180_WriteI2c_ByteMask(0x1D, 0x80, 0x80);

    //TIM_OE pin Tri-State
    _ADV7180_WriteI2c_ByteMask(0x04, 0x00, 0x08);

    //SFL Pin Disable (DE)
    _ADV7180_WriteI2c_ByteMask(0x04, 0x00, 0x02);

    //HS, VS, FIELD Data pin Tri-State
    _ADV7180_WriteI2c_ByteMask(0x03, 0x40, 0x40);
}

void Set_ADV7180_Tri_State_Disable()
{
    //LLC pin Tri-State
    _ADV7180_WriteI2c_ByteMask(0x1D, 0x00, 0x80);

    //TIM_OE pin Tri-State
    _ADV7180_WriteI2c_ByteMask(0x04, 0x08, 0x08);

    //SFL Pin Disable (DE)
    _ADV7180_WriteI2c_ByteMask(0x04, 0x00, 0x02);

    //HS, VS, FIELD Data pin Tri-State
    _ADV7180_WriteI2c_ByteMask(0x03, 0x00, 0x40);
}

ADV7180_INPUT_STANDARD Get_Auto_Detection_Result()
{
    uint8_t result;

    result = _ADV7180_ReadI2c_Byte(REG_STATUS1);
    result = (result & 0x70) >> 4;

    switch (result)
    {
    case ADV7180_NTSC_M_J:
        ADV7180_InWidth     = 720;
        ADV7180_InHeight    = 487;
        ADV7180_InFrameRate = 5994;
        gtADV7180CurMode    = ADV7180_NTSC_M_J;
        matchRes            = 1;
        //ithPrintf("NTSC M/J\n");
        break;

    case ADV7180_NTSC_4_43:
        ADV7180_InWidth     = 720;
        ADV7180_InHeight    = 487;
        ADV7180_InFrameRate = 5994;
        gtADV7180CurMode    = ADV7180_NTSC_4_43;
        matchRes            = 1;
        //ithPrintf("NTSC 4.43\n");
        break;

    //case ADV7180_PAL_M:
    //  ADV7180_InWidth = 720;
    //  ADV7180_InHeight = 480;
    //  ADV7180_InFrameRate = 5994;
    //  gtADV7180CurMode = ADV7180_PAL_M;
    //  ithPrintf("PAL_M\n");
    //  break;
    //case ADV7180_PAL_60:
    //  ADV7180_InWidth = 720;
    //  ADV7180_InHeight = 480;
    //  ADV7180_InFrameRate = 6000;
    //  gtADV7180CurMode = ADV7180_PAL_60;
    //  ithPrintf("PAL_60\n");
    //  break;
    case ADV7180_PAL_B_G_H_I_D:
        ADV7180_InWidth     = 720;
        ADV7180_InHeight    = 576;
        ADV7180_InFrameRate = 5000;
        gtADV7180CurMode    = ADV7180_PAL_B_G_H_I_D;
        matchRes            = 1;
        //ithPrintf("PAL B/G/H/I/D\n");
        break;

    //case ADV7180_SECAM:
    //  ADV7180_InWidth = 720;
    //  ADV7180_InHeight = 576;
    //  ADV7180_InFrameRate = 5000;
    //  gtADV7180CurMode = ADV7180_SECAM;
    //  ithPrintf("SECAM\n");
    //  break;
    //case ADV7180_PAL_COMBINATION_N:
    //  ADV7180_InWidth = 720;
    //  ADV7180_InHeight = 576;
    //  ADV7180_InFrameRate = 5000;
    //  gtADV7180CurMode = ADV7180_PAL_COMBINATION_N;
    //  ithPrintf("PAL Combination N\n");
    //  break;
    //case ADV7180_SECAM_525:
    //  ADV7180_InWidth = 720;
    //  ADV7180_InHeight = 480;
    //  ADV7180_InFrameRate = 5994;
    //  gtADV7180CurMode = ADV7180_SECAM_525;
    //  ithPrintf("SECAM 525\n");
    //  break;
    default:
        matchRes = 0;
        ithPrintf("Can not recognize\n");
        break;
    }
    return result;
}

uint16_t _ADV7180_InputSelection()
{
#ifdef AUTO_DETECT_INPUT
    uint16_t Value;

    Value = _ADV7180_ReadI2c_Byte(REG_STATUS1);
    if ((Value & 0x05) != 0x05)
    {
        Value = _ADV7180_ReadI2c_Byte(0x00);
        if ((Value & 0x0F) == 0x06)
        {
            ADV7180_Input_Mode(ADV7180_INPUT_CVBS);
            return ADV7180_INPUT_CVBS;
        }
        else
        {
            ADV7180_Input_Mode(ADV7180_INPUT_SVIDEO);
            return ADV7180_INPUT_SVIDEO;
        }
    }
    else
    {
        Value = _ADV7180_ReadI2c_Byte(0x00);
        if ((Value & 0x0F) == 0x06)
            return ADV7180_INPUT_SVIDEO;
        else
            return ADV7180_INPUT_CVBS;
    }
#else
    uint16_t Value;
    Value = _ADV7180_ReadI2c_Byte(0x00);
    if ((Value & 0x0F) == 0x06)
        return ADV7180_INPUT_SVIDEO;
    else
        return ADV7180_INPUT_CVBS;
#endif
}

void ADV7180Initial(ADV7180_INPUT_MODE mode)
{
    //ADV7180_PowerDown(MMP_FALSE);
    //gtADV7180PowerDown = MMP_FALSE;
    gtADV7180InitDone = MMP_FALSE;
    _ADV7180_SWReset();

    gtADV7180CurMode  = 0xFF;
    gtADV7180PreMode  = 0xFF;

    usleep(1000 * 10);

    ADV7180_Input_Mode(mode);
    gtADV7180CurDev   = mode;
    gtADV7180InitDone = MMP_TRUE;
}

void ADV7180_Input_Mode(ADV7180_INPUT_MODE mode)
{
    if (mode == ADV7180_INPUT_CVBS)
        _Set_ADV7180_Input_CVBS();
    else if (mode == ADV7180_INPUT_SVIDEO)
        _Set_ADV7180_Input_SVIDEO();
    else if (mode == ADV7180_INPUT_YPBPR)
        _Set_ADV7180_Input_YPrPb();

    //Y Range 16 - 235, UV Range 16 - 240
    _ADV7180_WriteI2c_ByteMask(0x04, 0x00, 0x01);

    //_ADV7180_WriteI2c_ByteMask(0x04, 0x01, 0x02);  // Benson test
    //Lock status set by horizontal lock and subcarrier lock
    //_ADV7180_WriteI2c_ByteMask(0x51, 0x80, 0x80);

    //drive strength
    _ADV7180_WriteI2c_Byte(0xF4, 0x04);
}

uint8_t ADV7180_IsStable()
{
    uint16_t Value;
    uint16_t IsStable;

    if (!gtADV7180InitDone || gtADV7180PowerDown)
        return MMP_FALSE;

    Value = _ADV7180_ReadI2c_Byte(REG_STATUS1);
    if (((Value & 0x85) == 0x05) || ((Value & 0x85) == 0x81)) //Color Burst or No Color Burst
    {
        Get_Auto_Detection_Result();

        if (gtADV7180CurMode != gtADV7180PreMode)
        {
            ithPrintf("ADV7180 Resolution = ");
            if (gtADV7180CurMode == ADV7180_NTSC_M_J)
                ithPrintf("NTSM_M_J\n");
            else if (gtADV7180CurMode == ADV7180_NTSC_4_43)
                ithPrintf("NTSC_4_43\n");
            else if (gtADV7180CurMode == ADV7180_PAL_M)
                ithPrintf("PAL_M\n");
            else if (gtADV7180CurMode == ADV7180_PAL_60)
                ithPrintf("PAL_60\n");
            else if (gtADV7180CurMode == ADV7180_PAL_B_G_H_I_D)
                ithPrintf("PAL_B_G_H_I_D\n");
            else if (gtADV7180CurMode == ADV7180_SECAM)
                ithPrintf("SECAM\n");
            else if (gtADV7180CurMode == ADV7180_PAL_COMBINATION_N)
                ithPrintf("PAL_COMBINATION_N\n");
            else if (gtADV7180CurMode == ADV7180_SECAM_525)
                ithPrintf("SECAM_525\n");
            else
                ithPrintf("Unknow Format\n");

            if (gtADV7180CurDev == ADV7180_INPUT_CVBS)
                ithPrintf("CVBS INPUT\n");
            else if (gtADV7180CurDev == ADV7180_INPUT_SVIDEO)
                ithPrintf("S-Video INPUT\n");

            gtADV7180PreMode = gtADV7180CurMode;
        }
        IsStable = MMP_TRUE;
    }
    else
    {
        gtADV7180CurDev = _ADV7180_InputSelection();
        IsStable        = MMP_FALSE;
    }

    return IsStable;
}

void ADV7180_PowerDown(
    uint8_t enable)
{
    //When PDBP is set to 1, setting the PWRDWN bit switches the ADV7180 to a chip-wide power-down mode.
    _ADV7180_WriteI2c_ByteMask(POWER_MANAGEMENT, 0x04, 0x04);

    if (enable)
    {
        gtADV7180InitDone  = MMP_FALSE;
        gtADV7180PowerDown = MMP_TRUE;
        _ADV7180_WriteI2c_ByteMask(POWER_MANAGEMENT, 0x20, 0x20);
    }
    else
    {
        gtADV7180PowerDown = MMP_FALSE;
        _ADV7180_WriteI2c_ByteMask(POWER_MANAGEMENT, 0x00, 0x20);
    }
}

uint8_t ADV7180_IsSVideoInput()
{
    uint16_t Value;

    if (!gtADV7180InitDone || gtADV7180PowerDown)
        return MMP_FALSE;

    if (gtADV7180CurDev == ADV7180_INPUT_SVIDEO)
        return MMP_TRUE;
    else
        return MMP_FALSE;
}

uint8_t ADV7180_IsPowerDown()
{
    return gtADV7180PowerDown;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

//X10LightDriver_t1.c
void ADV7180Initialize(uint16_t Mode)
{
    ithPrintf("ADV7180Initialize\n");

    if (Mode == 0)
        ADV7180Initial(ADV7180_INPUT_CVBS);
    else if (Mode == 1)
        ADV7180Initial(ADV7180_INPUT_SVIDEO);
}

void ADV7180Terminate(void)
{
    ithPrintf("ADV7180Terminate\n");
}

void ADV7180OutputPinTriState(uint8_t flag)
{
    if (flag == MMP_TRUE)
        Set_ADV7180_Tri_State_Enable();
    else
        Set_ADV7180_Tri_State_Disable();
}

uint8_t ADV7180IsSignalStable(uint16_t Mode)
{
    bool isStable;
    isStable = ADV7180_IsStable();
    return isStable;
}

uint16_t ADV7180GetProperty(MODULE_GETPROPERTY property)
{
    uint16_t index;

    Get_Auto_Detection_Result();

    switch (property)
    {
    case GetTopFieldPolarity:
        return MMP_TRUE;
        break;
    case GetHeight:
        return ADV7180_InHeight;
        break;
    case GetWidth:
        return ADV7180_InWidth;
        break;
    case Rate:
        return ADV7180_InFrameRate;
        break;
    case GetModuleIsInterlace:
        return 1;
        break;
    case matchResolution:
        return matchRes;
        break;
    default:
        ithPrintf("error property id =%d\n", property);
        return 0;
        break;
    }
}

uint8_t ADV7180GetStatus(MODULE_GETSTATUS Status)
{
    switch (Status)
    {
    case IsPowerDown:
        return ADV7180_IsPowerDown();
        break;
    case IsSVideoInput:
        return ADV7180_IsSVideoInput();
        break;
    default:
        ithPrintf("error status id =%d \n", Status);
        return 0;
        break;
    }
}

void ADV7180SetProperty(MODULE_SETPROPERTY Property, uint16_t Value)
{}

void ADV7180PowerDown(uint8_t enable)
{
    ADV7180_PowerDown(enable);
}

static void ADV7180SensorDriver_Destory(SensorDriver base)
{
    ADV7180SensorDriver self = (ADV7180SensorDriver)base;
    if (self)
        free(self);
}

static SensorDriverInterfaceStruct interface =
{
    ADV7180Initialize,
    ADV7180Terminate,
    ADV7180OutputPinTriState,
    ADV7180IsSignalStable,
    ADV7180GetProperty,
    ADV7180GetStatus,
    ADV7180SetProperty,
    ADV7180PowerDown,
    ADV7180SensorDriver_Destory
};

SensorDriver ADV7180SensorDriver_Create()
{
    ADV7180SensorDriver self = calloc(1, sizeof(ADV7180SensorDriverStruct));
    self->base.vtable = &interface;
    self->base.type   = "ADV7180";
    return (SensorDriver)self;
}

//end of X10LightDriver_t1.c