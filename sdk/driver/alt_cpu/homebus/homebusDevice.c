#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "alt_cpu/homebus/homebus.h"

#if defined(CFG_UART2_ENABLE)
#define HOMEBUS_RX_PORT ITP_DEVICE_UART2
#else
#error "Need UART2 ENABLE !"
#endif

#define DBG_HOMEBUS  		1
#if DBG_HOMEBUS
#define printf_hb	printf 
#else
#define printf_hb	
#endif
typedef struct
{
    pthread_t *readThread;
    uint32_t rxReadLen;
    uint32_t rxWriteIdx;
    uint32_t rxReadIdx;
    uint8_t txData[MAX_PORT_BUFFER_SIZE];
    uint8_t readData[MAX_PORT_BUFFER_SIZE];
    uint8_t uid[2];
    uint8_t readQuit;
    uint8_t status;
    uint8_t txLen;
    uint8_t txColsChk;
} HOMEBUS_DEV_HANDLE;

static uint8_t gpHomebusImage[] =
{
    #include "homebus.hex"
};

static pthread_mutex_t gHomebusReadMutex  = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t gHomebusStatusMutex  = PTHREAD_MUTEX_INITIALIZER;
static HOMEBUS_DEV_HANDLE gHomebusDevHandle = {0};

static void homebusProcessCommand(int cmdId);

static void homebusSetStatus(uint8_t sta)
{
    pthread_mutex_lock(&gHomebusStatusMutex);
    gHomebusDevHandle.status = sta;
    pthread_mutex_unlock(&gHomebusStatusMutex);
}

static uint8_t homebusGetStatus()
{
    uint8_t sta = 0;
    pthread_mutex_lock(&gHomebusStatusMutex);
    sta = gHomebusDevHandle.status;
    pthread_mutex_unlock(&gHomebusStatusMutex);
    return sta;
}

static void homebus_send_ack(uint8_t resp_id, bool isAck)
{
		uint8_t* pWriteAddress = (uint8_t*) (iteRiscGetTargetMemAddress(ALT_CPU_IMAGE_MEM_TARGET) + CMD_DATA_BUFFER_OFFSET);

		ithGpioSetOut(34);
		ithGpioSetMode(34, ITH_GPIO_MODE0);
		ithGpioClear(34);

		usleep(1000*2); //2ms //ack delay
		HOMEBUS_WRITE_DATA tHomebusWriteData = {2, {0, 0}};

			tHomebusWriteData.pWriteBuffer[0]=(resp_id>>4) | (resp_id<<4);
			if(isAck){
				tHomebusWriteData.pWriteBuffer[1]=0x06;
					printf_hb("ACK 0x%x 0x%x\n",tHomebusWriteData.pWriteBuffer[0],tHomebusWriteData.pWriteBuffer[1]);}
			else{
				tHomebusWriteData.pWriteBuffer[1]=0x15;				
				printf_hb("NACK 0x%x 0x%x\n",tHomebusWriteData.pWriteBuffer[0],tHomebusWriteData.pWriteBuffer[1]);}


		memcpy(pWriteAddress, &tHomebusWriteData, sizeof(HOMEBUS_READ_DATA));
		homebusProcessCommand(WRITE_DATA_CMD_ID);

		


		ithGpioSetOut(34);
		ithGpioSetMode(34, ITH_GPIO_MODE0);
		ithGpioSet(34);

}

