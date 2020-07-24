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
#include "alt_cpu/patternGen/patternGen.h"

//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//				  Macro Definition
//=============================================================================
PATTERN_GEN_WRITE_CMD_DATA gtPatternData[] =
{
    {PATTERN_GEN0, 6, INIFINITE_SEND_COUNT, {{0, 2000}, {1, 600}, {0, 200}, {1, 1000}, {0, 500}, {1, 10000}}},
    {PATTERN_GEN1, 6, 1, {{0, 2000}, {1, 600}, {0, 200}, {1, 1000}, {0, 500}, {1, 10000}}},
    {PATTERN_GEN2, 6, 2, {{0, 2000}, {1, 600}, {0, 200}, {1, 1000}, {0, 500}, {1, 10000}}},
    {PATTERN_GEN3, 6, 3, {{0, 2000}, {1, 600}, {0, 200}, {1, 1000}, {0, 500}, {1, 10000}}},
};


//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
void _SimpleTest(void)
{
    int altCpuEngineType = ALT_CPU_PATTERN_GEN;
    PATTERN_GEN_INIT_DATA tInitData = { 0 };

    //Load Pattern Generator Engine on ALT CPU
    printf("Load Pattern Generator Engine\n");
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_ALT_CPU_SWITCH_ENG, &altCpuEngineType);

    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT, NULL);


    printf("Init Pattern Generator Parameters\n");
    tInitData.patternGenId = PATTERN_GEN0;
    tInitData.cpuClock = ithGetRiscCpuClock();
    tInitData.patternGenGpio = 86;
    tInitData.defaultGpioValue = GPIO_PULL_HIGH;
    tInitData.timeUnitInUs = 1;
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT_PATTERN_GEN_PARAM, &tInitData);

    printf("Start Pattern Generator Write\n");
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_PATTERN_GEN_WRITE_DATA, &gtPatternData[0]);
}

void _MultiPleDeviceTest(void)
{
    int altCpuEngineType = ALT_CPU_PATTERN_GEN;
    PATTERN_GEN_INIT_DATA tInitData = { 0 };
    PATTERN_GEN_UPDATE_SEND_COUNT_CMD_DATA tUpdateData = { 0 };
    PATTERN_GEN_STOP_CMD_DATA tStopData = { 0 };

    //Load Pattern Generator Engine on ALT CPU
    printf("Load Pattern Generator Engine\n");
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_ALT_CPU_SWITCH_ENG, &altCpuEngineType);

    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT, NULL);


    printf("Init Pattern Generator0 Parameters\n");
    tInitData.patternGenId = PATTERN_GEN0;
    tInitData.cpuClock = ithGetRiscCpuClock();
    tInitData.patternGenGpio = 86;
    tInitData.defaultGpioValue = GPIO_PULL_HIGH;
    tInitData.timeUnitInUs = 1;    
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT_PATTERN_GEN_PARAM, &tInitData);

    printf("Init Pattern Generator1 Parameters\n");
    tInitData.patternGenId = PATTERN_GEN1;
    tInitData.cpuClock = ithGetRiscCpuClock();
    tInitData.patternGenGpio = 87;
    tInitData.defaultGpioValue = GPIO_PULL_LOW;
    //time unit is 10us
    tInitData.timeUnitInUs = 10;    
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT_PATTERN_GEN_PARAM, &tInitData);

    printf("Init Pattern Generator2 Parameters\n");
    tInitData.patternGenId = PATTERN_GEN2;
    tInitData.cpuClock = ithGetRiscCpuClock();
    tInitData.patternGenGpio = 88;
    tInitData.defaultGpioValue = GPIO_KEEP_LAST_VALUE;
    //time unit is 2us
    tInitData.timeUnitInUs = 2;
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT_PATTERN_GEN_PARAM, &tInitData);

    printf("Init Pattern Generator2 Parameters\n");
    tInitData.patternGenId = PATTERN_GEN3;
    tInitData.cpuClock = ithGetRiscCpuClock();
    tInitData.patternGenGpio = 89;
    tInitData.defaultGpioValue = GPIO_INPUT_MODE;
    tInitData.timeUnitInUs = 1;    
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT_PATTERN_GEN_PARAM, &tInitData);


    printf("Start Pattern Generator Write\n");
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_PATTERN_GEN_WRITE_DATA, &gtPatternData[0]);
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_PATTERN_GEN_WRITE_DATA, &gtPatternData[1]);
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_PATTERN_GEN_WRITE_DATA, &gtPatternData[2]);
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_PATTERN_GEN_WRITE_DATA, &gtPatternData[3]);

    usleep(100*1000);

    tUpdateData.patternGenId = PATTERN_GEN3;
    tUpdateData.sendCount = 1;
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_PATTERN_GEN_UPDATE_SEND_COUNT, &tUpdateData);

    usleep(100*1000);
    tStopData.patternGenId = PATTERN_GEN0;
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_PATTERN_GEN_STOP, &tStopData);


    usleep(1000*1000);
    tUpdateData.patternGenId = PATTERN_GEN0;
    tUpdateData.sendCount = INIFINITE_SEND_COUNT;
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_PATTERN_GEN_UPDATE_SEND_COUNT, &tUpdateData);


  
}



//=============================================================================
//                  Public Function Definition
//=============================================================================
void* TestFunc(void* arg)
{
    #ifdef CFG_DBG_TRACE
    uiTraceStart();
    #endif

    itpInit();
    //_SimpleTest();
    _MultiPleDeviceTest();



    while(1);

    #ifdef CFG_DBG_TRACE
    vTraceStop();
    #endif
    return 0;
}
