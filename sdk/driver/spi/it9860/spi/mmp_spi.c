//=============================================================================
//                              Include Files
//=============================================================================
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "spi_reg.h"
#include "ite/ith.h"
#include "../../include/ite/itp.h"
//#include "../../include/ite/ith_defs.h"

#include "../../include/ssp/mmp_spi.h"
#include "../../include/ssp/ssp_error.h"
#if defined (__OPENRTOS__)
#elif defined (__FREERTOS__)
    #include "or32.h"
#endif
#include "spi_hw.h"
#include "spi_msg.h"

//=============================================================================
//                              Compile Option
//=============================================================================
//#define ENABLE_SET_SPI_CLOCK_AS_40MHZ
//#define ENABLE_SET_SPI_CLOCK_AS_10MHZ

//=============================================================================
//                              Macro
//=============================================================================
#define TO_SPI_HW_PORT(port) ((port == SPI_0) ? SPI_HW_PORT_0 : SPI_HW_PORT_1)

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
typedef struct _SPI_OBJECT
{
    int                         refCount;
    pthread_mutex_t             mutex;
    SPI_HW_PORT                 hwPort;
    SPI_OP_MODE                 opMode;
    SPI_CONFIG_MAPPING_ENTRY    tMappingEntry;
    uint32_t                    dma_ch;
    uint32_t                    dma_slave_ch;
    char                        *ch_name;
    char                        *slave_ch_name;
    // Slave Mode
    bool                        slaveReadThreadTerminate;
    pthread_t                   slaveReadThread;
    SpiSlaveCallbackFunc        slaveCallBackFunc;
    float                       refclk;
#if defined(SPI_USE_DMA) && defined(SPI_USE_DMA_INTR)
    sem_t                       SpiDmaMutex;
#endif
} SPI_OBJECT;

typedef struct PORT_IO_MAPPING_ENTRY_TAG
{
    SPI_PORT             port;
    SPI_IO_MAPPING_ENTRY mapping_entry;
} PORT_IO_MAPPING_ENTRY;

static SPI_REF_CLK spi_clk_tab [] =
{
    {SPI_CLK_5K,   0.5f},
    {SPI_CLK_1M,   1.0f},
    {SPI_CLK_5M,   5.0f},
    {SPI_CLK_10M, 10.0f},
    {SPI_CLK_20M, 20.0f},
    {SPI_CLK_40M, 40.0f},
};

static SPI_OBJECT SpiObjects[2] =
{
    {
        0,                             // refCount
        PTHREAD_MUTEX_INITIALIZER      // mutex
    },
    {
        0,                             // refCount
        PTHREAD_MUTEX_INITIALIZER      // mutex
    }
};
	

static PORT_IO_MAPPING_ENTRY tSpiMisoMappingTable[] =
{
    //SPI_0
    {SPI_0, 22, 3}, {SPI_0, 33, 3},
    //SPI_1
    {SPI_1, 18, 3}, {SPI_1, 37, 3}, {SPI_1, 41, 3}

    ,{SPI_1, 17, 3} //lawrence add for spi slave
    ,{SPI_0, 21, 3} //lawrence add for spi slave
};

static PORT_IO_MAPPING_ENTRY tSpiMosiMappingTable[] =
{
    //SPI_0
    {SPI_0, 21, 3}, {SPI_0, 29, 3},
    //SPI_1
    {SPI_1, 17, 3}, {SPI_1, 36, 3}, {SPI_1, 40, 3}

    ,{SPI_1, 18, 3} //lawrence add for spi slave
    
    ,{SPI_0, 22, 3} //lawrence add for spi slave
};

static PORT_IO_MAPPING_ENTRY tSpiClockMappingTable[] =
{
    //SPI_0
    {SPI_0, 19, 3}, {SPI_0, 24, 3}, {SPI_0, 27, 3},
    //SPI_1
    {SPI_1, 15, 3}, {SPI_1, 34, 3}, {SPI_1, 38, 3}

};

static PORT_IO_MAPPING_ENTRY tSpiChipSelMappingTable[] =
{
    //SPI_0
    {SPI_0, 20, 3}, {SPI_0, 23, 3}, {SPI_0, 25, 3}, {SPI_0, 28, 3},
    //SPI_1
    {SPI_1, 16, 3}, {SPI_1, 35, 3}, {SPI_1, 39, 3}, {SPI_1, 42, 3}
};

static unsigned int     gSpiCtrlMethod                      = SPI_CONTROL_SLAVE;
static unsigned int     gShareGpioTable[SPI_SHARE_GPIO_MAX] = {0};
static SPI_CONTROL_MODE ctrl_mode = SPI_CONTROL_NOR;
static pthread_mutex_t  SwitchMutex;

//=============================================================================
//                              Private Function Declaration
//=============================================================================
static bool
_spiReadGarbage(
    SPI_PORT    port,
    uint32_t    garbageCount);

//=============================================================================
//                              Private Function Definition
//=============================================================================
#if defined(SPI_USE_DMA) && defined(SPI_USE_DMA_INTR)
static void
SpiDmaIntr(
    int         ch,
    void        *arg,
    uint32_t    int_status)
{
    int entrycnt = 0;
    int i = 0;

    entrycnt = sizeof(SpiObjects) / sizeof(SPI_OBJECT);
    for (i = 0; i < entrycnt; i++)
    {
        if (ch == SpiObjects[i].dma_ch || ch == SpiObjects[i].dma_slave_ch)
        {
            itpSemPostFromISR(&SpiObjects[i].SpiDmaMutex);
            break;
        }
    }
}
#endif

#ifdef SPI_USE_DMA
static bool
_spiWaitDmaIdle(
    SPI_PORT port,
    int dmachannel)
{
    #ifdef SPI_USE_DMA_INTR
    int result = 0;

    result = itpSemWaitTimeout(&SpiObjects[port].SpiDmaMutex, 5000);
    if (result)
    {
        /* Time out, fail! */
        return 0;
    }
    else
    {
        return 1;
    }
    #else
    uint32_t timeout_ms = 3000;

    while (ithDmaIsBusy(dmachannel) && --timeout_ms)
    {
        usleep(1000);
    }

    if (!timeout_ms)
    {
        /* DMA fail */
        return false;
    }

    return true;
    #endif
}

#endif

#ifdef SPI_USE_DMA
static bool
_spiInitDma(SPI_PORT port)
{
    bool                result      = true;
    #ifdef SPI_USE_DMA_INTR
    ITHDmaIntrHandler   spiDmaIntr  = SpiDmaIntr;
    #else
    ITHDmaIntrHandler   spiDmaIntr  = NULL;
    #endif

    if (port == SPI_0)
    {
        SpiObjects[port].ch_name        = "dma_spi0";
        SpiObjects[port].slave_ch_name  = "dma_slave_spi0";
    }
    else
    {
        SpiObjects[port].ch_name        = "dma_spi1";
        SpiObjects[port].slave_ch_name  = "dma_slave_spi1";
    }

    SpiObjects[port].dma_ch = ithDmaRequestCh(SpiObjects[port].ch_name, ITH_DMA_CH_PRIO_HIGH_3, spiDmaIntr, NULL);
    if (SpiObjects[port].dma_ch >= 0)
    {
        ithDmaReset(SpiObjects[port].dma_ch);
    }
    else
    {
        SPI_ERROR_MSG("Request DMA fail.\n");
        result = false;
    }

    SpiObjects[port].dma_slave_ch = ithDmaRequestCh(SpiObjects[port].slave_ch_name, ITH_DMA_CH_PRIO_HIGH_3, spiDmaIntr, NULL);
    if (SpiObjects[port].dma_slave_ch >= 0)
    {
        ithDmaReset(SpiObjects[port].dma_slave_ch);
    }
    else
    {
        SPI_ERROR_MSG("Request DMA fail.\n");
        result = false;
    }
    return result;
}

