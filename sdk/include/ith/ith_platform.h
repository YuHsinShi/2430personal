#ifndef ITH_PLATFORM_H
#define ITH_PLATFORM_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_platform Platform
 * Used by this module internal.
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#if defined(ANDROID)                // Android
    #include <stdbool.h>
    #include <stddef.h>
    #include <stdint.h>

extern uint32_t ithMmioBase;
    #define ITH_REG_BASE (ithMmioBase - ITH_MMIO_BASE)

#elif defined(__LINUX_ARM_ARCH__)    // Linux kernel
    #include <asm/io.h>

extern uint32_t ithMmioBase;
    #define ITH_REG_BASE (ithMmioBase - ITH_MMIO_BASE)

#elif defined(CONFIG_ARM)            // U-Boot
    #include <stdbool.h>
    #include "compiler.h"

    #define ITH_REG_BASE 0

#elif defined(_WIN32)                // Win32
    #include <stdbool.h>
    #include <stddef.h>
    #include <stdint.h>

    #define inline   __inline
    #define __func__ __FUNCTION__
    #define __attribute__(x)

#elif defined(__OPENRTOS__)         // OpenRTOS
    #include <stdbool.h>
    #include <stddef.h>
    #include <stdint.h>

    #define ITH_REG_BASE 0

#else
    #include <stdbool.h>
    #include <stddef.h>
    #include <stdint.h>

    #define ITH_REG_BASE 0

#endif // defined(ANDROID)

#define N01_BITS_MSK     (0x0001)                       //                        1
#define N02_BITS_MSK     (0x0003)                       //                       11
#define N03_BITS_MSK     (0x0007)                       //                      111
#define N04_BITS_MSK     (0x000F)                       //                     1111
#define N05_BITS_MSK     (0x001F)                       //                   1 1111
#define N06_BITS_MSK     (0x003F)                       //                  11 1111
#define N07_BITS_MSK     (0x007F)                       //                 111 1111
#define N08_BITS_MSK     (0x00FF)                       //                1111 1111
#define N09_BITS_MSK     (0x01FF)                       //              1 1111 1111
#define N10_BITS_MSK     (0x03FF)                       //             11 1111 1111
#define N11_BITS_MSK     (0x07FF)                       //            111 1111 1111
#define N12_BITS_MSK     (0x0FFF)                       //           1111 1111 1111
#define N13_BITS_MSK     (0x1FFF)                       //         1 1111 1111 1111
#define N14_BITS_MSK     (0x3FFF)                       //        11 1111 1111 1111
#define N15_BITS_MSK     (0x7FFF)                       //       111 1111 1111 1111
#define N16_BITS_MSK     (0xFFFF)                       //      1111 1111 1111 1111
#define N17_BITS_MSK     (0x1FFFF)                      //    1 1111 1111 1111 1111
#define N18_BITS_MSK     (0x3FFFF)                      //   11 1111 1111 1111 1111
#define N19_BITS_MSK     (0x7FFFF)                      //  111 1111 1111 1111 1111
#define N20_BITS_MSK     (0xFFFFF)                      // 1111 1111 1111 1111 1111

/**
 * Called when assert fail.
 *
 * @param exp   The evaluated expression.
 * @param file  The evaluated file.
 * @param line  The evaluated line of file.
 * @param func  The evaluated function.
 */
extern void ithAssertFail(const char *exp, const char *file, int line, const char *func);

/**
 * Busy-waiting delay.
 *
 * @param us microseconds.
 */
extern void ithDelay(unsigned long us);

/**
 * Locks mutex.
 *
 * @param mutex The mutex object.
 */
extern void ithLockMutex(void *mutex);

/**
 * Unlocks mutex.
 *
 * @param mutex The mutex object.
 */
extern void ithUnlockMutex(void *mutex);

/**
 * Flushes CPU's data cache.
 */
extern void ithFlushDCache(void);

/**
 * Invalidates CPU's data cache.
 */
extern void ithInvalidateDCache(void);

/**
 * Flushes CPU's data cache in specified range.
 *
 * @param ptr   the memory address to flush.
 * @param size  the memory size to flush.
 */
extern void ithFlushDCacheRange(void *ptr, uint32_t size);

/**
 * Flushes CPU's data cache in specified range.
 *
 * @param ptr   the memory address to flush.
 * @param size  the memory size to flush.
 */
extern void ithInvalidateDCacheRange(void *ptr, uint32_t size);

/**
 * Invalidates CPU's instruction cache.
 */
extern void ithInvalidateICache(void);

/**
 * CPU Doze, it's gated CPU clock until interrupt happend.
 */
extern void ithCpuDoze(void);

/**
 * Flushes Memory buffer.
 */
extern void ithFlushMemBuffer(void);

/**
 * Flushes AHB Wrap.
 */
extern void ithFlushAhbWrap(void);

/**
 * Reschedules task after ISR is finished.
 *
 * @param yield task yield if yield is true.
 */
extern void ithYieldFromISR(bool yield);

/**
 * Saves current interrupt states and disable interrupt.
 */
extern void ithEnterCritical(void);

/**
 * Restore previous saved interrupt states.
 */
extern void ithExitCritical(void);

/**
 * Forces a context switch.
 */
extern void ithTaskYield(void);

/**
 * CPU mode definitions.
 */
typedef enum
{
    ITH_CPU_USR = 0x10, ///< USR_MODE
    ITH_CPU_FIQ = 0x11, ///< FIQ_MODE
    ITH_CPU_IRQ = 0x12, ///< IRQ_MODE
    ITH_CPU_SVC = 0x13, ///< SVC_MODE
    ITH_CPU_ABT = 0x17, ///< ABT_MODE
    ITH_CPU_UND = 0x1B, ///< UND_MODE
    ITH_CPU_SYS = 0x1F  ///< SYS_MODE
} ITHCpuMode;

/**
 * Gets CPU mode.
 * @return The CPU mode.
 */
extern ITHCpuMode ithGetCpuMode(void);

/**
 * Initialize this module.
 */
void ithInit(void);

/**
 * Exit this module.
 */
void ithExit(void);

#ifdef __cplusplus
}
#endif

#endif // ITH_PLATFORM_H
/** @} */ // end of ith_platform
/** @} */ // end of ith