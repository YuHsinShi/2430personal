#include <sys/ioctl.h>
#include <assert.h>
#include <mqueue.h>
#include <string.h>
#include <unistd.h>
#include "ite/itp.h"
#include "project.h"
#include "scene.h"

#define EXT_MAX_QUEUE_SIZE      8
#define MAX_OUTDATA_SIZE        64


#ifdef _WIN32
#define WIN32_COM           5
#endif

static mqd_t extInQueue = -1;
static mqd_t extOutQueue = -1;
static pthread_t extTask;
static volatile bool extQuit;



#ifndef WIN32

#include "alt_cpu/alt_cpu_device.h"
#include "alt_cpu/homebus/homebus.h"

#define MAX_DATA_SIZE 		128

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

void homebus_senddata(char* buf,unsigned char len)
{
	printf("homebus_senddata %d \n", len);

	if(len >64)
	{
		printf("homebus_senddata %d \n", len);
		return;
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



	ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_WRITE_DATA, &tHomebusWriteData);
	//printf("homebus_senddata end\n");	

	
	ithGpioSetOut(34);
	ithGpioSetMode(34, ITH_GPIO_MODE0);
	ithGpioSet(34);
}

void homebus_recvdata(char* buf,unsigned char* len)
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


void Hlink_init()
{
	    int altCpuEngineType = ALT_CPU_HOMEBUS;

	 //Load Engine on ALT CPU
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_ALT_CPU_SWITCH_ENG, &altCpuEngineType);
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT, NULL);
//    homebus_test();	    
	pthread_create(&extTask, NULL, homebus_control, NULL);

}
#endif





void Hlink_send_state(int param)
{
#ifdef WIN32
	printf("Hlink_send_state %d \n", param);
#else 
	switch (param)
	{
	case HLINK_POWER_ON:
		homebus_api_power_on();
		break;
	case HLINK_POWER_OFF:
		homebus_api_power_off();
		break;

	default:
		printf("(ERROR) Hlink_send_state id = %d \n", param);
		break;

	}
#endif
	return;
}

bool Hlink_send(ITUWidget* widget, char* param)
{
	int i;
	i = atoi(param[0]);
	Hlink_send_state(i);
	return false;
}


static void* ExternalTask(void* arg)
{
    while (!extQuit)
    {

        usleep(10000);
    }
    mq_close(extInQueue);
	mq_close(extOutQueue);
    extInQueue = -1;
	extOutQueue = -1;

    return NULL;
}

void ExternalInit(void)
{
    struct mq_attr qattr;

    qattr.mq_flags = 0;
    qattr.mq_maxmsg = EXT_MAX_QUEUE_SIZE;
    qattr.mq_msgsize = sizeof(ExternalEvent);

    extInQueue = mq_open("external_in", O_CREAT | O_NONBLOCK, 0644, &qattr);
    assert(extInQueue != -1);

    extOutQueue = mq_open("external_out", O_CREAT | O_NONBLOCK, 0644, &qattr);
    assert(extOutQueue != -1);

    extQuit = false;


    pthread_create(&extTask, NULL, ExternalTask, NULL);
}

void ExternalExit(void)
{
    extQuit = true;
    pthread_join(extTask, NULL);
}

int ExternalReceive(ExternalEvent* ev)
{
    assert(ev);

    if (extQuit)
        return 0;

    if (mq_receive(extInQueue, (char*)ev, sizeof(ExternalEvent), 0) > 0)
        return 1;

    return 0;
}

int ExternalSend(ExternalEvent* ev)
{
    assert(ev);

    if (extQuit)
        return -1;

    return mq_send(extOutQueue, (char*)ev, sizeof(ExternalEvent), 0);
}
