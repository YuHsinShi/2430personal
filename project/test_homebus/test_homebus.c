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


#if 1
static void UartCallback(void* arg1, uint32_t arg2)
{

	uint8_t getstr1[32];
	int len = 0;
	len = read(ITP_DEVICE_UART2,getstr1,32);
	
	printf("0x%x-",getstr1[0]);
}
#endif


void homebus_test_uart()
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

	ioctl(ITP_DEVICE_UART2, ITP_IOCTL_REG_UART_DEFER_CB, (void*)UartCallback);

	tHomebusInitData.cpuClock = ithGetRiscCpuClock();
    tHomebusInitData.txdGpio = CFG_GPIO_HOMEBUS_TXD;
    tHomebusInitData.uid[0] = 0x01;
    tHomebusInitData.uid[1] = 0x01;
	ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_INIT_PARAM, &tHomebusInitData);




    tHomebusWriteData.len = 28;
    tHomebusWriteData.pWriteDataBuffer = &pWriteData[0];
	printf("Homebus Write(ret=%d) start :\n", len);

	len = ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_WRITE_DATA, &tHomebusWriteData);
	printf("Homebus Write(ret=%d) end :\n", len);

	


	while(1)
	{

	
			usleep(1000);
	
	}


/*

    tHomebusReadData.len = MAX_DATA_SIZE;//CMD_LEN;
    tHomebusReadData.pReadDataBuffer = pReadData;

	
while(1)
{
	len = ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_READ_DATA, &tHomebusReadData);

	len = ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_READ_DATA, &tHomebusReadData);
	if(len > 0) {
		printf("Homebus Read(%d) :\n", len);
		for(count = 0; count < len; count++) {
			printf("0x%x ", pReadData[count]);
		}
		printf("\r\n");
	}

		usleep(1000);

}
*/
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
	printf("Start Homebus\n");


	tHomebusInitData.cpuClock = ithGetRiscCpuClock();
	tHomebusInitData.txdGpio = CFG_GPIO_HOMEBUS_TXD;
	tHomebusInitData.rxdGpio = CFG_GPIO_HOMEBUS_RXD;		


	ithGpioSetOut(CFG_GPIO_HOMEBUS_TXD);
	ithGpioSetMode(CFG_GPIO_HOMEBUS_TXD, ITH_GPIO_MODE0);
	ithGpioSet(CFG_GPIO_HOMEBUS_TXD);
	ithGpioSetOut(CFG_GPIO_HOMEBUS_RXD);
	ithGpioSetMode(CFG_GPIO_HOMEBUS_RXD, ITH_GPIO_MODE0);
	ithGpioSet(CFG_GPIO_HOMEBUS_RXD);

	tHomebusInitData.uid[0]=0x02;
	tHomebusInitData.uid[1]=0x01;


	ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_INIT_PARAM, &tHomebusInitData);



}

int homebus_senddata(char* buf,unsigned char len)
{
	printf("homebus_senddata %d \n", len);
int ret;
	if(len >64)
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
		printf("0x%x ", buf[count]);
	}
	printf("\r\n");



	ret= ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_WRITE_DATA, &tHomebusWriteData);
	//printf("homebus_senddata end\n");	

	
	ithGpioSetOut(34);
	ithGpioSetMode(34, ITH_GPIO_MODE0);
	ithGpioSet(34);
	return ret;
}

void homebus_recvdata(char* buf,unsigned char* len)
{
unsigned int recv_len=0;
int count=0;
	uint8_t pReadData[MAX_DATA_SIZE] = { 0 };

	HOMEBUS_READ_DATA tHomebusReadData = { 0 };

    tHomebusReadData.len = MAX_DATA_SIZE;//CMD_LEN;
    tHomebusReadData.pReadDataBuffer = pReadData;

	recv_len = ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_READ_DATA, &tHomebusReadData);

if(recv_len>0)
{
	printf("Homebus Read(%d) :\n", recv_len);
	for(count = 0; count < recv_len; count++) {
		printf("0x%x ", pReadData[count]);
	}
	printf("\r\n");
}
	//memcpy(buf,pReadData,recv_len);
	//*len=recv_len;



}

void homebus_recvdata2(char* buf,unsigned char* len)
{
unsigned int recv_len=0;
int count=0;
	uint8_t pReadData[MAX_DATA_SIZE] = { 0 };

	HOMEBUS_READ_DATA tHomebusReadData = { 0 };

    tHomebusReadData.len = MAX_DATA_SIZE;//CMD_LEN;
    tHomebusReadData.pReadDataBuffer = pReadData;

int ret;
uint8_t data_len;
int timeout=10;

while(timeout>0)
{
	ret = ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_READ_DATA, &tHomebusReadData);
	if(recv_len >3)
	{
		data_len =pReadData[2];//protocol length
	}
	else
	{
		recv_len=recv_len+ret;
		timeout--;		
		usleep(1000);
		continue;
	}
	
	recv_len=recv_len+ret;
	if(recv_len>=data_len)
	{
		break;
	}
	else
	{
	    tHomebusReadData.pReadDataBuffer = &pReadData[count];
	}
	usleep(10*1000); //9600

		
}
if(recv_len>0)
{
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
	while(1)
	{
		tx_deal();
		rx_deal();
		init_tx_deal();//10 ms
		system_tx_check();//10 ms
		usleep(5000);
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
    homebus_test_uart();
	//homebus_control();
    homebus_test();

#ifdef CFG_DBG_TRACE
    vTraceStop();
#endif
    return 0;
}
