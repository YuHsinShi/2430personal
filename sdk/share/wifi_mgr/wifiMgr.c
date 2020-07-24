#if !defined(CFG_NET_WIFI_SDIO_NGPL)
#include <pthread.h>
#include <sys/ioctl.h>
#include "ite/itp.h"
#include "lwip/netif.h"
#include "wifiMgr.h"
#include "../dhcps/dhcps.h"

extern void dhcps_init(void);
extern void dhcps_deinit(void);

#ifdef CFG_NET_WIFI
#include "ite/ite_wifi.h"

/* Global Variable */
static pthread_t                        clientModeTask, wpsTask, processTask;

static sem_t                            semConnectStart, semConnectStop;
static pthread_mutex_t                  mutexALWiFi, mutexIni, mutexMode;

static sem_t                            semWPSStart, semWPSStop;
static pthread_mutex_t                  mutexALWiFiWPS;

static WIFI_MGR_SCANAP_LIST             gWifiMgrApList[WIFI_SCAN_LIST_NUMBER];
static WIFI_MGR_SCANAP_LIST             gWifiMgrTempApList[WIFI_SCAN_LIST_NUMBER];

static WIFIMGR_CONNSTATE_E              wifi_conn_state    = WIFIMGR_CONNSTATE_STOP;
static WIFIMGR_ECODE_E                  wifi_conn_ecode    = WIFIMGR_ECODE_SET_DISCONNECT;

static struct net_device_info           apInfo  = {0},    gScanApInfo = {0};
static struct timeval                   tvDHCP1 = {0, 0}, tvDHCP2     = {0, 0};

/* WifiMgr flags */
static WIFI_MGR_VAR                     gWifiMgrVar        =
{
    /* Default flag*/
    .WIFI_Mode              = WIFIMGR_MODE_CLIENT,
    .WifiMgrSetting         = {0},
    .MP_Mode                = 0,
    .Need_Set               = false,
    .WIFI_Init_Ready        = false,
    .SoftAP_Init_Ready      = false,
    .WIFI_Terminate         = false,
    .WPA_Terminate          = false,
    .Cancel_Connect         = false,
    .Cancel_WPS             = false,
    .No_SSID                = false,
    .No_Config_File         = false,
    .Is_First_Connect       = true ,
    .Is_WIFI_Available      = false,
    .Is_Temp_Disconnect     = false
};

static int  _WifiMgr_Sta_Remove_Same_SSID(int size);
static void _WifiMgr_Sta_List_Sort_Insert(int size);
static int  _WifiMgr_Sta_Scan_Post(void);


/* ================================================= */
/*
/*                                             Static Functions                                              */
/*
/* ================================================= */


static void
_WifiMgr_Create_Worker_Thread(pthread_t task, void *(*start_routine)(void *), void *arg)
{
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    //pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr, WIFI_STACK_SIZE);
    pthread_create(&task, &attr, start_routine, arg);
}


static void
_WifiMgr_Sta_Connect_Hidden_AP(void)
{
    struct net_device_info netInfo = {0};

    strcpy(netInfo.hidden_ssid, gWifiMgrVar.Ssid);
    netInfo.connect_hidden_ssid_enable = true;
    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_HIDDEN_SSID, &netInfo);
}


static int
_WifiMgr_Sta_Connect_Process(void)
{
    struct net_device_config netCfg = {0};
    ITPEthernetSetting setting;
    ITPWifiInfo wifiInfo;

    int nRet = WIFIMGR_ECODE_OK;
    int is_connected = 0, is_ssid_match = 0, dhcp_available = 0;
    int phase = 0, lastIdx = 0, triedSecu = 0, nSecurity = -1;
    unsigned long connect_cnt = 0;
    char *ssid, *password, *secumode;


    if (gWifiMgrVar.Cancel_Connect) {
        goto end;
    }

    ssid        = gWifiMgrVar.Ssid;
    password    = gWifiMgrVar.Password;
    secumode    = gWifiMgrVar.SecurityMode;

    if (gWifiMgrVar.MP_Mode) {
        printf("[WIFIMGR] Is mpMode, connect to default SSID.\r\n");
        // SSID
        snprintf(ssid, WIFI_SSID_MAXLEN, "%s", CFG_NET_WIFI_MP_SSID);
        // Password
        snprintf(password, WIFI_PASSWORD_MAXLEN, "%s", CFG_NET_WIFI_MP_PASSWORD);
#ifdef DTMF_DEC_HAS_SECUMODE
        // Security mode
        snprintf(secumode, WIFI_SECUMODE_MAXLEN, "%s", CFG_NET_WIFI_MP_SECURITY);
#endif
    }


    // dhcp
    setting.dhcp = gWifiMgrVar.WifiMgrSetting.setting.dhcp;

    // autoip
    setting.autoip = gWifiMgrVar.WifiMgrSetting.setting.autoip;

    // ipaddr
    setting.ipaddr[0] = gWifiMgrVar.WifiMgrSetting.setting.ipaddr[0];
    setting.ipaddr[1] = gWifiMgrVar.WifiMgrSetting.setting.ipaddr[1];
    setting.ipaddr[2] = gWifiMgrVar.WifiMgrSetting.setting.ipaddr[2];
    setting.ipaddr[3] = gWifiMgrVar.WifiMgrSetting.setting.ipaddr[3];

    // netmask
    setting.netmask[0] = gWifiMgrVar.WifiMgrSetting.setting.netmask[0];
    setting.netmask[1] = gWifiMgrVar.WifiMgrSetting.setting.netmask[1];
    setting.netmask[2] = gWifiMgrVar.WifiMgrSetting.setting.netmask[2];
    setting.netmask[3] = gWifiMgrVar.WifiMgrSetting.setting.netmask[3];

    // gateway
    setting.gw[0] = gWifiMgrVar.WifiMgrSetting.setting.gw[0];
    setting.gw[1] = gWifiMgrVar.WifiMgrSetting.setting.gw[1];
    setting.gw[2] = gWifiMgrVar.WifiMgrSetting.setting.gw[2];
    setting.gw[3] = gWifiMgrVar.WifiMgrSetting.setting.gw[3];

    printf("[WIFIMGR] ssid     = %s\r\n", ssid);
    printf("[WIFIMGR] password = %s\r\n", password);
    printf("[WIFIMGR] secumode = %s\r\n", secumode);

    if (gWifiMgrVar.Cancel_Connect) {
        goto end;
    }

    if (strlen(ssid) == 0)
    {
        printf("[WIFIMGR]%s() L#%ld: Error! Wifi setting has no SSID\r\n", __FUNCTION__, __LINE__);
        nRet = WIFIMGR_ECODE_NO_SSID;
        goto end;
    }

#if defined(CFG_NET_ETHERNET) && defined(CFG_NET_WIFI)
    printf("[WIFIMGR] check wifi netif %d \n",ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_NETIF_STATUS, NULL));
    // Check if the wifi netif is exist
    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_NETIF_STATUS, NULL) == 0) {
        printf("[WIFIMGR]%s() L#%ld: wifi need to add netif !\r\n", __FUNCTION__, __LINE__);
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_ADD_NETIF, NULL);
    }
