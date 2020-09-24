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
	uint8_t pWriteData[MAX_DATA_SIZE] = { 0x01, 0x01, 0x01};

	uint8_t pReadData[MAX_DATA_SIZE] = { 0 };
	int len = 0, count = 0;

    printf("Start Homebus\n");
    
    tHomebusInitData.cpuClock = ithGetRiscCpuClock();
	tHomebusInitData.txdGpio = CFG_GPIO_HOMEBUS_TXD;
    tHomebusInitData.rxdGpio = CFG_GPIO_HOMEBUS_RXD;
    tHomebusInitData.parity  = NONE;
    
	ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_INIT_PARAM, &tHomebusInitData);
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
    homebus_test();

#ifdef CFG_DBG_TRACE
    vTraceStop();
#endif
    return 0;
}
