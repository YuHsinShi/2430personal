#include <assert.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "ite/itp.h"


#define ITU_METER_BLUR_LEVEL_SLOW  0.02  // --> (1/50)
#define ITU_METER_BLUR_LEVEL_FAST  0.04  // --> (1/25)
//meter blur animation range as below
//if      (angle_shift_level >= ITU_METER_BLUR_LEVEL_FAST) --> big
//else if (angle_shift_level >= ITU_METER_BLUR_LEVEL_SLOW) --> medimum
//else --> small

#define ITU_METER_ARC_DIV  57.295779513082321  //(180.0f / M_PI)

static const char meterName[] = "ITUMeter";

static void ituMeterExit(ITUWidget* widget)
{
	ITUMeter* meter = (ITUMeter*)widget;
	assert(widget);
	ITU_ASSERT_THREAD();
}

bool ituMeterUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
	bool result = false;
	ITUMeter* meter = (ITUMeter*)widget;
	assert(meter);

	if (ev == ITU_EVENT_LOAD)
	{
		ituMeterLoadStaticData(meter);
		result = true;
	}
	else if ((ev == ITU_EVENT_MOUSEDOWN) || (ev == ITU_EVENT_MOUSEMOVE && meter->pressed))
	{
		if ((ev == ITU_EVENT_MOUSEDOWN) || (ev == ITU_EVENT_MOUSEMOVE && --meter->delayCount <= 0))
		{
			if (ituWidgetIsEnabled(widget))
			{
				int x = arg2 - widget->rect.x;
				int y = arg3 - widget->rect.y;

				if (ituWidgetIsInside(widget, x, y))
				{
					int orgX, orgY, vx1, vy1, vx2, vy2, dot, det, value;
					float angle = 0.0;

					orgX = widget->rect.width / 2;
					orgY = widget->rect.height / 2;

					//if (x >= orgX)
						vx2 = x - orgX;
					//else
					//	vx2 = orgX - x;

					vy2 = y - orgY;

					if (vx2 * vx2 + vy2 * vy2 >= meter->minRadius * meter->minRadius)
					{
						vx1 = 0;
						vy1 = -orgY;

						dot = vx1 * vx2 + vy1 *vy2;
						det = vx1 * vy2 - vy1 * vx2;
						angle = atan2f(det, dot) * ITU_METER_ARC_DIV; //(float)(180.0f / M_PI);

						//printf("(%d, %d) (%d, %d) angle=%f\n", vx1, vy1, vx2, vy2, angle);

						if ((angle < meter->startAngle && meter->startAngle < meter->endAngle) || angle < 0)
							angle += 360.0f;

						//fix for outside angle but maybe inside start-end range.
						if ((angle < meter->startAngle) && ((angle + 360.0f) >= meter->startAngle) && ((angle + 360.0f) <= meter->endAngle))
							angle += 360.0f;
						else if ((angle > meter->endAngle) && ((angle - 360.0f) <= meter->endAngle) && ((angle - 360.0f) >= meter->startAngle))
							angle -= 360.0f;

						if (meter->startAngle < meter->endAngle)
						{
							if (meter->startAngle <= angle && angle <= meter->endAngle)
							{
								int range = meter->endAngle - meter->startAngle;
								value = (int)roundf((angle - meter->startAngle) * meter->maxValue / range);

								ituMeterSetValue(meter, value);

								ituExecActions((ITUWidget*)meter, meter->actions, ITU_EVENT_CHANGED, value);
								result = widget->dirty = true;
							}
						}
						else if (meter->startAngle > meter->endAngle)
						{
							if (meter->startAngle >= 360 && angle < meter->endAngle)
								angle += 360;

							if (meter->startAngle >= angle && angle >= meter->endAngle)
							{
								int range = meter->startAngle - meter->endAngle;
								value = (int)roundf((meter->startAngle - angle) * meter->maxValue / range);

								ituMeterSetValue(meter, value);

								ituExecActions((ITUWidget*)meter, meter->actions, ITU_EVENT_CHANGED, value);
								result = widget->dirty = true;
							}
						}
						meter->pressed = true;
						meter->delayCount = meter->delay;
					}
				}
			}

		}
	}
	else if (ev == ITU_EVENT_MOUSEUP)
	{
		if (ituWidgetIsEnabled(widget))
		{
			if (meter->pressed)
			{
				meter->pressed = false;
				widget->dirty = true;

				result |= widget->dirty;
				return result;
			}
		}
	}
	else if (ev == ITU_EVENT_LAYOUT)
	{
		if (!meter->pointerIcon && (meter->pointerName[0] != '\0'))
		{
			meter->pointerIcon = (ITUIcon*)ituSceneFindWidget(ituScene, meter->pointerName);
		}
	}
	result |= ituIconUpdate(widget, ev, arg1, arg2, arg3);

	return widget->visible ? result : false;
}

void ituMeterDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
	ITUMeter* meter = (ITUMeter*)widget;
	ITURectangle prevClip = { 0, 0, 0, 0 };

	ITCTree* child = widget->tree.child;
	widget->tree.child = NULL;
	ituBackgroundDraw(widget, dest, x, y, alpha);
	widget->tree.child = child;

	ituWidgetSetClipping(widget, dest, x, y, &prevClip);

	x += widget->rect.x;
	y += widget->rect.y;
	alpha = alpha * widget->alpha / 255;

	if (meter->pointerIcon && meter->pointerIcon->surf)
	{
		bool first_load_draw = false;
		float local_clock_wise = 0.0;
		float diff = 0.0;
		float angle = 0.0;
		float last_angle = 0.0;

		if (meter->endAngle > meter->startAngle)
		{
			angle = meter->startAngle + (meter->endAngle - meter->startAngle) * meter->value / (float)meter->maxValue;
			last_angle = meter->startAngle + (meter->endAngle - meter->startAngle) * meter->lastvalue / (float)meter->maxValue;
		}
		else if (meter->endAngle < meter->startAngle)
		{
			angle = meter->startAngle - (meter->startAngle - meter->endAngle) * meter->value / (float)meter->maxValue;
			last_angle = meter->startAngle - (meter->startAngle - meter->endAngle) * meter->lastvalue / (float)meter->maxValue;
		}

		diff = (angle - last_angle);

		if (angle != last_angle)
		{
			if (meter->endAngle > meter->startAngle)
				meter->diffvalue = meter->value - meter->lastvalue;
			else if (meter->endAngle < meter->startAngle)
				meter->diffvalue = meter->lastvalue - meter->value;
		}

		//do animation fit when shift value small
		if (meter->shadowmode)
		{
			bool print_debug = false;
			float step = diff;
			float target = last_angle;

			if (meter->diffvalue != 0)
			{
				if (meter->endAngle > meter->startAngle)
					local_clock_wise = (meter->endAngle - meter->startAngle) * meter->diffvalue / (float)meter->maxValue;
				else if (meter->endAngle < meter->startAngle)
					local_clock_wise = (meter->startAngle - meter->endAngle) * meter->diffvalue / (float)meter->maxValue;
			}

			if (meter->lastvalue != meter->value) //do not fit when finger moving
			{
				int i = 0;
				ITUSurface* surfLocal = NULL;
				float step_range = fabs(local_clock_wise);
				float angle_shift_level = fabs((float)meter->diffvalue / meter->maxValue);

				if (angle_shift_level >= ITU_METER_BLUR_LEVEL_FAST) //big
				{
					for (i = 2; i >= 0; i--) //check valid surf [2]->[1]->[0]
					{
						if (local_clock_wise > 0.0)
							surfLocal = meter->cw_surf[i];
						else
							surfLocal = meter->acw_surf[i];

						if (surfLocal)
							break;
					}
				}
				else if (angle_shift_level >= ITU_METER_BLUR_LEVEL_SLOW) //medimum
				{
					for (i = 1; i >= 0; i--) //check valid surf [1]->[0]
					{
						if (local_clock_wise > 0.0)
							surfLocal = meter->cw_surf[i];
						else
							surfLocal = meter->acw_surf[i];

						if (surfLocal)
							break;
					}
				}
				else //small
				{
					if (local_clock_wise > 0.0)
						surfLocal = meter->cw_surf[0];
					else
						surfLocal = meter->acw_surf[0];
				}

				if (surfLocal == NULL)
					surfLocal = meter->pointerIcon->surf;

				//target pointer
				target = last_angle + (step / meter->shadow_loop_max) * (meter->shadow_loop_count + 1);

				if (surfLocal)
				{
#ifdef CFG_WIN32_SIMULATOR
					ituRotate(dest, x + meter->pointerIcon->widget.rect.x + meter->pointerX, y + meter->pointerIcon->widget.rect.y + meter->pointerY, surfLocal, meter->pointerX, meter->pointerY, target, 1.0f, 1.0f);
#else
					ituRotate(dest, x + meter->pointerIcon->widget.rect.x, y + meter->pointerIcon->widget.rect.y, surfLocal, meter->pointerX, meter->pointerY, target, 1.0f, 1.0f);
#endif
				}

			}
			else
			{
				if (meter->shadow_loop_count == 0)
					first_load_draw = true;
			}


			if (!first_load_draw)
			{
				meter->shadow_loop_count++;

				if ((meter->shadow_loop_count < meter->shadow_loop_max))
				{
					ituWidgetSetDirty(meter->pointerIcon, true);
					ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
					return;
				}
				else
				{
					meter->shadow_loop_count = 0;
					//ituWidgetSetDirty(meter, false);
					//ituWidgetSetDirty(meter->pointerIcon, false);
					ituWidgetSetDirty(meter->pointerIcon, true);
					ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
					meter->lastvalue = meter->value;
					return;
				}
			}
		}
		else //reset last value
		{
			//meter->lastvalue = meter->value;
		}

		// default drawing without any shadow fit
		if ((meter->shadowmode == 0) || first_load_draw)
		{
			if (0) //for debug usage
			{
				//ITUSurface* surf = meter->cw_surf[0];
				//ituBitBlt(dest, 0, 0, surf->width, surf->height, surf, 0, 0);
			}
			else if (meter->pointerIcon->widget.alpha == 255)
			{
#ifndef CFG_WIN32_SIMULATOR
				ituRotate(dest, x + meter->pointerIcon->widget.rect.x, y + meter->pointerIcon->widget.rect.y, meter->pointerIcon->surf, meter->pointerX, meter->pointerY, angle, 1.0f, 1.0f);
#else
				ituRotate(dest, x + meter->pointerIcon->widget.rect.x + meter->pointerX, y + meter->pointerIcon->widget.rect.y + meter->pointerY, meter->pointerIcon->surf, meter->pointerX, meter->pointerY, angle, 1.0f, 1.0f);
#endif
			}
			else
			{
				ITUSurface* surf = ituCreateSurface(widget->rect.width, widget->rect.height, 0, dest->format, NULL, 0);
				ituBitBlt(surf, 0, 0, widget->rect.width, widget->rect.height, dest, x, y);
#ifdef CFG_WIN32_SIMULATOR
				ituRotate(surf, meter->pointerIcon->widget.rect.x + meter->pointerX, meter->pointerIcon->widget.rect.y + meter->pointerY, meter->pointerIcon->surf, meter->pointerX, meter->pointerY, angle, 1.0f, 1.0f);
#else
				ituRotate(surf, meter->pointerIcon->widget.rect.x, meter->pointerIcon->widget.rect.y, meter->pointerIcon->surf, meter->pointerX, meter->pointerY, angle, 1.0f, 1.0f);
#endif
				ituAlphaBlend(dest, x, y, widget->rect.width, widget->rect.height, surf, 0, 0, meter->pointerIcon->widget.alpha);
				ituDestroySurface(surf);
				surf = NULL;
			}
			meter->lastvalue = meter->value;
		}
		ituWidgetSetDirty(meter->pointerIcon, false);

	}
	ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
}

