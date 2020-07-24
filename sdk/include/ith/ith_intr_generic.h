#ifndef ITH_INTR_GENERIC_H
#define ITH_INTR_GENERIC_H

#ifndef ITH_INTR_H
    #error "Never include this file directly. Use ith/ith_intr.h instead."
#endif

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_intr Interrupt
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Interrupt trigger mode definition.
 */
typedef enum
{
    ITH_INTR_LEVEL = 0,  ///< Level-trigger mode
    ITH_INTR_EDGE  = 1   ///< Edge-trigger mode
} ITHIntrTriggerMode;

typedef enum
{
    ITH_INTR_HIGH_RISING = 0,       ///< Active-high level trigger or rising-edge trigger
    ITH_INTR_LOW_FALLING = 1        ///< Active-low level trigger or falling-edge trigger
} ITHIntrTriggerLevel;

/**
 * Interrupt handler.
 *
 * @arg Custom argument.
 */
typedef void (*ITHIntrHandler)(void *arg);

/**
 * Initializes interrupt module.
 */
void ithIntrInit(void);

/**
 * Resets interrupt module.
 */
void ithIntrReset(void);

/**
 * Enables specified IRQ.
 *
 * @param intr The IRQ.
 */
static inline void ithIntrEnableIrq(ITHIntr intr)
{
    ithEnterCritical();

    if (intr < 32)
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ1_EN_REG, intr);
    else
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ2_EN_REG, intr - 32);

    ithExitCritical();
}

/**
 * Disables specified IRQ.
 *
 * @param intr The IRQ.
 */
static inline void ithIntrDisableIrq(ITHIntr intr)
{
    ithEnterCritical();

    if (intr < 32)
        ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ1_EN_REG, intr);
    else
        ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ2_EN_REG, intr - 32);

    ithExitCritical();
}

/**
 * Clears specified IRQ.
 *
 * @param intr The IRQ.
 */
static inline void ithIntrClearIrq(ITHIntr intr)
{
    if (intr < 32)
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ1_CLR_REG, intr);
    else
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ2_CLR_REG, intr - 32);
}

/**
 * Gets status of IRQ.
 *
 * @param intr1 The IRQ status 1.
 * @param intr2 The IRQ status 2.
 */
static inline void ithIntrGetStatusIrq(uint32_t *intr1, uint32_t *intr2)
{
    *intr1 = ithReadRegA(ITH_INTR_BASE + ITH_INTR_IRQ1_STATUS_REG);
    *intr2 = ithReadRegA(ITH_INTR_BASE + ITH_INTR_IRQ2_STATUS_REG);
}

/**
 * Sets trigger mode of IRQ.
 *
 * @param intr The IRQ.
 * @param mode The trigger mode.
 */
void ithIntrSetTriggerModeIrq(ITHIntr intr, ITHIntrTriggerMode mode);

/**
 * Sets trigger level of IRQ.
 *
 * @param intr The IRQ.
 * @param level The trigger level.
 */
void ithIntrSetTriggerLevelIrq(ITHIntr intr, ITHIntrTriggerLevel level);

/**
 * Registers IRQ handler.
 *
 * @param intr The IRQ.
 * @param handler The callback function.
 * @param arg Custom argument to pass to handler.
 */
void ithIntrRegisterHandlerIrq(ITHIntr intr, ITHIntrHandler handler, void *arg);

/**
 * Sets software IRQ.
 *
 * @param num The IRQ0 or IRQ1.
 */
static inline void ithIntrSetSwIrq(int num)
{
    if (num)
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ2_SWINTR_REG, ITH_INTR_SWINT_BIT);
    else
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ1_SWINTR_REG, ITH_INTR_SWINT_BIT);
}

/**
 * Clears software IRQ.
 *
 * @param num The IRQ0 or IRQ1.
 */
static inline void ithIntrClearSwIrq(int num)
{
    if (num)
        ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ2_SWINTR_REG, ITH_INTR_SWINT_BIT);
    else
        ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ1_SWINTR_REG, ITH_INTR_SWINT_BIT);
}

/**
 * Dispatches IRQ to handlers.
 */
void ithIntrDoIrq(void);

/**
 * Enables specified FIQ.
 *
 * @param intr The FIQ.
 */
static inline void ithIntrEnableFiq(ITHIntr intr)
{
    if (intr < 32)
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ1_EN_REG, intr);
    else
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ2_EN_REG, intr - 32);
}

/**
 * Disables specified FIQ.
 *
 * @param intr The FIQ.
 */
static inline void ithIntrDisableFiq(ITHIntr intr)
{
    if (intr < 32)
        ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ1_EN_REG, intr);
    else
        ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ2_EN_REG, intr - 32);
}

/**
 * Clears specified FIQ.
 *
 * @param intr The FIQ.
 */
static inline void ithIntrClearFiq(ITHIntr intr)
{
    if (intr < 32)
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ1_CLR_REG, intr);
    else
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ2_CLR_REG, intr - 32);
}

/**
 * Gets status of FIQ.
 *
 * @param intr1 The FIQ status 1.
 * @param intr2 The FIQ status 2.
 */
static inline void ithIntrGetStatusFiq(uint32_t *intr1, uint32_t *intr2)
{
    *intr1 = ithReadRegA(ITH_INTR_BASE + ITH_INTR_FIQ1_STATUS_REG);
    *intr2 = ithReadRegA(ITH_INTR_BASE + ITH_INTR_FIQ2_STATUS_REG);
}

/**
 * Sets trigger mode of FIQ.
 *
 * @param intr The FIQ.
 * @param mode The trigger mode.
 */
void ithIntrSetTriggerModeFiq(ITHIntr intr, ITHIntrTriggerMode mode);

/**
 * Sets trigger level of FIQ.
 *
 * @param intr The FIQ.
 * @param level The trigger level.
 */
void ithIntrSetTriggerLevelFiq(ITHIntr intr, ITHIntrTriggerLevel level);

/**
 * Registers FIQ handler.
 *
 * @param intr The IRQ.
 * @param handler The callback function.
 * @param arg Custom argument to pass to handler.
 */
void ithIntrRegisterHandlerFiq(ITHIntr intr, ITHIntrHandler handler, void *arg);

/**
 * Sets software FIQ.
 *
 * @param num The FIQ0 or FIQ1.
 */
static inline void ithIntrSetSwFiq(int num)
{
    if (num)
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ2_SWINTR_REG, ITH_INTR_SWINT_BIT);
    else
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ1_SWINTR_REG, ITH_INTR_SWINT_BIT);
}

/**
 * Clears software FIQ.
 *
 * @param num The FIQ0 or FIQ1.
 */
static inline void ithIntrClearSwFiq(int num)
{
    if (num)
        ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ2_SWINTR_REG, ITH_INTR_SWINT_BIT);
    else
        ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ1_SWINTR_REG, ITH_INTR_SWINT_BIT);
}

/**
 * Dispatches FIQ to handlers.
 */
void ithIntrDoFiq(void);

/**
 * Suspends interrupt module.
 */
void ithIntrSuspend(void);

/**
 * Resumes from suspends for interrupt module.
 */
void ithIntrResume(void);

/**
 * Print interrupt information.
 */
void ithIntrStats(void);

#ifdef __cplusplus
}
#endif

#endif // ITH_INTR_GENERIC_H
/** @} */ // end of ith_intr
/** @} */ // end of ith