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



int homebus_senddata(uint8_t* buf,unsigned char len)
{
	printf("Homebus Send(%d) :\n", len);
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


void homebus_control()
{

	while(1)
	{
		tx_deal();
		//rx_deal();
		init_tx_deal();//10 ms
		system_tx_check();//10 ms
		usleep(5000);
	}
}





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



    pthread_t readThread;
	pthread_create(&readThread, NULL, homebus_control, NULL);

	
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
#if 1
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
#endif   
        usleep(1000*100);
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

//	homebus_test_ABC();

    homebus_test();

#ifdef CFG_DBG_TRACE
    vTraceStop();
#endif
    return 0;
}
