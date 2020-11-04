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

//bool EmergencyOnEnter(ITUWidget* widget, char* param)
//{
//	return true;
//}


bool EmergencyBackgroundBtnOnPress(ITUWidget* widget, char* param)
{
	emergency_btn_show = false;

    return true;
}