void ituMeterInit(ITUMeter* meter)
{
	assert(meter);
	ITU_ASSERT_THREAD();

	memset(meter, 0, sizeof (ITUMeter));

	ituBackgroundInit(&meter->bg);

	ituWidgetSetType(meter, ITU_METER);
	ituWidgetSetName(meter, meterName);
	ituWidgetSetExit(meter, ituMeterExit);
	ituWidgetSetUpdate(meter, ituMeterUpdate);
	ituWidgetSetDraw(meter, ituMeterDraw);
}

void ituMeterLoad(ITUMeter* meter, uint32_t base)
{
	ITUWidget* widget = (ITUWidget*)meter;
	assert(meter);

	ituBackgroundLoad(&meter->bg, base);

	if (meter->pointerIcon)
	{
		meter->pointerIcon = (ITUIcon*)((uint32_t)meter->pointerIcon + base);
	}

	meter->shadow_loop_count = 0;

	ituWidgetSetExit(meter, ituMeterExit);
	ituWidgetSetUpdate(meter, ituMeterUpdate);
	ituWidgetSetDraw(meter, ituMeterDraw);

	if (!(widget->flags & ITU_EXTERNAL))
	{
		int i = 0;
		for (i = 0; i < 3; i++)
		{
			if (meter->cw_shadow[i])
			{
				ITUSurface* surf = (ITUSurface*)(base + (uint32_t)meter->cw_shadow[i]);

				if (surf->flags & ITU_COMPRESSED)
					meter->cw_surf[i] = ituSurfaceDecompress(surf);
				else
					meter->cw_surf[i] = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);

				meter->cw_shadow[i] = surf;
			}

			if (meter->acw_shadow[i])
			{
				ITUSurface* surf = (ITUSurface*)(base + (uint32_t)meter->acw_shadow[i]);

				if (surf->flags & ITU_COMPRESSED)
					meter->acw_surf[i] = ituSurfaceDecompress(surf);
				else
					meter->acw_surf[i] = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);

				meter->acw_shadow[i] = surf;
			}
		}
	}

	if ((meter->cw_shadow[0] == NULL) || (meter->acw_shadow[0] == NULL))
		meter->shadowmode = 0;
}

