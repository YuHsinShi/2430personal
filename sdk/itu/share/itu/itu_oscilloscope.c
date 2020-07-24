#include <assert.h>
#include <malloc.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char oscName[] = "ITUOscilloscope";

void ituOscilloscopeExit(ITUWidget* widget)
{
    ITUOscilloscope* osc = (ITUOscilloscope*) widget;
    assert(widget);
    ITU_ASSERT_THREAD();

    free(osc->values);
    osc->values = NULL;

    ituIconExit(widget);
}

bool ituOscilloscopeClone(ITUWidget* widget, ITUWidget** cloned)
{
    ITUOscilloscope* osc = (ITUOscilloscope*)widget;
    ITUOscilloscope* newOscilloscope;
    int* values;
    assert(widget);
    assert(cloned);
    ITU_ASSERT_THREAD();

    if (*cloned == NULL)
    {
        ITUWidget* newWidget = malloc(sizeof(ITUOscilloscope));
        if (newWidget == NULL)
            return false;

        memcpy(newWidget, widget, sizeof(ITUOscilloscope));
        newWidget->tree.child = newWidget->tree.parent = newWidget->tree.sibling = NULL;
        *cloned = newWidget;
    }

    newOscilloscope = (ITUOscilloscope*)*cloned;
    newOscilloscope->values = NULL;

    values = malloc(sizeof(int)* (osc->resolutionX + 1));
    if (values)
    {
        memcpy(values, osc->values, sizeof(int) * (osc->resolutionX + 1));
        newOscilloscope->values = values;
    }
    return ituWidgetCloneImpl(widget, cloned);
}

bool ituOscilloscopeUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUOscilloscope* osc = (ITUOscilloscope*)widget;
    assert(osc);

    result = ituIconUpdate(widget, ev, arg1, arg2, arg3);
    if (ev == ITU_EVENT_LAYOUT)
    {
        if (!osc->values)
        {
            osc->values = calloc(osc->resolutionX + 1, sizeof(int));
        }
    }
    return widget->visible ? result : false;
}

void ituOscilloscopeDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    int destx, desty, i, dx0, dx1, dy0, dy1;
    float w, h, gw, gh;
    ITURectangle prevClip;
    ITUOscilloscope* osc = (ITUOscilloscope*) widget;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    assert(osc);
    assert(dest);

    ituBackgroundDraw(widget, dest, x, y, alpha);

    destx = rect->x + x;
    desty = rect->y + y;

    ituWidgetSetClipping(widget, dest, x, y, &prevClip);

    w = (float)rect->width / osc->resolutionX;
    h = (float)rect->height / osc->resolutionY;

    if (osc->gridColor.alpha && memcmp(&widget->color, &osc->gridColor, sizeof(ITUColor)))
    {
        gw = (float)rect->width / osc->gridResolutionX;

        if (gw > 1.0f)
        {
            for (i = 1; i <= osc->gridResolutionX; ++i)
            {
                float offset = osc->offsetX * w;
                while (offset > gw)
                    offset -= gw;

                dx0 = (int)(i * gw - offset);
                dy0 = 0;
                dx1 = dx0;
                dy1 = rect->height;
                ituDrawLine(dest, destx + dx0, desty + dy0, destx + dx1, desty + dy1, &osc->gridColor, osc->gridLineWidth);
            }
        }

        gh = (float)rect->height / osc->gridResolutionY;

        if (gh > 1.0f)
        {
            for (i = 1; i < osc->gridResolutionY; ++i)
            {
                dx0 = 0;
                dy0 = (int)(i * gh);
                dx1 = rect->width;
                dy1 = dy0;
                ituDrawLine(dest, destx + dx0, desty + dy0, destx + dx1, desty + dy1, &osc->gridColor, osc->gridLineWidth);
            }
        }
    }

    if (w > 1.0f && h > 1.0f)
    {
        for (i = 0; i < osc->resolutionX; ++i)
        {
            if (i > 0)
            {
                dx0 = (int)(i * w);
                dy0 = (int)((osc->resolutionY - osc->values[i]) * h);
                dx1 = (int)(i * w);
                dy1 = (int)((osc->resolutionY - osc->values[i + 1]) * h);
                ituDrawLine(dest, destx + dx0, desty + dy0, destx + dx1, desty + dy1, &osc->fgColor, osc->lineWidth);
            }

            dx0 = (int)(i * w) - osc->lineWidth / 2;
            dy0 = (int)((osc->resolutionY - osc->values[i + 1]) * h);
            dx1 = (int)((i + 1) * w);
            dy1 = (int)((osc->resolutionY - osc->values[i + 1]) * h);
            //printf("%d %d %d %d\n", dx0, dy0, dx1, dy1);
            ituDrawLine(dest, destx + dx0, desty + dy0, destx + dx1, desty + dy1, &osc->fgColor, osc->lineWidth);
        }
    }

    ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
}

void ituOscilloscopeInit(ITUOscilloscope* osc)
{
    assert(osc);
    ITU_ASSERT_THREAD();

    memset(osc, 0, sizeof (ITUOscilloscope));

    ituBackgroundInit(&osc->bg);

    ituWidgetSetType(osc, ITU_OSCILLOSCOPE);
    ituWidgetSetName(osc, oscName);
    ituWidgetSetExit(osc, ituOscilloscopeExit);
    ituWidgetSetUpdate(osc, ituOscilloscopeUpdate);
    ituWidgetSetDraw(osc, ituOscilloscopeDraw);
}

void ituOscilloscopeLoad(ITUOscilloscope* osc, uint32_t base)
{
    assert(osc);

    ituBackgroundLoad(&osc->bg, base);
    ituWidgetSetExit(osc, ituOscilloscopeExit);
    ituWidgetSetUpdate(osc, ituOscilloscopeUpdate);
    ituWidgetSetDraw(osc, ituOscilloscopeDraw);
}

void ituOscilloscopeInput(ITUOscilloscope* osc, int value)
{
    assert(osc);
    ITU_ASSERT_THREAD();

    if (value <= osc->resolutionY)
    {
        int i;

        for (i = 0; i < osc->resolutionX; ++i)
        {
            osc->values[i] = osc->values[i + 1];
            //printf("%d ", osc->values[i]);
        }
        osc->values[osc->resolutionX] = value;
        //printf("%d\n", osc->values[osc->resolutionX]);

        if (++osc->offsetX >= osc->resolutionX)
        {
            osc->offsetX = 0;
        }
        ituWidgetSetDirty(osc, true);
    }
}