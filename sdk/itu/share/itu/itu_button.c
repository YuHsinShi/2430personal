#include <assert.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char btnName[] = "ITUButton";

//Special flag used to active the press alpha(image) delay mode
#define PRESS_ALPHA_DELAY_ARG 255

//The delay frame when active the press alpha(image) delay mode
#define PRESS_ALPHA_DELAY_FRAME 3

void ituButtonExit(ITUWidget* widget)
{
    ITUButton* btn = (ITUButton*) widget;
    assert(widget);
    ITU_ASSERT_THREAD();

    if (btn->pressSurf)
    {
        ituSurfaceRelease(btn->pressSurf);
        btn->pressSurf = NULL;
    }

    if (btn->focusSurf)
    {
        ituSurfaceRelease(btn->focusSurf);
        btn->focusSurf = NULL;
    }
    ituIconExit(widget);
}

bool ituButtonClone(ITUWidget* widget, ITUWidget** cloned)
{
    ITUButton* btn = (ITUButton*)widget;
    ITUButton* newBtn;
    ITUSurface* surf;
	int layoutmode = ITU_LAYOUT_CENTER;
	bool result;
	char* textSrc = ituTextGetString(&btn->text);
	char textOrg[50];
	memset(textOrg, 0, sizeof(char)* 50);
	memcpy(textOrg, textSrc, sizeof(char)* 50);

    assert(widget);
    assert(cloned);
    ITU_ASSERT_THREAD();

    if (*cloned == NULL)
    {
        ITUWidget* newWidget = malloc(sizeof(ITUButton));
        if (newWidget == NULL)
            return false;

        memcpy(newWidget, widget, sizeof(ITUButton));
        newWidget->tree.child = newWidget->tree.parent = newWidget->tree.sibling = NULL;

        *cloned = newWidget;
    }

    newBtn = (ITUButton*)*cloned;

	//fix cloned button with wrong text alignment
	if (newBtn)
	{
		switch (newBtn->text.layout)
		{
		case ITU_LAYOUT_TOP_LEFT:
			layoutmode = ITU_LAYOUT_LEFT;
			break;

		case ITU_LAYOUT_TOP_RIGHT:
			layoutmode = ITU_LAYOUT_RIGHT;
			break;

		case ITU_LAYOUT_TOP_CENTER:
			layoutmode = ITU_LAYOUT_UP;
			break;

		case ITU_LAYOUT_MIDDLE_LEFT:
			layoutmode = ITU_LAYOUT_LEFT;
			break;

		case ITU_LAYOUT_MIDDLE_RIGHT:
			layoutmode = ITU_LAYOUT_RIGHT;
			break;

		case ITU_LAYOUT_MIDDLE_CENTER:
			layoutmode = ITU_LAYOUT_CENTER;
			break;

		case ITU_LAYOUT_BOTTOM_LEFT:
			layoutmode = ITU_LAYOUT_LEFT;
			break;

		case ITU_LAYOUT_BOTTOM_RIGHT:
			layoutmode = ITU_LAYOUT_RIGHT;
			break;

		case ITU_LAYOUT_BOTTOM_CENTER:
			layoutmode = ITU_LAYOUT_DOWN;
			break;

		default:
			break;
		}
	}

    // change internal tree structure of new button
	newBtn->text.widget.tree.parent = &newBtn->bwin;
	newBtn->bwin.widget.tree.parent = newBtn;

	newBtn->bwin.widgets[layoutmode] = &newBtn->text.widget;
    newBtn->bwin.widget.tree.child = &newBtn->text.widget.tree;
    newBtn->bg.icon.widget.tree.child = &newBtn->bwin.widget.tree;

    surf = newBtn->bg.icon.staticSurf;

    if (surf)
    {
		if (surf->flags & ITU_COMPRESSED)
			newBtn->bg.icon.surf = ituSurfaceDecompress(surf);
        else
            newBtn->bg.icon.surf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }

    surf = newBtn->staticFocusSurf;

    if (surf)
    {
		if (surf->flags & ITU_COMPRESSED)
			newBtn->focusSurf = ituSurfaceDecompress(surf);
        else
            newBtn->focusSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }

    surf = newBtn->staticPressSurf;

    if (surf)
    {
		if (surf->flags & ITU_COMPRESSED)
			newBtn->pressSurf = ituSurfaceDecompress(surf);
        else
            newBtn->pressSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }

	//fix cloned button will draw last text into the new press surf (just work around)
	ituTextSetString(&btn->text, "");
	newBtn->text.string = NULL;
    result = ituBackgroundClone(widget, cloned);
	ituTextSetString(&btn->text, textOrg);
	ituTextSetString(&newBtn->text, textOrg);

	return result;
}