#endif

    memset(&netCfg, 0, sizeof(struct net_device_config));

    if (!strcmp(secumode, ITE_WIFI_SEC_UNKNOWN))
    {
        int i = 0, found1 = 0;

        wifi_conn_state = WIFIMGR_CONNSTATE_SCANNING;
        _WifiMgr_Sta_Scan_Post();
        wifi_conn_state = WIFIMGR_CONNSTATE_CONNECTING;

        phase = WIFI_TRYAP_PHASE_SAME_SSID;
        lastIdx = 0;
        triedSecu = 0;

        if (gWifiMgrVar.Cancel_Connect) {
            goto end;
        }

retry:
        nSecurity = -1;
        found1 = 0;
        for (i = lastIdx; i < apInfo.apCnt; i++)
        {
            is_ssid_match = 0;

            if (phase == WIFI_TRYAP_PHASE_SAME_SSID) {
                // search for the same SSID
                if (!strcmp(ssid, apInfo.apList[i].ssidName)) {
                    is_ssid_match = 1;
                }
            } else if (phase == WIFI_TRYAP_PHASE_EMPTY_SSID) {
                // search for the empty SSID
                if (strlen(apInfo.apList[i].ssidName) == 0) {
                    is_ssid_match = 1;
                }
            }

            if (is_ssid_match)
            {
                nSecurity = (apInfo.apList[i].securityOn) ? apInfo.apList[i].securityMode : WLAN_SEC_NOSEC;
                switch (nSecurity)
                {
                case WLAN_SEC_NOSEC:
                    if ((triedSecu & WIFI_TRIEDSECU_NOSEC) == 0)
                    {
                        triedSecu |= WIFI_TRIEDSECU_NOSEC;
                        found1 = 1;
                    }
                    break;
                case WLAN_SEC_WEP:
                    if ((triedSecu & WIFI_TRIEDSECU_WEP) == 0)
                    {
                        triedSecu |= WIFI_TRIEDSECU_WEP;
                        found1 = 1;
                    }
                    break;
                case WLAN_SEC_WPAPSK:
                    if ((triedSecu & WIFI_TRIEDSECU_WPAPSK) == 0)
                    {
                        triedSecu |= WIFI_TRIEDSECU_WPAPSK;
                        found1 = 1;
                    }
                    break;
                case WLAN_SEC_WPA2PSK:
                    if ((triedSecu & WIFI_TRIEDSECU_WPA2PSK) == 0)
                    {
                        triedSecu |= WIFI_TRIEDSECU_WPA2PSK;
                        found1 = 1;
                    }
                    break;
		case WLAN_SEC_WPAPSK_MIX:
                    if ((triedSecu & WIFI_TRIEDSECU_WPAPSK_MIX) == 0)
                    {
                        triedSecu |= WIFI_TRIEDSECU_WPAPSK_MIX;
                        found1 = 1;
                    }
                    break;
                }
            }

            if (found1)
            {
                printf("[WIFIMGR]%s() Found 1 AP matches! ssid = %s, securityOn = %ld, securityMode = %ld, <%02x:%02x:%02x:%02x:%02x:%02x>\r\n",
                    __FUNCTION__, apInfo.apList[i].ssidName, apInfo.apList[i].securityOn, apInfo.apList[i].securityMode,
                    apInfo.apList[i].apMacAddr[0], apInfo.apList[i].apMacAddr[1], apInfo.apList[i].apMacAddr[2], apInfo.apList[i].apMacAddr[3], apInfo.apList[i].apMacAddr[4], apInfo.apList[i].apMacAddr[5]);
                lastIdx = i + 1;
                break;
            }

            if (gWifiMgrVar.Cancel_Connect)
            {
                goto end;
            }
        }

        if ((triedSecu == WIFI_TRIEDSECU_NOSEC | WIFI_TRIEDSECU_WEP | WIFI_TRIEDSECU_WPAPSK | WIFI_TRIEDSECU_WPA2PSK | WIFI_TRIEDSECU_WPAPSK_MIX) ||
            ((phase == WIFI_TRYAP_PHASE_EMPTY_SSID) && (i == apInfo.apCnt)))
        {
            phase = WIFI_TRYAP_PHASE_FINISH;
        }

        if ((phase == WIFI_TRYAP_PHASE_SAME_SSID) && (i == apInfo.apCnt))
        {
            phase = WIFI_TRYAP_PHASE_EMPTY_SSID;
            lastIdx = 0;
        }

        if (gWifiMgrVar.Cancel_Connect)
        {
            goto end;
        }

        if (found1)
        {
            if (!apInfo.apList[i].securityOn || apInfo.apList[i].securityMode == WLAN_SEC_NOSEC)
            {
                netCfg.operationMode = WLAN_MODE_STA;
                snprintf(netCfg.ssidName, WIFI_SSID_MAXLEN, ssid);
                netCfg.securitySuit.securityMode = WLAN_SEC_NOSEC;
            }
            else if (apInfo.apList[i].securityMode == WLAN_SEC_WEP)
            {
                netCfg.operationMode = WLAN_MODE_STA;
                snprintf(netCfg.ssidName, WIFI_SSID_MAXLEN, ssid);
                netCfg.securitySuit.securityMode = WLAN_SEC_WEP;
                netCfg.securitySuit.authMode = WLAN_AUTH_OPENSYSTEM;
                snprintf(netCfg.securitySuit.wepKeys[0], WIFI_PASSWORD_MAXLEN, password);
                snprintf(netCfg.securitySuit.wepKeys[1], WIFI_PASSWORD_MAXLEN, password);
                snprintf(netCfg.securitySuit.wepKeys[2], WIFI_PASSWORD_MAXLEN, password);
                snprintf(netCfg.securitySuit.wepKeys[3], WIFI_PASSWORD_MAXLEN, password);
                netCfg.securitySuit.defaultKeyId = 0; /*From 0 to 3*/
            }
            else if (apInfo.apList[i].securityMode == WLAN_SEC_WPAPSK)
            {
                netCfg.operationMode = WLAN_MODE_STA;
                snprintf(netCfg.ssidName, WIFI_SSID_MAXLEN, ssid);
                netCfg.securitySuit.securityMode = WLAN_SEC_WPAPSK;
                snprintf(netCfg.securitySuit.preShareKey, WIFI_PASSWORD_MAXLEN, password);
            }
            else if (apInfo.apList[i].securityMode == WLAN_SEC_WPA2PSK)
            {
                netCfg.operationMode = WLAN_MODE_STA;
                snprintf(netCfg.ssidName, WIFI_SSID_MAXLEN, ssid);
                netCfg.securitySuit.securityMode = WLAN_SEC_WPA2PSK;
                snprintf(netCfg.securitySuit.preShareKey, WIFI_PASSWORD_MAXLEN, password);
            }
            else if (apInfo.apList[i].securityMode == WLAN_SEC_WPAPSK_MIX)
            {
                netCfg.operationMode = WLAN_MODE_STA;
                snprintf(netCfg.ssidName, WIFI_SSID_MAXLEN, ssid);
                netCfg.securitySuit.securityMode = WLAN_SEC_WPAPSK_MIX;
                snprintf(netCfg.securitySuit.preShareKey, WIFI_PASSWORD_MAXLEN, password);
            }
        }
        else
        {
            if (phase == WIFI_TRYAP_PHASE_FINISH) {
                printf("[WIFIMGR]%s() L#%ld: Cannot find the same SSID on air. Unknown Security! Cannot connect to WiFi AP!\r\n", __FUNCTION__, __LINE__);
                //ioctl(hLed1, ITP_IOCTL_OFF, NULL);
                nRet = WIFIMGR_ECODE_UNKNOWN_SECURITY;
                goto end;
            } else {
                printf("[WIFIMGR]%s() L#%ld: Cannot find the same SSID on air. Goto retry.\r\n", __FUNCTION__, __LINE__);
                goto retry;
            }
        }
    }
    else if (!strcmp(secumode, ITE_WIFI_SEC_OPEN))
    {
        netCfg.operationMode = WLAN_MODE_STA;
        snprintf(netCfg.ssidName, WIFI_SSID_MAXLEN, ssid);
        netCfg.securitySuit.securityMode = WLAN_SEC_NOSEC;
    }
    else if (!strcmp(secumode, ITE_WIFI_SEC_WEP_PSK))
    {
        netCfg.operationMode = WLAN_MODE_STA;
        snprintf(netCfg.ssidName, WIFI_SSID_MAXLEN, ssid);
        netCfg.securitySuit.securityMode = WLAN_SEC_WEP;
        netCfg.securitySuit.authMode = WLAN_AUTH_OPENSYSTEM;
        snprintf(netCfg.securitySuit.wepKeys[0], WIFI_PASSWORD_MAXLEN, password);
        snprintf(netCfg.securitySuit.wepKeys[1], WIFI_PASSWORD_MAXLEN, password);
        snprintf(netCfg.securitySuit.wepKeys[2], WIFI_PASSWORD_MAXLEN, password);
        snprintf(netCfg.securitySuit.wepKeys[3], WIFI_PASSWORD_MAXLEN, password);
        netCfg.securitySuit.defaultKeyId = 0; /*From 0 to 3*/
    }
    else if (!strcmp(secumode, ITE_WIFI_SEC_WPA_TKIP_PSK))
    {
        netCfg.operationMode = WLAN_MODE_STA;
        snprintf(netCfg.ssidName, WIFI_SSID_MAXLEN, ssid);
        netCfg.securitySuit.securityMode = WLAN_SEC_WPAPSK;
        snprintf(netCfg.securitySuit.preShareKey, WIFI_PASSWORD_MAXLEN, password);
    }
    else if (!strcmp(secumode, ITE_WIFI_SEC_WPA2_AES_PSK))
    {
        netCfg.operationMode = WLAN_MODE_STA;
        snprintf(netCfg.ssidName, WIFI_SSID_MAXLEN, ssid);
        netCfg.securitySuit.securityMode = WLAN_SEC_WPA2PSK;
        snprintf(netCfg.securitySuit.preShareKey, WIFI_PASSWORD_MAXLEN, password);
    }
    else if (!strcmp(secumode, ITE_WIFI_SEC_WPA_WPA2_MIXED))
    {
        netCfg.operationMode = WLAN_MODE_STA;
        snprintf(netCfg.ssidName, WIFI_SSID_MAXLEN, ssid);
        netCfg.securitySuit.securityMode = WLAN_SEC_WPAPSK_MIX;
        snprintf(netCfg.securitySuit.preShareKey, WIFI_PASSWORD_MAXLEN, password);
    } else {
        printf("other secumode %s \n",secumode);
        netCfg.operationMode = WLAN_MODE_STA;
        snprintf(netCfg.ssidName, WIFI_SSID_MAXLEN, ssid);
        netCfg.securitySuit.securityMode = WLAN_SEC_WPAPSK_MIX;
        snprintf(netCfg.securitySuit.preShareKey, WIFI_PASSWORD_MAXLEN, password);
    }

    if (gWifiMgrVar.Cancel_Connect) {
        goto end;
    }

    _WifiMgr_Sta_Connect_Hidden_AP();

    // try to connect to WiFi AP
	if (gWifiMgrVar.Client_On_Off){
        iteWPACtrlConnectNetwork(&netCfg);
	}

    // Wait for connecting...
    printf("[WIFIMGR] Wait for connecting\n");
    connect_cnt = WIFI_CONNECT_COUNT;
	while (!ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_CONNECTED, NULL)){
        putchar('.');
		//printf("[WIFIMGR] Wait for ITP_IOCTL_IS_CONNECTED \n");
		usleep(100*1000);
        if (gWifiMgrVar.Cancel_Connect) {
            printf("cancel connect................... \n");
            goto end;
        }
	}

    while (connect_cnt)
    {
        if (iteWPAConnectState())
        {
            printf("\r\n[WIFIMGR] WiFi AP is connected!\r\n");
            is_connected = 1;
            break;
        }
        putchar('.');
        fflush(stdout);
        connect_cnt--;
        if (connect_cnt == 0)
        {
            printf("\r\n[WIFIMGR]%s() L#%ld: Timeout! Cannot connect to %s!\r\n", __FUNCTION__, __LINE__, ssid);
            break;
        }

        if (gWifiMgrVar.Cancel_Connect)
        {
            goto end;
        }
        if (gWifiMgrVar.WIFI_Terminate) {
            printf("[Wifi mgr]terminate connect stop \n");
            goto end;
        }

        usleep(100000);
    }

    if (gWifiMgrVar.Cancel_Connect)
    {
        goto end;
    }

    if (!is_connected)
    {
        if (!strcmp(secumode, ITE_WIFI_SEC_UNKNOWN) &&
            (phase != WIFI_TRYAP_PHASE_FINISH))
        {
            printf("[WIFIMGR]%s() L#%ld: Error! Cannot connect to WiFi AP! Goto retry.\r\n", __FUNCTION__, __LINE__);
            goto retry;
        }
        else
        {
            printf("[WIFIMGR]%s() L#%ld: Error! Cannot connect to WiFi AP!\r\n", __FUNCTION__, __LINE__);
            //ioctl(hLed1, ITP_IOCTL_OFF, NULL);
            nRet = WIFIMGR_ECODE_CONNECT_ERROR;
            gWifiMgrVar.WifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_CLIENT_MODE_CONNECTING_FAIL);
            goto end;
        }
    }

    if (setting.dhcp) {
        // Wait for DHCP setting...
        printf("[WIFIMGR] Wait for DHCP setting");

    	ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_START_DHCP, NULL);

        connect_cnt = WIFI_CONNECT_DHCP_COUNT;
        while (connect_cnt)
        {
            if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_AVAIL, NULL))
            {
                printf("\r\n[WIFIMGR] DHCP setting OK\r\n");
                dhcp_available = 1;
                break;
            }
            putchar('.');
            fflush(stdout);
            connect_cnt--;
            if (connect_cnt == 0)
            {
                if (!strcmp(secumode, ITE_WIFI_SEC_UNKNOWN) &&
                    (phase != WIFI_TRYAP_PHASE_FINISH))
                {
                    printf("\r\n[WIFIMGR]%s() L#%ld: DHCP timeout! Goto retry.\r\n", __FUNCTION__, __LINE__);
                    goto retry;
                }
                else
                {
                    printf("\r\n[WIFIMGR]%s() L#%ld: DHCP timeout! connect fail!\r\n", __FUNCTION__, __LINE__);
                    nRet = WIFIMGR_ECODE_DHCP_ERROR;
                    goto end;
                }
            }

            if (gWifiMgrVar.Cancel_Connect)
            {
                goto end;
            }

            usleep(100000);
        }

        if (gWifiMgrVar.Cancel_Connect)
        {
            goto end;
        }
    }
    else
    {
        printf("[WIFIMGR] Manual setting IP\n");
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_RESET, &setting);
        dhcp_available = 1;
    }

    if (dhcp_available)
    {
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_GET_INFO, &wifiInfo);
        printf("[WIFIMGR] wifiInfo.active          = %ld\r\n", wifiInfo.active);
        printf("[WIFIMGR] wifiInfo.address         = 0x%X\r\n", wifiInfo.address);
        printf("[WIFIMGR] wifiInfo.address         = %ld.%ld.%ld.%ld\r\n",
                (wifiInfo.address & 0xFF), (wifiInfo.address & 0xFF00) >> 8, (wifiInfo.address & 0xFF0000) >> 16, (wifiInfo.address & 0xFF000000) >> 24);
        printf("[WIFIMGR] wifiInfo.displayName     = %s\r\n", wifiInfo.displayName);
        printf("[WIFIMGR] wifiInfo.hardwareAddress = %02X:%02X:%02X:%02X:%02X:%02X\r\n", wifiInfo.hardwareAddress[0], wifiInfo.hardwareAddress[1], wifiInfo.hardwareAddress[2], wifiInfo.hardwareAddress[3], wifiInfo.hardwareAddress[4], wifiInfo.hardwareAddress[5]);
        printf("[WIFIMGR] wifiInfo.name            = %s\r\n", wifiInfo.name);

        //usleep(1000*1000*5); //workaround random miss frames issue for cisco router

        if (gWifiMgrVar.WifiMgrSetting.wifiCallback)
            gWifiMgrVar.WifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_CONNECTION_FINISH);

        // start dhcp count
        gettimeofday(&tvDHCP1, NULL);

    }

