#ifndef __MMP_SENSOR_H__
#define __MMP_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "ite/ith.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef enum MODULE_GETPROPERTY_TAG
{
    GetTopFieldPolarity = 0,
    GetHeight,
    GetWidth,
    Rate,                  //frame rate
    GetModuleIsInterlace,  // interlaced = 1 ,progressive  = 0
    HPolarity,
    VPolarity,
    HStar,
    HEnd,
    VStar1,
    VEnd1,
    VStar2,
    VEnd2,
    matchResolution,
} MODULE_GETPROPERTY;

typedef enum MODULE_SETPROPERTY_TAG
{
    LEDON        = 0,
    LEDOFF       = 1,
    Customdefine = 2,
} MODULE_SETPROPERTY;

typedef enum MODULE_GETSTATUS_TAG
{
    IsPowerDown = 0,
    IsSVideoInput,  //ADV7180
} MODULE_GETSTATUS;

typedef struct SensorDriverStruct *SensorDriver;

typedef struct SensorDriverInterfaceStruct *SensorDriverInterface;

typedef struct SensorDriverStruct
{
    SensorDriverInterface vtable;
    const char            *type;
} SensorDriverStruct;

typedef struct SensorDriverInterfaceStruct
{
    void (*Init)(uint16_t Mode);
    void (*Terminate)(void);
    void (*OutputPinTriState)(uint8_t flag);
    uint8_t (*IsSignalStable)(uint16_t Mode);
    uint16_t (*GetProperty)(MODULE_GETPROPERTY Property);
    uint8_t (*GetStatus)(MODULE_GETSTATUS Status);
    void (*SetProperty)(MODULE_SETPROPERTY Property, uint16_t Value);
    void (*PowerDown)(uint8_t enable);
    void (*Destroy)(SensorDriver);
} SensorDriverInterfaceStruct;

SensorDriver ithSensorCreate(uint8_t *moduleName);
uint8_t  ithSensorIsSignalStable(SensorDriver self, uint16_t Mode);
void     ithSensorInit(SensorDriver self, uint16_t Mode);
void     ithSensorDestroy(SensorDriver self);
void     ithSensorDeInit(SensorDriver self);
uint16_t ithSensorGetProperty(SensorDriver self, MODULE_GETPROPERTY Property);
uint8_t  ithSensorGetStatus(SensorDriver self, MODULE_GETSTATUS Status);
void     ithSensorSetProperty(SensorDriver self, MODULE_SETPROPERTY Property, uint16_t Value);
void     ithSensorPowerDown(SensorDriver self, uint8_t Enable);
void     ithSensorOutputPinTriState(SensorDriver self, uint8_t Flag);

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

#ifdef __cplusplus
}
#endif

#endif