#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#if defined (CFG_NOR_USE_AXISPI)
#include "ssp/mmp_axispi.h"
#else
#include "ssp/mmp_spi.h"
#endif

#include "../../include/nor/ftssp.h"
#include "../../include/nor/mmp_nor.h"
#include "../../include/nor/nor.h"
#include "ite/itp.h"
#include "nor_msg.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define NOR_BUS             SPI_0
#define INVALID_SIZE        (-1)
#define SSP_POLLING_COUNT   (0x100000)
#define WRITE_DATA_COUNTS   8
#define WRITE_DATA_BYTES    28          //according to fifo length
#ifdef _WIN32
    #define PER_READ_BYTES  28          //14*2 (fifo size - input size)
#else
    #define PER_READ_BYTES  60          //15*4 (fifo size - input size)
#endif
#define PER_DMA_BYTES       2048

#ifdef __FREERTOS__
    #define WriteReg        ithWriteRegA
    #define ReadReg         ithReadRegA
#else
    #define WriteReg        ithWriteRegH
    #define ReadReg         ithReadRegH
#endif

#if defined (CFG_NOR_ENABLE_QUADMODE)
    #define QuadmodeEnable 1
    #if defined (CFG_NOR_ENABLE_DTRMODE)
        #define DTRmodeEnable 1
    #else
        #define DTRmodeEnable 0
    #endif
#else
    #define QuadmodeEnable 0
    #define DTRmodeEnable 0
#endif

#define AXISPI              0
//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct
{
    uint16_t    sectorNum;
    uint32_t    startAddr;
    uint32_t    endAddr;
} NOR_ADDR_MAP;

typedef struct
{
    uint16_t    bytesPerPage;
    uint16_t    pagesPerSector;
    uint32_t    bytesPerSector;
    uint16_t    sectorsPerBlock;
    uint16_t    totalBlocks;

    // Operation
    bool        use4BytesAddress;

    // Quad mode
    uint32_t    status;
    uint8_t     page_program_4PP;
    uint8_t     page_program_4PP4B;
} NOR_ADDR_CONTEXT;

typedef enum
{
    AMIC_A25L032,
    AMIC_A25Q032,
    ATMEL_AT26DF161,
    ATMEL_AT26D321,
    BY_25Q64AS,
    BY_25Q128AS,
    EON_EN25P32,
    EON_EN25B16,
    EON_EN25B32,
    EON_EN25B64,
    EON_EN25F16,
    EON_EN25F32,
    EON_EN25Q16,
    EON_EN25Q32A,
    EON_EN25F80,
    EON_EN25Q80A,
    EON_EN25Q64,
    EON_EN25QH32,
    ES_ES25M16A,
    ESMT_F25L16A,
    ESMT_F25L32Q,
    FS_25Q128F2TFI,
    MX_25L1605A,
    MX_25L3205D,
    MX_25L3235D,
    MX_25L1635D,
    MX_25L6445E,
    MX_25L12835F,
    MX_25L25735F,
    MX_25L51245G,
    GD_GD25Q64B,
    GD_GD25Q32,
    GD_GD25Q16,
    GD_GD25Q128C,
    GD_GD25Q256D,
    GD_GD25Q512MC,
    NUMON_M25P20,
    NUMON_M25P32,
    DS_25Q64M,
    DS_25Q4AM,
    SPAN_S25FL016A,
    SPAN_S25FL032A,
    SST_25VF016B,
    WIN_W25X16A,
    WIN_W25X32V,
    WIN_W25Q32JV,
    WIN_W25Q64JV,
    WIN_W25Q128JV,
    WIN_W25Q256JVFSQ,
    ZB_25VQ128,
    ZB_25VQ64,
    UNKNOW_VENDOR = 0xFFFF
} NOR_VENDOR_ID;

typedef struct
{
    uint8_t     manufatureID;
    uint16_t    deviceID;
    uint8_t     deviceID2;
} NOR_ID;

typedef struct
{
    //uint8_t       manufatureID;
    //uint16_t      deviceID;
    //uint8_t       deviceID2;
    NOR_ID          id;
    uint8_t         *deviceName;
    NOR_VENDOR_ID   vendorID;
} NOR_VENDOR_CONTEXT;

typedef enum
{
    NOR_BIG_ENDIAN,
    NOR_LITTLE_ENDIAN
} NOR_ENDIAN_TYPE;

//=============================================================================
//                              Global Data Definition
//=============================================================================
NOR_VENDOR_ID g_vendor = UNKNOW_VENDOR;
typedef struct _NOR_OBJECT
{
    int                 refCount;
    SPI_PORT            port;
    SPI_CSN             chipSelectPin;
    NOR_ID              id;
    NOR_ADDR_CONTEXT    context;
    pthread_mutex_t     mutex;
    uint32_t            protectCount;
    NOR_PROTECT_ITEM    *protectInfo;
} NOR_OBJECT;

static NOR_OBJECT   NorObjects[4] = {
    {
     0,                             // refCount
     SPI_0,                         // port
     SPI_CSN_0,                     // chipSelectPin
     {0},                           // id
     {0},                           // context
     PTHREAD_MUTEX_INITIALIZER,     // mutex
     0,                             // protectCount
     NULL                           // protectInfo
    },
    {
     0,                             // refCount
     SPI_0,                         // port
     SPI_CSN_1,                     // chipSelectPin
     {0},                           // id
     {0},                           // context
     PTHREAD_MUTEX_INITIALIZER,     // mutex
     0,                             // protectCount
     NULL                           // protectInfo
    },
    {
     0,                             // refCount
     SPI_1,                         // port
     SPI_CSN_0,                     // chipSelectPin
     {0},                           // id
     {0},                           // context
     PTHREAD_MUTEX_INITIALIZER,     // mutex
     0,                             // protectCount
     NULL                           // protectInfo
    },
    {
     0,                             // refCount
     SPI_1,                         // port
     SPI_CSN_1,                     // chipSelectPin
     {0},                           // id
     {0},                           // context
     PTHREAD_MUTEX_INITIALIZER,     // mutex
     0,                             // protectCount
     NULL                           // protectInfo
    }
};

NOR_VENDOR_CONTEXT  nor_support_vendor[] = {
    {0x37, 0x3016, 0x15, "AMIC__A25L032",    AMIC_A25L032    },
    {0x37, 0x4016, 0x15, "AMIC__A25Q032",    AMIC_A25Q032    },
    {0x1F, 0x4600, 0xFF, "ATMEL_AT26DF161",  ATMEL_AT26DF161 },
    {0x1F, 0x4700, 0xFF, "ATMEL__AT26D321",  ATMEL_AT26D321  },
    {0x68, 0x4017, 0x16, "BY_25Q64AS",       BY_25Q64AS      },
    {0x68, 0x4018, 0x17, "BY_25Q128AS",      BY_25Q128AS     },
    {0x1C, 0x2016, 0x15, "EON__EN25P32",     EON_EN25P32     },
    {0x1C, 0x2015, 0x34, "EON__EN25B16",     EON_EN25B16     },
    {0x1C, 0x2016, 0x35, "EON__EN25B32",     EON_EN25B32     },
    {0x1C, 0x2017, 0x36, "EON__EN25B64",     EON_EN25B64     },
    {0x1C, 0x3115, 0x14, "EON__EN25F16",     EON_EN25F16     },
    {0x1C, 0x3116, 0x15, "EON__EN25F32",     EON_EN25F32     },
    {0x1C, 0x3015, 0x14, "EON_EN25Q16",      EON_EN25Q16     },
    {0x1C, 0x3016, 0x15, "EON_EN25Q32A",     EON_EN25Q32A    },
    {0x1C, 0x3114, 0x13, "EON_EN25F80",      EON_EN25F80     },
    {0x1C, 0x3014, 0x13, "EON_EN25Q80A",     EON_EN25Q80A    },
    {0x1C, 0x3017, 0x16, "EON_EN25Q64",      EON_EN25Q64     },
    {0x1C, 0x7016, 0x15, "EON_EN25QH32",     EON_EN25QH32    },
    {0x4A, 0x3215, 0x14, "ES__ES25M16A",     ES_ES25M16A     },
    {0x8C, 0x2015, 0x14, "ESMT_F25L16PA",    ESMT_F25L16A    },
    {0x8C, 0x4116, 0x15, "ESMT_F25L32Q",     ESMT_F25L32Q    },
    {0xA1, 0x4018, 0x17, "FS_25Q128F2TFI",   FS_25Q128F2TFI  },
    {0xC2, 0x2015, 0x14, "MX__25L1605A",     MX_25L1605A     },
    {0xC2, 0x2016, 0x15, "MX__25L3205D",     MX_25L3205D     },
    {0xC2, 0x5E16, 0x5E, "MX__25L3235D",     MX_25L3235D     },
    {0xC2, 0x2415, 0x24, "MX__25L1635D",     MX_25L1635D     },
    {0xC2, 0x2017, 0x16, "MX_25L6445E",      MX_25L6445E     },
    {0xC2, 0x2018, 0x17, "MX_25L12835F",     MX_25L12835F    },
    {0xC2, 0x2019, 0x18, "MX_25L25735F",     MX_25L25735F    },
    {0xC2, 0x201A, 0x19, "MX_25L51245G",     MX_25L51245G    },
    {0xC8, 0x4017, 0x16, "GD_GD25Q64B",      GD_GD25Q64B     },
    {0xC8, 0x4016, 0x15, "GD_GD25Q32",       GD_GD25Q32      },
    {0xC8, 0x4015, 0x14, "GD_GD25Q16",       GD_GD25Q16      },
    {0xc8, 0x4018, 0x17, "GD_GD25Q128C",     GD_GD25Q128C    },
    {0xc8, 0x4019, 0x18, "GD_GD25Q256D",     GD_GD25Q256D    },
    {0xc8, 0x4020, 0x19, "GD_GD25Q512MC",    GD_GD25Q512MC   },
    {0x20, 0x2012, 0xFF, "NUMON_M25P20",     NUMON_M25P20    },
    {0x20, 0x2016, 0xFF, "NUMON_M25P32",     NUMON_M25P32    },
    {0x20, 0x7017, 0x16, "DS_25Q64M",        DS_25Q64M       },
    {0x20, 0x7018, 0x17, "DS_25Q4AM",        DS_25Q4AM       },
    {0x01, 0x0214, 0xFF, "SPAN_S25FL016A",   SPAN_S25FL016A  },
    {0x01, 0x0215, 0xFF, "SPAN_S25FL032A",   SPAN_S25FL032A  },
    {0xBF, 0x2541, 0x41, "SST_25VF016B",     SST_25VF016B    },
    {0xEF, 0x3015, 0x14, "WIN__W25X16A",     WIN_W25X16A     },
    {0xEF, 0x3016, 0x15, "WIN_W25X32V",      WIN_W25X32V     },
    {0xEF, 0x4016, 0x15, "WIN_W25Q32JV",     WIN_W25Q32JV    },
    {0xEF, 0x4017, 0x16, "WIN_W25Q64JV",     WIN_W25Q64JV    },
    {0xEF, 0x4018, 0x17, "WIN_W25Q128JV",    WIN_W25Q128JV   },
    {0xEF, 0x4019, 0x18, "WIN_W25Q256JVFSQ", WIN_W25Q256JVFSQ},
    {0x5E, 0x4018, 0x17, "ZB_25VQ128",       ZB_25VQ128      },
    {0x5E, 0x4017, 0x16, "ZB_25VQ64",        ZB_25VQ64       }
};

