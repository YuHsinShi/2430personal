#include "ite/itu.h"


extern bool MainOnEnter(ITUWidget* widget, char* param);
extern bool MainButtonOnMouseUp(ITUWidget* widget, char* param);

extern bool UartCaptureOnEnter(ITUWidget* widget, char* param);
extern bool UartCaptureOnTimer(ITUWidget* widget, char* param);
extern bool UartCaptureOnLeave(ITUWidget* widget, char* param);


extern bool PowerOnEnter(ITUWidget* widget, char* param);
extern bool PowerOnTimer(ITUWidget* widget, char* param);
extern bool PowerOnLeave(ITUWidget* widget, char* param);

extern bool SettingOnEnter(ITUWidget* widget, char* param);
extern bool Setting_next(ITUWidget* widget, char* param);
extern bool Setting_prev(ITUWidget* widget, char* param);

extern bool BurnerOnEnter(ITUWidget* widget, char* param);
extern bool BurnerOnTimer(ITUWidget* widget, char* param);
extern bool BurnerOnLeave(ITUWidget* widget, char* param);

extern bool LayerEditOnEnter(ITUWidget* widget, char* param);
extern bool LayerEditNextItem(ITUWidget* widget, char* param);

extern bool LayerEditPrevItem(ITUWidget* widget, char* param);

extern bool LayerEditSave(ITUWidget* widget, char* param);

extern bool LayerEditSelectNext(ITUWidget* widget, char* param);

ITUActionFunction actionFunctions[] =
{
	"MainOnEnter", MainOnEnter,
	"MainButtonOnMouseUp", MainButtonOnMouseUp,
	"UartCaptureOnTimer", UartCaptureOnTimer,
	"UartCaptureOnEnter", UartCaptureOnEnter,
	"UartCaptureOnLeave", UartCaptureOnLeave,
	
	"PowerOnEnter", PowerOnEnter,
	"PowerOnTimer", PowerOnTimer,
	"PowerOnLeave", PowerOnLeave,

	"SettingOnEnter", SettingOnEnter,
	"Setting_next", Setting_next,
	"Setting_prev", Setting_prev,

	"BurnerOnEnter", BurnerOnEnter,
	"BurnerOnTimer", BurnerOnTimer,
	"BurnerOnLeave",BurnerOnLeave,

	"LayerEditOnEnter", LayerEditOnEnter,
	"LayerEditNextItem", LayerEditNextItem,
	"LayerEditPrevItem", LayerEditPrevItem,
	"LayerEditSave", LayerEditSave,
	"LayerEditSelectNext", LayerEditSelectNext,
    NULL, NULL
};
