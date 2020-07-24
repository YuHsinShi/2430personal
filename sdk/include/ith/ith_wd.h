#ifndef ITH_WD_H
#define ITH_WD_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_wd Watch Dog
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Watch dog control definition.
 */
typedef enum
{
    ITH_WD_EN    = 0,       ///< The Watch Dog Timer enable
    ITH_WD_RESET = 1,       ///< The Watch Dog Timer system reset enable
    ITH_WD_INTR  = 2,       ///< The Watch Dog Timer system interrupt enable
    ITH_WD_EXT   = 3,       ///< The Watch Dog Timer external signal enable
    ITH_WD_CLOCK = 4,       ///< The Watch Dog Timer clock source
} ITHWatchDogCtrl;

/**
 * Enables specified watch dog control.
 *
 * @param ctrl the control to enable.
 */
static inline void ithWatchDogCtrlEnable(ITHWatchDogCtrl ctrl)
{
    ithSetRegBitA(ITH_WD_BASE + ITH_WD_CR_REG, ctrl);
}

/**
 * Disables specified watch dog control.
 *
 * @param ctrl the control to disable.
 */
static inline void ithWatchDogCtrlDisable(ITHWatchDogCtrl ctrl)
{
    ithClearRegBitA(ITH_WD_BASE + ITH_WD_CR_REG, ctrl);
}

/**
 * Enables watch dog.
 */
static inline void ithWatchDogEnable(void)
{
    ithWatchDogCtrlEnable(ITH_WD_EN);
}

/**
 * Disables watch dog.
 */
static inline void ithWatchDogDisable(void)
{
    ithWatchDogCtrlDisable(ITH_WD_EN);
    ithWatchDogCtrlDisable(ITH_WD_RESET);
}

/**
 * Restarts watch dog.
 */
static inline void ithWatchDogRestart(void)
{
    ithWriteRegA(ITH_WD_BASE + ITH_WD_RESTART_REG, ITH_WD_AUTORELOAD);
}

/**
 * Sets the reload value of watch dog.
 */
static inline void ithWatchDogSetReload(uint32_t count)
{
    ithWriteRegA(ITH_WD_BASE + ITH_WD_LOAD_REG, count);
}

/**
 * Gets the reload value of watch dog.
 */
static inline uint32_t ithWatchDogGetReload(void)
{
    return ithReadRegA(ITH_WD_BASE + ITH_WD_LOAD_REG);
}

/**
 * Gets the counter of watch dog.
 */
static inline uint32_t ithWatchDogGetCounter(void)
{
    return ithReadRegA(ITH_WD_BASE + ITH_WD_COUNTER_REG);
}

/**
 * Sets the timeout of watch dog (ms).
 *
 * @param ms the timeout value (ms).
 */
static inline void ithWatchDogSetTimeout(unsigned int ms)
{
    ithWatchDogSetReload((uint64_t)ithGetBusClock() * ms / 1000);
}

/**
 * Suspends watch dog module.
 */
void ithWatchDogSuspend(void);

/**
 * Resumes from suspend mode for watch dog module.
 */
void ithWatchDogResume(void);

#ifdef __cplusplus
}
#endif

#endif // ITH_WD_H
/** @} */ // end of ith_wd
/** @} */ // end of ith