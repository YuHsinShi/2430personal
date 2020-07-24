#include <assert.h>
#include <malloc.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"
#include "ite/itp.h"

static const char iconName[] = "ITUIcon";

void ituIconExit(ITUWidget* widget)
{
    ITUIcon* icon = (ITUIcon*) widget;
    assert(widget);
    ITU_ASSERT_THREAD();

    if (icon->filePath)
    {
        free(icon->filePath);
        icon->filePath = NULL;
    }

    if (widget->flags & ITU_LOADED)
    {
        if (icon->loadedSurf)
        {
            ituSurfaceRelease(icon->loadedSurf);
            icon->loadedSurf = NULL;
        }
        widget->flags &= ~ITU_LOADED;
    }

    if (icon->surf)
    {
        ituSurfaceRelease(icon->surf);
        icon->surf = NULL;
    }
    ituWidgetExitImpl(widget);
}

bool ituIconClone(ITUWidget* widget, ITUWidget** cloned)
{
    ITUIcon* icon = (ITUIcon*)widget;
    assert(widget);
    assert(cloned);
    ITU_ASSERT_THREAD();

    if (*cloned == NULL)
    {
        ITUWidget* newWidget = malloc(sizeof(ITUIcon));
        if (newWidget == NULL)
            return false;

        memcpy(newWidget, widget, sizeof(ITUIcon));
        newWidget->tree.child = newWidget->tree.parent = newWidget->tree.sibling = NULL;
        *cloned = newWidget;
    }

    if (!(icon->widget.flags & ITU_EXTERNAL) && icon->staticSurf)
    {
        ITUIcon* newIcon = (ITUIcon*)*cloned;
        ITUSurface* surf = icon->staticSurf;
        if (surf->flags & ITU_COMPRESSED)
            newIcon->surf = NULL;
        else
            newIcon->surf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);

        ituWidgetUpdate(newIcon, ITU_EVENT_LOAD, 0, 0, 0);
    }
    return ituWidgetCloneImpl(widget, cloned);
}

static void IconLoadExternalData(ITUIcon* icon, ITULayer* layer)
{
    ITUWidget* widget = (ITUWidget*)icon;
    ITUSurface* surf;

    assert(widget);

    if (!icon->staticSurf || icon->surf || !(widget->flags & ITU_EXTERNAL))
        return;

    if (!layer)
        layer = ituGetLayer(widget);

    surf = ituLayerLoadExternalSurface(layer, (uint32_t)icon->staticSurf);
    if (surf)
    {
        if (surf->flags & ITU_COMPRESSED)
            icon->surf = ituSurfaceDecompress(surf);
        else
            icon->surf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }
}

static void IconLoadImage(ITUIcon* icon, char* path)
{
    ITUWidget* widget = (ITUWidget*)icon;
    ITURectangle* rect = &widget->rect;
    ITUSurface* surf = NULL;
    char filepath[PATH_MAX];
    char* ptr;

    assert(widget);

    if (!(widget->flags & ITU_EXTERNAL_IMAGE))
        return;

    strcpy(filepath, path);
    strcat(filepath, widget->name);

    ptr = strrchr(filepath, '.');
    if (ptr)
    {
    ptr++;
    if (stricmp(ptr, "jpg") == 0)
    {
        if (widget->flags & ITU_FIT_TO_RECT)
            surf = ituJpegLoadFile(rect->width, rect->height, filepath, ITU_FIT_TO_RECT);
        else if (widget->flags & ITU_CUT_BY_RECT)
            surf = ituJpegLoadFile(rect->width, rect->height, filepath, ITU_CUT_BY_RECT);
        else
            surf = ituJpegLoadFile(0, 0, filepath, 0);
    }
    else if (stricmp(ptr, "png") == 0)
    {
        if (widget->flags & ITU_FIT_TO_RECT || widget->flags & ITU_CUT_BY_RECT)
            surf = ituPngLoadFile(rect->width, rect->height, filepath);
        else
            surf = ituPngLoadFile(0, 0, filepath);
        }
    }

    if (surf)
    {
        ituIconReleaseSurface(icon);
        icon->surf = surf;
    }
}

bool ituIconUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUIcon* icon = (ITUIcon*) widget;
    assert(icon);

    if (ev == ITU_EVENT_LOAD)
    {
        if (widget->flags & ITU_EXTERNAL)
            IconLoadExternalData(icon, NULL);
        else
            ituIconLoadStaticData(icon);

        result = true;
    }
    else if (ev == ITU_EVENT_LOAD_EXTERNAL)
    {
        IconLoadExternalData(icon, (ITULayer*)arg1);
        result = true;
    }
    else if (ev == ITU_EVENT_LOAD_IMAGE)
    {
        IconLoadImage(icon, (char*)arg1);
        result = true;
    }
    else if (ev == ITU_EVENT_RELEASE)
    {
        ituIconReleaseSurface(icon);
        result = true;
    }
    else if (ev == ITU_EVENT_TIMER)
    {
        if (widget->flags & ITU_LOADED)
        {
            if (icon->filePath)
            {
                free(icon->filePath);
                icon->filePath = NULL;
            }

            if (icon->loadedSurf)
            {
                if (icon->surf)
                    ituSurfaceRelease(icon->surf);

                icon->surf = icon->loadedSurf;
                icon->loadedSurf = NULL;
            }
            widget->flags &= ~ITU_LOADED;
            result = true;
        }
    }
    result |= ituWidgetUpdateImpl(widget, ev, arg1, arg2, arg3);
    return result;
}

void ituIconDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    int destx, desty;
    uint8_t desta;
    ITURectangle prevClip;
    ITUIcon* icon = (ITUIcon*) widget;
    ITURectangle* rect = &widget->rect;
    assert(icon);
    assert(dest);

    if (!icon->surf)
    {
        ituWidgetSetDirty(widget, false);
        return;
    }

    destx = rect->x + x;
    desty = rect->y + y;
    desta = alpha * widget->color.alpha / 255;
    desta = desta * widget->alpha / 255;

    if (widget->angle % 90 == 0)
        ituWidgetSetClipping(widget, dest, x, y, &prevClip);

    if (desta > 0)
    {
		if ((icon->use_rotate_mask > 0) && (widget->flags & ITU_STRETCH) && (icon->use_rotate_mask > 0) && (widget->angle == 0) && (widget->transformType == ITU_TRANSFORM_NONE))
		{
			ITUSurface* SF = NULL;
			int ow = icon->surf->width;
			int oh = icon->surf->height;
			int w = ((ow % 2) ? (ow + 1) : (ow));
			int h = ((oh % 2) ? (oh + 1) : (oh));
			int w2 = w / 2;
			int h2 = h / 2;
			ITUSurface* S1 = ituCreateSurface(w2, h2, 0, icon->surf->format, NULL, 0);
			ITUSurface* S2 = ituCreateSurface(w2, h2, 0, icon->surf->format, NULL, 0);
			ITUSurface* S3 = ituCreateSurface(w2, h2, 0, icon->surf->format, NULL, 0);
			ITUSurface* S4 = ituCreateSurface(w2, h2, 0, icon->surf->format, NULL, 0);
			ITUSurface* Sr = ituCreateSurface(w2, h2, 0, icon->surf->format, NULL, 0);

			//for debug usage
			//if (strcmp(widget->name, "") == 0)
			//{
			//	icon->start_rm_angle = -30;
			//	icon->end_rm_angle = 290;
			//}
			int v_angle1 = icon->start_rm_angle;
			int v_angle2 = icon->end_rm_angle;
			int angle_loop_count = 0;
			bool over_angle1 = false;
			bool over_angle4 = false;
			bool impossible_angle = false;
			ITUColor emptycolor;

			SF = ituCreateSurface(w, h, 0, icon->surf->format, NULL, 0);
			emptycolor.alpha = 0;
			emptycolor.red = 0;
			emptycolor.green = 0;
			emptycolor.blue = 0;
			ituColorFill(SF, 0, 0, w, h, &emptycolor);
			ituColorFill(S1, 0, 0, w2, h2, &emptycolor);
			ituColorFill(S2, 0, 0, w2, h2, &emptycolor);
			ituColorFill(S3, 0, 0, w2, h2, &emptycolor);
			ituColorFill(S4, 0, 0, w2, h2, &emptycolor);

			if (v_angle2 <= v_angle1)
				impossible_angle = true;
			else if (v_angle1 < 0)
			{
				if ((360 - v_angle2) <= (-v_angle1))
					impossible_angle = true;
				else
					over_angle4 = true;
			}
			else if (v_angle2 > 360)
			{
				if ((v_angle2 - 360) >= v_angle1)
					impossible_angle = true;
				else
					over_angle1 = true;
			}

			// wrong angle judgement
			if (impossible_angle)
			{
				printf("wrong rotate mask angle setting for %s\n", widget->name);
			}
			else
			{
				int i = 0;
				int j = 0;
				double pi = 0.0174532925; //3.14159265 / 180.0;

				//temporary set v_angle1 to 0 to active quadrant1~4 when over_angle4
				if (over_angle4)
					v_angle1 = 0;

				angle_loop_count = 1;
				while (angle_loop_count)
				{
					if (over_angle1)
					{
						if (angle_loop_count <= 1)
						{
							v_angle1 = 0;
							v_angle2 = icon->end_rm_angle - 360;
						}
						else
						{
							v_angle1 = icon->start_rm_angle;
							v_angle2 = 360;
						}

						angle_loop_count++;
					}
					else
						angle_loop_count = 0;

					if (((v_angle1 >= 0) && (v_angle1 < 90)) && (v_angle2 > v_angle1))
					{
						double a1 = ((v_angle1 == 0) ? (0.0) : (tan(pi * v_angle1)));
						double a2 = ((v_angle2 >= 90) ? (0.0) : (tan(pi * v_angle2)));
						//reset Sr for quadrant 1
						if (angle_loop_count < 3)
						{
							ituColorFill(Sr, 0, 0, w2, h2, &emptycolor);
							//ituBitBlt(Sr, 0, 0, w2, h2, icon->surf, w2, 0);
							ituAlphaBlend(Sr, 0, 0, w2, h2, icon->surf, w2, 0, 255);
						}

						for (j = 0; j < h2; j++)
						{
							int dline = 0;
							int start_i = -1;
							for (i = 0; i < w2; i++)
							{
								double dy = h2 - j;
								double dx = i;
								double da = dx / dy;

								//if (((da <= a2) || (a2 == 0.0)) && (da >= a1))
								//	ituColorFill(Sr, i, j, 1, 1, &emptycolor);
								if (((da <= a2) || (a2 == 0.0)) && (da >= a1))
								{
									dline++;
									if (start_i < 0)
										start_i = i;
								}
							}
							if (dline)
								ituColorFill(Sr, start_i, j, dline, 1, &emptycolor);
						}
						if ((!over_angle1) || (angle_loop_count > 2))
						{
							ituAlphaBlend(S1, 0, 0, w2, h2, Sr, 0, 0, 255);
							angle_loop_count = 0;
						}
					}
					else if ((!over_angle1) || (angle_loop_count > 2))
						ituAlphaBlend(S1, 0, 0, w2, h2, icon->surf, w2, 0, 255);
				}

				if (((v_angle1 >= 0) && (v_angle1 < 180)) && (v_angle2 > v_angle1) && (v_angle2 > 90))
				{
					double a1 = ((v_angle1 <= 90) ? (0.0) : (tan(pi * (v_angle1 - 90))));
					double a2 = ((v_angle2 >= 180) ? (0.0) : (tan(pi * (v_angle2 - 90))));
					//reset Sr for quadrant 2
					ituColorFill(Sr, 0, 0, w2, h2, &emptycolor);
					//ituBitBlt(Sr, 0, 0, w2, h2, icon->surf, w2, h2);
					ituAlphaBlend(Sr, 0, 0, w2, h2, icon->surf, w2, h2, 255);

					for (j = 0; j < h2; j++)
					{
						int dline = 0;
						int start_i = -1;
						for (i = 0; i < w2; i++)
						{
							double dy = i;
							double dx = j;
							double da = dx / dy;

							//if (((da <= a2) || (a2 == 0.0)) && (da >= a1))
							//	ituColorFill(Sr, i, j, 1, 1, &emptycolor);
							if (((da <= a2) || (a2 == 0.0)) && (da >= a1))
							{
								dline++;
								if (start_i < 0)
									start_i = i;
							}
						}
						if (dline)
							ituColorFill(Sr, start_i, j, dline, 1, &emptycolor);
					}
					ituAlphaBlend(S2, 0, 0, w2, h2, Sr, 0, 0, 255);
				}
				else
					ituAlphaBlend(S2, 0, 0, w2, h2, icon->surf, w2, h2, 255);

				if (((v_angle1 >= 0) && (v_angle1 < 270)) && (v_angle2 > v_angle1) && (v_angle2 > 180))
				{
					double a1 = ((v_angle1 <= 180) ? (0.0) : (tan(pi * (v_angle1 - 180))));
					double a2 = ((v_angle2 >= 270) ? (0.0) : (tan(pi * (v_angle2 - 180))));
					//reset Sr for quadrant 3
					ituColorFill(Sr, 0, 0, w2, h2, &emptycolor);
					//ituBitBlt(Sr, 0, 0, w2, h2, icon->surf, 0, h2);
					ituAlphaBlend(Sr, 0, 0, w2, h2, icon->surf, 0, h2, 255);

					for (j = 0; j < h2; j++)
					{
						int dline = 0;
						int start_i = -1;
						for (i = 0; i < w2; i++)
						{
							double dy = j;
							double dx = w2 - i;
							double da = dx / dy;

							//if (((da <= a2) || (a2 == 0.0)) && (da >= a1))
							//	ituColorFill(Sr, i, j, 1, 1, &emptycolor);
							if (((da <= a2) || (a2 == 0.0)) && (da >= a1))
							{
								dline++;
								if (start_i < 0)
									start_i = i;
							}
						}
						if (dline)
							ituColorFill(Sr, start_i, j, dline, 1, &emptycolor);
					}
					ituAlphaBlend(S3, 0, 0, w2, h2, Sr, 0, 0, 255);
				}
				else
					ituAlphaBlend(S3, 0, 0, w2, h2, icon->surf, 0, h2, 255);

				//reset to original v_angle1 when over_angle4
				if (over_angle4)
					v_angle1 = icon->start_rm_angle;

				angle_loop_count = 1;
				while (angle_loop_count)
				{
					if (over_angle4)
					{
						if (angle_loop_count <= 1)
						{
							v_angle1 = 360 + icon->start_rm_angle;
							v_angle2 = 360;
						}
						else
						{
							v_angle1 = 0;
							v_angle2 = icon->end_rm_angle;
						}

						angle_loop_count++;
					}
					else
						angle_loop_count = 0;

					if (((v_angle1 >= 0) && (v_angle1 < 360)) && (v_angle2 > v_angle1) && (v_angle2 > 270))
					{
						double a1 = ((v_angle1 <= 270) ? (0.0) : (tan(pi * (v_angle1 - 270))));
						double a2 = ((v_angle2 >= 360) ? (0.0) : (tan(pi * (v_angle2 - 270))));
						//reset Sr for quadrant 4
						if (angle_loop_count < 3)
						{
							ituColorFill(Sr, 0, 0, w2, h2, &emptycolor);
							//ituBitBlt(Sr, 0, 0, w2, h2, icon->surf, 0, 0);
							ituAlphaBlend(Sr, 0, 0, w2, h2, icon->surf, 0, 0, 255);
						}

						for (j = 0; j < h2; j++)
						{
							int dline = 0;
							int start_i = -1;
							for (i = 0; i < w2; i++)
							{
								double dy = w2 - i;
								double dx = h2 - j;
								double da = dx / dy;

								//if (((da <= a2) || (a2 == 0.0)) && (da >= a1))
								//	ituColorFill(Sr, i, j, 1, 1, &emptycolor);
								if (((da <= a2) || (a2 == 0.0)) && (da >= a1))
								{
									dline++;
									if (start_i < 0)
										start_i = i;
								}
							}
							if (dline)
								ituColorFill(Sr, start_i, j, dline, 1, &emptycolor);
						}
						if ((!over_angle4) || (angle_loop_count > 2))
						{
							ituAlphaBlend(S4, 0, 0, w2, h2, Sr, 0, 0, 255);
							angle_loop_count = 0;
						}
					}
					else if ((!over_angle4) || (angle_loop_count > 2))
						ituAlphaBlend(S4, 0, 0, w2, h2, icon->surf, 0, 0, 255);
				}

				ituAlphaBlend(SF, w2, 0, w2, h2, S1, 0, 0, 255);
				ituAlphaBlend(SF, w2, h2, w2, h2, S2, 0, 0, 255);
				ituAlphaBlend(SF, 0, h2, w2, h2, S3, 0, 0, 255);
				ituAlphaBlend(SF, 0, 0, w2, h2, S4, 0, 0, 255);

				ituDestroySurface(S1);
				ituDestroySurface(S2);
				ituDestroySurface(S3);
				ituDestroySurface(S4);
				ituDestroySurface(Sr);
			}

			ituStretchBlt(dest, destx, desty, rect->width, rect->height, SF, 0, 0, SF->width, SF->height);
			ituDestroySurface(SF);

			//else // v_angle1 for clockwise, v_angle2 for counterclockwise
			//{
			//	if (0)//((v_angle1 < 90) || ((v_angle2 > v_angle1) && (v_angle2 < 90)))
			//	{
			//		ITUSurface* masksf = ituCreateSurface(w, h2, 0, icon->surf->format, NULL, 0);
			//		ITUSurface* masksf1 = ituCreateSurface(w, h2, 0, icon->surf->format, NULL, 0);
			//		ITUSurface* masksf2 = ituCreateSurface(w, h2, 0, icon->surf->format, NULL, 0);
			//		ITUSurface* masksf3 = ituCreateSurface(w2, h, 0, icon->surf->format, NULL, 0);
			//		ITUSurface* masksf4 = ituCreateSurface(w2, h, 0, icon->surf->format, NULL, 0);
			//		ituColorFill(Sr, 0, 0, w2, h2, &emptycolor);
			//		ituBitBlt(Sr, 0, 0, w2, h2, icon->surf, w2, 0);
			//		
			//		if (v_angle1 > 0)
			//		{
			//			ituColorFill(masksf1, 0, 0, w, h2, &emptycolor);
			//			ituColorFill(masksf2, 0, 0, w, h2, &emptycolor);
			//			ituColorFill(masksf3, 0, 0, w2, h, &emptycolor);
			//			ituColorFill(masksf4, 0, 0, w2, h, &emptycolor);

			//			ituBitBlt(masksf1, w2, 0, w2, h2, icon->surf, w2, 0);
			//			ituRotate(masksf2, w2, h2, masksf1, w2, h2 + 1, -v_angle1, 1.0, 1.0);
			//			ituColorFill(Sr, 0, 0, w2, h2, &emptycolor);
			//			ituBitBlt(Sr, 0, 0, w2, h2, masksf2, w2, 0);
			//			
			//			ituBitBlt(masksf3, 0, 0, w2, h2, Sr, 0, 0);
			//			ituColorFill(Sr, 0, 0, w2, h2, &emptycolor);
			//			ituRotate(Sr, 0, h2 + 2, masksf3, 0, h2, v_angle1, 1.0, 1.0);
			//		}
			//		if (0)//(v_angle2)
			//		{
			//			ituColorFill(masksf1, 0, 0, w, h2, &emptycolor);
			//			ituColorFill(masksf2, 0, 0, w, h2, &emptycolor);
			//			ituColorFill(masksf3, 0, 0, w2, h, &emptycolor);
			//			ituColorFill(masksf4, 0, 0, w2, h, &emptycolor);

			//			ituBitBlt(masksf3, 0, 0, w2, h2, Sr, 0, 0);
			//			ituRotate(masksf4, 0, h2, masksf3, 0, h2 , v_angle2, 1.0, 1.0);
			//			ituColorFill(Sr, 0, 0, w2, h2, &emptycolor);
			//			ituBitBlt(Sr, 0, 0, w2, h2, masksf4, 0, 0);

			//			ituBitBlt(masksf1, w2, 0, w2, h2, Sr, 0, 0);
			//			ituColorFill(Sr, 0, 0, w2, h2, &emptycolor);
			//			ituRotate(Sr, 0, h2, masksf1, w2, h2, -v_angle2 +10, 1.0, 1.0);
			//		}


			//		ituAlphaBlend(S1, 0, 0, w2, h2, Sr, 0, 0, 255);
			//		
			//		ituDestroySurface(masksf);
			//		
			//	}
			//	ituAlphaBlend(S2, 0, 0, w2, h2, icon->surf, w2, h2, 255);
			//	ituAlphaBlend(S3, 0, 0, w2, h2, icon->surf, 0, h2, 255);
			//	ituAlphaBlend(S4, 0, 0, w2, h2, icon->surf, 0, 0, 255);

			//	ituAlphaBlend(SF, w2, 0, w2, h2, S1, 0, 0, 255);
			//	ituAlphaBlend(SF, w2, h2, w2, h2, S2, 0, 0, 255);
			//	ituAlphaBlend(SF, 0, h2, w2, h2, S3, 0, 0, 255);
			//	ituAlphaBlend(SF, 0, 0, w2, h2, S4, 0, 0, 255);

			//	ituDestroySurface(S1);
			//	ituDestroySurface(S2);
			//	ituDestroySurface(S3);
			//	ituDestroySurface(S4);
			//}
		}
        else if (desta == 255)
        {
			if (widget->flags & ITU_STRETCH)
			{
#ifdef CFG_WIN32_SIMULATOR
				if (widget->angle == 0)
				{
					if (widget->transformType == ITU_TRANSFORM_NONE)
					{
						ituStretchBlt(dest, destx, desty, rect->width, rect->height, icon->surf, 0, 0, icon->surf->width, icon->surf->height);
					}
					else
					{
						ITUSurface* surf = ituCreateSurface(icon->surf->width, icon->surf->height, 0, dest->format, NULL, 0);
						if (surf)
						{
							int w = (icon->surf->width - icon->surf->width * widget->transformX / 100) / 2;
							int h = (icon->surf->height - icon->surf->height * widget->transformY / 100) / 2;

							ituStretchBlt(surf, 0, 0, icon->surf->width, icon->surf->height, dest, destx, desty, rect->width, rect->height);

							switch (widget->transformType)
							{
							case ITU_TRANSFORM_TURN_LEFT:
								ituTransformBlt(surf, 0, 0, icon->surf, 0, 0, icon->surf->width, icon->surf->height, w, h, icon->surf->width - w, 0, icon->surf->width - w, icon->surf->height, w, icon->surf->height - h, true, ITU_PAGEFLOW_FOLD2, widget->transformType);
								break;

							case ITU_TRANSFORM_TURN_TOP:
								ituTransformBlt(surf, 0, 0, icon->surf, 0, 0, icon->surf->width, icon->surf->height, w, h, icon->surf->width - w, h, icon->surf->width, icon->surf->height - h, 0, icon->surf->height - h, true, ITU_PAGEFLOW_FOLD2, widget->transformType);
								break;

							case ITU_TRANSFORM_TURN_RIGHT:
								ituTransformBlt(surf, 0, 0, icon->surf, 0, 0, icon->surf->width, icon->surf->height, w, 0, icon->surf->width - w, h, icon->surf->width - w, icon->surf->height - h, w, icon->surf->height, false, ITU_PAGEFLOW_FOLD2, widget->transformType);
								break;

							case ITU_TRANSFORM_TURN_BOTTOM:
								ituTransformBlt(surf, 0, 0, icon->surf, 0, 0, icon->surf->width, icon->surf->height, 0, h, icon->surf->width, h, icon->surf->width - w, icon->surf->height - h, w, icon->surf->height - h, false, ITU_PAGEFLOW_FOLD2, widget->transformType);
								break;
							}
							ituStretchBlt(dest, destx, desty, rect->width, rect->height, surf, 0, 0, surf->width, surf->height);
							ituDestroySurface(surf);
						}
					}
				}
				else
				{
                    float scaleX, scaleY;
                    if (widget->angle == 90 || widget->angle == 270)
                    {
                        scaleX = (float)rect->width / icon->surf->height;
                        scaleY = (float)rect->height / icon->surf->width;
                    }
                    else
                    {
                        scaleX = (float)rect->width / icon->surf->width;
                        scaleY = (float)rect->height / icon->surf->height;
                    }
					ituRotate(dest, destx + rect->width / 2, desty + rect->height / 2, icon->surf, icon->surf->width / 2, icon->surf->height / 2, (float)widget->angle, scaleX, scaleY);
				}
#else
				float scaleX = (float)rect->width / icon->surf->width;
				float scaleY = (float)rect->height / icon->surf->height;

				ituTransform(
					dest, destx, desty, rect->width, rect->height,
					icon->surf, 0, 0, icon->surf->width, icon->surf->height,
					icon->surf->width / 2, icon->surf->height / 2,
					scaleX,
					scaleY,
					(float)widget->angle,
					0,
					true,
					true,
					desta);
#endif
			}
            else
            {
                if (widget->angle == 0)
                {
                    if (widget->transformType == ITU_TRANSFORM_NONE)
                    {
                        ituBitBlt(dest, destx, desty, rect->width, rect->height, icon->surf, 0, 0);
                    }
                    else
                    {
                        int w = (rect->width - rect->width * widget->transformX / 100) / 2;
                        int h = (rect->height - rect->height * widget->transformY / 100) / 2;

                        switch (widget->transformType)
                        {
                        case ITU_TRANSFORM_TURN_LEFT:
                            ituTransformBlt(dest, destx, desty, icon->surf, 0, 0, rect->width, rect->height, w, h, rect->width - w, 0, rect->width - w, rect->height, w, rect->height - h, true, ITU_PAGEFLOW_FOLD2, widget->transformType);
                            break;

                        case ITU_TRANSFORM_TURN_TOP:
                            ituTransformBlt(dest, destx, desty, icon->surf, 0, 0, rect->width, rect->height, w, h, rect->width - w, h, rect->width, rect->height - h, 0, rect->height - h, true, ITU_PAGEFLOW_FOLD2, widget->transformType);
                            break;

                        case ITU_TRANSFORM_TURN_RIGHT:
                            ituTransformBlt(dest, destx, desty, icon->surf, 0, 0, rect->width, rect->height, w, 0, rect->width - w, h, rect->width - w, rect->height - h, w, rect->height, false, ITU_PAGEFLOW_FOLD2, widget->transformType);
                            break;

                        case ITU_TRANSFORM_TURN_BOTTOM:
                            ituTransformBlt(dest, destx, desty, icon->surf, 0, 0, rect->width, rect->height, 0, h, rect->width, h, rect->width - w, rect->height - h, w, rect->height - h, false, ITU_PAGEFLOW_FOLD2, widget->transformType);
                            break;
                        }
                    }
                }
                else
                {
#ifdef CFG_WIN32_SIMULATOR
                    ituRotate(dest, destx + rect->width / 2, desty + rect->height / 2, icon->surf, icon->surf->width / 2, icon->surf->height / 2, (float)widget->angle, 1.0f, 1.0f);
#else
                    ituRotate(dest, destx, desty, icon->surf, icon->surf->width / 2, icon->surf->height / 2, (float)widget->angle, 1.0f, 1.0f);
#endif
                }
            }
        }
        else
        {
            if (widget->flags & ITU_STRETCH)
            {
#ifdef CFG_WIN32_SIMULATOR
                ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
                if (surf)
                {
                    ituBitBlt(surf, 0, 0, rect->width, rect->height, dest, destx, desty);

                    if (widget->angle == 0)
                    {
                        ituStretchBlt(surf, 0, 0, rect->width, rect->height, icon->surf, 0, 0, icon->surf->width, icon->surf->height);
                    }
                    else
                    {
                        float scaleX = (float)rect->width / icon->surf->width;
                        float scaleY = (float)rect->height / icon->surf->height;

                        ituRotate(surf, rect->width / 2, rect->height / 2, icon->surf, icon->surf->width / 2, icon->surf->height / 2, (float)widget->angle, scaleX, scaleY);
                    }
                    ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);                
                    ituDestroySurface(surf);
                }
#else
                float scaleX = (float)rect->width / icon->surf->width;
                float scaleY = (float)rect->height / icon->surf->height;

                ituTransform(
                    dest, destx, desty, rect->width, rect->height,
                    icon->surf, 0, 0, icon->surf->width, icon->surf->height,
                    icon->surf->width / 2, icon->surf->height / 2,
                    scaleX,
                    scaleY,
                    (float)widget->angle,
                    0,
                    true,
                    true,
                    desta);
#endif
            }
            else
            {
                ituAlphaBlend(dest, destx, desty, rect->width, rect->height, icon->surf, 0, 0, desta);                
            }
        }
    }

    if (widget->angle % 90 == 0)
        ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);

    ituWidgetDrawImpl(widget, dest, x, y, alpha);
}

