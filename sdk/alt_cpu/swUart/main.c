#include <stdarg.h>
#include <string.h>

#include "alt_cpu/swUart/swUart.h"
#include "alt_cpu/alt_cpu_utility.h"

#define ENDIAN_SWAP16(x) \
        (((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8))

#define ENDIAN_SWAP32(x) \
        (((x & 0x000000FF) << 24) | \
        ((x & 0x0000FF00) <<  8) | \
        ((x & 0x00FF0000) >>  8) | \
        ((x & 0xFF000000) >> 24))

// Uart Setting
static uint32_t gTickPerbit = 0;
static uint32_t gTxGpio = 0;
static uint32_t gRxGpio = 0;
static uint32_t gParity = 0;

// Tx Global Data 
static uint8_t gWriteData[MAX_UART_BUFFER_SIZE] = { 0 };
static uint32_t gTxWriteLen = 0;
static int gTxSendDataState = 0;
static uint8_t gTxInit = 0;

// Rx Global Data
static uint8_t gReadData[MAX_UART_BUFFER_SIZE] = { 0 };
static uint32_t gRxReadLen = 0;
static uint32_t gRxWriteIdx = 0;
static uint32_t gRxReadIdx = 0;
static uint8_t gRxInit = 0;

static void swUartProcessWriteCmd(void)
{
	SW_UART_WRITE_DATA* pWriteData = (SW_UART_WRITE_DATA*) CMD_DATA_BUFFER_OFFSET;
	int i = 0;

	if(gTxInit == 0) {
		// Tx un-init
		return;
	}

	gTxWriteLen = ENDIAN_SWAP32(pWriteData->len);

	for(i = 0; i < gTxWriteLen; i++) {
		gWriteData[i] = pWriteData->pWriteBuffer[i];
	}
}

static void swUartProcessReadCmd(void)
{
	SW_UART_READ_DATA* pReadData = (SW_UART_READ_DATA*) CMD_DATA_BUFFER_OFFSET;
	int i;
	int remainSize = 0;
	int cpySize = 0;
	int tailSize = 0;

	//Clear command buffer first
    for (i = 0; i < MAX_UART_BUFFER_SIZE; i++) {
        pReadData->pReadBuffer[i] = 0;
    }

	gRxReadLen = ENDIAN_SWAP32(pReadData->len);

	if(gRxWriteIdx == gRxReadIdx) {
		// No Rx Data
		pReadData->len = 0;
		return;
	}

	if(gRxInit == 0) {
		// Rx un-init
		pReadData->len = 0;
		return;
	}

	if(gRxWriteIdx > gRxReadIdx)
		remainSize = gRxWriteIdx - gRxReadIdx;
	else
		remainSize = MAX_UART_BUFFER_SIZE - gRxReadIdx + gRxWriteIdx;

	if(gRxReadLen <= remainSize)
		cpySize = gRxReadLen;
	else
		cpySize = remainSize;

	tailSize = MAX_UART_BUFFER_SIZE - gRxReadIdx;
	if(cpySize <= tailSize) {
		memcpy(pReadData->pReadBuffer, &gReadData[gRxReadIdx], cpySize);
	}
	else {
		memcpy(pReadData->pReadBuffer, &gReadData[gRxReadIdx], tailSize);
		memcpy(&pReadData->pReadBuffer[tailSize], &gReadData[0], cpySize - tailSize);
	}

	gRxReadIdx += cpySize;
	if(gRxReadIdx >= MAX_UART_BUFFER_SIZE) {
		gRxReadIdx -= MAX_UART_BUFFER_SIZE;
	}

	// The return Rx's Length of swUartRead()	
	pReadData->len = ENDIAN_SWAP32(cpySize);
		
}


static void swUartProcessInitCmd(void)
{
	SW_UART_INIT_DATA* pInitData = (SW_UART_INIT_DATA*) CMD_DATA_BUFFER_OFFSET;
	uint32_t cpuClk = ENDIAN_SWAP32(pInitData->cpuClock);
	uint32_t baudrate = ENDIAN_SWAP32(pInitData->baudrate);

	gTickPerbit = (cpuClk / baudrate) - 1;
	gTxGpio = ENDIAN_SWAP32(pInitData->uartTxGpio);
	gRxGpio = ENDIAN_SWAP32(pInitData->uartRxGpio);
	gParity = ENDIAN_SWAP32(pInitData->parity);

	gTxInit = 0;
	gRxInit = 0;

	//Tx GPIO init
	if(gTxGpio) {
		setGpioDir(gTxGpio, 0);  //output mode
		setGpioMode(gTxGpio, 0);
		setGpioValue(gTxGpio, 1);
		gTxInit = 1;
	}

	//Rx GPIO init
	if(gRxGpio) {
		setGpioDir(gRxGpio, 1); //input mode
		setGpioMode(gRxGpio, 0);
		gRxInit = 1;
	}	
}


static void swUartProcessSendDataOutCmd(void)
{
	uint32_t nextCheckTime = 0;
	uint8_t dataValue = 0;
	int i = 0, j = 0;
	int parity_num = 0;

	if(gTxInit == 0) {
		return;
	}

	for(j = 0; j < gTxWriteLen; j++) {
		dataValue = gWriteData[j];
		
		//start bit
		setGpioValue(gTxGpio, 0);
		nextCheckTime = gTickPerbit;
		resetTimer(0);
		startTimer(0);
		
		while (getTimer(0) <= nextCheckTime);

		//data bits
		for(i = 0; i < 7; i++) {
			if(dataValue & (0x1 << i)) {
				setGpioValue(gTxGpio, 1);
				parity_num++;
			}
			else {
				setGpioValue(gTxGpio, 0);
			}
			nextCheckTime+=gTickPerbit;
			while (getTimer(0) <= nextCheckTime);
		}
		
		//parity bit
		switch(gParity) {
			case EVEN:
				if(parity_num % 2) {
					setGpioValue(gTxGpio, 1);
				}
				else {
					setGpioValue(gTxGpio, 0);
				}
				break;
			case ODD:
				if(parity_num % 2) {
					setGpioValue(gTxGpio, 0);
				}
				else {
					setGpioValue(gTxGpio, 1);
				}
				break;
			case NONE:
				setGpioValue(gTxGpio, 0);
			default:
				break;
		}

		nextCheckTime+=gTickPerbit;
		while (getTimer(0) <= nextCheckTime);
		
		//stop bit
		setGpioValue(gTxGpio, 1);
		nextCheckTime+=gTickPerbit;
		while (getTimer(0) <= nextCheckTime);
		
	}

	if(gTxWriteLen > 0) {
		stopTimer(0);
		gTxWriteLen = 0;
	}
}



static void swUartMonitorReadDataCmd(void)
{
	static uint8_t ReadByte = 0x0;
	static uint8_t BitIdx = 0;
	static uint32_t nextReadTime = 0;
	static uint8_t CurReadBit = 0x0, PreReadBit = 0x0;
	static uint8_t NewByteFlag = 1;
	uint32_t CurTime = 0;

	if(gRxInit == 0) {
		return;
	}

	CurReadBit = getGpioValue(gRxGpio, 1);  //Get data from Rx GPIO
	if(CurReadBit == 0 && PreReadBit != CurReadBit && NewByteFlag) {
		resetTimer(1);
		startTimer(1);
		nextReadTime = gTickPerbit * 1.5;  //1.5 is for avoiding error of accuracy
		ReadByte = 0x0;
		NewByteFlag = 0;
	}
	else if((CurTime = getTimer(1)) >= nextReadTime && NewByteFlag == 0) {		
		if(BitIdx < 8) {			
			ReadByte |= (CurReadBit << BitIdx);	
			BitIdx++;
			NewByteFlag = 0;
		}
		else{
			// Receive 1 Byte done
			BitIdx = 0;
			NewByteFlag = 1;
			gReadData[gRxWriteIdx] = ReadByte;
			gRxWriteIdx++;
            if(gRxWriteIdx >= MAX_UART_BUFFER_SIZE) {
	        	gRxWriteIdx -= MAX_UART_BUFFER_SIZE;
        	}
		}
		nextReadTime += (gTickPerbit - (CurTime - nextReadTime));
	}

	PreReadBit = CurReadBit;
}

int main(int argc, char **argv)
{
    int inputCmd = 0;

    while(1)
    {
        inputCmd = ALT_CPU_COMMAND_REG_READ(REQUEST_CMD_REG);
        if (inputCmd && ALT_CPU_COMMAND_REG_READ(RESPONSE_CMD_REG) == 0)
        {
            switch(inputCmd)
            {
                case INIT_CMD_ID:
                    swUartProcessInitCmd();
                    break;
				case WRITE_DATA_CMD_ID:
					swUartProcessWriteCmd();
					break;
				case READ_DATA_CMD_ID:
					swUartProcessReadCmd();
					break;
				case SEND_DATA_OUT_ID:
					swUartProcessSendDataOutCmd();
					break;

                default:
                    break;
            }

			//ALT CPU Finish and Response
            ALT_CPU_COMMAND_REG_WRITE(RESPONSE_CMD_REG, (uint16_t) inputCmd);
        }

		swUartMonitorReadDataCmd();
    }
}
