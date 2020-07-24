#include <pthread.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"

#define TEST_STACK_SIZE 1024000

#if defined(CFG_TEST_RTC_COUNT)
extern void* TestFunc_count(void* arg);
#elif defined(CFG_TEST_RTC_MS)
extern void* TestFunc_MS(void* arg);
#elif defined(CFG_TEST_RTC_ALARM)
extern void* TestFunc_Alarm(void* arg);
#endif

int main(void)
{
    pthread_t task;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    attr.stacksize = TEST_STACK_SIZE;

#if defined(CFG_TEST_RTC_COUNT)
	pthread_create(&task, &attr, TestFunc_count, NULL);
#elif defined(CFG_TEST_RTC_MS)
	pthread_create(&task, &attr, TestFunc_MS, NULL);
#elif defined(CFG_TEST_RTC_ALARM)
	pthread_create(&task, &attr, TestFunc_Alarm, NULL);
#endif

    /* Now all the tasks have been started - start the scheduler. */
    vTaskStartScheduler();

    /* Should never reach here! */
    return 0;
}
