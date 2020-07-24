#ifdef CFG_NET_WIFI_SDIO_NGPL
#include <pthread.h>
#include "lwip/netif.h"
#include "../dhcps/dhcps.h"

#include "wifi_conf.h"
#include "wifi_constants.h"
#endif
#include "ite/itp.h"

#define WIFI_SSID_MAXLEN                 32
#define WIFI_PASSWORD_MAXLEN             256
#define WIFI_SECUMODE_MAXLEN             3
#define WIFI_SCAN_LIST_NUMBER            64
#define WIFI_STACK_SIZE                  80000   //(255 * 1024)
#define WIFI_INIFILENAME_MAXLEN          32
#define WIFIMGR_CHECK_WIFI_MSEC          1000
#define WIFIMGR_RECONNTECT_TIME          3
#define WIFIMGR_REOFFER_DHCP_TIME        3
#define WIFIMGR_REMOVE_ALL_SAME_SSID     0  //Set(0): Remove same MAC Addr but have same SSID. Set(1): Remove same SSID no matter with different  MAC.
#define WIFIMGR_SHOW_SCAN_LIST           0

#define WIFI_CONNECT_COUNT               (30 * 10)
#define WIFI_CONNECT_DHCP_COUNT          (20 * 10)
#define WIFI_SET_TIMEMAX                 (2  * 60 * 1000)      // mini second
#define WIFI_SWITCH_MODE_WAIT_TIME       (5  * 1000 * 1000)
#define WIFIMGR_RECONNTECT_MSEC          (60 * 1000)
#define WIFIMGR_TEMPDISCONN_MSEC         (40 * 1000)
#define WIFIMGR_DHCP_RENEW_MSEC          (8  * 60 * 1000)

#define WIFI_TRIEDSECU_NOSEC             0x1
#define WIFI_TRIEDSECU_WEP               0x2
#define WIFI_TRIEDSECU_WPAPSK            0x4
#define WIFI_TRIEDSECU_WPA2PSK           0x8
#define WIFI_TRIEDSECU_WPAPSK_MIX        0xF //for wpa tool

//ITE WIFI Definition of Encrypt
#define ITE_WIFI_SEC_OPEN                "0"
#define ITE_WIFI_SEC_WEP_PSK             "1"//WEP
#define ITE_WIFI_SEC_WPA_TKIP_PSK        "2"//WPA TKIP+PSK
#define ITE_WIFI_SEC_WPA_AES_PSK         "3"//WPA AES+PSK
#define ITE_WIFI_SEC_WPA2_AES_PSK        "4"//WPA2 AES+PSK
#define ITE_WIFI_SEC_WPA2_TKIP_PSK       "5"//WPA2 TKIP+PSK
#define ITE_WIFI_SEC_WPA2_MIXED_PSK      "6"//WPA2 AES/TKIP PSK
#define ITE_WIFI_SEC_WPA_WPA2_MIXED      "7"//WPA/WPA2 AES
#define ITE_WIFI_SEC_WPS_OPEN            "8"//WPS OPEN
#define ITE_WIFI_SEC_WPS_SECURE          "9"//WPS SECURE
#define ITE_WIFI_SEC_EAP_AES             "10"//EAP WPA2 AES
#define ITE_WIFI_SEC_UNKNOWN             "-1"

typedef enum tagWIFI_TRYAP_PHASE_E
{
    WIFI_TRYAP_PHASE_NONE = 0,
    WIFI_TRYAP_PHASE_SAME_SSID,
    WIFI_TRYAP_PHASE_EMPTY_SSID,
    WIFI_TRYAP_PHASE_FINISH,
    WIFI_TRYAP_PHASE_MAX,
} WIFI_TRYAP_PHASE_E;

typedef enum tagWIFIMGR_ECODE_E
{
    WIFIMGR_ECODE_OK = 0,
    WIFIMGR_ECODE_FAIL,
    WIFIMGR_ECODE_NOT_INIT,
    WIFIMGR_ECODE_ALLOC_MEM,
    WIFIMGR_ECODE_SEM_INIT,
    WIFIMGR_ECODE_MUTEX_INIT,
    WIFIMGR_ECODE_NO_LED,
    WIFIMGR_ECODE_NO_WIFI_DONGLE,
    WIFIMGR_ECODE_NO_INI_FILE,
    WIFIMGR_ECODE_NO_SSID,
    WIFIMGR_ECODE_CONNECT_ERROR,
    WIFIMGR_ECODE_DHCP_ERROR,
    WIFIMGR_ECODE_OPEN_FILE,
    WIFIMGR_ECODE_DTMF_DEC_TIMEOUT,
    WIFIMGR_ECODE_UNKNOWN_SECURITY,
    WIFIMGR_ECODE_SET_DISCONNECT,
    WIFIMGR_ECODE_MAX,
} WIFIMGR_ECODE_E;


