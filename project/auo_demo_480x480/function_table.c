#include "ite/itu.h"
extern bool MainLayerOnTimer(ITUWidget* widget, char* param);
extern bool MainLayerOnEnter(ITUWidget* widget, char* param);

extern bool LogoOnEnter(ITUWidget* widget, char* param);
ITUActionFunction actionFunctions[] =
{
	
	"MainLayerOnTimer", MainLayerOnTimer,
	"MainLayerOnEnter", MainLayerOnEnter,
	"LogoOnEnter", LogoOnEnter,
//	"MainLayerOnSimulateControl", MainLayerOnSimulateControl,

	NULL, NULL
};