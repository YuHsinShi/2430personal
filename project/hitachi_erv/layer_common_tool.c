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

ITUIcon* commonToolFilterDisableIcon = 0;
ITUText* commonToolFilterDisableText = 0;
ITUButton* commonToolFilterButton = 0;

bool CommonToolOnEnter(ITUWidget* widget, char* param)
{

	if (!commonToolFilterDisableIcon)
	{

		commonToolFilterDisableIcon = ituSceneFindWidget(&theScene, "commonToolFilterDisableIcon");
		assert(commonToolFilterDisableIcon);

		commonToolFilterDisableText = ituSceneFindWidget(&theScene, "commonToolFilterDisableText");
		assert(commonToolFilterDisableText);

		commonToolFilterButton = ituSceneFindWidget(&theScene, "commonToolFilterButton");
		assert(commonToolFilterButton);

	}

	ituWidgetSetVisible(commonToolFilterDisableIcon, !filter_btn_show);
	ituWidgetSetVisible(commonToolFilterDisableText, !filter_btn_show);
	ituWidgetSetVisible(commonToolFilterButton, filter_btn_show);

	return true;
}