end:

    if (gWifiMgrVar.Cancel_Connect)
    {
        printf("[WIFIMGR]%s() L#%ld: End. Cancel_Connect is set.\r\n", __FUNCTION__, __LINE__);
    }

    return nRet;
}


static int
_WifiMgr_Sta_Connect_Post(void)
{
    int nRet = WIFIMGR_ECODE_OK;

    if (!gWifiMgrVar.WIFI_Init_Ready)
        return WIFIMGR_ECODE_NOT_INIT;

    pthread_mutex_lock(&mutexALWiFi);
    if (wifi_conn_state == WIFIMGR_CONNSTATE_STOP) {
        gWifiMgrVar.Need_Set = false;
        sem_post(&semConnectStart);
    }
    pthread_mutex_unlock(&mutexALWiFi);

    return nRet;
}


static inline void
_WifiMgr_Sta_Entry_Info(char* ssid, char* password, char* secumode)
{
    if (ssid){
        // SSID
        snprintf(gWifiMgrVar.Ssid, WIFI_SSID_MAXLEN, ssid);
    }

    if (password){
        // Password
        snprintf(gWifiMgrVar.Password, WIFI_PASSWORD_MAXLEN, password);
    }

    if (secumode){
        // Security mode
        snprintf(gWifiMgrVar.SecurityMode, WIFI_SECUMODE_MAXLEN, secumode);
    }
}

static inline int
_WifiMgr_Sta_Compare_AP_MAC(int list_1, int list_2)
{
    if (gWifiMgrApList[list_1].apMacAddr != NULL && gWifiMgrApList[list_2].apMacAddr != NULL){
        return (gWifiMgrApList[list_1].apMacAddr[0] == gWifiMgrApList[list_2].apMacAddr[0]) && \
                       (gWifiMgrApList[list_1].apMacAddr[1] == gWifiMgrApList[list_2].apMacAddr[1])&&\
                       (gWifiMgrApList[list_1].apMacAddr[2] == gWifiMgrApList[list_2].apMacAddr[2])&&\
                       (gWifiMgrApList[list_1].apMacAddr[3] == gWifiMgrApList[list_2].apMacAddr[3])&&\
                       (gWifiMgrApList[list_1].apMacAddr[4] == gWifiMgrApList[list_2].apMacAddr[4])&&\
                       (gWifiMgrApList[list_1].apMacAddr[5] == gWifiMgrApList[list_2].apMacAddr[5]);
    } else {
        return -1;
    }
}


static void
_WifiMgr_Sta_List_Swap(int x, int y)
{
    WIFI_MGR_SCANAP_LIST temp;

    memcpy(&temp,&gWifiMgrApList[x],sizeof(WIFI_MGR_SCANAP_LIST));
    memcpy(&gWifiMgrApList[x],&gWifiMgrApList[y],sizeof(WIFI_MGR_SCANAP_LIST));
    memcpy(&gWifiMgrApList[y],&temp,sizeof(WIFI_MGR_SCANAP_LIST));
}


static
void _WifiMgr_Sta_List_Sort_Insert(int size)
{
    int i,j;
    for(i = 0; i < size; i++){
        for(j = i; j > 0; j--){
            if(gWifiMgrApList[j].rfQualityQuant > gWifiMgrApList[j - 1].rfQualityQuant){
                _WifiMgr_Sta_List_Swap(j, j-1);
            }
        }
    }
}


static int
_WifiMgr_Sta_Remove_Same_SSID(int size)
{
    int i,j;
    if (size < 1){
        return size;
    }

    for (i=size-1 ; i>0 ; i--){
        for (j = i ; j >=0 ; j --){
            if (strcmp(gWifiMgrApList[i].ssidName , gWifiMgrApList[j].ssidName)==0 && i!=j){
                //set power =0 , if the same ssid
#if WIFIMGR_REMOVE_ALL_SAME_SSID
                if (gWifiMgrApList[i].rfQualityQuant < gWifiMgrApList[j].rfQualityQuant ? 1:0)
                gWifiMgrApList[i].rfQualityQuant = 0;
                else
                    gWifiMgrApList[j].rfQualityQuant = 0;
#else
                if (_WifiMgr_Sta_Compare_AP_MAC(i, j))
                    gWifiMgrApList[i].rfQualityQuant = 0;
#endif
            }
        }
    }

    for (i = 0 , j =0 ; i < size ; i ++){
        if (gWifiMgrApList[i].rfQualityQuant > 0){
            memcpy(&gWifiMgrTempApList[j],&gWifiMgrApList[i],sizeof(WIFI_MGR_SCANAP_LIST));
            j++;
        }

    }

#if 0
    printf("RemoveSameSsid %d \n",j);
    for (i = 0; i < j; i++)
    {
        printf("[Wifi mgr] ssid = %32s, securityOn = %ld, securityMode = %ld, avgQuant = %d, avgRSSI = %d , <%02x:%02x:%02x:%02x:%02x:%02x>\r\n", gWifiMgrTempApList[i].ssidName, gWifiMgrTempApList[i].securityOn, gWifiMgrTempApList[i].securityMode,gWifiMgrTempApList[i].rfQualityQuant, gWifiMgrTempApList[i].rfQualityRSSI,
        gWifiMgrTempApList[i].apMacAddr[0], gWifiMgrTempApList[i].apMacAddr[1], gWifiMgrTempApList[i].apMacAddr[2], gWifiMgrTempApList[i].apMacAddr[3], gWifiMgrTempApList[i].apMacAddr[4], gWifiMgrTempApList[i].apMacAddr[5]);
    }
    printf("RemoveSameSsid -----\n");
#endif
    memcpy(&gWifiMgrApList[0],&gWifiMgrTempApList[0],sizeof(WIFI_MGR_SCANAP_LIST)*WIFI_SCAN_LIST_NUMBER);
    return j;

}


