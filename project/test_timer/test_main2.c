#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/itp.h"
#include "ite/ith.h"
#include <pthread.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"

typedef struct {
    uint32_t Valid;
    uint32_t Invalid;
    uint32_t Periodus;      //us
    uint8_t  TimerN;        //Timer
    uint8_t  DutyCycleLevel;//0~100
    uint8_t  GPIOpin;       //pin
}SWPWM;

static bool isGpioHigh = 0;
void _TimerIsr(void* data)
{
    SWPWM *in  = (SWPWM*) data;
	//
    //ithPrintf("in->TimerN = %d in->Periodus = %d in->DutyCycleLevel = %d\n",in->TimerN,in->Periodus,in->DutyCycleLevel);
	if(isGpioHigh)
	{
		ithGpioClear(in->GPIOpin);
        ithTimerSetTimeout(in->TimerN, in->Invalid);
        ithPrintf("swpwm->Valid = %d\n",in->Valid);
	}
	else
	{
		ithGpioSet(in->GPIOpin);
        ithTimerSetTimeout(in->TimerN, in->Valid);
        ithPrintf("swpwm->Valid = %d\n",in->Invalid);
	}
	isGpioHigh = !isGpioHigh;
}

static void _TimerInit(SWPWM *SWpwm)
{
    int INTRIRQ = ITH_INTR_TIMER1 + SWpwm->TimerN;
    //printf("SWpwm->TimerN = %d SWpwm->Periodus = %d(us) SWpwm->DutyCycleLevel = %d\n",SWpwm->TimerN,SWpwm->Periodus,SWpwm->DutyCycleLevel);
	//initial GPIO
	ithGpioSetOut(SWpwm->GPIOpin);
	ithGpioSetMode(SWpwm->GPIOpin, ITH_GPIO_MODE0);
    
    if(SWpwm->Valid == 100){
        ithGpioSet(SWpwm->GPIOpin);
        isGpioHigh = 1;
        return;
    }
    if(SWpwm->Invalid == 0){
        ithGpioClear(SWpwm->GPIOpin);
        isGpioHigh = 0;
        return;
    }
	
	// Reset Timer7
	ithTimerReset(SWpwm->TimerN);

	// Initialize Timer IRQ
	ithIntrDisableIrq(INTRIRQ);
	ithIntrClearIrq(INTRIRQ);
    
	// register Timer Handler to IRQ
	ithIntrRegisterHandlerIrq(INTRIRQ, _TimerIsr, (void*)SWpwm);

	// set Timer IRQ to edge trigger
	ithIntrSetTriggerModeIrq(INTRIRQ, ITH_INTR_EDGE);

	// set Timer IRQ to detect rising edge
	ithIntrSetTriggerLevelIrq(INTRIRQ, ITH_INTR_HIGH_RISING);

	// Enable Timer IRQ
	ithIntrEnableIrq(INTRIRQ);
    
    //set Timeout
    ithTimerSetTimeout(SWpwm->TimerN, SWpwm->Valid);
	//ithTimerCtrlEnable(ITH_TIMER2, ITH_TIMER_UPCOUNT);

	// set Timer periodic
	//ithTimerCtrlEnable(ITH_TIMER2, ITH_TIMER_PERIODIC);
	//ithTimerSetCounter(ITH_TIMER2, 0);
	//ithTimerSetMatch(ITH_TIMER2, configCPU_CLOCK_HZ / configTICK_RATE_HZ);

	ithTimerCtrlEnable(SWpwm->TimerN, ITH_TIMER_EN);
}

static void _TimerTerminate(SWPWM *SWpwm){
    ithTimerCtrlDisable(SWpwm->TimerN, ITH_TIMER_EN);
    ithTimerReset(SWpwm->TimerN);

    ithIntrDisableIrq(ITH_INTR_TIMER2);
    ithIntrClearIrq(ITH_INTR_TIMER2);
    
    ithGpioClear(SWpwm->GPIOpin);
    isGpioHigh = 0;    
}


void* TestFunc_IRQ(void* arg)
{  
    SWPWM *swpwm;
    swpwm->TimerN        = ITH_TIMER1;//Timer set
    swpwm->Periodus      = 1000000;      //Periods set
    swpwm->DutyCycleLevel= 60;        //dutycucle set
    swpwm->GPIOpin       = 34;        //output pin set
    swpwm->Valid         = swpwm->Periodus * swpwm->DutyCycleLevel / 100;
    swpwm->Invalid       = swpwm->Periodus * (100-swpwm->DutyCycleLevel) / 100;
    
	_TimerInit(swpwm);//Init
    
    //_TimerTerminate(swpwm);//Terminate
    
    printf("end IRQ TEST\n");
    
    return NULL;
}
