#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

#define STOPANYWHERE_TIMER_CYCLE_MAX  20         //17ms x 20 ~ 340ms
#define STOPANYWHERE_SLIDE_CHECK_TIMING  300     //300ms
#define STOPANYWHERE_SLIDE_REQUIRE_DIST  1800    //~X40 ~Y40
#define STOPANYWHERE_BOUNCE_RATIO 10 //10%
#define STOPANYWHERE_DRAGGING_EYEMOTION 0

#define STOPANYWHERE_SLIDE_FACTOR  1  //slide factor/speed default x 1

#define STOPANYWHERE_DRAGGING_PIPE 0
#define STOPANYWHERE_DRAGGING_PIPE_SIZE 100
#define STOPANYWHERE_MOUSEDOWN_LONGPRESS_DELAY 1000

//#define TEST_ALIGN 1
#define STOPANYWHERE_ALIGN_SHIFT_FACTOR 3
#define STOPANYWHERE_ALIGN_AVG_CENTER 0
#define STOPANYWHERE_ALIGN_LEFT_FIRST 0 //special case for some customer 2019/10/24

//Overlap operation related define
#define STOPANYWHERE_MIN_OL_DRAGACT_DIST 10

//Debug Switch List
#define STOPANYWHERE_DEBUG_SETXY 0
#define STOPANYWHERE_DEBUG_FIXXY 0

static const char sawName[] = "ITUStopAnywhere";

//use for tree config related
#define STOPANYWHERE_MAX_TREE_ITEM  128  //this value should be the N / 2 --> N is the Config saw_tree[N]
static int vSawActiveConfigTree[10][STOPANYWHERE_MAX_TREE_ITEM] = { { 0 } };
static ITUWidget* vSawActiveWidgetIndex[10] = { NULL };

bool ituStopAnywhereClone(ITUWidget* widget, ITUWidget** cloned)
{
	assert(widget);
	assert(cloned);
	ITU_ASSERT_THREAD();

	if (*cloned == NULL)
	{
		ITUWidget* newWidget = malloc(sizeof(ITUStopAnywhere));
		if (newWidget == NULL)
			return false;

		memcpy(newWidget, widget, sizeof(ITUStopAnywhere));
		newWidget->tree.child = newWidget->tree.parent = newWidget->tree.sibling = NULL;
		*cloned = newWidget;
	}

	return ituWidgetCloneImpl(widget, cloned);
}

ITUWidget* StopAnywhereGetVisibleChild(ITUStopAnywhere* saw)
{
	ITCTree *child, *tree = (ITCTree*)saw;
	assert(tree);

	for (child = tree->child; child; child = child->sibling)
	{
		if (child)
		{
			ITUWidget* widget = (ITUWidget*)child;

			if (widget->visible)
			{
				return widget;
			}
		}
	}
	return NULL;
}

int ituStopAnywhereCheckChildSame(ITUWidget* widget, bool horizontal)
{
	ITCTree* tree = (ITCTree*)widget;
	ITCTree *child = tree->child;
	int baseunit = 0;

	for (child = tree->child; child; child = child->sibling)
	{
		ITUWidget* ww = (ITUWidget*)child;
		int checksize = 0;

		if (horizontal)
			checksize = ww->rect.width;
		else
			checksize = ww->rect.height;

		if (ww)
		{
			if ((baseunit != 0) && (checksize != baseunit))
				return checksize;
			else
				baseunit = checksize;
		}
	}

	return baseunit;
}

int ituStopAnywhereFindActiveArrayIdx(ITUStopAnywhere* saw)
{
	ITUWidget* widget = (ITUWidget*)saw;
	int i = 0;
	for (i = 0; i < 10; i++)
	{
		if (vSawActiveWidgetIndex[i] == widget)
			return i;
	}
	return 0;
}

int ituStopAnywhereFixX(ITUStopAnywhere* saw, int targetX)
{
	int result_value = targetX;

	if (saw)
	{
		ITUWidget* widget = (ITUWidget*)saw;
		ITUWidget* child = StopAnywhereGetVisibleChild(saw);

		if (widget && child)
		{
			bool check_result = false;

			if (saw->bounce == 1)
			{
				int baseunit = ituStopAnywhereCheckChildSame(child, true);
				int alignavgshiftsize = (widget->rect.width % baseunit) / 2;

				if (!STOPANYWHERE_ALIGN_AVG_CENTER && ((alignavgshiftsize % 2) != 0))
					alignavgshiftsize = 0;

				int left_side_boundary = (widget->rect.width * STOPANYWHERE_BOUNCE_RATIO / 100) + alignavgshiftsize;
				int right_side_boundary = ((widget->rect.width * (100 - STOPANYWHERE_BOUNCE_RATIO)) / 100) - alignavgshiftsize;

				if (targetX > (0 + alignavgshiftsize))
				{
					saw->stopAnywhereFlags |= ITU_STOPANYWHERE_BOUNCING_L;
					saw->stopAnywhereFlags |= ITU_STOPANYWHERE_BOUNCING;

					if (targetX > left_side_boundary)
					{
						check_result = true;
						result_value = left_side_boundary;
					}
				}
				else if ((targetX + child->rect.width) < (widget->rect.width - alignavgshiftsize))
				{
					saw->stopAnywhereFlags |= ITU_STOPANYWHERE_BOUNCING_R;
					saw->stopAnywhereFlags |= ITU_STOPANYWHERE_BOUNCING;

					if ((targetX + child->rect.width) < right_side_boundary)
					{
						check_result = true;
						result_value = (right_side_boundary - child->rect.width);
					}
				}
				else
				{
					saw->stopAnywhereFlags &= ~ITU_STOPANYWHERE_BOUNCING;
					saw->stopAnywhereFlags &= ~ITU_STOPANYWHERE_BOUNCING_L;
					saw->stopAnywhereFlags &= ~ITU_STOPANYWHERE_BOUNCING_R;
				}
			}
			else
			{
				if (targetX > 0)
				{
					if (STOPANYWHERE_DEBUG_FIXXY)
						printf("[DEBUG] Func: %s [Fix X from %d to %d]\n", __func__, targetX, 0);

					check_result = true;
					result_value = 0;
				}
				else if ((targetX + child->rect.width) < widget->rect.width)
				{
					check_result = true;
					result_value = (widget->rect.width - child->rect.width);
				}
			}

			if (check_result)
			{
				//
			}

			return result_value;
		}
		else
		{
			printf("[ERROR] Func: %s [%d]\n", __func__, __LINE__);
			return result_value;
		}
	}
	else
	{
		printf("[ERROR] Func: %s [%d]\n", __func__, __LINE__);
		return result_value;
	}
}

int ituStopAnywhereFixY(ITUStopAnywhere* saw, int targetY)
{
	int result_value = targetY;

	if (saw)
	{
		ITUWidget* widget = (ITUWidget*)saw;
		ITUWidget* child = StopAnywhereGetVisibleChild(saw);

		if (widget && child)
		{
			bool check_result = false;

			if (saw->bounce == 2)
			{
				int top_side_boundary = (widget->rect.height * STOPANYWHERE_BOUNCE_RATIO / 100);
				int bottom_side_boundary = ((widget->rect.height * (100 - STOPANYWHERE_BOUNCE_RATIO)) / 100);

				if (targetY > 0)
				{
					saw->stopAnywhereFlags |= ITU_STOPANYWHERE_BOUNCING_T;
					saw->stopAnywhereFlags |= ITU_STOPANYWHERE_BOUNCING;

					if (targetY > top_side_boundary)
					{
						check_result = true;
						result_value = top_side_boundary;
					}
				}
				else if ((targetY + child->rect.height) < widget->rect.height)
				{
					saw->stopAnywhereFlags |= ITU_STOPANYWHERE_BOUNCING_B;
					saw->stopAnywhereFlags |= ITU_STOPANYWHERE_BOUNCING;

					if ((targetY + child->rect.height) < bottom_side_boundary)
					{
						check_result = true;
						result_value = (bottom_side_boundary - child->rect.height);
					}
				}
				else
				{
					saw->stopAnywhereFlags &= ~ITU_STOPANYWHERE_BOUNCING;
					saw->stopAnywhereFlags &= ~ITU_STOPANYWHERE_BOUNCING_T;
					saw->stopAnywhereFlags &= ~ITU_STOPANYWHERE_BOUNCING_B;
				}
			}
			else
			{
				if (targetY > 0)
				{
					if (STOPANYWHERE_DEBUG_FIXXY)
						printf("[DEBUG] Func: %s [Fix Y from %d to %d]\n", __func__, targetY, 0);

					check_result = true;
					result_value = 0;
				}
				else if ((targetY + child->rect.height) < widget->rect.height)
				{
					check_result = true;
					result_value = (widget->rect.height - child->rect.height);
				}
			}

			if (check_result)
			{
				//
			}

			return result_value;
		}
		else
		{
			printf("[ERROR] Func: %s [%d]\n", __func__, __LINE__);
			return result_value;
		}
	}
	else
	{
		printf("[ERROR] Func: %s [%d]\n", __func__, __LINE__);
		return result_value;
	}
}

