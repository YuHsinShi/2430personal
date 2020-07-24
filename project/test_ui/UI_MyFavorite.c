#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "SDL/SDL.h"
#include "ite/itu.h"

#define IconLinkSurface
#define GESTURE_THRESHOLD 40
#define Total_Album 7

static ITUScene scene;
static int favorite[Total_Album] = { 0 };
static int order[Total_Album];

static ITUIcon *mainCoverflowIcon[7] = { 0 };
static ITUIcon *mainIcon[7] = { 0 };
static ITUSprite *albumSprite = 0;
static ITULayer *albumLayer = 0;
static ITUCheckBox *albumCheckBox[7] = { 0 };
static ITUContainer *favoriteContainer[7] = { 0 };
static ITUContainer *favoriteAllContainer = 0;
static ITUText *favoriteDelText = 0;
static ITUStopAnywhere *favoriteStopAnywhere = 0;

void FavoriteMoveToTail(int type)
{
	int i, j, k;
	for (i = 0; i<Total_Album; i++) {
		if (order[i] == type)
			break;
	}
	for (j = i + 1; j<Total_Album; j++) {
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
	for (i = 0; i < Total_Album; i++) {
		if (order[i] == type)
			break;
	}
	for (j = i; j >= 0; j--)
		order[j] = order[j - 1];
	order[0] = type;
}

void RefreshFavoriteList()
{
	int i;
	for (i = 0; i<Total_Album; i++) {
		ituWidgetSetPosition(favoriteContainer[i], 272, 0);
		ituWidgetSetVisible(favoriteContainer[i], false);
	}
	printf("%d\n", __LINE__);
	i = 0;
	while (1) {
		if (i >= Total_Album)
			break;
		if (favorite[order[i]] == 0)
			break;
		else {
			ituWidgetSetPosition(favoriteContainer[order[i]], 0, 166 * i);
			ituWidgetSetVisible(favoriteContainer[order[i]], true);
		}
		i++;
	}
	printf("%d\n", __LINE__);
	if (i>2)
		ituWidgetSetDimension(favoriteAllContainer, 260, 146 + 166 * (i - 1));
	else
		ituWidgetSetDimension(favoriteAllContainer, 260, 412);
	printf("%d\n", __LINE__);

	if (i == 0) {
		ituWidgetSetVisible(favoriteStopAnywhere, false);
	}
	else {
		ituWidgetSetVisible(favoriteStopAnywhere, true);
	}
	printf("%d\n", __LINE__);
}

bool MainOnEnter(ITUWidget* widget, char* param)
{
	int i;
	if (!mainCoverflowIcon[0])
	{
		mainCoverflowIcon[0] = ituSceneFindWidget(&scene, "mainCoverflowIcon0");
		assert(mainCoverflowIcon[0]);
		mainCoverflowIcon[1] = ituSceneFindWidget(&scene, "mainCoverflowIcon1");
		assert(mainCoverflowIcon[1]);
		mainCoverflowIcon[2] = ituSceneFindWidget(&scene, "mainCoverflowIcon2");
		assert(mainCoverflowIcon[2]);
		mainCoverflowIcon[3] = ituSceneFindWidget(&scene, "mainCoverflowIcon3");
		assert(mainCoverflowIcon[3]);
		mainCoverflowIcon[4] = ituSceneFindWidget(&scene, "mainCoverflowIcon4");
		assert(mainCoverflowIcon[4]);
		mainCoverflowIcon[5] = ituSceneFindWidget(&scene, "mainCoverflowIcon5");
		assert(mainCoverflowIcon[5]);
		mainCoverflowIcon[6] = ituSceneFindWidget(&scene, "mainCoverflowIcon6");
		assert(mainCoverflowIcon[6]);

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

		favoriteContainer[0] = ituSceneFindWidget(&scene, "favoriteContainer1");
		assert(favoriteContainer[0]);
		favoriteContainer[1] = ituSceneFindWidget(&scene, "favoriteContainer2");
		assert(favoriteContainer[1]);
		favoriteContainer[2] = ituSceneFindWidget(&scene, "favoriteContainer3");
		assert(favoriteContainer[2]);
		favoriteContainer[3] = ituSceneFindWidget(&scene, "favoriteContainer4");
		assert(favoriteContainer[3]);
		favoriteContainer[4] = ituSceneFindWidget(&scene, "favoriteContainer5");
		assert(favoriteContainer[4]);
		favoriteContainer[5] = ituSceneFindWidget(&scene, "favoriteContainer6");
		assert(favoriteContainer[5]);
		favoriteContainer[6] = ituSceneFindWidget(&scene, "favoriteContainer7");
		assert(favoriteContainer[6]);
		favoriteAllContainer = ituSceneFindWidget(&scene, "favoriteAllContainer");
		assert(favoriteAllContainer);

		favoriteStopAnywhere = ituSceneFindWidget(&scene, "favoriteStopAnywhere");
		assert(favoriteStopAnywhere);

		favoriteDelText = ituSceneFindWidget(&scene, "favoriteDelText");
		assert(favoriteDelText);

		albumLayer = ituSceneFindWidget(&scene, "albumLayer");
		assert(albumLayer);
		
		albumSprite = ituSceneFindWidget(&scene, "albumSprite");
		assert(albumSprite);

		albumLayer = ituSceneFindWidget(&scene, "albumLayer");
		assert(albumLayer);

		albumCheckBox[0] = ituSceneFindWidget(&scene, "albumCheckBox0");
		assert(albumCheckBox[0]);
		albumCheckBox[1] = ituSceneFindWidget(&scene, "albumCheckBox1");
		assert(albumCheckBox[1]);
		albumCheckBox[2] = ituSceneFindWidget(&scene, "albumCheckBox2");
		assert(albumCheckBox[2]);
		albumCheckBox[3] = ituSceneFindWidget(&scene, "albumCheckBox3");
		assert(albumCheckBox[3]);
		albumCheckBox[4] = ituSceneFindWidget(&scene, "albumCheckBox4");
		assert(albumCheckBox[4]);
		albumCheckBox[5] = ituSceneFindWidget(&scene, "albumCheckBox5");
		assert(albumCheckBox[5]);
		albumCheckBox[6] = ituSceneFindWidget(&scene, "albumCheckBox6");
		assert(albumCheckBox[6]);

		for (i = 0; i < Total_Album;i++)
			order[i] = i;
	}

	for (i = 0; i<Total_Album; i++) {
		ituIconLinkSurface(mainCoverflowIcon[i], mainIcon[order[i]]);
	}
    return true;
}

bool MainButtonOnPress(ITUWidget* widget, char* param)
{
	int paramInt = atoi(param);
	ituLayerGoto(albumLayer);
	ituSpriteGoto(albumSprite, order[paramInt]);
	if (favorite[order[paramInt]])
		ituCheckBoxSetChecked(albumCheckBox[order[paramInt]], true);
	else
		ituCheckBoxSetChecked(albumCheckBox[order[paramInt]], false);
	
	return true;
}

bool FavoriteCancelButtonOnPress(ITUWidget* widget, char* param)
{
	ituTextSetString(favoriteDelText, ituTextGetString(ituGetVarTarget(0)));
	return true;
}

bool FavoriteDelConfirmButtonOnPress(ITUWidget* widget, char* param)
{
	int i, j, paramInt = atoi(ituGetVarParam(0)) - 1;
	FavoriteMoveToTail(paramInt);
	favorite[paramInt] = 0;

	RefreshFavoriteList();

	for (i = 0; i<Total_Album; i++)
		ituIconLinkSurface(mainCoverflowIcon[i], mainIcon[order[i]]);

	return true;
}

bool MainFavoriteButtonOnPress(ITUWidget* widget, char* param)
{
	RefreshFavoriteList();
	return true;
}

bool AlbumFavoriteCheckBoxOnPress(ITUWidget* widget, char* param)
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
	"MainButtonOnPress", MainButtonOnPress,
	"AlbumFavoriteCheckBoxOnPress", AlbumFavoriteCheckBoxOnPress,
	"FavoriteCancelButtonOnPress", FavoriteCancelButtonOnPress,
	"FavoriteDelConfirmButtonOnPress", FavoriteDelConfirmButtonOnPress,
	"MainFavoriteButtonOnPress", MainFavoriteButtonOnPress,
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
	ituSceneLoadFile(&scene, CFG_PRIVATE_DRIVE ":/UI_MyFavorite.itu");

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