#ifndef ITH_RTC_H
#define ITH_RTC_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_rtc RTC
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * RTC control definition.
 */
typedef enum
{
    ITH_RTC_EN              = 0,    ///< RTC enable
    ITH_RTC_INTR_SEC        = 1,    ///< RTC auto alarm per second
    ITH_RTC_INTR_MIN        = 2,    ///< RTC auto alarm per minute
    ITH_RTC_INTR_HOUR       = 3,    ///< RTC auto alarm per hour
    ITH_RTC_INTR_DAY        = 4,    ///< RTC auto alarm per day
    ITH_RTC_ALARM_INTR      = 5,    ///< RTC alarm interrupt
    ITH_RTC_DAY_ALARM_INTR  = 6,    ///< RTC day alarm interrupt
    ITH_RTC_WEEK_ALARM_INTR = 7,    ///< RTC week alarm interrupt
    ITH_RTC_COUNTER_LOAD    = 8,    ///< RTC counter load
    ITH_RTC_PWREN_ALARM_SEL = 15,   ///< PWREN Alarm Type
    ITH_RTC_PWREN_CTRL1     = 16,   ///< PWREN Alarm Control
    ITH_RTC_SW_POWEROFF     = 17,   ///< Software power down (auto set zero)
    ITH_RTC_PWREN_IO_DIR    = 21,   ///< PWREN pin driving strength
    ITH_RTC_PWREN_GPIO      = 24,   ///< PWREN GPIO Data
    ITH_RTC_RESET           = 31    ///< Asynchronous reset
} ITHRtcCtrl;

/**
 * RTC interrupt definition.
 */
typedef enum
{
    ITH_RTC_SEC   = 0,      ///< Indicate that rtc_sec interrupt has occurred.
    ITH_RTC_MIN   = 1,      ///< Indicate that rtc_min interrupt has occurred.
    ITH_RTC_HOUR  = 2,      ///< Indicate that rtc_hour interrupt has occurred.
    ITH_RTC_DAY   = 3,      ///< Indicate that rtc_day interrupt has occurred.
    ITH_RTC_ALARM = 4,      ///< Indicate that rtc_alarm interrupt has occurred.

    ITH_RTC_MAX_INTR
} ITHRtcIntr;

/**
 * RTC power enable I/O selection definition.
 */
typedef enum
{
    ITH_RTC_PWREN  = 0,     ///< PWREN
    ITH_RTC_VCC_OK = 1,     ///< VCC_OK
    ITH_RTC_INTR   = 2,     ///< RTC interrupt
    ITH_RTC_GPIO   = 3      ///< GPIO
} ITHRtcPowerEnableIoSelection;

/**
 * RTC power enable I/O selection definition.
 */
typedef enum
{
    ITH_RTC_DIV_SRC_INNER_12MHZ = 0,      ///< divider source is internal 12Mhz clock
    ITH_RTC_DIV_SRC_EXT_32KHZ   = 1,      ///< divider source is external 32kHz clock
} ITHRtcClockSource;

/**
 * Enables specified RTC control.
 *
 * @param ctrl the control to enable.
 */
static inline void ithRtcCtrlEnable(ITHRtcCtrl ctrl)
{
    ithSetRegBitA(ITH_RTC_BASE + ITH_RTC_CR_REG, ctrl);
}

/**
 * Disables specified RTC control.
 *
 * @param ctrl the control to disable.
 */
static inline void ithRtcCtrlDisable(ITHRtcCtrl ctrl)
{
    ithClearRegBitA(ITH_RTC_BASE + ITH_RTC_CR_REG, ctrl);
}

/**
 * Clears specified RTC interrupt.
 *
 * @param intr the interrupt to clear.
 */
static inline void ithRtcClearIntr(ITHRtcIntr intr)
{
    ithClearRegBitA(ITH_RTC_BASE + ITH_RTC_INTRSTATE_REG, intr);
}

/**
 * Gets the state of RTC interrupt.
 *
 * @return the state of RTC interrupt.
 */
static inline uint32_t ithRtcGetIntrState(void)
{
    return ithReadRegA(ITH_RTC_BASE + ITH_RTC_INTRSTATE_REG);
}

/**
 * Sets the RTC power enable I/O selection.
 *
 * @param sel the RTC power enable I/O selection.
 */
static inline void ithRtcSetPowerEnableIoSelection(ITHRtcPowerEnableIoSelection sel)
{
    ithWriteRegMaskA(ITH_RTC_BASE + ITH_RTC_PWREN_IOSEL_REG, sel << ITH_RTC_PWREN_IOSEL_BIT, ITH_RTC_PWREN_IOSEL_MASK);
}

/**
 * Set RTC source.
 *
 * @param clkSrc The clock source.
 */
void ithRtcSetDivSrc(ITHRtcClockSource clkSrc);

/**
 * Initializes RTC module.
 *
 * @param extClk The frequency of external clock.
 */
void ithRtcInit(unsigned long extClk);

/**
 * Enables RTC module.
 *
 * @return First booting or not.
 */
bool ithRtcEnable(void);

/**
 * Gets RTC time.
 *
 * @return the RTC time.
 */
unsigned long ithRtcGetTime(void);

/**
 * Sets RTC time.
 *
 * @param t the RTC time.
 */
void ithRtcSetTime(unsigned long t);

/**
 * Gets RTC state.
 *
 * @return the RTC state.
 */
static inline uint8_t ithRtcGetState(void)
{
    return (ithReadRegA(ITH_RTC_BASE + ITH_RTC_STATE_REG) & ITH_RTC_STATE_MASK) >> ITH_RTC_STATE_BIT;
}

/**
 * Sets RTC state.
 *
 * @return the RTC state.
 */
static inline void ithRtcSetState(uint8_t state)
{
    ithWriteRegMaskA(ITH_RTC_BASE + ITH_RTC_STATE_REG, state << ITH_RTC_STATE_BIT, ITH_RTC_STATE_MASK);
}

#ifdef __cplusplus
}
#endif

#endif // ITH_RTC_H
/** @} */ // end of ith_rtc
/** @} */ // end of ith