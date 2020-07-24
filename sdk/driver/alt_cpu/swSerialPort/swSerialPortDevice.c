#include <string.h>
#include "alt_cpu/swSerialPort/swSerialPort.h"

static uint8_t gpSwSerialPortImage[] =
{
    #include "swSerialPort.hex"
};

static void swSerialPortProcessCommand(int cmdId)
{
    int i = 0;
    ALT_CPU_COMMAND_REG_WRITE(REQUEST_CMD_REG, cmdId);
    while(1)
    {
        if (ALT_CPU_COMMAND_REG_READ(RESPONSE_CMD_REG) != cmdId) {
			//Waiting ALT CPU response
            continue;
        }
        else {
            break;
        }
    }
    ALT_CPU_COMMAND_REG_WRITE(REQUEST_CMD_REG, 0);
    for (i = 0; i < 1024; i++)
    {
        asm("");
    }
    ALT_CPU_COMMAND_REG_WRITE(RESPONSE_CMD_REG, 0);
}

static int swSerialPortIoctl(int file, unsigned long request, void *ptr, void *info)
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
            iteRiscLoadData(ALT_CPU_IMAGE_MEM_TARGET,gpSwSerialPortImage,sizeof(gpSwSerialPortImage));

            //Fire Alt CPU
            iteRiscFireCpu(ALT_CPU);
            break;
        }

        case ITP_IOCTL_SW_PORT_INIT_PARAM:
        {
            SW_PORT_INIT_DATA* ptInitData = (SW_PORT_INIT_DATA*) ptr;

			if(ptInitData->txGpio == 0xFFFFFFFF) { //if TxGpio = -1
				ptInitData->txGpio = 0;
			}

			if(ptInitData->rxGpio == 0xFFFFFFFF) { //if RxGpio = -1
				ptInitData->rxGpio = 0;
			}

            if(ptInitData->txEnableGpio == 0xFFFFFFFF) { //if txEnableGpio = -1
				ptInitData->txEnableGpio = 0;
			}
            
            memcpy(pWriteAddress, ptInitData, sizeof(SW_PORT_INIT_DATA));
            swSerialPortProcessCommand(INIT_CMD_ID);			
            break;
        }
        case ITP_IOCTL_SW_PORT_READ_DATA:
        {
            int i;
            SW_PORT_READ_DATA* ptReadData = (SW_PORT_READ_DATA*) ptr;
            if(ptReadData->len >= MAX_PORT_BUFFER_SIZE) {
                ithPrintf("sw serial port read data is too big\n");
                return 0;
            }
            memcpy(pWriteAddress, ptReadData, sizeof(SW_PORT_READ_DATA));
            swSerialPortProcessCommand(READ_DATA_CMD_ID);
            memcpy(ptReadData, pWriteAddress, sizeof(SW_PORT_READ_DATA));
            memcpy(ptReadData->pReadDataBuffer, ptReadData->pReadBuffer, ptReadData->len);
            usleep(100);
            return ptReadData->len;
        }
        case ITP_IOCTL_SW_PORT_WRITE_DATA:
        {
            int i;
            SW_PORT_WRITE_DATA *ptWriteData = (SW_PORT_WRITE_DATA*) ptr;
            if(ptWriteData->len >= MAX_PORT_BUFFER_SIZE) {
                ithPrintf("sw serial port write data is too big\n");
                return 0;
            }
            memcpy(ptWriteData->pWriteBuffer, ptWriteData->pWriteDataBuffer, ptWriteData->len);
            do {
                memcpy(pWriteAddress, ptWriteData, sizeof(SW_PORT_WRITE_DATA));
                swSerialPortProcessCommand(WRITE_DATA_CMD_ID);
                usleep(800);
            } while(ptWriteData->len != ((SW_PORT_WRITE_DATA*) pWriteAddress)->len);
            return ptWriteData->len;
        }
        default:
            break;
    }
    return 0;
}

const ITPDevice itpDeviceSwSerialPort =
{
    ":swSerialPort",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    itpWriteDefault,
    itpLseekDefault,
    swSerialPortIoctl,
    NULL
};
