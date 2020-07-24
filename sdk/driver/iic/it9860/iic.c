/*
 * Copyright (c) 2014 ITE Corp. All Rights Reserved.
 */
/** @file
 *  I2C API functoin file.
 *      Date: 2014/05/16
 *
 * @author Awin Huang
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include "mmp_iic.h"
#include "iic_msg.h"
#include "i2c_hwreg.h"
#include "ite/ith.h"
#include "ite/itp.h"

//=============================================================================
//                              Extern Reference
//=============================================================================


//=============================================================================
//                              Compile Option
//=============================================================================


//=============================================================================
//                              Macro Definition
//=============================================================================
#define IIC_MAGIC_ID_0  0x49494330
#define IIC0_DMA_TX_CH  0
#define IIC0_DMA_RX_CH  1
#define IIC1_DMA_TX_CH  2
#define IIC1_DMA_RX_CH  3
#define IIC2_DMA_TX_CH  4
#define IIC2_DMA_RX_CH  5
#define IIC3_DMA_TX_CH  6
#define IIC3_DMA_RX_CH  7
#define IIC_SLAVE_READ_SIZE 2

#define IIC_REG_BIT(var, n) (((var & (1 << n)) > 0) ? 1 : 0)

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef enum _IIC_STATE_FLAG
{
    TRANSMIT_DATA       = 0x1,
    RECEIVE_DATA        = 0x2,
    TRANSMIT_SERVICE    = 0x4,
    RECRIVE_SERVICE     = 0x8,
    NACK_EN             = 0x10
}IIC_STATE;

typedef enum DATA_MODE_TAG
{
    WO_STOP,                        //without stop
    W_STOP                          //with stop
} IIC_DATA_MODE;

//=============================================================================
//                              Global Data Definition
//=============================================================================
static IIC_DEVICE IicDevices[4] = {{PTHREAD_MUTEX_INITIALIZER}, {PTHREAD_MUTEX_INITIALIZER},
                                    {PTHREAD_MUTEX_INITIALIZER}, {PTHREAD_MUTEX_INITIALIZER}};
static uint32_t        pre_regData = 0;

const char* dma_name[4] = {"dma_iic0", "dma_iic1", "dma_iic2", "dma_iic3"};
//=============================================================================
//                              Private Function Declaration
//=============================================================================
static void*
IicSlavePollingThread(
    void* arg);

static void
Iic_Check_Iint(
    IIC_PORT port);

static bool
IicInitDevice(
    IIC_PORT    port,
    IIC_OP_MODE opMode);

static bool
IicInitDeviceHw(
    IIC_PORT    port,
    uint32_t    clock,
    uint32_t    delay);

static void
IicTerminateDevice(
    IIC_PORT port);

static void
IicTerminateDeviceHw(
    IIC_PORT port);

static uint32_t
IicSetClock(
    IIC_PORT    port,
    uint32_t    clock);

static bool
IicWaitTxFifoCount(
    IIC_PORT     port,
    uint32_t    requestCount,
    int32_t     timeout);

static bool
IicWaitRxFifoCount(
    IIC_PORT    port,
    uint32_t    requestCount,
    int32_t     timeout);

static uint32_t
IicFiFoSendData(
    IIC_PORT        port,
    uint8_t         slaveAddr,
    uint8_t*        inData,
    uint32_t        inDataSize,
    IIC_DATA_MODE   wStop);

static uint32_t
IicFiFoReceiveData(
    IIC_PORT    port,
    uint8_t     slaveAddr,
    uint8_t*    outData,
    uint32_t    outDataSize);

bool
IicSlaveReadWithCallback(
    IIC_PORT port);

void
IicSlaveFifoReadWithCallback(
    IIC_PORT port);

static uint32_t
IicSlaveCheckAck(
    IIC_PORT    port,
    IIC_STATE   state);

#ifdef IIC_USE_DMA
#ifdef IIC_USE_DMA_INTR
static void
IicDmaIntr(
    int         ch,
    void*       arg,
    uint32_t    int_status);
#endif

static bool
IicWaitDmaIdle(
    IIC_PORT port);

static uint32_t
IicFiFoDmaSendData(
    IIC_PORT        port,
    uint8_t         slaveAddr,
    uint8_t*        pbuffer,
    uint32_t        size,
    IIC_DATA_MODE   wStop);

static uint32_t
IicFiFoDmaReceiveData(
    IIC_PORT    port,
    uint8_t     slaveAddr,
    uint8_t*    pbuffer,
    uint32_t    size);
#endif

//=============================================================================
//                              Public Function Definition
//=============================================================================
uint32_t
mmpIicInitialize(
    IIC_PORT     port,
    IIC_OP_MODE  opMode,
    uint32_t     sclk_pin,
    uint32_t     data_pin,
    uint32_t     initClock,   // in Hz
    uint32_t     delay)
{
    uint32_t    result = 0;

    //printf("mmpIicInitialize\n");
    if (port < IIC_PORT_0  || port >= IIC_PORT_MAX)
    {
        IIC_ERROR_MSG("Invalid port(%d)\n", port);
        return false;
    }

    if (opMode < IIC_SLAVE_MODE || opMode > IIC_MASTER_MODE)
    {
        IIC_ERROR_MSG("Invalid opMode(%d)\n", opMode);
        return false;
    }

    pthread_mutex_lock(&IicDevices[port].funcMutex);

    do
    {
        if (IicDevices[port].refCount == 0)
        {
            // Init SW
            if (IicInitDevice(port, opMode) == false)
            {
                IIC_ERROR_MSG("IicInitDevice() fail.\n");
                break;
            }

            // Init HW
            if (IicInitDeviceHw(port, initClock, delay) == false)
            {
                IIC_ERROR_MSG("IicInitDeviceHw() fail.\n");
                break;
            }

            result = 1;
        }
        else
        {
            IicDevices[port].refCount++;
            IIC_ERROR_MSG("IIC port %d already inited. Reference count is %d\n", port, IicDevices[port].refCount);
            result = 1;
        }
    }while(0);

    // ERROR!
    if (result == 0)
    {
        IicTerminateDeviceHw(port);
        IicTerminateDevice(port);
    }

    pthread_mutex_unlock(&IicDevices[port].funcMutex);

    return result;
}

uint32_t
mmpIicTerminate(
    IIC_PORT     port)
{
    uint32_t result = 0;

    pthread_mutex_lock(&IicDevices[port].funcMutex);

    assert(IicDevices[port].refCount > 0);
    if (--IicDevices[port].refCount == 0)
    {
        IicTerminateDeviceHw(port);
        IicTerminateDevice(port);
    }

    pthread_mutex_unlock(&IicDevices[port].funcMutex);

    return result;
}

uint32_t
mmpIicStart(
    IIC_PORT port)
{
    uint32_t result = 0;
    return result;
}

uint32_t
mmpIicStop(
    IIC_PORT port)
{
    uint32_t result = 0;
    return result;
}

uint32_t
mmpIicRecieve(
    IIC_PORT port,
    uint32_t *data)
{
    return 0;
}

uint32_t
mmpIicSend(
    IIC_PORT port,
    uint32_t data)
{
    return 0;
}

#ifdef CFG_DEV_TEST
DEFINE_COULD_BE_MOCKED_NOT_VOID_FUNC5(
    uint32_t,
    mmpIicSendData,
    IIC_PORT     port,
    IIC_OP_MODE, mode,
    uint8_t,     slaveAddr,
    uint8_t,     regAddr,
    uint8_t*     outData,
    uint32_t     outDataSize)
#else
__attribute__((used)) uint32_t
mmpIicSendData(
    IIC_PORT    port,
    IIC_OP_MODE mode,
    uint8_t     slaveAddr,
    uint8_t     regAddr,
    uint8_t*    outData,
    uint32_t    outDataSize)
#endif
{
    uint32_t result = 0;
    uint8_t  pdbuf[256];
    uint16_t pad_size;

    Iic_Check_Iint(port);

    pthread_mutex_lock(&IicDevices[port].funcMutex);

    do
    {
        pdbuf[0] = regAddr;
        memcpy(&pdbuf[1], outData, outDataSize);
        pad_size    = outDataSize + 1;
#ifdef IIC_USE_DMA
        result = IicFiFoDmaSendData(port, slaveAddr, pdbuf, pad_size, W_STOP);
#else
        result = IicFiFoSendData(port, slaveAddr, pdbuf, pad_size, W_STOP);
#endif
        if (result == 1)
        {
            IIC_ERROR_MSG("Send data fail.\n");
            break;
        }
    }while(0);

    pthread_mutex_unlock(&IicDevices[port].funcMutex);

    return result;
}

uint32_t
mmpIicSendDataEx(
    IIC_PORT    port,
    IIC_OP_MODE mode,
    uint8_t     slaveAddr,
    uint8_t*    outData,
    uint32_t    outDataSize)
{
    uint32_t result = 0;

    Iic_Check_Iint(port);

    pthread_mutex_lock(&IicDevices[port].funcMutex);

    do
    {
#ifdef IIC_USE_DMA
        result = IicFiFoDmaSendData(port, slaveAddr, outData, outDataSize, W_STOP);
#else
        result = IicFiFoSendData(port, slaveAddr, outData, outDataSize, W_STOP);
#endif
        if (result == 1)
        {
            IIC_ERROR_MSG("Send data fail.\n");
            break;
        }
        result = 0;
    }while(0);

    pthread_mutex_unlock(&IicDevices[port].funcMutex);

    return result;
}

#ifdef CFG_DEV_TEST
DEFINE_COULD_BE_MOCKED_NOT_VOID_FUNC5(
    uint32_t,
    mmpIicReceiveData,
    IIC_PORT     port,
    IIC_OP_MODE, mode,
    uint8_t,     slaveAddr,
    uint8_t*     outData,
    uint32_t     outDataSize,
    uint8_t*     inData,
    uint32_t     inDataSize)
#else
uint32_t
mmpIicReceiveData(
    IIC_PORT    port,
    IIC_OP_MODE mode,
    uint8_t     slaveAddr,
    uint8_t*    outData,
    uint32_t    outDataSize,
    uint8_t*    inData,
    uint32_t    inDataSize)
#endif
{
    uint32_t    result = 0;

    Iic_Check_Iint(port);

    pthread_mutex_lock(&IicDevices[port].funcMutex);

    do
    {
        if (outData && outDataSize)
        {
#ifdef IIC_USE_DMA
            //printf("IicFiFoDmaSendData enter\n");
            result = IicFiFoDmaSendData(port, slaveAddr, outData, outDataSize, WO_STOP);
            //printf("IicFiFoDmaSendData leave\n");
#else
            result = IicFiFoSendData(port, slaveAddr, outData, outDataSize, WO_STOP);
#endif
            if (result == 1)
            {
                IIC_ERROR_MSG("Write data fail.\n");
                break;
            }
        }
#ifdef IIC_USE_DMA
        //usleep(3000000);
        //printf("IicFiFoDmaReceiveData enter\n");
        result = IicFiFoDmaReceiveData(port, slaveAddr, inData, inDataSize);
        //printf("IicFiFoDmaReceiveData leave\n");
#else
        result = IicFiFoReceiveData(port, slaveAddr, inData, inDataSize);
#endif
        if (result == 1)
        {
            IIC_ERROR_MSG("Read data fail.\n");
            break;
        }
        result = 0;
    }while(0);

    pthread_mutex_unlock(&IicDevices[port].funcMutex);

    return result;
}

void
mmpIicLockModule(
    IIC_PORT port)
{
    Iic_Check_Iint(port);
    pthread_mutex_lock(&IicDevices[port].funcMutex);
}

void
mmpIicReleaseModule(
    IIC_PORT port)
{
    Iic_Check_Iint(port);
    pthread_mutex_unlock(&IicDevices[port].funcMutex);
}

uint32_t
mmpIicSetClockRate(
    IIC_PORT    port,
    uint32_t    clock)
{
    uint32_t    result = 0;

    Iic_Check_Iint(port);

    pthread_mutex_lock(&IicDevices[port].funcMutex);
    IicDevices[port].clockRate = IicSetClock(port, clock);
    result = IicDevices[port].clockRate;
    pthread_mutex_unlock(&IicDevices[port].funcMutex);

    return result;
}

uint32_t
mmpIicGetClockRate(
    IIC_PORT port)
{
    uint32_t currClock = 0;

    Iic_Check_Iint(port);

    pthread_mutex_lock(&IicDevices[port].funcMutex);
    currClock = IicDevices[port].clockRate;
    pthread_mutex_unlock(&IicDevices[port].funcMutex);

    return currClock;
}

uint32_t
mmpIicGenStop(
    IIC_PORT port)
{
    return 0;
}

void
mmpIicSetSlaveMode(
    IIC_PORT port,
    uint32_t slaveAddr)
{
    /* Set slave address */
    ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_SLAVE_ADDR, slaveAddr);

    /* Write CR: Enable all interrupts and I2C enable, and disable SCL enable */
    ithWriteRegA(
        IicDevices[port].regAddrBase + REG_I2C_ICR,
        REG_BIT_INTR_ALL);

    ithWriteRegA(
        IicDevices[port].regAddrBase + REG_I2C_CONTROL,
        REG_BIT_CONTL_TRANSFER_BYTE |
        //REG_BIT_CONTL_GC |
        REG_BIT_CONTL_I2C_ENABLE);
}

