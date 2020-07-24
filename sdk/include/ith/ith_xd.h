#ifndef ITH_XD_H
#define ITH_XD_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_xd xD
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Suspends XD module.
 */
void ithXdSuspend(void);

/**
 * Resumes from suspend mode for XD module.
 */
void ithXdResume(void);

/**
 * Enable XD clock.
 */
void ithXdEnableClock(void);

/**
 * Disable XD clock.
 */
void ithXdDisableClock(void);

#ifdef __cplusplus
}
#endif

#endif // ITH_XD_H
/** @} */ // end of ith_xd
/** @} */ // end of ith