#ifndef __SW_UART__
#define __SW_UART__

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

#define ITP_IOCTL_SW_PORT_INIT_PARAM	     			 ITP_IOCTL_CUSTOM_CTL_ID1
#define ITP_IOCTL_SW_PORT_READ_DATA                      ITP_IOCTL_CUSTOM_CTL_ID2
#define ITP_IOCTL_SW_PORT_WRITE_DATA                     ITP_IOCTL_CUSTOM_CTL_ID3

enum SerialPortParity
{
	NONE,
	ODD,
	EVEN,
};

enum SerialPortID
{
	ALT_SW_RS232_0,
    ALT_SW_RS232_1,
	ALT_SW_RS485_0,
    ALT_SW_RS485_1,
    SW_PORT_COUNT, //Must be the last entry;
};

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct
{
	unsigned long id;
    unsigned long cpuClock;
    unsigned long baudrate;
    unsigned long txGpio; // RS232 : TX, RS485 : Din  D- B
	unsigned long rxGpio; // RS232 : RX, RS485 : Dout D+ A
    unsigned long txEnableGpio;
	unsigned long parity;
} SW_PORT_INIT_DATA;

typedef struct
{
	unsigned long id;
    unsigned long len;
	unsigned char pWriteBuffer[MAX_PORT_BUFFER_SIZE];
    unsigned char *pWriteDataBuffer;
} SW_PORT_WRITE_DATA;

typedef struct
{
	unsigned long id;
    unsigned long len;
	unsigned char pReadBuffer[MAX_PORT_BUFFER_SIZE];
    unsigned char *pReadDataBuffer;
} SW_PORT_READ_DATA;



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





