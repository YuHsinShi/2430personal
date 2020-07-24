#ifndef ITH_NAND_H
#define ITH_NAND_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_nand NAND
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * NAND page size definition.
 */
typedef enum
{
    ITH_NAND_512 = 0,       ///< 512 bytes
    ITH_NAND_2K  = 1,       ///< 2k
    ITH_NAND_4K  = 2,       ///< 4k
    ITH_NAND_8K  = 3        ///< 8k
} ITHNandPageSize;

/**
 * Gets current NAND page size
 *
 * @return the page size.
 */
static inline ITHNandPageSize ithNandGetPageSize(void)
{
    return (ITHNandPageSize)((ithReadRegA(ITH_NAND_BASE + ITH_NAND_AUTOBOOTCFG_REG) & ITH_NAND_AUTOBOOTCFG_MASK) >> ITH_NAND_AUTOBOOTCFG_BIT);
}

/**
 * Suspends NAND module.
 */
void ithNandSuspend(void);

/**
 * Resumes from suspend mode for NAND module.
 */
void ithNandResume(void);

/**
 * Enable NAND clock.
 */
void ithNandEnableClock(void);

/**
 * Disable NAND clock.
 */
void ithNandDisableClock(void);

#ifdef __cplusplus
}
#endif

#endif // ITH_NAND_H
/** @} */ // end of ith_nand
/** @} */ // end of ith