#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char simpleAnimationName[] = "ITUSimpleAnimation";

static void SimpleAnimationOnStop(ITUSimpleAnimation* sanim)
{
    // DO NOTHING
}

bool ituSimpleAnimationClone(ITUWidget* widget, ITUWidget** cloned)
{
    assert(widget);
    assert(cloned);
    ITU_ASSERT_THREAD();

    if (*cloned == NULL)
    {
        ITUWidget* newWidget = malloc(sizeof(ITUSimpleAnimation));
        if (newWidget == NULL)
            return false;

        memcpy(newWidget, widget, sizeof(ITUSimpleAnimation));
        newWidget->tree.child = newWidget->tree.parent = newWidget->tree.sibling = NULL;
        *cloned = newWidget;
    }

    return ituWidgetCloneImpl(widget, cloned);
}

bool ituSimpleAnimationUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    ITUSimpleAnimation* sanim = (ITUSimpleAnimation*) widget;
    bool result = false;
    assert(sanim);

    if (ev == ITU_EVENT_TIMER)
    {
        if (sanim->playing)
        {
            if (--sanim->delayCount <= 0)
            {
                if (sanim->frame >= sanim->totalframe)
                {
                    if (sanim->playCount > 0 && --sanim->playCount <= 0)
                    {
                        ituSimpleAnimationStop(sanim);
                        sanim->frame = 0;

                        if ((sanim->animationFlags & ITU_ANIM_MOVE) || (sanim->animationFlags & ITU_ANIM_EASE_IN) || (sanim->animationFlags & ITU_ANIM_EASE_OUT) || (sanim->animationFlags & ITU_ANIM_SCALE))
                            memcpy(&sanim->child->rect, &sanim->orgRect, sizeof (ITURectangle));

                        if (sanim->animationFlags & ITU_ANIM_COLOR)
                        {
                            memcpy(&sanim->child->color, &sanim->orgColor, sizeof (ITUColor));
                            sanim->child->alpha = sanim->orgAlpha;
                        }

                        if (sanim->animationFlags & ITU_ANIM_ROTATE)
                            sanim->child->angle = sanim->orgAngle;

                        if (sanim->animationFlags & ITU_ANIM_TRANSFORM)
                        {
                            sanim->child->transformX = sanim->orgTransformX;
                            sanim->child->transformY = sanim->orgTransformY;
                        }

                        if (sanim->animationFlags & ITU_ANIM_REVERSE)
                            ituSimpleAnimationGoto(sanim, sanim->totalframe);
                        else
                            ituSimpleAnimationGoto(sanim, 0);

                        sanim->playCount = 0;
                        ituSimpleAnimationOnStop(sanim);
                        ituExecActions((ITUWidget*)sanim, sanim->actions, ITU_EVENT_STOPPED, 0);
                    }
                    else
                    {
                        if (sanim->animationFlags & ITU_ANIM_REVERSE)
                        {
                            if (sanim->repeat)
                            {
                                if (sanim->animationFlags & ITU_ANIM_CYCLE)
                                    sanim->animationFlags &= ~ITU_ANIM_REVERSE;

                                sanim->frame = 0;
                            }
                            else
                            {
                                ituSimpleAnimationStop(sanim);
                                sanim->frame = 0;

                                if ((sanim->animationFlags & ITU_ANIM_MOVE) || (sanim->animationFlags & ITU_ANIM_EASE_IN) || (sanim->animationFlags & ITU_ANIM_EASE_OUT) || (sanim->animationFlags & ITU_ANIM_SCALE))
                                    memcpy(&sanim->child->rect, &sanim->orgRect, sizeof (ITURectangle));

                                if (sanim->animationFlags & ITU_ANIM_COLOR)
                                {
                                    memcpy(&sanim->child->color, &sanim->orgColor, sizeof (ITUColor));
                                    sanim->child->alpha = sanim->orgAlpha;
                                }

                                if (sanim->animationFlags & ITU_ANIM_ROTATE)
                                    sanim->child->angle = sanim->orgAngle;

                                if (sanim->animationFlags & ITU_ANIM_TRANSFORM)
                                {
                                    sanim->child->transformX = sanim->orgTransformX;
                                    sanim->child->transformY = sanim->orgTransformY;
                                }
                                ituSimpleAnimationOnStop(sanim);
                                ituExecActions((ITUWidget*)sanim, sanim->actions, ITU_EVENT_STOPPED, 0);
                            }
                        }
                        else
                        {
                            if (sanim->repeat)
                            {
                                if (sanim->animationFlags & ITU_ANIM_CYCLE)
                                    sanim->animationFlags |= ITU_ANIM_REVERSE;

                                sanim->frame = 0;
                            }
                            else
                            {
                                ituSimpleAnimationStop(sanim);
                                ituSimpleAnimationOnStop(sanim);
                                ituExecActions((ITUWidget*)sanim, sanim->actions, ITU_EVENT_STOPPED, 0);
                            }
                        }
                    }
                }
                else
                {
                    int frame;

                    ++sanim->frame;

                    if (sanim->animationFlags & ITU_ANIM_REVERSE)
                    {
                        frame = sanim->totalframe - sanim->frame;
                    }
                    else
                    {
                        frame = sanim->frame;
                    }

                    if (sanim->animationFlags & ITU_ANIM_MOVE)
                    {
                        sanim->child->rect.x = sanim->orgRect.x + (sanim->keyRect.x - sanim->orgRect.x) * frame / sanim->totalframe;
                        sanim->child->rect.y = sanim->orgRect.y + (sanim->keyRect.y - sanim->orgRect.y) * frame / sanim->totalframe;
                    }
					else if (sanim->animationFlags & ITU_ANIM_EASE_IN)
					{
						float step = (float)frame / sanim->totalframe;
						step = step * step * step;
                        sanim->child->rect.x = sanim->orgRect.x + (int)((sanim->keyRect.x - sanim->orgRect.x) * step);
                        sanim->child->rect.y = sanim->orgRect.y + (int)((sanim->keyRect.y - sanim->orgRect.y) * step);
					}
					else if (sanim->animationFlags & ITU_ANIM_EASE_OUT)
					{
						float step = (float)frame / sanim->totalframe;
						step = step - 1;
						step = step * step * step + 1;
                        sanim->child->rect.x = sanim->orgRect.x + (int)((sanim->keyRect.x - sanim->orgRect.x) * step);
                        sanim->child->rect.y = sanim->orgRect.y + (int)((sanim->keyRect.y - sanim->orgRect.y) * step);
					}

                    if (sanim->animationFlags & ITU_ANIM_SCALE)
                    {
						if (!(sanim->animationFlags & ITU_ANIM_MOVE) && !(sanim->animationFlags & ITU_ANIM_EASE_IN) && !(sanim->animationFlags & ITU_ANIM_EASE_OUT) && (sanim->animationFlags & ITU_ANIM_SCALE_CENTER))
                        {
                            sanim->child->rect.x = sanim->orgRect.x - (sanim->keyRect.width - sanim->orgRect.width) / 2 * frame / sanim->totalframe;
                            sanim->child->rect.y = sanim->orgRect.y - (sanim->keyRect.height - sanim->orgRect.height) / 2 * frame / sanim->totalframe;
                            sanim->child->rect.width = sanim->orgRect.width + (sanim->keyRect.width - sanim->orgRect.width) * frame / sanim->totalframe / 2;
                            sanim->child->rect.height = sanim->orgRect.height + (sanim->keyRect.height - sanim->orgRect.height) * frame / sanim->totalframe / 2;
                        }
						else if (sanim->animationFlags & ITU_ANIM_EASE_IN)
						{
							float step = (float)frame / sanim->totalframe;
							step = step * step * step;
                            sanim->child->rect.width = sanim->orgRect.width + (int)((sanim->keyRect.width - sanim->orgRect.width) * step);
                            sanim->child->rect.height = sanim->orgRect.height + (int)((sanim->keyRect.height - sanim->orgRect.height) * step);
						}
						else if (sanim->animationFlags & ITU_ANIM_EASE_OUT)
						{
							float step = (float)frame / sanim->totalframe;
							step = step - 1;
							step = step * step * step + 1;
                            sanim->child->rect.width = sanim->orgRect.width + (int)((sanim->keyRect.width - sanim->orgRect.width) * step);
                            sanim->child->rect.height = sanim->orgRect.height + (int)((sanim->keyRect.height - sanim->orgRect.height) * step);
						}
                        else
                        {
                            sanim->child->rect.width = sanim->orgRect.width + (sanim->keyRect.width - sanim->orgRect.width) * frame / sanim->totalframe;
                            sanim->child->rect.height = sanim->orgRect.height + (sanim->keyRect.height - sanim->orgRect.height) * frame / sanim->totalframe;
                            //printf("n=%s c=%d/%d t=%d/%d k=%d/%d f=%d/%d\n", target->name, sanim->child->rect.width, sanim->child->rect.height, target->rect.width, target->rect.height, sanim->keyRect.width, sanim->keyRect.height, frame, sanim->totalframe);
                        }
                    }
                    if (sanim->animationFlags & ITU_ANIM_COLOR)
                    {
                        sanim->child->color.alpha = sanim->orgColor.alpha + (sanim->keyColor.alpha - sanim->orgColor.alpha) * frame / sanim->totalframe;
                        sanim->child->color.red = sanim->orgColor.red + (sanim->keyColor.red - sanim->orgColor.red) * frame / sanim->totalframe;
                        sanim->child->color.green = sanim->orgColor.green + (sanim->keyColor.green - sanim->orgColor.green) * frame / sanim->totalframe;
                        sanim->child->color.blue = sanim->orgColor.blue + (sanim->keyColor.blue - sanim->orgColor.blue) * frame / sanim->totalframe;
                        sanim->child->alpha = sanim->orgAlpha + (sanim->keyAlpha - sanim->orgAlpha) * frame / sanim->totalframe;
                    }
                    if (sanim->animationFlags & ITU_ANIM_ROTATE)
                    {
                        sanim->child->angle = sanim->orgAngle + (sanim->keyAngle - sanim->orgAngle) * frame / sanim->totalframe;
                    }
                    if (sanim->animationFlags & ITU_ANIM_TRANSFORM)
                    {
                        sanim->child->transformX = sanim->orgTransformX + (sanim->keyTransformX - sanim->orgTransformX) * frame / sanim->totalframe;
                        sanim->child->transformY = sanim->orgTransformY + (sanim->keyTransformY - sanim->orgTransformY) * frame / sanim->totalframe;
                    }
                }
                sanim->delayCount = sanim->delay;
                result = widget->dirty = true;
            }
        }
    }
    else if (ev == ITU_EVENT_LAYOUT)
    {
        if (!sanim->child)
        {
            ITUWidget* target;

            sanim->child = (ITUWidget*) itcTreeGetChildAt(sanim, 0);
            if (sanim->child)
            {
                memcpy(&sanim->orgRect, &sanim->child->rect, sizeof (ITURectangle));
                memcpy(&sanim->keyRect, &sanim->child->rect, sizeof (ITURectangle));
                memcpy(&sanim->orgColor, &sanim->child->color, sizeof (ITUColor));
                sanim->orgAlpha = sanim->child->alpha;
                memcpy(&sanim->keyColor, &sanim->child->color, sizeof (ITUColor));
                sanim->keyAlpha = sanim->child->alpha;
                sanim->orgAngle = sanim->child->angle;
                sanim->keyAngle = sanim->child->angle;
                sanim->orgTransformX = sanim->child->transformX;
                sanim->orgTransformY = sanim->child->transformY;
                sanim->keyTransformX = sanim->child->transformX;
                sanim->keyTransformY = sanim->child->transformY;
                sanim->frame = 0;
                result = widget->dirty = true;
            }

            target = (ITUWidget*)itcTreeGetChildAt(sanim, 1);
            if (target)
            {
                if ((sanim->animationFlags & ITU_ANIM_MOVE) || (sanim->animationFlags & ITU_ANIM_EASE_IN) || (sanim->animationFlags & ITU_ANIM_EASE_OUT) || (sanim->animationFlags & ITU_ANIM_SCALE))
                    memcpy(&sanim->keyRect, &target->rect, sizeof (ITURectangle));

                if (sanim->animationFlags & ITU_ANIM_COLOR)
                {
                    memcpy(&sanim->keyColor, &target->color, sizeof (ITUColor));
                    sanim->keyAlpha = target->alpha;
                }
                if (sanim->animationFlags & ITU_ANIM_ROTATE)
                    sanim->keyAngle = target->angle;

                if (sanim->animationFlags & ITU_ANIM_TRANSFORM)
                {
                    sanim->keyTransformX = target->transformX;
                    sanim->keyTransformY = target->transformY;
                }
            }
        }
    }
    else if (ev == ITU_EVENT_MOUSEDOWN || ev == ITU_EVENT_MOUSEUP || ev == ITU_EVENT_MOUSEDOUBLECLICK || ev == ITU_EVENT_MOUSEMOVE || ev == ITU_EVENT_MOUSELONGPRESS ||
        ev == ITU_EVENT_TOUCHSLIDELEFT || ev == ITU_EVENT_TOUCHSLIDEUP || ev == ITU_EVENT_TOUCHSLIDERIGHT || ev == ITU_EVENT_TOUCHSLIDEDOWN || ev == ITU_EVENT_TOUCHPINCH)
    {
        if (sanim->child)
        {
            arg2 -= widget->rect.x;
            arg3 -= widget->rect.y;
            result |= ituWidgetUpdate(sanim->child, ev, arg1, arg2, arg3);
            return widget->visible ? result : false;
        }
    }
    result |= ituWidgetUpdateImpl(widget, ev, arg1, arg2, arg3);
    return widget->visible ? result : false;
}

void ituSimpleAnimationDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    ITUSimpleAnimation* sanim = (ITUSimpleAnimation*) widget;

    if (sanim->child)
    {
        ITURectangle prevClip;

        ituWidgetSetClipping(widget, dest, x, y, &prevClip);

        x += widget->rect.x;
        y += widget->rect.y;
        alpha = alpha * widget->alpha / 255;

        if ((sanim->animationFlags & ITU_ANIM_MOTION_BLUR))
        {
            int frame = -1;

            if (sanim->animationFlags & ITU_ANIM_REVERSE)
            {
                if (sanim->frame < sanim->totalframe - 1)
                    frame = sanim->frame + 1;
            }
            else
            {
                if (sanim->frame > 0)
                    frame = sanim->frame - 1;
            }

            if (frame != -1)
            {
                ITURectangle currRect;

                memcpy(&currRect, &sanim->child->rect, sizeof (ITURectangle));

                if (sanim->animationFlags & ITU_ANIM_MOVE)
                {
                    sanim->child->rect.x = sanim->orgRect.x + (sanim->keyRect.x - sanim->orgRect.x) * frame / sanim->totalframe;
                    sanim->child->rect.y = sanim->orgRect.y + (sanim->keyRect.y - sanim->orgRect.y) * frame / sanim->totalframe;
                }
                else if (sanim->animationFlags & ITU_ANIM_EASE_IN)
                {
                    float step = (float)frame / sanim->totalframe;
                    step = step * step * step;
                    sanim->child->rect.x = sanim->orgRect.x + (int)((sanim->keyRect.x - sanim->orgRect.x) * step);
                    sanim->child->rect.y = sanim->orgRect.y + (int)((sanim->keyRect.y - sanim->orgRect.y) * step);
                }
                else if (sanim->animationFlags & ITU_ANIM_EASE_OUT)
                {
                    float step = (float)frame / sanim->totalframe;
                    step = step - 1;
                    step = step * step * step + 1;
                    sanim->child->rect.x = sanim->orgRect.x + (int)((sanim->keyRect.x - sanim->orgRect.x) * step);
                    sanim->child->rect.y = sanim->orgRect.y + (int)((sanim->keyRect.y - sanim->orgRect.y) * step);
                }
                if (sanim->animationFlags & ITU_ANIM_SCALE)
                {
                    if (!(sanim->animationFlags & ITU_ANIM_MOVE) && !(sanim->animationFlags & ITU_ANIM_EASE_IN) && !(sanim->animationFlags & ITU_ANIM_EASE_OUT) && (sanim->animationFlags & ITU_ANIM_SCALE_CENTER))
                    {
                        sanim->child->rect.x = sanim->orgRect.x - (sanim->keyRect.width - sanim->orgRect.width) / 2 * frame / sanim->totalframe;
                        sanim->child->rect.y = sanim->orgRect.y - (sanim->keyRect.height - sanim->orgRect.height) / 2 * frame / sanim->totalframe;
                        sanim->child->rect.width = sanim->orgRect.width + (sanim->keyRect.width - sanim->orgRect.width) * frame / sanim->totalframe / 2;
                        sanim->child->rect.height = sanim->orgRect.height + (sanim->keyRect.height - sanim->orgRect.height) * frame / sanim->totalframe / 2;
                    }
                    else if (sanim->animationFlags & ITU_ANIM_EASE_IN)
                    {
                        float step = (float)frame / sanim->totalframe;
                        step = step * step * step;
                        sanim->child->rect.width = sanim->orgRect.width + (int)((sanim->keyRect.width - sanim->orgRect.width) * step);
                        sanim->child->rect.height = sanim->orgRect.height + (int)((sanim->keyRect.height - sanim->orgRect.height) * step);
                    }
                    else if (sanim->animationFlags & ITU_ANIM_EASE_OUT)
                    {
                        float step = (float)frame / sanim->totalframe;
                        step = step - 1;
                        step = step * step * step + 1;
                        sanim->child->rect.width = sanim->orgRect.width + (int)((sanim->keyRect.width - sanim->orgRect.width) * step);
                        sanim->child->rect.height = sanim->orgRect.height + (int)((sanim->keyRect.height - sanim->orgRect.height) * step);
                    }
                    else
                    {
                        sanim->child->rect.width = sanim->orgRect.width + (sanim->keyRect.width - sanim->orgRect.width) * frame / sanim->totalframe;
                        sanim->child->rect.height = sanim->orgRect.height + (sanim->keyRect.height - sanim->orgRect.height) * frame / sanim->totalframe;
                        //printf("n=%s c=%d/%d t=%d/%d k=%d/%d f=%d/%d\n", target->name, sanim->child->rect.width, sanim->child->rect.height, target->rect.width, target->rect.height, sanim->keyRect.width, sanim->keyRect.height, frame, sanim->totalframe);
                    }
                }
                ituWidgetDraw(sanim->child, dest, x, y, alpha / 5);

                memcpy(&sanim->child->rect, &currRect, sizeof (ITURectangle));
            }
        }

        ituWidgetDraw(sanim->child, dest, x, y, alpha);

        ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
    }
    ituDirtyWidget(sanim, false);
}

void ituSimpleAnimationOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    ITUSimpleAnimation* sanim = (ITUSimpleAnimation*) widget;
    assert(widget);

    switch (action)
    {
    case ITU_ACTION_PLAY:
        if (param[0] != '\0')
        {
            sanim->repeat = false;
            sanim->playCount = atoi(param);
            ituSimpleAnimationPlay(sanim);
        }
        else
        {
            ituSimpleAnimationPlay(sanim);
        }
        break;

    case ITU_ACTION_BACK:
        if (param[0] != '\0')
        {
            sanim->repeat = false;
            sanim->playCount = atoi(param);
            ituSimpleAnimationReversePlay(sanim);
        }
        else
        {
            ituSimpleAnimationReversePlay(sanim);
        }
        break;

    case ITU_ACTION_STOP:
        ituSimpleAnimationStop(sanim);
        ituSimpleAnimationOnStop(sanim);
        ituExecActions(widget, sanim->actions, ITU_EVENT_STOPPED, 0);
        break;

    case ITU_ACTION_GOTO:
        if (param[0] != '\0')
            ituSimpleAnimationGoto(sanim, atoi(param));
        break;

    case ITU_ACTION_RELOAD:
        ituSimpleAnimationReset(sanim);
        break;

    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

void ituSimpleAnimationInit(ITUSimpleAnimation* sanim)
{
    assert(sanim);
    ITU_ASSERT_THREAD();

    memset(sanim, 0, sizeof (ITUSimpleAnimation));

    ituWidgetInit(&sanim->widget);

    ituWidgetSetType(sanim, ITU_ANIMATION);
    ituWidgetSetName(sanim, simpleAnimationName);
    ituWidgetSetClone(sanim, ituSimpleAnimationClone);
    ituWidgetSetUpdate(sanim, ituSimpleAnimationUpdate);
    ituWidgetSetDraw(sanim, ituSimpleAnimationDraw);
    ituWidgetSetOnAction(sanim, ituSimpleAnimationOnAction);
    ituSimpleAnimationSetOnStop(sanim, SimpleAnimationOnStop);

    ituSimpleAnimationSetDelay(sanim, 10);
    sanim->child = (ITUWidget*) itcTreeGetChildAt(sanim, 0);
}

void ituSimpleAnimationLoad(ITUSimpleAnimation* sanim, uint32_t base)
{
    assert(sanim);

    ituWidgetLoad((ITUWidget*)sanim, base);
    ituWidgetSetClone(sanim, ituSimpleAnimationClone);
    ituWidgetSetUpdate(sanim, ituSimpleAnimationUpdate);
    ituWidgetSetDraw(sanim, ituSimpleAnimationDraw);
    ituWidgetSetOnAction(sanim, ituSimpleAnimationOnAction);
    ituSimpleAnimationSetOnStop(sanim, SimpleAnimationOnStop);

    if (sanim->playing)
        sanim->delayCount = sanim->delay;
}

void ituSimpleAnimationSetDelay(ITUSimpleAnimation* sanim, int delay)
{
    assert(sanim);
    ITU_ASSERT_THREAD();

    sanim->delay           = delay;
    sanim->widget.dirty    = true;
}

void ituSimpleAnimationPlay(ITUSimpleAnimation* sanim)
{
    ITUWidget* target;

    assert(sanim);
    ITU_ASSERT_THREAD();

    if (sanim->frame == sanim->totalframe)
        return;

	sanim->animationFlags &= ~ITU_ANIM_REVERSE;

    target = (ITUWidget*) itcTreeGetChildAt(sanim, 1);

    sanim->delayCount = sanim->delay;

    if (target)
    {
        sanim->playing = true;
    }
    else if (sanim->child)
    {
        if ((sanim->animationFlags & ITU_ANIM_MOVE) || (sanim->animationFlags & ITU_ANIM_EASE_IN) || (sanim->animationFlags & ITU_ANIM_EASE_OUT) || (sanim->animationFlags & ITU_ANIM_SCALE))
            memcpy(&sanim->child->rect, &sanim->orgRect, sizeof (ITURectangle));

        if (sanim->animationFlags & ITU_ANIM_COLOR)
        {
            memcpy(&sanim->child->color, &sanim->orgColor, sizeof (ITUColor));
            sanim->child->alpha = sanim->orgAlpha;
        }

        if (sanim->animationFlags & ITU_ANIM_ROTATE)
            sanim->child->angle = sanim->orgAngle;

        if (sanim->animationFlags & ITU_ANIM_TRANSFORM)
        {
            sanim->child->transformX = sanim->orgTransformX;
            sanim->child->transformY = sanim->orgTransformY;
        }
        ituSimpleAnimationOnStop(sanim);
        ituExecActions((ITUWidget*)sanim, sanim->actions, ITU_EVENT_STOPPED, 0);
    }
    sanim->widget.dirty = true;
}

void ituSimpleAnimationStop(ITUSimpleAnimation* sanim)
{
    assert(sanim);
    ITU_ASSERT_THREAD();

    if (!sanim->playing)
        return;

    sanim->playing         = false;
    sanim->widget.dirty    = true;
}

void ituSimpleAnimationGoto(ITUSimpleAnimation* sanim, int frame)
{
    ITUWidget* target = NULL;
    ITU_ASSERT_THREAD();

    if (frame < 0 || frame > sanim->totalframe)
        return;

    target = (ITUWidget*) itcTreeGetChildAt(sanim, 1);
    if (target)
    {
        if (sanim->animationFlags & ITU_ANIM_MOVE)
        {
            sanim->child->rect.x = sanim->orgRect.x + (sanim->keyRect.x - sanim->orgRect.x) * frame / sanim->totalframe;
            sanim->child->rect.y = sanim->orgRect.y + (sanim->keyRect.y - sanim->orgRect.y) * frame / sanim->totalframe;
        }
		else if (sanim->animationFlags & ITU_ANIM_EASE_IN)
		{
			float step = (float)frame / sanim->totalframe;
			step = step * step * step;
            sanim->child->rect.x = sanim->orgRect.x + (int)((sanim->keyRect.x - sanim->orgRect.x) * step);
            sanim->child->rect.y = sanim->orgRect.y + (int)((sanim->keyRect.y - sanim->orgRect.y) * step);
		}
		else if (sanim->animationFlags & ITU_ANIM_EASE_OUT)
		{
			float step = (float)frame / sanim->totalframe;
			step = step - 1;
			step = step * step * step + 1;
            sanim->child->rect.x = sanim->orgRect.x + (int)((sanim->keyRect.x - sanim->orgRect.x) * step);
            sanim->child->rect.y = sanim->orgRect.y + (int)((sanim->keyRect.y - sanim->orgRect.y) * step);
		}

        if (sanim->animationFlags & ITU_ANIM_SCALE)
        {
			if (!(sanim->animationFlags & ITU_ANIM_MOVE) && !(sanim->animationFlags & ITU_ANIM_EASE_IN) && !(sanim->animationFlags & ITU_ANIM_EASE_OUT) && (sanim->animationFlags & ITU_ANIM_SCALE_CENTER))
            {
                sanim->child->rect.x = sanim->orgRect.x - (sanim->keyRect.width - sanim->orgRect.width) / 2 * frame / sanim->totalframe;
                sanim->child->rect.y = sanim->orgRect.y - (sanim->keyRect.height - sanim->orgRect.height) / 2 * frame / sanim->totalframe;
                sanim->child->rect.width = sanim->orgRect.width + (sanim->keyRect.width - sanim->orgRect.width) * frame / sanim->totalframe / 2;
                sanim->child->rect.height = sanim->orgRect.height + (sanim->keyRect.height - sanim->orgRect.height) * frame / sanim->totalframe / 2;
            }
			else if (sanim->animationFlags & ITU_ANIM_EASE_IN)
			{
				float step = (float)frame / sanim->totalframe;
				step = step * step * step;
                sanim->child->rect.width = sanim->orgRect.width + (int)((sanim->keyRect.width - sanim->orgRect.width) * step);
                sanim->child->rect.height = sanim->orgRect.height + (int)((sanim->keyRect.height - sanim->orgRect.height) * step);
			}
			else if (sanim->animationFlags & ITU_ANIM_EASE_OUT)
			{
				float step = (float)frame / sanim->totalframe;
				step = step - 1;
				step = step * step * step + 1;
                sanim->child->rect.width = sanim->orgRect.width + (int)((sanim->keyRect.width - sanim->orgRect.width) * step);
                sanim->child->rect.height = sanim->orgRect.height + (int)((sanim->keyRect.height - sanim->orgRect.height) * step);
			}
            else
            {
                sanim->child->rect.width = sanim->orgRect.width + (sanim->keyRect.width - sanim->orgRect.width) * frame / sanim->totalframe;
                sanim->child->rect.height = sanim->orgRect.height + (sanim->keyRect.height - sanim->orgRect.height) * frame / sanim->totalframe;
            }
            //printf("n=%s c=%d/%d t=%d/%d k=%d/%d f=%d/%d\n", target->name, sanim->child->rect.width, sanim->child->rect.height, target->rect.width, target->rect.height, sanim->keyRect.width, sanim->keyRect.height, frame, sanim->totalframe);
        }
        if (sanim->animationFlags & ITU_ANIM_COLOR)
        {
            sanim->child->color.alpha = sanim->orgColor.alpha + (sanim->keyColor.alpha - sanim->orgColor.alpha) * frame / sanim->totalframe;
            sanim->child->color.red = sanim->orgColor.red + (sanim->keyColor.red - sanim->orgColor.red) * frame / sanim->totalframe;
            sanim->child->color.green = sanim->orgColor.green + (sanim->keyColor.green - sanim->orgColor.green) * frame / sanim->totalframe;
            sanim->child->color.blue = sanim->orgColor.blue + (sanim->keyColor.blue - sanim->orgColor.blue) * frame / sanim->totalframe;
            sanim->child->alpha = sanim->orgAlpha + (sanim->keyAlpha - sanim->orgAlpha) * frame / sanim->totalframe;
        }
        if (sanim->animationFlags & ITU_ANIM_ROTATE)
        {
            sanim->child->angle = sanim->orgAngle + (sanim->keyAngle - sanim->orgAngle) * frame / sanim->totalframe;
        }
        if (sanim->animationFlags & ITU_ANIM_TRANSFORM)
        {
            sanim->child->transformX = sanim->orgTransformX + (sanim->keyTransformX - sanim->orgTransformX) * frame / sanim->totalframe;
            sanim->child->transformY = sanim->orgTransformY + (sanim->keyTransformY - sanim->orgTransformY) * frame / sanim->totalframe;
        }
        sanim->frame = frame;
        sanim->widget.dirty = true;
    }
}

void ituSimpleAnimationReset(ITUSimpleAnimation* sanim)
{
    ITU_ASSERT_THREAD();

    ituSimpleAnimationStop(sanim);
    sanim->frame = 0;
    if (sanim->child)
    {
        if ((sanim->animationFlags & ITU_ANIM_MOVE) || (sanim->animationFlags & ITU_ANIM_EASE_IN) || (sanim->animationFlags & ITU_ANIM_EASE_OUT) || (sanim->animationFlags & ITU_ANIM_SCALE))
            memcpy(&sanim->child->rect, &sanim->orgRect, sizeof (ITURectangle));

        if (sanim->animationFlags & ITU_ANIM_COLOR)
        {
            memcpy(&sanim->child->color, &sanim->orgColor, sizeof (ITUColor));
            sanim->child->alpha = sanim->orgAlpha;
        }

        if (sanim->animationFlags & ITU_ANIM_ROTATE)
            sanim->child->angle = sanim->orgAngle;

        if (sanim->animationFlags & ITU_ANIM_TRANSFORM)
        {
            sanim->child->transformX = sanim->orgTransformX;
            sanim->child->transformY = sanim->orgTransformY;
        }
        sanim->child = NULL;
    }
	sanim->animationFlags &= ~ITU_ANIM_REVERSE;
    ituWidgetUpdate(sanim, ITU_EVENT_LAYOUT, 0, 0, 0);
}

void ituSimpleAnimationReversePlay(ITUSimpleAnimation* sanim)
{
    ITUWidget* target;
    assert(sanim);
    ITU_ASSERT_THREAD();

    if (sanim->frame == 0)
        return;

    target = (ITUWidget*) itcTreeGetChildAt(sanim, 1);

    sanim->delayCount = sanim->delay;
    sanim->animationFlags |= ITU_ANIM_REVERSE;
    sanim->frame = 0;

    if (target)
    {
        sanim->playing = true;
    }
    else if (sanim->child)
    {
        if ((sanim->animationFlags & ITU_ANIM_MOVE) || (sanim->animationFlags & ITU_ANIM_EASE_IN) || (sanim->animationFlags & ITU_ANIM_EASE_OUT) || (sanim->animationFlags & ITU_ANIM_SCALE))
            memcpy(&sanim->child->rect, &sanim->orgRect, sizeof (ITURectangle));

        if (sanim->animationFlags & ITU_ANIM_COLOR)
        {
            memcpy(&sanim->child->color, &sanim->orgColor, sizeof (ITUColor));
            sanim->child->alpha = sanim->orgAlpha;
        }

        if (sanim->animationFlags & ITU_ANIM_ROTATE)
            sanim->child->angle = sanim->orgAngle;

        if (sanim->animationFlags & ITU_ANIM_TRANSFORM)
        {
            sanim->child->transformX = sanim->orgTransformX;
            sanim->child->transformY = sanim->orgTransformY;
        }
        ituSimpleAnimationOnStop(sanim);
        ituExecActions((ITUWidget*)sanim, sanim->actions, ITU_EVENT_STOPPED, 0);
    }
    sanim->widget.dirty = true;
}
