/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
#include <stdint.h>

#define CACHE_LINESZ 32
#define TIMER_LOAD_VAL  0xFFFFFFFF


void ithFlushDCache(void)
{
#ifdef CFG_CPU_WB
    __asm__ __volatile__ (
        "mov r3,#0\n"
        "mcr p15,0,r3,c7,c10,0\n"   /* flush d-cache all */
        "mcr p15,0,r3,c7,c10,4\n"   /* flush d-cache write buffer */
        :
        :
        : "r3"  /* clobber list */
        );
#endif // CFG_CPU_WB
}

__attribute__((used)) void ithInvalidateDCache(void)
{
    __asm__ __volatile__ (
        "mov r3,#0\n"
        "mcr p15,0,r3,c7,c6,0\n"    /* invalidate d-cache all */
        :
        :
        : "r3"  /* clobber list */
        );
}

__attribute__((used)) void ithFlushDCacheRange(void* addr, uint32_t size)
{
#ifdef CFG_CPU_WB
    unsigned int linesz = CACHE_LINESZ;
    unsigned long start = (uint32_t) addr;

    // aligned to cache line
    size += (start % linesz) ? linesz : 0;
    start &= ~(linesz - 1);
    // do it!
    __asm__ __volatile__ (
        "mov r0,%0\n"
        "mov r1,%1\n"
        "add r1,r0,r1\n"
        "1:\n"
        "mcr p15,0,r0,c7,c10,1\n"
        "add r0,r0,%2\n"
        "cmp r0,r1\n"
        "blo 1b\n"
        "mov r0,#0\n"
        "mcr p15,0,r0,c7,c10,4\n"
        :
        : "r"(start), "r"(size), "r"(linesz)    /* input */
        : "r0", "r1", "cc"  /* clobber list */
        );
#endif // CFG_CPU_WB
}

__attribute__((used)) void ithInvalidateDCacheRange(void* addr, uint32_t size)
{
    unsigned int linesz = CACHE_LINESZ;
    unsigned long start = (uint32_t) addr;

    // aligned to cache line
    size += (start % linesz) ? linesz : 0;
    start &= ~(linesz - 1);

    __asm__ __volatile__ (
        "mov r0,%0\n"
        "mov r1,%1\n"
        "add r1,r0,r1\n"
        "1:\n"
        "mcr p15,0,r0,c7,c6,1\n"
        "add r0,r0,%2\n"
        "cmp r0,r1\n"
        "blo 1b\n"
        :
        : "r"(start), "r"(size), "r"(linesz)    /* input */
        : "r0", "r1", "cc"  /* clobber list */
        );
}

void ithInvalidateICache(void)
{
    __asm__ __volatile__ (
        "mcr p15,0,%0,c7,c14,0\n"
        "mcr p15,0,%0,c7,c5,0\n"    /* invalidate i-cache all */
        :
        :
        "r"(0)  /* clobber list */
        );
}

__attribute__((used)) void ithFlushMemBuffer(void)
{
    __asm__ __volatile__ (
        "mov r3,#0\n"
        "mcr p15,0,r3,c7,c10,4\n"   /* flush d-cache write buffer */
        :
        :
        : "r3"  /* clobber list */
        );
}