//=============================================================================
//                              Private Function Definition
//=============================================================================
static bool
norReadDeviceID(
    NOR_OBJECT  *norObject,
    NOR_ID      *id)
{
    bool        result      = true;
    uint8_t     command[5]  = {0};
    uint8_t     id1[2]      = {0};
    uint8_t     id2[3]      = {0};

#if defined (CFG_NOR_USE_AXISPI)
    SPI_IOMODE  iomode      = SPI_IOMODE_0; //serial mode
    SPI_DTRMODE dtrmode     = SPI_DTRMODE_0; //DTR mode off

    do
    {
        command[0] = JEDEC_READ_ID;
        if (mmpAxiSpiPioRead(norObject->port, iomode, dtrmode, command, 1, 0, 0, (uint32_t *)&id2, 3) == false)
        {
            result = false;
            break;
        }

        command[0] = READ_ID_CMD;
        if (mmpAxiSpiPioRead(norObject->port, iomode, dtrmode, command, 1, 0x0, 3, (uint32_t *)&id1, 2) == false)
        {
            result = false;
            break;
        }
        id->manufatureID    = id1[0];
        id->deviceID        = (((uint16_t)id2[1]) << 8) | id2[2];
        id->deviceID2       = id1[1];
    } while (0);
#else
    do
    {
        command[0] = JEDEC_READ_ID;
        if (mmpSpiPioRead(norObject->port, command, 1, id2, 3, 0) == false)
        {
            result = false;
            break;
        }
        command[0] = READ_ID_CMD;
        if (mmpSpiPioRead(norObject->port, command, 4, id1, 2, 0) == false)
        {
            result = false;
            break;
        }
        id->manufatureID    = id1[0];
        id->deviceID        = (((uint16_t)id2[1]) << 8) | id2[2];
        id->deviceID2       = id1[1];
    } while (0);
#endif

    return result;
}

static void
norGetContext(
    NOR_OBJECT          *norObject,
    NOR_VENDOR_CONTEXT  *context)
{
    switch (context->vendorID)
    {
    case NUMON_M25P20:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 4;
        break;

    case ESMT_F25L16A:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 32;
        break;

    case EON_EN25P32:
    case NUMON_M25P32:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 64;
        break;

    case EON_EN25B16:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 32;   //31x64k + 1x32k + 1x16k + 1x8k + 2x4k
        break;

    case MX_25L1605A:
    case MX_25L1635D:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 32;
        if (QuadmodeEnable)
            norObject->context.status           = 0x40;
        break;

    case WIN_W25X16A:
    case ATMEL_AT26DF161:
    case SPAN_S25FL016A:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 32;
        break;

    case EON_EN25F16:
    case EON_EN25Q16:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 32;   //31x64k + 1x32k + 1x16k + 1x8k + 2x4k
        break;

    case MX_25L3235D:
    case MX_25L3205D:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 64;
        if (QuadmodeEnable)
            norObject->context.status           = 0x40;
        break;

    case WIN_W25Q32JV:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 64;
        if (QuadmodeEnable)
        {
            norObject->context.page_program_4PP = 0x32;
            norObject->context.status           = 0x200;
        }
        break;

    case EON_EN25B32:
    case EON_EN25F32:
    case EON_EN25Q32A:
    case EON_EN25QH32:
    case SPAN_S25FL032A:
    case WIN_W25X32V:
    case ESMT_F25L32Q:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 64;
        break;

    case SST_25VF016B:
        norObject->context.bytesPerPage     = 512;
        norObject->context.pagesPerSector   = 128;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 32;
        break;

    case AMIC_A25L032:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 64;
        break;

    case ATMEL_AT26D321:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 64;
        break;

    case ES_ES25M16A:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 32;
        break;

    case EON_EN25B64:
    case EON_EN25Q64:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 128;
        break;

    case DS_25Q64M:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 128;
        if (QuadmodeEnable)
            norObject->context.page_program_4PP = 0x32;
        break;

    case BY_25Q64AS:
    case ZB_25VQ64:
    case WIN_W25Q64JV:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 128;
        if (QuadmodeEnable)
        {
            norObject->context.page_program_4PP = 0x32;
            norObject->context.status           = 0x200;
        }
        break;

    case MX_25L6445E:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 128;
        if (QuadmodeEnable)
            norObject->context.status           = 0x40;
        break;

    case MX_25L12835F:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 256;
        if (QuadmodeEnable)
            norObject->context.status           = 0x40;
        break;

    case DS_25Q4AM:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 256;
        if (QuadmodeEnable)
            norObject->context.page_program_4PP = 0x32;
        break;

    case BY_25Q128AS:
    case ZB_25VQ128:
    case WIN_W25Q128JV:
    case FS_25Q128F2TFI:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 256;
        if (QuadmodeEnable)
        {
            norObject->context.page_program_4PP = 0x32;
            norObject->context.status           = 0x200;
        }
        break;

    case MX_25L25735F:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 512;
        norObject->context.use4BytesAddress = true;
        if (QuadmodeEnable)
            norObject->context.status           = 0x40;
        break;

    case WIN_W25Q256JVFSQ:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 512;
        norObject->context.use4BytesAddress = true;
        if (QuadmodeEnable)
        {
            norObject->context.page_program_4PP   = 0x32;
            norObject->context.page_program_4PP4B = 0x34;
            norObject->context.status             = 0x200;
        }
        break;

    case MX_25L51245G:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 1024;
        norObject->context.use4BytesAddress = true;
        if (QuadmodeEnable)
            norObject->context.status           = 0x40;
        break;

    case EON_EN25F80:
    case EON_EN25Q80A:
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 16;
        break;

    case GD_GD25Q64B:   // 8 MB
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 128;
        if (QuadmodeEnable)
            norObject->context.status           = 0x2;
        break;

    case GD_GD25Q32:    // 4 MB
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 64;
        if (QuadmodeEnable)
            norObject->context.status           = 0x2;
        break;

    case GD_GD25Q16:    // 2 MB
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 256;
        norObject->context.bytesPerSector   = 64 * 1024;
        norObject->context.sectorsPerBlock  = 1;
        norObject->context.totalBlocks      = 32;
        if (QuadmodeEnable)
            norObject->context.status           = 0x2;
        break;

    case GD_GD25Q128C:     // 16 MB
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 16;
        norObject->context.bytesPerSector   = 4 * 1024;
        norObject->context.sectorsPerBlock  = 16;
        norObject->context.totalBlocks      = 256;
        if (QuadmodeEnable)
        {
            norObject->context.page_program_4PP = 0x32;
            norObject->context.status           = 0x200;
        }
        break;

    case GD_GD25Q256D:     // 32 MB
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 16;
        norObject->context.bytesPerSector   = 4 * 1024;
        norObject->context.sectorsPerBlock  = 16;
        norObject->context.totalBlocks      = 512;
        norObject->context.use4BytesAddress = true;
        if (QuadmodeEnable)
        {
            norObject->context.page_program_4PP   = 0x32;
            norObject->context.page_program_4PP4B = 0x34;
            norObject->context.status             = 0x200;
        }
        break;

    case GD_GD25Q512MC:    // 64 MB
        norObject->context.bytesPerPage     = 256;
        norObject->context.pagesPerSector   = 16;
        norObject->context.bytesPerSector   = 4 * 1024;
        norObject->context.sectorsPerBlock  = 16;
        norObject->context.totalBlocks      = 1024;
        norObject->context.use4BytesAddress = true;
        if (QuadmodeEnable)
            norObject->context.status           = 0x2;
        break;

    default:
        break;
    }
}

