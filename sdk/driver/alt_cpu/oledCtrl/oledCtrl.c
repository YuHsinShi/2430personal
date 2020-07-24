#include <string.h>
#include "alt_cpu/oledCtrl/oledCtrl.h"

static uint8_t gpOledCtrlImage[] =
{
    #include "oledCtrl.hex"
};


static void oledCtrlProcessCommand(int cmdId)
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

static int oledCtrlIoctl(int file, unsigned long request, void *ptr, void *info)
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
            iteRiscLoadData(ALT_CPU_IMAGE_MEM_TARGET,gpOledCtrlImage,sizeof(gpOledCtrlImage));

            //Fire Alt CPU
            iteRiscFireCpu(ALT_CPU);
            break;
        }
        case ITP_IOCTL_OLED_CTRL_INIT_PARAM:
        {
            OLED_CTRL_INIT* ptInitData = (OLED_CTRL_INIT*) ptr;
            memcpy(pWriteAddress, ptInitData, sizeof(OLED_CTRL_INIT));
            oledCtrlProcessCommand(INIT_CMD_ID);
            break;
        }
        case ITP_IOCTL_OLED_CTRL_FLIP:
        {
            OLED_CTRL_FLIP* ptInitData = (OLED_CTRL_FLIP*) ptr;
            memcpy(pWriteAddress, ptInitData, sizeof(OLED_CTRL_FLIP));
            oledCtrlProcessCommand(FLIP_CMD_ID);
            break;
        }
        case ITP_IOCTL_OLED_CTRL_DISPMODE:
        {
            OLED_CTRL_DISPMODE* ptInitData = (OLED_CTRL_DISPMODE*) ptr;
            memcpy(pWriteAddress, ptInitData, sizeof(OLED_CTRL_DISPMODE));
            oledCtrlProcessCommand(DISPMODE_CMD_ID);
            break;
        }
        default:
            break;
    }
    return 0;
}

const ITPDevice itpDeviceOledCtrl =
{
    ":oledCtrl",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    itpWriteDefault,
    itpLseekDefault,
    oledCtrlIoctl,
    NULL
};