void ituIconInit(ITUIcon* icon)
{
    assert(icon);
    ITU_ASSERT_THREAD();

    memset(icon, 0, sizeof (ITUIcon));

    ituWidgetInit(&icon->widget);

    ituWidgetSetType(icon, ITU_ICON);
    ituWidgetSetName(icon, iconName);
    ituWidgetSetExit(icon, ituIconExit);
    ituWidgetSetClone(icon, ituIconClone);
    ituWidgetSetUpdate(icon, ituIconUpdate);
    ituWidgetSetDraw(icon, ituIconDraw);
}

void ituIconLoad(ITUIcon* icon, uint32_t base)
{
    assert(icon);

    ituWidgetLoad((ITUWidget*)icon, base);
    ituWidgetSetExit(icon, ituIconExit);
    ituWidgetSetClone(icon, ituIconClone);
    ituWidgetSetUpdate(icon, ituIconUpdate);
    ituWidgetSetDraw(icon, ituIconDraw);

    if (!(icon->widget.flags & ITU_EXTERNAL) && icon->staticSurf)
    {
        ITUSurface* surf = (ITUSurface*)(base + (uint32_t)icon->staticSurf);
        if (surf->flags & ITU_COMPRESSED)
            icon->surf = NULL;
        else
            icon->surf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);

        icon->staticSurf = surf;
    }
}