static void ButtonLoadExternalData(ITUButton* btn, ITULayer* layer)
{
    ITUWidget* widget = (ITUWidget*)btn;
    ITUSurface* surf;

    assert(widget);

    if (!(widget->flags & ITU_EXTERNAL))
        return;

    if (!layer)
        layer = ituGetLayer(widget);

    if (btn->staticFocusSurf && !btn->focusSurf)
    {
        surf = ituLayerLoadExternalSurface(layer, (uint32_t)btn->staticFocusSurf);

        if (surf->flags & ITU_COMPRESSED)
            btn->focusSurf = ituSurfaceDecompress(surf);
        else
            btn->focusSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }

    if (btn->staticPressSurf && !btn->pressSurf)
    {
        surf = ituLayerLoadExternalSurface(layer, (uint32_t)btn->staticPressSurf);

        if (surf->flags & ITU_COMPRESSED)
            btn->pressSurf = ituSurfaceDecompress(surf);
        else
            btn->pressSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }
}

bool ituButtonUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUButton* btn = (ITUButton*) widget;
    assert(btn);

    if (ev == ITU_EVENT_LAYOUT)
    {
        ituTextResize(&btn->text);
        btn->bwin.widget.rect.width = widget->rect.width;
        btn->bwin.widget.rect.height = widget->rect.height;
    }

    result |= ituIconUpdate(widget, ev, arg1, arg2, arg3);

    if (ev == ITU_EVENT_MOUSEDOWN)
    {
        if (ituWidgetIsEnabled(widget) && !result)
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                if (btn->mouseUpDelay > 0)
                    btn->mouseUpDelayCount = itpGetTickCount();

                if (btn->mouseLongPressDelay > 0)
                    btn->mouseLongPressDelayCount = itpGetTickCount();

				if (arg1 == PRESS_ALPHA_DELAY_ARG)
				{
					if (btn->press_alpha_delay_count == 0)
						btn->press_alpha_delay_count = PRESS_ALPHA_DELAY_FRAME;

					widget->dirty = true;
					btn->pressed = true;
				}
				else
				{
					ituButtonSetPressed(btn, true);
				}

                result |= ituExecActions((ITUWidget*)btn, btn->actions, ev, 0);

                if (widget->type == ITU_BUTTON)
                    result |= ituWidgetOnPress(widget, ev, arg1, x, y);

                result |= widget->dirty;
                return result;
            }
        }
    }
    else if (ev == ITU_EVENT_MOUSEUP)
    {
        if (ituWidgetIsEnabled(widget))
        {
			ITULayer* thisLayer = ituGetLayer(widget);
			btn->press_alpha_delay_count = 0;

			if (thisLayer)
			{
				ITUWidget* parentLayer = (ITUWidget*)thisLayer;

				//clear ITU_CHILD_LONG_DRAGGING when using dynamic menu
				if (parentLayer->flags & ITU_CHILD_LONG_DRAGGING)
				{
					parentLayer->flags &= ~ITU_CHILD_LONG_DRAGGING;
					//printf("layer %s is removed ITU_CHILD_LONG_DRAGGING\n", parentLayer->name);
				}
			}

			if (thisLayer && btn->pressed)
			{
				if (!thisLayer->widget.visible)
					ituButtonSetPressed(btn, false);
			}

            if (btn->pressed)
            {
                ituButtonSetPressed(btn, false);

                if (btn->mouseUpDelay == 0 || btn->mouseUpDelayCount == UINT_MAX)
                {
                    result |= ituExecActions((ITUWidget*)btn, btn->actions, ev, 0);
                    btn->mouseUpDelayCount = 0;
                    btn->mouseLongPressDelayCount = 0;
                }

				if (widget->type == ITU_BUTTON)
				{
					ituFocusWidget(btn);
				}

                result |= widget->dirty;
                return result;
            }
        }
    }
    else if (ev == ITU_EVENT_MOUSEDOUBLECLICK || ev == ITU_EVENT_MOUSELONGPRESS ||
        ev == ITU_EVENT_TOUCHSLIDELEFT || ev == ITU_EVENT_TOUCHSLIDEUP || ev == ITU_EVENT_TOUCHSLIDERIGHT || ev == ITU_EVENT_TOUCHSLIDEDOWN)
    {
        if (ituWidgetIsEnabled(widget) && !result)
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

			btn->press_alpha_delay_count = 0;

            if (!btn->mouseLongPressDelay && (!widget->rect.width || !widget->rect.height || ituWidgetIsInside(widget, x, y)))
            {
                result |= ituExecActions(widget, btn->actions, ev, arg1);
            }
            if (btn->pressed && (ev != ITU_EVENT_MOUSELONGPRESS) && (btn->fsg1 == 0))
            {
                ituButtonSetPressed(btn, false);
                result |= widget->dirty;
            }
			else if (ev == ITU_EVENT_MOUSELONGPRESS) //use for dynamic menu
			{
				ITUWidget* parentLayer = (ITUWidget*)ituGetLayer(widget);
				if (parentLayer)
				{
					parentLayer->flags |= ITU_CHILD_LONG_DRAGGING;
					//printf("layer %s is set to ITU_CHILD_LONG_DRAGGING\n", parentLayer->name);
				}
			}
        }
    }
	else if (ev == ITU_EVENT_MOUSEMOVE)
	{
		if (ituWidgetIsEnabled(widget))
		{
			int x = arg2 - widget->rect.x;
			int y = arg3 - widget->rect.y;

			//check when move outside the button
			bool bWidgetInSideCheck = ((ituWidgetIsInside(widget, x, y)) ? (false) : (true));

			//disable outside check when using trackbar
			if (btn && bWidgetInSideCheck && ituButtonIsPressed(btn))
			{
				ITCTree* btnTree = (ITCTree*)btn;
				ITCTree* parent = btnTree->parent;

				if (parent)
				{
					ITUWidget* pWidget = (ITUWidget*)parent;
					if (pWidget->type == ITU_TRACKBAR)
						bWidgetInSideCheck = false;
				}
			}

			//disable outside check when using dynamic menu (ITU_CHILD_LONG_DRAGGING)
			if (bWidgetInSideCheck)
			{
				ITUWidget* parentLayer = (ITUWidget*)ituGetLayer(widget);
				if (parentLayer)
				{
					if (parentLayer->flags & ITU_CHILD_LONG_DRAGGING)
					{
						bWidgetInSideCheck = false;
						//printf("remove button outside check when layer under ITU_CHILD_LONG_DRAGGING\n");
					}
				}
			}

			if (((btn->press_alpha_delay_count < 255) && (arg1 == PRESS_ALPHA_DELAY_ARG)) || bWidgetInSideCheck)
			{
				if (ituButtonIsPressed(btn))
				{
					//printf("unpress %s\n", widget->name);
					ituButtonSetPressed(btn, false);
				}
				btn->press_alpha_delay_count = 0;
			}
		}
	}
    else if (ev >= ITU_EVENT_CUSTOM || ev == ITU_EVENT_TIMER)
    {
        if (ituWidgetIsEnabled(widget))
        {
			if (ev == ITU_EVENT_TIMER)
			{
				if ((btn->press_alpha_delay_count > 0) && (btn->press_alpha_delay_count < 255))
				{
					if (btn->press_alpha_delay_count == 1)
					{
						ituWidgetSetColor(widget, btn->pressColor.alpha, btn->pressColor.red, btn->pressColor.green, btn->pressColor.blue);
						//btn->pressed = true;
						widget->dirty = true;
					}

					btn->press_alpha_delay_count--;
				}

                if (btn->pressed)
                {
                    if (btn->mouseUpDelayCount != 0 && btn->mouseUpDelayCount != UINT_MAX)
                    {
                        if (itpGetTickDuration(btn->mouseUpDelayCount) >= btn->mouseUpDelay)
                            btn->mouseUpDelayCount = UINT_MAX;
                    }

                    if (btn->mouseLongPressDelayCount != 0)
                    {
                        if (itpGetTickDuration(btn->mouseLongPressDelayCount) >= btn->mouseLongPressDelay)
                        {
                            result |= ituExecActions(widget, btn->actions, ITU_EVENT_MOUSELONGPRESS, 0);
                            btn->mouseLongPressDelayCount = 0;
                            btn->mouseUpDelayCount = 0;
                        }
                    }
                }
			}
            result |= ituExecActions((ITUWidget*)btn, btn->actions, ev, arg1);
        }
    }
    else if (ev == ITU_EVENT_LOAD)
    {
        ituButtonLoadStaticData(btn);
        result = true;
    }
    else if (ev == ITU_EVENT_LOAD_EXTERNAL)
    {
        ButtonLoadExternalData(btn, (ITULayer*)arg1);
        result = true;
    }
    else if (ev == ITU_EVENT_RELEASE)
    {
        ituButtonReleaseSurface(btn);
        result = true;
    }
    else if (ituWidgetIsActive(widget) && ituWidgetIsEnabled(widget) && !result)
    {
        switch (ev)
        {
        case ITU_EVENT_KEYDOWN:
            if (arg1 == ituScene->enterKey)
            {
                ituButtonSetPressed(btn, true);
                ituFocusWidget(btn);
                result |= ituExecActions((ITUWidget*)btn, btn->actions, ev, arg1);
            }
            break;

        case ITU_EVENT_KEYUP:
            if (arg1 == ituScene->enterKey)
            {
                ituButtonSetPressed(btn, false);
                result |= ituExecActions((ITUWidget*)btn, btn->actions, ev, arg1);
            }
            break;
        }
        result |= widget->dirty;
    }
    else if (ev == ITU_EVENT_LAYOUT)
    {
        ituButtonSetPressed(btn, btn->pressed);
        result = widget->dirty = true;
    }
    return result;
}

