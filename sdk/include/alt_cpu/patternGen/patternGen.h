#ifndef __RSL_SLAVE__
#define __RSL_SLAVE__

#ifdef __cplusplus
extern "C" {
#endif

#include "alt_cpu/alt_cpu_device.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define MAX_CMD_DATA_BUFFER_SIZE    640
#define CMD_DATA_BUFFER_OFFSET      (16 * 1024 - MAX_CMD_DATA_BUFFER_SIZE)

#define INIT_CMD_ID                 1
#define WRITE_CMD_ID                2
#define UPDATE_SEND_COUNT_CMD_ID    3
#define STOP_CMD_ID                 4

#define ITP_IOCTL_INIT_PATTERN_GEN_PARAM            ITP_IOCTL_CUSTOM_CTL_ID1
#define ITP_IOCTL_PATTERN_GEN_WRITE_DATA            ITP_IOCTL_CUSTOM_CTL_ID2
#define ITP_IOCTL_PATTERN_GEN_UPDATE_SEND_COUNT     ITP_IOCTL_CUSTOM_CTL_ID3
#define ITP_IOCTL_PATTERN_GEN_STOP                  ITP_IOCTL_CUSTOM_CTL_ID4

#define MAX_WRITE_DATA_COUNT                        128

#define INIFINITE_SEND_COUNT                        0xFFFFFFFF

typedef enum
{
    PATTERN_GEN0 = 0,
    PATTERN_GEN1,
    PATTERN_GEN2,
    PATTERN_GEN3,
    PATTERN_GEN_COUNT, //Must be the last entry;
} PATTERN_GEN_ID;

typedef enum
{
    GPIO_PULL_LOW = 0,
    GPIO_PULL_HIGH,
    GPIO_KEEP_LAST_VALUE,
    GPIO_INPUT_MODE
} DEFAULT_GPIO_VALUE;

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct
{
    unsigned long patternGenId;
    unsigned long cpuClock;
    unsigned long patternGenGpio;
    unsigned long defaultGpioValue; //Once repeat count down to 0, patternGenGpio will set back to default GPIO value.
    unsigned long timeUnitInUs; //period time unit in microseconds.
} PATTERN_GEN_INIT_DATA;

typedef struct
{
    unsigned short value; //0 or 1
    unsigned short period; //keep period in time units (0 ~ 65535 * timeUnitInUs)
} PATTERN_WRITE_DATA;

//Update the new write pattern.
typedef struct
{
    unsigned long patternGenId;
    unsigned long validDataCount;
    unsigned long sendCount;
    PATTERN_WRITE_DATA tWriteData[MAX_WRITE_DATA_COUNT];
} PATTERN_GEN_WRITE_CMD_DATA;

//Use for write pattern is not changed.
typedef struct
{
    unsigned long patternGenId;
    unsigned long sendCount;
} PATTERN_GEN_UPDATE_SEND_COUNT_CMD_DATA;

typedef struct
{
    unsigned long patternGenId;
} PATTERN_GEN_STOP_CMD_DATA;

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





