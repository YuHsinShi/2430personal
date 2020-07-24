#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "SDL/SDL.h"
#include "ite/itu.h"

#define GESTURE_THRESHOLD 40
#define TOTAL_ITEMS 10

static int firstIndex;
static int selectIndex;
static int finish;
static int favorite[TOTAL_ITEMS] = { 0 };
static int order[TOTAL_ITEMS];

static ITUScene scene;
static ITUIcon *mainIcon[TOTAL_ITEMS] = { 0 };
static ITUIcon *mainFavoriteIcon[TOTAL_ITEMS] = { 0 };
static ITUIcon *imageSIcon[TOTAL_ITEMS] = { 0 };
static ITUIcon *imageBIcon[TOTAL_ITEMS] = { 0 };
static ITUIcon *focusAnimationItemIcon0[4] = { 0 };
static ITUIcon *focusAnimationItemIcon1[4] = { 0 };
static ITUIcon *focusAnimationBIcon0 = 0;
static ITUIcon *focusAnimationBIcon1 = 0;
static ITUIcon *focusAnimationSIcon0 = 0;
static ITUIcon *focusAnimationSIcon1 = 0;
static ITUIcon *focusLargeIcon = 0;
static ITUIcon *focusLargeFavoriteIcon = 0;
static ITUText *mainText[TOTAL_ITEMS] = { 0 };
static ITUText *imageText[TOTAL_ITEMS] = { 0 };
static ITUText *focusTitleText = 0;
static ITUText *focusTitleMinText = 0;
static ITUText *focusTitleHourText = 0;
static ITUIcon *focusItemIcon[5] = { 0 };
static ITUIcon *focusItemFavoriteIcon[5] = { 0 };
static ITUContainer *mainContainer[TOTAL_ITEMS] = { 0 };
static ITUContainer *focusItemContainer[5] = { 0 };
static ITUCoverFlow *mainCoverFlow = 0;
static ITUBackgroundButton *focusLargeBackgroundButton = 0;
static ITUAnimation *focusAnimation0 = 0;
static ITUAnimation *focusAnimation1 = 0;
static ITUAnimation *focusAnimation2 = 0;
static ITULayer *selectLayer = 0;
static ITUSprite *selectSprite = 0;


void FavoriteMoveToTail(int type)
{
	int i, j, k;
	for (i = 0; i<TOTAL_ITEMS; i++) {
		if (order[i] == type)
			break;
	}
	for (j = i + 1; j<TOTAL_ITEMS; j++) {
		if (favorite[order[j]] == 0)
			break;
	}
	for (k = i; k<j - 1; k++)
		order[k] = order[k + 1];
	order[j - 1] = type;
}

void FavoriteMoveToHead(int type)
{
	int i, j;
	if (type == order[0])  // already in order 0
		return;
	for (i = 0; i < TOTAL_ITEMS; i++) {
		if (order[i] == type)
			break;
	}
	for (j = i; j >= 0; j--)
		order[j] = order[j - 1];
	order[0] = type;
}

