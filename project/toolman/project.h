/** @file
 * Template project definition.
 *
 */
/** @defgroup template
 *  @{
 */
#ifndef PROJECT_H
#define PROJECT_H

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup template_audio Audio Player
 *  @{
 */

typedef int (*AudioPlayCallback)(int state);

/**
 * Initializes audio module.
 */
void AudioInit(void);

/**
 * Exits audio module.
 */
void AudioExit(void);

/**
 * Plays the specified wav file.
 *
 * @param filename The specified wav file to play.
 * @param func The callback function.
 * @return 0 for success, otherwise failed.
 */
int AudioPlay(char* filename, AudioPlayCallback func);

/**
 * Stops playing sound.
 */
void AudioStop(void);

int AudioPlayMusic(char* filename, AudioPlayCallback func);

/**
 * Plays keypad sound.
 */
void AudioPlayKeySound(void);
void AudioPauseKeySound(void);
void AudioResumeKeySound(void);

/**
 * Sets the volume of keypad sound.
 *
 * @param level the percentage of volume.
 */
void AudioSetKeyLevel(int level);

/**
 * Mutes all audio.
 */
void AudioMute(void);

/**
 * Un-mutes all audio.
 */
void AudioUnMute(void);

/**
 * Determines whether this audio is muted or not.
 *
 * @return true muted, false otherwise.
 */
bool AudioIsMuted(void);

bool AudioIsPlaying(void);

void AudioSetVolume(int level);
void AudioSetLevel(int level);
int AudioGetVolume(void);

/** @} */ // end of template_audio

typedef struct
{
	int enable;			//settings
	int baud_rate;		//settings
	int fileMaxsize;	//settings
	int fileInterval;	//settings
	int timestamp;		//settings
	int alive_flag;
}UART_SET;

/** @defgroup template_config Configuration
 *  @{
 */

typedef struct
{
    // network
    int dhcp;                                                           ///< Enable DHCP or not
    char ipaddr[16];                                                    ///< IP address
    char netmask[16];                                                   ///< Netmask
    char gw[16];                                                        ///< Gateway address
    char dns[16];                                                       ///< DNS address

    // sound
    char      keysound[PATH_MAX];                                       // /< Key sound file path
    int       keylevel;                                                 // /< Key volume percentage, range is 0~100
    int       audiolevel;                                               // /< Audio volume percentage, range is 0~100

	UART_SET uart[5];
} Config;

/**
 * Global instance variable of configuration.
 */
extern Config theConfig;

/**
 * Loads configuration file.
 */
void ConfigInit(void);

/**
 * Exits configuration.
 */
void ConfigExit(void);

/**
 * Updates CRC files.
 *
 * @param filepath The file path to update the CRC value. NULL for ini file on public drive.
 */
void ConfigUpdateCrc(char* filepath);

/**
 * Saves the public part of configuration to file.
 */
void ConfigSave(void);

/** @} */ // end of template_config

/** @defgroup template_external External
 *  @{
 */
#define EXTERNAL_BUFFER_SIZE 1024 ///< External buffer size

typedef enum
{
    EXTERNAL_SHOW_MSG,  ///< Show message
    EXTERNAL_TEST0,     ///< Test #0
    EXTERNAL_TEST1,     ///< Test #1
    EXTERNAL_TEST2,     ///< Test #2
    EXTERNAL_TEST3,     ///< Test #3
    EXTERNAL_TEST4,     ///< Test #4
    EXTERNAL_TEST5      ///< Test #5
} ExternalEventType;

typedef struct
{
    ExternalEventType type;
    int arg1;
    int arg2;
    int arg3;
    uint8_t buf1[EXTERNAL_BUFFER_SIZE];

} ExternalEvent;

/**
 * Initializes external module.
 */
void ExternalInit(void);

/**
 * Exits external module.
 */
void ExternalExit(void);

/**
 * Receives external module event.
 *
 * @param ev The external event.
 * @return 0 for no event yet, otherwise for event occured.
 */
int ExternalReceive(ExternalEvent* ev);

/**
 * Sends external module event.
 *
 * @param ev The external event.
 * @return 0 for success, otherwise for failure.
 */
int ExternalSend(ExternalEvent* ev);

/**
 * Initializes external process module.
 */
void ExternalProcessInit(void);

/**
 * Processes external module event.
 *
 * @param ev The external event.
 * @return 0 for no event yet, otherwise for event occured.
 */
void ExternalProcessEvent(ExternalEvent* ev);

/** @} */ // end of template_external

/** @defgroup template upgarde_uart
 *  @{
 */
#if defined(CFG_UPGRADE_FROM_UART)
#if defined(CFG_UPGRADE_UART0)
#define UPGRADE_UART_PORT	ITP_DEVICE_UART0
#elif defined(CFG_UPGRADE_UART1)
#define UPGRADE_UART_PORT	ITP_DEVICE_UART1
#else
#define UPGRADE_UART_PORT	ITP_DEVICE_UART0
#endif
#define UPGRADE_PATTERN				0x1A

#define ACK20						0x14
#define ACK50						0x32
#define ACK100						0x64
#define ACK150						0x96
#define ACK200						0xC8
#define ACK210						0xD2
#define ACK211						0xD3
#define ACK220						0xDC
#define ACK221						0xDD

//the total check times is CHECK_NUM or CHECK_NUM+1
#define CHECK_NUM			4		
#define RETRY_SIZE			5
#define RETRY_CHECKSUM		1
#define RETRY_DATA			1

/**
 * Initializes Upgrade Fw by Uart module.
 */
void UpgradeUartInit(void);

/**
 * Exits Upgrade Fw by Uart module.
 */
void UpgradeUartExit(void);
#endif
/** @} */ // end of template upgarde_uart

/** @defgroup template_network Network
 *  @{
 */
/**
 * Initializes network module.
 */
void NetworkInit(void);
void NetworkSupriseRemove(void);
void NetworkSupriseInsert(void);

/**
 * Resets network module.
 */
void NetworkReset(void);

/**
 * Determines whether the network is ready or not.
 *
 * @return true for ready; false for net ready yet.
 */
bool NetworkIsReady(void);

/** @} */ // end of template_network


/** @defgroup template_storage Storage
 *  @{
 */

typedef enum
{
    STORAGE_NONE = -1,
    STORAGE_USB,
    STORAGE_SD,
    STORAGE_INTERNAL,

    STORAGE_MAX_COUNT
} StorageType;

typedef enum
{
    STORAGE_UNKNOWN,
    STORAGE_SD_INSERTED,
    STORAGE_SD_REMOVED,
    STORAGE_USB_INSERTED,
    STORAGE_USB_REMOVED,
    STORAGE_USB_DEVICE_INSERTED,
    STORAGE_USB_DEVICE_REMOVED
} StorageAction;

/**
 * Initializes storage module.
 */
void StorageInit(void);

StorageAction StorageCheck(void);
StorageType StorageGetCurrType(void);
void StorageSetCurrType(StorageType type);
char* StorageGetDrivePath(StorageType type);
bool StorageIsInUsbDeviceMode(void);

/** @} */ // end of template_storage

/** @defgroup template_upgrade Upgrade
 *  @{
 */
/**
 * Quit value definition.
 */
typedef enum
{
    QUIT_NONE,                  ///< Do not quit
    QUIT_DEFAULT,               ///< Quit for nothing
    QUIT_RESET_FACTORY,         ///< Quit to reset to factory setting
    QUIT_UPGRADE_FIRMWARE,      ///< Quit to upgrade firmware
    QUIT_UPGRADE_WEB,           ///< Quit to wait web upgrading
    QUIT_RESET_NETWORK,         ///< Quit to reset network
    QUIT_UPGRADE_UART			///< Quit to upgrade firmware by Uart
} QuitValue;

/**
 * Initializes upgrade module.
 *
 * @return 0 for initializing success, non-zero for initializing failed and the value will be the QuitValue.
 */
int UpgradeInit(void);

/**
 * Sets the CRC value of the specified file path.
 *
 * @param filepath The file path to update the CRC value.
 */
void UpgradeSetFileCrc(char* filepath);

/**
 * Sets the URL to upgrade.
 *
 * @param url The url to download and upgrade.
 */
void UpgradeSetUrl(char* url);

/**
 * Sets the stream to upgrade.
 *
 * @param stream The stream to upgrade.
 */
void UpgradeSetStream(void* stream);

/**
 * Processes the upgrade procedure by QuitValue.
 *
 * @param code The QuitValue.
 * @return 0 for process success; otherwise failed.
 */
int UpgradeProcess(int code);

/**
 * Is upgrading ready or not.
 *
 * @return true for ready; otherwise not ready yet.
 */
bool UpgradeIsReady(void);

/**
 * Is upgrading finished or not.
 *
 * @return true for finished; otherwise not finish yet.
 */
bool UpgradeIsFinished(void);

/**
 * Gets the upgrading result.
 *
 * @return 0 for success, failed otherwise.
 */
int UpgradeGetResult(void);

/** @} */ // end of template_upgrade

/** @defgroup template_webserver Web Server
 *  @{
 */
/**
 * Initializes web server module.
 */
void WebServerInit(void);

/**
 * Exits web server module.
 */
void WebServerExit(void);

/** @} */ // end of template_webserver

void PlayVideo(int x, int y, int width, int height, int bgColor, int volume);
void WaitPlayVideoFinish(void);

void PlayMjpeg(int x, int y, int width, int height, int bgColor, int volume);
void WaitPlayMjpegFinish(void);

#ifdef	CFG_DYNAMIC_LOAD_TP_MODULE
void DynamicLoadTpModule(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* PROJECT_H */
/** @} */ // end of template
