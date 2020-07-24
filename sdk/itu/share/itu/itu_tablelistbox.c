#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"

#define UNDRAGGING_DECAY 20//10
#define BOUNDARY_TOR 3//30

static const char tablelistboxName[] = "ITUTableListBox";
static bool bottom_touch = false;

static void TableListBox_ReduceItemCount(ITUWidget* widget, int NewCount)
{
	ITCTree*  thisNode;
	ITCTree*  prevSibling;
	int i, count = itcTreeGetChildCount(widget);

	if ((NewCount < count) && (NewCount > 0))
	{
		for (i = 0; i < (count - NewCount); i++)
		{
			int target = itcTreeGetChildCount(widget) - 1;
			thisNode = (ITCTree*)itcTreeGetChildAt(widget, target);

			if (thisNode->parent == NULL)
				return;

			prevSibling = thisNode->parent->child;

			if (prevSibling == thisNode)
			{
				// This is the first child node
				thisNode->parent->child = thisNode->sibling;
			}
			else
			{
				// Find the previous sibling node
				for (; prevSibling->sibling != thisNode;
					prevSibling = prevSibling->sibling);

					prevSibling->sibling = thisNode->sibling;
			}

			thisNode->parent = thisNode->sibling = NULL;
		}
	}
}

static void TableListBoxSync(ITUTableListBox* tablelistbox)
{
    ITUWidget* widget = (ITUWidget*)tablelistbox;
    ITCTree* node;

    for (node = widget->tree.parent->child; node; node = node->sibling)
    {
        ITUWidget* target = (ITUWidget*)node;

        if (target != widget && (target->type == ITU_TABLELISTBOX || target->type == ITU_TABLEICONLISTBOX))
        {
            if (widget->flags & ITU_PROGRESS)
                target->flags |= ITU_PROGRESS;
            else
                target->flags &= ~ITU_PROGRESS;

            if (widget->flags & ITU_DRAGGING)
                target->flags |= ITU_DRAGGING;
            else
                target->flags &= ~ITU_DRAGGING;

            if (widget->flags & ITU_UNDRAGGING)
                target->flags |= ITU_UNDRAGGING;
            else
                target->flags &= ~ITU_UNDRAGGING;

            if (widget->flags & ITU_BOUNCING)
                target->flags |= ITU_BOUNCING;
            else
                target->flags &= ~ITU_BOUNCING;

            if (target->type == ITU_TABLELISTBOX)
            {
                ITUTableListBox* targetslistbox = (ITUTableListBox*)target;
                targetslistbox->touchOffset = tablelistbox->touchOffset;
                targetslistbox->inc = tablelistbox->inc;
                targetslistbox->frame = tablelistbox->frame;
                targetslistbox->touchY = tablelistbox->touchY;
            }
            else //if (target->type == ITU_TABLEICONLISTBOX)
            {
                ITUTableIconListBox* targettableiconlistbox = (ITUTableIconListBox*)target;
                targettableiconlistbox->touchOffset = tablelistbox->touchOffset;
                targettableiconlistbox->inc = tablelistbox->inc;
                targettableiconlistbox->frame = tablelistbox->frame;
                targettableiconlistbox->touchY = tablelistbox->touchY;
            }
        }
    }
}

static void TableListBoxSelect(ITUTableListBox* tablelistbox, int index)
{
    ITUWidget* widget = (ITUWidget*)tablelistbox;
    ITCTree* node;
    ITUWidget* parent = (ITUWidget*)widget->tree.parent;
    
    if (parent->type == ITU_TABLEGRID)
    {
        ITUTableGrid* grid = (ITUTableGrid*)parent;

        grid->selectRow = index;
        grid->selectColumn = 0;
        
        ituExecActions((ITUWidget*)grid, grid->actions, ITU_EVENT_SELECT, index);

        if (grid->tableGridFlags & ITU_TABLEGRID_SINGLE_SELECT)
        {
            for (node = parent->tree.child; node; node = node->sibling)
            {
                ITUWidget* target = (ITUWidget*)node;
                if (target != widget)
                    ituListBoxSelect((ITUListBox*)target, -1);
            }

            for (node = parent->tree.child; node; node = node->sibling)
            {
                ITUWidget* target = (ITUWidget*)node;
                if (target == widget)
                {
                    break;
                }
                else
                {
                    grid->selectColumn++;
                }
            }
            ituListBoxSelect((ITUListBox*)widget, index);
            return;
        }
    }

    for (node = parent->tree.child; node; node = node->sibling)
    {
        ITUWidget* target = (ITUWidget*)node;

        if (target->type == ITU_TABLELISTBOX || target->type == ITU_TABLEICONLISTBOX)
        {
            ituListBoxSelect((ITUListBox*)target, index);
        }
    }
}

