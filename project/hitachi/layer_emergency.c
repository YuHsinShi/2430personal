#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"

/* widgets:
emergencyLayer
Background140
Text164
Icon166
emergencyBackgroundButton
*/

//ITUSprite* emergencyBackgroundSprite = 0;
//
//bool EmergencyOnEnter(ITUWidget* widget, char* param)
//{
//	if (!emergencyBackgroundSprite)
//	{
//		emergencyBackgroundSprite = ituSceneFindWidget(&theScene, "emergencyBackgroundSprite");
//		assert(emergencyBackgroundSprite);
//	}
//
//	ituSpriteGoto(emergencyBackgroundSprite, BgIndex[modeIndex]);
//	return true;
//}


bool EmergencyBackgroundBtnOnPress(ITUWidget* widget, char* param)
{
	emergency_btn_show = false;

	ITULayer* layer;

	layer = (ITULayer*)ituGetVarTarget(0);

	ituLayerGoto(layer);

    return true;
}

