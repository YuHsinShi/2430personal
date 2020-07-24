#ifndef ITH_MEMWRAP_H
#define ITH_MEMWRAP_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_memwrap Memory Wrap
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Memory Wrap module definition.
 */
typedef enum
{
    ITH_MEM_AXI0WRAP = ITH_MEMWRAP_AXI0_REG,    ///< MEM AXI0 Wrap
    ITH_MEM_AXI1WRAP = ITH_MEMWRAP_AXI1_REG,    ///< MEM AXI0 Wrap
    ITH_MEM_AXI2WRAP = ITH_MEMWRAP_AXI2_REG,    ///< MEM AXI0 Wrap
    ITH_MEM_AXI3WRAP = ITH_MEMWRAP_AXI3_REG     ///< MEM AXI0 Wrap
} ITHMemWrap;

static inline int ithMemWrapFlush(ITHMemWrap wrap_idx)
{
    if ((wrap_idx == ITH_MEM_AXI2WRAP) ||
        (wrap_idx == ITH_MEM_AXI3WRAP) ||
        ((wrap_idx == ITH_MEM_AXI0WRAP) && (ithReadRegA(ITH_MEM_BASE + 0x204) & 0x8000)) ||
        ((wrap_idx == ITH_MEM_AXI1WRAP) && (ithReadRegA(ITH_MEM_BASE + 0x244) & 0x8000)))
    {
        int timeout = 100;
        ithSetRegBitA(ITH_MEM_BASE + wrap_idx, ITH_MEMWRAP_FLUSH_BIT);

        //  Wait MEM Wrap flush finish!
        while ((ithReadRegA(ITH_MEM_BASE + wrap_idx) & ITH_MEMWRAP_FLUSH_MASK) && timeout--);

        return (timeout <= 0) ? -1 : 0;
    }

    return 0;
}

#ifdef __cplusplus
}
#endif

#endif // ITH_MEMWRAP_H
/** @} */ // end of ith_memstat
/** @} */ // end of ith