#ifndef __RSL_SLAVE__
#define __RSL_SLAVE__

#ifdef __cplusplus
extern "C" {
#endif

#include "alt_cpu/alt_cpu_device.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define MAX_CMD_DATA_BUFFER_SIZE                256
#define CMD_DATA_BUFFER_OFFSET                  (16 * 1024 - MAX_CMD_DATA_BUFFER_SIZE)

#define INIT_CMD_ID                             1
#define READ_DATA_CMD_ID                        2
#define WRITE_DATA_CMD_ID                       3
#define READ_RAW_DATA_CMD_ID                    4
#define WRITE_RAW_DATA_CMD_ID                   5
#define SET_WRITE_COUNTER_CMD_ID                6

#define ITP_IOCTL_ALT_CPU_INIT_PARAMETER        ITP_IOCTL_CUSTOM_CTL_ID1
#define ITP_IOCTL_ALT_CPU_READ_RAW_DATA         ITP_IOCTL_CUSTOM_CTL_ID2
#define ITP_IOCTL_ALT_CPU_WRITE_RAW_DATA        ITP_IOCTL_CUSTOM_CTL_ID3

#define RSL_SLAVE_INFINITE_WRITE                0xFFFFFFFF

typedef enum
{
    RSL_SLAVE0 = 0,
    RSL_SLAVE1,
    RSL_SLAVE_COUNT, //Must be the last entry;
} RSL_SLAVE_ID;


//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct
{
    unsigned long slaveId;
    unsigned long cpuClock;
    unsigned long clkGpio;
    unsigned long dataGpio;
    unsigned long dataWriteGpio;
    unsigned long firstReadDelayUs;
    unsigned long readPeriod; //us
    unsigned long firstWriteDelayUs;
    unsigned long writePeriod; //us
} RSL_SLAVE_INIT_DATA;

typedef struct
{
    unsigned long bSuccess;
    unsigned long slaveId;
    unsigned char pReadBuffer[64];
} RSL_SLAVE_READ_RAW_DATA;

typedef struct
{
    unsigned long slaveId;
    unsigned long writeRepeatCount;
    unsigned char pWriteBuffer[64];
} RSL_SLAVE_WRITE_RAW_DATA;

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