#if defined (CFG_NOR_USE_AXISPI)
static bool
norSendCommand(
    NOR_OBJECT      *norObject,
    FLASH_CMD_TYPE  cmdType,
    uint32_t        arg,
    uint32_t        outDataBuffer,
    uint32_t        outDataBufferSize)
{
    bool        result          = true;
    uint8_t     cmd             = 0;
    uint8_t     commands[6]     = {0};
    uint32_t    commandDataSize = 0;
    uint32_t    address         = 0;
    SPI_IOMODE  iomode          = SPI_IOMODE_0;
    SPI_DTRMODE dtrmode         = SPI_DTRMODE_0;

    switch (cmdType)
    {
    case ERASE_ALL:
        //C7h/ 60h
        commands[0] = ERASE_BULK_CMD;
        result      = mmpAxiSpiPioWrite(norObject->port, iomode, commands, 1, NULL, 0, 0, 0);
        break;

    case WRITE_EN:
        //06h
        commands[0] = WRITE_EN_CMD;
        result      = mmpAxiSpiPioWrite(norObject->port, iomode, commands, 1, NULL, 0, 0, 0);
        break;

    case WRITE_DIS:
        //04h
        commands[0] = WRITE_DIS_CMD;
        result      = mmpAxiSpiPioWrite(norObject->port, iomode, commands, 1, NULL, 0, 0, 0);
        break;

    case WRITE_STATUS_EN:
        commands[0] = WRITE_STATUS_EN_CMD;
        result      = mmpAxiSpiPioWrite(norObject->port, iomode, commands, 1, NULL, 0, 0, 0);
        break;

    case WRITE_STATUS:
        //01h S7-S0
        commands[0] = WRITE_STATUS_CMD;
        //commands[1] = arg;
        if (norObject->context.use4BytesAddress == true)
        {
            commands[1]     = arg >> 24;
            commands[2]     = arg >> 16;
            commands[3]     = arg >> 8;
            commands[4]     = arg;
            commandDataSize = 5;
        }
        else
        {
            commands[1]     = arg >> 24;
            commands[2]     = arg >> 16;
            commands[3]     = arg >> 8;
            commands[4]     = arg;
            commandDataSize = 4;
        }

        result = mmpAxiSpiPioWrite(norObject->port, iomode, commands, 1, &arg, 0, &outDataBuffer, outDataBufferSize);
        break;

    case WRITE_STATUS_2:
        //31h S15-S8
        commands[0] = WRITE_STATUS_2_CMD;
        //commands[1] = arg;
        if (norObject->context.use4BytesAddress == true)
        {
            commands[1]     = arg >> 24;
            commands[2]     = arg >> 16;
            commands[3]     = arg >> 8;
            commands[4]     = arg;
            commandDataSize = 5;
        }
        else
        {
            commands[1]     = arg >> 24;
            commands[2]     = arg >> 16;
            commands[3]     = arg >> 8;
            commands[4]     = arg;
            commandDataSize = 4;
        }

        result = mmpAxiSpiPioWrite(norObject->port, iomode, commands, 1, &arg, 0, &outDataBuffer, outDataBufferSize);
        break;

    case ERASE_SECTOR:
        //20h A23-A16 A15-A8 A7-A0
        if (norObject->context.use4BytesAddress == true)
        {
            cmd = ERASE_SECOTR4B_CMD;
        }
        else
        {
            cmd = ERASE_SECOTR_CMD;
        }
        commands[0] = cmd;
        if (norObject->context.use4BytesAddress == true)
        {
            commands[1]     = arg >> 24;
            commands[2]     = arg >> 16;
            commands[3]     = arg >> 8;
            commands[4]     = arg;
            commandDataSize = 5;
        }
        else
        {
            commands[1]     = arg >> 24;
            commands[2]     = arg >> 16;
            commands[3]     = arg >> 8;
            commands[4]     = arg;
            commandDataSize = 4;
        }
    #if defined(_WIN32) || (defined(__OPENRTOS__) || defined(__FREERTOS__))
        if (norObject->context.use4BytesAddress)
        {
            result = mmpAxiSpiPioWrite(norObject->port, iomode, commands, 1, &arg, 4, 0, 0);
        }
        else
        {
            result = mmpAxiSpiPioWrite(norObject->port, iomode, commands, 1, &arg, 3, 0, 0);
        }
    #else
        if (norObject->context.use4BytesAddress == true)
        {
            result = mmpAxiSpiPioWrite(norObject->port, iomode, commands, commandDataSize, 0, 0, 8);
        }
        else
        {
            result = mmpAxiSpiPioWrite(norObject->port, iomode, commands, 4, 0, 0, 32);
        }
    #endif
        break;

    case ERASE_BLOCK:
        //D8h/ 52h A23-A16 A15-A8 A7-A0
        if (norObject->context.use4BytesAddress == true)
        {
            cmd = ERASE_BLOCK4B_CMD;
        }
        else
        {
            cmd = ERASE_BLOCK_CMD;
        }
        commands[0] = cmd;
        if (norObject->context.use4BytesAddress == true)
        {
            commands[1]     = (uint8_t)(arg >> 24);
            commands[2]     = (uint8_t)(arg >> 16);
            commands[3]     = (uint8_t)(arg >> 8);
            commands[4]     = (uint8_t)arg;
            commandDataSize = 5;
        }
        else
        {
            commands[1]     = (uint8_t)(arg >> 24);
            commands[2]     = (uint8_t)(arg >> 16);
            commands[3]     = (uint8_t)(arg >> 8);
            commands[4]     = (uint8_t)arg;
            commandDataSize = 4;
        }
    #if defined(_WIN32) || (defined(__OPENRTOS__) || defined(__FREERTOS__))
        if (norObject->context.use4BytesAddress == true)
        {
            result = mmpAxiSpiPioWrite(norObject->port, iomode, commands, 1, &arg, 4, 0, 0);
        }
        else
        {
            result = mmpAxiSpiPioWrite(norObject->port, iomode, commands, 1, &arg, 3, 0, 0);
        }
    #else
        if (norObject->context.use4BytesAddress == true)
        {
            result = mmpAxiSpiPioWrite(norObject->port, iomode, commands, commandDataSize, 0, 0, 8);
        }
        else
        {
            result = mmpAxiSpiPioWrite(norObject->port, iomode, commands, 4, 0, 0, 32);
        }
    #endif
        break;

    case AUTO_WRITE:
        break;

    case AUTO_WRITE_DATA_ONLY:
        break;

    case WRITE:
        //02h A23-A16 A15-A8 A7-A0 D7-D0 (Next byte) continuous
        if (norObject->context.use4BytesAddress == true)
        {
            if (QuadmodeEnable)
            {
                if (norObject->context.page_program_4PP4B)
                {
                    cmd     = norObject->context.page_program_4PP4B;
                    iomode  = SPI_IOMODE_2;
                }
                else
                {
                    cmd     = PAGE_PROGRAM_4PP4B_CMD;
                    iomode  = SPI_IOMODE_4;
                }
                mmpAxiSpiSetDummyByte(NOR_BUS, 6);
            }
            else
                cmd = PAGE_PROGRAM4B_CMD;
        }
        else
        {
            if (QuadmodeEnable)
            {
                if (norObject->context.page_program_4PP)
                {
                    cmd     = norObject->context.page_program_4PP;
                    iomode  = SPI_IOMODE_2;
                }
                else
                {
                    cmd     = PAGE_PROGRAM_4PP_CMD;
                    iomode  = SPI_IOMODE_4;
                }
                mmpAxiSpiSetDummyByte(NOR_BUS, 6);
            }
            else
                cmd = PAGE_PROGRAM_CMD;
        }
        commands[0] = cmd;
        if (norObject->context.use4BytesAddress == true)
        {
            commands[1]     = (uint8_t)(arg >> 24);
            commands[2]     = (uint8_t)(arg >> 16);
            commands[3]     = (uint8_t)(arg >> 8);
            commands[4]     = (uint8_t)arg;
            commandDataSize = 5;
        }
        else
        {
            commands[1]     = (uint8_t)(arg >> 24);
            commands[2]     = (uint8_t)(arg >> 16);
            commands[3]     = (uint8_t)(arg >> 8);
            commands[4]     = (uint8_t)arg;
            commandDataSize = 4;
        }
    #if defined(_WIN32) || (defined(__OPENRTOS__) || defined(__FREERTOS__))
        if (norObject->context.use4BytesAddress == true)
        {
        #if 0
            //if(outDataBufferSize >= 16 || (outDataBufferSize & 1))
            if (outDataBufferSize >= 32)
            {
                result = mmpSpiDmaWrite(norObject->port, commands, 1, &arg, 4, (uint8_t *)outDataBuffer, outDataBufferSize);
            }
            else
            {
                result = mmpSpiPioWrite(norObject->port, commands, 1, &arg, 4, (uint8_t *)outDataBuffer, outDataBufferSize);
            }
        #else
            uint32_t    remainsize  = outDataBufferSize;
            uint32_t    wsize       = 0;
            uint32_t    waddr       = arg;
            uint32_t    wDMASize    = 0;
            uint8_t     *wptr       = (uint8_t *)outDataBuffer;
            uint8_t     cnt         = 0;
            static int  ret         = 0;

            uint8_t     cmd;

            int         i = 0;

            while (remainsize)
            {
                cnt = (uint8_t)((uint32_t)wptr % 4);

                if (cnt) //unalign address
                {
                    {
                        cmd     = WRITE_EN_CMD;
                        result  = mmpAxiSpiPioWrite(norObject->port, iomode, &cmd, 1, NULL, 0, 0, 0);
                    }
                    result              = mmpAxiSpiPioWrite(norObject->port, iomode, commands, 1, &waddr, 4, (uint32_t *)wptr, 4 - cnt);
                    {
                        cmd     = READ_STATUS_CMD;
                        mmpAxiSpiPioRead(norObject->port, iomode, dtrmode, &cmd, 1, 0, 0, 0, 0);
                    }
                    wptr                += (4 - cnt);
                    remainsize          -= (4 - cnt);
                    outDataBufferSize   -= (4 - cnt);
                    waddr               += (4 - cnt);
                }
                else //align address
                {
                    if (remainsize >= 4)
                    {
                        if ((remainsize % 4) == 0)
                        {
                            wDMASize    = remainsize;
                            remainsize  -= remainsize;
                        }
                        else
                        {
                            outDataBufferSize   = (outDataBufferSize >> 2) << 2;
                            wDMASize            = outDataBufferSize;
                            remainsize          -= outDataBufferSize;
                        }

                        {
                            cmd     = WRITE_EN_CMD;
                            result  = mmpAxiSpiPioWrite(norObject->port, iomode, &cmd, 1, NULL, 0, 0, 0);
                        }

                        ret++;
                        result  = mmpAxiSpiDmaWrite(norObject->port, iomode, commands, 1, &waddr, 4, wptr, wDMASize);

                        {
                            cmd     = READ_STATUS_CMD;
                            mmpAxiSpiPioRead(norObject->port, iomode, dtrmode, &cmd, 1, 0, 0, 0, 0);
                        }

                        wptr    += wDMASize;
                        waddr   += wDMASize;
                    }
                    else
                    {
                        {
                            cmd     = WRITE_EN_CMD;
                            result  = mmpAxiSpiPioWrite(norObject->port, iomode, &cmd, 1, NULL, 0, 0, 0);
                        }

                        result      = mmpAxiSpiPioWrite(norObject->port, iomode, commands, 1, &waddr, 4, (uint32_t *)wptr, remainsize);

                        {
                            cmd     = READ_STATUS_CMD;
                            mmpAxiSpiPioRead(norObject->port, iomode, dtrmode, &cmd, 1, 0, 0, 0, 0);
                        }

                        waddr       += remainsize;
                        wptr        += remainsize;
                        remainsize  = 0;
                    }
                }
            }

        #endif
        }
        else
        {
            uint32_t    remainsize  = outDataBufferSize;
            uint32_t    wsize       = 0;
            uint32_t    waddr       = arg;
            uint32_t    wDMASize    = 0;
            uint8_t     *wptr       = (uint8_t *)outDataBuffer;
            uint8_t     cnt         = 0;
            static int  ret         = 0;

            int         i           = 0;
            uint8_t     cmd;

            while (remainsize)
            {
                cnt = (uint8_t)((uint32_t)wptr % 4);
                if (cnt) //unalign address
                {
                    {
                        cmd     = WRITE_EN_CMD;
                        result  = mmpAxiSpiPioWrite(norObject->port, iomode, &cmd, 1, NULL, 0, 0, 0);
                    }

                    result              = mmpAxiSpiPioWrite(norObject->port, iomode, commands, 1, &waddr, 3, (uint32_t *)wptr, 4 - cnt);

                    {
                        cmd     = READ_STATUS_CMD;
                        mmpAxiSpiPioRead(norObject->port, iomode, dtrmode, &cmd, 1, 0, 0, 0, 0);
                    }

                    wptr                += (4 - cnt);
                    remainsize          -= (4 - cnt);
                    outDataBufferSize   -= (4 - cnt);
                    waddr               += (4 - cnt);
                }
                else //align address
                {
                    if (remainsize >= 4)
                    {
                        if ((remainsize % 4) == 0)
                        {
                            wDMASize    = remainsize;
                            remainsize  -= remainsize;
                        }
                        else
                        {
                            outDataBufferSize   = (outDataBufferSize >> 2) << 2;
                            wDMASize            = outDataBufferSize;
                            remainsize          -= outDataBufferSize;
                        }

                        {
                            cmd     = WRITE_EN_CMD;
                            result  = mmpAxiSpiPioWrite(norObject->port, iomode, &cmd, 1, NULL, 0, 0, 0);
                        }

                        ret++;
                        result  = mmpAxiSpiDmaWrite(norObject->port, iomode, commands, 1, &waddr, 3, wptr, wDMASize);

                        {
                            cmd     = READ_STATUS_CMD;
                            mmpAxiSpiPioRead(norObject->port, iomode, dtrmode, &cmd, 1, 0, 0, 0, 0);
                        }

                        wptr    += wDMASize;
                        waddr   += wDMASize;
                    }
                    else
                    {
                        {
                            cmd     = WRITE_EN_CMD;
                            result  = mmpAxiSpiPioWrite(norObject->port, iomode, &cmd, 1, NULL, 0, 0, 0);
                        }

                        result      = mmpAxiSpiPioWrite(norObject->port, iomode, commands, 1, &waddr, 3, (uint32_t *)wptr, remainsize);

                        {
                            cmd     = READ_STATUS_CMD;
                            mmpAxiSpiPioRead(norObject->port, iomode, dtrmode, &cmd, 1, 0, 0, 0, 0);
                        }

                        waddr       += remainsize;
                        wptr        += remainsize;
                        remainsize  = 0;
                    }
                }
            }
        }
    #else
        if (norObject->context.use4BytesAddress == true)
        {
            result = mmpAxiSpiPioWrite(norObject->port, iomode, commands, commandDataSize, (uint8_t *)outDataBuffer, outDataBufferSize, 32);
        }
        else
        {
            result = mmpAxiSpiPioWrite(norObject->port, iomode, commands, 4, (uint8_t *)outDataBuffer, outDataBufferSize, 32);
        }
    #endif
        break;

    case READ:
        //03h A23-A16 A15-A8 A7-A0 (D7-D0) (Next byte) continuous
        if (norObject->context.use4BytesAddress == true)
        {
            if (QuadmodeEnable)
            {
                if (DTRmodeEnable)
                {
                    cmd = READ4B4B_DTR_CMD;
                    dtrmode = SPI_DTRMODE_1;
                }
                else
                    cmd = READ4B4B_CMD;

                iomode = SPI_IOMODE_4;
                mmpAxiSpiSetDummyByte(NOR_BUS, 6);
            }
            else
            {
                cmd = FAST_READ_CMD;
                iomode = SPI_IOMODE_5;
                mmpAxiSpiSetDummyByte(NOR_BUS, 8);
            }
        }
        else
        {
            if (QuadmodeEnable)
            {
                if (DTRmodeEnable)
                {
                    cmd = QuadREAD_DTR_CMD;
                    dtrmode = SPI_DTRMODE_1;
                }
                else
                    cmd = QuadREAD_CMD;

                iomode = SPI_IOMODE_4;
                mmpAxiSpiSetDummyByte(NOR_BUS, 6);
            }
            else
            {
                cmd = FAST_READ_CMD;
                iomode = SPI_IOMODE_5;
                mmpAxiSpiSetDummyByte(NOR_BUS, 8);
            }
        }

        commands[0] = cmd;
        if (norObject->context.use4BytesAddress == true)
        {
            commands[1]     = arg >> 24;
            commands[2]     = arg >> 16;
            commands[3]     = arg >> 8;
            commands[4]     = arg;
            commandDataSize = 5;
        }
        else
        {
            commands[1]     = arg >> 24;
            commands[2]     = arg >> 16;
            commands[3]     = arg >> 8;
            commands[4]     = arg;
            commandDataSize = 4;
        }

    #if defined(_WIN32) || (defined(__OPENRTOS__) || defined(__FREERTOS__))
        if (norObject->context.use4BytesAddress == true)
        {
            if (outDataBufferSize >= 16)
            {
                result = mmpAxiSpiDmaRead(norObject->port, iomode, dtrmode, commands, 1, &arg, 4, (uint8_t *)outDataBuffer, outDataBufferSize);
            }
            else
            {
                result = mmpAxiSpiPioRead(norObject->port, iomode, dtrmode, commands, 1, &arg, 4, (uint32_t *)outDataBuffer, outDataBufferSize);
            }
        }
        else
        {
            if (outDataBufferSize >= 16)
            {
                result = mmpAxiSpiDmaRead(norObject->port, iomode, dtrmode, commands, 1, &arg, 3, (uint8_t *)outDataBuffer, outDataBufferSize);
            }
            else
            {
                result = mmpAxiSpiPioRead(norObject->port, iomode, dtrmode, commands, 1, &arg, 3, (uint32_t *)outDataBuffer, outDataBufferSize);
            }
        }
    #else
        if (norObject->context.use4BytesAddress == true)
        {
            result = mmpAxiSpiPioRead(norObject->port, iomode, dtrmode, commands, commandDataSize, (uint8_t *)outDataBuffer, outDataBufferSize, 32);
        }
        else
        {
            result = mmpAxiSpiPioRead(norObject->port, iomode, dtrmode, commands, 4, (uint8_t *)outDataBuffer, outDataBufferSize, 32);
        }
    #endif
        break;

    case FAST_READ:
        //0Bh A23-A16 A15-A8 A7-A0 dummy (D7-D0) (Next Byte) continuous
        //TODO
        if (norObject->context.use4BytesAddress == true)
        {
            cmd = FAST_READ4B_CMD;
        }
        else
        {
            cmd = FAST_READ_CMD;
        }
        commands[0] = cmd;
        if (norObject->context.use4BytesAddress == true)
        {
            commands[1]     = arg >> 24;
            commands[2]     = arg >> 16;
            commands[3]     = arg >> 8;
            commands[4]     = arg;
            commandDataSize = 5;
        }
        else
        {
            commands[1]     = arg >> 24;
            commands[2]     = arg >> 16;
            commands[3]     = arg >> 8;
            commands[4]     = arg;
            commands[5]     = 0x00; // dummy byte
            commandDataSize = 5;
        }
    #if defined(_WIN32) || (defined(__OPENRTOS__) || defined(__FREERTOS__))
        if (norObject->context.use4BytesAddress == true)
        {
            result = mmpAxiSpiPioRead(norObject->port, iomode, dtrmode, commands, 1, &arg, 4, (uint32_t *)outDataBuffer, outDataBufferSize);
        }
        else
        {
            result = mmpAxiSpiPioRead(norObject->port, iomode, dtrmode, commands, 1, &arg, 3, (uint32_t *)outDataBuffer, outDataBufferSize);
        }
    #else
        if (norObject->context.use4BytesAddress == true)
        {
            result = mmpAxiSpiPioRead(norObject->port, iomode, dtrmode, commands, commandDataSize, (uint8_t *)outDataBuffer, outDataBufferSize, 8);
        }
        else
        {
            result = mmpAxiSpiPioRead(norObject->port, iomode, dtrmode, commands, 4, (uint8_t *)outDataBuffer, outDataBufferSize, 32);
        }
    #endif
        break;

    case READ_STATUS:
        //05h (S7-S0)(1)
        commands[0] = READ_STATUS_CMD;
        mmpAxiSpiPioRead(norObject->port, iomode, dtrmode, commands, 1, 0, 0, (uint32_t *)outDataBuffer, outDataBufferSize);
        break;

     case READ_STATUS_2:
        //35h (S15-S8)(1)
        commands[0] = READ_STATUS_2_CMD;
        mmpAxiSpiPioRead(norObject->port, iomode, dtrmode, commands, 1, 0, 0, (uint32_t *)outDataBuffer, outDataBufferSize);
        break;

    case READ_ID:
        //9Fh (M7-M0) (ID15-ID8) (ID7-ID0)
        commands[0] = JEDEC_READ_ID;
        result      = mmpAxiSpiPioRead(norObject->port, iomode, dtrmode, commands, 1, 0, 0, (uint32_t *)outDataBuffer, 5);
        break;

    case READ_DEVICE:
        //90h dummy dummy 00h (M7-M0) (ID7-ID0)
        commands[0] = READ_ID_CMD;
        commands[1] = arg >> 24;
        commands[2] = arg >> 16;
        commands[3] = arg >> 8;
        commands[4] = arg;
        result      = mmpAxiSpiPioRead(norObject->port, iomode, dtrmode, commands, 1, &arg, 3, (uint32_t *)outDataBuffer, 2);
        break;
    case EN4B_MODE:
        //B7h
        cmd         = EN4B_CMD;
        commands[0] = cmd;
        result      = mmpAxiSpiPioWrite(norObject->port, iomode, commands, 1, 0, 0, 0, 0);
        break;
    default:
        NOR_ERROR_MSG("Unknown command %d\n", cmdType);
        break;
    }

    return result;
}

