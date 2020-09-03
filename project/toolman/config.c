#include <sys/ioctl.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "iniparser/iniparser.h"
#include "ite/itp.h"
#include "project.h"
#include "scene.h"

#define INI_FILENAME "config.ini"

Config theConfig;
UART_SET uart[5];

static dictionary* cfgIni;
static bool cfgIsSaving;
static int cfgSavingCount;
static pthread_mutex_t cfgMutex  = PTHREAD_MUTEX_INITIALIZER;




void Setting_PrintAll()
{
int i;
	for (i = 1; i <= 5; i++)
	{
		printf("CH %d \n",i);
		//printf("enable =%d\n",uart[i - 1].enable );
		printf("baud_rate =%d\n",uart[i - 1].baud_rate );
		printf("fileMaxsize =%d\n",uart[i - 1].fileMaxsize );
		printf("fileInterval =%d\n",uart[i - 1].fileInterval );
		printf("timestamp =%d\n",uart[i - 1].timestamp );

	}

}


void SettingInit()
{
	static dictionary* setting_ini;
	char tmp[64] = { 0 };
	int i;

	setting_ini = iniparser_load("B:/setting.ini");
	if (!setting_ini)
	{
		setting_ini = dictionary_new(0);
		assert(setting_ini);
		for (i = 1; i <= 5; i++)
		{
			snprintf(tmp, 64, "uart%d", i);
			dictionary_set(setting_ini, tmp, NULL);

		}
	}
	char* str;
	for (i = 1; i <= 5; i++)
	{
		//snprintf(tmp, 64, "uart%d:on_off", i);
		//uart[i - 1].enable = iniparser_getint(setting_ini, tmp, 1);

		snprintf(tmp, 64, "uart%d:baud_rate", i);
		uart[i - 1].baud_rate = iniparser_getint(setting_ini, tmp, 115200);

		snprintf(tmp, 64, "uart%d:parity", i);
		str = iniparser_getstring(setting_ini, tmp, "NONE");
		strcpy(uart[i - 1].parity, str);

		snprintf(tmp, 64, "uart%d:databit", i);
		uart[i - 1].databit = iniparser_getint(setting_ini, tmp, 8);

		snprintf(tmp, 64, "uart%d:stopbit", i);
		uart[i - 1].stopbit = iniparser_getint(setting_ini, tmp, 1);







		snprintf(tmp, 64, "uart%d:log_size", i);
		uart[i - 1].fileMaxsize = iniparser_getint(setting_ini, tmp, 16);

		snprintf(tmp, 64, "uart%d:log_time", i);
		uart[i - 1].fileInterval = iniparser_getint(setting_ini, tmp, 60);

		snprintf(tmp, 64, "uart%d:timestamp", i);
		uart[i - 1].timestamp = iniparser_getint(setting_ini, tmp, 1);

	}
	Setting_PrintAll();

}


void ConfigInit(void)
{
    cfgIni = iniparser_load(CFG_PUBLIC_DRIVE ":/" INI_FILENAME);
    if (!cfgIni)
    {
        cfgIni = dictionary_new(0);
        assert(cfgIni);

        dictionary_set(cfgIni, "tcpip", NULL);
        dictionary_set(cfgIni, "sound", NULL);
    }
	/*
    // network
    theConfig.dhcp = iniparser_getboolean(cfgIni, "tcpip:dhcp", 1);
    strncpy(theConfig.ipaddr, iniparser_getstring(cfgIni, "tcpip:ipaddr", "192.168.1.1"), sizeof (theConfig.ipaddr) - 1);
    strncpy(theConfig.netmask, iniparser_getstring(cfgIni, "tcpip:netmask", "255.255.255.0"), sizeof (theConfig.netmask) - 1);
    strncpy(theConfig.gw, iniparser_getstring(cfgIni, "tcpip:gw", "192.168.1.254"), sizeof (theConfig.gw) - 1);
    strncpy(theConfig.dns, iniparser_getstring(cfgIni, "tcpip:dns", "192.168.1.254"), sizeof (theConfig.dns) - 1);

    // sound
    strcpy(theConfig.keysound, iniparser_getstring(cfgIni, "sound:keysound", "key1.wav"));
    theConfig.keylevel = iniparser_getint(cfgIni, "sound:keylevel", 80);
    theConfig.audiolevel = iniparser_getint(cfgIni, "sound:audiolevel", 80);
	*/


    cfgSavingCount = 0;
}

void ConfigExit(void)
{
    iniparser_freedict(cfgIni);
    cfgIni = NULL;
}

static void ConfigSavePublic(void)
{
    FILE* f;
    char buf[8];

    // network
    iniparser_set(cfgIni, "tcpip:dhcp", theConfig.dhcp ? "y" : "n");
    iniparser_set(cfgIni, "tcpip:ipaddr", theConfig.ipaddr);
    iniparser_set(cfgIni, "tcpip:netmask", theConfig.netmask);
    iniparser_set(cfgIni, "tcpip:gw", theConfig.gw);
    iniparser_set(cfgIni, "tcpip:dns", theConfig.dns);

    // sound
    iniparser_set(cfgIni, "sound:keysound", theConfig.keysound);

    sprintf(buf, "%d", theConfig.keylevel);
    iniparser_set(cfgIni, "sound:keylevel", buf);

    sprintf(buf, "%d", theConfig.audiolevel);
    iniparser_set(cfgIni, "sound:audiolevel", buf);

    // save to file
    f = fopen(CFG_PUBLIC_DRIVE ":/" INI_FILENAME, "wb");
	if (!f)
    {
	    printf("cannot open ini file: %s\n", CFG_PUBLIC_DRIVE ":/" INI_FILENAME);
        return;
    }

    iniparser_dump_ini(cfgIni, f);
    fclose(f);
}

static void* ConfigSaveTask(void* arg)
{
    char* filepath = CFG_PUBLIC_DRIVE ":/" INI_FILENAME;
    int savingCount;

    cfgIsSaving = true;

    do
    {
        savingCount = cfgSavingCount;

        pthread_mutex_lock(&cfgMutex);

        ConfigSavePublic();

#ifdef CFG_CHECK_FILES_CRC_ON_BOOTING
        UpgradeSetFileCrc(filepath);
#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
        ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
#endif
        BackupSave();
    #else
#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
        ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
#endif
#endif

        pthread_mutex_unlock(&cfgMutex);

    } while (savingCount != cfgSavingCount);

    cfgIsSaving = false;

    return NULL;
}

void ConfigUpdateCrc(char* filepath)
{
#ifdef CFG_CHECK_FILES_CRC_ON_BOOTING
    pthread_mutex_lock(&cfgMutex);

    if (filepath)
        UpgradeSetFileCrc(filepath);
    else
        UpgradeSetFileCrc(CFG_PUBLIC_DRIVE ":/" INI_FILENAME);

#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
    ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
#endif

    pthread_mutex_unlock(&cfgMutex);
#endif // CFG_CHECK_FILES_CRC_ON_BOOTING
}

void ConfigSave(void)
{
    pthread_t task;
    pthread_attr_t attr;

    cfgSavingCount++;

    if (cfgIsSaving)
        return;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&task, &attr, ConfigSaveTask, NULL);
}
