#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/itp.h"
#include "ite/ith.h"
#include <pthread.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"

#if (CFG_CHIP_FAMILY == 9860)
#define MAX_TIMER   ITH_TIMER9
#else
#define MAX_TIMER   ITH_TIMER8
#endif

static int clk = 0;
static int cnt = 0;

unsigned int PalGetClock(void)
{
    return xTaskGetTickCount();
}

unsigned long PalGetDuration(unsigned int clock)
{
    return (xTaskGetTickCount() - clock) / portTICK_PERIOD_MS;
}

void timer_isr(void* data)
{
    uint32_t timer = (uint32_t)data;

	ithPrintf("\tTimer%d Interrupt occur, clk=%d, IntrState=0x%08X, cnt=%d\n", timer + 1, PalGetDuration(clk), ithTimerGetIntrState(timer), ++cnt);
    //ithPrintf("T%d\n", timer+1);
    //ithPrintf("\tTimer%d isr, cnt=%d\n", timer + 1, ++cnt);
    //printf("\tTimer%d Interrupt occur, clk=%d, IntrState=0x%08X, cnt=%d\n", timer + 1, PalGetDuration(clk), ithTimerGetIntrState(timer), ++cnt);
    clk = PalGetClock();
    ithTimerClearIntr(timer);

}

void* TestFunc_count(void* arg)
{
    /* Basic test*/
    /*
    teset timer count us sleeptime
    up count the counter
    ithTimerGetTime() get time
    count sleeptime=1000000 use time2-time1
    test 10 time
    */
    int i;
    for(i=ITH_TIMER1; i<=MAX_TIMER; i++)
    {
        int status;
        int n = 0;
        unsigned int time1=0;
        unsigned int time2=0;
        int sleeptime=1000000;

        if (i == ITH_TIMER8){
            printf("###system clock Timer%d : we dont test timer%d\n",(i+1),(i+1));
            continue;
        }


        printf("Timer%d test start\n",(i+1));

        ithTimerReset(i);//reset timer
        ithTimerCtrlEnable(i, ITH_TIMER_UPCOUNT);//up count the counter
        ithTimerSetCounter(i, 0x0);//init counter to 0
        ithTimerSetLoad(i,0x0);//set reload

        printf("ithTimerGetCounter = %d\n",ithTimerGetCounter(i));
        ithTimerEnable(i);//enabel time
        while(1){
            time1=ithTimerGetTime(i);
            ithDelay(sleeptime);
            time2=ithTimerGetTime(i);
            printf("sleep interval = %d us ,timercounter = %d\n",time2-time1,ithTimerGetCounter(i));
            if(n==5)
                break;
            else
                n++;
        }
        ithTimerCtrlDisable(i, ITH_TIMER_EN);
        ithTimerReset(i);

        printf("###Timer%d test end\n",(i+1));

    }
    printf("=======end test======\n");
    return ;

}

void* TestFunc_timeout(void* arg)
{
    /* Basic test*/
    int status;
    int i = 0;
    int us = 0;
    int clock = 0;

    for (us=10000; us<=10000000; us*=10)
    {
        // Interrupt test
        printf("\n###### %dms timeout testing ######\n", us/1000);

		for (i = ITH_TIMER1; i <= MAX_TIMER; i++)
        {
            if (/*i == ITH_TIMER1 || i == USEDTIMER || */i == ITH_TIMER8)
                continue;

            ithTimerReset(i);
            ithTimerClearIntr(i);
            ithTimerSetTimeout(i, us);
            ithTimerEnable(i);

            clock = PalGetClock();

            while(1)
            {
				status = ithTimerGetIntrState(i);
                if(status & 0x7<<((i & 7) * 4))
                {
                    printf("[TIMER%d] intr gap time = %d ms status = 0x%08x \n",i+1, PalGetDuration(clock), status);
                    break;
                }
                usleep(200);
            }

            printf("Timer%d leaving\n", i+1);
			ithTimerDisable(i);
            ithTimerReset(i);
            ithTimerClearIntr(i);
        }
    }
    printf("**********end***************\n");
}

void* TestFunc_IRQ(void* arg)
{
    int status;
    int i = 0;
    int us = 0;
    for (us=10000; us<=1000000; us*=10)
    {
        // Interrupt test
        printf("\n###### %dms timeout testing ######\n", us/1000);

        for(i=ITH_TIMER1; i<=MAX_TIMER; i++)
        {
            // timer used in : ITE970
			// ITH_TIMER1: backlight
			// ITH_TIMER2: watchdog
			// ITH_TIMER3: VCD :itp_stats_tascvcd.c
			// ITH_TIMER4: RTC in alpha :itp_rtc_assist.c
			// ITH_TIMER5: backlight : itp_backlight.c
			// ITH_TIMER6: task
            // ITH_TIMER7: power
            // ITH_TIMER8: Operation System
            if (/*i == ITH_TIMER1 || i == USEDTIMER || */i == ITH_TIMER8)
                continue;

            ithTimerReset(i);

            {
                // Initialize Timer IRQ
                ithIntrDisableIrq(ITH_INTR_TIMER1 + i);
                ithIntrClearIrq(ITH_INTR_TIMER1 + i);

                // register Timer Handler to IRQ
                ithIntrRegisterHandlerIrq(ITH_INTR_TIMER1 + i, timer_isr, (void*)i);

                // set Timer IRQ to edge trigger
                ithIntrSetTriggerModeIrq(ITH_INTR_TIMER1 + i, ITH_INTR_EDGE);

                // set Timer IRQ to detect rising edge
                ithIntrSetTriggerLevelIrq(ITH_INTR_TIMER1 + i, ITH_INTR_HIGH_RISING);

                // Enable Timer IRQ
                ithIntrEnableIrq(ITH_INTR_TIMER1 + i);
            }

            ithTimerSetTimeout(i, us);
            clk = PalGetClock();
			ithTimerEnable(i);

            usleep(us*10); // goal: trigger 10 times

            printf("###### Timer%d leaving\n", i+1);
			ithTimerDisable(i);
			ithTimerReset(i);
			{
				ithIntrDisableIrq(ITH_INTR_TIMER1 + i);
				ithIntrClearIrq(ITH_INTR_TIMER1 + i);
			}
        }
    }
    printf("end IRQ TEST\n");

    return NULL;
}

