#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "ite/itp.h"
#include "alt_cpu/rslSlave/rslSlave.h"

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
void* Rsl0SlaveTest(void* arg)
{
    int testRun = 0;
    int bInit = 0;
    int altCpuEngineType = ALT_CPU_RSL_SLAVE;
    int bReadRawData = 1;
    int bWriteRawData = 1;
    int bPrintPrintBits = 1;
    int bProcessWrite = 1;
    int writeCounter = 3;
    int totalBytes = 0;
    
    while (1)
    {
        RSL_SLAVE_INIT_DATA tInitData = { 0 };
        RSL_SLAVE_WRITE_RAW_DATA tWriteData = { 0 };
        RSL_SLAVE_READ_RAW_DATA tReadData = { 0 };

        int i = 0;
        int j = 0;
        int k = 0;

        //Init RSL0 Slave Parameter
        if (!bInit)
        {
            tInitData.slaveId = RSL_SLAVE0;
            tInitData.clkGpio = 87;
            tInitData.cpuClock = ithGetRiscCpuClock();
            tInitData.dataGpio = 89;
            tInitData.dataWriteGpio = 85;

            //set delay read/write timing
            tInitData.firstReadDelayUs = 250;
            tInitData.readPeriod = 100;
            tInitData.firstWriteDelayUs = 250;
            tInitData.writePeriod = 100;

            printf("Process Slave0 Init Cmd\n");
            ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_ALT_CPU_INIT_PARAMETER, &tInitData);
            bInit = 1;
        }

        if (bProcessWrite)
        {
            printf("Process Slave0 Write Raw Data\n");
            tWriteData.slaveId = RSL_SLAVE0;
            tWriteData.writeRepeatCount = 3;
            for (i = 0; i < 32; i++)
            {
                tWriteData.pWriteBuffer[i] = tWriteData.pWriteBuffer[i + 32] = i;
                
            }
            ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_ALT_CPU_WRITE_RAW_DATA, &tWriteData);
            bProcessWrite = 0;
        }

        tReadData.slaveId = RSL_SLAVE0;
        tReadData.bSuccess = 0;
        totalBytes = ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_ALT_CPU_READ_RAW_DATA, &tReadData);

        if (totalBytes)
        {
            printf("---------Read Slave0 Raw Data: %d----------------\n", testRun++);
            for (i = 0; i < totalBytes; i +=8)
            {
                for (j = 0; j < 8; j++)
                {
                    uint8_t dataByte = tReadData.pReadBuffer[i + j];
                    if (bPrintPrintBits)
                    {
                        for (k = 0; k < 8; k++)
                        {
                            printf("%d", ((dataByte >> k) & 0x1));
                        }
                        printf(", ");
                    }
                    else
                    {
                        printf("%02X, ", dataByte);
                    }
                }
                printf("\n");
            }
            printf("----------------------------------\n");
            testRun++;
        }
        usleep(200*1000);
    }
}

void* Rsl1SlaveTest(void* arg)
{
    int testRun = 0;
    int bInit = 0;
    int altCpuEngineType = ALT_CPU_RSL_SLAVE;
    int bReadRawData = 1;
    int bWriteRawData = 1;
    int bPrintPrintBits = 1;
    int bProcessWrite = 1;
    int writeCounter = 3;
    int totalBytes = 0;
    
    while (1)
    {
        RSL_SLAVE_INIT_DATA tInitData = { 0 };
        RSL_SLAVE_WRITE_RAW_DATA tWriteData = { 0 };
        RSL_SLAVE_READ_RAW_DATA tReadData = { 0 };

        int i = 0;
        int j = 0;
        int k = 0;

        //Init RSL0 Slave Parameter
        if (!bInit)
        {
            tInitData.slaveId = RSL_SLAVE1;
            tInitData.clkGpio = 86;
            tInitData.cpuClock = ithGetRiscCpuClock();
            tInitData.dataGpio = 88;
            tInitData.dataWriteGpio = 88;

            //set delay read/write timing
            tInitData.firstReadDelayUs = 250;
            tInitData.readPeriod = 100;
            tInitData.firstWriteDelayUs = 250;
            tInitData.writePeriod = 100;

            printf("Process Slave1 Init Cmd\n");
            ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_ALT_CPU_INIT_PARAMETER, &tInitData);
            bInit = 1;
        }

        if (bProcessWrite)
        {
            printf("Process Slave1 Write Raw Data\n");
            tWriteData.slaveId = RSL_SLAVE1;
            tWriteData.writeRepeatCount = RSL_SLAVE_INFINITE_WRITE;

            for (i = 0; i < 32; i++)
            {
                tWriteData.pWriteBuffer[i] = tWriteData.pWriteBuffer[i + 32] = i;
                
            }
            ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_ALT_CPU_WRITE_RAW_DATA, &tWriteData);

            bProcessWrite = 0;
        }

        tReadData.slaveId = RSL_SLAVE1;
        tReadData.bSuccess = 0;
        totalBytes = ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_ALT_CPU_READ_RAW_DATA, &tReadData);

        if (totalBytes)
        {
            printf("---------Read Slave1 Raw Data: %d----------------\n", testRun);
            for (i = 0; i < totalBytes; i +=8)
            {
                for (j = 0; j < 8; j++)
                {
                    uint8_t dataByte = tReadData.pReadBuffer[i + j];
                    if (bPrintPrintBits)
                    {
                        for (k = 0; k < 8; k++)
                        {
                            printf("%d", ((dataByte >> k) & 0x1));
                        }
                        printf(", ");
                    }
                    else
                    {
                        printf("%02X, ", dataByte);
                    }
                }
                printf("\n");
            }
            printf("----------------------------------\n");
            testRun++;
        }
        usleep(200*1000);
    }

}

void* TestFunc(void* arg)
{
    int altCpuEngineType = ALT_CPU_RSL_SLAVE;
    pthread_t task_rsl_0;
    pthread_t task_rsl_1;
    pthread_attr_t attr;

#ifdef CFG_DBG_TRACE
    uiTraceStart();
#endif

    itpInit();

    //Load RSL Slave Engine on ALT CPU
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_ALT_CPU_SWITCH_ENG, &altCpuEngineType);
    ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT, NULL);
    pthread_attr_init(&attr);
    pthread_create(&task_rsl_0, &attr, Rsl0SlaveTest, NULL);
    
    pthread_create(&task_rsl_1, &attr, Rsl1SlaveTest, NULL);

    pthread_join(task_rsl_0, NULL);
    pthread_join(task_rsl_1, NULL);

#ifdef CFG_DBG_TRACE
    vTraceStop();
#endif
    return 0;
}
