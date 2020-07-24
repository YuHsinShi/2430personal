#include <pthread.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"
#include "ite/itp.h"


extern void* TestFunc(void* arg);


static void* main_task(void* arg)
{
    // init pal
    itpInit();

    TestFunc(NULL);
    
    // do the test
    for (;;)
    {
        sleep(1);
    }
    
    return NULL;
}

int main(void)
{
    pthread_t task;
    pthread_attr_t attr;
    
    pthread_attr_init(&attr);
    pthread_create(&task, &attr, main_task, NULL);

    /* Now all the tasks have been started - start the scheduler. */
    vTaskStartScheduler();

    /* Should never reach here! */
    return 0;
}