uint32_t
mmpIicSlaveRead(
    IIC_PORT port,
    uint32_t slaveAddr,
    uint8_t *inutBuffer,
    uint32_t inputBufferLength)
{
    printf("IT9860 not support this function!!!\n");
    return 0;
}

bool
mmpIicSetSlaveModeCallback(
    IIC_PORT            port,
    uint32_t            slaveAddr,
    IicReceiveCallback  recvCallback,
    IicWriteCallback    writeCallback)
{
    Iic_Check_Iint(port);

    if (IicDevices[port].opMode != IIC_SLAVE_MODE)
    {
        IIC_ERROR_MSG("Device not in slave mode.\n");
        return false;
    }

    pthread_mutex_lock(&IicDevices[port].funcMutex);

    IicDevices[port].slaveAddr = slaveAddr;
    IicDevices[port].recvCallback = recvCallback;
    IicDevices[port].writeCallback = writeCallback;

    pthread_mutex_unlock(&IicDevices[port].funcMutex);

    return true;
}

#if 0
/**
 * mmpIicSlaveRead()
 *
 * @param outputBuffer: To receive data from IIC master.
 * @param outputBufferLength: The buffer length of outputBuffer.
 *
 * @return MMP_TRUE if data received. MMP_FALSE if no data received.
 */
