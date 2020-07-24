#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/time.h>
#include "ite/itp.h"
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

#define COVERFLOW_FAST_SLIDE_TIMECHECK 140
#define ITU_COVERFLOW_ANYSTOP       0x10
#define ITU_COVERFLOW_ANYBOUNCE       0x20
#define ITU_COVERFLOW_ANYBOUNCE1       0x40
#define ITU_COVERFLOW_ANYBOUNCE2       0x80
#define ITU_BOUNCE_1 0x1000
#define ITU_BOUNCE_2 0x2000

#define COVERFLOW_FACTOR 10
#define COVERFLOW_PROCESS_STAGE1 0.2f
#define COVERFLOW_PROCESS_STAGE2 0.4f
#define COVERFLOW_OVERLAP_MAX_PERCENTAGE 80
#define COVERFLOW_SMOOTH_LASTFRAME_COUNT 5

#define COVERFLOW_DEBUG_FOCUSINDEX 0
#define COVERFLOW_DEBUG_SETXY 0

static const char coverFlowName[] = "ITUCoverFlow";

int CoverFlowGetVisibleChildCount(ITUCoverFlow* coverflow)
{
    int i = 0;
    ITCTree *child, *tree = (ITCTree*)coverflow;
    assert(tree);

    for (child = tree->child; child; child = child->sibling)
    {
        ITUWidget* widget = (ITUWidget*)child;
        if (widget->visible)
            i++;
    }
    return i;
}

ITUWidget* CoverFlowGetVisibleChild(ITUCoverFlow* coverflow, int index)
{
    int i = 0;
    ITCTree *child, *tree = (ITCTree*)coverflow;
    assert(tree);

    for (child = tree->child; child; child = child->sibling)
    {
        ITUWidget* widget = (ITUWidget*)child;

		if (widget->visible)
		{
			if (i++ == index)
				return widget;
		}
    }
    return NULL;
}

float CoverFlowAniStepCal(ITUCoverFlow* coverFlow)
{
	int way = (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING) ? (-1) : (1);
	float step = 0.0;

	if (way > 0)
	{
		step = (float)coverFlow->frame / (float)coverFlow->totalframe;
	}
	else
	{
		step = (float)(coverFlow->totalframe - coverFlow->frame) / (float)coverFlow->totalframe;
	}

	//step = step - 1;

	return step;
}

int CoverFlowCheckBoundaryTouch(ITUWidget* widget)
{
	ITUCoverFlow* coverFlow = (ITUCoverFlow*)widget;
	ITUWidget* childbase = CoverFlowGetVisibleChild(coverFlow, 0);
	int count = CoverFlowGetVisibleChildCount(coverFlow);
	int base_size;
	int max_neighbor_item;
	int max_width_item;
	int result = 0;

	if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
	{
		base_size = childbase->rect.height - (((coverFlow->overlapsize > 0)) ? (coverFlow->overlapsize) : (0));
	}
	else
	{
		base_size = childbase->rect.width - (((coverFlow->overlapsize > 0)) ? (coverFlow->overlapsize) : (0));
	}

	max_neighbor_item = ((widget->rect.width / base_size) - 1) / 2;
	max_width_item = widget->rect.width / base_size;

	if (max_neighbor_item == 0)
		max_neighbor_item++;

	if (coverFlow->focusIndex >= max_neighbor_item)
	{
		if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_neighbor_item))
			result = ITU_BOUNCE_2;
		else
		{
			if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_width_item))
				result = ITU_BOUNCE_2;
		}
	}
	else
		result = ITU_BOUNCE_1;

	if (result)
		coverFlow->temp3 = result;

	return result;
}

void CoverFlowFlushQueue(ITUWidget* widget, ITUCoverFlow* coverFlow, int count, int widget_size, int base_size)
{
	if (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP))
	{
		int i = 0;
		bool no_queue = true;
		coverFlow->frame = 0;
		ituCoverFlowOnCoverChanged(coverFlow, widget);

		for (i = 0; i < COVERFLOW_MAX_PROCARR_SIZE; i++)
		{
			if (coverFlow->procArr[i] != 0)
			{
				coverFlow->procArr[i] = 0;

				if ((i + 1) < COVERFLOW_MAX_PROCARR_SIZE)
				{
					if (coverFlow->procArr[i + 1] != 0)
						no_queue = false;
				}

				break;
			}
		}

		if (no_queue)
		{
			coverFlow->inc = 0;
			widget->flags &= ~ITU_UNDRAGGING;
			ituWidgetUpdate(widget, ITU_EVENT_LAYOUT, 0, 0, 0);
		}
		else
		{
			bool boundary_touch = false;

			if (coverFlow->boundaryAlign)
			{
				int max_neighbor_item = ((widget_size / base_size) - 1) / 2;

				coverFlow->slideCount = 0;

				if (max_neighbor_item == 0)
					max_neighbor_item++;

				if (coverFlow->focusIndex >= max_neighbor_item)
				{
					if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_neighbor_item))
						boundary_touch = true;
					else
					{
						ITUWidget* cf = CoverFlowGetVisibleChild(coverFlow, count - 1);
						if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
						{
							if ((cf->rect.y + cf->rect.height) <= widget_size)
								boundary_touch = true;
						}
						else
						{
							if ((cf->rect.x + cf->rect.width) <= widget_size)
								boundary_touch = true;
						}
					}
				}
				else
					boundary_touch = true;
			}

			if (!boundary_touch)
			{
				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
				{
					if (coverFlow->procArr[i + 1] < 0)
						ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDEDOWN, 0, widget->rect.x, widget->rect.y);
					else if (coverFlow->procArr[i + 1] > 0)
						ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDEUP, 0, widget->rect.x, widget->rect.y);
				}
				else
				{
					if (coverFlow->procArr[i + 1] < 0)
						ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDERIGHT, 0, widget->rect.x, widget->rect.y);
					else if (coverFlow->procArr[i + 1] > 0)
						ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDELEFT, 0, widget->rect.x, widget->rect.y);
				}
			}
		}

		ituScene->dragged = NULL;
	}
}

void CoverFlowCleanQueue(ITUCoverFlow* coverflow)
{
	int i;

	for (i = COVERFLOW_MAX_PROCARR_SIZE - 1; i >= 0; i--)
	{
		coverflow->procArr[i] = 0;
	}

}

void ituCoverFlowSetXY(ITUCoverFlow* coverFlow, int index, int xy, int line)
{
	ITUWidget* coverflowwidget = (ITUWidget*)coverFlow;
	ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);

	if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
	{
		ituWidgetSetY(child, xy);

		if (COVERFLOW_DEBUG_SETXY)
			printf("[Debug][Coverflow %s][Child %d][SetY %d][%d]\n", coverflowwidget->name, index, xy, line);
	}
	else
	{
		ituWidgetSetX(child, xy);

		if (COVERFLOW_DEBUG_SETXY)
			printf("[Debug][Coverflow %s][Child %d][SetX %d][%d]\n", coverflowwidget->name, index, xy, line);
	}
}

void CoverFlowLayout(ITUWidget* widget)
{
	ITUCoverFlow* coverFlow = (ITUCoverFlow*)widget;

	if (coverFlow != NULL)
	{
		int i, count = CoverFlowGetVisibleChildCount(coverFlow);
		int base_size;
		bool open_debug = false;
		ITUWidget* childbase = CoverFlowGetVisibleChild(coverFlow, 0);

		if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
		{
			base_size = childbase->rect.height - (((coverFlow->overlapsize > 0)) ? (coverFlow->overlapsize) : (0));
		}
		else
		{
			base_size = childbase->rect.width - (((coverFlow->overlapsize > 0)) ? (coverFlow->overlapsize) : (0));
		}

		if (count > 0)
		{
			if (coverFlow->focusIndex > (count - 1))
				coverFlow->focusIndex = count - 1;
			else if (coverFlow->focusIndex < 0)
				coverFlow->focusIndex = 0;

			if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
			{
				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
				{
					int index, count2;

					count2 = count / 2 + 1;
					index = coverFlow->focusIndex;

					for (i = 0; i < count2; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
						int fy = widget->rect.height / 2 - child->rect.height / 2;
						fy += i * child->rect.height;
						ituCoverFlowSetXY(coverFlow, index, fy, __LINE__);

						if (index >= count - 1)
							index = 0;
						else
							index++;
					}

					count2 = count - count2;
					for (i = 0; i < count2; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
						int fy = widget->rect.height / 2 - child->rect.height / 2;
						fy -= count2 * child->rect.height;
						fy += i * child->rect.height;
						ituCoverFlowSetXY(coverFlow, index, fy, __LINE__);

						if (index >= count - 1)
							index = 0;
						else
							index++;
					}
				}
				else //[LAYOUT][Vertical][non-cycle]
				{
					for (i = 0; i < count; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
						int fy = widget->rect.height / 2 - base_size / 2;

						if (coverFlow->boundaryAlign)
						{
							int max_neighbor_item = ((widget->rect.height / base_size) - 1) / 2;
							int max_height_item = (widget->rect.height / base_size);
							fy = 0;

							if (max_neighbor_item == 0)
								max_neighbor_item++;

							if (coverFlow->focusIndex > 0) //>= max_neighbor_item) //Bless new debug
							{
								//if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_neighbor_item))
								if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_height_item))
									fy = widget->rect.height - (count * base_size);
								else
									fy -= base_size * coverFlow->focusIndex;
							}
							else
								fy = 0;
						}
						else
						{
							fy -= base_size * coverFlow->focusIndex;
						}

						if (coverFlow->overlapsize > 0)
						{
							fy += i * base_size;
							ituCoverFlowSetXY(coverFlow, i, fy - coverFlow->overlapsize, __LINE__);
						}
						else
						{
							fy += i * child->rect.height;
							ituCoverFlowSetXY(coverFlow, i, fy, __LINE__);
						}
					}
				}
			}
			else
			{
				//[LAYOUT][Horizontal][cycle]
				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
				{
					int index, count2;

					count2 = count / 2 + 1;
					index = coverFlow->focusIndex;

					for (i = 0; i < count2; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
						int fx = widget->rect.width / 2 - child->rect.width / 2;
						fx += i * child->rect.width;
						ituCoverFlowSetXY(coverFlow, index, fx, __LINE__);

						if (index >= count - 1)
							index = 0;
						else
							index++;
					}

					count2 = count - count2;
					for (i = 0; i < count2; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
						int fx = widget->rect.width / 2 - child->rect.width / 2;
						fx -= count2 * child->rect.width;
						fx += i * child->rect.width;
						ituCoverFlowSetXY(coverFlow, index, fx, __LINE__);

						if (index >= count - 1)
							index = 0;
						else
							index++;
					}
				}
				else //[LAYOUT][Horizontal][non-cycle]
				{
					for (i = 0; i < count; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
						int fx = widget->rect.width / 2 - base_size / 2;

						if (coverFlow->boundaryAlign)
						{
							int max_neighbor_item = ((widget->rect.width / base_size) - 1) / 2;
							int max_width_item = (widget->rect.width / base_size);

							fx = 0;

							if (max_neighbor_item == 0)
								max_neighbor_item++;

							if (coverFlow->focusIndex > 0)
							{
								if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_width_item))
									fx = widget->rect.width - (count * base_size);
								else
									fx -= base_size * coverFlow->focusIndex;
							}
							else
								fx = 0;
						}
						else
						{
							fx -= base_size * coverFlow->focusIndex;
						}

						if (coverFlow->overlapsize > 0)
						{
							fx += i * base_size;
							ituCoverFlowSetXY(coverFlow, i, fx - coverFlow->overlapsize, __LINE__);
						}
						else
						{
							fx += i * child->rect.width;
							ituCoverFlowSetXY(coverFlow, i, fx, __LINE__);
						}
					}
				}
			}

			if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ENABLE_ALL) == 0)
			{
				for (i = 0; i < count; ++i)
				{
					ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);

					if (i == coverFlow->focusIndex)
						ituWidgetEnable(child);
					else
						ituWidgetDisable(child);
				}
			}
			widget->flags &= ~ITU_DRAGGING;
			coverFlow->touchCount = 0;

			//fix for stop anywhere not display after load
			if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
			{
				ITUWidget* widget = (ITUWidget*)coverFlow;
				int count = CoverFlowGetVisibleChildCount(coverFlow);
				int i = 0;
				int fd = 0;
				int move_step = 0;
				ITUWidget* child_1 = CoverFlowGetVisibleChild(coverFlow, 0);
				ITUWidget* child_2 = CoverFlowGetVisibleChild(coverFlow, count - 1);

				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
				{
					if ((child_1->rect.y > 0) || ((child_2->rect.y + child_2->rect.height) < widget->rect.height))
					{
						if (child_1->rect.y > 0)
							move_step = 0 - child_1->rect.y;
						else
							move_step = widget->rect.height - (child_2->rect.y + child_2->rect.height);

						for (i = 0; i < count; i++)
						{
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
							fd = child->rect.y;
							fd += move_step;
							ituCoverFlowSetXY(coverFlow, i, fd, __LINE__);
						}

						coverFlow->frame = 0;
					}
				}
				else
				{
					if ((child_1->rect.x > 0) || ((child_2->rect.x + child_2->rect.width) < widget->rect.width))
					{
						if (child_1->rect.x > 0)
							move_step = 0 - child_1->rect.x;
						else
							move_step = widget->rect.width - (child_2->rect.x + child_2->rect.width);

						for (i = 0; i < count; i++)
						{
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
							fd = child->rect.x;
							fd += move_step;
							ituCoverFlowSetXY(coverFlow, i, fd, __LINE__);
						}

						coverFlow->frame = 0;
					}
				}
			}

			if (open_debug)
			{
				int x;
				ITUWidget* cc;

				for (x = 0; x < count; x++)
				{
					cc = CoverFlowGetVisibleChild(coverFlow, x);

					if (x != 1)
						continue;

					if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
						printf("[CoverFlowLayout] [%d] [%d]\n", x, cc->rect.y);
					else
						printf("[CoverFlowLayout] [%d] [%d]\n", x, cc->rect.x);
				}
			}
		}
	}
}

void ituCoverFlowFixFC(ITUCoverFlow* coverFlow)
{
	int count = CoverFlowGetVisibleChildCount(coverFlow);

	if ((count > 0) && (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)))
	{
		//printf("FC checking\n");
		if (coverFlow->focusIndex >= count)
			coverFlow->focusIndex = count - 1;
		else if (coverFlow->focusIndex < 0)
			coverFlow->focusIndex = 0;
	}
}