static int
_WifiMgr_Sta_Scan_Post(void)
{
    int nRet = WIFIMGR_ECODE_OK;
    int nWifiState = 0;
    int i = 0, j = 0, k = 0;
    struct net_device_info tmpApInfo = {0};
    int found1 = 0;

    memset(&apInfo, 0, sizeof(struct net_device_info));

    if (gWifiMgrVar.Cancel_Connect) {
        goto end;
    }

    for (i = 0; i < 5; i++)
    {
        printf("[WIFIMGR]%s() SCAN Round <%ld> ==========================\r\n", __FUNCTION__, i);
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_SCAN, NULL);
        while (1)
        {
            nWifiState = ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_STATE, NULL);
            printf("[WIFIMGR]%s() nWifiState=0x%X\r\n", __FUNCTION__, nWifiState);
            if ((nWifiState & WLAN_SITE_MONITOR) == 0) {
                // scan finish
                printf("[WIFIMGR]%s() Scan AP Finish!\r\n", __FUNCTION__);
                break;
            }

            if (gWifiMgrVar.Cancel_Connect) {
                goto end;
            }

            usleep(1000 * 1000);
        }

        memset(&tmpApInfo, 0, sizeof(struct net_device_info));
        read(ITP_DEVICE_WIFI, &tmpApInfo, (size_t)NULL);
        printf("[WIFIMGR]%s() L#%ld: tmpApInfo.apCnt = %ld\r\n", __FUNCTION__, __LINE__, tmpApInfo.apCnt);
        for (j = 0; j < tmpApInfo.apCnt; j++)
        {
            found1 = 0;
            for (k = 0; k < apInfo.apCnt; k++)
            {
                if (!memcmp(apInfo.apList[k].apMacAddr, tmpApInfo.apList[j].apMacAddr, 6)){
                    found1 = 1;
                    break;
                }
            }

            if (!found1 && apInfo.apCnt < WIFI_SCAN_LIST_NUMBER) {
                memcpy(&apInfo.apList[apInfo.apCnt], &tmpApInfo.apList[j], sizeof(apInfo.apList[0]));
                apInfo.apCnt++;
            }

            if (gWifiMgrVar.Cancel_Connect) {
                goto end;
            }
        }

        if (gWifiMgrVar.Cancel_Connect) {
            goto end;
        }
    }

    if (gWifiMgrVar.Cancel_Connect) {
        goto end;
    }

    printf("[WIFIMGR]%s() L#%ld: apInfo.apCnt = %ld\r\n", __FUNCTION__, __LINE__, apInfo.apCnt);
    for (i = 0; i < apInfo.apCnt; i++)
    {
        printf("[WIFIMGR] ssid = %32s, securityOn = %ld, securityMode = %ld, <%02x:%02x:%02x:%02x:%02x:%02x>\r\n", apInfo.apList[i].ssidName, apInfo.apList[i].securityOn, apInfo.apList[i].securityMode,
            apInfo.apList[i].apMacAddr[0], apInfo.apList[i].apMacAddr[1], apInfo.apList[i].apMacAddr[2], apInfo.apList[i].apMacAddr[3], apInfo.apList[i].apMacAddr[4], apInfo.apList[i].apMacAddr[5]);
    }

end:

    if (gWifiMgrVar.Cancel_Connect) {
        printf("[WIFIMGR WPS]%s() L#%ld: End. Cancel_WPS is set.\r\n", __FUNCTION__, __LINE__);
    }

    return nRet;
}


static int
_WifiMgr_Sta_Scan_Process(struct net_device_info *apInfo)
{
    int nRet = 0;
    int nWifiState = 0;
    int i = 0;
    int nHideSsid = 0;

    memset(apInfo, 0, sizeof(struct net_device_info));

    printf("[Wifi mgr]%s() Start to SCAN AP ==========================\r\n", __FUNCTION__);
    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_SCAN, NULL);
    while (1)
    {
        nWifiState = ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_STATE, NULL);
        //printf("[Presentation]%s() nWifiState=0x%X\r\n", __FUNCTION__, nWifiState);
        if ((nWifiState & WLAN_SITE_MONITOR) == 0)
        {
            // scan finish
            printf("[Wifi mgr]%s() Scan AP Finish!\r\n", __FUNCTION__);
            break;
        }
        usleep(100 * 1000);
    }

    read(ITP_DEVICE_WIFI, apInfo, (size_t)NULL);
    printf("[Wifi mgr]%s() ScanApInfo.apCnt = %ld\r\n", __FUNCTION__, apInfo->apCnt);

    for (i = 0; i < apInfo->apCnt; i++)
    {
        if (strlen(apInfo->apList[i].ssidName)>0)
        {
            gWifiMgrApList[i].channelId = apInfo->apList[i].channelId;
            gWifiMgrApList[i].operationMode = apInfo->apList[i].operationMode ;
            gWifiMgrApList[i].rfQualityQuant = apInfo->apList[i].rfQualityQuant;
            gWifiMgrApList[i].rfQualityRSSI = apInfo->apList[i].rfQualityRSSI;
            gWifiMgrApList[i].securityMode = apInfo->apList[i].securityMode;
            gWifiMgrApList[i].securityOn        = apInfo->apList[i].securityOn;
            memcpy(gWifiMgrApList[i].apMacAddr,apInfo->apList[i].apMacAddr,8);
            memcpy(gWifiMgrApList[i].ssidName,apInfo->apList[i].ssidName,32);
        } else {
            nHideSsid ++;
    }
    }
    apInfo->apCnt = apInfo->apCnt -nHideSsid;

    _WifiMgr_Sta_List_Sort_Insert(apInfo->apCnt);

    apInfo->apCnt = _WifiMgr_Sta_Remove_Same_SSID(apInfo->apCnt);

#if WIFIMGR_SHOW_SCAN_LIST
    for (i = 0; i < apInfo->apCnt; i++)
    {
        printf("[Wifi mgr] ssid = %32s, securityOn = %3s, securityMode = %2ld, avgQuant = %3u %%, avgRSSI = %2d dBm , <%02x:%02x:%02x:%02x:%02x:%02x>\r\n",
        gWifiMgrApList[i].ssidName, gWifiMgrApList[i].securityOn ? "ON":"OFF", WifiMgr_Secu_8188E_To_ITE(gWifiMgrApList[i].securityMode),
        gWifiMgrApList[i].rfQualityQuant, gWifiMgrApList[i].rfQualityRSSI,
        gWifiMgrApList[i].apMacAddr[0], gWifiMgrApList[i].apMacAddr[1], gWifiMgrApList[i].apMacAddr[2],
        gWifiMgrApList[i].apMacAddr[3], gWifiMgrApList[i].apMacAddr[4], gWifiMgrApList[i].apMacAddr[5]);
    }
#endif

    printf("[Wifi mgr]%s() End to SCAN AP ============================\r\n", __FUNCTION__);
    return apInfo->apCnt;
}


static void*
_WifiMgr_Sta_Thread(void* arg)
{
    int nRet = WIFIMGR_ECODE_OK;

    while (1)
    {
        sem_wait(&semConnectStart);

        if (gWifiMgrVar.WIFI_Terminate) {
            printf("[WIFIMGR] Terminate _WifiMgr_Sta_Thread(0) \n");
            break;
        }

        if (gWifiMgrVar.Need_Set){
            wifi_conn_state = WIFIMGR_CONNSTATE_SETTING;
            printf("[WIFIMGR] START to Set!\r\n");
            wifi_conn_ecode = nRet = WIFIMGR_ECODE_OK;

            gWifiMgrVar.Need_Set = false;
            printf("[WIFIMGR] Set finish!\r\n");
        }
        usleep(1000);

        if (nRet == WIFIMGR_ECODE_OK) {
            wifi_conn_state = WIFIMGR_CONNSTATE_CONNECTING;

            printf("[WIFIMGR] START to Connect!\r\n");
            iteWPACtrlWpsCancel();

			/* Wait Wifi turn on at UI */
			while (!gWifiMgrVar.Client_On_Off){
				memset(gWifiMgrVar.Ssid    , 0, sizeof(gWifiMgrVar.Ssid));
				memset(gWifiMgrVar.Password, 0, sizeof(gWifiMgrVar.Password));
				memset(gWifiMgrVar.SecurityMode, 0, sizeof(gWifiMgrVar.SecurityMode));

				usleep(100*1000);
                if(gWifiMgrVar.WIFI_Terminate)
                {
                    printf("[WIFIMGR] Terminate _WifiMgr_Sta_Thread(1) \n");
                    wifi_conn_state = WIFIMGR_CONNSTATE_STOP;
                    goto end;
                }
			}

            gWifiMgrVar.Cancel_WPS = true;
            wifi_conn_ecode = _WifiMgr_Sta_Connect_Process();
            gWifiMgrVar.Cancel_WPS = false;
            printf("[WIFIMGR] Connect finish!\r\n");

        }
        wifi_conn_state = WIFIMGR_CONNSTATE_STOP;
        usleep(1000);
    }
end:
    wifi_conn_state = WIFIMGR_CONNSTATE_STOP;
    return NULL;
}


