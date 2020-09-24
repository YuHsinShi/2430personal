#include <string.h>
#include "alt_cpu/homebus/homebus.h"

static uint8_t gpHomebusImage[] =
{
    #include "homebus.hex"
};

static void homebusProcessCommand(int cmdId)
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

static int homebusIoctl(int file, unsigned long request, void *ptr, void *info)
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
            iteRiscLoadData(ALT_CPU_IMAGE_MEM_TARGET,gpHomebusImage,sizeof(gpHomebusImage));

            //Fire Alt CPU
            iteRiscFireCpu(ALT_CPU);
            break;
        }

        case ITP_IOCTL_HOMEBUS_INIT_PARAM:
        {
            HOMEBUS_INIT_DATA* ptInitData = (HOMEBUS_INIT_DATA*) ptr;

            if(ptInitData->txdGpio == 0xFFFFFFFF || ptInitData->rxdGpio == 0xFFFFFFFF) {
                ithPrintf("[Homebus] set pin error !\n");
                return -1;
            }
            
            memcpy(pWriteAddress, ptInitData, sizeof(HOMEBUS_INIT_DATA));
            homebusProcessCommand(INIT_CMD_ID);			
            break;
        }
        case ITP_IOCTL_HOMEBUS_READ_DATA:
        {
            int i;
            HOMEBUS_READ_DATA* ptReadData = (HOMEBUS_READ_DATA*) ptr;
            if(ptReadData->len >= MAX_PORT_BUFFER_SIZE) {
                ithPrintf("[Homebus] read len overflow !\n");
                return 0;
            }
            memcpy(pWriteAddress, ptReadData, sizeof(HOMEBUS_READ_DATA));
            homebusProcessCommand(READ_DATA_CMD_ID);
            memcpy(ptReadData, pWriteAddress, sizeof(HOMEBUS_READ_DATA));
            memcpy(ptReadData->pReadDataBuffer, ptReadData->pReadBuffer, ptReadData->len);
            return ptReadData->len;
        }
        case ITP_IOCTL_HOMEBUS_WRITE_DATA:
        {
            int i;
            HOMEBUS_WRITE_DATA *ptWriteData = (HOMEBUS_WRITE_DATA*) ptr;
            if(ptWriteData->len >= MAX_PORT_BUFFER_SIZE) {
                ithPrintf("[Homebus] write len overflow !\n");
                return 0;
            }
            memcpy(ptWriteData->pWriteBuffer, ptWriteData->pWriteDataBuffer, ptWriteData->len);
            memcpy(pWriteAddress, ptWriteData, sizeof(HOMEBUS_WRITE_DATA));
            while(1) {
                homebusProcessCommand(WRITE_DATA_CMD_ID);
                if(((HOMEBUS_WRITE_DATA*) pWriteAddress)->len == TX_COLLISION) {
                    ithPrintf("[Homebus] write COLLISION !\n");
                    //collision
                    usleep(1000*6); //6ms
                    // send 0xaa
                    HOMEBUS_WRITE_DATA tChkData = {1,{0xaa}};
                    memcpy(pWriteAddress, &tChkData, sizeof(HOMEBUS_WRITE_DATA));
                    continue;
                } else if (((HOMEBUS_WRITE_DATA*) pWriteAddress)->len == 1 && ((HOMEBUS_WRITE_DATA*) pWriteAddress)->pWriteBuffer[0] == 0xaa) {
                    ithPrintf("[Homebus] write COLLISION re-send !\n");
                    //collision
                    //0xaa ok re-send data
                    memcpy(pWriteAddress, ptWriteData, sizeof(HOMEBUS_WRITE_DATA));
                    continue;
                } else if (((HOMEBUS_WRITE_DATA*) pWriteAddress)->len == RX_BUSY) {
                    ithPrintf("[Homebus] write RX BUSY !\n");
                    //rx busy
                    usleep(1000*6); //6ms
                    memcpy(pWriteAddress, ptWriteData, sizeof(HOMEBUS_WRITE_DATA));
                    continue;
                } else {
                    // ithPrintf("[Homebus] write success !(%d)\n", ((HOMEBUS_WRITE_DATA*) pWriteAddress)->len);
                    usleep(1000*100); //100ms
                    break; //success
                }
            }

            return ptWriteData->len;
        }
        default:
            break;
    }
    return 0;
}

const ITPDevice itpDeviceHomebus =
{
    ":homebus",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    itpWriteDefault,
    itpLseekDefault,
    homebusIoctl,
    NULL
};