void ituButtonCalPos(ITUWidget* widget, int* imgdestx, int* imgdesty)
{
	ITUButton* btn = (ITUButton*)widget;
	ITUSurface* imgSurf;
	ITURectangle* rect = (ITURectangle*)&widget->rect;
	imgSurf = btn->bg.icon.surf;

	if ((btn->imagepos == ITU_LAYOUT_TOP_LEFT) || (btn->imagepos < 2))
		return;

	if (btn->imagepos == ITU_LAYOUT_TOP_CENTER)
	{
		*imgdestx += (rect->width - imgSurf->width) / 2;
	}
	else if (btn->imagepos == ITU_LAYOUT_TOP_RIGHT)
	{
		*imgdestx += (rect->width - imgSurf->width);
	}
	else if (btn->imagepos == ITU_LAYOUT_MIDDLE_LEFT)
	{
		*imgdesty += (rect->height - imgSurf->height) / 2;
	}
	else if (btn->imagepos == ITU_LAYOUT_MIDDLE_RIGHT)
	{
		*imgdestx += (rect->width - imgSurf->width);
		*imgdesty += (rect->height - imgSurf->height) / 2;
	}
	else if (btn->imagepos == ITU_LAYOUT_BOTTOM_LEFT)
	{
		*imgdesty += (rect->height - imgSurf->height);
	}
	else if (btn->imagepos == ITU_LAYOUT_BOTTOM_CENTER)
	{
		*imgdestx += (rect->width - imgSurf->width) / 2;
		*imgdesty += (rect->height - imgSurf->height);
	}
	else if (btn->imagepos == ITU_LAYOUT_BOTTOM_RIGHT)
	{
		*imgdestx += (rect->width - imgSurf->width);
		*imgdesty += (rect->height - imgSurf->height);
	}
	else //default middle_center
	{
		*imgdestx += (rect->width - imgSurf->width) / 2;
		*imgdesty += (rect->height - imgSurf->height) / 2;
	}
}

void ituButtonDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    int destx, desty;
    uint8_t desta;
    ITURectangle prevClip;
    ITUButton* btn = (ITUButton*) widget;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    assert(btn);
    assert(dest);

    if ((btn->pressed && btn->pressSurf) || (widget->active && btn->focusSurf))
    {
        ITUSurface* srcSurf;

        destx = rect->x + x;
        desty = rect->y + y;
        desta = alpha * widget->color.alpha / 255;
        desta = desta * widget->alpha / 255;

        if (widget->angle == 0)
            ituWidgetSetClipping(widget, dest, x, y, &prevClip);

        if (btn->pressed && btn->pressSurf)
        {
			if ((btn->press_alpha_delay_count > 0) && (btn->press_alpha_delay_count < 255))
				return;
			
			srcSurf = btn->pressSurf;
        }
        else // widget->active == true
        {
            srcSurf = btn->focusSurf;
        }

        if (!srcSurf || 
            (srcSurf->width < widget->rect.width || srcSurf->height < widget->rect.height) ||
            (srcSurf->format == ITU_ARGB1555 || srcSurf->format == ITU_ARGB4444 || srcSurf->format == ITU_ARGB8888))
        {
            if (desta > 0)
            {
                if (desta == 255)
                {
                    ituColorFill(dest, destx, desty, rect->width, rect->height, &widget->color);
                }
                else
                {
                    ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
                    if (surf)
                    {
                        ituColorFill(surf, 0, 0, rect->width, rect->height, &widget->color);
                        ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);                
                        ituDestroySurface(surf);
                    }
                }
            }
        }

		//Bless added to re-calculate position for current layout alignment
		ituButtonCalPos(widget, &destx, &desty);

        if (srcSurf)
        {
            desta = alpha * widget->alpha / 255;
            if (desta > 0)
            {
                if (desta == 255)
                {
                    if (widget->flags & ITU_STRETCH)
                        ituStretchBlt(dest, destx, desty, rect->width, rect->height, srcSurf, 0, 0, srcSurf->width, srcSurf->height);
                    else
                        ituBitBlt(dest, destx, desty, srcSurf->width, srcSurf->height, srcSurf, 0, 0);
                }
                else
                {
                    if (widget->flags & ITU_STRETCH)
                    {
                        ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
                        if (surf)
                        {
                            ituBitBlt(surf, 0, 0, rect->width, rect->height, dest, destx, desty);
                            ituStretchBlt(surf, 0, 0, rect->width, rect->height, srcSurf, 0, 0, srcSurf->width, srcSurf->height);
                            ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);
                            ituDestroySurface(surf);
                        }
                    }
                    else
                    {
                        ituAlphaBlend(dest, destx, desty, srcSurf->width, srcSurf->height, srcSurf, 0, 0, desta);
                    }
                }
            }
        }
        if (widget->angle == 0)
            ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);

        ituWidgetDrawImpl(widget, dest, x, y, alpha);
    }
    else
    {
		if ((btn->imagepos != ITU_LAYOUT_TOP_LEFT) && (btn->imagepos > 1))
		{
			ITUSurface* imgSurf = btn->bg.icon.surf;
			uint8_t imgdest = alpha * widget->alpha / 255;
			destx = rect->x + x;
			desty = rect->y + y;

			if (widget->color.alpha)
			{
				if (widget->color.alpha < 255)
				{
					ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
					if (surf)
					{
						ITUColor color;
						ituSetColor(&color, 255, widget->color.red, widget->color.green, widget->color.blue);
						ituColorFill(surf, 0, 0, rect->width, rect->height, &color);
						ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, widget->color.alpha);
						ituDestroySurface(surf);
					}
				}
				else
					ituColorFill(dest, destx, desty, rect->width, rect->height, &widget->color);
			}

			//Bless added to re-calculate position for current layout alignment
			ituButtonCalPos(widget, &destx, &desty);

			if (imgdest == 255)
				ituBitBlt(dest, destx, desty, imgSurf->width, imgSurf->height, imgSurf, 0, 0);
			else
				ituAlphaBlend(dest, destx, desty, imgSurf->width, imgSurf->height, imgSurf, 0, 0, imgdest);

			ituWidgetDrawImpl(widget, dest, x, y, alpha);
		}
		else
			ituBackgroundDraw(widget, dest, x, y, alpha);
    }
}

void ituButtonOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    ITUButton* button = (ITUButton*) widget;
    assert(button);

    switch (action)
    {
    case ITU_ACTION_DODELAY0:
        ituExecActions(widget, button->actions, ITU_EVENT_DELAY0, atoi(param));
        break;

    case ITU_ACTION_DODELAY1:
        ituExecActions(widget, button->actions, ITU_EVENT_DELAY1, atoi(param));
        break;

    case ITU_ACTION_DODELAY2:
        ituExecActions(widget, button->actions, ITU_EVENT_DELAY2, atoi(param));
        break;

    case ITU_ACTION_DODELAY3:
        ituExecActions(widget, button->actions, ITU_EVENT_DELAY3, atoi(param));
        break;

    case ITU_ACTION_DODELAY4:
        ituExecActions(widget, button->actions, ITU_EVENT_DELAY4, atoi(param));
        break;

    case ITU_ACTION_DODELAY5:
        ituExecActions(widget, button->actions, ITU_EVENT_DELAY5, atoi(param));
        break;

    case ITU_ACTION_DODELAY6:
        ituExecActions(widget, button->actions, ITU_EVENT_DELAY6, atoi(param));
        break;

    case ITU_ACTION_DODELAY7:
        ituExecActions(widget, button->actions, ITU_EVENT_DELAY7, atoi(param));
        break;

    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

void ituButtonInit(ITUButton* btn)
{
    assert(btn);
    ITU_ASSERT_THREAD();

    memset(btn, 0, sizeof (ITUButton));

    ituBackgroundInit(&btn->bg);
    ituBorderWindowInit(&btn->bwin);
    ituTextInit(&btn->text);

    ituWidgetSetType(btn, ITU_BUTTON);
    ituWidgetSetName(btn, btnName);
    ituWidgetSetExit(btn, ituButtonExit);
    ituWidgetSetClone(btn, ituButtonClone);
    ituWidgetSetUpdate(btn, ituButtonUpdate);
    ituWidgetSetDraw(btn, ituButtonDraw);
    ituWidgetSetOnAction(btn, ituButtonOnAction);

    ituBorderWindowAdd(&btn->bwin, &btn->text.widget, ITU_LAYOUT_CENTER);
}

void ituButtonLoad(ITUButton* btn, uint32_t base)
{
    ITUWidget* widget = (ITUWidget*)btn;
    assert(btn);

    ituBackgroundLoad(&btn->bg, base);
    ituWidgetSetExit(btn, ituButtonExit);
    ituWidgetSetClone(btn, ituButtonClone);
    ituWidgetSetUpdate(btn, ituButtonUpdate);
    ituWidgetSetDraw(btn, ituButtonDraw);
    ituWidgetSetOnAction(btn, ituButtonOnAction);

    if (!(widget->flags & ITU_EXTERNAL))
    {
        if (btn->staticFocusSurf)
        {
            ITUSurface* surf = (ITUSurface*)(base + (uint32_t)btn->staticFocusSurf);
            if (surf->flags & ITU_COMPRESSED)
                btn->focusSurf = NULL;
            else
                btn->focusSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);

            btn->staticFocusSurf = surf;
        }

        if (btn->staticPressSurf)
        {
            ITUSurface* surf = (ITUSurface*)(base + (uint32_t)btn->staticPressSurf);
            if (surf->flags & ITU_COMPRESSED)
                btn->pressSurf = NULL;
            else
                btn->pressSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);

            btn->staticPressSurf = surf;
        }
    }

    switch (btn->text.layout)
    {
    case ITU_LAYOUT_TOP_LEFT:
        btn->bwin.widgets[ITU_LAYOUT_LEFT] = &btn->text.widget;
        break;

    case ITU_LAYOUT_TOP_RIGHT:
        btn->bwin.widgets[ITU_LAYOUT_RIGHT] = &btn->text.widget;
        break;

    case ITU_LAYOUT_TOP_CENTER:
        btn->bwin.widgets[ITU_LAYOUT_UP] = &btn->text.widget;
        break;

    case ITU_LAYOUT_MIDDLE_LEFT:
        btn->bwin.widgets[ITU_LAYOUT_LEFT] = &btn->text.widget;
        break;

    case ITU_LAYOUT_MIDDLE_RIGHT:
        btn->bwin.widgets[ITU_LAYOUT_RIGHT] = &btn->text.widget;
        break;

    case ITU_LAYOUT_MIDDLE_CENTER:
        btn->bwin.widgets[ITU_LAYOUT_CENTER] = &btn->text.widget;
        break;

    case ITU_LAYOUT_BOTTOM_LEFT:
        btn->bwin.widgets[ITU_LAYOUT_LEFT] = &btn->text.widget;
        break;

    case ITU_LAYOUT_BOTTOM_RIGHT:
        btn->bwin.widgets[ITU_LAYOUT_RIGHT] = &btn->text.widget;
        break;

    case ITU_LAYOUT_BOTTOM_CENTER:
        btn->bwin.widgets[ITU_LAYOUT_DOWN] = &btn->text.widget;
        break;

    default:
        btn->bwin.widgets[btn->text.layout] = &btn->text.widget;
        break;
    }
}

