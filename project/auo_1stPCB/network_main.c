#include <pthread.h>
#include "ctrlboard.h"

/* Eason Refined in Mar. 2020 */

static void* NetworkEthTask(void* arg)
{
#if defined(CFG_NET_ETHERNET)
    NetworkPreSetting();

    while (!NetworkIsExit())
    {
        /* Network Functions Process*/
        NetworkEthernetProcess();

        sleep(1);
    }
#endif

    return NULL;
}

static void* NetworkWifiTask(void* arg)
{
#if defined(CFG_NET_WIFI)
    NetworkWifiPreSetting();

    while (1)
    {
        NetworkWifiProcess();

        sleep(1);
    }
#endif

    return NULL;
}

static void CreateWorkerThread(void *(*start_routine)(void *), void *arg)
{
    pthread_t      task;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&task, &attr, start_routine, arg);
}

void NetworkInit(void)
{
    /* Create Network Thread */
    /* Ethernet Thread */
    CreateWorkerThread(NetworkEthTask,  NULL);

    /* WIFI Thread */
    CreateWorkerThread(NetworkWifiTask, NULL);
}

