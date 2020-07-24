#ifndef ITH_UIENC_H
#define ITH_UIENC_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_uienc Ui Encode
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Enables the clock of Ui Encode module.
 */
void ithUiEncEnableClock(void);

/**
 * Disables the clock of Ui Encod module.
 */
void ithUiEncDisableClock(void);

/**
 * Resets the regisers of Ui Encod module.
 */
void ithUiEncResetReg(void);

/**
 * Resets the engine of Ui Encod module.
 */
void ithUiEncResetEngine(void);

#ifdef __cplusplus
}
#endif

#endif // ITH_UIENC_H
/** @} */ // end of ith_uienc
/** @} */ // end of ith