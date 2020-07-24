#ifndef ITH_TIMER_H
#define ITH_TIMER_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_timer Timer
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Timer definition.
 */
typedef enum
{
    ITH_TIMER1 = 0,                                     ///< Timer #1
    ITH_TIMER2 = 1,                                     ///< Timer #2
    ITH_TIMER3 = 2,                                     ///< Timer #3
    ITH_TIMER4 = 3,                                     ///< Timer #4
    ITH_TIMER5 = 4,                                     ///< Timer #5
    ITH_TIMER6 = 5,                                     ///< Timer #6
    ITH_TIMER7 = 6,                                     ///< Timer #7
    ITH_TIMER8 = 7,                                     ///< Timer #8
    ITH_TIMER9 = 8										///< Timer #9
} ITHTimer;

#include "ith/ith_timer_generic.h"

#ifdef __cplusplus
}
#endif

#endif // ITH_TIMER_H
/** @} */ // end of ith_timer
/** @} */ // end of ith