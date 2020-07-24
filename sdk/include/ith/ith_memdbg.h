#ifndef ITH_MEMDGB_H
#define ITH_MEMDGB_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_memdbg Memory Debug
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Memory Debug module definition.
 */
typedef enum
{
    ITH_MEMDBG0 = ITH_MEM_BASE+ITH_MEMDBG0_BASE_REG, ///< MEMDBG #0
    ITH_MEMDBG1 = ITH_MEM_BASE+ITH_MEMDBG1_BASE_REG  ///< MEMDBG #1
} ITHMemDbgModule;

/**
 * Memory Debug mode definition.
 */
typedef enum
{
    ITH_MEMDBG_READWRITE = 0x0, ///< Both of Read and Write Requests
    ITH_MEMDGB_READONLY  = 0x1, ///< Only Read Request
    ITH_MEMDGB_WRITEONLY = 0x2  ///< Only Write Request
} ITHMemDbgMode;

/**
* Memory Debug AXI0 ID definition.
*/
typedef enum
{
    ITH_MEMDBG_ARM_I    = 0x00, ///< ARM instruction read id
    ITH_MEMDGB_ARM_D    = 0x10, ///< ARM data read and write id
    ITH_MEMDGB_ARM_P    = 0x20, ///< ARM p port read and write id
    ITH_MEMDBG_DMA_CH0  = 0x30, ///< DMA channel 0 read and write id
    ITH_MEMDBG_DMA_CH1  = 0x31, ///< DMA channel 1 read and write id
    ITH_MEMDBG_DMA_CH2  = 0x32, ///< DMA channel 2 read and write id
    ITH_MEMDBG_DMA_CH3  = 0x33, ///< DMA channel 3 read and write id
    ITH_MEMDBG_DMA_CH4  = 0x34, ///< DMA channel 4 read and write id
    ITH_MEMDBG_DMA_CH5  = 0x35, ///< DMA channel 5 read and write id
    ITH_MEMDBG_DMA_CH6  = 0x36, ///< DMA channel 6 read and write id
    ITH_MEMDBG_DMA_CH7  = 0x37, ///< DMA channel 7 read and write id
    ITH_MEMDGB_AHB_BRI0 = 0x40, ///< AHB Bridge 0 read and write id
    ITH_MEMDGB_AHB_BRI1 = 0x50, ///< AHB Bridge 1 read and write id
    ITH_MEMDBG_AHB_BRI2 = 0x60, ///< AHB Bridge 2 read and write id
    ITH_MEMDGB_GMAC     = 0x70, ///< GMAC read and write id
    ITH_MEMDGB_ARM_DMA  = 0x80  ///< ARM's DMA read and write id
} ITHMemDbgAXI0Id;

/**
* Memory Debug AXI1 ID definition.
*/
typedef enum
{
    ITH_MEMDBG_H264_READ  = 0, ///< H264 read id
    ITH_MEMDGB_H264_WRITE = 1  ///< H264 write id
} ITHMemDbgAXI1Id;

/**
 * Memory Debug AXI2 ID definition.
 */
typedef enum
{
    ITH_MEMDBG_LCD          = 0, ///< LCD read id
    ITH_MEMDGB_VP_READ      = 1, ///< VP read id
    ITH_MEMDGB_SADC_WRITE   = 2, ///< SADC write id
    ITH_MEMDBG_CMDQ0        = 3, ///< CMDQ 0 read and write id
    ITH_MEMDGB_RISC         = 4, ///< RISC read and write id
    ITH_MEMDGB_I2S_WRITE    = 5, ///< I2S write id
    ITH_MEMDBG_USB          = 6, ///< USB read and write id
    ITH_MEMDGB_TSI          = 7, ///< TSI read and write id
    ITH_MEMDGB_DCPS         = 8, ///< DCPS read and write id
    ITH_MEMDGB_UIENC        = 9, ///< UIENC read and write id
    ITH_MEMDGB_BIST2        = 10 ///< BIST 2 read and write id
} ITHMemDbgAXI2Id;

