#ifndef ITH_CHIPID_H
#define ITH_CHIPID_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_chipid Chip ID
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    ITH_IT9072    = 0x6,   ///< 128 LQFP
    ITH_IT9076    = 0x4,   ///< 128 LQFP
    ITH_IT9078    = 0x1,   ///< 216 LQFP
    ITH_IT9079    = 0x5,   ///< 176 TQFP
    ITH_IT9079H   = 0x7,   ///< 144 TQFP
    ITH_IT9852_WB = 0x4,   ///< 128 LQFP
    ITH_IT9852_ET = 0x5,   ///< 128 LQFP
    ITH_IT9856_WB = 0x6,   ///< 144 TQFP
    ITH_IT9856_ET = 0x7,   ///< 144 TQFP
    ITH_IT9854_WB = 0x0,   ///< 128 LQFP
    ITH_IT9854_ET = 0x1,   ///< 128 LQFP
    ITH_IT9866_WB = 0x2,   ///< 144 TQFP
    ITH_IT9866_ET = 0x3    ///< 144 TQFP
} ITHPackageId;

/**
 * Gets device ID.
 *
 * @return The device ID.
 */
static inline unsigned int ithGetDeviceId(void)
{
    return (ithReadRegA(ITH_HOST_BASE + ITH_REVISION_ID_REG) >> 16);
}

/**
 * Gets device revision ID.
 *
 * @return The device revision ID.
 */
static inline unsigned int ithGetRevisionId(void)
{
    return (ithReadRegA(ITH_HOST_BASE + ITH_REVISION_ID_REG) & 0x0FFF);
}


/**
 * Gets device package ID.
 *
 * @return The device package ID.
 */
ITHPackageId ithGetPackageId(void);

#ifdef __cplusplus
}
#endif

#endif // ITH_CHIPID_H
/** @} */ // end of ith_chipid
/** @} */ // end of ith