bool ituCoverFlowUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
	int widget_size, base_size;
    ITUCoverFlow* coverFlow = (ITUCoverFlow*) widget;
	assert(coverFlow);

	if (coverFlow)
	{
		ITUWidget* childbase = CoverFlowGetVisibleChild(coverFlow, 0);
		ITUWidget* cc = NULL;
		int count = CoverFlowGetVisibleChildCount(coverFlow);
        int i, min_w, min_h, min = 0;

        if (!childbase)
            return result;

		min_w = childbase->rect.width;
		min_h = childbase->rect.height;

		for (i = 1; i < count; i++)
		{
			cc = CoverFlowGetVisibleChild(coverFlow, i);

			if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
			{
				if (cc->rect.height < min_h)
				{
					min_h = cc->rect.height;
					childbase = cc;
				}
			}
			else
			{
				if (cc->rect.width < min_w)
				{
					min_w = cc->rect.width;
					childbase = cc;
				}
			}
		}

		if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
		{
			base_size = childbase->rect.height - (((coverFlow->overlapsize > 0)) ? (coverFlow->overlapsize) : (0));
			widget_size = widget->rect.height;
		}
		else
		{
			base_size = childbase->rect.width - (((coverFlow->overlapsize > 0)) ? (coverFlow->overlapsize) : (0));
			widget_size = widget->rect.width;
		}

		if (coverFlow->bounceRatio <= 0)
			coverFlow->bounceRatio = base_size;
	}

    if ((widget->flags & ITU_TOUCHABLE) && ituWidgetIsEnabled(widget) && (ev == ITU_EVENT_MOUSEDOWN || ev == ITU_EVENT_MOUSEUP))
    {
        int x = arg2 - widget->rect.x;
		int y = arg3 - widget->rect.y;

		coverFlow->boundary_touch_memo = 0;

		//if (ev == ITU_EVENT_MOUSEUP)
		//	coverFlow->touchPos = 0;

        if (ituWidgetIsInside(widget, x, y))
            result |= ituFlowWindowUpdate(widget, ev, arg1, arg2, arg3);
    }
    else
    {
        result |= ituFlowWindowUpdate(widget, ev, arg1, arg2, arg3);
    }

    if (widget->flags & ITU_TOUCHABLE) 
    {
		bool fast_slide = false;

		if (ev == ITU_EVENT_TOUCHSLIDELEFT || ev == ITU_EVENT_TOUCHSLIDERIGHT || ev == ITU_EVENT_TOUCHSLIDEUP || ev == ITU_EVENT_TOUCHSLIDEDOWN)
		{
			// to fix when slide at Non-Cycle mode boundary area
			if (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)) 
			{
				int count = CoverFlowGetVisibleChildCount(coverFlow);
				ITUWidget* child_first = CoverFlowGetVisibleChild(coverFlow, 0);
				ITUWidget* child_last  = CoverFlowGetVisibleChild(coverFlow, count - 1);
				int pos1, pos2;
				bool bForceMouseUp = false;

				if (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP))
				{
					if (ev == ITU_EVENT_TOUCHSLIDELEFT)
					{
						int next = ((coverFlow->inc < 0) ? (1) : (0));
						//printf("[LEFT] [inc %d] [Fi %d] [frame %d]\n", coverFlow->inc, coverFlow->focusIndex, coverFlow->frame);
						if ((coverFlow->focusIndex + next) >= (count - 1))
							return true;

						if (coverFlow->bounceRatio)
						{
							if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
							{
								if ((child_last->rect.y + base_size) <= (widget_size - (base_size / coverFlow->bounceRatio)))
									return true;
							}
							else
							{
								if ((child_last->rect.x + base_size) <= (widget_size - (base_size / coverFlow->bounceRatio)))
									return true;
							}
						}
					}
					else if (ev == ITU_EVENT_TOUCHSLIDERIGHT)
					{
						int next = ((coverFlow->inc > 0) ? (-1) : (0));
						//printf("[RIGHT] [inc %d] [Fi %d] [frame %d]\n", coverFlow->inc, coverFlow->focusIndex, coverFlow->frame);
						if ((coverFlow->focusIndex + next) <= 0)
							return true;
					}
					else if (ev == ITU_EVENT_TOUCHSLIDEUP)
					{
						int next = ((coverFlow->inc < 0) ? (1) : (0));
						//printf("[UP] [inc %d] [Fi %d] [frame %d]\n", coverFlow->inc, coverFlow->focusIndex, coverFlow->frame);
						if ((coverFlow->focusIndex + next) >= (count - 1))
							return true;
					}
					else if (ev == ITU_EVENT_TOUCHSLIDEDOWN)
					{
						int next = ((coverFlow->inc > 0) ? (-1) : (0));
						//printf("[DOWN] [inc %d] [Fi %d] [frame %d]\n", coverFlow->inc, coverFlow->focusIndex, coverFlow->frame);
						if ((coverFlow->focusIndex + next) <= 0)
							return true;
					}
				}

				if ((coverFlow->boundaryAlign) || (count <= 2))
				{
					pos1 = 0;
					pos2 = widget_size;
				}
				else
				{
					pos1 = (widget_size - base_size) / 2;
					pos2 = (widget_size + base_size) / 2;
				}

				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
				{
					if (child_first->rect.y > pos1)
					{
						bForceMouseUp = true;
					}
					else if ((child_last->rect.y + child_last->rect.height) < pos2)
					{
						bForceMouseUp = true;
					}
				}
				else
				{
					if (child_first->rect.x > pos1)
					{
						bForceMouseUp = true;
					}
					else if ((child_last->rect.x + child_last->rect.width) < pos2)
					{
						bForceMouseUp = true;
					}
				}

				if (bForceMouseUp)
				{
					ituUnPressWidget(widget);
					ituWidgetUpdate(coverFlow, ITU_EVENT_MOUSEUP, arg1, arg2, arg3);
					return true;
				}
			}

			//this should be check for goose again
			if ((itpGetTickCount() - coverFlow->clock) < COVERFLOW_FAST_SLIDE_TIMECHECK)
				fast_slide = true;

			coverFlow->slide_diff = arg1;

			if (ituWidgetIsEnabled(widget) && !result)
			{
				int x = arg2 - widget->rect.x;
				int y = arg3 - widget->rect.y;

				if (!widget->rect.width || !widget->rect.height || ituWidgetIsInside(widget, x, y))
				{
					result |= ituExecActions(widget, coverFlow->actions, ev, arg1);
				}
			}

			///////try to fix no slide and no mouseup at the same time
			if ((coverFlow->slideMaxCount == 0) && (coverFlow->prevnext_trigger == 0))
			{
				//coverFlow->coverFlowFlags |= ITU_COVERFLOW_SLIDING;
				ituWidgetUpdate(coverFlow, ITU_EVENT_MOUSEUP, arg1, arg2, arg3);
			}
		}

		if (((ev == ITU_EVENT_TOUCHSLIDELEFT || ev == ITU_EVENT_TOUCHSLIDERIGHT) 
			&& ((coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL) == 0)
			&& (coverFlow->slideMaxCount > 0)) || coverFlow->prevnext_trigger)
		{
			coverFlow->touchCount = 0;

			if (ituWidgetIsEnabled(widget))
			{
				int x = arg2 - widget->rect.x;
				int y = arg3 - widget->rect.y;

				if (ituWidgetIsInside(widget, x, y))
				{
					int count = CoverFlowGetVisibleChildCount(coverFlow);
					if (count > 0)
					{
						bool boundary_touch = false;
						bool boundary_touch_left = false;
						bool boundary_touch_right = false;
						////try to fix the mouse up shadow(last frame) diff when sliding start(frame 0)
						int offset, absoffset, interval;
						offset = x - coverFlow->touchPos;
						interval = offset / base_size;
						offset -= (interval * base_size);
						absoffset = offset > 0 ? offset : -offset;

						if (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING))
						{
							if (absoffset > base_size / 2)
								coverFlow->frame = (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
							else if (absoffset)
								coverFlow->frame = coverFlow->totalframe - (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
							else
								coverFlow->frame = 0;
						}
						
						//if (widget->flags & ITU_DRAGGABLE)
						//	coverFlow->frame = coverFlow->totalframe - ((abs(x - coverFlow->touchPos) * coverFlow->totalframe)/base_size) + 1;

						fast_slide = false;

						if ((!(widget->flags & ITU_DRAGGABLE)) || fast_slide)
						{//debug here
							//if (!(widget->flags & ITU_DRAGGABLE))
							if (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING))
								coverFlow->frame = 0;
							printf("[CoverFlow][Fast Slide!!]\n\n");
						}
						else
							printf("[CoverFlow][Normal Slide!!]\n\n");

                        ituUnPressWidget(widget);

						//check boundary touch for H non-cycle
						if (coverFlow->boundaryAlign)
						{
							int max_neighbor_item = ((widget_size / base_size) - 1) / 2;

							coverFlow->slideCount = 0;

							if (max_neighbor_item == 0)
								max_neighbor_item++;

							if (coverFlow->focusIndex >= max_neighbor_item)
							{
								if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_neighbor_item))
								{
									boundary_touch = true;
									boundary_touch_right = true;
									coverFlow->coverFlowFlags |= ITU_BOUNCE_2;
								}
								else
								{
									ITUWidget* cf = CoverFlowGetVisibleChild(coverFlow, count - 1);
									if ((cf->rect.x + cf->rect.width) <= widget_size)
									{
										boundary_touch = true;
										boundary_touch_right = true;
										coverFlow->coverFlowFlags |= ITU_BOUNCE_2;
									}
								}
							}
							else
							{
								boundary_touch = true;
								boundary_touch_left = true;
								coverFlow->coverFlowFlags |= ITU_BOUNCE_1;
							}
						}

						if (ev == ITU_EVENT_TOUCHSLIDELEFT)
						{//debugging
							if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING) && (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)))
							{
								if (coverFlow->inc > 0)
								{
									if (coverFlow->totalframe != coverFlow->org_totalframe)
									{
										coverFlow->frame /= 2;
										coverFlow->totalframe = coverFlow->org_totalframe;
										printf("[mark totalframe %d]\n", coverFlow->totalframe);
									}
									coverFlow->frame = coverFlow->totalframe - coverFlow->frame;
									coverFlow->inc *= -1;
									if (coverFlow->focusIndex > 0)
									{
										if (COVERFLOW_DEBUG_FOCUSINDEX)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - 1, __LINE__);

										coverFlow->focusIndex--;
									}
									return true;
								}
								else if (coverFlow->inc < 0)
								{
									if (coverFlow->totalframe != coverFlow->org_totalframe)
									{
										coverFlow->frame /= 2;
										coverFlow->totalframe = coverFlow->org_totalframe;
										printf("[mark totalframe %d]\n", coverFlow->totalframe);
									}
									if (coverFlow->focusIndex < (count - 2))
									{
										coverFlow->frame = 0;

										if (COVERFLOW_DEBUG_FOCUSINDEX)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex + 1, __LINE__);

										coverFlow->focusIndex++;
									}
									else
									{
										coverFlow->frame = coverFlow->totalframe;

										if (COVERFLOW_DEBUG_FOCUSINDEX)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex + 1, __LINE__);

										coverFlow->focusIndex++;
									}
									ituCoverFlowFixFC(coverFlow);
									ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
									return true;
								}
								else
								{
									if (coverFlow->focusIndex <= (count - 2))
									{
										coverFlow->frame = 0;

										if (COVERFLOW_DEBUG_FOCUSINDEX)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex + 1, __LINE__);

										coverFlow->focusIndex++;
									}
								}
							}

							//if ((coverFlow->slideMaxCount > 0) && (widget->flags & ITU_DRAGGABLE))//(coverFlow->boundaryAlign)
							//fix for non-draggable will make animation reverse
							if ((coverFlow->slideMaxCount > 0) || (coverFlow->prevnext_trigger))
							{
								coverFlow->prevnext_trigger = 0;
								coverFlow->coverFlowFlags |= ITU_COVERFLOW_SLIDING;
								coverFlow->touchCount = 0;
							}

							if (widget->flags & ITU_DRAGGING)
							{
								widget->flags &= ~ITU_DRAGGING;
								ituScene->dragged = NULL;
								coverFlow->inc = 0;
							}

							if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE) ||
								(coverFlow->focusIndex < (count - 1)) || boundary_touch)
							{
								if (count > 0)
								{
									if (widget->flags & ITU_DRAGGING)
									{
										widget->flags &= ~ITU_DRAGGING;
										ituScene->dragged = NULL;
										coverFlow->inc = 0;
									}

									

									if (coverFlow->inc == 0)
										coverFlow->inc = 0 - base_size;

									if (boundary_touch)
									{
										ITUWidget* cf = CoverFlowGetVisibleChild(coverFlow, count - 1);
										if (((cf->rect.x + cf->rect.width) <= widget_size) || (cf->rect.width > widget_size))
										{
											coverFlow->inc = -1;
											coverFlow->frame = coverFlow->totalframe - 1;

											//if ((boundary_touch) && (coverFlow->focusIndex > 0))
											//	coverFlow->focusIndex = CoverFlowGetVisibleChildCount(coverFlow) - widget_size / base_size;

											if (boundary_touch_right && !(widget->flags & ITU_DRAGGING) && coverFlow->bounceRatio > 0)
											{
												coverFlow->inc = 0 - (base_size / coverFlow->bounceRatio);

												if (COVERFLOW_DEBUG_FOCUSINDEX)
													printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex + 1, __LINE__);

												coverFlow->focusIndex++;
												
												widget->flags |= ITU_BOUNCING;
												coverFlow->frame = 0;
											}
										}
									}
								}
							}
							else if (coverFlow->focusIndex >= count - 1)
							{//maybe useless now
								//try to fix the ScaleCoverFlow side effect for non-cycle mode
								if ((!(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)) && (coverFlow->boundaryAlign == 0))
								{
									coverFlow->inc = 0;
								}
								else if ((count) > 0 && !(widget->flags & ITU_DRAGGING) && coverFlow->bounceRatio > 0)
								{
									if (coverFlow->inc == 0)
										coverFlow->inc = 0 - (base_size / coverFlow->bounceRatio);

									widget->flags |= ITU_BOUNCING;
								}
							}
						}
						else // if (ev == ITU_EVENT_TOUCHSLIDERIGHT)
						{//debugging
							if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING) && (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)))
							{
								if (coverFlow->inc < 0)
								{
									if (coverFlow->totalframe != coverFlow->org_totalframe)
									{
										coverFlow->frame /= 2;
										coverFlow->totalframe = coverFlow->org_totalframe;
										printf("[mark totalframe %d]\n", coverFlow->totalframe);
									}
									coverFlow->frame = coverFlow->totalframe - coverFlow->frame;
									coverFlow->inc *= -1;
									if (coverFlow->focusIndex < (count - 1))
									{
										if (COVERFLOW_DEBUG_FOCUSINDEX)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex + 1, __LINE__);

										coverFlow->focusIndex++;
									}
									return true;
								}
								else if (coverFlow->inc > 0)
								{
									if (coverFlow->totalframe != coverFlow->org_totalframe)
									{
										coverFlow->frame /= 2;
										coverFlow->totalframe = coverFlow->org_totalframe;
										printf("[mark totalframe %d]\n", coverFlow->totalframe);
									}
									if (coverFlow->focusIndex > 1)
									{
										coverFlow->frame = 0;

										if (COVERFLOW_DEBUG_FOCUSINDEX)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - 1, __LINE__);

										coverFlow->focusIndex--;
									}
									else
									{
										coverFlow->frame = coverFlow->totalframe;

										if (COVERFLOW_DEBUG_FOCUSINDEX)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - 1, __LINE__);

										coverFlow->focusIndex--;
									}
									ituCoverFlowFixFC(coverFlow);
									ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
									return true;
								}
								else
								{
									if (coverFlow->focusIndex >= 1)
									{
										coverFlow->frame = 0;

										if (COVERFLOW_DEBUG_FOCUSINDEX)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - 1, __LINE__);

										coverFlow->focusIndex--;
									}
								}
							}

							//if ((coverFlow->slideMaxCount > 0) && (widget->flags & ITU_DRAGGABLE))//(coverFlow->boundaryAlign)
							//fix for non-draggable will make animation reverse
							if ((coverFlow->slideMaxCount > 0) || (coverFlow->prevnext_trigger))
							{
								coverFlow->prevnext_trigger = 0;
								coverFlow->coverFlowFlags |= ITU_COVERFLOW_SLIDING;
								coverFlow->touchCount = 0;
							}

							if (widget->flags & ITU_DRAGGING)
							{
								widget->flags &= ~ITU_DRAGGING;
								ituScene->dragged = NULL;
								coverFlow->inc = 0;
							}

							if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE) ||
								(coverFlow->focusIndex > 0) || boundary_touch)
							{
								if (count > 0)
								{
									if (widget->flags & ITU_DRAGGING)
									{
										widget->flags &= ~ITU_DRAGGING;
										ituScene->dragged = NULL;
										coverFlow->inc = 0;
									}

									//if (boundary_touch)
									//	coverFlow->focusIndex -= ((coverFlow->focusIndex > 1) ? (2) : (0));

									

									if (coverFlow->inc == 0)
										coverFlow->inc = base_size;

									if (boundary_touch)
									{
										ITUWidget* cf = CoverFlowGetVisibleChild(coverFlow, count - 1);
										if ((cf->rect.x + cf->rect.width) <= widget_size)
										{
											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("[Debug][CoverFlow][FC %d ", coverFlow->focusIndex);

											coverFlow->focusIndex = CoverFlowGetVisibleChildCount(coverFlow) - widget_size / base_size;

											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("to %d][%d]\n", coverFlow->focusIndex, __LINE__);
										}

										cf = CoverFlowGetVisibleChild(coverFlow, 0);
										if ((cf->rect.x) >= 0)
										{
											coverFlow->inc = 1;
											coverFlow->frame = coverFlow->totalframe - 1;
										}

										if (boundary_touch_left && !(widget->flags & ITU_DRAGGING) && coverFlow->bounceRatio > 0)
										{
											coverFlow->inc = (base_size / coverFlow->bounceRatio);

											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex + 1, __LINE__);

											coverFlow->focusIndex++;
											widget->flags |= ITU_BOUNCING;
											coverFlow->frame = 0;
										}
									}
								}
							}
							else if (coverFlow->focusIndex <= 0)
							{//maybe useless now
								//try to fix the ScaleCoverFlow side effect for non-cycle mode
								if ((!(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)) && (coverFlow->boundaryAlign == 0))
								{
									if (widget->flags & ITU_DRAGGING)
										widget->flags &= ~ITU_DRAGGING;
									coverFlow->inc = 0;
								}
								else if (count > 0 && !(widget->flags & ITU_DRAGGING) && coverFlow->bounceRatio > 0)
								{
									if (coverFlow->inc == 0)
										coverFlow->inc = (base_size / coverFlow->bounceRatio);

									widget->flags |= ITU_BOUNCING;
									//coverFlow->frame = 1;
								}
							}
						}
						result = true;
					}
				}
			}

			if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
				coverFlow->frame = 0;
		}
        else if (((ev == ITU_EVENT_TOUCHSLIDEUP || ev == ITU_EVENT_TOUCHSLIDEDOWN) 
			&& (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
			&& (coverFlow->slideMaxCount > 0)) || coverFlow->prevnext_trigger)
        {
            coverFlow->touchCount = 0;

			if (ituWidgetIsEnabled(widget))// && (widget->flags & ITU_DRAGGABLE))
            {
                int x = arg2 - widget->rect.x;
                int y = arg3 - widget->rect.y;

                if (ituWidgetIsInside(widget, x, y))
                {
					int count = CoverFlowGetVisibleChildCount(coverFlow);
					if (count > 0)
					{
						bool boundary_touch = false;
						bool boundary_touch_top = false;
						bool boundary_touch_bottom = false;
						////try to fix the mouse up shadow(last frame) diff when sliding start(frame 0)
						int offset, absoffset, interval;
						offset = y - coverFlow->touchPos;
						interval = offset / base_size;
						offset -= (interval * base_size);
						absoffset = offset > 0 ? offset : -offset;

						if (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING))
						{
							if (absoffset > base_size / 2)
								coverFlow->frame = (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
							else if (absoffset)
								coverFlow->frame = coverFlow->totalframe - (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
							else
								coverFlow->frame = 0;
						}

						if ((!(widget->flags & ITU_DRAGGABLE)) || fast_slide)
						{
							//if (!(widget->flags & ITU_DRAGGABLE))
							if (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING))
								coverFlow->frame = 0;
							LOG_DBG "[CoverFlow][Fast Slide!!]\n\n" LOG_END
						}
						else
							LOG_DBG "[CoverFlow][Normal Slide!!]\n\n" LOG_END

                        ituUnPressWidget(widget);

						if (coverFlow->boundaryAlign)
						{
							int max_neighbor_item = ((widget->rect.height / base_size) - 1) / 2;

							coverFlow->slideCount = 0;

							if (max_neighbor_item == 0)
								max_neighbor_item++;

							if (coverFlow->focusIndex >= max_neighbor_item)
							{
								if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_neighbor_item))
								{
									boundary_touch = true;
									boundary_touch_bottom = true;
									coverFlow->coverFlowFlags |= ITU_BOUNCE_2;
								}
								else
								{
									ITUWidget* cf = CoverFlowGetVisibleChild(coverFlow, count - 1);
									if ((cf->rect.y + cf->rect.height) <= widget_size)
									{
										boundary_touch = true;
										boundary_touch_bottom = true;
										coverFlow->coverFlowFlags |= ITU_BOUNCE_2;
									}
								}
							}
							else
							{
								boundary_touch = true;
								boundary_touch_top = true;
								coverFlow->coverFlowFlags |= ITU_BOUNCE_1;
							}
						}

						if (ev == ITU_EVENT_TOUCHSLIDEUP)
						{
							if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING) && (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)))
							{
								if (coverFlow->inc > 0)
								{
									if (coverFlow->totalframe != coverFlow->org_totalframe)
									{
										coverFlow->frame /= 2;
										coverFlow->totalframe = coverFlow->org_totalframe;
										printf("[mark totalframe %d]\n", coverFlow->totalframe);
									}
									coverFlow->frame = coverFlow->totalframe - coverFlow->frame;
									coverFlow->inc *= -1;
									if (coverFlow->focusIndex > 0)
									{
										if (COVERFLOW_DEBUG_FOCUSINDEX)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - 1, __LINE__);

										coverFlow->focusIndex--;
									}
									return true;
								}
								else if (coverFlow->inc < 0)
								{
									if (coverFlow->totalframe != coverFlow->org_totalframe)
									{
										coverFlow->frame /= 2;
										coverFlow->totalframe = coverFlow->org_totalframe;
										printf("[mark totalframe %d]\n", coverFlow->totalframe);
									}
									if (coverFlow->focusIndex < (count - 2))
									{
										coverFlow->frame = 0;

										if (COVERFLOW_DEBUG_FOCUSINDEX)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex + 1, __LINE__);

										coverFlow->focusIndex++;
									}
									else
									{
										coverFlow->frame = coverFlow->totalframe;

										if (COVERFLOW_DEBUG_FOCUSINDEX)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex + 1, __LINE__);

										coverFlow->focusIndex++;
									}
									ituCoverFlowFixFC(coverFlow);
									ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
									return true;
								}
								else
								{
									if (coverFlow->focusIndex <= (count - 2))
									{
										coverFlow->frame = 0;

										if (COVERFLOW_DEBUG_FOCUSINDEX)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex + 1, __LINE__);

										coverFlow->focusIndex++;
									}
								}
							}


							//if ((coverFlow->slideMaxCount > 0) && (widget->flags & ITU_DRAGGABLE))//(coverFlow->boundaryAlign)
							//fix for non-draggable will make animation reverse
							if ((coverFlow->slideMaxCount > 0) || (coverFlow->prevnext_trigger))
							{
								coverFlow->prevnext_trigger = 0;
								coverFlow->coverFlowFlags |= ITU_COVERFLOW_SLIDING;
								coverFlow->touchCount = 0;
							}

							if (widget->flags & ITU_DRAGGING)
							{
								widget->flags &= ~ITU_DRAGGING;
								ituScene->dragged = NULL;
								coverFlow->inc = 0;
							}

							if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE) ||
								(coverFlow->focusIndex < count - 1) || boundary_touch)
							{
								if (count > 0)
								{
									if (widget->flags & ITU_DRAGGING)
									{
										widget->flags &= ~ITU_DRAGGING;
										ituScene->dragged = NULL;
										coverFlow->inc = 0;
									}

									//if (boundary_touch)
									//	coverFlow->focusIndex += ((coverFlow->focusIndex < (count - 2)) ? (1) : (0));

									

									if (coverFlow->inc == 0)
										coverFlow->inc = 0 - base_size;

									if (boundary_touch)
									{
										ITUWidget* cf = CoverFlowGetVisibleChild(coverFlow, count - 1);
										if ((cf->rect.y + cf->rect.height) <= widget_size)
										{
											coverFlow->inc = -1;
											coverFlow->frame = coverFlow->totalframe - 1;


											if ((boundary_touch) && (coverFlow->focusIndex > 0))
											{
												if (COVERFLOW_DEBUG_FOCUSINDEX)
													printf("[Debug][CoverFlow][FC %d ", coverFlow->focusIndex);

												coverFlow->focusIndex = CoverFlowGetVisibleChildCount(coverFlow) - widget_size / base_size;

												if (COVERFLOW_DEBUG_FOCUSINDEX)
													printf("to %d][%d]\n", coverFlow->focusIndex, __LINE__);
											}

											if (boundary_touch_bottom && !(widget->flags & ITU_DRAGGING) && coverFlow->bounceRatio > 0)
											{
												coverFlow->inc = 0 - (base_size / coverFlow->bounceRatio);

												if (COVERFLOW_DEBUG_FOCUSINDEX)
													printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex + 1, __LINE__);

												coverFlow->focusIndex++;

												widget->flags |= ITU_BOUNCING;
												coverFlow->frame = 0;
											}
										}
									}
								}
							}
							else if (coverFlow->focusIndex >= count - 1)
							{//maybe useless now
								//try to fix the ScaleCoverFlow side effect for non-cycle mode
								if ((!(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)) && (coverFlow->boundaryAlign == 0))
								{
									coverFlow->inc = 0;
								}
								else if (count > 0 && !(widget->flags & ITU_DRAGGING) && coverFlow->bounceRatio > 0)
								{
									if (coverFlow->inc == 0)
										coverFlow->inc = 0 - (base_size / coverFlow->bounceRatio);

									widget->flags |= ITU_BOUNCING;
									//coverFlow->frame = 1;
								}
							}
						}
						else // if (ev == ITU_EVENT_TOUCHSLIDEDOWN)
						{
							if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING) && (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)))
							{
								if (coverFlow->inc < 0)
								{
									if (coverFlow->totalframe != coverFlow->org_totalframe)
									{
										coverFlow->frame /= 2;
										coverFlow->totalframe = coverFlow->org_totalframe;
										printf("[mark totalframe %d]\n", coverFlow->totalframe);
									}
									coverFlow->frame = coverFlow->totalframe - coverFlow->frame;
									coverFlow->inc *= -1;
									if (coverFlow->focusIndex < (count - 1))
									{
										if (COVERFLOW_DEBUG_FOCUSINDEX)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex + 1, __LINE__);

										coverFlow->focusIndex++;
									}
									return true;
								}
								else if (coverFlow->inc > 0)
								{
									if (coverFlow->totalframe != coverFlow->org_totalframe)
									{
										coverFlow->frame /= 2;
										coverFlow->totalframe = coverFlow->org_totalframe;
										printf("[mark totalframe %d]\n", coverFlow->totalframe);
									}
									if (coverFlow->focusIndex > 1)
									{
										coverFlow->frame = 0;

										if (COVERFLOW_DEBUG_FOCUSINDEX)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - 1, __LINE__);

										coverFlow->focusIndex--;
									}
									else
									{
										coverFlow->frame = coverFlow->totalframe;

										if (COVERFLOW_DEBUG_FOCUSINDEX)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - 1, __LINE__);

										coverFlow->focusIndex--;
									}
									ituCoverFlowFixFC(coverFlow);
									ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
									return true;
								}
								else
								{
									if (coverFlow->focusIndex >= 1)
									{
										coverFlow->frame = 0;

										if (COVERFLOW_DEBUG_FOCUSINDEX)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - 1, __LINE__);

										coverFlow->focusIndex--;
									}
								}
							}

							//if ((coverFlow->slideMaxCount > 0) && (widget->flags & ITU_DRAGGABLE))//(coverFlow->boundaryAlign)
							//fix for non-draggable will make animation reverse
							if ((coverFlow->slideMaxCount > 0) || (coverFlow->prevnext_trigger))
							{
								coverFlow->prevnext_trigger = 0;
								coverFlow->coverFlowFlags |= ITU_COVERFLOW_SLIDING;
								coverFlow->touchCount = 0;
							}

							if (widget->flags & ITU_DRAGGING)
							{
								widget->flags &= ~ITU_DRAGGING;
								ituScene->dragged = NULL;
								coverFlow->inc = 0;
							}

							if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE) ||
								(coverFlow->focusIndex > 0) || boundary_touch)
							{
								if (count > 0)
								{
									if (widget->flags & ITU_DRAGGING)
									{
										widget->flags &= ~ITU_DRAGGING;
										ituScene->dragged = NULL;
										coverFlow->inc = 0;
									}

									//if (boundary_touch)
									//	coverFlow->focusIndex -= ((coverFlow->focusIndex > 1) ? (1) : (0));
									if (boundary_touch)
									{
										if ((COVERFLOW_DEBUG_FOCUSINDEX) && (coverFlow->focusIndex > 1))
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - 2, __LINE__);

										coverFlow->focusIndex -= ((coverFlow->focusIndex > 1) ? (2) : (0));
									}

									

									if (coverFlow->inc == 0)
										coverFlow->inc = base_size;

									if (boundary_touch)
									{
										ITUWidget* cf = CoverFlowGetVisibleChild(coverFlow, count - 1);
										//if ((cf->rect.y + cf->rect.height) <= widget_size)
										//{
										//	coverFlow->focusIndex = CoverFlowGetVisibleChildCount(coverFlow) - widget_size / base_size;
										//}

										cf = CoverFlowGetVisibleChild(coverFlow, 0);
										if ((cf->rect.y) >= 0)
										{
											coverFlow->inc = +1;
											coverFlow->frame = coverFlow->totalframe - 1;
										}

										if (boundary_touch_top && !(widget->flags & ITU_DRAGGING) && coverFlow->bounceRatio > 0)
										{
											coverFlow->inc = (base_size / coverFlow->bounceRatio);

											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex + 1, __LINE__);

											coverFlow->focusIndex += 1;
											widget->flags |= ITU_BOUNCING;
											coverFlow->frame = 0;
										}
									}
									//coverFlow->frame = 1;
								}
							}
							else if (coverFlow->focusIndex <= 0)
							{//maybe useless now
								//try to fix the ScaleCoverFlow side effect for non-cycle mode
								if ((!(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)) && (coverFlow->boundaryAlign == 0))
								{
									coverFlow->inc = 0;
								}
								else if (count > 0 && !(widget->flags & ITU_DRAGGING) && coverFlow->bounceRatio > 0)
								{
									if (coverFlow->inc == 0)
										coverFlow->inc = (base_size / coverFlow->bounceRatio);

									widget->flags |= ITU_BOUNCING;
									//coverFlow->frame = 1;
								}
							}
						}
						result = true;
					}
                }
            }

			if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
				coverFlow->frame = 0;
        }
        else if (ev == ITU_EVENT_MOUSEMOVE)
        {
			if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGING))
            {
                int x = arg2 - widget->rect.x;
                int y = arg3 - widget->rect.y;

				if ((ituWidgetIsInside(widget, x, y)) && (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)))
                {
                    int i, dist, offset, count = CoverFlowGetVisibleChildCount(coverFlow);
					int center_align_pos1;
					int center_align_pos2;

					if (coverFlow->boundaryAlign)
					{
						center_align_pos1 = 0;
						center_align_pos2 = widget_size;
					}
					else
					{
						center_align_pos1 = ((widget_size - base_size) / 2);
						center_align_pos2 = center_align_pos1 + base_size;
					}

					//printf("[coverflow][dragging][%d, %d]\n", x, y);
                    
					if (count > 0)
					{
						if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
						{
							dist = y - coverFlow->touchPos;
						}
						else
						{
							dist = x - coverFlow->touchPos;
						}

						//printf("[MOVE]dist %d touchPos %d\n", dist, coverFlow->touchPos);

						if ((!(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)) && (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP)))
						{
							if (coverFlow->bounceRatio > 0)
							{//the case when drag touch the boundary and move back away
								int min_item_boundary = 0;
								int max_item_boundary = 0;
								int min_item_diffvalue = 0;
								int max_item_diffvalue = 0;
								bool b_touch_min = false;
								bool b_touch_max = false;
								bool print_check = false;
								
								if (1)//(coverFlow->boundaryAlign)
								{

									min_item_boundary = coverFlow->boundary_memo1 + dist;
									max_item_boundary = coverFlow->boundary_memo2 + dist;

									min_item_diffvalue = min_item_boundary - (base_size / coverFlow->bounceRatio) - center_align_pos1;
									max_item_diffvalue = (max_item_boundary - center_align_pos2) + (base_size / coverFlow->bounceRatio);

									if (min_item_boundary >= ((base_size / coverFlow->bounceRatio) + center_align_pos1))
									{
										b_touch_min = true;
									}
									else if (max_item_boundary <= (center_align_pos2 - (base_size / coverFlow->bounceRatio)))
									{
										b_touch_max = true;
									}

								}

								//printf("memo %d toupos %d\n", coverFlow->boundary_touch_memo, coverFlow->touchPos);

								if (coverFlow->boundary_touch_memo == 0)
								{
									if (b_touch_min)
									{
										coverFlow->boundary_touch_memo = min_item_diffvalue;
										if (print_check)
											printf("active1...%d\n", coverFlow->boundary_touch_memo);
									}
									else if (b_touch_max)
									{
										coverFlow->boundary_touch_memo = max_item_diffvalue;
										if (print_check)
											printf("active2...%d\n", coverFlow->boundary_touch_memo);
									}
								}
								else if ((min_item_diffvalue > coverFlow->boundary_touch_memo) && (coverFlow->boundary_touch_memo > 0))
								{
									coverFlow->boundary_touch_memo = min_item_diffvalue;
									if (print_check)
										printf("active3...%d\n", coverFlow->boundary_touch_memo);
								}
								else if ((max_item_diffvalue < coverFlow->boundary_touch_memo) && (coverFlow->boundary_touch_memo < 0))
								{
									coverFlow->boundary_touch_memo = max_item_diffvalue;
									if (print_check)
										printf("active4...%d\n", coverFlow->boundary_touch_memo);
								}
								else
								{
									if ((max_item_diffvalue > coverFlow->boundary_touch_memo) && (coverFlow->boundary_touch_memo < 0))
									{
										if (min_item_diffvalue <= 0)
										{
											dist += coverFlow->boundary_touch_memo * -1;
											if (print_check)
												printf("dist5....%d\n", dist);
										}
										else
										{
											coverFlow->touchPos += coverFlow->boundary_touch_memo;
											if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
												dist = y - coverFlow->touchPos;
											else
												dist = x - coverFlow->touchPos;
											coverFlow->boundary_touch_memo = 0;
											if (print_check)
												printf("dist6....%d\n", dist);
										}
									}
									else if ((min_item_diffvalue < coverFlow->boundary_touch_memo) && (coverFlow->boundary_touch_memo > 0))
									{
										if (min_item_diffvalue >= 0)
										{
											dist -= coverFlow->boundary_touch_memo;
											if (print_check)
												printf("dist7....%d\n", dist);
										}
										else
										{

											coverFlow->touchPos += coverFlow->boundary_touch_memo;
											if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
												dist = y - coverFlow->touchPos;
											else
												dist = x - coverFlow->touchPos;
											coverFlow->boundary_touch_memo = 0;
											if (print_check)
												printf("dist8....%d\n", dist);
										}
									}
								}

								/*if (coverFlow->boundary_touch_memo)
								{
									ITUWidget* child_1 = CoverFlowGetVisibleChild(coverFlow, 0);
									ITUWidget* child_2 = CoverFlowGetVisibleChild(coverFlow, count - 1);

									if ((child_1->rect.x + dist) >(base_size / coverFlow->bounceRatio))
									{
										b_touch = true;
										if (x > coverFlow->boundary_touch_memo)
											coverFlow->boundary_touch_memo = x;
										else
											coverFlow->touchPos = coverFlow->boundary_touch_memo;
									}
									else if ((child_2->rect.x + base_size + dist) < (widget->rect.width - (base_size / coverFlow->bounceRatio)))
									{
										b_touch = true;
										if (coverFlow->boundary_touch_memo == 0)
											coverFlow->boundary_touch_memo = x;
										else
										{
											if (x < coverFlow->boundary_touch_memo)
												coverFlow->boundary_touch_memo = x;
											else
												coverFlow->touchPos = coverFlow->boundary_touch_memo;
										}
									}
								}*/
								///////////////
							}
						}

						//if (dist < 0)
						//	dist = -dist;
						//printf("Fc %d BB %d\n", coverFlow->focusIndex, coverFlow->boundary_touch_memo);

						if ((abs(dist) >= ITU_DRAG_DISTANCE) || coverFlow->boundary_touch_memo)
						{
							ituUnPressWidget(widget);
							ituWidgetUpdate(widget, ITU_EVENT_DRAGGING, 0, 0, 0);
						}

						if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
						{
							int dist_spec;

							if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
								dist_spec = y - coverFlow->init_drag_xy;
							else
								dist_spec = x - coverFlow->init_drag_xy;

							if (abs(dist_spec) >= ITU_DRAG_DISTANCE)
							{
								ituUnPressWidget(widget);
								//ituWidgetUpdate(widget, ITU_EVENT_DRAGGING, 0, 0, 0);
							}
							//printf("dist_spec is %d, y %d, pos %d\n", dist_spec, y, Arr[0]);
						}

						////////////////////////////
						// ITU_EVENT_MOUSEMOVE
						// Vertical Mode
						////////////////////////////
						if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
						{
							//offset = y - coverFlow->touchPos;
							offset = dist;
							//printf("0: offset=%d\n", offset);
							if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
							{
								if (abs(offset) < (base_size / coverFlow->min_change_dist_factor))
								{
									if (coverFlow->temp1 == 0)
										coverFlow->temp1 = 1;
								}
								else
									coverFlow->temp1 = -1;

								if (offset >= base_size)
								{
									if (COVERFLOW_DEBUG_FOCUSINDEX)
										printf("[Debug][CoverFlow][FC %d ", coverFlow->focusIndex);

									if (coverFlow->focusIndex > 0)
										coverFlow->focusIndex--;
									else
										coverFlow->focusIndex = count - 1;

									if (COVERFLOW_DEBUG_FOCUSINDEX)
										printf("to %d][%d]\n", coverFlow->focusIndex, __LINE__);

									offset -= base_size;
									coverFlow->touchPos += base_size;
									coverFlow->temp1 = -1;
								}

								if (offset <= (base_size * -1))
								{
									if (COVERFLOW_DEBUG_FOCUSINDEX)
										printf("[Debug][CoverFlow][FC %d ", coverFlow->focusIndex);

									if (coverFlow->focusIndex < (count - 1))
										coverFlow->focusIndex++;
									else
										coverFlow->focusIndex = 0;

									if (COVERFLOW_DEBUG_FOCUSINDEX)
										printf("to %d][%d]\n", coverFlow->focusIndex, __LINE__);

									offset += base_size;
									coverFlow->touchPos -= base_size;
									coverFlow->temp1 = -1;
								}
							}

							if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
							{
								int index, count2;

								count2 = count / 2 + 1;
								index = coverFlow->focusIndex;

								for (i = 0; i < count2; ++i)
								{
									ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
									int fy = widget->rect.height / 2 - child->rect.height / 2;
									fy += i * child->rect.height;
									fy += offset;

									ituCoverFlowSetXY(coverFlow, index, fy, __LINE__);

									if (index >= count - 1)
										index = 0;
									else
										index++;
								}

								count2 = count - count2;
								for (i = 0; i < count2; ++i)
								{
									ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
									int fy = widget->rect.height / 2 - child->rect.height / 2;
									fy -= count2 * child->rect.height;
									fy += i * child->rect.height;
									fy += offset;

									ituCoverFlowSetXY(coverFlow, index, fy, __LINE__);

									if (index >= count - 1)
										index = 0;
									else
										index++;
								}
							}
							else
							{
								//limit the move under non-cycle/Vertical boundaryAlign mode
								int fy = 0;
								bool b_touch = false;

								if ((coverFlow->boundaryAlign) && (coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP))
								{
									ITUWidget* child_1 = CoverFlowGetVisibleChild(coverFlow, 0);
									ITUWidget* child_2 = CoverFlowGetVisibleChild(coverFlow, count - 1);

									if ((child_1->rect.y + offset) > (base_size / coverFlow->bounceRatio))
									{
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE1;
										b_touch = true;
									}
									else if ((child_2->rect.y + base_size + offset) < (widget->rect.height - (base_size / coverFlow->bounceRatio)))
									{
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE2;
										b_touch = true;
									}
								}
								/*else if (coverFlow->boundaryAlign)
								{
									ITUWidget* child_1 = CoverFlowGetVisibleChild(coverFlow, 0);
									ITUWidget* child_2 = CoverFlowGetVisibleChild(coverFlow, count - 1);
									int child_height = child_1->rect.height;

									if ((child_1->rect.y + coverFlow->overlapsize) > 0)
									{
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE1;
										b_touch = true;
									}
									else if ((child_2->rect.y + coverFlow->overlapsize + base_size) < widget->rect.height)
									{
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE2;
										b_touch = true;
									}
								}*/

								if (count == 1) //for special one item case
								{
									if (coverFlow->bounceRatio > 0)
									{
										if (offset >= base_size / coverFlow->bounceRatio)
											offset = base_size / coverFlow->bounceRatio;
										else if (offset <= -1 * base_size / coverFlow->bounceRatio)
											offset = -1 * base_size / coverFlow->bounceRatio;
									}
								}
								else if ((coverFlow->boundary_memo1 + offset) >= ((base_size / coverFlow->bounceRatio) + center_align_pos1))
								{
									offset = ((base_size / coverFlow->bounceRatio) + center_align_pos1) - coverFlow->boundary_memo1;
								}
								else if ((coverFlow->boundary_memo2 + offset) <= (center_align_pos2 - (base_size / coverFlow->bounceRatio)))
								{
									offset = ((center_align_pos2 - (base_size / coverFlow->bounceRatio)) - coverFlow->boundary_memo2);
								}
								/*else if (coverFlow->focusIndex <= 0)
								{
									if (coverFlow->bounceRatio > 0)
									{
										if (offset >= base_size / coverFlow->bounceRatio)
											offset = base_size / coverFlow->bounceRatio;
									}
									else
									{
										offset = 0;
									}
								}
								else if (coverFlow->focusIndex >= count - 1)
								{
									if (coverFlow->bounceRatio > 0)
									{
										if (offset <= -base_size / coverFlow->bounceRatio)
											offset = -base_size / coverFlow->bounceRatio;
									}
									else
									{
										offset = 0;
									}
								}*/

								if (coverFlow->bounceRatio == 0)
									offset = 0;

								for (i = 0; i < count; ++i)
								{//[MOVE][Vertical][non-cycle][layout]
									ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
									//int fy = widget->rect.height / 2 - child->rect.height / 2;

									if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
									{
										if (i == 0)
										{
											ITUWidget* child_1 = CoverFlowGetVisibleChild(coverFlow, 0);
											ITUWidget* child_2 = CoverFlowGetVisibleChild(coverFlow, count - 1);
											int local_touchpos = 0;
											fy = child_1->rect.y + offset;

											if (b_touch)
											{
												if ((child_1->rect.y + offset) > (base_size / coverFlow->bounceRatio))
												{
													fy = base_size / coverFlow->bounceRatio;
													local_touchpos = 0;
												}
												else if ((child_2->rect.y + base_size + offset) < (widget->rect.height - (base_size / coverFlow->bounceRatio)))
												{
													fy = (widget->rect.height - (base_size / coverFlow->bounceRatio)) - count * base_size;
													local_touchpos = widget_size;
												}
											}

											if (!b_touch)
												coverFlow->touchPos = y;
											else
												coverFlow->touchPos = local_touchpos; //coverFlow->boundary_touch_memo;
										}
									}
									else
									{
										fy = widget->rect.height / 2 - base_size / 2;
									}

									if (coverFlow->boundaryAlign && b_touch)
									{
										if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
										{
											//widget->flags |= ITU_UNDRAGGING;
											//ituScene->dragged = NULL;
											coverFlow->boundary_touch_memo = y;

											if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE1) && (offset > 0))
											{
												break;
											}
											else if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE2) && (offset < 0))
											{
												break;
											}
										}
										else
											break;
									}

									if (!((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP)))
									{
										if (coverFlow->boundaryAlign)
										{
											int max_neighbor_item = ((widget->rect.height / base_size) - 1) / 2;
											int max_height_item = widget->rect.height / base_size;
											fy = 0;

											if (max_neighbor_item == 0)
												max_neighbor_item++;

											if (coverFlow->focusIndex > 0)//>= max_neighbor_item) //Bless debug now
											{
												//if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_neighbor_item))
												if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_height_item))
													fy = widget->rect.height - (count * base_size);
												else
													fy -= base_size * coverFlow->focusIndex;
											}
											else
												fy = 0;
										}
										else
										{
											fy -= base_size * coverFlow->focusIndex;
										}
									}

									if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
									{
										//fy += offset;
										ituCoverFlowSetXY(coverFlow, i, fy + (i * child->rect.height), __LINE__);
									}
									else
									{
										if (coverFlow->overlapsize > 0)
										{
											fy += i * base_size;
											fy += offset;
											ituCoverFlowSetXY(coverFlow, i, fy - coverFlow->overlapsize, __LINE__);
										}
										else
										{
											fy += i * child->rect.height;
											fy += offset;
											ituCoverFlowSetXY(coverFlow, i, fy, __LINE__);
										}
									}

									if (i == 0)
									{
										if (coverFlow->overlapsize > 0)
											coverFlow->movelog = fy - coverFlow->overlapsize;
										else
											coverFlow->movelog = fy;
									}
								}
							}
						}
						////////////////////////////
						// ITU_EVENT_MOUSEMOVE
						// Horizontal Mode
						////////////////////////////
						else
						{
							//offset = x - coverFlow->touchPos;
							offset = dist;

							//printf("0: offset=%d\n", offset);
							if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
							{
								if (abs(offset) < (base_size / coverFlow->min_change_dist_factor))
								{
									if (coverFlow->temp1 == 0)
										coverFlow->temp1 = 1;
								}
								else
									coverFlow->temp1 = -1;

								if (offset >= base_size)
								{
									if (COVERFLOW_DEBUG_FOCUSINDEX)
										printf("[Debug][CoverFlow][FC %d ", coverFlow->focusIndex);

									if (coverFlow->focusIndex > 0)
										coverFlow->focusIndex--;
									else
										coverFlow->focusIndex = count - 1;

									if (COVERFLOW_DEBUG_FOCUSINDEX)
										printf("to %d][%d]\n", coverFlow->focusIndex, __LINE__);

									offset -= base_size;
									coverFlow->touchPos += base_size;
									coverFlow->temp1 = -1;
								}

								if (offset <= (base_size * -1))
								{
									if (COVERFLOW_DEBUG_FOCUSINDEX)
										printf("[Debug][CoverFlow][FC %d ", coverFlow->focusIndex);

									if (coverFlow->focusIndex < (count - 1))
										coverFlow->focusIndex++;
									else
										coverFlow->focusIndex = 0;

									if (COVERFLOW_DEBUG_FOCUSINDEX)
										printf("to %d][%d]\n", coverFlow->focusIndex, __LINE__);

									offset += base_size;
									coverFlow->touchPos -= base_size;
									coverFlow->temp1 = -1;
								}
							}

							if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
							{
								int index, count2;
								//workaround for wrong left-side display with hide child
								count2 = count / 2 + 1 - ((offset > 0) ? (1) : (0));
								//count2 = count / 2 + 1;
								index = coverFlow->focusIndex;

								for (i = 0; i < count2; ++i)
								{
									ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
									int fx = widget->rect.width / 2 - child->rect.width / 2;
									fx += i * child->rect.width;
									fx += offset;

									ituCoverFlowSetXY(coverFlow, index, fx, __LINE__);

									if (index >= count - 1)
										index = 0;
									else
										index++;
								}

								count2 = count - count2;
								for (i = 0; i < count2; ++i)
								{
									ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
									int fx = widget->rect.width / 2 - child->rect.width / 2;
									fx -= count2 * child->rect.width;
									fx += i * child->rect.width;
									fx += offset;

									ituCoverFlowSetXY(coverFlow, index, fx, __LINE__);

									if (index >= count - 1)
										index = 0;
									else
										index++;
								}
							}
							else
							{
								//limit the move under non-cycle/Horizontal boundaryAlign mode
								int fx = 0;
								int boundary_over = 0;
								bool b_touch = false;

								if ((coverFlow->boundaryAlign) && (coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP))
								{
									ITUWidget* child_1 = CoverFlowGetVisibleChild(coverFlow, 0);
									ITUWidget* child_2 = CoverFlowGetVisibleChild(coverFlow, count - 1);

									if ((child_1->rect.x + offset) > (base_size / coverFlow->bounceRatio))
									{
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE1;
										b_touch = true;
									}
									else if ((child_2->rect.x + base_size + offset) < (widget->rect.width - (base_size / coverFlow->bounceRatio)))
									{
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE2;
										b_touch = true;
									}
								}
								/*else if (coverFlow->boundaryAlign)
								{
									ITUWidget* child_1 = CoverFlowGetVisibleChild(coverFlow, 0);
									ITUWidget* child_2 = CoverFlowGetVisibleChild(coverFlow, count - 1);

									if ((child_1->rect.x + coverFlow->overlapsize) > 0)
									{
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE1;
										b_touch = true;
									}
									else if ((child_2->rect.x + coverFlow->overlapsize + base_size) < widget->rect.width)
									{
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE2;
										b_touch = true;
									}
								}*/

								if (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP))
								{
									if (count == 1) //for special one item case
									{
										if (coverFlow->bounceRatio > 0)
										{
											if (offset >= base_size / coverFlow->bounceRatio)
												offset = base_size / coverFlow->bounceRatio;
											else if (offset <= -1 * base_size / coverFlow->bounceRatio)
												offset = -1 * base_size / coverFlow->bounceRatio;
										}
									}
									else if ((coverFlow->boundary_memo1 + offset) >= ((base_size / coverFlow->bounceRatio) + center_align_pos1))
									{
										offset = ((base_size / coverFlow->bounceRatio) + center_align_pos1) - coverFlow->boundary_memo1;
									}
									else if ((coverFlow->boundary_memo2 + offset) <= (center_align_pos2 - (base_size / coverFlow->bounceRatio)))
									{
										offset = ((center_align_pos2 - (base_size / coverFlow->bounceRatio)) - coverFlow->boundary_memo2);
									}
									/*else if (coverFlow->focusIndex <= 0)
									{
										if (coverFlow->bounceRatio > 0)
										{
											if (offset >= base_size / coverFlow->bounceRatio)
												offset = base_size / coverFlow->bounceRatio;
										}
										else
										{
											offset = 0;
										}
									}
									else if (coverFlow->focusIndex >= count - 1)
									{
										ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, 0);
										if (coverFlow->bounceRatio > 0)
										{
											if (offset <= -base_size / coverFlow->bounceRatio)
												offset = -base_size / coverFlow->bounceRatio;
										}
										else
										{
											offset = 0;
										}
									}*/
								}

								if (coverFlow->bounceRatio == 0)
									offset = 0;

								for (i = 0; i < count; ++i)
								{
									ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
									//int fx = widget->rect.width / 2 - child->rect.width / 2;

									if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
									{
										if (i == 0)
										{
											ITUWidget* child_1 = CoverFlowGetVisibleChild(coverFlow, 0);
											ITUWidget* child_2 = CoverFlowGetVisibleChild(coverFlow, count - 1);
											int local_touchpos = 0;
											fx = child_1->rect.x + offset;

											if (b_touch)
											{
												if ((child_1->rect.x + offset) > (base_size / coverFlow->bounceRatio))
												{
													fx = base_size / coverFlow->bounceRatio;
													local_touchpos = 0;
												}
												else if ((child_2->rect.x + base_size + offset) < (widget->rect.width - (base_size / coverFlow->bounceRatio)))
												{
													fx = (widget->rect.width - (base_size / coverFlow->bounceRatio)) - count * base_size;
													local_touchpos = widget_size;
												}
											}

											if (!b_touch)
											{
												coverFlow->touchPos = x;
											}
											else
											{
												coverFlow->touchPos = local_touchpos; //coverFlow->boundary_touch_memo;
											}
										}
									}
									else
									{
										fx = widget->rect.width / 2 - base_size / 2;
									}

									if (coverFlow->boundaryAlign && b_touch)
									{
										if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
										{
											//widget->flags |= ITU_UNDRAGGING;
											//ituScene->dragged = NULL;
											coverFlow->boundary_touch_memo = x;

											if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE1) && (offset > 0))
											{
												break;
											}
											else if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE2) && (offset < 0))
											{
												break;
											}
										}
										else
											break;
									}

									if (!((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP)))
									{
										if (coverFlow->boundaryAlign)
										{//[MOVE][Horizontal][non-cycle][layout]
											int max_neighbor_item = ((widget->rect.width / base_size) - 1) / 2;
											int max_width_item = widget->rect.width / base_size;
											fx = 0;

											if (max_neighbor_item == 0)
												max_neighbor_item++;



											if (coverFlow->focusIndex > 0) //>= max_neighbor_item) //Bless debug now
											{
												//if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_neighbor_item))
												if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_width_item))
													fx = widget->rect.width - (count * base_size);
												else
													fx -= base_size * coverFlow->focusIndex;
											}
											else
												fx = 0;
										}
										else
										{
											fx -= base_size * coverFlow->focusIndex;
										}
									}

									if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
									{
										ituCoverFlowSetXY(coverFlow, i, fx + (i * child->rect.width), __LINE__);
									}
									else
									{
										//for powei
										if (coverFlow->overlapsize > 0)
										{
											fx += i * base_size;
											fx += offset;
											ituCoverFlowSetXY(coverFlow, i, fx - coverFlow->overlapsize, __LINE__);
										}
										else
										{
											fx += i * base_size;
											fx += offset;

											//boundary check again
											if ((i == 0) && (coverFlow->boundaryAlign))
											{
												int base_bounce = ((coverFlow->bounceRatio <= 0) ? (1) : (base_size / coverFlow->bounceRatio));
												int start_check_pos = fx;
												int end_check_pos = fx + (count * base_size);

												if (start_check_pos > base_bounce)
												{
													boundary_over = base_bounce - start_check_pos;
													printf("[Boundary over][bypass][line %d]\n", __LINE__);
												}
												else if (end_check_pos < (widget_size - base_bounce))
												{
													boundary_over = (widget_size - base_bounce) - end_check_pos;
													printf("[Boundary over][bypass][line %d]\n", __LINE__);
												}
											}

											if (boundary_over)
											{
												fx += boundary_over;
											}

											ituCoverFlowSetXY(coverFlow, i, fx, __LINE__);
										}

										if (0)//(i == (count - 1))
										{
											if ((coverFlow->focusIndex == 0) && (offset > 0))
											{
												coverFlow->boundary_touch_memo = x;
												//coverFlow->mousedown_position = x;
												//proArr[COVERFLOW_MAX_PROCARR_SIZE + 1] = x;
												//mousedown_position = x;
												//coverFlow->touchPos = x;
											}
											else if ((coverFlow->focusIndex == (count - 1)) && (offset < 0))
											{
												coverFlow->boundary_touch_memo = x;
												//coverFlow->mousedown_position = x;
												//proArr[COVERFLOW_MAX_PROCARR_SIZE + 1] = x;
												//mousedown_position = x;
												//coverFlow->touchPos = x;
											}
											//ituWidgetSetCustomData(coverFlow, proArr);
										}

									}
									if (i == 0)
									{
										if (coverFlow->overlapsize > 0)
											coverFlow->movelog = fx - coverFlow->overlapsize;
										else
											coverFlow->movelog = fx;
									}
								}
							}
						}
						result = true;
						ituDirtyWidget(widget, result);
					}
                }
				else
				{
					ituUnPressWidget(widget);
					//ituWidgetUpdate(coverFlow, ITU_EVENT_MOUSEUP, arg1, arg2, arg3);
					if (!ituWidgetIsInside(widget, x, y))
					{
						if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING))
							ituWidgetUpdate(coverFlow, ITU_EVENT_MOUSEUP, arg1, arg2, arg3);
					}
					return true;
				}
            }
        }
        else if (ev == ITU_EVENT_MOUSEDOWN)
        {
			int x = arg2 - widget->rect.x;
			int y = arg3 - widget->rect.y;
			int count = CoverFlowGetVisibleChildCount(coverFlow);

			if (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
			{
				if (coverFlow->focusIndex < 0)
					coverFlow->focusIndex = 0;
				else if (coverFlow->focusIndex >(count - 1))
					coverFlow->focusIndex = count - 1;

				if (coverFlow->temp3)
					coverFlow->temp3 = 0;
			}

			coverFlow->clock = itpGetTickCount();
			coverFlow->temp1 = 0;

			//For memo multi touch down
			//coverFlow->temp2++;

			if (ituWidgetIsInside(widget, x, y))
			{
				if (ituScene->dragged != NULL)
				{
					ITCTree* tree = (ITCTree*)ituScene->dragged;
					ITUWidget* ww = (ITUWidget*)tree->parent;

					if (ww->type != ITU_TRACKBAR)
					{
						printf("other widget dragging, name %s\n", widget->name);
						ituUnPressWidget(widget);
						return true;
					}
					else
						return true;
				}
				//else
				//	printf("name %s\n", widget->name);

				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
				{
					ITUWidget* c1 = CoverFlowGetVisibleChild(coverFlow, 0);

					//side effect when use prev or next to slide but set slidemaxcount is 0
					if (coverFlow->slideMaxCount == 0)
						return true;

					coverFlow->frame = coverFlow->totalframe;
					//printf("sliding bypass action.\n");

					if (coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP)
					{
						if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
						{
							coverFlow->mousedown_position = c1->rect.y;
							//coverFlow->boundary_touch_memo = 0;
							coverFlow->touchPos = y;
						}
						else
						{
							coverFlow->mousedown_position = c1->rect.x;
							//coverFlow->boundary_touch_memo = 0;
							coverFlow->touchPos = x;

						}

						//coverFlow->inc = 0;
						//coverFlow->frame = 0;
						//CoverFlowCleanQueue(coverFlow);
						//widget->flags &= ~ITU_COVERFLOW_SLIDING;
						//widget->flags |= ITU_DRAGGING;
						//ituScene->dragged = widget;
						//return true;
					}
					else
					{
						if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
						{
							coverFlow->mousedown_position = c1->rect.y;
							coverFlow->boundary_touch_memo = 0;
							coverFlow->touchPos = y;
						}
						else
						{
							coverFlow->mousedown_position = c1->rect.x;
							coverFlow->boundary_touch_memo = 0;
							coverFlow->touchPos = x;

						}
					}

					if (count > 1)
					{
						if ((coverFlow->inc < 0) && (coverFlow->focusIndex < (count - 1)))
						{
							//coverFlow->focusIndex++;
							//coverFlow->inc = 0;
							coverFlow->frame = coverFlow->totalframe;
							ituCoverFlowFixFC(coverFlow);
							ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
						}
						else if ((coverFlow->inc > 0) && (coverFlow->focusIndex > 0))
						{
							//coverFlow->focusIndex--;
							//coverFlow->inc = 0;
							coverFlow->frame = coverFlow->totalframe;
							ituCoverFlowFixFC(coverFlow);
							ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
						}
					}

					CoverFlowCleanQueue(coverFlow);

					if (widget->flags & ITU_DRAGGABLE)
					{
						widget->flags |= ITU_DRAGGING;
						ituScene->dragged = widget;
					}
					return true;
				}
			}

			if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
			{
				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
				{
					coverFlow->init_drag_xy = y;
				}
				else
				{
					coverFlow->init_drag_xy = x;
				}

				coverFlow->frame = coverFlow->totalframe;
			}

			if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGABLE))
			{
				int count = CoverFlowGetVisibleChildCount(coverFlow);
				ITUWidget* c1 = CoverFlowGetVisibleChild(coverFlow, 0);
				ITUWidget* c2 = CoverFlowGetVisibleChild(coverFlow, count - 1);

				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
				{
					coverFlow->mousedown_position = c1->rect.y;
					coverFlow->boundary_memo1 = c1->rect.y;
					coverFlow->boundary_memo2 = c2->rect.y + base_size;
				}
				else
				{
					coverFlow->mousedown_position = c1->rect.x;
					coverFlow->boundary_memo1 = c1->rect.x;
					coverFlow->boundary_memo2 = c2->rect.x + base_size;
				}
			}

            if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGABLE) && coverFlow->bounceRatio > 0)
            {
                if (ituWidgetIsInside(widget, x, y))
                {
                    if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
                    {
						//if ((!(coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP)) || (coverFlow->frame == 0))
						coverFlow->touchPos = y;
                    }
                    else
                    {
						//if ((!(coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP)) || (coverFlow->frame == 0))
						coverFlow->touchPos = x;
                    }

                    if (widget->flags & ITU_HAS_LONG_PRESS)
                    {
                        coverFlow->touchCount = 1;
                    }
                    else
                    {
                        widget->flags |= ITU_DRAGGING;
                        ituScene->dragged = widget;
                    }
                    //result = true;
                }
            }
        }
        else if (ev == ITU_EVENT_MOUSEUP)
        {
			//For check multi touch down
			//if (coverFlow->temp2 > 0)
			//	coverFlow->temp2--;

			//if (coverFlow->temp2 > 0)
			//{
			//	ituUnPressWidget(widget);
			//	return true;
			//}

			if ((!(coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)) && coverFlow->boundaryAlign && (coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP))
			{ //fix the error position when drag too much outside or too fast then mouse up
				int count = CoverFlowGetVisibleChildCount(coverFlow);
				int i = 0;
				int fd = 0;
				int move_step = 0;
				ITUWidget* child_1 = CoverFlowGetVisibleChild(coverFlow, 0);
				ITUWidget* child_2 = CoverFlowGetVisibleChild(coverFlow, count - 1);

				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
				{
					if ((child_1->rect.y > 0) || ((child_2->rect.y + child_2->rect.height) < widget->rect.height))
					{
						if (child_1->rect.y > 0)
							move_step = 0 - child_1->rect.y;
						else
							move_step = widget->rect.height - (child_2->rect.y + child_2->rect.height);

						for (i = 0; i < count; i++)
						{
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
							fd = child->rect.y;
							fd += move_step;
							ituCoverFlowSetXY(coverFlow, i, fd, __LINE__);
						}

						coverFlow->frame = 0;
					}
				}
				else
				{
					if ((child_1->rect.x > 0) || ((child_2->rect.x + child_2->rect.width) < widget->rect.width))
					{
						if (child_1->rect.x > 0)
							move_step = 0 - child_1->rect.x;
						else
							move_step = widget->rect.width - (child_2->rect.x + child_2->rect.width);

						for (i = 0; i < count; i++)
						{
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
							fd = child->rect.x;
							fd += move_step;
							ituCoverFlowSetXY(coverFlow, i, fd, __LINE__);
						}

						coverFlow->frame = 0;
					}
				}
			}

			if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGABLE) && ((widget->flags & ITU_DRAGGING)) 
				&& (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP)))
			{
                int count = CoverFlowGetVisibleChildCount(coverFlow);
                int x = arg2 - widget->rect.x;
                int y = arg3 - widget->rect.y;

                result = false; //Bless debug

                if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
                {
                    if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
                    {
                        if (!result && count > 0)
                        {
                            ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, 0);

                            if (coverFlow->inc == 0)
                            {
                                int offset, absoffset, interval;
                                
                                offset = y - coverFlow->touchPos;
                                interval = offset / child->rect.height;
                                offset -= (interval * child->rect.height);
                                absoffset = offset > 0 ? offset : -offset;

                                if (absoffset > child->rect.height / 2)
                                {
									coverFlow->frame = (int)((float)coverFlow->totalframe * (float)absoffset / (float)child->rect.height);

									if (COVERFLOW_DEBUG_FOCUSINDEX && interval)
										printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - interval, __LINE__);

                                    coverFlow->focusIndex -= interval;

                                    if (offset >= 0)
                                    {
                                        //coverFlow->inc = child->rect.height;
										//coverFlow->focusIndex--;

										if (coverFlow->focusIndex < 0)
										{
											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex + count, __LINE__);

											coverFlow->focusIndex += count;
										}
										ituCoverFlowFixFC(coverFlow);
										ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
										LOG_DBG "1: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                    }
                                    else
                                    {
                                        //coverFlow->inc = -child->rect.height;
										//coverFlow->focusIndex++;

										if (coverFlow->focusIndex >= count)
										{
											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - count, __LINE__);

											coverFlow->focusIndex -= count;
										}
										ituCoverFlowFixFC(coverFlow);
										ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
										LOG_DBG "2: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                    }

									//try fix bug bbbbb
									coverFlow->inc = offset;
									/*while (coverFlow->inc > (base_size / 2))
									{
										coverFlow->inc -= base_size;
									}
									while (coverFlow->inc < (-1 * base_size / 2))
									{
										coverFlow->inc += base_size;
									}*/
                                }
                                else if (absoffset)
                                {
									//coverFlow->frame = coverFlow->totalframe - (int)((float)coverFlow->totalframe * (float)absoffset / (float)child->rect.height);
									//coverFlow->frame = coverFlow->totalframe - (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
									coverFlow->frame = (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);

                                    if (offset >= 0)
                                    {
                                        //coverFlow->inc = -child->rect.height;
										//coverFlow->focusIndex -= interval;// +1;

										if (coverFlow->focusIndex < 0)
										{
											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex + count, __LINE__);

											coverFlow->focusIndex += count;
										}

										LOG_DBG "3: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                    }
                                    else
                                    {
                                        //coverFlow->inc = child->rect.height;
										//coverFlow->focusIndex -= interval;// -1;

										if (coverFlow->focusIndex >= count)
										{
											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - count, __LINE__);

											coverFlow->focusIndex -= count;
										}

										LOG_DBG "4: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                    }

									if (coverFlow->temp1 == 1) //small dragging
									{
										coverFlow->inc = offset * -1;
										//coverFlow->frame = coverFlow->totalframe - (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
									}
									else
									{
										coverFlow->inc = offset;
									}
									//coverFlow->inc = offset;
                                }
                            }
                            widget->flags |= ITU_UNDRAGGING;
                            ituScene->dragged = NULL;
                        }
                    }
                    else
                    {
						//if (!result && count > 0)
						if ((count > 0) && (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)))
                        {//[MOUSEUP][Vertical][non-cycle][layout]
                            //ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, 0);
							bool boundary_touch = false;
							//[MOUSEUP][Vertical][non-cycle][layout]
							if (coverFlow->boundaryAlign)
							{
								int max_neighbor_item = ((widget->rect.height / base_size) - 1) / 2;
								int max_height_item = widget->rect.height / base_size;

								if (max_neighbor_item == 0)
									max_neighbor_item++;

								if (coverFlow->focusIndex >= max_neighbor_item)
								{
									if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_neighbor_item))
										boundary_touch = true;
									else
									{
										if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_height_item))
											boundary_touch = true;
									}
								}
								else
									boundary_touch = true;

								if (!boundary_touch) //re-check again for some special case that with bad max_neighbor_item
								{ //this case should debug for some extreme item count
									ITUWidget* child1 = CoverFlowGetVisibleChild(coverFlow, 0);
									ITUWidget* child2 = CoverFlowGetVisibleChild(coverFlow, count - 1);
									int min_item_pos, max_item_pos, min_boundary_value, max_boundary_value;

									if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
									{
										min_item_pos = child1->rect.y;
										max_item_pos = child2->rect.y + base_size;
									}
									else
									{
										min_item_pos = child1->rect.x;
										max_item_pos = child2->rect.x + base_size;
									}

									min_boundary_value = base_size / coverFlow->bounceRatio;
									max_boundary_value = widget_size - (base_size / coverFlow->bounceRatio);

									if (min_item_pos >= min_boundary_value)
									{
										boundary_touch = true;

										if (COVERFLOW_DEBUG_FOCUSINDEX)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, 0, __LINE__);

										coverFlow->focusIndex = 0;
									}
									else if (max_item_pos <= max_boundary_value)
									{
										boundary_touch = true;

										if (COVERFLOW_DEBUG_FOCUSINDEX)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, count - 1, __LINE__);

										coverFlow->focusIndex = count - 1;
									}
								}
							}

                            if (coverFlow->inc == 0)
                            {
                                int offset, absoffset, interval;
                                
                                //offset = y - coverFlow->touchPos;//PPAP
								offset = CoverFlowGetDraggingDist(coverFlow);
								interval = offset / base_size;
								offset -= (interval * base_size);
                                //absoffset = offset > 0 ? offset : -offset;

								//////////check bounce
								if (coverFlow->focusIndex <= 0)
								{
									if (coverFlow->bounceRatio > 0)
									{
										if (offset >= base_size / coverFlow->bounceRatio)
											offset = base_size / coverFlow->bounceRatio;

										if (offset == 0)
											offset = CoverFlowGetDraggingDist(coverFlow);
									}
									else
									{
										offset = 0;
									}
								}
								else if (coverFlow->focusIndex >= count - 1)
								{
									if (coverFlow->bounceRatio > 0)
									{
										if (offset <= -base_size / coverFlow->bounceRatio)
											offset = -base_size / coverFlow->bounceRatio;

										if (offset == 0)
											offset = CoverFlowGetDraggingDist(coverFlow);
									}
									else
									{
										offset = 0;
									}
								}
								absoffset = offset > 0 ? offset : -offset;

								if ((absoffset > (base_size / 2)) || (absoffset > (widget_size / coverFlow->min_change_dist_factor)))
                                {//small shift alignment
									if (offset >= 0)
									{
										//coverFlow->frame = (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
										//coverFlow->frame = coverFlow->totalframe - (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
										coverFlow->frame = 0;

										if (coverFlow->focusIndex > interval)
										{
											//fix the alignment wrong when mouseup at maximum focusindex then stop wrong layout when mouseup done.
											int mod = 1;
											if ((coverFlow->boundaryAlign) && (coverFlow->focusIndex == (count - 1)))
											{
												int max_touch_start_size_index = (count - (widget_size / base_size));
												mod = coverFlow->focusIndex - max_touch_start_size_index + 1;
											}

											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("[Debug][CoverFlow][FC %d ", coverFlow->focusIndex);

											coverFlow->focusIndex -= interval + mod;

											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("to %d][%d]\n", coverFlow->focusIndex, __LINE__);

											if (coverFlow->focusIndex < 0)
												coverFlow->focusIndex = 0;

											if (boundary_touch)
											{
												while ((CoverFlowCheckBoundaryTouch(widget) == ITU_BOUNCE_2) && (coverFlow->focusIndex > 0))
												{
													if (COVERFLOW_DEBUG_FOCUSINDEX)
														printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - 1, __LINE__);

													coverFlow->focusIndex--;
												}
											}
											//if (boundary_touch)
											//	coverFlow->focusIndex = CoverFlowGetVisibleChildCount(coverFlow) - widget->rect.height / base_size;
											//coverFlow->inc = base_size;
											//coverFlow->focusIndex -= interval + 1;
											ituCoverFlowFixFC(coverFlow);
											ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
											LOG_DBG "5: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
											//if (boundary_touch)
											//	coverFlow->focusIndex -= (interval != 0) ? (((offset >= 0) ? (1) : (-1))) : (((absoffset > child->rect.height / 2) ? (((offset >= 0) ? (1) : (-1))) : (0)));
										}
										else
										{
											//coverFlow->inc = -base_size;

											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, -1, __LINE__);

											coverFlow->focusIndex = -1;
											LOG_DBG "6: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
										}
									}
									else
									{
										//coverFlow->frame = (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
										//coverFlow->frame = coverFlow->totalframe - (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
										coverFlow->frame = 0;

										if (coverFlow->focusIndex < count + interval - 1)
										{
											//coverFlow->inc = -base_size;
											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("[Debug][CoverFlow][FC %d ", coverFlow->focusIndex);

											coverFlow->focusIndex -= interval - 1;

											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("to %d][%d]\n", coverFlow->focusIndex, __LINE__);

											if (coverFlow->focusIndex > (count - 1))
												coverFlow->focusIndex = count - 1;

											ituCoverFlowFixFC(coverFlow);
											ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
											LOG_DBG "7: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
											//if (boundary_touch)
											//	coverFlow->focusIndex -= (interval != 0) ? (((offset >= 0) ? (1) : (-1))) : (((absoffset > child->rect.height / 2) ? (((offset >= 0) ? (1) : (-1))) : (0)));
										}
										else
										{
											//coverFlow->inc = base_size;
											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, count, __LINE__);

											coverFlow->focusIndex = count;
											LOG_DBG "8: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
										}
									}
									//try fix bug bbbbb
									coverFlow->inc = offset * -1;
									/*while (coverFlow->inc > (base_size / 2))
									{
										coverFlow->inc -= base_size;
									}
									while (coverFlow->inc < (-1 * base_size / 2))
									{
										coverFlow->inc += base_size;
									}*/
                                }
								else if (absoffset)
								{
									//try debug 0911
									//coverFlow->frame = coverFlow->totalframe - (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
									coverFlow->frame = (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);

									if (offset >= 0)
									{//big shift alignment
										//if ((boundary_touch) && (coverFlow->focusIndex > 0))
										//	coverFlow->focusIndex = CoverFlowGetVisibleChildCount(coverFlow) - widget->rect.height / base_size;

										//coverFlow->inc = -base_size;
										int lastf = coverFlow->focusIndex;

										//fix the alignment wrong when mouseup at maximum focusindex then stop wrong layout when mouseup done.
										int mod = 0;
										if ((coverFlow->boundaryAlign) && (coverFlow->focusIndex == (count - 1)))
										{
											int max_touch_start_size_index = (count - (widget_size / base_size));
											mod = coverFlow->focusIndex - max_touch_start_size_index;
										}

										if (COVERFLOW_DEBUG_FOCUSINDEX && interval)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - interval - mod, __LINE__);

										coverFlow->focusIndex -= interval + mod;

										//if (boundary_touch)
										//	coverFlow->focusIndex -= (interval != 0) ? (((offset >= 0) ? (1) : (-1))) : (((absoffset > base_size / 2) ? (((offset >= 0) ? (1) : (-1))) : (0)));

										//small shift alignment
										if (boundary_touch)
										{
											while ((CoverFlowCheckBoundaryTouch(widget) == ITU_BOUNCE_2) && (coverFlow->focusIndex > 0))
											{
												if (COVERFLOW_DEBUG_FOCUSINDEX)
													printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - 1, __LINE__);

												coverFlow->focusIndex--;
											}
										}

										//if (coverFlow->focusIndex < -1)
										//	coverFlow->focusIndex = -1;
										if (coverFlow->focusIndex < 0)
											coverFlow->focusIndex = 0;

										if (lastf != coverFlow->focusIndex)
										{
											ituCoverFlowFixFC(coverFlow);
											ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
										}
										LOG_DBG "9: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
									}
									else
									{
										//coverFlow->inc = base_size;
										int lastf = coverFlow->focusIndex;

										if (COVERFLOW_DEBUG_FOCUSINDEX && interval)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - interval, __LINE__);

										coverFlow->focusIndex -= interval;

										if (0)//(boundary_touch)
										{
											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("[Debug][CoverFlow][FC %d ", coverFlow->focusIndex);

											coverFlow->focusIndex -= (interval != 0) ? (((offset >= 0) ? (1) : (-1))) : (((absoffset > base_size / 2) ? (((offset >= 0) ? (1) : (-1))) : (0)));

											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("to %d][%d]\n", coverFlow->focusIndex, __LINE__);
										}

										if (coverFlow->focusIndex >= count + 1)
											coverFlow->focusIndex = count;

										if (lastf != coverFlow->focusIndex)
										{
											ituCoverFlowFixFC(coverFlow);
											ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
										}
										LOG_DBG "10: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
									}
									coverFlow->inc = offset;
								}
                            }
                        }

						if (widget)
						{
							ITUWidget* c1 = CoverFlowGetVisibleChild(coverFlow, 0);
							ITUWidget* c2 = CoverFlowGetVisibleChild(coverFlow, count - 1);
							int loop = 0;
							int pos_center = (widget_size / 2) - (base_size / 2);

							if (coverFlow->inc)
							{
								if (((coverFlow->focusIndex <= 0) && (c1->rect.y == pos_center)) || ((coverFlow->focusIndex >= (count - 1)) && (c2->rect.y == pos_center)))
								{
									ituScene->dragged = NULL;
									coverFlow->frame = 0;
									coverFlow->inc = 0;
									for (loop = 0; loop < 5; loop++)
									{
										printf("[MouseUP][check fail][Y]\n");
									}
								}
							}
						}
						else
						{
							widget->flags |= ITU_UNDRAGGING;
							ituScene->dragged = NULL;
						}
                        //widget->flags |= ITU_UNDRAGGING;
                        //ituScene->dragged = NULL;
                    }
                }
                else
                {
                    if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
                    {
                        if (count > 0)//bless
                        {
                            ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, 0);

							if (coverFlow->inc == 0)
                            {
                                int offset, absoffset, interval;
                                
                                offset = x - coverFlow->touchPos;
								interval = offset / child->rect.width;
                                offset -= (interval * child->rect.width);
                                absoffset = offset > 0 ? offset : -offset;

								if (absoffset > child->rect.width / 2)
                                {
									coverFlow->frame = (int)((float)coverFlow->totalframe * (float)absoffset / (float)child->rect.width);
                                    //coverFlow->frame = absoffset / ((child->rect.width / coverFlow->totalframe) + 1);

									if (COVERFLOW_DEBUG_FOCUSINDEX && interval)
										printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - interval, __LINE__);

                                    coverFlow->focusIndex -= interval;

                                    if (offset >= 0)
                                    {
                                        //coverFlow->inc = child->rect.width;
										//coverFlow->focusIndex--;

										if (coverFlow->focusIndex < 0)
										{
											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex + count, __LINE__);

											coverFlow->focusIndex += count;
										}
										ituCoverFlowFixFC(coverFlow);
										ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
										LOG_DBG "1: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                    }
                                    else
                                    {
                                        //coverFlow->inc = -child->rect.width;
										//coverFlow->focusIndex++;

										if (coverFlow->focusIndex >= count)
										{
											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - count, __LINE__);

											coverFlow->focusIndex -= count;
										}
										ituCoverFlowFixFC(coverFlow);
										ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
										LOG_DBG "2: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                    }

									//try fix bug
									//coverFlow->inc = offset;
									coverFlow->inc = offset;
									/*while (coverFlow->inc > (base_size / 2))
									{
										coverFlow->inc -= base_size;
									}
									while (coverFlow->inc < (-1 * base_size / 2))
									{
										coverFlow->inc += base_size;
									}*/
                                }
                                else if (absoffset)
                                {
									coverFlow->frame = (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);

                                    if (offset >= 0)
                                    {
										//coverFlow->focusIndex -= interval;// +1;
										//coverFlow->focusIndex--;
										//offset += base_size;

										if (coverFlow->focusIndex < 0)
										{
											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex + count, __LINE__);

											coverFlow->focusIndex += count;
										}

										LOG_DBG "3: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                    }
                                    else
                                    {
										//coverFlow->focusIndex -= interval;// -1;
										//coverFlow->focusIndex++;
										//offset -= base_size;

										if (coverFlow->focusIndex >= count)
										{
											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - count, __LINE__);

											coverFlow->focusIndex -= count;
										}

										LOG_DBG "4: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                    }

									if (coverFlow->temp1 == 1) //small dragging
									{
										coverFlow->inc = offset * -1;
										//coverFlow->frame = coverFlow->totalframe - (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
									}
									else
									{
										coverFlow->inc = offset;
									}
                                }
                                widget->flags |= ITU_UNDRAGGING;
                                ituScene->dragged = NULL;
                            }
                        }
                    }
                    else
                    {
                        //if (count > 0)
						if ((count > 0) && (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)))
                        {
                            //ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, 0);
							bool boundary_touch = false;
							//[MOUSEUP][Horizontal][non-cycle][layout]
							if (coverFlow->boundaryAlign)
							{
								int max_neighbor_item = ((widget->rect.width / base_size) - 1) / 2;
								int max_width_item = widget->rect.width / base_size;

								if (max_neighbor_item == 0)
									max_neighbor_item++;

								if (coverFlow->focusIndex >= max_neighbor_item)
								{
									if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_neighbor_item))
										boundary_touch = true;
									else
									{
										if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_width_item))
											boundary_touch = true;
									}
								}
								else
									boundary_touch = true;

								if (!boundary_touch) //re-check again for some special case that with bad max_neighbor_item
								{ //this case should debug for some extreme item count
									ITUWidget* child1 = CoverFlowGetVisibleChild(coverFlow, 0);
									ITUWidget* child2 = CoverFlowGetVisibleChild(coverFlow, count - 1);
									int min_item_pos, max_item_pos, min_boundary_value, max_boundary_value;

									if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
									{
										min_item_pos = child1->rect.y;
										max_item_pos = child2->rect.y + base_size;
									}
									else
									{
										min_item_pos = child1->rect.x;
										max_item_pos = child2->rect.x + base_size;
									}

									min_boundary_value = base_size / coverFlow->bounceRatio;
									max_boundary_value = widget_size - (base_size / coverFlow->bounceRatio);

									if (min_item_pos >= min_boundary_value)
									{
										boundary_touch = true;

										if (COVERFLOW_DEBUG_FOCUSINDEX)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, 0, __LINE__);

										coverFlow->focusIndex = 0;
									}
									else if (max_item_pos <= max_boundary_value)
									{
										boundary_touch = true;

										if (COVERFLOW_DEBUG_FOCUSINDEX)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, count - 1, __LINE__);

										coverFlow->focusIndex = count - 1;
									}
								}
							}

                            if (coverFlow->inc == 0)
                            {
                                int offset, absoffset, interval;
                                
                                //offset = x - coverFlow->touchPos;//PPAP
								offset = CoverFlowGetDraggingDist(coverFlow);
								interval = offset / base_size;
								offset -= (interval * base_size);
                                //absoffset = offset > 0 ? offset : -offset;

								///////////// check bounce
								if (coverFlow->focusIndex <= 0)
								{
									ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, 0);
									if (coverFlow->bounceRatio > 0)
									{
										if (offset >= base_size / coverFlow->bounceRatio)
											offset = base_size / coverFlow->bounceRatio;

										if (offset == 0)
											offset = CoverFlowGetDraggingDist(coverFlow);
									}
									else
									{
										offset = 0;
									}
								}
								else if (coverFlow->focusIndex >= count - 1)
								{
									ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, 0);
									if (coverFlow->bounceRatio > 0)
									{
										if (offset <= -base_size / coverFlow->bounceRatio)
											offset = -base_size / coverFlow->bounceRatio;

										if (offset == 0)
											offset = CoverFlowGetDraggingDist(coverFlow);
									}
									else
									{
										offset = 0;
									}
								}
								absoffset = offset > 0 ? offset : -offset;

								if ((absoffset > (base_size / 2)) || (absoffset > (widget_size / coverFlow->min_change_dist_factor)))
                                {//f1
									//coverFlow->frame = coverFlow->totalframe - absoffset / (base_size / coverFlow->totalframe);

                                    if (offset >= 0)
                                    {
										//coverFlow->frame = absoffset / (base_size / coverFlow->totalframe) + 1;
										//coverFlow->frame = ((absoffset * coverFlow->totalframe) / base_size) + 1;
										//coverFlow->frame = (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
										//coverFlow->frame = coverFlow->totalframe - (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
										coverFlow->frame = 0;

                                        if (coverFlow->focusIndex > interval)
                                        {
											//fix the alignment wrong when mouseup at maximum focusindex then stop wrong layout when mouseup done.
											int mod = 1;
											int max_touch_start_size_index = (count - (widget_size / base_size));

											if ((coverFlow->boundaryAlign) && (coverFlow->focusIndex > max_touch_start_size_index))
											{
												mod = coverFlow->focusIndex - max_touch_start_size_index + 1;
											}

											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("[Debug][CoverFlow][FC %d ", coverFlow->focusIndex);

											coverFlow->focusIndex -= interval + mod;

											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("to %d][%d]\n", coverFlow->focusIndex, __LINE__);

											if (coverFlow->focusIndex < 0)
												coverFlow->focusIndex = 0;

											//small shift alignment
											if (0)//(boundary_touch)
											{
												while ((CoverFlowCheckBoundaryTouch(widget) == ITU_BOUNCE_2) && (coverFlow->focusIndex > 0))
												{
													if (COVERFLOW_DEBUG_FOCUSINDEX)
														printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - 1, __LINE__);

													coverFlow->focusIndex--;
												}
											}

											//if (boundary_touch)
											//	coverFlow->focusIndex = 0; //CoverFlowGetVisibleChildCount(coverFlow) - widget->rect.width / base_size;
											
											//coverFlow->inc = base_size;
											ituCoverFlowFixFC(coverFlow);
											ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
											LOG_DBG "5: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
											//if (boundary_touch)
											//	coverFlow->focusIndex -= (interval != 0) ? (((offset >= 0) ? (1) : (-1))) : (((absoffset > child->rect.width / 2) ? (((offset >= 0) ? (1) : (-1))) : (0)));
                                        }
                                        else
                                        {
											//coverFlow->inc = -base_size;
											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, -1, __LINE__);

                                            coverFlow->focusIndex = -1;
											LOG_DBG "6: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                        }
                                    }
                                    else
                                    {
										//coverFlow->frame = absoffset / (base_size / coverFlow->totalframe) + 1;
										//coverFlow->frame = ((absoffset * coverFlow->totalframe) / base_size) + 1;
										//coverFlow->frame = (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
										//coverFlow->frame = coverFlow->totalframe - (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
										coverFlow->frame = 0;

                                        if (coverFlow->focusIndex < count + interval - 1)
                                        {
											//coverFlow->inc = -base_size;
											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("[Debug][CoverFlow][FC %d ", coverFlow->focusIndex);

                                            coverFlow->focusIndex -= interval - 1;

											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("to %d][%d]\n", coverFlow->focusIndex, __LINE__);

											if (coverFlow->focusIndex > (count - 1))
												coverFlow->focusIndex = count - 1;

											ituCoverFlowFixFC(coverFlow);
											ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
											LOG_DBG "7: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
											//if (boundary_touch)
											//	coverFlow->focusIndex -= (interval != 0) ? (((offset >= 0) ? (1) : (-1))) : (((absoffset > child->rect.width / 2) ? (((offset >= 0) ? (1) : (-1))) : (0)));
                                        }
                                        else
                                        {
											//coverFlow->inc = base_size;
											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, count, __LINE__);

                                            coverFlow->focusIndex = count;
											LOG_DBG "8: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                        }
                                    }
									//try fix bug
									coverFlow->inc = offset * -1;
									/*while (coverFlow->inc > (base_size / 2))
									{
										coverFlow->inc -= base_size;
									}
									while (coverFlow->inc < (-1 * base_size / 2))
									{
										coverFlow->inc += base_size;
									}*/
                                }
                                else if (absoffset)
                                {
									//try debug 0911
									//coverFlow->frame = coverFlow->totalframe - (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
									coverFlow->frame = (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);

                                    if (offset >= 0)
                                    {
										//big shift alignment
										//if ((boundary_touch) && (coverFlow->focusIndex > 0))
										//	coverFlow->focusIndex = CoverFlowGetVisibleChildCount(coverFlow) - widget->rect.width / base_size;
										
										int lastf = coverFlow->focusIndex;
										int max_touch_start_size_index = (count - (widget_size / base_size));
										//fix the alignment wrong when mouseup at maximum focusindex then stop wrong layout when mouseup done.
										int mod = 0;
										if ((coverFlow->boundaryAlign) && (coverFlow->focusIndex >= max_touch_start_size_index))
										{
											mod = coverFlow->focusIndex - max_touch_start_size_index;
										}

										if (COVERFLOW_DEBUG_FOCUSINDEX && interval)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - interval - mod, __LINE__);

                                        coverFlow->focusIndex -= interval + mod;

										//if (boundary_touch)
										//	coverFlow->focusIndex -= (interval != 0) ? (((offset >= 0) ? (1) : (-1))) : (((absoffset > child->rect.width / 2) ? (((offset >= 0) ? (1) : (-1))) : (0)));
										
										//small shift alignment
										if (0)//(boundary_touch) //1/31
										{
											while ((CoverFlowCheckBoundaryTouch(widget) == ITU_BOUNCE_2) && (coverFlow->focusIndex > 0))
											{
												if (COVERFLOW_DEBUG_FOCUSINDEX)
													printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - 1, __LINE__);

												coverFlow->focusIndex--;
											}
										}

										if (coverFlow->focusIndex < 0)
											coverFlow->focusIndex = 0;

										if (lastf != coverFlow->focusIndex)
										{
											ituCoverFlowFixFC(coverFlow);
											ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
										}
										LOG_DBG "9: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                    }
                                    else
                                    {
										//coverFlow->inc = base_size;
										int lastf = coverFlow->focusIndex;

										if (COVERFLOW_DEBUG_FOCUSINDEX && interval)
											printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - interval, __LINE__);

                                        coverFlow->focusIndex -= interval;

										if (0)//(boundary_touch)
										{
											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("[Debug][CoverFlow][FC %d ", coverFlow->focusIndex);

											coverFlow->focusIndex -= (interval != 0) ? (((offset >= 0) ? (1) : (-1))) : (((absoffset > base_size / 2) ? (((offset >= 0) ? (1) : (-1))) : (0)));

											if (COVERFLOW_DEBUG_FOCUSINDEX)
												printf("to %d][%d]\n", coverFlow->focusIndex, __LINE__);
										}

                                        if (coverFlow->focusIndex >= count + 1)
                                            coverFlow->focusIndex = count;

										if (lastf != coverFlow->focusIndex)
										{
											ituCoverFlowFixFC(coverFlow);
											ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
										}
										LOG_DBG "10: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                    }
									coverFlow->inc = offset;
                                }

								if (widget)
								{
									ITUWidget* c1 = CoverFlowGetVisibleChild(coverFlow, 0);
									ITUWidget* c2 = CoverFlowGetVisibleChild(coverFlow, count - 1);
									int loop = 0;
									int pos_center = (widget_size / 2) - (base_size / 2);

									if (coverFlow->inc)
									{
										if (((coverFlow->focusIndex <= 0) && (c1->rect.x == pos_center)) || ((coverFlow->focusIndex >= (count - 1)) && (c2->rect.x == pos_center)))
										{
											ituScene->dragged = NULL;
											coverFlow->frame = 0;
											coverFlow->inc = 0;
											for (loop = 0; loop < 5; loop++)
											{
												printf("[MouseUP][check fail][X]\n");
											}
										}
										else
										{
											widget->flags |= ITU_UNDRAGGING;
											ituScene->dragged = NULL;
										}
									}
								}
								else
								{
									widget->flags |= ITU_UNDRAGGING;
									ituScene->dragged = NULL;
								}
                                //widget->flags |= ITU_UNDRAGGING;
                                //ituScene->dragged = NULL;
                            }
                        }
                    }
                }
                result = true;
            }
            widget->flags &= ~ITU_DRAGGING;
            coverFlow->touchCount = 0;
			coverFlow->touchPos = 0;
        }
    }

    if (ev == ITU_EVENT_TIMER)
    {
        if (coverFlow->touchCount > 0)
        {
            int x, y, dist;

            assert(widget->flags & ITU_HAS_LONG_PRESS);

            ituWidgetGetGlobalPosition(widget, &x, &y);

            if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
            {
                dist = ituScene->lastMouseY - (y + coverFlow->touchPos);
            }
            else
            {
                dist = ituScene->lastMouseX - (x + coverFlow->touchPos);
            }

            if (dist < 0)
                dist = -dist;

            if (dist >= ITU_DRAG_DISTANCE)
            {
                widget->flags |= ITU_DRAGGING;
                ituScene->dragged = widget;
                coverFlow->touchCount = 0;
            }
        }
		else if ((widget->flags & ITU_DRAGGING) && (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING) && (coverFlow->frame <= coverFlow->totalframe))
		{ //for goose check
			coverFlow->frame++;

			if (coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP)
			{
				//do nothing
			}
			else
			{
				if (coverFlow->inc > 0)
				{
					if (COVERFLOW_DEBUG_FOCUSINDEX)
						printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - 1, __LINE__);

					coverFlow->focusIndex--;
				}
				else
				{
					if (COVERFLOW_DEBUG_FOCUSINDEX)
						printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex + 1, __LINE__);

					coverFlow->focusIndex++;
				}

				ituWidgetUpdate(widget, ITU_EVENT_LAYOUT, 0, 0, 0);

				if (coverFlow->inc > 0)
				{
					if (COVERFLOW_DEBUG_FOCUSINDEX)
						printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex + 1, __LINE__);

					coverFlow->focusIndex++;
				}
				else
				{
					if (COVERFLOW_DEBUG_FOCUSINDEX)
						printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - 1, __LINE__);

					coverFlow->focusIndex--;
				}

				ituCoverFlowFixFC(coverFlow);
				return true;
			}
		}

		//for goose check
		if ((!(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)) && (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)) && (widget->flags & ITU_UNDRAGGING))
		{
			int count = CoverFlowGetVisibleChildCount(coverFlow);
			ITUWidget* c1 = CoverFlowGetVisibleChild(coverFlow, 0);
			ITUWidget* c2 = CoverFlowGetVisibleChild(coverFlow, count - 1);
			//int loop = 0;
			int pos = (widget_size / 2) + (base_size / 2);

			if (0)//(coverFlow->inc)
			{
				if ((c1->rect.x >= pos)
					|| ((c2->rect.x + c2->rect.width) <= 0)
					|| ((widget_size - (c2->rect.x + c2->rect.width)) >= (widget_size / coverFlow->bounceRatio))
					|| (c1->rect.x >= (widget_size / coverFlow->bounceRatio)))
				{
					coverFlow->inc = 0;
					coverFlow->frame = 0;
					/*for (loop = 0; loop < 5; loop++)
					{
						printf("[trigger0]\n");
					}*/
					widget->flags &= ~ITU_DRAGGING;
					widget->flags &= ~ITU_UNDRAGGING;
					ituWidgetUpdate(widget, ITU_EVENT_LAYOUT, 0, 0, 0);
					printf("llllllllllllllllllllll\n");
				}
			}

			if (coverFlow->inc < 0)
			{
				if ((coverFlow->focusIndex >= (count - 1)) && ((c2->rect.x + c2->rect.width) == pos) && (coverFlow->frame == 0))
				{
					/*for (loop = 0; loop < 5; loop++)
					{
						printf("[trigger1][inc %d][Findex %d][c2x %d]\n", coverFlow->inc, coverFlow->focusIndex, c2->rect.x);
					}*/
					coverFlow->inc = 0;
					widget->flags &= ~ITU_DRAGGING;
					widget->flags &= ~ITU_UNDRAGGING;
				}
			}
			else if (coverFlow->inc > 0)
			{
				if ((coverFlow->focusIndex <= 0) && ((c1->rect.x + c1->rect.width) == pos) && (coverFlow->frame == 0))
				{
					/*for (loop = 0; loop < 5; loop++)
					{
						printf("[trigger2][inc %d][Findex %d][c1x %d]\n", coverFlow->inc, coverFlow->focusIndex, c1->rect.x);
					}*/
					coverFlow->inc = 0;
					widget->flags &= ~ITU_DRAGGING;
					widget->flags &= ~ITU_UNDRAGGING;
				}
			}
		}

		if (coverFlow->inc)
        {
            int i, count = CoverFlowGetVisibleChildCount(coverFlow);
			int split_shift = 0;

			result = true;

			//if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
			//	printf("[Coverflow][Sliding][frame %d/%d]\n", coverFlow->frame, coverFlow->totalframe);
			//else
			//	printf("[Coverflow][Undragging][frame %d/%d]\n", coverFlow->frame, coverFlow->totalframe);

			//try to fix dual bounce
			if ((widget->flags & ITU_BOUNCING) && ((coverFlow->coverFlowFlags & ITU_BOUNCE_1) && (coverFlow->coverFlowFlags & ITU_BOUNCE_2)))
			{
				ITUWidget* child_1 = CoverFlowGetVisibleChild(coverFlow, 0);

				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
				{
					if (child_1->rect.y < (0 - base_size))
						coverFlow->coverFlowFlags &= ~ITU_BOUNCE_1;
					else
						coverFlow->coverFlowFlags &= ~ITU_BOUNCE_2;
				}
				else
				{
					if (child_1->rect.x < (0 - base_size))
						coverFlow->coverFlowFlags &= ~ITU_BOUNCE_1;
					else
						coverFlow->coverFlowFlags &= ~ITU_BOUNCE_2;
				}

				//if ((coverFlow->coverFlowFlags & ITU_BOUNCE_1) && (coverFlow->coverFlowFlags & ITU_BOUNCE_2))
				//	coverFlow->coverFlowFlags &= ~ITU_BOUNCE_1;

			}

			if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
			{
				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
				{
					if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE1) || (coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE2))
					{
						coverFlow->frame = coverFlow->totalframe;
					}
					else
					{
						int fx = 0;
						int fy = 0;
						int move_step = 0;
						ITUWidget* child_1 = CoverFlowGetVisibleChild(coverFlow, 0);
						ITUWidget* child_2 = CoverFlowGetVisibleChild(coverFlow, count - 1);
						float step = (float)coverFlow->frame / (float)coverFlow->totalframe;
						//step = step * (float)M_PI / 2;
						//step = sinf(step);
						//move_step = (int)(coverFlow->inc * step);
						move_step = (int)(((float)coverFlow->inc / (float)COVERFLOW_FACTOR) * (float)coverFlow->slide_diff / 40.0);

						if (step <= COVERFLOW_PROCESS_STAGE1)
							move_step /= 3;
						else if (step <= COVERFLOW_PROCESS_STAGE2)
							move_step /= 6;
						else
							move_step /= 12;

						if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
						{
							if ((coverFlow->bounceRatio > 0) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE))
							{
								int tor = child_1->rect.height / coverFlow->bounceRatio;

								if ((child_1->rect.y + move_step) > tor)
								{
									move_step = tor - child_1->rect.y;
									coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE;
								}
								else if ((child_2->rect.y + child_2->rect.height + move_step) < (widget->rect.height - tor))
								{
									move_step = widget->rect.height - tor - (child_2->rect.y + child_2->rect.height);
									coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE;
								}
							}
							else
							{
								if (coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE)
								{
									int tor_step = ((child_1->rect.height / coverFlow->bounceRatio) / 10) + 1;
									move_step = (coverFlow->inc > 0) ? (-1 * tor_step) : (1 * tor_step);

									if (((child_1->rect.y + move_step) <= 0) && (child_1->rect.y > 0))
									{
										move_step = 0 - child_1->rect.y;
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE1;
									}
									else if (((child_2->rect.y + child_2->rect.height + move_step) >= widget->rect.height) && (child_2->rect.y < widget->rect.height))
									{
										move_step = widget->rect.height - (child_2->rect.y + child_2->rect.height);
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE2;
									}
								}
								else
								{
									if ((child_1->rect.y + move_step) > 0)
									{
										move_step = 0 - child_1->rect.y;
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE1;
									}
									else if ((child_2->rect.y + child_2->rect.height + move_step) < widget->rect.height)
									{
										move_step = widget->rect.height - (child_2->rect.y + child_2->rect.height);
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE2;
									}
								}
							}

							for (i = 0; i < count; i++)
							{
								ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
								fy = child->rect.y;

								if (coverFlow->frame > 0)
									fy += move_step;

								ituCoverFlowSetXY(coverFlow, i, fy, __LINE__);
								//printf("[fy] %d [step] %.3f\n", fy, step);
							}

							//printf("[Frame %d]move_step %d\n", coverFlow->frame, move_step);
						}
						else
						{ //For Horizontal
							if ((coverFlow->bounceRatio > 0) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE))
							{
								int tor = child_1->rect.width / coverFlow->bounceRatio;

								if ((child_1->rect.x + move_step) > tor)
								{
									move_step = tor - child_1->rect.x;
									coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE;
								}
								else if ((child_2->rect.x + child_2->rect.width + move_step) < (widget->rect.width - tor))
								{
									move_step = widget->rect.width - tor - (child_2->rect.x + child_2->rect.width);
									coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE;
								}
							}
							else
							{
								if (coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE)
								{
									int tor_step = ((child_1->rect.width / coverFlow->bounceRatio) / 10) + 1;
									move_step = (coverFlow->inc > 0) ? (-1 * tor_step) : (1 * tor_step);

									if (((child_1->rect.x + move_step) < 0) && (child_1->rect.x > 0))
									{
										move_step = 0 - child_1->rect.x;
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE1;
									}
									else if (((child_2->rect.x + child_2->rect.width + move_step) > widget->rect.width) && (child_2->rect.x < widget->rect.width))
									{
										move_step = widget->rect.width - (child_2->rect.x + child_2->rect.width);
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE2;
									}
								}
								else
								{
									if ((child_1->rect.x + move_step) > 0)
									{
										move_step = 0 - child_1->rect.x;
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE1;
									}
									else if ((child_2->rect.x + child_2->rect.width + move_step) < widget->rect.width)
									{
										move_step = widget->rect.width - (child_2->rect.x + child_2->rect.width);
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE2;
									}
								}
							}

							for (i = 0; i < count; i++)
							{
								ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
								fx = child->rect.x;

								if (coverFlow->frame > 0)
									fx += move_step;

								ituCoverFlowSetXY(coverFlow, i, fx, __LINE__);
								//printf("[fx] %d [step] %.3f\n", fx, step);
							}

							//printf("[Frame %d]move_step %d\n", coverFlow->frame, move_step);
						}
					}
				}
				else
					coverFlow->frame = coverFlow->totalframe;
			}
            else if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
            { // <<< ITU_EVENT_TIMER >>>//bless
				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
				{
					int index, count2;
					float step = 0.0;
					int local_inc = coverFlow->inc;
					int local_fi = coverFlow->focusIndex;

					while (local_inc > (base_size / 2))
					{
						local_inc -= base_size;
					}
					while (local_inc < (-1 * base_size / 2))
					{
						local_inc += base_size;
					}

					//bbbbb
					// cubic ease out: y = (x - 1)^3 + 1
					//step = step - 1;
					//step = step * step * step + 1;

					//workaround for wrong left-side display with hide child
					count2 = count / 2 + 1 - ((coverFlow->inc > 0) ? (1) : (0));
					//count2 = count / 2 + 1;
					index = local_fi;

					if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
					{
						int fy = widget->rect.height / 2 - base_size / 2;
						local_inc = coverFlow->inc;
						step = 1.0 - CoverFlowAniStepCal(coverFlow);
						//from focus to right side all
						for (i = 0; i < count2; ++i)
						{
							int ci = ((index >(count - 1)) ? (0) : (index));
							int fix = (int)(local_inc * step);
							int local_fy = fy + i * base_size;
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, ci);

							//try to fix the next child is wrong position under cycle mode
							if (child)
							{
								int check_item = (((ci + 1) > (count - 1)) ? (0) : (ci + 1));
								ITUWidget* child_next = CoverFlowGetVisibleChild(coverFlow, check_item);

								if (child_next)
								{
									if ((child_next->rect.y < child->rect.y) && ((i < (count2 - 1))))
										ituCoverFlowSetXY(coverFlow, check_item, child->rect.y + base_size, __LINE__);
								}
							}

							if (((local_fy + fix) < child->rect.y) && (local_inc > 0))
							{
								int ff = 1;
								while ((local_fy + fix) < child->rect.y)
								{
									step = ((float)(coverFlow->frame + ff) / (float)coverFlow->totalframe);
									fix = (int)(local_inc * step);
									ff++;
								}
								coverFlow->frame += (ff - 1);
								//break;
							}
							else if (((local_fy + fix) > child->rect.y) && (local_inc < 0) && (i < (count2 - 1)))
							{
								int ff = 1;
								while ((local_fy + fix) > child->rect.y)
								{
									step = ((float)(coverFlow->frame + ff) / (float)coverFlow->totalframe);
									fix = (int)(local_inc * step);
									ff++;
								}
								coverFlow->frame += (ff - 1);
								//break;
							}
							local_fy += fix;

							ituCoverFlowSetXY(coverFlow, ci, local_fy, __LINE__);
							index = ci + 1;
						}

						count2 = count - count2;
						index = local_fi - 1;
						fy = fy - base_size;
						//from left side all to focus -1
						for (i = 0; i < count2; ++i)
						{
							int ci = ((index < 0) ? (count - 1) : (index));
							int fix = (int)(local_inc * step);
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, ci);
							int local_fy = fy;
							local_fy -= i * base_size;
							local_fy += fix;

							ituCoverFlowSetXY(coverFlow, ci, local_fy, __LINE__);
							index = ci - 1;

							//fix for user use bad design (item count less than display size + 2)
							if ((coverFlow->inc > 0) && (count2 <= 1))
							{
								int prev2 = ((index < 0) ? (count - 1) : (index));
								child = CoverFlowGetVisibleChild(coverFlow, prev2);
								if (child->rect.y > widget->rect.height)
									count2++;
							}
						}
					}
					else
					{
						int local_frame = coverFlow->frame;
						step = 1.0 - CoverFlowAniStepCal(coverFlow);

						if (coverFlow->temp1 < 1)
						{
							local_inc = ((coverFlow->inc > 0) ? (base_size) : (-base_size));
							step = (float)coverFlow->frame / (float)coverFlow->totalframe;
						}
						else
						{
							local_inc = coverFlow->inc * -1;
						}

						//printf("frame %d fix %d\n", coverFlow->frame, (int)(local_inc * step));

						for (i = 0; i < count2; ++i)
						{
							int ci = ((index >(count - 1)) ? (0) : (index));
							int fix = (int)(local_inc * step);
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, ci);
							bool layout_check = true;
							int fy = widget->rect.height / 2 - base_size / 2;

							//count = 2, when right way(inc > 0) the right child will be at left side
							if ((coverFlow->inc > 0) && (count == 2))
								fy -= i * base_size;
							else
								fy += i * base_size;
							//debugging layout_check (something wrong)
							//if (((fx\y > 0) && (child->rect.y > 0)) || ((fy < 0) && (child->rect.y < 0)))
							//	layout_check = true;

							if (((fy + fix) > child->rect.y) && (local_inc < 0) && layout_check && (coverFlow->temp1 < 1))
							{
								int ff = 1;
								while (((fy + fix) > child->rect.y) && (local_frame < coverFlow->totalframe))
								{
									local_frame++;
									step = (float)local_frame / (float)coverFlow->totalframe;
									fix = (int)(local_inc * step);
								}
								step = (float)local_frame / (float)coverFlow->totalframe;
							}
							else if (((fy + fix) < child->rect.y) && (local_inc > 0) && layout_check && (coverFlow->temp1 < 1))
							{
								int ff = 1;
								while (((fy + fix) < child->rect.y) && (local_frame < coverFlow->totalframe))
								{
									local_frame++;
									step = (float)local_frame / (float)coverFlow->totalframe;
									fix = (int)(local_inc * step);
								}
								step = (float)local_frame / (float)coverFlow->totalframe;
							}

							fy += fix;
							ituCoverFlowSetXY(coverFlow, ci, fy, __LINE__);
							index = ci + 1;
						}

						count2 = count - count2;
						index = local_fi - 1;
						for (i = 0; i < count2; ++i)
						{
							int ci = ((index < 0) ? (count - 1) : (index));
							int fix = (int)(local_inc * step);
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, ci);
							int fy = widget->rect.height / 2 - base_size / 2;
							fy -= base_size;
							fy -= i * base_size;
							fy += fix;
							ituCoverFlowSetXY(coverFlow, ci, fy, __LINE__);
							index = ci - 1;

							//fix for user use bad design (item count less than display size + 2)
							if ((coverFlow->inc > 0) && (count2 <= 1))
							{
								int prev2 = ((index < 0) ? (count - 1) : (index));
								child = CoverFlowGetVisibleChild(coverFlow, prev2);
								if (child->rect.y > widget->rect.height)
									count2++;
							}
						}
					}

					//for goose do more one frame at last frame (slow down)
					if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING) && (coverFlow->frame == (coverFlow->totalframe - 1)) && (count <= COVERFLOW_SMOOTH_LASTFRAME_COUNT))
					{
						if (coverFlow->org_totalframe == coverFlow->totalframe)
						{
							coverFlow->totalframe *= 2;
							coverFlow->frame = coverFlow->totalframe - 2;
						}
					}

					result = true;
				}
                else if (widget->flags & ITU_BOUNCING)
                {
					// <<< ITU_EVENT_TIMER >>>
					// <<< Vertical / Non-Cycle >>>
                    float step = (float)coverFlow->frame / (float)coverFlow->totalframe;
					step = step - 1;
					step = step * step * step + 1;

                    //printf("step=%f\n", step);

                    for (i = 0; i < count; ++i)
                    {
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);

						//int fy = widget->rect.height / 2 - base_size / 2;
						//fy -= base_size * coverFlow->focusIndex;
						//fy += i * base_size;

						int fy;
						if (coverFlow->coverFlowFlags & ITU_BOUNCE_1)
						{
							fy = 0;
						}
						else if (coverFlow->coverFlowFlags & ITU_BOUNCE_2)
						{
							fy = widget_size - count * base_size;
						}
						else
						{
							if (coverFlow->focusIndex == 0)
								fy = 0;
							else
								fy = widget_size - (coverFlow->focusIndex + 1) * base_size;
						}

						fy += i * base_size;

						fy += (int)(coverFlow->inc * step - coverFlow->overlapsize);
						ituCoverFlowSetXY(coverFlow, i, fy, __LINE__);
						//if (i == 0)
						//	printf("fy %d\n", fy);
                    }
                }
                else
                {
					// <<< ITU_EVENT_TIMER >>>
					// <<< Vertical / Non-Cycle >>>
					bool wrong_pos_check = true;
					//working next
					//if (coverFlow->boundaryAlign == 0)
					//	wrong_pos_check = false;

					while (wrong_pos_check)
					{
						int local_movelog = coverFlow->movelog;
						int local_focusindex = coverFlow->focusIndex;
						int current_first_child_pos;
						int cal_init_pos = 0;
						float step = 0.0;
						ITUWidget* childlast = CoverFlowGetVisibleChild(coverFlow, count - 1);
						int childlast_tor_dist = 0;
						int tor_dist = (base_size / ((coverFlow->bounceRatio > 0) ? (coverFlow->bounceRatio) : (base_size)));

						if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
						{
							childlast_tor_dist = (widget_size - (childlast->rect.y + base_size));
						}
						else
						{
							childlast_tor_dist = (widget_size - (childlast->rect.x + base_size));
						}

						wrong_pos_check = false;
						step = CoverFlowAniStepCal(coverFlow);

						/*if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
						{
						step = (float)(coverFlow->totalframe - coverFlow->frame) / (float)coverFlow->totalframe;
						}
						else
						{
						step = (float)coverFlow->frame / (float)coverFlow->totalframe;
						}
						step = step - 1;
						step = step * step * step + 1;*/

						for (i = 0; i < count; i++)
						{
							int fix = 0;
							int temp_local_cal = 0;
							int child_rect_pos;
							int childlast_rect_pos;
							bool align_start_side = true;
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);

							if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
							{
								child_rect_pos = child->rect.y;
								childlast_rect_pos = childlast->rect.y;
							}
							else
							{
								child_rect_pos = child->rect.x;
								childlast_rect_pos = childlast->rect.x;
							}

							if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
							{
								fix = (base_size - (int)((float)(base_size)* step));
								fix *= ((coverFlow->inc > 0) ? (1) : (-1));
							}
							else //Undragging mode
							{
								fix = (base_size - (int)((float)(base_size)* step));
								fix *= ((coverFlow->inc > 0) ? (1) : (-1));
							}

							if (coverFlow->boundaryAlign)
							{
								if (i == 0)
								{
									bool check_position = true;

									if ((widget_size - (childlast_rect_pos + base_size)) >= 0)
									{
										align_start_side = false;
									}
									else
									{
										if (coverFlow->temp3 == ITU_BOUNCE_2)
										{
											if (coverFlow->inc < 0)
											{
												//if (fix > 0)
												//	fix = fix - base_size;
											}
										}

										align_start_side = true;
									}

									if (align_start_side)
									{
										while (check_position)
										{
											if (abs((0 - (base_size * local_focusindex)) - child_rect_pos) <= base_size)
												check_position = false;
											else
											{
												local_focusindex += ((coverFlow->inc > 0) ? (-1) : (1));

												if (local_focusindex < 0)
												{
													local_focusindex = 0;
													check_position = false;
												}
												else if (local_focusindex >(count - 1))
												{
													local_focusindex = count - 1;
													check_position = false;
												}
											}
										}

										cal_init_pos = 0 - (base_size * local_focusindex);

										if (coverFlow->temp3 == ITU_BOUNCE_2)
										{
											//coverFlow->temp3 = 0;
											//cal_init_pos += (base_size * 1);
										}

										//printf("align [start] side mode [frame %d] [cal init pos %d]\n", coverFlow->frame, cal_init_pos);
									}
									else
									{
										int focusIndexMax = (count - 1) - (((widget_size / base_size) / 2) + 1);
										int align_fix_dist = 0;

										while (check_position)
										{
											if (abs((widget_size - (count * base_size)) + align_fix_dist - child_rect_pos) <= base_size)
												check_position = false;
											else
											{
												local_focusindex += ((coverFlow->inc > 0) ? (-1) : (1));

												if (local_focusindex < 0)
												{
													align_fix_dist = 0;
													local_focusindex = 0;
													check_position = false;
												}
												else if (local_focusindex >(count - 1))
												{
													align_fix_dist = 0;
													local_focusindex = count - 1;
													check_position = false;
												}

												if (local_focusindex < focusIndexMax)
													align_fix_dist = (focusIndexMax - local_focusindex) * base_size;
											}
										}
										cal_init_pos = widget_size - (count * base_size) + align_fix_dist;
										//printf("align [end] side mode [frame %d]\n", coverFlow->frame);
									}
								}
								else
								{
									cal_init_pos += base_size;
								}

								temp_local_cal = (widget_size - (childlast_rect_pos + base_size + fix));

								if ((coverFlow->temp3 == ITU_BOUNCE_2) || (widget_size == base_size))
								{
									//coverFlow->temp3 = 0;
									temp_local_cal = 0;
								}

								if ((temp_local_cal > (base_size / coverFlow->bounceRatio)) && ((childlast_rect_pos + base_size) <= widget_size))
								{
									i = count;

									if (coverFlow->frame < coverFlow->totalframe)
									{
										temp_local_cal = ((childlast_rect_pos + base_size) - widget_size);
										if (temp_local_cal >(base_size / 5))
										{
											for (i = 0; i < count; i++)
											{
												child = CoverFlowGetVisibleChild(coverFlow, i);

												if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
												{
													child_rect_pos = child->rect.y;
												}
												else
												{
													child_rect_pos = child->rect.x;
												}

												ituCoverFlowSetXY(coverFlow, i, child_rect_pos - coverFlow->overlapsize - (temp_local_cal / 5), __LINE__);
											}
										}
										else
										{
											for (i = 0; i < count; i++)
											{
												child = CoverFlowGetVisibleChild(coverFlow, i);

												if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
												{
													child_rect_pos = child->rect.y;
												}
												else
												{
													child_rect_pos = child->rect.x;
												}

												ituCoverFlowSetXY(coverFlow, i, child_rect_pos - coverFlow->overlapsize - temp_local_cal, __LINE__);
											}
										}
									}

									continue;
								}


								if (coverFlow->frame <= coverFlow->totalframe)
								{
									bool local_debug_print = false;

									if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
									{
										if (((coverFlow->inc > 0) && ((cal_init_pos + fix) > child_rect_pos)) || ((coverFlow->inc < 0) && ((cal_init_pos + fix) < child_rect_pos)))
										{
											///check the first and last child position is outside or not
											int base_bounce = ((coverFlow->bounceRatio <= 0) ? (base_size) : (coverFlow->bounceRatio));
											bool check_outside = false;

											if (i == 0)
											{
												if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
												{
													if (child->rect.y > (base_size / base_bounce))
														check_outside = true;
												}
												else
												{
													if (child->rect.x > (base_size / base_bounce))
														check_outside = true;
												}
											}
											else if (i == (count - 1))
											{
												if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
												{
													if ((child->rect.y + base_size) < (widget_size - (base_size / base_bounce)))
														check_outside = true;
												}
												else
												{
													if ((child->rect.x + base_size) < (widget_size - (base_size / base_bounce)))
														check_outside = true;
												}
											}

											if ((i == 0) && local_debug_print)
												printf("[D][%d] [frame %d]\n", cal_init_pos + fix, coverFlow->frame);

											if (!check_outside)
											{
												ituCoverFlowSetXY(coverFlow, i, cal_init_pos - coverFlow->overlapsize + fix, __LINE__);
											}
										}
										else
										{
											if ((i == 0) && local_debug_print)
											{
												if (coverFlow->inc > 0)
													printf("[%d + %d] > [%d] [frame %d]\n", cal_init_pos, fix, child_rect_pos, coverFlow->frame);
												else
													printf("[%d + %d] < [%d] [frame %d]\n", cal_init_pos, fix, child_rect_pos, coverFlow->frame);
											}
										}
									}
									else //Undragging mode
									{
										if (((coverFlow->inc > 0) && ((cal_init_pos + fix) < child_rect_pos)) || ((coverFlow->inc < 0) && ((cal_init_pos + fix) > child_rect_pos)))
										{
											if ((i == 0) && local_debug_print)
												printf("[D][%d] [frame %d]\n", cal_init_pos + fix, coverFlow->frame);

											ituCoverFlowSetXY(coverFlow, i, cal_init_pos - coverFlow->overlapsize + fix, __LINE__);
										}
										else
										{
											if ((i == 0) && local_debug_print)
											{
												if (coverFlow->inc > 0)
													printf("[%d + %d] < [%d] [frame %d]\n", cal_init_pos, fix, child_rect_pos, coverFlow->frame);
												else
													printf("[%d + %d] > [%d] [frame %d]\n", cal_init_pos, fix, child_rect_pos, coverFlow->frame);
											}
										}
									}
								}
							}
							else //default center align mode
							{
								cal_init_pos = ((widget_size - base_size) / 2) - ((coverFlow->focusIndex - i) * base_size);

								if (coverFlow->frame <= coverFlow->totalframe)
								{
									bool local_debug_print = false;

									if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
									{
										if (((coverFlow->inc > 0) && ((cal_init_pos + fix) > child_rect_pos)) || ((coverFlow->inc < 0) && ((cal_init_pos + fix) < child_rect_pos)))
										{
											if ((i == 0) && local_debug_print)
												printf("[D][%d] [frame %d]\n", cal_init_pos + fix, coverFlow->frame);

											ituCoverFlowSetXY(coverFlow, i, cal_init_pos - coverFlow->overlapsize + fix, __LINE__);
										}
										else
										{
											if ((i == 0) && local_debug_print)
											{
												if (coverFlow->inc > 0)
													printf("[%d + %d] > [%d] [frame %d]\n", cal_init_pos, fix, child_rect_pos, coverFlow->frame);
												else
													printf("[%d + %d] < [%d] [frame %d]\n", cal_init_pos, fix, child_rect_pos, coverFlow->frame);
											}
										}
									}
									else //Undragging mode
									{
										if (((coverFlow->inc > 0) && ((cal_init_pos + fix) < child_rect_pos)) || ((coverFlow->inc < 0) && ((cal_init_pos + fix) > child_rect_pos)))
										{
											if ((i == 0) && local_debug_print)
												printf("[D][%d] [frame %d]\n", cal_init_pos + fix, coverFlow->frame);

											ituCoverFlowSetXY(coverFlow, i, cal_init_pos - coverFlow->overlapsize + fix, __LINE__);
										}
										else
										{
											if ((i == 0) && local_debug_print)
											{
												if (coverFlow->inc > 0)
													printf("[%d + %d] < [%d] [frame %d]\n", cal_init_pos, fix, child_rect_pos, coverFlow->frame);
												else
													printf("[%d + %d] > [%d] [frame %d]\n", cal_init_pos, fix, child_rect_pos, coverFlow->frame);
											}

											////////// try to save more frame when wrong position 2018/4/12
											if (i == 0)
											{
												int savecount = 0;
												bool recheck = true;
												while (recheck)
												{
													coverFlow->frame++;
													step = CoverFlowAniStepCal(coverFlow);

													if (coverFlow->frame < coverFlow->totalframe)
													{
														if (abs(coverFlow->inc) >= (base_size / 2))
														{
															fix = (base_size - (int)((float)(base_size)* step));

															if (coverFlow->inc < 0)
															{
																if (fix > 0)
																	fix *= -1;
															}
														}
														else
														{
															fix = (base_size - (int)((float)(base_size)* step));
															fix *= ((coverFlow->inc > 0) ? (1) : (-1));
														}

														if (((coverFlow->inc > 0) && ((cal_init_pos + fix) < child_rect_pos)) || ((coverFlow->inc < 0) && ((cal_init_pos + fix) > child_rect_pos)))
														{
															coverFlow->frame--;
															i = count;
															recheck = false;
														}
														else
														{
															savecount++;
															//printf("[Coverflow][Undragging][Position sync save frame %d]\n", savecount);
														}
													}
													else
													{
														recheck = false;
													}
												}
											}
											/////////////////////////////////
										}
									}
								}
							}

							if (coverFlow->frame <= coverFlow->totalframe)
							{
								//for goose do more one frame at last frame (slow down)
								if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING) && (coverFlow->frame == (coverFlow->totalframe - 1)) && (count <= COVERFLOW_SMOOTH_LASTFRAME_COUNT))
								{
									if (coverFlow->org_totalframe == coverFlow->totalframe)
									{
										coverFlow->totalframe *= 2;
										coverFlow->frame = coverFlow->totalframe - 2;
									}
								}
							}
						}

						if (coverFlow->focusIndex < 0)
							coverFlow->focusIndex = 0;
						else if (coverFlow->focusIndex > (count - 1))
							coverFlow->focusIndex = count - 1;

						/*
						for (i = 0; i < count; ++i)
						{
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
							int fy = widget->rect.height / 2 - base_size / 2;

							if (coverFlow->boundaryAlign)
							{
								int max_neighbor_item = ((widget->rect.width / base_size) - 1) / 2;
								int max_width_item = (widget->rect.width / base_size);

								//fy = 0;
								fy = i * base_size;

								if (max_neighbor_item == 0)
									max_neighbor_item++;

								fy -= (base_size * coverFlow->focusIndex);
								//if (coverFlow->focusIndex == (count - 1))
								//{
								//	fy -= base_size * (coverFlow->focusIndex - 0);
								//}
								//else if (coverFlow->focusIndex >= 0) //>= max_neighbor_item)
								//{
								//	if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_width_item))
								//		fy = widget->rect.height - (count * base_size);
								//	else
								//		fy -= base_size * (coverFlow->focusIndex - 0);
								//}
								//else
								//	fy = 0;
							}
							else
							{
								//fy = base_size - (coverFlow->focusIndex * base_size);
								fy = ((widget_size - base_size) / 2) - ((coverFlow->focusIndex - i) * base_size);
							}




							if (coverFlow->overlapsize > 0)
							{
								int fix;
								if (way > 0)
								{
									fix = (base_size - (int)(base_size * step));
									fix *= ((coverFlow->inc > 0) ? (1) : (-1));
								}
								else
								{
									fix = (int)(base_size * step);
									fix *= ((coverFlow->inc > 0) ? (-1) : (1));
								}

								if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
								{
									if (coverFlow->inc > 0)
										fix += base_size;
									else
										fix -= base_size;
								}

								//fy += i * base_size;

								//fix the slide start position not sync move last position
								if ((i == 0) && (coverFlow->movelog != 0))
								{
									if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
									{
										if (coverFlow->inc > 0)
										{
											if ((fy + fix - coverFlow->overlapsize) < coverFlow->movelog)
												wrong_pos_check = true;
										}
										else
										{
											if ((fy + fix - coverFlow->overlapsize) > coverFlow->movelog)
												wrong_pos_check = true;
										}
									}
									else
									{
										if (coverFlow->inc > 0)
										{
											if ((fy + fix - coverFlow->overlapsize) > coverFlow->movelog)
												wrong_pos_check = true;
										}
										else
										{
											if ((fy + fix - coverFlow->overlapsize) < coverFlow->movelog)
												wrong_pos_check = true;
										}
									}

									if (!wrong_pos_check)
										coverFlow->movelog = 0;

									if (wrong_pos_check)
									{
										coverFlow->frame++;
										i = count;

										if (coverFlow->frame >= (coverFlow->totalframe + 1))
											wrong_pos_check = false;

										continue;
									}
								}

								//////////////////////////////////////////////
								///// Vertical-NonCycle Overlap Split mode
								//////////////////////////////////////////////

								if ((coverFlow->split > 0) && (abs(coverFlow->inc) == base_size) && (coverFlow->frame > coverFlow->totalframe))
								{
									if (i == (count - 1))
									{
										coverFlow->frame = ((coverFlow->totalframe * COVERFLOW_MIN_SFRAME_PERCENT_SPLIT) / 100);

										i = count;
										continue;
									}
								}

								if (coverFlow->split > 0)
								{
									int sd = ((coverFlow->totalframe * 6) / 10);
									float dev = (coverFlow->totalframe - sd);

									float pi = 3.1415926;
									float pos_pi = ((float)(coverFlow->frame - sd) / dev) * pi;


									if ((coverFlow->frame >= sd) && (coverFlow->frame <= coverFlow->totalframe))
									{
										bool check_inside = false;

										if (((fy + fix) >= 0) && ((fy + fix) <= widget_size))
											check_inside = true;
										else if (((fy + fix + base_size) >= 0) && ((fy + fix + base_size) <= widget_size))
											check_inside = true;

										if (check_inside)
										{
											split_shift = (int)((i - coverFlow->focusIndex) * coverFlow->split * sin(pos_pi));

											if ((coverFlow->frame == coverFlow->totalframe) && (split_shift == 0))
											{
												split_shift = (int)(coverFlow->split * 0.3);
											}
										}

										//printf("split_shift %d, frame %d, inc %d\n", split_shift, coverFlow->frame, coverFlow->inc);
									}
									else
										split_shift = 0;
								}
								else
									split_shift = 0;

								if (coverFlow->frame <= coverFlow->totalframe)
								{
									ituWidgetSetY(child, fy + fix - coverFlow->overlapsize + split_shift);
									//printf("[frame %d][C %d][Y %d]\n", coverFlow->frame, i, fy + fix - coverFlow->overlapsize);
								}
							}
							else
							{
								int fix;
								if (way > 0)
								{
									fix = (base_size - (int)(base_size * step));
									fix *= ((coverFlow->inc > 0) ? (1) : (-1));
								}
								else
								{
									fix = (int)(base_size * step);
									fix *= ((coverFlow->inc > 0) ? (-1) : (1));
								}

								if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
								{
									if (coverFlow->inc > 0)
										fix += base_size;
									else
										fix -= base_size;
								}

								//fy += i * child->rect.height;

								//fix the slide start position not sync move last position
								if ((i == 0) && (coverFlow->movelog != 0))
								{
									if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
									{
										if (coverFlow->inc > 0)
										{
											if ((fy + fix) < coverFlow->movelog)
												wrong_pos_check = true;
										}
										else
										{
											if ((fy + fix) > coverFlow->movelog)
												wrong_pos_check = true;
										}
									}
									else
									{
										if (coverFlow->inc > 0)
										{
											if ((fy + fix) > coverFlow->movelog)
												wrong_pos_check = true;
										}
										else
										{
											if ((fy + fix) < coverFlow->movelog)
												wrong_pos_check = true;
										}
									}

									if (!wrong_pos_check)
										coverFlow->movelog = 0;

									if (wrong_pos_check) //bless debug for sc
									{
										coverFlow->frame++;
										i = count;

										if (coverFlow->frame >= (coverFlow->totalframe + 1))
											wrong_pos_check = false;

										continue;
									}
								}

								//////////////////////////////////////////////
								///// Vertical-NonCycle Split mode
								//////////////////////////////////////////////

								if ((coverFlow->split > 0) && (abs(coverFlow->inc) == base_size) && (coverFlow->frame > coverFlow->totalframe))
								{
									if (i == (count - 1))
									{
										coverFlow->frame = ((coverFlow->totalframe * COVERFLOW_MIN_SFRAME_PERCENT_SPLIT) / 100);

										i = count;
										continue;
									}
								}

								if (coverFlow->split > 0)
								{
									int sd = ((coverFlow->totalframe * 6) / 10);
									float dev = (coverFlow->totalframe - sd);

									float pi = 3.1415926;
									float pos_pi = ((float)(coverFlow->frame - sd) / dev) * pi;


									if ((coverFlow->frame >= sd) && (coverFlow->frame <= coverFlow->totalframe))
									{
										bool check_inside = false;

										if (((fy + fix) >= 0) && ((fy + fix) <= widget_size))
											check_inside = true;
										else if (((fy + fix + base_size) >= 0) && ((fy + fix + base_size) <= widget_size))
											check_inside = true;

										if (check_inside)
										{
											split_shift = (int)((i - coverFlow->focusIndex) * coverFlow->split * sin(pos_pi));

											if ((coverFlow->frame == coverFlow->totalframe) && (split_shift == 0))
											{
												split_shift = (int)(coverFlow->split * 0.3);
											}
										}

										//printf("split_shift %d, frame %d, inc %d\n", split_shift, coverFlow->frame, coverFlow->inc);
									}
									else
										split_shift = 0;
								}
								else
									split_shift = 0;

								if (coverFlow->frame <= coverFlow->totalframe)
								{
									ituWidgetSetY(child, fy + fix + split_shift);
									//printf("[frame %d][C %d][Y %d]\n", coverFlow->frame, i, fy + fix);

									//for goose do more one frame at last frame (slow down)
									if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING) && (coverFlow->frame == (coverFlow->totalframe - 1)) && (count <= COVERFLOW_SMOOTH_LASTFRAME_COUNT))
									{
										if (coverFlow->org_totalframe == coverFlow->totalframe)
										{
											coverFlow->totalframe *= 2;
											coverFlow->frame = coverFlow->totalframe - 2;
										}
									}
								}
							}
						}//for
						*/
					}
                }
            }
            else
            {
				// <<< ITU_EVENT_TIMER >>>
				// <<< Horizontal >>>
				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
				{
					int index, count2;
					float step = 0.0;
					int local_inc = coverFlow->inc;
					int local_fi = coverFlow->focusIndex;

					while (local_inc > (base_size / 2))
					{
						local_inc -= base_size;
					}
					while (local_inc < (-1 * base_size / 2))
					{
						local_inc += base_size;
					}

					//bbbbb
					// cubic ease out: y = (x - 1)^3 + 1
					//step = step - 1;
					//step = step * step * step + 1;

					//workaround for wrong left-side display with hide child
					count2 = count / 2 + 1 - ((coverFlow->inc > 0) ? (1) : (0));
					//count2 = count / 2 + 1;

					index = local_fi;

					if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
					{
						int fx = widget->rect.width / 2 - base_size / 2;
						local_inc = coverFlow->inc;
						step = 1.0 - CoverFlowAniStepCal(coverFlow);

						//from focus to right side all
						for (i = 0; i < count2; ++i)
						{
							int ci = ((index >(count - 1)) ? (0) : (index));
							int fix = (int)(local_inc * step);
							int local_fx = fx + i * base_size;
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, ci);

							//try to fix the next child is wrong position under cycle mode
							if (child)
							{
								int check_item = (((ci + 1) > (count - 1)) ? (0) : (ci + 1));
								ITUWidget* child_next = CoverFlowGetVisibleChild(coverFlow, check_item);

								if (child_next)
								{
									if ((child_next->rect.x < child->rect.x) && (i < (count2 - 1)))
										ituCoverFlowSetXY(coverFlow, check_item, child->rect.x + base_size, __LINE__);
								}
							}

							if (((local_fx + fix) < child->rect.x) && (local_inc > 0))
							{
								int ff = 1;
								while ((local_fx + fix) < child->rect.x)
								{
									step = ((float)(coverFlow->frame + ff) / (float)coverFlow->totalframe);
									fix = (int)(local_inc * step);
									ff++;
								}
								coverFlow->frame += (ff - 1);
								//break;
							}
							else if (((local_fx + fix) > child->rect.x) && (local_inc < 0) && (i < (count2 - 1)))
							{
								int ff = 1;
								while ((local_fx + fix) > child->rect.x)
								{
									step = ((float)(coverFlow->frame + ff) / (float)coverFlow->totalframe);
									fix = (int)(local_inc * step);
									ff++;
								}
								coverFlow->frame += (ff - 1);
								//break;
							}
							local_fx += fix;

							ituCoverFlowSetXY(coverFlow, ci, local_fx, __LINE__);
							index = ci + 1;
						}

						count2 = count - count2;
						index = local_fi - 1;
						fx = fx - base_size;
						//from left side all to focus -1
						for (i = 0; i < count2; ++i)
						{
							int ci = ((index < 0) ? (count - 1) : (index));
							int fix = (int)(local_inc * step);
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, ci);
							int local_fx = fx;
							local_fx -= i * base_size;
							local_fx += fix;

							ituCoverFlowSetXY(coverFlow, ci, local_fx, __LINE__);
							index = ci - 1;

							//fix for user use bad design (item count less than display size + 2)
							if ((coverFlow->inc > 0) && (count2 <= 1))
							{
								int prev2 = ((index < 0) ? (count - 1) : (index));
								child = CoverFlowGetVisibleChild(coverFlow, prev2);
								if (child->rect.x > widget->rect.width)
									count2++;
							}
						}
					}
					else
					{
						int local_frame = coverFlow->frame;
						step = 1.0 - CoverFlowAniStepCal(coverFlow);

						if (coverFlow->temp1 < 1)
						{
							local_inc = ((coverFlow->inc > 0) ? (base_size) : (-base_size));
							step = (float)coverFlow->frame / (float)coverFlow->totalframe;
						}
						else
						{
							local_inc = coverFlow->inc * -1;
						}

						//printf("frame %d fix %d\n", coverFlow->frame, (int)(local_inc * step));

						for (i = 0; i < count2; ++i)
						{
							int ci = ((index >(count - 1)) ? (0) : (index));
							int fix = (int)(local_inc * step);
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, ci);
							bool layout_check = true;
							int fx = widget->rect.width / 2 - base_size / 2;

							//count = 2, when right way(inc > 0) the right child will be at left side
							if ((coverFlow->inc > 0) && (count == 2))
								fx -= i * base_size;
							else
								fx += i * base_size;
							//debugging layout_check (something wrong)
							//if (((fx > 0) && (child->rect.x > 0)) || ((fx < 0) && (child->rect.x < 0)))
							//	layout_check = true;

							if (((fx + fix) > child->rect.x) && (local_inc < 0) && layout_check && (coverFlow->temp1 < 1))
							{
								int ff = 1;
								while (((fx + fix) > child->rect.x) && (local_frame < coverFlow->totalframe))
								{
									local_frame++;
									step = (float)local_frame / (float)coverFlow->totalframe;
									fix = (int)(local_inc * step);
								}
								step = (float)local_frame / (float)coverFlow->totalframe;
							}
							else if (((fx + fix) < child->rect.x) && (local_inc > 0) && layout_check && (coverFlow->temp1 < 1))
							{
								int ff = 1;
								while (((fx + fix) < child->rect.x) && (local_frame < coverFlow->totalframe))
								{
									local_frame++;
									step = (float)local_frame / (float)coverFlow->totalframe;
									fix = (int)(local_inc * step);
								}
								step = (float)local_frame / (float)coverFlow->totalframe;
							}

							fx += fix;
							ituCoverFlowSetXY(coverFlow, ci, fx, __LINE__);
							printf("[%d] [%d]\n", ci, fx);
							index = ci + 1;
						}

						count2 = count - count2;
						index = local_fi - 1;
						for (i = 0; i < count2; ++i)
						{
							int ci = ((index < 0) ? (count - 1) : (index));
							int fix = (int)(local_inc * step);
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, ci);
							int fx = widget->rect.width / 2 - base_size / 2;
							fx -= base_size;
							fx -= i * base_size;
							fx += fix;
							ituCoverFlowSetXY(coverFlow, ci, fx, __LINE__);
							index = ci - 1;

							//fix for user use bad design (item count less than display size + 2)
							if ((coverFlow->inc > 0) && (count2 <= 1))
							{
								int prev2 = ((index < 0) ? (count - 1) : (index));
								child = CoverFlowGetVisibleChild(coverFlow, prev2);
								if (child->rect.x > widget->rect.width)
									count2++;
							}
						}
					}

					//for goose do more one frame at last frame (slow down)
					if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING) && (coverFlow->frame == (coverFlow->totalframe - 1)) && (count <= COVERFLOW_SMOOTH_LASTFRAME_COUNT))
					{
						if (coverFlow->org_totalframe == coverFlow->totalframe)
						{
							coverFlow->totalframe *= 2;
							coverFlow->frame = coverFlow->totalframe - 2;
						}
					}

					result = true;
				}
				else if (widget->flags & ITU_BOUNCING)
                {
					// <<< ITU_EVENT_TIMER >>>
					// <<< Horizontal / Non-Cycle >>>
					float step = (float)coverFlow->frame / (float)coverFlow->totalframe;
					step = step - 1;
					step = step * step * step + 1;

                    //printf("frame=%d step=%f\n", coverFlow->frame, step);
                    for (i = 0; i < count; ++i)
                    {
                        ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
                        //int fx = widget->rect.width / 2 - base_size / 2;
						//fx -= base_size * coverFlow->focusIndex;
						int fx;
						if (coverFlow->coverFlowFlags & ITU_BOUNCE_1)
						{
							fx = 0;
						}
						else if (coverFlow->coverFlowFlags & ITU_BOUNCE_2)
						{
							fx = widget_size - (count * base_size);
						}
						else
						{
							if (coverFlow->focusIndex == 0)
								fx = 0;
							else
								fx = widget_size - (coverFlow->focusIndex + 1) * base_size;
						}


						fx += i * base_size;
                        fx += (int)(coverFlow->inc * step);

						ituCoverFlowSetXY(coverFlow, i, fx - coverFlow->overlapsize, __LINE__);
                    }
                }
                else
                {
					// <<< ITU_EVENT_TIMER >>>
					// <<< Horizontal / Non-Cycle >>>
					bool wrong_pos_check = true;

					//if (coverFlow->boundaryAlign == 0)
					//	wrong_pos_check = false;
					//working now
					while (wrong_pos_check)
					{
						int local_movelog = coverFlow->movelog;
						int local_focusindex = coverFlow->focusIndex;
						int current_first_child_pos;
						int cal_init_pos = 0;
						float step = 0.0;
						ITUWidget* childlast = CoverFlowGetVisibleChild(coverFlow, count - 1);
						int childlast_tor_dist = 0;
						int tor_dist = (base_size / ((coverFlow->bounceRatio > 0) ? (coverFlow->bounceRatio) : (base_size)));

						if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
						{
							childlast_tor_dist = (widget_size - (childlast->rect.y + base_size));
						}
						else
						{
							childlast_tor_dist = (widget_size - (childlast->rect.x + base_size));
						}

						wrong_pos_check = false;
						step = CoverFlowAniStepCal(coverFlow);

						/*if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
						{
							step = (float)(coverFlow->totalframe - coverFlow->frame) / (float)coverFlow->totalframe;
						}
						else
						{
							step = (float)coverFlow->frame / (float)coverFlow->totalframe;
						}
						step = step - 1;
						step = step * step * step + 1;*/

						for (i = 0; i < count; i++)
						{
							int fix = 0;
							int temp_local_cal = 0;
							int child_rect_pos;
							int childlast_rect_pos;
							bool align_start_side = true;
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);

							if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
							{
								child_rect_pos = child->rect.y;
								childlast_rect_pos = childlast->rect.y;
							}
							else
							{
								child_rect_pos = child->rect.x;
								childlast_rect_pos = childlast->rect.x;
							}

							if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
							{
								fix = (base_size - (int)((float)(base_size)* step));
								fix *= ((coverFlow->inc > 0) ? (1) : (-1));
							}
							else //Undragging mode
							{
								if (abs(coverFlow->inc) >= (base_size / 2))
								{
									fix = (base_size - (int)((float)(base_size)* step));

									if (coverFlow->inc < 0)
									{
										if (fix > 0)
											fix *= -1;
									}
								}
								else
								{
									fix = (base_size - (int)((float)(base_size)* step));
									fix *= ((coverFlow->inc > 0) ? (1) : (-1));
								}
							}

							if (coverFlow->boundaryAlign)
							{
								if (i == 0)
								{
									bool check_position = true;

									if ((widget_size - (childlast_rect_pos + base_size)) >= 0)
									{
										align_start_side = false;
									}
									else
									{
										if (coverFlow->temp3 == ITU_BOUNCE_2)
										{
											if (coverFlow->inc < 0)
											{
												//if (fix > 0)
												//	fix = fix - base_size;
											}
										}

										align_start_side = true;
									}

									if (align_start_side)
									{
										while (check_position)
										{
											if (abs((0 - (base_size * local_focusindex)) - child_rect_pos) <= base_size)
												check_position = false;
											else
											{
												local_focusindex += ((coverFlow->inc > 0) ? (-1) : (1));

												if (local_focusindex < 0)
												{
													local_focusindex = 0;
													check_position = false;
												}
												else if (local_focusindex > (count - 1))
												{
													local_focusindex = count - 1;
													check_position = false;
												}
											}
										}

										cal_init_pos = 0 - (base_size * local_focusindex);

										if (coverFlow->temp3 == ITU_BOUNCE_2)
										{
											//coverFlow->temp3 = 0;
											//cal_init_pos += (base_size * 1);
										}

										//printf("align [start] side mode [frame %d] [cal init pos %d]\n", coverFlow->frame, cal_init_pos);
									}
									else
									{
										int focusIndexMax = (count - 1) - (((widget_size / base_size) / 2) + 1);
										int align_fix_dist = 0;

										while (check_position)
										{
											if (abs((widget_size - (count * base_size)) + align_fix_dist - child_rect_pos) <= base_size)
												check_position = false;
											else
											{
												local_focusindex += ((coverFlow->inc > 0) ? (-1) : (1));

												if (local_focusindex < 0)
												{
													align_fix_dist = 0;
													local_focusindex = 0;
													check_position = false;
												}
												else if (local_focusindex >(count - 1))
												{
													align_fix_dist = 0;
													local_focusindex = count - 1;
													check_position = false;
												}

												if (local_focusindex < focusIndexMax)
													align_fix_dist = (focusIndexMax - local_focusindex) * base_size;
											}
										}
										cal_init_pos = widget_size - (count * base_size) + align_fix_dist;
										//printf("align [end] side mode [frame %d]\n", coverFlow->frame);
									}
								}
								else
								{
									cal_init_pos += base_size;
								}

								//temp_local_cal = (widget_size - (cal_init_pos + fix + (count * base_size)));
								temp_local_cal = (widget_size - (childlast_rect_pos + base_size + fix));
								
								if ((coverFlow->temp3 == ITU_BOUNCE_2) || (widget_size == base_size))
								{
									//coverFlow->temp3 = 0;
									temp_local_cal = 0;
								}


								if ((temp_local_cal > (base_size / coverFlow->bounceRatio)) && ((childlast_rect_pos + base_size) <= widget_size))
								{
									bool local_debug_print = false;
									i = count;

									if (coverFlow->frame < coverFlow->totalframe)
									{
										temp_local_cal = ((childlast_rect_pos + base_size) - widget_size);
										if (temp_local_cal > (base_size / 5))
										{
											for (i = 0; i < count; i++)
											{
												child = CoverFlowGetVisibleChild(coverFlow, i);
												ituCoverFlowSetXY(coverFlow, i, child->rect.x - coverFlow->overlapsize - (temp_local_cal / 5), __LINE__);
											}
										}
										else
										{
											for (i = 0; i < count; i++)
											{
												child = CoverFlowGetVisibleChild(coverFlow, i);
												ituCoverFlowSetXY(coverFlow, i, child->rect.x - coverFlow->overlapsize - temp_local_cal, __LINE__);
											}
										}
									}

									if (local_debug_print)
										printf("[Temp local cal][Continue][%d]\n", coverFlow->frame);

									continue;
								}


								if (coverFlow->frame <= coverFlow->totalframe)
								{
									bool local_debug_print = false;

									if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
									{
										if (((coverFlow->inc > 0) && ((cal_init_pos + fix) > child_rect_pos)) || ((coverFlow->inc < 0) && ((cal_init_pos + fix) < child_rect_pos)))
										{
											///check the first and last child position is outside or not
											int base_bounce = ((coverFlow->bounceRatio <= 0) ? (base_size) : (coverFlow->bounceRatio));
											bool check_outside = false;

											if (i == 0)
											{
												if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
												{
													if (child->rect.y > (base_size / base_bounce))
														check_outside = true;
												}
												else
												{
													if (child->rect.x > (base_size / base_bounce))
														check_outside = true;
												}
											}
											else if (i == (count - 1))
											{
												if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
												{
													if ((child->rect.y + base_size) < (widget_size - (base_size / base_bounce)))
														check_outside = true;
												}
												else
												{
													if ((child->rect.x + base_size) < (widget_size - (base_size / base_bounce)))
														check_outside = true;
												}
											}

											if ((i == 0) && local_debug_print)
												printf("[D][%d] [frame %d]\n", cal_init_pos + fix, coverFlow->frame);

											if (!check_outside)
											{
												ituCoverFlowSetXY(coverFlow, i, cal_init_pos - coverFlow->overlapsize + fix, __LINE__);
											}
											else if (i == 0)
											{
												i = count;
												continue;
											}
										}
										else
										{
											if ((i == 0) && local_debug_print)
											{
												if (coverFlow->inc > 0)
													printf("[%d + %d] > [%d] [frame %d]\n", cal_init_pos, fix, child_rect_pos, coverFlow->frame);
												else
													printf("[%d + %d] < [%d] [frame %d]\n", cal_init_pos, fix, child_rect_pos, coverFlow->frame);
											}
										}
									}
									else //Undragging mode
									{
										if (((coverFlow->inc > 0) && ((cal_init_pos + fix) < child_rect_pos)) || ((coverFlow->inc < 0) && ((cal_init_pos + fix) > child_rect_pos)))
										{
											if ((i == 0) && local_debug_print)
											{
												if (coverFlow->inc > 0)
													printf("[+][D][%d + %d] < [%d] [frame %d]\n", cal_init_pos, fix, child_rect_pos, coverFlow->frame);
												else
													printf("[-][D][%d + %d] > [%d] [frame %d]\n", cal_init_pos, fix, child_rect_pos, coverFlow->frame);
											}

											ituCoverFlowSetXY(coverFlow, i, cal_init_pos - coverFlow->overlapsize + fix, __LINE__);
										}
										else
										{
											if ((i == 0) && local_debug_print)
											{
												if (coverFlow->inc > 0)
													printf("==+==[%d + %d] < [%d] [frame %d]\n", cal_init_pos, fix, child_rect_pos, coverFlow->frame);
												else
													printf("==-==[%d + %d] > [%d] [frame %d]\n", cal_init_pos, fix, child_rect_pos, coverFlow->frame);
											}
										}
									}
								}
							}
							else //default center align mode
							{
								cal_init_pos = ((widget_size - base_size) / 2) - ((coverFlow->focusIndex - i) * base_size);

								if (coverFlow->frame <= coverFlow->totalframe)
								{
									bool local_debug_print = false;

									if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
									{
										if (((coverFlow->inc > 0) && ((cal_init_pos + fix) > child_rect_pos)) || ((coverFlow->inc < 0) && ((cal_init_pos + fix) < child_rect_pos)))
										{
											if ((i == 0) && local_debug_print)
												printf("[D][%d] [frame %d]\n", cal_init_pos + fix, coverFlow->frame);

											ituCoverFlowSetXY(coverFlow, i, cal_init_pos - coverFlow->overlapsize + fix, __LINE__);
										}
										else
										{
											if ((i == 0) && local_debug_print)
											{
												if (coverFlow->inc > 0)
													printf("[%d + %d] > [%d] [frame %d]\n", cal_init_pos, fix, child_rect_pos, coverFlow->frame);
												else
													printf("[%d + %d] < [%d] [frame %d]\n", cal_init_pos, fix, child_rect_pos, coverFlow->frame);
											}
										}
									}
									else //Undragging mode
									{
										if (((coverFlow->inc > 0) && ((cal_init_pos + fix) < child_rect_pos)) || ((coverFlow->inc < 0) && ((cal_init_pos + fix) > child_rect_pos)))
										{
											if ((i == 0) && local_debug_print)
												printf("[D][%d] [frame %d]\n", cal_init_pos + fix, coverFlow->frame);

											ituCoverFlowSetXY(coverFlow, i, cal_init_pos - coverFlow->overlapsize + fix, __LINE__);
										}
										else
										{
											if ((i == 0) && local_debug_print)
											{
												if (coverFlow->inc > 0)
													printf("[%d + %d] < [%d] [frame %d]\n", cal_init_pos, fix, child_rect_pos, coverFlow->frame);
												else
													printf("[%d + %d] > [%d] [frame %d]\n", cal_init_pos, fix, child_rect_pos, coverFlow->frame);
											}

											////////// try to save more frame when wrong position 2018/4/12
											if (i == 0)
											{
												int savecount = 0;
												bool recheck = true;
												while (recheck)
												{
													coverFlow->frame++;
													step = CoverFlowAniStepCal(coverFlow);

													if (coverFlow->frame < coverFlow->totalframe)
													{
														if (abs(coverFlow->inc) >= (base_size / 2))
														{
															fix = (base_size - (int)((float)(base_size)* step));

															if (coverFlow->inc < 0)
															{
																if (fix > 0)
																	fix *= -1;
															}
														}
														else
														{
															fix = (base_size - (int)((float)(base_size)* step));
															fix *= ((coverFlow->inc > 0) ? (1) : (-1));
														}

														if (((coverFlow->inc > 0) && ((cal_init_pos + fix) < child_rect_pos)) || ((coverFlow->inc < 0) && ((cal_init_pos + fix) > child_rect_pos)))
														{
															coverFlow->frame--;
															i = count;
															recheck = false;
														}
														else
														{
															savecount++;
															//printf("[Coverflow][Undragging][Position sync save frame %d]\n", savecount);
														}
													}
													else
													{
														recheck = false;
													}
												}
											}
											/////////////////////////////////
										}
									}
								}
							}

							if (coverFlow->frame <= coverFlow->totalframe)
							{
								//for goose do more one frame at last frame (slow down)
								if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING) && (coverFlow->frame == (coverFlow->totalframe - 1)) && (count <= COVERFLOW_SMOOTH_LASTFRAME_COUNT))
								{
									if (coverFlow->org_totalframe == coverFlow->totalframe)
									{
										coverFlow->totalframe *= 2;
										coverFlow->frame = coverFlow->totalframe - 2;
									}
								}
							}
						}
						

						if (coverFlow->focusIndex < 0)
							coverFlow->focusIndex = 0;
						else if (coverFlow->focusIndex > (count - 1))
							coverFlow->focusIndex = count - 1;

						/*
						for (i = 0; i < count; ++i)
						{
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
							int fx = widget->rect.width / 2 - base_size / 2;

							if (i == 0)
							{
								current_first_child_pos = child->rect.x;
								local_focusindex += (((coverFlow->inc < 0) ? (1) : (-1)) * ((coverFlow->movelog / base_size) + 1));
							}

							if (coverFlow->boundaryAlign)
							{
								int max_neighbor_item = ((widget->rect.width / base_size) - 1) / 2;
								int max_width_item = (widget->rect.width / base_size);

								fx = i * base_size;

								if (max_neighbor_item == 0)
									max_neighbor_item++;

								if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
									fx -= (base_size * local_focusindex);
								else
									fx -= (base_size * coverFlow->focusIndex);
							}
							else
							{
								//fx = base_size - (coverFlow->focusIndex * base_size);
								fx = ((widget_size - base_size) / 2) - ((coverFlow->focusIndex - i) * base_size);
							}

							if (coverFlow->overlapsize > 0)
							{
								int fix;

								if (way > 0)
								{
									fix = (base_size - (int)(base_size * step));
									fix *= ((coverFlow->inc > 0) ? (1) : (-1));
								}
								else
								{
									fix = (int)(base_size * step);
									fix *= ((coverFlow->inc > 0) ? (-1) : (1));
								}

								//if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
								//{
								//	if (coverFlow->inc > 0)
								//		fix += base_size;
								//	else
								//		fix -= base_size;
								//}

								//fx += i * base_size;

								//fix the slide start position not sync move last position
								if ((i == 0) && (coverFlow->movelog != 0))
								{
									if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
									{
										if (coverFlow->inc > 0)
										{
											if ((fx + fix - coverFlow->overlapsize) < coverFlow->movelog)
												wrong_pos_check = true;
										}
										else
										{
											if ((fx + fix - coverFlow->overlapsize) > coverFlow->movelog)
												wrong_pos_check = true;
										}
									}
									else
									{
										if (coverFlow->inc > 0)
										{
											if ((fx + fix - coverFlow->overlapsize) > coverFlow->movelog)
												wrong_pos_check = true;
										}
										else
										{
											if ((fx + fix - coverFlow->overlapsize) < coverFlow->movelog)
												wrong_pos_check = true;
										}
									}
									//0607
									//if (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING))
									//	wrong_pos_check = false;

									if (!wrong_pos_check)
										coverFlow->movelog = 0;

									if (wrong_pos_check)
									{
										coverFlow->frame++;
										i = count;

										if (coverFlow->frame >= (coverFlow->totalframe + 1))
											wrong_pos_check = false;

										continue;
									}
								}

								//////////////////////////////////////////////
								///// Horizontal-NonCycle Overlap Split mode
								//////////////////////////////////////////////

								if ((coverFlow->split > 0) && (abs(coverFlow->inc) == base_size) && (coverFlow->frame > coverFlow->totalframe))
								{
									if (i == (count - 1))
									{
										coverFlow->frame = ((coverFlow->totalframe * COVERFLOW_MIN_SFRAME_PERCENT_SPLIT) / 100);

										i = count;
										continue;
									}
								}

								if (coverFlow->split > 0)
								{
									int sd = ((coverFlow->totalframe * 6) / 10);
									float dev = (coverFlow->totalframe - sd);

									float pi = 3.1415926;
									float pos_pi = ((float)(coverFlow->frame - sd) / dev) * pi;


									if ((coverFlow->frame >= sd) && (coverFlow->frame <= coverFlow->totalframe))
									{
										bool check_inside = false;

										if (((fx + fix) >= 0) && ((fx + fix) <= widget_size))
											check_inside = true;
										else if (((fx + fix + base_size) >= 0) && ((fx + fix + base_size) <= widget_size))
											check_inside = true;

										if (check_inside)
										{
											split_shift = (int)((i - coverFlow->focusIndex) * coverFlow->split * sin(pos_pi));

											if ((coverFlow->frame == coverFlow->totalframe) && (split_shift == 0))
											{
												split_shift = (int)(coverFlow->split * 0.3);
											}
										}

										//printf("split_shift %d, frame %d, inc %d\n", split_shift, coverFlow->frame, coverFlow->inc);
									}
									else
										split_shift = 0;
								}
								else
									split_shift = 0;

								if (coverFlow->frame <= coverFlow->totalframe)
								{
									ituWidgetSetX(child, fx + fix - coverFlow->overlapsize + split_shift);
									//printf("[frame %d][C %d][X %d]\n", coverFlow->frame, i, fx + fix - coverFlow->overlapsize);
								}
							}
							else
							{
								int fix;

								if (way > 0)
								{
									fix = (base_size - (int)((float)(base_size)* step));
									fix *= ((coverFlow->inc > 0) ? (1) : (-1));
								}
								else
								{
									//int factor_mod = (base_size + ((abs(coverFlow->movelog) / base_size) * base_size));
									fix = (int)(base_size * step);
									fix *= ((coverFlow->inc > 0) ? (-1) : (1));

									if (i == 0)
										printf("<<< fix value %d  [Frame %d] >>>\n", fix, coverFlow->frame);
								}

								if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
								{
									if (coverFlow->inc > 0)
										fix += base_size;
									else
										fix -= base_size;

									//if (coverFlow->inc > 0)
									//	fix += (base_size + ((abs(coverFlow->movelog) / base_size) * base_size));
									//else
									//	fix -= (base_size + ((abs(coverFlow->movelog) / base_size) * base_size));

									//if (i == 0)
									//	printf("mlog %d, factor %d\n", coverFlow->movelog, (abs(coverFlow->movelog) / base_size));
								}

								while (local_movelog < -base_size)
								{
									local_movelog += base_size;
								}

								while (local_movelog > base_size)
								{
									local_movelog -= base_size;
								}

								if (i == 0)
									printf("[F0x %d][real_fx %d]\n", fx, current_first_child_pos);

								//fx += i * child->rect.width;

								//fix the slide start position not sync move last position
								if ((i == 0) && (local_movelog != 0))
								{
									if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
									{
										if (coverFlow->inc > 0)
										{
											if ((fx + fix) < local_movelog)
												wrong_pos_check = true;

											if (wrong_pos_check)
												printf("==>> fixing[%d + %d] > [%d].... Frame %d\n", fx, fix, local_movelog, coverFlow->frame);
										}
										else
										{
											if ((fx + fix) > local_movelog)
												wrong_pos_check = true;

											if (wrong_pos_check)
												printf("==>> fixing[%d + %d] < [%d].... Frame %d\n", fx, fix, local_movelog, coverFlow->frame);
										}
									}
									else
									{
										if (coverFlow->inc > 0)
										{
											if ((fx + fix) > local_movelog)
												wrong_pos_check = true;
										}
										else
										{
											if ((fx + fix) < local_movelog)
												wrong_pos_check = true;
										}
									}

									if (!wrong_pos_check)
									{
										//coverFlow->movelog = 0;
									}

									if (wrong_pos_check) //bless debug for sc
									{
										coverFlow->frame++;
										i = count;

										if (coverFlow->frame >= (coverFlow->totalframe + 1))
											wrong_pos_check = false;

										//when position fix too much and can't do any animation
										//redo animation after position sync
										if (!wrong_pos_check)
										{
											if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
											{
												fix = coverFlow->movelog;
												fix *= -1;
												//fix *= ((coverFlow->inc > 0) ? (-1) : (1));

												//if (coverFlow->inc > 0)
												//	fix += base_size;
												//else
												//	fix -= base_size;

												for (i = 0; i < count; i++)
												{
													ITUWidget* clocal = CoverFlowGetVisibleChild(coverFlow, i);
													int fxlocal = widget->rect.width / 2 - base_size / 2;

													if (coverFlow->boundaryAlign)
													{
														fxlocal = i * base_size;
														fxlocal -= (base_size * coverFlow->focusIndex);
													}
													else
													{
														fxlocal = ((widget_size - base_size) / 2) - ((coverFlow->focusIndex - i) * base_size);
													}
													ituWidgetSetX(clocal, fxlocal + fix);
													//ituWidgetSetX(clocal, clocal->rect.x + fix);
												}
											}
											coverFlow->frame = 0;
											coverFlow->movelog = 0;
										}

										continue;
									}
								}

								//////////////////////////////////////
								///// Horizontal-NonCycle Split mode
								//////////////////////////////////////

								if ((coverFlow->split > 0) && (abs(coverFlow->inc) == base_size) && (coverFlow->frame > coverFlow->totalframe))
								{
									if (i == (count - 1))
									{
										coverFlow->frame = ((coverFlow->totalframe * COVERFLOW_MIN_SFRAME_PERCENT_SPLIT) / 100);

										i = count;
										continue;
									}
								}

								if (coverFlow->split > 0)
								{
									int sd = ((coverFlow->totalframe * 6) / 10);
									float dev = (coverFlow->totalframe - sd);

									float pi = 3.1415926;
									float pos_pi = ((float)(coverFlow->frame - sd) / dev) * pi;


									if ((coverFlow->frame >= sd) && (coverFlow->frame <= coverFlow->totalframe))
									{
										bool check_inside = false;

										if (((fx + fix) >= 0) && ((fx + fix) <= widget_size))
											check_inside = true;
										else if (((fx + fix + base_size) >= 0) && ((fx + fix + base_size) <= widget_size))
											check_inside = true;

										if (check_inside)
										{
											split_shift = (int)((i - coverFlow->focusIndex) * coverFlow->split * sin(pos_pi));

											if ((coverFlow->frame == coverFlow->totalframe) && (split_shift == 0))
											{
												split_shift = (int)(coverFlow->split * 0.3);
												//printf("fixed!!!!!\n");
											}
										}

										//printf("split_shift %d, frame %d, inc %d\n", split_shift, coverFlow->frame, coverFlow->inc);
									}
									else
										split_shift = 0;
								}
								else
									split_shift = 0;

								if (coverFlow->frame <= coverFlow->totalframe)
								{
									ituWidgetSetX(child, fx + fix + split_shift);
									//if (i == 0)
									//printf("[frame %d][C %d][X %d]\n", coverFlow->frame, i, fx + fix);

									//for goose do more one frame at last frame (slow down)
									if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING) && (coverFlow->frame == (coverFlow->totalframe - 1)) && (count <= COVERFLOW_SMOOTH_LASTFRAME_COUNT))
									{
										if (coverFlow->org_totalframe == coverFlow->totalframe)
										{
											coverFlow->totalframe *= 2;
											coverFlow->frame = coverFlow->totalframe - 2;
										}
									}
								}
							}
						}//for
						*/

					}
                }
            }

            coverFlow->frame++;
			//printf("coverflow frame %d  fd %d\n", coverFlow->frame, coverFlow->focusIndex);

			//try motion
			if ((count > COVERFLOW_SMOOTH_LASTFRAME_COUNT) && (coverFlow->slideMaxCount > 2) && (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING))
			{
				int cycle_step = (coverFlow->slideMaxCount - coverFlow->slideCount);

				//printf("slidecount %d\n", coverFlow->slideCount);

				if ((coverFlow->totalframe == coverFlow->org_totalframe) && (cycle_step <= 2))
				{
					coverFlow->totalframe *= 2;
				}
				else if ((coverFlow->totalframe != coverFlow->org_totalframe) && (cycle_step > 2))
				{
					coverFlow->totalframe = coverFlow->org_totalframe;
				}

				coverFlow->frame += (cycle_step / 2);
				//printf("[%d]coverFlow->frame %d\n", coverFlow->slideCount, coverFlow->frame);
				//printf("%d\t", coverFlow->frame);

				//printf("%d\t", cycle_step / 3);
			}

			//for goose reset frame at last frame (slow down)
			//this case onlt use for count small (<= COVERFLOW_SMOOTH_LASTFRAME_COUNT)
			if ((coverFlow->frame > coverFlow->totalframe) && (count <= COVERFLOW_SMOOTH_LASTFRAME_COUNT))
			{
				if (coverFlow->org_totalframe != coverFlow->totalframe)
				{
					coverFlow->totalframe /= 2;
					coverFlow->frame = coverFlow->totalframe + 1;
				}
			}

			// <<< ITU_EVENT_TIMER >>>
			// <<< Frame END >>>
            if (coverFlow->frame > coverFlow->totalframe)
            {
				printf("\n");

				if (widget->flags & ITU_BOUNCING)
				{
					widget->flags &= ~ITU_BOUNCING;
					coverFlow->coverFlowFlags &= ~ITU_BOUNCE_1;
					coverFlow->coverFlowFlags &= ~ITU_BOUNCE_2;
				}

				if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
				{
					bool alignment_done = true;
					ITUWidget* child_1 = CoverFlowGetVisibleChild(coverFlow, 0);
					ITUWidget* child_2 = CoverFlowGetVisibleChild(coverFlow, count - 1);

					if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
					{
						if (child_1->rect.y > 0)
						{
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_ANYBOUNCE1;
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_ANYBOUNCE2;
							alignment_done = false;
							coverFlow->frame--;
						}
						else if ((child_2->rect.y + child_2->rect.height) < (widget->rect.height))
						{
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_ANYBOUNCE1;
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_ANYBOUNCE2;
							alignment_done = false;
							coverFlow->frame--;
						}
					}
					else
					{
						if ((child_1->rect.x) > 0)
						{
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_ANYBOUNCE1;
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_ANYBOUNCE2;
							alignment_done = false;
							coverFlow->frame--;
						}
						else if ((child_2->rect.x + child_2->rect.width) < widget->rect.width)
						{
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_ANYBOUNCE1;
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_ANYBOUNCE2;
							alignment_done = false;
							coverFlow->frame--;
						}
					}

					//to avoid bounce turn back not finish when frame end.
					if (alignment_done)
					{
						if (coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE)
						{
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_ANYBOUNCE;
						}

						if (coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE1)
						{
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_ANYBOUNCE1;

							if (COVERFLOW_DEBUG_FOCUSINDEX)
								printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, 0, __LINE__);

							coverFlow->focusIndex = 0;

							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_SLIDING;
							coverFlow->inc = 0;
							coverFlow->frame = 0;
						}
						else if (coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE2)
						{
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_ANYBOUNCE2;

							if (COVERFLOW_DEBUG_FOCUSINDEX)
								printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, count - 1, __LINE__);

							coverFlow->focusIndex = count - 1;

							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_SLIDING;
							coverFlow->inc = 0;
							coverFlow->frame = 0;
						}

						if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
						{
							//coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_SLIDING;
							//coverFlow->frame = 0;
							//coverFlow->inc = 0;

							if (coverFlow->slideCount < coverFlow->slideMaxCount)
							{
								coverFlow->frame = coverFlow->totalframe + 1;
							}
							else
							{
								coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_SLIDING;
								coverFlow->inc = 0;
								coverFlow->frame = 0;
							}
						}
					}
				}
				//here two case should be debug long time
                else if (coverFlow->inc > 0)
                {
					// <<< ITU_EVENT_TIMER >>>
					// <<< Frame END >>>
					// <<< INC > 0 >>>
					if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING) || !(widget->flags & ITU_DRAGGABLE))// || (!coverFlow->boundaryAlign))
					{
						if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
						{
							if (COVERFLOW_DEBUG_FOCUSINDEX)
								printf("[Debug][CoverFlow][FC %d ", coverFlow->focusIndex);

							if (coverFlow->focusIndex <= 0)
								coverFlow->focusIndex = count - 1;
							else
								coverFlow->focusIndex--;

							if (COVERFLOW_DEBUG_FOCUSINDEX)
								printf("to %d][%d]\n", coverFlow->focusIndex, __LINE__);

							ituCoverFlowFixFC(coverFlow);
							ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
							//ituCoverFlowOnCoverChanged(coverFlow, widget);
						}
						else
						{
							if (coverFlow->focusIndex > 0)
							{
								//fix the alignment wrong when sliding start at maximum focusindex then stop wrong layout when sliding done.
								int mod = 1;
								int max_touch_start_size_index = (count - (widget_size / base_size));
								/*if ((coverFlow->boundaryAlign) && (coverFlow->focusIndex == (count - 1)) && (widget_size > (base_size * 2)))
								{
									int max_touch_start_size_index = (count - (widget_size / base_size));
									mod = coverFlow->focusIndex - max_touch_start_size_index;
								}*/
								if ((coverFlow->boundaryAlign) && (coverFlow->focusIndex >= max_touch_start_size_index) && (widget_size > (base_size * 2)))
								{
									mod = coverFlow->focusIndex - max_touch_start_size_index + 1;
								}

								if (COVERFLOW_DEBUG_FOCUSINDEX)
									printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex - mod, __LINE__);

								coverFlow->focusIndex -= mod;
								ituCoverFlowFixFC(coverFlow);
								ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
								//ituCoverFlowOnCoverChanged(coverFlow, widget);
							}
						}
					}
					else if (widget->flags & ITU_UNDRAGGING)
					{
						if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE) && (coverFlow->temp1 < 1))
						//if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
						{
							if (COVERFLOW_DEBUG_FOCUSINDEX)
								printf("[Debug][CoverFlow][FC %d ", coverFlow->focusIndex);

							if (coverFlow->focusIndex <= 0)
								coverFlow->focusIndex = count - 1;
							else
								coverFlow->focusIndex--;

							if (COVERFLOW_DEBUG_FOCUSINDEX)
								printf("to %d][%d]\n", coverFlow->focusIndex, __LINE__);

							ituCoverFlowFixFC(coverFlow);
							ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
						}
					}
                }
                else
                {
					// <<< ITU_EVENT_TIMER >>>
					// <<< Frame END >>>
					// <<< INC < 0 >>>
					if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING) || !(widget->flags & ITU_DRAGGABLE))// || (!coverFlow->boundaryAlign))
					{
						if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
						{
							if (COVERFLOW_DEBUG_FOCUSINDEX)
								printf("[Debug][CoverFlow][FC %d ", coverFlow->focusIndex);

							if (coverFlow->focusIndex >= count - 1)
								coverFlow->focusIndex = 0;
							else
								coverFlow->focusIndex++;

							if (COVERFLOW_DEBUG_FOCUSINDEX)
								printf("to %d][%d]\n", coverFlow->focusIndex, __LINE__);

							ituCoverFlowFixFC(coverFlow);
							ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
							//ituCoverFlowOnCoverChanged(coverFlow, widget);
						}
						else
						{
							if (coverFlow->focusIndex < (count - 1))
							{
								if (COVERFLOW_DEBUG_FOCUSINDEX)
									printf("[Debug][CoverFlow][FC %d to %d][%d]\n", coverFlow->focusIndex, coverFlow->focusIndex + 1, __LINE__);

								coverFlow->focusIndex++;
								ituCoverFlowFixFC(coverFlow);
								ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
								//ituCoverFlowOnCoverChanged(coverFlow, widget);
							}
						}
					}
					else if (widget->flags & ITU_UNDRAGGING)
					{
						if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE) && (coverFlow->temp1 < 1))
						//if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
						{
							if (COVERFLOW_DEBUG_FOCUSINDEX)
								printf("[Debug][CoverFlow][FC %d ", coverFlow->focusIndex);

							if (coverFlow->focusIndex >= count - 1)
								coverFlow->focusIndex = 0;
							else
								coverFlow->focusIndex++;

							if (COVERFLOW_DEBUG_FOCUSINDEX)
								printf("to %d][%d]\n", coverFlow->focusIndex, __LINE__);

							ituCoverFlowFixFC(coverFlow);
							ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
						}
					}
                }

				if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING))// && coverFlow->boundaryAlign)
				{
					coverFlow->slideCount++;

					//if (((coverFlow->slideCount + 1) >= coverFlow->slideMaxCount) || (coverFlow->focusIndex < 0) || (coverFlow->focusIndex > (count - 1)))
					if (((coverFlow->slideCount + 1) >= coverFlow->slideMaxCount) || (coverFlow->focusIndex < 0) || (coverFlow->focusIndex >(count - 1)))
					{
						if (coverFlow->frame <= coverFlow->totalframe)
						{
							coverFlow->slideCount--;
						}
						else
						{
							coverFlow->slideCount = 0;
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_SLIDING;

							coverFlow->frame = 0;
							coverFlow->inc = 0;

							if (!(widget->flags & ITU_DRAGGING)) //for sliding then mousedown --> do not perform this(keep dragging for goose)
							{
								CoverFlowFlushQueue(widget, coverFlow, count, widget_size, base_size);
							}
						}
					}
					else //should do animation again when slidecount < slidemaxcount
					{
						if (widget->flags & ITU_DRAGGING)
						{
							coverFlow->inc = 0;
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_SLIDING;
						}

						if (coverFlow->inc)
						{
							coverFlow->frame = 1; //use 1 to make the animation frame more smooth
							return true;
						}
					}
				}
				else if (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP))
				{
					// <<< ITU_EVENT_TIMER >>>
					// <<< QUEUE process >>>
					//long long *customdata = (long long *)ituWidgetGetCustomData(coverFlow);
					int i = 0;
					bool no_queue = true;
					coverFlow->frame = 0;
					//do not change here 20170920
					//ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
					ituCoverFlowOnCoverChanged(coverFlow, widget);

					for (i = 0; i < COVERFLOW_MAX_PROCARR_SIZE; i++)
					{
						if (coverFlow->procArr[i] != 0)
						{
							coverFlow->procArr[i] = 0;

							if ((i + 1) < COVERFLOW_MAX_PROCARR_SIZE)
							{
								if (coverFlow->procArr[i + 1] != 0)
									no_queue = false;
							}

							break;
						}
					}

					if (no_queue)
					{
						coverFlow->inc = 0;
						widget->flags &= ~ITU_UNDRAGGING;
						//mark now, debug side effect for wheel update
						ituWidgetUpdate(widget, ITU_EVENT_LAYOUT, 0, 0, 0);
						//CoverFlowLayout(widget);
					}
					else
					{
						bool boundary_touch = false;

						if (coverFlow->boundaryAlign)
						{
							int max_neighbor_item = ((widget_size / base_size) - 1) / 2;

							coverFlow->slideCount = 0;

							if (max_neighbor_item == 0)
								max_neighbor_item++;

							if (coverFlow->focusIndex >= max_neighbor_item)
							{
								if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_neighbor_item))
									boundary_touch = true;
								else
								{
									ITUWidget* cf = CoverFlowGetVisibleChild(coverFlow, count - 1);
									if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
									{
										if ((cf->rect.y + cf->rect.height) <= widget_size)
											boundary_touch = true;
									}
									else
									{
										if ((cf->rect.x + cf->rect.width) <= widget_size)
											boundary_touch = true;
									}
								}
							}
							else
								boundary_touch = true;
						}

						if (!boundary_touch)
						{
							if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
							{
								if (coverFlow->procArr[i + 1] < 0)
									ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDEDOWN, 0, widget->rect.x, widget->rect.y);
								else if (coverFlow->procArr[i + 1] > 0)
									ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDEUP, 0, widget->rect.x, widget->rect.y);
							}
							else
							{
								if (coverFlow->procArr[i + 1] < 0)
									ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDERIGHT, 0, widget->rect.x, widget->rect.y);
								else if (coverFlow->procArr[i + 1] > 0)
									ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDELEFT, 0, widget->rect.x, widget->rect.y);
							}
						}
					}

					ituScene->dragged = NULL;
				}
            }
        }
    }

    if (widget->flags & ITU_TOUCHABLE)
    {
		// <<< Default Event Clear >>>
        if (ev == ITU_EVENT_MOUSEDOWN || ev == ITU_EVENT_MOUSEUP || ev == ITU_EVENT_MOUSEDOUBLECLICK || 
			ev == ITU_EVENT_MOUSELONGPRESS || ev == ITU_EVENT_TOUCHSLIDELEFT || ev == ITU_EVENT_TOUCHSLIDEUP || 
			ev == ITU_EVENT_TOUCHSLIDERIGHT || ev == ITU_EVENT_TOUCHSLIDEDOWN)
        {
            if (ituWidgetIsEnabled(widget))
            {
                int x = arg2 - widget->rect.x;
                int y = arg3 - widget->rect.y;

                if (ituWidgetIsInside(widget, x, y))
                {
                    result |= widget->dirty;
                    return widget->visible ? result : false;
                }
            }
        }
    }

    if (ev == ITU_EVENT_LAYOUT)
    {
        int i, count = CoverFlowGetVisibleChildCount(coverFlow);

		ITUWidget* childlast = CoverFlowGetVisibleChild(coverFlow, count - 1);
		int lastboundary = 0;

		if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
		{
			lastboundary = childlast->rect.y + base_size;
		}
		else
		{
			lastboundary = childlast->rect.x + base_size;
		}

		if (count > 0)
		{
			//if (coverFlow->focusIndex >= count)
			//	coverFlow->focusIndex = count - 1;

			if (coverFlow->focusIndex > (count - 1))
				coverFlow->focusIndex = count - 1;
			else if (coverFlow->focusIndex < 0)
				coverFlow->focusIndex = 0;

			if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
			{
				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
				{
					int index, count2;

					count2 = count / 2 + 1;
					index = coverFlow->focusIndex;

					for (i = 0; i < count2; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
						int fy = widget->rect.height / 2 - child->rect.height / 2;
						fy += i * child->rect.height;
						ituCoverFlowSetXY(coverFlow, index, fy, __LINE__);

						if (index >= count - 1)
							index = 0;
						else
							index++;
					}

					count2 = count - count2;
					for (i = 0; i < count2; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
						int fy = widget->rect.height / 2 - child->rect.height / 2;
						fy -= count2 * child->rect.height;
						fy += i * child->rect.height;
						ituCoverFlowSetXY(coverFlow, index, fy, __LINE__);

						if (index >= count - 1)
							index = 0;
						else
							index++;
					}
				}
				else //[LAYOUT][Vertical][non-cycle]
				{
					for (i = 0; i < count; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
						int fy = widget->rect.height / 2 - base_size / 2;

						//if (coverFlow->boundaryAlign)
						//{
						//	int max_neighbor_item = ((widget->rect.height / base_size) - 1) / 2;
						//	int max_height_item = (widget->rect.height / base_size);
						//	fy = 0;

						//	if (max_neighbor_item == 0)
						//		max_neighbor_item++;

						//	if (coverFlow->focusIndex > 0) //>= max_neighbor_item) //Bless new debug
						//	{
						//		//if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_neighbor_item))
						//		if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_height_item))
						//			fy = widget->rect.height - (count * base_size);
						//		else
						//			fy -= base_size * coverFlow->focusIndex;
						//	}
						//	else
						//		fy = 0;
						//}
						if (coverFlow->boundaryAlign)
						{
							if ((base_size * count) <= widget_size)
								fy = 0;
							else if (base_size >= widget_size)
								fy = 0 - (coverFlow->focusIndex * base_size);
							else
							{
								//try fix wrong boundary position when big focus index under non-cycle mode 2019/10/21
								int max_touch_start_size_index = (count - 1 - (widget_size / base_size));

								if (coverFlow->focusIndex <= max_touch_start_size_index)
									fy = 0 - (coverFlow->focusIndex * base_size);
								else
									//fy = 0 - (max_touch_start_size_index * base_size);
									fy = widget_size - count * base_size; //try fix wrong boundary position when big focus index under non-cycle mode 2019/10/21
							}
						}
						else
						{
							fy -= base_size * coverFlow->focusIndex;
						}

						if (coverFlow->overlapsize > 0)
						{
							fy += i * base_size;
							ituCoverFlowSetXY(coverFlow, i, fy - coverFlow->overlapsize, __LINE__);
						}
						else
						{
							fy += i * child->rect.height;
							ituCoverFlowSetXY(coverFlow, i, fy, __LINE__);
						}
					}
				}
			}
			else
			{
				//[LAYOUT][Horizontal][cycle]
				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
				{
					int index, count2;

					count2 = count / 2 + 1;
					index = coverFlow->focusIndex;

					for (i = 0; i < count2; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
						int fx = widget->rect.width / 2 - child->rect.width / 2;
						fx += i * child->rect.width;
						ituCoverFlowSetXY(coverFlow, index, fx, __LINE__);

						if (index >= count - 1)
							index = 0;
						else
							index++;
					}

					count2 = count - count2;
					for (i = 0; i < count2; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
						int fx = widget->rect.width / 2 - child->rect.width / 2;
						fx -= count2 * child->rect.width;
						fx += i * child->rect.width;
						ituCoverFlowSetXY(coverFlow, index, fx, __LINE__);

						if (index >= count - 1)
							index = 0;
						else
							index++;
					}
				}
				else //[LAYOUT][Horizontal][non-cycle]
				{
					for (i = 0; i < count; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
						ITUWidget* childlast = CoverFlowGetVisibleChild(coverFlow, count - 1);
						int fx = widget->rect.width / 2 - base_size / 2;

						/*if (coverFlow->boundaryAlign)
						{
							int max_neighbor_item = ((widget->rect.width / base_size) - 1) / 2;
							int max_width_item = (widget->rect.width / base_size);

							fx = 0;

							if (max_neighbor_item == 0)
								max_neighbor_item++;

							if (coverFlow->focusIndex > 0)
							{
								if (((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_width_item)) || (lastboundary <= widget_size))
								{
									fx = widget->rect.width - (count * base_size);
									printf("layout A\n");
								}
								else
								{
									fx -= base_size * coverFlow->focusIndex;
									printf("layout B\n");
								}
							}
							else
								fx = 0;
						}*/
						if (coverFlow->boundaryAlign)
						{
							if ((base_size * count) <= widget_size)
								fx = 0;
							else if (base_size >= widget_size)
								fx = 0 - (coverFlow->focusIndex * base_size);
							else
							{
								//try fix wrong boundary position when big focus index under non-cycle mode 2019/10/21
								int max_touch_start_size_index = (count - 1 - (widget_size / base_size));

								if (coverFlow->focusIndex <= max_touch_start_size_index)
									fx = 0 - (coverFlow->focusIndex * base_size);
								else
									//fx = 0 - (max_touch_start_size_index * base_size);
									fx = widget_size - count * base_size; //try fix wrong boundary position when big focus index under non-cycle mode 2019/10/21
							}
						}
						else
						{
							fx -= base_size * coverFlow->focusIndex;
						}

						if (coverFlow->overlapsize > 0)
						{
							fx += i * base_size;
							ituCoverFlowSetXY(coverFlow, i, fx - coverFlow->overlapsize, __LINE__);
						}
						else
						{
							fx += i * child->rect.width;
							ituCoverFlowSetXY(coverFlow, i, fx, __LINE__);
						}
					}
				}
			}

			if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ENABLE_ALL) == 0)
			{
				for (i = 0; i < count; ++i)
				{
					ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);

					if (i == coverFlow->focusIndex)
						ituWidgetEnable(child);
					else
						ituWidgetDisable(child);
				}
			}

			if ((widget->flags & ITU_DRAGGING) && (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING))
			{
				//for goose prevent mousedown can not drag when under sliding
			}
			else
			{
				widget->flags &= ~ITU_DRAGGING;
				coverFlow->touchCount = 0;
			}

			//fix for stop anywhere not display after load
			if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
			{
				ITUWidget* widget = (ITUWidget*)coverFlow;
				int count = CoverFlowGetVisibleChildCount(coverFlow);
				int i = 0;
				int fd = 0;
				int move_step = 0;
				ITUWidget* child_1 = CoverFlowGetVisibleChild(coverFlow, 0);
				ITUWidget* child_2 = CoverFlowGetVisibleChild(coverFlow, count - 1);

				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
				{
					if ((child_1->rect.y > 0) || ((child_2->rect.y + child_2->rect.height) < widget->rect.height))
					{
						if (child_1->rect.y > 0)
							move_step = 0 - child_1->rect.y;
						else
							move_step = widget->rect.height - (child_2->rect.y + child_2->rect.height);

						for (i = 0; i < count; i++)
						{
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
							fd = child->rect.y;
							fd += move_step;
							ituCoverFlowSetXY(coverFlow, i, fd, __LINE__);
						}

						coverFlow->frame = 0;
					}
				}
				else
				{
					if ((child_1->rect.x > 0) || ((child_2->rect.x + child_2->rect.width) < widget->rect.width))
					{
						if (child_1->rect.x > 0)
							move_step = 0 - child_1->rect.x;
						else
							move_step = widget->rect.width - (child_2->rect.x + child_2->rect.width);

						for (i = 0; i < count; i++)
						{
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
							fd = child->rect.x;
							fd += move_step;
							ituCoverFlowSetXY(coverFlow, i, fd, __LINE__);
						}

						coverFlow->frame = 0;
					}
				}
			}
		}
    }

    result |= widget->dirty;

	//fix bug when widget not visible but will still have dirty
	if ((widget->visible == 0) && (widget->dirty))
	{
		ituDirtyWidget(widget, false);
	}
		

    return widget->visible ? result : false;
}

void ituCoverFlowDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    int destx, desty;
    uint8_t desta;
    ITURectangle prevClip;
    ITURectangle* rect = (ITURectangle*) &widget->rect;

    assert(widget);
    assert(dest);

    destx = rect->x + x;
    desty = rect->y + y;
    desta = alpha * widget->color.alpha / 255;
    desta = desta * widget->alpha / 255;
   
    ituWidgetSetClipping(widget, dest, x, y, &prevClip);

    if (desta == 255)
    {
        ituColorFill(dest, destx, desty, rect->width, rect->height, &widget->color);
    }
    else if (desta > 0)
    {
        ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
        if (surf)
        {
            ituColorFill(surf, 0, 0, rect->width, rect->height, &widget->color);
            ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);                
            ituDestroySurface(surf);
        }
    }

	if ((desta == 0) || (desta == 255))
	{
		ITUCoverFlow* coverflow = (ITUCoverFlow*)widget;

		if (coverflow->totalframe == 0)
			coverflow->totalframe = coverflow->org_totalframe;

		if (coverflow->inc)
		{
			int way = ((coverflow->inc > 0) ? (-1) : (1));
			int step = (widget->rect.width / coverflow->totalframe);

			if (!(widget->flags & ITU_DRAGGING))
			{
				if (coverflow->eye_motion >= 2)
				{
					ituFlowWindowDraw(widget, dest, x + (step * way), y, 60);
					//printf("[coverflow]eye_motion[2]\n");
				}

				if (coverflow->eye_motion >= 1)
				{
					ituFlowWindowDraw(widget, dest, x + ((step / 2) * way), y, 120);
					//printf("[coverflow]eye_motion[1]\n");
				}
			}

			ituFlowWindowDraw(widget, dest, x, y, 255);
		}
		else
		{
			ituFlowWindowDraw(widget, dest, x, y, alpha);
		}
	}
	else
	{
		ituFlowWindowDraw(widget, dest, x, y, alpha);
	}

    ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
    ituDirtyWidget(widget, false);
}

void ituCoverFlowOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    ITUCoverFlow* coverFlow = (ITUCoverFlow*) widget;
    assert(coverFlow);

    switch (action)
    {
    case ITU_ACTION_PREV:
        ituCoverFlowPrev((ITUCoverFlow*)widget);
        break;

    case ITU_ACTION_NEXT:
        ituCoverFlowNext((ITUCoverFlow*)widget);
        break;

    case ITU_ACTION_GOTO:
        if (param[0] != '\0')
            ituCoverFlowGoto((ITUCoverFlow*)widget, atoi(param));
        break;

    case ITU_ACTION_DODELAY0:
        ituExecActions(widget, coverFlow->actions, ITU_EVENT_DELAY0, atoi(param));
        break;

    case ITU_ACTION_DODELAY1:
        ituExecActions(widget, coverFlow->actions, ITU_EVENT_DELAY1, atoi(param));
        break;

    case ITU_ACTION_DODELAY2:
        ituExecActions(widget, coverFlow->actions, ITU_EVENT_DELAY2, atoi(param));
        break;

    case ITU_ACTION_DODELAY3:
        ituExecActions(widget, coverFlow->actions, ITU_EVENT_DELAY3, atoi(param));
        break;

    case ITU_ACTION_DODELAY4:
        ituExecActions(widget, coverFlow->actions, ITU_EVENT_DELAY4, atoi(param));
        break;

    case ITU_ACTION_DODELAY5:
        ituExecActions(widget, coverFlow->actions, ITU_EVENT_DELAY5, atoi(param));
        break;

    case ITU_ACTION_DODELAY6:
        ituExecActions(widget, coverFlow->actions, ITU_EVENT_DELAY6, atoi(param));
        break;

    case ITU_ACTION_DODELAY7:
		if ((!(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)) && (coverFlow->boundaryAlign))
			coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYSTOP;
        ituExecActions(widget, coverFlow->actions, ITU_EVENT_DELAY7, atoi(param));
        break;

    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

static void CoverFlowOnCoverChanged(ITUCoverFlow* coverFlow, ITUWidget* widget)
{
	// DO NOTHING
}

void ituCoverFlowInit(ITUCoverFlow* coverFlow, ITULayout layout)
{
    assert(coverFlow);
    ITU_ASSERT_THREAD();

    memset(coverFlow, 0, sizeof (ITUCoverFlow));

    if (layout == ITU_LAYOUT_VERTICAL)
        coverFlow->coverFlowFlags &= ITU_COVERFLOW_VERTICAL;

    ituFlowWindowInit(&coverFlow->fwin, layout);

    ituWidgetSetType(coverFlow, ITU_COVERFLOW);
    ituWidgetSetName(coverFlow, coverFlowName);
    ituWidgetSetUpdate(coverFlow, ituCoverFlowUpdate);
    ituWidgetSetDraw(coverFlow, ituCoverFlowDraw);
    ituWidgetSetOnAction(coverFlow, ituCoverFlowOnAction);
    ituCoverFlowSetCoverChanged(coverFlow, CoverFlowOnCoverChanged);
}

void ituCoverFlowLoad(ITUCoverFlow* coverFlow, uint32_t base)
{
    assert(coverFlow);

    ituFlowWindowLoad(&coverFlow->fwin, base);

    ituWidgetSetUpdate(coverFlow, ituCoverFlowUpdate);
    ituWidgetSetDraw(coverFlow, ituCoverFlowDraw);
    ituWidgetSetOnAction(coverFlow, ituCoverFlowOnAction);
    ituCoverFlowSetCoverChanged(coverFlow, CoverFlowOnCoverChanged);

	if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
		coverFlow->boundaryAlign = false;
	//else
	//	coverFlow->slideMaxCount = 1;

	if (coverFlow->slideMaxCount)
	{
		if ((coverFlow->slideMaxCount < 3) && (coverFlow->slideMaxCount > 0))
			coverFlow->slideMaxCount = 1;
	}

	//to avoid integer div zero
	if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
	{
		ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, 0); 

		if (coverFlow->overlapsize > (child->rect.height * COVERFLOW_OVERLAP_MAX_PERCENTAGE / 100))
			coverFlow->overlapsize = child->rect.height * COVERFLOW_OVERLAP_MAX_PERCENTAGE / 100;

		//not support vertical yet
		coverFlow->overlapsize = 0;

		if (coverFlow->totalframe > child->rect.height)
			coverFlow->totalframe = child->rect.height;
	}
	else
	{
		ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, 0);

		if (coverFlow->overlapsize > (child->rect.width * COVERFLOW_OVERLAP_MAX_PERCENTAGE / 100))
			coverFlow->overlapsize = child->rect.width * COVERFLOW_OVERLAP_MAX_PERCENTAGE / 100;

		if (coverFlow->totalframe > child->rect.width)
			coverFlow->totalframe = child->rect.width;
	}
}