/**
* Memory Debug AXI3 ID definition.
*/
typedef enum
{
    ITH_MEMDBG_CAP      = 0, ///< CAP read and write id
    ITH_MEMDGB_VP_WRITE = 1, ///< VP write id
    ITH_MEMDGB_JPEG     = 2, ///< JPEG read and write id
    ITH_MEMDBG_2D       = 3, ///< 2D read and write id
    ITH_MEMDGB_CMDQ1    = 4, ///< CMDQ 1 read and write id
    ITH_MEMDGB_I2S_READ = 5, ///< I2S read id
    ITH_MEMDBG_DPU      = 6, ///< DPU read and write id
    ITH_MEMDGB_TSM      = 7, ///< TSM read and write id
    ITH_MEMDGB_BIST0    = 8, ///< BIST 0 read and write id
    ITH_MEMDGB_BIST1    = 9, ///< BIST 1 read and write id
    ITH_MEMDGB_IQ       = 10 ///< IQ read and write id
} ITHMemDbgAXI3Id;

/**
 * Enables memory debug. interrupt
 *
 */
static inline void ithMemDbgEnableIntr(void)
{
    uint32_t mask = ITH_MEMDBG_AXI0INT_EN_MASK |
        ITH_MEMDBG_AXI1INT_EN_MASK |
        ITH_MEMDBG_AXI2INT_EN_MASK |
        ITH_MEMDBG_AXI3INT_EN_MASK;
    ithWriteRegMaskA(ITH_MEMDBG0 + ITH_MEMDBG_EN_REG, 0x0, mask);
    ithWriteRegMaskA(ITH_MEMDBG1 + ITH_MEMDBG_EN_REG, 0x0, mask);
}

/**
 * Disable memory debug. interrupt
 *
 */
static inline void ithMemDbgDisableIntr(void)
{
    uint32_t mask = ITH_MEMDBG_AXI0INT_EN_MASK |
        ITH_MEMDBG_AXI1INT_EN_MASK |
        ITH_MEMDBG_AXI2INT_EN_MASK |
        ITH_MEMDBG_AXI3INT_EN_MASK;
    ithWriteRegMaskA(ITH_MEMDBG0 + ITH_MEMDBG_EN_REG, mask, mask);
    ithWriteRegMaskA(ITH_MEMDBG1 + ITH_MEMDBG_EN_REG, mask, mask);
}

/**
 * Enables memory debug.
 *
 * @param module The memory debug module.
 */
static inline void ithMemDbgEnable(ITHMemDbgModule module)
{
    ithSetRegBitA(module + ITH_MEMDBG_EN_REG, ITH_MEMDBG_EN_BIT);
}

/**
 * Disables memory debug.
 *
 * @param module The memory debug module.
 */
static inline void ithMemDbgDisable(ITHMemDbgModule module)
{
    ithClearRegBitA(module + ITH_MEMDBG_EN_REG, ITH_MEMDBG_EN_BIT);
}

/**
 * Sets memory debug mode.
 *
 * @param module The memory debug module.
 * @param mode The memory debug mode.
 */
static inline void ithMemDbgSetMode(ITHMemDbgModule module, ITHMemDbgMode mode)
{
    ithWriteRegMaskA(module + ITH_MEMDBG_EN_REG, mode << ITH_MEMDBG_MODE_BIT, ITH_MEMDBG_MODE_MASK);
}

/**
 * Sets memory debug range.
 *
 * @param module The memory debug module.
 * @param topAddr The top address.
 * @param bottomAddr The bottom address.
 */
static inline void ithMemDbgSetRange(ITHMemDbgModule module, uint32_t topAddr, uint32_t bottomAddr)
{
    ithWriteRegA(module + ITH_MEMDBG_ADDR_LO_REG, topAddr);
    ithWriteRegA(module + ITH_MEMDBG_ADDR_HI_REG, bottomAddr);
}

/**
 * Gets memory debug status.
 *
 * @return The status.
 */
static inline uint16_t ithMemDbgGetStatus(void)
{
    return 0;
}

/**
 * Gets memory debug status.
 *
 * @param module The memory debug module.
 * @return The flag.
 */