void ituStopAnywhereSetXY(ITUStopAnywhere* saw, int x, int y, int line)
{
	if (saw)
	{
		ITUWidget* sawwidget = (ITUWidget*)saw;
		ITUWidget* child = StopAnywhereGetVisibleChild(saw);

		if (child)
		{
			int fixX = ituStopAnywhereFixX(saw, x);
			int fixY = ituStopAnywhereFixY(saw, y);

			ituWidgetSetPosition(child, fixX, fixY);

			if (STOPANYWHERE_DEBUG_SETXY)
				printf("[Debug][StopAnywhere %s][X %d][Y %d] [%d]\n", sawwidget->name, fixX, fixY, line);
		}
	}
}

bool ituStopAnywhereFindLoop(ITCTree* tree)
{
	ITCTree *child = tree->child;

	//ITUWidget* checkwidget = (ITUWidget*)child;
	//if (checkwidget)
	//	printf("widget name %s\n", checkwidget->name);

	for (child = tree->child; child; child = child->sibling)
	{
		ITUWidget* ww = (ITUWidget*)child;
		if (ww)
		{
			if (ww->type == ITU_BUTTON)
			{
				ITUButton* button = (ITUButton*)ww;

				if (button->pressed)
				{
					ituExecActions(ww, button->actions, ITU_EVENT_MOUSELONGPRESS, 0);
					return true;
				}
			}
			else
			{
				if (ituStopAnywhereFindLoop(child))
					return true;
			}
		}
	}

	

	return false;
}

bool ituStopAnywhereCheckDraggableChildXY(ITCTree* tree, int x, int y)
{
	ITCTree *child = tree->child;

	//ITUWidget* checkwidget = (ITUWidget*)child;
	//if (checkwidget && checkwidget->name)
	//	printf("widget name %s\n", checkwidget->name);

	for (child = tree->child; child; child = child->sibling)
	{
		ITUWidget* ww = (ITUWidget*)child;
		if (ww)
		{
			int xx = x - ww->rect.x;
			int yy = y - ww->rect.y;
			//printf("widget name %s\n", ww->name);
			if (ww->type == ITU_TRACKBAR)
			{
				ITUTrackBar* tbar = (ITUTrackBar*)ww;
				//printf("checking %d, %d for %s\n", xx, yy, ww->name);
				if (ituWidgetIsInside(ww, xx, yy) && ituButtonIsPressed(tbar->tracker))
					return true;
			}
			else
			{
				if (ituStopAnywhereCheckDraggableChildXY(child, xx , yy))
					return true;
			}
		}
	}

	return false;
}

int ituStopAnywhereAlignCal(ITUWidget* widget, ITUWidget* sawchild, bool horizontal)
{
	int startBoundary = 0;
	int endBoundary = ((horizontal) ? (widget->rect.width) : (widget->rect.height));
	int resultFirst = 0;
	int resultLast = 0;
	int result = 0;
	int finalchildCenterPos = 0;
	bool debug_printf = false;
	bool checkAvgUnit = false;

	ITCTree *child, *tree = (ITCTree*)sawchild;
	ITUWidget* firstChild = NULL;
	ITUWidget* lastChild = NULL;
	ITUWidget* finalChild = NULL;
	ITUStopAnywhere* saw = (ITUStopAnywhere*)widget;

	if (STOPANYWHERE_ALIGN_AVG_CENTER)
	{
		int baseunit = ituStopAnywhereCheckChildSame(sawchild, horizontal);

		if (baseunit)
		{
			int alignavgshiftsize = ((endBoundary - startBoundary) % baseunit) / 2;

			if ((alignavgshiftsize % 2) == 0)
			{
				startBoundary += alignavgshiftsize;
				endBoundary -= alignavgshiftsize;
			}
		}
	}

	if (tree)
	{
		for (child = tree->child; child; child = child->sibling)
		{
			if (child)
			{
				ITUWidget* item = (ITUWidget*)child;
				int checkValuePos = ((horizontal) ? (item->rect.x + sawchild->rect.x) : (item->rect.y + sawchild->rect.y));
				int checkValueSiz = ((horizontal) ? (item->rect.width) : (item->rect.height));

				if ((checkValuePos < startBoundary) && ((checkValuePos + checkValueSiz) > startBoundary))
				{
					firstChild = item;
					//if ((checkValuePos * -1) <= (checkValueSiz / 2))
					//	resultFirst = checkValuePos * -1;
					//else
					//	resultFirst = (checkValuePos + checkValueSiz) * -1;

					if ((startBoundary - checkValuePos) <= (checkValueSiz / 2))
						resultFirst = startBoundary - checkValuePos;
					else
						resultFirst = ((checkValuePos + checkValueSiz) - startBoundary) * -1;
				}

				if ((checkValuePos < endBoundary) && ((checkValuePos + checkValueSiz) > endBoundary))
				{
					lastChild = item;
					if ((endBoundary - checkValuePos) <= (checkValueSiz / 2))
						resultLast = endBoundary - checkValuePos;
					else
						resultLast = ((checkValuePos + checkValueSiz) - endBoundary) * -1;
				}

				finalChild = (ITUWidget*)child;
				finalchildCenterPos = (horizontal) ? (finalChild->rect.x + finalChild->rect.width / 2 + sawchild->rect.x) : (finalChild->rect.y + finalChild->rect.height / 2 + sawchild->rect.y);
			}
		}
	}

	result = ((abs(resultFirst) <= abs(resultLast)) ? (resultFirst) : (resultLast));

#ifdef STOPANYWHERE_ALIGN_LEFT_FIRST
	if (finalchildCenterPos < endBoundary)
	{
		result = resultLast;
	}
	else
	{
		result = resultFirst;
	}
#endif

	if ((saw->frame == 0) && debug_printf)
		printf("[Align Target Cal] [%d]\n", result);

	return result;
}

void ituStopAnywhereAlignAni(ITUStopAnywhere* saw, ITUWidget* sawchild, int target, bool horizontal)
{
	if (target != 0)
	{
		int ani_pos = (target / STOPANYWHERE_ALIGN_SHIFT_FACTOR);
		bool debug_printf = false;

		if (abs(ani_pos) == 0)
			ani_pos = target;

		if (horizontal)
		{
			if (debug_printf)
				printf("[StopAnywhere doing Align from %d to %d] [frame %d]\n", saw->childX, saw->childX + ani_pos, saw->frame);
			ituWidgetSetX(sawchild, saw->childX + ani_pos);
			saw->childX = ituWidgetGetX(sawchild);
		}
		else
		{
			if (debug_printf)
				printf("[StopAnywhere doing Align from %d to %d] [frame %d]\n", saw->childY, saw->childY + ani_pos, saw->frame);
			ituWidgetSetY(sawchild, saw->childY + ani_pos);
			saw->childY = ituWidgetGetY(sawchild);
		}
	}
}