#else
static bool
norSendCommand(
    NOR_OBJECT      *norObject,
    FLASH_CMD_TYPE  cmdType,
    uint32_t        arg,
    uint32_t        outDataBuffer,
    uint32_t        outDataBufferSize)
{
    bool        result          = true;
    uint8_t     cmd             = 0;
    uint8_t     commands[5]     = {0};
    uint32_t    commandDataSize = 0;

    switch (cmdType)
    {
    case ERASE_ALL:
        //C7h/ 60h
        commands[0] = ERASE_BULK_CMD;
        result      = mmpSpiPioWrite(norObject->port, commands, 1, NULL, 0, 0);
        break;

    case WRITE_EN:
        //06h
        commands[0] = WRITE_EN_CMD;
        result      = mmpSpiPioWrite(norObject->port, commands, 1, NULL, 0, 0);
        break;

    case WRITE_DIS:
        //04h
        commands[0] = WRITE_DIS_CMD;
        result      = mmpSpiPioWrite(norObject->port, commands, 1, NULL, 0, 0);
        break;

    case WRITE_STATUS_EN:
        commands[0] = WRITE_STATUS_EN_CMD;
        result      = mmpSpiPioWrite(norObject->port, commands, 1, NULL, 0, 0);
        break;

    case WRITE_STATUS:
        //01h S7-S0
        commands[0] = WRITE_STATUS_CMD;
        commands[1] = arg;
        result      = mmpSpiPioWrite(norObject->port, commands, 2, NULL, 0, 0);
        break;

    case WRITE_STATUS_2:
        //31h S15-S8
        commands[0] = WRITE_STATUS_2_CMD;
        commands[1] = arg;
        result      = mmpSpiPioWrite(norObject->port, commands, 2, NULL, 0, 0);
        break;

    case ERASE_SECTOR:
        //20h A23-A16 A15-A8 A7-A0
        if (norObject->context.use4BytesAddress == true)
        {
            cmd = ERASE_SECOTR4B_CMD;
        }
        else
        {
            cmd = ERASE_SECOTR_CMD;
        }
        commands[0] = cmd;
        if (norObject->context.use4BytesAddress == true)
        {
            commands[1]     = arg >> 24;
            commands[2]     = arg >> 16;
            commands[3]     = arg >> 8;
            commands[4]     = arg;
            commandDataSize = 5;
        }
        else
        {
            commands[1]     = arg >> 16;
            commands[2]     = arg >> 8;
            commands[3]     = arg;
            commandDataSize = 4;
        }
    #if defined(_WIN32) || (defined(__OPENRTOS__) || defined(__FREERTOS__))
        if (norObject->context.use4BytesAddress)
        {
            result = mmpSpiPioWrite(norObject->port, commands, commandDataSize, 0, 0, 8);
        }
        else
        {
            result = mmpSpiPioWrite(norObject->port, commands, 4, 0, 0, 16);
        }
    #else
        if (norObject->context.use4BytesAddress == true)
        {
            result = mmpSpiPioWrite(norObject->port, commands, commandDataSize, 0, 0, 8);
        }
        else
        {
            result = mmpSpiPioWrite(norObject->port, commands, 4, 0, 0, 32);
        }
    #endif
        break;

    case ERASE_BLOCK:
        //D8h/ 52h A23-A16 A15-A8 A7-A0
        if (norObject->context.use4BytesAddress == true)
        {
            cmd = ERASE_BLOCK4B_CMD;
        }
        else
        {
            cmd = ERASE_BLOCK_CMD;
        }
        commands[0] = cmd;
        if (norObject->context.use4BytesAddress == true)
        {
            commands[1]     = (uint8_t)(arg >> 24);
            commands[2]     = (uint8_t)(arg >> 16);
            commands[3]     = (uint8_t)(arg >> 8);
            commands[4]     = (uint8_t)arg;
            commandDataSize = 5;
        }
        else
        {
            commands[1]     = (uint8_t)(arg >> 16);
            commands[2]     = (uint8_t)(arg >> 8);
            commands[3]     = (uint8_t)arg;
            commandDataSize = 4;
        }
    #if defined(_WIN32) || (defined(__OPENRTOS__) || defined(__FREERTOS__))
        if (norObject->context.use4BytesAddress == true)
        {
            result = mmpSpiPioWrite(norObject->port, commands, commandDataSize, 0, 0, 8);
        }
        else
        {
            result = mmpSpiPioWrite(norObject->port, commands, 4, 0, 0, 16);
        }
    #else
        if (norObject->context.use4BytesAddress == true)
        {
            result = mmpSpiPioWrite(norObject->port, commands, commandDataSize, 0, 0, 8);
        }
        else
        {
            result = mmpSpiPioWrite(norObject->port, commands, 4, 0, 0, 32);
        }
    #endif
        break;

    case AUTO_WRITE:
        break;

    case AUTO_WRITE_DATA_ONLY:
        break;

    case WRITE:
        //02h A23-A16 A15-A8 A7-A0 D7-D0 (Next byte) continuous
        if (norObject->context.use4BytesAddress == true)
        {
            cmd = PAGE_PROGRAM4B_CMD;
        }
        else
        {
            cmd = PAGE_PROGRAM_CMD;
        }
        commands[0] = cmd;
        if (norObject->context.use4BytesAddress == true)
        {
            commands[1]     = (uint8_t)(arg >> 24);
            commands[2]     = (uint8_t)(arg >> 16);
            commands[3]     = (uint8_t)(arg >> 8);
            commands[4]     = (uint8_t)arg;
            commandDataSize = 5;
        }
        else
        {
            commands[1]     = (uint8_t)(arg >> 16);
            commands[2]     = (uint8_t)(arg >> 8);
            commands[3]     = (uint8_t)arg;
            commandDataSize = 4;
        }
    #if defined(_WIN32) || (defined(__OPENRTOS__) || defined(__FREERTOS__))
        if (norObject->context.use4BytesAddress == true)
        {
            if (outDataBufferSize >= 16 || (outDataBufferSize & 1))
            {
                {
                    cmd     = WRITE_EN_CMD;
                    result  = mmpSpiPioWrite(norObject->port, &cmd, 1, NULL, 0, 0);
                }

                result = mmpSpiDmaWrite(norObject->port, commands, commandDataSize, (uint8_t *)outDataBuffer, outDataBufferSize, 8);
            }
            else
            {
                {
                    cmd     = WRITE_EN_CMD;
                    result  = mmpSpiPioWrite(norObject->port, &cmd, 1, NULL, 0, 0);
                }

                result = mmpSpiPioWrite(norObject->port, commands, commandDataSize, (uint8_t *)outDataBuffer, outDataBufferSize, 8);
            }
        }
        else
        {
            if (outDataBufferSize >= 32 /*g_norAddrMap.bytesPerPage*/ || (outDataBufferSize & 1))
            {
                {
                    cmd     = WRITE_EN_CMD;
                    result  = mmpSpiPioWrite(norObject->port, &cmd, 1, NULL, 0, 0);
                }

                result = mmpSpiDmaWrite(norObject->port, commands, 4, (uint8_t *)outDataBuffer, outDataBufferSize, 8);
            }
            else
            {
                {
                    cmd     = WRITE_EN_CMD;
                    result  = mmpSpiPioWrite(norObject->port, &cmd, 1, NULL, 0, 0);
                }

                result = mmpSpiPioWrite(norObject->port, commands, 4, (uint8_t *)outDataBuffer, outDataBufferSize, 16);
            }
        }
    #else
        if (norObject->context.use4BytesAddress == true)
        {
            result = mmpSpiPioWrite(norObject->port, commands, commandDataSize, (uint8_t *)outDataBuffer, outDataBufferSize, 32);
        }
        else
        {
            result = mmpSpiPioWrite(norObject->port, commands, 4, (uint8_t *)outDataBuffer, outDataBufferSize, 32);
        }
    #endif
        break;

    case READ:
        //03h A23-A16 A15-A8 A7-A0 (D7-D0) (Next byte) continuous
        if (norObject->context.use4BytesAddress == true)
        {
            cmd = READ4B_CMD;
        }
        else
        {
            cmd = READ_CMD;
        }
        commands[0] = cmd;
        if (norObject->context.use4BytesAddress == true)
        {
            commands[1]     = arg >> 24;
            commands[2]     = arg >> 16;
            commands[3]     = arg >> 8;
            commands[4]     = arg;
            commandDataSize = 5;
        }
        else
        {
            commands[1]     = arg >> 16;
            commands[2]     = arg >> 8;
            commands[3]     = arg;
            commandDataSize = 4;
        }
    #if defined(_WIN32) || (defined(__OPENRTOS__) || defined(__FREERTOS__))
        if (norObject->context.use4BytesAddress == true)
        {
            if (outDataBufferSize >= 16 || (outDataBufferSize & 1))
            {
                result = mmpSpiDmaRead(norObject->port, commands, commandDataSize, (uint8_t *)outDataBuffer, outDataBufferSize, 8);
            }
            else
            {
                result = mmpSpiPioRead(norObject->port, commands, commandDataSize, (uint8_t *)outDataBuffer, outDataBufferSize, 8);
            }
        }
        else
        {
            if (outDataBufferSize >= 16 || (outDataBufferSize & 1))
            {
                result = mmpSpiDmaRead(norObject->port, commands, commandDataSize, (uint8_t *)outDataBuffer, outDataBufferSize, 8);
            }
            else
            {
                result = mmpSpiPioRead(norObject->port, commands, commandDataSize, (uint8_t *)outDataBuffer, outDataBufferSize, 16);
            }
        }
    #else
        if (norObject->context.use4BytesAddress == true)
        {
            result = mmpSpiPioRead(norObject->port, commands, commandDataSize, (uint8_t *)outDataBuffer, outDataBufferSize, 32);
        }
        else
        {
            result = mmpSpiPioRead(norObject->port, commands, commandDataSize, (uint8_t *)outDataBuffer, outDataBufferSize, 32);
        }
    #endif
        break;

    case FAST_READ:
        //0Bh A23-A16 A15-A8 A7-A0 dummy (D7-D0) (Next Byte) continuous
        //TODO
        if (norObject->context.use4BytesAddress == true)
        {
            cmd = FAST_READ4B_CMD;
        }
        else
        {
            cmd = FAST_READ_CMD;
        }
        commands[0] = cmd;
        if (norObject->context.use4BytesAddress == true)
        {
            commands[1]     = arg >> 24;
            commands[2]     = arg >> 16;
            commands[3]     = arg >> 8;
            commands[4]     = arg;
            commandDataSize = 5;
        }
        else
        {
            commands[1]     = arg >> 16;
            commands[2]     = arg >> 8;
            commands[3]     = arg;
            commands[4]     = 0x00; // dummy byte
            commandDataSize = 5;
        }
    #if defined(_WIN32) || (defined(__OPENRTOS__) || defined(__FREERTOS__))
        if (norObject->context.use4BytesAddress == true)
        {
            result = mmpSpiPioRead(norObject->port, commands, commandDataSize, (uint8_t *)outDataBuffer, outDataBufferSize, 8);
        }
        else
        {
            result = mmpSpiPioRead(norObject->port, commands, 4, (uint8_t *)outDataBuffer, outDataBufferSize, 16);
        }
    #else
        if (norObject->context.use4BytesAddress == true)
        {
            result = mmpSpiPioRead(norObject->port, commands, commandDataSize, (uint8_t *)outDataBuffer, outDataBufferSize, 8);
        }
        else
        {
            result = mmpSpiPioRead(norObject->port, commands, 4, (uint8_t *)outDataBuffer, outDataBufferSize, 32);
        }
    #endif
        break;

    case READ_STATUS:
        //05h (S7-S0)(1)
        commands[0] = READ_STATUS_CMD;
        mmpSpiPioRead(norObject->port, commands, 1, (uint8_t *)outDataBuffer, 1, 0);
        break;

    case READ_STATUS_2:
        //35h (S15-S8)(1)
        commands[0] = READ_STATUS_2_CMD;
        mmpSpiPioRead(norObject->port, commands, 1, (uint8_t *)outDataBuffer, 1, 0);
        break;

    case READ_ID:
        //9Fh (M7-M0) (ID15-ID8) (ID7-ID0)
        commands[0] = JEDEC_READ_ID;
        result      = mmpSpiPioRead(norObject->port, commands, 1, (uint8_t *)outDataBuffer, 5, 0);
        break;

    case READ_DEVICE:
        //90h dummy dummy 00h (M7-M0) (ID7-ID0)
        commands[0] = READ_ID_CMD;
        commands[1] = arg >> 16;
        commands[2] = arg >> 8;
        commands[3] = arg;
        result      = mmpSpiPioRead(norObject->port, commands, 4, (uint8_t *)outDataBuffer, 2, 0);
        break;
    case EN4B_MODE:
        //B7h
        cmd         = EN4B_CMD;
        commands[0] = cmd;
        result      = mmpSpiPioWrite(norObject->port, commands, 1, 0, 0, 8);
        break;
    default:
        NOR_ERROR_MSG("Unknown command %d\n", cmdType);
        break;
    }

    return result;
}

