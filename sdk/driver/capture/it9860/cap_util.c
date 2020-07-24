
#include "capture_types.h"
#include "capture_config.h"
#include "capture_util.h"
//=============================================================================
//                Constant Definition
//=============================================================================

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
//=============================================================================
/**
 * Convert standard float s[8].23 to float s[e].m
 *
 * @param  value
 * @param  e
 * @param  m
 * @return
 */
//=============================================================================
MMP_UINT32
CAP_FLOATToFix(
    MMP_FLOAT   value,
    MMP_UINT32  e,
    MMP_UINT32  m)
{
    MMP_UINT32  dwValue = *(MMP_UINT32 *)&value;
    MMP_UINT16  i       = 0;

    if (value < 0.0f)
    {
        value *= -1.0f;
        for (i = 0; i < m; i++)
            value *= 2;

        dwValue = sisftol(value);
        dwValue = ~dwValue + 1;
    }
    else
    {
        for (i = 0; i < m; i++)
            value *= 2;

        dwValue = sisftol(value);
    }

    return dwValue;
}

//=============================================================================
/**
 * Function for CAP scaling weighting matrix setting.
 *
 * @param
 * @return
 */
//=============================================================================
MMP_FLOAT
capSinc(
    MMP_FLOAT x)
{
    MMP_FLOAT pi = 3.14159265358979f;

    if (x == 0.0f)
    {
        return 1.0f;
    }
    else
    {
        return (sissin(pi * x) / (pi * x));
    }
}

//=============================================================================
/**
 * Function for CAP scaling weighting matrix setting.
 *
 * @param
 * @return
 */
//=============================================================================
MMP_FLOAT
capRcos(
    MMP_FLOAT x)
{
    MMP_FLOAT   pi  = 3.14159265358979f;
    MMP_FLOAT   r   = 0.5f;

    if (x == 0.0f)
    {
        return 1.0f;
    }
    else if (x == -1.0f / (2.0f * r) || x == 1.0f / (2.0f * r))
    {
        return (r / 2.0f * sissin(pi / (2.0f * r)));
    }
    else
    {
        return (sissin(pi * x) / (pi * x) * siscos(r * pi * x) / (1 - 4 * r * r * x * x));
    }
}

//=============================================================================
/**
 * Catmull-Rom Cubic interpolation.
 *
 * @param
 * @return
 */
//=============================================================================
MMP_FLOAT
capCubic01(
    MMP_FLOAT x)
{
    if (fabs(x) <= 1.0)
        return ((MMP_FLOAT) (1.5 * fabs(x * x * x) - 2.5 * x * x + 1.0));
    else if (fabs(x) > 1.0 && fabs(x) <= 2.0)
        return ((MMP_FLOAT) (-0.5 * fabs(x * x * x) + 2.5 * x * x - 4.0 * fabs(x) + 2.0));
    else
        return 0.0;
}