void ituCoverFlowGoto(ITUCoverFlow* coverFlow, int index)
{
    assert(coverFlow);
    ITU_ASSERT_THREAD();

    if (coverFlow->focusIndex == index)
        return;
  
	if (COVERFLOW_DEBUG_FOCUSINDEX)
		printf("[Debug][CoverFlow][FC %d ", coverFlow->focusIndex);

    coverFlow->focusIndex = index;

	if (COVERFLOW_DEBUG_FOCUSINDEX)
		printf("to %d][%d]\n", coverFlow->focusIndex, __LINE__);

    ituWidgetUpdate(coverFlow, ITU_EVENT_LAYOUT, 0, 0, 0);
}

void ituCoverFlowPrev(ITUCoverFlow* coverflow)
{
    ITUWidget* widget = (ITUWidget*) coverflow;
    unsigned int oldFlags = widget->flags;

	//Bless added for PoWei requirement --> prev/next work queue
	int i = 0;
	bool no_queue = true;
    ITU_ASSERT_THREAD();

	for (i = COVERFLOW_MAX_PROCARR_SIZE - 1; i >= 0; i--)
	{
		//fix bug
		if ((!(coverflow->coverFlowFlags & ITU_COVERFLOW_CYCLE)) && (coverflow->boundaryAlign == 0))
		{
			if (coverflow->focusIndex <= 0)
			{
				no_queue = false;
				break;
			}
		}

		if ((i - 1) >= 0)
		{
			if (coverflow->procArr[i - 1] != 0)
			{
				coverflow->procArr[i] = -1;
				no_queue = false;
				break;
			}
		}
		else
			coverflow->procArr[i] = -1;
	}

    widget->flags |= ITU_TOUCHABLE;

	if (no_queue)
	{
		coverflow->touchPos = 0;

		if (coverflow->slideMaxCount == 0)
			coverflow->prevnext_trigger = 1;

		if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
			ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDEDOWN, 0, widget->rect.x, widget->rect.y);
		else
			ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDERIGHT, 0, widget->rect.x, widget->rect.y);
	}

    if ((oldFlags & ITU_TOUCHABLE) == 0)
        widget->flags &= ~ITU_TOUCHABLE;
}

