#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "ite/itp.h"
#include "alt_cpu/alt_cpu_device.h"
#include "alt_cpu/swPwm/swPwm.h"

//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//				  Macro Definition
//=============================================================================

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
void* TestFunc(void* arg)
{
    int testRun = 0;
    int bInit = 0;
    int altCpuEngineType = ALT_CPU_SW_PWM;
    SW_PWM_INIT_DATA tPwmInitData = { 0 };
    SW_PWM_UPDATE_DUTY_CYCLE_DATA tDutyCycle = { 0 };

    #ifdef CFG_DBG_TRACE
    uiTraceStart();
    #endif

    itpInit();

    //Load SW PWM Engine on ALT CPU
    printf("Load SW PWM Engine\n");
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_ALT_CPU_SWITCH_ENG, &altCpuEngineType);

    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT, NULL);

#ifdef CFG_SW_PWM0_ENABLE
    printf("PWM0 -  Frequency: %u Hz, Duty Cycle: %u, GPIO: %u\n", CFG_SW_PWM0_CLOCK, CFG_SW_PWM0_DUTY_CYCLE, CFG_GPIO_SW_PWM0);
    tPwmInitData.pwmId = SW_PWM0;
    tPwmInitData.cpuClock = ithGetRiscCpuClock();
    tPwmInitData.pwmClk = CFG_SW_PWM0_CLOCK;
    tPwmInitData.pwmDutyCycle = CFG_SW_PWM0_DUTY_CYCLE;
    tPwmInitData.pwmGpio = CFG_GPIO_SW_PWM0;
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT_PWM_PARAM, &tPwmInitData);        
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_SW_PWM_RUN, &tPwmInitData.pwmId);
#endif


#ifdef CFG_SW_PWM1_ENABLE
    printf("PWM1 -  Frequency: %u Hz, Duty Cycle: %u, GPIO: %u\n", CFG_SW_PWM1_CLOCK, CFG_SW_PWM1_DUTY_CYCLE, CFG_GPIO_SW_PWM1);
    tPwmInitData.pwmId = SW_PWM1;
    tPwmInitData.cpuClock = ithGetRiscCpuClock();
    tPwmInitData.pwmClk = CFG_SW_PWM1_CLOCK;
    tPwmInitData.pwmDutyCycle = CFG_SW_PWM1_DUTY_CYCLE;
    tPwmInitData.pwmGpio = CFG_GPIO_SW_PWM1;
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT_PWM_PARAM, &tPwmInitData);
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_SW_PWM_RUN, &tPwmInitData.pwmId);
#endif

#ifdef CFG_SW_PWM2_ENABLE
    printf("PWM2 -  Frequency: %u Hz, Duty Cycle: %u, GPIO: %u\n", CFG_SW_PWM2_CLOCK, CFG_SW_PWM2_DUTY_CYCLE, CFG_GPIO_SW_PWM2);
    tPwmInitData.pwmId = SW_PWM2;
    tPwmInitData.cpuClock = ithGetRiscCpuClock();
    tPwmInitData.pwmClk = CFG_SW_PWM2_CLOCK;
    tPwmInitData.pwmDutyCycle = CFG_SW_PWM2_DUTY_CYCLE;
    tPwmInitData.pwmGpio = CFG_GPIO_SW_PWM2;
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT_PWM_PARAM, &tPwmInitData);
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_SW_PWM_RUN, &tPwmInitData.pwmId);
#endif

#ifdef CFG_SW_PWM3_ENABLE
    printf("PWM3 -  Frequency: %u Hz, Duty Cycle: %u, GPIO: %u\n", CFG_SW_PWM3_CLOCK, CFG_SW_PWM3_DUTY_CYCLE, CFG_GPIO_SW_PWM3);
    tPwmInitData.pwmId = SW_PWM3;
    tPwmInitData.cpuClock = ithGetRiscCpuClock();
    tPwmInitData.pwmClk = CFG_SW_PWM3_CLOCK;
    tPwmInitData.pwmDutyCycle = CFG_SW_PWM3_DUTY_CYCLE;
    tPwmInitData.pwmGpio = CFG_GPIO_SW_PWM3;
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT_PWM_PARAM, &tPwmInitData);
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_SW_PWM_RUN, &tPwmInitData.pwmId);
#endif

    do 
    {
        usleep(10 * 1000 * 1000);
        printf("Test Run %u\n", ++testRun);
        printf("---------------------------------------------------\n");
        //Test Update duty
        printf("Chang Duty Cycle SW_PWM0, SW_PWM1\n");
#ifdef CFG_SW_PWM0_ENABLE
        tDutyCycle.pwmId = SW_PWM0;
        tDutyCycle.pwmDutyCycle = 20;
        ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_SW_PWM_UPDATE_DUTY_CYCLE, &tDutyCycle);
#endif

#ifdef CFG_SW_PWM1_ENABLE
        tDutyCycle.pwmId = SW_PWM1;
        tDutyCycle.pwmDutyCycle = 80;
        ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_SW_PWM_UPDATE_DUTY_CYCLE, &tDutyCycle);
#endif

    usleep(5 * 1000 * 1000);

    printf("Chang Duty Cycle SW_PWM2 to 0, SW_PWM3 to 100\n");
    //Test Update duty cycle 0, 100
#ifdef CFG_SW_PWM2_ENABLE
        tDutyCycle.pwmId = SW_PWM2;
        tDutyCycle.pwmDutyCycle = 0;
        ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_SW_PWM_UPDATE_DUTY_CYCLE, &tDutyCycle);
#endif
    
#ifdef CFG_SW_PWM3_ENABLE
        tDutyCycle.pwmId = SW_PWM3;
        tDutyCycle.pwmDutyCycle = 100;
        ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_SW_PWM_UPDATE_DUTY_CYCLE, &tDutyCycle);
#endif

    usleep(10 * 1000 * 1000);

        printf("Chang Duty Cycle SW_PWM0, SW_PWM1, SW_PWM2, SW_PWM3 to original setting\n");
        //Recover original duty cycle setup.
#ifdef CFG_SW_PWM0_ENABLE
        tDutyCycle.pwmId = SW_PWM0;
        tDutyCycle.pwmDutyCycle = CFG_SW_PWM0_DUTY_CYCLE;
        ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_SW_PWM_UPDATE_DUTY_CYCLE, &tDutyCycle);
#endif

#ifdef CFG_SW_PWM1_ENABLE
        tDutyCycle.pwmId = SW_PWM1;
        tDutyCycle.pwmDutyCycle = CFG_SW_PWM1_DUTY_CYCLE;
        ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_SW_PWM_UPDATE_DUTY_CYCLE, &tDutyCycle);
#endif

#ifdef CFG_SW_PWM2_ENABLE
        tDutyCycle.pwmId = SW_PWM2;
        tDutyCycle.pwmDutyCycle = CFG_SW_PWM2_DUTY_CYCLE;
        ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_SW_PWM_UPDATE_DUTY_CYCLE, &tDutyCycle);
#endif
    
#ifdef CFG_SW_PWM3_ENABLE
        tDutyCycle.pwmId = SW_PWM3;
        tDutyCycle.pwmDutyCycle = CFG_SW_PWM3_DUTY_CYCLE;
        ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_SW_PWM_UPDATE_DUTY_CYCLE, &tDutyCycle);
#endif

        printf("---------------------------------------------------\n");
    } while(1);

    #ifdef CFG_DBG_TRACE
    vTraceStop();
    #endif
    return 0;
}