#endif

static uint32_t
norWaitReady(
    NOR_OBJECT  *norObject,
    uint32_t    sleepTime)
{
    int32_t     timeOut = SSP_POLLING_COUNT;
    uint32_t    state   = NOR_ERROR_STATUS_BUSY;
    uint8_t     value   = 0;

#if defined (CFG_NOR_USE_AXISPI)

    if (norSendCommand(norObject, READ_STATUS, 0, 0, 0))
    {
        state = NOR_DEVICE_READY;
    }
    else
        printf("-- nor busy --\n");
#else
    while (1)
    {
        norSendCommand(norObject, READ_STATUS, 0, (uint32_t)&value, 1);
        if (value & NOR_DEVICE_BUSY)
        {
            if (sleepTime)
            {
                usleep(sleepTime);
            }
            timeOut--;
            if (timeOut < 0)
            {
                state = NOR_ERROR_DEVICE_TIMEOUT;
                break;
            }
            else
            {
                continue;
            }
        }
        else
        {
            state = NOR_DEVICE_READY;
            break;
        }
    }
#endif
    return state;
}

static bool
norErase(
    NOR_OBJECT  *norObject,
    uint32_t    address)
{
    bool result = true;

    do
    {
        // Write enable
        if (norSendCommand(norObject, WRITE_EN, 0, 0, 0) == false)
        {
            NOR_ERROR_MSG("Send WRITE_EN fail.\n");
            result = false;
            break;
        }
        // Erase
        if (norSendCommand(norObject, ERASE_BLOCK, address, 0, 0) == false)
        {
            NOR_ERROR_MSG("Send ERASE_BLOCK fail.\n");
            result = false;
            break;
        }
    } while (0);

    // Wait erase finished
    if (norWaitReady(norObject, 100))
    {
        NOR_ERROR_MSG("Wait nor ready fail\n");
        result = false;
    }

    return result;
}

static bool
norEnableWrite(
    NOR_OBJECT *norObject)
{
    uint32_t    readStatus  = 0;
    bool        result      = true;

    do
    {
        // Unlock write protected
        if (norSendCommand(norObject, WRITE_EN, 0, 0, 0) == false)
        {
            NOR_ERROR_MSG("Send WRITE_EN fail.\n");
            result = false;
            break;
        }
        if (norSendCommand(norObject, WRITE_STATUS, 0, norObject->context.status, 1) == false)
        {
            NOR_ERROR_MSG("Send WRITE_STATUS fail.\n");
            result = false;
            break;
        }
        if (norWaitReady(norObject, 0))
        {
            NOR_ERROR_MSG("Wait nor ready fail.\n");
            result = false;
            break;
        }
        if (norObject->context.status >> 8)
        {
            if (norSendCommand(norObject, WRITE_EN, 0, 0, 0) == false)
            {
                NOR_ERROR_MSG("Send WRITE_EN fail.\n");
                result = false;
                break;
            }
            if (norSendCommand(norObject, WRITE_STATUS_2, 0, norObject->context.status >> 8, 1) == false)
            {
                NOR_ERROR_MSG("Send WRITE_STATUS_2 fail.\n");
                result = false;
                break;
            }
            if (norWaitReady(norObject, 0))
            {
                NOR_ERROR_MSG("Wait nor ready fail.\n");
                result = false;
                break;
            }
        }

        if (norSendCommand(norObject, READ_STATUS, 0, (uint32_t)&readStatus, 1) == false)
        {
            NOR_ERROR_MSG("Send READ_STATUS fail.\n");
            result = false;
            break;
        }
        if ((readStatus & NOR_WRITE_PROTECT) != 0)
        {
            NOR_ERROR_MSG("Norflash in write protect mode.\n");
            result = false;
            break;
        }
    } while (0);

    return result;
}

