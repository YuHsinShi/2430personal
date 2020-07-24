#ifndef ITH_ISP_H
#define ITH_ISP_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_isp ISP
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ISP core definition.
 */
typedef enum
{
    ITH_ISP_CORE0,   ///< ISP Core 0
    ITH_ISP_CORE1    ///< ISP Core 1
} ITHIspCore;

/**
 * ISP tiling mode.
 */
typedef enum
{
    ITH_ISP_TILING_READ,   ///< Read Operation
    ITH_ISP_TILING_WRITE   ///< Write Operation
} ITHIspTilingMode;

/**
 * ISP tiling width.
 */
typedef enum
{
    ITH_ISP_TILING_WIDTH_64,   ///< 64 byte
    ITH_ISP_TILING_WIDTH_128   ///< 128 byte
} ITHIspTilingWidth;

/**
 * ISP tiling pitch.
 */
typedef enum
{
    ITH_ISP_TILING_PITCH_1536,   ///< 1536 byte
    ITH_ISP_TILING_PITCH_2048,   ///< 2048 byte
    ITH_ISP_TILING_PITCH_3072,   ///< 3072 byte
    ITH_ISP_TILING_PITCH_4096,   ///< 4096 byte
    ITH_ISP_TILING_PITCH_6144,   ///< 6144 byte
    ITH_ISP_TILING_PITCH_8192    ///< 8192 byte
} ITHIspTilingPitch;

/**
 * Enables ISP's shared clock.
 */
void ithIspEnableSharedClock(void);

/**
 * Disables ISP's shared clock.
 */
void ithIspDisableSharedClock(void);

/**
 * Enables ISP's clock.
 */
void ithIspEnableClock();

/**
 * Disables ISP's clock.
 */
void ithIspDisableClock();

/**
 * Sets ISP's clock source.
 * 
 * @param src
 */
void ithIspSetClockSource(ITHClockSource src);

/**
 * Resets ISP's registers.
 * 
 * @param ispcore
 */
void ithIspResetReg(ITHIspCore ispcore);

/**
 * Resets all ISP's registers.
 */
void ithIspResetAllReg(void);

void ithIspQResetReg(void);

/**
 * Resets ISP's engine.
 * 
 * @param ispcore
 */
void ithIspResetEngine(ITHIspCore ispcore);

/**
 * Gets ISP's clock source.
 * 
 * @return
 */
static inline ITHClockSource ithIspGetClockSource(void)
{
    return (ITHClockSource)((ithReadRegA(ITH_HOST_BASE + ITH_ISP_CLK_REG) & ITH_ICLK_SRC_SEL_MASK) >> ITH_ICLK_SRC_SEL_BIT);
}

/**
 * Enables ISP's tiling.
 * 
 * @param mode
 * @param width
 * @param pitch
 */
void ithIspEnableTiling(ITHIspTilingMode mode, ITHIspTilingWidth width, ITHIspTilingPitch pitch);

/**
 * Disables ISP's tiling.
 * 
 * @param mode
 */
void ithIspDisableTiling(ITHIspTilingMode mode);

#ifdef __cplusplus
}
#endif

#endif // ITH_ISP_H
/** @} */ // end of ith_isp
/** @} */ // end of ith