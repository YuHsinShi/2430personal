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
#define OLED_STATE_ADDRESS          (CMD_DATA_BUFFER_OFFSET - 4)

#define INIT_CMD_ID                 1
#define FLIP_CMD_ID                 2
#define DISPMODE_CMD_ID             3

#define ITP_IOCTL_OLED_CTRL_INIT_PARAM            ITP_IOCTL_CUSTOM_CTL_ID1
#define ITP_IOCTL_OLED_CTRL_FLIP                  ITP_IOCTL_CUSTOM_CTL_ID2
#define ITP_IOCTL_OLED_CTRL_DISPMODE              ITP_IOCTL_CUSTOM_CTL_ID3

#define OLED_CTRL_STATE_IDEL               0
#define OLED_CTRL_STATE_INIT               1
#define OLED_CTRL_STATE_WRITE              2

typedef enum
{
    OLED0 = 0,
    OLED1,
    OLED_COUNT, //Must be the last entry;
} OLED_ID;

typedef enum
{
    DISP_OFF = 0,
    DISP_ON,
} OLED_DISPMODE;

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct
{
    uint32_t oledId;
    uint32_t gpioCSN;
    uint32_t colOffset;
    uint32_t maxRow;
    uint32_t maxCol;
    uint32_t buffAddrA;
    uint32_t buffAddrB;
    uint32_t buffAddrC;
} OLED_CTRL_INIT;

typedef struct
{
    uint32_t oledId;
    uint32_t addrIndex;
} OLED_CTRL_FLIP;

typedef struct
{
    uint32_t oledId;
    uint32_t dispMode;
} OLED_CTRL_DISPMODE;

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