#endif

static float
_GetWclkClock()
{
    uint32_t PLL_numerator = 0;
    uint32_t PLL_pre_divider = 0;
    uint32_t PLL_post_divider = 0;
    uint32_t reg_Val = 0;
    uint32_t reg_1C = ithReadRegA(0xD800001C);
    uint32_t WCLK_Src = (reg_1C & 0x7000) >> 12;
    uint32_t WCLK_Ratio = reg_1C & 0x3FF;
    uint32_t CLK_base = 12;
    float    wclk_value = 0;

    switch (WCLK_Src)
    {
    case 0: // From PLL1 output1
        reg_Val = ithReadRegA(0xD8000100);
        PLL_numerator = (reg_Val & 0x7F0000) >> 16;
        PLL_pre_divider = reg_Val & 0x1F;
        PLL_post_divider = (reg_Val & 0x7F00) >> 8;
        break;

    case 2: // From PLL2 output1
        reg_Val = ithReadRegA(0xD8000110);
        PLL_numerator = (reg_Val & 0x7F0000) >> 16;
        PLL_pre_divider = reg_Val & 0x1F;
        PLL_post_divider = (reg_Val & 0x7F00) >> 8;
        break;

    case 3: // From PLL2 output2
        reg_Val = ithReadRegA(0xD8000118);
        PLL_numerator = (reg_Val & 0x7F0000) >> 16;
        PLL_pre_divider = reg_Val & 0x1F;
        PLL_post_divider = (reg_Val & 0x7F00) >> 8;
        break;

    case 4: // From PLL3 output1
        reg_Val = ithReadRegA(0xD8000120);
        PLL_numerator = (reg_Val & 0x7F0000) >> 16;
        PLL_pre_divider = reg_Val & 0x1F;
        PLL_post_divider = (reg_Val & 0x7F00) >> 8;
        break;

    case 1: // From PLL1 output2
    case 5: // From PLL3 output2
    case 6: // From PLL3 output3
    case 7: // From CKSYS (12MHz)
        SPI_ERROR_MSG("Unknown clock source %d\n", WCLK_Src);
        assert(0);
        break;
    }

    wclk_value = (float)CLK_base * 1 / PLL_pre_divider * PLL_numerator * 1 / PLL_post_divider / WCLK_Ratio;
    return wclk_value;
}

static void
_spiResetSpiObject(
    SPI_OBJECT *object)
{
    object->hwPort                      = SPI_HW_PORT_0;
    object->opMode                      = SPI_OP_MASTR;
    object->slaveReadThreadTerminate    = false;
    //object->slaveReadThread = NULL;
    object->slaveCallBackFunc           = NULL;
}

// #define CFG_SPI0_MOSI_GPIO          18
// #define CFG_SPI0_MISO_GPIO          19
// #define CFG_SPI0_CLOCK_GPIO         20
// #define CFG_SPI0_CHIP_SEL_GPIO      14

// #define CFG_SPI1_MOSI_GPIO          29
// #define CFG_SPI1_MISO_GPIO          30
// #define CFG_SPI1_CLOCK_GPIO         31
// #define CFG_SPI1_CHIP_SEL_GPIO      15

static bool
_spiAssign970Gpio(
    SPI_PORT port)
{
    int                         i           = 0;
    int                         entryCount  = 0;
    int                         mosiGpio    = 0;
    int                         misoGpio    = 0;
    int                         clockGpio   = 0;
    int                         chipSelGpio = 0;
    SPI_CONFIG_MAPPING_ENTRY    *ptMappingEntry;

    ptMappingEntry = &SpiObjects[port].tMappingEntry;
    if (port == SPI_0)
    {
#ifdef CFG_SPI0_ENABLE
        mosiGpio    = CFG_SPI0_MOSI_GPIO;
        misoGpio    = CFG_SPI0_MISO_GPIO;
        clockGpio   = CFG_SPI0_CLOCK_GPIO;
        chipSelGpio = CFG_SPI0_CHIP_SEL_GPIO;
        //printf("(%d, %d, %d, %d)\n", mosiGpio, misoGpio, clockGpio, chipSelGpio);
#else
        return false;
#endif
    }
    else if (port == SPI_1)
    {
#ifdef CFG_SPI1_ENABLE
        mosiGpio    = CFG_SPI1_MOSI_GPIO;
        misoGpio    = CFG_SPI1_MISO_GPIO;
        clockGpio   = CFG_SPI1_CLOCK_GPIO;
        chipSelGpio = CFG_SPI1_CHIP_SEL_GPIO;

        //printf("-- %d %d %d %d --\n", mosiGpio, misoGpio, clockGpio, chipSelGpio);
#else
        return false;
#endif
    }
    else
    {
        return false;
    }

    //MOSI
    entryCount = sizeof(tSpiMosiMappingTable) / sizeof(PORT_IO_MAPPING_ENTRY);
    for (i = 0; i < entryCount; i++)
    {
        if (port == tSpiMosiMappingTable[i].port
            && mosiGpio == tSpiMosiMappingTable[i].mapping_entry.gpioPin)
        {
            break;
        }
    }
    if (i >= entryCount)
    {
        return false;
    }
    ptMappingEntry->spiMosi.gpioPin     = tSpiMosiMappingTable[i].mapping_entry.gpioPin;
    ptMappingEntry->spiMosi.gpioMode    = tSpiMosiMappingTable[i].mapping_entry.gpioMode;

    //MISO
    entryCount                          = sizeof(tSpiMisoMappingTable) / sizeof(PORT_IO_MAPPING_ENTRY);
    for (i = 0; i < entryCount; i++)
    {
        if (port == tSpiMisoMappingTable[i].port
            && misoGpio == tSpiMisoMappingTable[i].mapping_entry.gpioPin)
        {
            break;
        }
    }
    if (i >= entryCount)
    {
        return false;
    }
    ptMappingEntry->spiMiso.gpioPin     = tSpiMisoMappingTable[i].mapping_entry.gpioPin;
    ptMappingEntry->spiMiso.gpioMode    = tSpiMisoMappingTable[i].mapping_entry.gpioMode;

    //CLOCK
    entryCount                          = sizeof(tSpiClockMappingTable) / sizeof(PORT_IO_MAPPING_ENTRY);
    for (i = 0; i < entryCount; i++)
    {
        if (port == tSpiClockMappingTable[i].port
            && clockGpio == tSpiClockMappingTable[i].mapping_entry.gpioPin)
        {
            break;
        }
    }
    if (i >= entryCount)
    {
        return false;
    }
    ptMappingEntry->spiClock.gpioPin    = tSpiClockMappingTable[i].mapping_entry.gpioPin;
    ptMappingEntry->spiClock.gpioMode   = tSpiClockMappingTable[i].mapping_entry.gpioMode;

    //CHIP SEL
    entryCount                          = sizeof(tSpiChipSelMappingTable) / sizeof(PORT_IO_MAPPING_ENTRY);
    for (i = 0; i < entryCount; i++)
    {
        if (port == tSpiChipSelMappingTable[i].port
            && chipSelGpio == tSpiChipSelMappingTable[i].mapping_entry.gpioPin)
        {
            break;
        }
    }
    if (i >= entryCount)
    {
        return false;
    }
    ptMappingEntry->spiChipSel.gpioPin  = tSpiChipSelMappingTable[i].mapping_entry.gpioPin;
    ptMappingEntry->spiChipSel.gpioMode = tSpiChipSelMappingTable[i].mapping_entry.gpioMode;

    ithGpioSetDriving(mosiGpio, ITH_GPIO_DRIVING_3);
    ithGpioSetDriving(misoGpio, ITH_GPIO_DRIVING_3);
    ithGpioSetDriving(clockGpio, ITH_GPIO_DRIVING_3);
    ithGpioSetDriving(chipSelGpio, ITH_GPIO_DRIVING_3);

    return true;
}

