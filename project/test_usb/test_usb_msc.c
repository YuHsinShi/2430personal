#include <pthread.h>
#include <stdio.h>
#include <malloc.h>
#include <sys/ioctl.h>
#include <string.h>
#include "ite/itp.h"


#define LOOP_NUM    8*2
#define BUF_SIZE    128*1024

static uint8_t*  pattern[16];
static uint8_t*  readBuf[16];
static uint8_t  filePath[16][256] = {0};

#define err()   do { goto end; }while(0)
int ReadWriteTest(int idx)
{
    FILE* file = NULL;
    int i, res;
    uint32_t diff;

    printf("\n\nT%d: start read/write test............\n", idx);

    pattern[idx] = malloc(BUF_SIZE);
    readBuf[idx] = malloc(BUF_SIZE);

    for(i=0; i<BUF_SIZE; i++) /** fill pattern */
        pattern[idx][i] = (i+idx) % 0x100;

    /***************/
    /* Write       */
    /***************/
    file = fopen(filePath[idx], "w");
    if(!file)
    {
        printf("T%d: open %s for write fail! \n\n", idx, filePath[idx]);
        err();
    }
    printf("T%d: write data.... \n", idx);
    for(i=0; i<LOOP_NUM; i++)
    {
        printf("T%d:@\n", idx);
        res = fwrite(pattern[idx], 1, BUF_SIZE, file);
        if(res != BUF_SIZE)
        {
            printf("T%d: real write size 0x%X != write size 0x%X \n", idx, res, BUF_SIZE);
            goto end;
        }
    }
    res = 0;
    printf("T%d: Write finish!\n", idx);
    if(file)
    {
        res=fclose(file);
        file = NULL;
        if(res) goto end;
    }

    /*******************/
    /* Read & Compare  */
    /*******************/
    printf("T%d: compare data! \n", idx);
    file = fopen(filePath[idx], "r");
    if(!file)
    {
        printf("T%d: open %s for read fail! \n\n", idx, filePath[idx]);
        goto end;
    }
    printf("T%d: read data from card.. \n", idx);
    for(i=0; i<LOOP_NUM; i++)
    {
        printf("T%d:#\n", idx);
        memset(readBuf[idx], 0x55, BUF_SIZE);
        res = fread(readBuf[idx], 1, BUF_SIZE, file);
        if(res != BUF_SIZE)
        {
            printf("T%d: read back size 0x%X != read size 0x%X \n", idx, res, BUF_SIZE);
            goto end;
        }
        diff = memcmp((void*)pattern[idx], (void*)readBuf[idx], BUF_SIZE);
        if(diff)
            printf("T%d: diff = %d \n", idx, diff);

        if(diff)
		{
			uint32_t j = 0, count=0;
            printf("T%d: i = %d \n", idx, i);
			for(j=0; j<BUF_SIZE; j++)
			{
				if(pattern[idx][j] != readBuf[idx][j])
                {
					printf("T%d: write buffer[%X] = %02X, read buffer1[%X] = %02X \n", idx, j, pattern[idx][j], j, readBuf[idx][j]);
                    //if(count++ > 50)
                    //    while(1);
                }
			}
			err();
		}
    }
    res = 0;
    printf("T%d: Compare Data End!!\n\n\n", idx);

    //usleep(200*1000);

end:
    if (pattern[idx]) {
        free(pattern[idx]);
        pattern[idx] = NULL;
    }
    if (readBuf[idx]) {
        free(readBuf[idx]);
        readBuf[idx] = NULL;
    }
    if (file)
    {
        res = fclose(file);
        file = NULL;
    }
    return res;
}




#define TEST_STACK_SIZE 1024*100

static pthread_t task[16];
static pthread_attr_t attr[16];
static int taskId[16];
static int taskRes[16] = {0};

static void* TaskFunc(void* arg)
{
    int taskId = *(int*)arg;

    do {
        taskRes[taskId] = ReadWriteTest(taskId);
    } while(taskRes[taskId]==0);
        
    return (void*)&taskRes[taskId];
}

void* TestFunc(void* arg)
{
    ITPDriveStatus driveStatus;
    int mscIndex;
    void* res;

    while(1)
    {
        if (read(ITP_DEVICE_DRIVE, &driveStatus, sizeof (ITPDriveStatus)) == sizeof (ITPDriveStatus))
        {
            if(driveStatus.avail)
            {
                mscIndex = driveStatus.disk - ITP_DISK_MSC00;
                if (!task[mscIndex]) {
                    printf("\n\n MSC %d is mount ready! \n\n", mscIndex);
                    sprintf(filePath[mscIndex], "%smsc_%d.dat", driveStatus.name, mscIndex);
                    printf(" file name: %s \n", filePath[mscIndex]);

                    taskId[mscIndex] = mscIndex;
                    pthread_attr_init(&attr[mscIndex]);
                    pthread_attr_setstacksize(&attr[mscIndex], TEST_STACK_SIZE);
                    pthread_create(&task[mscIndex], &attr[mscIndex], TaskFunc, &taskId[mscIndex]);
                }
            }
            else
            {
                mscIndex = driveStatus.disk - ITP_DISK_MSC00;
                printf("\n\n MSC %d is UnMount! \n", mscIndex);

                pthread_join(task[mscIndex], &res);
                task[mscIndex] = 0;
                printf("\n\n Thread %d join (leave id:0x%X)!! res=0x%X \n\n", mscIndex, *((int*)res), res);
            }
        }
        usleep(1*1000*1000);
    }
}
