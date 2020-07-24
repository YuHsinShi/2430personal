#include <assert.h>
#include <malloc.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char clipperName[] = "ITUClipper";

void ituClipperExit(ITUWidget* widget)
{
    ITUBlur* clipper = (ITUBlur*)widget;
    assert(clipper);
    ITU_ASSERT_THREAD();

    if (clipper->maskSurf)
    {
        ituSurfaceRelease(clipper->maskSurf);
        clipper->maskSurf = NULL;
    }

    ituWidgetExitImpl(&clipper->widget);
}

bool ituClipperClone(ITUWidget* widget, ITUWidget** cloned)
{
    ITUBlur* clipper = (ITUBlur*)widget;
    ITUBlur* newClipper;
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

    newClipper = (ITUBlur*)*cloned;
    newClipper->maskSurf = NULL;

    return ituWidgetCloneImpl(widget, cloned);
}

bool ituClipperUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUClipper* clipper = (ITUClipper*) widget;
    assert(clipper);

    if (ev == ITU_EVENT_LAYOUT)
    {
        if (clipper->maskSurf)
        {
            ituSurfaceRelease(clipper->maskSurf);
            clipper->maskSurf = NULL;
        }

        if (!clipper->outside)
        {
            ITUWidget* parent = (ITUWidget*)widget->tree.parent;
            ituWidgetSetBound(parent, parent->rect.x + widget->rect.x, parent->rect.y + widget->rect.y, widget->rect.width, widget->rect.height);
        }
    }
    result |= ituWidgetUpdateImpl(widget, ev, arg1, arg2, arg3);

    return widget->visible ? result : false;
}

void ituClipperDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    ITUClipper* clipper = (ITUClipper*)widget;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    ITUWidget* parent = (ITUWidget*)widget->tree.parent;
    assert(clipper);
    assert(dest);

    if (!clipper->maskSurf && clipper->outside)
    {
        clipper->maskSurf = ituCreateSurface(parent->rect.width, parent->rect.height, 0, dest->format, NULL, 0);
    }

    if (clipper->maskSurf)
    {
        ituBitBlt(clipper->maskSurf, 0, 0, parent->rect.width, parent->rect.height, dest, x, y);
    }
    ituWidgetDrawImpl(widget, dest, x, y, alpha);
}

void ituClipperPostDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    ITUClipper* clipper = (ITUClipper*)widget;

    if (clipper->outside && clipper->maskSurf)
    {
        ITURectangle* rect = (ITURectangle*)&widget->rect;
        ITUWidget* parent = (ITUWidget*)widget->tree.parent;
        int dx, dy, sx, sy, w, h;
        assert(clipper);
        assert(dest);

        dx = x;
        dy = y;
        sx = 0;
        sy = 0;
        w = parent->rect.width;
        h = rect->y;
        ituBitBlt(dest, dx, dy, w, h, clipper->maskSurf, sx, sy);

        dx = x;
        dy = rect->y + y;
        sx = 0;
        sy = rect->y;
        w = rect->x;
        h = parent->rect.height - rect->y;
        ituBitBlt(dest, dx, dy, w, h, clipper->maskSurf, sx, sy);

        dx = rect->x + x + rect->width;
        dy = rect->y + y;
        sx = rect->x + rect->width;
        sy = rect->y;
        w = parent->rect.width - rect->width - rect->x;
        h = parent->rect.height - rect->y;
        ituBitBlt(dest, dx, dy, w, h, clipper->maskSurf, sx, sy);

        dx = rect->x + x;
        dy = rect->y + y + rect->height;
        sx = rect->x;
        sy = rect->y + rect->height;
        w = rect->width;
        h = parent->rect.height - rect->height - rect->y;
        ituBitBlt(dest, dx, dy, w, h, clipper->maskSurf, sx, sy);
    }
}

void ituClipperInit(ITUClipper* clipper)
{
    assert(clipper);
    ITU_ASSERT_THREAD();

    memset(clipper, 0, sizeof (ITUClipper));

    ituWidgetInit(&clipper->widget);
    
    ituWidgetSetType(clipper, ITU_CLIPPER);
    ituWidgetSetName(clipper, clipperName);
    ituWidgetSetUpdate(clipper, ituClipperUpdate);
    ituWidgetSetDraw(clipper, ituClipperDraw);
}

void ituClipperLoad(ITUClipper* clipper, uint32_t base)
{
    assert(clipper);

    ituWidgetLoad((ITUWidget*)clipper, base);
    ituWidgetSetUpdate(clipper, ituClipperUpdate);
    ituWidgetSetDraw(clipper, ituClipperDraw);
}