static bool
_spiSetGPIO(
    SPI_PORT port)
{
    int                         setGpio     = 0;
    int                         setMode     = 0;
    SPI_CONFIG_MAPPING_ENTRY    *ptMappingEntry;

    uint32_t                    gpiomode    = 0;

    if (!_spiAssign970Gpio(port))
    {
        printf("BBAA\n");
        return false;
    }

    ptMappingEntry  = &SpiObjects[port].tMappingEntry;

    // Set MOSI
    setGpio         = ptMappingEntry->spiMosi.gpioPin;
    setMode         = ptMappingEntry->spiMosi.gpioMode;

    //ithWriteRegMaskA(ITH_GPIO_BASE + (ITH_GPIO1_MODE_REG + (setGpio / 16) * 4),
    //    (setMode << ((setGpio & 0xF) * 2)),
    //    (0x03 << ((setGpio & 0xF) * 2)));
    ithGpioSetMode(setGpio, setMode);

    // Set MISO
    setGpio = ptMappingEntry->spiMiso.gpioPin;
    setMode = ptMappingEntry->spiMiso.gpioMode;
    //ithWriteRegMaskA(ITH_GPIO_BASE + (ITH_GPIO1_MODE_REG + (setGpio / 16) * 4),
    //    (setMode << ((setGpio & 0xF) * 2)),
    //    (0x03 << ((setGpio & 0xF) * 2)));
    ithGpioSetMode(setGpio, setMode);

    // Set CLOCK
    setGpio = ptMappingEntry->spiClock.gpioPin;
    setMode = ptMappingEntry->spiClock.gpioMode;
    //ithWriteRegMaskA(ITH_GPIO_BASE + (ITH_GPIO1_MODE_REG + (setGpio / 16) * 4),
    //    (setMode << ((setGpio & 0xF) * 2)),
    //    (0x03 << ((setGpio & 0xF) * 2)));
    ithGpioSetMode(setGpio, setMode);

    // Set CHIP SEL
    setGpio = ptMappingEntry->spiChipSel.gpioPin;
    setMode = ptMappingEntry->spiChipSel.gpioMode;
    //ithWriteRegMaskA(ITH_GPIO_BASE + (ITH_GPIO1_MODE_REG + (setGpio / 16) * 4),
    //    (setMode << ((setGpio & 0xF) * 2)),
    //    (0x03 << ((setGpio & 0xF) * 2)));
    ithGpioSetMode(setGpio, setMode);
    return true;
}

static void
_spiSetRegisters(
    SPI_PORT    port,
    SPI_OP_MODE mode,
    SPI_FORMAT  format,
    uint32_t    clockDivider)
{
    SPI_HW_PORT spiPort = (port == SPI_0) ? SPI_HW_PORT_0 : SPI_HW_PORT_1;

    if (spiPort == SPI_HW_PORT_1)
    {
        spiSetCLKSRC(spiPort, W8CLK);
    }

    spiSetFrameSyncPolarity(spiPort, SPI_FS_ACTIVE_HIGH);
    spiSetFrameFormat(spiPort);

    switch (format)
    {
    case CPO_1_CPH_0:
        spiSetSCLKPolarity(spiPort, SPI_SCLK_REMAIN_HIGH);
        spiSetSCLKPhase(spiPort, SPI_SCLK_PHASE_0);
        break;
    case CPO_0_CPH_1:
        spiSetSCLKPolarity(spiPort, SPI_SCLK_REMAIN_LOW);
        spiSetSCLKPhase(spiPort, SPI_SCLK_PHASE_1);
        break;
    case CPO_1_CPH_1:
        spiSetSCLKPolarity(spiPort, SPI_SCLK_REMAIN_HIGH);
        spiSetSCLKPhase(spiPort, SPI_SCLK_PHASE_1);
        break;
    case CPO_0_CPH_0:
    default:
        spiSetSCLKPolarity(spiPort, SPI_SCLK_REMAIN_LOW);
        spiSetSCLKPhase(spiPort, SPI_SCLK_PHASE_0);
        break;
    }

#ifdef CFG_SPI0_40MHZ_ENABLE
    if (port == SPI_0)
    {
        spiSetSCLKPolarity(spiPort, SPI_SCLK_REMAIN_HIGH);
        spiSetSCLKPhase(spiPort, SPI_SCLK_PHASE_1);
        spiSetClockOutInv(spiPort, SPI_OUT_CLK_INVERSE);
        spiSetClockOutBypass(spiPort, SPI_OUT_CLK_BYPASS_ENABLE);
        spiSetClockDelay(spiPort, 0x7D);
    }
#endif

    spiSetClockDivider(spiPort, clockDivider);

    if (mode == SPI_OP_MASTR)
    {
        spiSetMasterMode(spiPort);
    }
    else
    {
        spiSetSlaveMode(spiPort);
    }
}

static bool
_spiReadGarbage(
    SPI_PORT    port,
    uint32_t    garbageCount)
{
    uint32_t    readCount   = garbageCount;
    uint32_t    i           = 0;
    int32_t     timeOut     = 1000000;

    while (readCount)
    {
        uint32_t rxFifoCount = spiGetRxFifoValidCount(TO_SPI_HW_PORT(port));
        if (rxFifoCount > 0)
        {
            uint32_t val = spiReadData(TO_SPI_HW_PORT(port));
            readCount--;
            i++;
        }
        else
        {
            usleep(1);
            if ((--timeOut) < 0)
            {
                break;
            }
        }
    }

    return (timeOut >= 0);
}

static bool
_spiFifoWriteData(
    SPI_PORT    port,
    uint8_t     *data,
    uint32_t    dataSize,
    bool        abandonRxData)
{
    uint32_t    writeCount  = dataSize;
    uint32_t    i           = 0;
    int32_t     timeOut     = 1000;

    ithFlushDCacheRange((void *)data, dataSize);
    ithFlushMemBuffer();

    while (writeCount > 0)
    {
        if ((16 - spiGetTxFifoValidCount(TO_SPI_HW_PORT(port))) > 0)
        {
            spiWriteData(TO_SPI_HW_PORT(port), *(data + i));
            writeCount--;
            i++;

            if ((abandonRxData == true)
                && (_spiReadGarbage(port, 1) == false))
            {
                timeOut = -1;
                break;
            }
        }
        else
        {
            usleep(1000);
            if ((--timeOut) < 0)
            {
                break;
            }
        }
    }

    return (timeOut >= 0);
}