bool MainOnEnter(ITUWidget* widget, char* param)
{
	int i;
	if (!imageSIcon[0])
	{
		imageSIcon[0] = ituSceneFindWidget(&scene, "imageSIcon0");
		assert(imageSIcon[0]);

		imageSIcon[1] = ituSceneFindWidget(&scene, "imageSIcon1");
		assert(imageSIcon[1]);

		imageSIcon[2] = ituSceneFindWidget(&scene, "imageSIcon2");
		assert(imageSIcon[2]);

		imageSIcon[3] = ituSceneFindWidget(&scene, "imageSIcon3");
		assert(imageSIcon[3]);

		imageSIcon[4] = ituSceneFindWidget(&scene, "imageSIcon4");
		assert(imageSIcon[4]);

		imageSIcon[5] = ituSceneFindWidget(&scene, "imageSIcon5");
		assert(imageSIcon[5]);

		imageSIcon[6] = ituSceneFindWidget(&scene, "imageSIcon6");
		assert(imageSIcon[6]);

		imageSIcon[7] = ituSceneFindWidget(&scene, "imageSIcon7");
		assert(imageSIcon[7]);

		imageSIcon[8] = ituSceneFindWidget(&scene, "imageSIcon8");
		assert(imageSIcon[8]);

		imageSIcon[9] = ituSceneFindWidget(&scene, "imageSIcon9");
		assert(imageSIcon[9]);

		imageBIcon[0] = ituSceneFindWidget(&scene, "imageBIcon0");
		assert(imageBIcon[0]);

		imageBIcon[1] = ituSceneFindWidget(&scene, "imageBIcon1");
		assert(imageBIcon[1]);

		imageBIcon[2] = ituSceneFindWidget(&scene, "imageBIcon2");
		assert(imageBIcon[2]);

		imageBIcon[3] = ituSceneFindWidget(&scene, "imageBIcon3");
		assert(imageBIcon[3]);

		imageBIcon[4] = ituSceneFindWidget(&scene, "imageBIcon4");
		assert(imageBIcon[4]);

		imageBIcon[5] = ituSceneFindWidget(&scene, "imageBIcon5");
		assert(imageBIcon[5]);

		imageBIcon[6] = ituSceneFindWidget(&scene, "imageBIcon6");
		assert(imageBIcon[6]);

		imageBIcon[7] = ituSceneFindWidget(&scene, "imageBIcon7");
		assert(imageBIcon[7]);

		imageBIcon[8] = ituSceneFindWidget(&scene, "imageBIcon8");
		assert(imageBIcon[8]);

		imageBIcon[9] = ituSceneFindWidget(&scene, "imageBIcon9");
		assert(imageBIcon[9]);

		mainIcon[0] = ituSceneFindWidget(&scene, "mainIcon0");
		assert(mainIcon[0]);

		mainIcon[1] = ituSceneFindWidget(&scene, "mainIcon1");
		assert(mainIcon[1]);

		mainIcon[2] = ituSceneFindWidget(&scene, "mainIcon2");
		assert(mainIcon[2]);

		mainIcon[3] = ituSceneFindWidget(&scene, "mainIcon3");
		assert(mainIcon[3]);

		mainIcon[4] = ituSceneFindWidget(&scene, "mainIcon4");
		assert(mainIcon[4]);

		mainIcon[5] = ituSceneFindWidget(&scene, "mainIcon5");
		assert(mainIcon[5]);

		mainIcon[6] = ituSceneFindWidget(&scene, "mainIcon6");
		assert(mainIcon[6]);

		mainIcon[7] = ituSceneFindWidget(&scene, "mainIcon7");
		assert(mainIcon[7]);

		mainIcon[8] = ituSceneFindWidget(&scene, "mainIcon8");
		assert(mainIcon[8]);

		mainIcon[9] = ituSceneFindWidget(&scene, "mainIcon9");
		assert(mainIcon[9]);

		mainFavoriteIcon[0] = ituSceneFindWidget(&scene, "mainFavoriteIcon0");
		assert(mainFavoriteIcon[0]);

		mainFavoriteIcon[1] = ituSceneFindWidget(&scene, "mainFavoriteIcon1");
		assert(mainFavoriteIcon[1]);

		mainFavoriteIcon[2] = ituSceneFindWidget(&scene, "mainFavoriteIcon2");
		assert(mainFavoriteIcon[2]);

		mainFavoriteIcon[3] = ituSceneFindWidget(&scene, "mainFavoriteIcon3");
		assert(mainFavoriteIcon[3]);

		mainFavoriteIcon[4] = ituSceneFindWidget(&scene, "mainFavoriteIcon4");
		assert(mainFavoriteIcon[4]);

		mainFavoriteIcon[5] = ituSceneFindWidget(&scene, "mainFavoriteIcon5");
		assert(mainFavoriteIcon[5]);

		mainFavoriteIcon[6] = ituSceneFindWidget(&scene, "mainFavoriteIcon6");
		assert(mainFavoriteIcon[6]);

		mainFavoriteIcon[7] = ituSceneFindWidget(&scene, "mainFavoriteIcon7");
		assert(mainFavoriteIcon[7]);

		mainFavoriteIcon[8] = ituSceneFindWidget(&scene, "mainFavoriteIcon8");
		assert(mainFavoriteIcon[8]);

		mainFavoriteIcon[9] = ituSceneFindWidget(&scene, "mainFavoriteIcon9");
		assert(mainFavoriteIcon[9]);

		mainContainer[0] = ituSceneFindWidget(&scene, "mainContainer0");
		assert(mainContainer[0]);

		mainContainer[1] = ituSceneFindWidget(&scene, "mainContainer1");
		assert(mainContainer[1]);

		mainContainer[2] = ituSceneFindWidget(&scene, "mainContainer2");
		assert(mainContainer[2]);

		mainContainer[3] = ituSceneFindWidget(&scene, "mainContainer3");
		assert(mainContainer[3]);

		mainContainer[4] = ituSceneFindWidget(&scene, "mainContainer4");
		assert(mainContainer[4]);

		mainContainer[5] = ituSceneFindWidget(&scene, "mainContainer5");
		assert(mainContainer[5]);

		mainContainer[6] = ituSceneFindWidget(&scene, "mainContainer6");
		assert(mainContainer[6]);

		mainContainer[7] = ituSceneFindWidget(&scene, "mainContainer7");
		assert(mainContainer[7]);

		mainContainer[8] = ituSceneFindWidget(&scene, "mainContainer8");
		assert(mainContainer[8]);

		mainContainer[9] = ituSceneFindWidget(&scene, "mainContainer9");
		assert(mainContainer[9]);

		mainText[0] = ituSceneFindWidget(&scene, "mainText0");
		assert(mainText[0]);

		mainText[1] = ituSceneFindWidget(&scene, "mainText1");
		assert(mainText[1]);

		mainText[2] = ituSceneFindWidget(&scene, "mainText2");
		assert(mainText[2]);

		mainText[3] = ituSceneFindWidget(&scene, "mainText3");
		assert(mainText[3]);

		mainText[4] = ituSceneFindWidget(&scene, "mainText4");
		assert(mainText[4]);

		mainText[5] = ituSceneFindWidget(&scene, "mainText5");
		assert(mainText[5]);

		mainText[6] = ituSceneFindWidget(&scene, "mainText6");
		assert(mainText[6]);

		mainText[7] = ituSceneFindWidget(&scene, "mainText7");
		assert(mainText[7]);

		mainText[8] = ituSceneFindWidget(&scene, "mainText8");
		assert(mainText[8]);

		mainText[9] = ituSceneFindWidget(&scene, "mainText9");
		assert(mainText[9]);

		imageText[0] = ituSceneFindWidget(&scene, "imageText0");
		assert(imageText[0]);

		imageText[1] = ituSceneFindWidget(&scene, "imageText1");
		assert(imageText[1]);

		imageText[2] = ituSceneFindWidget(&scene, "imageText2");
		assert(imageText[2]);

		imageText[3] = ituSceneFindWidget(&scene, "imageText3");
		assert(imageText[3]);

		imageText[4] = ituSceneFindWidget(&scene, "imageText4");
		assert(imageText[4]);

		imageText[5] = ituSceneFindWidget(&scene, "imageText5");
		assert(imageText[5]);

		imageText[6] = ituSceneFindWidget(&scene, "imageText6");
		assert(imageText[6]);

		imageText[7] = ituSceneFindWidget(&scene, "imageText7");
		assert(imageText[7]);

		imageText[8] = ituSceneFindWidget(&scene, "imageText8");
		assert(imageText[8]);

		imageText[9] = ituSceneFindWidget(&scene, "imageText9");
		assert(imageText[9]);

		mainCoverFlow = ituSceneFindWidget(&scene, "mainCoverFlow");
		assert(mainCoverFlow);

		for (i = 0; i < TOTAL_ITEMS; i++)
			order[i] = i;

	}

	for (i = 0; i<TOTAL_ITEMS; i++) {
		ituIconLinkSurface(mainIcon[i], imageSIcon[order[i]]);
		ituTextSetString(mainText[i], ituTextGetString(imageText[order[i]]));
		if (favorite[order[i]])
			ituWidgetSetVisible(mainFavoriteIcon[i], true);
		else
			ituWidgetSetVisible(mainFavoriteIcon[i], false);
	}
	ituCoverFlowGoto(mainCoverFlow, 0);
	return true;
}

