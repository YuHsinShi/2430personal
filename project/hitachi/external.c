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



void Hlink_init()
{
/*
	    int altCpuEngineType = ALT_CPU_HOMEBUS;

	 //Load Engine on ALT CPU
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_ALT_CPU_SWITCH_ENG, &altCpuEngineType);
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT, NULL);
//    homebus_test();	    
	pthread_create(&extTask, NULL, homebus_control, NULL);
	
*/


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