uint32_t
mmpIicSlaveRead(
    IIC_DEVICE* dev,
    uint8_t*    inutBuffer,
    uint32_t    inputBufferLength)
{
    uint32_t outputBufferWriteIndex = 0;
    uint32_t regData                = 0;

    /* Set slave address */
    ithWriteRegA(REG_I2C_SLAVE_ADDR, dev->slaveAddr);

    /* Write CR: Enable all interrupts and I2C enable, and disable SCL enable */
    ithWriteRegA(
        REG_I2C_CONTROL,
        REG_BIT_CONTL_TRANSFER_BYTE |
        REG_BIT_CONTL_GC |
        REG_BIT_CONTL_I2C_ENABLE);

    ithWriteRegA(REG_I2C_ICR, REG_BIT_INTR_ALL);

    /* Read status, check if recevice matched slave address */
    regData = ithReadRegA(REG_I2C_STATUS);
    if ( IIC_REG_BIT(regData, 8) == 1 &&    /* SAM */
         IIC_REG_BIT(regData, 5) == 1 &&    /* DR */
         IIC_REG_BIT(regData, 2) == 1 &&    /* I2CB */
         IIC_REG_BIT(regData, 1) == 0 &&    /* ACK */
         IIC_REG_BIT(regData, 0) == 0 )     /* RW */
    {
        while (1)
        {
            /* Enable transfer bit */
            ithWriteRegA(
                REG_I2C_CONTROL,
                REG_BIT_CONTL_TRANSFER_BYTE |
                REG_BIT_CONTL_GC |
                REG_BIT_CONTL_I2C_ENABLE);
            ithWriteRegA(REG_I2C_ICR, REG_BIT_INTR_ALL);

            regData = ithReadRegA(REG_I2C_STATUS);
            if ( IIC_REG_BIT(regData, 7) == 1 ) /* Detect STOP condition */
            {
                /* Receive STOP */
                goto end;
            }

            if ( IIC_REG_BIT(regData, 5) == 1   )   /* Receive ack */
            {
                /* Read data */
                uint32_t iicInputData = 0;

                iicInputData = ithReadRegA(REG_I2C_DATA);
                inutBuffer[outputBufferWriteIndex] = iicInputData & 0x000000FF;
                outputBufferWriteIndex++;
                if ( outputBufferWriteIndex >= inputBufferLength )
                {
                    goto end;
                }
            }
        }
    }

end:
    return outputBufferWriteIndex;
}
#endif

uint32_t
mmpIicSlaveFiFoWrite(
    IIC_PORT    port,
    uint8_t*    outputBuffer,
    uint32_t    outputBufferLength)
{
    uint32_t    i = 0;
    uint32_t    result = 0;
    uint32_t    remainderSize = outputBufferLength;
    uint8_t*    dmaWriteBuf = NULL;
    uint16_t*   writeBuffer = NULL;

    // Enable IIC controller
    // Enable IIC SCLK
    // Enable FIFO mode
    ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_CONTROL,
        REG_BIT_CONTL_I2C_ENABLE |
        REG_BIT_CONTL_FIFO_EN);

    // Set endian
    ithWriteRegMaskA(IicDevices[port].regAddrBase + REG_I2C_CONTROL, 1 << 9, 1 << 9);

    // Set slave address
    ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_SLAVE_ADDR, IicDevices[port].slaveAddr);

#ifdef IIC_USE_DMA
    // Prepare data buffer
    do
    {
        uint8_t*    mappedSysRam = NULL;

        writeBuffer = (uint16_t*)calloc(1, outputBufferLength * sizeof(uint16_t));
        if (writeBuffer == NULL)
        {
            printf("ERROR at %s[%d]\n", __FUNCTION__, __LINE__);
            break;
        }
        for (i = 0; i <= outputBufferLength; i++)
        {
            writeBuffer[i] = outputBuffer[i];
        }

        dmaWriteBuf = (uint8_t *)itpVmemAlloc(outputBufferLength * sizeof(uint16_t));
        if(dmaWriteBuf == NULL)
        {
            printf("ERROR at %s[%d]\n", __FUNCTION__, __LINE__);
            break;
        }

        mappedSysRam = ithMapVram((uint32_t)dmaWriteBuf, outputBufferLength * sizeof(uint16_t), ITH_VRAM_WRITE);
        memcpy(mappedSysRam, writeBuffer, outputBufferLength * sizeof(uint16_t));
        ithFlushDCacheRange((void*)mappedSysRam, outputBufferLength * sizeof(uint16_t));
        ithUnmapVram(mappedSysRam, outputBufferLength * sizeof(uint16_t));

        ithDmaSetSrcAddr(IicDevices[port].dmaChannel, (uint32_t)dmaWriteBuf);
        ithDmaSetSrcParams(IicDevices[port].dmaChannel, ITH_DMA_WIDTH_16, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
        ithDmaSetDstParams(IicDevices[port].dmaChannel, ITH_DMA_WIDTH_16, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
        ithDmaSetTxSize(IicDevices[port].dmaChannel, outputBufferLength * 2);
        ithDmaSetBurst(IicDevices[port].dmaChannel, ITH_DMA_BURST_4);
        ithDmaSetDstAddr(IicDevices[port].dmaChannel, IicDevices[port].regAddrBase + REG_I2C_DATA);
        ithDmaSetRequest(IicDevices[port].dmaChannel, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM, ITH_DMA_HW_HANDSHAKE_MODE, IIC0_DMA_TX_CH + port*2);
        ithDmaStart(IicDevices[port].dmaChannel);   // Fire DMA

        // Set threshold
        ithWriteRegMaskA(IicDevices[port].regAddrBase + REG_I2C_ICR, 4 << 20, 0xF << 20);

        // Enable TX DMA
        ithWriteRegMaskA(IicDevices[port].regAddrBase + REG_I2C_ICR, 0x02, 0x02);

        if (IicWaitDmaIdle(port) == false)
        {
            printf("ERROR at %s[%d]\n", __FUNCTION__, __LINE__);
            result = 1;
            break;
        }
        // Disable TX DMA
        ithWriteRegMaskA(IicDevices[port].regAddrBase + REG_I2C_ICR, 0x00, 0x02);

        result = 0;
    }while(0);

    if (writeBuffer)
    {
        free(writeBuffer);
    }
    if (dmaWriteBuf)
    {
        itpVmemFree((uint32_t)dmaWriteBuf);
    }
#else
    while(remainderSize)
    {
        uint32_t    sendData = 0;
        if (IicWaitTxFifoCount(port, 1, 1000) == false)
        {
            IIC_ERROR_MSG("Wait TX FIFO timeout(status=0x%x)!\n", ithReadRegA(IicDevices[port].regAddrBase + REG_I2C_STATUS));
            result = 1;
            break;
        }

        sendData = outputBuffer[i];
        ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_DATA, (uint8_t)sendData);

        i++;
        remainderSize--;
    }
#endif

    return result;
}

uint32_t
mmpIicSlaveWrite(
    IIC_PORT    port,
    uint32_t    slaveAddr,
    uint8_t*    outputBuffer,
    uint32_t    outputBufferLength)
{
    uint32_t iicResult  = 0;
    uint32_t regData    = 0;
    uint32_t writeIndex = 0;

    Iic_Check_Iint(port);

    //pthread_mutex_lock(&dev->funcMutex);

    /* Set slave address */
    ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_SLAVE_ADDR, IicDevices[port].slaveAddr);

    for ( writeIndex = 0; writeIndex < outputBufferLength; writeIndex++ )
    {
        ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_DATA, outputBuffer[writeIndex]);
        /* Enable transfer bit */
        /* Write CR: Enable all interrupts and I2C enable, and disable SCL enable */
        ithWriteRegMaskA(IicDevices[port].regAddrBase + REG_I2C_CONTROL,
            REG_BIT_CONTL_TRANSFER_BYTE,
            REG_BIT_CONTL_TRANSFER_BYTE);
        //ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_ICR, REG_BIT_INTR_ALL);

        /* Check ACK */
        iicResult = IicSlaveCheckAck(port, TRANSMIT_DATA);
        if ( iicResult )
        {
            // Check ACK fail
            goto end;
        }
    }

end:
    //pthread_mutex_unlock(&dev->funcMutex);
    return iicResult;
}