static int
_WifiMgr_HostAP_Init(void)
{
    int nRet = WIFIMGR_ECODE_OK;
    ITPWifiInfo wifiInfo;

    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFIAP_ENABLE, NULL);
    usleep(1000*1000);

    if (gWifiMgrVar.MP_Mode)
    {
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_GET_INFO, &wifiInfo);
        snprintf(gWifiMgrVar.Mac_String, sizeof(gWifiMgrVar.Mac_String), "_%02X:%02X", wifiInfo.hardwareAddress[4], wifiInfo.hardwareAddress[5]);
        iteHostapdSetSSIDWithMac(gWifiMgrVar.Mac_String);
    }

    dhcps_init();
    usleep(1000*10);

    iteStartHostapdCtrl();

    do
    {
        usleep(1000);
    } while (!iteHOSTAPDCtrlIsReady());

    usleep(1000);

    return nRet;
}


static int
_WifiMgr_HostAP_Terminate(void)
{
    int nRet = WIFIMGR_ECODE_OK;

    iteStopHostapdCtrl();
    do
    {
        usleep(1000*200);
    }while(!iteStopHostapdDone());
    usleep(1000*200);

    dhcps_deinit();
    usleep(1000*10);

    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFIAP_DISABLE, NULL);
    usleep(1000*1000);

    return nRet;
}


int
_WifiMgr_WPS_Connect_Post(void)
{
    int nRet = WIFIMGR_ECODE_OK;

    if (!gWifiMgrVar.WIFI_Init_Ready) {
        return WIFIMGR_ECODE_NOT_INIT;
    }

    pthread_mutex_lock(&mutexALWiFi);
    if (wifi_conn_state == WIFIMGR_CONNSTATE_STOP) {
        gWifiMgrVar.Need_Set = true;
        sem_post(&semWPSStart);
//        sem_post(&semConnectStart);
    }
    pthread_mutex_unlock(&mutexALWiFi);

    return nRet;
}


static int
_WifiMgr_WPS_Init(void)
{
    int nRet = WIFIMGR_ECODE_OK;
    struct net_device_config netCfg = {0};
    unsigned long connect_cnt = 0;
    int is_connected = 0, dhcp_available = 0;
    ITPWifiInfo wifiInfo;
    ITPEthernetSetting setting;

    struct net_device_config wpsNetCfg = {0};
    int len = 0;
    char ssid[WIFI_SSID_MAXLEN];
    char password[WIFI_PASSWORD_MAXLEN];

    if (gWifiMgrVar.Cancel_WPS) {
        goto end;
    }

    netCfg.operationMode = WLAN_MODE_STA;
    memset(netCfg.ssidName, 0, sizeof(netCfg.ssidName));
    netCfg.securitySuit.securityMode = WLAN_SEC_WPS;

    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_CONNECTED, NULL)) {

        iteWPACtrlDisconnectNetwork();
        usleep(1000*100);
        // dhcp
        setting.dhcp = 0;

        // autoip
        setting.autoip = 0;

        // ipaddr
        setting.ipaddr[0] = 0;
        setting.ipaddr[1] = 0;
        setting.ipaddr[2] = 0;
        setting.ipaddr[3] = 0;

        // netmask
        setting.netmask[0] = 0;
        setting.netmask[1] = 0;
        setting.netmask[2] = 0;
        setting.netmask[3] = 0;

        // gateway
        setting.gw[0] = 0;
        setting.gw[1] = 0;
        setting.gw[2] = 0;
        setting.gw[3] = 0;

        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_RESET, &setting);

    }

    iteWPACtrlConnectNetwork(&netCfg);

    if (gWifiMgrVar.Cancel_WPS)
    {
        goto end;
    }

    // Wait for connecting...
    printf("[WIFIMGR WPS] Wait for connecting");
    connect_cnt = WIFI_CONNECT_COUNT;
    while (connect_cnt)
    {
        if (iteWPACtrlWpsIsComplete()) {
            printf("\r\n[WIFIMGR WPS] WiFi AP is connected!\r\n");
            WifiMgr_Sta_Cancel_Connect();
            is_connected = 1;
            break;
        }
        putchar('.');
        fflush(stdout);
        connect_cnt--;
        if (connect_cnt == 0) {
            printf("\r\n[WIFIMGR WPS]%s() L#%ld: Timeout! Cannot connect to WIFI AP!\r\n", __FUNCTION__, __LINE__);
            break;
        }

        if (gWifiMgrVar.Cancel_WPS) {
            goto end;
        }

        usleep(100000);
    }

    if (!is_connected) {
        printf("[WIFIMGR WPS]%s() L#%ld: Error! Cannot connect to WiFi AP!\r\n", __FUNCTION__, __LINE__);
        nRet = WIFIMGR_ECODE_CONNECT_ERROR;
        goto end;
    }

    if (gWifiMgrVar.Cancel_WPS) {
        goto end;
    }

    // Wait for DHCP setting...
    printf("[WIFIMGR WPS] Wait for DHCP setting");
    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_START_DHCP, NULL);
    connect_cnt = WIFI_CONNECT_COUNT;
    while (connect_cnt)
    {
        if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_AVAIL, NULL)) {
            printf("\r\n[WIFIMGR WPS] DHCP setting OK\r\n");
            dhcp_available = 1;
            break;
        }
        putchar('.');
        fflush(stdout);
        connect_cnt--;
        if (connect_cnt == 0) {
            printf("\r\n[WIFIMGR WPS]%s() L#%ld: DHCP timeout! connect fail!\r\n", __FUNCTION__, __LINE__);
            nRet = WIFIMGR_ECODE_DHCP_ERROR;
            goto end;
        }

        if (gWifiMgrVar.Cancel_WPS) {
            goto end;
        }
        usleep(100000);
    }

    if (dhcp_available)
    {
        // Get WPS info
        iteWPACtrlGetNetwork(&wpsNetCfg);
        // trim the " char
        memset(ssid, 0, WIFI_SSID_MAXLEN);
        len = strlen(wpsNetCfg.ssidName);
        memcpy(ssid, wpsNetCfg.ssidName + 1, len - 2);
        memset(password, 0, WIFI_PASSWORD_MAXLEN);
        len = strlen(wpsNetCfg.securitySuit.preShareKey);
        memcpy(password, wpsNetCfg.securitySuit.preShareKey + 1, len - 2);

        printf("[WIFIMGR WPS] WPS Info:\r\n");
        printf("[WIFIMGR WPS] WPS SSID     = %s\r\n", ssid);
        printf("[WIFIMGR WPS] WPS Password = %s\r\n", password);
        printf("[WIFIMGR WPS] WPS Security = %ld\r\n", wpsNetCfg.securitySuit.securityMode);
    }

    end:

    if (gWifiMgrVar.Cancel_WPS)
    {
        printf("[WIFIMGR WPS]%s() L#%ld: End. Cancel_WPS is set.\r\n", __FUNCTION__, __LINE__);
    }

    return nRet;
}


static void*
_WifiMgr_WPS_Thread(void* arg)
{
    int nRet = WIFIMGR_ECODE_OK;

    while (1)
    {
        sem_wait(&semWPSStart);
        if (gWifiMgrVar.WIFI_Terminate) {
            printf("[WIFIMGR] Terminate WifiMgr_WPS_ThreadFunc \n");
            break;
        }

        printf("[WIFIMGR] START to Connect WPS!\r\n");
        wifi_conn_ecode = _WifiMgr_WPS_Init();
        printf("[WIFIMGR] Connect WPS finish!\r\n");


        usleep(1000);
    }

    return NULL;
}


static int
_WifiMgr_WPA_Init(void)
{
    int nRet = WIFIMGR_ECODE_OK;
	struct net_device_config netCfg = {0};

#ifndef CFG_NET_WIFI_WPA
    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_LINK_AP, &netCfg);
#else
    // start wpa state machine
    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_ENABLE, NULL);
    usleep(1000*1000);

    iteStartWPACtrl();
    do
    {
        usleep(1000);
    } while (!iteWPACtrlIsReady());
#endif

    return nRet;
}


static int
_WifiMgr_WPA_Terminate(void)
{
    int nRet = WIFIMGR_ECODE_OK;

    iteWPACtrlTerminate();
    iteWPACtrlDisconnectNetwork();

    printf("WifiMgr_WPA_Terminate \n");
    iteStopWPACtrl();
    do
    {
        usleep(1000*100);
        printf("wait WifiMgr_WPA_Terminate %d %d \n", iteStopWPADone(),iteWPADeinitDone());
    }while(iteStopWPADone()==0 || iteWPADeinitDone()==0);

    usleep(1000*100);

    printf("WifiMgr_WPA_Terminate end %d ,%d  \n",iteStopWPADone(),iteWPADeinitDone());
    return nRet;
}


