#ifndef SARADC_ERROR_H
#define SARADC_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                Constant Definition
//=============================================================================
typedef enum SARADC_RESULT_TAG
{
    SARADC_SUCCESS,
    SARADC_ERR_INVALID_INPUT_PARAM,
    SARADC_ERR_INVALID_GPIO_SETTING,
    SARADC_ERR_NOT_INITIALIZE,
    SARADC_ERR_NOT_IDLE,
    SARADC_ERR_IS_IDLE,
    SARADC_ERR_INVALID_XAIN,
    SARADC_ERR_ALLOCATION_FAIL,
    SARADC_ERR_OFFSET_OUT_OF_RANGE,
    SARADC_ERR_INVALID_CALIBRATION
} SARADC_RESULT;

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif /* SARADC_ERROR_H */

