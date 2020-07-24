#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "SDL/SDL.h"
#include "ite/itu.h"

#define GESTURE_THRESHOLD 40
#define MOUSEDOWN_LONGPRESS_DELAY   1000
#define MenuRowDistance  150
#define MenuColumnDistance 200
#define MenuFirstX 78
#define MenuFirstY 60
#define MenuTotalApp 10
#define MenuRowRange 5
#define MenuPageNum 2

static ITUScene scene;
static int moveTableX[2][10] = {
	{ 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 },
	{ 80, 80, 80, 80, 80, 80, 80, 80, 80, 80 }
};

static int moveTableY[10] = {
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15
};

static ITUContainer *mainContainer[MenuTotalApp] = { 0 };
static ITUAnimation *mainAnimation[MenuTotalApp] = { 0 };
static ITUContainer *mainSubContainer[MenuPageNum][MenuTotalApp] = { 0 };
static ITUAnimation *mainSub0Animation[MenuTotalApp] = { 0 };
static ITUAnimation *mainSub1Animation[MenuTotalApp] = { 0 };
static ITUIcon *mainIcon[MenuTotalApp][3] = { 0 };
static ITUIcon *mainTrashCanIcon = 0;
static ITUCoverFlow *mainCoverFlow = 0;
static ITUButton *mainButton[MenuTotalApp] = { 0 };

static bool longPressState = false, arranging = false, is_trash = false, moving = false;
static int mainIndex, lastX, lastY, lastPos, offsetX, offsetY;
static int hold_cnt = 0, arrange_cnt = 0, margin_hold_cnt = 0, page_index = 0;
static int arrange_start, arrange_end, main_total_app, main_row_num;
static int order[MenuPageNum][10] = { 0 };
static int page_total_app[MenuPageNum];

static int getCurrentPosition(int x, int y)
{
	int pos = -1, i, j;
	for (i = 0; i < main_row_num; i++){
		if ((y >= (MenuFirstY + MenuRowDistance * i))
			&& (y < (MenuFirstY + MenuRowDistance * (i + 1)))){
			for (j = 0; j < MenuRowRange; j++){
				if ((x >= (MenuFirstX + MenuColumnDistance * j)) 
					&& (x < (MenuFirstX + MenuColumnDistance * (j+1)))) 
					pos = i * MenuRowRange + j;
			}
		}
	}
	if (pos == -1 || pos > page_total_app[page_index] - 1)
		pos = page_total_app[page_index] - 1;
	return pos;
}

bool MainOnEnter(ITUWidget* widget, char* param)
{
	int i, j;
	char name[128];
	main_total_app = MenuTotalApp;
	if (main_total_app % MenuRowRange != 0)
		main_row_num = main_total_app / MenuRowRange + 1;
	else
		main_row_num = main_total_app / MenuRowRange;
	if (!mainContainer[0])
	{
		for (i = 0; i < main_total_app; i++){
			sprintf(name, "mainContainer%d\0", i);
			mainContainer[i] = ituSceneFindWidget(&scene, name);
			assert(mainContainer[i]);
			sprintf(name, "mainAnimation%d\0", i);
			mainAnimation[i] = ituSceneFindWidget(&scene, name);
			assert(mainAnimation[i]);
			sprintf(name, "mainSub0Animation%d\0", i);
			mainSub0Animation[i] = ituSceneFindWidget(&scene, name);
			assert(mainSub0Animation[i]);
			sprintf(name, "mainSub1Animation%d\0", i);
			mainSub1Animation[i] = ituSceneFindWidget(&scene, name);
			assert(mainSub1Animation[i]);
			sprintf(name, "mainButton%d\0", i);
			mainButton[i] = ituSceneFindWidget(&scene, name);
			assert(mainButton[i]);
			for (j = 0; j < 3; j++){
				sprintf(name, "mainIcon%d%d\0", i, j);
				mainIcon[i][j] = ituSceneFindWidget(&scene, name);
				assert(mainIcon[i][j]);
			}
		}
		for (i = 0; i < MenuPageNum; i++){
			for (j = 0; j < main_total_app; j++){
				sprintf(name, "mainSub%dContainer%d\0", i, j);
				mainSubContainer[i][j] = ituSceneFindWidget(&scene, name);
				assert(mainSubContainer[i][j]);
			}
		}
		mainTrashCanIcon = ituSceneFindWidget(&scene, "mainTrashCanIcon");
		assert(mainTrashCanIcon);
		mainCoverFlow = ituSceneFindWidget(&scene, "mainCoverFlow");
		assert(mainCoverFlow);
	}
	for (i = 0; i < main_total_app; i++){
		order[0][i] = i;
		order[1][i] = -1;
	}
	page_total_app[0] = MenuTotalApp;
	page_total_app[1] = 0;
	longPressState = false;
	mainIndex = -1;
	return true;
}

