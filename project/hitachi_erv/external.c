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
//static mqd_t extOutQueue = -1;
static pthread_t extTask;
static volatile bool extQuit;



#ifndef WIN32

#include "alt_cpu/alt_cpu_device.h"
#include "alt_cpu/homebus/homebus.h"

void MCU_logic_control_IR()
{
	unsigned char buff[64]={0};
	int cmd_len=0;

	cmd_len = read(ITP_DEVICE_IR0, buff, 64);
	if(cmd_len>0)
	{
		remote_cmd_buffer_in(buff,cmd_len);//cmd in 
		remote_deal_program();
	}

}


void MCU_logic_control()
{
printf("homebus_logic_control \n");


	int i;

	int counter=10;

	while(1)
	{
	
	   // pthread_mutex_lock(&gThreadMutex);

		system_tx_check();
	
		init_tx_deal();

		if(0==counter){  tx_deal();	counter=10; } // do until 100ms 
		//rx_deal();		
		//if(have_ir)
		
		MCU_logic_control_IR();//follow original IR module logic 


		//pthread_mutex_unlock(&gThreadMutex);

		usleep(10*1000);//10ms 
		counter--;
		
	}
}


void Hlink_init()
{
	homebus_init(); //h link engine
	
	pthread_t readThread;
	pthread_create(&readThread, NULL, MCU_logic_control, NULL);

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
	i = atoi(param);
	Hlink_send_state(i);
	return false;
}



void ExternalInit(void)
{
    struct mq_attr qattr;

    qattr.mq_flags = 0;
    qattr.mq_maxmsg = EXT_MAX_QUEUE_SIZE;
    qattr.mq_msgsize = sizeof(ExternalEvent);

    extInQueue = mq_open("external_in", O_CREAT | O_NONBLOCK, 0644, &qattr);
    assert(extInQueue != -1);

    //extOutQueue = mq_open("external_out", O_CREAT | O_NONBLOCK, 0644, &qattr);
    //assert(extOutQueue != -1);

    extQuit = false;


 //   pthread_create(&extTask, NULL, IR_Probe_Task, NULL);
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

   // return mq_send(extOutQueue, (char*)ev, sizeof(ExternalEvent), 0);
}