void* TestFunc_PWM_BACKLIGHT(void* arg)
{
    /*
    default set
    backlight gpio pin : 49
    GPIO mode : 3
    ITH_PWM : ITH_PWM3
    CFG_BACKLIGHT_DUTY_CYCLES = {95, 92, 88, 82, 75, 65, 55, 40, 25, 5}

    // PWM used in : ITE9850
    1.ITH_PWM1~ITH_PWM6 with 6 set HW PWM set   (recommend)
    2.CFG_ALT_CPU_ENABLE : ALT-CPU can support 4 SW PWM set (recommend)
    3.test_main2.c example (not recommend)
    */
    unsigned int blDutyCycleTable[] = { CFG_BACKLIGHT_DUTY_CYCLES };
	int gpio_pin = CFG_GPIO_BACKLIGHT_PWM;//
    int PWM = CFG_GPIO_PWM_NUMBER-1;
    int mode = CFG_GPIO_PWM_MODE;
    int count;
    printf("gpio_pin:%d PWM:%d mode:%d\n",gpio_pin,PWM,mode);

    itpInit();
    sleep(1);//wait time

    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_RESET, NULL);

    for(count = 0 ; count <= 100 ; count++){
        //ithPwmSetDutyCycle(PWM,blDutyCycleTable[count%10]);
		ithPwmSetDutyCycle(PWM,90);
        printf("ITH_PWM%d enable ,blDutyCycleTable = %d .gpio=%d\n",PWM+1,blDutyCycleTable[count%10],gpio_pin);
        ithPwmEnable(PWM, gpio_pin, mode);
        sleep(1);
        printf("ITH_PWM%d disable\n",PWM+1);
        ithPwmDisable(PWM,gpio_pin);
    }
    return NULL;
}

void* TestFunc_PWM_LED(void* arg)
{
    /*
    control LED light
    SET ,gpio_pin,PWM,mode
    */
    int gpio_pin = 18;
    int PWM = ITH_PWM2;
    int mode = 2;
    int count=9;

    ithPwmInit(PWM,1000,100);
    ithPwmReset(PWM,gpio_pin,mode);

    while(1)
	{
        ithPwmSetDutyCycle(PWM,count);
        usleep(500000);
        printf("ITH_PWM%d enable ,DutyCycle = %d\n",PWM+1,count);
        ithPwmEnable(PWM, gpio_pin, mode);
        //usleep(100000);
        //printf("ITH_PWM%d disable\n",PWM+1);
        //ithPwmDisable(PWM,gpio_pin);
        count+=10;
        if(count>100) count=9;
    }
    return NULL;
}

void* TestFunc_PWM_All_Pin(void* arg)
{
	/*
	test all pin with pwm1~8
	*/
#define START_PIN 0
#define END_PIN 10
	int gpio_pin;
	int PWM = ITH_PWM1;
	int mode = 0;
	int count = 9;
	int skipPin[] = { 0, 1, 2, 3 };
	int skipIndex = 0;
//#define SINGLE_PIN_TEST
#ifdef SINGLE_PIN_TEST
	gpio_pin = 42;
	while (1)
	{
		printf("test Pin: %d\n", gpio_pin);
		ithPwmInit(ITH_PWM7, 1000, 100);
		ithPwmReset(ITH_PWM7, gpio_pin, mode);
		while (1)
		{
			ithPwmSetDutyCycle(ITH_PWM7, count);
			usleep(500000);
			printf("ITH_PWM%d enable ,DutyCycle = %d\n", ITH_PWM7 + 1, count);
			ithPwmEnable(ITH_PWM7, gpio_pin, mode);
			count += 20;
			if (count > 100)
			{
				count = 9;
				break;
			}
		}
	}
#else
	for (gpio_pin = START_PIN; gpio_pin <= END_PIN; gpio_pin++)
	{
		while (gpio_pin == skipPin[skipIndex])
		{
			gpio_pin++;
			skipIndex++;
		}
		printf("test Pin: %d\n", gpio_pin);
		for (PWM = ITH_PWM1; PWM <= ITH_PWM7; PWM++)
		{
			ithPwmInit(PWM, 1000, 100);
			ithPwmReset(PWM, gpio_pin, mode);
			while (1)
			{
				ithPwmSetDutyCycle(PWM, count);
				usleep(500000);
				printf("ITH_PWM%d enable ,DutyCycle = %d\n", PWM + 1, count);
				ithPwmEnable(PWM, gpio_pin, mode);
				count += 20;
				if (count > 100)
				{
					count = 9;
					break;
				}
			}
			printf("ITH_PWM%d disable\n", PWM + 1);
			ithPwmDisable(PWM, gpio_pin);
		}
	}
#endif
	return NULL;
}
