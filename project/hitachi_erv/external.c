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


#if 1 //wifi module
#include "openrtos/FreeRTOS.h"

static TickType_t start_tick_uart_wifi;
unsigned char outbuff[256] ={0};
unsigned char outlen =0;
int need_to_send_frame =0;

static void start_timeout_tick(void)
{
    /* Set first ticks value */
    start_tick_uart_wifi = xTaskGetTickCount();
}

static unsigned int get_elapsed_timeout_msec()
{
    TickType_t tick = xTaskGetTickCount();
    if (tick >= start_tick_uart_wifi)
        return ((tick - start_tick_uart_wifi) / portTICK_PERIOD_MS);
    else
        return ((0xFFFFFFFF - start_tick_uart_wifi + tick) / portTICK_PERIOD_MS);

}


int uart_module_framesend(unsigned char* buf,unsigned char frame_len)
{
	//outlen=;
if(0==need_to_send_frame)
{	
	memcpy(outbuff,buf,frame_len);
	need_to_send_frame=1;
	return 1;

}else
	return 0;
	
}


void uart_wifi_module_check()
{
		ithPrintf("uart_wifi_module_check \n");
int i;
		unsigned char tmp[128] ={0};
		unsigned char buff[256] ={0};

		int len;
		int pos=0;;
		WifiModulePowerOn();
		wifi_module_logic_ini();

		while(1)
		{
			len = read(ITP_DEVICE_UART3, tmp, 128);
			if(len>0)
			{	
				memcpy(buff+pos,tmp,len);
				pos+=len;
				start_timeout_tick();				

				while(1) //recv uart until timeout
				{
					len = read(ITP_DEVICE_UART3, tmp, 128);
					if(len>0)
					{
						memcpy(buff+pos,tmp,len);
						pos+=len;
						start_timeout_tick();			
					}

					if(get_elapsed_timeout_msec()>5 ) //frame end 
					{
						wifi_rx_frame_in(buff,len);
						len=0;
						pos=0;
						
						break;
					}
					
					usleep(2000);
				}	

			}
		
			//check cmd queue if something need to be send
			if(need_to_send_frame)
			{
				write(ITP_DEVICE_UART3, outbuff,outlen);
				need_to_send_frame=0;

			}
			
			usleep(50*1000);
		}

}



#endif 



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

void MCU_logic_control_hlink()
{
	static int counter=10;

	system_tx_check();
	init_tx_deal();
	
	if(0==counter){  tx_deal(); counter=10; } // do until 100ms 

	counter--;

}

MCU_logic_control_wifi()
{

	wifi_rx_data();
	wifi_tx_data();


}


void MCU_logic_control()
{
printf("homebus_logic_control \n");


	int i;


	while(1)
	{
	
	   // pthread_mutex_lock(&gThreadMutex);
		//

		//rx_deal();		
		//if(have_ir)
		MCU_logic_control_hlink();
	  	//MCU_logic_control_wifi();
		MCU_logic_control_IR();//follow original IR module logic 


		//pthread_mutex_unlock(&gThreadMutex);

		usleep(10*1000);//10ms 
		
	}
}


void Hlink_init()
{
	homebus_init(); //h link engine
	
	pthread_t wifi_tid;
	pthread_create(&wifi_tid, NULL, uart_wifi_module_check, NULL);

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







