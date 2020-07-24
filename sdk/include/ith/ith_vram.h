/**
 * @file ith_vram.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2019-08-01
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef ITH_VRAM_H
#define ITH_VRAM_H

#include "ith/ith_utility.h"

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_vram VRAM
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#define ITH_VRAM_READ      0x1  ///< Mapping flag for read.
#define ITH_VRAM_WRITE     0x2  ///< Mapping flag for write. 

// Video memroy management
#define ITH_VMEM_MCB_COUNT 2048  ///< Maximum Video memory control block

/**
 * Video memory control block definition.
 */
typedef struct
{
    ITHList  *next;             ///< next block
    ITHList  *prev;             ///< previous block
    uint32_t addr;              ///< memory address
    uint32_t size  : 30;        ///< memory size
    uint32_t state : 2;         ///< memory state
} ITHVmemMcb;

/**
 * Video memory management global data definition.
 */
typedef struct
{
    uint32_t   startAddr;                   ///< start address to management
    uint32_t   totalSize;                   ///< total size to management
    void       *mutex;                      ///< mutex to protect allocation/free
    ITHVmemMcb mcbs[ITH_VMEM_MCB_COUNT];    ///< pre-allocate memory control blocks
    uint32_t   usedMcbCount;                ///< used memory control block count
    ITHList    usedMcbList;                 ///< used memory control block list
    uint32_t   freeSize;                    ///< current free size
} ITHVmem;

/**
 * Global video memory management instance
 */
extern ITHVmem *ithVmem;

#if defined(ANDROID) || defined(__LINUX_ARM_ARCH__) || defined(_WIN32)
void *ithMapVram(uint32_t vram_addr, uint32_t size, uint32_t flags);
void ithUnmapVram(void *sys_addr, uint32_t size);
uint32_t ithSysAddr2VramAddr(void *sys_addr);
void *ithVramAddr2SysAddr(uint32_t vram_addr);
#else // U-Boot and OpenRTOS

/**
 * Maps VRAM to an accessible address.
 *
 * @param vram_addr The start VRAM address to map.
 * @param size The mapping size.
 * @param flags The flags. Can be union of ITH_VRAM_READ and ITH_VRAM_WRITE.
 * @return The accessible address. NULL indicates failed.
 * @see ithUnmapVram()
 * @par Example:
 * @code
    uint16_t* base = ithMapVram(lcd_addr, lcd_pitch * lcd_height, ITH_VRAM_WRITE);
    uint16_t* ptr = base;

    // fill black to lcd screen
    for (y = 0; y < lcd_height; y++)
        for (x = 0; x < lcd_width; x++)
 * ptr++ = 0x0;

    ithUnmapVram(base, lcd_pitch * lcd_height);

 * @endcode
 */
__attribute__((always_inline))
static inline void *ithMapVram(uint32_t vram_addr, uint32_t size, uint32_t flags)
{
    return (void *) vram_addr;
}

/**
 * Unmaps a VRAM accessible address.
 *
 * @param sys_addr The VRAM accessible address.
 * @param size The mapping size.
 * @see ithMapVram()
 */
__attribute__((always_inline))
static inline void ithUnmapVram(void *sys_addr, uint32_t size)
{
    // DO NOTHING
}

__attribute__((always_inline))
static inline uint32_t ithSysAddr2VramAddr(void *sys_addr)
{
    return (uint32_t)sys_addr;
}

__attribute__((always_inline))
static inline void *ithVramAddr2SysAddr(uint32_t vram_addr)
{
    return (void *)vram_addr;
}

#endif // defined(ANDROID)|| defined(__LINUX_ARM_ARCH__)|| defined(_WIN32)

/**
 * Reads VRAM data to system memory.
 *
 * @param dest The destination system memory address.
 * @param src The source VRAM address.
 * @param size The size will be read.
 */
void ithReadVram(void *dest, uint32_t src, uint32_t size);

/**
 * Writes system memory data to VRAM.
 *
 * @param dest The destination VRAM address.
 * @param src The source system memory address.
 * @param size The size will be written.
 */
void ithWriteVram(uint32_t dest, const void *src, uint32_t size);

/**
 * Copies memory block between VRAM.
 *
 * @param dest The VRAM address will be copied to.
 * @param src The VRAM address will be copied from.
 * @param size The block size will be copied.
 */
void ithCopyVram(uint32_t dest, uint32_t src, uint32_t size);

/**
 * Sets VRAM to a specified character.
 *
 * @param dest Address of destination.
 * @param c Character to set.
 * @param size The VRAM size to be set.
 */
void ithSetVram(uint32_t dest, int c, uint32_t size);

/**
 * Initializes video memory management module
 *
 * @param vmem the video memory management instance
 */
void ithVmemInit(ITHVmem *vmem);

/**
 * Allocates video memory.
 *
 * @param size The amount of memory you want to allocate, in bytes.
 * @return Allocated memory address, or 0 if an error occurred
 */
uint32_t ithVmemAlloc(uint32_t size);

/**
 * The ithVmemAlignedAlloc function is similar to the memalign function in that
 * it returns a video memory of size bytes aligned to a multiple of alignment.
 *
 * @param alignment The alignment that you want to use for the memory. This must be a multiple of size( void *).
 * @param size The amount of memory you want to allocate, in bytes.
 * @return Allocated memory address, or 0 if an error occurred
 */
uint32_t ithVmemAlignedAlloc(uint32_t alignment, uint32_t size);

/**
 * Releases allocated video memory.
 *
 * @param addr The allocated video memory address.
 */
void ithVmemFree(uint32_t addr);

/**
 * Prints the video memory management status.
 */
void ithVmemStats(void);

#ifdef __cplusplus
}
#endif

#endif // ITH_VRAM_H
/** @} */ // end of ith_vram
/** @} */ // end of ith