bool FocusOnEnter(ITUWidget* widget, char* param)
{
	int i;

	if (!focusTitleText)
	{
		focusTitleText = ituSceneFindWidget(&scene, "focusTitleText");
		assert(focusTitleText);

		focusTitleMinText = ituSceneFindWidget(&scene, "focusTitleMinText");
		assert(focusTitleMinText);

		focusTitleHourText = ituSceneFindWidget(&scene, "focusTitleHourText");
		assert(focusTitleHourText);

		focusLargeIcon = ituSceneFindWidget(&scene, "focusLargeIcon");
		assert(focusLargeIcon);

		focusLargeFavoriteIcon = ituSceneFindWidget(&scene, "focusLargeFavoriteIcon");
		assert(focusLargeFavoriteIcon);

		focusLargeBackgroundButton = ituSceneFindWidget(&scene, "focusLargeBackgroundButton");
		assert(focusLargeBackgroundButton);

		focusItemIcon[0] = ituSceneFindWidget(&scene, "focusItem0Icon");
		assert(focusItemIcon[0]);

		focusItemIcon[1] = ituSceneFindWidget(&scene, "focusItem1Icon");
		assert(focusItemIcon[1]);

		focusItemIcon[2] = ituSceneFindWidget(&scene, "focusItem2Icon");
		assert(focusItemIcon[2]);

		focusItemIcon[3] = ituSceneFindWidget(&scene, "focusItem3Icon");
		assert(focusItemIcon[3]);

		focusItemIcon[4] = ituSceneFindWidget(&scene, "focusItem4Icon");
		assert(focusItemIcon[4]);

		focusItemFavoriteIcon[0] = ituSceneFindWidget(&scene, "focusItem0FavoriteIcon");
		assert(focusItemFavoriteIcon[0]);

		focusItemFavoriteIcon[1] = ituSceneFindWidget(&scene, "focusItem1FavoriteIcon");
		assert(focusItemFavoriteIcon[1]);

		focusItemFavoriteIcon[2] = ituSceneFindWidget(&scene, "focusItem2FavoriteIcon");
		assert(focusItemFavoriteIcon[2]);

		focusItemFavoriteIcon[3] = ituSceneFindWidget(&scene, "focusItem3FavoriteIcon");
		assert(focusItemFavoriteIcon[3]);

		focusItemFavoriteIcon[4] = ituSceneFindWidget(&scene, "focusItem4FavoriteIcon");
		assert(focusItemFavoriteIcon[4]);

		focusItemContainer[0] = ituSceneFindWidget(&scene, "focusItem0Container");
		assert(focusItemContainer[0]);

		focusItemContainer[1] = ituSceneFindWidget(&scene, "focusItem1Container");
		assert(focusItemContainer[1]);

		focusItemContainer[2] = ituSceneFindWidget(&scene, "focusItem2Container");
		assert(focusItemContainer[2]);

		focusItemContainer[3] = ituSceneFindWidget(&scene, "focusItem3Container");
		assert(focusItemContainer[3]);

		focusItemContainer[4] = ituSceneFindWidget(&scene, "focusItem4Container");
		assert(focusItemContainer[4]);

		focusAnimation0 = ituSceneFindWidget(&scene, "focusAnimation0");
		assert(focusAnimation0);

		focusAnimation1 = ituSceneFindWidget(&scene, "focusAnimation1");
		assert(focusAnimation1);

		focusAnimation2 = ituSceneFindWidget(&scene, "focusAnimation2");
		assert(focusAnimation2);

		focusAnimationItemIcon0[0] = ituSceneFindWidget(&scene, "focusAnimationItem0Icon0");
		assert(focusAnimationItemIcon0[0]);

		focusAnimationItemIcon0[1] = ituSceneFindWidget(&scene, "focusAnimationItem1Icon0");
		assert(focusAnimationItemIcon0[1]);

		focusAnimationItemIcon0[2] = ituSceneFindWidget(&scene, "focusAnimationItem2Icon0");
		assert(focusAnimationItemIcon0[2]);

		focusAnimationItemIcon0[3] = ituSceneFindWidget(&scene, "focusAnimationItem3Icon0");
		assert(focusAnimationItemIcon0[3]);

		focusAnimationItemIcon1[0] = ituSceneFindWidget(&scene, "focusAnimationItem0Icon1");
		assert(focusAnimationItemIcon1[0]);

		focusAnimationItemIcon1[1] = ituSceneFindWidget(&scene, "focusAnimationItem1Icon1");
		assert(focusAnimationItemIcon1[1]);

		focusAnimationItemIcon1[2] = ituSceneFindWidget(&scene, "focusAnimationItem2Icon1");
		assert(focusAnimationItemIcon1[2]);

		focusAnimationItemIcon1[3] = ituSceneFindWidget(&scene, "focusAnimationItem3Icon1");
		assert(focusAnimationItemIcon1[3]);

		focusAnimationBIcon0 = ituSceneFindWidget(&scene, "focusAnimationBIcon0");
		assert(focusAnimationBIcon0);

		focusAnimationBIcon1 = ituSceneFindWidget(&scene, "focusAnimationBIcon1");
		assert(focusAnimationBIcon1);

		focusAnimationSIcon0 = ituSceneFindWidget(&scene, "focusAnimationSIcon0");
		assert(focusAnimationSIcon0);

		focusAnimationSIcon1 = ituSceneFindWidget(&scene, "focusAnimationSIcon1");
		assert(focusAnimationSIcon1);

		selectLayer = ituSceneFindWidget(&scene, "selectLayer");
		assert(selectLayer);

		selectSprite = ituSceneFindWidget(&scene, "selectSprite");
		assert(selectSprite);
	}

	ituWidgetSetVisible(focusItemContainer[0], false);
	ituWidgetSetVisible(focusItemContainer[1], false);
	ituWidgetSetVisible(focusItemContainer[2], false);
	ituWidgetSetVisible(focusItemContainer[3], false);
	ituWidgetSetVisible(focusItemContainer[4], false);
	ituWidgetSetVisible(focusAnimation0, true);
	ituWidgetSetVisible(focusAnimation1, false);
	ituWidgetSetVisible(focusAnimation2, false);
	ituWidgetShow(focusLargeBackgroundButton, ITU_EFFECT_FADE, 20);
	for (i = 0; i<5; i++)
		ituWidgetDisable(focusItemContainer[i]);

	firstIndex = mainCoverFlow->focusIndex;
	if (firstIndex>TOTAL_ITEMS - 5) firstIndex = TOTAL_ITEMS - 5;
	selectIndex = atoi(scene.variables[0].param) - firstIndex; // set0 - firstIndex
	ituTextSetString(focusTitleText, ituTextGetString(imageText[order[firstIndex + selectIndex]]));//ituTextGetString(imageText[order[firstIndex + selectIndex]]));
	ituWidgetSetVisible(imageBIcon[order[firstIndex + selectIndex]], true);
	ituIconLinkSurface(focusLargeIcon, imageBIcon[order[firstIndex + selectIndex]]);
	if (favorite[order[firstIndex + selectIndex]])
		ituWidgetSetVisible(focusLargeFavoriteIcon, true);
	else
		ituWidgetSetVisible(focusLargeFavoriteIcon, false);

	switch (selectIndex) {
	case 0:
		ituWidgetSetPosition(focusAnimation0, -34 + 171, 0);
		ituWidgetSetPosition(focusLargeBackgroundButton, -34, 0);
		for (i = 0; i<4; i++) {
			ituIconLinkSurface(focusAnimationItemIcon0[i], imageSIcon[order[firstIndex + i + 1]]);
			ituIconLinkSurface(focusAnimationItemIcon1[i], imageSIcon[order[firstIndex + i + 1]]);
		}
		ituAnimationPlay(focusAnimation0, 0);
		break;
	case 1:
		ituIconLinkSurface(focusItemIcon[0], imageSIcon[order[firstIndex]]);
		if (favorite[order[firstIndex]])
			ituWidgetSetVisible(focusItemFavoriteIcon[0], true);
		else
			ituWidgetSetVisible(focusItemFavoriteIcon[0], false);
		ituWidgetSetVisible(focusItemContainer[0], true);
		ituWidgetSetPosition(focusAnimation0, -34 + 171 * 2, 0);
		ituWidgetSetPosition(focusLargeBackgroundButton, -34 + 171, 0);

		for (i = 0; i<4; i++) {
			ituIconLinkSurface(focusAnimationItemIcon0[i], imageSIcon[order[firstIndex + i + 2]]);
			ituIconLinkSurface(focusAnimationItemIcon1[i], imageSIcon[order[firstIndex + i + 2]]);
		}
		ituAnimationPlay(focusAnimation0, 0);
		break;
	case 2:
		ituIconLinkSurface(focusItemIcon[3], imageSIcon[order[firstIndex + 3]]);
		if (favorite[order[firstIndex + 3]])
			ituWidgetSetVisible(focusItemFavoriteIcon[3], true);
		else
			ituWidgetSetVisible(focusItemFavoriteIcon[3], false);
		ituIconLinkSurface(focusItemIcon[4], imageSIcon[order[firstIndex + 4]]);
		if (favorite[order[firstIndex + 4]])
			ituWidgetSetVisible(focusItemFavoriteIcon[4], true);
		else
			ituWidgetSetVisible(focusItemFavoriteIcon[4], false);
		ituWidgetSetVisible(focusItemContainer[3], true);
		ituWidgetSetVisible(focusItemContainer[4], true);
		ituWidgetSetPosition(focusAnimation0, -34 - 171 * 4, 0);
		ituWidgetSetPosition(focusLargeBackgroundButton, -34, 0);
		for (i = 0; i<4; i++) {
			int index = firstIndex - i + 1;
			if (index <0) index = 0;
			ituIconLinkSurface(focusAnimationItemIcon0[3 - i], imageSIcon[order[index]]);
			ituIconLinkSurface(focusAnimationItemIcon1[3 - i], imageSIcon[order[index]]);
		}
		firstIndex += 2;
		ituCoverFlowGoto(mainCoverFlow, firstIndex);
		ituAnimationReversePlay(focusAnimation0, -1);
		break;
	case 3:
		ituIconLinkSurface(focusItemIcon[4], imageSIcon[order[firstIndex + 4]]);
		if (favorite[order[firstIndex + 4]])
			ituWidgetSetVisible(focusItemFavoriteIcon[4], true);
		else
			ituWidgetSetVisible(focusItemFavoriteIcon[4], false);
		ituWidgetSetVisible(focusItemContainer[4], true);
		ituWidgetSetPosition(focusAnimation0, -34 - 171 * 3, 0);
		ituWidgetSetPosition(focusLargeBackgroundButton, -34 + 171, 0);

		for (i = 0; i<4; i++) {
			int index = firstIndex - i + 2;
			if (index <0) index = 0;
			ituIconLinkSurface(focusAnimationItemIcon0[3 - i], imageSIcon[order[index]]);
			ituIconLinkSurface(focusAnimationItemIcon1[3 - i], imageSIcon[order[index]]);
		}
		firstIndex += 2;
		ituCoverFlowGoto(mainCoverFlow, firstIndex);
		ituAnimationReversePlay(focusAnimation0, -1);
		break;
	case 4:
		ituWidgetSetPosition(focusAnimation0, -34 - 171 * 2, 0);
		ituWidgetSetPosition(focusLargeBackgroundButton, -34 + 171 * 2, 0);
		for (i = 0; i<4; i++) {
			int index = firstIndex - i + 3;
			if (index <0) index = 0;
			ituIconLinkSurface(focusAnimationItemIcon0[3 - i], imageSIcon[order[index]]);
			ituIconLinkSurface(focusAnimationItemIcon1[3 - i], imageSIcon[order[index]]);
		}
		firstIndex += 2;
		ituCoverFlowGoto(mainCoverFlow, firstIndex);
		ituAnimationReversePlay(focusAnimation0, -1);
		break;
	}

	finish = 0;

	return true;
}

