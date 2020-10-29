#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"

/* widgets:
commonToolLayer
Background120
commonToolDownloadButton
commonToolConsultButton
commonToolScreenLockButton
commonToolDownloadText
commonToolConsultText
commonToolScreenLockText
BackgroundButton14
Text99
*/
//ITUCoverFlow* mainCoverFlow;
//ITULayer* screenLockLayer;
//
//bool CommonToolScreenLockBtnOnPress(ITUWidget* widget, char* param)
//{
//	if (!mainCoverFlow)
//	{
//		mainCoverFlow = ituSceneFindWidget(&theScene, "mainCoverFlow");
//		assert(mainCoverFlow);
//	}
//
//	if (!screenLockLayer)
//	{
//		screenLockLayer = ituSceneFindWidget(&theScene, "screenLockLayer");
//		assert(screenLockLayer);
//	}
//
//	ituCoverFlowGoto(mainCoverFlow, 1);
//	ituLayerGoto(screenLockLayer);
//}