static void TableListBoxCheck(ITUTableListBox* tablelistbox, int index)
{
    ITUWidget* widget = (ITUWidget*)tablelistbox;
    ITCTree* node;

    for (node = widget->tree.parent->child; node; node = node->sibling)
    {
        ITUWidget* target = (ITUWidget*)node;

        if (target->type == ITU_TABLELISTBOX || target->type == ITU_TABLEICONLISTBOX)
        {
            ituListBoxCheck((ITUListBox*)target, index);
        }
    }
}

bool ituTableListBoxUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUListBox* listbox = (ITUListBox*) widget;
    ITUTableListBox* tablelistbox = (ITUTableListBox*) widget;
    assert(tablelistbox);

    if (ev == ITU_EVENT_LAYOUT && (arg1 != ITU_ACTION_FOCUS))
    {
        int i, count = itcTreeGetChildCount(tablelistbox);

        for (i = 0; i < count; ++i)
        {
            ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(tablelistbox, i);
            int fy = i * child->rect.height;
            fy += tablelistbox->touchOffset;
            ituWidgetSetY(child, fy);
        }

        tablelistbox->preindex = -1;
        result = true;
    }
    else if (ev == ITU_EVENT_MOUSEMOVE)
    {
        if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGING))
        {
            int y = arg3 - widget->rect.y;
            int offset = y + tablelistbox->touchOffset - tablelistbox->touchY;
            int i, count = itcTreeGetChildCount(widget);
            int topY, bottomY;
            ITUWidget* child = (ITUWidget*)((ITCTree*)tablelistbox)->child;
            if (child)
            {
				if (count > tablelistbox->listbox.itemCount)
					count = tablelistbox->listbox.itemCount;
                topY = child->rect.y;
                bottomY = topY + child->rect.height * count;
            }
            else
            {
                topY = bottomY = 0;
            }

            //printf("0: touchOffset=%d offset=%d y=%d\n", tablelistbox->touchOffset, offset, y);

            //if (topY <= widget->rect.height / 2 &&
            //    bottomY >= widget->rect.height / 2)
			if ((topY <= BOUNDARY_TOR) && (bottomY >= (widget->rect.height/2 - BOUNDARY_TOR)))
            {
                for (i = 0; i < count; ++i)
                {
                    ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(widget, i);
                    int fy = i * child->rect.height;
                    fy += offset;
                    ituWidgetSetY(child, fy);
                }
            }
        }
    }
    else if (ev == ITU_EVENT_MOUSEDOWN)
    {
        if (ituWidgetIsEnabled(widget) && !(widget->flags & ITU_UNDRAGGING))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                if (tablelistbox->inc && !(widget->flags & ITU_BOUNCING))
                {
					int i = 0, fixy = 0, fy = 0;
					int count = itcTreeGetChildCount(widget);
					ITUWidget* child = (ITUWidget*)((ITCTree*)tablelistbox)->child;

					if (child)
						tablelistbox->touchOffset = child->rect.y;

					for (i = 0; i < count; i++)
					{
						child = (ITUWidget*)itcTreeGetChildAt(tablelistbox, i);
						if ((child->rect.y < 0) && ((child->rect.y + child->rect.height) > 0))
						{
							if (tablelistbox->inc > 0)
								fixy -= child->rect.y;
							else
								fixy -= (child->rect.y + child->rect.height);

							i = count;
						}
					}

					tablelistbox->touchY = y;
					widget->flags |= ITU_DRAGGING;
					ituScene->dragged = widget;

					tablelistbox->frame = 0;
					tablelistbox->inc = 0;
					
                    TableListBoxSync(tablelistbox);
                }
                else
                {
                    ITCTree* node = ((ITCTree*)tablelistbox)->child;
                    int count, i = 0;

                    tablelistbox->preindex = -1;
                    count = listbox->itemCount;

                    for (; node; node = node->sibling)
                    {
                        ITUWidget* item = (ITUWidget*) node;
                        int x1, y1;

                        if (i >= count)
                            break;

                        x1 = x - item->rect.x;
                        y1 = y - item->rect.y;

                        if (ituWidgetIsInside(item, x1, y1))
                        {
                            tablelistbox->preindex = count + i;
                            break;
                        }
                        i++;
                    }

                    if (widget->flags & ITU_DRAGGABLE)
                    {
                        tablelistbox->touchY = y;

                        if (widget->flags & ITU_HAS_LONG_PRESS)
                        {
                            tablelistbox->touchCount = 1;
                        }
                        else
                        {
                            widget->flags |= ITU_DRAGGING;
                            ituScene->dragged = widget;
                            TableListBoxSync(tablelistbox);
                        }
                    }
                }
                result = true;
            }
        }
    }
    else if (ev == ITU_EVENT_MOUSEUP || ev == ITU_EVENT_MOUSEDOUBLECLICK)
    {
        if (ituWidgetIsEnabled(widget))
        {
            int dist;
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if ((widget->flags & ITU_DRAGGABLE) && (widget->flags & ITU_DRAGGING))
            {
                int topY, bottomY;
				ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(tablelistbox, 0);

                if (child)
                {
                    int count = itcTreeGetChildCount(widget);
					if (count > tablelistbox->listbox.itemCount)
						count = tablelistbox->listbox.itemCount;

                    tablelistbox->touchOffset = child->rect.y;
                    topY = child->rect.y;
                    bottomY = topY + child->rect.height * count;
                }
                else
                {
                    tablelistbox->touchOffset = topY = bottomY = 0;
                }

                if (topY > 0)
                {
                    int check = (tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY)) - topY * (tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY)) / (widget->rect.height / 2);
					tablelistbox->frame = (check >= 0)?(check):(0);
                    dist = tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY);
                    tablelistbox->inc = (dist != 0) ? -(widget->rect.height / 2) / dist : -(widget->rect.height / 2);											
                    //printf("1: frame=%d topY=%d inc=%d\n", tablelistbox->frame, topY, tablelistbox->inc);
                    widget->flags |= ITU_UNDRAGGING;
                }
                else if (bottomY < widget->rect.height)
                {
					bottom_touch = true;
                    tablelistbox->frame = bottomY * (tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY)) / (widget->rect.height / 2);
                    dist = tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY);
                    tablelistbox->inc = (dist != 0) ? (widget->rect.height / 2) / dist : (widget->rect.height / 2);											
                    //printf("2: frame=%d bottomY=%d inc=%d\n", tablelistbox->frame, bottomY, tablelistbox->inc);
                    widget->flags |= ITU_UNDRAGGING;
                }
				else
					widget->flags |= ITU_UNDRAGGING;
                ituScene->dragged = NULL;
                widget->flags &= ~ITU_DRAGGING;
                TableListBoxSync(tablelistbox);
                result = true;
            }
            
            dist = y - tablelistbox->touchY;
            if (dist < 0)
                dist = -dist;

			if (ituWidgetIsInside(widget, x, y) && ((dist < ITU_DRAG_DISTANCE) || !(widget->flags & ITU_DRAGGABLE)))
            {
                ITCTree* node = ((ITCTree*)tablelistbox)->child;
                int count, i = 0;

                count = listbox->itemCount;

                for (; node; node = node->sibling)
                {
                    ITUWidget* item = (ITUWidget*) node;
                    int x1, y1;

                    if (i >= count)
                        break;

                    x1 = x - item->rect.x;
                    y1 = y - item->rect.y;

                    if (ituWidgetIsInside(item, x1, y1) && tablelistbox->preindex == count + i)
                    {
                        if (ev == ITU_EVENT_MOUSEUP)
                        {
                            if (!(widget->flags & ITU_LONG_PRESSING))
                            {
                                TableListBoxSelect(tablelistbox, i);
                                ituExecActions(widget, listbox->actions, ITU_EVENT_SELECT, i);
                            }
                        }
                        else
                        {
                            ituListBoxOnSelection(listbox, (ITUScrollText*)item, true);
                        }
                        break;
                    }
                    i++;
                }
                tablelistbox->preindex = -1;
                result = true;
            }
            tablelistbox->touchCount = 0;
            widget->flags &= ~ITU_LONG_PRESSING;
        }
    }
    else if (ev == ITU_EVENT_MOUSELONGPRESS)
    {
        if (ituWidgetIsEnabled(widget))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                ITCTree* node = ((ITCTree*)tablelistbox)->child;
                int count, i = 0;

                count = listbox->itemCount;

                for (; node; node = node->sibling)
                {
                    ITUWidget* item = (ITUWidget*) node;
                    int x1, y1;

                    if (i >= count)
                        break;

                    x1 = x - item->rect.x;
                    y1 = y - item->rect.y;

                    if (ituWidgetIsInside(item, x1, y1) && tablelistbox->preindex == count + i)
                    {
                        TableListBoxCheck(tablelistbox, i);
                        ituExecActions(widget, listbox->actions, ev, i);
                        result |= widget->dirty;
                        break;
                    }
                    i++;
                }
                widget->flags |= ITU_LONG_PRESSING;
            }
        }
        tablelistbox->touchCount = 0;
    }
    else if (ev == ITU_EVENT_TOUCHSLIDEUP || ev == ITU_EVENT_TOUCHSLIDEDOWN)
    {
        tablelistbox->touchCount = 0;

        if (ituWidgetIsEnabled(widget))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                if (ev == ITU_EVENT_TOUCHSLIDEUP)
                {
                    int count = itcTreeGetChildCount(tablelistbox);

                    if (widget->flags & ITU_DRAGGING)
                    {
                        widget->flags &= ~ITU_DRAGGING;
                        ituScene->dragged = NULL;
                        tablelistbox->inc = 0;
                    }

					if (tablelistbox->inc == 0)
					{
						tablelistbox->inc = 0 - widget->rect.height * tablelistbox->slidePage;
					}

					tablelistbox->frame = 1;
                    ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_TOUCHSLIDEUP, 0);
                }
                else // if (ev == ITU_EVENT_TOUCHSLIDEDOWN)
                {
                    if (widget->flags & ITU_DRAGGING)
                    {
                        widget->flags &= ~ITU_DRAGGING;
                        ituScene->dragged = NULL;
                        tablelistbox->inc = 0;
                    }

                    if (tablelistbox->inc == 0)
                        tablelistbox->inc = widget->rect.height * tablelistbox->slidePage;

                    tablelistbox->frame = 1;
                    ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_TOUCHSLIDEDOWN, 0);
                }
                TableListBoxSync(tablelistbox);
                result = true;
            }
        }
    }
    else if (ev == ITU_EVENT_TIMER)
    {
		int currentcount = itcTreeGetChildCount(tablelistbox);

		if (tablelistbox->listbox.itemCount < currentcount)
		{
			if (tablelistbox->listbox.itemCount > 0)
			{
                TableListBox_ReduceItemCount((ITUWidget*)tablelistbox, tablelistbox->listbox.itemCount);
				ituWidgetUpdate(tablelistbox, ITU_EVENT_LOAD, 0, 0, 0);
				ituWidgetUpdate(tablelistbox, ITU_EVENT_LAYOUT, 0, 0, 0);
				return true;
			}
		}

        if (tablelistbox->touchCount > 0)
        {
            int y, dist;

            assert(widget->flags & ITU_HAS_LONG_PRESS);

            ituWidgetGetGlobalPosition(widget, NULL, &y);

            dist = ituScene->lastMouseY - (y + tablelistbox->touchY);

            if (dist < 0)
                dist = -dist;

            if (dist >= ITU_DRAG_DISTANCE)
            {
                widget->flags |= ITU_DRAGGING;
                ituScene->dragged = widget;
                tablelistbox->touchCount = 0;
                TableListBoxSync(tablelistbox);
            }
        }

        if (widget->flags & ITU_UNDRAGGING)
        {
            int i, count = itcTreeGetChildCount(tablelistbox);
			ITUWidget* childfirst = (ITUWidget*)itcTreeGetChildAt(tablelistbox, 0);
			ITUWidget* childlast = (ITUWidget*)itcTreeGetChildAt(tablelistbox, count - 1);

			if (count > tablelistbox->listbox.itemCount)
				count = tablelistbox->listbox.itemCount;

            if (tablelistbox->inc > 0)
            {
				int fy = 0, fixy = 0;
				int maxc = 0;
				
                for (i = 0; i < count; ++i)
                {
                    ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(tablelistbox, i);
					//maxc = widget->rect.height / child->rect.height;
					maxc = (int)(round((double)widget->rect.height / child->rect.height));

					if (!(widget->flags & ITU_DRAGGABLE))
					{
						fy = 0 - (child->rect.height * (count - maxc));

						fy += i * child->rect.height;
					}
					else
					{
						if ((childlast->rect.y + childlast->rect.height) <= widget->rect.height)//(bottom_touch)
						{
							fy = 0 - (child->rect.height * (count - maxc));
							fy += i * child->rect.height;
							fy += tablelistbox->inc *(tablelistbox->frame + 1) / tablelistbox->totalframe;
						}
						else if (childfirst->rect.y >= 0)
						{
							fy = i * child->rect.height;
						}
						else
						{
							if (fixy == 0)
							{
								int j = 0;
								for (j = 0; j < count; ++j)
								{
									ITUWidget* cc = (ITUWidget*)itcTreeGetChildAt(tablelistbox, j);
									if ((cc->rect.y < 0) && ((cc->rect.y + cc->rect.height) > 0))
									{
										fixy -= cc->rect.y;
										j = count;
									}
								}
							}

							fy = child->rect.y + fixy;
						}
					}

					if (count <= maxc)
					{
						fy = i * child->rect.height;
					}
                    ituWidgetSetY(child, fy);//here
					//printf("[fy1] %d\n", fy);
					//printf("[TIMER]child %d, fy, %d, recth %d, inc %d\n", i, fy, child->rect.height, tablelistbox->inc);
                }

				if (bottom_touch)
					bottom_touch = false;
            }
			else if (tablelistbox->inc < 0)
            {
				int fy = 0, fixy = 0;
				int maxc = 0;

                for (i = 0; i < count; ++i)
                {
                    ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(tablelistbox, i);
					//maxc = widget->rect.height / child->rect.height;
					maxc = (int)(round((double)widget->rect.height / child->rect.height));
					
					if (!(widget->flags & ITU_DRAGGABLE))
					{
						fy = 0 - (child->rect.height * (count - maxc));

						fy += i * child->rect.height;
					}
					else
					{
						if ((childlast->rect.y + childlast->rect.height) <= widget->rect.height)//(bottom_touch)
						{
							fy = 0 - (child->rect.height * (count - maxc));
							fy += i * child->rect.height;
							fy += tablelistbox->inc *(tablelistbox->frame + 1) / tablelistbox->totalframe;
						}
						else if (childfirst->rect.y >= 0)
						{
							fy = i * child->rect.height;
						}
						else
						{
							if (fixy == 0)
							{
								int j = 0;
								for (j = 0; j < count; ++j)
								{
									ITUWidget* cc = (ITUWidget*)itcTreeGetChildAt(tablelistbox, j);
									if ((cc->rect.y < 0) && ((cc->rect.y + cc->rect.height) > 0))
									{
										fixy -= cc->rect.y;
										j = count;
									}
								}
							}

							fy = child->rect.y + fixy;
						}
					}
					//int fy = i * child->rect.height;
                    ituWidgetSetY(child, fy);
					//printf("[fy2] %d\n", fy);
                }
				result = true;
            }
			else
			{
				int fixy = 0;

				for (i = 0; i < count; ++i)
				{
					ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(tablelistbox, i);

					if (child->rect.y >= 0)
					{
						fixy = child->rect.y;
						i = count;
						continue;
					}
					else if ((child->rect.y + child->rect.height) >= 0)
					{
						fixy = (child->rect.y + child->rect.height);
						i = count;
						continue;
					}
				}

				if (fixy > 0)
				{
					for (i = 0; i < count; ++i)
					{
						ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(tablelistbox, i);
						ituWidgetSetY(child, child->rect.y - fixy);
						//if (i == 0)
						//	printf("[fixy][timer undragging][%d]\n", fixy);
					}
				}
				tablelistbox->frame = tablelistbox->totalframe;
			}
            tablelistbox->frame++;

            if (tablelistbox->frame > (tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY)))
            {
				int maxc = (int)(round((double)widget->rect.height / childfirst->rect.height));

				if ((childlast->rect.y + childlast->rect.height) <= widget->rect.height)//(bottom_touch)
				{
					for (i = 0; i < count; i++)
					{
						ITUWidget* cc = (ITUWidget*)itcTreeGetChildAt(tablelistbox, i);
						int fy = 0;
						if (count >= maxc)
							fy = 0 - (cc->rect.height * (count - maxc));
						fy += i * cc->rect.height;
						ituWidgetSetY(cc, fy);
					}

					tablelistbox->touchOffset = childfirst->rect.y;
				}
				else
				{
					if (childfirst)
						tablelistbox->touchOffset = childfirst->rect.y;
					else
						tablelistbox->touchOffset = 0;
				}

                tablelistbox->frame = 0;
                tablelistbox->inc = 0;
                widget->flags &= ~ITU_UNDRAGGING;
            }
            result = true;
        }
        else if (tablelistbox->inc)
        {
            int i, count = itcTreeGetChildCount(tablelistbox);
            int topY, bottomY;
            ITUWidget* child = (ITUWidget*)((ITCTree*)tablelistbox)->child;
			if (count > tablelistbox->listbox.itemCount)
				count = tablelistbox->listbox.itemCount;

            if (child)
            {
				int maxc = (int)(round((double)widget->rect.height / child->rect.height));
                topY = child->rect.y;
                bottomY = topY + child->rect.height * count;

				if (count <= maxc) //fix for too less item
				{
					child->rect.y = 0;
					topY = 0;
					bottomY = widget->rect.height;
					tablelistbox->frame = tablelistbox->totalframe;
					tablelistbox->inc = 0;
				}
            }
            else
            {
                topY = bottomY = 0;
            }

            if (topY > widget->rect.height / 2)
            {
                tablelistbox->frame = (tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY)) - topY * (tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY)) / (widget->rect.height / 2);
                i = tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY);
                tablelistbox->inc = (i != 0) ? -(widget->rect.height / 2) / i : -(widget->rect.height / 2);
                //printf("3: frame=%d topY=%d inc=%d\n", tablelistbox->frame, topY, tablelistbox->inc);
                widget->flags |= ITU_UNDRAGGING;
            }
            else if (bottomY < widget->rect.height / 2)
            {
                tablelistbox->frame = bottomY * (tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY)) / (widget->rect.height / 2);
                i = tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY);
                tablelistbox->inc = (i != 0) ? (widget->rect.height / 2) / i : (widget->rect.height / 2);
                //printf("4: frame=%d bottomY=%d inc=%d\n", tablelistbox->frame, bottomY, tablelistbox->inc);
                widget->flags |= ITU_UNDRAGGING;
            }
            else
            {
                float lamda = 5.0f * (float)tablelistbox->frame / tablelistbox->totalframe;
                float step = tablelistbox->inc - tablelistbox->inc * expf(-lamda);

                //printf("step=%f %f %f\n", step, tablelistbox->inc * expf(-lamda), expf(-lamda));

                for (i = 0; i < count; ++i)
                {
                    ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(tablelistbox, i);
					int fy = tablelistbox->touchOffset;
                    fy += i * child->rect.height;
                    fy += (int)step;

                    ituWidgetSetY(child, fy);
					//printf("[fy3] %d\n", fy);
                }
                tablelistbox->frame++;

                if (tablelistbox->frame > tablelistbox->totalframe)
                {
                    if (topY > 0)
                    {
                        tablelistbox->frame = (tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY)) - topY * (tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY)) / (widget->rect.height / 2);
                        i = tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY);
                        tablelistbox->inc = (i != 0) ? -(widget->rect.height / 2) / i : -(widget->rect.height / 2);						
                        //printf("5: frame=%d topY=%d inc=%d\n", tablelistbox->frame, topY, tablelistbox->inc);
                        widget->flags |= ITU_UNDRAGGING;
                    }
                    else if (bottomY < widget->rect.height)
                    {
                        tablelistbox->frame = bottomY * (tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY)) / (widget->rect.height / 2);
                        i = tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY);
                        tablelistbox->inc = (i != 0) ? (widget->rect.height / 2) / i : (widget->rect.height / 2);						
                        //printf("6: frame=%d bottomY=%d inc=%d\n", tablelistbox->frame, bottomY, tablelistbox->inc);
                        widget->flags |= ITU_UNDRAGGING;
                    }
                    else
                    {
						//fix the slide self stop position
						int fixy = 0;

						for (i = 0; i < count; i++)
						{
							ITUWidget* cc = (ITUWidget*)itcTreeGetChildAt(tablelistbox, i);
							if ((cc->rect.y < 0) && ((cc->rect.y + cc->rect.height) > 0))
							{
								if (tablelistbox->inc > 0)
									fixy = 0 - cc->rect.y;
								else
									fixy = 0 - (cc->rect.y + cc->rect.height);
								i = count;
							}
						}

						if (fixy != 0)
						{
							for (i = 0; i < count; i++)
							{
								ITUWidget* cc = (ITUWidget*)itcTreeGetChildAt(tablelistbox, i);
								ituWidgetSetY(cc, cc->rect.y + fixy);
							}
						}

                        if (child)
                            tablelistbox->touchOffset = child->rect.y;

                        //printf("7: frame=%d bottomY=%d inc=%d\n", tablelistbox->frame, bottomY, tablelistbox->inc);

                        tablelistbox->frame = 0;
                        tablelistbox->inc = 0;
                    }
                }
            }
            result = true;
        }
    }
    
    if (!result)
    {
        result = ituListBoxUpdate(widget, ev, arg1, arg2, arg3);
    }
    result |= widget->dirty;
    return widget->visible ? result : false;
}

void ituTableListBoxOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    assert(widget);

    switch (action)
    {
    default:
        ituListBoxOnAction(widget, action, param);
        break;
    }
}

void ituTableListBoxOnLoadPage(ITUListBox* listbox, int pageIndex)
{
    ITUTableListBox* tablelistbox = (ITUTableListBox*) listbox;
    int i, count = itcTreeGetChildCount(tablelistbox);

    for (i = 0; i < count; ++i)
    {
        ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(tablelistbox, i);
        int fy = i * child->rect.height;
        ituWidgetSetY(child, fy);
    }

    tablelistbox->preindex = -1;
    tablelistbox->touchOffset = 0;
}

void ituTableListBoxInit(ITUTableListBox* tablelistbox, int width)
{
    assert(tablelistbox);
    ITU_ASSERT_THREAD();

    memset(tablelistbox, 0, sizeof (ITUTableListBox));

    ituListBoxInit(&tablelistbox->listbox, width);

    ituWidgetSetType(tablelistbox, ITU_TABLELISTBOX);
    ituWidgetSetName(tablelistbox, tablelistboxName);
    ituWidgetSetUpdate(tablelistbox, ituTableListBoxUpdate);
    ituWidgetSetOnAction(tablelistbox, ituTableListBoxOnAction);
    ituListBoxSetOnLoadPage(tablelistbox, ituTableListBoxOnLoadPage);
}

void ituTableListBoxLoad(ITUTableListBox* tablelistbox, uint32_t base)
{
    assert(tablelistbox);

    ituListBoxLoad(&tablelistbox->listbox, base);

    ituWidgetSetUpdate(tablelistbox, ituTableListBoxUpdate);
    ituWidgetSetOnAction(tablelistbox, ituTableListBoxOnAction);
    ituListBoxSetOnLoadPage(tablelistbox, ituTableListBoxOnLoadPage);
}

int ituTableListBoxGetItemCount(ITUTableListBox* tablelistbox)
{
    ITU_ASSERT_THREAD();
    return itcTreeGetChildCount(tablelistbox);
}
