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

static const char* stringScenesEngNameArray[] =
{
	"Work",
	"Home",
	"Cinema",
	"Stay Out"
};

const char* StringGetScenesEngName(int index)
{
	return stringScenesEngNameArray[index];
}
static const char* stringMainChtNameArray[] =
{
	"音 樂",
	"場 景",
	"燈 光",
	"窗 簾"

};

const char* StringGetMainChtName(int index)
{
	return stringMainChtNameArray[index];
}

static const char* stringMainEngNameArray[] =
{
	"Music",
	"Mode",
	"Light",
	"Curtain"
};

const char* StringGetMainEngName(int index)
{
	return stringMainEngNameArray[index];
}
static const char* stringSettingChtNameArray[] =
{
	"螢幕亮度",
	"音量",
	"螢幕保護時間",
	"秒",
	"螢幕保護",
	"打開WiFi",
	"關閉WiFi",
	"溫度校正",
	"現在溫度",
	"校正溫度",
	"校正偏移量",
	"設備型號",
	"軟體版本",
	"硬體版本",
	"本機IP",
	"本機MAC",
	"重啟",
	"恢復出廠設置",
	"設定",
	"重啟",
	"恢復出廠設置",
	"黑屏",
	"時鐘",
	"確認",
	"取消",
	"確認",
	"取消"

};

const char* StringGetSettingChtName(int index)
{
	return stringSettingChtNameArray[index];
}

static const char* stringSettingEngNameArray[] =
{
	"Brightness",
	"Volume",
	"Screen Saving Time",
	"Sec",
	"Screen Saving Mode",
	"Enable WiFi",
	"Disable WiFi",
	"Calibration",
	"Temperature",
	"Calibration",
	"Offset",
	"Part Number",
	"S/W Version",
	"H/W Version",
	"Local IP",
	"MAC",
	"Reset",
	"Factory Restore",
	"Set",
	"Reset",
	"Factory Restore",
	"Black Screen",
	"Clock",
	"OK",
	"Cancel",
	"OK",
	"Cancel"
};

const char* StringGetSettingEngName(int index)
{
	return stringSettingEngNameArray[index];
}
static const char* stringLightChtNameArray[] =
{
	"客廳燈1",
	"客廳燈2",
	"客廳燈3",
	"客廳燈4",
	"客廳燈5",
	"客廳燈6",
	"請打掃",
	"勿打擾",
	"緊急求救"

};

const char* StringGetLightChtName(int index)
{
	return stringLightChtNameArray[index];
}

static const char* stringLightEngNameArray[] =
{
	"Living Room Light1",
	"Living Room Light2",
	"Living Room Light3",
	"Living Room Light4",
	"Living Room Light5",
	"Living Room Light6",
	"Clean Up",
	"Do not Disturb",
	"S.O.S"
};

const char* StringGetLightEngName(int index)
{
	return stringLightEngNameArray[index];
}


const char* StringGetWiFiConnected(void)
{
		return "已連接";
}
const char* StringGetWiFiConnectedEng(void)
{
	return "Connected";
}