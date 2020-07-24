#include <pthread.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"

extern void* TestFunc_Wifi_Ethernet(void* arg);

int main(void)
{
    pthread_t task_wifi_eth;
    pthread_attr_t attr_wifi_eth;

    pthread_attr_init(&attr_wifi_eth);

    pthread_create(&task_wifi_eth, &attr_wifi_eth, TestFunc_Wifi_Ethernet, NULL);

    /* Now all the tasks have been started - start the scheduler. */
    vTaskStartScheduler();

    /* Should never reach here! */
    return 0;
}