bool MainOnTimer(ITUWidget* widget, char* param)
{
	bool updated = false;
	int i, curX, curY, curPos, tmp;
	if (longPressState == true) {
		widget = (ITUWidget*)mainCoverFlow;
		widget->flags &= ~ITU_DRAGGING;
		curX = scene.lastMouseX;
		curY = scene.lastMouseY;
		curPos = getCurrentPosition(curX, curY);
		//printf("\n%d\n", curPos);
		ituWidgetSetX(mainContainer[mainIndex], curX - offsetX);
		ituWidgetSetY(mainContainer[mainIndex], curY - offsetY);
		// touch trash can alpha goto 100
		if (ituWidgetGetX(mainContainer[mainIndex]) > 819 
			&& ituWidgetGetX(mainContainer[mainIndex]) < 938 
			&& ituWidgetGetY(mainContainer[mainIndex]) > 386 
			&& ituWidgetGetY(mainContainer[mainIndex]) < 506)
		{
			for (i = 0; i < 3; i++)
				ituWidgetSetAlpha(mainIcon[mainIndex][i], 100);
			is_trash = true;
		}
		else {
			for (i = 0; i < 3; i++)
				ituWidgetSetAlpha(mainIcon[mainIndex][i], 255);
			is_trash = false;
		}
		// touch trash can alpha goto 100

		// page goto next/prev
		if (ituWidgetGetX(mainContainer[mainIndex]) > 966
			|| ituWidgetGetX(mainContainer[mainIndex]) < -10){
			margin_hold_cnt++;
			// move to last immedietly
			curPos = page_total_app[page_index] - 1;
		}
		else
			margin_hold_cnt = 0;
		// page goto next/prev

		if (abs(curX - lastX)<5 && abs(curY - lastY)<5 && curPos >= 0) {
			hold_cnt++;
		}
		else {
			hold_cnt = 0;
			lastX = curX;
			lastY = curY;
		}
		// margin determine
		if (margin_hold_cnt > 20 && !moving){
			if (ituWidgetGetX(mainContainer[mainIndex]) > 966
				&& page_index < MenuPageNum - 1){
				ituCoverFlowNext(mainCoverFlow);
				moving = true;
			}
			else if (ituWidgetGetX(mainContainer[mainIndex]) < -10
				&& page_index > 0){
				ituCoverFlowPrev(mainCoverFlow);
				moving = true;
			}
			mainButton[mainIndex]->pressed = true;
			margin_hold_cnt = 0;
		}
		// margin determine
		// arrange order
		if (hold_cnt>1 && curPos != lastPos && arranging == false) {
			printf("arrange.... start from %d to %d\n", lastPos, curPos);
			arranging = true;
			arrange_cnt = 0;
			arrange_start = lastPos;
			arrange_end = curPos;
			tmp = order[page_index][arrange_start];
			if (arrange_start<arrange_end) {
				for (i = arrange_start; i < arrange_end; i++)
					order[page_index][i] = order[page_index][i + 1];
			}
			else {
				for (i = arrange_start; i > arrange_end; i--)
					order[page_index][i] = order[page_index][i - 1];
			}
			order[page_index][arrange_end] = tmp;
			lastPos = curPos;
		}
		// arrange order
		if (arranging == true && arrange_cnt < 10)
		{
			if (arrange_start<arrange_end) {
				for (i = arrange_start; i < arrange_end; i++) {
					if (i == 4) {
						ituWidgetSetX(mainSubContainer[page_index][order[page_index][i]], ituWidgetGetX(mainSubContainer[page_index][order[page_index][i]]) + moveTableX[1][arrange_cnt]);
						ituWidgetSetY(mainSubContainer[page_index][order[page_index][i]], ituWidgetGetY(mainSubContainer[page_index][order[page_index][i]]) - moveTableY[arrange_cnt]);
					}
					else
						ituWidgetSetX(mainSubContainer[page_index][order[page_index][i]], ituWidgetGetX(mainSubContainer[page_index][order[page_index][i]]) - moveTableX[0][arrange_cnt]);
				}
			}
			else {
				for (i = arrange_start; i > arrange_end; i--) {
					if (i == 5) {
						ituWidgetSetX(mainSubContainer[page_index][order[page_index][i]], ituWidgetGetX(mainSubContainer[page_index][order[page_index][i]]) - moveTableX[1][arrange_cnt]);
						ituWidgetSetY(mainSubContainer[page_index][order[page_index][i]], ituWidgetGetY(mainSubContainer[page_index][order[page_index][i]]) + moveTableY[arrange_cnt]);
					}
					else
						ituWidgetSetX(mainSubContainer[page_index][order[page_index][i]], ituWidgetGetX(mainSubContainer[page_index][order[page_index][i]]) + moveTableX[0][arrange_cnt]);
				}
			}
			arrange_cnt++;
		}
		if (arrange_cnt >= 10) {
			arranging = false;
		}
		updated = true;
	}
	return updated;
}

