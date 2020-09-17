#ifndef NCP18_SENSOR_H
#define NCP18_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "project.h"

#define NCP18_ERROR_T_C     99
#define NCP_18_TOLERANCE    6

void NCP18_Init(void);

float NCP18_Detect(void);

#ifdef __cplusplus
}
#endif

#endif /* NCP18_SENSOR_H */