typedef enum tagWIFIMGR_CONNSTATE_E
{
    WIFIMGR_CONNSTATE_STOP = 0,
    WIFIMGR_CONNSTATE_SETTING,
    WIFIMGR_CONNSTATE_SCANNING,
    WIFIMGR_CONNSTATE_CONNECTING,
    WIFIMGR_CONNSTATE_MAX,
} WIFIMGR_CONNSTATE_E;

typedef enum tagWIFIMGR_STATE_CALLBACK_E
{
    WIFIMGR_STATE_CALLBACK_CONNECTION_FINISH = 0,
    WIFIMGR_STATE_CALLBACK_CLIENT_MODE_DISCONNECT_30S,
    WIFIMGR_STATE_CALLBACK_CLIENT_MODE_RECONNECTION,
    WIFIMGR_STATE_CALLBACK_CLIENT_MODE_TEMP_DISCONNECT,
    WIFIMGR_STATE_CALLBACK_CLIENT_MODE_CONNECTING_FAIL,
    WIFIMGR_STATE_CALLBACK_CLIENT_MODE_CONNECTING_CANCEL,
    WIFIMGR_STATE_CALLBACK_CLIENT_MODE_SLEEP_SAVE_INFO,
    WIFIMGR_STATE_CALLBACK_CLIENT_MODE_SLEEP_CLEAN_INFO,
    WIFIMGR_STATE_CALLBACK_SWITCH_CLIENT_SOFTAP_FINISH,
    WIFIMGR_STATE_CALLBACK_MAX,
} WIFIMGR_STATE_CALLBACK_E;


typedef enum tagWIFIMGR_MODE_E
{
    WIFIMGR_MODE_CLIENT = 0,
    WIFIMGR_MODE_SOFTAP,
    WIFIMGR_MODE_MAX,
} WIFIMGR_MODE_E;

typedef enum tagWIFIMGR_SWTICH_ON_OFF_E
{
    WIFIMGR_SWITCH_OFF = 0,
    WIFIMGR_SWITCH_ON,
} WIFIMGR_SWTICH_ON_OFF_E;


typedef struct WIFI_MGR_SETTING_TAG
{
    int             (*wifiCallback)(int nCondition);
    char            ssid[WIFI_SSID_MAXLEN];
    char            password[WIFI_PASSWORD_MAXLEN];
#ifdef CFG_NET_WIFI_SDIO_NGPL
    unsigned long   secumode;
#else
    char            secumode[WIFI_SECUMODE_MAXLEN];
#endif
    ITPEthernetSetting setting;
}WIFI_MGR_SETTING;

typedef struct WIFI_MGR_SCANAP_LIST_TAG
{
    unsigned char   name[16];
#ifdef CFG_NET_WIFI_SDIO_NGPL
    unsigned char   apMacAddr[6];
    signed short    rfQualityRSSI; //RSSI(dBm)
    unsigned long   securityMode;
#else
    unsigned char   apMacAddr[6+2];
    signed char     rfQualityRSSI; //RSSI(dBm)
    int             securityMode;  /*Sec. Mode*/
#endif
    int             channelId;
    unsigned char   ssidName[32];
    int             operationMode;
    int             securityOn;
    unsigned char   rfQualityQuant; //Percent : 0~100
    unsigned char   reserved[2];
    int             bitrate;
} WIFI_MGR_SCANAP_LIST;

