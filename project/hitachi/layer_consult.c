#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "project.h"
#include "ite/itp.h"
#include "sys/ioctl.h"

/* widgets:
consultLayer
Background120
Background126
Text1
Text128
Icon127
BackgroundButton14
Text99
*/

bool ConsultBackBackgroundBtnOnPress(ITUWidget* widget, char* param)
{
	ITULayer* layer;

	layer = (ITULayer*)ituGetVarTarget(0);

	ituLayerGoto(layer);
    return true;
}

