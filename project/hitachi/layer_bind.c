#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"

/* widgets:
bindLayer
bindBackground1
backgroundButton0
Icon202
Text206
bindBackground0
Text212
Icon131
bindBackgroundSprite
Background3
Background9
Background10
Background11
Background15
*/
//ITUSprite* bindBackgroundSprite = 0;

//bool BindOnEnter(ITUWidget* widget, char* param)
//{
//	if (!bindBackgroundSprite)
//	{
//		bindBackgroundSprite = ituSceneFindWidget(&theScene, "bindBackgroundSprite");
//		assert(bindBackgroundSprite);
//	}
//
//	ituSpriteGoto(bindBackgroundSprite, BgIndex[modeIndex]);
//
//	return true;
//}