void ituMeterSetValue(ITUMeter* meter, int value)
{
	assert(meter);
	ITU_ASSERT_THREAD();

	if (meter->value == value)
		return;

	if (value < 0 || value > meter->maxValue)
	{
		LOG_WARN "incorrect value: %d\n", value LOG_END
			return;
	}

	meter->value = value;

	//ituWidgetUpdate(meter, ITU_EVENT_LAYOUT, 0, 0, 0);
	ituExecActions((ITUWidget*)meter, meter->actions, ITU_EVENT_CHANGED, value);
	ituWidgetSetDirty(meter, true);
}

void ituMeterLoadStaticData(ITUMeter* meter)
{
	int i = 0;
	ITUWidget* widget = (ITUWidget*)meter;
	assert(meter);

	if (widget->flags & ITU_EXTERNAL)
		return;

	for (i = 0; i < 3; i++)
	{
		ITUSurface* surf = NULL;

		if (!meter->cw_shadow[i] || meter->cw_surf[i])
			break;

		surf = meter->cw_shadow[i];

		if (surf->flags & ITU_COMPRESSED)
			meter->cw_surf[i] = ituSurfaceDecompress(surf);
		else
			meter->cw_surf[i] = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
	}

	for (i = 0; i < 3; i++)
	{
		ITUSurface* surf = NULL;

		if (!meter->acw_shadow[i] || meter->acw_surf[i])
			break;

		surf = meter->acw_shadow[i];

		if (surf->flags & ITU_COMPRESSED)
			meter->acw_surf[i] = ituSurfaceDecompress(surf);
		else
			meter->acw_surf[i] = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
	}
}

int ituMeterSetShadowMode(ITUMeter* meter, int sh_mode)
{
	assert(meter);
	ITU_ASSERT_THREAD();

	if (sh_mode > 0)
	{
		int result = 0;

		if ((meter->cw_shadow[0] == NULL) || (meter->acw_shadow[0] == NULL))
			result = 0;
		else if ((meter->cw_shadow[1] == NULL) || (meter->acw_shadow[1] == NULL))
			result = 1;
		else if ((meter->cw_shadow[2] == NULL) || (meter->acw_shadow[2] == NULL))
			result = 2;
		else
			result = 3;

		if (result > 0)
			meter->shadowmode = sh_mode;

		return result;
	}
	else
	{
		meter->shadowmode = 0;
		return 0;
	}
}