static bool
norDisableWrite(
    NOR_OBJECT *norObject)
{
    uint32_t    readStatus  = 0;
    bool        result      = true;

    do
    {
        // Unlock write protected
        if (norSendCommand(norObject, WRITE_DIS, 0, 0, 0) == false)
        {
            NOR_ERROR_MSG("Send WRITE_DIS fail.\n");
            result = false;
            break;
        }
        if (norWaitReady(norObject, 0))
        {
            NOR_ERROR_MSG("Wait nor ready fail.\n");
            result = false;
            break;
        }
    } while (0);

    return result;
}

static bool
norWrite(
    NOR_OBJECT  *norObject,
    uint32_t    address,
    uint8_t     *inData,
    uint32_t    inDataSize)
{
    bool result = true;

    do
    {
        uint32_t    remainderSize       = inDataSize;
        uint32_t    chunkSize           = 0;
        uint32_t    currWriteAddress    = address;
        uint8_t     *currWritePtr       = inData;

        while (remainderSize)
        {
            chunkSize = (remainderSize >= 256) ? 256 : remainderSize;

            if (norSendCommand(norObject, WRITE, currWriteAddress, (uint32_t)currWritePtr, chunkSize) == false)
            {
                NOR_ERROR_MSG("Send WRITE fail.\n");
                result = false;
                break;
            }

            if (norWaitReady(norObject, 0))
            {
                NOR_ERROR_MSG("Wait nor ready fail\n");
                result = false;
            }

            currWriteAddress    += chunkSize;
            currWritePtr        += chunkSize;
            remainderSize       -= chunkSize;
        }
    } while (0);

    return result;
}

static bool
norRead(
    NOR_OBJECT  *norObject,
    uint32_t    address,
    uint8_t     *inData,
    uint32_t    inDataSize)
{
    bool        result          = true;
    uint32_t    remainderSize   = inDataSize;
    uint32_t    currReadAddress = address;
    uint32_t    writeBufferPtr  = (uint32_t)inData;

    while (remainderSize)
    {
#if defined (CFG_NOR_USE_AXISPI)
        uint32_t    chunkSize   = (remainderSize >= (norObject->context.bytesPerSector)) ? (norObject->context.bytesPerSector) : remainderSize;
#else
        uint32_t    chunkSize   = (remainderSize >= (norObject->context.bytesPerSector - 1)) ? (norObject->context.bytesPerSector - 1) : remainderSize;
#endif
        NOR_LOG_MSG("currReadAddress = 0x%08X, chunkSize = %d\n", currReadAddress, chunkSize);
        if (norSendCommand(norObject, READ, currReadAddress, writeBufferPtr, chunkSize) == true)
        {
            currReadAddress += chunkSize;
            writeBufferPtr  += chunkSize;
            remainderSize   -= chunkSize;
        }
        else
        {
            NOR_ERROR_MSG("Read fail in address 0x%08X\n", currReadAddress);
            result = false;
            break;
        }
    }

    return result;
}

bool
norUpdate(
    NOR_OBJECT  *norObject,
    uint32_t    address,
    uint8_t     *inData,
    uint32_t    inDataSize)
{
    bool        result              = true;
    uint32_t    blockStartAddress   = (address / norObject->context.bytesPerSector) * norObject->context.bytesPerSector;

    do
    {
        if (blockStartAddress == address)
        {
            // Doesn't need copy.
            if (norErase(norObject, blockStartAddress) == false)
            {
                NOR_ERROR_MSG("norErase(0x%08X, 0x%08X) fail.\n", norObject, blockStartAddress);
                result = false;
                break;
            }

            if (norWrite(norObject, address, inData, inDataSize) == false)
            {
                NOR_ERROR_MSG("norWrite(0x%08X, 0x%08X, 0x%08X, 0x%08X) fail.\n", norObject, address, inData, inDataSize);
                result = false;
                break;
            }
        }
        else
        {
            // Need copy original and update data.
            uint32_t    dataUpdateOffset    = (address - blockStartAddress);
            //uint8_t*	readBackBuffer = (uint8_t*)itpVmemAlloc(norObject->context.bytesPerSector);
            uint8_t     *readBackBuffer     = (uint8_t *)itpVmemAlignedAlloc(32, norObject->context.bytesPerSector);

            if (readBackBuffer == NULL)
            {
                NOR_ERROR_MSG("Out of memory\n");
                result = false;
                break;
            }

            norRead(norObject, blockStartAddress, readBackBuffer, norObject->context.bytesPerSector);
            memcpy(readBackBuffer + dataUpdateOffset, inData, inDataSize);

            // Doesn't need copy.
            if (norErase(norObject, blockStartAddress) == false)
            {
                NOR_ERROR_MSG("norErase(0x%08X, 0x%08X) fail.\n", norObject, blockStartAddress);
                itpVmemFree((uint32_t)readBackBuffer);
                result = false;
                break;
            }

            if (norWrite(norObject, blockStartAddress, readBackBuffer, norObject->context.bytesPerSector) == false)
            {
                NOR_ERROR_MSG("norWrite(0x%08X, 0x%08X, 0x%08X, 0x%08X) fail.\n", norObject, address, inData, inDataSize);
                itpVmemFree((uint32_t)readBackBuffer);
                result = false;
                break;
            }
        }
    } while (0);

    return result;
}

//=============================================================================
//                              Public Function Definition
//=============================================================================
NOR_RESULT
NorInitial(
    SPI_PORT    port,
    SPI_CSN     chipSelectPin)
{
    uint32_t    norIndex    = port * 2 + chipSelectPin;
    NOR_OBJECT  *norObject  = &NorObjects[norIndex];
    NOR_ID      id          = {0};
    NOR_RESULT  result      = 1;

    pthread_mutex_lock(&norObject->mutex);
    if (norObject->refCount == 0)
    {
        uint32_t    i       = 0;
        bool        foundId = false;

        ithLockMutex(ithStorMutex);
        ithStorageSelect(ITH_STOR_NOR);

#if defined (CFG_NOR_USE_AXISPI)
        if (DTRmodeEnable)
            result = mmpAxiSpiInitialize(NOR_BUS, SPI_OP_MASTR, CPO_0_CPH_0, SPI_CLK_20M, SPI_CLK_DIV_1);
        else
            result = mmpAxiSpiInitialize(NOR_BUS, SPI_OP_MASTR, CPO_0_CPH_0, SPI_CLK_20M, SPI_CLK_DIV_0);
#else
    #ifdef CFG_SPI0_ENABLE
        result = mmpSpiInitialize(SPI_0, SPI_OP_MASTR, CPO_0_CPH_0, SPI_CLK_20M);
    #endif
    #ifdef CFG_SPI1_ENABLE
        result = mmpSpiInitialize(SPI_1, SPI_OP_MASTR, CPO_0_CPH_0, SPI_CLK_20M);
    #endif
#endif

        norReadDeviceID(norObject, &id);
        norDisableWrite(norObject);

        for (i = 0; i < (sizeof(nor_support_vendor) / sizeof(nor_support_vendor[0])); i++)
        {
            if ((id.manufatureID == nor_support_vendor[i].id.manufatureID)
                && (id.deviceID == nor_support_vendor[i].id.deviceID)
                && (id.deviceID2 == nor_support_vendor[i].id.deviceID2 || nor_support_vendor[i].id.deviceID2 == 0xFF))
            {
                foundId = true;
                break;
            }
        }
        if (foundId == true)
        {
            memcpy(&norObject->id, &id, sizeof(NOR_ID));
            norGetContext(norObject, &nor_support_vendor[i]);

            g_vendor = nor_support_vendor[i].vendorID;

            printf( "\n========================================\n");
            printf( "             NOR (%d, %d) init\n", port, chipSelectPin);
            printf( "========================================\n");
            printf( "Manufacturer    : 0x%02X\n",       nor_support_vendor[i].id.manufatureID);
            printf( "Device ID1      : 0x%04X\n",       nor_support_vendor[i].id.deviceID);
            printf( "Device ID2      : 0x%02X\n",       nor_support_vendor[i].id.deviceID2);
            printf( "Name            : %s\n",           nor_support_vendor[i].deviceName);
            printf( "Page Size       : %d Bytes\n",     norObject->context.bytesPerPage);
            printf( "Sector Size     : %d Bytes\n",     norObject->context.bytesPerSector);
            printf( "Sector in Block : %d\n",           norObject->context.sectorsPerBlock);
            printf( "Total Blocks    : %d\n",           norObject->context.totalBlocks);
            printf( "Size            : %d MB\n\n",      (uint32_t)norObject->context.bytesPerSector * norObject->context.sectorsPerBlock * norObject->context.totalBlocks / 1048567);
        }
        else
        {
            NOR_ERROR_MSG("Unsupport norflash 0x%02X 0x%04X 0x%02X\n", id.manufatureID, id.deviceID, id.deviceID2);
        }

        if (foundId == true)
        {
            norObject->refCount = 1;
            if (norObject->context.use4BytesAddress)
            {
                printf("4 byte mode is on+\n");
                norSendCommand(norObject, EN4B_MODE, 0, 0, 0);
                printf("4 byte mode is on-\n");
            }
        }

        if (QuadmodeEnable)
        {
            printf("Quad mode is on+\n");
            norSendCommand(norObject, WRITE_EN, 0, 0, 0);
            norSendCommand(norObject, WRITE_STATUS, 0, norObject->context.status, 1);
            if (norWaitReady(norObject, 100))
            {
                printf("--- NOR wait Ready Fail !!!! ---\n");
            }
            if (norObject->context.status >> 8)
            {
                norSendCommand(norObject, WRITE_EN, 0, 0, 0);
                norSendCommand(norObject, WRITE_STATUS_2, 0, norObject->context.status >> 8, 1);
                if (norWaitReady(norObject, 100))
                {
                    printf("--- NOR wait Ready Fail !!!! ---\n");
                }
            }
            printf("Quad mode is on-\n");
        }

        ithUnlockMutex(ithStorMutex);
    }
    else
    {
        norObject->refCount++;
    }

    pthread_mutex_unlock(&norObject->mutex);

    if (norObject->refCount > 0)
        result = 0;

    return (result);
}

NOR_RESULT
NorTerminate(
    SPI_PORT    port,
    SPI_CSN     chipSelectPin)
{
    uint32_t    norIndex    = port * 2 + chipSelectPin;
    NOR_OBJECT  *norObject  = &NorObjects[norIndex];
    NOR_RESULT  result      = 0;

    pthread_mutex_lock(&norObject->mutex);
    if (--norObject->refCount == 0)
    {
        pthread_mutex_t mutex = norObject->mutex;

        if (norObject->protectCount && norObject->protectInfo)
            itpVmemFree((uint32_t)(norObject->protectInfo));

        memset(norObject, 0x00, sizeof(NOR_OBJECT));
        norObject->mutex = mutex;
    }
    pthread_mutex_unlock(&norObject->mutex);
    return result;
}

