#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

#define ITUCIRCLEPROGRESSBAR_MAX_ITEM  15

static char cpbNameID[ITUCIRCLEPROGRESSBAR_MAX_ITEM][50] = { { NULL } };
static int cpbRotatedIdx[ITUCIRCLEPROGRESSBAR_MAX_ITEM] = { -1 };
static int cpbRotatedCT[ITUCIRCLEPROGRESSBAR_MAX_ITEM] = { 0 };
static ITUSurface* cpbStepSurf[ITUCIRCLEPROGRESSBAR_MAX_ITEM] = { NULL };

static const char circleProgressBarName[] = "ITUCircleProgressBar";

int ituCircleProgressBarGetNameID(ITUWidget* widget)
{
	int i = 0;

	if (!widget)
		return -1;

	for (i = 0; i < ITUCIRCLEPROGRESSBAR_MAX_ITEM; i++)
	{
		if (strlen(cpbNameID[i]) == 0)
		{
			ITUColor color = { 0, 0, 0, 0 };
			strcpy(cpbNameID[i], widget->name);
			cpbRotatedIdx[i] = -1;
			if (cpbStepSurf[i])
			{
				ituSurfaceRelease(cpbStepSurf[i]);
				cpbStepSurf[i] = NULL;
			}

			cpbStepSurf[i] = ituCreateSurface(widget->rect.width, widget->rect.height, 0, ITU_ARGB8888, NULL, 0);
			ituColorFill(cpbStepSurf[i], 0, 0, widget->rect.width, widget->rect.height, &color);
			return i;
		}
		else
		{
			if (strcmp(cpbNameID[i], widget->name) == 0)
				return i;
		}
	}

	return -1;
}

void ituCircleProgressBarStepSurfClear(ITUWidget* widget)
{
	if (widget)
	{
		int id = ituCircleProgressBarGetNameID(widget);
		if (id >= 0)
		{
			ituSurfaceRelease(cpbStepSurf[id]);
			cpbStepSurf[id] = NULL;
		}
	}
}

static void ituCircleProgressBarExit(ITUWidget* widget)
{
    ITUCircleProgressBar* bar = (ITUCircleProgressBar*) widget;
    assert(widget);
	ITU_ASSERT_THREAD();

    if (bar->cacheSurf)
    {
        ituDestroySurface(bar->cacheSurf);
        bar->cacheSurf = NULL;
		ituCircleProgressBarStepSurfClear(widget);
    }
}

bool ituCircleProgressBarUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUCircleProgressBar* bar = (ITUCircleProgressBar*) widget;
    assert(bar);

    if (ev == ITU_EVENT_LAYOUT)
    {
        if (!bar->progressIcon && (bar->progressName[0] != '\0'))
        {
            bar->progressIcon = (ITUIcon*) ituSceneFindWidget(ituScene, bar->progressName);
        }

        if (!bar->percentText && (bar->percentName[0] != '\0'))
        {
            bar->percentText = (ITUText*) ituSceneFindWidget(ituScene, bar->percentName);
        }

        if (bar->percentText)
        {
			char buf[5] = { 0 };

            sprintf(buf, "%i", bar->value);
            ituTextSetString(bar->percentText, buf);

            result = widget->dirty = true;
        }

        if (!bar->cacheSurf && bar->cacheCount > 0 && bar->progressIcon && bar->progressIcon->surf && bar->value > 0 && (bar->endAngle != bar->startAngle))
        {
            ITURectangle* rect = &widget->rect;

            if (bar->cacheCount <= bar->maxValue)
            {
				bar->cacheSurf = ituCreateSurface(widget->rect.width, widget->rect.height, 0, ITU_ARGB8888, NULL, 0);
                if (bar->cacheSurf)
                {
                    int i = 0;
                    ITUColor color = { 0, 0, 0, 0 };
					ituColorFill(bar->cacheSurf, 0, 0, widget->rect.width, widget->rect.height, &color);

                    for (i = 0; i < bar->cacheCount; ++i)
                    {
                        float angle = 0.0;

						if (bar->endAngle > bar->startAngle)
							angle = bar->startAngle + (bar->endAngle - bar->startAngle) * i / (float)bar->maxValue;
						else if (bar->endAngle < bar->startAngle)
							angle = bar->startAngle - (bar->startAngle - bar->endAngle) * i / (float)bar->maxValue;

#ifndef CFG_WIN32_SIMULATOR
						ituRotate(bar->cacheSurf, widget->rect.width / 2, (widget->rect.height / 2) - bar->progressIcon->surf->height, bar->progressIcon->surf, 0, bar->progressIcon->surf->height, angle, 1.0f, 1.0f);
#else
						ituRotate(bar->cacheSurf, widget->rect.width / 2, widget->rect.height / 2, bar->progressIcon->surf, 0, bar->progressIcon->surf->height, angle, 1.0f, 1.0f);
#endif
                    }
                }
            }
        }
    }
    result |= ituIconUpdate(widget, ev, arg1, arg2, arg3);
    return widget->visible ? result : false;
}

void ituCircleProgressBarDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    ITUCircleProgressBar* bar = (ITUCircleProgressBar*) widget;
	ITURectangle prevClip = { 0, 0, 0, 0 };
    ITURectangle* rect = &widget->rect;
    int destx = 0, desty = 0;
    uint8_t desta = 0;
    uint8_t progressIconVisible = 0;
	int fix_line_tick = 0;
	int initC = (bar->cacheCount > 0) ? (bar->value / bar->cacheCount) : (-1);
	int currentCPBID = ituCircleProgressBarGetNameID(widget);
	int cacheRotateTick = 0;

    if (bar->progressIcon)
    {
        progressIconVisible = bar->progressIcon->widget.visible;
        bar->progressIcon->widget.visible = false;
    }
    ituBackgroundDraw(widget, dest, x, y, alpha);

    if (bar->progressIcon)
    {
        bar->progressIcon->widget.visible = progressIconVisible;
    }

    ituWidgetSetClipping(widget, dest, x, y, &prevClip);

    destx = rect->x + x;
    desty = rect->y + y;
    desta = alpha * widget->alpha / 255;

    if (desta > 0)
    {
        if (bar->progressIcon && bar->progressIcon->surf && bar->value > 0 && (bar->endAngle != bar->startAngle))
        {
            float angle = 0.0;
            int i = 0;
			int cacheT = 0;

            if (bar->cacheSurf && bar->value >= bar->cacheCount)
            {
				if (initC == cpbRotatedIdx[currentCPBID])
					initC = -1;
				else
					cpbRotatedIdx[currentCPBID] = initC;

				if (initC > 0)
				{
					ITUColor color = { 0, 0, 0, 0 };
					ituColorFill(cpbStepSurf[currentCPBID], 0, 0, cpbStepSurf[currentCPBID]->width, cpbStepSurf[currentCPBID]->height, &color);
					//ituBitBlt(cpbStepSurf[currentCPBID], 0, 0, rect->width, rect->height, bar->bg.icon.surf, 0, 0);
					//ituStretchBlt(cpbStepSurf[currentCPBID], 0, 0, rect->width, rect->height, bar->bg.icon.surf, 0, 0, bar->bg.icon.surf->width, bar->bg.icon.surf->height);
					//ituAlphaBlend(cpbStepSurf[currentCPBID], 0, 0, cpbStepSurf[currentCPBID]->width, cpbStepSurf[currentCPBID]->height, bar->bg.icon.surf, 0, 0, desta);

					//ituBitBlt(cpbStepSurf[currentCPBID], 0, 0, rect->width, rect->height, dest, destx, desty);
					printf("[CPB][cpbStepSurf_empty][%d]\n", initC);
				}

				cacheRotateTick = 0;
				for (i = initC; i > 0; i--)
                {
					if (bar->endAngle > bar->startAngle)
					{
						angle = (bar->endAngle - bar->startAngle) * (i - 1) * (bar->cacheCount) / (float)bar->maxValue;
						//printf("angle 1:%f \n", angle);
					}
					else if (bar->endAngle < bar->startAngle)
					{
						angle = -(bar->startAngle - bar->endAngle) * (i - 1) * (bar->cacheCount) / (float)bar->maxValue;
						//printf("angle 2 (%.3f) :(%d %d) %d %d\n", angle, bar->startAngle, bar->endAngle, i, bar->maxValue);
					}

					cacheT += bar->cacheCount;

					if (i > 1)
						fix_line_tick += 1;

                    if (desta == 255)
                    {
#ifndef CFG_WIN32_SIMULATOR
						//ituRotate(dest, destx + bar->progressIcon->widget.rect.x - (bar->cacheSurf->width / 2), desty + bar->progressIcon->widget.rect.y + bar->progressIcon->surf->height - (bar->cacheSurf->height / 2), bar->cacheSurf,
						//	bar->cacheSurf->width / 2, bar->cacheSurf->height / 2, angle, 1.0f, 1.0f);

						//ituRotate(cpbStepSurf[currentCPBID], cpbStepSurf[currentCPBID]->width / 2, (cpbStepSurf[currentCPBID]->height / 2) - (bar->cacheSurf->height / 2),
						//	bar->cacheSurf, bar->cacheSurf->width / 2, bar->cacheSurf->height / 2, angle, 1.0f, 1.0f);
						ituRotate(cpbStepSurf[currentCPBID], 0, 0,
							bar->cacheSurf, bar->cacheSurf->width / 2, bar->cacheSurf->height / 2, angle, 1.0f, 1.0f);
#else
						ituRotate(cpbStepSurf[currentCPBID], cpbStepSurf[currentCPBID]->width / 2, cpbStepSurf[currentCPBID]->height / 2, bar->cacheSurf, bar->cacheSurf->width / 2, bar->cacheSurf->height / 2, angle, 1.0f, 1.0f);
#endif
                    }
                    else
                    {
#ifdef CFG_WIN32_SIMULATOR
						/*ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
						if (surf)
						{
							ituBitBlt(surf, 0, 0, rect->width, rect->height, dest, destx, desty);
							ituRotate(surf, bar->progressIcon->widget.rect.x, bar->progressIcon->widget.rect.y + bar->progressIcon->surf->height, bar->cacheSurf, bar->cacheSurf->width / 2, bar->cacheSurf->height / 2, angle, 1.0f, 1.0f);
							ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);
							ituDestroySurface(surf);
						}*/
						ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
						if (surf)
						{
							ituRotate(surf, surf->width / 2, surf->height / 2, bar->cacheSurf, bar->cacheSurf->width / 2, bar->cacheSurf->height / 2, angle, 1.0f, 1.0f);
							ituAlphaBlend(cpbStepSurf[currentCPBID], 0, 0, cpbStepSurf[currentCPBID]->width, cpbStepSurf[currentCPBID]->height, surf, 0, 0, desta);
							ituDestroySurface(surf);
						}
#else
						//ituTransform(
						//	dest, destx + bar->progressIcon->widget.rect.x - (bar->cacheSurf->width / 2), desty + bar->progressIcon->widget.rect.y + bar->progressIcon->surf->height - (bar->cacheSurf->height / 2), rect->width, rect->height,
						//	bar->cacheSurf, 0, 0, bar->cacheSurf->width, bar->cacheSurf->height,
						//	bar->cacheSurf->width / 2, bar->cacheSurf->height / 2,
						//	1.0f, 1.0f, angle, 0, true, true, desta);
						ituTransform(
							cpbStepSurf[currentCPBID], 0, 0, cpbStepSurf[currentCPBID]->width, cpbStepSurf[currentCPBID]->height,
							bar->cacheSurf, 0, 0, bar->cacheSurf->width, bar->cacheSurf->height,
							bar->cacheSurf->width / 2, bar->cacheSurf->height / 2,
							1.0f, 1.0f, angle, 0, true, true, desta);
#endif
                    }
					cacheRotateTick++;
					printf("[CPB][initC %d][cache rotate %d]\n", initC, cacheRotateTick);
                }

				//to fix edge line
				if (initC >= 0)
				{
					

					for (i = fix_line_tick; i > 0; i--)
					{
						if (bar->endAngle > bar->startAngle)
							angle = bar->startAngle + (bar->endAngle - bar->startAngle) * (bar->cacheCount + 0) * fix_line_tick / (float)bar->maxValue;
						else if (bar->endAngle < bar->startAngle)
							angle = bar->startAngle - (bar->startAngle - bar->endAngle) * (bar->cacheCount + 0) * fix_line_tick / (float)bar->maxValue;

						fix_line_tick--;

						if (desta == 255)
						{
#ifndef CFG_WIN32_SIMULATOR
							//ituRotate(dest, destx + bar->progressIcon->widget.rect.x, desty + bar->progressIcon->widget.rect.y, bar->progressIcon->surf, 0, bar->progressIcon->surf->height, angle, 1.0f, 1.0f);
							//ituRotate(cpbStepSurf[currentCPBID], cpbStepSurf[currentCPBID]->width / 2, 
							//	(cpbStepSurf[currentCPBID]->height / 2) - bar->progressIcon->surf->height, 
							//	bar->progressIcon->surf, 0, bar->progressIcon->surf->height, angle, 1.0f, 1.0f);
							ituRotate(cpbStepSurf[currentCPBID], cpbStepSurf[currentCPBID]->width / 2,
								(cpbStepSurf[currentCPBID]->height / 2) - bar->progressIcon->surf->height,
								bar->progressIcon->surf, 0, bar->progressIcon->surf->height, angle, 1.0f, 1.0f);

#else
							//ituRotate(dest, destx + bar->progressIcon->widget.rect.x, desty + bar->progressIcon->widget.rect.y + bar->progressIcon->surf->height, bar->progressIcon->surf, 0, bar->progressIcon->surf->height, angle, 1.0f, 1.0f);
							ituRotate(cpbStepSurf[currentCPBID], cpbStepSurf[currentCPBID]->width / 2, cpbStepSurf[currentCPBID]->height / 2, bar->progressIcon->surf, 0, bar->progressIcon->surf->height, angle, 1.0f, 1.0f);
#endif
						}
						else
						{
#ifdef CFG_WIN32_SIMULATOR
							ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
							if (surf)
							{
								//ituBitBlt(surf, 0, 0, rect->width, rect->height, dest, destx, desty);
								//ituRotate(surf, bar->progressIcon->widget.rect.x, bar->progressIcon->widget.rect.y + bar->progressIcon->surf->height, bar->progressIcon->surf, 0, bar->progressIcon->surf->height, angle, 1.0f, 1.0f);
								//ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);
								//ituDestroySurface(surf);

								ituRotate(surf, surf->width / 2, surf->height / 2 - bar->progressIcon->surf->height, bar->progressIcon->surf, 0, bar->progressIcon->surf->height, angle, 1.0f, 1.0f);
								ituAlphaBlend(cpbStepSurf[currentCPBID], 0, 0, cpbStepSurf[currentCPBID]->width, cpbStepSurf[currentCPBID]->height, surf, 0, 0, desta);
								ituDestroySurface(surf);
							}
#else
							//ituTransform(
							//	dest, destx + bar->progressIcon->widget.rect.x, desty + bar->progressIcon->widget.rect.y, rect->width, rect->height,
							//	bar->progressIcon->surf, 0, 0, bar->progressIcon->surf->width, bar->progressIcon->surf->height,
							//	0, bar->progressIcon->surf->height,
							//	1.0f, 1.0f, angle, 0, true, true, desta);
							ituTransform(
								cpbStepSurf[currentCPBID], 0, 0, cpbStepSurf[currentCPBID]->width, cpbStepSurf[currentCPBID]->height,
								bar->progressIcon->surf, 0, 0, bar->progressIcon->surf->width, bar->progressIcon->surf->height,
								0, bar->progressIcon->surf->height,
								1.0f, 1.0f, angle, 0, true, true, desta);
#endif
						}
					}




					
					//ituBitBlt(cpbStepSurf[currentCPBID], 0, 0, rect->width, rect->height, dest, destx, desty);

					/*for (i = fix_line_tick; i > 0; i--)
					{
						if (bar->endAngle > bar->startAngle)
							angle = bar->startAngle + (bar->endAngle - bar->startAngle) * (bar->cacheCount + 0) * fix_line_tick / (float)bar->maxValue;
						else if (bar->endAngle < bar->startAngle)
							angle = bar->startAngle - (bar->startAngle - bar->endAngle) * (bar->cacheCount + 0) * fix_line_tick / (float)bar->maxValue;

						fix_line_tick--;

						if (desta == 255)
						{
#ifndef CFG_WIN32_SIMULATOR
							ituRotate(cpbStepSurf[currentCPBID], cpbStepSurf[currentCPBID]->width / 2, cpbStepSurf[currentCPBID]->height / 2 - bar->progressIcon->surf->height, bar->progressIcon->surf, 0, bar->progressIcon->surf->height, angle, 1.0f, 1.0f);
#else
							ituRotate(cpbStepSurf[currentCPBID], cpbStepSurf[currentCPBID]->width / 2, cpbStepSurf[currentCPBID]->height / 2, bar->progressIcon->surf, 0, bar->progressIcon->surf->height, angle, 1.0f, 1.0f);
#endif
						}
						else
						{
#ifdef CFG_WIN32_SIMULATOR
							ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
							if (surf)
							{
								ituBitBlt(surf, 0, 0, rect->width, rect->height, dest, destx, desty);
								ituRotate(surf, bar->progressIcon->widget.rect.x, bar->progressIcon->widget.rect.y + bar->progressIcon->surf->height, bar->progressIcon->surf, 0, bar->progressIcon->surf->height, angle, 1.0f, 1.0f);
								ituAlphaBlend(cpbStepSurf[currentCPBID], 0, 0, rect->width, rect->height, surf, 0, 0, desta);
								ituDestroySurface(surf);
							}
#else
							ituTransform(
								cpbStepSurf[currentCPBID], cpbStepSurf[currentCPBID]->width / 2, cpbStepSurf[currentCPBID]->height / 2, rect->width, rect->height,
								bar->progressIcon->surf, 0, 0, bar->progressIcon->surf->width, bar->progressIcon->surf->height,
								0, bar->progressIcon->surf->height,
								1.0f, 1.0f, angle, 0, true, true, desta);
#endif
						}
					}*/
				}
				//else
				{
					ituBitBlt(dest, destx, desty, rect->width, rect->height, cpbStepSurf[currentCPBID], 0, 0);
					//ituAlphaBlend(dest, destx, desty, rect->width, rect->height, cpbStepSurf[currentCPBID], 0, 0, desta);

					if (initC > 0)
						printf("[CPB][draw cache to dest]\n");
				}

				if (cacheT)
				{
					i = cacheT;
					cpbRotatedCT[ituCircleProgressBarGetNameID(widget)] = i;
				}
				else
				{
					i = cpbRotatedCT[ituCircleProgressBarGetNameID(widget)];
				}
            }

			for (; i <= bar->value; ++i)
			{
				if (bar->endAngle > bar->startAngle)
					angle = bar->startAngle + (bar->endAngle - bar->startAngle) * i / (float)bar->maxValue;
				else if (bar->endAngle < bar->startAngle)
					angle = bar->startAngle - (bar->startAngle - bar->endAngle) * i / (float)bar->maxValue;

				if (desta == 255)
				{
#ifndef CFG_WIN32_SIMULATOR
					ituRotate(dest, destx + bar->progressIcon->widget.rect.x, desty + bar->progressIcon->widget.rect.y, bar->progressIcon->surf, 0, bar->progressIcon->surf->height, angle, 1.0f, 1.0f);
#else
					ituRotate(dest, destx + bar->progressIcon->widget.rect.x, desty + bar->progressIcon->widget.rect.y + bar->progressIcon->surf->height, bar->progressIcon->surf, 0, bar->progressIcon->surf->height, angle, 1.0f, 1.0f);
#endif
				}
				else
				{
#ifdef CFG_WIN32_SIMULATOR
					ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
					if (surf)
					{
						ituBitBlt(surf, 0, 0, rect->width, rect->height, dest, destx, desty);
						ituRotate(surf, bar->progressIcon->widget.rect.x, bar->progressIcon->widget.rect.y + bar->progressIcon->surf->height, bar->progressIcon->surf, 0, bar->progressIcon->surf->height, angle, 1.0f, 1.0f);
						ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);
						ituDestroySurface(surf);
					}
#else
					ituTransform(
						dest, destx + bar->progressIcon->widget.rect.x, desty + bar->progressIcon->widget.rect.y, rect->width, rect->height,
						bar->progressIcon->surf, 0, 0, bar->progressIcon->surf->width, bar->progressIcon->surf->height,
						0, bar->progressIcon->surf->height,
						1.0f, 1.0f, angle, 0, true, true, desta);
#endif
				}

				//printf("[CPB]draw value %d --> %d\n", bar->value, i);
			}
            ituWidgetSetDirty(bar->progressIcon, false);
        }
    }
    ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
    ituDirtyWidget(bar, false);
}

void ituCircleProgressBarOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    assert(widget);

    switch (action)
    {
    case ITU_ACTION_GOTO:
        ituCircleProgressBarSetValue((ITUCircleProgressBar*)widget, atoi(param));
        break;

    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

void ituCircleProgressBarInit(ITUCircleProgressBar* bar)
{
    assert(bar);
    ITU_ASSERT_THREAD();

    memset(bar, 0, sizeof (ITUCircleProgressBar));

    ituBackgroundInit(&bar->bg);

    ituWidgetSetType(bar, ITU_CIRCLEPROGRESSBAR);
    ituWidgetSetName(bar, circleProgressBarName);
    ituWidgetSetExit(bar, ituCircleProgressBarExit);
    ituWidgetSetUpdate(bar, ituCircleProgressBarUpdate);
    ituWidgetSetDraw(bar, ituCircleProgressBarDraw);
    ituWidgetSetOnAction(bar, ituCircleProgressBarOnAction);
}

void ituCircleProgressBarLoad(ITUCircleProgressBar* bar, uint32_t base)
{
    assert(bar);

    ituBackgroundLoad(&bar->bg, base);

    if (bar->progressIcon)
        bar->progressIcon = (ITUIcon*)((uint32_t)bar->progressIcon + base);

    if (bar->percentText)
        bar->percentText = (ITUText*)((uint32_t)bar->percentText + base);

    ituWidgetSetExit(bar, ituCircleProgressBarExit);
    ituWidgetSetUpdate(bar, ituCircleProgressBarUpdate);
    ituWidgetSetDraw(bar, ituCircleProgressBarDraw);
    ituWidgetSetOnAction(bar, ituCircleProgressBarOnAction);

	ituCircleProgressBarGetNameID(bar);
}

void ituCircleProgressBarSetValue(ITUCircleProgressBar* bar, int value)
{
    assert(bar);
    ITU_ASSERT_THREAD();

    if (value < 0 || value > bar->maxValue)
    {
        LOG_WARN "incorrect value: %d\n", value LOG_END
        return;
    }
    bar->value = value;

    ituWidgetUpdate(bar, ITU_EVENT_LAYOUT, 0, 0, 0);
    ituWidgetSetDirty(bar, true);
}
