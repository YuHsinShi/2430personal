#ifndef SARADC_TYPE_H
#define SARADC_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

//=============================================================================
//                Constant Definition
//=============================================================================
typedef enum SARADC_PORT_TAG
{
    SARADC_0,
    SARADC_1,
    SARADC_2,
    SARADC_3,
    SARADC_4,
    SARADC_5,
    SARADC_6,
    SARADC_7
} SARADC_PORT;

typedef enum SARADC_AMPLIFY_GAIN_TAG
{
    SARADC_AMPLIFY_1X,    // no amplified
    SARADC_AMPLIFY_2X,
    SARADC_AMPLIFY_4X,
    SARADC_AMPLIFY_8X
} SARADC_AMPLIFY_GAIN;

typedef enum SARADC_MODE_AVG_TAG
{
    SARADC_MODE_AVG_DISABLE,    // no average and INTR notification
    SARADC_MODE_AVG_ENABLE
} SARADC_MODE_AVG;

typedef enum SARADC_MODE_STORE_TAG
{
    SARADC_MODE_STORE_DISABLE,       // no DRAM saving
    SARADC_MODE_STORE_RAW_ENABLE,
    SARADC_MODE_STORE_AVG_ENABLE
} SARADC_MODE_STORE;

typedef enum SARADC_CLK_DIV_TAG
{
    SARADC_CLK_DIV_0,
    SARADC_CLK_DIV_1,
    SARADC_CLK_DIV_2,
    SARADC_CLK_DIV_3,
    SARADC_CLK_DIV_4,
    SARADC_CLK_DIV_5,
    SARADC_CLK_DIV_6,
    SARADC_CLK_DIV_7,
    SARADC_CLK_DIV_8,
    SARADC_CLK_DIV_9,
    SARADC_CLK_DIV_10,
    SARADC_CLK_DIV_11,
    SARADC_CLK_DIV_12,
    SARADC_CLK_DIV_13,
    SARADC_CLK_DIV_14,
    SARADC_CLK_DIV_15,
    SARADC_CLK_DIV_16,
    SARADC_CLK_DIV_17,
    SARADC_CLK_DIV_18,
    SARADC_CLK_DIV_19,
    SARADC_CLK_DIV_20,
    SARADC_CLK_DIV_21,
    SARADC_CLK_DIV_22,
    SARADC_CLK_DIV_23,
    SARADC_CLK_DIV_24,
    SARADC_CLK_DIV_25,
    SARADC_CLK_DIV_26,
    SARADC_CLK_DIV_27,
    SARADC_CLK_DIV_28,
    SARADC_CLK_DIV_29,
    SARADC_CLK_DIV_30,
    SARADC_CLK_DIV_31
} SARADC_CLK_DIV;

typedef enum SARADC_XAIN_SIGNAL_TAG
{
    SARADC_XAIN_SIGNAL_0,    // standard input
    SARADC_XAIN_SIGNAL_1,    // 0.0V
    SARADC_XAIN_SIGNAL_2,    // 0.825V
    SARADC_XAIN_SIGNAL_3,    // 1.8V
    SARADC_XAIN_SIGNAL_4,    // 2.475V
    SARADC_XAIN_SIGNAL_5     // 3.3V
} SARADC_XAIN_SIGNAL;

typedef enum SARADC_AVG_CAL_COUNT_TAG
{
    SARADC_AVG_CAL_COUNT_0,    // 32 / (2^0): 32 counts
    SARADC_AVG_CAL_COUNT_1,    // 32 / (2^1): 16 counts
    SARADC_AVG_CAL_COUNT_2,
    SARADC_AVG_CAL_COUNT_3     // 32 / (2^3): 4 counts
} SARADC_AVG_CAL_COUNT;

typedef enum SARADC_TRIG_AVG_TAG
{
    SARADC_TRIG_AVG_ALWAYS,        // always trigger
    SARADC_TRIG_AVG_PULSE,         // trigger if pulse > (maxAVGTrigger - minAVGTrigger)
    SARADC_TRIG_AVG_LEVEL_HIGH,
    SARADC_TRIG_AVG_LEVEL_LOW,
    SARADC_TRIG_AVG_LEVEL          // trigger if (average value > maxAVGTrigger) or (average value < minAVGTrigger)
} SARADC_TRIG_AVG;

typedef enum SARADC_INTR_AVG_TAG
{
    SARADC_INTR_AVG_VALID,
    SARADC_INTR_AVG_ABOMAX,
    SARADC_INTR_AVG_UNDMIN
} SARADC_INTR_AVG;

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct XAIN_INFO_TAG
{
    uint8_t         *baseAddr;
    uint16_t        rptr;
    uint16_t        maxAVGDetect;
    uint16_t        minAVGDetect;
    SARADC_TRIG_AVG trigAVG;
    uint16_t        maxAVGTrigger;
    uint16_t        minAVGTrigger;
} XAIN_INFO;

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

#endif /* SARADC_TYPE_H */