bool MainButtonOnLongPress(ITUWidget* widget, char* param)
{
	int i, j;
	page_index = mainCoverFlow->focusIndex;
	mainIndex = atoi(param);
	// make all visible animation play
	ituAnimationPlay(mainAnimation[mainIndex], 0);
	for (i = 0; i < MenuPageNum; i++){
		for (j = 0; j < page_total_app[i]; j++){
			if (order[i][j] < 0)
				continue;
			ituAnimationPlay(mainSub0Animation[order[i][j]], 0);
			ituAnimationPlay(mainSub1Animation[order[i][j]], 0);
		}
	}
	// make all visible animation play
	ituWidgetSetVisible(mainSubContainer[page_index][mainIndex], false);
	ituWidgetSetVisible(mainContainer[mainIndex], true);
	ituWidgetToBottom(mainContainer[mainIndex]);
	lastX = ituWidgetGetX(mainContainer[mainIndex]);
	lastY = ituWidgetGetY(mainContainer[mainIndex]);
	offsetX = scene.lastMouseX - lastX;
	offsetY = scene.lastMouseY - lastY;
	lastPos = getCurrentPosition(scene.lastMouseX, scene.lastMouseY);
	longPressState = true;
	hold_cnt = 0;
	return true;
}

bool MainButtonOnMouseUp(ITUWidget* widget, char* param)
{
	int i, j, x, y;
	if (longPressState == true) {
		ituWidgetSetVisible(mainContainer[mainIndex], false);
		ituWidgetSetVisible(mainSubContainer[page_index][mainIndex], true);
		longPressState = false;
		arranging = false;
		arrange_cnt = 0;
		for (i = 0; i < MenuPageNum; i++){
			for (j = 0; j < page_total_app[i]; j++){
				if (order[i][j] < 0)
					continue;
				ituAnimationGoto(mainSub0Animation[order[i][j]], 0);
				ituAnimationGoto(mainSub1Animation[order[i][j]], 0);

				if (j < MenuRowRange) { x = MenuFirstX + MenuColumnDistance * j; y = MenuFirstY; }
				else { x = MenuFirstX + MenuColumnDistance * (j - MenuRowRange); y = MenuFirstY + MenuRowDistance; }
				ituWidgetSetPosition(mainSubContainer[i][order[i][j]], x, y);
				ituWidgetSetPosition(mainContainer[order[i][j]], x, y);
			}
		}
		if (is_trash && (hold_cnt > 1)) {
			ituWidgetSetVisible(mainContainer[mainIndex], false);
			ituWidgetSetVisible(mainSubContainer[page_index][mainIndex], false);
			order[page_index][page_total_app[page_index] - 1] = -1;
			main_total_app--;
			page_total_app[page_index]--;
		}
		mainIndex = -1;
	}
	return true;
}

bool MainCoverFlowOnChange(ITUWidget* widget, char* param)
{
	if (longPressState == true) {
		order[page_index][page_total_app[page_index] - 1] = -1;
		page_total_app[page_index]--;
		page_index = mainCoverFlow->focusIndex;
		page_total_app[page_index]++;
		order[page_index][page_total_app[page_index] - 1] = mainIndex;
		lastPos = page_total_app[page_index] - 1;
	}
	moving = false;
	return true;
}

ITUActionFunction actionFunctions[] =
{
	"MainOnEnter", MainOnEnter,
	"MainOnTimer", MainOnTimer,
	"MainButtonOnLongPress", MainButtonOnLongPress,
	"MainButtonOnMouseUp", MainButtonOnMouseUp,
	"MainCoverFlowOnChange", MainCoverFlowOnChange,
	NULL, NULL
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
	window = SDL_CreateWindow("UI Hexahedron Flip",
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
	ituSceneLoadFile(&scene, CFG_PRIVATE_DRIVE ":/UI_Dynamic_Menu.itu");

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

		if (mouseDownTick > 0 && (SDL_GetTicks() - mouseDownTick >= MOUSEDOWN_LONGPRESS_DELAY))
		{
			printf("long press: %d %d\n", lastx, lasty);
			result |= ituSceneUpdate(&scene, ITU_EVENT_MOUSELONGPRESS, 1, lastx, lasty);
			mouseDownTick = 0;
		}

		result |= ituSceneUpdate(&scene, ITU_EVENT_TIMER, 0, 0, 0);
		if (result)
		{
			ituSceneDraw(&scene, ituGetDisplaySurface());
			ituFlip(ituGetDisplaySurface());
		}

		delay = 17- (SDL_GetTicks() - tick);
		if (delay > 0)
		{
			SDL_Delay(delay);
		}
	}

	SDL_Quit();
	return (0);
}