typedef struct WIFI_MGR_VAR_TAG
{
    /* WIFI Connect Info */
    char            Ssid[WIFI_SSID_MAXLEN];
    char            Password[WIFI_PASSWORD_MAXLEN];
#ifdef CFG_NET_WIFI_SDIO_NGPL
    unsigned long   SecurityMode;
#else
    char            SecurityMode[WIFI_SECUMODE_MAXLEN];
#endif
    char            Mac_String[32];

    /* WIFI HostAP Mode Info */
    char            ApSsid[WIFI_SSID_MAXLEN];
    char            ApPassword[WIFI_PASSWORD_MAXLEN];

    WIFIMGR_MODE_E   WIFI_Mode;
    WIFI_MGR_SETTING WifiMgrSetting;

    unsigned int    Client_On_Off;
    int             MP_Mode;

    /* WIFI Flags */
    bool            Need_Set;
    bool            Pre_Scan;
    bool            Start_Scan;
    bool            WIFI_Init_Ready;
    bool            SoftAP_Hidden;
    bool            SoftAP_Init_Ready;
    bool            WIFI_Terminate;
    bool            WPA_Terminate;
    bool            Cancel_Connect;
    bool            Cancel_WPS;
    bool            No_SSID;
    bool            No_Config_File;
    bool            Is_First_Connect;   // first connect when the system start up
    bool            Is_WIFI_Available;
    bool            Is_Temp_Disconnect; // is temporary disconnect
}WIFI_MGR_VAR;



/***************** WifiMgr API *****************/

/* Init WIFI mode */
int  WifiMgr_Init(WIFIMGR_MODE_E init_mode, int mp_mode, WIFI_MGR_SETTING wifiSetting);

/* Terminate WIFI mode */
int  WifiMgr_Terminate(void);

int  WifiMgr_Sta_Is_Available(int* is_available);

int  WifiMgr_Is_WPA_Terminating(void);

/* Get all of  "WIFI_MGR_SCANAP_LIST" and return AP list's conut */
int  WifiMgr_Get_Scan_AP_Info(WIFI_MGR_SCANAP_LIST* pList);

int  WifiMgr_Get_Connect_State(int* conn_state, int* e_code);

int  WifiMgr_Get_MAC_Address(char cMac[6]);

int  WifiMgr_Get_WIFI_Mode(void);

/* Get number of connecting device to AP */
int  WifiMgr_Get_HostAP_Device_Number(void);

/* Not ready(-1), Ready(0) */
int  WifiMgr_Get_HostAP_Ready(void);

#ifdef CFG_NET_WIFI_SDIO_NGPL
int  WifiMgr_HostAP_First_Start(void);
#endif

int  WifiMgr_Sta_Connect(char* ssid, char* password, char* secumode);

int  WifiMgr_Sta_Disconnect(void);

int  WifiMgr_Sta_Sleep_Disconnect(void);

/* Switch "Client to SoftAP" or "SoftAP to Client" */
int  WifiMgr_Sta_HostAP_Switch(WIFI_MGR_SETTING wifiSetting);

void WifiMgr_Sta_Cancel_Connect(void);

void WifiMgr_Sta_Not_Cancel_Connect(void);

/* Check WIFI status: WIFI is turned close(0), WIFI is turned open(1) */
void WifiMgr_Sta_Switch(int status);

unsigned long WifiMgr_Secu_ITE_To_8189F(char* ite_security_enum);

char* WifiMgr_Secu_8189F_To_ITE(unsigned long rtw_security_8189f);

int WifiMgr_Secu_8188E_To_ITE(int rtw_security_8188eu);


/* Old API Name */
#define wifiMgr_get_connect_state           WifiMgr_Get_Connect_State
#define wifiMgr_get_scan_ap_info            WifiMgr_Get_Scan_AP_Info
#define wifiMgr_get_APMode_Ready            WifiMgr_Get_HostAP_Ready
#define wifiMgr_get_softap_device_number    WifiMgr_Get_HostAP_Device_Number
#define wifiMgr_clientMode_connect_ap       WifiMgr_Sta_Connect
#define wifiMgr_clientMode_disconnect       WifiMgr_Sta_Disconnect
#define wifiMgr_clientMode_sleep_disconnect WifiMgr_Sta_Sleep_Disconnect
#define WifiMgr_clientMode_switch           WifiMgr_Sta_Switch
#define wifiMgr_is_wifi_available           WifiMgr_Sta_Is_Available
#define wifiMgr_CancelConnect               WifiMgr_Sta_Cancel_Connect
#define wifiMgr_Not_CancelConnect           WifiMgr_Sta_Not_Cancel_Connect
#define wifiMgr_softap_set_hidden           WifiMgr_HostAP_Set_Hidden
#define wifiMgr_init                        WifiMgr_Init
#define wifiMgr_terminate                   WifiMgr_Terminate
#define WifiMgr_firstStartSoftAP_Mode       WifiMgr_HostAP_First_Start
#define WifiMgr_Switch_ClientSoftAP_Mode    WifiMgr_Sta_HostAP_Switch
#define WifiMgr_Is_Wpa_Wifi_Terminating     WifiMgr_Sta_WPA_Terminate_Status