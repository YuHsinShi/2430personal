#include <string.h>
#include "alt_cpu/patternGen/patternGen.h"

static uint8_t gpPatternGenImage[] =
{
    #include "patternGen.hex"
};


static void patternGenProcessCommand(int cmdId)
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

static int patternGenIoctl(int file, unsigned long request, void *ptr, void *info)
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
            iteRiscLoadData(ALT_CPU_IMAGE_MEM_TARGET,gpPatternGenImage,sizeof(gpPatternGenImage));

            //Fire Alt CPU
            iteRiscFireCpu(ALT_CPU);
            break;
        }
        case ITP_IOCTL_INIT_PATTERN_GEN_PARAM:
        {
            PATTERN_GEN_INIT_DATA* ptInitData = (PATTERN_GEN_INIT_DATA*) ptr;
            memcpy(pWriteAddress, ptInitData, sizeof(PATTERN_GEN_INIT_DATA));
            patternGenProcessCommand(INIT_CMD_ID);
            break;
        }
        case ITP_IOCTL_PATTERN_GEN_WRITE_DATA:
        {
            PATTERN_GEN_WRITE_CMD_DATA* ptWriteDataCmd = (PATTERN_GEN_WRITE_CMD_DATA*) ptr;
            
            memcpy(pWriteAddress, ptWriteDataCmd, sizeof(PATTERN_GEN_WRITE_CMD_DATA));
            patternGenProcessCommand(WRITE_CMD_ID);
            break;
        }
        case ITP_IOCTL_PATTERN_GEN_UPDATE_SEND_COUNT:
        {
            PATTERN_GEN_UPDATE_SEND_COUNT_CMD_DATA* ptUpdateCmd = (PATTERN_GEN_UPDATE_SEND_COUNT_CMD_DATA*) ptr;
            
            memcpy(pWriteAddress, ptUpdateCmd, sizeof(PATTERN_GEN_UPDATE_SEND_COUNT_CMD_DATA));
            patternGenProcessCommand(UPDATE_SEND_COUNT_CMD_ID);
            break;
        }
        case ITP_IOCTL_PATTERN_GEN_STOP:
        {
            PATTERN_GEN_STOP_CMD_DATA* ptStopCmd = (PATTERN_GEN_STOP_CMD_DATA*) ptr;
            memcpy(pWriteAddress, ptStopCmd, sizeof(PATTERN_GEN_STOP_CMD_DATA));
            patternGenProcessCommand(STOP_CMD_ID);
            break;
        }
        default:
            break;
    }
    return 0;
}

const ITPDevice itpDevicePatternGen =
{
    ":patternGen",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    itpWriteDefault,
    itpLseekDefault,
    patternGenIoctl,
    NULL
};
