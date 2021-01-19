#include <sys/ioctl.h>
#include <unistd.h>



#define USB_MODE_MSC 0
#define USB_MODE_ACM 1
static int mode=USB_MODE_MSC;


void set_mode_acm()
{
	printf("###SET ACM MODE ###\n");

	mode = USB_MODE_ACM;
}

int is_in_acm_mode()
{


	if(USB_MODE_ACM == mode)
		return 1;
	else	
		return 0;
}

int is_in_msc_mode()
{
	if(	is_in_acm_mode())
		return 0;
	else
		return 1;
}


#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "openrtos/FreeRTOS.h"
#include "openrtos/queue.h"
#include "uart/uart.h"


#define ADD_CAPTURE_TO_DEB_LOG 	1

#if ADD_CAPTURE_TO_DEB_LOG
#define log_add_print ithPrintf
#else 
#define log_add_print 

#endif

#define TEST_PORT       ITP_DEVICE_UART0
#define TEST_ITH_PORT	ITH_UART0
#define TEST_DEVICE     itpDeviceUart0
#define TEST_BAUDRATE   CFG_UART0_BAUDRATE
#define TEST_GPIO_RX    CFG_GPIO_UART0_RX
#define TEST_GPIO_TX    CFG_GPIO_UART0_TX
static sem_t UartSemFifo;
#define UartCommandLen 9


void timer_isr_uart(void* data)
{
    uint32_t timer = (uint32_t)data;
	//printf("timer_isr_uart\n");

    ithTimerClearIntr(timer);

	sem_post(&UartSemFifo);
}


static void UartIntrHandler(unsigned int pin, void *arg)
{
	ITHTimer timer = (ITHTimer)arg;
	//ithPrintf("timer=%d\n", timer);
	ithGpioClearIntr(pin);
	ithTimerCtrlDisable(timer, ITH_TIMER_EN);	
	ithTimerSetTimeout(timer, 1000);//us
	ithTimerCtrlEnable(timer, ITH_TIMER_EN);	
}

void InitTimer(ITHTimer timer, ITHIntr intr)
{
	ithTimerReset(timer);
	
	// Initialize Timer IRQ
	ithIntrDisableIrq(intr);
	ithIntrClearIrq(intr);

	// register Timer Handler to IRQ
	ithIntrRegisterHandlerIrq(intr, timer_isr_uart, (void*)timer);

	// set Timer IRQ to edge trigger
	ithIntrSetTriggerModeIrq(intr, ITH_INTR_EDGE);

	// set Timer IRQ to detect rising edge
	ithIntrSetTriggerLevelIrq(intr, ITH_INTR_HIGH_RISING);

	// Enable Timer IRQ
	ithIntrEnableIrq(intr);
}
void InitUartIntr(ITHUartPort port)
{
	ithEnterCritical();
#if 1
    ithGpioClearIntr(TEST_GPIO_RX);
    ithGpioSetIn(TEST_GPIO_RX);
    ithGpioRegisterIntrHandler((unsigned int)TEST_GPIO_RX, UartIntrHandler, (void*)ITH_TIMER5);
	
    ithGpioCtrlDisable(TEST_GPIO_RX, ITH_GPIO_INTR_LEVELTRIGGER);   /* use edge trigger mode */
    ithGpioCtrlEnable(TEST_GPIO_RX, ITH_GPIO_INTR_BOTHEDGE); /* both edge */    
    ithIntrEnableIrq(ITH_INTR_GPIO);
    ithGpioEnableIntr(TEST_GPIO_RX);
#else
	//printf("port = 0x%x\n", port);
	ithIntrDisableIrq(ITH_INTR_UART2);
    ithUartClearIntr(ITH_UART2);
    ithIntrClearIrq(ITH_INTR_UART2);

    ithIntrSetTriggerModeIrq(ITH_INTR_UART2, ITH_INTR_LEVEL);
    ithIntrRegisterHandlerIrq(ITH_INTR_UART2, UartIntrHandler2, (void *)ITH_UART2);
    ithUartEnableIntr(ITH_UART2, ITH_UART_RX_READY);

    /* Enable the interrupts. */
    ithIntrEnableIrq(ITH_INTR_UART2);
#endif
    ithExitCritical();
}

void* target_log_capture(void* arg)
{
usleep(100*1000);
	ithPrintf("\n###target_log_capture start!\n");

	int len = 0, count = 0;
	char getstr[256] = "", sendstr[256] = "";

	UART_OBJ *pUartInfo	= (UART_OBJ*)malloc(sizeof(UART_OBJ));
	pUartInfo->port		= TEST_ITH_PORT;
	pUartInfo->parity	= 0;
	pUartInfo->txPin	= TEST_GPIO_TX;
	pUartInfo->rxPin	= TEST_GPIO_RX;
	pUartInfo->baud		= TEST_BAUDRATE;
	pUartInfo->timeout	= 0;
	pUartInfo->mode		= UART_FIFO_MODE;
	pUartInfo->forDbg	= false;

	itpRegisterDevice(TEST_PORT, &TEST_DEVICE);
	ioctl(TEST_PORT, ITP_IOCTL_INIT, (void*)pUartInfo);

	InitTimer(ITH_TIMER5, ITH_INTR_TIMER5);
	InitUartIntr(TEST_PORT);


	sem_init(&UartSemFifo, 0, 0);

	while (1)
	{
		sem_wait(&UartSemFifo);
		// 9860 FIFO buffer maximum depth : 128
		len = read(TEST_PORT, getstr , 128);
		if(len>0)
		{
		  getstr[len]=0;	  
		  log_add_print("<\n%s",  getstr);
		  while(len)
		  {
		 	 len = read(TEST_PORT, getstr , 128);
			 		  getstr[len]=0;
			  log_add_print("%s",  getstr);
		  }
		  
		log_add_print(">\n");
		}
	}
}



int target_log_start()
{

	static pthread_t target_log;

	pthread_create(&target_log, NULL, target_log_capture, NULL);

return target_log;
}

int SDL_main(int argc, char *argv[])
{


if(	is_in_acm_mode())
{
	printf("###ENTER ACM MODE ###\n");

	cliInit();
}
else
{
	printf("###ENTER MSC MODE ###\n");
}


usleep(5000*1000);

target_auto_detect();
usleep(1000);

burn_led_congtrol();

//usleep(1000);
//target_log_start();
//usleep(1000);
//target_script_load();

int burn_task;
	while(1)
	{
		if(1== get_key_pressed())
		{
			burn_task=burn_process_start();
			printf("burn_task =%d \n",burn_task);
			pthread_join(burn_task,NULL);
			printf("burn_task END ===\n");
		}
		usleep(50*1000);
	}
}