bool ituIconLoadJpegData(ITUIcon* icon, uint8_t* data, int size)
{
    ITUWidget* widget = (ITUWidget*)icon;
    ITURectangle* rect = &widget->rect;
    ITUSurface* surf;
    bool result = false;

    assert(icon);
    ITU_ASSERT_THREAD();

    if (widget->flags & ITU_LOADING)
    {
        widget->flags &= ~ITU_LOADING;
        icon->loadedSurf = NULL;
    }
    else if (widget->flags & ITU_LOADED)
    {
        if (icon->filePath)
        {
            free(icon->filePath);
            icon->filePath = NULL;
        }

        if (icon->loadedSurf)
        {
            ituSurfaceRelease(icon->loadedSurf);
            icon->loadedSurf = NULL;
        }
        widget->flags &= ~ITU_LOADED;
    }
    else
    {
        free(icon->filePath);
        icon->filePath = NULL;

        if (widget->flags & ITU_FIT_TO_RECT)
            surf = ituJpegLoad(rect->width, rect->height, data, size, ITU_FIT_TO_RECT);
        else if (widget->flags & ITU_CUT_BY_RECT)
            surf = ituJpegLoad(rect->width, rect->height, data, size, ITU_CUT_BY_RECT);
        else
            surf = ituJpegLoad(0, 0, data, size, 0);
        if (surf)
        {
            if (icon->surf)
                ituSurfaceRelease(icon->surf);

            icon->surf = surf;
            result = true;
        }
        ituWidgetSetDirty(icon, true);
    }
    return result;
}

