//=============================================================================
//                              Include Files
//=============================================================================
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "ite/ith.h"
#include "../../include/ite/itp.h"
#include "../../include/ssp/mmp_axispi.h"
#include "../../include/ssp/ssp_error.h"
#if defined (__OPENRTOS__)
#elif defined (__FREERTOS__)
    #include "or32.h"
#endif
#include "axispi_hw.h"
#include "axispi_reg.h"
#include "spi_msg.h"
//=============================================================================
//                              Compile Option
//=============================================================================
//#define ENABLE_SET_SPI_CLOCK_AS_40MHZ
//#define ENABLE_SET_SPI_CLOCK_AS_10MHZ

//=============================================================================
//                              Macro
//=============================================================================
#define TO_SPI_HW_PORT(port) SPI_HW_PORT_0

#define SWAP_ENDIAN32(value) \
    ((((value) >> 24) & 0x000000FF) | \
     (((value) >> 8) & 0x0000FF00) | \
     (((value) << 8) & 0x00FF0000) | \
     (((value) << 24) & 0xFF000000))

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
} SPI_OBJECT;

typedef struct PORT_IO_MAPPING_ENTRY_TAG
{
    SPI_PORT             port;
    SPI_IO_MAPPING_ENTRY mapping_entry;
} PORT_IO_MAPPING_ENTRY;

static SPI_OBJECT SpiObjects[1] =
{
    {
        0,                             // refCount
        PTHREAD_MUTEX_INITIALIZER      // mutex
    }
};

static PORT_IO_MAPPING_ENTRY tSpiMisoMappingTable[] =
{
    {SPI_0, 7, 1}
};

static PORT_IO_MAPPING_ENTRY tSpiMosiMappingTable[] =
{
    {SPI_0, 6, 1}
};

static PORT_IO_MAPPING_ENTRY tSpiClockMappingTable[] =
{
    {SPI_0, 10, 1}
};

static PORT_IO_MAPPING_ENTRY tSpiChipSelMappingTable[] =
{
    {SPI_0, 5, 1}, {SPI_0, 11, 1}
};

static PORT_IO_MAPPING_ENTRY tSpiWpMappingTable[] =
{
    {SPI_0, 8, 1}
};

static PORT_IO_MAPPING_ENTRY tSpiHoldMappingTable[] =
{
    {SPI_0, 9, 1}
};

#ifdef SPI_USE_DMA
static int              DmaSpiChannel = 0;
#endif
#if defined(SPI_USE_DMA) && defined(SPI_USE_DMA_INTR)
static sem_t            SpiDmaMutex;
#endif

static unsigned int     gSpiCtrlMethod                      = SPI_CONTROL_SLAVE;
static unsigned int     gShareGpioTable[SPI_SHARE_GPIO_MAX] = {0};
static SPI_CONTROL_MODE ctrl_mode   = SPI_CONTROL_NOR;
static pthread_mutex_t  SwitchMutex;
static unsigned int     gDummyByte  = 0;

//=============================================================================
//                              Private Function Declaration
//=============================================================================

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
    itpSemPostFromISR(&SpiDmaMutex);
}
#endif

