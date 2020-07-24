#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/itp.h"

static bool quit = false;

//The following example attempts to test suspend andresume.
void* Test_suspend(void *arg)
{
    int count = 0;

    while (!quit)
    {
        printf("%d\n", count++);
        sleep(1);
    }
    return NULL;
}

void* TestFunc(void* arg)
{
    pthread_t thid;

	itpInit();

	printf("\nTest: Test_suspend()\n---------------\n");
    if (pthread_create(&thid, NULL, Test_suspend, NULL) != 0)
    {
        perror("pthread_create() error");	exit(2);
    }

    sleep(3);

	printf("\nTest: suspend\n---------------\n");
    itpTaskSuspend(thid);

    sleep(3);

    printf("\nTest: resume\n---------------\n");
    itpTaskResume(thid);

    sleep(3);
    quit = true;

    pthread_join(thid, NULL);
    printf("\nEnd the test\n");

    return NULL;
}