static void* IconLoadJpegFileTask(void* arg)
{
    ITUWidget* widget = (ITUWidget*) arg;
    ITUIcon* icon = (ITUIcon*)widget;
    ITURectangle* rect = &widget->rect;
    ITUSurface* surf;
    char filepath[PATH_MAX];
    assert(widget);

    LOG_DBG "+IconLoadJpegFileTask\n" LOG_END

    if (!icon->filePath)
        goto end;

    strcpy(filepath, icon->filePath);

    if (widget->flags & ITU_FIT_TO_RECT)
        surf = ituJpegLoadFile(rect->width, rect->height, filepath, ITU_FIT_TO_RECT);
    else if (widget->flags & ITU_CUT_BY_RECT)
        surf = ituJpegLoadFile(rect->width, rect->height, filepath, ITU_CUT_BY_RECT);
    else
        surf = ituJpegLoadFile(0, 0, filepath, 0);

    icon->loadedSurf = surf;
    if (surf)
        widget->flags |= ITU_LOADED;

end:
    widget->flags &= ~ITU_LOADING;

    LOG_DBG "-IconLoadJpegFileTask\n" LOG_END
    return NULL;
}

void ituIconLoadJpegFile(ITUIcon* icon, char* filepath)
{
    ITUWidget* widget = (ITUWidget*)icon;
    assert(widget);
    ITU_ASSERT_THREAD();

    if ((widget->flags & ITU_LOADING) || (widget->flags & ITU_LOADED))
        return;

    if (strlen(filepath) > 0)
    {
        pthread_t task;
        pthread_attr_t attr;

        assert(!icon->filePath);
        icon->filePath = strdup(filepath);

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        widget->flags |= ITU_LOADING;
        pthread_create(&task, &attr, IconLoadJpegFileTask, icon);
    }
}

