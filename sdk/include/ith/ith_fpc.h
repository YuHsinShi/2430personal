#ifndef ITH_FPC_H
#define ITH_FPC_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_fpc FPC & STC
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Gets base clock of STC.
 *
 * @return the base clock of STC.
 */
static inline uint32_t ithStcGetBaseClock(void)
{
    return 0;
}

typedef enum
{
    STATE_FREE,
    STATE_STOP,
    STATE_PAUSE,
    STATE_RUN
} STC_STATE;

typedef struct STCInfo_TAG
{
    STC_STATE state;
    int64_t   offset;
    uint64_t  last_pause;
    uint64_t  pause_duration;
    uint64_t  duration;
    uint32_t  stcBaseCountHi;
} STCInfo;

#define STC_MAX_CNT 4

/**
 * Gets base clock of STC. (64-bits)
 *
 * @return the base clock of STC. (64-bits)
 */
uint64_t ithStcGetBaseClock64(STCInfo *pstc_info);

#ifdef __cplusplus
}
#endif

#endif // ITH_FPC_H
/** @} */ // end of ith_fpc
/** @} */ // end of ith