static void*
_WifiMgr_Main_Process_Thread(void *arg)
{
    int nRet = 0, nPlayState = 0;
    int wifi_mode_now = 0, is_softap_ready = 0;
    int bIsAvail = 0, nWiFiConnState = 0, nWiFiConnEcode = 0;

    int nCheckCnt = WIFIMGR_CHECK_WIFI_MSEC;
    long temp_disconn_time = 0;
    static struct timeval tv1 = {0, 0}, tv2 = {0, 0};
    static struct timeval tv3_temp = {0, 0}, tv4_temp = {0, 0};

    WIFIMGR_CONNSTATE_E connstate = WIFIMGR_CONNSTATE_STOP;

    gWifiMgrVar.Is_First_Connect = true;
    gWifiMgrVar.Is_Temp_Disconnect = false;
    gWifiMgrVar.No_Config_File = false;
    gWifiMgrVar.No_SSID = false;

    usleep(20000);

    while (1)
    {
        nCheckCnt--;
        if (gWifiMgrVar.WIFI_Terminate) {
            printf("[WIFIMGR] Terminate WifiMgr_Process_ThreadFunc \n");
            break;
        }

        usleep(1000);
        if (nCheckCnt == 0) {
            wifi_mode_now = WifiMgr_Get_WIFI_Mode();

            if (wifi_mode_now == WIFIMGR_MODE_SOFTAP) {
#ifdef CFG_NET_WIFI_HOSTAPD
                // Soft AP mode
                if (!gWifiMgrVar.SoftAP_Init_Ready) {
                    is_softap_ready = iteHOSTAPDCtrlIsReady();
                    printf("[WIFIMGR]%s() L#%ld: is_softap_ready=%ld\r\n", __FUNCTION__, __LINE__, is_softap_ready);
                    if (is_softap_ready) {
                        gWifiMgrVar.SoftAP_Init_Ready = true;
                        connstate = WIFIMGR_CONNSTATE_STOP;
                        if (gWifiMgrVar.WifiMgrSetting.wifiCallback)
                            gWifiMgrVar.WifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_CONNECTION_FINISH);
                    }
                }
#endif
            } else {
                // Client mode
                if (gWifiMgrVar.Is_First_Connect) {
                    // first time connect when the system start up
                    nRet = _WifiMgr_Sta_Connect_Post();
                    if (nRet == WIFIMGR_ECODE_OK) {
                        connstate = WIFIMGR_CONNSTATE_CONNECTING;
                    }
                    gWifiMgrVar.Is_First_Connect = false;

                    goto end;
                }

                if (connstate == WIFIMGR_CONNSTATE_SETTING ||
                    connstate == WIFIMGR_CONNSTATE_CONNECTING) {
                    nRet = WifiMgr_Get_Connect_State(&nWiFiConnState, &nWiFiConnEcode);
                    if (nWiFiConnState == WIFIMGR_CONNSTATE_STOP) {
                        connstate = WIFIMGR_CONNSTATE_STOP;
                        // the connecting was finish
                        if (nWiFiConnEcode == WIFIMGR_ECODE_OK) {
                            nRet = WifiMgr_Sta_Is_Available(&bIsAvail);
                            if (!bIsAvail) {
                                // fail, restart the timer
                                gettimeofday(&tv1, NULL);
                            }
                        } else {
                            printf("[WIFIMGR]%s() L#%ld: Error! nWiFiConnEcode = 0%ld\r\n", __FUNCTION__, __LINE__, nWiFiConnEcode);

                            // connection has error
                            if (nWiFiConnEcode == WIFIMGR_ECODE_NO_INI_FILE) {
                                gWifiMgrVar.No_Config_File = true;
                            }
                            if (nWiFiConnEcode == WIFIMGR_ECODE_NO_SSID) {
                                gWifiMgrVar.No_SSID = true;
                            } else {
                                // fail, restart the timer
                                gettimeofday(&tv1, NULL);
                            }
                        }
                    }
                    goto end;
                }

                nRet = WifiMgr_Sta_Is_Available(&bIsAvail);
                nRet = WifiMgr_Get_Connect_State(&nWiFiConnState, &nWiFiConnEcode);

				if ((ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_SLEEP_STATUS, NULL) && gWifiMgrVar.Is_Temp_Disconnect)
					gWifiMgrVar.Is_Temp_Disconnect = false;

                if (bIsAvail){
                    if (gWifiMgrVar.Is_Temp_Disconnect) {
                        gWifiMgrVar.Is_Temp_Disconnect = false;     // reset
                        printf("[WIFIMGR]%s() L#%ld: WiFi auto re-connected!\r\n", __FUNCTION__, __LINE__);
                        if (gWifiMgrVar.WifiMgrSetting.wifiCallback)
                            gWifiMgrVar.WifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_CLIENT_MODE_RECONNECTION);
                    }

                    if (!gWifiMgrVar.Is_WIFI_Available) {
                        // prev is not available, curr is available
                        gWifiMgrVar.Is_WIFI_Available = true;
                        gWifiMgrVar.No_Config_File = false;
                        gWifiMgrVar.No_SSID = false;
                        printf("[WIFIMGR]%s() L#%ld: WiFi auto re-connected!\r\n", __FUNCTION__, __LINE__);
                    }
                    gettimeofday(&tvDHCP2, NULL);
                    if (itpTimevalDiff(&tvDHCP1, &tvDHCP2) > WIFIMGR_DHCP_RENEW_MSEC) {
                        printf("====>Send DHCP Discover!!!!!\n");
                        printf("DHCP renew %d \n", itpTimevalDiff(&tvDHCP1, &tvDHCP2));
                        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_RENEW_DHCP, NULL);
                        gettimeofday(&tvDHCP1, NULL);
                        gettimeofday(&tvDHCP2, NULL);
                    } else {
                   //     printf("DHCP wait renew  %d \n", itpTimevalDiff(&tvDHCP1, &tvDHCP2));
                    }
                } else {
                    if (gWifiMgrVar.Is_WIFI_Available){
                        if (!gWifiMgrVar.Is_Temp_Disconnect  && nWiFiConnEcode == WIFIMGR_ECODE_OK)
                        {
                            // first time detect
                            gWifiMgrVar.Is_Temp_Disconnect = true;
                            gettimeofday(&tv3_temp, NULL);
                            printf("[WIFIMGR]%s() L#%ld: WiFi temporary disconnected!%d %d\r\n", __FUNCTION__, __LINE__,nWiFiConnState,nWiFiConnEcode);
                            if (gWifiMgrVar.WifiMgrSetting.wifiCallback)
                                gWifiMgrVar.WifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_CLIENT_MODE_TEMP_DISCONNECT);
                        } else if (nWiFiConnEcode == WIFIMGR_ECODE_OK){
                            gettimeofday(&tv4_temp, NULL);
                            temp_disconn_time = itpTimevalDiff(&tv3_temp, &tv4_temp);
                            printf("[WIFIMGR]%s() L#%ld: temp disconnect time = %ld sec. %d %d\r\n", __FUNCTION__, __LINE__, temp_disconn_time / 1000 , nWiFiConnState,nWiFiConnEcode);
                            if (temp_disconn_time >= WIFIMGR_TEMPDISCONN_MSEC) {
                                printf("[WIFIMGR]%s() L#%ld: WiFi temporary disconnected over %ld sec. Services should be shut down.\r\n", __FUNCTION__, __LINE__, temp_disconn_time / 1000);
                                gWifiMgrVar.Is_Temp_Disconnect = false;     // reset

                                if (gWifiMgrVar.WifiMgrSetting.wifiCallback)
                                    gWifiMgrVar.WifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_CLIENT_MODE_DISCONNECT_30S);

                                // prev is available, curr is not available
                                gWifiMgrVar.Is_WIFI_Available = false;
                            }
                        }
                    } else {
                        // prev is not available, curr is not available
                        if (gWifiMgrVar.No_Config_File || gWifiMgrVar.No_SSID) {
                            // has no data to connect, skip
                            goto end;
                        }

                        nRet = WifiMgr_Get_Connect_State(&nWiFiConnState, &nWiFiConnEcode);
                        switch (nWiFiConnState)
                        {
                            case WIFIMGR_CONNSTATE_STOP:
                                gettimeofday(&tv2, NULL);
                                if (itpTimevalDiff(&tv1, &tv2) >= WIFIMGR_RECONNTECT_MSEC) {
                                    nRet = _WifiMgr_Sta_Connect_Post();
                                    if (nRet == WIFIMGR_ECODE_OK) {
                                        connstate = WIFIMGR_CONNSTATE_CONNECTING;
                                    }
                                }
                                break;
                            case WIFIMGR_CONNSTATE_SETTING:
                                break;
                            case WIFIMGR_CONNSTATE_CONNECTING:
                                break;
                        }
                    }
                }
            }

    end:
            nCheckCnt = WIFIMGR_CHECK_WIFI_MSEC;
        }
    }
    return NULL;
}


/* ================================================= */




/* ================================================= */
/*
/*                                             Static Functions                                              */
/*
/* ================================================= */



int
WifiMgr_Get_Connect_State(int *conn_state, int *e_code)
{
    int nRet = WIFIMGR_ECODE_OK;

    if (!gWifiMgrVar.WIFI_Init_Ready) {
        *conn_state = 0;
        *e_code = 0;
        return WIFIMGR_ECODE_NOT_INIT;
    }

    pthread_mutex_lock(&mutexALWiFi);
    *conn_state = wifi_conn_state;
    *e_code = wifi_conn_ecode;
    pthread_mutex_unlock(&mutexALWiFi);

    return nRet;
}


int
WifiMgr_Get_MAC_Address(char cMac[6])
{
    ITPWifiInfo wifiInfo;

    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_GET_INFO, &wifiInfo);
    cMac[0] = wifiInfo.hardwareAddress[0];
    cMac[1] = wifiInfo.hardwareAddress[1];
    cMac[2] = wifiInfo.hardwareAddress[2];
    cMac[3] = wifiInfo.hardwareAddress[3];
    cMac[4] = wifiInfo.hardwareAddress[4];
    cMac[5] = wifiInfo.hardwareAddress[5];

    return 0;
}


int
WifiMgr_Get_WIFI_Mode(void)
{
    return gWifiMgrVar.WIFI_Mode;
}