//=============================================================================
//                              Private Function Definition
//=============================================================================
static void*
IicSlavePollingThread(
    void* arg)
{
    IIC_PORT port = (IIC_PORT)arg;

    while(IicDevices[port].stopSlaveThread == false)
    {
        pthread_mutex_lock(&IicDevices[port].funcMutex);
        //IicSlaveReadWithCallback(port);
        IicSlaveFifoReadWithCallback(port);
        pthread_mutex_unlock(&IicDevices[port].funcMutex);
        usleep(1000);
    }

    pthread_exit(NULL);
    return 0;
}

static void
Iic_Check_Iint(
    IIC_PORT port)
{
    if (port >= IIC_PORT_MAX)       { IIC_ERROR_MSG("Invalid device!\n"); assert(0); }
    if (IicDevices[port].refCount < 1 || IicDevices[port].magicID != IIC_MAGIC_ID_0 + port) { IIC_ERROR_MSG("Device not init!\n"); assert(0); } \
}

static bool
IicInitDevice(
    IIC_PORT    port,
    IIC_OP_MODE opMode)
{
    bool result = false;

    do
    {
        IicDevices[port].magicID = IIC_MAGIC_ID_0 + port;
        IicDevices[port].port = port;
        IicDevices[port].refCount++;
        IicDevices[port].opMode = opMode;
        IicDevices[port].regAddrBase = ITH_IIC_BASE + (port * 0x100);
#ifdef IIC_USE_DMA
#ifdef IIC_USE_DMA_INTR
        IicDevices[port].dmaChannel = ithDmaRequestCh(dma_name[port], ITH_DMA_CH_PRIO_HIGH_3, IicDmaIntr, &IicDevices[port]);
#else
        IicDevices[port].dmaChannel = ithDmaRequestCh(dma_name[port], ITH_DMA_CH_PRIO_HIGH_3, NULL, &IicDevices[port]);
#endif
        if (IicDevices[port].dmaChannel < 0)
        {
            IIC_ERROR_MSG("Request DMA fail.\n");
            break;
        }
        ithDmaReset(IicDevices[port].dmaChannel);
#ifdef IIC_USE_DMA_INTR
        if (sem_init(&IicDevices[port].dmaMutex, 0, 0) < 0)
        {
            IIC_ERROR_MSG("Request semaphore fail.\n");
            break;
        }
#endif
#endif
        IicDevices[port].stopSlaveThread = false;
        if (IicDevices[port].opMode == IIC_SLAVE_MODE)
        {
            int threadResult;

            threadResult = pthread_create(&IicDevices[port].slaveThread, NULL, IicSlavePollingThread, (void *)IicDevices[port].port);
            if (threadResult)
            {
                IIC_ERROR_MSG("Create thread for slave mode fail.\n");
                break;
            }
        }

        result = true;
    }while(0);

    if (result == false)
    {
#ifdef IIC_USE_DMA
        if (IicDevices[port].dmaChannel >= 0)
        {
            ithDmaFreeCh(IicDevices[port].dmaChannel);
        }
#ifdef IIC_USE_DMA_INTR
        if (IicDevices[port].dmaMutex.__sem_lock)
        {
            sem_destroy(&IicDevices[port].dmaMutex);
        }
#endif
#endif
        IicDevices[port].stopSlaveThread = true;
    }

    return result;
}

static bool
IicInitDeviceHw(
    IIC_PORT    port,
    uint32_t    clock,
    uint32_t    delay)
{
    bool        result = true;
	uint32_t    data;

    do
    {
        // HDMI
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_MISC_SET_REG, (1 << 28), (1 << 29) | (1 << 28));

        // GPIO
#ifndef _WIN32
#ifdef CFG_I2C0_ENABLE
        if (IicDevices[port].port == IIC_PORT_0)
        {
            //for HW not work issue(GPIO 21-24)      
	        if((20 < CFG_GPIO_IIC0_CLK)  && (CFG_GPIO_IIC0_CLK < 25))	
	            IicDevices[port].ioSCLK = CFG_GPIO_IIC0_CLK - 21;
	        else
	            IicDevices[port].ioSCLK = CFG_GPIO_IIC0_CLK;
	        if((20 < CFG_GPIO_IIC0_DATA) && (CFG_GPIO_IIC0_DATA < 25))	
	            IicDevices[port].ioSDA = CFG_GPIO_IIC0_DATA - 21;
            else
	            IicDevices[port].ioSDA = CFG_GPIO_IIC0_DATA;
            ithWriteRegA(ITH_GPIO_BASE + 0x238, (1 << 15) | (1 << 31) | (IicDevices[port].ioSCLK << 16) | (IicDevices[port].ioSDA));
	        //reset IIC0
	        ithWriteRegMaskA(ITH_HOST_BASE + 0x1c, (1 << 18), (1 << 18));
	        usleep(5);
	        ithWriteRegMaskA(ITH_HOST_BASE + 0x1c, (0 << 18), (1 << 18));
        }
#endif
#ifdef CFG_I2C1_ENABLE
        if(IicDevices[port].port == IIC_PORT_1)
        {
            //for HW not work issue(GPIO 21-24)    
            if((20 < CFG_GPIO_IIC1_CLK)  && (CFG_GPIO_IIC1_CLK < 25))
	            IicDevices[port].ioSCLK = CFG_GPIO_IIC1_CLK - 21;
	        else
	            IicDevices[port].ioSCLK = CFG_GPIO_IIC1_CLK;
	        if((20 < CFG_GPIO_IIC1_DATA) && (CFG_GPIO_IIC1_DATA < 25))		
	            IicDevices[port].ioSDA = CFG_GPIO_IIC1_DATA - 21;
	        else
	            IicDevices[port].ioSDA = CFG_GPIO_IIC1_DATA;
            ithWriteRegA(ITH_GPIO_BASE + 0x23c, (1 << 15) | (1 << 31) | (IicDevices[port].ioSCLK << 16) | (IicDevices[port].ioSDA));
	        //reset IIC1
	        ithWriteRegMaskA(ITH_HOST_BASE + 0x1c, (1 << 19), (1 << 19));
	        usleep(5);
	        ithWriteRegMaskA(ITH_HOST_BASE + 0x1c, (0 << 19), (1 << 19));
        }
#endif
#ifdef CFG_I2C2_ENABLE
        if(IicDevices[port].port == IIC_PORT_2)
        {
            //for HW not work issue(GPIO 21-24)    
            if((20 < CFG_GPIO_IIC2_CLK)  && (CFG_GPIO_IIC2_CLK < 25))
	            IicDevices[port].ioSCLK = CFG_GPIO_IIC2_CLK - 21;
            else
                IicDevices[port].ioSCLK = CFG_GPIO_IIC2_CLK;
	        if((20 < CFG_GPIO_IIC2_DATA) && (CFG_GPIO_IIC2_DATA < 25))		
                IicDevices[port].ioSDA = CFG_GPIO_IIC2_DATA - 21;
	        else
	            IicDevices[port].ioSDA = CFG_GPIO_IIC2_DATA;
            ithWriteRegA(ITH_GPIO_BASE + 0x240, (1 << 15) | (1 << 31) | (IicDevices[port].ioSCLK << 16) | (IicDevices[port].ioSDA));
	        //reset IIC2
	        ithWriteRegMaskA(ITH_HOST_BASE + 0x1c, (1 << 20), (1 << 20));
	        usleep(5);
	        ithWriteRegMaskA(ITH_HOST_BASE + 0x1c, (0 << 20), (1 << 20));
        }
#endif
#ifdef CFG_I2C3_ENABLE
        if(IicDevices[port].port == IIC_PORT_3)
        {
            //for HW not work issue(GPIO 21-24)  
            if((20 < CFG_GPIO_IIC3_CLK)  && (CFG_GPIO_IIC3_CLK < 25))
	            IicDevices[port].ioSCLK = CFG_GPIO_IIC3_CLK - 21;
	        else
	            IicDevices[port].ioSCLK = CFG_GPIO_IIC3_CLK;
	        if((20 < CFG_GPIO_IIC3_DATA) && (CFG_GPIO_IIC3_DATA < 25))		
                IicDevices[port].ioSDA = CFG_GPIO_IIC3_DATA - 21;
	        else
	            IicDevices[port].ioSDA = CFG_GPIO_IIC3_DATA;
            ithWriteRegA(ITH_GPIO_BASE + 0x244, (1 << 15) | (1 << 31) | (IicDevices[port].ioSCLK << 16) | (IicDevices[port].ioSDA));
            //reset IIC3
	        ithWriteRegMaskA(ITH_HOST_BASE + 0x1c, (1 << 21), (1 << 21));
	        usleep(5);
	        ithWriteRegMaskA(ITH_HOST_BASE + 0x1c, (0 << 21), (1 << 21));
        }
#endif
#endif
        //Reset HW, but IT970/IT9860 not use, they have another reset mode.
        //ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_CONTROL, REG_BIT_CONTL_I2C_RESET);
        //usleep(5);
        //ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_CONTROL, 0u);

		//Set SDA Hold Time delay from SCL
		if (delay)
    	{
        	data    = ithReadRegA(IicDevices[port].regAddrBase + REG_I2C_GLITCH);
        	data    &= ~(REG_MASK_TSR);
        	data    |= delay;
        	// Setup GSR
        	data    |= (0x7 << 10);
        	ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_GLITCH, data);
    	}
		//Set delay
        IicDevices[port].delay = delay;

		//Set clock rate
        IicDevices[port].clockRate = IicSetClock(port, clock);
        printf(">>>>>>>>>IIC clock=%d\n", IicDevices[port].clockRate);

        ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_CONTROL,
            REG_BIT_CONTL_I2C_ENABLE |
            REG_BIT_CONTL_CLK_ENABLE |
            REG_BIT_CONTL_FIFO_EN);
    }while(0);

    return result;
}