static bool
_spiFifoReadData(
    SPI_PORT    port,
    uint8_t     *outData,
    uint32_t    outDataSize)
{
    uint32_t    readCount   = outDataSize;
    uint32_t    i           = 0;
    int32_t     timeOut     = 1000000;

    spiSetFreeRunCount(TO_SPI_HW_PORT(port), outDataSize);
    spiSetFreeRunRxLock(TO_SPI_HW_PORT(port), SPI_RXLOCK_STOP_UNTIL_RXFIF_THAN_TXFIFO);
    spiFreeRunEnable(TO_SPI_HW_PORT(port));

    while (readCount)
    {
        uint32_t rxFifoCount = spiGetRxFifoValidCount(TO_SPI_HW_PORT(port));
        if (rxFifoCount > 0)
        {
            uint32_t val = spiReadData(TO_SPI_HW_PORT(port));
            *(outData + i) = val;
            readCount--;
            i++;
            SPI_VERBOSE_LOG("Read 0x%02X\n", val);
        }
        else
        {
            usleep(1);
            if ((--timeOut) < 0)
            {
                break;
            }
        }
    }
    spiFreeRunDisable(TO_SPI_HW_PORT(port));

    return (timeOut >= 0);
}

#ifdef SPI_USE_DMA
static bool
_spiDmaWriteData(
    SPI_PORT    port,
    uint8_t     *data,
    uint32_t    dataSize)
{
    bool result = true;

    ithFlushDCacheRange((void *)data, dataSize);
    ithFlushMemBuffer();

    spiSetTxFifoThreshold(TO_SPI_HW_PORT(port), 8);
    spiTxUnderrunInteruptEnable(TO_SPI_HW_PORT(port));
    spiTxThresholdInterruptEnable(TO_SPI_HW_PORT(port));

    ithDmaSetSrcAddr(SpiObjects[port].dma_ch, (uint32_t)data);
    ithDmaSetSrcParams(SpiObjects[port].dma_ch, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
    ithDmaSetDstParams(SpiObjects[port].dma_ch, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
    ithDmaSetTxSize(SpiObjects[port].dma_ch, dataSize);
    ithDmaSetBurst(SpiObjects[port].dma_ch, ITH_DMA_BURST_8);
    ithDmaSetDstAddr(SpiObjects[port].dma_ch, TO_SPI_HW_PORT(port) + REG_SPI_DATA);
    ithDmaSetRequest(SpiObjects[port].dma_ch, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM, ITH_DMA_HW_HANDSHAKE_MODE, (port == SPI_0) ? ITH_DMA_SPI0_TX : ITH_DMA_SPI1_TX);
    ithDmaStart(SpiObjects[port].dma_ch); // Fire DMA
    spiTxDmaRequestEnable(TO_SPI_HW_PORT(port));

    // Wait ilde
    if (_spiWaitDmaIdle(port, SpiObjects[port].dma_ch) == false)
    {
        SPI_ERROR_MSG("Wait DMA idle timeout.\n");
        result = false;
    }

    spiTxDmaRequestDisable(TO_SPI_HW_PORT(port));
    spiTxUnderrunInteruptDisable(TO_SPI_HW_PORT(port));
    spiTxThresholdInterruptDisable(TO_SPI_HW_PORT(port));

    return result;
}

static bool
_spiDmaReadData(
    SPI_PORT    port,
    uint8_t     *outData,
    uint32_t    outDataSize)
{
    bool        result      = true;
    uint32_t    readCount   = outDataSize;
    uint32_t    i           = 0;

    spiSetRxFifoThreshold(TO_SPI_HW_PORT(port), 1);
    spiRxOverrunInteruptEnable(TO_SPI_HW_PORT(port));
    spiRxThresholdInterruptEnable(TO_SPI_HW_PORT(port));
    spiSetFreeRunCount(TO_SPI_HW_PORT(port), outDataSize);
    spiSetFreeRunRxLock(TO_SPI_HW_PORT(port), SPI_RXLOCK_STOP_UNTIL_RXFIF_THAN_TXFIFO);
    spiFreeRunEnable(TO_SPI_HW_PORT(port));

    ithDmaSetDstAddr(SpiObjects[port].dma_slave_ch, (uint32_t)outData);
    ithDmaSetSrcParams(SpiObjects[port].dma_slave_ch, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
    ithDmaSetDstParams(SpiObjects[port].dma_slave_ch, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
    ithDmaSetTxSize(SpiObjects[port].dma_slave_ch, outDataSize);
    ithDmaSetBurst(SpiObjects[port].dma_slave_ch, ITH_DMA_BURST_1);
    ithDmaSetSrcAddr(SpiObjects[port].dma_slave_ch, TO_SPI_HW_PORT(port) + REG_SPI_DATA);
    ithDmaSetRequest(SpiObjects[port].dma_slave_ch, ITH_DMA_HW_HANDSHAKE_MODE, (port == SPI_0) ? ITH_DMA_SPI0_RX : ITH_DMA_SPI1_RX, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM);
    ithDmaStart(SpiObjects[port].dma_slave_ch);                     // Fire DMA
    spiRxDmaRequestEnable(TO_SPI_HW_PORT(port));    // SPI DMA enable

                                                    // Wait ilde
    if (_spiWaitDmaIdle(port, SpiObjects[port].dma_slave_ch) == false)
    {
        SPI_ERROR_MSG("Wait DMA idle timeout.\n");
        result = false;
    }

    spiRxDmaRequestDisable(TO_SPI_HW_PORT(port));
    spiRxOverrunInteruptDisable(TO_SPI_HW_PORT(port));
    spiRxThresholdInterruptDisable(TO_SPI_HW_PORT(port));
    spiFreeRunDisable(TO_SPI_HW_PORT(port));

    ithInvalidateDCacheRange(outData, outDataSize);
    return result;
}

#endif

static bool
_spiWaitTxFifoEmpty(
    SPI_PORT port)
{
    int32_t timeOut = 1000;

    while (spiGetTxFifoValidCount(TO_SPI_HW_PORT(port)) > 0)
    {
        usleep(1000);
        if ((--timeOut) < 0)
        {
            break;
        }
    }

    return (timeOut >= 0);
}

static bool
_spiWaitRxFifoEmpty(
    SPI_PORT port)
{
    int32_t timeOut = 1000;

    while (spiGetRxFifoValidCount(TO_SPI_HW_PORT(port)) > 0)
    {
        usleep(1000);
        if ((--timeOut) < 0)
        {
            break;
        }
    }

    return (timeOut >= 0);
}

static bool
_spiWaitEngineIdle(
    SPI_PORT port)
{
    int32_t timeOut = 1000;

    while (spiIsBusy(TO_SPI_HW_PORT(port)) == true)
    {
        usleep(1000);
        if ((--timeOut) < 0)
        {
            break;
        }
    }

    return (timeOut >= 0);
}

static void *
_SpiSlaveReadThread(
    void *args)
{
    SPI_OBJECT *spiObject = (SPI_OBJECT *)args;
    uint32_t port = (spiObject->hwPort == SPI_HW_PORT_0) ? SPI_0 : SPI_1;

    spiClearTxFifo(TO_SPI_HW_PORT(port), SPI_FIFO_CLEAR);
    spiClearRxFifo(TO_SPI_HW_PORT(port), SPI_FIFO_CLEAR);
    spiSetDataLength(TO_SPI_HW_PORT(port), 7);
    spiEngineStart(TO_SPI_HW_PORT(port));

    while (spiObject->slaveReadThreadTerminate == false)
    {
        uint32_t rxFifoCount = spiGetRxFifoValidCount(spiObject->hwPort);

        if (rxFifoCount > 0)
        {
            uint32_t readCount = rxFifoCount;
            while (readCount > 0)
            {
                uint32_t val = spiReadData(spiObject->hwPort);
                if (spiObject->slaveCallBackFunc != NULL)
                {
                    spiObject->slaveCallBackFunc(val);
                }

                while (1)
                {
                    if ((16 - spiGetTxFifoValidCount(TO_SPI_HW_PORT(port))) > 0)
                    {
                        spiWriteData(TO_SPI_HW_PORT(port), (uint8_t)val);
                        break;
                    }
                }

                readCount--;
            }
        }
        else
        {
            usleep(1);
        }
    }

    pthread_exit(NULL);
    return 0;
}

int32_t
mmpSpiInitialize(
    SPI_PORT    port,
    SPI_OP_MODE mode,
    SPI_FORMAT  format,
    SPI_CLK_LAB spiclk)
{
    int      entrycnt;
    int      i;
    float    wclk;
    uint32_t sclk_divider;
    float    tmpclk;
    int32_t  result = 0;

    SPI_FUNC_ENTRY;

    if (port >= SPI_PORT_MAX)
    {
        result = 1;
        goto end;
    }

    pthread_mutex_lock(&SpiObjects[port].mutex);

    if (SpiObjects[port].refCount == 0)
    {
        entrycnt     = 0;
        i            = 0;
        wclk         = _GetWclkClock();
        sclk_divider = 0;

        _spiResetSpiObject(&SpiObjects[port]);
        SpiObjects[port].refCount   = 1;
        SpiObjects[port].hwPort     = TO_SPI_HW_PORT(port);
        SpiObjects[port].opMode     = mode;
        SpiObjects[port].refclk     = 20.0f;

        entrycnt                    = sizeof(spi_clk_tab) / sizeof(SPI_REF_CLK);
        for (i = 0; i < entrycnt; i++)
        {
            if (spiclk == spi_clk_tab[i].spi_clk_lab)
            {
                SpiObjects[port].refclk = spi_clk_tab[i].refclock;
                break;
            }
        }

        for (sclk_divider = 0; sclk_divider < 300; sclk_divider++)
        {
            tmpclk = 0.0f;
            tmpclk = (wclk / (((float)sclk_divider + 1) * 2));

            if (tmpclk <= SpiObjects[port].refclk)
                break;
        }

        if (_spiSetGPIO(port) == false)
        {
            SPI_ERROR_MSG("_spiSetGPIO() fail, port: %d\n", port);
            result = 1;
            goto end;
        }

        _spiSetRegisters(port, mode, format, sclk_divider);

        printf( "\n========================================\n");
        printf( "               SPI %d init\n", port);
        printf( "========================================\n");
        printf( "Version       : %d.%d.%d\n",   spiGetMajorVersion(TO_SPI_HW_PORT(port)), spiGetMinorVersion(TO_SPI_HW_PORT(port)), spiGetReleaseVersion(TO_SPI_HW_PORT(port)));
        printf( "SClock Divider: %d\n",        sclk_divider);
        printf( "WCLK          : %.2f MHz\n",   wclk);
        printf( "SPI Clock     : %.2f MHz\n\n", wclk / (((float)sclk_divider + 1) * 2));

#ifdef SPI_USE_DMA
        if (_spiInitDma(port) == false)
        {
            SPI_ERROR_MSG("_spiInitDma() fail.\n");
            result = 1;
            goto end;
        }
    #ifdef SPI_USE_DMA_INTR
        sem_init(&SpiObjects[port].SpiDmaMutex, 0, 0);
    #endif
#endif
        if (mode == SPI_OP_SLAVE)
        {
            pthread_create(&SpiObjects[port].slaveReadThread, NULL, _SpiSlaveReadThread, &SpiObjects[port]);
        }
    }
    else
    {
        SpiObjects[port].refCount++;
        printf("SPI already initialed. refCount = %d\n", SpiObjects[port].refCount);
    }

end:
    pthread_mutex_unlock(&SpiObjects[port].mutex);
    SPI_FUNC_LEAVE;

    //printf("loopback mode start\n");
    //ithWriteRegMaskA(TO_SPI_HW_PORT(port) + REG_SPI_CONTROL_0, (1<<7), (1<<7));
    //printf("loopback mode end\n");
    return result;
}

int32_t
mmpSpiTerminate(
    SPI_PORT port)
{
    int32_t result = 0;
    pthread_mutex_destroy(&SwitchMutex);
    pthread_mutex_lock(&SpiObjects[port].mutex);

    SpiObjects[port].refCount--;
    if (SpiObjects[port].refCount == 0)
    {
        spiClearTxFifo(TO_SPI_HW_PORT(port), SPI_FIFO_CLEAR);
        usleep(1000);
        spiClearTxFifo(TO_SPI_HW_PORT(port), SPI_FIFO_NORMAL);
        spiEngineStop(TO_SPI_HW_PORT(port));
#ifdef SPI_USE_DMA
        ithDmaFreeCh(SpiObjects[port].dma_ch);
        ithDmaFreeCh(SpiObjects[port].dma_slave_ch);
    #ifdef SPI_USE_DMA_INTR
        sem_destroy(&SpiObjects[port].SpiDmaMutex);
    #endif
#endif

        if (SpiObjects[port].opMode == SPI_OP_SLAVE)
        {
            SpiObjects[port].slaveReadThreadTerminate = true;
        }

        printf( "========================================\n");
        printf( "             SPI %d terminated.\n", port);
        printf( "========================================\n");
    }

    pthread_mutex_unlock(&SpiObjects[port].mutex);

    return result;
}

#ifdef SPI_USE_DMA
int32_t
mmpSpiDmaWrite(
    SPI_PORT    port,
    uint8_t     *inCommand,
    uint32_t    inCommandSize,
    uint8_t     *inData,
    uint32_t    inDataSize,
    uint8_t     dataLength)
{
    int32_t result = true;

    pthread_mutex_lock(&SpiObjects[port].mutex);

    if (_spiSetGPIO(port) == false)
        return false;

    spiSetCSN(TO_SPI_HW_PORT(port), SPI_CSN_ALWAYS_ZERO);
    spiClearTxFifo(TO_SPI_HW_PORT(port), SPI_FIFO_CLEAR);
    spiClearRxFifo(TO_SPI_HW_PORT(port), SPI_FIFO_CLEAR);
    spiSetDataLength(TO_SPI_HW_PORT(port), 7);
    spiEngineStart(TO_SPI_HW_PORT(port));
    //printf("-- DMA Write --\n");
    do
    {
        // Write command & data
        if (_spiFifoWriteData(port, inCommand, inCommandSize, false) == false)
        {
            SPI_ERROR_MSG("Write command fail.\n");
            result = false;
            break;
        }

        if (_spiDmaWriteData(port, inData, inDataSize) == false)
        {
            SPI_ERROR_MSG("_spiDmaWriteData() fail.\n");
            result = false;
            break;
        }

        if (_spiWaitTxFifoEmpty(port) == false)
        {
            SPI_ERROR_MSG("Wait TX FIFO empty timeout.\n");
            result = false;
            break;
        }
        if (_spiWaitEngineIdle(port) == false)
        {
            SPI_ERROR_MSG("Wait engine idle timeout.\n");
            result = false;
            break;
        }
    } while (0);

    spiEngineStop(TO_SPI_HW_PORT(port));
    spiSetCSN(TO_SPI_HW_PORT(port), SPI_CSN_NORMAL);

    pthread_mutex_unlock(&SpiObjects[port].mutex);

    return result;
}

#else
int32_t
mmpSpiDmaWrite(
    SPI_PORT    port,
    uint8_t     *inCommand,
    uint32_t    inCommandSize,
    uint8_t     *inData,
    uint32_t    inDataSize,
    uint8_t     dataLength)
{
    return mmpSpiPioWrite(port, inCommand, inCommandSize, inData, inDataSize, 0);
}

#endif

#ifdef SPI_USE_DMA
int32_t
mmpSpiDmaRead(
    SPI_PORT    port,
    uint8_t     *inCommand,
    uint32_t    inCommandSize,
    uint8_t     *outData,
    uint32_t    outDataSize,
    uint8_t     dataLength)
{
    int32_t result = true;

    if (outData == NULL
        || outDataSize == 0)
    {
        SPI_LOG_MSG("Parameter error! outData = 0x%08X, outDataSize = %d\n", outData, outDataSize);
        return false;
    }

    pthread_mutex_lock(&SpiObjects[port].mutex);

    if (_spiSetGPIO(port) == false)
        return false;

    spiSetCSN(TO_SPI_HW_PORT(port), SPI_CSN_ALWAYS_ZERO);
    spiClearTxFifo(TO_SPI_HW_PORT(port), SPI_FIFO_CLEAR);
    spiClearRxFifo(TO_SPI_HW_PORT(port), SPI_FIFO_CLEAR);
    spiSetDataLength(TO_SPI_HW_PORT(port), 7);
    spiEngineStart(TO_SPI_HW_PORT(port));

    do
    {
        // Write command & data
        if (_spiFifoWriteData(port, inCommand, inCommandSize, true) == false)
        {
            SPI_ERROR_MSG("Write command fail.\n");
            result = false;
            break;
        }
        if (_spiDmaReadData(port, outData, outDataSize) == false)
        {
            SPI_ERROR_MSG("Read data fail.\n");
            result = false;
            break;
        }
        // Wait ilde
        if (_spiWaitTxFifoEmpty(port) == false)
        {
            SPI_ERROR_MSG("Wait TX FIFO empty timeout.\n");
            result = false;
            break;
        }
        if (_spiWaitRxFifoEmpty(port) == false)
        {
            SPI_ERROR_MSG("Wait RX FIFO empty timeout.\n");
            result = false;
            break;
        }
        if (_spiWaitEngineIdle(port) == false)
        {
            SPI_ERROR_MSG("Wait engine idle timeout.\n");
            result = false;
            break;
        }
    } while (0);

    spiEngineStop(TO_SPI_HW_PORT(port));
    spiSetCSN(TO_SPI_HW_PORT(port), SPI_CSN_NORMAL);

    pthread_mutex_unlock(&SpiObjects[port].mutex);
    return result;
}

#else
int32_t
mmpSpiDmaRead(
    SPI_PORT    port,
    uint8_t     *inCommand,
    uint32_t    inCommandSize,
    uint8_t     *outData,
    uint32_t    outDataSize,
    uint8_t     dataLength)
{
    return mmpSpiPioRead(port, inCommand, inCommandSize, outData, outDataSize, 0);
}

#endif

int32_t
mmpSpiPioWrite(
    SPI_PORT    port,
    uint8_t     *inCommand,
    uint32_t    inCommandSize,
    uint8_t     *inData,
    uint32_t    inDataSize,
    uint8_t     dataLength)
{
    int32_t result = true;

    pthread_mutex_lock(&SpiObjects[port].mutex);

    if (_spiSetGPIO(port) == false)
        return false;

    spiSetCSN(TO_SPI_HW_PORT(port), SPI_CSN_ALWAYS_ZERO);
    spiClearTxFifo(TO_SPI_HW_PORT(port), SPI_FIFO_CLEAR);
    spiClearRxFifo(TO_SPI_HW_PORT(port), SPI_FIFO_CLEAR);
    spiSetDataLength(TO_SPI_HW_PORT(port), 7);
    spiEngineStart(TO_SPI_HW_PORT(port));

    do
    {
        // Write command & data
        if (_spiFifoWriteData(port, inCommand, inCommandSize, false) == false)
        {
            SPI_ERROR_MSG("Write command fail.\n");
            result = false;
            break;
        }
        if ((inData != NULL)
            && (_spiFifoWriteData(port, inData, inDataSize, false) == false))
        {
            SPI_ERROR_MSG("Write data fail.\n");
            result = false;
            break;
        }

        // Wait ilde
        if (_spiWaitTxFifoEmpty(port) == false)
        {
            SPI_ERROR_MSG("Wait TX FIFO empty timeout.\n");
            result = false;
            break;
        }
        if (_spiWaitEngineIdle(port) == false)
        {
            SPI_ERROR_MSG("Wait engine idle timeout.\n");
            result = false;
            break;
        }
    } while (0);

    spiEngineStop(TO_SPI_HW_PORT(port));
    spiSetCSN(TO_SPI_HW_PORT(port), SPI_CSN_NORMAL);

    pthread_mutex_unlock(&SpiObjects[port].mutex);

    return result;
}

int32_t
mmpSpiPioRead(
    SPI_PORT    port,
    uint8_t     *inCommand,
    uint32_t    inCommandSize,
    uint8_t     *outData,
    uint32_t    outDataSize,
    uint8_t     dataLength)
{
    int32_t result      = true;
#ifdef CFG_CPU_WB
    uint8_t *command    = (uint8_t *)itpVmemAlloc(inCommandSize);
#else
    uint8_t *command    = inCommand;
#endif
    if (!command)
    {
        SPI_LOG_MSG("No enough memory\n");
        return false;
    }

    if (outData == NULL
        || outDataSize == 0)
    {
        SPI_LOG_MSG("Parameter error! outData = 0x%08X, outDataSize = %d\n", outData, outDataSize);
        return false;
    }

#ifdef CFG_CPU_WB
    memcpy(command, inCommand, inCommandSize);
#endif

    pthread_mutex_lock(&SpiObjects[port].mutex);

    if (_spiSetGPIO(port) == false)
        return false;

    spiSetCSN(TO_SPI_HW_PORT(port), SPI_CSN_ALWAYS_ZERO);
    spiClearTxFifo(TO_SPI_HW_PORT(port), SPI_FIFO_CLEAR);
    spiClearRxFifo(TO_SPI_HW_PORT(port), SPI_FIFO_CLEAR);
    spiSetDataLength(TO_SPI_HW_PORT(port), 7);
    spiEngineStart(TO_SPI_HW_PORT(port));

    do
    {
        // Write command & data
        if (_spiFifoWriteData(port, command, inCommandSize, true) == false)
        {
            SPI_ERROR_MSG("Write command fail.\n");
            result = false;
            break;
        }
       // usleep(10000); // SPI slave test (waiting for slave Tx sending)
        if (_spiFifoReadData(port, outData, outDataSize) == false)
        {
            SPI_ERROR_MSG("Read data fail.\n");
            result = false;
            break;
        }
        // Wait ilde
        if (_spiWaitTxFifoEmpty(port) == false)
        {
            SPI_ERROR_MSG("Wait TX FIFO empty timeout.\n");
            result = false;
            break;
        }
        if (_spiWaitRxFifoEmpty(port) == false)
        {
            SPI_ERROR_MSG("Wait RX FIFO empty timeout.\n");
            result = false;
            break;
        }
        if (_spiWaitEngineIdle(port) == false)
        {
            SPI_ERROR_MSG("Wait engine idle timeout.\n");
            result = false;
            break;
        }
    } while (0);

    spiEngineStop(TO_SPI_HW_PORT(port));
    spiSetCSN(TO_SPI_HW_PORT(port), SPI_CSN_NORMAL);

    pthread_mutex_unlock(&SpiObjects[port].mutex);

#ifdef CFG_CPU_WB
    itpVmemFree((uint32_t)command);
#endif
    return result;
}

void
mmpSpiSetClockDivider(
    SPI_PORT    port,
    uint16_t    divider)
{
    pthread_mutex_lock(&SpiObjects[port].mutex);
    spiSetClockDivider(TO_SPI_HW_PORT(port), divider);
    pthread_mutex_unlock(&SpiObjects[port].mutex);
}

void
mmpSpiSetSlaveCallbackFunc(
    SPI_PORT                port,
    SpiSlaveCallbackFunc    callbackFunc)
{
    pthread_mutex_lock(&SpiObjects[port].mutex);

    SpiObjects[port].slaveCallBackFunc = callbackFunc;

    pthread_mutex_unlock(&SpiObjects[port].mutex);
}

bool
mmpSpiSlavePioWrite(
    SPI_PORT    port,
    uint8_t     *inData,
    uint32_t    inDataSize)
{
    bool result = true;

    pthread_mutex_lock(&SpiObjects[port].mutex);

    if (_spiSetGPIO(port) == false)
        return false;

    spiSetCSN(TO_SPI_HW_PORT(port), SPI_CSN_ALWAYS_ZERO);
    spiClearTxFifo(TO_SPI_HW_PORT(port), SPI_FIFO_CLEAR);
    spiClearRxFifo(TO_SPI_HW_PORT(port), SPI_FIFO_CLEAR);
    spiSetDataLength(TO_SPI_HW_PORT(port), 7);
    spiEngineStart(TO_SPI_HW_PORT(port));

    do
    {
        // Write data
        if (_spiFifoWriteData(port, inData, inDataSize, false) == false)
        {
            SPI_ERROR_MSG("Write command fail.\n");
            result = false;
            break;
        }

        // Wait ilde
        if (_spiWaitTxFifoEmpty(port) == false)
        {
            SPI_ERROR_MSG("Wait TX FIFO empty timeout.\n");
            result = false;
            break;
        }
        if (_spiWaitEngineIdle(port) == false)
        {
            SPI_ERROR_MSG("Wait engine idle timeout.\n");
            result = false;
            break;
        }
    } while (0);

    spiEngineStop(TO_SPI_HW_PORT(port));
    spiSetCSN(TO_SPI_HW_PORT(port), SPI_CSN_NORMAL);

    pthread_mutex_unlock(&SpiObjects[port].mutex);

    return result;
}

uint32_t mmpSpiPioTransfer(
    SPI_PORT    port,
    void        *tx_buf,
    void        *rx_buf,
    uint32_t    buflen)
{
    uint32_t    result      = 0;
    uint32_t    readCount   = buflen;
    uint32_t    i           = 0;
    uint8_t     *boutput    = (uint8_t *)(rx_buf);

    pthread_mutex_lock(&SpiObjects[port].mutex);

    if (_spiSetGPIO(port) == false)
        return false;

    spiSetCSN(TO_SPI_HW_PORT(port), SPI_CSN_ALWAYS_ZERO);
    spiClearTxFifo(TO_SPI_HW_PORT(port), SPI_FIFO_CLEAR);
    spiClearRxFifo(TO_SPI_HW_PORT(port), SPI_FIFO_CLEAR);
    spiSetDataLength(TO_SPI_HW_PORT(port), 7);
    spiEngineStart(TO_SPI_HW_PORT(port));

    do
    {
        // Write command & data
        if (_spiFifoWriteData(port, tx_buf, buflen, false) == false)
        {
            SPI_ERROR_MSG("Write command fail.\n");
            result = 1;
            break;
        }

        // Wait ilde
        if (_spiWaitTxFifoEmpty(port) == false)
        {
            SPI_ERROR_MSG("Wait TX FIFO empty timeout.\n");
            result = 1;
            break;
        }

        while (readCount)
        {
            uint32_t rxFifoCount = spiGetRxFifoValidCount(TO_SPI_HW_PORT(port));
            if (rxFifoCount > 0)
            {
                uint32_t val = spiReadData(TO_SPI_HW_PORT(port));
                *(boutput + i) = val;
                readCount--;
                i++;
                SPI_VERBOSE_LOG("Read 0x%02X\n", val);
            }
        }

        //ithInvalidateDCacheRange(rx_buf, buflen);

        if (_spiWaitRxFifoEmpty(port) == false)
        {
            SPI_ERROR_MSG("Wait RX FIFO empty timeout.\n");
            result = 1;
            break;
        }

        if (_spiWaitEngineIdle(port) == false)
        {
            SPI_ERROR_MSG("Wait engine idle timeout.\n");
            result = 1;
            break;
        }
    } while (0);

    spiEngineStop(TO_SPI_HW_PORT(port));
    spiSetCSN(TO_SPI_HW_PORT(port), SPI_CSN_NORMAL);

    pthread_mutex_unlock(&SpiObjects[port].mutex);
    return result;
}

uint32_t mmpSpiDmaTransfer(
    SPI_PORT    port,
    void        *tx_buf,
    void        *rx_buf,
    uint32_t    buflen)
{
    uint32_t result = 0;

    pthread_mutex_lock(&SpiObjects[port].mutex);

    if (_spiSetGPIO(port) == false)
        return false;

    spiSetCSN(TO_SPI_HW_PORT(port), SPI_CSN_ALWAYS_ZERO);
    spiClearTxFifo(TO_SPI_HW_PORT(port), SPI_FIFO_CLEAR);
    spiClearRxFifo(TO_SPI_HW_PORT(port), SPI_FIFO_CLEAR);
    spiSetDataLength(TO_SPI_HW_PORT(port), 7);
    spiEngineStart(TO_SPI_HW_PORT(port));

    do
    {
        {
            bool result = true;

            spiSetTxFifoThreshold(TO_SPI_HW_PORT(port), 8);
            spiTxUnderrunInteruptEnable(TO_SPI_HW_PORT(port));
            spiTxThresholdInterruptEnable(TO_SPI_HW_PORT(port));

            ithDmaSetSrcAddr(SpiObjects[port].dma_ch, (uint32_t)tx_buf);
            ithDmaSetSrcParams(SpiObjects[port].dma_ch, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
            ithDmaSetDstParams(SpiObjects[port].dma_ch, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
            ithDmaSetTxSize(SpiObjects[port].dma_ch, buflen);
            ithDmaSetBurst(SpiObjects[port].dma_ch, ITH_DMA_BURST_8);
            ithDmaSetDstAddr(SpiObjects[port].dma_ch, TO_SPI_HW_PORT(port) + REG_SPI_DATA);
            ithDmaSetRequest(SpiObjects[port].dma_ch, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM, ITH_DMA_HW_HANDSHAKE_MODE, (port == SPI_0) ? ITH_DMA_SPI0_TX : ITH_DMA_SPI1_TX);
            ithDmaStart(SpiObjects[port].dma_ch);// Fire DMA
            spiTxDmaRequestEnable(TO_SPI_HW_PORT(port));

            // Wait ilde
            //if (_spiWaitDmaIdle(port, SpiObjects[port].dma_ch) == false)
            //{
            //	SPI_ERROR_MSG("Wait DMA idle timeout.\n");
            //	result = false;
            //}

            //spiTxDmaRequestDisable(TO_SPI_HW_PORT(port));
            //spiTxUnderrunInteruptDisable(TO_SPI_HW_PORT(port));
            //spiTxThresholdInterruptDisable(TO_SPI_HW_PORT(port));

            //return result;
        }

        {
            bool        result  = true;
            uint32_t    i       = 0;

            spiSetRxFifoThreshold(TO_SPI_HW_PORT(port), 1);
            spiRxOverrunInteruptEnable(TO_SPI_HW_PORT(port));
            spiRxThresholdInterruptEnable(TO_SPI_HW_PORT(port));
            //spiSetFreeRunCount(TO_SPI_HW_PORT(port), outDataSize);
            //spiSetFreeRunRxLock(TO_SPI_HW_PORT(port), SPI_RXLOCK_STOP_UNTIL_RXFIF_THAN_TXFIFO);
            //spiFreeRunEnable(TO_SPI_HW_PORT(port));

            ithDmaSetDstAddr(SpiObjects[port].dma_slave_ch, (uint32_t)rx_buf);
            ithDmaSetSrcParams(SpiObjects[port].dma_slave_ch, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
            ithDmaSetDstParams(SpiObjects[port].dma_slave_ch, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
            ithDmaSetTxSize(SpiObjects[port].dma_slave_ch, buflen);
            ithDmaSetBurst(SpiObjects[port].dma_slave_ch, ITH_DMA_BURST_1);
            ithDmaSetSrcAddr(SpiObjects[port].dma_slave_ch, TO_SPI_HW_PORT(port) + REG_SPI_DATA);
            ithDmaSetRequest(SpiObjects[port].dma_slave_ch, ITH_DMA_HW_HANDSHAKE_MODE, (port == SPI_0) ? ITH_DMA_SPI0_RX : ITH_DMA_SPI1_RX, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM);
            ithDmaStart(SpiObjects[port].dma_slave_ch);     // Fire DMA
            spiRxDmaRequestEnable(TO_SPI_HW_PORT(port));    // SPI DMA enable

            while (1)
                ;
            // Wait TX DMA ilde
            if (_spiWaitDmaIdle(port, SpiObjects[port].dma_ch) == false)
            {
                SPI_ERROR_MSG("Wait TX DMA idle timeout.\n");
                result = false;
            }

            // Wait RX DMA ilde
            if (_spiWaitDmaIdle(port, SpiObjects[port].dma_slave_ch) == false)
            {
                SPI_ERROR_MSG("Wait RX DMA idle timeout.\n");
                result = false;
            }

            spiTxDmaRequestDisable(TO_SPI_HW_PORT(port));
            spiTxUnderrunInteruptDisable(TO_SPI_HW_PORT(port));
            spiTxThresholdInterruptDisable(TO_SPI_HW_PORT(port));
            spiRxDmaRequestDisable(TO_SPI_HW_PORT(port));
            spiRxOverrunInteruptDisable(TO_SPI_HW_PORT(port));
            spiRxThresholdInterruptDisable(TO_SPI_HW_PORT(port));
            //spiFreeRunDisable(TO_SPI_HW_PORT(port));

#ifdef __OPENRTOS__
            ithFlushDCacheRange((void*)rx_buf, buflen);
            ithInvalidateDCacheRange(rx_buf, buflen);
#endif

            pthread_mutex_unlock(&SpiObjects[port].mutex);
            return result;
        }
    } while (0);
}

uint32_t mmpSpiTransfer(
    SPI_PORT    port,
    void        *tx_buf,
    void        *rx_buf,
    uint32_t    buflen)
{
    uint32_t result = 0;
    if (buflen >= 16)
        result = mmpSpiDmaTransfer(port, tx_buf, rx_buf, buflen);
    else
        result = mmpSpiPioTransfer(port, tx_buf, rx_buf, buflen);

    return result;
}

void
mmpSpiResetControl(
    void)
{
#if 0
    pthread_mutex_unlock(&SwitchMutex);
    //nor_using = false;
#else
    return;
#endif
}

static void
_SwitchSpiGpioPin(
    SPI_CONTROL_MODE controlMode)
{
    int pin_ori = gShareGpioTable[ctrl_mode];
    int pin_dst = gShareGpioTable[controlMode];

    if (pin_ori && pin_dst)
    {
        if (gSpiCtrlMethod == SPI_CONTROL_SLAVE)
        {
            ithGpioSet(pin_ori);
            ithGpioSetOut(pin_ori);

            ithGpioClear(pin_dst);
            ithGpioSetIn(pin_dst);
            usleep(10);
        }
        else
        {
            ithGpioSet(pin_ori);        
            ithGpioSetOut(pin_ori);
            
            ithGpioClear(pin_dst);      
            ithGpioSetOut(pin_dst);
            usleep(10);

            while (1)
            {
                if (!ithGpioGet(pin_dst) && ithGpioGet(pin_ori)) break;
            }
        }
    }
}

void
mmpSpiInitShareGpioPin(
    unsigned int *pins)
{
    unsigned int i;

    printf("init SPI share pin:[%x]\n", ITH_COUNT_OF(pins));
    //ithWriteRegMaskA( REG_SSP_FREERUN, (0x1 << 2), (0x1 << 2));
    pthread_mutex_init(&SwitchMutex, NULL);

    for (i = 0; i < SPI_SHARE_GPIO_MAX; i++)
    {
        if ((pins[i] < 64) && (pins[i] > 0)) gShareGpioTable[i] = pins[i];
        else gShareGpioTable[i] = 0;

        printf("pin[%x]=[%d,%d]\n", i, pins[i], gShareGpioTable[i]);
    }

    if (gShareGpioTable[SPI_CONTROL_NAND]) gSpiCtrlMethod = SPI_CONTROL_NAND;
    printf("gSpiCtrlMethod = %x\n", gSpiCtrlMethod, SPI_CONTROL_NAND);
}

void
mmpSpiSetControlMode(
    SPI_CONTROL_MODE controlMode)
{
#if 0
    uint32_t    mask    = 0;
    uint32_t    value   = 0;

    pthread_mutex_lock(&SwitchMutex);
    //printf("SetSpiGpio:[%x,%x]\n",controlMode,ctrl_mode);

    if (controlMode != ctrl_mode)
    {
        _SwitchSpiGpioPin(controlMode);
        ctrl_mode = controlMode;
    }
#else
    return;
#endif
}

int32_t
mmpSpiDmaReadNoDropFirstByte(
    SPI_PORT    port,
    uint8_t     *inputData,
    int32_t     inputSize,
    void        *pdes,
    int32_t     size,
    uint8_t     dataLength)
{
    return 0;
}
