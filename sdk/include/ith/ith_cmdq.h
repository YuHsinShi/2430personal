#ifndef ITH_CMDQ_H
#define ITH_CMDQ_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_cmdq Command Queue
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#define ITH_CMDQ_BURST_CMD_SIZE  8    ///< Burst command header size
#define ITH_CMDQ_SINGLE_CMD_SIZE 8    ///< Single command size

/**
 * Command queue global data definition.
 */
typedef struct
{
    uint32_t addr;      ///< command queue address
    uint32_t size;      ///< command queue size
    void     *mutex;    ///< command queue mutex for lock/unlock
} ITHCmdQ;

/**
 * Command queue control definition
 */
typedef enum
{
    ITH_CMDQ_FLIPBUFMODE    = 1,     ///< Flip Buffer Mode
    ITH_CMDQ_TURBOFLIP      = 2,     ///< Turbo Flip
    ITH_CMDQ_CMDERRSTOP     = 3,     ///< Enable Cmd Error Stop
    ITH_CMDQ_BIGENDIAN      = 12,    ///< Enable Big Endian
    ITH_CMDQ_INTR           = 13,    ///< Enable CmdQ Interrupt
    ITH_CMDQ_VP0CMDAHB      = 24,    ///< Enable VP0 Cmd to AHB Interface
    ITH_CMDQ_WAITVP0BUSYAHB = 25,    ///< Enable Wait VP0 Busy in AHB Interface
    ITH_CMDQ_VP1CMDAHB      = 26,    ///< Enable VP1 Cmd to AHB Interface
    ITH_CMDQ_WAITVP1BUSYAHB = 27     ///< Enable Wait VP1 Busy in AHB Interface
} ITHCmdQCtrl;

/**
 * Makes burst command header.
 *
 * @param ptr the place to make.
 * @param addr the address to send to.
 * @param size the command size.
 */
#define ITH_CMDQ_BURST_CMD(ptr, addr, size) \
    do { \
        *(ptr)++ = (addr) | 0x80000001; \
        *(ptr)++ = (size) / sizeof(uint32_t); \
    } while (0)

/**
 * Makes single command.
 *
 * @param ptr the place to make.
 * @param addr the address to send to.
 * @param data the command data.
 */
#define ITH_CMDQ_SINGLE_CMD(ptr, addr, data) \
    do { \
        *(ptr)++ = (addr); \
        *(ptr)++ = (data); \
    } while (0)

/**
 * Global command queue instance
 */
extern ITHCmdQ *ithCmdQ;

extern ITHCmdQ *ithCmdQ1;

/**
 * Command queue port register offset definition
 */
typedef enum
{
    ITH_CMDQ0_OFFSET = 0x0,
    ITH_CMDQ1_OFFSET = ITH_CMDQ_BASE_OFFSET
} ITHCmdQPortOffset;

/**
 * Initializes command queue.
 *
 * @param cmdQ The command queue definition.
 * @param portOffset The port offset of command queue.
 */
void ithCmdQInit(ITHCmdQ* cmdQ, ITHCmdQPortOffset portOffset);

/**
 * Destroys command queue.
 *
 * @param portOffset The port offset of command queue to destroy.
 */
void ithCmdQExit(ITHCmdQPortOffset portOffset);

/**
 * Resets command queue.
 *
 * @param portOffset The port offset of command queue to reset.
 */
void ithCmdQReset(ITHCmdQPortOffset portOffset);

/**
 * Locks command queue.
 */
static inline void ithCmdQLock(ITHCmdQPortOffset portOffset)
{
    if (portOffset == ITH_CMDQ0_OFFSET)
        ithLockMutex(ithCmdQ->mutex);
    else
        ithLockMutex(ithCmdQ1->mutex);
}

/**
 * Unlocks command queue.
 */
static inline void ithCmdQUnlock(ITHCmdQPortOffset portOffset)
{
    if (portOffset == ITH_CMDQ0_OFFSET)
        ithUnlockMutex(ithCmdQ->mutex);
    else
        ithUnlockMutex(ithCmdQ1->mutex);
}

/**
 * Prints command queue status.
 */
void ithCmdQStats(void);

#ifdef __cplusplus
}
#endif

#endif // ITH_CMDQ_H
/** @} */ // end of ith_cmdq
/** @} */ // end of ith