bool
NorErase(
    SPI_PORT    port,
    SPI_CSN     chipSelectPin,
    uint32_t    address,
    uint32_t    eraseLength)
{
    uint32_t    norIndex    = port * 2 + chipSelectPin;
    NOR_OBJECT  *norObject  = &NorObjects[norIndex];
    bool        result      = true;

    pthread_mutex_lock(&norObject->mutex);
    ithLockMutex(ithStorMutex);
    ithStorageSelect(ITH_STOR_NOR);

    do
    {
        uint32_t    headBlockStart      = (address / norObject->context.bytesPerSector) * norObject->context.bytesPerSector;
        uint32_t    headBlockEnd        = headBlockStart + norObject->context.bytesPerSector;
        uint32_t    tailBlockStart      = ((address + eraseLength - 1) / norObject->context.bytesPerSector) * norObject->context.bytesPerSector;
        uint32_t    tailBlockEnd        = ((address + eraseLength) == tailBlockStart) ? tailBlockStart : (tailBlockStart + norObject->context.bytesPerSector);
        uint32_t    programBlockCount   = (tailBlockEnd - headBlockStart) / norObject->context.bytesPerSector;
        uint32_t    i                   = 0;
        uint32_t    currNorAddress      = headBlockStart;

        if (headBlockStart != address)
        {
            NOR_ERROR_MSG("Input address(0x%08X) not 64KB alignment. Erase from 0x%08X to 0x%08X\n", headBlockStart, tailBlockEnd);
        }

        if (norEnableWrite(norObject) == false)
        {
            NOR_ERROR_MSG("norEnableWrite() fail.\n");
            result = false;
            break;
        }

        for (i = 0; i < programBlockCount; i++, currNorAddress += norObject->context.bytesPerSector)
        {
            if (norErase(norObject, currNorAddress) == false)
            {
                NOR_ERROR_MSG("norErase(0x%08X, 0x%08X) fail.\n", norObject, currNorAddress);
                result = false;
                break;
            }
        }

        if (norDisableWrite(norObject) == false)
        {
            NOR_ERROR_MSG("norDisableWrite() fail.\n");
            result = false;
            break;
        }
    } while (0);

    ithUnlockMutex(ithStorMutex);
    pthread_mutex_unlock(&norObject->mutex);

    return result;
}

NOR_RESULT
NorEraseAll(
    SPI_PORT    port,
    SPI_CSN     chipSelectPin)
{
    NOR_RESULT  result      = 0;
    uint32_t    norIndex    = port * 2 + chipSelectPin;
    NOR_OBJECT  *norObject  = &NorObjects[norIndex];

    pthread_mutex_lock(&norObject->mutex);
    ithLockMutex(ithStorMutex);
    ithStorageSelect(ITH_STOR_NOR);

    do
    {
        if (norSendCommand(norObject, WRITE_EN, 0, 0, 0) == false)
        {
            NOR_ERROR_MSG("Send WRITE_EN fail.\n");
            result = 1;
            break;
        }

        // Erase all
        if (norSendCommand(norObject, ERASE_ALL, 0, 0, 0) == false)
        {
            NOR_ERROR_MSG("Send ERASE_ALL fail.\n");
            result = 1;
            break;
        }

        // Wait erase finished
        if (norWaitReady(norObject, 100))
        {
            NOR_ERROR_MSG("Wait nor ready fail\n");
            result = 1;
        }

        if (norSendCommand(norObject, WRITE_DIS, 0, 0, 0) == false)
        {
            NOR_ERROR_MSG("Send WRITE_DIS fail.\n");
            result = 1;
            break;
        }
    } while (0);

    ithUnlockMutex(ithStorMutex);
    pthread_mutex_unlock(&norObject->mutex);

    return result;
}

NOR_RESULT
NorRead(
    SPI_PORT    port,
    SPI_CSN     chipSelectPin,
    uint32_t    address,
    uint8_t     *outData,
    uint32_t    outDataSize)
{
    NOR_RESULT  result          = 0;
    uint32_t    norIndex        = port * 2 + chipSelectPin;
    NOR_OBJECT  *norObject      = &NorObjects[norIndex];
    uint32_t    remainderSize   = outDataSize;
    uint32_t    currReadAddress = address;
    uint32_t    writeBufferPtr  = (uint32_t)outData;

    if ((address + outDataSize) > (norObject->context.bytesPerSector * norObject->context.sectorsPerBlock * norObject->context.totalBlocks))
    {
        NOR_ERROR_MSG("Read from 0x%08X to 0x%08X, out of boundary.\n", address, address + outDataSize);
        return 1;
    }

    pthread_mutex_lock(&norObject->mutex);
    ithLockMutex(ithStorMutex);
    ithStorageSelect(ITH_STOR_NOR);

    result = norRead(norObject, address, outData, outDataSize);

    ithUnlockMutex(ithStorMutex);
    pthread_mutex_unlock(&norObject->mutex);

    if (result == true)
        result = 0;
    else
        result = 1;

    return result;
}

NOR_RESULT
NorSWProtect(
    SPI_PORT            port,
    SPI_CSN             chipSelectPin,
    uint32_t            totalCount,
    NOR_PROTECT_ITEM    *protectInfo)
{
    uint32_t    norIndex   = port * 2 + chipSelectPin;
    NOR_OBJECT  *norObject = &NorObjects[norIndex];
    NOR_RESULT  result     = 0;

    if ((!totalCount && protectInfo) || (totalCount && !protectInfo))
    {
        NOR_ERROR_MSG("totalCount: %d, protectInfo: %08X, invalid parameter.\n", totalCount, protectInfo);
        return 1;
    }

    pthread_mutex_lock(&norObject->mutex);

    if (norObject->protectCount && norObject->protectInfo)
        itpVmemFree((uint32_t)(norObject->protectInfo));

    if (totalCount)
    {
        norObject->protectInfo = (NOR_PROTECT_ITEM *)itpVmemAlignedAlloc(32, totalCount * sizeof(NOR_PROTECT_ITEM));

        if (norObject->protectInfo == NULL)
        {
            NOR_ERROR_MSG("Out of memory\n");
            pthread_mutex_unlock(&norObject->mutex);
            return 1;
        }

        memcpy(norObject->protectInfo, protectInfo, totalCount * sizeof(NOR_PROTECT_ITEM));
    }
    else
    {
        norObject->protectInfo = protectInfo;
    }
    norObject->protectCount = totalCount;

    pthread_mutex_unlock(&norObject->mutex);

    return result;
}

NOR_RESULT
NorWrite(
    SPI_PORT    port,
    SPI_CSN     chipSelectPin,
    uint32_t    address,
    uint8_t     *inData,
    uint32_t    inDataSize)
{
    uint32_t         norIndex   = port * 2 + chipSelectPin;
    NOR_OBJECT       *norObject = &NorObjects[norIndex];
    uint32_t         SWPIndex   = 0;
    NOR_PROTECT_ITEM *SWPITemp  = NULL;
    NOR_RESULT       result     = 0;

    if (inData == NULL || inDataSize == 0)
    {
        NOR_ERROR_MSG("inData = 0x%08X, inDataSize = %d, nothing to write.\n", inData, inDataSize);
        return 1;
    }

    if ((address + inDataSize) > (norObject->context.bytesPerSector * norObject->context.sectorsPerBlock * norObject->context.totalBlocks))
    {
        NOR_ERROR_MSG("Write from 0x%08X to 0x%08X, out of boundary.\n", address, address + inDataSize);
        return 1;
    }

    pthread_mutex_lock(&norObject->mutex);

    if (norObject->protectCount && norObject->protectInfo)
    {
        for (SWPIndex = 0; SWPIndex < norObject->protectCount; SWPIndex++)
        {
            SWPITemp = norObject->protectInfo + SWPIndex;
            if (SWPITemp->protectSize)
            {
                if (address >= SWPITemp->protectStartAddr + SWPITemp->protectSize ||
                        SWPITemp->protectStartAddr >= address + inDataSize)
                {
                    continue;
                }
                else
                {
                    NOR_ERROR_MSG("address = 0x%08X, inDataSize = %d, nothing to write because SW protect[%d]:{0x%X,%d}\n",
                            address, inDataSize, SWPIndex, SWPITemp->protectStartAddr, SWPITemp->protectSize);
                    pthread_mutex_unlock(&norObject->mutex);
                    return 1;
                }

            }
        }
    }

    ithLockMutex(ithStorMutex);
    ithStorageSelect(ITH_STOR_NOR);

    do
    {
        uint32_t    headBlockStart      = (address / (norObject->context.bytesPerSector * norObject->context.sectorsPerBlock)) * (norObject->context.bytesPerSector * norObject->context.sectorsPerBlock);
        uint32_t    headBlockEnd        = headBlockStart + (norObject->context.bytesPerSector * norObject->context.sectorsPerBlock);
        uint32_t    headBlockAddrOffset = address - headBlockStart;
        uint32_t    tailBlockStart      = ((address + inDataSize - 1) / (norObject->context.bytesPerSector * norObject->context.sectorsPerBlock)) * (norObject->context.bytesPerSector * norObject->context.sectorsPerBlock);
        uint32_t    tailBlockEnd        = ((address + inDataSize) == tailBlockStart) ? tailBlockStart : (tailBlockStart + (norObject->context.bytesPerSector * norObject->context.sectorsPerBlock));
        uint32_t    tailBlockAddrOffset = (address + inDataSize) - tailBlockStart;
        uint32_t    programBlockCount   = (tailBlockEnd - headBlockStart) / (norObject->context.bytesPerSector * norObject->context.sectorsPerBlock);

        NOR_LOG_MSG("headBlockStart = 0x%08X\n",        headBlockStart);
        NOR_LOG_MSG("headBlockEnd = 0x%08X\n",          headBlockEnd);
        NOR_LOG_MSG("headBlockAddrOffset = 0x%08X\n",   headBlockAddrOffset);
        NOR_LOG_MSG("tailBlockStart = 0x%08X\n",        tailBlockStart);
        NOR_LOG_MSG("tailBlockEnd = 0x%08X\n",          tailBlockEnd);
        NOR_LOG_MSG("tailBlockAddrOffset = 0x%08X\n",   tailBlockAddrOffset);
        NOR_LOG_MSG("programBlockCount = %d\n",         programBlockCount);

        if (norEnableWrite(norObject) == false)
        {
            NOR_ERROR_MSG("norEnableWrite() fail.\n");
            result = 1;
            break;
        }

        if (programBlockCount > 1)
        {
            uint32_t    i               = 0;
            uint32_t    currNorAddress  = address;
            uint8_t     *currDataPtr    = inData;

            //printf("1. ## cNorAddr = 0x%x, currDataPtr = 0x%x ##\n", currNorAddress, currDataPtr);

            for (i = 0; i < programBlockCount; i++)
            {
                if (i == 0)
                {
                    //printf("2. ## cNorAddr = 0x%x, currDataPtr = 0x%x ##\n", currNorAddress, currDataPtr);
                    //printf("2. ## update size = %d ##\n", (headBlockEnd - address));
                    if (norUpdate(norObject, currNorAddress, currDataPtr, (headBlockEnd - address)) == false)
                    {
                        result = 1;
                        break;
                    }
                    currNorAddress  += (headBlockEnd - address);
                    currDataPtr     += (headBlockEnd - address);
                }
                else if (i == (programBlockCount - 1))
                {
                    //printf("3. ## cNorAddr = 0x%x, currDataPtr = 0x%x ##\n", currNorAddress, currDataPtr);
                    //printf("3. ## tailBlockAddrOffset = %d ##\n", (tailBlockAddrOffset));
                    if (norUpdate(norObject, currNorAddress, currDataPtr, tailBlockAddrOffset) == false)
                    {
                        result = 1;
                        break;
                    }
                }
                else
                {
                    //printf("4. ## cNorAddr = 0x%x, currDataPtr = 0x%x ##\n", currNorAddress, currDataPtr);
                    //printf("4. ## update size = %d ##\n", norObject->context.bytesPerSector * norObject->context.sectorsPerBlock);
                    //printf("4. ## [%x], [%x], [%x], [%x]##\n ", *currDataPtr, *(currDataPtr+1), *(currDataPtr+2), *(currDataPtr+3));
                    if (norUpdate(norObject, currNorAddress, currDataPtr, norObject->context.bytesPerSector * norObject->context.sectorsPerBlock) == false)
                    {
                        result = 1;
                        break;
                    }
                    currNorAddress  += norObject->context.bytesPerSector * norObject->context.sectorsPerBlock;
                    currDataPtr     += norObject->context.bytesPerSector * norObject->context.sectorsPerBlock;
                }
            }
        }
        else
        {
            if (norUpdate(norObject, address, inData, inDataSize) == false)
            {
                result = 1;
                break;
            }
        }

        if (norDisableWrite(norObject) == false)
        {
            NOR_ERROR_MSG("norDisableWrite() fail.\n");
            result = 1;
            break;
        }
    } while (0);

    ithUnlockMutex(ithStorMutex);
    pthread_mutex_unlock(&norObject->mutex);

    //printf(" -- norWrite --\n");

    /*
       if (address == 0x1)
       {
       uint8_t	*readBackBuffer = NULL;
       int i = 0;
       unsigned char RDSR = 0x05;
       unsigned char RDX1 = 0x03;
       readBackBuffer = (uint8_t*)memalign(32, inDataSize);
       mmpSpiPioRead(norObject->port, &RDSR, 1, 0x0, 0x0, 0x0, 0x0);
       printf("-- address = 0x%x, inDatasize = %d --\n", address, inDataSize);
       mmpSpiDmaRead(norObject->port, &RDX1, 1, &address, 3, readBackBuffer, inDataSize);
       for (i = 0 ; i < 64; i++)
        printf("rdbkbuf[%d]=0x%x\n", i , readBackBuffer[i]);

       while(1);
       }*/

    return result;
}

