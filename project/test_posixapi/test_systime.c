#include <sys/time.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "ite/itp.h"

void test_gettimeofday()
{
  struct timeval tv, tv2;
  unsigned long long start_utime, end_utime;

  gettimeofday(&tv,NULL);
  printf("The time is %s\n", ctime(&tv.tv_sec));                                     
}

void test_settimeofday()
{
	struct timeval tv;
	struct timezone tz;
	struct tm *tm, mytime;
	
	gettimeofday(&tv, &tz);
	tm = localtime(&tv.tv_sec);
	memcpy(&mytime, tm, sizeof (struct tm));
	
    mytime.tm_mday = 1;
    mytime.tm_mon = 3 - 1;
    mytime.tm_year = 2000 - 1900;
	tv.tv_sec = mktime ( &mytime );
	
	if(settimeofday(&tv,&tz) == -1)
	{
		if(errno == EPERM)
			printf("The calling process has insufficient privilege to call settimeofday()\n");
		else if( errno == EINVAL)
			printf("Timezone (or something else) is invalid.\n");
		else if(errno == EFAULT)
			printf("One of tv or tz pointed outside the accessible address space.\n");
		else
			printf("Unknow Error.\n");
	}
}

void* TestFunc(void* arg)
{
	itpInit();
	
	printf("\nTest : settimeofday()\n-------------------\n");
    test_settimeofday();

	printf("\nTest : gettimeofday()\n-------------------\n");
	test_gettimeofday();
    
    printf("\nEnd the test\n");
}

