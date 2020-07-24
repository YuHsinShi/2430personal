/** @file
 * ITE DoorBell Indoor Modules.
 *
 * @author Jim Tan
 * @version 1.0
 * @date 2013
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
/** @defgroup doorbell_indoor ITE DoorBell Indoor Modules
 *  @{
 */
#ifndef DOORBELL_H
#define DOORBELL_H

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup doorbell_indoor_deviceinfo Device Infomation
 *  @{
 */
/**
 * Device type definition.
 */
typedef enum
{
    DEVICE_CENTER,      ///< Control Server
    DEVICE_OUTDOOR,     ///< Door Camera
    DEVICE_ENTRANCE,    ///< Lobby Phone
    DEVICE_BUILDING,    ///< Building Phone
    DEVICE_WALL,        ///< Wall Phone
    DEVICE_INDOOR,      ///< Indoor Phone
    DEVICE_MANAGER,     ///< Administrator Unit
    DEVICE_INDOOR2,     ///< Indoor Phone with internal SD storage
    DEVICE_MOBILE,      ///< Mobile Phone
    DEVICE_INTERCOM,    ///< Intercom
    DEVICE_IPCAM,       ///< IP Camera
    DEVICE_UNKNOWN      ///< Unknown Device
} DeviceType;

/**
 * Device information definition.
 */
typedef struct
{
    DeviceType type;    ///< Device type.
    char area[4];       ///< Area code of address, 2 characters.
    char building[4];   ///< Building code of address, 2 characters.
    char unit[4];       ///< Unit code of address, 2 characters.
    char floor[4];      ///< Floor code of address, 2 characters.
    char room[4];       ///< Room code of address, 2 characters.
    char ext[4];        ///< Extension code of address, 2 characters.
    char alias[128];    ///< Alias name written in XML of address book.
    char ip[16];        ///< IP address.
    char id[16];        ///< Ipcam id
    char pw[16];        /// <Ipcam password
} DeviceInfo;

extern DeviceInfo theCenterInfo;    ///< Device information of control server.
extern DeviceInfo theDeviceInfo;    ///< Device information of this device.
extern DeviceInfo theEntranceInfo;  ///< Device information of lobby phone.
extern DeviceInfo theOutdoorInfo;   ///< Device information of door camera.

/**
 * Initializes device information module.
 */
void DeviceInfoInit(void);

/**
 * Initializes device information module by DHCP setting.
 *
 * @param ip The IP address gotten from DHCP server.
 */
void DeviceInfoInitByDhcp(char* ip);

/**
 * Determines whether this device is in engineer mode or not.
 *
 * @return true in engineer mode, false in normal mode.
 */
bool DeviceInfoIsInEngineerMode(void);

/** @} */ // end of doorbell_indoor_deviceinfo

/** @defgroup doorbell_indoor_addressbook Address Book
 *  @{
 */
/**
 * Maximum Door Camera count.
 */
#define MAX_OUTDOOR_COUNT 10

/**
 * Maximum Administrator Unit count.
 */
#define MAX_MANAGER_COUNT 10

/**
 * Maximum camera device count.
 */
#define MAX_CAMERA_COUNT 99

/**
 * Maximum ip camera count.
 */
#define MAX_IPCAMERA_COUNT 9

/**
 * Remove old AddressBook.
 */
void RemoveAddressbook(void);

/**
 * Initializes address book module.
 */
void AddressBookInit(void);

/**
 * Exits address book module.
 */
void AddressBookExit(void);

/**
 * Gets version of addressbook.
 *
 * @return the version string. Remember to free it after used to avoid memory leak.
 */
char* AddressBookGetVersion(void);

/**
 * Gets device information of control server from addressbook.
 *
 * @param info The pointer to retrieve the DeviceInfo of control server.
 */
void AddressBookGetCenterInfo(DeviceInfo* info);

/**
 * Gets device information from addressbook by IP address.
 *
 * @param info The pointer to retrieve the DeviceInfo.
 * @param ip The IP address of device.
 */
void AddressBookGetDeviceInfo(DeviceInfo* info, char* ip);

/**
 * Gets device information of lobby phone from addressbook.
 *
 * @param info The pointer to retrieve the DeviceInfo of lobby phone.
 * @param deviceInfo The DeviceInfo of indoor phone which belong to the lobby phone.
 */
void AddressBookGetEntranceInfo(DeviceInfo* info, DeviceInfo* deviceInfo);

/**
 * Gets device information of door camera from addressbook.
 *
 * @param info The pointer to retrieve the DeviceInfo of door camera.
 * @param deviceInfo The DeviceInfo of indoor phone which the door camera belong to.
 */
void AddressBookGetOutdoorInfo(DeviceInfo* info, DeviceInfo* deviceInfo);

/**
 * Gets IP address by address information from addressbook.
 *
 * @param area The area code of address.
 * @param building The building code of address.
 * @param unit The unit code of address.
 * @param floor The floor code of address.
 * @param room The room code of address.
 * @param ext The ext code of address. Can be NULL if not exist.
 * @return the IP address string. Remember to free it after used to avoid memory leak.
 */
char* AddressBookGetDeviceIP(char* area, char* building, char* unit, char* floor, char* room, char* ext);

/**
 * Gets array of indoor phones by address information from addressbook.
 *
 * @param infoArray The array of DeviceInfo to fill in.
 * @param count Maximum count of DeviceInfo in array.
 * @param area The area code of address.
 * @param building The building code of address.
 * @param unit The unit code of address.
 * @param floor The floor code of address.
 * @param room The room code of address.
 * @return the actual count of filled DeviceInfo in array.
 */
int AddressBookGetDeviceInfoArray(DeviceInfo infoArray[], int count, char* area, char* building, char* unit, char* floor, char* room);

/**
 * Gets array of devices with camera from addressbook.
 *
 * @param infoArray The array of DeviceInfo to fill in.
 * @param count Maximum count of DeviceInfo in array.
 * @param info The pointer to the DeviceInfo of this device.
 * @return the actual count of filled DeviceInfo in array.
 */
int AddressBookGetCameraInfoArray(DeviceInfo infoArray[], int count, DeviceInfo* devinfo);

/**
 * Gets array of lobby devices from addressbook.
 *
 * @param infoArray The array of DeviceInfo to fill in.
 * @param count Maximum count of DeviceInfo in array.
 * @return the actual count of filled DeviceInfo in array.
 */
int AddressBookGetLobbyInfoArray(DeviceInfo infoArray[], int count);

/**
 * Gets array of administrator units from addressbook.
 *
 * @param infoArray The array of DeviceInfo to fill in.
 * @param count Maximum count of DeviceInfo in array.
 * @return the actual count of filled DeviceInfo in array.
 */
int AddressBookGetManagerInfoArray(DeviceInfo infoArray[], int count);

/**
 * Gets array of ip camera from addressbook.
 *
 * @param infoArray The array of DeviceInfo to fill in.
 * @param count Maximum count of DeviceInfo in array.
 * @return the actual count of filled DeviceInfo in array.
 */
int AddressBookGetIPCameraInfoArray(DeviceInfo infoArray[], int count);

/**
 * Gets IP address of gateway from addressbook.
 *
 * @param info The pointer to the DeviceInfo of this device.
 * @return the IP address string of gateway. Remember to free it after used to avoid memory leak.
 */
char* AddressBookGetGateway(DeviceInfo* info);

/**
 * Gets netmask from addressbook.
 *
 * @param info The pointer to the DeviceInfo of this device.
 * @return the netmask string. Remember to free it after used to avoid memory leak.
 */
char* AddressBookGetNetMask(DeviceInfo* info);

/**
 * Is the outdoor address exist in the addressbook.
 *
 * @param area The area code of address.
 * @param building The building code of address.
 * @param unit The unit code of address.
 * @param floor The floor code of address.
 * @param room The room code of address.
 * @param ext The ext code of address.
 * @return true if exist, false otherwise.
 */
bool AddressBookIsOutdoorExist(char* area, char* building, char* unit, char* floor, char* room, char* ext);

/**
 * Gets array of door cameras by address information from addressbook.
 *
 * @param infoArray The array of DeviceInfo to fill in.
 * @param count Maximum count of DeviceInfo in array.
 * @param area The area code of address.
 * @param building The building code of address.
 * @param unit The unit code of address.
 * @param floor The floor code of address.
 * @param room The room code of address.
 * @return the actual count of filled DeviceInfo in array.
 */
int AddressBookGetOutdoorInfoArray(DeviceInfo infoArray[], int count, char* area, char* building, char* unit, char* floor, char* room);

/**
 * Exports address book to USB disk or SD card.
 */
void AddressBookExport(void);

/** @} */ // end of doorbell_indoor_addressbook

/** @defgroup doorbell_indoor_audio Audio Player
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
int AudioGetVolume(void);

/** @} */ // end of doorbell_indoor_audio

/** @defgroup doorbell_indoor_audiorecord Audio Recorder
 *  @{
 */
/**
 * Initializes audio recorder module.
 */
void AudioRecordInit(void);

/**
 * Exits audio recorder module.
 */
void AudioRecordExit(void);

/**
 * Start recording to the specified file.
 *
 * @param filepath The specified file to record.
 * @return 0 for success, otherwise failed.
 */
int AudioRecordStartRecord(char* filepath);

/**
 * Stop recording.
 *
 * @return 0 for success, otherwise failed.
 */
int AudioRecordStopRecord(void);

/**
 * Gets the time length of the specified recorded file, in seconds.
 *
 * @param filepath The specified file path.
 * @return The time length, in seconds.
 */
int AudioRecordGetTimeLength(char* filepath);

/**
 * Plays the specified recorded file.
 *
 * @param filepath The specified recorded file to play.
 * @return 0 for success, otherwise failed.
 */
int AudioRecordStartPlay(char* filepath);

/**
 * Stops playing recorded file.
 */
void AudioRecordStopPlay(void);

/** @} */ // end of doorbell_indoor_audiorecord

/**
 * Stops audio player.
 */
void AudioPlayerStop(void);

/** @defgroup doorbell_indoor_calllog Call Logs
 *  @{
 */

/**
 * Call log type definition.
 */
typedef enum
{
    CALLLOG_UNKNOWN,   ///< Unknown call
    CALLLOG_MISSED,    ///< Missed call
    CALLLOG_RECEIVED,  ///< Received call
    CALLLOG_SENT       ///< Sent call
} CallLogType;

/**
 * Calling log definition
 */
typedef struct
{
    CallLogType type;   ///< Call log type
    char ip[16];        ///< IP address of target
    char time[32];      ///< Calling time
    int duration;       ///< Calling duration, in seconds.
    int refkey;         ///< Index number as reference key
} CallLogEntry;

/**
 * Initializes call log module.
 */
void CallLogInit(void);

/**
 * Exits call log module.
 */
void CallLogExit(void);

/**
 * Gets total count of call logs.
 *
 * @return the total count of call logs.
 */
int CallLogGetCount(void);

/**
 * Gets the call log entry by index.
 *
 * @param index The index of call log entry.
 * @return The call log entry, NULL if not found.
 */
CallLogEntry* CallLogGetEntry(int index);

/**
 * Deletes the call log entry by index.
 *
 * @param index The index of call log entry.
 */
void CallLogDeleteEntry(int index);

/**
 * Clears the call log entries.
 */
void CallLogClearEntries(void);
/**
 * Gets the count of current missed calls.
 *
 * @return The call log entry, NULL if not found.
 */
int CallLogGetCurrMissedCount(void);

/**
 * Clears the count of current missed calls.
 */
void CallLogClearCurrMissedCount(void);

/** @} */ // end of doorbell_indoor_calllog

/** @defgroup doorbell_indoor_capture Capture
 *  @{
 */

#define CAPUTRE_MAX_FILENAME_SIZE  40  ///< Maximum size of captured file name.
#define CAPUTRE_MAX_NAME_SIZE      40  ///< Maximum count of capture item name.

/**
 * Capture media definitions.
 */
typedef enum
{
    MEDIA_TEXT,     ///< Text media type.
    MEDIA_IMAGE,    ///< Image media type.
    MEDIA_VIDEO,    ///< Video media type.
    MEDIA_AUDIO     ///< Audio media type.
} MediaType;

/**
 * Captured file entry.
 */
typedef struct
{
    MediaType type; ///< The media type.
    char filename[CAPUTRE_MAX_FILENAME_SIZE];   ///< The captured file name.
    char name[CAPUTRE_MAX_NAME_SIZE];           ///< The name show on the UI.
    time_t time;    ///< The captured time.
    int sec;        ///< The time length, in seconds.
    bool played;    ///< The file is played or not.
} CaptureEntry;

/**
 * Initializes capture module.
 */
void CaptureInit(void);

/**
 * Exits capture module.
 */
void CaptureExit(void);

/**
 * Gets total count of captured file.
 *
 * @return the total count of captured file.
 */
int CaptureGetCount(void);

/**
 * Gets unplay count of captured file.
 *
 * @return unplay count of captured file.
 */
int CaptureGetUnplayCount(void);

/**
 * Gets the captured entry by index.
 *
 * @param index The index of captured entry.
 * @return The captured entry, NULL if not found.
 */
CaptureEntry* CaptureGetEntry(int index);

/**
 * Deletes the captured entry by index.
 *
 * @param index The index of captured entry.
 */
void CaptureDeleteEntry(int index);

/**
 * Deletes the Snapshotcaptured entry by index.
 *
 * @param index The index of captured entry.
 */
void CaptureSnapshotDeleteEntry(int index);

/**
 * Playing the captured file is finished or not.
 *
 * @return true for finished, false for otherwise.
 */
bool CaptureIsPlayFinished(void);

/**
 * Plays the captured file. Only for MEDIA_VIDEO and MEDIA_AUDIO types.
 *
 * @param index The index of captured file to play.
 * @return 0 for success, otherwise failed.
 */
int CaptureStartPlay(int index);

/**
 * Stops playing captured file.
 */
void CaptureStopPlay(void);

/**
 * Is recording or not.
 *
 * @return true if recording, false otherwise.
 */
bool CaptureIsRecording(void);

bool IPCamIsRecording(void);
int IPCamStartStreaming(void);
int IPCamStopStreaming(void);
int IPCamStartRecord(MediaType type, char* addr);
int IPCamStopRecord(void);
int IPCamSnapshot(char* addr, time_t t);

/**
 * Starts capturing.
 *
 * @param type The media type. Only for MEDIA_VIDEO and MEDIA_AUDIO types.
 * @param addr The IP address of target to capture.
 * @return 0 for success, otherwise failed.
 */
int CaptureStartRecord(MediaType type, char* addr);

/**
 * Stops capturing.
 *
 * @return 0 for success, otherwise failed.
 */
int CaptureStopRecord(void);

/**
 * start to Snapshots current image.
 *
 * @param filepath The file path to save the snapshot.
 * @return 0 for success, otherwise failed.
 */

int CaptureSnapshotStart(char* filepath);

/**
 * Snapshots current image.
 *
 * @param addr The IP address of target to snapshot.
 * @param t The snapshot time. 0 for current time.
 * @return 0 for success, otherwise failed.
 */
int CaptureSnapshot(char* addr, time_t t);

/**
 * Gets the file name of snapshot by time.
 *
 * @param time The snapshot time. The format is like 01/01/2014 12:00:00
 * @return The file name.
 */
char* CaptureGetSnapshotFileNameByTime(char* time);

/**
 * Gets the path of capture directory.
 *
 * @return The path.
 */
char* CaptureGetPath(void);

/**
 * Checks the disk space is availabe to Snapshot current image.
 *
 * @return true for success, otherwise failed.
 */
bool CaptureCheckDiskSpace(int max_file_size, MediaType media_type);

/** @} */ // end of doorbell_indoor_capture

/** @defgroup doorbell_indoor_cardlist Card List
 *  @{
 */
/**
 * Initializes card list module.
 */
void CardListInit(void);

/**
 * Exits card list module.
 */
void CardListExit(void);

/**
 * Gets version of card list.
 *
 * @return the version integer.
 */
int CardListGetVersion(void);

/**
 * Reloads the card list.
 */
void CardListReload(void);

/** @} */ // end of doorbell_indoor_cardlist

/** @defgroup doorbell_indoor_house House
 *  @{
 */
#define HOUSE_MAX_AIRCONDITIONER_COUNT  8  ///< Maximum count of air conditioner.
#define HOUSE_MAX_CURTAIN_COUNT         8  ///< Maximum count of curtain.
#define HOUSE_MAX_LIGHT_COUNT           8  ///< Maximum count of light.
#define HOUSE_MAX_PLUG_COUNT            8  ///< Maximum count of plug.
#define HOUSE_MAX_TV_COUNT              8  ///< Maximum count of TV.

/**
 * House mode definition.
 */
typedef enum
{
    HOUSE_NONE,     ///< Non-setting mode.
    HOUSE_INDOOR,   ///< Indoor mode.
    HOUSE_OUTDOOR,  ///< Outdoor mode.
    HOUSE_SLEEP     ///< Sleep mode.
} HouseMode;

/**
 * House device definition.
 */
typedef enum
{
    HOUSE_AIRCONDITIONER,   ///< Air Conditioner.
    HOUSE_CURTAIN,          ///< Curtain.
    HOUSE_LIGHT,            ///< Light.
    HOUSE_PLUG,             ///< Plug.
    HOUSE_TV,               ///< TV.

    HOUSE_DEVICE_COUNT      ///< Total type count of house devices.
} HouseDevice;

/**
 * Action commands for house devices.
 */
typedef enum
{
    HOUSE_OPEN,     ///< Open/Turn on command.
    HOUSE_CLOSE,    ///< Close/Turn off command.
    HOUSE_STOP,     ///< Stop command.
    HOUSE_GET,      ///< Get information command.
    HOUSE_SET       ///< Set information command.
} HouseAction;

/**
 * Status definition of house device.
 */
typedef enum
{
    HOUSE_OFF,  ///< The device is off/closed.
    HOUSE_ON    ///< The device is on/opened.
} HouseStatus;

/**
 * House device information definition.
 */
typedef struct
{
    HouseDevice device; ///< House device.
    char name[32];      ///< Dvice name.
    HouseStatus status; ///< Dvice status.
    int index;          ///< Dvice index.
} HouseInfo;

/**
 * Air conditioner mode definition.
 */
typedef enum
{
    HOUSE_AC_AUTO,  ///< Auto mode
    HOUSE_AC_COOL,  ///< Cool mode
    HOUSE_AC_WARM,  ///< Warm mode
    HOUSE_AC_FAN,   ///< Fan mode
    HOUSE_AC_DRY    ///< Dry mode
} HouseAirConditionerMode;

/**
 * Air conditioner setting definition.
 */
typedef struct
{
    HouseAirConditionerMode mode;   ///< The mode setting.
    int wind;                       ///< The wind setting.
    float temperature;              ///< The temperature setting.
} HouseAirConditionerSetting;

/**
 * The function definition of initializing house module.
 */
typedef void (*HouseInitFunc)(void);

/**
 * The function definition of exiting house module.
 */
typedef void (*HouseExitFunc)(void);

/**
 * The function definition of getting information array of house module.
 *
 * @param infoArray The pointer to retrieve the array of HouseInfo of module instance.
 * @param count Maximum count of HouseInfo in array.
 */
typedef int (*HouseGetInfoArrayFunc)(HouseInfo infoArray[], int count);

/**
 * The function definition of executing action command on house module.
 *
 * @param index The index of module instance to execute.
 * @param action The action command to execute.
 * @param data Additional data. NULL if no data.
 * @return The result value depends on the action command.
 */
typedef int (*HouseExecActionFunc)(int index, HouseAction action, void* data);

/**
 * House entry definition.
 */
typedef struct
{
    HouseInitFunc           Init;           ///< Initialize function.
    HouseExitFunc           Exit;           ///< Exit function.
    HouseGetInfoArrayFunc   GetInfoArray;   ///< GetInfoArray function.
    HouseExecActionFunc     ExecAction;     ///< ExecAction function.
} HouseEntry;

/**
 * Initializes house module.
 */
void HouseInit(void);

/**
 * Exits house module.
 */
void HouseExit(void);

/**
 * Sets house mode.
 *
 * @param mode The mode to set.
 */
void HouseSetMode(HouseMode mode);

/**
 * Gets array of HouseInfo by house device type.
 *
 * @param dev The house device type.
 * @param infoArray The array of HouseInfo to fill in.
 * @param count Maximum count of HouseInfo in array.
 * @return The actual count of filled HouseInfo in array.
 */
int HouseGetInfoArray(HouseDevice dev, HouseInfo infoArray[], int count);

/**
 * Executes action command on specified house module.
 *
 * @param dev The house device type.
 * @param index The index of module instance to execute.
 * @param action The action command to execute.
 * @param data Additional data. NULL if no data.
 * @return The result value depends on the action command.
 */
int HouseExecAction(HouseDevice dev, int index, HouseAction action, void* data);

/** @} */ // end of doorbell_indoor_house

/** @defgroup doorbell_indoor_config Configuration
 *  @{
 */
/**
 * Do not disturb mode definition.
 */
typedef enum
{
    DONOT_DISTURB_NONE, ///< Do nothing
    DONOT_DISTURB_MUTE, ///< Mute
    DONOT_DISTURB_ALL,  ///< All time
    DONOT_DISTURB_TIME  ///< Time duration
} DoNotDisturbMode;

/**
 * Do not disturb options definition.
 */
typedef enum
{
    DONOT_DISTURB_HANGUP,       ///< Hang up
    DONOT_DISTURB_VOICEMEMO,    ///< Voice memo
    DONOT_DISTURB_MESSAGE,      ///< Send message to control server
    DONOT_DISTURB_REDIRECT      ///< Redirect to control server
} DoNotDisturbOption;

/**
 * Language type definition.
 */
typedef enum
{
    LANG_ENG,   ///< English
    LANG_CHT,   ///< Traditional Chinese
    LANG_CHS    ///< Simplified Chinese
} LangType;

/**
 * Configuration definition.
 */
typedef struct
{
    // network
    int dhcp;                                                           ///< Enable DHCP or not
    char ipaddr[16];                                                    ///< IP address
    char netmask[16];                                                   ///< Netmask
    char gw[16];                                                        ///< Gateway address
    char dns[16];                                                       ///< DNS address

	// network multi interface
	char ipaddr_0[16];                                                  ///< Netif_0 IP address
    char netmask_0[16];                                                 ///< Netif_0 Netmask
    char gw_0[16];                                                      ///< Netif_0 Gateway address

	// dhcp setting
	char default_ipaddr[16];
	char dhcp_ipaddr[16];
	bool dhcp_ipaddr_writen;


    // display
    int brightness;                                                     ///< Brightness, the range is 0~9
    int screensaver_time;                                               ///< Time to enter screen saver mode, unit is minute
    int screensaver_type;                                               ///< Screen saver type @see ScreensaverType
    int lang;                                                           ///< Language type @see LangType

    // phone
    char ringsound[PATH_MAX];                                           ///< Ring sound file path
    char keysound[PATH_MAX];                                            ///< Key sound file path
    int keylevel;                                                       ///< Key volume percentage, range is 0~100

    // guard
    int guard_mode;                                                     ///< Guard mode @see GuardMode
    char guard_password[8];                                             ///< Guard password
    char guard_sensor_initvalues[16];                                   ///< Intiial values of guard sensors
    char guard_indoor_sensors[16];                                      ///< Enabled sensors of guard indoor mode
    char guard_outdoor_sensors[16];                                     ///< Enabled sensors of guard outdoor mode
    char guard_sleep_sensors[16];                                       ///< Enabled sensors of guard sleep mode
    int warnlevel;                                                      ///< Warn volume percentage, range is 0~100

    // house
    int house_mode;                                                         ///< House mode @see HouseMode
    char house_indoor_airconditioners[HOUSE_MAX_AIRCONDITIONER_COUNT + 4];  ///< Enabled air conditioners of house indoor mode
    char house_indoor_curtains[HOUSE_MAX_CURTAIN_COUNT + 4];                ///< Enabled curtains of house indoor mode
    char house_indoor_lights[HOUSE_MAX_LIGHT_COUNT + 4];                    ///< Enabled lights of house indoor mode
    char house_indoor_plugs[HOUSE_MAX_PLUG_COUNT + 4];                      ///< Enabled plugs of house indoor mode
    char house_indoor_tvs[HOUSE_MAX_TV_COUNT + 4];                          ///< Enabled TVs of house indoor mode

    char house_outdoor_airconditioners[HOUSE_MAX_AIRCONDITIONER_COUNT + 4]; ///< Enabled air conditioners of house outdoor mode
    char house_outdoor_curtains[HOUSE_MAX_CURTAIN_COUNT + 4];               ///< Enabled curtains of house outdoor mode
    char house_outdoor_lights[HOUSE_MAX_LIGHT_COUNT + 4];                   ///< Enabled lights of house outdoor mode
    char house_outdoor_plugs[HOUSE_MAX_PLUG_COUNT + 4];                     ///< Enabled plugs of house outdoor mode
    char house_outdoor_tvs[HOUSE_MAX_TV_COUNT + 4];                         ///< Enabled TVs of house outdoor mode

    char house_sleep_airconditioners[HOUSE_MAX_AIRCONDITIONER_COUNT + 4];   ///< Enabled air conditioners of house sleep mode
    char house_sleep_curtains[HOUSE_MAX_CURTAIN_COUNT + 4];                 ///< Enabled curtains of house sleep mode
    char house_sleep_lights[HOUSE_MAX_LIGHT_COUNT + 4];                     ///< Enabled lights of house sleep mode
    char house_sleep_plugs[HOUSE_MAX_PLUG_COUNT + 4];                       ///< Enabled plugs of house sleep mode
    char house_sleep_tvs[HOUSE_MAX_TV_COUNT + 4];                           ///< Enabled TVs of house sleep mode

    int airconditioner_celsius;                                         ///< Air conditioner temperature unit is celsius or not

    // setting
    char engineer_password[8];                                          ///< Password of entering engineer mode
    char area[4];                                                       ///< Area code of address of this device
    char building[4];                                                   ///< Building code of address of this device
    char unit[4];                                                       ///< Unit code of address of this device
    char floor[4];                                                      ///< Floor code of address of this device
    char room[4];                                                       ///< Room code of address of this device
    char ext[4];                                                        ///< Extension code of address of this device
    char outdoor_ipaddr[16];                                            ///< IP address of door camera device
    int dial_time;                                                      ///< Maximum dialing time (sec)
    int calling_time;                                                   ///< Maximum calling time (sec)
    int missed_call_count;                                              ///< Missed call count
    int muted;                                                          ///< Audio muted or not
    int touch_calibration;                                              ///< Need to do the touch calibration or not
    int do_not_disturb_mode;                                            ///< House mode @see DoNotDisturbMode
    int do_not_disturb_option;                                          ///< Option of do not disturb mode @see DoNotDisturbOption
    int do_not_disturb_start_hour;                                      ///< Start hour to enter do not disturb mode
    int do_not_disturb_start_min;                                       ///< Start minute to enter do not disturb mode
    int do_not_disturb_end_hour;                                        ///< End hour to exit do not disturb mode
    int do_not_disturb_end_min;                                         ///< End minute to exit do not disturb mode
    int bgcolor_red;                                                    ///< Red color value of background
    int bgcolor_green;                                                  ///< Green color value of background
    int bgcolor_blue;                                                   ///< Blue color value of background
    char wallpaper[PATH_MAX];                                           ///< Wallpaper file path
    int audiolevel;                                                     ///< Audio volume percentage, range is 0~100

    // mobile
    char mobile_user0[64];
    char mobile_user1[64];
    char mobile_user2[64];
    char mobile_user3[64];
    char mobile_user4[64];

    // wifi
    int wifi_mode;
    int wifi_on_off;
    char ssid[64];
    char password[256];
    char secumode[3];

	// login
	char user_id[64];
	char user_password[64];
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

/**
 * Saves the private parts of configuration to file.
 */
void ConfigSavePrivate(void);

/**
 * Saves the temp parts of configuration to file.
 */
void ConfigSaveTemp(void);

/** @} */ // end of doorbell_indoor_config

/** @defgroup doorbell_indoor_guard Guard
 *  @{
 */
/**
 * Guard mode definition.
 */
typedef enum
{
    GUARD_NONE,     ///< Disabling guard mode
    GUARD_INDOOR,   ///< Indoor guard mode
    GUARD_OUTDOOR,  ///< Outdoor guard mode
    GUARD_SLEEP     ///< Sleep guard mode
} GuardMode;

/**
 * Guard sensors definition.
 */
typedef enum
{
    GUARD_EMERGENCY,    ///< Emergency sensor
    GUARD_INFRARED,     ///< Infrared sensor
    GUARD_DOOR,         ///< Door sensor
    GUARD_WINDOW,       ///< Window sensor
    GUARD_SMOKE,        ///< Smoke sensor
    GUARD_GAS,          ///< Gas sensor
    GUARD_AREA,         ///< Area sensor
    GUARD_ROB,          ///< Rob sensor

    GUARD_SENSOR_COUNT  ///< Total sensor count
} GuardSensor;

/**
 * Guard status definition.
 */
typedef enum
{
    GUARD_ENABLE,   ///< Guard is enabled
    GUARD_DISABLE,  ///< Guard is disabled
    GUARD_ALARM,    ///< Guard is alarming
    GUARD_UNALARM   ///< Guard is un-alarmed
} GuardStatus;

/**
 * Guard type definition.
 */
typedef enum
{
    GUARD_TYPE_EMERGENCY,    ///< Emergency alarm
    GUARD_TYPE_INFRARED,     ///< Infrared alarm
    GUARD_TYPE_DOOR,         ///< Door alarm
    GUARD_TYPE_WINDOW,       ///< Window alarm
    GUARD_TYPE_SMOKE,        ///< Smoke alarm
    GUARD_TYPE_GAS,          ///< Gas alarm
    GUARD_TYPE_AREA,         ///< Area alarm
    GUARD_TYPE_ROB,          ///< Rob alarm
    GUARD_TYPE_OPENDOOR,     ///< Open door alarm
} GuardType;

/**
 * Guard state definition.
 */
typedef enum
{
    GUARD_UNHANDLED,    ///< Guard event is unhandled
    GUARD_HANDLED       ///< Guard event is handled
} GuardState;

/**
 * Guard logging definition.
 */
typedef struct
{
    unsigned long timestamp;    ///< Time stamp of logging
    GuardSensor sensor;         ///< Guard sensor of logging
    GuardStatus status;         ///< Guard status of logging
    GuardType type;             ///< Guard type of logging
    GuardState state;           ///< Guard event state
} GuardLog;

/**
 * Initializes guard module.
 */
void GuardInit(void);

/**
 * Exits guard module.
 */
void GuardExit(void);

/**
 * Sets guard mode.
 *
 * @param mode The mode to set.
 */
void GuardSetMode(GuardMode mode);

/**
 * Checks guard events.
 *
 * @return 0 for no event; 1 for events need to be silent; -1 for events need to be alarming.
 */
int GuardCheck(void);

/**
 * Gets the status of the specified sensor.
 *
 * @param sensor The sensor to get status.
 * @return The status of sensor.
 */
GuardStatus GuardGetSensorStatus(GuardSensor sensor);

/**
 * Gets the last active time of the specified sensor.
 *
 * @param sensor The sensor to get time.
 * @return The timestamp of sensor.
 */
unsigned long GuardGetSensorTime(GuardSensor sensor);

/**
 * Gets total count of guard logging.
 *
 * @return The total count.
 */
int GuardGetLogCount(void);

/**
 * Gets the guard logging entry by the specified index.
 *
 * @param index The index to get logging entry.
 * @return The guard logging entry.
 */
GuardLog* GuardGetLog(int index);

/**
 * Deletes the guard logging entry by the specified index.
 *
 * @param index The index to delete logging entry.
 */
void GuardDeleteLog(int index);

/**
 * Clears all the guard logging entries.
 */
void GuardClearLog(void);

/**
 * Guard is alarming or not.
 *
 * @return The guard is alarming or not.
 */
bool GuardIsAlarming(void);

/**
 * Sets the guard alarming status.
 *
 * @param alarming The alarming status.
 */
void GuardSetAlarming(bool alarming);

/**
 * Receives the XML of guard sync log.
 *
 * @param xml The guard log XML to parse.
 * @param size The size of XML string.
 */
void GuardReceiveSyncLog(char* xml, int size);

/** @} */ // end of doorbell_indoor_guard

/** @defgroup doorbell_indoor_event Event
 *  @{
 */
/**
 * Calling out actions of uploading event definitions.
 */
typedef enum
{
    EVENT_CALLOUT_START,    ///< Calling out starting
    EVENT_CALLOUT_END,      ///< Calling out ended
    EVENT_CALLOUT_NOACK     ///< No response of calling out
} EventCalloutAction;

/**
 * Opening door mode of uploading event definitions.
 */
typedef enum
{
    EVENT_OPENDOOR_REMOTE,      ///< Opening door by remote device
    EVENT_OPENDOOR_PASSWORD,    ///< Opening door by password
    EVENT_OPENDOOR_CARD         ///< Opening door by card
} EventOpenDoorMode;

/**
 * Initializes event module.
 */
void EventInit(void);

/**
 * Exits event module.
 */
void EventExit(void);

/**
 * Writes calling out log to upload.
 *
 * @param ip The IP address of target to call out.
 * @param action The action occured.
 */
void EventWriteCalloutLog(char* ip, EventCalloutAction action);

/**
 * Writes guard log to upload.
 *
 * @param sensor The trigged sensor.
 * @param status The guard status.
 * @param timestamp The trigged time-stamp.
 */
void EventWriteGuardLog(GuardSensor sensor, GuardStatus status, time_t timestamp);

/**
 * Writes registering device event to upload.
 *
 * @param hardwareAddress The MAC address of this device to upload.
 */
void EventWriteRegisterDevice(uint8_t hardwareAddress[]);

/**
 * Writes opening door event to upload.
 *
 * @param mode The mode of open door.
 * @param info The device information. Can be NULL.
 * @param verified Success to open door or not.
 * @param card Card number if opening door by card. Can be NULL.
 */
void EventWriteOpenDoorLog(EventOpenDoorMode mode, DeviceInfo* info, bool verified, char* card);

/**
 * Writes SMS event to upload.
 *
 * @param ip The IP address of target which need to send ths SMS message.
 */
void EventWriteSmsLog(char* ip);

/** @} */ // end of doorbell_indoor_event

/** @defgroup doorbell_indoor_linphone Linphone
 *  @{
 */
/**
 * Initializes linphone module.
 */
void LinphoneInit(void);

/**
 * Exits linphone module.
 */
void LinphoneExit(void);

/**
 * Gets the percentage of voice volume.
 *
 * @return The percentage of voice volume, range is 0~100.
 */
int LinphoneGetVoiceLevel(void);

/**
 * Sets the percentage of voice volume.
 *
 * @param level The percentage of voice volume, range is 0~100.
 */
void LinphoneSetVoiceLevel(int level);

/**
 * Gets the percentage of MIC volume.
 *
 * @return The percentage of MIC volume, range is 0~100.
 */
int LinphoneGetMicLevel(void);

/**
 * Sets the percentage of MIC volume.
 *
 * @param level The percentage of MIC volume, range is 0~100.
 */
void LinphoneSetMicLevel(int level);

/**
 * Plays ring sound.
 */
void LinphonePlayRingSound(int arg);

/**
 * Sets the file path of ring sound. Must be WAV file.
 *
 * @param file the file path of ring sound.
 */
void LinphoneSetRingSound(char* file);

/**
 * Gets the percentage of ring volume.
 *
 * @return The percentage of ring volume, range is 0~100.
 */
int LinphoneGetRingLevel(void);

/**
 * Sets the percentage of ring volume.
 *
 * @param level The percentage of ring volume, range is 0~100.
 * @param active Active or not.
 */
void LinphoneSetRingLevel(int level, bool active);

/**
 * Plays key sound.
 */
void LinphonePlayKeySound(void);

void LinphonePauseKeySound(void);
void LinphoneResumeKeySound(void);

/**
 * Sets the percentage of key volume.
 *
 * @param level The percentage of key volume, range is 0~100.
 */
void LinphoneSetKeyLevel(int level);

/**
 * Gets the percentage of warn volume.
 *
 * @return The percentage of warn volume, range is 0~100.
 */
int LinphoneGetWarnLevel(void);

/**
 * Sets the percentage of warn volume.
 *
 * @param level The percentage of warn volume, range is 0~100.
 */
void LinphoneSetWarnLevel(int level);

/**
 * Plays warning sound.
 */
void LinphonePlayWarnSound(void);

/**
 * Makes a call to the specified IP address.
 *
 * @param ip The specified IP address to call.
 * @param video Enables video feature.
 * @param earlyMedia Enables early media feature.
 */
void LinphoneCall(char* ip, bool video, bool earlyMedia);

/**
 * Watches camera of the specified IP address.
 *
 * @param ip The specified IP address to watch.
 */
void LinphoneWatchCamera(char* ip);

/**
 * Stop watching camera.
 */
void LinphoneStopCamera(void);

/**
 * Answers a call for the specified call ID.
 *
 * @param id The call ID to answer.
 */
void LinphoneAnswer(int id);

/**
 * Mutes microphone.
 */
void LinphoneMuteMic(void);

/**
 * Un-mutes microphone.
 */
void LinphoneUnMuteMic(void);

/**
 * Terminates the specified call ID.
 *
 * @param id The call ID to terminate.
 */
void LinphoneTerminate(int id);

/**
 * Adds a contact.
 *
 * @param info The device information of the contact.
 * @param blacklist Adds the contact to black list or not.
 * @param comment The comment of the contact.
 */
void LinphoneAddFriend(DeviceInfo* info, bool blacklist, char* comment);

/**
 * Updates a contact.
 *
 * @param addr The address of the contact to update.
 * @param blacklist Updates the contact to black list or not.
 * @param comment The comment of the contact.
 */
void LinphoneUpdateFriend(char* addr, bool blacklist, char* comment);

/**
 * Deletes a contact.
 *
 * @param addr The address of the contact to delete.
 */
void LinphoneDeleteFriend(char* addr);

/**
 * Clears all contacts.
 */
void LinphoneClearFriends(void);

/**
 * Clears all missed logs.
 */
void LinphoneClearMissedLogs(void);

/**
 * Clears all received logs.
 */
void LinphoneClearReceivedLogs(void);

/**
 * Clears all sent logs.
 */
void LinphoneClearSentLogs(void);

/**
 * Deletes a calling log.
 *
 * @param refkey The reference key of the calling log to delete.
 */
void LinphoneDeleteCallLog(int refkey);

/**
 * Sets the window ID to display video.
 *
 * @param widget The widget pointer to display video.
 */
void LinphoneSetWindowID(void* widget);

/**
 * Plays the video message on the control server.
 *
 * @param ip The IP address of control server.
 * @param id The ID of video message to play.
 */
void LinphonePlayVideoMessage(char* ip, int id);

/**
 * Makes calls to the devices in the same time.
 *
 * @param infoArray the array of DeviceInfo to make calls.
 * @param count The count of DeviceInfo in the infoArray.
 * @param video Enables video feature.
 * @param earlyMedia Enables early media feature.
 */
void LinphoneCallDevices(DeviceInfo infoArray[], int count, bool video, bool earlyMedia);

/**
 * Terminates all calls except the specified call ID.
 *
 * @param id Only the specified ID don't to terminate.
 */
void LinphoneTerminateOthers(long id);

/**
 * Stops playing sound.
 */
void LinphoneStopSound(void);

/**
 * Enables do not disturb mode.
 */
void LinphoneEnableDoNotDisturb(void);

/**
 * Disables do not disturb mode.
 */
void LinphoneDisableDoNotDisturb(void);

/**
 * Checks the status of do not disturb mode.
 *
 * @return 0 for not changed; 1 for enabling; -1 for disabling.
 */
int LinphoneCheckDoNotDisturb(void);

/**
 * Determines whether it is in do not disturb mode or not.
 *
 * @return true for it is in do not disturb mode, false otherwise.
 */
bool LinphoneIsInDoNotDisturb(void);

/**
 * Enables busy mode.
 */
void LinphoneEnableBusy(void);

/**
 * Disables busy mode.
 */
void LinphoneDisableBusy(void);

/**
 * Redirects current call to the specified IP address.
 *
 * @param ip The IP address to redirect to.
 */
void LinphoneRedirect(char* ip);

/**
 * Snapshots current video to the specified file path.
 *
 * @param filepath The file path to save the snapshot.
 */
void LinphoneSnapshot(char* filepath);

/**
 * Starts a voice memo recording to the specified file path.
 *
 * @param filepath The file path to save the voice memo.
 */
void LinphoneStartVoiceMemoRecord(char* filepath);

/**
 * Stops the voice memo recording.
 */
void LinphoneStopVoiceMemoRecord(void);

/**
 * Starts playing a voice memo recording.
 *
 * @param filepath The file path of the voice memo to play.
 */
void LinphoneStartVoiceMemoPlay(char* filepath);

/**
 * Stops playing the voice memo.
 */
void LinphoneStopVoiceMemoPlay(void);

/**
 * Registers to SIP server. The SIP server address is the center address. The SIP ID is the device address like 01-01-01-01-01-01.
 */
void LinphoneRegister(void);
#ifdef CFG_DUAL_REGISTER_SIP_SERVER
void* LinphoneRegTask(void);
void LinphoneAccount_1(void);
void LinphoneAccount_2(void);
void LinphoneRegisterAll(int priority);
#endif
void LinphoneStartIPCamStreaming(void);
void LinphoneStopIPCamStreaming(void);
void LinphoneStartIPCamRecord(char* filepath);
void LinphoneStopIPCamRecord(void);
void LinphoneIPCamSnapshot(char* filepath);

/**
 * Starts a video memo recording to the specified file path.
 *
 * @param filepath The file path to save the video memo.
 */
void LinphoneStartVideoMemoRecord(char* filepath);

/**
 * Stops the video memo recording.
 */
void LinphoneStopVideoMemoRecord(void);

/**
 * Starts playing a video memo recording.
 *
 * @param filepath The file path of the video memo to play.
 */
void LinphoneStartVideoMemoPlay(char* filepath);

/**
 * Stops playing the video memo.
 */
void LinphoneStopVideoMemoPlay(void);

void LinphoneStartMediaVideoPlay(void);
void LinphoneStopMediaVideoPlay(void);

void LinphoneStartMediaAudioPlay(void);
void LinphoneStopMediaAudioPlay(void);

/** @} */ // end of doorbell_indoor_linphone

/** @defgroup doorbell_indoor_message Message
 *  @{
 */
/**
 * Message definition
 */
typedef struct
{
    int id;         ///< Message ID
    char* topic;    ///< Message Topic
    char* time;     ///< Message Time
    char* image;    ///< The image URL of message content
    int read;       ///< Is the message read or not. 1: read 0: not read yet.
} Message;

/**
 * The function definition of getting message list.
 *
 * @param messages The pointer to retrieve the array of Message.
 * @param count Maximum count of Message in array.
 */
typedef void (*MessageListGetCallback)(Message* messages, int count);

/**
 * The function definition of message notify.
 */
typedef void (*MessageNotifyCallback)(void);

/**
 * The function definition of getting the content of text message.
 *
 * @param image The pointer to retrieve the JPEG image data.
 * @param imageSize The image size.
 */
typedef void (*MessageTextContentGetCallback)(uint8_t* image, int imageSize);

/**
 * Initializes message module.
 */
void MessageInit(void);

/**
 * Exits message module.
 */
void MessageExit(void);

/**
 * Gets the list of text messages.
 *
 * @param func The callback function to call when the downloading finished.
 */
void MessageTextListGet(MessageListGetCallback func);

/**
 * Deletes the specified text message.
 *
 * @param id The ID of text message.
 * @param func The callback function to call when the deleting finished.
 */
void MessageTextDelete(int id, MessageNotifyCallback func);

/**
 * Gets the content of specified text message.
 *
 * @param image The URL of the content of text message.
 * @param func The callback function to call when the downloading finished.
 */
void MessageTextContentGet(char* image, MessageTextContentGetCallback func);

/**
 * Gets the list of video messages.
 *
 * @param func The callback function to call when the downloading finished.
 */
void MessageVideoListGet(MessageListGetCallback func);

/**
 * Deletes the specified video message.
 *
 * @param id The ID of video message.
 * @param func The callback function to call when the deleting finished.
 */
void MessageVideoDelete(int id, MessageNotifyCallback func);

/** @} */ // end of doorbell_indoor_message

/** @defgroup doorbell_indoor_network Network
 *  @{
 */
/**
 * Initializes network module.
 */
void NetworkInit(void);

/**
 * Exits network module.
 */
void NetworkExit(void);

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

/**
 * Determines whether the control server is online or not.
 *
 * @return true for online; false for offline.
 */
bool NetworkServerIsReady(void);

/**
 * Updates current time from SNTP server.
 */
void NetworkSntpUpdate(void);

/** @} */ // end of doorbell_indoor_network

/** @defgroup doorbell_indoor_peripheral Peripheral
 *  @{
 */
/**
 * Initializes peripheral module.
 */
void PeripheralInit(void);

/**
 * Opens door.
 */
void PeripheralOpenDoor(void);

/**
 * Determines whether the door is opened or not.
 *
 * @return true for opened; false for closed.
 */
bool PeripheralDoorIsOpened(void);

/**
 * Reads the card number.
 *
 * @return the card number if exist; NULL for no card exists.
 */
char* PeripheralReadCard(void);

/** @} */ // end of doorbell_indoor_peripheral

/** @defgroup doorbell_indoor_photo Photo Loader
 *  @{
 */

typedef void (*PhotoLoadCallback)(uint8_t* data, int size);

void PhotoInit(void);

void PhotoExit(void);

int PhotoLoad(char* filename, PhotoLoadCallback func);

/** @} */ // end of doorbell_indoor_photo

/** @defgroup doorbell_indoor_qrcode QRCode
 *  @{
 */
/**
 * The function definition of registering to get the image of qrcode.
 *
 * @param image The pointer to retrieve the JPEG image data.
 * @param imageSize The image size.
 */
typedef void (*QRCodeRegisterCallback)(uint8_t* image, int imageSize);

/**
 * Initializes qrcode module.
 */
void QRCodeInit(void);

/**
 * Exits qrcode module.
 */
void QRCodeExit(void);

/**
 * Rigisters the specified qrcode.
 *
 * @param user The user name.
 * @param func The callback function to call when the downloading qrcode image finished.
 */
void QRCodeRegister(char* user, QRCodeRegisterCallback func);

/** @} */ // end of doorbell_indoor_qrcode

/** @defgroup doorbell_indoor_remote Remote
 *  @{
 */
/**
 * Control Server/Door Camera information definition.
 */
typedef struct
{
    char firmwareVersion[64];   ///< Firmware version
    char firmwareUrl[256];      ///< Firmware URL
    int addressBookVersion;     ///< Address book version
    char addressBookUrl[256];   ///< Address book URL
    int screensaverVersion;     ///< Screensaver image version
    char screensaverUrl[256];   ///< Screensaver image URL
    int cardListVersion;        ///< Card list version
    char cardListUrl[256];      ///< Card list URL
    int settingVersion;         ///< Settings version
    char settingUrl[256];       ///< Settings URL
    int advertisementVersion;   ///< Advertisement version
    char advertisementUrl[256]; ///< Advertisement URL
    int ringLevel;              ///< Ring level
    int playLevel;              ///< Play level
    int recLevel;               ///< Record level
    int dialTime;               ///< Dial time
    int voiceMemoTime;          ///< Voice memo time
    int callingTime;            ///< Calling time
} ServerInfo;

extern ServerInfo theOutdoorServerInfo;   ///< Door camera information.

/**
 * The function definition of getting the weather information.
 *
 * @param xml The pointer to retrieve the weather XML data.
 * @param size The XML size.
 */
typedef void (*RemoteGetWeatherCallback)(char* xml, int size);

/**
 * The function definition of getting the door camera information.
 *
 * @param info The pointer to retrieve ServerInfo.
 */
typedef void (*RemoteGetOutdoorInfoCallback)(ServerInfo* info);

/**
 * Initializes remote module.
 */
void RemoteInit(void);

/**
 * Exits remote module.
 */
void RemoteExit(void);

/**
 * Opens door of the specified IP address.
 *
 * @param ip The IP address to open door.
 */
void RemoteOpenDoor(char* ip);

/**
 * Verifies the card number from the control server.
 *
 * @param cardnum The card number to verify.
 * @return true for valid; false for invalid.
 */
bool RemoteCheckCard(char* cardnum);

/**
 * Invokes the elevator.
 *
 * @param up The direction is up or not.
 */
void RemoteCallElevator(bool up);

/**
 * Gets the information of control server.
 *
 * @return The ServerInfo of control server.
 */
ServerInfo* RemoteGetServerInfo(void);

/**
 * Sets the configuration to door camera.
 *
 * @param ip the IP address string.
 * @param area The area code of address.
 * @param building The building code of address.
 * @param unit The unit code of address.
 * @param floor The floor code of address.
 * @param room The room code of address.
 * @param ext The ext code of address.
 * @return 0 for success; otherwise for fail.
 */
int RemoteSetOutdoorSetting(char* ip, char* area, char* building, char* unit, char* floor, char* room, char* ext);

/**
 * Gets the weather information.
 *
 * @param func The callback function to call when the downloading finished.
 */
void RemoteGetWeather(RemoteGetWeatherCallback func);

/**
 * Sets the do not disturb configuration to the other extension devices of indoor phone.
 */
void RemoteSetDontDisturbConfig(void);

/**
 * Gets the do not disturb configuration from the first device of indoor phone.
 */
void RemoteGetDontDisturbConfig(void);

/**
 * Sets the ext calling state to the other extension devices of indoor phone.
 */
void RemoteSetExtCallingState(int ext_calling);

/**
 * Gets the door camera information.
 *
 * @param ip The door camera ip.
 * @param func The callback function to call when the downloading finished.
 */
void RemoteGetOutdoorInfo(char* ip, RemoteGetOutdoorInfoCallback func);

/**
 * Sets the door camera configuration.
 *
 * @param ip The door camera ip.
 * @param ringLevel The ring level.
 * @param playLevel The play level.
 * @param recLevel The record level.
 * @param dialTime The dial time.
 * @param voiceMemoTime The voice memo time.
 * @param callingTime The calling time.
 */
void RemoteSetOutdoorConfig(char* ip, int ringLevel, int playLevel, int recLevel, int dialTime, int voiceMemoTime, int callingTime);

/** @} */ // end of doorbell_indoor_remote

/** @defgroup doorbell_indoor_screen Screen
 *  @{
 */
/**
 * Screensaver type definition.
 */
typedef enum
{
    SCREENSAVER_NONE,   ///< No screensaver
    SCREENSAVER_CLOCK,  ///< Clock sreensaver
    SCREENSAVER_BLANK,  ///< Black screen screensaver
    SCREENSAVER_PHOTO   ///< Photo screensaver
} ScreensaverType;

/**
 * Initializes screen module.
 */
void ScreenInit(void);

/**
 * Turns off screen.
 */
void ScreenOff(void);

/**
 * Turns on screen.
 */
void ScreenOn(void);

/**
 * Is the screen off or on.
 *
 * @return true for off; false for on.
 */
bool ScreenIsOff(void);

/**
 * Sets the brightness.
 *
 * @param value The brightness value. Range 0~9.
 */
void ScreenSetBrightness(int value);

/**
 * Re-counts the time to enter screensaver.
 */
void ScreenSaverRefresh(void);

/**
 * Checks whether it is about time to enter screensaver mode.
 *
 * @return 0 for net yet, otherwise for entering screensaver mode currently.
 */
int ScreenSaverCheck(void);

/**
 * Pause to count down the time to enter screensaver mode.
 */
void ScreenSaverPause(void);

/**
 * Continue to count down the time to enter screensaver mode.
 */
void ScreenSaverContinue(void);

/**
 * Is on screensaver mode or not.
 */
bool ScreenSaverIsScreenSaving(void);

/**
 * Takes a screenshot to USB drive.
 *
 * @param lcdSurf The LCD surface widget.
 */
void Screenshot(void* lcdSurf);

/**
 * Clears screen.
 *
 */
void ScreenClear(void);

/** @} */ // end of doorbell_indoor_screen

/** @defgroup doorbell_indoor_storage Storage
 *  @{
 */

typedef enum
{
    STORAGE_NONE = -1,
    STORAGE_USB,
    STORAGE_SD,

    STORAGE_MAX_COUNT
} StorageType;

typedef enum
{
    STORAGE_UNKNOWN,
    STORAGE_SD_INSERTED,
    STORAGE_SD_REMOVED,
    STORAGE_USB_INSERTED,
    STORAGE_USB_REMOVED
} StorageAction;

/**
 * Initializes storage module.
 */
void StorageInit(void);

StorageAction StorageCheck(void);
StorageType StorageGetCurrType(void);
void StorageSetCurrType(StorageType type);
char* StorageGetDrivePath(StorageType type);

/** @} */ // end of doorbell_indoor_storage

/** @defgroup doorbell_indoor_string String
 *  @{
 */

/**
 * Setting type definition.
 */
typedef enum
{
    SETTING_DISPLAY,    ///< Display
    SETTING_SOUND,      ///< Sound
    SETTING_VOICE,      ///< Voice
    SETTING_DISTURB,    ///< Do not disburb
    SETTING_LANGUAGE,   ///< Language
    SETTING_NETWORK,    ///< Network
    SETTING_SENSOR,     ///< Sensor
    SETTING_TERMINAL,   ///< Terminal number
    SETTING_OUTDOOR,    ///< Door Camera
    SETTING_MISC,       ///< Misc
    SETTING_SYSTEM,     ///< System

    SETTING_COUNT       ///< Setting item count
} SettingType;

/**
 * Gets address description by address information.
 *
 * @param area The area code of address.
 * @param building The building code of address.
 * @param unit The unit code of address.
 * @param floor The floor code of address.
 * @param room The room code of address.
 * @param ext The ext code of address. Can be NULL if not exist.
 * @return the address string. Remember to free it after used to avoid memory leak.
 */
char* StringGetRoomAddress(char* area, char* building, char* unit, char* floor, char* room, char* ext);

/**
 * Gets camera description by address information.
 *
 * @param area The area code of address.
 * @param building The building code of address.
 * @param unit The unit code of address.
 * @return the camera string. Remember to free it after used to avoid memory leak.
 */
char* StringGetCameraPosition(char* area, char* building, char* unit);

/**
 * Gets system information description.
 *
 * @param count The pointer to retrieve the count of entry.
 * @return the array of system information strings.
 */
const char* StringGetSystemInfoArray(int* count);

/**
 * Gets device type description.
 *
 * @param type The device type.
 * @return the device type string.
 */
const char* StringGetDeviceType(DeviceType type);

/**
 * Gets the description of calling log type.
 *
 * @param type The calling log type.
 * @return the string of calling log type.
 */
const char* StringGetCallLogType(CallLogType type);

const char* StringGetGuardMode(GuardMode mode);

/**
 * Gets the description of guard sensor.
 *
 * @param sensor The guard sensor.
 * @return the string of guard sensor.
 */
const char* StringGetGuardSensor(GuardSensor sensor);

/**
 * Gets the description of status of guard sensor.
 *
 * @param abnormal Is abnormal or not.
 * @return the string of status of guard sensor.
 */
const char* StringGetGuardSensorStatus(bool abnormal);

const char* StringGetGuardSensorState(bool enabled);

/**
 * Gets the description of status of guard sensor.
 *
 * @param status The status of guard sensor.
 * @return the string of status of guard sensor.
 */
const char* StringGetGuardStatus(GuardStatus status);

/**
 * Gets the description of guard type.
 *
 * @return the string of guard type.
 */
const char* StringGetGuardType(GuardType type);

/**
 * Gets the description of guard state.
 *
 * @return the string of guard state.
 */
const char* StringGetGuardState(GuardState state);

/**
 * Gets the description of house status.
 *
 * @param status The house status.
 * @return the string of house status.
 */
const char* StringGetHouseStatus(HouseStatus status);

/**
 * Gets the description of house mode.
 *
 * @param mode The house mode.
 * @return the string of house mode.
 */
const char* StringGetHouseMode(HouseMode mode);

/**
 * Gets the description of working mode.
 *
 * @param engineer Is in engineer mode or not.
 * @return the string of working mode.
 */
const char* StringGetWorkMode(bool engineer);

/**
 * Gets setting type description.
 *
 * @param type The setting type.
 * @return the setting type string.
 */
const char* StringGetSettingType(SettingType type);

/**
 * Gets the description of WiFi connected.
 *
 * @return the string of WiFi connected.
 */
const char* StringGetWiFiConnected(void);

/** @} */ // end of doorbell_indoor_string

/** @defgroup doorbell_indoor_upgrade Upgrade
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
    QUIT_RESET_TEMP_DRIVE,      ///< Quit to reset temp drive
    QUIT_UPGRADE_FIRMWARE,      ///< Quit to upgrade firmware
    QUIT_UPGRADE_RESOURCE,      ///< Quit to upgrade resource
    QUIT_UPGRADE_ADDRESSBOOK,   ///< Quit to upgrade address book
    QUIT_UPGRADE_CARDLIST,      ///< Quit to upgrade card list
    QUIT_UPGRADE_WEB,           ///< Quit to wait web upgrading
    QUIT_RESET_NETWORK          ///< Quit to reset network
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
 * Gets the XML string of the device information.
 *
 * @param size The pointer to retrieve the string size.
 * @return The XML string. Remember to free it after used to avoid memory leak.
 */
char* UpgradeGetDeviceInfo(int* size);

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

/**
 * Gets the SD drive path.
 *
 * @return the SD drive path. NULL for SD not exist.
 */
char* UpgradeGetSDPath(void);

/** @} */ // end of doorbell_indoor_upgrade

/** @defgroup doorbell_indoor_videomemo Video Memo
 *  @{
 */

#define VIDEOMEMO_MAX_FILENAME_SIZE  40  ///< Maximum size of video memo file name.
#define VIDEOMEMO_MAX_NAME_SIZE      40  ///< Maximum count of video memo item name.

/**
 * Video memo definition
 */
typedef struct
{
    MediaType type; ///< Media type. Can be MEDIA_VIDEO or MEDIA_AUDIO
    char filename[VIDEOMEMO_MAX_FILENAME_SIZE]; ///< File name
    char name[VIDEOMEMO_MAX_NAME_SIZE];         ///< Name displayed in UI
    time_t time;    ///< Created time
    int sec;        ///< The time length
    bool played;    ///< Is played or not
} VideoMemoEntry;

/**
 * Initializes video memo module.
 */
void VideoMemoInit(void);

/**
 * Exits video memo module.
 */
void VideoMemoExit(void);

/**
 * Gets total count of video memo file.
 *
 * @return the total count of video memo file.
 */
int VideoMemoGetCount(void);

/**
 * Gets unplay count of video memo file.
 *
 * @return unplay count of video memo file.
 */
int VideoMemoGetUnplayCount(void);

/**
 * Gets the video memo entry by index.
 *
 * @param index The index of video memo entry.
 * @return The video memo entry, NULL if not found.
 */
VideoMemoEntry* VideoMemoGetEntry(int index);

/**
 * Deletes the video memo entry by index.
 *
 * @param index The index of video memo entry.
 */
void VideoMemoDeleteEntry(int index);

/**
 * Playing the video memo file is finished or not.
 *
 * @return true for finished, false for otherwise.
 */
bool VideoMemoIsPlayFinished(void);

/**
 * Plays the video memo file.
 *
 * @param index The index of video memo file to play.
 * @return 0 for success, otherwise failed.
 */
int VideoMemoStartPlay(int index);

/**
 * Stops playing video memo file.
 */
void VideoMemoStopPlay(void);

/**
 * Starts recording.
 *
 * @param type The media type. Only for MEDIA_VIDEO and MEDIA_AUDIO types.
 * @param addr The IP address of target to record.
 * @return 0 for success, otherwise failed.
 */
int VideoMemoStartRecord(MediaType type, char* addr);

/**
 * Stops recording.
 *
 * @return 0 for success, otherwise failed.
 */
int VideoMemoStopRecord(void);

/**
 * Gets the path of video memo directory.
 *
 * @return The path.
 */
char* VideoMemoGetPath(void);
bool VideoMemoCheckDiskSpace(int max_file_size);

/** @} */ // end of doorbell_indoor_videomemo

/** @defgroup doorbell_indoor_videorecord Video Recorder
 *  @{
 */
/**
 * Initializes video recorder module.
 */
void VideoRecordInit(void);

/**
 * Exits video recorder module.
 */
void VideoRecordExit(void);

int IPCamStreamStartStreaming(void);
int IPCamStreamStopStreaming(void);
int IPCamRecordStartRecord(char* filepath);
int IPCamRecordStopRecord(void);

/**
 * Start recording to the specified file.
 *
 * @param filepath The specified file to record.
 * @return 0 for success, otherwise failed.
 */
int VideoRecordStartRecord(char* filepath);

/**
 * Stop recording.
 *
 * @return 0 for success, otherwise failed.
 */
int VideoRecordStopRecord(void);

/**
 * Gets the time length of the specified recorded file, in seconds.
 *
 * @param filepath The specified file path.
 * @return The time length, in seconds.
 */
int VideoRecordGetTimeLength(char* filepath);

/**
 * The function definition of video record play finished.
 */
typedef void (*VideoRecordPlayFinishedCallback)(void);

/**
 * Plays the specified recorded file.
 *
 * @param filepath The specified recorded file to play.
 * @param func The callback function to call when the playing finished.
 * @return 0 for success, otherwise failed.
 */
int VideoRecordStartPlay(char* filepath, VideoRecordPlayFinishedCallback func);

/**
 * Stops playing recorded file.
 */
void VideoRecordStopPlay(void);

/** @} */ // end of doorbell_indoor_videorecord

/** @defgroup doorbell_indoor_voicememo Voice Memo
 *  @{
 */

#define VOICEMEMO_MAX_NAME_SIZE      40  ///< Maximum count of voice memo item name.

/**
 * Voice memo definition
 */
typedef struct
{
    char name[VOICEMEMO_MAX_NAME_SIZE]; ///< Name displayed in UI
    time_t time;    ///< Created time
    int sec;        ///< The time length
    bool played;    ///< Is played or not
} VoiceMemoEntry;

/**
 * Initializes voice memo module.
 */
void VoiceMemoInit(void);

/**
 * Exits voice memo module.
 */
void VoiceMemoExit(void);

/**
 * Gets total count of voice memo file.
 *
 * @return the total count of voice memo file.
 */
int VoiceMemoGetCount(void);

/**
 * Gets unplay count of voice memo file.
 *
 * @return unplay count of voice memo file.
 */
int VoiceMemoGetUnplayCount(void);

/**
 * Gets the voice memo entry by index.
 *
 * @param index The index of voice memo entry.
 * @return The voice memo entry, NULL if not found.
 */
VoiceMemoEntry* VoiceMemoGetEntry(int index);

/**
 * Deletes the voice memo entry by index.
 *
 * @param index The index of voice memo entry.
 */
void VoiceMemoDeleteEntry(int index);

/**
 * Plays the voice memo file.
 *
 * @param index The index of voice memo file to play.
 * @return 0 for success, otherwise failed.
 */
int VoiceMemoStartPlay(int index);

/**
 * Stops playing voice memo file.
 */
void VoiceMemoStopPlay(void);

/**
 * Starts recording.
 *
 * @return 0 for success, otherwise failed.
 */
int VoiceMemoStartRecord(void);

/**
 * Stops recording.
 *
 * @return 0 for success, otherwise failed.
 */
int VoiceMemoStopRecord(void);

/**
 * Checks the disk space is availabe to record voice memo.
 *
 * @return true for success, otherwise failed.
 */
bool VoiceMemoCheckDiskSpace(void);

/** @} */ // end of doorbell_indoor_voicememo

/** @defgroup doorbell_indoor_weather Weather
 *  @{
 */
/**
 * Initializes weather module.
 */
void WeatherInit(void);

/**
 * Exits weather module.
 */
void WeatherExit(void);

/**
 * Is the weather XML parsed or not.
 *
 * @return true for paring success, false otherwise.
 */
bool WeatherIsAvailable(void);

/**
 * Gets the date description.
 *
 * @param day The day index starting from today. The range is 0~4.
 * @return The date string. Remember to free it after used to avoid memory leak.
 */
char* WeatherGetDate(int day);

/**
 * Gets the symbol index.
 *
 * @param day The day index starting from today. The range is 0~4.
 * @return The symbol index. The range is 0~8.
 */
int WeatherGetSymbolIndex(int day);

/**
 * Gets the minimum temperature.
 *
 * @param day The day index starting from today. The range is 0~4.
 * @return The temperature value.
 */
float WeatherGetTemperatureMin(int day);

/**
 * Gets the maximum temperature.
 *
 * @param day The day index starting from today. The range is 0~4.
 * @return The temperature value.
 */
float WeatherGetTemperatureMax(int day);

/**
 * Gets the symbol description.
 *
 * @param day The day index starting from today. The range is 0~4.
 * @return The symbol string. Remember to free it after used to avoid memory leak.
 */
char* WeatherGetSymbolName(int day);

/**
 * Gets the wind speed description.
 *
 * @param day The day index starting from today. The range is 0~4.
 * @return The wind speed string. Remember to free it after used to avoid memory leak.
 */
char* WeatherGetWindSpeedName(int day);

char* WeatherGetLocation(void);
float WeatherGetCurrTemperature(int day);
char* WeatherGetWindDirection(int day);
int WeatherGetHumidity(int day);
float WeatherGetWindSpeed(int day);

/** @} */ // end of doorbell_indoor_weather

/** @defgroup doorbell_indoor_webserver Web Server
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

/**
 * Gets the count of new text messages.
 *
 * @return The count of new text messages.
 */
int WebServerGetNewTextMsgCount(void);

/**
 * Gets the count of new video messages.
 *
 * @return The count of new video messages.
 */
int WebServerGetNewVideoMsgCount(void);

/** @} */ // end of doorbell_indoor_webserver

#ifdef __cplusplus
}
#endif

#endif /* DOORBELL_H */
/** @} */ // end of doorbell_indoor