#ifdef SPI_USE_DMA
static bool
_spiWaitDmaIdle(
    int dmachannel)
{
    #ifdef SPI_USE_DMA_INTR
    int result = 0;

    result = itpSemWaitTimeout(&SpiDmaMutex, 5000);
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
    uint16_t    PLL1_numerator = ithReadRegH(0x00A4) & 0x3FF;
    uint16_t    reg_1C = ithReadRegH(0x001C);
    uint16_t    reg_A2 = ithReadRegH(0x00A2);
    uint16_t    wclkRatio = reg_1C & 0x3FF;
    uint16_t    clockSrc = (reg_1C & 0x3800) >> 11;
    uint16_t    regValue, pllDivider;
    float       wclk_value = 0;

    switch (clockSrc)
    {
    case 0: // From PLL1 output1
        regValue    = ithReadRegH(0x00A2);
        pllDivider  = regValue & 0x007F;
        break;

    case 1: // From PLL1 output2
        regValue    = ithReadRegH(0x00A2);
        pllDivider  = (regValue & 0x7F00) >> 8;
        break;

    case 2: // From PLL2 output1
    case 3: // From PLL2 output2
    case 4: // From PLL3 output1
    case 5: // From PLL3 output2
    case 6: // From CKSYS (12MHz)
    case 7: // From Ring OSC (200KHz)
        SPI_ERROR_MSG("Unknown clock source %d\n", clockSrc);
        assert(0);
        break;
    }

    wclk_value = (float)PLL1_numerator / (wclkRatio + 1) / pllDivider;
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
    int                         wpGpio      = 0;
    int                         holdGpio    = 0;

    SPI_CONFIG_MAPPING_ENTRY    *ptMappingEntry;

    ptMappingEntry = &SpiObjects[port].tMappingEntry;
    if (port == SPI_0)
    {
#if defined (CFG_AXISPI_ENABLE)
        mosiGpio    = CFG_AXISPI_MOSI_GPIO;
        misoGpio    = CFG_AXISPI_MISO_GPIO;
        clockGpio   = CFG_AXISPI_CLOCK_GPIO;
        chipSelGpio = CFG_AXISPI_CHIP_SEL_GPIO;
        wpGpio      = CFG_AXISPI_WP_GPIO;
        holdGpio    = CFG_AXISPI_HOLD_GPIO;
#else
        return false;
#endif
    }
    else
    {
        printf(" UnKnown SPI Port !!!");
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

    //WP
    entryCount                          = sizeof(tSpiWpMappingTable) / sizeof(PORT_IO_MAPPING_ENTRY);
    for (i = 0; i < entryCount; i++)
    {
        if (port == tSpiWpMappingTable[i].port
            && wpGpio == tSpiWpMappingTable[i].mapping_entry.gpioPin)
        {
            break;
        }
    }
    if (i >= entryCount)
    {
        return false;
    }
    ptMappingEntry->spiWp.gpioPin   = tSpiWpMappingTable[i].mapping_entry.gpioPin;
    ptMappingEntry->spiWp.gpioMode  = tSpiWpMappingTable[i].mapping_entry.gpioMode;

    //HOLD
    entryCount                      = sizeof(tSpiHoldMappingTable) / sizeof(PORT_IO_MAPPING_ENTRY);
    for (i = 0; i < entryCount; i++)
    {
        if (port == tSpiHoldMappingTable[i].port
            && holdGpio == tSpiHoldMappingTable[i].mapping_entry.gpioPin)
        {
            break;
        }
    }
    if (i >= entryCount)
    {
        return false;
    }
    ptMappingEntry->spiHold.gpioPin     = tSpiHoldMappingTable[i].mapping_entry.gpioPin;
    ptMappingEntry->spiHold.gpioMode    = tSpiHoldMappingTable[i].mapping_entry.gpioMode;

    ithGpioSetDriving(mosiGpio, ITH_GPIO_DRIVING_1);
    ithGpioSetDriving(misoGpio, ITH_GPIO_DRIVING_1);
    ithGpioSetDriving(clockGpio, ITH_GPIO_DRIVING_1);
    ithGpioSetDriving(chipSelGpio, ITH_GPIO_DRIVING_1);
    ithGpioSetDriving(wpGpio, ITH_GPIO_DRIVING_1);
    ithGpioSetDriving(holdGpio, ITH_GPIO_DRIVING_1);

    return true;
}

static bool
_spiSetGPIO(
    SPI_PORT port)
{
    int                         setGpio = 0;
    int                         setMode = 0;
    SPI_CONFIG_MAPPING_ENTRY    *ptMappingEntry;

    //if (ithGetDeviceId() == 0x970)
    {
        if (!_spiAssign970Gpio(port))
        {
            return false;
        }
    }
    //else
    //{
    //    return false;
    //}

    ptMappingEntry  = &SpiObjects[port].tMappingEntry;

    // Set MOSI
    setGpio         = ptMappingEntry->spiMosi.gpioPin;
    setMode         = ptMappingEntry->spiMosi.gpioMode;
    ithGpioSetMode(setGpio, setMode);

    // Set MISO
    setGpio = ptMappingEntry->spiMiso.gpioPin;
    setMode = ptMappingEntry->spiMiso.gpioMode;
    ithGpioSetMode(setGpio, setMode);

    // Set CLOCK
    setGpio = ptMappingEntry->spiClock.gpioPin;
    setMode = ptMappingEntry->spiClock.gpioMode;
    ithGpioSetMode(setGpio, setMode);

    // Set CHIP SEL
    setGpio = ptMappingEntry->spiChipSel.gpioPin;
    setMode = ptMappingEntry->spiChipSel.gpioMode;
    ithGpioSetMode(setGpio, setMode);

    // Set WP
    setGpio = ptMappingEntry->spiWp.gpioPin;
    setMode = ptMappingEntry->spiWp.gpioMode;
    ithGpioSetMode(setGpio, setMode);

    // Set HOLD
    setGpio = ptMappingEntry->spiHold.gpioPin;
    setMode = ptMappingEntry->spiHold.gpioMode;
    ithGpioSetMode(setGpio, setMode);

    return true;
}

bool AXISPIWriteData(uint8_t *data, uint32_t dataSize)
{
    //the Max FIFO count is 32, so we check the TX FIFO space ,
    //if it is valid ,TX FIFO Valid count should not be zero. (push one data, TX FIFO -1)
    uint32_t    writeCount  = dataSize;
    uint32_t    i           = 0;
    int32_t     timeOut     = 10000000;
    uint32_t    value       = 0;

    while (writeCount > 0)
    {
        if ((AxiSpiReadTXFIFOValidSpace()) > 0)
        {
            if (writeCount >= 4)
            {
                value       = 0;
                value       = (((data[i]) << 24) | ((data[i + 1]) << 16) | ((data[i + 2]) << 8) | (data[i + 3]));
                AxiSpiWriteData(value);
                writeCount  -= 4;
                i           += 4;
            }
            else if (writeCount == 3)
            {
                value       = 0;
                value       = (((data[i]) << 24) | ((data[i + 1]) << 16) | ((data[i + 2]) << 8));
                AxiSpiWriteData(value);
                writeCount  -= 3;
                i           += 3;
            }
            else if (writeCount == 2)
            {
                value       = 0;
                value       = (((data[i]) << 24) | ((data[i + 1]) << 16));
                AxiSpiWriteData(value);
                writeCount  -= 2;
                i           += 2;
            }
            else
            {
                value       = 0;
                value       = (((data[i]) << 24));
                AxiSpiWriteData(value);
                writeCount  -= 1;
                i++;
            }
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

bool AXISPIReadData(uint8_t *outData, uint32_t outDataSize)
{
    //printf("AXISPIReadData\n");
    uint32_t    readCount   = outDataSize;
    uint32_t    i           = 0;
    uint32_t    j           = 0;
    int32_t     timeOut     = 10000000;

    while (readCount)
    {
        uint32_t rxFifoCount = AxiSpiReadRXFIFODataCount();
        if (rxFifoCount > 0)
        {
            uint32_t val = AxiSpiReadData();

            if (readCount >= 4)
            {
                readCount       -= 4;
                outData[i]      = (val & 0x000000FF) >> 0;
                outData[i + 1]  = (val & 0x0000FF00) >> 8;
                outData[i + 2]  = (val & 0x00FF0000) >> 16;
                outData[i + 3]  = (val & 0xFF000000) >> 24;
                //printf("[%x], [%x], [%x], [%x]\n", outData[i], outData[i+1], outData[i+2], outData[i+3]);
                i               += 4;
            }
            else
            {
                if (readCount == 3)
                {
                    outData[i]      = (val & 0x000000FF) >> 0;
                    outData[i + 1]  = (val & 0x0000FF00) >> 8;
                    outData[i + 2]  = (val & 0x00FF0000) >> 16;
                    //printf("out[%d]= [%x], out[%d]= [%x], out[%d]= [%x]\n", i, outData[i], i+1, outData[i+1], i+2, outData[i+2]);
                    readCount       -= 3;
                    i               += 3;
                }
                else if (readCount == 2)
                {
                    outData[i]      = (val & 0x000000FF) >> 0;
                    outData[i + 1]  = (val & 0x0000FF00) >> 8;
                    //printf("out[%d]=[%x], out[%d]= [%x]\n", i, outData[i], i+1, outData[i+1]);
                    readCount       -= 2;
                    i               += 2;
                }
                else
                {
                    outData[i]  = (val & 0x000000FF) >> 0;
                    //printf("out[%d]= [%x]\n", i, outData[i]);
                    readCount   -= 1;
                    i           += 1;
                }
            }
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

#ifdef SPI_USE_DMA
static bool
_spiDmaWriteData(
    uint8_t     *data,
    uint32_t    dataSize)
{
    bool        result  = true;
    uint32_t    value   = 0;

    DmaSpiChannel = SpiObjects[0].dma_ch;
                                                            //printf("_spiDmaWriteData data=0x%x,DmaSpiChannel=%d\n",data,DmaSpiChannel);

                                                            //ithFlushDCacheRange((void*)data, dataSize);
                                                            //ithFlushMemBuffer();

    ithWriteRegMaskA((ITH_DMA_BASE + 0x4C), 0x0<<(DmaSpiChannel&0x7), 0x1<<(DmaSpiChannel&0x7)); //Indian change

    ithWriteRegMaskA(SPI_REG_CMD_W1, (0 << 5), (1 << 5));   /* bit 4: RX fifo endian , bit 5 : TX Fifo*/

                                                            //printf("write dma channel = %d\n", DmaSpiChannel);

    ithDmaSetSrcAddr(DmaSpiChannel, (uint32_t)data);
    ithDmaSetSrcParamsA(DmaSpiChannel, ITH_DMA_WIDTH_32, ITH_DMA_CTRL_INC);
    ithDmaSetDstParamsA(DmaSpiChannel, ITH_DMA_WIDTH_32, ITH_DMA_CTRL_FIX);
    //printf("--- write datasize = 0x%x ---\n", dataSize);
    ithDmaSetTxSize(DmaSpiChannel, dataSize);
    ithDmaSetBurst(DmaSpiChannel, ITH_DMA_BURST_8);
    ithDmaSetDstAddr(DmaSpiChannel, SPI_REG_DATA_PORT);
    ithDmaSetRequest(DmaSpiChannel, ITH_DMA_NORMAL_MODE, ITH_DMA_AXISPI_TX_RX, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_AXISPI_TX_RX);

    ithDmaStart(DmaSpiChannel); // Fire DMA
                                //AxiSpiCmdCompletIntrEnable();
    AxiSpiDMAEnable();          //spiTxDmaRequestEnable(TO_SPI_HW_PORT(port));
                                //while(1);

                                // Wait idle
    #if 1
    if (_spiWaitDmaIdle(DmaSpiChannel) == false)
    {
        printf("Wait DMA idle timeout.\n");
        result = false;
    }
    #endif

    AxiSpiDMADisable();

    return result;
}

static bool
_spiDmaReadData(
    uint8_t     *outData,
    uint32_t    outDataSize)
{
    bool        result      = true;
    uint32_t    readCount   = outDataSize;
    uint32_t    i           = 0;

    DmaSpiChannel = SpiObjects[0].dma_slave_ch;

    //printf("_spiDmaReadData outData=0x%x,DmaSpiChannel=%d\n",outData,DmaSpiChannel);
    AxiSpiSetRxFifoThod(1);

    ithWriteRegMaskA((ITH_DMA_BASE + 0x4C), 0x0<<(DmaSpiChannel&0x7), 0x1<<(DmaSpiChannel&0x7)); //Indian change

    ithWriteRegMaskA(SPI_REG_CMD_W1, (0<<4), (1<<4)); /* bit 4: RX fifo endian , bit 5 : TX Fifo*/

    ithDmaSetDstAddr(DmaSpiChannel, (uint32_t)outData);
    ithDmaSetSrcParamsA(DmaSpiChannel, ITH_DMA_WIDTH_32, ITH_DMA_CTRL_FIX);
    ithDmaSetDstParamsA(DmaSpiChannel, ITH_DMA_WIDTH_32, ITH_DMA_CTRL_INC);

    ithDmaSetTxSize(DmaSpiChannel, outDataSize);
    ithDmaSetBurst(DmaSpiChannel, ITH_DMA_BURST_16);
    ithDmaSetSrcAddr(DmaSpiChannel, SPI_REG_DATA_PORT);
    ithDmaSetRequest(DmaSpiChannel, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_AXISPI_TX_RX, ITH_DMA_NORMAL_MODE, ITH_DMA_AXISPI_TX_RX);
    ithDmaStart(DmaSpiChannel); // Fire DMA
    AxiSpiDMAEnable();          //spiRxDmaRequestEnable(TO_SPI_HW_PORT(port));	// SPI DMA enable

                                // Wait ilde
    if (_spiWaitDmaIdle(DmaSpiChannel) == false)
    {
        printf("Wait DMA idle timeout.\n");
        result = false;
    }
    AxiSpiDMADisable();

    ithInvalidateDCacheRange(outData, outDataSize);
    return result;
}

#endif

void
mmpAxiSpiSetDummyByte(SPI_PORT port, unsigned int dummybyte)
{
    pthread_mutex_lock(&SpiObjects[port].mutex);
    gDummyByte = dummybyte;
    pthread_mutex_unlock(&SpiObjects[port].mutex);
}

int32_t
mmpAxiSpiInitialize(
    SPI_PORT    port,
    SPI_OP_MODE mode,
    SPI_FORMAT  format,
    SPI_CLK_LAB spiclk,
    SPI_CLK_DIV divider)
{
    int32_t result = 0;

    SPI_FUNC_ENTRY;

    if (port >= SPI_PORT_MAX)
    {
        result = 1;
        goto end;
    }

    pthread_mutex_lock(&SpiObjects[port].mutex);

    if (SpiObjects[port].refCount == 0)
    {
        /*
           int entrycnt = 0;
           int i = 0;
           float wclk = _GetWclkClock();
           uint32_t sclk_divider = 0;

           _spiResetSpiObject(&SpiObjects[port]);
           SpiObjects[port].refCount = 1;
           SpiObjects[port].hwPort   = TO_SPI_HW_PORT(port);
           SpiObjects[port].opMode   = mode;
           SpiObjects[port].refclk   = 20.0f;

           entrycnt = sizeof(spi_clk_tab)/sizeof(SPI_REF_CLK);
           for (i = 0; i< entrycnt; i++)
           {
            if(spiclk == spi_clk_tab[i].spi_clk_lab)
            {
                SpiObjects[port].refclk = spi_clk_tab[i].refclock;
                break;
            }
           }

           for (sclk_divider=0; sclk_divider < 80; sclk_divider++)
           {
            float tmpclk = 0.0f;
            tmpclk = (wclk / (((float)sclk_divider + 1) * 2));

            if (tmpclk <= SpiObjects[port].refclk)
                break;
           }
         */
        AxiSpiSetClkDiv(divider);

        if (_spiSetGPIO(port) == false)
        {
            SPI_ERROR_MSG("_spiSetGPIO() fail.\n");
            result = 1;
            goto end;
        }

        /*
           _spiSetRegisters(port, mode, format, sclk_divider);

           printf("\n========================================\n");
           printf("               SPI %d init\n", port);
           printf("========================================\n");
           printf("Version       : %d.%d.%d\n", spiGetMajorVersion(TO_SPI_HW_PORT(port)), spiGetMinorVersion(TO_SPI_HW_PORT(port)), spiGetReleaseVersion(TO_SPI_HW_PORT(port)));
           printf("SClock Divider : %d\n", sclk_divider);
           printf("WCLK          : %.2f MHz\n", wclk);
           printf("SPI Clock     : %.2f MHz\n\n", wclk / (((float)sclk_divider + 1) * 2));
         */
#ifdef SPI_USE_DMA
        if (_spiInitDma(port) == false)
        {
            SPI_ERROR_MSG("_spiInitDma() fail.\n");
            result = 1;
            goto end;
        }
    #ifdef SPI_USE_DMA_INTR
        sem_init(&SpiDmaMutex, 0, 0);
    #endif
#endif
        //if (mode == SPI_OP_SLAVE)
        //{
        //	pthread_create(&SpiObjects[port].slaveReadThread, NULL, _SpiSlaveReadThread, &SpiObjects[port]);
        //}
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
mmpAxiSpiTerminate(
    SPI_PORT port)
{
    int32_t result = 0;
    pthread_mutex_destroy(&SwitchMutex);
    pthread_mutex_lock(&SpiObjects[port].mutex);

    SpiObjects[port].refCount--;
    if (SpiObjects[port].refCount == 0)
    {
#ifdef SPI_USE_DMA
        ithDmaFreeCh(DmaSpiChannel);
    #ifdef SPI_USE_DMA_INTR
        sem_destroy(&SpiDmaMutex);
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
mmpAxiSpiDmaWrite(
    SPI_PORT        port,
    SPI_IOMODE      iomode,
    uint8_t         *inCommand,
    SPI_CMD_LEN     inCommandLen,
    uint32_t        *slaveaddr,
    SPI_ADDR_LEN    slaveaddrLen,
    uint8_t         *inData,
    uint32_t        inDataSize)
{
    int32_t     result      = true;

    //printf("++ DMA Write ++\n");
    int32_t     remainsize  = inDataSize;
    //int32_t  currentsize = inDataSize;;
    uint32_t    wDMASize    = 0;
    uint8_t     *wpaddr     = inData;
    int32_t     timeOut     = 10000000;

    pthread_mutex_lock(&SpiObjects[port].mutex);

    if (_spiSetGPIO(port) == false)
        return false;

    if (slaveaddr == 0)
        AxiSpiWriteAddr(0);
    else
        AxiSpiWriteAddr(*slaveaddr);
    AxiSpiWriteAddrLength(slaveaddrLen);
    AxiSpiWriteDataCounter(inDataSize); //Data Counter. (4 byte = 32bit)
    AxiSpiWriteInstLength(inCommandLen);
    AxiSpiWriteDummy(0);

    #if 1
    AxiSpiWriteInstCode4bitWriteEnable((uint32_t)*inCommand, iomode);
    #else
    //AxiSpiWriteInstCodeWriteEnable((uint32_t)*inCommand);
	if ((uint32_t)*inCommand == 0x38 || (uint32_t)*inCommand == 0x3E || (uint32_t)*inCommand == 0x32)
    {
        //printf("--dma write cmd = 0x%x --\n", (uint32_t)*inCommand);
        AxiSpiWriteInstCode4bitWriteEnable((uint32_t)*inCommand);
    }
    else
        AxiSpiWriteInstCodeWriteEnable((uint32_t)*inCommand);
    #endif

    do
    {
        if (_spiDmaWriteData(inData, inDataSize) == 0)
            printf("write fail!\n");

        // Wait ilde
        while (0x00 == AxiSpiReadIntrStatus())
        {
            if (--timeOut < 0)
            {
                printf("AxiSpiReadIntrStatus fail!\n");
                result = false;
                break;
            }
            else
            {
                usleep(1);
            }
        }
        AxiSpiSetIntrStatus();
    } while (0);

    pthread_mutex_unlock(&SpiObjects[port].mutex);

    return result;
}
#else
int32_t
mmpAxiSpiDmaWrite(
    SPI_PORT        port,
    SPI_IOMODE      iomode,
    uint8_t         *inCommand,
    SPI_CMD_LEN     inCommandLen,
    uint32_t        *slaveaddr,
    SPI_ADDR_LEN    slaveaddrLen,
    uint8_t         *inData,
    uint32_t        inDataSize)
{
    return mmpAxiSpiPioWrite(port, iomode, inCommand, inCommandLen, slaveaddr, slaveaddrLen, inData, inDataSize);
}
#endif

#ifdef SPI_USE_DMA
int32_t
mmpAxiSpiDmaRead(
    SPI_PORT        port,
    SPI_IOMODE      iomode,
    SPI_DTRMODE     dtrmode,
    uint8_t         *inCommand,
    SPI_CMD_LEN     inCommandLen,
    uint32_t        *slaveaddr,
    SPI_ADDR_LEN    slaveaddrLen,
    uint8_t         *outData,
    uint32_t        outDataSize)
{
    int32_t result = true;

    //printf("++ DMA Read ++\n");

    int32_t     remainsize  = outDataSize;
    uint32_t    rDMASize    = 0;
    uint8_t     *rpaddr     = outData;
    int32_t     timeOut     = 10000000;

    pthread_mutex_lock(&SpiObjects[port].mutex);

    if (_spiSetGPIO(port) == false)
        return false;

    //ithWriteRegMaskA(0xd800006c , (1<<31), (1<<31));
    while (AxiSpiReadRXFIFODataCount())
    {
        AxiSpiReadData();
    }

    if (slaveaddr == 0)
        AxiSpiWriteAddr(0);
    else
        AxiSpiWriteAddr(*slaveaddr);
    AxiSpiWriteAddrLength(slaveaddrLen);
    AxiSpiWriteDataCounter(outDataSize);//Data Counter. (4 byte = 32bit)
    AxiSpiWriteInstLength(inCommandLen);
    AxiSpiWriteDummy(0);

    #if 1
    if ((iomode == SPI_IOMODE_5) || (iomode == SPI_IOMODE_4) || (iomode == SPI_IOMODE_2))
        AxiSpiWriteDummy(gDummyByte);

    AxiSpiWriteInstCode4bitRead((uint32_t)*inCommand, (iomode == SPI_IOMODE_5) ? SPI_IOMODE_0 : iomode, dtrmode);
    #else
    if ((uint32_t)*inCommand == 0xEB || (uint32_t)*inCommand == 0xEC)
    {
        AxiSpiWriteDummy(6);
        //printf("-- dma read cmd 0x%x --\n", (uint32_t)*inCommand);
        AxiSpiWriteInstCode4bitRead((uint32_t)*inCommand);
    }
    else
        AxiSpiWriteInstCodeRead((uint32_t)*inCommand);
    #endif

    if ((outDataSize % 4) == 0)
    {
        rDMASize    = outDataSize;
        remainsize  -= outDataSize;
    }
    else
    {
        outDataSize = (outDataSize >> 2) << 2;
        rDMASize    = outDataSize;
        remainsize  -= outDataSize;
    }

    //printf("rDMASize = %d\n", rDMASize);
    usleep(2);    //It's nesscessary for NAND to avoid from DMA fail. 
    
    do
    {
        if (_spiDmaReadData(rpaddr, rDMASize) == 0)
        {
            printf("Read fail!\n");
            while (1);
        }
        // Wait ilde
        while (0x00 == AxiSpiReadIntrStatus())
        {
            if (--timeOut < 0)
            {
                printf("AxiSpiReadIntrStatus fail!\n");
                result = false;
                break;
            }
            else
            {
                usleep(1);
            }
        }
        AxiSpiSetIntrStatus();
    } while (0);

    rpaddr += rDMASize;

    //printf("remainsize = %d\n", remainsize);
    if (remainsize)
    {
        AXISPIReadData(rpaddr, remainsize);
        remainsize = 0;
    }

    pthread_mutex_unlock(&SpiObjects[port].mutex);

    //printf("--  DMA Read result=%d--\n", result);
    return result;
}
#else
int32_t
mmpAxiSpiDmaRead(
    SPI_PORT        port,
    SPI_IOMODE      iomode,
    SPI_DTRMODE     dtrmode,
    uint8_t         *inCommand,
    SPI_CMD_LEN     inCommandLen,
    uint32_t        *slaveaddr,
    SPI_ADDR_LEN    slaveaddrLen,
    uint8_t         *outData,
    uint32_t        outDataSize)
{
    return mmpAxiSpiPioRead(port, iomode, dtrmode, inCommand, inCommandLen, slaveaddr, slaveaddrLen, outData, outDataSize);
}
#endif

int32_t
mmpAxiSpiPioWrite(
    SPI_PORT        port,
    SPI_IOMODE      iomode,
    uint8_t         *inCommand,
    SPI_CMD_LEN     inCommandLen,
    uint32_t        *slaveaddr,
    SPI_ADDR_LEN    slaveaddrLen,
    uint32_t        *inData,
    uint32_t        inDataSize)
{
    int32_t     result  = true;
    uint32_t    value   = 0;
    int32_t     timeOut = 10000000;

    pthread_mutex_lock(&SpiObjects[port].mutex);

    if (_spiSetGPIO(port) == false)
        return false;

    //printf("++ pio Write ++\n");

    if (slaveaddr == 0)
        AxiSpiWriteAddr(0);
    else
        AxiSpiWriteAddr(*slaveaddr);
    AxiSpiWriteAddrLength(slaveaddrLen);
    AxiSpiWriteDataCounter(inDataSize);//Data Counter. (4 byte = 32bit)
    AxiSpiWriteInstLength(inCommandLen);
    AxiSpiWriteDummy(0);
    ithWriteRegMaskA(SPI_REG_CMD_W1, (1 << 5), (1 << 5)); //big endian

#if 1
    AxiSpiWriteInstCode4bitWriteEnable((uint32_t)*inCommand, iomode);
#else
    if ((uint32_t)*inCommand == 0x38 || (uint32_t)*inCommand == 0x3E)
    {
        AxiSpiWriteInstCode4bitWriteEnable((uint32_t)*inCommand);
    }
    else
    {
        AxiSpiWriteInstCodeWriteEnable((uint32_t)*inCommand);
    }
#endif

    do
    {
        if (AXISPIWriteData((uint8_t *)inData, inDataSize) == false)
        {
            printf("-- AxiSpi Write Fail !!! ---\n");
        }

        while (0x00 == AxiSpiReadIntrStatus())
        {
            if (--timeOut < 0)
            {
                printf("AxiSpiReadIntrStatus fail!\n");
                result = false;
                break;
            }
            else
            {
                usleep(1);
            }
        }
        AxiSpiSetIntrStatus();
    } while (0);

    pthread_mutex_unlock(&SpiObjects[port].mutex);

    return result;
}

int32_t
mmpAxiSpiPioRead(
    SPI_PORT        port,
    SPI_IOMODE      iomode,
    SPI_DTRMODE     dtrmode,
    uint8_t         *inCommand,
    SPI_CMD_LEN     inCommandLen,
    uint32_t        *slaveaddr,
    SPI_ADDR_LEN    slaveaddrLen,
    uint32_t        *outData,
    uint32_t        outDataSize)
{
    int32_t result  = true;
    int32_t timeOut = 10000000;

    if (inCommandLen == NONE_BYTE || inCommandLen == RESERVE_BYTE)
    {
        printf("Error Instruction Length !!!\n");
        result = false;
        return result;
    }

#ifdef CFG_CPU_WB
    uint8_t *command    = (uint8_t *)itpVmemAlloc(inCommandLen);
#else
    uint8_t *command    = inCommand;
#endif

    if (!command)
    {
        printf("No enough memory\n");
        return false;
    }

    //if (   outData == NULL
    //    || outDataSize == 0)
    //{
    //	printf("Parameter error! outData = 0x%08X, outDataSize = %d\n", outData, outDataSize);
    //	return false;
    //}

#ifdef CFG_CPU_WB
    memcpy(command, inCommand, inCommandLen);
#endif

    pthread_mutex_lock(&SpiObjects[port].mutex);

    if (_spiSetGPIO(port) == false)
        return false;

    if (slaveaddr == 0)
        AxiSpiWriteAddr(0);
    else
        AxiSpiWriteAddr(*slaveaddr);
    AxiSpiWriteAddrLength(slaveaddrLen);
    AxiSpiWriteDataCounter(outDataSize);//Data Counter. (4 byte = 32bit)
    AxiSpiWriteInstLength(inCommandLen);
    AxiSpiWriteDummy(0);

    //ithWriteRegMaskA(SPI_REG_CMD_W1, (1<<4), (1<<4)); /* bit 4: RX fifo endian , bit 5 : TX Fifo*/

                                                                            //if ((uint32_t)*inCommand == 0x05) // Read Nor Status cmd
    if ( ((uint32_t)*inCommand == 0x05) || (((uint32_t)*inCommand == 0x0F) && (*slaveaddr==0xC0)) ) // Read Nor/Nand Status cmd
    {
        if (outDataSize)
            AxiSpiWriteInstCodeReadStatus((uint32_t)*inCommand);
        else
            AxiSpiWriteInstCodeReadStatusEnable((uint32_t)*inCommand);
    }
    else
    {
#if 1
        if ((iomode == SPI_IOMODE_5) || (iomode == SPI_IOMODE_4) || (iomode == SPI_IOMODE_2))
            AxiSpiWriteDummy(gDummyByte);

        AxiSpiWriteInstCode4bitRead((uint32_t)*inCommand, (iomode == SPI_IOMODE_5) ? SPI_IOMODE_0 : iomode, dtrmode);
#else
        if ((uint32_t)*inCommand == 0xEB || (uint32_t)*inCommand == 0xEC)
        {
            AxiSpiWriteDummy(6);
            AxiSpiWriteInstCode4bitRead((uint32_t)*inCommand);
        }
        else
            AxiSpiWriteInstCodeRead((uint32_t)*inCommand);
#endif
    }

    do
    {
        if (AXISPIReadData((uint8_t *)outData, outDataSize) == false)
        {
            printf("-- AxiSpi Read Fail !!! ---\n");
            while (1);
        }

        while (0x00 == AxiSpiReadIntrStatus())
        {
            if (--timeOut < 0)
            {
                printf("AxiSpiReadIntrStatus fail!\n");
                result = false;
                break;
            }
            else
            {
                usleep(1);
            }
        }
        AxiSpiSetIntrStatus();
        //printf("## cmd =%x , 0x18=0x%08x\n", (uint32_t)*inCommand, ithReadRegA(REG_SPI_BASE+0x18));
    } while (0);

    pthread_mutex_unlock(&SpiObjects[port].mutex);

#ifdef CFG_CPU_WB
    itpVmemFree((uint32_t)command);
#endif
    return result;
}

void
mmpAxiSpiResetControl(
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
mmpAxiSpiInitShareGpioPin(
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
mmpAxiSpiSetControlMode(
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