#include <stdio.h>
#include <unistd.h>
#include "ite/itp.h"
#include "nedmalloc.h"

static nedpool  *memPool1, *memPool2;

void* TestFunc(void* arg)
{
    void* ptr;
	itpInit();

	printf("\nTest: nedcreatepool()\n---------------\n");
    memPool1= nedcreatepool(0x200000, 0);
    if (!memPool1)
    {
        printf("nedcreatepool() fail\n");
        return NULL;
    }

    memPool2 = nedcreatepool(0x100000, 0);
    if (!memPool2)
    {
        printf("nedcreatepool() fail\n");
        return NULL;
    }

	printf("\nTest: nedpmemalign()\n---------------\n");
    ptr = nedpmemalign(memPool1, 32, 0x200000);
    if (!ptr)
    {
        printf("nedpmemalign() fail\n");
        return NULL;
    }

    printf("\nTest: nedpmalloc_stats()\n---------------\n");
    nedpmalloc_stats(memPool1);

    printf("\nTest: nedpfree()\n---------------\n");
    nedpfree(memPool1, ptr);

    printf("\nTest: nedpmalloc()\n---------------\n");
    ptr = nedpmalloc(memPool2, 0x100000);
    if (!ptr)
    {
        printf("nedpmemalign() fail\n");
        return NULL;
    }

    printf("\nTest: nedpmalloc_stats()\n---------------\n");
    nedpmalloc_stats(memPool2);

    printf("\nTest: nedpfree()\n---------------\n");
    nedpfree(memPool2, ptr);

    printf("\nEnd the test\n");

    return NULL;
}

