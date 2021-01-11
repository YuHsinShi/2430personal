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
    uint32_t tickCnt;
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
static pthread_mutex_t gHomebusTickMutex  = PTHREAD_MUTEX_INITIALIZER;
static HOMEBUS_DEV_HANDLE gHomebusDevHandle = {0};


static pthread_mutex_t gThreadMutex  = PTHREAD_MUTEX_INITIALIZER;


static void homebusProcessCommand(int cmdId);



void homebus_logic_control()
{
printf("homebus_logic_control \n");


	int counter=10;

	while(1)
	{
	
	   // pthread_mutex_lock(&gThreadMutex);

		system_tx_check();
	
		init_tx_deal();

		if(0==counter){  tx_deal();	counter=10; } // do until 100ms 
		//rx_deal();		


		//pthread_mutex_unlock(&gThreadMutex);

		usleep(10*1000);//10ms 
		counter--;
		
	}
}

int homebus_senddata(uint8_t* buf,unsigned char len)
{
	printf("[HL SEND](%d) :", len);
int ret;
	if(0) //len >64)
	{
		//printf("homebus_senddata %d \n", len);
		return -1;
	}
	
	int count;
	ithGpioSetOut(34);
	ithGpioSetMode(34, ITH_GPIO_MODE0);
	ithGpioClear(34);

	HOMEBUS_WRITE_DATA tHomebusWriteData = { 0 };
		
	tHomebusWriteData.len = len;
	tHomebusWriteData.pWriteDataBuffer =buf;

	for(count = 0; count < len; count++) {
		printf("0x%2x ", buf[count]);
	}
	printf("\r\n");



	ret= ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_WRITE_DATA, &tHomebusWriteData);
	//printf("homebus_senddata end\n");	

	
	ithGpioSetOut(34);
	ithGpioSetMode(34, ITH_GPIO_MODE0);
	ithGpioSet(34);
	
	return ret;
}


void homebus_init()
{

    printf("Start homebus_init\n");
    
    int altCpuEngineType = ALT_CPU_HOMEBUS;
	HOMEBUS_INIT_DATA tHomebusInitData = { 0 };
    tHomebusInitData.cpuClock = ithGetRiscCpuClock();
	tHomebusInitData.txdGpio = CFG_GPIO_HOMEBUS_TXD;
    // tHomebusInitData.rxdGpio = CFG_GPIO_UART2_RX;//CFG_GPIO_HOMEBUS_RXD;
    // tHomebusInitData.parity  = NONE;
    tHomebusInitData.uid[0] = 0x01;
    tHomebusInitData.uid[1] = 0x01;
	ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_ALT_CPU_SWITCH_ENG, &altCpuEngineType);
	ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT, NULL);
	ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_INIT_PARAM, &tHomebusInitData);
    printf("homebus_initt finished\n");


}


static void homebusSetTickCnt()
{
    pthread_mutex_lock(&gHomebusTickMutex);
    gHomebusDevHandle.tickCnt = itpGetTickCount();
    pthread_mutex_unlock(&gHomebusTickMutex);
}

static uint32_t homebusGetTickCnt()
{
    uint32_t tick = 0;
    pthread_mutex_lock(&gHomebusTickMutex);
    tick = gHomebusDevHandle.tickCnt;
    pthread_mutex_unlock(&gHomebusTickMutex);
    return tick;
}

static void homebusSetStatus(uint8_t sta)
{
    pthread_mutex_lock(&gHomebusStatusMutex);
//	if(gHomebusDevHandle.status != sta)
//	    printf_hb("[homebusSetStatus] current(%d) new(%d)\n", gHomebusDevHandle.status, sta);
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
    tHomebusWriteData.pWriteBuffer[0] = (resp_id == 0x41 ? 0x41 : (resp_id>>4) | (resp_id<<4));
    tHomebusWriteData.pWriteBuffer[1] = (isAck ? 0x06 : 0x15);

    memcpy(pWriteAddress, &tHomebusWriteData, sizeof(HOMEBUS_READ_DATA));
    homebusProcessCommand(WRITE_DATA_CMD_ID);	

    ithGpioSetOut(34);
    ithGpioSetMode(34, ITH_GPIO_MODE0);
    ithGpioSet(34);
		
	printf_hb("[HB SEND- ACK/NACK] 0x%x 0x%x\n",tHomebusWriteData.pWriteBuffer[0],tHomebusWriteData.pWriteBuffer[1]);

}