int WifiMgr_Get_Scan_AP_Info(WIFI_MGR_SCANAP_LIST* pList)
{
    int nApCount;
	if(gWifiMgrVar.WIFI_Terminate)
	{
        printf("%s:return ap count = 0 cause of WIFI_Terminate is 1\n", __func__);
		return 0;
	}

    if (!gWifiMgrVar.WIFI_Init_Ready) {
        printf("WifiMgr_Get_Scan_AP_Info  !WIFI_Init_Ready \n ");
        return WIFIMGR_ECODE_NOT_INIT;
    }


    pthread_mutex_lock(&mutexMode);

	if (gWifiMgrVar.Client_On_Off){
        nApCount = _WifiMgr_Sta_Scan_Process(&gScanApInfo);
        memcpy(pList,gWifiMgrApList,sizeof(WIFI_MGR_SCANAP_LIST)*WIFI_SCAN_LIST_NUMBER);
	}

    pthread_mutex_unlock(&mutexMode);

    printf("WifiMgr_Get_Scan_AP_Info %d  \n",nApCount);

    return nApCount;

}


int WifiMgr_Get_HostAP_Ready(void)
{
    return gWifiMgrVar.SoftAP_Init_Ready;
}


int WifiMgr_Get_HostAP_Device_Number(void)
{
    int stacount = 0;
    if (gWifiMgrVar.WIFI_Mode ==WIFIMGR_MODE_SOFTAP) {
        stacount = ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_STANUM, NULL);
        printf("sta number = %d\n",stacount);
    }
	return stacount;
}


int
WifiMgr_Sta_Is_Available(int *is_available)
{
    int nRet = WIFIMGR_ECODE_OK;
    int is_connected = 0, is_avail = 0;

    if (!gWifiMgrVar.WIFI_Init_Ready) {
        *is_available = 0;
        return WIFIMGR_ECODE_NOT_INIT;
    }

    is_connected = ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_CONNECTED, NULL);
    is_avail = ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_AVAIL, NULL);
    *is_available = is_connected && is_avail && iteWPAConnectState();

    return nRet;
}


int
WifiMgr_Sta_Connect(char* ssid, char* password, char* secumode)
{
    int nRet = WIFIMGR_ECODE_OK;

    if (!gWifiMgrVar.WIFI_Init_Ready) {
        return WIFIMGR_ECODE_NOT_INIT;
    }

    _WifiMgr_Sta_Entry_Info(ssid, password, secumode);

    if (wifi_conn_state == WIFIMGR_CONNSTATE_STOP) {
        gWifiMgrVar.Need_Set = false;
        sem_post(&semConnectStart);
    }
    gettimeofday(&tvDHCP1, NULL);
    gettimeofday(&tvDHCP2, NULL);

    return WIFIMGR_ECODE_OK;
}


int
WifiMgr_Sta_Disconnect(void)
{

    ITPEthernetSetting setting;

    int nRet = WIFIMGR_ECODE_OK;

    if (!gWifiMgrVar.WIFI_Init_Ready) {
        return WIFIMGR_ECODE_NOT_INIT;
    }

    if (wifi_conn_state == WIFIMGR_CONNSTATE_CONNECTING){
        WifiMgr_Sta_Cancel_Connect();
	}

	printf("WifiMgr_Sta_Disconnect \n");

    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_CONNECTED, NULL)) {

        iteWPACtrlDisconnectNetwork();
        usleep(1000*100);
        // dhcp
        setting.dhcp = 0;

        // autoip
        setting.autoip = 0;

        // ipaddr
        setting.ipaddr[0] = 0;
        setting.ipaddr[1] = 0;
        setting.ipaddr[2] = 0;
        setting.ipaddr[3] = 0;

        // netmask
        setting.netmask[0] = 0;
        setting.netmask[1] = 0;
        setting.netmask[2] = 0;
        setting.netmask[3] = 0;

        // gateway
        setting.gw[0] = 0;
        setting.gw[1] = 0;
        setting.gw[2] = 0;
        setting.gw[3] = 0;

        //ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_RESET, &setting);
    }

    printf("WifiMgr_Sta_Disconnect end \n");
    wifi_conn_state = WIFIMGR_CONNSTATE_STOP;
    wifi_conn_ecode = WIFIMGR_ECODE_SET_DISCONNECT;
    gettimeofday(&tvDHCP1, NULL);
    gettimeofday(&tvDHCP2, NULL);

    usleep(1000*100);
    WifiMgr_Sta_Not_Cancel_Connect();
    return WIFIMGR_ECODE_OK;
}


int
WifiMgr_Sta_Sleep_Disconnect(void)
{
    ITPEthernetSetting setting;

    int nRet = WIFIMGR_ECODE_OK;

    if (!gWifiMgrVar.WIFI_Init_Ready) {
        return WIFIMGR_ECODE_NOT_INIT;
    }

	printf("WifiMgr_Sta_Sleep_Disconnect \n");
    wifi_conn_state = WIFIMGR_CONNSTATE_STOP;
    wifi_conn_ecode = WIFIMGR_ECODE_SET_DISCONNECT;
    usleep(1000*100);

    return WIFIMGR_ECODE_OK;
}


void
WifiMgr_Sta_Switch(int status)
{
	gWifiMgrVar.Client_On_Off = status;
}


void
WifiMgr_Sta_Cancel_Connect(void)
{
    gWifiMgrVar.Cancel_Connect = true;
}


void
WifiMgr_Sta_Not_Cancel_Connect(void)
{
    gWifiMgrVar.Cancel_Connect = false;
}


int
WifiMgr_Sta_WPA_Terminate_Status (void)
{
    return (gWifiMgrVar.WIFI_Terminate || gWifiMgrVar.WPA_Terminate);
}


int
WifiMgr_Secu_8188E_To_ITE(int rtw_security_8188eu)
{
    static int ite_security_enum;

    /* Translate 8188eu WIFI security enum to 8189eu WIFI security enum*/
    if (rtw_security_8188eu == WLAN_SEC_NOSEC)
        ite_security_enum = 0;
    else if (rtw_security_8188eu == WLAN_SEC_WEP)
        ite_security_enum = 1;
    else if (rtw_security_8188eu == WLAN_SEC_WPAPSK)
        ite_security_enum = 2;
    else if (rtw_security_8188eu == WLAN_SEC_WPAPSK_AES)
        ite_security_enum = 3;
    else if (rtw_security_8188eu == WLAN_SEC_WPA2PSK)
        ite_security_enum = 4;
    else if (rtw_security_8188eu == WLAN_SEC_WPA2PSK_TKIP)
        ite_security_enum = 5;
    else if (rtw_security_8188eu == WLAN_SEC_WPAPSK_MIX)
        ite_security_enum = 7;
    else if (rtw_security_8188eu == WLAN_SEC_WPS)
        ite_security_enum = 9;
    else if (rtw_security_8188eu == WLAN_SEC_EAP)
        ite_security_enum = 10;
    else
        ite_security_enum = -1;

    //printf("WifiMgr_Secu_8188EU_To_ITE: 8189EU(%d) -> ITE(%d)\n", rtw_security_8188eu, ite_security_enum);

    return ite_security_enum;
}