int find_abs_X(ITUWidget* widget, int x)
{
	ITCTree* tree = (ITCTree*)widget;
	ITCTree* parent = (ITCTree*)tree->parent;
	int result = x;

	while (parent)
	{
		ITUWidget* parentWidget = (ITUWidget*)parent;
		result += parentWidget->rect.x;
		parent = parent->parent;
	}

	return result;
}

int find_abs_Y(ITUWidget* widget, int y)
{
	ITCTree* tree = (ITCTree*)widget;
	ITCTree* parent = (ITCTree*)tree->parent;
	int result = y;

	while (parent)
	{
		ITUWidget* parentWidget = (ITUWidget*)parent;
		result += parentWidget->rect.y;
		parent = parent->parent;
	}

	return result;
}

void ituStopAnywhereSetDrag(ITUStopAnywhere* current_drag_saw, ITUStopAnywhere* target_drag_saw, int argX, int argY)
{
	ITUWidget* current_drag_widget = (ITUWidget*)current_drag_saw;
	ITUWidget* target_drag_widget = (ITUWidget*)target_drag_saw;
	ITUWidget* current_drag_sawchild = StopAnywhereGetVisibleChild(current_drag_saw);
	ITUWidget* target_drag_sawchild  = StopAnywhereGetVisibleChild(target_drag_saw);

	if (ituScene->dragged == current_drag_widget)
	{
		ITCTree* sawTree = (ITCTree*)target_drag_saw;
		ITCTree* parent = sawTree->parent;
		int toX = argX; - target_drag_widget->rect.x;
		int toY = argY; - target_drag_widget->rect.y;

		current_drag_widget->flags &= ~ITU_DRAGGING;
		ituScene->dragged = NULL;
		current_drag_saw->touchX = 0;
		current_drag_saw->touchY = 0;
		current_drag_saw->childX = ituWidgetGetX(current_drag_sawchild);
		current_drag_saw->childY = ituWidgetGetY(current_drag_sawchild);
		current_drag_saw->frame = 0;
		if (STOPANYWHERE_DRAGGING_PIPE)
		{
			current_drag_saw->logXs = 0;
			current_drag_saw->logYs = 0;
		}

		while (parent)
		{
			ITUWidget* parent_widget = (ITUWidget*)parent;
			toX -= parent_widget->rect.x;
			toY -= parent_widget->rect.y;
			parent = parent->parent;
		}

		toX -= target_drag_widget->rect.x;
		toY -= target_drag_widget->rect.y;

		if (ituWidgetIsInside(target_drag_widget, toX, toY))
		{
			uint32_t longclock = itpGetTickCount();
			target_drag_saw->lastclock = longclock; //long press clock init

			target_drag_saw->initX = target_drag_saw->touchX = toX;
			target_drag_saw->initY = target_drag_saw->touchY = toY;

			target_drag_saw->timerX = target_drag_saw->lastTimerX = toX;
			target_drag_saw->timerY = target_drag_saw->lastTimerY = toY;
			target_drag_saw->childX = ituWidgetGetX(target_drag_sawchild);
			target_drag_saw->childY = ituWidgetGetY(target_drag_sawchild);

			target_drag_saw->timerCycleCount = 0;
			target_drag_saw->clock = itpGetTickCount();

			target_drag_widget->flags |= ITU_DRAGGING;
			ituScene->dragged = target_drag_widget;
			printf("API dragging %s\n", target_drag_widget->name);

			if (STOPANYWHERE_DRAGGING_PIPE)
			{
				int i;
				for (i = 0; i < STOPANYWHERE_DRAGGING_PIPE_SIZE; i++)
				{
					target_drag_saw->dragPIPE[i] = -1;
				}
				target_drag_saw->logDrag = ((target_drag_saw->hv_mode == 1) ? (toX) : (toY));
				target_drag_saw->logXs = 0;
				target_drag_saw->logYs = 0;
			}
		}
	}
}

