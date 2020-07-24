#ifndef ITH_UTILITY_H
#define ITH_UTILITY_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_utility Utility
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

// Macros
/**
 * Assertion at compile time.
 *
 * @param e Specifies any logical expression.
 */
#define ITH_STATIC_ASSERT(e) \
    typedef int ITH_STATIC_ASSERT_DUMMY_ ## __LINE__[(e) * 2 - 1]

/**
 * Counts the element number of an array.
 *
 * @param array The array.
 * @return The element number of array.
 */
#define ITH_COUNT_OF(array)            (sizeof(array) / sizeof(array[0]))

/**
 * Aligns a value to its ceil.
 *
 * @param value The value will be aligned.
 * @param align The alignment. It must be a power of 2.
 * @return The aligned value.
 */
#define ITH_ALIGN_UP(value, align)     (((value) + ((align) - 1)) & ~((align) - 1))

/**
 * Aligns a value to its floor.
 *
 * @param value The value will be aligned.
 * @param align The alignment. It must be a power of 2.
 * @return The aligned value.
 */
#define ITH_ALIGN_DOWN(value, align)   ((value) & ~((align) - 1))

/**
 * Determines whether the value is aligned.
 *
 * @param value The value.
 * @param align The alignment. It must be a power of 2.
 * @return Whether the value is aligned.
 */
#define ITH_IS_ALIGNED(value, align)   (((value) & ((align) - 1)) == 0)

/**
 * Determines whether the value is unaligned.
 *
 * @param value The value.
 * @param align The alignment. It must be a power of 2.
 * @return Whether the value is unaligned.
 */
#define ITH_IS_UNALIGNED(value, align) (((value) & ((align) - 1)) != 0)

/**
 * Determines whether the value is power of two.
 *
 * @param x The value.
 * @return Whether the value is power of two.
 */
#define ITH_IS_POWER_OF_TWO(x)         ((((x) - 1) & (x)) == 0)

/**
 * Calculates the absolute value.
 *
 * @param x The value.
 * @return The absolute value.
 */
#define ITH_ABS(x)                     (((x) >= 0) ? (x) : -(x))

/**
 * Returns the larger of two values.
 *
 * @param a Values of any numeric type to be compared.
 * @param b Values of any numeric type to be compared.
 * @return The larger of its arguments.
 */
#define ITH_MAX(a, b)                  (((a) > (b)) ? (a) : (b))

/**
 * Returns the smaller of two values.
 *
 * @param a Values of any numeric type to be compared.
 * @param b Values of any numeric type to be compared.
 * @return The smaller of its arguments.
 */
#define ITH_MIN(a, b)                  (((a) < (b)) ? (a) : (b))

/**
 * Swaps two values.
 *
 * @param a The value.
 * @param b The another value.
 * @param type The value type.
 */
#define ITH_SWAP(a, b, type) \
    do { type tmp = (a); a = (b); b = tmp; } while (0)

/**
 * Converts 16-bit value to another endian integer.
 *
 * @param value The value.
 * @return The converted value.
 */
static inline uint16_t ithBswap16(uint16_t value)
{
    return ((value & 0x00FF) << 8) |
           ((value & 0xFF00) >> 8);
}

/**
 * Converts 32-bit value to another endian integer.
 *
 * @param value The value.
 * @return The converted value.
 */
static inline uint32_t ithBswap32(uint32_t value)
{
    return ((value & 0x000000FF) << 24) |
           ((value & 0x0000FF00) << 8) |
           ((value & 0x00FF0000) >> 8) |
           ((value & 0xFF000000) >> 24);
}

/**
 * Converts 64-bit value to another endian integer.
 *
 * @param value The value.
 * @return The converted value.
 */
static inline uint64_t ithBswap64(uint64_t value)
{
    return ((value & 0xff00000000000000ull) >> 56) |
           ((value & 0x00ff000000000000ull) >> 40) |
           ((value & 0x0000ff0000000000ull) >> 24) |
           ((value & 0x000000ff00000000ull) >> 8 ) |
           ((value & 0x00000000ff000000ull) << 8 ) |
           ((value & 0x0000000000ff0000ull) << 24) |
           ((value & 0x000000000000ff00ull) << 40) |
           ((value & 0x00000000000000ffull) << 56);
}

/**
 * Packs colors to a RGB565 format value.
 *
 * @param r red value.
 * @param g green value.
 * @param b blue value.
 * @return The packed value.
 */
#define ITH_RGB565(r, g, b) \
    ((((uint16_t)(r) >> 3) << 11) | (((uint16_t)(g) >> 2) << 5) | ((uint16_t)(b) >> 3))

/**
 * Packs colors to a ARGB1555 format value.
 *
 * @param a alpha value.
 * @param r red value.
 * @param g green value.
 * @param b blue value.
 * @return The packed value.
 */