static inline int32_t ithMemDbgGetFlag(ITHMemDbgModule module, uint32_t *id0,
    uint32_t *id1, uint32_t *id2, uint32_t *id3)
{
    uint32_t id_status = ithReadRegA(module + ITH_MEMDBG_STATUS_REG);
    uint32_t int_status = ithReadRegA(module + ITH_MEMDBG_EN_REG);

    (*id0) = ITH_MEMDBG_PASSID_AXI0_NONE;
    (*id1) = ITH_MEMDBG_PASSID_AXI1_NONE;
    (*id2) = ITH_MEMDBG_PASSID_AXI2_NONE;
    (*id3) = ITH_MEMDBG_PASSID_AXI3_NONE;

    if (!(int_status & ITH_MEMDBG_INT_STATUS_MASK))
        return -1;

    if (int_status & ITH_MEMDBG_AXI0INT_S_MASK)
        (*id0) = (id_status & ITH_MEMDBG_AXI0ID_MASK) >> ITH_MEMDBG_AXI0ID_BIT;
    if (int_status & ITH_MEMDBG_AXI1INT_S_MASK)
        (*id1) = (id_status & ITH_MEMDBG_AXI1ID_MASK) >> ITH_MEMDBG_AXI1ID_BIT;
    if (int_status & ITH_MEMDBG_AXI2INT_S_MASK)
        (*id2) = (id_status & ITH_MEMDBG_AXI2ID_MASK) >> ITH_MEMDBG_AXI2ID_BIT;
    if (int_status & ITH_MEMDBG_AXI3INT_S_MASK)
        (*id3) = (id_status & ITH_MEMDBG_AXI3ID_MASK) >> ITH_MEMDBG_AXI3ID_BIT;

    return 0;
}

static inline void ithMemDbgSetPassId0(ITHMemDbgModule module, uint32_t id0,
    uint32_t id1, uint32_t id2, uint32_t id3)
{
    ithWriteRegMaskA(module + ITH_MEMDBG_PASSID0_REG, id0 << ITH_MEMDBG_PASSID_AXI0_BIT, ITH_MEMDBG_PASSID_AXI0_MASK);
    ithWriteRegMaskA(module + ITH_MEMDBG_PASSID0_REG, id1 << ITH_MEMDBG_PASSID_AXI1_BIT, ITH_MEMDBG_PASSID_AXI1_MASK);
    ithWriteRegMaskA(module + ITH_MEMDBG_PASSID0_REG, id2 << ITH_MEMDBG_PASSID_AXI2_BIT, ITH_MEMDBG_PASSID_AXI2_MASK);
    ithWriteRegMaskA(module + ITH_MEMDBG_PASSID0_REG, id3 << ITH_MEMDBG_PASSID_AXI3_BIT, ITH_MEMDBG_PASSID_AXI3_MASK);
}

static inline void ithMemDbgSetPassId1(ITHMemDbgModule module, uint32_t id0,
    uint32_t id1, uint32_t id2, uint32_t id3)
{
    ithWriteRegMaskA(module + ITH_MEMDBG_PASSID1_REG, id0 << ITH_MEMDBG_PASSID_AXI0_BIT, ITH_MEMDBG_PASSID_AXI0_MASK);
    ithWriteRegMaskA(module + ITH_MEMDBG_PASSID1_REG, id1 << ITH_MEMDBG_PASSID_AXI1_BIT, ITH_MEMDBG_PASSID_AXI1_MASK);
    ithWriteRegMaskA(module + ITH_MEMDBG_PASSID1_REG, id2 << ITH_MEMDBG_PASSID_AXI2_BIT, ITH_MEMDBG_PASSID_AXI2_MASK);
    ithWriteRegMaskA(module + ITH_MEMDBG_PASSID1_REG, id3 << ITH_MEMDBG_PASSID_AXI3_BIT, ITH_MEMDBG_PASSID_AXI3_MASK);
}

/**
 * Clear interrupt.
 */
static inline void ithMemDbgClearIntr(void)
{
    ithSetRegBitA(ITH_MEMDBG0 + ITH_MEMDBG_EN_REG, ITH_MEMDBG_CLR_BIT);
    ithSetRegBitA(ITH_MEMDBG1 + ITH_MEMDBG_EN_REG, ITH_MEMDBG_CLR_BIT);
}

#ifdef __cplusplus
}
#endif

#endif // ITH_MEMDGB_H
/** @} */ // end of ith_memdbg
/** @} */ // end of ith