void ituIconLoadJpegFileSync(ITUIcon* icon, char* filepath)
{
    ITUWidget* widget = (ITUWidget*)icon;
    assert(widget);
    ITU_ASSERT_THREAD();

    if (strlen(filepath) > 0)
    {
        ITURectangle* rect = &widget->rect;
        ITUSurface* surf;

        if (widget->flags & ITU_FIT_TO_RECT)
            surf = ituJpegLoadFile(rect->width, rect->height, filepath, ITU_FIT_TO_RECT);
        else if (widget->flags & ITU_CUT_BY_RECT)
            surf = ituJpegLoadFile(rect->width, rect->height, filepath, ITU_CUT_BY_RECT);
        else
            surf = ituJpegLoadFile(0, 0, filepath, 0);

        if (surf)
        {
            if (icon->surf)
                ituSurfaceRelease(icon->surf);

            icon->surf = surf;
        }
    }
}

bool ituIconLoadPngData(ITUIcon* icon, uint8_t* data, int size)
{
    ITUWidget* widget = (ITUWidget*)icon;
    ITURectangle* rect = &widget->rect;
    ITUSurface* surf;
    bool result = false;

    assert(icon);
    ITU_ASSERT_THREAD();

    if (widget->flags & ITU_LOADING)
    {
        widget->flags &= ~ITU_LOADING;
        icon->loadedSurf = NULL;
    }
    else if (widget->flags & ITU_LOADED)
    {
        if (icon->filePath)
        {
            free(icon->filePath);
            icon->filePath = NULL;
        }

        if (icon->loadedSurf)
        {
            ituSurfaceRelease(icon->loadedSurf);
            icon->loadedSurf = NULL;
        }
        widget->flags &= ~ITU_LOADED;
    }
    else
    {
        free(icon->filePath);
        icon->filePath = NULL;

        if (widget->flags & ITU_FIT_TO_RECT || widget->flags & ITU_CUT_BY_RECT)
            surf = ituPngLoad(rect->width, rect->height, data, size);
        else
            surf = ituPngLoad(0, 0, data, size);
        if (surf)
        {
            if (icon->surf)
                ituSurfaceRelease(icon->surf);

            icon->surf = surf;
            result = true;
        }
        ituWidgetSetDirty(icon, true);
    }
    return result;
}