static void *read_thread_func(void *arg)
{
    uint32_t rIdx = 0;
    uint8_t rBuff[MAX_PORT_BUFFER_SIZE] = {0};
    uint32_t fLen = 0;
    uint8_t no_data_cnt = 0;
    uint32_t rxTick = 0;
    HOMEBUS_READ_DATA tHomebusReadData = {0};
    uint8_t* pWriteAddress = (uint8_t*) (iteRiscGetTargetMemAddress(ALT_CPU_IMAGE_MEM_TARGET) + CMD_DATA_BUFFER_OFFSET);

	int i;
    while(!gHomebusDevHandle.readQuit)
    {
        memset(tHomebusReadData.pReadBuffer, 0x0, MAX_PORT_BUFFER_SIZE);
        tHomebusReadData.len = read(HOMEBUS_RX_PORT, tHomebusReadData.pReadBuffer, 128);
        
        if(tHomebusReadData.len > 0)
        {
            rxTick = itpGetTickCount();
            
            if(homebusGetStatus() == TX_ING)
            {
                //tx loopback chk
                memcpy(rBuff+rIdx, tHomebusReadData.pReadBuffer, tHomebusReadData.len);
                rIdx += tHomebusReadData.len;
                if(gHomebusDevHandle.txColsChk == 1 && rIdx == 1 && rBuff[0] == 0xaa) {homebusSetStatus(TX_NO_COLLISION); goto dropData;}
                else if (rIdx == gHomebusDevHandle.txLen + 2 && rBuff[gHomebusDevHandle.txLen+1] == 0x06) {homebusSetStatus(TX_ACK); goto dropData;}
                else if (rIdx == gHomebusDevHandle.txLen + 2 && rBuff[gHomebusDevHandle.txLen+1] == 0x15) {homebusSetStatus(TX_NACK); goto dropData;}
                else if (memcmp(gHomebusDevHandle.txData, rBuff, rIdx) != 0){			
					//collision
                    homebusSetStatus(TX_COLLISION); goto dropData;
                }  
                continue;
            }
            else if(homebusGetStatus() == RX_ACK)
            {
                //rx ack loopback chk bypass
                rIdx += tHomebusReadData.len;
                if(rIdx == 2) {homebusSetStatus(IDLE); goto dropData;}
                continue;
            }
            
            //rx state
            // for(int i = 0; i < tHomebusReadData.len; i++) printf("[%d]=%x\n", i, tHomebusReadData.pReadBuffer[i]);
            homebusSetStatus(RX_ING);
            memcpy(rBuff+rIdx, tHomebusReadData.pReadBuffer, tHomebusReadData.len);
            rIdx += tHomebusReadData.len;
            // for(int i = 0; i < rIdx; i++) printf("[%d]=%x\n", i, rBuff[i]);
            if(rIdx > 3) fLen = rBuff[2];
            
            if(rIdx > 0 && rBuff[0] != 0x12 && rBuff[0] != 0x17 && rBuff[0] != 0x41&& rBuff[0] != 0x21 && rBuff[0] != 0x71) 
				{/*printf_hb("[HL Drop]header error 0x%x len=%d\n", rBuff[0], tHomebusReadData.len);*/ homebusSetStatus(IDLE); goto dropData;}
            if(rIdx > 4 && rBuff[3] == gHomebusDevHandle.uid[0] && rBuff[4] == gHomebusDevHandle.uid[1]) 
				{printf_hb("[HL Drop]addr error 0x%x 0x%x\n",rBuff[3],rBuff[4] ); homebusSetStatus(IDLE); goto dropData;}
            // if(rIdx > 6){
                // printf("!!@@ %x %x %x %x\n", rBuff[5], rBuff[6], gHomebusDevHandle.uid[0], gHomebusDevHandle.uid[1]);
            // }
            if(rIdx > 6 && rBuff[5] != gHomebusDevHandle.uid[0] && rBuff[6] != gHomebusDevHandle.uid[1]) 
				{printf_hb("[HL Drop]addr2 error 0x%x 0x%x\n",rBuff[5],rBuff[6] ); homebusSetStatus(IDLE); goto dropData;}
			
            if(rIdx == rBuff[2]) //full frame
            {
                uint8_t bcc = rBuff[1];
                int i = 0;
                for(i = 2; i < fLen-1; i++)
                    bcc ^= rBuff[i];
                homebusSetStatus(RX_ACK);
                if(bcc == rBuff[fLen-1]) {
                    //ack



					homebus_send_ack(rBuff[0],1);


				
                    pthread_mutex_lock(&gHomebusReadMutex);
                    //cpy to readData
                    for(i = 0; i < fLen; i++)
                    {
                        gHomebusDevHandle.readData[gHomebusDevHandle.rxWriteIdx] = rBuff[i];
                        gHomebusDevHandle.rxWriteIdx++;
                        if(gHomebusDevHandle.rxWriteIdx == MAX_PORT_BUFFER_SIZE) {
                            gHomebusDevHandle.rxWriteIdx = 0;
                        }
                    }
                    pthread_mutex_unlock(&gHomebusReadMutex);


					
                    goto dropData;
                } else {

                    //nack
					homebus_send_ack(rBuff[0],0);

				
                    goto dropData;
                }
            }
        } else {
            no_data_cnt = itpGetTickDuration(rxTick);
            // printf("1111115555555555 no_data_cnt(%d)(%d)\n",no_data_cnt,homebusGetStatus());
            if(no_data_cnt > 5) {
                if(homebusGetStatus() == TX_ING) {
                    //TX_ACK no response
                    if (gHomebusDevHandle.txColsChk == 1) 
						homebusSetStatus(TX_COLLISION);
                    else if(rIdx == gHomebusDevHandle.txLen) 
						homebusSetStatus(TX_ACK);
                    else 
						homebusSetStatus(TX_COLLISION);
                }
                else if (homebusGetStatus() == RX_ACK)
                {
                    homebusSetStatus(IDLE);
                }
                goto dropData;
            }
        }
        usleep(100);
        continue;
dropData:
        rIdx = fLen = no_data_cnt = 0;
        rxTick = itpGetTickCount();
        memset(rBuff, 0x0, MAX_PORT_BUFFER_SIZE);
    }
}

