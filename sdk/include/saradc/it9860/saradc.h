#ifndef SARADC_H
#define SARADC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "saradc/saradc_type.h"
#include "saradc/saradc_error.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define SARADC_XAIN_MAX_COUNT    8
#define SARADC_WB_SIZE_ALIGN     256

//=============================================================================
//                Macro Definition
//=============================================================================
#if defined(WIN32)

    #define SARADC_EXPORTS
    #if defined(SARADC_EXPORTS)
        #define SARADC_API __declspec(dllexport)
    #else
        #define SARADC_API __declspec(dllimport)
    #endif

#else
    #define SARADC_API extern
#endif

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct SARADC_IO_MAPPING_ENTRY_TAG
{
    int gpioPin;
    int gpioMode;
} SARADC_IO_MAPPING_ENTRY;

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
//=============================================================================
/**
 * SARADC initialization.
 *
 * @param modeAVG            set average notification.
 * @param modeStore          set DRAM storing type.
 * @param amplifyDriving     determine XAIN driving.
 * @param divider            set clock divider of SARADC.
 * @return SARADC_SUCCESS if succeed, error codes of SARADC_ERR otherwise.
 */
//=============================================================================
SARADC_API SARADC_RESULT
mmpSARInitialize(
    SARADC_MODE_AVG     modeAVG,
    SARADC_MODE_STORE   modeStore,
    SARADC_AMPLIFY_GAIN amplifyDriving,
    SARADC_CLK_DIV      divider);

//=============================================================================
/**
 * Set Write buffer size of SARADC.
 *
 * @param    wbSize set Write buffer range for SARADC accessing.
 * @return SARADC_SUCCESS if succeed, error codes of SARADC_ERR otherwise.
 */
//=============================================================================
SARADC_API SARADC_RESULT
mmpSARSetWriteBufferSize(
    uint16_t wbSize);

//=============================================================================
/**
 * Set average calculation count of SARADC.
 *
 * @param avgCalCount    set average calculation count for SARADC reference
 * @return SARADC_SUCCESS if succeed, error codes of SARADC_ERR otherwise.
 */
//=============================================================================
SARADC_API SARADC_RESULT
mmpSARSetAVGCalCount(
    SARADC_AVG_CAL_COUNT avgCalCount);

//=============================================================================
/**
 * Set store average count of SARADC trigger.
 *
 * @param trigStoreCount    set store average count for SARADC reference, 0 means always
 * @return SARADC_SUCCESS if succeed, error codes of SARADC_ERR otherwise.
 */
//=============================================================================
SARADC_API SARADC_RESULT
mmpSARSetTRIGStoreCount(
    uint8_t trigStoreCount);

//=============================================================================
/**
 * Select XAIN input signal of SARADC.
 *
 * @param signal    set XAIN calibration signal for SARADC reference
 * @return SARADC_SUCCESS if succeed, error codes of SARADC_ERR otherwise.
 */
//=============================================================================
SARADC_API SARADC_RESULT
mmpSARSelectXAINSignal(
    SARADC_XAIN_SIGNAL signal);

//=============================================================================
/**
* SARADC termination.
*
* @return SARADC_SUCCESS if succeed, error codes of SARADC_ERR otherwise.
*/
//=============================================================================
SARADC_API SARADC_RESULT
mmpSARTerminate(
    void);

//=============================================================================
/**
 * Fire SARADC engine.
 *
 * @return SARADC_SUCCESS if succeed, error codes of SARADC_ERR otherwise.
 */
//=============================================================================
SARADC_API SARADC_RESULT
mmpSARFire(
    void);

//=============================================================================
/**
 * Stop SARADC engine.
 *
 * @return SARADC_SUCCESS if succeed, error codes of SARADC_ERR otherwise.
 */
//=============================================================================
SARADC_API SARADC_RESULT
mmpSARStop(
    void);

//=============================================================================
/**
 * Set XAIN memory base of SARADC.
 *
 * @param hwPort      indicate which XAIN will be applied.
 * @param baseAddr    determine memory base of XAIN.
 * @return SARADC_SUCCESS if succeed, error codes of SARADC_ERR otherwise.
 */
//=============================================================================
SARADC_API SARADC_RESULT
mmpSARSetMEMBase(
    SARADC_PORT hwPort,
    uint8_t     *baseAddr);

//=============================================================================
/**
 * Set XAIN trigger rule of SARADC.
 *
 * @param hwPort           indicate which XAIN will be applied.
 * @param trigAVG          determine average trigger type of XAIN.
 * @param maxAVGTrigger    determine max trigger rule of XAIN.
 * @param minAVGTrigger    determine min trigger rule of XAIN.
 * @return SARADC_SUCCESS if succeed, error codes of SARADC_ERR otherwise.
 */
//=============================================================================
SARADC_API SARADC_RESULT
mmpSARSetAVGTriggerRule(
    SARADC_PORT     hwPort,
    SARADC_TRIG_AVG trigAVG,
    uint16_t        maxAVGTrigger,
    uint16_t        minAVGTrigger);

//=============================================================================
/**
 * Set XAIN detection rule of SARADC.
 *
 * @param hwPort          indicate which XAIN will be applied.
 * @param maxAVGDetect    determine max detection rule of XAIN.
 * @param minAVGDetect    determine min detection rule of XAIN.
 * @return SARADC_SUCCESS if succeed, error codes of SARADC_ERR otherwise.
 */