static void
IicTerminateDevice(
    IIC_PORT port)
{
#ifdef IIC_USE_DMA
    if (IicDevices[port].dmaChannel >= 0)
    {
        ithDmaFreeCh(IicDevices[port].dmaChannel);
    }
#ifdef IIC_USE_DMA_INTR
    if (IicDevices[port].dmaMutex.__sem_lock)
    {
        sem_destroy(&IicDevices[port].dmaMutex);
    }
#endif
#endif

    IicDevices[port].stopSlaveThread = true;
    IicDevices[port].refCount = 0;
    IicDevices[port].magicID = 0;
}

static void
IicTerminateDeviceHw(
    IIC_PORT port)
{
    return;
}

static uint32_t
IicSetClock(
    IIC_PORT    port,
    uint32_t    clock)
{
    uint32_t pclk;
    uint32_t div;
    uint32_t glitch;
    uint32_t count;
    uint32_t gsr;
    uint32_t currentClock = 0;

#if defined(__OPENRTOS__) || defined(WIN32)
    pclk = ithGetBusClock();
#elif defined(__FREERTOS__)
    pclk = or32_getBusCLK();
#else
    //pclk = REF_CLOCK;//fpga extern clk
    // For FPGA
    pclk = 27000000/2;
#endif

    glitch        = ithReadRegA(IicDevices[port].regAddrBase + REG_I2C_GLITCH);
    div           = (pclk/clock);
    gsr           = (glitch & REG_MASK_GSR) >> REG_SHIFT_GSR;
    count         = (((div - gsr - 4) / 2) & REG_MASK_CLK_DIV_COUNT) ; //count = (((div + gsr -1)/2) & REG_MASK_CLK_DIV_COUNT) ;
    currentClock = (uint32_t)(pclk / (2 * count + gsr + 4)); //gCurrentClock = (uint32_t)(pclk/(2*count - gsr +1));

    //set i2c div
    ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_CLOCK_DIV, count);

    return currentClock;
}

static bool
IicWaitTxFifoCount(
    IIC_PORT    port,
    uint32_t    requestCount,
    int32_t     timeout)
{
    bool    result = true;

    while(1)
    {
        uint32_t txFifoEntries = (ithReadRegA(IicDevices[port].regAddrBase + REG_I2C_STATUS) >> 20) & 0x0F;

        if (txFifoEntries >= requestCount)
        {
            break;
        }
        else
        {
            usleep(1);
            if (--timeout <= 0)
            {
                result = false;
                break;
            }
        }
    }

    return result;
}

static bool
IicWaitRxFifoCount(
    IIC_PORT    port,
    uint32_t    requestCount,
    int32_t     timeout)
{
    bool    result = true;

    while(1)
    {
        uint32_t rxFifoEntries = (ithReadRegA(IicDevices[port].regAddrBase + REG_I2C_STATUS) >> 24) & 0x0F;

        if (rxFifoEntries >= requestCount)
        {
            break;
        }
        else
        {
            usleep(1);
            if (--timeout <= 0)
            {
                result = false;
                break;
            }
        }
    }

    return result;
}

static uint32_t
IicFiFoSendData(
    IIC_PORT        port,
    uint8_t         slaveAddr,
    uint8_t*        inData,
    uint32_t        inDataSize,
    IIC_DATA_MODE   wStop)
{
    uint32_t    result = 0;
    uint32_t    i = 0;
    uint32_t    sendData = 0;
    uint32_t    remainderSize = inDataSize;
    uint32_t    timeout = 1000;

    // Enable IIC controller
    // Enable IIC SCLK
    // Enable FIFO mode
	if(IicWaitTxFifoCount(port, 1, 10000) == false)
	{
		// reset HW
		printf("(%s)reset IIC when TX FIFO still full\n", __func__);
        if (IicInitDeviceHw(port, IicDevices[port].clockRate, IicDevices[port].delay) == false)
        {
            IIC_ERROR_MSG("IicInitDeviceHw() reset fail.\n");
        }
		result = 1;
		return result;
	}
    //usleep(1000);
    slaveAddr <<= 1;
    slaveAddr &= ~REG_BIT_READ_ENABLE;

    sendData = slaveAddr;
    //sendData |= (0 << 10);  // ACK bit
    sendData |= (1 << 8);   // Start bit

    // Byte 1, address
    ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_DATA, (uint16_t)sendData);

    while(remainderSize)
    {
        if (IicWaitTxFifoCount(port, 1, 10000) == false)
        {
            IIC_ERROR_MSG("Wait TX FIFO timeout(status=0x%x)!\n", ithReadRegA(IicDevices[port].regAddrBase + REG_I2C_STATUS));
            result = 1;
            break;
        }

        sendData = inData[i];
        //sendData |= (0 << 10);  // ACK bit
        if (wStop && remainderSize == 1)
        {
            // Last byte
            sendData |= (1 << 9);   // Stop bit
        }
        ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_DATA, (uint16_t)sendData);

        i++;
        remainderSize--;
    }

    return result;
}

