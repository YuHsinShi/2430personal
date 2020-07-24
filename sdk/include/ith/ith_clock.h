#ifndef ITH_CLOCK_H
#define ITH_CLOCK_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_clock Clock
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Clock definition.
 */
typedef enum
{
    ITH_MCLK = ITH_MEM_CLK1_REG,    ///< MCLK PHY
    ITH_MCLK2 = ITH_MEM_CLK2_REG,   ///< MCLK WRAP
    ITH_NCLK = ITH_AHB_CLK_REG,     ///< NCLK
    ITH_WCLK = ITH_APB_CLK1_REG,    ///< WCLK
    ITH_FCLK = ITH_AXI_CLK_REG      ///< AXI CLK
} ITHClock;

/**
 * Clock output definition.
 */
typedef enum
{
    ITH_CLK_PLL1_OUTPUT1 = 0, ///< From PLL1 output1
    ITH_CLK_PLL1_OUTPUT2 = 1, ///< From PLL1 output2
    ITH_CLK_PLL2_OUTPUT1 = 2, ///< From PLL2 output1
    ITH_CLK_PLL2_OUTPUT2 = 3, ///< From PLL2 output2
    ITH_CLK_PLL3_OUTPUT1 = 4, ///< From PLL3 output1
    ITH_CLK_PLL3_OUTPUT2 = 5, ///< From PLL3 output2
    ITH_CLK_PLL3_OUTPUT3 = 6, ///< From PLL3 output3
    ITH_CLK_CKSYS        = 7  ///< From CKSYS (12MHz)
} ITHClockSource;

/**
 * PLL definition.
 */
typedef enum
{
    ITH_PLL1 = 0, ///< PLL1
    ITH_PLL2 = 1, ///< PLL2
    ITH_PLL3 = 2  ///< PLL3
} ITHPll;

/**
 * Initialize clock module.
 */
void ithClockInit(void);

/**
 * Enter sleep mode on clock module.
 */
void ithClockSleep(void);

/**
 * Wakeup from sleep mode on clock module.
 */
void ithClockWakeup(void);

/**
 * Enter suspend mode on clock module.
 */
void ithClockSuspend(void);

/**
 * Resume from suspend mode on clock module.
 */
void ithClockResume(void);

/**
 * Gets clock source.
 *
 * @param clk The clock
 * @return The clock source
 */
static inline ITHClockSource ithClockGetSource(ITHClock clk)
{
    return (ITHClockSource)((ithReadRegH(clk) & ITH_MCLK_SRC_SEL_MASK) >> ITH_MCLK_SRC_SEL_BIT);
}

/**
 * Sets clock source.
 *
 * @param clk The clock
 * @param src The clock source to set
 */
void ithClockSetSource(ITHClock clk, ITHClockSource src);

static inline unsigned int ithClockGetRatio(ITHClock clk)
{
    return (ithReadRegA(clk) & ITH_AXICLK_RATIO_MASK) >> ITH_AXICLK_RATIO_BIT;
}

/**
 * Sets clock ratio.
 *
 * @param clk The clock
 * @param ratio The clock ratio to set.
 */
void ithClockSetRatio(ITHClock clk, unsigned int ratio);

/**
 * Gets CPU's clock ratio.
 *
 * @return The CPU clock ratio.
 */
static inline unsigned int ithGetCpuClockRatio(void)
{
    return ithClockGetRatio(ITH_FCLK);
}

/**
 * Sets CPU's clock ratio.
 *
 * @param ratio The clock ratio to set.
 */
static inline void ithSetCpuClockRatio(unsigned int ratio)
{
    ithClockSetRatio(ITH_FCLK, ratio);
}

/**
 * Sets CPU's clock ratio mutiple.
 *
 * @param ratio The recent clock ratio.
 * @param multiple The recent clock ratio`s multiple to set.
 */
static void ithSetCpuClockRatioMultiple(unsigned int ratio , unsigned int multiple)
{
    unsigned int value = 0;
    if(multiple)
    {
        value = ratio * multiple;
        ithClockSetRatio(ITH_FCLK, value);
    }else
        return;
}

/**
 * Gets memory's clock ratio.
 *
 * @return The memory clock ratio.
 */
