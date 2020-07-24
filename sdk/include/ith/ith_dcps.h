#ifndef ITH_DCPS_H
#define ITH_DCPS_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_dcps Decompress
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * the structure of the decompress status.
 */
typedef struct
{
    uint32_t DcpsSetIndex;      ///< The number that the function "ithDcpsStart()" has been called.
    uint32_t DcpsDoneIndex;     ///< The number that the function "ithDcpsStart()" has been finished.
    uint32_t CmdQueSetIndex;    ///< The number that command queue has been set.
    uint32_t CmdQueDoneIndex;   ///< The number that command queue has been finished.
    uint32_t DcpsStatus;        ///< The status flag for debug
} ITH_DCPS_STATUS, *pITH_DCPS_STATUS;

/**
 * the structure of the decompress parameters.
 */
typedef struct
{
    uint8_t  *srcbuf;           ///< The source buffer address.
    uint8_t  *dstbuf;           ///< The destination buffer address.
    uint32_t srcLen;            ///< The source buffer length.
    uint32_t dstLen;            ///< The destination buffer length.
    uint32_t BlkSize;           ///<
    uint32_t TotalCmdqCount;    ///<
    uint32_t RegDcpsCmdqCnt;    ///<
    uint32_t RegDcpsStatus;     ///<
    uint8_t  IsEnableComQ;      ///< The flag of enabling command queue function.(0 is disable)
    uint8_t  DcpsMode;          ///< The flag of enabling UCL/BRFLZ.(0 is UCL, 1:BRFLZ)
    uint8_t  LzCpsBytePerPxl;   ///< The flag of enabling UCL/BRFLZ.(0 is UCL, 1:BRFLZ)
} ITH_DCPS_INFO, *pITH_DCPS_INFO;

/**
 * To initialize the decompress driver.
 *
 * @param DcpsInfo
 */
void ithDcpsInit(ITH_DCPS_INFO *DcpsInfo);

/**
 * To Enable decompress interrupt.
 */
void ithDcpsEnIntr(void);

/**
 * To Disable decompress interrupt.
 */
void ithDcpsDisIntr(void);

/**
 * To clear decompress interrupt.
 */
void ithDcpsClearIntr(void);

/**
 * To get the counter that command queue has excuted for decompressing.
 *
 * @param DcpsInfo The value that command queue has excuted for decompressing.
 */
void ithDcpsGetCmdqCount(ITH_DCPS_INFO *DcpsInfo);

/**
 * To get the counter that command queue has excuted for decompressing.
 *
 * @param DcpsDoneLen  The bytes that has decompressed.
 */
void ithDcpsGetDoneLen(uint32_t *DcpsDoneLen);

/**
 * Start to execute decompressing function.
 *
 * @param DcpsInfo the strucure contains all parameters of decompressing
 * @return true if succeed, false if failed.
 */
bool ithDcpsFire(ITH_DCPS_INFO *DcpsInfo);

/**
 * To get the status of decompress.
 *
 * @param DcpsInfo The current decompress status.
 */
void ithDcpsGetStatus(ITH_DCPS_INFO *DcpsInfo);

/**
 * To get the current decompress status.
 *
 * @param DcpsInfo The current value of register "ITH_DCPS_REG_DSR".
 */
void ithDcpsWait(ITH_DCPS_INFO *DcpsInfo);

/**
 * To terminate the decompress driver.
 */
void ithDcpsExit(void);

/**
 * Suspends DCPS module.
 */
void ithDcpsSuspend(void);

/**
 * Resumes DCPS module.
 */
void ithDcpsResume(void);

/**
 * Enable Decompress clock.
 */
void ithDcpsEnableClock(void);

/**
 * Disable Decompress clock.
 */
void ithDcpsDisableClock(void);

/**
 * Reset Decompress Engine.
 */
void ithDcpsResetEngine(void);

#ifdef __cplusplus
}
#endif

#endif // ITH_DCPS_H
/** @} */ // end of ith_dcps
/** @} */ // end of ith