void ituCoverFlowNext(ITUCoverFlow* coverflow)
{
	ITUWidget* widget = (ITUWidget*)coverflow;
	unsigned int oldFlags = widget->flags;

	//Bless added for PoWei requirement --> prev/next work queue
	int i = 0;
	bool no_queue = true;
    ITU_ASSERT_THREAD();

	for (i = COVERFLOW_MAX_PROCARR_SIZE - 1; i >= 0; i--)
	{
		//fix bug
		if ((!(coverflow->coverFlowFlags & ITU_COVERFLOW_CYCLE)) && (coverflow->boundaryAlign == 0))
		{
			int count = CoverFlowGetVisibleChildCount(coverflow);

			if (coverflow->focusIndex >= (count - 1))
			{
				no_queue = false;
				break;
			}
		}

		if ((i - 1) >= 0)
		{
			if (coverflow->procArr[i - 1] != 0)
			{
				coverflow->procArr[i] = 1;
				no_queue = false;
				break;
			}
		}
		else
			coverflow->procArr[i] = 1;
	}

	widget->flags |= ITU_TOUCHABLE;

	if (no_queue)
	{
		coverflow->touchPos = 0;

		if (coverflow->slideMaxCount == 0)
			coverflow->prevnext_trigger = 1;

		if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
			ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDEUP, 0, widget->rect.x, widget->rect.y);
		else
			ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDELEFT, 0, widget->rect.x, widget->rect.y);
	}

    if ((oldFlags & ITU_TOUCHABLE) == 0)
        widget->flags &= ~ITU_TOUCHABLE;
}


