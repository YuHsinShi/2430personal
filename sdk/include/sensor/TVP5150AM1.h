#ifndef __TVP5150_H__
#define __TVP5150_H__

#include "ite/itp.h"
#include "ith/ith_defs.h"
#include "ite/ith.h"
#include "mmp_sensor.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 *
 */

#ifndef MMP_TRUE
    #define MMP_TRUE  1
#endif
#ifndef MMP_FALSE
    #define MMP_FALSE 0
#endif

typedef struct SensorDriverTVP5150Struct *TVP5150SensorDriver;
SensorDriver TVP5150SensorDriver_Create();
void TVP5150Initialize(uint16_t Mode);
void TVP5150OutputPinTriState(uint8_t flag);
uint8_t TVP5150IsSignalStable(uint16_t Mode);
uint16_t TVP5150GetProperty(MODULE_GETPROPERTY property);
uint8_t TVP5150GetStatus(MODULE_GETSTATUS Status);
void TVP5150SetProperty(MODULE_SETPROPERTY Property, uint16_t Value);
void TVP5150PowerDown(uint8_t enable);

#ifdef __cplusplus
}
#endif

#endif