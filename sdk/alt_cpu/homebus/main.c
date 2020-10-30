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
#define RXBAUDRATE 9600

typedef enum
{
    RX_IDLE = 0,
    RX_PROCESSING,
    RX_ACK,
} MONITOR_RX_STATE;

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
    uint32_t rxState;
    uint32_t txdGpio;
    uint32_t rxdGpio;
    uint32_t txWriteLen;
    uint32_t rxReadLen;
    uint32_t rxWriteIdx;
    uint32_t rxReadIdx;
    uint32_t txTickPerbit;
    uint32_t rxTickPerbit;
    uint32_t rxChkTime;
    uint32_t rxNextReadTime;
    uint32_t init;
    uint32_t tickPerMs;

    uint8_t parity;
    uint8_t readByte;
    uint8_t rxBitIdx;
    uint8_t rxPreReadBit;

    uint8_t pWriteData[MAX_PORT_BUFFER_SIZE];
    uint8_t pReadData[MAX_PORT_BUFFER_SIZE];
} HOMEBUS_HANDLE;

static HOMEBUS_HANDLE gptHomebusHandle = { 0 };

static inline __attribute__((always_inline))
uint8_t homebusIsTxCollision(uint8_t val)
{
    HOMEBUS_HANDLE *ptHandle = &gptHomebusHandle;
    if(ptHandle->rxState == RX_ACK) return 0; //ack no chk collision
    else if(getGpioValue(ptHandle->rxdGpio, 1) == val){
        return 0;
    } else {
        return 1;
    }
}

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
            // if(homebusIsTxCollision(dataValue & (0x1 << i))) return TX_COLLISION; //collision
            setGpioValue(ptHandle->txdGpio, 1);
            nextChkTime += ptHandle->txTickPerbit;
            while (getDuration(0, chkTime) <= nextChkTime);
            // if(homebusIsTxCollision(dataValue & (0x1 << i))) return TX_COLLISION; //collision
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

    {
        HOMEBUS_HANDLE *ptHandle = &gptHomebusHandle;
        if(ptHandle->init == 0) {
            // Tx un-init
            return;
        }
        else if(ptHandle->rxState != RX_IDLE)
        {
            pWriteData->len = ENDIAN_SWAP32(RX_BUSY); //rx busy
            return;
        }
        else
        {
            ptHandle->txWriteLen = ENDIAN_SWAP32(pWriteData->len);
            memcpy(ptHandle->pWriteData, pWriteData->pWriteBuffer, ptHandle->txWriteLen);
            pWriteData->len = homebusTxSendDataOut();
            pWriteData->len = ENDIAN_SWAP32(pWriteData->len);
        }
    }
}

static void homebusProcessReadCmd(void)
{
	HOMEBUS_READ_DATA* pReadData = (HOMEBUS_READ_DATA*) CMD_DATA_BUFFER_OFFSET;
	int remainSize = 0;
	int cpySize = 0;
	int tailSize = 0;

    HOMEBUS_HANDLE *ptHandle = &gptHomebusHandle;
    if(ptHandle->rxWriteIdx == ptHandle->rxReadIdx) {
        // No Rx Data
        pReadData->len = 0;
        return;
    }

    if(ptHandle->init == 0) {
        // Rx un-init
        pReadData->len = 0;
        return;
    }

    ptHandle->rxReadLen = ENDIAN_SWAP32(pReadData->len);

    if(ptHandle->rxWriteIdx > ptHandle->rxReadIdx)
        remainSize = ptHandle->rxWriteIdx - ptHandle->rxReadIdx;
    else
        remainSize = MAX_PORT_BUFFER_SIZE - ptHandle->rxReadIdx + ptHandle->rxWriteIdx;

    if(ptHandle->rxReadLen <= remainSize)
        cpySize = ptHandle->rxReadLen;
    else
        cpySize = remainSize;

    tailSize = MAX_PORT_BUFFER_SIZE - ptHandle->rxReadIdx;
    if(cpySize <= tailSize) {
        memcpy(pReadData->pReadBuffer, &ptHandle->pReadData[ptHandle->rxReadIdx], cpySize);
    }
    else {
        memcpy(pReadData->pReadBuffer, &ptHandle->pReadData[ptHandle->rxReadIdx], tailSize);
        memcpy(&pReadData->pReadBuffer[tailSize], &ptHandle->pReadData[0], cpySize - tailSize);
    }

    ptHandle->rxReadIdx += cpySize;
    if(ptHandle->rxReadIdx >= MAX_PORT_BUFFER_SIZE) {
        ptHandle->rxReadIdx -= MAX_PORT_BUFFER_SIZE;
    }

    // The return Rx's Length
    pReadData->len = ENDIAN_SWAP32(cpySize);
}

