#include "ite/ith.h"
#include "ite/itp.h"
#include "iot_bus.h"
#include "pthread.h"

#define WRITE_OP_SYNC               0x00471234
#define READ_OP_SYNC                0x01471234

typedef struct PROTOCOL_BASE_CMD_TAG
{
    uint32_t syncWord;
    uint32_t id;
    uint32_t size;
    uint32_t paramSize;
} PROTOCOL_BASE_CMD;

static pthread_mutex_t protocolMutex = PTHREAD_MUTEX_INITIALIZER;
uint8_t gpWriteBuffer[4096] = { 0 };

int IotProtocolWrite(uint32_t id, uint8_t* txBuffer, int txSize)
{
    PROTOCOL_BASE_CMD tCmd = { 0 };
    int result = 0;
    int writeSize = sizeof(PROTOCOL_BASE_CMD);
    tCmd.syncWord = WRITE_OP_SYNC;
    tCmd.id = id;
    tCmd.size = txSize;

    pthread_mutex_lock(&protocolMutex);
    memcpy(gpWriteBuffer, &tCmd, writeSize);
    if (txBuffer && txSize)
    {
        memcpy(&gpWriteBuffer[writeSize], txBuffer, txSize);
        writeSize += txSize;
    }

    if (!IotBusWrite(gpWriteBuffer, writeSize))
    {
            result = -1;
    }
    pthread_mutex_unlock(&protocolMutex);

    return result;
}

int IotProtocolRead(uint32_t id, uint8_t* param, int pararmSize, uint8_t* rxBuffer, int rxSize)
{
    PROTOCOL_BASE_CMD tCmd = { 0 };
    int result = 0;
    int writeSize = sizeof(PROTOCOL_BASE_CMD);
    tCmd.syncWord = READ_OP_SYNC;
    tCmd.id = id;
    if (param && pararmSize)
    {
        tCmd.paramSize = pararmSize;
    }
    tCmd.size = rxSize;

    pthread_mutex_lock(&protocolMutex);

    memcpy(gpWriteBuffer, &tCmd, writeSize);
    if (tCmd.paramSize)
    {
        memcpy(&gpWriteBuffer[writeSize], param, pararmSize);
        writeSize += pararmSize;
    }

    if (IotBusWrite(gpWriteBuffer, writeSize))
    {
        if (tCmd.size)
        {
            if (IotBusRead(rxBuffer, rxSize) != rxSize)
            {
                result = -1;
            }
        }
    }
    pthread_mutex_unlock(&protocolMutex);

    return result;
}
