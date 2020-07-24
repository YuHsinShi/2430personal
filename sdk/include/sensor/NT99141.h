#ifndef __NT99141_H__
#define __NT99141_H__

#include "ite/itp.h"
#include "ith/ith_defs.h"
#include "ite/ith.h"
#include "mmp_sensor.h"

#ifdef __cplusplus
extern "C" {
#endif

//X10LightDriver_t1.h
typedef struct NT99141SensorDriverStruct *NT99141SensorDriver;
SensorDriver NT99141SensorDriver_Create();
static void NT99141SensorDriver_Destory(SensorDriver base);
void NT99141Initialize(uint16_t Mode);
void NT99141Terminate(void);
void NT99141OutputPinTriState(uint8_t flag);
uint16_t NT99141GetProperty(MODULE_GETPROPERTY property);
uint8_t NT99141GetStatus(MODULE_GETSTATUS Status);
void NT99141PowerDown(uint8_t enable);
uint8_t NT99141IsSignalStable(uint16_t Mode);
//end of X10LightDriver_t1.h

#ifdef __cplusplus
}
#endif

#endif