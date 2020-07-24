#ifndef ITH_SARADC_H
#define ITH_SARADC_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_saradc SARADC
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Enables the clock of SARADC.
 */
void ithSARADCEnableClock(void);

/**
 * Disables the clock of SARADC.
 */
void ithSARADCDisableClock(void);

/**
 * Resets the engine of SARADC.
 */
void ithSARADCResetEngine(void);

#ifdef __cplusplus
}
#endif

#endif // ITH_SARADC_H
/** @} */ // end of ith_saradc
/** @} */ // end of ith