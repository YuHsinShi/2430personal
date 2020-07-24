#include "ite/itu.h"
#include "itu_cfg.h"

unsigned int ituFormat2Bpp(ITUPixelFormat format)
{
    switch (format)
    {
    case ITU_RGB565:
    case ITU_ARGB1555:
    case ITU_ARGB4444:
    case ITU_RGB565A8:
        return 16;
        break;

    case ITU_ARGB8888:
        return 32;
        break;

    case ITU_A8:
    case ITU_MASK_A8:
        return 8;
        break;

    case ITU_MONO:
        return 1;
        break;

    default:
        return 0;
    }
}