static void homebusReadThreadStart()
{
    if(gHomebusDevHandle.readThread != NULL)
    {
        gHomebusDevHandle.readQuit = 1;
        pthread_join(*gHomebusDevHandle.readThread, NULL);
        free(gHomebusDevHandle.readThread);
    }
    memset(&gHomebusDevHandle, 0x0, sizeof(gHomebusDevHandle));
    gHomebusDevHandle.readThread = (pthread_t *)malloc(sizeof(pthread_t));
    pthread_create(gHomebusDevHandle.readThread, NULL, read_thread_func, NULL);
}

static void homebusProcessReadData(HOMEBUS_READ_DATA* pReadData)
{
	int remainSize = 0;
	int cpySize = 0;
	int tailSize = 0;

    pthread_mutex_lock(&gHomebusReadMutex);

    gHomebusDevHandle.rxReadLen = pReadData->len;

    if(gHomebusDevHandle.rxWriteIdx == gHomebusDevHandle.rxReadIdx)
        goto end;
    else if(gHomebusDevHandle.rxWriteIdx > gHomebusDevHandle.rxReadIdx)
        remainSize = gHomebusDevHandle.rxWriteIdx - gHomebusDevHandle.rxReadIdx;
    else
        remainSize = MAX_PORT_BUFFER_SIZE - gHomebusDevHandle.rxReadIdx + gHomebusDevHandle.rxWriteIdx;

    if(gHomebusDevHandle.rxReadLen <= remainSize)
        cpySize = gHomebusDevHandle.rxReadLen;
    else
        cpySize = remainSize;

    tailSize = MAX_PORT_BUFFER_SIZE - gHomebusDevHandle.rxReadIdx;
    if(cpySize <= tailSize) {
        memcpy(pReadData->pReadDataBuffer, &gHomebusDevHandle.readData[gHomebusDevHandle.rxReadIdx], cpySize);
    }
    else {
        memcpy(pReadData->pReadDataBuffer, &gHomebusDevHandle.readData[gHomebusDevHandle.rxReadIdx], tailSize);
        memcpy(&pReadData->pReadDataBuffer[tailSize], &gHomebusDevHandle.readData[0], cpySize - tailSize);
    }

    gHomebusDevHandle.rxReadIdx += cpySize;
    if(gHomebusDevHandle.rxReadIdx >= MAX_PORT_BUFFER_SIZE) {
        gHomebusDevHandle.rxReadIdx -= MAX_PORT_BUFFER_SIZE;
    }
end:
    pthread_mutex_unlock(&gHomebusReadMutex);
    
    // The return Rx's Length
    pReadData->len = cpySize;
}

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

            if(ptInitData->txdGpio == 0xFFFFFFFF) {
                ithPrintf("[Homebus] set pin error !\n");
                return -1;
            }
            memcpy(pWriteAddress, ptInitData, sizeof(HOMEBUS_INIT_DATA));
            homebusProcessCommand(INIT_CMD_ID);			
            //read thread start
            homebusReadThreadStart();
            //uid
            memcpy(gHomebusDevHandle.uid, ptInitData->uid, sizeof(gHomebusDevHandle.uid));
            ithPrintf("[Homebus] ID : %x %x\n", gHomebusDevHandle.uid[0], gHomebusDevHandle.uid[1]);
            break;
        }
        case ITP_IOCTL_HOMEBUS_READ_DATA:
        {
            HOMEBUS_READ_DATA* ptReadData = (HOMEBUS_READ_DATA*) ptr;
            if(ptReadData->len >= MAX_PORT_BUFFER_SIZE) {
                ithPrintf("[Homebus] read len overflow !\n");
                return 0;
            }
            
            homebusProcessReadData(ptReadData);
            
            return ptReadData->len;
        }
        case ITP_IOCTL_HOMEBUS_WRITE_DATA:
        {
            uint8_t cnt = 0;
            HOMEBUS_WRITE_DATA *ptWriteData = (HOMEBUS_WRITE_DATA*) ptr;
            if(ptWriteData->len >= MAX_PORT_BUFFER_SIZE) {
                ithPrintf("[Homebus] write len overflow !\n");
                return 0;
            }
            gHomebusDevHandle.txLen = ptWriteData->len;
            memcpy(gHomebusDevHandle.txData, ptWriteData->pWriteDataBuffer, ptWriteData->len);
            memcpy(ptWriteData->pWriteBuffer, ptWriteData->pWriteDataBuffer, ptWriteData->len);
            memcpy(pWriteAddress, ptWriteData, sizeof(HOMEBUS_WRITE_DATA));

			gHomebusDevHandle.txColsChk = 0;

            while(1) {
                while(homebusGetStatus() != IDLE) {
                    //wait to idle
                    ithPrintf("[Homebus] write RX BUSY !\n");
                    usleep(1000*6); //6ms
                }                
				
                homebusSetStatus(TX_ING);
                homebusProcessCommand(WRITE_DATA_CMD_ID);
                
                while(homebusGetStatus() == TX_ING) {
                    //wait to ack
                    // ithPrintf("[Homebus] wait TX ack !\n");
                    usleep(1000); //1ms
                }
                
                if(homebusGetStatus() == TX_COLLISION) {
                    ithPrintf("[Homebus] write COLLISION !\n");
                    homebusSetStatus(IDLE);
                    gHomebusDevHandle.txColsChk = 1;
                    //collision
                    if(cnt < 3)
                        usleep(1000*6); //6ms
                    else usleep(1000*100); //100ms
                    cnt++;
                    // send 0xaa
                    HOMEBUS_WRITE_DATA tChkData = {1,{0xaa}};
                    memcpy(pWriteAddress, &tChkData, sizeof(HOMEBUS_WRITE_DATA));
                    continue;
                } else if (homebusGetStatus() == TX_NO_COLLISION) {
                    ithPrintf("[Homebus] write COLLISION re-send !\n");
                    homebusSetStatus(IDLE);
                    gHomebusDevHandle.txColsChk = 0;
                    //collision
                    //0xaa ok re-send data
                    cnt = 0;
                    memcpy(pWriteAddress, ptWriteData, sizeof(HOMEBUS_WRITE_DATA));
                    continue;
                } else {
                    if(homebusGetStatus() == TX_NACK) {
                        ithPrintf("[Homebus] write NACK !\n");
                        homebusSetStatus(IDLE);
                        if(cnt < 3) usleep(1000*6); //6ms
                        else if(cnt < 10) usleep(1000*100); //100ms
                        else break; //retry cnt > 10
                        cnt++;
                        memcpy(pWriteAddress, ptWriteData, sizeof(HOMEBUS_WRITE_DATA));
                        continue;
                    } else {
                        // ithPrintf("[Homebus] write success !(%d)(%d)\n", ((HOMEBUS_WRITE_DATA*) pWriteAddress)->len, homebusGetStatus());
                        //TX_ACK
                        homebusSetStatus(IDLE);
                        cnt = 0;
                        usleep(1000*100); //100ms
                        break; //success
                    }
                }
            }
            return cnt;
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