static uint32_t
IicFiFoReceiveData(
    IIC_PORT    port,
    uint8_t     slaveAddr,
    uint8_t*    outData,
    uint32_t    outDataSize)
{
    uint32_t    i;
    uint32_t    result = 0;
    uint32_t    addrData = 0;
    uint32_t    readData = 0;
    uint32_t    remainderSize = outDataSize;

    // Enable IIC controller
    // Enable IIC SCLK
    // Enable FIFO mode
    #if 0
    ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_CONTROL,
        REG_BIT_CONTL_I2C_ENABLE |
        REG_BIT_CONTL_CLK_ENABLE |
        REG_BIT_CONTL_FIFO_EN);
    #endif

	if(IicWaitTxFifoCount(port, 1, 10000) == false)
	{
		// reset HW
		printf("(%s)reset IIC when TX FIFO still full\n", __func__);
        if (IicInitDeviceHw(port, IicDevices[port].clockRate, IicDevices[port].delay) == false)
        {
            IIC_ERROR_MSG("IicInitDeviceHw() reset fail.\n");
        }
		result = 1;
		return result;
	}

    slaveAddr <<= 1;
    slaveAddr |= REG_BIT_READ_ENABLE;

    addrData = slaveAddr;
    //addrData |= (0 << 10);  // ACK bit
    addrData |= (1 << 8);   // Start bit

    // Byte 1, address
    ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_DATA, (uint16_t)addrData);
    i = 0;
	//set recv size and enable free run mode(auto ack)
    ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_DATA_RCV_STATE, (remainderSize << 16) | 0x7);

    while(remainderSize)
    {
        //uint32_t dummyBytes = 0;

        //dummyBytes |= (0 << 10);  // ACK bit
        //if (remainderSize == 1)
        //{
            // Last byte
        //  dummyBytes |= (1 << 10);    // NACK bit
        //  dummyBytes |= (1 << 9); // Stop bit
        //}
        //ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_DATA, (uint16_t)dummyBytes);

        if (IicWaitRxFifoCount(port, 1, 10000) == false)
        {
            IIC_ERROR_MSG("Wait RX FIFO timeout(status=0x%x)!\n", ithReadRegA(IicDevices[port].regAddrBase + REG_I2C_STATUS));
			//IIC_ERROR_MSG("read data(status=0x%x)!\n", ithReadRegA(IicDevices[port].regAddrBase + REG_I2C_DATA_RCV_STATE));
            result = 1;
            break;
        }

#ifdef _WIN32
        readData = ithReadRegH(0x8000 + 0x0C);  // REG_I2C_DATA
#else
        readData = ithReadRegA(IicDevices[port].regAddrBase + REG_I2C_DATA);
#endif
        readData = readData & 0xFF;
        outData[i] = readData;

        i++;
        remainderSize--;
    }

    return result;
}

bool
IicSlaveReadWithCallback(
    IIC_PORT port)
{
    uint32_t outputBufferWriteIndex = 0;
    uint32_t regData = 0, regData2 = 0;

    /* Set slave address */
    //ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_SLAVE_ADDR, IicDevices[port].slaveAddr);

    /* Write CR: Enable all interrupts and I2C enable, and disable SCL enable */
    //ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_CONTROL,
    //  REG_BIT_CONTL_TRANSFER_BYTE |
    //  REG_BIT_CONTL_I2C_ENABLE);
    //ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_ICR, REG_BIT_INTR_ALL);

    //usleep(1000);
    /* Read status, check if recevice matched slave address */
    //printf("-----------------\n");

    //read/write start flag in non-define register
    regData = ithReadRegA(IicDevices[port].regAddrBase + REG_I2C_FEATURE);
    if(IIC_REG_BIT(regData, 12) == 1 || IIC_REG_BIT(regData, 13) == 1)
    {
        regData = ithReadRegA(IicDevices[port].regAddrBase + REG_I2C_STATUS) | pre_regData;
        pre_regData = 0x0;
        //printf("reg=0x%x\n", regData);
    }
    if ( IIC_REG_BIT(regData, 8) == 1 &&    /* SAM */
         IIC_REG_BIT(regData, 5) == 1 &&    /* DR */
         IIC_REG_BIT(regData, 2) == 1 &&    /* I2CB */
         IIC_REG_BIT(regData, 1) == 0 )
    {
        if ( IIC_REG_BIT(regData, 0) == 0 ) /* RW */
        {
            // Slave read
            /* Enable transfer bit */
            ithWriteRegMaskA(
                IicDevices[port].regAddrBase + REG_I2C_CONTROL,
                REG_BIT_CONTL_TRANSFER_BYTE,
                REG_BIT_CONTL_TRANSFER_BYTE);
        }
        while (1)
        {
            if ( IIC_REG_BIT(regData, 0) == 0 ) /* RW */
            {
                regData2 = ithReadRegA(IicDevices[port].regAddrBase + REG_I2C_STATUS);
                if ( IIC_REG_BIT(regData2, 7) == 1 )    /* Detect STOP condition */
                {
                    /* Receive STOP */
                    pre_regData = regData2 & (0xFFFFFF7F);
                    goto end;
                }
                if ( IIC_REG_BIT(regData2, 11) == 1 )   /* Detect START condition */
                {
                    /* Receive START */
                    pre_regData = regData2;
                    goto end;
                }
                if ( IIC_REG_BIT(regData2, 5) == 1 )
                {
                    /* Read data */
                    uint32_t iicInputData = 0;

                    iicInputData = ithReadRegA(IicDevices[port].regAddrBase + REG_I2C_DATA);
                    // Slave read
                    /* Enable transfer bit */
                    ithWriteRegMaskA(
                        IicDevices[port].regAddrBase + REG_I2C_CONTROL,
                        REG_BIT_CONTL_TRANSFER_BYTE,
                        REG_BIT_CONTL_TRANSFER_BYTE);

                    if ( IicDevices[port].recvCallback )
                    {
                        IicDevices[port].recvCallback((uint8_t*)&iicInputData, 1);
                        outputBufferWriteIndex++;
                        //while(1);
                    }
                }
            }
            else if ( IIC_REG_BIT(regData, 0) == 1 )    /* RW */
            {
                if ( IIC_REG_BIT(regData, 7) == 1 ) /* Detect STOP condition */
                {
                    /* Receive STOP */
                    goto end;
                }

                // Slave write
                if ( IicDevices[port].writeCallback )
                {
                    IicDevices[port].writeCallback(IicDevices[port]);
                    goto end;
                    //gslavethread = false;
                    //break;
                }
                //regData = ithReadRegA(IicDevices[port].regAddrBase + REG_I2C_STATUS);
            }
        }
    }

end:
    return outputBufferWriteIndex;
}