bool ituStopAnywhereUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
	bool result, horizontal;
	ITUStopAnywhere* saw = (ITUStopAnywhere*)widget;
	ITUWidget* sawchild = NULL;
	int vSawActiveId = ituStopAnywhereFindActiveArrayIdx(saw);
	assert(saw);

	sawchild = StopAnywhereGetVisibleChild(saw);

	if (sawchild == NULL)
		return false;

	horizontal = (sawchild->rect.width > widget->rect.width);

	result = ituWidgetUpdateImpl(widget, ev, arg1, arg2, arg3);

	if ((ev == ITU_EVENT_MOUSELONGPRESS) && (widget->flags & ITU_HAS_LONG_PRESS))
	{
		int x = arg2 - widget->rect.x;
		int y = arg3 - widget->rect.y;
		ITUWidget* child_widget = StopAnywhereGetVisibleChild(saw);
		ITCTree* pMainContainerTree = (ITCTree*)child_widget;
		ITCTree* pTreeChild = NULL;

		if (ituWidgetIsInside(widget, x, y))
		{
			int vDebugCount = 0;
			bool vDebugPrint = false;
			int vId = 0;

			//try to check first child should not be a stopanywhere
			if (child_widget)
			{
				if (child_widget->type != ITU_STOPANYWHERE)
				{
					//touch position should be compared with the position of the main container
					if (horizontal)
						saw->logTP = x - child_widget->rect.x;
					else
						saw->logTP = y - child_widget->rect.y;
				}
			}

			for (pTreeChild = pMainContainerTree->child; pTreeChild; pTreeChild = pTreeChild->sibling)
			{
				if (pTreeChild)
				{
					ITUWidget* pChildWidget = (ITUWidget*)pTreeChild;
					int vRectPos1 = (horizontal) ? (pChildWidget->rect.x) : (pChildWidget->rect.y);
					int vRectPos2 = (horizontal) ? (vRectPos1 + pChildWidget->rect.width) : (vRectPos1 + pChildWidget->rect.height);
					if (pChildWidget)
					{
						if (vDebugPrint)
							printf("[SAW] checking item index %d - %s\n", vDebugCount, pChildWidget->name);

						if ((vRectPos1 <= saw->logTP) && (vRectPos2 > saw->logTP) && (saw->logDrag < 0))
						{
							//to memo the dragging item
							saw->logDrag = vRectPos1;
							saw->dragItem = pChildWidget;
							if (vDebugPrint)
								printf("[SAW] Found dragging item %s\n", pChildWidget->name);//
						}
						if (saw->logDrag >= 0) //when found dragging item, start to push it to the end of tree to display above each item
						{
							if (pTreeChild->sibling)
							{
								ITCTree* pSwapTmp = pTreeChild->sibling;
								ITUWidget* pSwapTmpWidget = (ITUWidget*)pSwapTmp;
								//should change the vSawActiveConfigTree when perform itcTreeSwap to make sure our array data sync
								int vSwap = vSawActiveConfigTree[vSawActiveId][vId];
								itcTreeSwap(pTreeChild, pTreeChild->sibling);
								vSawActiveConfigTree[vSawActiveId][vId] = vSawActiveConfigTree[vSawActiveId][vId + 1];
								vSawActiveConfigTree[vSawActiveId][vId + 1] = vSwap;
								if (vDebugPrint)
									printf("[SAW] switch item %s to next %s\n", pChildWidget->name, pSwapTmpWidget->name);
								//due to perform itcTreeSwap(pTreeChild, pTreeChild->sibling) ex: pTreeChild(2) -> 12345 -> 13245
								//for loop next step pTreeChild(2) = pTreeChild->sibling(4) ex: pTreeChild will be 4
								//use pSwapTmp to reset it to correct(2)
								pTreeChild = pSwapTmp;
							}
						}
						vDebugCount++;
					}
				}
				vId++;
			}
		}
	}
	else if (ev == ITU_EVENT_LOAD)
	{
		ITCTree* pMainContainerTree = (ITCTree*)StopAnywhereGetVisibleChild(saw);
		int vTreeCount = itcTreeGetChildCount(pMainContainerTree);
		int i = 0;
		for (i = 0; i < vTreeCount; i++)
		{
			vSawActiveConfigTree[vSawActiveId][i] = i;
		}

		if (saw->itemalignmode)
		{
			int count = 0;
			bool printDebug = false;
			ITCTree *child, *tree = (ITCTree*)sawchild;

			if (tree)
			{
				for (child = tree->child; child; child = child->sibling)
				{
					if (child)
					{
						count++;
						if (printDebug)
						{
							ITUWidget* item = (ITUWidget*)child;

							if (count == 1)
								printf("================================\n[StopAnywhere] [AlignMode : %s]\n", ((horizontal) ? ("Horizontal") : ("Vectical")));

							printf("[%s] item (%d)(%s) at (%d, %d) [next should be at (%d, %d)]\n", widget->name, count, item->name, item->rect.x, item->rect.y,
								item->rect.x + ((horizontal) ? (item->rect.width) : (0)),
								item->rect.y + ((horizontal) ? (0) : (item->rect.height)));
						}
					}
				}
			}
		}
	}
	else if (ev == ITU_EVENT_TIMER)
	{
		if ((widget->flags & ITU_DRAGGING) && (saw->logTP <= 0))
		{
			uint32_t longclock = saw->lastclock;

			if (longclock)
			{
				if ((itpGetTickCount() - longclock) >= STOPANYWHERE_MOUSEDOWN_LONGPRESS_DELAY)
				{
					ITCTree *tree = (ITCTree*)sawchild;
					ituStopAnywhereFindLoop(tree);

					saw->lastclock = 0; //long press clock reset
				}
			}

			if (saw->timerCycleCount < STOPANYWHERE_TIMER_CYCLE_MAX) //17ms x 14 ~ 250ms
				saw->timerCycleCount++;
			else
			{
				saw->timerCycleCount = 0;
				saw->clock = itpGetTickCount();
				//saw->lastTimerX = ituWidgetGetX(sawchild);
				//saw->lastTimerY = ituWidgetGetY(sawchild);
				saw->lastTimerX = saw->timerX;
				saw->lastTimerY = saw->timerY;
			}

			if (STOPANYWHERE_DRAGGING_PIPE)
			{
				if (saw->logXs < saw->logYs)
				{
					int distX = 0;
					int distY = 0;

					if (saw->hv_mode == 1)
					{
						distX = saw->dragPIPE[saw->logXs % STOPANYWHERE_DRAGGING_PIPE_SIZE];
					}
					else
					{
						distY = saw->dragPIPE[saw->logXs % STOPANYWHERE_DRAGGING_PIPE_SIZE];
					}

					ituStopAnywhereSetXY(saw, (saw->childX + distX), (saw->childY + distY), __LINE__);

					saw->childX = ituWidgetGetX(sawchild);
					saw->childY = ituWidgetGetY(sawchild);

					//printf("drag arr logXs[%d] to child[%d] distX[%d]\n", saw->logXs, saw->childX, distX);

					saw->logXs++;
				}
				else
				{
				}
			}
		}
		else if (widget->flags & ITU_UNDRAGGING)
		{
			bool debug_msg = false;

			if (saw->stopAnywhereFlags & ITU_STOPANYWHERE_BOUNCING)
			{
				saw->frame += 2;

				if (saw->bounce == 1) //Horizontal bounce
				{
					int baseunit = ituStopAnywhereCheckChildSame(sawchild, true);
					int alignavgshiftsize = (widget->rect.width % baseunit) / 2;

					if (saw->itemalignmode)
					{
						alignavgshiftsize = ituStopAnywhereAlignCal(widget, sawchild, true);
					}

					if (!STOPANYWHERE_ALIGN_AVG_CENTER && ((alignavgshiftsize % 2) != 0))
						alignavgshiftsize = 0;

					if (saw->frame >= saw->totalframe)
					{
						if (saw->stopAnywhereFlags & ITU_STOPANYWHERE_BOUNCING_L)
						{
							saw->stopAnywhereFlags &= ~ITU_STOPANYWHERE_BOUNCING_L;
							ituWidgetSetX(sawchild, 0 + alignavgshiftsize);
						}
						else if (saw->stopAnywhereFlags & ITU_STOPANYWHERE_BOUNCING_R)
						{
							saw->stopAnywhereFlags &= ~ITU_STOPANYWHERE_BOUNCING_R;
							ituWidgetSetX(sawchild, (widget->rect.width - sawchild->rect.width) - alignavgshiftsize);
						}

						if (debug_msg)
							printf("[StopAnywhere][Undragging][X to %d][Frame %d]\n", sawchild->rect.x, saw->frame);

						saw->frame = 0;
						saw->stopAnywhereFlags &= ~ITU_STOPANYWHERE_BOUNCING;
						widget->flags &= ~ITU_UNDRAGGING;
						ituExecActions(widget, saw->actions, ITU_EVENT_STOPPED, 0);
					}
					else
					{
						int ani_pos = saw->childX;

						if (saw->stopAnywhereFlags & ITU_STOPANYWHERE_BOUNCING_L)
							ani_pos = ((saw->childX - alignavgshiftsize) * (saw->totalframe - saw->frame) / saw->totalframe) + alignavgshiftsize;
						else if (saw->stopAnywhereFlags & ITU_STOPANYWHERE_BOUNCING_R)
							ani_pos = saw->childX + ((widget->rect.width - alignavgshiftsize - (saw->childX + sawchild->rect.width)) * saw->frame / saw->totalframe);

						ituWidgetSetX(sawchild, ani_pos);

						if (debug_msg)
							printf("[StopAnywhere][Undragging][X to %d][Frame %d]\n", sawchild->rect.x, saw->frame);
					}
				}
				else if (saw->bounce == 2) //Vertical bounce
				{
					int baseunit = ituStopAnywhereCheckChildSame(sawchild, false);
					int alignavgshiftsize = (widget->rect.height % baseunit) / 2;

					if (saw->itemalignmode)
					{
						alignavgshiftsize = ituStopAnywhereAlignCal(widget, sawchild, true);
					}

					if (!STOPANYWHERE_ALIGN_AVG_CENTER && ((alignavgshiftsize % 2) != 0))
						alignavgshiftsize = 0;

					if (saw->frame >= saw->totalframe)
					{
						if (saw->stopAnywhereFlags & ITU_STOPANYWHERE_BOUNCING_T)
						{
							saw->stopAnywhereFlags &= ~ITU_STOPANYWHERE_BOUNCING_T;
							ituWidgetSetY(sawchild, 0 + alignavgshiftsize);
						}
						else if (saw->stopAnywhereFlags & ITU_STOPANYWHERE_BOUNCING_B)
						{
							saw->stopAnywhereFlags &= ~ITU_STOPANYWHERE_BOUNCING_B;
							ituWidgetSetY(sawchild, (widget->rect.height - sawchild->rect.height) - alignavgshiftsize);
						}

						if (debug_msg)
							printf("[StopAnywhere][Undragging][Y to %d][Frame %d]\n", sawchild->rect.y, saw->frame);

						saw->frame = 0;
						saw->stopAnywhereFlags &= ~ITU_STOPANYWHERE_BOUNCING;
						widget->flags &= ~ITU_UNDRAGGING;
						ituExecActions(widget, saw->actions, ITU_EVENT_STOPPED, 0);
					}
					else
					{
						int ani_pos = saw->childY;

						if (saw->stopAnywhereFlags & ITU_STOPANYWHERE_BOUNCING_T)
							ani_pos = ((saw->childY - alignavgshiftsize) * (saw->totalframe - saw->frame) / saw->totalframe) + alignavgshiftsize;
						else if (saw->stopAnywhereFlags & ITU_STOPANYWHERE_BOUNCING_B)
							ani_pos = saw->childY + ((widget->rect.height - alignavgshiftsize - (saw->childY + sawchild->rect.height)) * saw->frame / saw->totalframe);

						ituWidgetSetY(sawchild, ani_pos);

						if (debug_msg)
							printf("[StopAnywhere][Undragging][Y to %d][Frame %d]\n", sawchild->rect.y, saw->frame);
					}
				}
			}
			else if (saw->itemalignmode)
			{
				int target = ituStopAnywhereAlignCal(widget, sawchild, horizontal);

				saw->frame++;

				ituStopAnywhereAlignAni(saw, sawchild, target, horizontal);

				if (saw->frame >= saw->totalframe)
				{
					saw->frame = 0;
					widget->flags &= ~ITU_UNDRAGGING;
					ituExecActions(widget, saw->actions, ITU_EVENT_STOPPED, 0);
				}
			}
		}
		else if (saw->stopAnywhereFlags & ITU_STOPANYWHERE_SLIDING)
		{
			int slideX = 0;
			int slideY = 0;

			saw->frame++;
			
			slideX = saw->factorX / (saw->frame + 1);
			slideY = saw->factorY / (saw->frame + 1);

			ituStopAnywhereSetXY(saw, (saw->childX + slideX), (saw->childY + slideY), __LINE__);

			//printf("frame %d, [%d,%d] step dist %d, %d\n", saw->frame, saw->childX, saw->childY, slideX, slideY);

			if ((saw->childX == sawchild->rect.x) && (saw->childY == sawchild->rect.y))
			{
				saw->frame = saw->totalframe;
			}

			saw->childX = ituWidgetGetX(sawchild);
			saw->childY = ituWidgetGetY(sawchild);

			ituExecActions(widget, saw->actions, ITU_EVENT_CHANGED, 0);

			if (saw->frame >= saw->totalframe)
			{
				saw->stopAnywhereFlags &= ~ITU_STOPANYWHERE_SLIDING;
				saw->frame = 0;

				if (saw->stopAnywhereFlags & ITU_STOPANYWHERE_BOUNCING)
				{
					widget->flags |= ITU_UNDRAGGING;
				}
				else if (saw->itemalignmode)
				{
					widget->flags |= ITU_UNDRAGGING;
				}
				else
					ituExecActions(widget, saw->actions, ITU_EVENT_STOPPED, 0);
			}
		}
	}
	else if (ev == ITU_EVENT_MOUSEDOWN)
	{
		if (saw->stopAnywhereFlags & ITU_STOPANYWHERE_SLIDING)
		{
			saw->stopAnywhereFlags &= ~ITU_STOPANYWHERE_SLIDING;
			saw->frame = 0;
			saw->factorX = 0;
			saw->factorY = 0;

			ituUnPressWidget(sawchild);
		}

		if (widget->flags & ITU_DRAGGABLE)
		{
			int x = arg2 - widget->rect.x;
			int y = arg3 - widget->rect.y;
			ITCTree *tree = (ITCTree*)saw;
			bool dragChildFirstCheck = ituStopAnywhereCheckDraggableChildXY(tree, x, y);
			printf("StopAnywhereCheckDraggableChild check %s\n", dragChildFirstCheck ? ("true") : ("false"));

			if (saw->stopAnywhereFlags & ITU_STOPANYWHERE_BOUNCING)
			{
				ituUnPressWidget(sawchild);
			}
			else if ((ituWidgetIsInside(widget, x, y)) && (ituScene->dragged == NULL) && (!dragChildFirstCheck))
			{
				uint32_t longclock = itpGetTickCount();
				saw->lastclock = longclock; //long press clock init

				saw->initX = saw->touchX = x;
				saw->initY = saw->touchY = y;
				//saw->lastTimerX = saw->childX = ituWidgetGetX(sawchild);
				//saw->lastTimerY = saw->childY = ituWidgetGetY(sawchild);

				saw->timerX = saw->lastTimerX = x;
				saw->timerY = saw->lastTimerY = y;
				saw->childX = ituWidgetGetX(sawchild);
				saw->childY = ituWidgetGetY(sawchild);

				saw->timerCycleCount = 0;
				saw->clock = itpGetTickCount();

				widget->flags |= ITU_DRAGGING;
				ituScene->dragged = widget;

				if (STOPANYWHERE_DRAGGING_PIPE)
				{
					int i;
					for (i = 0; i < STOPANYWHERE_DRAGGING_PIPE_SIZE; i++)
					{
						saw->dragPIPE[i] = -1;
					}
					saw->logDrag = ((saw->hv_mode == 1) ? (x) : (y));
					saw->logXs = 0;
					saw->logYs = 0;
				}
			}
		}
	}
	else if (ev == ITU_EVENT_MOUSEUP)
	{
		//reset long press dragging related condition
		if (saw->logTP >= 0)
		{
			ITUWidget* pItemCurr = saw->dragItem;
			ITCTree* pMainContainerTree = (ITCTree*)StopAnywhereGetVisibleChild(saw);
			ITCTree* pTreeChild = NULL;
			int vTreeCount = itcTreeGetChildCount(pMainContainerTree);
			int i = vTreeCount;
			int vPos = 0;

			//Bubble sort
			while (--i > 0)
			{
				int vId = 0;
				for (pTreeChild = pMainContainerTree->child; pTreeChild; pTreeChild = pTreeChild->sibling)
				{
					if (pTreeChild->sibling)
					{
						ITUWidget* pChild = (ITUWidget*)pTreeChild;
						ITUWidget* pChildNext = (ITUWidget*)pTreeChild->sibling;
						int vChildPos = (horizontal) ? (pChild->rect.x) : (pChild->rect.y);
						int vChildPosNext = (horizontal) ? (pChildNext->rect.x) : (pChildNext->rect.y);
						if (vChildPos > vChildPosNext)
						{
							//switch item order and index array
							//should change the vSawActiveConfigTree when perform itcTreeSwap to make sure our array data sync
							int vSwap = vSawActiveConfigTree[vSawActiveId][vId];
							itcTreeSwap(pTreeChild, pTreeChild->sibling);
							vSawActiveConfigTree[vSawActiveId][vId] = vSawActiveConfigTree[vSawActiveId][vId + 1];
							vSawActiveConfigTree[vSawActiveId][vId + 1] = vSwap;
						}
					}
					vId++;
				}
			}
			//rearrange position
			for (pTreeChild = pMainContainerTree->child; pTreeChild; pTreeChild = pTreeChild->sibling)
			{
				ITUWidget* pChild = (ITUWidget*)pTreeChild;
				int vChildPos = (horizontal) ? (pChild->rect.x) : (pChild->rect.y);
				if (horizontal)
				{
					ituWidgetSetX(pChild, vPos);
					vPos += pChild->rect.width;
				}
				else
				{
					ituWidgetSetY(pChild, vPos);
					vPos += pChild->rect.height;
				}
			}

			saw->logTP = -1;
			saw->logDrag = -1;
			saw->lastclock = 0;
		}

		if ((widget->flags & ITU_DRAGGING) || (saw->stopAnywhereFlags & ITU_STOPANYWHERE_BOUNCING))
		{
			uint32_t timetick = itpGetTickCount();

			if (saw->stopAnywhereFlags & ITU_STOPANYWHERE_BOUNCING)
			{
				ituUnPressWidget(sawchild);
			}

			if ((timetick - saw->clock) <= STOPANYWHERE_SLIDE_CHECK_TIMING)
			{
				//int valueX = ituWidgetGetX(sawchild) - saw->lastTimerX;
				//int valueY = ituWidgetGetY(sawchild) - saw->lastTimerY;
				int valueX = saw->timerX - saw->lastTimerX;
				int valueY = saw->timerY - saw->lastTimerY;

				int lastDistX = abs(valueX);
				int lastDistY = abs(valueY);
				int lastDist = (lastDistX * lastDistX) + (lastDistY * lastDistY);

				if (lastDist > STOPANYWHERE_SLIDE_REQUIRE_DIST)
				{
					saw->factorX = valueX * STOPANYWHERE_SLIDE_FACTOR;
					saw->factorY = valueY * STOPANYWHERE_SLIDE_FACTOR;

					saw->frame = 0;
					saw->stopAnywhereFlags |= ITU_STOPANYWHERE_SLIDING;

					if (STOPANYWHERE_DRAGGING_PIPE)
					{
						//ituStopAnywhereSetXY(saw, (saw->childX + distX), (saw->childY + distY), __LINE__);
						saw->logXs = 0;
						saw->logYs = 0;
					}


					printf("Sliding factorX %d, factorY %d\n", saw->factorX, saw->factorY);
				}
				else
				{
					printf("[1800][lastDist, lastDistX, lastDistY] [%d, %d, %d]\n", lastDist, lastDistX, lastDistY);
				}
			}
			else
				printf("[%d][Clock][Tick, saw_last_clock][%d, %d]\n", STOPANYWHERE_SLIDE_CHECK_TIMING, timetick, saw->clock);

			saw->touchX = 0;
			saw->touchY = 0;
			saw->childX = ituWidgetGetX(sawchild);
			saw->childY = ituWidgetGetY(sawchild);

			widget->flags &= ~ITU_DRAGGING;
			ituScene->dragged = NULL;

			if (!(saw->stopAnywhereFlags & ITU_STOPANYWHERE_SLIDING))
			{
				if ((saw->stopAnywhereFlags & ITU_STOPANYWHERE_BOUNCING) || (saw->itemalignmode))
					widget->flags |= ITU_UNDRAGGING;
				else
					ituExecActions(widget, saw->actions, ITU_EVENT_STOPPED, 0);

				saw->frame = 0;
				ituWidgetSetDirty(widget, true);
			}
		}
	}
	else if (ev == ITU_EVENT_MOUSEMOVE)
	{
		if ((widget->flags & ITU_DRAGGING) && (saw->logTP >= 0))
		{
			ITUWidget* pItemCurr = saw->dragItem;
			ITUWidget* pMainContainerWidget = StopAnywhereGetVisibleChild(saw);
			ITCTree* pMainContainerTree = (ITCTree*)pMainContainerWidget;
			ITCTree* pTreeChild = NULL;
			//touch position should be compared with the position of the main container
			int x = arg2 - widget->rect.x - pMainContainerWidget->rect.x;
			int y = arg3 - widget->rect.y - pMainContainerWidget->rect.y;
			int vDist = (horizontal) ? (x - saw->logTP) : (y - saw->logTP);

			if (pItemCurr)
			{
				//check dragging item move limit
				bool vCheckMoveRule = true;
				if (horizontal)
				{
					if ((pItemCurr->rect.x + vDist) <= (0 - pMainContainerWidget->rect.x))
						vCheckMoveRule = false;
					else if ((pItemCurr->rect.x + vDist) >= (0 - pMainContainerWidget->rect.x + widget->rect.width))
						vCheckMoveRule = false;
				}
				else
				{
					if ((pItemCurr->rect.y + vDist) <= (0 - pMainContainerWidget->rect.y))
						vCheckMoveRule = false;
					else if ((pItemCurr->rect.y + vDist) >= (0 - pMainContainerWidget->rect.y + widget->rect.height))
						vCheckMoveRule = false;
				}

				if (vCheckMoveRule)
				{
					if (horizontal)
					{
						ituWidgetSetX(pItemCurr, pItemCurr->rect.x + vDist);
						saw->logTP = x;
					}
					else
					{
						ituWidgetSetY(pItemCurr, pItemCurr->rect.y + vDist);
						saw->logTP = y;
					}
				}

				for (pTreeChild = pMainContainerTree->child; pTreeChild; pTreeChild = pTreeChild->sibling)
				{
					ITUWidget* pChildWidget = (ITUWidget*)pTreeChild;
					//center of the tree child
					int vRectPos1 = (horizontal) ? (pChildWidget->rect.x) : (pChildWidget->rect.y);
					int vRectPos2 = (horizontal) ? (vRectPos1 + pChildWidget->rect.width / 2) : (vRectPos1 + pChildWidget->rect.height / 2);
					//boundary of the pItemCurr
					int vRectPosCurr1 = (horizontal) ? (pItemCurr->rect.x) : (pItemCurr->rect.y);
					int vRectPosCurr2 = (horizontal) ? (vRectPosCurr1 + pItemCurr->rect.width) : (vRectPosCurr1 + pItemCurr->rect.height);

					if ((!pTreeChild) || (pTreeChild == pItemCurr))
						continue;

					if (vDist < 0) //move to left or top
					{
						//drawing item move left/top and over center line of another item
						if ((vRectPosCurr1 <= vRectPos2) && (vRectPosCurr1 > vRectPos1))
						{
							if (horizontal)
								ituWidgetSetX(pChildWidget, pChildWidget->rect.x + pItemCurr->rect.width);
							else
								ituWidgetSetY(pChildWidget, pChildWidget->rect.y + pItemCurr->rect.height);

							break;
						}
					}
					else //move to right ot bottom
					{
						//drawing item move right/bottom and over center line of another item
						if ((vRectPosCurr2 >= vRectPos2) && (vRectPosCurr1 < vRectPos1))
						{
							if (horizontal)
								ituWidgetSetX(pChildWidget, pChildWidget->rect.x - pItemCurr->rect.width);
							else
								ituWidgetSetY(pChildWidget, pChildWidget->rect.y - pItemCurr->rect.height);

							break;
						}
					}
				}
			}
		}
		else if (widget->flags & ITU_DRAGGING)
		{
			int x = arg2 - widget->rect.x;
			int y = arg3 - widget->rect.y;
			int distX = x - saw->touchX;
			int distY = y - saw->touchY;
			int Xtd = x - saw->initX;
			int Ytd = y - saw->initY;
			bool X_dir = (abs(distX) > abs(distY)) ? (true) : (false);
			bool Y_dir = (abs(distX) < abs(distY)) ? (true) : (false);

			//Overlap operation can not support bounce mode
			if (saw->bounce == 0)
			{
				if (((sawchild->rect.width == widget->rect.width) && X_dir && (abs(Xtd) > STOPANYWHERE_MIN_OL_DRAGACT_DIST)) || ((sawchild->rect.height == widget->rect.height) && Y_dir && (abs(Ytd) > STOPANYWHERE_MIN_OL_DRAGACT_DIST)))
				{
					ITUWidget* child_widget = StopAnywhereGetVisibleChild(saw);
					ITUWidget* child_saw = (ITUStopAnywhere*)child_widget;
					ITCTree* localtree = (ITCTree*)saw;
					ITUWidget* localparent = (ITUWidget*)localtree->parent;
					int find_index = 0;
					bool finding_check = true;

					//try to find child stopanywhere first
					while (child_widget)
					{
						if (child_widget->type == ITU_STOPANYWHERE)
						{
							ITCTree* point = (ITCTree*)child_widget;
							ITCTree* parent = point->parent;
							ITCTree* child;
							ITUWidget* parentW = (ITUWidget*)parent;
							for (child = parent->child; child; child = child->sibling)
							{
								if (child)
								{
									ITUWidget* childw = (ITUWidget*)child;

									int fix_X = widget->rect.x + childw->rect.x + parentW->rect.x;
									int fix_Y = widget->rect.y + childw->rect.y + parentW->rect.y;
									//printf("ContainerTop %d, %d\n", parentW->rect.x, parentW->rect.y);

									if (ituWidgetIsInside(childw, arg2 - fix_X, arg3 - fix_Y))
									{
										ITUStopAnywhere* target = (ITUStopAnywhere*)childw;
										ituStopAnywhereSetDrag(saw, target, find_abs_X(widget, arg2), find_abs_Y(widget, arg3));
										finding_check = false;
										break;
									}
								}
							}

							break;
						}
						else
						{
							child_widget = StopAnywhereGetVisibleChild(child_widget);
						}
					}

					if (finding_check)
					{
						ITCTree* sawTree = (ITCTree*)saw;
						ITCTree* parent = sawTree->parent;

						while (parent)
						{
							ITUWidget* pWidget = (ITUWidget*)parent;
							if (pWidget->type == ITU_STOPANYWHERE)
							{
								ITUStopAnywhere* target = (ITUStopAnywhere*)pWidget;
								ituStopAnywhereSetDrag(saw, target, find_abs_X(widget, arg2), find_abs_Y(widget, arg3));
								finding_check = false;
								break;
							}
							else
								parent = parent->parent;
						}
					}
				}
			}

			saw->timerX = x;
			saw->timerY = y;

			if ((abs(Xtd) >= ITU_DRAG_DISTANCE) || (abs(Ytd) >= ITU_DRAG_DISTANCE))
			{
				saw->lastclock = 0; //long press clock init

				ituUnPressWidget(sawchild);
				//printf("[Unpress] x,y[%d,%d] init[%d,%d]\n", x, y, saw->initX, saw->initY);
			}

			if (ituWidgetIsInside(widget, x, y))
			{
				//ituStopAnywhereSetXY(saw, (saw->childX + distX), (saw->childY + distY), __LINE__);
				saw->touchX = x;
				saw->touchY = y;
				saw->childX = ituWidgetGetX(sawchild);
				saw->childY = ituWidgetGetY(sawchild);

				ituExecActions(widget, saw->actions, ITU_EVENT_CHANGED, 0);

				if (STOPANYWHERE_DRAGGING_PIPE)
				{
					int i = saw->logDrag;
					int target = ((horizontal) ? (x) : (y));
					int count = 0;
					int avg = abs(i - target) / 3;
					int mode = ((i < target) ? (0) : (1));
					bool working = true;

					if (avg == 0)
						avg = 1;

					while (working)
					{
						//int ddd;
						int step = ((i < target) ? (avg) : (-avg));
						i += step;

						if ((i >= target) && (mode == 0))
						{
							if (i > target)
								step -= (i - target);
							working = false;
						}
						else if ((i <= target) && (mode == 1))
						{
							if (i < target)
								step += (target - i);
							working = false;
						}

						saw->dragPIPE[saw->logYs % STOPANYWHERE_DRAGGING_PIPE_SIZE] = step; //i - saw->logDrag;

						//ddd = arr[saw->temp2 % STOPANYWHERE_DRAGGING_PIPE_SIZE];
						//ddd++;

						saw->logYs++;
						count++;
					}

					saw->logDrag = target;
					//printf("drag arr item %d arranged.\n", count);
				}
				else
				{
					ituStopAnywhereSetXY(saw, (saw->childX + distX), (saw->childY + distY), __LINE__);
				}
			}
		}
	}

	result |= widget->dirty;

	//if (result)
	//	printf("[StopAnywhere][%s][Update True].\n", widget->name);

	return widget->visible ? result : false;
}

void ituStopAnywhereDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
	int destx, desty;
	uint8_t desta;
	ITUStopAnywhere* saw = (ITUStopAnywhere*)widget;
	ITURectangle* rect = &widget->rect;
	ITCTree* node;
	ITURectangle prevClip;
	assert(saw);
	assert(dest);

	if (!widget->visible)
		return;

	destx = rect->x + x;
	desty = rect->y + y;
	desta = alpha * widget->alpha / 255;

	if (widget->angle == 0)
	{
		for (node = widget->tree.child; node; node = node->sibling)
		{
			ITUWidget* child = (ITUWidget*)node;

			if (child == NULL)
				continue;

			//if ((widget->flags & ITU_DRAGGING) && (child != ituScene->dragged))
			ituWidgetSetClipping(widget, dest, x, y, &prevClip);

			if (child->visible && ituWidgetIsOverlapClipping(child, dest, destx, desty))
			{
				bool motiondebug = false;
				bool movecheck = false;

				if ((abs(saw->logXs - saw->childX) >= 2) || (abs(saw->logYs - saw->childY) >= 2))
					movecheck = true;

				if (STOPANYWHERE_DRAGGING_PIPE)
					movecheck = false;

				if ((widget->flags & ITU_DRAGGING) && (STOPANYWHERE_DRAGGING_EYEMOTION) && movecheck)
				{
					int x_step = saw->logXs - saw->childX;
					int y_step = saw->logYs - saw->childY;

					

					if (STOPANYWHERE_DRAGGING_EYEMOTION == 2)
					{
						ituWidgetDraw(node, dest, destx + (x_step / 2), desty + (y_step / 2), desta / 2);
						if (motiondebug)
							printf("[StopAnywhere]eye_motion[1][%d,%d]\n\n", destx + (x_step / 2), desty + (y_step / 2));

						ituWidgetDraw(node, dest, destx + x_step, desty + y_step, desta / 4);
						if (motiondebug)
							printf("[StopAnywhere]eye_motion[2][%d,%d]\n", destx + x_step, desty + y_step);

						
					}
					else if (STOPANYWHERE_DRAGGING_EYEMOTION == 1)
					{
						/*ituWidgetDraw(node, dest, destx + x_step, desty + y_step, desta / 4);
						if (motiondebug)
							printf("[StopAnywhere]eye_motion[1][%d,%d]\n", destx + x_step, desty + y_step);*/

						ituWidgetDraw(node, dest, destx + (x_step / 2), desty + (y_step / 2), desta / 4);
						if (motiondebug)
							printf("[StopAnywhere]eye_motion[1][%d,%d]\n\n", destx + (x_step / 2), desty + (y_step / 2));
					}

					ituWidgetDraw(node, dest, destx, desty, desta);

					saw->logXs = saw->childX;
					saw->logYs = saw->childY;
				}
				else
					ituWidgetDraw(node, dest, destx, desty, desta);

				child->dirty = false;
			}

			//if ((widget->flags & ITU_DRAGGING) && (child != ituScene->dragged))
			ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
		}
	}
	else
	{
		ITUSurface* surf;

		surf = ituCreateSurface(rect->width, rect->height, 0, ITU_ARGB8888, NULL, 0);
		if (surf)
		{
			ITUColor color = { 0, 0, 0, 0 };

			ituColorFill(surf, 0, 0, rect->width, rect->height, &color);

			for (node = widget->tree.child; node; node = node->sibling)
			{
				ITUWidget* child = (ITUWidget*)node;

				if (child->visible && ituWidgetIsOverlapClipping(child, dest, destx, desty))
					ituWidgetDraw(node, surf, 0, 0, desta);

				child->dirty = false;
			}

#ifdef CFG_WIN32_SIMULATOR
			ituRotate(dest, destx + rect->width / 2, desty + rect->height / 2, surf, surf->width / 2, surf->height / 2, (float)widget->angle, 1.0f, 1.0f);
#else
			ituRotate(dest, destx, desty, surf, surf->width / 2, surf->height / 2, (float)widget->angle, 1.0f, 1.0f);
#endif
			ituDestroySurface(surf);
		}
	}

	ituDirtyWidget(widget, false);
}

void ituStopAnywhereOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
	ITUStopAnywhere* saw = (ITUStopAnywhere*)widget;
	assert(saw);

	switch (action)
	{
	case ITU_ACTION_RELOAD:
		ituStopAnywhereUpdate(widget, ITU_EVENT_LAYOUT, 0, 0, 0);
		break;

	default:
		ituWidgetOnActionImpl(widget, action, param);
		break;
	}
}

void ituStopAnywhereInit(ITUStopAnywhere* saw)
{
    assert(saw);
    ITU_ASSERT_THREAD();

    memset(saw, 0, sizeof (ITUStopAnywhere));

	ituWidgetInit(&saw->widget);
    
    ituWidgetSetType(saw, ITU_STOPANYWHERE);
    ituWidgetSetName(saw, sawName);
	ituWidgetSetUpdate(saw, ituStopAnywhereUpdate);
	ituWidgetSetDraw(saw, ituStopAnywhereDraw);
	ituWidgetSetOnAction(saw, ituStopAnywhereOnAction);
	ituWidgetSetClone(saw, ituStopAnywhereClone);
}

void ituStopAnywhereLoad(ITUStopAnywhere* saw, uint32_t base)
{
	int vIdx = 0;
    assert(saw);

	//saw->bounce = 2;
	ituWidgetLoad(&saw->widget, base);
	ituWidgetSetUpdate(saw, ituStopAnywhereUpdate);
	ituWidgetSetDraw(saw, ituStopAnywhereDraw);
	ituWidgetSetOnAction(saw, ituStopAnywhereOnAction);
	ituWidgetSetClone(saw, ituStopAnywhereClone);

	for (vIdx = 0; vIdx < 10; vIdx++)
	{
		if (vSawActiveWidgetIndex[vIdx] == NULL)
		{
			vSawActiveWidgetIndex[vIdx] = (ITUWidget*)saw;
			break;
		}
	}

	if (STOPANYWHERE_DRAGGING_PIPE)
	{
		ITUWidget* widget = (ITUWidget*)saw;
		ITUWidget* sawchild = StopAnywhereGetVisibleChild(saw);
		saw->logTP = -1;

		if (((sawchild->rect.width - ituWidgetGetWidth(saw)) > 100) && (sawchild->rect.height <= ituWidgetGetHeight(saw)))
			saw->hv_mode = 1; //Horizontal
		else if (((sawchild->rect.height - ituWidgetGetHeight(saw)) > 100) && (sawchild->rect.width <= ituWidgetGetWidth(saw)))
			saw->hv_mode = 2; //Vertical

		saw->logXs = 0;
		saw->logYs = 0;
	}
	else
	{
		//use logTP to memo long press position x or y (horizontal or vertical mode)
		//use logDrag to memo long press dragging item initial position x or y (horizontal or vertical mode)
		saw->logTP = -1;
		saw->logDrag = -1;
	}
}

