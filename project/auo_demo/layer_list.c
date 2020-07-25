#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"

#define LIST_PAGE_COUNT 2

static ITUBackground* listBackground;
static ITURadioBox* listScrollDownRadioBox;
static ITURadioBox* listFadeInRadioBox;
static ITURadioBox* listScrollFadeRadioBox;
static ITURadioBox* listEnlargeFadeRadioBox;

bool ListTypeListBoxOnLoad(ITUWidget* widget, char* param)
{
    ITCTree* node;
    ITUListBox* listbox = (ITUListBox*)widget;
    ITUScrollListBox* slistbox = (ITUScrollListBox*)widget;
    int i, count, pageIndex;
    GuardSensor sensor;

    if (listbox->pageIndex == 0)
    {
        // initialize
        listbox->pageIndex = 1;
		listbox->focusIndex = -1;
    }

    listbox->pageCount = LIST_PAGE_COUNT;
    pageIndex = listbox->pageIndex;

    count = ituScrollListBoxGetItemCount(slistbox);
    node = ituScrollListBoxGetLastPageItem(slistbox);

    for (i = 0; i < count; i++)
    {
        ITUScrollText* scrolltext = (ITUScrollText*) node;

        sensor = i % GUARD_SENSOR_COUNT;
        ituTextSetString(scrolltext, StringGetGuardSensor(sensor));

        node = node->sibling;
    }

    node = ituScrollListBoxGetCurrPageItem(slistbox);
    for (i = 0; i < count; i++)
    {
        ITUScrollText* scrolltext = (ITUScrollText*) node;

        sensor = i % GUARD_SENSOR_COUNT;
        ituTextSetString(scrolltext, StringGetGuardSensor(sensor));

        node = node->sibling;
    }

    node = ituScrollListBoxGetNextPageItem(slistbox);
    for (i = 0; i < count; i++)
    {
        ITUScrollText* scrolltext = (ITUScrollText*) node;

        sensor = i % GUARD_SENSOR_COUNT;
        ituTextSetString(scrolltext, StringGetGuardSensor(sensor));

        node = node->sibling;
    }
    listbox->pageIndex = pageIndex;
    listbox->itemCount = count;

    return true;
}

bool ListDateListBoxOnLoad(ITUWidget* widget, char* param)
{
    ITCTree* node;
    ITUListBox* listbox = (ITUListBox*)widget;
    ITUScrollListBox* slistbox = (ITUScrollListBox*)widget;
    int i, count, pageIndex;

    if (listbox->pageIndex == 0)
    {
        // initialize
        listbox->pageIndex = 1;
		listbox->focusIndex = -1;
    }

    listbox->pageCount = LIST_PAGE_COUNT;
    pageIndex = listbox->pageIndex;

    count = ituScrollListBoxGetItemCount(slistbox);
    node = ituScrollListBoxGetLastPageItem(slistbox);
    for (i = 0; i < count; i++)
    {
        ITUScrollText* scrolltext = (ITUScrollText*) node;

        ituTextSetString(scrolltext, "2015/7/10");

        node = node->sibling;
    }

    node = ituScrollListBoxGetCurrPageItem(slistbox);
    for (i = 0; i < count; i++)
    {
        ITUScrollText* scrolltext = (ITUScrollText*) node;

        ituTextSetString(scrolltext, "2015/7/10");

        node = node->sibling;
    }

    node = ituScrollListBoxGetNextPageItem(slistbox);
    for (i = 0; i < count; i++)
    {
        ITUScrollText* scrolltext = (ITUScrollText*) node;

        ituTextSetString(scrolltext, "2015/7/10");

        node = node->sibling;
    }
    listbox->pageIndex = pageIndex;
    listbox->itemCount = count;

    return true;
}

bool ListTimeListBoxOnLoad(ITUWidget* widget, char* param)
{
    ITCTree* node;
    ITUListBox* listbox = (ITUListBox*)widget;
    ITUScrollListBox* slistbox = (ITUScrollListBox*)widget;
    int i, count, pageIndex;

    if (listbox->pageIndex == 0)
    {
        // initialize
        listbox->pageIndex = 1;
		listbox->focusIndex = -1;
    }

    listbox->pageCount = LIST_PAGE_COUNT;
    pageIndex = listbox->pageIndex;

    count = ituScrollListBoxGetItemCount(slistbox);
    node = ituScrollListBoxGetLastPageItem(slistbox);
    for (i = 0; i < count; i++)
    {
        ITUScrollText* scrolltext = (ITUScrollText*) node;

        ituTextSetString(scrolltext, "00:12:00");

        node = node->sibling;
    }

    node = ituScrollListBoxGetCurrPageItem(slistbox);
    for (i = 0; i < count; i++)
    {
        ITUScrollText* scrolltext = (ITUScrollText*) node;

        ituTextSetString(scrolltext, "00:12:00");

        node = node->sibling;
    }

    node = ituScrollListBoxGetNextPageItem(slistbox);
    for (i = 0; i < count; i++)
    {
        ITUScrollText* scrolltext = (ITUScrollText*) node;

        ituTextSetString(scrolltext, "00:12:00");

        node = node->sibling;
    }
    listbox->pageIndex = pageIndex;
    listbox->itemCount = count;

    return true;
}

bool ListOnEnter(ITUWidget* widget, char* param)
{
    if (!listBackground)
    {
        listBackground = ituSceneFindWidget(&theScene, "listBackground");
        assert(listBackground);

		listScrollDownRadioBox = ituSceneFindWidget(&theScene, "listScrollDownRadioBox");
		assert(listScrollDownRadioBox);

		listFadeInRadioBox = ituSceneFindWidget(&theScene, "listFadeInRadioBox");
		assert(listFadeInRadioBox);

		listScrollFadeRadioBox = ituSceneFindWidget(&theScene, "listScrollFadeRadioBox");
		assert(listScrollFadeRadioBox);

		listEnlargeFadeRadioBox = ituSceneFindWidget(&theScene, "listEnlargeFadeRadioBox");
		assert(listEnlargeFadeRadioBox);
    }

	ituRadioBoxSetChecked(listScrollDownRadioBox, false);
	ituRadioBoxSetChecked(listFadeInRadioBox, false);
	ituRadioBoxSetChecked(listScrollFadeRadioBox, false);
	ituRadioBoxSetChecked(listEnlargeFadeRadioBox, false);

    return true;
}

void ListReset(void)
{
    listBackground = NULL;
}
