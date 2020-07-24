#ifndef ITH_MEMSTAT_H
#define ITH_MEMSTAT_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_memstat Memory Statistic
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Memory Statistic request definition.
 */
typedef enum
{
    ITH_MEMSTAT_I2S_READ    = 0x0,  ///< I2S read request
    ITH_MEMSTAT_I2S_WRITE   = 0x1,  ///< I2S write request
    ITH_MEMSTAT_TSI_WRITE   = 0x2,  ///< TSI write request
    ITH_MEMSTAT_HOST_WRITE  = 0x3,  ///< Host bus controller write request
    ITH_MEMSTAT_HOST_READ   = 0x4,  ///< Host bus controller read request
    ITH_MEMSTAT_LCD_READ    = 0x5,  ///< LCD controller read request
    ITH_MEMSTAT_ISP_READ    = 0x6,  ///< ISP read request
    ITH_MEMSTAT_CAP_READ    = 0x7,  ///< CAP write request
    ITH_MEMSTAT_CMDQ_READ   = 0x8,  ///< Command queue read request
    ITH_MEMSTAT_IQUEUE_READ = 0x9,  ///< I queue read request
    ITH_MEMSTAT_USB         = 0xA,  ///< USB controller read/write request
    ITH_MEMSTAT_ARM         = 0xB,  ///< ARM read/write request
    ITH_MEMSTAT_VIDEO_READ  = 0xC,  ///< Video engine read request
    ITH_MEMSTAT_VIDEO_WRITE = 0xD,  ///< Video engine write request
    ITH_MEMSTAT_ISP_WRITE   = 0xE,  ///< ISP write request
    ITH_MEMSTAT_TS          = 0xF,  ///< TS parser read/write request
    ITH_MEMSTAT_RISC        = 0x10, ///< RISC read/write request
    ITH_MEMSTAT_AHB         = 0x11, ///< AHB read/write request
    ITH_MEMSTAT_UENC_READ   = 0x12, ///< UEncoder engine read request
    ITH_MEMSTAT_UENC_WRITE  = 0x13, ///< UEncoder engine write request
    ITH_MEMSTAT_OPENVG      = 0x14, ///< Open VG read/write request
    ITH_MEMSTAT_MEMBIST     = 0x15, ///< Memory BIST read/write request
    ITH_MEMSTAT_MEMBIST1    = 0x16  ///< Memory BIST1 read/write request
} ITHMemStatRequest;

/**
 * Sets memory access service count period.
 *
 * @param period The memory access service count period.
 */
static inline void ithMemStatSetServCountPeriod(uint16_t period)
{
}

/**
 * Enables Memory access service counter.
 */
static inline void ithMemStatServCounterEnable(void)
{
}

/**
 * Disables Memory access service counter.
 */
static inline void ithMemStatServCounterDisable(void)
{
}

/**
 * Sets memory service0 request.
 *
 * @param rq The memory stat request.
 */
static inline void ithMemStatSetServ0Request(ITHMemStatRequest rq)
{
}

/**
 * Sets memory service1 request.
 *
 * @param rq The memory stat request.
 */
static inline void ithMemStatSetServ1Request(ITHMemStatRequest rq)
{
}

/**
 * Gets memory service access number.
 *
 * @return The memory service access number.
 */
static inline uint32_t ithMemStatGetServNum(void)
{
    return 0;
}

/**
 * Gets memory service0 access number.
 *
 * @return The memory service0 access number.
 */
static inline uint32_t ithMemStatGetServ0Num(void)
{
    return 0;
}

/**
 * Gets memory service0 access number.
 *
 * @return The memory service0 access number.
 */
static inline uint32_t ithMemStatGetServ1Num(void)
{
    return 0;
}

/**
 * Gets memory all service requests time counts in indication count period.
 *
 * @return The memory all service requests time counts in indication count period.
 */
static inline uint32_t ithMemStatGetAllServCount(void)
{
    return 0;
}

/**
 * Gets memory time counts in indication count period.
 *
 * @return The memory time counts in indication count period.
 */
static inline uint32_t ithMemStatGetServCount(void)
{
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif // ITH_MEMSTAT_H
/** @} */ // end of ith_memstat
/** @} */ // end of ith