void ituButtonSetPressed(ITUButton* btn, bool pressed)
{
    ITUWidget* widget = (ITUWidget*) btn;
    assert(btn);
    ITU_ASSERT_THREAD();

    if (pressed && btn->pressColor.alpha > 0)
    {
        ituWidgetSetColor(widget, btn->pressColor.alpha, btn->pressColor.red, btn->pressColor.green, btn->pressColor.blue);
    }
    else
    {
        if (widget->active && btn->focusColor.alpha > 0)
        {
            ituWidgetSetColor(widget, btn->focusColor.alpha, btn->focusColor.red, btn->focusColor.green, btn->focusColor.blue);
        }
        else
        {
            ituWidgetSetColor(widget, btn->bgColor.alpha, btn->bgColor.red, btn->bgColor.green, btn->bgColor.blue);
        }
    }
    btn->pressed = pressed;
    widget->dirty = true;
}

void ituButtonSetStringImpl(ITUButton* btn, char* string)
{
    assert(btn);
    ITU_ASSERT_THREAD();

    ituTextSetString(&btn->text, string);
    ituTextResize(&btn->text);
    ituWidgetUpdate(btn, ITU_EVENT_LAYOUT, 0, 0, 0);
}

void ituButtonLoadStaticData(ITUButton* btn)
{
    ITUWidget* widget = (ITUWidget*)btn;
    ITUSurface* surf;

    if (widget->flags & ITU_EXTERNAL)
        return;

    if (btn->staticFocusSurf && !btn->focusSurf)
    {
        surf = btn->staticFocusSurf;

        if (surf->flags & ITU_COMPRESSED)
            btn->focusSurf = ituSurfaceDecompress(surf);
        else
            btn->focusSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }

    if (btn->staticPressSurf && !btn->pressSurf)
    {
        surf = btn->staticPressSurf;

        if (surf->flags & ITU_COMPRESSED)
            btn->pressSurf = ituSurfaceDecompress(surf);
        else
            btn->pressSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }
}

