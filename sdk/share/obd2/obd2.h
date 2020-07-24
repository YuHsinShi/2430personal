#ifndef OBD2_H
#define OBD2_H

#include <malloc.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "string.h"
#include "stdio.h"

#define WAIT_ECU() usleep(5000)

#define OBD_PIDMAP_SIZE 24
/* PID define */
#define PID_ENGINE_LOAD 0x04
#define PID_COOLANT_TEMP 0x05
#define PID_SHORT_TERM_FUEL_TRIM_1 0x06
#define PID_LONG_TERM_FUEL_TRIM_1 0x07
#define PID_SHORT_TERM_FUEL_TRIM_2 0x08
#define PID_LONG_TERM_FUEL_TRIM_2 0x09
#define PID_FUEL_PRESSURE 0x0A
#define PID_INTAKE_MAP 0x0B
#define PID_RPM 0x0C
#define PID_SPEED 0x0D
#define PID_TIMING_ADVANCE 0x0E
#define PID_INTAKE_TEMP 0x0F
#define PID_MAF_FLOW 0x10
#define PID_THROTTLE 0x11
#define PID_AUX_INPUT 0x1E
#define PID_RUNTIME 0x1F
#define PID_DISTANCE_WITH_MIL 0x21
#define PID_COMMANDED_EGR 0x2C
#define PID_EGR_ERROR 0x2D
#define PID_COMMANDED_EVAPORATIVE_PURGE 0x2E
#define PID_FUEL_LEVEL 0x2F
#define PID_WARMS_UPS 0x30
#define PID_DISTANCE 0x31
#define PID_EVAP_SYS_VAPOR_PRESSURE 0x32
#define PID_BAROMETRIC 0x33
#define PID_CATALYST_TEMP_B1S1 0x3C
#define PID_CATALYST_TEMP_B2S1 0x3D
#define PID_CATALYST_TEMP_B1S2 0x3E
#define PID_CATALYST_TEMP_B2S2 0x3F
#define PID_CONTROL_MODULE_VOLTAGE 0x42
#define PID_ABSOLUTE_ENGINE_LOAD 0x43
#define PID_AIR_FUEL_EQUIV_RATIO 0x44
#define PID_RELATIVE_THROTTLE_POS 0x45
#define PID_AMBIENT_TEMP 0x46
#define PID_ABSOLUTE_THROTTLE_POS_B 0x47
#define PID_ABSOLUTE_THROTTLE_POS_C 0x48
#define PID_ACC_PEDAL_POS_D 0x49
#define PID_ACC_PEDAL_POS_E 0x4A
#define PID_ACC_PEDAL_POS_F 0x4B
#define PID_COMMANDED_THROTTLE_ACTUATOR 0x4C
#define PID_TIME_WITH_MIL 0x4D
#define PID_TIME_SINCE_CODES_CLEARED 0x4E
#define PID_ETHANOL_FUEL 0x52
#define PID_FUEL_RAIL_PRESSURE 0x59

typedef enum {
    PRO_CAN_11B_500K = 0,
    PRO_CAN_29B_500K = 1,
    PRO_CAN_29B_250K = 2,
    PRO_CAN_11B_250K = 3,
    //PRO_ISO_9141_2 = 4,          /* not support */
    //PRO_KWP2000_5KBPS = 5,   /* not support */
    //PRO_KWP2000_FAST = 6,     /* not support */
} OBD_PROTOCOLS;

typedef enum {
    RequestPowertrainData = 0x1,     /* Service 1: current powertrain diagnostic data */
    //RequestfreezeData       = 0x2,         /* Service 2: Request powertrain frame freeze data */
    RequestTruoubleCode   = 0x3,     /* Service 3: Request  emission-related diagnostic truoble code */
} OBD_Services;

typedef struct OBD_DTC_NODE{
    char     system;  /* B:body C:chassis P:Powertrain U:Network */
    uint32_t dtc_l;   /* low bits */
    uint32_t dtc_h;   /* height bits */
    uint32_t num;     /* number of codes */
}OBD_DTC_NODE;



/* OBD2 init */
void OBD2Init(OBD_PROTOCOLS pt);

/* Get PID value */
bool OBD2ReadPID(uint8_t pid, int* result);

/* Get DTC value */
bool OBD2ReadDTC(OBD_DTC_NODE* dtc_ptr);





#endif