int ituStopAnywhereGetChildX(ITUStopAnywhere* saw)
{
	assert(saw);

	return saw->childX;
}

int ituStopAnywhereGetChildY(ITUStopAnywhere* saw)
{
	assert(saw);

	return saw->childY;
}

void ituStopAnywhereResetXY(ITUStopAnywhere* saw)
{
	assert(saw);

	ituStopAnywhereSetXY(saw, 0, 0, __LINE__);
}

void ituStopAnywhereSyncTreeData(ITUStopAnywhere* saw, char* pConfigWord)
{
	ITCTree* pMainContainerTree = (ITCTree*)StopAnywhereGetVisibleChild(saw);
	int vTreeCount = itcTreeGetChildCount(pMainContainerTree);
	int vSawActiveId = ituStopAnywhereFindActiveArrayIdx(saw);
	int i;
	int len = 0;
	char* pChar = (char*)malloc(sizeof(char)* 256);
	memset(pChar, '\0', sizeof(char) * 256);
	

	if (vTreeCount > STOPANYWHERE_MAX_TREE_ITEM)
		return;

	for (i = 0; i < vTreeCount; i++)
	{
		if (vSawActiveConfigTree[vSawActiveId][i] >= 0)
		{
			char vTmp[10] = { '\0' };
			sprintf(vTmp, "%d%s", vSawActiveConfigTree[vSawActiveId][i], ((i + 1) < vTreeCount) ? (",") : (""));
			strcat(pChar, vTmp);
		}
	}

	len = strlen(pChar);

	if (len > 0)
	{
		memset(pConfigWord, '\0', sizeof(char)* 256);
		memcpy(pConfigWord, pChar, sizeof(char)* len);
	}

	free(pChar);
}