bool FocusAnimation0OnStopped(ITUWidget* widget, char* param)
{
	int i;
	ituAnimationReset(focusAnimation0);
	ituWidgetSetVisible(focusAnimation0, false);

	for (i = 0; i<5; i++)
		ituWidgetEnable(focusItemContainer[i]);

	/*if (finish == 1) {
		ituLayerGoto(mainLayer);
		return true;
	}*/

	switch (selectIndex) {
	case 0:
		ituIconLinkSurface(focusItemIcon[3], imageSIcon[order[firstIndex + 1]]);
		if (favorite[order[firstIndex + 1]])
			ituWidgetSetVisible(focusItemFavoriteIcon[3], true);
		else
			ituWidgetSetVisible(focusItemFavoriteIcon[3], false);
		ituIconLinkSurface(focusItemIcon[4], imageSIcon[order[firstIndex + 2]]);
		if (favorite[order[firstIndex + 2]])
			ituWidgetSetVisible(focusItemFavoriteIcon[4], true);
		else
			ituWidgetSetVisible(focusItemFavoriteIcon[4], false);
		ituWidgetSetVisible(focusItemContainer[3], true);
		ituWidgetSetVisible(focusItemContainer[4], true);
		break;
	case 1:
		ituIconLinkSurface(focusItemIcon[4], imageSIcon[order[firstIndex + 2]]);
		if (favorite[order[firstIndex + 2]])
			ituWidgetSetVisible(focusItemFavoriteIcon[4], true);
		else
			ituWidgetSetVisible(focusItemFavoriteIcon[4], false);
		ituWidgetSetVisible(focusItemContainer[4], true);
		break;
	case 2:
		break;
	case 3:
		ituIconLinkSurface(focusItemIcon[0], imageSIcon[order[firstIndex]]);
		if (favorite[order[firstIndex]])
			ituWidgetSetVisible(focusItemFavoriteIcon[0], true);
		else
			ituWidgetSetVisible(focusItemFavoriteIcon[0], false);
		ituWidgetSetVisible(focusItemContainer[0], true);
		break;
	case 4:
		ituIconLinkSurface(focusItemIcon[0], imageSIcon[order[firstIndex]]);
		if (favorite[order[firstIndex]])
			ituWidgetSetVisible(focusItemFavoriteIcon[0], true);
		else
			ituWidgetSetVisible(focusItemFavoriteIcon[0], false);
		ituIconLinkSurface(focusItemIcon[1], imageSIcon[order[firstIndex + 1]]);
		if (favorite[order[firstIndex + 1]])
			ituWidgetSetVisible(focusItemFavoriteIcon[1], true);
		else
			ituWidgetSetVisible(focusItemFavoriteIcon[1], false);
		ituWidgetSetVisible(focusItemContainer[0], true);
		ituWidgetSetVisible(focusItemContainer[1], true);
		break;
	}

	return true;
}

