#ifndef __USERDEFINE_H__
#define __USERDEFINE_H__

#include "ite/itp.h"
#include "ith/ith_defs.h"
#include "ite/ith.h"
#include "mmp_sensor.h"

#ifdef __cplusplus
extern "C" {
#endif

//X10LightDriver_t1.h
typedef struct UserSensorDriverStruct *UserSensorDriver;
SensorDriver UserSensorDriver_Create();
static void UserSensorDriver_Destory(SensorDriver base);
void UserInitialize(uint16_t Mode);
void UserTerminate(void);
void UserOutputPinTriState(uint8_t flag);
uint16_t UserGetProperty(MODULE_GETPROPERTY property);
uint8_t UserGetStatus(MODULE_GETSTATUS Status);
void UserPowerDown(uint8_t enable);
uint8_t UserIsSignalStable(uint16_t Mode);
//end of X10LightDriver_t1.h

#ifdef __cplusplus
}
#endif

#endif