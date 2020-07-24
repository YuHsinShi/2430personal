#include <assert.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char tableGridName[] = "ITUTableGrid";

void ituTableGridExit(ITUWidget* widget)
{
    ITCTree* node;

    for (node = widget->tree.child; node; node = node->sibling)
    {
        ITCTree* childnode;

        for (childnode = node->child; childnode; childnode = childnode->sibling)
        {
            //ITUWidget* item = (ITUWidget*)ituWidgetGetCustomData(childnode);
			ITUWidget* childwidget = (ITUWidget*)childnode;
			ITUWidget* item = (ITUWidget*)childwidget->pObj;
            if (item && (item->flags & ITU_DYNAMIC))
            {
                ituWidgetExit(item);
            }
        }
    }
}

static bool TableGridItemUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    //ITUWidget* item = (ITUWidget*)ituWidgetGetCustomData(widget);
	ITUWidget* item = (ITUWidget*)widget->pObj;
    if (item)
    {
        return ituWidgetUpdate(item, ev, arg1, arg2, arg3);
    }
    return ituScrollTextUpdate(widget, ev, arg1, arg2, arg3);
}

static void TableGridItemDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    //ITUWidget* item = (ITUWidget*)ituWidgetGetCustomData(widget);
	ITUWidget* item = (ITUWidget*)widget->pObj;
    ituScrollTextDraw(widget, dest, x, y, alpha);

    if (item)
    {
        ituWidgetDraw(item, dest, x + widget->rect.x - item->rect.x, y + widget->rect.y - item->rect.y, alpha);
    }
}

bool ituTableGridUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result;
    ITUTableGrid* grid = (ITUTableGrid*) widget;
    assert(grid);

    result = ituIconUpdate(widget, ev, arg1, arg2, arg3);

    if (ev == ITU_EVENT_LAYOUT)
    {
        ITCTree* node;
        int x = 0;

        for (node = widget->tree.child; node; node = node->sibling)
        {
            ITUTableListBox* tlistbox = (ITUTableListBox*)node;
            
            tlistbox->listbox.itemCount = grid->lastRow + 1;
            ituWidgetSetPosition(node, x, 0);
            ituWidgetSetHeight(node, widget->rect.height);
            x += ituWidgetGetWidth(node);

            ITCTree* childnode;

            for (childnode = node->child; childnode; childnode = childnode->sibling)
            {
                ituWidgetSetUpdate(childnode, TableGridItemUpdate);
                ituWidgetSetDraw(childnode, TableGridItemDraw);
            }
        }
    }
    return widget->visible ? result : false;
}

void ituTableGridDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    ITURectangle prevClip;
    ITUTableGrid* grid = (ITUTableGrid*)widget;
    assert(grid);
    assert(dest);

    ituBackgroundDraw(widget, dest, x, y, alpha);

    if (grid->lineSize > 0)
    {
        int destx, desty;
        uint8_t desta;
        ITURectangle* rect = (ITURectangle*)&widget->rect;

        ituWidgetSetClipping(widget, dest, x, y, &prevClip);

        destx = rect->x + x;
        desty = rect->y + y;
        desta = alpha * widget->color.alpha / 255;
        desta = desta * widget->alpha / 255;

        if (desta == 255)
        {
            ITCTree* node;
            int xx = 0;

            for (node = widget->tree.child; node; node = node->sibling)
            {
                ITUWidget* child = (ITUWidget*)node;

                xx += ituWidgetGetWidth(child);
                ituColorFill(dest, destx + xx, desty - grid->lineSize, grid->lineSize, rect->height, &grid->lineColor);
            }

            if (widget->tree.child)
            {
                for (node = widget->tree.child->child; node; node = node->sibling)
                {
                    ITUWidget* childchild = (ITUWidget*)node;
                    ituColorFill(dest, destx, desty + childchild->rect.y + childchild->rect.height - grid->lineSize, rect->width, grid->lineSize, &grid->lineColor);
                }
            }

            ituColorFill(dest, destx, desty, rect->width, grid->lineSize, &grid->lineColor);
            ituColorFill(dest, destx, desty + rect->height - grid->lineSize, rect->width, grid->lineSize, &grid->lineColor);
            ituColorFill(dest, destx, desty + grid->lineSize, grid->lineSize, rect->height - grid->lineSize * 2, &grid->lineColor);
            ituColorFill(dest, destx + rect->width - grid->lineSize, desty + grid->lineSize, grid->lineSize, rect->height - grid->lineSize * 2, &grid->lineColor);
        }
        else if (desta > 0)
        {
            ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, ITU_ARGB4444, NULL, 0);
            if (surf)
            {
                ITUColor black = { 0, 0, 0, 0 };
                ituColorFill(surf, 0, 0, rect->width, rect->height, &black);
                ituColorFill(surf, 0, 0, rect->width, grid->lineSize, &grid->lineColor);
                ituColorFill(surf, 0, rect->height - grid->lineSize, rect->width, grid->lineSize, &grid->lineColor);
                ituColorFill(surf, 0, grid->lineSize, grid->lineSize, rect->height - grid->lineSize * 2, &grid->lineColor);
                ituColorFill(surf, rect->width - grid->lineSize, grid->lineSize, grid->lineSize, rect->height - grid->lineSize * 2, &grid->lineColor);
                ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);
                ituDestroySurface(surf);
            }
        }
        ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
    }
}

void ituTableGridOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    ITUTableGrid* tableGrid = (ITUTableGrid*) widget;
    assert(tableGrid);

    switch (action)
    {
    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

ITUWidget* ituTableGridAddItem(ITUTableGrid* grid, ITUWidget* templateItem, bool clone)
{
    ITUWidget* child;
    assert(grid);
    ITU_ASSERT_THREAD();

    child = itcTreeGetChildAt(grid, grid->lastColumn);
    if (child)
    {
        ITUWidget* childchild = (ITUWidget*)itcTreeGetChildAt(child, grid->lastRow);
        if (childchild)
        {
            ITUWidget* cloned = NULL;

            if (clone)
            {
                ituWidgetClone(templateItem, &cloned);
            }
            else
            {
                cloned = templateItem;
            }
            
            if (cloned)
            {
                //ITUWidget* old = ituWidgetGetCustomData(childchild);
				ITUWidget* childwidget = (ITUWidget*)childchild;
				ITUWidget* old = (ITUWidget*)childwidget->pObj;
                if (old && (old->flags & ITU_DYNAMIC))
                {
                    ituWidgetExit(old);
                }

                //ituWidgetSetCustomData(childchild, cloned);
				childwidget->pObj = (ITCTree*)cloned;

                if (grid->lastColumn == 0)
                {
                    ITCTree* node;
                    for (node = ((ITCTree*)grid)->child; node; node = node->sibling)
                    {
                        ITUTableListBox* tlistbox = (ITUTableListBox*)node;
                        tlistbox->listbox.itemCount = grid->lastRow + 1;
                    }
                }

                grid->lastColumn++;

                if (grid->lastColumn >= itcTreeGetChildCount(grid) && (grid->lastRow + 1) < itcTreeGetChildCount(child))
                {
                    ITCTree* node;

                    grid->lastColumn = 0;
                    grid->lastRow++;

                    for (node = ((ITCTree*)grid)->child; node; node = node->sibling)
                    {
                        ITUTableListBox* tlistbox = (ITUTableListBox*)node;
                        tlistbox->listbox.itemCount = grid->lastRow;
                    }
                }
                return cloned;
            }
        }
    }
    LOG_WARN "ituTableGridAddItem() failed" LOG_END
    return NULL;
}

void ituTableGridAddAll(ITUTableGrid* grid, ITUWidget* parent, bool clone)
{
    ITCTree* node;
    assert(grid);
    assert(parent);
    ITU_ASSERT_THREAD();

    for (node = parent->tree.child; node; node = node->sibling)
    {
        if (!ituTableGridAddItem(grid, (ITUWidget*)(node), clone))
            return;
    }
}

ITUWidget* ituTableGridGetItem(ITUTableGrid* grid, int row, int col)
{
    ITUWidget* child;
    assert(grid);
    ITU_ASSERT_THREAD();

    child = itcTreeGetChildAt(grid, col);
    if (child)
    {
        ITUWidget* childchild = (ITUWidget*)itcTreeGetChildAt(child, row);
        if (childchild)
        {
            //return (ITUWidget*)ituWidgetGetCustomData(childchild);
			ITUWidget* childwidget = (ITUWidget*)childchild;
			ITUWidget* item = (ITUWidget*)childwidget->pObj;
			return item;
        }
    }
    return NULL;
}

ITUWidget* ituTableGridSetItem(ITUTableGrid* grid, ITUWidget* templateItem, bool clone, int row, int col)
{
    ITUWidget* child;
    assert(grid);
    ITU_ASSERT_THREAD();

    child = itcTreeGetChildAt(grid, col);
    if (child)
    {
        ITUWidget* childchild = (ITUWidget*)itcTreeGetChildAt(child, row);
        if (childchild)
        {
            ITUWidget* cloned = NULL;

            if (clone)
            {
                ituWidgetClone(templateItem, &cloned);
            }
            else
            {
                cloned = templateItem;
            }

            if (cloned)
            {
                //ITUWidget* old = ituWidgetGetCustomData(childchild);
				ITUWidget* childwidget = (ITUWidget*)childchild;
				ITUWidget* old = (ITUWidget*)childwidget->pObj;
                if (old && (old->flags & ITU_DYNAMIC))
                {
                    ituWidgetExit(old);
                }
                //ituWidgetSetCustomData(childchild, cloned);
				childwidget->pObj = (ITCTree*)cloned;
                return cloned;
            }
        }
    }
    return NULL;
}

void ituTableGridSelect(ITUTableGrid* grid, int row, int col)
{
    ITUWidget* widget = (ITUWidget*)grid;
    assert(grid);
    ITU_ASSERT_THREAD();

    if (grid->tableGridFlags & ITU_TABLEGRID_SINGLE_SELECT)
    {
        if (col >= 0 && row >= 0)
        {
            ITUWidget* child = itcTreeGetChildAt(grid, col);
            if (child)
            {
                ituListBoxSelect((ITUListBox*)(child), row);
            }
        }
        else
        {
            ITCTree* node;
            for (node = widget->tree.child; node; node = node->sibling)
            {
                ituListBoxSelect((ITUListBox*)(node), -1);
            }
        }
    }
    else
    {
        if (row >= 0)
        {
            ITCTree* node;
            for (node = widget->tree.child; node; node = node->sibling)
            {
                ituListBoxSelect((ITUListBox*)(node), row);
            }
        }
        else
        {
            ITCTree* node;
            for (node = widget->tree.child; node; node = node->sibling)
            {
                ituListBoxSelect((ITUListBox*)(node), -1);
            }
        }
    }
}

void ituTableGridInit(ITUTableGrid* tableGrid)
{
    assert(tableGrid);
    ITU_ASSERT_THREAD();

    memset(tableGrid, 0, sizeof (ITUTableGrid));

    ituBackgroundInit(&tableGrid->bg);

    ituWidgetSetType(tableGrid, ITU_TABLEGRID);
    ituWidgetSetName(tableGrid, tableGridName);
    ituWidgetSetExit(tableGrid, ituTableGridExit);
    ituWidgetSetUpdate(tableGrid, ituTableGridUpdate);
    ituWidgetSetDraw(tableGrid, ituTableGridDraw);
    ituWidgetSetOnAction(tableGrid, ituTableGridOnAction);
}

void ituTableGridLoad(ITUTableGrid* tableGrid, uint32_t base)
{
    assert(tableGrid);

    ituBackgroundLoad(&tableGrid->bg, base);
    ituWidgetSetExit(tableGrid, ituTableGridExit);
    ituWidgetSetUpdate(tableGrid, ituTableGridUpdate);
    ituWidgetSetDraw(tableGrid, ituTableGridDraw);
    ituWidgetSetOnAction(tableGrid, ituTableGridOnAction);
}