static inline unsigned int ithGetMemClockRatio(void)
{
    return ithClockGetRatio(ITH_MCLK);
}

/**
 * Sets memory's clock ratio.
 *
 * @param ratio The clock ratio to set.
 */
static inline void ithSetMemClockRatio(unsigned int ratio)
{
    ithClockSetRatio(ITH_MCLK, ratio);
}

/**
 * Sets memory's clock ratio mutiple.
 *
 * @param ratio The recent clock ratio.
 * @param multiple The recent clock ratio`s multiple to set.
 */
static void ithSetMemClockRatioMultiple(unsigned int ratio , unsigned int multiple)
{
    unsigned int value = 0;
    if(multiple)
    {
        value = ratio * multiple;
        ithClockSetRatio(ITH_MCLK, value);
    }else
        return;
}

/**
 * Gets bus's clock ratio.
 *
 * @return The bus clock ratio.
 */
static inline unsigned int ithGetBusClockRatio(void)
{
    return ithClockGetRatio(ITH_WCLK);
}

/**
 * Sets bus's clock ratio.
 *
 * @param ratio The clock ratio to set.
 */
static inline void ithSetBusClockRatio(unsigned int ratio)
{
    ithClockSetRatio(ITH_WCLK, ratio);
}

/**
 * Sets bus's clock ratio mutiple.
 *
 * @param ratio The recent clock ratio.
 * @param multiple The recent clock ratio`s multiple to set.
 */
static void ithSetBusClockRatioMultiple(unsigned int ratio , unsigned int multiple)
{
    unsigned int value = 0;
    if(multiple)
    {
        value = ratio * multiple;
        ithClockSetRatio(ITH_WCLK, value);
    }else
        return;
}

/**
 * Gets CPU clock.
 *
 * @return The CPU clock value.
 */
unsigned int ithGetCpuClock(void);

/*
 * Get RISC CPU clock in Hz
 */
unsigned int ithGetRiscCpuClock(void);

/**
 * Gets memory clock.
 *
 * @return The memory clock value.
 */
unsigned int ithGetMemClock(void);

/**
 * Gets bus clock.
 *
 * @return The bus clock value.
 */
unsigned int ithGetBusClock(void);

/*
 * Get NCLK in Hz
 */
unsigned int ithGetNclk(void);

/*
 * Get A0CLK ~ A4CLK in Hz
 */
unsigned int ithGetA0_4clk(void);

/**
 * Enables specified PLL.
 *
 * @param pll The PLL to enable.
 */
void ithClockEnablePll(ITHPll pll);

/**
 * Disables specified PLL.
 *
 * @param pll The PLL to disable.
 */
void ithClockDisablePll(ITHPll pll);

/**
 * Clock output definition.
 */
typedef enum
{
    ITH_SP_UP       = 0, ///< up-spread
    ITH_SP_DOWN     = 1, ///< down-spread
    ITH_SP_CENTER   = 2  ///< center-spread
} ITHSpreadSpectrumMode;

/**
 * Enables spread spectrum.
 *
 * @param pll PLL to enable.
 */
static inline void ithEnableSpreadSpectrum(ITHPll pll)
{
    ithSetRegBitH(ITH_PLL1_SET6_REG + pll * 0x10, ITH_PLL1_SP_EN_BIT);
}

/**
 * Disables spread spectrum.
 *
 * @param pll PLL to disable.
 */
static inline void ithDisableSpreadSpectrum(ITHPll pll)
{
    ithClearRegBitH(ITH_PLL1_SET6_REG + pll * 0x10, ITH_PLL1_SP_EN_BIT);
}

/**
 * Sets spread spectrum parameters.
 *
 * @param pll PLL to set.
 * @param mode Spread spectrum mode.
 * @param width Modulation width in 1/1000 unit, recommand 1 ~ 5.
 * @param freq Modulation frequency in Hz, recommand 30K ~ 50KHz.
 */
void ithSetSpreadSpectrum(ITHPll pll, ITHSpreadSpectrumMode mode, uint32_t width, uint32_t freq);

/**
 * Print clock information.
 */
void ithClockStats(void);

#ifdef __cplusplus
}
#endif

#endif // ITH_CLOCK_H
/** @} */ // end of ith_clock
/** @} */ // end of ith