bool FocusAnimation1OnStopped(ITUWidget* widget, char* param)
{
	int i;
	ituAnimationReset(focusAnimation1);
	ituWidgetSetVisible(focusAnimation1, false);
	for (i = 0; i<5; i++)
		ituWidgetEnable(focusItemContainer[i]);

	return true;
}

bool FocusAnimation2OnStopped(ITUWidget* widget, char* param)
{
	int i;
	ituAnimationReset(focusAnimation2);
	ituWidgetSetVisible(focusAnimation2, false);
	for (i = 0; i<5; i++)
		ituWidgetEnable(focusItemContainer[i]);
		
	switch (selectIndex) {
	case 0:
		ituIconLinkSurface(focusItemIcon[3], imageSIcon[order[firstIndex + 1]]);
		if (favorite[order[firstIndex + 1]])
			ituWidgetSetVisible(focusItemFavoriteIcon[3], true);
		else
			ituWidgetSetVisible(focusItemFavoriteIcon[3], false);
		ituWidgetSetVisible(focusItemContainer[3], true);
		break;
	case 4:
		ituIconLinkSurface(focusItemIcon[1], imageSIcon[order[firstIndex + 1]]);
		if (favorite[order[firstIndex + 1]])
			ituWidgetSetVisible(focusItemFavoriteIcon[1], true);
		else
			ituWidgetSetVisible(focusItemFavoriteIcon[1], false);
		ituWidgetSetVisible(focusItemContainer[1], true);
		break;
	}

	return true;
}