int CoverFlowGetFirstDisplayIndex(ITUCoverFlow* coverflow)
{
	assert(coverflow);
    ITU_ASSERT_THREAD();

	if (coverflow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
		return -1;
	else
	{
		int i, count = CoverFlowGetVisibleChildCount(coverflow);

		for (i = 0; i < count; i++)
		{
			ITUWidget* child = CoverFlowGetVisibleChild(coverflow, i);

			if (child)
			{
				if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
				{
					if ((child->rect.y + child->rect.height) > 0)
						return i;
				}
				else
				{
					if ((child->rect.x + child->rect.width) > 0)
						return i;
				}
			}
			else
				return -1;
		}

		return -1;
	}
}

int CoverFlowGetDraggingDist(ITUCoverFlow* coverflow)
{
	ITUWidget* widget = (ITUWidget*)coverflow;
    ITU_ASSERT_THREAD();

	if (!widget)
	{
		return 0;
	}
	else
	{
		if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGING))
		{
			ITUWidget* child = CoverFlowGetVisibleChild(coverflow, 0);
			int pos;

			if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
			{
				pos = child->rect.y;
			}
			else
			{
				pos = child->rect.x;
			}

			return (pos - coverflow->mousedown_position);
		}
		else
		{
			return 0;
		}
	}
}

bool CoverFlowCheckIdle(ITUCoverFlow* coverflow)
{
	ITUWidget* widget = (ITUWidget*)coverflow;

	if (coverflow->inc)
		return false;

	if (coverflow->frame)
		return false;

	if (coverflow->totalframe != coverflow->org_totalframe)
		return false;

	if (widget->flags & ITU_DRAGGING)
		return false;

	if (coverflow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
		return false;

	return true;
}
