#include <string.h>
#include "alt_cpu/swUart/swUart.h"

static uint8_t gpSwUartImage[] =
{
    #include "swUart.hex"
};

#if 0
static void swUartDebug(int reg_num)
{
	int RegBase = 0x16E2;
	int i;
	int tmp = 0;

	while(i <= (reg_num - 1)*2){  //reg_num max: 7
		tmp = ALT_CPU_COMMAND_REG_READ(RegBase+i);
		printf("0x%x: %x\n", RegBase+i, tmp);
		i = i+2;
	}
}
#endif

static void swUartProcessCommand(int cmdId)
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

static int swUartRead(int file, char *ptr, int len, void* info)
{
	uint8_t* pReadAddress = (uint8_t*) (iteRiscGetTargetMemAddress(ALT_CPU_IMAGE_MEM_TARGET) + CMD_DATA_BUFFER_OFFSET);
	SW_UART_READ_DATA ptReadData = { 0 };
	int i;

	ptReadData.len = len;
	//Copy Rx data's Len to ALT CPU memory
	memcpy(pReadAddress, &ptReadData, sizeof(SW_UART_READ_DATA));

	if(len >= MAX_UART_BUFFER_SIZE) {
		ithPrintf("swUart read data is too big\n");
		return 0;
	}

	memset(ptr, 0, len);
	swUartProcessCommand(READ_DATA_CMD_ID);

	//Copy Rx data from ALT CPU
	memcpy(&ptReadData, pReadAddress, sizeof(SW_UART_READ_DATA));

	for(i = 0; i < ptReadData.len; i++) {
		ptr[i] = ptReadData.pReadBuffer[i];
	}
	
	return ptReadData.len;
}

static int swUartWrite(int file, char *ptr, int len, void* info)
{
	uint8_t* pWriteAddress = (uint8_t*) (iteRiscGetTargetMemAddress(ALT_CPU_IMAGE_MEM_TARGET) + CMD_DATA_BUFFER_OFFSET);
	SW_UART_WRITE_DATA ptWriteData = { 0 };
	int i;

	if(len >= MAX_UART_BUFFER_SIZE) {
		ithPrintf("swUart write data is too big\n");
		return 0;
	}

	ptWriteData.len = len;

	for(i = 0; i < len; i++) {
		ptWriteData.pWriteBuffer[i] = ptr[i];
	}

	//copy Tx data to ALT CPU memory
	memcpy(pWriteAddress, &ptWriteData, sizeof(SW_UART_WRITE_DATA));

	swUartProcessCommand(WRITE_DATA_CMD_ID);
	swUartProcessCommand(SEND_DATA_OUT_ID);
	
	return len;
}

//Just for SW Uart Debug
static int SwUartDbgPutchar(int c)
{
    static unsigned char pBuffer[64] = { 0 };
    static int curIndex = 0;
	
    pBuffer[curIndex++] = (unsigned char) c;
    
    if (curIndex >= 64)
    {
        itpWrite(ITP_DEVICE_ALT_CPU, pBuffer, curIndex);
        curIndex = 0;
    }
    return c;
}

static int swUartIoctl(int file, unsigned long request, void *ptr, void *info)
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
            iteRiscLoadData(ALT_CPU_IMAGE_MEM_TARGET,gpSwUartImage,sizeof(gpSwUartImage));

            //Fire Alt CPU
            iteRiscFireCpu(ALT_CPU);
            break;
        }
		case ITP_IOCTL_INIT_DBG_UART:
		{
			//For Software Uart Debug
			ithPutcharFunc = SwUartDbgPutchar;
			break;
		}
        case ITP_IOCTL_INIT_UART_PARAM:
        {
            SW_UART_INIT_DATA* ptInitData = (SW_UART_INIT_DATA*) ptr;

			if(ptInitData->uartTxGpio == 0xFFFFFFFF) { //if TxGpio = -1
				ptInitData->uartTxGpio = 0;
			}

			if(ptInitData->uartRxGpio == 0xFFFFFFFF) { //if RxGpio = -1
				ptInitData->uartRxGpio = 0;
			}

            memcpy(pWriteAddress, ptInitData, sizeof(SW_UART_INIT_DATA));
            swUartProcessCommand(INIT_CMD_ID);			
            break;
        }
        default:
            break;
    }
    return 0;
}

const ITPDevice itpDeviceSwUart =
{
    ":swUart",
    itpOpenDefault,
    itpCloseDefault,
    swUartRead,
    swUartWrite,
    itpLseekDefault,
    swUartIoctl,
    NULL
};