static void homebusProcessInitCmd(void)
{
	HOMEBUS_INIT_DATA* pInitData = (HOMEBUS_INIT_DATA*) CMD_DATA_BUFFER_OFFSET;
    uint32_t cpuClk = ENDIAN_SWAP32(pInitData->cpuClock);
    HOMEBUS_HANDLE *ptHandle = &gptHomebusHandle;
    
    ptHandle->txTickPerbit = (cpuClk / TXBAUDRATE);
    ptHandle->rxTickPerbit = (cpuClk / RXBAUDRATE);
    ptHandle->tickPerMs = (cpuClk / 1000);
    ptHandle->txdGpio = ENDIAN_SWAP32(pInitData->txdGpio);
    ptHandle->rxdGpio = ENDIAN_SWAP32(pInitData->rxdGpio);
    // ptHandle->parity = ENDIAN_SWAP32(pInitData->parity);
    
    //TXD GPIO init
    setGpioMode(ptHandle->txdGpio, 0);
    setGpioDir(ptHandle->txdGpio, 0);  //output mode
    setGpioValue(ptHandle->txdGpio, 1);

    //RXD GPIO init
    setGpioMode(ptHandle->rxdGpio, 0);
    setGpioDir(ptHandle->rxdGpio, 1); //input mode
    
    ptHandle->init = 1;
}

static void homebusMonitorRx(void)
{
	uint8_t CurReadBit = 0;
    HOMEBUS_HANDLE *ptHandle = &gptHomebusHandle;
    
    if(ptHandle->init == 0) return;
    
    CurReadBit = getGpioValue(ptHandle->rxdGpio, 1);  //Get data from Rx GPIO
    if(ptHandle->rxState == RX_IDLE || ptHandle->rxState == RX_ACK) {
        if(CurReadBit == 0 && ptHandle->rxPreReadBit != CurReadBit)
        {
            ptHandle->rxChkTime = getCurTimer(0);
            ptHandle->rxNextReadTime = ptHandle->rxTickPerbit;
            ptHandle->readByte = 0x0;
            ptHandle->rxState = RX_PROCESSING;
        }
        else if(getDuration(0, ptHandle->rxChkTime) >= ptHandle->rxNextReadTime && ptHandle->rxState == RX_ACK) {
            // data frame end, send ack
            ptHandle->txWriteLen = 2;
            ptHandle->pWriteData[0] = 0x21;
            ptHandle->pWriteData[1] = 0x06;
            homebusTxSendDataOut();
            ptHandle->rxState = RX_IDLE;
        }
	}
    else if(getDuration(0, ptHandle->rxChkTime) >= ptHandle->rxNextReadTime && ptHandle->rxState == RX_PROCESSING) {
        if(ptHandle->rxBitIdx < 8) {
			ptHandle->readByte |= (CurReadBit << ptHandle->rxBitIdx);	
			ptHandle->rxBitIdx++;
            ptHandle->rxNextReadTime += ptHandle->rxTickPerbit;
		}
		else{
			// Receive 1 Byte done
			ptHandle->rxBitIdx = 0;
            ptHandle->rxState = RX_ACK;
			ptHandle->pReadData[ptHandle->rxWriteIdx] = ptHandle->readByte;
			ptHandle->rxWriteIdx++;
            if(ptHandle->rxWriteIdx >= MAX_PORT_BUFFER_SIZE) {
	        	ptHandle->rxWriteIdx -= MAX_PORT_BUFFER_SIZE;
        	}
            ptHandle->rxNextReadTime += (ptHandle->tickPerMs<<2);// 4ms for ack
		}
	}
	ptHandle->rxPreReadBit = CurReadBit;
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
				case READ_DATA_CMD_ID:
					homebusProcessReadCmd();
					break;
                default:
                    break;
            }
            ALT_CPU_COMMAND_REG_WRITE(RESPONSE_CMD_REG, (uint16_t) inputCmd);
        }
        homebusMonitorRx();
    }
}
