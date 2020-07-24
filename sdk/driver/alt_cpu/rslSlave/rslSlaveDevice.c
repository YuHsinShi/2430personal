#include <string.h>
#include "alt_cpu/rslSlave/rslSlave.h"

static uint8_t gpRslSlaveImage[] =
{
    #include "rslSlave.hex"
};

static void rslSlaveProcessCommand(int cmdId)
{
    int i = 0;

    ALT_CPU_COMMAND_REG_WRITE(REQUEST_CMD_REG, cmdId);
    while(1)
    {
        if (ALT_CPU_COMMAND_REG_READ(RESPONSE_CMD_REG) != cmdId)
            continue;
        else
            break;
    }
    ALT_CPU_COMMAND_REG_WRITE(REQUEST_CMD_REG, 0);
    for (i = 0; i < 1024; i++)
    {
        asm("");
    }
    ALT_CPU_COMMAND_REG_WRITE(RESPONSE_CMD_REG, 0);
}

static int rslSlaveIoctl(int file, unsigned long request, void *ptr, void *info)
{
    uint8_t* pWriteAddress = (uint8_t*) (iteRiscGetTargetMemAddress(ALT_CPU_IMAGE_MEM_TARGET) + CMD_DATA_BUFFER_OFFSET);

    switch (request)
    {
        case ITP_IOCTL_INIT:
        {

            //Stop ALT CPU
            iteRiscResetCpu(ALT_CPU);

            //Clear Commuication Engine and command buffer
            memset(pWriteAddress, 0x0, MAX_CMD_DATA_BUFFER_SIZE);
            ALT_CPU_COMMAND_REG_WRITE(REQUEST_CMD_REG, 0);
            ALT_CPU_COMMAND_REG_WRITE(RESPONSE_CMD_REG, 0);

            //Load Engine First
            iteRiscLoadData(ALT_CPU_IMAGE_MEM_TARGET,gpRslSlaveImage,sizeof(gpRslSlaveImage));

            //Fire Alt CPU
            iteRiscFireCpu(ALT_CPU);
            break;
        }
        case ITP_IOCTL_ALT_CPU_INIT_PARAMETER:
        {
            RSL_SLAVE_INIT_DATA* ptInitData = (RSL_SLAVE_INIT_DATA*) ptr;
            //Send Init command
            memcpy(pWriteAddress, ptInitData, sizeof(RSL_SLAVE_INIT_DATA));
            rslSlaveProcessCommand(INIT_CMD_ID);
            break;
        }
        case ITP_IOCTL_ALT_CPU_READ_RAW_DATA:
        {
            RSL_SLAVE_READ_RAW_DATA* ptReadRawData = (RSL_SLAVE_READ_RAW_DATA*) ptr;
            memcpy(pWriteAddress, ptReadRawData, sizeof(RSL_SLAVE_READ_RAW_DATA));
            rslSlaveProcessCommand(READ_RAW_DATA_CMD_ID);
            memcpy(ptReadRawData, pWriteAddress, sizeof(RSL_SLAVE_READ_RAW_DATA));
            if (ptReadRawData->bSuccess)
            {
                return 64;
            }
            else
            {
                return 0;
            }
            break;
        }
        case ITP_IOCTL_ALT_CPU_WRITE_RAW_DATA:
        {
            int i = 0;
            RSL_SLAVE_WRITE_RAW_DATA *ptWriteRawData = (RSL_SLAVE_WRITE_RAW_DATA*) ptr;
            memcpy(pWriteAddress, ptWriteRawData, sizeof(RSL_SLAVE_WRITE_RAW_DATA));
            rslSlaveProcessCommand(WRITE_RAW_DATA_CMD_ID);
            break;
        }
        default:
            break;
    }
    return 0;
}

const ITPDevice itpDeviceRslSlave =
{
    ":rslSlave",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    itpWriteDefault,
    itpLseekDefault,
    rslSlaveIoctl,
    NULL
};
