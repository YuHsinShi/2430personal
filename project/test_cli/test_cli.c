#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include "ite/cli.h"

void* TestFunc(void* arg)
{
    cliInit();

    while (1)
        sleep(1);

    return NULL;
}