static void* IconLoadPngFileTask(void* arg)
{
    ITUWidget* widget = (ITUWidget*) arg;
    ITUIcon* icon = (ITUIcon*)widget;
    ITURectangle* rect = &widget->rect;
    ITUSurface* surf;
    char filepath[PATH_MAX];
    assert(widget);

    LOG_DBG "+IconLoadPngFileTask\n" LOG_END

    if (!icon->filePath)
        goto end;

    strcpy(filepath, icon->filePath);

    if (widget->flags & ITU_FIT_TO_RECT || widget->flags & ITU_CUT_BY_RECT)
        surf = ituPngLoadFile(rect->width, rect->height, filepath);
    else
        surf = ituPngLoadFile(0, 0, filepath);

    icon->loadedSurf = surf;
    if (surf)
        widget->flags |= ITU_LOADED;

end:
    widget->flags &= ~ITU_LOADING;

    LOG_DBG "-IconLoadPngFileTask\n" LOG_END
    return NULL;
}

void ituIconLoadPngFile(ITUIcon* icon, char* filepath)
{
    ITUWidget* widget = (ITUWidget*)icon;
    assert(widget);
    ITU_ASSERT_THREAD();

    if ((widget->flags & ITU_LOADING) || (widget->flags & ITU_LOADED))
        return;

    if (strlen(filepath) > 0)
    {
        pthread_t task;
        pthread_attr_t attr;

        assert(!icon->filePath);
        icon->filePath = strdup(filepath);

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        widget->flags |= ITU_LOADING;
        pthread_create(&task, &attr, IconLoadPngFileTask, icon);
    }
}