//=============================================================================
SARADC_API SARADC_RESULT
mmpSARSetAVGDetectRule(
    SARADC_PORT hwPort,
    uint16_t    maxAVGDetect,
    uint16_t    minAVGDetect);

//=============================================================================
/**
 * Set SARADC Read pointer.
 *
 * @param hwPort    indicate which XAIN will be applied.
 * @param rptr      determine Read pointer offset of XAIN.
 * @return SARADC_SUCCESS if succeed, error codes of SARADC_ERR otherwise.
 */
//=============================================================================
SARADC_API SARADC_RESULT
mmpSARSetReadPointer(
    SARADC_PORT hwPort,
    uint16_t    rptr);

//=============================================================================
/**
 * Read SARADC Write pointer.
 *
 * @param hwPort    indicate which XAIN will be applied.
 * @param wptr      return Write pointer offset that XAIN recorded.
 * @return SARADC_SUCCESS if succeed, error codes of SARADC_ERR otherwise.
 */
//=============================================================================
SARADC_API SARADC_RESULT
mmpSARReadWritePointer(
    SARADC_PORT hwPort,
    uint16_t    *wptr);

//=============================================================================
/**
 * Read SARADC average register.
 *
 * @param hwPort    indicate which XAIN will be applied.
 * @param avg       return average value that XAIN recorded.
 * @return SARADC_SUCCESS if succeed, error codes of SARADC_ERR otherwise.
 */
//=============================================================================
SARADC_API SARADC_RESULT
mmpSARReadAVGREG(
    SARADC_PORT hwPort,
    uint16_t    *avg);

//=============================================================================
/**
 * Check Overwrite Interrupt of SARADC.
 *
 * @param hwPort    indicate which XAIN will be checked.
 * @return true if memory base has been overwritten, false otherwise.
 */
//=============================================================================
SARADC_API bool
mmpSARIsOverwritingMEM(
    SARADC_PORT hwPort);

//=============================================================================
/**
 * Check average detection Interrupt of SARADC.
 *
 * @param hwPort    indicate which XAIN will be checked.
 * @return SARADC_INTR_AVG_VALID if digtal value was within valid range,
 * SARADC_INTR_AVG_ABOMAX and SARADC_INTR_AVG_UNDMIN otherwise.
 */
//=============================================================================
SARADC_API SARADC_INTR_AVG
mmpSARIsOutOfRange(
    SARADC_PORT hwPort);

//=============================================================================
/**
 * Check Event occurrence Interrupt of SARADC.
 *
 * @param hwPort    indicate which XAIN will be checked.
 * @return true if event has been occurred, false otherwise.
 */
//=============================================================================
SARADC_API bool
mmpSARIsEventOccurrence(
    SARADC_PORT hwPort);

//=============================================================================
/**
 * Reset SARADC XAIN notification.
 *
 * @param hwPort    indicate which XAIN will be checked.
 * @return SARADC_SUCCESS if succeed, error codes of SARADC_ERR otherwise.
 */
//=============================================================================
SARADC_API SARADC_RESULT
mmpSARResetXAINNotification(
    SARADC_PORT hwPort);

//=============================================================================
/**
 * Reset SARADC engine.
 *
 * @return SARADC_SUCCESS if succeed, error codes of SARADC_ERR otherwise.
 */
//=============================================================================
SARADC_API SARADC_RESULT
mmpSARReset(
    void);

//=============================================================================
/**
 * Enable SARADC XAIN set.
 *
 * @param enXAIN    the bits stand for XAIN numbers from low bit to high bit.
 * @return SARADC_SUCCESS if succeed, error codes of SARADC_ERR otherwise.
 */
//=============================================================================
SARADC_API SARADC_RESULT
mmpSAREnableXAIN(
    uint8_t enXAIN);

//=============================================================================
/**
 * SARADC conversion.
 *
 * @param hwPort    indicate which XAIN will be applied.
 * @param wbSize    set Write buffer range for SARADC accessing.
 * @param avg       return average value that XAIN recorded.
 * @return SARADC_SUCCESS if succeed, error codes of SARADC_ERR otherwise.
 */
//=============================================================================
SARADC_API SARADC_RESULT
mmpSARConvert(
    SARADC_PORT hwPort,
    uint16_t    wbSize,
    uint16_t    *avg);

//=============================================================================
/**
 * Collect SARADC output.
 *
 * @param baseAddr    indicate memory base of XAIN.
 * @param wbSize      indicate Write buffer range for SARADC accessing.
 * @param rptr        indicate Read pointer offset that XAIN recorded.
 * @param wptr        indicate Write pointer offset that XAIN recorded.
 * @param avg         return average value that XAIN recorded.
 * @return SARADC_SUCCESS if succeed, error codes of SARADC_ERR otherwise.
 */
//=============================================================================
SARADC_API SARADC_RESULT
mmpSARCollectOutput(
    uint8_t  *baseAddr,
    uint16_t wbSize,
    uint16_t rptr,
    uint16_t wptr,
    uint16_t *avg);

//=============================================================================
/**
 * SARADC table calibration.
 *
 * @param input     indicate real SARADC output.
 * @param output    return table calibration output which is calculated by input.
 * @return SARADC_SUCCESS if succeed, error codes of SARADC_ERR otherwise.
 */
//=============================================================================
SARADC_API SARADC_RESULT
mmpSARTableCalibrate(
    uint16_t input,
    uint16_t *output);

#ifdef __cplusplus
}
#endif

#endif /* SARADC_H */

