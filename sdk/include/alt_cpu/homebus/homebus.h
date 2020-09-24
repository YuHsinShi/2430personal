#ifndef __HOMEBUS__
#define __HOMEBUS__

#ifdef __cplusplus
extern "C" {
#endif

#include "alt_cpu/alt_cpu_device.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define MAX_CMD_DATA_BUFFER_SIZE    512
#define CMD_DATA_BUFFER_OFFSET      (16 * 1024 - MAX_CMD_DATA_BUFFER_SIZE)

#define MAX_PORT_BUFFER_SIZE 		256

#define INIT_CMD_ID                 1
#define WRITE_DATA_CMD_ID			2
#define READ_DATA_CMD_ID			3

#define ITP_IOCTL_HOMEBUS_INIT_PARAM	     			   ITP_IOCTL_CUSTOM_CTL_ID1
#define ITP_IOCTL_HOMEBUS_READ_DATA                      ITP_IOCTL_CUSTOM_CTL_ID2
#define ITP_IOCTL_HOMEBUS_WRITE_DATA                     ITP_IOCTL_CUSTOM_CTL_ID3

typedef enum
{
	NONE,
	ODD,
	EVEN,
} HOMEBUS_PARITY;

typedef enum
{
	TX_COLLISION = -1,
    RX_BUSY = -2,
} HOMEBUS_EXCEPTION_CODE;

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct
{
    unsigned int cpuClock;
    unsigned int txdGpio;
	unsigned int rxdGpio;
	unsigned int parity;
} HOMEBUS_INIT_DATA;

typedef struct
{
    unsigned int len;
	unsigned char pWriteBuffer[MAX_PORT_BUFFER_SIZE];
    unsigned char *pWriteDataBuffer;
} HOMEBUS_WRITE_DATA;

typedef struct
{
    unsigned int len;
	unsigned char pReadBuffer[MAX_PORT_BUFFER_SIZE];
    unsigned char *pReadDataBuffer;
} HOMEBUS_READ_DATA;



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