void ituIconLoadPngFileSync(ITUIcon* icon, char* filepath)
{
    ITUWidget* widget = (ITUWidget*)icon;
    assert(widget);
    ITU_ASSERT_THREAD();

    if (strlen(filepath) > 0)
    {
        ITURectangle* rect = &widget->rect;
        ITUSurface* surf;

        if (widget->flags & ITU_FIT_TO_RECT || widget->flags & ITU_CUT_BY_RECT)
            surf = ituPngLoadFile(rect->width, rect->height, filepath);
        else
            surf = ituPngLoadFile(0, 0, filepath);

        if (surf)
        {
            if (icon->surf)
                ituSurfaceRelease(icon->surf);

            icon->surf = surf;
        }
    }
}

void ituIconLoadStaticData(ITUIcon* icon)
{
    ITUWidget* widget = (ITUWidget*)icon;
    ITUSurface* surf;

    if (!icon->staticSurf || icon->surf || (widget->flags & ITU_EXTERNAL) || (widget->flags & ITU_EXTERNAL_IMAGE))
        return;

    surf = icon->staticSurf;

    if (surf->flags & ITU_COMPRESSED)
        icon->surf = ituSurfaceDecompress(surf);
    else
        icon->surf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
}

void ituIconReleaseSurface(ITUIcon* icon)
{
    ITUWidget* widget = (ITUWidget*)icon;
    ITU_ASSERT_THREAD();

    if (icon->surf)
    {
        ituSurfaceRelease(icon->surf);
        icon->surf = NULL;

        if (widget->flags & ITU_EXTERNAL)
        {
            ITULayer* layer = ituGetLayer(widget);
            if (icon->staticSurf)
            {
                ituLayerReleaseExternalSurface(layer);
            }
        }
    }
}

void ituIconLinkSurface(ITUIcon* icon, ITUIcon* src)
{
    assert(icon);
    assert(src);
    ITU_ASSERT_THREAD();

    if (icon->surf)
    {
        ituSurfaceRelease(icon->surf);
        icon->surf = NULL;
    }

    if (src->surf)
    {
        ITUSurface *parentSurf = (ITUSurface *)src->surf->parent;

        if (parentSurf && (parentSurf->flags & ITU_STATIC))
        {
            assert(parentSurf->flags & ITU_COMPRESSED);
            assert(parentSurf->lockSize);

            if ((!(src->widget.flags & ITU_EXTERNAL) && src->staticSurf) || ((src->widget.flags & ITU_EXTERNAL) && (icon->widget.flags & ITU_EXTERNAL)))
            {
                icon->staticSurf = src->staticSurf;
            }
            parentSurf->lockSize++;
            icon->surf = src->surf;
        }
        else if (!(src->widget.flags & ITU_EXTERNAL) && src->staticSurf)
        {
            ITUSurface* staticSurf = src->staticSurf;
            assert((staticSurf->flags & ITU_COMPRESSED) == 0);
            icon->surf = ituCreateSurface(staticSurf->width, staticSurf->height, staticSurf->pitch, staticSurf->format, (const uint8_t*)staticSurf->addr, staticSurf->flags);
        }
        else
        {
            icon->surf = src->surf;
        }
    }
}

void ituIconRmSetEnable(ITUIcon* icon, bool enable)
{
	if (icon)
	{
		icon->use_rotate_mask = (enable) ? (1) : (0);
		ituDirtyWidget(icon, true);
		ituWidgetUpdate(icon, ITU_EVENT_LAYOUT, 0, 0, 0);
	}
}

bool ituIconRmIsEnable(ITUIcon* icon)
{
	if (icon)
	{
		if (icon->use_rotate_mask > 0)
			return true;
		else
			return false;
	}
	else
		return false;
}

bool ituIconRmSetAngle(ITUIcon* icon, int start_angle, int end_angle)
{
	if (icon)
	{
		bool check_angle = false;

		if (end_angle <= start_angle)
			return false;
		else if (start_angle < 0)
		{
			if ((360 - end_angle) <= (-start_angle))
				return false;
			else
				check_angle = true;
		}
		else if (end_angle > 360)
		{
			if ((end_angle - 360) >= start_angle)
				return false;
			else
				check_angle = true;
		}
		else
			check_angle = true;

		if (check_angle)
		{
			icon->start_rm_angle = start_angle;
			icon->end_rm_angle = end_angle;
			if (icon->use_rotate_mask > 0)
			{
				ituIconRmSetEnable(icon, true);
			}
		}
		return check_angle;
	}
	else
		return false;
}