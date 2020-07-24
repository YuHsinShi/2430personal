#ifndef ITH_AHB0_H
#define ITH_AHB0_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_ahb0 AHB0
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Set AHB0 control register as default value.
 */
static inline void ithAhb0SetCtrlReg(void)
{
    ithWriteRegA(ITH_AHB0_BASE + ITH_AHB0_CTRL_REG, ITH_AHB0_CTRL_REG_DEFAULT);
}

#ifdef __cplusplus
}
#endif

#endif // ITH_AHB0_H
/** @} */ // end of ith_ahb0
/** @} */ // end of ith