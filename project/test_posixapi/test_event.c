#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/itp.h"

#define BITS_TO_WAIT    0x0101
#define BITS_TO_SET     0x0101
#define MS_TO_WAIT      500 //(unsigned long)(-1)

ITPEvent* ev;
uint32_t tickCount;

//The following example attempts to test wait event bits.
void* Test_event_wait(void *arg)
{
  unsigned long bits;

  tickCount = itpGetTickCount();

  printf("wait bits 0x%X\n", BITS_TO_WAIT);
  bits = itpEventWaitBits(ev, BITS_TO_WAIT, true, true, MS_TO_WAIT);// wait event
  printf("wait result: 0x%X\n", bits);

  //bits = itpEventWaitBits(ev, BITS_TO_WAIT, false, true, MS_TO_WAIT);// wait event
  //printf("wait result: 0x%X\n", bits);

  printf("duration: %u\n", itpGetTickDuration(tickCount));

  itpEventDestroy(ev);

  return NULL;
}

//The following example attempts to test set event bits.
void Test_event_set(void)
{
  int result;

  usleep(100 * 1000);

  printf("set bits 0x%X\n", BITS_TO_SET);
  result = itpEventSetBits(ev, BITS_TO_SET);
  printf("set result: %d\n", result);
}

void* TestFunc(void* arg)
{
    pthread_t thid;

	itpInit();

    ev = itpEventCreate();// Initializes an event object
    if (!ev)
    {
        perror("itpEventCreate() error");	exit(1);
    }

	printf("\nTest: Test_event_wait()\n---------------\n");
    if (pthread_create(&thid, NULL, Test_event_wait, NULL) != 0)
    {
        perror("pthread_create() error");	exit(2);
    }

	printf("\nTest: Test_event_set()\n---------------\n");
	Test_event_set();

    pthread_join(thid, NULL);
    printf("\nEnd the test\n");

    return NULL;
}