bool FocusItemButtonOnPress(ITUWidget* widget, char* param)
{
	int i, item;
	selectIndex = atoi(param);
	for (i = 0; i<5; i++)
		ituWidgetDisable(focusItemContainer[i]);

	if (selectIndex<2) item = firstIndex + selectIndex;
	else item = firstIndex + selectIndex - 2;
	ituTextSetString(focusTitleText, ituTextGetString(imageText[order[item]]));

	ituWidgetSetVisible(focusLargeBackgroundButton, false);
	ituWidgetShow(focusLargeBackgroundButton, ITU_EFFECT_FADE, 20);

	switch (selectIndex) {
	case 0:
		ituWidgetSetVisible(imageBIcon[order[firstIndex]], true);
		ituIconLinkSurface(focusLargeIcon, imageBIcon[order[firstIndex]]);
		if (favorite[order[firstIndex]])
			ituWidgetSetVisible(focusLargeFavoriteIcon, true);
		else
			ituWidgetSetVisible(focusLargeFavoriteIcon, false);
		ituWidgetSetPosition(focusLargeBackgroundButton, -34, 0);
		ituWidgetHide(focusItemContainer[0], ITU_EFFECT_FADE, 20);
		if (!ituWidgetIsVisible(focusItemContainer[1])) {
			// case 0:
			ituIconLinkSurface(focusItemIcon[3], imageSIcon[order[firstIndex + 1]]);
			if (favorite[order[firstIndex + 1]])
				ituWidgetSetVisible(focusItemFavoriteIcon[3], true);
			else
				ituWidgetSetVisible(focusItemFavoriteIcon[3], false);
			ituWidgetShow(focusItemContainer[3], ITU_EFFECT_FADE, 20);
			ituWidgetSetVisible(focusAnimation1, true);
			ituWidgetSetPosition(focusAnimation1, -34 + 171, 0);
			ituWidgetSetVisible(imageBIcon[order[firstIndex + 1]], true);
			ituIconLinkSurface(focusAnimationBIcon0, imageBIcon[order[firstIndex + 1]]);
			ituIconLinkSurface(focusAnimationBIcon1, imageBIcon[order[firstIndex + 1]]);
			ituAnimationPlay(focusAnimation1, 0);
		}
		else {
			// case 1:
			ituWidgetSetVisible(focusItemContainer[1], false);
			ituIconLinkSurface(focusItemIcon[4], imageSIcon[order[firstIndex + 2]]);
			if (favorite[order[firstIndex + 2]])
				ituWidgetSetVisible(focusItemFavoriteIcon[4], true);
			else
				ituWidgetSetVisible(focusItemFavoriteIcon[4], false);
			ituWidgetShow(focusItemContainer[4], ITU_EFFECT_FADE, 20);
			ituWidgetSetVisible(focusAnimation1, true);
			ituWidgetSetVisible(focusAnimation2, true);
			ituWidgetSetPosition(focusAnimation1, -34 + 171 * 2, 0);
			ituWidgetSetPosition(focusAnimation2, -34 + 171, 0);
			ituWidgetSetVisible(imageBIcon[order[firstIndex + 2]], true);
			ituIconLinkSurface(focusAnimationBIcon0, imageBIcon[order[firstIndex + 2]]);
			ituIconLinkSurface(focusAnimationBIcon1, imageBIcon[order[firstIndex + 2]]);
			ituIconLinkSurface(focusAnimationSIcon0, imageSIcon[order[firstIndex + 1]]);
			ituIconLinkSurface(focusAnimationSIcon1, imageSIcon[order[firstIndex + 1]]);
			ituAnimationPlay(focusAnimation1, 0);
			ituAnimationPlay(focusAnimation2, 0);
		}
		break;
	case 1:
		ituWidgetSetVisible(imageBIcon[order[firstIndex + 1]], true);
		ituIconLinkSurface(focusLargeIcon, imageBIcon[order[firstIndex + 1]]);
		if (favorite[order[firstIndex + 1]])
			ituWidgetSetVisible(focusLargeFavoriteIcon, true);
		else
			ituWidgetSetVisible(focusLargeFavoriteIcon, false);
		ituWidgetSetPosition(focusLargeBackgroundButton, -34 + 171, 0);
		ituWidgetHide(focusItemContainer[1], ITU_EFFECT_FADE, 20);
		ituIconLinkSurface(focusItemIcon[4], imageSIcon[order[firstIndex + 2]]);
		if (favorite[order[firstIndex + 2]])
			ituWidgetSetVisible(focusItemFavoriteIcon[4], true);
		else
			ituWidgetSetVisible(focusItemFavoriteIcon[4], false);
		ituWidgetShow(focusItemContainer[4], ITU_EFFECT_FADE, 20);
		ituWidgetSetVisible(focusAnimation1, true);
		ituWidgetSetPosition(focusAnimation1, -34 + 171 * 2, 0);
		ituWidgetSetVisible(imageBIcon[order[firstIndex + 2]], true);
		ituIconLinkSurface(focusAnimationBIcon0, imageBIcon[order[firstIndex + 2]]);
		ituIconLinkSurface(focusAnimationBIcon1, imageBIcon[order[firstIndex + 2]]);
		ituAnimationPlay(focusAnimation1, 0);
		break;
	case 3:
		ituWidgetSetVisible(imageBIcon[order[firstIndex + 1]], true);
		ituIconLinkSurface(focusLargeIcon, imageBIcon[order[firstIndex + 1]]);
		if (favorite[order[firstIndex + 1]])
			ituWidgetSetVisible(focusLargeFavoriteIcon, true);
		else
			ituWidgetSetVisible(focusLargeFavoriteIcon, false);
		ituWidgetSetPosition(focusLargeBackgroundButton, -34 + 171, 0);
		ituWidgetHide(focusItemContainer[3], ITU_EFFECT_FADE, 20);
		ituIconLinkSurface(focusItemIcon[0], imageSIcon[order[firstIndex]]);
		if (favorite[order[firstIndex]])
			ituWidgetSetVisible(focusItemFavoriteIcon[0], true);
		else
			ituWidgetSetVisible(focusItemFavoriteIcon[0], false);
		ituWidgetShow(focusItemContainer[0], ITU_EFFECT_FADE, 20);
		ituWidgetSetVisible(focusAnimation1, true);
		ituWidgetSetPosition(focusAnimation1, -34 - 171 * 2, 0);
		ituWidgetSetVisible(imageBIcon[order[firstIndex]], true);
		ituIconLinkSurface(focusAnimationBIcon0, imageBIcon[order[firstIndex]]);
		ituIconLinkSurface(focusAnimationBIcon1, imageBIcon[order[firstIndex]]);
		ituAnimationReversePlay(focusAnimation1, -1);
		break;
	case 4:
		ituWidgetSetVisible(imageBIcon[order[firstIndex + 2]], true);
		ituIconLinkSurface(focusLargeIcon, imageBIcon[order[firstIndex + 2]]);
		if (favorite[order[firstIndex + 2]])
			ituWidgetSetVisible(focusLargeFavoriteIcon, true);
		else
			ituWidgetSetVisible(focusLargeFavoriteIcon, false);
		ituWidgetSetPosition(focusLargeBackgroundButton, -34 + 171 * 2, 0);
		ituWidgetHide(focusItemContainer[4], ITU_EFFECT_FADE, 20);
		if (!ituWidgetIsVisible(focusItemContainer[3])) {
			// case 0:
			ituIconLinkSurface(focusItemIcon[1], imageSIcon[order[firstIndex + 1]]);
			ituWidgetShow(focusItemContainer[1], ITU_EFFECT_FADE, 20);
			if (favorite[order[firstIndex + 1]])
				ituWidgetSetVisible(focusItemFavoriteIcon[1], true);
			else
				ituWidgetSetVisible(focusItemFavoriteIcon[1], false);
			ituWidgetSetVisible(focusAnimation1, true);
			ituWidgetSetPosition(focusAnimation1, -34 - 171, 0);
			ituWidgetSetVisible(imageBIcon[order[firstIndex + 1]], true);
			ituIconLinkSurface(focusAnimationBIcon0, imageBIcon[order[firstIndex + 1]]);
			ituIconLinkSurface(focusAnimationBIcon1, imageBIcon[order[firstIndex + 1]]);
			ituAnimationReversePlay(focusAnimation1, -1);
		}
		else {
			// case 1:
			ituWidgetSetVisible(focusItemContainer[3], false);
			ituIconLinkSurface(focusItemIcon[0], imageSIcon[order[firstIndex]]);
			ituWidgetShow(focusItemContainer[0], ITU_EFFECT_FADE, 20);
			if (favorite[order[firstIndex]])
				ituWidgetSetVisible(focusItemFavoriteIcon[0], true);
			else
				ituWidgetSetVisible(focusItemFavoriteIcon[0], false);
			ituWidgetSetVisible(focusAnimation1, true);
			ituWidgetSetVisible(focusAnimation2, true);
			ituWidgetSetPosition(focusAnimation1, -34 - 171 * 2, 0);
			ituWidgetSetPosition(focusAnimation2, -34 + 171, 0);
			ituWidgetSetVisible(imageBIcon[order[firstIndex]], true);
			ituIconLinkSurface(focusAnimationBIcon0, imageBIcon[order[firstIndex]]);
			ituIconLinkSurface(focusAnimationBIcon1, imageBIcon[order[firstIndex]]);
			ituIconLinkSurface(focusAnimationSIcon0, imageSIcon[order[firstIndex + 1]]);
			ituIconLinkSurface(focusAnimationSIcon1, imageSIcon[order[firstIndex + 1]]);
			ituAnimationReversePlay(focusAnimation1, -1);
			ituAnimationReversePlay(focusAnimation2, -1);
		}
		break;
	}
	
	return true;
}

