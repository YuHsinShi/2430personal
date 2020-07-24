#ifndef ITH_HOST_H
#define ITH_HOST_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_host Host
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Enters suspend mode on host module.
 */
void ithHostSuspend(void);

/**
 * Resumes from suspend mode on host module.
 */
void ithHostResume(void);

#ifdef __cplusplus
}
#endif

#endif // ITH_HOST_H
/** @} */ // end of ith_host
/** @} */ // end of ith