void ituButtonReleaseSurface(ITUButton* btn)
{
    ITUWidget* widget = (ITUWidget*)btn;
    ITULayer* layer = NULL;
    ITU_ASSERT_THREAD();

    if (btn->pressSurf)
    {
        ituSurfaceRelease(btn->pressSurf);
        btn->pressSurf = NULL;

        if (widget->flags & ITU_EXTERNAL)
        {
            layer = ituGetLayer(widget);

            if (btn->staticPressSurf)
            {
                ituLayerReleaseExternalSurface(layer);
            }
        }
    }

    if (btn->focusSurf)
    {
        ituSurfaceRelease(btn->focusSurf);
        btn->focusSurf = NULL;

        if (widget->flags & ITU_EXTERNAL)
        {
            if (!layer)
                layer = ituGetLayer(widget);

            if (btn->staticFocusSurf)
            {
                ituLayerReleaseExternalSurface(layer);
            }
        }
    }
}

void ituButtonSetTextLayoutImpl(ITUButton* btn, ITULayout layout)
{
    int i;
    assert(btn);
    ITU_ASSERT_THREAD();

    for (i = 0; i < 5; ++i)
        btn->bwin.widgets[i] = NULL;

    switch (layout)
    {
    case ITU_LAYOUT_TOP_LEFT:
        btn->bwin.widgets[ITU_LAYOUT_LEFT] = &btn->text.widget;
        break;

    case ITU_LAYOUT_TOP_RIGHT:
        btn->bwin.widgets[ITU_LAYOUT_RIGHT] = &btn->text.widget;
        break;

    case ITU_LAYOUT_TOP_CENTER:
        btn->bwin.widgets[ITU_LAYOUT_UP] = &btn->text.widget;
        break;

    case ITU_LAYOUT_MIDDLE_LEFT:
        btn->bwin.widgets[ITU_LAYOUT_LEFT] = &btn->text.widget;
        break;

    case ITU_LAYOUT_MIDDLE_RIGHT:
        btn->bwin.widgets[ITU_LAYOUT_RIGHT] = &btn->text.widget;
        break;

    case ITU_LAYOUT_MIDDLE_CENTER:
        btn->bwin.widgets[ITU_LAYOUT_CENTER] = &btn->text.widget;
        break;

    case ITU_LAYOUT_BOTTOM_LEFT:
        btn->bwin.widgets[ITU_LAYOUT_LEFT] = &btn->text.widget;
        break;

    case ITU_LAYOUT_BOTTOM_RIGHT:
        btn->bwin.widgets[ITU_LAYOUT_RIGHT] = &btn->text.widget;
        break;

    case ITU_LAYOUT_BOTTOM_CENTER:
        btn->bwin.widgets[ITU_LAYOUT_DOWN] = &btn->text.widget;
        break;

    default:
        btn->bwin.widgets[layout] = &btn->text.widget;
        break;
    }
    btn->text.layout = layout;
    ituWidgetUpdate(btn, ITU_EVENT_LAYOUT, 0, 0, 0);
}

//void ituButtonSetCheckStat(ITUButton* btn, bool ckstat)
//{
//	assert(btn);
//	ITU_ASSERT_THREAD();
//
//}

void ituButtonSetSlideMouseUP(ITUButton* btn)
{
	assert(btn);
	ITU_ASSERT_THREAD();
	btn->fsg1 = 1;
}
