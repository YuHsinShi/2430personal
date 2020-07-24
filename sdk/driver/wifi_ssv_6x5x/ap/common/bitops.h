/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _BITOPS_H
#define _BITOPS_H

#include <ssv_types.h>


//---------------------------
#define __u64				ssv_type_u64
#define DIV_ROUND_UP(n, d)	(((n) + (d) - 1) / (d))
//----------------------------

#define BITS_PER_LONG		32


#ifndef BIT
#define BIT(nr)			(1UL << (nr))
#endif

#define BIT_MASK(nr)		(1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)		((nr) / BITS_PER_LONG)
#define BITS_PER_BYTE		8
#define BITS_TO_LONGS(nr)	DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))

int ffs(int x); 


#endif