bool FocusLargeBackgroundButtonOnPress(ITUWidget* widget, char* param)
{
	int item;
	if (selectIndex<2) item = firstIndex + selectIndex;
	else item = firstIndex + selectIndex - 2;
	switch (order[item]) {
	case 0: ituSpriteGoto(selectSprite, 0); break;
	case 1: ituSpriteGoto(selectSprite, 1); break;
	case 2: ituSpriteGoto(selectSprite, 2); break;
	case 3: ituSpriteGoto(selectSprite, 3); break;
	case 4: ituSpriteGoto(selectSprite, 4); break;
	case 5: ituSpriteGoto(selectSprite, 5); break;
	case 6: ituSpriteGoto(selectSprite, 6); break;
	case 7: ituSpriteGoto(selectSprite, 7); break;
	case 8: ituSpriteGoto(selectSprite, 8); break;
	case 9: ituSpriteGoto(selectSprite, 9); break;
	}
	ituLayerGoto(selectLayer);
	return true;
}

bool SelectFavoriteCheckBoxOnPress(ITUWidget* widget, char* param)
{
	int i, paramInt = atoi(param);

	if (ituCheckBoxIsChecked(widget)) {
		favorite[paramInt] = 1;
		FavoriteMoveToHead(paramInt);
	}
	else {
		FavoriteMoveToTail(paramInt);
		favorite[paramInt] = 0;
	}
	return true;
}

ITUActionFunction actionFunctions[] =
{
	"MainOnEnter", MainOnEnter,
	"FocusOnEnter", FocusOnEnter,
	"FocusAnimation0OnStopped", FocusAnimation0OnStopped,
	"FocusAnimation1OnStopped", FocusAnimation1OnStopped,
	"FocusAnimation2OnStopped", FocusAnimation2OnStopped,
	"FocusItemButtonOnPress", FocusItemButtonOnPress,
	"FocusLargeBackgroundButtonOnPress", FocusLargeBackgroundButtonOnPress,
	"SelectFavoriteCheckBoxOnPress", SelectFavoriteCheckBoxOnPress,
	NULL, NULL,
};

int SDL_main(int argc, char *argv[])
{
	SDL_Window *window;
	SDL_Event ev;
	int done, delay, lastx, lasty;
	uint32_t tick, dblclk, lasttick, mouseDownTick;

	// wait mouting USB storage
#ifndef _WIN32
	sleep(5);
#endif

	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		return (1);
	}
	window = SDL_CreateWindow("UI CoverFlow overlap MyFavorite",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		1024, 600, 0);
	if (!window)
	{
		printf("Couldn't create 1024x600 window: %s\n",
			SDL_GetError());
		SDL_Quit();
		exit(2);
	}

	// init itu
	ituLcdInit();

#ifdef CFG_M2D_ENABLE
	ituM2dInit();
#else
	ituSWInit();