int
WifiMgr_Init(WIFIMGR_MODE_E init_mode, int mp_mode, WIFI_MGR_SETTING wifiSetting)
{
    int nRet = WIFIMGR_ECODE_OK;
    pthread_attr_t attr, attr1, attr2;

    printf("[WIFIMGR] WifiMgr will initial %s Mode\n", (init_mode == WIFIMGR_MODE_CLIENT) ? "Client" :
        (init_mode == WIFIMGR_MODE_SOFTAP) ? "HostAP(SoftAP)" : "Unknown");

    while(gWifiMgrVar.WIFI_Terminate){
         printf("[WIFIMGR] WifiMgr not finished yet \n");
         usleep(200*1000);
    }

    wifi_conn_state         = WIFIMGR_CONNSTATE_STOP;
    wifi_conn_ecode         = WIFIMGR_ECODE_SET_DISCONNECT;
    gWifiMgrVar.Need_Set    = false;
    gWifiMgrVar.MP_Mode     = mp_mode;

    gWifiMgrVar.WifiMgrSetting.wifiCallback = wifiSetting.wifiCallback;

    if (gWifiMgrVar.WIFI_Mode == WIFIMGR_MODE_CLIENT){
        _WifiMgr_Sta_Entry_Info(wifiSetting.ssid, wifiSetting.password, wifiSetting.secumode);
    }

    // default select dhcp
    gWifiMgrVar.WifiMgrSetting.setting.dhcp = 1;
    if (wifiSetting.setting.ipaddr[0]>0){
        memcpy(&gWifiMgrVar.WifiMgrSetting.setting,&wifiSetting.setting,sizeof(ITPEthernetSetting));
    }

    // Check if the wifi driver is exist
    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_GET_NET_DEVICE, NULL) == 0) {
        // has no wifi driver!
        printf("[WIFIMGR]%s() L#%ld: Error! Has no WiFi driver!!\r\n", __FUNCTION__, __LINE__);
        nRet = WIFIMGR_ECODE_NO_WIFI_DONGLE;
        goto end;
    }

    // Check if the wifi dongle is exist
    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_DEVICE_READY, NULL) == 0) {
        // has no wifi device!
        printf("[WIFIMGR]%s() L#%ld: Error! Has no WiFi device!!\r\n", __FUNCTION__, __LINE__);
        nRet = WIFIMGR_ECODE_NO_WIFI_DONGLE;
        goto end;
    }

    // init semaphore
    nRet = sem_init(&semConnectStart, 0, 0);
    if (nRet == -1) {
        printf("[WIFIMGR] ERROR, semConnectStart sem_init() fail!\r\n");
        nRet = WIFIMGR_ECODE_SEM_INIT;
        goto err_end;
    }

    nRet = sem_init(&semConnectStop, 0, 0);
    if (nRet == -1) {
        printf("[WIFIMGR] ERROR, semConnectStop sem_init() fail!\r\n");
        nRet = WIFIMGR_ECODE_SEM_INIT;
        goto err_end;
    }

    nRet = sem_init(&semWPSStart, 0, 0);
    if (nRet == -1) {
        printf("[WIFIMGR] ERROR, semWPSStart sem_init() fail!\r\n");
        nRet = WIFIMGR_ECODE_SEM_INIT;
        goto err_end;
    }

    nRet = sem_init(&semWPSStop, 0, 0);
    if (nRet == -1) {
        printf("[WIFIMGR] ERROR, semWPSStop sem_init() fail!\r\n");
        nRet = WIFIMGR_ECODE_SEM_INIT;
        goto err_end;
    }

    // init mutex
    nRet = pthread_mutex_init(&mutexALWiFi, NULL);
    if (nRet != 0) {
        printf("[WIFIMGR] ERROR, mutexALWiFi pthread_mutex_init() fail! nRet = %ld\r\n", nRet);
        nRet = WIFIMGR_ECODE_MUTEX_INIT;
        goto err_end;
    }

    nRet = pthread_mutex_init(&mutexALWiFiWPS, NULL);
    if (nRet != 0) {
        printf("[WIFIMGR] ERROR, mutexALWiFiWPS pthread_mutex_init() fail! nRet = %ld\r\n", nRet);
        nRet = WIFIMGR_ECODE_MUTEX_INIT;
        goto err_end;
    }

    nRet = pthread_mutex_init(&mutexIni, NULL);
    if (nRet != 0) {
        printf("[WIFIMGR] ERROR, mutexIni pthread_mutex_init() fail! nRet = %ld\r\n", nRet);
        nRet = WIFIMGR_ECODE_MUTEX_INIT;
        goto err_end;
    }

    nRet = pthread_mutex_init(&mutexMode, NULL);
    if (nRet != 0) {
        printf("[WIFIMGR] ERROR, mutexMode pthread_mutex_init() fail! nRet = %ld\r\n", nRet);
        nRet = WIFIMGR_ECODE_MUTEX_INIT;
        goto err_end;
    }

    // create thread
    printf("[WIFIMGR] Start to create thread \n");

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, WIFI_STACK_SIZE);
    pthread_create(&clientModeTask, &attr, _WifiMgr_Sta_Thread, NULL);

    pthread_attr_init(&attr1);
    pthread_attr_setstacksize(&attr1, WIFI_STACK_SIZE);
    pthread_create(&wpsTask, &attr1, _WifiMgr_WPS_Thread, NULL);

    pthread_attr_init(&attr2);
    pthread_attr_setstacksize(&attr2, WIFI_STACK_SIZE);
    pthread_create(&processTask, &attr2, _WifiMgr_Main_Process_Thread, NULL);


    gWifiMgrVar.WIFI_Mode = init_mode;

    if (gWifiMgrVar.WIFI_Mode == WIFIMGR_MODE_SOFTAP){
#ifdef CFG_NET_WIFI_HOSTAPD
		_WifiMgr_HostAP_Init();
#endif
	}else{
		_WifiMgr_WPA_Init();
	}

    WifiMgr_Sta_Not_Cancel_Connect();
    gWifiMgrVar.Cancel_WPS = false;

    gWifiMgrVar.WIFI_Init_Ready = true;
    gWifiMgrVar.WIFI_Terminate = false;
end:
    return nRet;

err_end:
    pthread_mutex_destroy(&mutexMode);
    pthread_mutex_destroy(&mutexIni);
    pthread_mutex_destroy(&mutexALWiFiWPS);
    sem_destroy(&semWPSStop);
    sem_destroy(&semWPSStart);
    pthread_mutex_destroy(&mutexALWiFi);
    sem_destroy(&semConnectStop);
    sem_destroy(&semConnectStart);

    return nRet;
}



int
WifiMgr_Terminate(void)
{
    int nRet = WIFIMGR_ECODE_OK;

    printf("WifiMgr_Terminate \n");
    if (!gWifiMgrVar.WIFI_Init_Ready) {
        return WIFIMGR_ECODE_NOT_INIT;
    }

    gWifiMgrVar.WPA_Terminate = true;
    if (gWifiMgrVar.WIFI_Mode == WIFIMGR_MODE_SOFTAP) {
#ifdef CFG_NET_WIFI_HOSTAPD
        _WifiMgr_HostAP_Terminate();
#endif
    } else {
        gWifiMgrVar.Cancel_WPS = true;
        printf("WifiMgr_Terminate  WifiMgr_WPA_Terminate \n");

        _WifiMgr_WPA_Terminate();
    }

    gWifiMgrVar.WIFI_Terminate = true;
    gWifiMgrVar.SoftAP_Init_Ready = false;
    sem_post(&semWPSStart);
    sem_post(&semConnectStart);

    pthread_join(clientModeTask, NULL);
    pthread_join(wpsTask, NULL);
    pthread_join(processTask, NULL);

    pthread_mutex_destroy(&mutexMode);
    pthread_mutex_destroy(&mutexIni);
    pthread_mutex_destroy(&mutexALWiFiWPS);
    sem_destroy(&semWPSStop);
    sem_destroy(&semWPSStart);
    pthread_mutex_destroy(&mutexALWiFi);
    sem_destroy(&semConnectStop);
    sem_destroy(&semConnectStart);

    gWifiMgrVar.WIFI_Init_Ready = false;
    printf("WifiMgr_Terminate ~~~~~\n");
    gWifiMgrVar.WIFI_Terminate = false;
    gWifiMgrVar.WPA_Terminate = false;
    WifiMgr_Sta_Not_Cancel_Connect();
    gettimeofday(&tvDHCP1, NULL);
    gettimeofday(&tvDHCP2, NULL);

    return nRet;
}


static int gCountT = 0;
static WIFI_MGR_SETTING gWifiSetting;
static  struct timeval gSwitchStart, gSwitchEnd;
static  struct timeval gSwitchCount;

int 
_WifiMgr_Sta_HostAP_Switch_Calculate_Time(void)
{
    if (itpTimevalDiff(&gSwitchCount,&gSwitchEnd)> 1000){
        printf(" %d , %d  \n",itpTimevalDiff(&gSwitchCount,&gSwitchEnd)+(gCountT*1000),itpTimevalDiff(&gSwitchStart,&gSwitchEnd));
        gCountT++;
        gettimeofday(&gSwitchCount, NULL);
    }
}


static void*
_WifiMgr_Sta_HostAP_Switch_Thread(void *arg)
{
    int nTemp;

    printf("WifiMgr_Sta_HostAP_Switch \n");

    if (!gWifiMgrVar.WIFI_Init_Ready) {
        //return WIFIMGR_ECODE_NOT_INIT;
        return NULL;
    }

    gWifiMgrVar.WIFI_Mode = WifiMgr_Get_WIFI_Mode();

    WifiMgr_Terminate();

    gCountT = 0;
    gettimeofday(&gSwitchCount, NULL);

    if (gWifiMgrVar.WIFI_Mode == WIFIMGR_MODE_SOFTAP){
        // init client mode
        printf("WifiMgr_Sta_HostAP_Switch init client  \n");
        nTemp = WifiMgr_Init(WIFIMGR_MODE_CLIENT, 0, gWifiSetting);
    } else {
        // init softap mode
        printf("WifiMgr_Sta_HostAP_Switch init softap  \n");
        do
        {
            usleep(1000*400);
            gettimeofday(&gSwitchEnd, NULL);

            _WifiMgr_Sta_HostAP_Switch_Calculate_Time();
        }while(!iteWPADeinitDone() && (itpTimevalDiff(&gSwitchStart,&gSwitchEnd)<15000));
        printf("WifiMgr_Sta_HostAP_Switch deinit done  \n");

        nTemp = WifiMgr_Init(WIFIMGR_MODE_SOFTAP, 0, gWifiSetting);
    }

    if (gWifiMgrVar.WifiMgrSetting.wifiCallback)
        gWifiMgrVar.WifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_SWITCH_CLIENT_SOFTAP_FINISH);

    return NULL;

}


int WifiMgr_Sta_HostAP_Switch(WIFI_MGR_SETTING wifiSetting)
{

    pthread_t task;
    pthread_attr_t attr;
#ifdef CFG_NET_WIFI_HOSTAPD

#else

    return 0;
#endif
    gettimeofday(&gSwitchStart, NULL);

    memcpy(&gWifiSetting,&wifiSetting,sizeof(WIFI_MGR_SETTING));

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr, WIFI_STACK_SIZE);
    pthread_create(&task, &attr, _WifiMgr_Sta_HostAP_Switch_Thread, NULL);

}


#elif defined(_WIN32)

int WifiMgr_Get_Scan_AP_Info(WIFI_MGR_SCANAP_LIST* pList)
{
    int nApCount = 0;

    return nApCount;
}

int
WifiMgr_Get_Connect_State(int *conn_state, int *e_code)
{
    int nRet = WIFIMGR_ECODE_OK;

    *conn_state = 0;
    *e_code = 0;


    return nRet;
}

int
WifiMgr_Sta_Connect(char* ssid, char* password, char* secumode)
{
    int nRet = WIFIMGR_ECODE_OK;

    return WIFIMGR_ECODE_OK;
}

int WifiMgr_Sta_Disconnect(void)
{
    return WIFIMGR_ECODE_OK;
}

void WifiMgr_Sta_Cancel_Connect(void)
{
    return;
}
#endif

#endif
