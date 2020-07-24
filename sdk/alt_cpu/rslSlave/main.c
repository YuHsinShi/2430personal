#include <stdarg.h>

#include "alt_cpu/rslSlave/rslSlave.h"
#include "alt_cpu/alt_cpu_utility.h"

#define ENABLE_TIME_CHECK 0

#define ENDIAN_SWAP16(x) \
        (((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8))

#define ENDIAN_SWAP32(x) \
        (((x & 0x000000FF) << 24) | \
        ((x & 0x0000FF00) <<  8) | \
        ((x & 0x00FF0000) >>  8) | \
        ((x & 0xFF000000) >> 24))

#define MONITOR_STATE_NULL_STATE            0
#define MONITOR_STATE_READ_WAITING_CLK      1
#define MONITOR_STATE_READ_DELAY            2
#define MONITOR_STATE_READ_DATA             3
#define MONITOR_STATE_WRITE_WAITING_CLK     4
#define MONITOR_STATE_WRITE_DELAY           5
#define MONITOR_STATE_WRITE_DATA            6

typedef struct
{
    uint32_t state;
    uint32_t bInit;
    uint32_t clkGpio;
    uint32_t readDataGpio;
    uint32_t writeDataGpio;
    uint32_t firstReadDelayTickCount;
    uint32_t readPeriodTickCount;
    uint32_t firstWriteDelayTickCount;
    uint32_t writePeriodTickCount;
    uint32_t checkTime;    
    uint32_t delayTicks;
    uint32_t rawReadIndex;
    uint32_t rawWriteIndex;
    uint32_t writeRepeatCounter;
    uint32_t bValidData;
    uint32_t prevClkPinVal;
    char     rawWriteData[64];
    char     rawReadData[64];
    char     saveRawReadData[64];
} RSL_SLAVE_HANDLE;

static RSL_SLAVE_HANDLE gptSlaveHandle[RSL_SLAVE_COUNT] = { 0 };
static uint32_t gTickPerUs = 0;

static void slaveProcessInitCmd(void)
{
    RSL_SLAVE_INIT_DATA* pInitData = (RSL_SLAVE_INIT_DATA*) CMD_DATA_BUFFER_OFFSET;
    uint32_t slaveId = ENDIAN_SWAP32(pInitData->slaveId);
    uint32_t riscClk = ENDIAN_SWAP32(pInitData->cpuClock);
    uint8_t* pAddr = (uint8_t*)pInitData;


    if (slaveId < RSL_SLAVE_COUNT)
    {
        RSL_SLAVE_HANDLE *ptHandle = &gptSlaveHandle[slaveId];
        gTickPerUs = riscClk / (1000 * 1000);

        ptHandle->clkGpio = ENDIAN_SWAP32(pInitData->clkGpio);
        ptHandle->readDataGpio = ENDIAN_SWAP32(pInitData->dataGpio);
        ptHandle->writeDataGpio = ENDIAN_SWAP32(pInitData->dataWriteGpio);
        ptHandle->firstReadDelayTickCount = ENDIAN_SWAP32(pInitData->firstReadDelayUs) * gTickPerUs;
        ptHandle->firstWriteDelayTickCount = ENDIAN_SWAP32(pInitData->firstWriteDelayUs) * gTickPerUs;
        ptHandle->readPeriodTickCount = ENDIAN_SWAP32(pInitData->readPeriod) * gTickPerUs;
        ptHandle->writePeriodTickCount = ENDIAN_SWAP32(pInitData->writePeriod) * gTickPerUs;
        ptHandle->bInit = 1;
        ptHandle->bValidData = 0;
        ptHandle->rawReadIndex = 0;
        ptHandle->rawWriteIndex = 0;
        ptHandle->checkTime = 0;
        ptHandle->delayTicks = 0;

        setGpioMode(ptHandle->clkGpio, 0);
        setGpioMode(ptHandle->readDataGpio, 0);
        setGpioMode(ptHandle->writeDataGpio, 0);
        setGpioDir(ptHandle->clkGpio, 1);
        setGpioDir(ptHandle->readDataGpio, 1);
        setGpioDir(ptHandle->writeDataGpio, 1);
    }
}

static void slaveProcessReadRawDataCmd(void)
{
    RSL_SLAVE_READ_RAW_DATA* ptReadRawData = (RSL_SLAVE_READ_RAW_DATA*) CMD_DATA_BUFFER_OFFSET;
    uint32_t slaveId = ENDIAN_SWAP32(ptReadRawData->slaveId);
    int i = 0;

    if (slaveId < RSL_SLAVE_COUNT)
    {
        RSL_SLAVE_HANDLE *ptHandle = &gptSlaveHandle[slaveId];
        if (ptHandle->bValidData)
        {
            ptReadRawData->bSuccess = ENDIAN_SWAP32(ptHandle->bValidData);
            for (i = 0; i < 64; i++)
            {
                ptReadRawData->pReadBuffer[i] = ptHandle->saveRawReadData[i];
            }
        }
        else
        {
            ptReadRawData->bSuccess = 0;
            for (i = 0; i < 64; i++)
            {
                ptReadRawData->pReadBuffer[i] = 0;
            }
        }
        ptHandle->bValidData = 0;
    }
}

static void slaveRawReadDataToSaveData(RSL_SLAVE_HANDLE *ptHandle)
{
    int i = 0;
    for (i = 0; i < 64; i++)
    {
        ptHandle->saveRawReadData[i] = ptHandle->rawReadData[i];
        ptHandle->rawReadData[i] = 0;
    }
    ptHandle->bValidData = 1;
}

static void slaveProcessWriteRawDataCmd(void)
{
    RSL_SLAVE_WRITE_RAW_DATA* ptWriteRawData = (RSL_SLAVE_WRITE_RAW_DATA*) CMD_DATA_BUFFER_OFFSET;
    uint32_t slaveId = ENDIAN_SWAP32(ptWriteRawData->slaveId);
    int i = 0;

    if (slaveId < RSL_SLAVE_COUNT)
    {
        RSL_SLAVE_HANDLE *ptHandle = &gptSlaveHandle[slaveId];
        ptHandle->writeRepeatCounter = ENDIAN_SWAP32(ptWriteRawData->writeRepeatCount);
        for (i = 0; i < 64; i++)
        {
            ptHandle->rawWriteData[i] = ptWriteRawData->pWriteBuffer[i];
        }
    }
}

static void slaveMonitorMasterInput(void)
{
    int i = 0;
    int slaveId = 0;
    int clkValue = 0;
    int dataValue = 0;
    int clockCount = 0;
    int bufferIndex = 0;
    int clockValue = 0;

    for (slaveId = RSL_SLAVE0; slaveId < RSL_SLAVE_COUNT; slaveId++)
    {
        RSL_SLAVE_HANDLE *ptHandle = &gptSlaveHandle[slaveId];
        if (ptHandle->bInit)
        {
            switch(ptHandle->state)
            {
                case MONITOR_STATE_NULL_STATE:
                {
                    //1. Preparation
                    ptHandle->rawReadIndex = 0;
                    ptHandle->rawWriteIndex = 0;
                    ptHandle->prevClkPinVal = 0;
                    ptHandle->checkTime = getCurTimer(0);
                    ptHandle->state = MONITOR_STATE_READ_WAITING_CLK;
                    break;
                }
                case MONITOR_STATE_READ_WAITING_CLK:
                {
                    clockValue = getGpioValue(ptHandle->clkGpio,1);

                    if (clockValue != ptHandle->prevClkPinVal && clockValue)
                    {
                        ptHandle->state = MONITOR_STATE_READ_DELAY;
                        ptHandle->prevClkPinVal = 0;
                        ptHandle->delayTicks = ptHandle->firstReadDelayTickCount;
                        ptHandle->checkTime = getCurTimer(0);
                    }
                    if (ptHandle->state == MONITOR_STATE_READ_WAITING_CLK)
                    {
                        //Check Timeout
                        if (getDuration(0, ptHandle->checkTime) >= 1000 * gTickPerUs)
                        {
                            ptHandle->state = MONITOR_STATE_NULL_STATE;
                        }
                        break;
                    }
                }
                case MONITOR_STATE_READ_DELAY:
                {
                    if (getDuration(0, ptHandle->checkTime) >= ptHandle->delayTicks)
                    {
                        ptHandle->state = MONITOR_STATE_READ_DATA;
                    }
                    else
                    {
                        break;
                    }
                }
                case MONITOR_STATE_READ_DATA:
                {
                    uint32_t bitOffset = ptHandle->rawReadIndex & 0x7;
                    bufferIndex = (ptHandle->rawReadIndex >> 3);

                    dataValue = getGpioValue(ptHandle->readDataGpio,1);
                    ptHandle->rawReadData[bufferIndex] |= (dataValue << bitOffset);
                    ptHandle->checkTime = getCurTimer(0);

                    if (bitOffset == 4)
                    {
                        if (bufferIndex == 63)
                        {
                            ptHandle->state = MONITOR_STATE_WRITE_WAITING_CLK;
                        }
                        else
                        {
                            ptHandle->state = MONITOR_STATE_READ_WAITING_CLK;
                        }
                        ptHandle->rawReadIndex += 4;
                    }
                    else
                    {
                        ptHandle->delayTicks = ptHandle->readPeriodTickCount;
                        ptHandle->state = MONITOR_STATE_READ_DELAY;                        
                        ptHandle->rawReadIndex++;
                    }
                    break;
                }
                case MONITOR_STATE_WRITE_WAITING_CLK:
                {
                    clockValue = getGpioValue(ptHandle->clkGpio,1);
                    if (clockValue != ptHandle->prevClkPinVal && clockValue)
                    {
                        ptHandle->state = MONITOR_STATE_WRITE_DELAY;
                        ptHandle->prevClkPinVal = 0;
                        ptHandle->delayTicks = ptHandle->firstWriteDelayTickCount;
                        ptHandle->checkTime = getCurTimer(0);
                    }
                    if (ptHandle->state == MONITOR_STATE_WRITE_WAITING_CLK)
                    {
                        //Check Timeout
                        if (getDuration(0, ptHandle->checkTime) >= 1000 * gTickPerUs)
                        {
                            ptHandle->state = MONITOR_STATE_NULL_STATE;
                            //Set As Input
                            setGpioDir(ptHandle->writeDataGpio, 1);
                        }
                        break;
                    }
                }
                case MONITOR_STATE_WRITE_DELAY:
                {
                    if (getDuration(0, ptHandle->checkTime) >= ptHandle->delayTicks)
                    {
                        ptHandle->state = MONITOR_STATE_WRITE_DATA;
                    }
                    else
                    {
                        break;
                    }
                }
                case MONITOR_STATE_WRITE_DATA:
                {
                    uint32_t bitOffset = ptHandle->rawWriteIndex & 0x7;
                    bufferIndex = (ptHandle->rawWriteIndex >> 3);
                    if (ptHandle->rawWriteIndex == 0)
                    {
                        if (ptHandle->writeRepeatCounter)
                        {
                            setGpioDir(ptHandle->writeDataGpio, 0);
                        }
                    }
                    ptHandle->checkTime = getCurTimer(0);

                    if (bitOffset == 5)
                    {
                        dataValue = 0;
                    }
                    else
                    {
                        dataValue = ((ptHandle->rawWriteData[bufferIndex] >> bitOffset) & 0x1);
                    }
                    if (ptHandle->writeRepeatCounter)
                    {
                        setGpioValue(ptHandle->writeDataGpio, dataValue);
                        ptHandle->delayTicks = ptHandle->writePeriodTickCount;
                    }

                    if (bitOffset  < 5)
                    {
                        ptHandle->delayTicks = ptHandle->writePeriodTickCount;
                        ptHandle->rawWriteIndex++;
                        ptHandle->state = MONITOR_STATE_WRITE_DELAY;
                    }
                    else
                    {
                        if (bufferIndex == 63)
                        {
                            slaveRawReadDataToSaveData(ptHandle);
                            if (ptHandle->writeRepeatCounter)
                            {
                                if (ptHandle->writeRepeatCounter != RSL_SLAVE_INFINITE_WRITE)
                                {
                                    ptHandle->writeRepeatCounter--;
                                }
                                if (ptHandle->writeRepeatCounter == 0)
                                {
                                    for (i = 0; i < 64; i++)
                                    {
                                        ptHandle->rawWriteData[i] = 0;
                                    }
                                }                            
                                setGpioDir(ptHandle->writeDataGpio, 1);
                            }
                            ptHandle->state = MONITOR_STATE_NULL_STATE;
                        }
                        else
                        {
                            ptHandle->state = MONITOR_STATE_WRITE_WAITING_CLK;
                        }                       
                        ptHandle->rawWriteIndex += 3;
                    }
                    break;
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    //Set GPIO and Clock Setting
    int inputCmd = 0;
    startTimer(0);

    while(1)
    {
        inputCmd = ALT_CPU_COMMAND_REG_READ(REQUEST_CMD_REG);
        if (inputCmd && ALT_CPU_COMMAND_REG_READ(RESPONSE_CMD_REG) == 0)
        {
            switch(inputCmd)
            {
                case INIT_CMD_ID:
                    slaveProcessInitCmd();
                    break;
                case READ_RAW_DATA_CMD_ID:
                    slaveProcessReadRawDataCmd();
                    break;
                case WRITE_RAW_DATA_CMD_ID:
                    slaveProcessWriteRawDataCmd();
                    break;
                default:
                    break;
            }
            ALT_CPU_COMMAND_REG_WRITE(RESPONSE_CMD_REG, (uint16_t) inputCmd);
        }
        slaveMonitorMasterInput();
    }
}
