#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ite/itu.h"
#include "itu_cfg.h"

static const char tableBarName[] = "ITUTableBar";

bool ituTableBarUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUTableBar* bar = (ITUTableBar*) widget;
    assert(bar);

    if (ev == ITU_EVENT_TIMER)
    {
        if (bar->tableListBox)
        {
            int count = itcTreeGetChildCount(bar->tableListBox);
            if (count > 0)
            {
                ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(bar->tableListBox, 0);
                int totalhigh = child->rect.height * count;
                int y = ituWidgetGetY(child);
                int pos = widget->rect.height * (0 - y) / (totalhigh - ituWidgetGetHeight(bar->tableListBox));

                if (bar->length == 0)
                {
                    bar->length = widget->rect.height * ituWidgetGetHeight(bar->tableListBox) / totalhigh;
                    bar->hideDelayCount = bar->hideDelay;
                    widget->alpha = 255;
                    result = widget->dirty = true;
                }

                if (pos != bar->pos)
                {
                    bar->pos = pos;
                    bar->hideDelayCount = bar->hideDelay;
                    widget->alpha = 255;
                    result = widget->dirty = true;
                }
                else if (bar->hideDelayCount > 0)
                {
                    if (--bar->hideDelayCount == 0)
                    {
                        bar->fadeOutDelayCount = bar->fadeOutDelay;
                    }
                }
                else if (bar->fadeOutDelayCount > 0)
                {
                    widget->alpha = 255 * bar->fadeOutDelayCount / bar->fadeOutDelay;
                    if (--bar->fadeOutDelayCount == 0)
                    {
                        widget->alpha = 0;
                    }
                    result = widget->dirty = true;
                }
            }
        }
    }
    else if (ev == ITU_EVENT_LAYOUT)
    {
        if (!bar->tableListBox && (bar->tableListBoxName[0] != '\0'))
        {
            bar->tableListBox = (ITUTableListBox*)ituSceneFindWidget(ituScene, bar->tableListBoxName);
            if (bar->tableListBox)
            {
                int count = itcTreeGetChildCount(bar->tableListBox);

                if (count > 0)
                {
                    ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(bar->tableListBox, 0);
                    int totalhigh = child->rect.height * count;
                    bar->length = widget->rect.height * ituWidgetGetHeight(bar->tableListBox) / totalhigh;
                    result = widget->dirty = true;
                }
            }
        }
    }
    result |= ituIconUpdate(widget, ev, arg1, arg2, arg3);

    return widget->visible ? result : false;
}

void ituTableBarDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    int destx, desty;
    uint8_t desta;
    ITURectangle prevClip;
    ITUTableBar* bar = (ITUTableBar*) widget;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    assert(bar);
    assert(dest);

    destx = rect->x + x;
    desty = rect->y + y;
    desta = alpha * widget->alpha / 255;

    ituWidgetSetClipping(widget, dest, x, y, &prevClip);

    if (desta == 255)
    {
        if (widget->color.alpha == 255)
        {
            ituColorFill(dest, destx, desty, rect->width, rect->height, &widget->color);
        }
        else if (widget->color.alpha > 0)
        {
            ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
            if (surf)
            {
                ituColorFill(surf, 0, 0, rect->width, rect->height, &widget->color);
                ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, widget->color.alpha);
                ituDestroySurface(surf);
            }
        }
            
        if (bar->length > 0)
        {
            ituColorFill(dest, destx, desty + bar->pos, rect->width, bar->length, &bar->fgColor);
        }
    }
    else if (desta > 0)
    {
        ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
        if (surf)
        {
            ituBitBlt(surf, 0, 0, rect->width, rect->height, dest, destx, desty);

            if (widget->color.alpha == 255)
            {
                ituColorFill(surf, 0, 0, rect->width, rect->height, &widget->color);
            }
            else if (widget->color.alpha > 0)
            {
                ITUSurface* surf2 = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
                if (surf2)
                {
                    ituColorFill(surf2, 0, 0, rect->width, rect->height, &widget->color);
                    ituAlphaBlend(surf, 0, 0, rect->width, rect->height, surf2, 0, 0, widget->color.alpha);
                    ituDestroySurface(surf2);
                }
            }
            
            if (bar->length > 0)
            {
                ituColorFill(surf, 0, bar->pos, rect->width, bar->length, &bar->fgColor);
            }
            ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);                
            ituDestroySurface(surf);
        }
    }
    ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
    ituWidgetDrawImpl(widget, dest, x, y, alpha);
}

void ituTableBarOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    assert(widget);

    switch (action)
    {
    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

void ituTableBarInit(ITUTableBar* bar)
{
    assert(bar);
    ITU_ASSERT_THREAD();

    memset(bar, 0, sizeof (ITUTableBar));

    ituBackgroundInit(&bar->bg);
    ituWidgetSetType(bar, ITU_TABLEBAR);
    ituWidgetSetName(bar, tableBarName);
    ituWidgetSetUpdate(bar, ituTableBarUpdate);
    ituWidgetSetDraw(bar, ituTableBarDraw);
    ituWidgetSetOnAction(bar, ituTableBarOnAction);
}

void ituTableBarLoad(ITUTableBar* bar, uint32_t base)
{
    assert(bar);

    ituBackgroundLoad(&bar->bg, base);
    ituWidgetSetUpdate(bar, ituTableBarUpdate);
    ituWidgetSetDraw(bar, ituTableBarDraw);
    ituWidgetSetOnAction(bar, ituTableBarOnAction);

    if (bar->tableListBox)
    {
        bar->tableListBox = (ITUTableListBox*)((uint32_t)bar->tableListBox + base);
    }
}
