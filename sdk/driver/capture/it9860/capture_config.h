#ifndef __CAP_CONFIG_H__
#define __CAP_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                Constant Definition
//=============================================================================

#define COLOR_MATRIX_VIDEOMODE
#ifdef CFG_SENSOR_ENABLE
#define DIGITAL_SENSOR_DEV
//#define ANALOG_SENSOR_DEV
#endif

/*define use how many capture max = 1*/
#define CAP_DEVICE_ID_MAX 1
/*define use how many memory buffer */
#define CAPTURE_MEM_BUF_COUNT 6
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

#endif