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
static dictionary* cfgIni;
static bool cfgIsSaving;
static bool cfgSavingCount;
static pthread_mutex_t cfgMutex  = PTHREAD_MUTEX_INITIALIZER;

extern void BackupSave(void);

void ConfigInit(void)
{
    cfgIni = iniparser_load(CFG_PUBLIC_DRIVE ":/" INI_FILENAME);
    if (!cfgIni)
    {
        cfgIni = dictionary_new(0);
        assert(cfgIni);

        dictionary_set(cfgIni, "tcpip", NULL);
        dictionary_set(cfgIni, "sound", NULL);
		dictionary_set(cfgIni, "temperature", NULL);
		dictionary_set(cfgIni, "default", NULL);
    }

    // network
    theConfig.dhcp = iniparser_getboolean(cfgIni, "tcpip:dhcp", 1);
    strncpy(theConfig.ipaddr, iniparser_getstring(cfgIni, "tcpip:ipaddr", "192.168.1.1"), sizeof (theConfig.ipaddr) - 1);
    strncpy(theConfig.netmask, iniparser_getstring(cfgIni, "tcpip:netmask", "255.255.255.0"), sizeof (theConfig.netmask) - 1);
    strncpy(theConfig.gw, iniparser_getstring(cfgIni, "tcpip:gw", "192.168.1.254"), sizeof (theConfig.gw) - 1);
    strncpy(theConfig.dns, iniparser_getstring(cfgIni, "tcpip:dns", "192.168.1.254"), sizeof (theConfig.dns) - 1);

    // sound
    strcpy(theConfig.keysound, iniparser_getstring(cfgIni, "sound:keysound", "key1.wav"));
    theConfig.keylevel = iniparser_getint(cfgIni, "sound:keylevel", 20);
    theConfig.audiolevel = iniparser_getint(cfgIni, "sound:audiolevel", 15);

	//temperature
	theConfig.brightness = iniparser_getint(cfgIni, "temperature:brightness", 8);
	theConfig.screensaver_time = iniparser_getint(cfgIni, "temperature:screensaver_time", 15);
	theConfig.screensaver_type = iniparser_getint(cfgIni, "temperature:screensaver_type", 1);
	theConfig.scene = iniparser_getint(cfgIni, "temperature:scene", 2);
    theConfig.ncp18_offset = (float)iniparser_getdouble(cfgIni, "temperature:ncp18_offset", 0);

	//default
	theConfig.def_keylevel = iniparser_getint(cfgIni, "default:keylevel", 20);
	theConfig.def_audiolevel = iniparser_getint(cfgIni, "default:def_audiolevel", 15);
	theConfig.def_brightness = iniparser_getint(cfgIni, "default:brightness", 8);
	theConfig.def_screensaver_time = iniparser_getint(cfgIni, "default:screensaver_time", 15);
	theConfig.def_screensaver_type = iniparser_getint(cfgIni, "default:screensaver_type", 1);

	// SSID
	snprintf(theConfig.ssid, 64, iniparser_getstring(cfgIni, "wifi:ssid", ""));
	// Password
	snprintf(theConfig.password, 256, iniparser_getstring(cfgIni, "wifi:password", ""));
	// Security mode
	snprintf(theConfig.secumode, 3, iniparser_getstring(cfgIni, "wifi:secumode", "NA"));
	// wifi mode, client:0  softAP:1
	theConfig.wifi_mode = iniparser_getint(cfgIni, "wifi:wifi_mode", WIFI_CLIENT);
	// wifi switch on/off
	theConfig.wifi_on_off = iniparser_getint(cfgIni, "wifi:wifi_on_off", 0);

	//AP mode SSID
	snprintf(theConfig.ap_ssid, 64, iniparser_getstring(cfgIni, "wifi:ap_ssid", ""));
	// AP mode Password
	snprintf(theConfig.ap_password, 256, iniparser_getstring(cfgIni, "wifi:ap_password", ""));

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
    char buf[16];

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

	//temperature
	sprintf(buf, "%d", theConfig.brightness);
	iniparser_set(cfgIni, "temperature:brightness", buf);
	
	sprintf(buf, "%d", theConfig.screensaver_time);
	iniparser_set(cfgIni, "temperature:screensaver_time", buf);
	
	sprintf(buf, "%d", theConfig.screensaver_type);
	iniparser_set(cfgIni, "temperature:screensaver_type", buf);

	sprintf(buf, "%d", theConfig.scene);
	iniparser_set(cfgIni, "temperature:scene", buf);

    sprintf(buf, "%f", theConfig.ncp18_offset);
    iniparser_set(cfgIni, "temperature:ncp18_offset", buf);

	//default
	sprintf(buf, "%d", theConfig.def_keylevel);
	iniparser_set(cfgIni, "default:keylevel", buf);

	sprintf(buf, "%d", theConfig.def_audiolevel);
	iniparser_set(cfgIni, "default:audiolevel", buf);

	sprintf(buf, "%d", theConfig.def_brightness);
	iniparser_set(cfgIni, "default:brightness", buf);

	sprintf(buf, "%d", theConfig.def_screensaver_time);
	iniparser_set(cfgIni, "default:screensaver_time", buf);

	sprintf(buf, "%d", theConfig.def_screensaver_type);
	iniparser_set(cfgIni, "default:screensaver_type", buf);

	// Wifi SSID
	iniparser_set(cfgIni, "wifi:ssid", theConfig.ssid);
	// Password
	iniparser_set(cfgIni, "wifi:password", theConfig.password);
	// Security mode
	iniparser_set(cfgIni, "wifi:secumode", theConfig.secumode);
	// wifi mode
	sprintf(buf, "%d", theConfig.wifi_mode);
	iniparser_set(cfgIni, "wifi:wifi_mode", buf);
	// wifi switch on/off
	sprintf(buf, "%d", theConfig.wifi_on_off);
	iniparser_set(cfgIni, "wifi:wifi_on_off", buf);


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