void
IicSlaveFifoReadWithCallback(
    IIC_PORT port)
{
	static uint32_t     slaveBufferReadIndex = 0;
	static uint8_t     	slave_readData[256]  = {0};
	static bool         read_data_end        = true;
    uint32_t 			regData              = 0;

    /* Set slave address */
    ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_SLAVE_ADDR, IicDevices[port].slaveAddr);

    ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_CONTROL,
        REG_BIT_CONTL_CHECK_DOWN_EN |
        REG_BIT_CONTL_I2C_ENABLE |
        REG_BIT_CONTL_FIFO_EN);
    ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_ICR, REG_BIT_INTR_ALL);

    /* Read status, check if recevice matched slave address */
    regData = ithReadRegA(IicDevices[port].regAddrBase + REG_I2C_STATUS);
    if (IIC_REG_BIT(regData, 28) == 1)  /* REQUEST DOWN */
    {
        if ( IIC_REG_BIT(regData, 0) == 0  || read_data_end == false) /* W or read data not end */
        {
            printf("Slave in READ\n");
            {
#ifdef IIC_USE_DMA
                uint8_t*    dmaReadBuf = NULL;
                uint8_t*    recvBuffer = NULL;
                uint32_t    recvDataSize = (regData >> 24) & 0xF;


                dmaReadBuf = (uint8_t *)itpVmemAlloc(recvDataSize);
                if(dmaReadBuf == NULL)
                {
                    printf("ERROR at %s[%d]\n", __FUNCTION__, __LINE__);
                }

                recvBuffer = (uint8_t*)malloc(recvDataSize);
                assert(recvBuffer != NULL);

                ithDmaSetDstAddr(IicDevices[port].dmaChannel, (uint32_t)dmaReadBuf);
                ithDmaSetSrcParams(IicDevices[port].dmaChannel, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
                ithDmaSetDstParams(IicDevices[port].dmaChannel, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
                ithDmaSetTxSize(IicDevices[port].dmaChannel, recvDataSize);
                ithDmaSetBurst(IicDevices[port].dmaChannel, ITH_DMA_BURST_1);
                ithDmaSetSrcAddr(IicDevices[port].dmaChannel, IicDevices[port].regAddrBase + REG_I2C_DATA);
                ithDmaSetRequest(IicDevices[port].dmaChannel, ITH_DMA_HW_HANDSHAKE_MODE, IIC0_DMA_TX_CH + port*2 + 1, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM);
                ithDmaStart(IicDevices[port].dmaChannel);   // Fire DMA

                // Set threshold
                ithWriteRegMaskA(IicDevices[port].regAddrBase + REG_I2C_ICR, 1 << 16, 0xF << 16);

                // Enable RX DMA
                ithWriteRegMaskA(IicDevices[port].regAddrBase + REG_I2C_ICR, 0x01, 0x01);

                if (IicWaitDmaIdle(port) == false)
                {
                    printf("ERROR at %s[%d]\n", __FUNCTION__, __LINE__);
                }

                {
                    uint8_t* mappedSysRam = NULL;

                    mappedSysRam = ithMapVram((uint32_t)dmaReadBuf, recvDataSize, ITH_VRAM_READ);
                    ithInvalidateDCacheRange(mappedSysRam, recvDataSize);
                    memcpy(recvBuffer, mappedSysRam, recvDataSize);
                    ithUnmapVram(mappedSysRam, recvDataSize);
                }

                // TEST CODE
                //// Disable RX DMA
                //ithWriteRegMaskA(dev->regAddrBase + REG_I2C_ICR, 0x00, 0x01);

                if (dmaReadBuf)
                {
                    itpVmemFree((uint32_t)dmaReadBuf);
                }

                if ( IicDevices[port].recvCallback )
                {
                    IicDevices[port].recvCallback(recvBuffer, recvDataSize);
                }
                free(recvBuffer);

                if (IIC_REG_BIT(regData, 29) == 1)
                    ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_STATUS,
                        REG_BIT_STATUS_DATA_END_CLEAR |
                        REG_BIT_STATUS_CHECK_CLEAR);
#else
                uint8_t     i, recvDataSize = (regData >> 24) & 0xF;
				if(recvDataSize > 8)
					printf("size=%d, should less than 8!\n", recvDataSize);
				else if(recvDataSize > 0)
				{
	                for(i = 0; i < recvDataSize; i++)
	                {
	                    slave_readData[slaveBufferReadIndex] = ithReadRegA(IicDevices[port].regAddrBase + REG_I2C_DATA);
	                    slave_readData[slaveBufferReadIndex] = slave_readData[slaveBufferReadIndex] & 0xFF;
	                    slaveBufferReadIndex++;
	                }
				}
                if (IIC_REG_BIT(regData, 29) == 1)
                {
                	if ( IicDevices[port].recvCallback )
	                {
	                    IicDevices[port].recvCallback(slave_readData, slaveBufferReadIndex);
	                }

                    ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_STATUS,
                        REG_BIT_STATUS_DATA_END_CLEAR |
                        REG_BIT_STATUS_CHECK_CLEAR);
					slaveBufferReadIndex = 0;
					read_data_end = true;
					//goto end;
				}else
					read_data_end = false;
#endif
            }
        }
        else if ( IIC_REG_BIT(regData, 0) == 1 )    /* R */
        {
            printf("Slave in WRITE\n");
            // Slave write
            if ( IicDevices[port].writeCallback )
            {
                IicDevices[port].writeCallback(IicDevices[port]);
                ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_STATUS,
                    REG_BIT_STATUS_DATA_END_CLEAR |
                    REG_BIT_STATUS_CHECK_CLEAR);
                //goto end;
            }
        }
    }
    else
    {
        //printf("Slave Address Not Match!\n");
    }

end:
    return;
}


static uint32_t
IicSlaveCheckAck(
    IIC_PORT    port,
    IIC_STATE   state)
{
    uint32_t    i      = 0;
    uint32_t    data   = 0;
    uint32_t    result = 0;

    while(++i)
    {
        data = ithReadRegA(IicDevices[port].regAddrBase + REG_I2C_STATUS);

        if(state & TRANSMIT_DATA)
        {
            if(data & REG_BIT_STATUS_DATA_TRANSFER_DONE)
            {
                if(data & REG_BIT_STATUS_NON_ACK)
                {
                    result = I2C_NON_ACK;
                    goto end;
                }
                if(data & REG_BIT_STATUS_ARBITRATION_LOSS)
                {
                    result = I2C_ARBITRATION_LOSS;
                    goto end;
                }

                if(state & RECRIVE_SERVICE)
                {
                    if(!(data & REG_BIT_STATUS_RECEIVE_MODE))
                    {
                        printf("[%d] ERROR~~~~~\n", __LINE__);
                        result = I2C_MODE_TRANSMIT_ERROR;
                        goto end;
                    }
                }

                //no error and leave loop
                result = 0;
                goto end;
            }
        }
        else if(state & RECEIVE_DATA)
        {
            if(data & REG_BIT_STATUS_DATA_RECEIVE_DONE)
            {
                if(state & NACK_EN)
                {
                    if(!(data & REG_BIT_STATUS_NON_ACK))
                    {
                        result = I2C_INVALID_ACK;
                        goto end;
                    }
                }
                else
                {
                    if(data & REG_BIT_STATUS_NON_ACK)
                    {
                        result = I2C_NON_ACK;
                        goto end;
                    }
                }

                if(!(data & REG_BIT_STATUS_RECEIVE_MODE))
                {
                    result = I2C_MODE_RECEIVE_ERROR;
                    goto end;
                }
                //no error and leave loop
                result = 0;
                goto end;
            }
        }

        if(i > 20000)
        {
            result = I2C_WAIT_TRANSMIT_TIME_OUT;
            goto end;

        }
    }

end:
    return result;
}



#ifdef IIC_USE_DMA
#ifdef IIC_USE_DMA_INTR
static void
IicDmaIntr(
    int         ch,
    void*       arg,
    uint32_t    int_status)
{
    IIC_DEVICE* dev = (IIC_DEVICE*)arg;

    itpSemPostFromISR(&dev->dmaMutex);
}
#endif

static bool
IicWaitDmaIdle(
    IIC_PORT port)
{
#ifdef IIC_USE_DMA_INTR
    return (itpSemWaitTimeout(&IicDevices[port].dmaMutex, 5000) > 0) ? false : true;
#else
    uint32_t timeout_ms = 60000;

    while(ithDmaIsBusy(IicDevices[port].dmaChannel) && --timeout_ms)
    {
        //printf("IicWaitDmaIdle IIC + 0x18 = 0x%X\n", ithReadRegA(IicDevices[port].regAddrBase + REG_I2C_BUS_MONITOR));
        usleep(1000);
    }

    return (!timeout_ms) ? false : true;
#endif
}

