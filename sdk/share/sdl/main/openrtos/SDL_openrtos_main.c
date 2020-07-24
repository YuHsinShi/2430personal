
/* Include the SDL main definition header */
#include "SDL_main.h"
#include <sys/ioctl.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"
#include "ite/itp.h"
#include "ite/ite_risc.h"

#define MAIN_STACK_SIZE 100000

static void* MainTask(void* arg)
{
    char *argv[2];

    // init pal
    itpInit();

    argv[0] = "SDL_app";
    argv[1] = NULL;

    SDL_main(1, argv);
    return NULL;
}

#ifdef main
#undef main
int
main(int argc, char *argv[])
{
    pthread_t task;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, MAIN_STACK_SIZE);
    pthread_create(&task, &attr, MainTask, NULL);

    /* Now all the tasks have been started - start the scheduler. */
    vTaskStartScheduler();

    /* Should never reach here! */
    return 0;

}
#endif

/* vi: set ts=4 sw=4 expandtab: */
