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

// #define RS485_CMD_LEN	RS232_CMD_LEN
// #define MAX_RS485_SIZE	MAX_RS232_SIZE

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


	ithGpioSetOut(34);
	ithGpioSetMode(34, ITH_GPIO_MODE0);
	ithGpioClear(34);



	HOMEBUS_INIT_DATA tHomebusInitData = { 0 };
    HOMEBUS_READ_DATA tHomebusReadData = { 0 };
    HOMEBUS_WRITE_DATA tHomebusWriteData = { 0 };
//	uint8_t pWriteData[MAX_DATA_SIZE] = { 0x31, 0x32, 0x33};

	
	uint8_t pWriteData[MAX_DATA_SIZE] = 
	{ 
		0x21,0x00,0x1C,0x02,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0xA0,0x18,0x1B,0x10,0x86,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2B 
//		0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,
//		0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa 
//		0x21,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10,0x11,0x12,0x13,0x14,0x15,
//		0x16,0x17,0x18,0x19,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27 
//		0x21,0x01,0x16,0x16,0x16,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
//		0x16,0x16,0x16,0x18,0x16,0x16,0x18,0x18,0x18,0x16,0x18,0x18 

	};

	uint8_t pReadData[MAX_DATA_SIZE] = { 0 };
	int len = 0, count = 0;

    printf("Start Homebus\n");


	tHomebusInitData.cpuClock = ithGetRiscCpuClock();
	tHomebusInitData.txdGpio = CFG_GPIO_HOMEBUS_TXD;
    tHomebusInitData.rxdGpio = CFG_GPIO_HOMEBUS_RXD;



	ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_INIT_PARAM, &tHomebusInitData);

	while(1)
{
	printf("Next\n");


    tHomebusWriteData.len = 28;
    tHomebusWriteData.pWriteDataBuffer = &pWriteData[0];

	len = ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_WRITE_DATA, &tHomebusWriteData);

	


	usleep(10*1000*1000);
//return;
}
	


	#if 1
			len = ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_READ_DATA, &tHomebusReadData);
			if(len > 0) {
				printf("Homebus Read(%d) :\n", len);
				for(count = 0; count < len; count++) {
					printf("0x%x ", pReadData[count]);
				}
				printf("\r\n");
			}
		#endif   
		usleep(1000*1000*5);

	
return;

    printf("Homebus init OK\n");
	usleep(1000*10);

    tHomebusReadData.len = 1;//CMD_LEN;
    tHomebusReadData.pReadDataBuffer = pReadData;

    tHomebusWriteData.len = 2;//CMD_LEN;
    tHomebusWriteData.pWriteDataBuffer = pWriteData;

    while(1)
    {
        tHomebusReadData.len = 1;
        // printf("write data *****\n");
        len = ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_WRITE_DATA, &tHomebusWriteData);
        // printf("write data &&&&& (%d)\n", len);
#if 1
        len = ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_READ_DATA, &tHomebusReadData);
        if(len > 0) {
            printf("Homebus Read(%d) :\n", len);
            for(count = 0; count < len; count++) {
                printf("0x%x ", pReadData[count]);
            }
            printf("\r\n");
        }
#endif   
        usleep(1000*1000*1);
    }
}



void homebus_init()
{






	HOMEBUS_INIT_DATA tHomebusInitData = { 0 };
	HOMEBUS_READ_DATA tHomebusReadData = { 0 };
	HOMEBUS_WRITE_DATA tHomebusWriteData = { 0 };
	//	uint8_t pWriteData[MAX_DATA_SIZE] = { 0x31, 0x32, 0x33};

	printf("Start Homebus\n");


	tHomebusInitData.cpuClock = ithGetRiscCpuClock();
	tHomebusInitData.txdGpio = CFG_GPIO_HOMEBUS_TXD;
	tHomebusInitData.rxdGpio = CFG_GPIO_HOMEBUS_RXD;		


	ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_INIT_PARAM, &tHomebusInitData);



}

void homebus_senddata(char* buf,char len)
{
	printf("homebus_senddata \n", len);
	int count;
	ithGpioSetOut(34);
	ithGpioSetMode(34, ITH_GPIO_MODE0);
	ithGpioClear(34);

	HOMEBUS_WRITE_DATA tHomebusWriteData = { 0 };
		
	tHomebusWriteData.len = len;
	tHomebusWriteData.pWriteDataBuffer =buf;

	for(count = 0; count < len; count++) {
		printf("0x%x ", buf[count]);
	}
	printf("\r\n");



	ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_WRITE_DATA, &tHomebusWriteData);
	//printf("homebus_senddata end\n");	

	
	ithGpioSetOut(34);
	ithGpioSetMode(34, ITH_GPIO_MODE0);
	ithGpioSet(34);
}

void homebus_recvdata(char* buf,unsigned char* len)
{
unsigned int recv_len;
int count;
	uint8_t pReadData[MAX_DATA_SIZE] = { 0 };

	HOMEBUS_READ_DATA tHomebusReadData = { 0 };

    tHomebusReadData.len = MAX_DATA_SIZE;//CMD_LEN;
    tHomebusReadData.pReadDataBuffer = pReadData;

	recv_len = ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_READ_DATA, &tHomebusReadData);


if(recv_len>50)	
{
	printf("recv_len ERROR (%d)\n", recv_len);
	return;
}
	if(recv_len > 0) {
		printf("Homebus Read(%d) :\n", recv_len);
		for(count = 0; count < recv_len; count++) {
			printf("0x%x ", pReadData[count]);
		}
		printf("\r\n");
	}

	//memcpy(buf,pReadData,recv_len);
	//*len=recv_len;



}

void homebus_control()
{
		homebus_init();

static int counter;
int flag=0;
/*
while(1)
{

	ithGpioSetOut(34);
	ithGpioSetMode(34, ITH_GPIO_MODE0);
	ithGpioSet(34);

	homebus_recvdata(0,0);
	
	usleep(10000);
}
*/
	while(1)
	{
		tx_deal();
		rx_deal();
		init_tx_deal();//10 ms
		system_tx_check();//10 ms
		usleep(5000);
/*
		if(999==counter)
		{
			if(0==flag)
			{
			printf("power on\n");
				homebus_api_power_on();
				flag =1;
			}
			else
			{
				printf("power off\n");

				homebus_api_power_off();
				flag =0;
			}
			counter=0;

		}
		counter++;
			
*/
		}
}



void* TestFunc(void* arg)
{
    int altCpuEngineType = ALT_CPU_HOMEBUS;

#ifdef CFG_DBG_TRACE
    uiTraceStart();
#endif

    itpInit();

    //Load Engine on ALT CPU
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_ALT_CPU_SWITCH_ENG, &altCpuEngineType);
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT, NULL);
//    homebus_test();
	homebus_control();

#ifdef CFG_DBG_TRACE
    vTraceStop();
#endif
    return 0;
}
