#ifndef KERNEL_H
#define KERNEL_H

#include <stdio.h>


#if !defined(_WIN32)

/**
* swap - swap values of @a and @b
* @a: first value
* @b: second value
*/
#define swap(a, b) \
do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)

#endif


#ifndef round_up
#define round_up	ITH_ALIGN_UP
#endif

#ifndef round_down
#define round_down	ITH_ALIGN_DOWN
#endif



#endif // KERNEL_H
