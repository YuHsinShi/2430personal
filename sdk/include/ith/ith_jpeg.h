#ifndef ITH_JPEG_H
#define ITH_JPEG_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_jpeg JPEG
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Enables the clock of JPEG module.
 */
void ithJpegEnableClock(void);

/**
 * Disables the clock of JPEG module.
 */
void ithJpegDisableClock(void);

/**
 * Resets the regisers of JPEG module.
 */
void ithJpegResetReg(void);

/**
 * Resets the engine of JPEG module.
 */
void ithJpegResetEngine(void);

/**
 * Turn on/off jpeg tiling mode.
 * 
 * @param b_mode
 */
void ithJpegSetTilingModeOnOff(bool b_mode);

#if defined(CFG_JPEG_HW_ENABLE)
extern bool bTilingModeOn;
#endif
static inline unsigned int ithIsTilingModeOn(void)
{
//#if defined(CFG_TILING_MODE_OFF)
//  return 0;
//#else
//  #if defined(CFG_DOORBELL_INDOOR) || defined(CFG_DOORBELL_ADMIN)
//    if (ithGetRevisionId() == 0)
//      return 0;
//    else
//      return 1;
//  #else
//  return 1;
//  #endif
//#endif

#if defined(CFG_TILING_MODE_OFF)
    return 0;
#else
    if (ithReadRegH(0x300) & 0x8000)
        return 0;
    else
    {
    #if defined(CFG_JPEG_HW_ENABLE)
        if (bTilingModeOn)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    #else
        return 1;
    #endif
    }
#endif
}

static inline unsigned int ithTilingPitch(void)
{
    volatile uint16_t value;
    unsigned int      pitch;

    value = ithReadRegH(0x300) & 0x7000;

    switch (value)
    {
    case 0x3000:
        pitch = 2048;
        break;
    case 0x4000:
        pitch = 3072;
        break;
    case 0x5000:
        pitch = 4096;
        break;
    default:
        pitch = 2048;
        break;
    }

    return pitch;
}

#ifdef __cplusplus
}
#endif

#endif // ITH_JPEG_H
/** @} */ // end of ith_jpeg
/** @} */ // end of ith