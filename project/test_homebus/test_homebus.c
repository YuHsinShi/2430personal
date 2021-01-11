#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "ite/itp.h"
#include "alt_cpu/alt_cpu_device.h"
#include "alt_cpu/homebus/homebus.h"

//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//				  Macro Definition
//=============================================================================
#define CMD_LEN	3
#define MAX_DATA_SIZE	128

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================








void homebus_test()
{
	HOMEBUS_INIT_DATA tHomebusInitData = { 0 };
    HOMEBUS_READ_DATA tHomebusReadData = { 0 };
    HOMEBUS_WRITE_DATA tHomebusWriteData = { 0 };
	uint8_t pWriteData[MAX_DATA_SIZE] = { 0x21, 0x0, 0x1c, 0x2, 0x1, 0xff, 0xff, 0xff, 0xff, 0x1, 0x80, 0x0, 0xff, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x61 };
	uint8_t pReadData[MAX_DATA_SIZE] = { 0 };
	int len = 0, count = 0, ret = 0;

    printf("Start Homebus\n");
    
    tHomebusInitData.cpuClock = ithGetRiscCpuClock();
	tHomebusInitData.txdGpio = CFG_GPIO_HOMEBUS_TXD;
    // tHomebusInitData.rxdGpio = CFG_GPIO_UART2_RX;//CFG_GPIO_HOMEBUS_RXD;
    // tHomebusInitData.parity  = NONE;
    tHomebusInitData.uid[0] = 0x01;
    tHomebusInitData.uid[1] = 0x01;
    
	ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_INIT_PARAM, &tHomebusInitData);
    printf("Homebus init OK\n");



	
	usleep(1000*10);

    tHomebusReadData.len = 1;//CMD_LEN;
    tHomebusReadData.pReadDataBuffer = pReadData;

    while(1)
    {
        tHomebusReadData.len = MAX_DATA_SIZE;
#if 0//1
        // printf("write data *****\n");
        ret = ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_WRITE_DATA, &tHomebusWriteData);
        if(ret == 10) printf("write data Fail (%d)\n", ret);
        else printf("write data OK (%d)\n", ret);
        // printf("write data &&&&& (%d)\n", len);
        // usleep(1000*1000*3);
#endif
#if 0
        len = ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_READ_DATA, &tHomebusReadData);
        // printf("read data &&&&& (%d)\n", len);
        if(len > 0) {
            printf("Homebus Recv(%d) :\n", len);
            for(count = 0; count < len; count++) {
                printf("0x%x ", pReadData[count]);
            }
            printf("\r\n");

		//maybe there are many frames queue

		
			rx_data_update(pReadData,len);
			rx_deal();


        }
#endif   
        usleep(1000*10);
        // usleep(1000*1);
        // usleep(1000*1000*1);
    }
}








void homebus_test_ABC()
{
	HOMEBUS_INIT_DATA tHomebusInitData = { 0 };
    HOMEBUS_READ_DATA tHomebusReadData = { 0 };
    HOMEBUS_WRITE_DATA tHomebusWriteData = { 0 };
	uint8_t pWriteData[MAX_DATA_SIZE] = {0};
	uint8_t pReadData[MAX_DATA_SIZE] = { 0 };
	int len = 0, count = 0, ret = 0;

    printf("Start Homebus\n");
    
    tHomebusInitData.cpuClock = ithGetRiscCpuClock();
	tHomebusInitData.txdGpio = CFG_GPIO_HOMEBUS_TXD;
    // tHomebusInitData.rxdGpio = CFG_GPIO_UART2_RX;//CFG_GPIO_HOMEBUS_RXD;
    // tHomebusInitData.parity  = NONE;
    tHomebusInitData.uid[0] = 0x01;
    tHomebusInitData.uid[1] = 0x01;
    
	ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_INIT_PARAM, &tHomebusInitData);
    printf("Homebus init OK\n");


		uint8_t i;

		for(i=0;i<0xff;i++)
		{
			pWriteData[i]=i;

		}

	

    while(1)
    {


		homebus_senddata(pWriteData,255);	


        tHomebusReadData.len = 255;
        len = ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_READ_DATA, &tHomebusReadData);
        // printf("read data &&&&& (%d)\n", len);
        if(len > 0) {
            printf("Homebus Recv(%d) :\n", len);
            for(count = 0; count < len; count++) {
                printf("0x%x ", pReadData[count]);
            }
            printf("\r\n");
//			rx_data_lenth =(unsigned char) len;
//			memcpy(rx_data,pReadData,len);
			rx_data_update(pReadData,len);
			rx_deal();
        }
        usleep(1000*500);
        // usleep(1000*1);
        // usleep(1000*1000*1);
    }
}


//24 15 04 01 48 69 73 65 6E 73 65 2D 50 43 2D 50 31 48 45 51 32 FA 57 配网

