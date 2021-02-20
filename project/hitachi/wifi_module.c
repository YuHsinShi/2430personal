#include <sys/ioctl.h>
#include <assert.h>
#include <stdio.h>
#include "ite/itp.h"





#ifndef WIN32
//#define printr_wifimodule	 printf
#define printr_wifimodule	 

typedef enum 
{
	STAGE_BOOTING_UP,
	STAGE_HANDSHAKING,
	STAGE_DATATRASFER,
	STAGE_UNKNOWN,
} WIFI_STAGE;

//24 15 04 01 48 69 73 65 6E 73 65 2D 50 43 2D 50 31 48 45 51 32 FA 57 配网

int WifiModuleRequestMatch()
{
	int len = 0;
	char buff[256] ={0};
	unsigned char cmd_request[32]={0x24,0x15,0x04,0x01,0x48,0x69,0x73,0x65,0x6E,0x73,0x65,0x2D,0x50,0x43,0x2D,0x50,
								 0x31,0x48,0x45,0x51,0x32,0xFA,0x57};
	printf("WifiModuleRequestMatch==\n");

	len = write(ITP_DEVICE_UART3,cmd_request, 23);		
	usleep(200*1000); //wait for response		
	len = read(ITP_DEVICE_UART3, buff, 128);
	
	if(len !=0){
		if(1 == wifi_rx_frame_crc_check(buff,(unsigned char)len)){
			printr_wifimodule("===========WifiModuleRequestMatch==============================\n");
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
			printr_wifimodule("===========WifiModuleQuery Status==============================\n");
			printr_wifimodule("link status =%d 	   (0:disconnect,1:weak signal,2:medium siganl,3: strong signal) \n",buff[3]);
			printr_wifimodule("link reply command=%d  (0: no opreation, 1: begin confirm) \n",buff[4]);
			printr_wifimodule("link result=%d	       (0:link fail, 1: link OK) \n",buff[5]);			
			printr_wifimodule("FW version =%x%x%x \n",buff[6],buff[7],buff[8]);
			printr_wifimodule("==============================================================\n");	
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
WIFI_STAGE stage=STAGE_BOOTING_UP;

	WifiModulePowerOn();
static int flag_configed;

	while (1)
	{
	
 		WifiModuleQuery();	
	
		usleep(3000*1000);
	}

	while (1)
	{
		switch(stage)
		{
		case STAGE_BOOTING_UP:
				//wait "MODE" key pressed to start 
				break;
		case STAGE_HANDSHAKING:
			
			//query wifi moudle every 30 seconds
			ret=WifiModuleQuery();
			//CONDITION 1: NETWORK　is OKAY //QUERY status every 30 seconds
			
			//CONDITION 2: NETWORK　is FAIL  //show UI for connect wifi fail
			
			//CONDITION 3: MODULE　no replied //wait 5 seconds and polling 3 time  //show module fail UI

		
			break;
		case STAGE_DATATRASFER:
			
			break;

		default:
			break;
		}
	
	

		usleep(1000*1000);
	}




}

void wifi_module_ini()
{
 	pthread_t wifi_tid;
	pthread_create(&wifi_tid, NULL, WifiModuleHandle, NULL);
    return 0;
}

#endif