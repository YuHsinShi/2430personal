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
#include "alt_cpu/rslMaster/rslMaster.h"

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
    int bSent = 0;
    int altCpuEngineType = ALT_CPU_RSL_MASTER;

    #ifdef CFG_DBG_TRACE
    uiTraceStart();
    #endif

    itpInit();

    //Load RSL Master Engine on ALT CPU
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_ALT_CPU_SWITCH_ENG, &altCpuEngineType);

    while (1)
    {
        RSL_MASTER_INIT_DATA tInitData = { 0 };
        uint8_t pWriteData[40] = { 0 };
        uint8_t pReadData[40] = { 0 };
        int j = 0;
        int i = 0;

        if (!bInit)
        {
            tInitData.clkGpio = CFG_GPIO_RSL_MASTER_CLK;
            tInitData.cpuClock = ithGetRiscCpuClock();
            tInitData.dataGpio = CFG_GPIO_RSL_MASTER_DATA;
            printf("Process Init Cmd\n");
            ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT, &tInitData);

            bInit = 1;
        }

        if (bSent == 0)
        {
            printf("Process Write Cmd\n");
            memset(&pWriteData[0], 0x55, 10);
            memset(&pWriteData[8], 0x02, 10);
            memset(&pWriteData[16], 0x03, 10);
            memset(&pWriteData[24], 0x04, 10);
            memset(&pWriteData[32], 0x05, 8);
            pWriteData[0] = 0x1;

            write(ITP_DEVICE_ALT_CPU, pWriteData, 40);
            printf("Process Send Data out\n");
            ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_ALT_CPU_SEND_DATA, NULL);
            bSent = 1;
        }

        printf("Process Read Cmd\n");
        memset(pReadData, 0x0, 40);
        read(ITP_DEVICE_ALT_CPU, pReadData, 40);
        printf("---------Read Data----------------\n");
        for (i = 0; i < 40; i += 8)
        {
            for (j = 0; j < 8; j++)
            {
                printf("%02X, ", pReadData[i + j]);
            }
            printf("\n");
        }
        printf("----------------------------------\n");

        testRun++;
        usleep(1 * 1000 * 1000);
    }

    #ifdef CFG_DBG_TRACE
    vTraceStop();
    #endif
    return 0;
}