#define ITH_ARGB1555(a, r, g, b) \
    ((((uint16_t)(a) >> 7) << 15) | (((uint16_t)(r) >> 3) << 10) | (((uint16_t)(g) >> 3) << 5) | ((uint16_t)(b) >> 3))

/**
 * Packs colors to a ARGB4444 format value.
 *
 * @param a alpha value.
 * @param r red value.
 * @param g green value.
 * @param b blue value.
 * @return The packed value.
 */
#define ITH_ARGB4444(a, r, g, b) \
    ((((uint16_t)(a) >> 4) << 12) | (((uint16_t)(r) >> 4) << 8) | (((uint16_t)(g) >> 4) << 4) | ((uint16_t)(b) >> 4))

/**
 * Packs colors to a ARGB8888 format value.
 *
 * @param a alpha value.
 * @param r red value.
 * @param g green value.
 * @param b blue value.
 * @return The packed value.
 */
#define ITH_ARGB8888(a, r, g, b) \
    (((uint32_t)(a) << 24) | ((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b))

/**
 * Sets the first len words (16-bits) of the block of memory pointed by ptr to the specified value.
 *
 * @param dst Pointer to the block of memory to fill.
 * @param val Value to be set. The value is passed as an int, but the function fills the block of memory using the uint16_t conversion of this value.
 * @param len Number of words (16-bits) to be set to the value.
 */
static inline void ithMemset16(void *dst, int val, size_t len)
{
    uint16_t *p = (uint16_t *)dst;

    while (len--)
        *p++ = val;
}

/**
 * Sets the first len words (32-bits) of the block of memory pointed by ptr to the specified value.
 *
 * @param dst Pointer to the block of memory to fill.
 * @param val Value to be set. The value is passed as an int, but the function fills the block of memory using the uint32_t conversion of this value.
 * @param len Number of words (32-bits) to be set to the value.
 */
static inline void ithMemset32(void *dst, int val, size_t len)
{
    uint32_t *p = (uint32_t *)dst;

    while (len--)
        *p++ = val;
}

/** List node */
typedef struct ITHListTag
{
    struct ITHListTag *next; /**< Next node */
    struct ITHListTag *prev; /**< Previous node */
} ITHList;

/**
 * Pushes a node to the front of list.
 *
 * @param list The list.
 * @param node The node wiil be pushed.
 */
void ithListPushFront(ITHList *list, void *node);

/**
 * Pushes a node to the back of list.
 *
 * @param list The list.
 * @param node The node wiil be pushed.
 */
void ithListPushBack(ITHList *list, void *node);

/**
 * Inserts a node before another node.
 *
 * @param list The list.
 * @param listNode The node will be after the inserted node.
 * @param node The node wiil be inserted.
 */
void ithListInsertBefore(ITHList *list, void *listNode, void *node);

/**
 * Inserts a node after another node.
 *
 * @param list The list.
 * @param listNode The node will be before the inserted node.
 * @param node The node wiil be inserted.
 */
void ithListInsertAfter(ITHList *list, void *listNode, void *node);

/**
 * Removes a node from a list.
 *
 * @param list The list.
 * @param node The node wiil be removed.
 */
void ithListRemove(ITHList *list, void *node);

/**
 * Clears every nodes in a list.
 *
 * @param list The list.
 * @param dtor The destructor to destroy every node. Can be MMP_NULL.
 */
void ithListClear(ITHList *list, void (*dtor)(void *));

/**
 * Converts unsigned long value to binary format string.
 *
 * @param s The binary format string.
 * @param i The unsigned long value.
 */
char *ithUltob(char *s, unsigned long i);

/**
 * Prints host register values.
 *
 * @param addr the start register address to dump
 * @param size the size of register block to dump
 */
void ithPrintRegH(uint16_t addr, unsigned int size);

/**
 * Prints AMBA register values.
 *
 * @param addr the start register address to dump
 * @param size the size of register block to dump
 */
void ithPrintRegA(uint32_t addr, unsigned int size);

/**
 * Prints VRAM values on 8-bit format.
 *
 * @param addr the start address to dump
 * @param size the size of block to dump
 */
void ithPrintVram8(uint32_t addr, unsigned int size);

/**
 * Prints VRAM values on 16-bit format.
 *
 * @param addr the start address to dump
 * @param size the size of block to dump
 */
void ithPrintVram16(uint32_t addr, unsigned int size);

/**
 * Prints VRAM values on 32-bit format.
 *
 * @param addr the start address to dump
 * @param size the size of block to dump
 */
void ithPrintVram32(uint32_t addr, unsigned int size);

// Print functions

/**
 * Putchar callback function for ithPrintf().
 *
 * @param c the character to output
 * @return same as c
 */
extern int (*ithPutcharFunc)(int c);

/**
 * General printf() function.
 *
 * @param fmt Format control.
 * @param ... Optional arguments.
 * @return the number of characters printed, or a negative value if an error occurs.
 */
int ithPrintf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif // ITH_UTILITY_H
/** @} */ // end of ith_utility
/** @} */ // end of ith