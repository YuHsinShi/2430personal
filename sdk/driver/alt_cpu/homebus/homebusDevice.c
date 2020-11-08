#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "alt_cpu/homebus/homebus.h"

typedef struct
{
    pthread_t *readThread;
    uint32_t rxReadLen;
    uint32_t rxWriteIdx;
    uint32_t rxReadIdx;
    uint8_t readData[MAX_PORT_BUFFER_SIZE];
    uint8_t uid[2];
    uint8_t readQuit;
    uint8_t status;
} HOMEBUS_DEV_HANDLE;

static uint8_t gpHomebusImage[] =
{
    #include "homebus.hex"
};

static pthread_mutex_t gHomebusMutex  = PTHREAD_MUTEX_INITIALIZER;
static HOMEBUS_DEV_HANDLE gHomebusDevHandle = {0};

static void homebusProcessCommand(int cmdId);

#define PAERENT_CONTROL		1
#define CHILD_CONTROL		0

static int  parsing_header(char header)
{
if(header == 0x12 || header == 0x17 || header == 0x41)
	return 1;//from indoor use specail rule
else if(header == 0x21 || header == 0x71)
	return 2;//from line control self
else
	return -1; //unknown

}

static void *read_thread_func(void *arg)
{
    uint32_t rIdx = 0;
    uint8_t rBuff[MAX_PORT_BUFFER_SIZE] = {0};
    uint32_t fLen = 0;
    uint8_t no_data_cnt = 0;

	int ret; 
	uint8_t tmp_addr;

	
    volatile HOMEBUS_READ_DATA tHomebusReadData = {0};
    uint8_t* pWriteAddress = (uint8_t*) (iteRiscGetTargetMemAddress(ALT_CPU_IMAGE_MEM_TARGET) + CMD_DATA_BUFFER_OFFSET);
    while(!gHomebusDevHandle.readQuit)
    {
        tHomebusReadData.len = MAX_PORT_BUFFER_SIZE;
        memset(tHomebusReadData.pReadBuffer, 0x0, MAX_PORT_BUFFER_SIZE);
        pthread_mutex_lock(&gHomebusMutex);
        memcpy(pWriteAddress, &tHomebusReadData, sizeof(HOMEBUS_READ_DATA));
        homebusProcessCommand(READ_DATA_CMD_ID);
        memcpy(&tHomebusReadData, pWriteAddress, sizeof(HOMEBUS_READ_DATA));
        pthread_mutex_unlock(&gHomebusMutex);
        
        if(tHomebusReadData.len > 0)
        {
        
		printf("#%d,%d status=%d#\n",rIdx,tHomebusReadData.len,gHomebusDevHandle.status);
            no_data_cnt = 0;
            
            if(gHomebusDevHandle.status == TX_ING)
            {
                if(tHomebusReadData.pReadBuffer[1] == 0x06) 
					gHomebusDevHandle.status = TX_ACK;
                else 
					gHomebusDevHandle.status = TX_NACK;
                continue;
            }
            // for(int i = 0; i < tHomebusReadData.len; i++) printf("[%d]=%x\n", i, tHomebusReadData.pReadBuffer[i]);
            gHomebusDevHandle.status = RX_ING;
            memcpy(rBuff+rIdx, tHomebusReadData.pReadBuffer, tHomebusReadData.len);
            rIdx += tHomebusReadData.len;

			//printf("#%d,%d#\n",rIdx,tHomebusReadData.len);

			
            // for(int i = 0; i < rIdx; i++) printf("[%d]=%x\n", i, rBuff[i]);
            if(rIdx > 3) fLen = rBuff[2];

			ret=parsing_header(rBuff[0]);
			
            if(1 == ret) //from indoor use specail rule
			{ 
				if(rIdx > 6)
				{
					if(PAERENT_CONTROL){ tmp_addr = 0x01;}else{ tmp_addr = 0x02;}; //depends on the mode set by upper layer

					if(tmp_addr != rBuff[6])
						{printf("(hlink)INDOOR DROP \n"); gHomebusDevHandle.status = IDLE; goto dropData;}

				}

			}
			else if(2 == ret) //from line control self
			{ 
				if(rIdx > 4 && rBuff[3] == gHomebusDevHandle.uid[0] && rBuff[4] == gHomebusDevHandle.uid[1]) 
					{printf("(hlink)LOOP DROP add =0x%x 0x%x\n",rBuff[3],rBuff[4]); gHomebusDevHandle.status = IDLE; goto dropData;}
				if(rIdx > 6 && rBuff[5] != gHomebusDevHandle.uid[0] && rBuff[6] != gHomebusDevHandle.uid[1]) 
					{printf("(hlink)OTHERS DROP\n"); gHomebusDevHandle.status = IDLE; goto dropData;}

				
			}
			else //illegal data
			{
				{gHomebusDevHandle.status = IDLE; goto dropData;}
			}



            if(rIdx == rBuff[2])
            {
                uint8_t bcc = rBuff[1];
                int i = 0;
                for(i = 2; i < fLen-1; i++)
                    bcc ^= rBuff[i];
                gHomebusDevHandle.status = RX_ACK;
                if(bcc == rBuff[fLen-1]) {
                    //ack
                    HOMEBUS_WRITE_DATA tHomebusWriteData = {2, {0, 0x06}};//{2, RX_ACK, {0, 0x06}};
                    tHomebusWriteData.pWriteBuffer[0] = (rBuff[0]>>4) | (rBuff[0]<<4);

					usleep(1000); //delay 1ms
					
                    pthread_mutex_lock(&gHomebusMutex);
                    memcpy(pWriteAddress, &tHomebusWriteData, sizeof(HOMEBUS_READ_DATA));					
                    homebusProcessCommand(WRITE_DATA_CMD_ID);
                    //cpy to readData
                    for(i = 0; i < fLen; i++)
                    {
                        gHomebusDevHandle.readData[gHomebusDevHandle.rxWriteIdx] = rBuff[i];
                        gHomebusDevHandle.rxWriteIdx++;
                        if(gHomebusDevHandle.rxWriteIdx == MAX_PORT_BUFFER_SIZE) {
                            gHomebusDevHandle.rxWriteIdx = 0;
                        }
                    }
                    pthread_mutex_unlock(&gHomebusMutex);
                    gHomebusDevHandle.status = IDLE;

					{printf("(hlink)RX ACK ind=%d,fLen=%d \n",gHomebusDevHandle.rxWriteIdx,fLen);}

					
                } else {
                    // printf("1111115555555555\n");
                    //nack
                    HOMEBUS_WRITE_DATA tHomebusWriteData = {2, {0, 0x15}};
                    tHomebusWriteData.pWriteBuffer[0] = (rBuff[0]>>4) | (rBuff[0]<<4);

					
					usleep(1000); //delay 1ms
					
                    pthread_mutex_lock(&gHomebusMutex);
                    memcpy(pWriteAddress, &tHomebusWriteData, sizeof(HOMEBUS_READ_DATA));
                    homebusProcessCommand(WRITE_DATA_CMD_ID);
                    pthread_mutex_unlock(&gHomebusMutex);
                    gHomebusDevHandle.status = IDLE;

					{printf("(hlink)RX NON ACK \n");}


					
                    goto dropData;
                }
                // gHomebusDevHandle.status = IDLE;
                
              }
        }
        else 
		{
		
		//printf("no_data_cnt =%d (%d)# \n", no_data_cnt,gHomebusDevHandle.status);
            //len == 0 cnt++ if cnt > 5 drop
            no_data_cnt++;
            if(no_data_cnt > 5) {
				
                if(gHomebusDevHandle.status == TX_ING) {
                    //TX_ACK no response
                    gHomebusDevHandle.status = TX_NACK;
							
					usleep(1000); //1ms
                    continue;
                }
                goto dropData;
            }
        }
        
        usleep(1000); //1ms
        continue;
dropData:
        rIdx = fLen = no_data_cnt = 0;
        memset(rBuff, 0x0, MAX_PORT_BUFFER_SIZE);
        // gHomebusDevHandle.status = IDLE;
        // usleep(1000); //1ms
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
    // gHomebusDevHandle.readQuit = 0;
    memset(&gHomebusDevHandle, 0x0, sizeof(gHomebusDevHandle));
    gHomebusDevHandle.readThread = (pthread_t *)malloc(sizeof(pthread_t));
   pthread_create(gHomebusDevHandle.readThread, NULL, read_thread_func, NULL);
}

static void homebusProcessReadData(HOMEBUS_READ_DATA* pReadData)
{
	int remainSize = 0;
	int cpySize = 0;
	int tailSize = 0;

    pthread_mutex_lock(&gHomebusMutex);

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
    pthread_mutex_unlock(&gHomebusMutex);
    
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
     volatile uint8_t* pWriteAddress = (uint8_t*) (iteRiscGetTargetMemAddress(ALT_CPU_IMAGE_MEM_TARGET) + CMD_DATA_BUFFER_OFFSET);
    switch (request)
    {
        case ITP_IOCTL_INIT:
        {
            //Stop ALT CPU
            iteRiscResetCpu(ALT_CPU);

            //Clear Commuication Engine and command buffer
            pthread_mutex_lock(&gHomebusMutex);
            memset(pWriteAddress, 0x0, MAX_CMD_DATA_BUFFER_SIZE);
            pthread_mutex_unlock(&gHomebusMutex);
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
            pthread_mutex_lock(&gHomebusMutex);
            memcpy(pWriteAddress, ptInitData, sizeof(HOMEBUS_INIT_DATA));
            homebusProcessCommand(INIT_CMD_ID);			
            pthread_mutex_unlock(&gHomebusMutex);
            //read thread start
           // homebusReadThreadStart();
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
            volatile HOMEBUS_WRITE_DATA *ptWriteData = (HOMEBUS_WRITE_DATA*) ptr;
            if(ptWriteData->len >= MAX_PORT_BUFFER_SIZE) {
                ithPrintf("[Homebus] write len overflow !\n");
                return 0;
            }
            memcpy(ptWriteData->pWriteBuffer, ptWriteData->pWriteDataBuffer, ptWriteData->len);
            while(gHomebusDevHandle.status != IDLE) {
                //wait to idle
                ithPrintf("[Homebus] write RX BUSY (%x)!\n",gHomebusDevHandle.status);
                usleep(1000*6); //6ms
            }
            pthread_mutex_lock(&gHomebusMutex);
            memcpy(pWriteAddress, ptWriteData, sizeof(HOMEBUS_WRITE_DATA));
			homebusProcessCommand(WRITE_DATA_CMD_ID);

            pthread_mutex_unlock(&gHomebusMutex);
			
			#if 0
            while(1) {
				
                gHomebusDevHandle.status = TX_ING;
                pthread_mutex_lock(&gHomebusMutex);
                homebusProcessCommand(WRITE_DATA_CMD_ID);
				 
                if(((HOMEBUS_WRITE_DATA*) pWriteAddress)->len == TX_COLLISION) {
                    pthread_mutex_unlock(&gHomebusMutex);
                    ithPrintf("[Homebus] write COLLISION !\n");
                    gHomebusDevHandle.status = IDLE;
                    //collision
                    if(cnt < 3)
                        usleep(1000*6); //6ms
                    else usleep(1000*100); //100ms
                    cnt++;
                    // send 0xaa
                    HOMEBUS_WRITE_DATA tChkData = {1,{0xaa}};
                    pthread_mutex_lock(&gHomebusMutex);
                    memcpy(pWriteAddress, &tChkData, sizeof(HOMEBUS_WRITE_DATA));
                    pthread_mutex_unlock(&gHomebusMutex);
                    continue;
                } else if (((HOMEBUS_WRITE_DATA*) pWriteAddress)->len == 1 && ((HOMEBUS_WRITE_DATA*) pWriteAddress)->pWriteBuffer[0] == 0xaa) {
                    pthread_mutex_unlock(&gHomebusMutex);
                    ithPrintf("[Homebus] write COLLISION re-send !\n");
                    gHomebusDevHandle.status = IDLE;
                    //collision
                    //0xaa ok re-send data
                    cnt = 0;
                    pthread_mutex_lock(&gHomebusMutex);
                    memcpy(pWriteAddress, ptWriteData, sizeof(HOMEBUS_WRITE_DATA));
                    pthread_mutex_unlock(&gHomebusMutex);
                    continue;
                } else if (((HOMEBUS_WRITE_DATA*) pWriteAddress)->len == RX_BUSY) {
                    pthread_mutex_unlock(&gHomebusMutex);
                    ithPrintf("[Homebus] write RX BUSY !\n");
                    gHomebusDevHandle.status = IDLE;
                    //rx busy
                    usleep(1000*6); //6ms
                    
                   // ithPrintf("[Homebus] write RX BUSY A!\n");
                    pthread_mutex_lock(&gHomebusMutex);
                    memcpy(pWriteAddress, ptWriteData, sizeof(HOMEBUS_WRITE_DATA));
                    pthread_mutex_unlock(&gHomebusMutex);
                  //  ithPrintf("[Homebus] write RX BUSY B!\n");

                    continue;
                } else {
					
				//ithPrintf("[Homebus] write RX BUSY 3    (%d)!\n",gHomebusDevHandle.status );
                    pthread_mutex_unlock(&gHomebusMutex);

                    while(gHomebusDevHandle.status == TX_ING){usleep(100);};
					//ithPrintf("[Homebus] write RX BUSY 4!\n");

                    if(gHomebusDevHandle.status == TX_NACK) {
                        ithPrintf("[Homebus] write NACK !\n");
                        gHomebusDevHandle.status = IDLE;
                        if(cnt < 3) usleep(1000*6); //6ms
                        else if(cnt < 10) usleep(1000*100); //100ms
                        else break; //retry cnt > 10
                        cnt++;
                        pthread_mutex_lock(&gHomebusMutex);
                        memcpy(pWriteAddress, ptWriteData, sizeof(HOMEBUS_WRITE_DATA));
                        pthread_mutex_unlock(&gHomebusMutex);
                        continue;
                    } else {
                        ithPrintf("[Homebus] write success !(%d)(%d)\n", ((HOMEBUS_WRITE_DATA*) pWriteAddress)->len, gHomebusDevHandle.status);
                        //TX_ACK
                        gHomebusDevHandle.status = IDLE;
                        cnt = 0;
                        usleep(1000*100); //100ms
                        break; //success
                    }
                }
            }
			#endif
            return cnt;//ptWriteData->len;
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
