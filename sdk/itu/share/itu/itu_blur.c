#include <assert.h>
#include <malloc.h>
#include <math.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char blurName[] = "ITUBlur";

#define KERNEL_SIZE 7
#define HALF_KERNEL KERNEL_SIZE / 2

static void blur_impl_horizontal_pass_generic(uint32_t *src, uint32_t *dst, int width, int height) {
    uint32_t *o_src = src;
    int row, column;
    for (row = 0; row < height; row++) {
        for (column = 0; column < width; column++, src++) {
            uint32_t rgbaIn[KERNEL_SIZE + 1];

            // handle borders
            int leftBorder = column < HALF_KERNEL;
            int rightBorder = column > width - HALF_KERNEL;
            int i = 0, k;
            uint32_t acc[4] = { 0 };

            if (leftBorder) {
                // for kernel size 7x7 and column == 0, we have:
                // x x x P0 P1 P2 P3
                // first loop mirrors P{0..3} to fill x's,
                // second one loads P{0..3}
                for (; i < HALF_KERNEL - column; i++)
                    rgbaIn[i] = *(src + (HALF_KERNEL - i));
                for (; i < KERNEL_SIZE; i++)
                    rgbaIn[i] = *(src - (HALF_KERNEL - i));
            }
            else if (rightBorder) {
                for (; i < width - column; i++)
                    rgbaIn[i] = *(src + i);
                for (k = 0; i < KERNEL_SIZE; i++, k++)
                    rgbaIn[i] = *(src - k);
            }
            else {
                for (; i < KERNEL_SIZE; i++) {
                    if ((uintptr_t)((src + 4 * i - HALF_KERNEL) + 1)
                >(uintptr_t) (o_src + (height * width)))
                break;
                    rgbaIn[i] = *(src + i - HALF_KERNEL);
                }
            }

            for (i = 0; i < KERNEL_SIZE; i++) {
                acc[0] += (rgbaIn[i] & 0xFF000000) >> 24;
                acc[1] += (rgbaIn[i] & 0x00FF0000) >> 16;
                acc[2] += (rgbaIn[i] & 0x0000FF00) >> 8;
                acc[3] += (rgbaIn[i] & 0x000000FF) >> 0;
            }

            for (i = 0; i < 4; i++)
                acc[i] *= 1.0 / KERNEL_SIZE;

            *(dst + height * column + row) = (acc[0] << 24) |
                (acc[1] << 16) |
                (acc[2] << 8) |
                (acc[3] << 0);
        }
    }
}

void ituBlurExit(ITUWidget* widget)
{
    ITUBlur* blur = (ITUBlur*) widget;
    assert(blur);
    ITU_ASSERT_THREAD();

    if (blur->maskSurf)
    {
        ituSurfaceRelease(blur->maskSurf);
        blur->maskSurf = NULL;
    }

    ituWidgetExitImpl(&blur->widget);
}

bool ituBlurClone(ITUWidget* widget, ITUWidget** cloned)
{
    ITUBlur* blur = (ITUBlur*)widget;
    ITUBlur* newBlur;
    assert(widget);
    assert(cloned);
    ITU_ASSERT_THREAD();

    if (*cloned == NULL)
    {
        ITUWidget* newWidget = malloc(sizeof(ITUBlur));
        if (newWidget == NULL)
            return false;

        memcpy(newWidget, widget, sizeof(ITUBlur));
        newWidget->tree.child = newWidget->tree.parent = newWidget->tree.sibling = NULL;
        *cloned = newWidget;
    }

    newBlur = (ITUBlur*)*cloned;
    newBlur->maskSurf = NULL;

    return ituWidgetCloneImpl(widget, cloned);
}

bool ituBlurUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUBlur* blur = (ITUBlur*) widget;
    assert(blur);

    if (ev == ITU_EVENT_LAYOUT)
    {
        if (blur->maskSurf)
        {
            ituSurfaceRelease(blur->maskSurf);
            blur->maskSurf = NULL;
        }
    }
    result |= ituWidgetUpdateImpl(widget, ev, arg1, arg2, arg3);
    
    return widget->visible ? result : false;
}

void ituBlurDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    int destx, desty;
    ITUBlur* blur = (ITUBlur*) widget;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    assert(blur);
    assert(dest);

    destx = rect->x + x;
    desty = rect->y + y;

    if (!blur->maskSurf && blur->factor > 0)
    {
        ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, ITU_ARGB8888, NULL, 0);
        if (surf)
        {
            ITUSurface* surf2 = ituCreateSurface(rect->width, rect->height, 0, ITU_ARGB8888, NULL, 0);
            if (surf2)
            {
                uint32_t* srcPtr;
                ituBitBlt(surf, 0, 0, rect->width, rect->height, dest, destx, desty);
                srcPtr = (uint32_t*)ituLockSurface(surf, 0, 0, rect->width, rect->height);
                if (srcPtr)
                {
                    uint32_t* destPtr = (uint32_t*)ituLockSurface(surf2, 0, 0, rect->width, rect->height);
                    if (destPtr)
                    {
                        int i;
                        for (i = 0; i < blur->iter; i++)
                        {
                            blur_impl_horizontal_pass_generic(srcPtr, destPtr, rect->width, rect->height);
                            blur_impl_horizontal_pass_generic(destPtr, srcPtr, rect->height, rect->width);
                        }
                        ituUnlockSurface(surf2);
                        blur->maskSurf = surf;
                    }
                    ituUnlockSurface(surf);
                }
                ituDestroySurface(surf2);
            }
        }
    }

    if (blur->maskSurf)
    {
        ituBitBlt(dest, destx, desty, rect->width, rect->height, blur->maskSurf, 0, 0);
        if (widget->flags & ITU_PROGRESS)
        {
            ituSurfaceRelease(blur->maskSurf);
            blur->maskSurf = NULL;
        }
    }
    ituWidgetDrawImpl(widget, dest, x, y, alpha);
}

void ituBlurInit(ITUBlur* blur)
{
    assert(blur);
    ITU_ASSERT_THREAD();

    memset(blur, 0, sizeof (ITUBlur));

    ituWidgetInit(&blur->widget);

    ituWidgetSetType(blur, ITU_LAYER);
    ituWidgetSetName(blur, blurName);
    ituWidgetSetExit(blur, ituBlurExit);
    ituWidgetSetClone(blur, ituBlurClone);
    ituWidgetSetUpdate(blur, ituBlurUpdate);
    ituWidgetSetDraw(blur, ituBlurDraw);
}

void ituBlurLoad(ITUBlur* blur, uint32_t base)
{
    assert(blur);

    ituWidgetLoad((ITUWidget*)blur, base);
    ituWidgetSetExit(blur, ituBlurExit);
    ituWidgetSetClone(blur, ituBlurClone);
    ituWidgetSetUpdate(blur, ituBlurUpdate);
    ituWidgetSetDraw(blur, ituBlurDraw);
}
