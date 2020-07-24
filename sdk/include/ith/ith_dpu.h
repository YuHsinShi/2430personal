#ifndef ITH_DPU_H
#define ITH_DPU_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_dpu encryption/descryption
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    AES_ECB_MODE = 0,
    AES_CBC_MODE,
    AES_OFB_MODE,
    AES_CFB_MODE,
    AES_CTR_MODE,
    DES_ECB_MODE,
    DES_CBC_MODE,
    DES_OFB_MODE,
    DES_CFB_MODE,
    DES_CTR_MODE,
    DES3_ECB_MODE,
    DES3_CBC_MODE,
    DES3_OFB_MODE,
    DES3_CFB_MODE,
    DES3_CTR_MODE,
    CSA_MODE,
    CRC_MODE,
    UNKOWN_MODE
} ITH_DPU_MODE;

/**
 * To set the dpu control register by DPU mode and cipher.
 */
void ithDpuClearCtrl(void);

/**
 * To clear interrupt.
 */
void ithDpuClearIntr(void);

/**
 * To set the dpu control register by DPU mode and cipher.
 */
void ithDpuInitCtrl(ITH_DPU_MODE DpuMode);

/**
 * To set the dpu CRC initial value as 0xFFFFFFFF.
 */
void ithDpuInitCrc(void);

/**
 * To enable DPU interrupt function
 */
void ithDpuEnableIntr(void);

/**
 * To disable DPU interrupt function
 */
void ithDpuDisableIntr(void);

/**
 * To set the dpu CRC initial value as 0xFFFFFFFF.
 *
 * @param SrcAddr the source address
 */
void ithDpuSetSrcAddr(unsigned int SrcAddr);

/**
 * To set the dpu CRC result.
 *
 * @param Reg32 the source address
 */
void ithDpuGetCrcValue(unsigned int *Reg32);

/**
 * To set the dpu CRC data value to calculate a new CRC vlaue.
 *
 * @param CrcData the CRC data
 */
void ithDpuSetCrcData(unsigned int CrcData);

/**
 * To set the dpu CRC initial value as 0xFFFFFFFF.
 *
 * @param DstAddr the distnation address
 */
void ithDpuSetDstAddr(unsigned int DstAddr);

/**
 * To set the dpu as encryption mode.
 */
void ithDpuSetEncrypt(void);

/**
 * To set the dpu as descryption mode.
 */
void ithDpuSetDescrypt(void);

/**
 * To fire dpu engine.
 */
void ithDpuFire(void);

/**
 * To set the dpu excution size.
 *
 * @param size the DPU excution size
 */
void ithDpuSetSize(unsigned int size);

/**
 * To set the encryption key.
 *
 * @param key the pointer of key
 * @param len the key length(in bit)
 */
void ithDpuSetKey(uint32_t *key, uint8_t len);

/**
 * To set the encryption vector.
 *
 * @param vector the pointer of vector
 * @param len the vector length(in bit)
 */
void ithDpuSetVector(uint32_t *vector, uint8_t len);

/**
 * Suspends DPU module.
 */
void ithDpuSuspend(void);

/**
 * Resumes DPU module.
 */
void ithDpuResume(void);

/**
 * To wait the dpu engine done.
 *
 * @return true if succeed, false if failed.
 */
bool ithDpuWait(void);

/**
 * Set DPU endian.
 *
 * @param EndianIndex the endian index
 */
void ithDpuSetDpuEndian(unsigned int EndianIndex);

/**
 * Enable DPU clock.
 */
void ithDpuEnableClock(void);

/**
 * Disable DPU clock.
 */
void ithDpuDisableClock(void);

#ifdef __cplusplus
}
#endif

#endif // ITH_DPU_H
/** @} */ // end of ith_dpu
/** @} */ // end of ith