bool ituStopAnywhereLoadTreeSeq(ITUStopAnywhere* saw, char* pConfigWord)
{
	bool vPrintDebug = true;

	ITCTree* pMainContainerTree = (ITCTree*)StopAnywhereGetVisibleChild(saw);
	ITUWidget* pMainContainerWidget = (ITUWidget*)pMainContainerTree;
	ITCTree* pTreeChild = NULL;
	int vTreeCount = itcTreeGetChildCount(pMainContainerTree);
	int vSawActiveId = ituStopAnywhereFindActiveArrayIdx(saw);
	ITCTree** pArrayOrg = (ITCTree**)malloc(sizeof(ITCTree*)* vTreeCount);
	int i = 0;
	int vPos = 0;
	int vConfigArr[STOPANYWHERE_MAX_TREE_ITEM] = { -1 };
	char *vDelim = ",";
	char *vStr = strtok(pConfigWord, vDelim);
	memset(vConfigArr, -1, sizeof(int)* STOPANYWHERE_MAX_TREE_ITEM);

	vConfigArr[i] = atoi(vStr);
	if ((vConfigArr[i] < 0) || (vConfigArr[i] >= vTreeCount))
		return false;
	else
		vSawActiveConfigTree[vSawActiveId][i] = vConfigArr[i];

	i = 1;
	while (vStr != NULL)
	{
		vStr = strtok(NULL, vDelim);
		if (vStr)
		{
			vConfigArr[i] = atoi(vStr);
			vSawActiveConfigTree[vSawActiveId][i] = vConfigArr[i];
			i++;
		}
	}

	if (vPrintDebug)
	{
		i = 0;
		printf("=====================================================\n");
		printf("[StopAnywhere] Loaded Tree Seq for %s\n", saw->widget.name);
		while ((vConfigArr[i] >= 0) && (i < STOPANYWHERE_MAX_TREE_ITEM))
		{
			printf("%d", vConfigArr[i]);
			if (vConfigArr[i + 1] >= 0)
				printf(", ");
			i++;
		}
		printf("\n=====================================================\n");
	}

	i = 0;
	for (pTreeChild = pMainContainerTree->child; pTreeChild; pTreeChild = pTreeChild->sibling)
	{
		pArrayOrg[i] = pTreeChild;
		//pTreeChild->parent = NULL;
		//pTreeChild->child = NULL;

		if (vPrintDebug)
		{
			ITUWidget* pTmp = (ITUWidget*)pArrayOrg[i];
			printf("[StopAnywhere] pArrayOrg[%d] is %s\n", i, pTmp->name);
		}

		i++;
	}

	for (i = 0; i < (vTreeCount - 1); i++)
	{
		if (vConfigArr[i] < 0)
			break;
		else
		{
			if (vConfigArr[i + 1] >= 0)
			{
				pArrayOrg[vConfigArr[i]]->sibling = pArrayOrg[vConfigArr[i + 1]];
				if (vPrintDebug)
				{
					ITUWidget* pTmp1 = (ITUWidget*)pArrayOrg[vConfigArr[i]];
					ITUWidget* pTmp2 = (ITUWidget*)pArrayOrg[vConfigArr[i + 1]];
					printf("[StopAnywhere] set %s->sibling to %s\n", pTmp1->name, pTmp2->name);
				}
			}

			if (i == (vTreeCount - 2))
			{
				pArrayOrg[vConfigArr[i + 1]]->sibling = NULL;
			}

			if (i == 0)
			{
				pMainContainerTree->child = pArrayOrg[vConfigArr[i]];
				//pArrayOrg[vConfigArr[i]]->parent = pMainContainerTree;
			}
		}
	}
	
	//rearrange position
	for (pTreeChild = pMainContainerTree->child; pTreeChild; pTreeChild = pTreeChild->sibling)
	{
		ITUWidget* pChild = (ITUWidget*)pTreeChild;
		bool vHorizontal = (pMainContainerWidget->rect.width >= pMainContainerWidget->rect.height);

		if (vPrintDebug)
			printf("[StopAnywhere] set %s %s to %d\n", pChild->name, (vHorizontal) ? ("x") : ("y"), vPos);

		if (vHorizontal)
		{
			ituWidgetSetX(pChild, vPos);
			vPos += pChild->rect.width;
		}
		else
		{
			ituWidgetSetY(pChild, vPos);
			vPos += pChild->rect.height;
		}
	}

	free(pArrayOrg);
	return true;
}