NOR_RESULT
NorWriteWithoutErase(
    SPI_PORT    port,
    SPI_CSN     chipSelectPin,
    uint32_t    address,
    uint8_t     *inData,
    uint32_t    inDataSize)
{
    uint32_t         norIndex   = port * 2 + chipSelectPin;
    NOR_OBJECT       *norObject = &NorObjects[norIndex];
    uint32_t         SWPIndex   = 0;
    NOR_PROTECT_ITEM *SWPITemp  = NULL;
    NOR_RESULT       result     = 0;

    if ((address + inDataSize) > (norObject->context.bytesPerSector * norObject->context.sectorsPerBlock * norObject->context.totalBlocks))
    {
        NOR_ERROR_MSG("Write from 0x%08X to 0x%08X, out of boundary.\n", address, address + inDataSize);
        return 1;
    }

    if (address & 0xFF)
    {
        NOR_ERROR_MSG("Input address(0x%08X) not 256 Bytes alignment.\n", address);
        return 1;
    }

    pthread_mutex_lock(&norObject->mutex);

    if (norObject->protectCount && norObject->protectInfo)
    {
        for (SWPIndex = 0; SWPIndex < norObject->protectCount; SWPIndex++)
        {
            SWPITemp = norObject->protectInfo + SWPIndex;
            if (SWPITemp->protectSize)
            {
                if (address >= SWPITemp->protectStartAddr + SWPITemp->protectSize ||
                        SWPITemp->protectStartAddr >= address + inDataSize)
                {
                    continue;
                }
                else
                {
                    NOR_ERROR_MSG("address = 0x%08X, inDataSize = %d, nothing to write because SW protect[%d]:{0x%X,%d}\n",
                            address, inDataSize, SWPIndex, SWPITemp->protectStartAddr, SWPITemp->protectSize);
                    pthread_mutex_unlock(&norObject->mutex);
                    return 1;
                }

            }
        }
    }

    ithLockMutex(ithStorMutex);
    ithStorageSelect(ITH_STOR_NOR);

    do
    {
        if (norEnableWrite(norObject) == false)
        {
            NOR_ERROR_MSG("norEnableWrite() fail.\n");
            result = 1;
            break;
        }

        if (norWrite(norObject, address, inData, inDataSize) == false)
        {
            NOR_ERROR_MSG("norWrite(0x%08X, 0x%08X, 0x%08X, %d) fail.\n", norObject, address, inData, inDataSize);
            result = 1;
            break;
        }

        if (norDisableWrite(norObject) == false)
        {
            NOR_ERROR_MSG("norDisableWrite() fail.\n");
            result = 1;
            break;
        }
    } while (0);

    ithUnlockMutex(ithStorMutex);
    pthread_mutex_unlock(&norObject->mutex);

    return result;
}

uint8_t
NorGetManufacturerID(
    SPI_PORT    port,
    SPI_CSN     chipSelectPin)
{
    uint32_t    norIndex    = port * 2 + chipSelectPin;
    NOR_OBJECT  *norObject  = &NorObjects[norIndex];
    uint8_t     result      = 0;

    pthread_mutex_lock(&norObject->mutex);
    result = norObject->id.manufatureID;
    pthread_mutex_unlock(&norObject->mutex);

    return result;
}

uint16_t
NorGetDeviceID1(
    SPI_PORT    port,
    SPI_CSN     chipSelectPin)
{
    uint32_t    norIndex    = port * 2 + chipSelectPin;
    NOR_OBJECT  *norObject  = &NorObjects[norIndex];
    uint16_t    result      = 0;

    pthread_mutex_lock(&norObject->mutex);
    result = norObject->id.deviceID;
    pthread_mutex_unlock(&norObject->mutex);

    return result;
}

uint8_t
NorGetDeviceID2(
    SPI_PORT    port,
    SPI_CSN     chipSelectPin)
{
    uint32_t    norIndex    = port * 2 + chipSelectPin;
    NOR_OBJECT  *norObject  = &NorObjects[norIndex];
    uint8_t     result      = 0;

    pthread_mutex_lock(&norObject->mutex);
    result = norObject->id.deviceID2;
    pthread_mutex_unlock(&norObject->mutex);

    return result;
}

const uint32_t
NorGetPageSize(
    SPI_PORT    port,
    SPI_CSN     chipSelectPin)
{
    uint32_t    norIndex    = port * 2 + chipSelectPin;
    NOR_OBJECT  *norObject  = &NorObjects[norIndex];
    uint32_t    result      = 0;

    pthread_mutex_lock(&norObject->mutex);
    result = norObject->context.bytesPerPage;
    pthread_mutex_unlock(&norObject->mutex);

    return result;
}

const uint16_t
NorGetPagePerSector(
    SPI_PORT    port,
    SPI_CSN     chipSelectPin)
{
    uint32_t    norIndex    = port * 2 + chipSelectPin;
    NOR_OBJECT  *norObject  = &NorObjects[norIndex];
    uint16_t    result      = 0;

    pthread_mutex_lock(&norObject->mutex);
    result = norObject->context.pagesPerSector;
    pthread_mutex_unlock(&norObject->mutex);

    return result;
}

const uint32_t
NorGetSectorSize(
    SPI_PORT    port,
    SPI_CSN     chipSelectPin)
{
    uint32_t    norIndex    = port * 2 + chipSelectPin;
    NOR_OBJECT  *norObject  = &NorObjects[norIndex];
    uint32_t    result      = 0;

    pthread_mutex_lock(&norObject->mutex);
    result = norObject->context.bytesPerSector;
    pthread_mutex_unlock(&norObject->mutex);

    return result;
}

const uint16_t
NorGetSectorPerBlock(
    SPI_PORT    port,
    SPI_CSN     chipSelectPin)
{
    uint32_t    norIndex    = port * 2 + chipSelectPin;
    NOR_OBJECT  *norObject  = &NorObjects[norIndex];
    uint16_t    result      = 0;

    pthread_mutex_lock(&norObject->mutex);
    result = norObject->context.sectorsPerBlock;
    pthread_mutex_unlock(&norObject->mutex);

    return result;
}

const uint32_t
NorGetBlockNumber(
    SPI_PORT    port,
    SPI_CSN     chipSelectPin)
{
    uint32_t    norIndex    = port * 2 + chipSelectPin;
    NOR_OBJECT  *norObject  = &NorObjects[norIndex];
    uint32_t    result      = 0;

    pthread_mutex_lock(&norObject->mutex);
    result = norObject->context.totalBlocks;
    pthread_mutex_unlock(&norObject->mutex);

    return result;
}

const uint32_t
NorGetCapacity(
    SPI_PORT    port,
    SPI_CSN     chipSelectPin)
{
    uint32_t    norIndex    = port * 2 + chipSelectPin;
    NOR_OBJECT  *norObject  = &NorObjects[norIndex];
    uint32_t    result      = 0;

    pthread_mutex_lock(&norObject->mutex);
    result = (uint32_t)norObject->context.bytesPerSector * norObject->context.sectorsPerBlock * norObject->context.totalBlocks;
    pthread_mutex_unlock(&norObject->mutex);

    return result;
}

uint32_t
NorGetAttitude(
    SPI_PORT        port,
    SPI_CSN         chipSelectPin,
    NOR_ATTITUDE    atti)
{
    uint32_t    norIndex    = port * 2 + chipSelectPin;
    NOR_OBJECT  *norObject  = &NorObjects[norIndex];
    uint32_t    result      = 0;
    uint32_t    data        = 0;

    pthread_mutex_lock(&norObject->mutex);

    switch (atti)
    {
    case NOR_ATTITUDE_ERASE_UNIT:
        data = (uint32_t)norObject->context.bytesPerSector;
        break;

    case NOR_ATTITUDE_DEVICE_COUNT:
        data = sizeof(nor_support_vendor) / sizeof(nor_support_vendor[0]);
        break;

    case NOR_ATTITUDE_CURRENT_DEVICE_ID:
        data = g_vendor;
        break;

    case NOR_ATTITUDE_PAGE_SIZE:
        data = (uint32_t)norObject->context.bytesPerPage;
        break;

    case NOR_ATTITUDE_PAGE_PER_SECTOR:
        data = (uint32_t)norObject->context.pagesPerSector;
        break;

    case NOR_ATTITUDE_SECTOR_PER_BLOCK:
        data = (uint32_t)norObject->context.sectorsPerBlock;
        break;

    case NOR_ATTITUDE_BLOCK_SIZE:
        data = (uint32_t)norObject->context.totalBlocks;
        break;

    case NOR_ATTITUDE_TOTAL_SIZE:
        data = (uint32_t)(norObject->context.totalBlocks * norObject->context.sectorsPerBlock * norObject->context.pagesPerSector);
        break;

    default:
        break;
    }

    pthread_mutex_unlock(&norObject->mutex);

    return data;
}