static uint32_t
IicFiFoDmaSendData(
    IIC_PORT        port,
    uint8_t         slaveAddr,
    uint8_t*        pbuffer,
    uint32_t        size,
    IIC_DATA_MODE   wStop)
{
    uint32_t    i, j;
    uint32_t    result = 0;
    uint32_t    sendData = 0;
    uint32_t    remainderSize = size;
    uint16_t*   writeBuffer = NULL;
    uint8_t*    dmaWriteBuf = NULL;

    // Enable IIC controller
    // Enable IIC SCLK
    // Enable FIFO mode
    ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_CONTROL,
        REG_BIT_CONTL_I2C_ENABLE |
        REG_BIT_CONTL_CLK_ENABLE |
        REG_BIT_CONTL_FIFO_EN);

    // Set endian
    ithWriteRegMaskA(IicDevices[port].regAddrBase + REG_I2C_CONTROL, 1 << 9, 1 << 9);
    usleep(1000);

    slaveAddr <<= 1;
    slaveAddr &= ~REG_BIT_READ_ENABLE;

    // Prepare data buffer
    do
    {
        uint8_t* mappedSysRam = NULL;

        writeBuffer = (uint16_t*)calloc(1, (size + 1) * sizeof(uint16_t));
        if (writeBuffer == NULL)
        {
            printf("ERROR at %s[%d]\n", __FUNCTION__, __LINE__);
            break;
        }

        // slave addr
        writeBuffer[0] = slaveAddr;
        writeBuffer[0] |= (0 << 10);    // ACK bit
        writeBuffer[0] |= (1 << 8);     // Start bit

        for (i = 1, j = 0; i <= size; i++, j++)
        {
            writeBuffer[i] = pbuffer[j];
            if (wStop && i == size)
            {
                writeBuffer[i] |= (1 << 9); // Stop bit
            }
        }

        dmaWriteBuf = (uint8_t *)itpVmemAlloc((size + 1) * sizeof(uint16_t));
        if(dmaWriteBuf == NULL)
        {
            printf("ERROR at %s[%d]\n", __FUNCTION__, __LINE__);
            break;
        }

        mappedSysRam = ithMapVram((uint32_t)dmaWriteBuf, (size + 1) * sizeof(uint16_t), ITH_VRAM_WRITE);
        memcpy(mappedSysRam, writeBuffer, (size + 1) * sizeof(uint16_t));
        ithFlushDCacheRange((void*)mappedSysRam, (size + 1) * sizeof(uint16_t));
        ithUnmapVram(mappedSysRam, (size + 1) * sizeof(uint16_t));
        ithFlushMemBuffer();

        ithDmaSetSrcAddr(IicDevices[port].dmaChannel, (uint32_t)dmaWriteBuf);
        ithDmaSetSrcParams(IicDevices[port].dmaChannel, ITH_DMA_WIDTH_16, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
        ithDmaSetDstParams(IicDevices[port].dmaChannel, ITH_DMA_WIDTH_16, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
        ithDmaSetTxSize(IicDevices[port].dmaChannel, (size + 1) * 2);
        ithDmaSetBurst(IicDevices[port].dmaChannel, ITH_DMA_BURST_4);
        ithDmaSetDstAddr(IicDevices[port].dmaChannel, IicDevices[port].regAddrBase + REG_I2C_DATA);
        ithDmaSetRequest(IicDevices[port].dmaChannel, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM, ITH_DMA_HW_HANDSHAKE_MODE, IIC0_DMA_TX_CH + port*2);
        ithDmaStart(IicDevices[port].dmaChannel);   // Fire DMA

        // Set threshold
        ithWriteRegMaskA(IicDevices[port].regAddrBase + REG_I2C_ICR, 4 << 20, 0xF << 20);

        // Enable TX DMA
        ithWriteRegMaskA(IicDevices[port].regAddrBase + REG_I2C_ICR, 0x02, 0x02);

        //printf("IicFiFoDmaSendData\n");
        //usleep(1000*10);
        if (IicWaitDmaIdle(port) == false)
        {
            printf("ERROR at %s[%d]\n", __FUNCTION__, __LINE__);
            break;
        }

        // Disable TX DMA
        ithWriteRegMaskA(IicDevices[port].regAddrBase + REG_I2C_ICR, 0x00, 0x02);

        result = 0;
    }while(0);

    if (writeBuffer)
    {
        free(writeBuffer);
    }

    if (dmaWriteBuf)
    {
        itpVmemFree((uint32_t)dmaWriteBuf);
    }

    return result;
}

static uint32_t
IicFiFoDmaReceiveData(
    IIC_PORT    port,
    uint8_t     slaveAddr,
    uint8_t*    pbuffer,
    uint32_t    size)
{
    uint32_t    i;
    uint32_t    result = 0;
    uint32_t    addrData = 0;
    uint32_t    readData = 0;
    uint32_t    remainderSize = size;
    uint8_t*    dmaReadBuf = NULL;

    // Enable IIC controller
    // Enable IIC SCLK
    // Enable FIFO mode
    ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_CONTROL,
        REG_BIT_CONTL_I2C_ENABLE |
        REG_BIT_CONTL_CLK_ENABLE |
        REG_BIT_CONTL_FIFO_EN);

    slaveAddr <<= 1;
    slaveAddr |= REG_BIT_READ_ENABLE;

    addrData = slaveAddr;
    addrData |= (0 << 10);  // ACK bit
    addrData |= (1 << 8);   // Start bit

    // Byte 1, address
    ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_DATA, (uint16_t)addrData);

    dmaReadBuf = (uint8_t *)itpVmemAlloc(size);
    if(dmaReadBuf == NULL)
    {
        printf("ERROR at %s[%d]\n", __FUNCTION__, __LINE__);
    }

    ithDmaSetDstAddr(IicDevices[port].dmaChannel, (uint32_t)dmaReadBuf);
    ithDmaSetSrcParams(IicDevices[port].dmaChannel, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
    ithDmaSetDstParams(IicDevices[port].dmaChannel, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
    ithDmaSetTxSize(IicDevices[port].dmaChannel, size);
    ithDmaSetBurst(IicDevices[port].dmaChannel, ITH_DMA_BURST_1);
    ithDmaSetSrcAddr(IicDevices[port].dmaChannel, IicDevices[port].regAddrBase + REG_I2C_DATA);
    ithDmaSetRequest(IicDevices[port].dmaChannel, ITH_DMA_HW_HANDSHAKE_MODE,IIC0_DMA_TX_CH + port*2 + 1, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM);
    ithDmaStart(IicDevices[port].dmaChannel);   // Fire DMA

    // Set threshold
    ithWriteRegMaskA(IicDevices[port].regAddrBase + REG_I2C_ICR, 1 << 16, 0xF << 16);

    // Enable RX DMA
    ithWriteRegMaskA(IicDevices[port].regAddrBase + REG_I2C_ICR, 0x01, 0x01);

    //for-loop to push dummy data
    //for (i = 0; i < size; i++)
    //{
    //  uint16_t dummyBytes = 0;
    //
    //  if (IicWaitTxFifoCount(port, 1, 1000) == false)
    //  {
    //      IIC_ERROR_MSG("Wait TX FIFO timeout!\n");
    //      result = 1;
    //      break;
    //  }
    //
    //  dummyBytes |= (0 << 10);    // ACK bit
    //  if (i == (size - 1))
    //  {
    //      // Last byte
    //      dummyBytes |= (1 << 10); //NACK bit
    //      dummyBytes |= (1 << 9); // Stop bit
    //  }
    //  ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_DATA, (uint16_t)dummyBytes);
    //}
    ithWriteRegA(IicDevices[port].regAddrBase + REG_I2C_DATA_RCV_STATE, (size << 16) | 0x7);


    //printf("IicFiFoDmaReceiveData\n");
    //usleep(1000*10);
    if (IicWaitDmaIdle(port) == false)
    {
        printf("ERROR at %s[%d]\n", __FUNCTION__, __LINE__);
        result = 1;
    }

    {
        uint8_t* mappedSysRam = NULL;

        mappedSysRam = ithMapVram((uint32_t)dmaReadBuf, size, ITH_VRAM_READ);
        ithInvalidateDCacheRange(mappedSysRam, size);
        memcpy(pbuffer, mappedSysRam, size);
        ithUnmapVram(mappedSysRam, size);
    }

    // Disable RX DMA
    ithWriteRegMaskA(IicDevices[port].regAddrBase + REG_I2C_ICR, 0x00, 0x01);

    if (dmaReadBuf)
    {
        itpVmemFree((uint32_t)dmaReadBuf);
    }

    return result;
}
#endif