int WifiModuleRequestMatch()
{
	int len = 0;
	unsigned char buff[256] ={0};
	unsigned char cmd_request[32]={0x24,0x15,0x04,0x01,0x48,0x69,0x73,0x65,0x6E,0x73,0x65,0x2D,0x50,0x43,0x2D,0x50,
								 0x31,0x48,0x45,0x51,0x32,0xFA,0x57};
	printf("WifiModuleRequestMatch==\n");

	len = write(ITP_DEVICE_UART3,cmd_request, 23);		
	usleep(200*1000); //wait for response		
	len = read(ITP_DEVICE_UART3, buff, 128);
	
	if(len !=0){
		if(1 == wifi_rx_frame_crc_check(buff,(unsigned char)len)){
			printf("=ifiModuleRequestMatch==\n");
			int i;
			for(i=0;i<=len;i++)
				printf("0x%x ",buff[i]);

			printf(" \n");
			
			return 1;

		}
		else{
			
		printf("crc error \n");
		
		return -1;
		}
	
		
	}
	else
	{
		printf("query no response\n");
		return -2;

	}
	
return 1;
}

//24 15 04 00 48 69 73 65 6E 73 65 2D 50 43 2D 50 31 48 45 51 32 AB AB 查询
int WifiModuleQuery()
{
	int len = 0;
	char buff[256] ={0};
	unsigned char cmd_query[32]={0x24,0x15,0x04,0x00,0x48,0x69,0x73,0x65,0x6E,0x73,0x65,0x2D,0x50,0x43,0x2D,0x50,
								 0x31,0x48,0x45,0x51,0x32,0xAB,0xAB};



	len = write(ITP_DEVICE_UART3,cmd_query, 23);		
	//printf("send %d byte\n",len);
	usleep(50*1000); //wait for response		
	len = read(ITP_DEVICE_UART3, buff, 128);
	if(len !=0){
		
		if(1 == wifi_rx_frame_crc_check(buff,(unsigned char)len)){
			printf("===========WifiModuleQuery Status==============================\n");
			printf("link status =%d 	   (0:disconnect,1:weak signal,2:medium siganl,3: strong signal) \n",buff[3]);
			printf("link reply command=%d  (0: no opreation, 1: begin confirm) \n",buff[4]);
			printf("link result=%d	       (0:link fail, 1: link OK) \n",buff[5]);			
			printf("FW version =%x%x%x \n",buff[6],buff[7],buff[8]);
			printf("==============================================================\n");	
			return 1;
		}
		else{
			
			printf("crc error \n");
			return -1;
		}
	
		
	}
	else
	{
		printf("query no response\n");
		return -2;

	}
return 1;
}

typedef enum 
{
	STAGE_BOOTING_UP,
	STAGE_HANDSHAKING,
	STAGE_DATATRASFER,
	STAGE_UNKNOWN,
} WIFI_STAGE;
void WifiModulePowerOn()
{
#define WiFiGPIO_PWR	50
	ithGpioSetOut(WiFiGPIO_PWR);
	ithGpioSetMode(WiFiGPIO_PWR, ITH_GPIO_MODE0);
	ithGpioClear(WiFiGPIO_PWR); //POWER ON
		usleep(2*1000*1000); //wait for response		

}
void* WifiModuleHandle(void* arg)
{
	int len = 0, count = 0;
	char getstr[256] = "", sendstr[256] = "";
	int ret;
WIFI_STAGE stage=STAGE_HANDSHAKING;

	WifiModulePowerOn();
//static int flag_configed;
while(1)
{
	if(1== WifiModuleQuery())
		break;
	
	usleep(3000*1000);
}
	while (1)
	{
		switch(stage)
		{
		case STAGE_BOOTING_UP:
				//wait "MODE" key pressed to start
				WifiModuleQuery();
				break;
		case STAGE_HANDSHAKING:
			
			//query wifi moudle every 30 seconds
			if(1 == WifiModuleRequestMatch() ){
			//CONDITION 1: NETWORK　is OKAY //QUERY status every 30 seconds
			
			//CONDITION 2: NETWORK　is FAIL  //show UI for connect wifi fail
			
			//CONDITION 3: MODULE　no replied //wait 5 seconds and polling 3 time  //show module fail UI
			stage=STAGE_BOOTING_UP;

			
			}
			sleep(3);
			break;
		case STAGE_DATATRASFER:
			
			break;

		default:
			break;
		}


		usleep(1000*1000);
	}

	while (1)
	{
	
 		WifiModuleQuery();	
	
		usleep(3000*1000);
	}


}

void* TestFunc_homebus(void* arg)
{
    int altCpuEngineType = ALT_CPU_HOMEBUS;

#ifdef CFG_DBG_TRACE
    uiTraceStart();
#endif

    itpInit();




    //Load Engine on ALT CPU
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_ALT_CPU_SWITCH_ENG, &altCpuEngineType);
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT, NULL);

//	homebus_test_ABC();

 //   homebus_test();
	homebus_init();

#ifdef CFG_DBG_TRACE
    vTraceStop();
#endif
    return 0;
}


//for wifi
void* TestFunc(void* arg)
{
    int altCpuEngineType = ALT_CPU_HOMEBUS;

#ifdef CFG_DBG_TRACE
    uiTraceStart();
#endif

    itpInit();

	pthread_t wifi_tid;
	pthread_create(&wifi_tid, NULL, WifiModuleHandle, NULL);

#ifdef CFG_DBG_TRACE
    vTraceStop();
#endif
    return 0;
}

