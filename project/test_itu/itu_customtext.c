#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "itu_custom.h"

static const char customTextName[] = "ITUCustomText";

void ituCustomTextExit(ITUWidget* widget)
{
    ITUCustomText* text = (ITUCustomText*) widget;
    assert(widget);
    ITU_ASSERT_THREAD();

    if (text->string)
    {
        free(text->string);
        text->string = NULL;
    }
    ituWidgetExitImpl(widget);
}

bool ituCustomTextClone(ITUWidget* widget, ITUWidget** cloned)
{
    ITUCustomText* text = (ITUCustomText*)widget;
    assert(widget);
    assert(cloned);
    ITU_ASSERT_THREAD();

    if (*cloned == NULL)
    {
        ITUWidget* newWidget = malloc(sizeof(ITUCustomText));
        if (newWidget == NULL)
            return false;

        memcpy(newWidget, widget, sizeof(ITUCustomText));
        newWidget->tree.child = newWidget->tree.parent = newWidget->tree.sibling = NULL;
        *cloned = newWidget;
    }

    if (text->string)
    {
        ITUCustomText* newText = (ITUCustomText*)*cloned;
        newText->string =strdup(text->string);
    }
    return ituWidgetCloneImpl(widget, cloned);
}

bool ituCustomTextUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result;
    ITUCustomText* text = (ITUCustomText*) widget;
    assert(text);

    result = ituWidgetUpdateImpl(widget, ev, arg1, arg2, arg3);
    return widget->visible ? result : false;
}

void ituCustomTextDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    ITUCustomText* text = (ITUCustomText*) widget;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    int destx, desty;
    uint8_t desta, destbga;
    ITURectangle prevClip;
    char* string;
    unsigned int style = ITU_FT_STYLE_DEFAULT;
    assert(text);
    assert(dest);

    if (widget->rect.width == 0 || widget->rect.height == 0)
    {
        widget->dirty = false;
        return;
    }

    if (text->string)
    {
        string = text->string;
    }
    else if (text->stringSet)
    {
        char buf[512];
        if (text->stringSet->strings[1] && text->stringSet->strings[2])
            sprintf(buf, "%s%s%s", text->stringSet->strings[1], text->stringSet->strings[0], text->stringSet->strings[2]);
        else if (text->stringSet->strings[1])
            sprintf(buf, "%s%s", text->stringSet->strings[1], text->stringSet->strings[0]);
        else if (text->stringSet->strings[2])
            sprintf(buf, "%s%s", text->stringSet->strings[0], text->stringSet->strings[2]);
        else
            sprintf(buf, "%s", text->stringSet->strings[0]);

        string = strdup(buf);
    }
    else
    {
        widget->dirty = false;
        return;
    }
    destx = rect->x + x;
    desty = rect->y + y;
    desta = alpha * widget->color.alpha / 255;
    desta = desta * widget->alpha / 255;
    destbga = alpha * text->bgColor.alpha / 255;
    destbga = destbga * widget->alpha / 255;

    if ((widget->flags & ITU_CLIP_DISABLED) == 0)
        ituWidgetSetClipping(widget, dest, x, y, &prevClip);
    
    if (destbga == 255)
    {
        ituColorFill(dest, destx, desty, rect->width, rect->height, &text->bgColor);
    }
    else if (destbga > 0)
    {
#ifdef CFG_WIN32_SIMULATOR
        ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
        if (surf)
        {
            ituColorFill(surf, 0, 0, rect->width, rect->height, &text->bgColor);
            ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, destbga);                
            ituDestroySurface(surf);
        }
#else
        ituColorFillBlend(dest, destx, desty, rect->width, rect->height, &text->bgColor, true, true, destbga);
#endif
    }

    ituFtSetCurrentFont(0);

    if (desta == 255)
    {
        ituSetColor(&dest->fgColor, desta, widget->color.red, widget->color.green, widget->color.blue);

        if (text->fontHeight > 0)
            ituFtSetFontSize(text->fontWidth, text->fontHeight);

        ituFtSetFontStyle(style);
        ituFtDrawText(dest, destx, desty, string);
    }
    else if (desta > 0)
    {
        ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
        if (surf)
        {
            int dx = 0, dy = 0;

            if (dest->format != ITU_RGB565)
            {
                ITUColor color = { 0, 0, 0, 0 };
                ituColorFill(surf, 0, 0, rect->width, rect->height, &color);
            }
            ituBitBlt(surf, 0, 0, rect->width, rect->height, dest, destx, desty);
            ituSetColor(&surf->fgColor, desta, widget->color.red, widget->color.green, widget->color.blue);
            if (text->fontHeight > 0)
                ituFtSetFontSize(text->fontWidth, text->fontHeight);

            ituFtSetFontStyle(style);

            ituFtDrawText(surf, dx, dy, string);
            ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);                
            ituDestroySurface(surf);
        }
    }
    if ((widget->flags & ITU_CLIP_DISABLED) == 0)
        ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);

    ituWidgetDrawImpl(widget, dest, x, y, alpha);
}

void ituCustomTextOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    ITUCustomText* text = (ITUCustomText*) widget;
    assert(widget);

    switch (action)
    {
    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

void ituCustomTextInit(ITUCustomText* text)
{
    assert(text);
    ITU_ASSERT_THREAD();

    memset(text, 0, sizeof (ITUCustomText));

    ituWidgetInit(&text->widget);

    ituWidgetSetType(text, ITU_TEXT);
    ituWidgetSetName(text, customTextName);
    ituWidgetSetExit(text, ituCustomTextExit);
    ituWidgetSetClone(text, ituCustomTextClone);
    ituWidgetSetUpdate(text, ituCustomTextUpdate);
    ituWidgetSetDraw(text, ituCustomTextDraw);
    ituWidgetSetOnAction(text, ituCustomTextOnAction);

    ituSetColor(&text->widget.color, 255, 255, 255, 255);
}

void ituCustomTextLoad(ITUWidget* widget, uint32_t base)
{
    ITUCustomText* text = (ITUCustomText*)widget;
    assert(text);

    ituWidgetLoad((ITUWidget*)text, base);
    
    ituWidgetSetExit(text, ituCustomTextExit);
    ituWidgetSetClone(text, ituCustomTextClone);
    ituWidgetSetUpdate(text, ituCustomTextUpdate);
    ituWidgetSetDraw(text, ituCustomTextDraw);
    ituWidgetSetOnAction(text, ituCustomTextOnAction);

    if (text->stringSet)
        text->stringSet = (ITUStringSet*)(base + (uint32_t)text->stringSet);
}

void ituCustomTextSetFontWidth(ITUCustomText* text, int height)
{
    assert(text);
    ITU_ASSERT_THREAD();
    text->fontWidth = height;
    text->widget.dirty = true;
}

void ituCustomTextSetFontHeight(ITUCustomText* text, int height)
{
    assert(text);
    ITU_ASSERT_THREAD();
    text->fontHeight = height;
    text->widget.dirty = true;
}

void ituCustomTextSetFontSize(ITUCustomText* text, int size)
{
    assert(text);
    ITU_ASSERT_THREAD();
    text->fontWidth = size;
    text->fontHeight = size;
    text->widget.dirty = true;
}

void ituCustomTextSetStringImpl(ITUCustomText* text, char* string)
{
    assert(text);
    ITU_ASSERT_THREAD();

    if (text->string)
        free(text->string);

	if (string)
    {
        text->string = strdup(string);
    }
    else
    {
        text->string = NULL;
    }
    if (string)
    {
        if (text->fontHeight > 0)
        {
            ituFtSetCurrentFont(0);
            ituFtSetFontSize(text->fontWidth, text->fontHeight);
        }
        ituFtSetFontStyle(ITU_FT_STYLE_DEFAULT);
    }
    text->widget.dirty = true;
}

char* ituCustomTextGetStringImpl(ITUCustomText* text)
{
    assert(text);
    ITU_ASSERT_THREAD();

    if (text->string)
        return text->string;
    else if (text->stringSet)
        return text->stringSet->strings[0];
    else
        return NULL;
}

void ituCustomTextSetBackColor(ITUCustomText* text, uint8_t alpha, uint8_t red, uint8_t green, uint8_t blue)
{
    assert(text);
    ITU_ASSERT_THREAD();

    text->bgColor.alpha = alpha;
    text->bgColor.red   = red;
    text->bgColor.green = green;
    text->bgColor.blue  = blue;
    text->widget.dirty  = true;
}

void ituCustomTextResize(ITUCustomText* text)
{
    char* string;
    ITURectangle* rect = (ITURectangle*) &text->widget.rect;
    assert(text);
    ITU_ASSERT_THREAD();

    if (text->string)
        string = text->string;
    else if (text->stringSet)
        string = text->stringSet->strings[0];
    else
        return;

    if (text->fontHeight > 0)
    {
        ituFtSetCurrentFont(0);
        ituFtSetFontSize(text->fontWidth, text->fontHeight);
    }

    ituFtSetFontStyle(ITU_FT_STYLE_DEFAULT);

    if (rect->height == 0)
    {
        ituFtGetTextDimension(string, &rect->width, &rect->height);
        if (rect->height)
            rect->height += rect->height / 4;
    }
    else
    {
        ituFtGetTextDimension(string, &rect->width, NULL);
    }
    text->widget.dirty  = true;
}
