#include <stdio.h>
#include <string.h>
#include "ite/ith.h"
#include "project.h"

// need to install VS90SP1-KB980263-x86.exe for vs2008
#pragma execution_character_set("utf-8")

static const char* stringScenesNameArray[] =
{
	"工作",
	"在家",
	"電影",
	"離家"
};

const char* StringGetScenesName(int index)
{
	return stringScenesNameArray[index];
}

const char* StringGetWiFiConnected(void)
{
		return "已連接";
}