#endif

	ituFtInit();
	ituFtLoadFont(0, CFG_PRIVATE_DRIVE ":/font/wqyMicroHei.ttf", ITU_GLYPH_8BPP);

	// load itu file
	tick = SDL_GetTicks();

	ituSceneInit(&scene, NULL);
	ituSceneSetFunctionTable(&scene, actionFunctions);
	ituSceneLoadFile(&scene, CFG_PRIVATE_DRIVE ":/UI_CoverFlow_overlap_MyFavoriteBread.itu");

	printf("loading time: %dms\n", SDL_GetTicks() - tick);

	scene.leftKey = SDLK_LEFT;
	scene.upKey = SDLK_UP;
	scene.rightKey = SDLK_RIGHT;
	scene.downKey = SDLK_DOWN;

	/* Watch keystrokes */
	done = dblclk = lasttick = lastx = lasty = mouseDownTick = 0;
	while (!done)
	{
		bool result = false;

		tick = SDL_GetTicks();

		while (SDL_PollEvent(&ev))
		{
			switch (ev.type)
			{
			case SDL_MOUSEMOTION:
				result = ituSceneUpdate(&scene, ITU_EVENT_MOUSEMOVE, ev.button.button, ev.button.x, ev.button.y);
				break;

			case SDL_MOUSEBUTTONDOWN:
				mouseDownTick = SDL_GetTicks();
				if (mouseDownTick - dblclk <= 300)
				{
					result = ituSceneUpdate(&scene, ITU_EVENT_MOUSEDOUBLECLICK, ev.button.button, ev.button.x, ev.button.y);
					dblclk = mouseDownTick = 0;
				}
				else
				{
					result = ituSceneUpdate(&scene, ITU_EVENT_MOUSEDOWN, ev.button.button, ev.button.x, ev.button.y);
					dblclk = mouseDownTick;
					lastx = ev.button.x;
					lasty = ev.button.y;
				}
				break;

			case SDL_MOUSEBUTTONUP:
				if (SDL_GetTicks() - dblclk <= 300)
				{
					int xdiff = abs(ev.button.x - lastx);
					int ydiff = abs(ev.button.y - lasty);

					if (xdiff >= GESTURE_THRESHOLD)
					{
						if (ev.button.x > lastx)
						{
							printf("mouse: slide to right\n");
							result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDERIGHT, 1, ev.button.x, ev.button.y);
						}
						else
						{
							printf("mouse: slide to left\n");
							result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDELEFT, 1, ev.button.x, ev.button.y);
						}
					}
					else if (ydiff >= GESTURE_THRESHOLD)
					{
						if (ev.button.y > lasty)
						{
							printf("mouse: slide to down\n");
							result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDEDOWN, 1, ev.button.x, ev.button.y);
						}
						else
						{
							printf("mouse: slide to up\n");
							result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDEUP, 1, ev.button.x, ev.button.y);
						}
					}
				}
				result |= ituSceneUpdate(&scene, ITU_EVENT_MOUSEUP, ev.button.button, ev.button.x, ev.button.y);
				mouseDownTick = 0;
				break;

			case SDL_FINGERMOTION:
				printf("touch: move %d, %d\n", ev.tfinger.x, ev.tfinger.y);
				result = ituSceneUpdate(&scene, ITU_EVENT_MOUSEMOVE, 1, ev.tfinger.x, ev.tfinger.y);
				break;

			case SDL_FINGERDOWN:
				printf("touch: down %d, %d\n", ev.tfinger.x, ev.tfinger.y);
				{
					mouseDownTick = SDL_GetTicks();
					if (mouseDownTick - dblclk <= 300)
					{
						result = ituSceneUpdate(&scene, ITU_EVENT_MOUSEDOUBLECLICK, 1, ev.tfinger.x, ev.tfinger.y);
						dblclk = mouseDownTick = 0;
					}
					else
					{
						result = ituSceneUpdate(&scene, ITU_EVENT_MOUSEDOWN, 1, ev.tfinger.x, ev.tfinger.y);
						dblclk = mouseDownTick;
						lastx = ev.tfinger.x;
						lasty = ev.tfinger.y;
					}
				}
				break;

			case SDL_FINGERUP:
				printf("touch: up %d, %d\n", ev.tfinger.x, ev.tfinger.y);
				if (SDL_GetTicks() - dblclk <= 300)
				{
					int xdiff = abs(ev.tfinger.x - lastx);
					int ydiff = abs(ev.tfinger.y - lasty);

					if (xdiff >= GESTURE_THRESHOLD)
					{
						if (ev.tfinger.x > lastx)
						{
							printf("touch: slide to right %d %d\n", ev.tfinger.x, ev.tfinger.y);
							result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDERIGHT, 1, ev.tfinger.x, ev.tfinger.y);
						}
						else
						{
							printf("touch: slide to left %d %d\n", ev.button.x, ev.button.y);
							result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDELEFT, 1, ev.tfinger.x, ev.tfinger.y);
						}
					}
					else if (ydiff >= GESTURE_THRESHOLD)
					{
						if (ev.tfinger.y > lasty)
						{
							printf("touch: slide to down %d %d\n", ev.tfinger.x, ev.tfinger.y);
							result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDEDOWN, 1, ev.tfinger.x, ev.tfinger.y);
						}
						else
						{
							printf("touch: slide to up %d %d\n", ev.tfinger.x, ev.tfinger.y);
							result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDEUP, 1, ev.tfinger.x, ev.tfinger.y);
						}
					}
				}
				result |= ituSceneUpdate(&scene, ITU_EVENT_MOUSEUP, 1, ev.tfinger.x, ev.tfinger.y);
				mouseDownTick = 0;
				break;

			case SDL_QUIT:
				done = 1;
				break;
			}
		}

		result |= ituSceneUpdate(&scene, ITU_EVENT_TIMER, 0, 0, 0);
		if (result)
		{
			ituSceneDraw(&scene, ituGetDisplaySurface());
			ituFlip(ituGetDisplaySurface());
		}

		delay = 33 - (SDL_GetTicks() - tick);
		if (delay > 0)
		{
			SDL_Delay(delay);
		}
	}

	SDL_Quit();
	return (0);
}