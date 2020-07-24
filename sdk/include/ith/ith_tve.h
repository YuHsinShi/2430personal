#ifndef ITH_TVE_H
#define ITH_TVE_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_tve TV Encoder
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Resets TV encoder.
 */
void ithTveReset(void);

/**
 * Enables the clock of TV encoder.
 */
void ithTveEnableClock(void);

/**
 * Disables the clock of TV encoder.
 */
void ithTveDisableClock(void);

/**
 * Enables the power of TV encoder.
 */
void ithTveEnablePower(void);

/**
 * Disables the power of TV encoder.
 */
void ithTveDisablePower(void);

#ifdef __cplusplus
}
#endif

#endif // ITH_TVE_H
/** @} */ // end of ith_tve
/** @} */ // end of ith