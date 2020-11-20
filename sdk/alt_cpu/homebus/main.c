#include <stdarg.h>
#include <string.h>

#include "alt_cpu/homebus/homebus.h"
#include "alt_cpu/alt_cpu_utility.h"

#define ENDIAN_SWAP16(x) \
        (((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8))

#define ENDIAN_SWAP32(x) \
        (((x & 0x000000FF) << 24) | \
        ((x & 0x0000FF00) <<  8) | \
        ((x & 0x00FF0000) >>  8) | \
        ((x & 0xFF000000) >> 24))

#define TXBAUDRATE 19200

typedef enum
{
    TX_IDLE = 0,
    TX_START_BIT = TX_IDLE,
    TX_DATA_BITS,
    TX_PARITY_BIT,
    TX_STOP_BIT,
    TX_STOP_END,
} MONITOR_TX_STATE;

typedef struct
{
    uint32_t txState;
    uint32_t txdGpio;
    uint32_t txWriteLen;
    uint32_t txTickPerbit;
    uint32_t init;
    uint32_t parity;
    uint8_t pWriteData[MAX_PORT_BUFFER_SIZE];
} HOMEBUS_HANDLE;

static HOMEBUS_HANDLE gptHomebusHandle = { 0 };

static int homebusTxSendDataOut(void)
{
    HOMEBUS_HANDLE *ptHandle = &gptHomebusHandle;
	uint32_t nextChkTime = 0;
    uint32_t chkTime = 0;
	uint8_t dataValue = 0;
	int i = 0, j = 0;
	int parity_num = 0;
    ptHandle->parity = ODD;
    
	for(j = 0; j < ptHandle->txWriteLen; j++) {
		dataValue = ptHandle->pWriteData[j];
		parity_num = 0;
        
		//start bit
		setGpioValue(ptHandle->txdGpio, 0);
        chkTime = getCurTimer(0);
		nextChkTime = ptHandle->txTickPerbit;
		while (getDuration(0, chkTime) <= nextChkTime);
        setGpioValue(ptHandle->txdGpio, 1);
        nextChkTime += ptHandle->txTickPerbit;
        while (getDuration(0, chkTime) <= nextChkTime);

		//data bits
		for(i = 0; i < 8; i++) {
			if(dataValue & (0x1 << i)) {
				setGpioValue(ptHandle->txdGpio, 1);
				parity_num++;
			}
			else {
				setGpioValue(ptHandle->txdGpio, 0);
			}
			nextChkTime += ptHandle->txTickPerbit;
			while (getDuration(0, chkTime) <= nextChkTime);
            setGpioValue(ptHandle->txdGpio, 1);
            nextChkTime += ptHandle->txTickPerbit;
            while (getDuration(0, chkTime) <= nextChkTime);
		}
		
		//parity bit
		switch(ptHandle->parity) {
			case EVEN:
				if(parity_num % 2) {
					setGpioValue(ptHandle->txdGpio, 1);
				}
				else {
					setGpioValue(ptHandle->txdGpio, 0);
				}
				break;
			case ODD:
				if(parity_num % 2) {
					setGpioValue(ptHandle->txdGpio, 0);
				}
				else {
					setGpioValue(ptHandle->txdGpio, 1);
				}
                ptHandle->parity = EVEN;
				break;
			case NONE:
				setGpioValue(ptHandle->txdGpio, 0);
			default:
				break;
		}
		nextChkTime += ptHandle->txTickPerbit;
		while (getDuration(0, chkTime) <= nextChkTime);
        setGpioValue(ptHandle->txdGpio, 1);
        nextChkTime += ptHandle->txTickPerbit;
        while (getDuration(0, chkTime) <= nextChkTime);

		//stop bit
		setGpioValue(ptHandle->txdGpio, 1);
		nextChkTime += (ptHandle->txTickPerbit<<2); //wait txTickPerbit*4
		while (getDuration(0, chkTime) <= nextChkTime);
	}

    return ptHandle->txWriteLen; //success
}

static void homebusProcessWriteCmd(void)
{
    HOMEBUS_WRITE_DATA* pWriteData = (HOMEBUS_WRITE_DATA*) CMD_DATA_BUFFER_OFFSET;

    HOMEBUS_HANDLE *ptHandle = &gptHomebusHandle;
    if(ptHandle->init == 0) {
        // Tx un-init
        return;
    }

    ptHandle->txWriteLen = ENDIAN_SWAP32(pWriteData->len);
    memcpy(ptHandle->pWriteData, pWriteData->pWriteBuffer, ptHandle->txWriteLen);
    pWriteData->len = homebusTxSendDataOut();
    pWriteData->len = ENDIAN_SWAP32(pWriteData->len);
}

static void homebusProcessInitCmd(void)
{
	HOMEBUS_INIT_DATA* pInitData = (HOMEBUS_INIT_DATA*) CMD_DATA_BUFFER_OFFSET;
    uint32_t cpuClk = ENDIAN_SWAP32(pInitData->cpuClock);
    HOMEBUS_HANDLE *ptHandle = &gptHomebusHandle;
    
    ptHandle->txTickPerbit = (cpuClk / TXBAUDRATE);
    ptHandle->txdGpio = ENDIAN_SWAP32(pInitData->txdGpio);
    
    //TXD GPIO init
    setGpioMode(ptHandle->txdGpio, 0);
    setGpioDir(ptHandle->txdGpio, 0);  //output mode
    setGpioValue(ptHandle->txdGpio, 1);

    ptHandle->init = 1;
}

int main(int argc, char **argv)
{
    int inputCmd = 0;
    memset(&gptHomebusHandle, 0, sizeof(gptHomebusHandle));
    startTimer(0);

    while(1)
    {
        inputCmd = ALT_CPU_COMMAND_REG_READ(REQUEST_CMD_REG);
        if (inputCmd && ALT_CPU_COMMAND_REG_READ(RESPONSE_CMD_REG) == 0)
        {
            switch(inputCmd)
            {
                case INIT_CMD_ID:
                    homebusProcessInitCmd();
                    break;
				case WRITE_DATA_CMD_ID:
                    homebusProcessWriteCmd();
                    break;
                default:
                    break;
            }
            ALT_CPU_COMMAND_REG_WRITE(RESPONSE_CMD_REG, (uint16_t) inputCmd);
        }
    }
}