static void *read_thread_func(void *arg)
{
    uint32_t rIdx = 0;
    uint8_t rBuff[MAX_PORT_BUFFER_SIZE] = {0};
    uint32_t fLen = 0;
    uint8_t no_data_cnt = 0;
    HOMEBUS_READ_DATA tHomebusReadData = {0};

	// int i;
    while(!gHomebusDevHandle.readQuit)
    {
        memset(tHomebusReadData.pReadBuffer, 0x0, MAX_PORT_BUFFER_SIZE);
        tHomebusReadData.len = read(HOMEBUS_RX_PORT, tHomebusReadData.pReadBuffer, 128);
        
        if(tHomebusReadData.len > 0)
        {

			//pthread_mutex_lock(&gThreadMutex);
			rx_homebus_receive_parser_bypass(tHomebusReadData.len,tHomebusReadData.pReadBuffer);
			///pthread_mutex_unlock(&gThreadMutex);


            homebusSetTickCnt();
            
            if(homebusGetStatus() == TX_ING)
            {
                //tx loopback chk
                memcpy(rBuff+rIdx, tHomebusReadData.pReadBuffer, tHomebusReadData.len);
                rIdx += tHomebusReadData.len;
                if(gHomebusDevHandle.txColsChk == 1 && rIdx == 1 && rBuff[0] == 0xaa) {homebusSetStatus(TX_NO_COLLISION); goto dropData;}
                else if (rIdx == gHomebusDevHandle.txLen + 2 && rBuff[gHomebusDevHandle.txLen+1] == 0x06) {	printf_hb("[HB RECV- ACK] 0x%x 0x%x \n",rBuff[gHomebusDevHandle.txLen],rBuff[gHomebusDevHandle.txLen+1]);  homebusSetStatus(TX_ACK); goto dropData;}
                else if (rIdx == gHomebusDevHandle.txLen + 2 && rBuff[gHomebusDevHandle.txLen+1] == 0x15) {printf_hb("[HB RECV- NACK] 0x%x 0x%x \n",rBuff[gHomebusDevHandle.txLen],rBuff[gHomebusDevHandle.txLen+1]); homebusSetStatus(TX_NACK); goto dropData;}
                else if (memcmp(gHomebusDevHandle.txData, rBuff, MIN(rIdx, gHomebusDevHandle.txLen)) != 0){			
					//collision
                    for(int i = 0; i < rIdx; i++) printf("[%d]=%x\n", i, rBuff[i]);
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
            else if(homebusGetStatus() == IDLE || homebusGetStatus() == RX_ING)
            {
	            //rx state
	            // for(int i = 0; i < tHomebusReadData.len; i++) printf("[%d]=%x\n", i, tHomebusReadData.pReadBuffer[i]);
	            homebusSetStatus(RX_ING);
	            memcpy(rBuff+rIdx, tHomebusReadData.pReadBuffer, tHomebusReadData.len);
	            rIdx += tHomebusReadData.len;
	            // for(int i = 0; i < rIdx; i++) printf("[%d]=%x\n", i, rBuff[i]);
	            if(rIdx > 3) fLen = rBuff[2];
	            
	            if(rIdx > 0 && rBuff[0] != 0x12 && rBuff[0] != 0x17 && rBuff[0] != 0x41&& rBuff[0] != 0x21 && rBuff[0] != 0x71) 
					{/*printf_hb("[HL Drop]header error 0x%x len=%d\n", rBuff[0], tHomebusReadData.len);*/ homebusSetStatus(IDLE); goto dropData;}
	            if(rIdx > 4 && (rBuff[0] == 0x41 || rBuff[0] == 0x21) && rBuff[3] == gHomebusDevHandle.uid[0] && rBuff[4] == gHomebusDevHandle.uid[1]) 
					{printf_hb("[HL Drop]addr error ,header:0x%x,add= (0x%x 0x%x)\n",rBuff[0],rBuff[3],rBuff[4] ); homebusSetStatus(IDLE); goto dropData;}
	            // if(rIdx > 6){
	                // printf("!!@@ %x %x %x %x\n", rBuff[5], rBuff[6], gHomebusDevHandle.uid[0], gHomebusDevHandle.uid[1]);
	            // }
	           // if(rIdx > 6 && rBuff[5] != gHomebusDevHandle.uid[0] && rBuff[6] != gHomebusDevHandle.uid[1]) 
				//	{printf_hb("[HL Drop]addr2 error 0x%x 0x%x\n",rBuff[5],rBuff[6] ); homebusSetStatus(IDLE); goto dropData;}
				
	            if(rIdx == rBuff[2]) //full frame
	            {
	                uint8_t bcc = rBuff[1];
	                int i = 0;
	                for(i = 2; i < fLen-1; i++)
	                    bcc ^= rBuff[i];
	                homebusSetStatus(RX_ACK);
	                if(bcc == rBuff[fLen-1]) {

					
						printf("[HL GET]FULL FRAME %d\n",fLen);
				
	                    //ack
						homebus_send_ack(rBuff[0],1);
					/*
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
	                    */
	                    goto dropData;
	                }
					else 
					{
	                    //nack
						homebus_send_ack(rBuff[0],0);
	                    goto dropData;
	                }
	            }
        	}
        } else {
            no_data_cnt = itpGetTickDuration(homebusGetTickCnt());
            // printf("[HB time out cnt] no_data_cnt(%d)(%d)\n",no_data_cnt,homebusGetStatus());
            if(no_data_cnt > 5) {
                if(homebusGetStatus() == TX_ING) {
                    //TX_ACK no response
                    if (gHomebusDevHandle.txColsChk == 1) {
                        printf("[HB time out cnt] txColsChk == 1 TX_COLLISION no_data_cnt(%d)(%d)\n",no_data_cnt,homebusGetStatus());
						homebusSetStatus(TX_COLLISION);
                    } else if(rIdx == gHomebusDevHandle.txLen) {
						homebusSetStatus(TX_ACK);
                    } else {
                        printf("[HB time out cnt] line 192 TX_COLLISION no_data_cnt(%d)(%d)(%d)(%d)\n",no_data_cnt,homebusGetStatus(),rIdx,gHomebusDevHandle.txLen);
                        for(int i = 0; i < rIdx; i++) printf("[%d]=%x\n", i, rBuff[i]);
						homebusSetStatus(TX_COLLISION);
                    }
                }
                else
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
        homebusSetTickCnt();
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
			
            //uid
            memcpy(gHomebusDevHandle.uid, ptInitData->uid, sizeof(gHomebusDevHandle.uid));
            ithPrintf("[Homebus] ID : %x %x\n", gHomebusDevHandle.uid[0], gHomebusDevHandle.uid[1]);

			//pthread_mutex_init(&gThreadMutex, NULL);
            //read thread start
            homebusReadThreadStart();



			pthread_t readThread;
			pthread_create(&readThread, NULL, homebus_logic_control, NULL);

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
                    ithPrintf("[Homebus] write RX BUSY waiting %d!\n",homebusGetStatus() );
                    usleep(1000*6); //6ms
                }                
				
                homebusSetTickCnt();
                homebusSetStatus(TX_ING);
				//add io pull low
				ithGpioSetOut(34);
				ithGpioSetMode(34, ITH_GPIO_MODE0);
				ithGpioClear(34);
                homebusProcessCommand(WRITE_DATA_CMD_ID);
				//add io pull high


				ithGpioSetOut(34);
				ithGpioSetMode(34, ITH_GPIO_MODE0);
				ithGpioSet(34);
				
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

					    usleep(1000*12); //do not send too quick wait 10-17 ms
					    
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
                    } else if(homebusGetStatus() == TX_ACK) {
                       // printf_hb("[Homebus] write success !(%d)(%d)\n", ((HOMEBUS_WRITE_DATA*) pWriteAddress)->len, homebusGetStatus());
                        //TX_ACK
                        homebusSetStatus(IDLE);
                        cnt = 0;
                        usleep(1000*100); //100ms
                        break; //success
                    } else {
                        ithPrintf("[Homebus] write status error !(%d)\n", homebusGetStatus());
                        homebusSetStatus(IDLE);
                        cnt = 10;
                        break;
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
