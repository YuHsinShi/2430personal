#ifdef CFG_NET_WIFI_SDIO_NGPL
#include <sys/ioctl.h>
#include "lwip/dhcp.h"
#include "wifiMgr.h"
#include "ite/ite_wifi.h"


/* Extern Function */
extern void         dhcps_init(void);
extern void         dhcps_deinit(void);
extern uint8_t*     LwIP_GetIP(struct netif *pnetif);
extern TickType_t   xTaskGetTickCount(void);
extern struct netif xnetif[NET_IF_NUM];


/* Global Variable */
static pthread_t                         ClientModeTask, ProcessTask;

static sem_t                             semConnectStart, semConnectStop;
static pthread_mutex_t                   mutexALWiFi, mutexIni, mutexMode;

static sem_t                             semWPSStart, semWPSStop;
static pthread_mutex_t                   mutexALWiFiWPS;

static WIFIMGR_CONNSTATE_E               wifi_conn_state    = WIFIMGR_CONNSTATE_STOP;
static WIFIMGR_ECODE_E                   wifi_conn_ecode    = WIFIMGR_ECODE_SET_DISCONNECT;

static WIFI_MGR_SCANAP_LIST              gWifiMgrApList[WIFI_SCAN_LIST_NUMBER];

static WIFI_MGR_SETTING                  gWifiMgrSetting    = {0};
static struct net_device_info            gScanApInfo        = {0};

/* WifiMgr flags */
static WIFI_MGR_VAR                      gWifiMgrVar        =
{
    .WIFI_Mode              = WIFIMGR_MODE_CLIENT,
    .MP_Mode                = 0,
    .Need_Set               = false,
    .Pre_Scan               = false,
    .Start_Scan             = false,
    .WIFI_Init_Ready        = false,
    .SoftAP_Hidden          = false,
    .SoftAP_Init_Ready      = false,
    .WIFI_Terminate         = false,
    .WPA_Terminate          = false,
    .Cancel_Connect         = false,
    .Cancel_WPS             = false,
    .Is_WIFI_Available      = false
};


/* ================================================= */
/*
/* Static Functions */
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


// sta connect to ap
static int
_WifiMgr_Sta_Connect_Process(void)
{
    ITPEthernetSetting setting;
    unsigned long tick1 = xTaskGetTickCount();
    unsigned long tick2, tick3;

    int nRet = WIFIMGR_ECODE_OK;
    int is_connected = 0, is_ssid_match = 0, dhcp_available = 0;
    int phase = 0, nSecurity = -1, nAPCount = 0;
    unsigned long connect_cnt = 0, retry_connect_cnt = 0, retry_dhcp_cnt = 0;
    char *ssid, *password;
    WIFI_MGR_SCANAP_LIST pList[64];

    rtw_security_t      security_type;
    int 				ssid_len;
    int 				password_len;
    int 				key_id;
    void				*semaphore;

    if (gWifiMgrVar.Cancel_Connect) {
        goto end;
    }

    ssid            = gWifiMgrVar.Ssid;
    password        = gWifiMgrVar.Password;
    security_type   = gWifiMgrVar.SecurityMode;

    if (gWifiMgrVar.MP_Mode) {
        printf("[WIFIMGR] Is MP mode, connect to default SSID.\r\n");
        // SSID
        snprintf(ssid,                  32, "%s", CFG_NET_WIFI_MP_SSID);
        // Password
        snprintf(password, RTW_MAX_PSK_LEN, "%s", CFG_NET_WIFI_MP_PASSWORD);
#ifdef DTMF_DEC_HAS_SECUMODE
        // Security mode
        snprintf(security_type,          8, "%s", CFG_NET_WIFI_MP_SECURITY);
#endif
    }

    if(gWifiMgrVar.Pre_Scan) {
        /* Select SSID after user do scanning on touch panel */
        memcpy(pList, gWifiMgrApList, sizeof(WIFI_MGR_SCANAP_LIST)*WIFI_SCAN_LIST_NUMBER);
        nAPCount = gScanApInfo.apCnt;
    } else {
        /* Get WIFI list while the Wifimgr want to reconnect old SSID */
        if (strcmp(gWifiMgrVar.Ssid, "") != 0)
            nAPCount = WifiMgr_Get_Scan_AP_Info(pList);
        else
            goto end;
    }

    for (int i = 0; i < nAPCount; i++) {
		/* Find the match SSID */
        if (!strcmp(ssid, pList[i].ssidName)) {
            is_ssid_match = 1;
            printf("[%s] Wanna connect to [%s], list is matched [%s(0x%X)]\n",
                __FUNCTION__, ssid, pList[i].ssidName, pList[i].securityMode);
            if (pList[i].securityMode != 0){
                security_type = pList[i].securityMode;
            } else {
                security_type = RTW_SECURITY_OPEN;
            }
            break;
        }
    }
    gWifiMgrVar.SecurityMode = security_type ;

    ssid_len = strlen((const char *)ssid);
    password_len = strlen((const char *)password);
    key_id = 0;
    semaphore = NULL;

retry:

    nRet = wifi_connect(ssid,
    			security_type,
    			password,
    			ssid_len,
    			password_len,
    			key_id,
    			semaphore);
    printf("[%s] wifi_connect results: %d\n", __FUNCTION__, nRet);

    /* If SSID can be found but connection was ERROR, try again. */
    if (is_ssid_match == 1 && nRet == RTW_ERROR && retry_connect_cnt < WIFIMGR_RECONNTECT_TIME){
        retry_connect_cnt++;
        WifiMgr_Sta_Disconnect();
        usleep(500*1000);
        printf("[%s] wifi connect retry %d times(SSID can be found)\n", __FUNCTION__, retry_connect_cnt);
        goto retry;
    } else if (is_ssid_match == 1 && nRet == RTW_TIMEOUT && retry_connect_cnt < WIFIMGR_RECONNTECT_TIME){
        retry_connect_cnt++;
        WifiMgr_Sta_Disconnect();
        usleep(500*1000);
        printf("[%s] wifi connect retry %d times(SSID connect timeout)\n", __FUNCTION__, retry_connect_cnt);
        goto retry;
    } else if (is_ssid_match == 0 && nRet == RTW_ERROR && retry_connect_cnt < WIFIMGR_RECONNTECT_TIME) {
        /* If SSID can NOT be found and connection was ERROR, try again. */
        retry_connect_cnt++;
        printf("[%s] wifi connect retry %d times(SSID can NOT be found)\n", __FUNCTION__, retry_connect_cnt);
        goto retry;
    } else if (nRet == RTW_ERROR && retry_connect_cnt >= WIFIMGR_RECONNTECT_TIME) {
        printf("[%s] wifi connect retry failed, quit this SSID(%s)\n", __FUNCTION__, ssid);
        gWifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_CLIENT_MODE_CONNECTING_FAIL);
        goto end;
    }

    /* Another connection error code */
    if (nRet != RTW_SUCCESS) {
        gWifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_CLIENT_MODE_CONNECTING_FAIL);
        goto end;
    }

    /* Connect OK!! Reset count and go to DHCP phase */
    retry_connect_cnt = 0;

    // dhcp
    setting.dhcp        = gWifiMgrSetting.setting.dhcp;

    // autoip
    setting.autoip      = gWifiMgrSetting.setting.autoip;

    // ipaddr
    setting.ipaddr[0]   = gWifiMgrSetting.setting.ipaddr[0];
    setting.ipaddr[1]   = gWifiMgrSetting.setting.ipaddr[1];
    setting.ipaddr[2]   = gWifiMgrSetting.setting.ipaddr[2];
    setting.ipaddr[3]   = gWifiMgrSetting.setting.ipaddr[3];

    // netmask
    setting.netmask[0]  = gWifiMgrSetting.setting.netmask[0];
    setting.netmask[1]  = gWifiMgrSetting.setting.netmask[1];
    setting.netmask[2]  = gWifiMgrSetting.setting.netmask[2];
    setting.netmask[3]  = gWifiMgrSetting.setting.netmask[3];

    // gateway
    setting.gw[0]       = gWifiMgrSetting.setting.gw[0];
    setting.gw[1]       = gWifiMgrSetting.setting.gw[1];
    setting.gw[2]       = gWifiMgrSetting.setting.gw[2];
    setting.gw[3]       = gWifiMgrSetting.setting.gw[3];

    printf("[WIFIMGR] ssid     = %s\r\n", ssid);
    printf("[WIFIMGR] password = %s\r\n", password);
    printf("[WIFIMGR] security_type = %s(0x%x) \r\n",
        ((security_type == RTW_SECURITY_OPEN ) ? "Open" :
        (security_type == RTW_SECURITY_WEP_PSK ) ? "WEP" :
        (security_type == RTW_SECURITY_WPA_TKIP_PSK ) ? "WPA TKIP" :
        (security_type == RTW_SECURITY_WPA_AES_PSK ) ? "WPA AES" :
        (security_type == RTW_SECURITY_WPA2_AES_PSK ) ? "WPA2 AES" :
        (security_type == RTW_SECURITY_WPA2_TKIP_PSK ) ? "WPA2 TKIP" :
        (security_type == RTW_SECURITY_WPA2_MIXED_PSK ) ? "WPA2 Mixed" :
        (security_type == RTW_SECURITY_WPA_WPA2_MIXED ) ? "WPA/WPA2 AES" : "Unknown"), security_type);

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

    if (gWifiMgrVar.Cancel_Connect) {
        goto end;
    }

    // Wait for connecting...
    printf("[WIFIMGR] Wait for connecting\n");
    if (setting.dhcp) {
        ip_addr_set_zero(&xnetif[0].ip_addr);
        // Wait for DHCP setting...
        printf("[WIFIMGR] Wait for DHCP setting");

        dhcp_start(&xnetif[0]);
        tick3 = xTaskGetTickCount();
        unsigned char *ip = LwIP_GetIP(&xnetif[0]);
        printf("\r\n\nIP set zero.\nGot IP after %dms.\n", (tick3-tick1));

        connect_cnt = WIFI_CONNECT_DHCP_COUNT;
        while (connect_cnt)
        {
            ip = LwIP_GetIP(&xnetif[0]);

            if (ip[0]!=0)
            {
                printf("\r\n[WIFIMGR] DHCP setting OK\r\n");
                dhcp_available = 1;
                gWifiMgrVar.Pre_Scan = false;
                break;
            }
            putchar('.');
            fflush(stdout);
            connect_cnt--;
            if (connect_cnt == 0)
            {
                if ((security_type != RTW_SECURITY_UNKNOWN || security_type != RTW_SECURITY_OPEN) &&
                    retry_dhcp_cnt < WIFIMGR_REOFFER_DHCP_TIME)
                {
                    printf("\r\n[WIFIMGR]%s() L#%ld: DHCP timeout! Goto retry(retry %d times).\r\n", __FUNCTION__, __LINE__, retry_dhcp_cnt);
                    retry_dhcp_cnt++;
                    goto retry;
                }
                else
                {
                    printf("\r\n[WIFIMGR]%s() L#%ld: DHCP timeout! connect fail!\r\n", __FUNCTION__, __LINE__);
                    nRet = WIFIMGR_ECODE_DHCP_ERROR;
                    goto end;
                }
            }

            if (gWifiMgrVar.Cancel_Connect || gWifiMgrVar.WIFI_Terminate)
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
        ioctl(ITP_DEVICE_WIFI_NGPL, ITP_IOCTL_GET_INFO, NULL);
    }

    if (dhcp_available)
    {
        //usleep(1000*1000*5); //workaround random miss frames issue for cisco router

        if (gWifiMgrSetting.wifiCallback)
            gWifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_CONNECTION_FINISH);

        printf("wifi_set_autoreconnect \n");
        wifi_set_autoreconnect(1);

        while (!ioctl(ITP_DEVICE_WIFI_NGPL, ITP_IOCTL_IS_AVAIL, NULL)){
            usleep(100*1000);
        }

        ioctl(ITP_DEVICE_WIFI_NGPL, ITP_IOCTL_GET_INFO, NULL);
    }

end:
    if (gWifiMgrVar.Cancel_Connect)
    {
        printf("[WIFIMGR]%s() L#%ld: End. Cancel_Connect is set.\r\n", __FUNCTION__, __LINE__);

        if (gWifiMgrSetting.wifiCallback)
            gWifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_CLIENT_MODE_CONNECTING_CANCEL);
    }

	/* Sava last time connect info otherwise WIFI can't reconnect for wake up */
    if (gWifiMgrSetting.wifiCallback)
        gWifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_CLIENT_MODE_SLEEP_SAVE_INFO);

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


static inline void
_WifiMgr_Sta_Entry_Info(char* ssid, char* password, unsigned long secumode)
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
        gWifiMgrVar.SecurityMode = secumode;
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


static void
_WifiMgr_Sta_List_Sort_Insert(int size)
{
    int i,j;
    for(i = 0; i < size; i++){
        for(j = i; j > 0; j--){
            if(gWifiMgrApList[j].rfQualityRSSI > gWifiMgrApList[j - 1].rfQualityRSSI){
                _WifiMgr_Sta_List_Swap(j, j-1);
            }
        }
    }
}


static int
_WifiMgr_Sta_Remove_Same_SSID(int size)
{
    int i, j, mac_cmp;
    WIFI_MGR_SCANAP_LIST gWifiMgrTempApList[WIFI_SCAN_LIST_NUMBER];

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
    printf("RemoveSameSsid -----\n\n");
#endif
    memcpy(&gWifiMgrApList[0],&gWifiMgrTempApList[0],sizeof(WIFI_MGR_SCANAP_LIST)*WIFI_SCAN_LIST_NUMBER);
    return j;
}


static void
_WifiMgr_Sta_Scan_Result_Print( rtw_scan_result_t* record )
{
#if 0
    printf( "%s\t ", ( record->bss_type == RTW_BSS_TYPE_ADHOC ) ? "Adhoc" : "Infra" );
    // printf( MAC_FMT, MAC_ARG(record->BSSID.octet) );
    printf( " %d\t ", record->signal_strength );
    printf( " %d\t  ", record->channel );
    printf( " %d\t  ", record->wps_type );
    printf( "%s\t\t ", ( record->security == RTW_SECURITY_OPEN ) ? "Open" :
    ( record->security == RTW_SECURITY_WEP_PSK ) ? "WEP" :
    ( record->security == RTW_SECURITY_WPA_TKIP_PSK ) ? "WPA TKIP" :
    ( record->security == RTW_SECURITY_WPA_AES_PSK ) ? "WPA AES" :
    ( record->security == RTW_SECURITY_WPA2_AES_PSK ) ? "WPA2 AES" :
    ( record->security == RTW_SECURITY_WPA2_TKIP_PSK ) ? "WPA2 TKIP" :
    ( record->security == RTW_SECURITY_WPA2_MIXED_PSK ) ? "WPA2 Mixed" :
    ( record->security == RTW_SECURITY_WPA_WPA2_MIXED ) ? "WPA/WPA2 AES" :
    "Unknown" );

    printf( " %s ", record->SSID.val );
    printf( "\r\n" );
#endif
}


static rtw_result_t
_WifiMgr_Sta_Scan_Result_Handler( rtw_scan_handler_result_t* malloced_scan_result )
{
    static int ApNum = 0;
	unsigned char convert_quant;
    struct net_device_info *apInfo;

    //	netDeviceInfo->apList[i].rfQualityRSSI = iwEvt.u.qual.level;

    if (malloced_scan_result->scan_complete != RTW_TRUE) {
        rtw_scan_result_t* record = &malloced_scan_result->ap_details;
        record->SSID.val[record->SSID.len] = 0; /* Ensure the SSID is null terminated */

        apInfo = malloced_scan_result->user_data;
		/* SSID */
        memcpy(apInfo->apList[apInfo->apCnt].ssidName,record->SSID.val ,32);

		/* MAC */
		memcpy(apInfo->apList[apInfo->apCnt].apMacAddr, record->BSSID.octet, 6*sizeof(unsigned char));

		/* Power Level in dBm*/
        apInfo->apList[apInfo->apCnt].rfQualityRSSI =  record->signal_strength;

		/* Signal Quality */
		if (record->signal_strength <= -100)
			convert_quant = 0;
		else if(record->signal_strength >= -30)
			convert_quant = 100;
		else
			convert_quant = 150 + 1.67*(record->signal_strength);

		apInfo->apList[apInfo->apCnt].rfQualityQuant = convert_quant;

		/* Security */
        apInfo->apList[apInfo->apCnt].securityMode = record->security; //unsigned long

		/* Channel */
        apInfo->apList[apInfo->apCnt].channelId = record->channel ;

        //printf("apCnt %d, %s[%02x:%02x:%02x:%02x:%02x:%02x]\n", apInfo->apCnt, apInfo->apList[apInfo->apCnt].ssidName, MAC_ARG(apInfo->apList[apInfo->apCnt].apMacAddr));
        apInfo->apCnt ++;
        //RTW_API_INFO( "%d\t ", ++ApNum );
        _WifiMgr_Sta_Scan_Result_Print(record);
    } else{
        ApNum = 0;
    }
    gWifiMgrVar.Start_Scan = false;
    return RTW_SUCCESS;
}


static int
_WifiMgr_Sta_Scan_Process(struct net_device_info *apInfo)
{
    int nRet = 0, scan_result;
    int nWifiState = 0;
    int i = 0;
    int nHideSsid = 0;
    if (!gWifiMgrVar.WIFI_Init_Ready) {
        printf("scanWifiAp  !WIFI_Init_Ready \n ");
        return WIFIMGR_ECODE_NOT_INIT;
    }

    memset(apInfo, 0, sizeof(struct net_device_info));

    printf("[Wifi mgr]%s() Start to SCAN AP ==========================\r\n", __FUNCTION__);

	scan_result = wifi_scan_networks(_WifiMgr_Sta_Scan_Result_Handler, apInfo);

    if(scan_result == RTW_SUCCESS){
		gWifiMgrVar.Pre_Scan = true;
	}else{
        printf("\n\rERROR: wifi scan failed, result code(%d).\n", scan_result);
        return WIFIMGR_ECODE_NOT_INIT;
    }

    gWifiMgrVar.Start_Scan = true;

    while (1)
    {
        nWifiState = (int)gWifiMgrVar.Start_Scan;
        //printf("[Presentation]%s() nWifiState=0x%X\r\n", __FUNCTION__, nWifiState);
        if (nWifiState == 0)
        {
            // scan finish
            printf("[Wifi mgr]%s() Scan AP Finish!\r\n", __FUNCTION__);
            break;
        }
        usleep(100 * 1000);
    }

    printf("[Wifi mgr]%s() ScanApInfo.apCnt = %ld\r\n", __FUNCTION__, apInfo->apCnt);

#if 0//WIFIMGR_SHOW_SCAN_LIST
    for (i = 0; i < apInfo->apCnt; i++)
    {
        printf("[Wifi mgr] ssid = %32s (%d), securityOn = %ld, securityMode = 0x%x , avgQuant = %d, avgRSSI = %d , <%02x:%02x:%02x:%02x:%02x:%02x>\r\n",
			apInfo->apList[i].ssidName, strlen(apInfo->apList[i].ssidName),apInfo->apList[i].securityOn, apInfo->apList[i].securityMode,apInfo->apList[i].rfQualityQuant, apInfo->apList[i].rfQualityRSSI,
        apInfo->apList[i].apMacAddr[0], apInfo->apList[i].apMacAddr[1], apInfo->apList[i].apMacAddr[2], apInfo->apList[i].apMacAddr[3], apInfo->apList[i].apMacAddr[4], apInfo->apList[i].apMacAddr[5]);
    }
#endif

    for (i = 0; i < apInfo->apCnt; i++)
    {
        if (strlen(apInfo->apList[i].ssidName)>0)
        {
            gWifiMgrApList[i].channelId = apInfo->apList[i].channelId;
            gWifiMgrApList[i].operationMode = apInfo->apList[i].operationMode ;
            gWifiMgrApList[i].rfQualityQuant = apInfo->apList[i].rfQualityQuant;
            gWifiMgrApList[i].rfQualityRSSI = apInfo->apList[i].rfQualityRSSI;
            gWifiMgrApList[i].securityMode = apInfo->apList[i].securityMode;
            /* For security */
            memcpy(gWifiMgrApList[i].apMacAddr, apInfo->apList[i].apMacAddr, 6);
            memcpy(gWifiMgrApList[i].ssidName,  apInfo->apList[i].ssidName, 32);
        } else {
            nHideSsid ++;
        }
    }

#if WIFIMGR_SHOW_SCAN_LIST
    apInfo->apCnt = apInfo->apCnt -nHideSsid;

    _WifiMgr_Sta_List_Sort_Insert(apInfo->apCnt);

    apInfo->apCnt = _WifiMgr_Sta_Remove_Same_SSID(apInfo->apCnt);

    for (i = 0; i < apInfo->apCnt; i++)
    {
        printf("[Wifi mgr] SSID = %32s, securityMode =  %16s, avgQuant = %d, power = %4d dBm , <%02X:%02X:%02X:%02X:%02X:%02X>\r\n",
			gWifiMgrApList[i].ssidName,
			//gWifiMgrApList[i].securityMode,
			((gWifiMgrApList[i].securityMode == RTW_SECURITY_OPEN ) ? "Open" :
			(gWifiMgrApList[i].securityMode == RTW_SECURITY_WEP_PSK ) ? "WEP" :
			(gWifiMgrApList[i].securityMode == RTW_SECURITY_WPA_TKIP_PSK ) ? "WPA TKIP" :
			(gWifiMgrApList[i].securityMode == RTW_SECURITY_WPA_AES_PSK ) ? "WPA AES" :
			(gWifiMgrApList[i].securityMode == RTW_SECURITY_WPA2_AES_PSK ) ? "WPA2 AES" :
			(gWifiMgrApList[i].securityMode == RTW_SECURITY_WPA2_TKIP_PSK ) ? "WPA2 TKIP" :
			(gWifiMgrApList[i].securityMode == RTW_SECURITY_WPA2_MIXED_PSK ) ? "WPA2 Mixed" :
			(gWifiMgrApList[i].securityMode == RTW_SECURITY_WPA_WPA2_MIXED ) ? "WPA/WPA2 AES" : "Unknown"),
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
        printf("====>ClientModeThreadFunc\n");
        sem_wait(&semConnectStart);

        if (gWifiMgrVar.WIFI_Terminate) {
            printf("[Wifi mgr]terminate _WifiMgr_Sta_Thread(0) \n");
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
        printf("====>nRet: %d\n", nRet);

		if (strcmp(gWifiMgrVar.Ssid, "") == 0)
			nRet = WIFIMGR_ECODE_NO_SSID;
		else
			nRet = WIFIMGR_ECODE_OK;

        if (nRet == WIFIMGR_ECODE_OK) {
            wifi_conn_state = WIFIMGR_CONNSTATE_CONNECTING;

            printf("[WIFIMGR] START to Connect!\r\n");

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


static inline void
_WifiMgr_HostAP_Entry_Info(char* ssid, char* password)
{
    if (ssid){
        // SSID
        snprintf(gWifiMgrVar.ApSsid, WIFI_SSID_MAXLEN, ssid);
    }
    if (password){
        // Password
        snprintf(gWifiMgrVar.ApPassword, WIFI_PASSWORD_MAXLEN, password);
    }
}


static int
_WifiMgr_HostAP_Init(void)
{
    int nRet = WIFIMGR_ECODE_OK;
    ITPWifiInfo wifiInfo;

    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFIAP_ENABLE, NULL);

    if (gWifiMgrVar.SoftAP_Hidden){
        //wifi_start_ap_with_hidden_ssid("AP_Mode",RTW_SECURITY_WPA2_AES_PSK,"12345678",7,8,4);
        wifi_start_ap_with_hidden_ssid(gWifiMgrVar.ApSsid,RTW_SECURITY_WPA2_AES_PSK,gWifiMgrVar.ApPassword,strlen(gWifiMgrVar.ApSsid),strlen(gWifiMgrVar.ApPassword),4);
    } else {
        //wifi_start_ap("AP_Mode",RTW_SECURITY_WPA2_AES_PSK,"12345678",7,8,4);
        wifi_start_ap(gWifiMgrVar.ApSsid,RTW_SECURITY_WPA2_AES_PSK,gWifiMgrVar.ApPassword,strlen(gWifiMgrVar.ApSsid),strlen(gWifiMgrVar.ApPassword),4);
    }
    dhcps_init();

    usleep(1000);

    return nRet;
}


static int
_WifiMgr_HostAP_Terminate(void)
{
    int nRet = WIFIMGR_ECODE_OK;

    dhcps_deinit();
    usleep(1000*10);

    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFIAP_DISABLE, NULL);
    usleep(1000*1000);

    return nRet;
}


static int
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

        usleep(1000*100);
        // dhcp
        setting.dhcp = 0;

        // autoip
        setting.autoip = 0;

        // ipaddr
        setting.ipaddr[0] =0;
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


    if (gWifiMgrVar.Cancel_WPS)
    {
        goto end;
    }

    // Wait for connecting...
    printf("[WIFIMGR WPS] Wait for connecting");
    connect_cnt = WIFI_CONNECT_COUNT;
    while (connect_cnt)
    {
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
        printf("[WIFIMGR WPS]%s() L#%ld: End. gWifiMgrVar.Cancel_WPS is set.\r\n", __FUNCTION__, __LINE__);
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
            printf("[Wifi mgr]terminate WifiMgr_WPS_ThreadFunc \n");
            break;
        }

        printf("[WIFIMGR] START to Connect WPS!\r\n");
        wifi_conn_ecode = _WifiMgr_WPS_Init();
        printf("[WIFIMGR] Connect WPS finish!\r\n");


        usleep(1000);
    }

    return NULL;
}

static void*
_WifiMgr_Main_Process_Thread(void *arg)
{
    int nRet = 0;
    int bIsAvail = 0, nWiFiConnState = 0, nWiFiConnEcode = 0;
    int nPlayState = 0;
    WIFIMGR_CONNSTATE_E gWifi_connstate = WIFIMGR_CONNSTATE_STOP;

    int nCheckCnt = WIFIMGR_CHECK_WIFI_MSEC;
    static struct timeval tv1 = {0, 0}, tv2 = {0, 0};
    static struct timeval tv3_temp = {0, 0}, tv4_temp = {0, 0};
    long temp_disconn_time = 0;
    int wifi_mode_now = 0, is_softap_ready = 1;


    gWifiMgrVar.Is_First_Connect = true;
    gWifiMgrVar.Is_Temp_Disconnect = false;
    gWifiMgrVar.No_Config_File = false;
    gWifiMgrVar.No_SSID = false;

    usleep(20000);
    printf("====>ProcessThreadFunc\n");

    while (1)
    {
        nCheckCnt--;
        if (gWifiMgrVar.WIFI_Terminate) {
            printf("[Wifi mgr]terminate WifiMgr_Process_ThreadFunc \n");
            break;
        }

        usleep(1000);
        if (nCheckCnt == 0) {
            wifi_mode_now = WifiMgr_Get_WIFI_Mode();

            if (wifi_mode_now == WIFIMGR_MODE_SOFTAP){
                // Soft AP mode
                if (!gWifiMgrVar.SoftAP_Init_Ready) {
                    printf("[Main]%s() L#%ld: is_softap_ready=%ld\r\n", __FUNCTION__, __LINE__, is_softap_ready);
                    if (is_softap_ready) {
                        gWifiMgrVar.SoftAP_Init_Ready = true;
                        gWifi_connstate = WIFIMGR_CONNSTATE_STOP;
                        if (gWifiMgrSetting.wifiCallback)
                            gWifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_CONNECTION_FINISH);
                    }
                }
            }
            else
            {
                // Client mode
                if (gWifiMgrVar.Is_First_Connect) {
                    // first time connect when the system start up
                    nRet = _WifiMgr_Sta_Connect_Post();
                    if (nRet == WIFIMGR_ECODE_OK) {
                        gWifi_connstate = WIFIMGR_CONNSTATE_CONNECTING;
                    }
                    gWifiMgrVar.Is_First_Connect = false;

                    goto end;
                }
                if (gWifi_connstate == WIFIMGR_CONNSTATE_SETTING ||
                    gWifi_connstate == WIFIMGR_CONNSTATE_CONNECTING)
                {
                    nRet = WifiMgr_Get_Connect_State(&nWiFiConnState, &nWiFiConnEcode);
                    if (nWiFiConnState == WIFIMGR_CONNSTATE_STOP) {
                        gWifi_connstate = WIFIMGR_CONNSTATE_STOP;
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

                if (bIsAvail)
                {
                    if (gWifiMgrVar.Is_Temp_Disconnect) {
                        gWifiMgrVar.Is_Temp_Disconnect = false;     // reset
                        printf("[WIFIMGR]%s() L#%ld: WiFi auto re-connected!\r\n", __FUNCTION__, __LINE__);
                        if (gWifiMgrSetting.wifiCallback)
                            gWifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_CLIENT_MODE_RECONNECTION);
                    }

                    if (!gWifiMgrVar.Is_WIFI_Available) {
                        // prev is not available, curr is available
                        gWifiMgrVar.Is_WIFI_Available = true;
                        gWifiMgrVar.No_Config_File = false;
                        gWifiMgrVar.No_SSID = false;
                        printf("[WIFIMGR]%s() L#%ld: WiFi auto re-connected!\r\n", __FUNCTION__, __LINE__);
                    }
                } else {
                    if (gWifiMgrVar.Is_WIFI_Available){
                        if (!gWifiMgrVar.Is_Temp_Disconnect && nWiFiConnEcode == WIFIMGR_ECODE_OK)
                        {
                            // first time detect
                            gWifiMgrVar.Is_Temp_Disconnect = true;
                            gettimeofday(&tv3_temp, NULL);
                            printf("[WIFIMGR]%s() L#%ld: WiFi temporary disconnected!%d %d\r\n", __FUNCTION__, __LINE__,nWiFiConnState,nWiFiConnEcode);
                            if (gWifiMgrSetting.wifiCallback)
                                gWifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_CLIENT_MODE_TEMP_DISCONNECT);
                        } else if (nWiFiConnEcode == WIFIMGR_ECODE_OK){
                            gettimeofday(&tv4_temp, NULL);
                            temp_disconn_time = itpTimevalDiff(&tv3_temp, &tv4_temp);
                            printf("[WIFIMGR]%s() L#%ld: temp disconnect time = %ld sec. %d %d\r\n", __FUNCTION__, __LINE__, temp_disconn_time / 1000 , nWiFiConnState,nWiFiConnEcode);
                            if (temp_disconn_time >= WIFIMGR_TEMPDISCONN_MSEC) {
                                printf("[WIFIMGR]%s() L#%ld: WiFi temporary disconnected over %ld sec. Services should be shut down.\r\n", __FUNCTION__, __LINE__, temp_disconn_time / 1000);
                                gWifiMgrVar.Is_Temp_Disconnect = false;     // reset

                                if (gWifiMgrSetting.wifiCallback)
                                    gWifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_CLIENT_MODE_DISCONNECT_30S);

                                // prev is available, curr is not available
                                gWifiMgrVar.Is_WIFI_Available = false;
                            }
                        }
                    }
                    else
                    {
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
                                //nRet = _WifiMgr_Sta_Connect_Post();
                                if (nRet == WIFIMGR_ECODE_OK) {
                                    gWifi_connstate = WIFIMGR_CONNSTATE_CONNECTING;
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

static int
_WifiMgr_Enable_PowerSave(void)
{

    return wifi_enable_powersave();
}


static int
_WifiMgr_Disable_PowerSave(void)
{

    return wifi_disable_powersave();
}


/* ================================================= */




/* ================================================= */
/*
/* Static Functions */
/*
/* ================================================= */


int
WifiMgr_Get_Connect_State(int *conn_state, int *e_code)
{
    int nRet = WIFIMGR_ECODE_OK;

    if (!gWifiMgrVar.WIFI_Init_Ready) {
        *conn_state = 0;
        *e_code = WIFIMGR_ECODE_NOT_INIT;
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
    wifi_get_mac_address(cMac);
    return 0;
}


int
WifiMgr_Get_WIFI_Mode(void)
{
    return gWifiMgrVar.WIFI_Mode;
}


int
WifiMgr_Get_Scan_AP_Info(WIFI_MGR_SCANAP_LIST* pList)
{
    int nApCount;

    if (!gWifiMgrVar.WIFI_Init_Ready) {
        printf("WifiMgr_Get_Scan_AP_Info  !WIFI_Init_Ready \n ");
        return WIFIMGR_ECODE_NOT_INIT;
    }

    pthread_mutex_lock(&mutexMode);


    nApCount = _WifiMgr_Sta_Scan_Process(&gScanApInfo);
    memcpy(pList,gWifiMgrApList,sizeof(WIFI_MGR_SCANAP_LIST)*WIFI_SCAN_LIST_NUMBER);

    pthread_mutex_unlock(&mutexMode);

    printf("WifiMgr_Get_Scan_AP_Info %d  \n",nApCount);

    return nApCount;

}


int
WifiMgr_Get_HostAP_Ready(void)
{
    printf("WifiMgr_Get_HostAP_Ready: %d \n", wifi_is_ready_to_transceive(RTW_AP_INTERFACE));
    return wifi_is_ready_to_transceive(RTW_AP_INTERFACE);
}


int
WifiMgr_Get_HostAP_Device_Number(void)
{
    int stacount = 0;
    int client_number;
    struct {
        int    count;
        rtw_mac_t mac_list[3];
    } client_info;

    wifi_get_associated_client_list(&client_info, sizeof(client_info));

#if 0
    printf("\n\rWifiMgr_Get_WIFI_Mode:");
    printf("\n\r==============================");

    if(client_info.count == 0)
        printf("\n\rClient Num: 0\n\r");
    else
    {
        printf("\n\rClient Num: %d", client_info.count);
        printf("\n\r");
    }
#endif

    return client_info.count;
}


int
WifiMgr_Sta_Is_Available(int* is_available)
{
    int nRet = WIFIMGR_ECODE_OK;
    int is_connected = 0, is_avail = 0;

    if (!gWifiMgrVar.WIFI_Init_Ready) {
        *is_available = 0;
        return WIFIMGR_ECODE_NOT_INIT;
    }

    //is_connected = ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_CONNECTED, NULL);
    nRet = wext_get_ssid(WLAN0_NAME, gWifiMgrVar.Ssid);
    is_avail = ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_AVAIL, NULL);

    if (nRet < 0){
        *is_available  = 0;
    } else {
        *is_available  = nRet;
    }

    return nRet;
}


int
WifiMgr_Sta_Connect(char* ssid, char* password, char* secumode)
{
    int nRet = WIFIMGR_ECODE_OK;
    unsigned long secumode_8189f;

    if (!gWifiMgrVar.WIFI_Init_Ready) {
        return WIFIMGR_ECODE_NOT_INIT;
    }

    secumode_8189f = WifiMgr_Secu_ITE_To_8189F(secumode);

    _WifiMgr_Sta_Entry_Info(ssid, password, secumode_8189f);

    if (wifi_conn_state == WIFIMGR_CONNSTATE_STOP) {
        gWifiMgrVar.Need_Set = false;
        sem_post(&semConnectStart);
    }

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

        nRet = wifi_disconnect();

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


void
WifiMgr_HostAP_Set_Hidden(void)
{
    gWifiMgrVar.SoftAP_Hidden = true;
}

unsigned long WifiMgr_Secu_ITE_To_8189F(char* ite_security_enum)
{
    unsigned long rtw_security_8189ftv;

    /* Translate ITE WIFI security enum to 8189ftv WIFI security enum*/
    if (strcmp(ite_security_enum, ITE_WIFI_SEC_OPEN) == 0)
        rtw_security_8189ftv = RTW_SECURITY_OPEN;
    else if (strcmp(ite_security_enum, ITE_WIFI_SEC_WEP_PSK) == 0)
        rtw_security_8189ftv = RTW_SECURITY_WEP_PSK;
    else if (strcmp(ite_security_enum, ITE_WIFI_SEC_WPA_TKIP_PSK) == 0)
        rtw_security_8189ftv = RTW_SECURITY_WPA_TKIP_PSK;
    else if (strcmp(ite_security_enum, ITE_WIFI_SEC_WPA2_AES_PSK) == 0)
        rtw_security_8189ftv = RTW_SECURITY_WPA2_AES_PSK;
    else if (strcmp(ite_security_enum, ITE_WIFI_SEC_WPA_AES_PSK) == 0)
        rtw_security_8189ftv = RTW_SECURITY_WPA_AES_PSK;
    else if (strcmp(ite_security_enum, ITE_WIFI_SEC_WPA2_TKIP_PSK) == 0)
        rtw_security_8189ftv = RTW_SECURITY_WPA2_TKIP_PSK;
    else if (strcmp(ite_security_enum, ITE_WIFI_SEC_WPA2_MIXED_PSK) == 0)
        rtw_security_8189ftv = RTW_SECURITY_WPA2_MIXED_PSK;
    else if (strcmp(ite_security_enum, ITE_WIFI_SEC_WPA_WPA2_MIXED) == 0)
        rtw_security_8189ftv = RTW_SECURITY_WPA_WPA2_MIXED;
    else if (strcmp(ite_security_enum, ITE_WIFI_SEC_WPS_SECURE) == 0)
        rtw_security_8189ftv = RTW_SECURITY_WPS_SECURE;
    else
        rtw_security_8189ftv = RTW_SECURITY_UNKNOWN;

    printf("WifiMgr_Secu_ITE_To_8189F: ITE(%s) -> 8189FTV(0x%x)\n", ite_security_enum, rtw_security_8189ftv);

    return rtw_security_8189ftv;
}


char* WifiMgr_Secu_8189F_To_ITE(unsigned long rtw_security_8189f)
{
    static char ite_security_enum[3];

    /* Translate 8189ftv WIFI security enum to ITE WIFI security enum*/
    if (rtw_security_8189f == RTW_SECURITY_OPEN)
        strcpy(ite_security_enum, "0");
    else if (rtw_security_8189f == RTW_SECURITY_WEP_PSK)
        strcpy(ite_security_enum, "1");
    else if (rtw_security_8189f == RTW_SECURITY_WPA_TKIP_PSK)
        strcpy(ite_security_enum, "2");
    else if (rtw_security_8189f == RTW_SECURITY_WPA_AES_PSK)
        strcpy(ite_security_enum, "3");
    else if (rtw_security_8189f == RTW_SECURITY_WPA2_AES_PSK)
        strcpy(ite_security_enum, "4");
    else if (rtw_security_8189f == RTW_SECURITY_WPA2_TKIP_PSK)
        strcpy(ite_security_enum, "5");
    else if (rtw_security_8189f == RTW_SECURITY_WPA2_MIXED_PSK)
        strcpy(ite_security_enum, "6");
    else if (rtw_security_8189f == RTW_SECURITY_WPA_WPA2_MIXED)
        strcpy(ite_security_enum, "7");
    else if (rtw_security_8189f == RTW_SECURITY_WPS_OPEN)
        strcpy(ite_security_enum, "8");
    else if (rtw_security_8189f == RTW_SECURITY_WPS_SECURE)
        strcpy(ite_security_enum, "9");
    else
        strcpy(ite_security_enum, "NA");

    printf("WifiMgr_Secu_8189F_To_ITE: 8189FTV(0x%X) -> ITE(%s)\n", rtw_security_8189f, ite_security_enum);

    return ite_security_enum;
}


int
WifiMgr_Init(WIFIMGR_MODE_E init_mode, int mp_mode,WIFI_MGR_SETTING wifiSetting)
{
    int nRet = WIFIMGR_ECODE_OK;
    pthread_attr_t attr, attr1,attr2;

    while(gWifiMgrVar.WIFI_Terminate){
         printf("WifiMgr not finished yet \n");
         usleep(200*1000);
    }

    wifi_conn_state         = WIFIMGR_CONNSTATE_STOP;
    wifi_conn_ecode         = WIFIMGR_ECODE_SET_DISCONNECT;
    gWifiMgrVar.Need_Set    = false;
    gWifiMgrVar.MP_Mode     = mp_mode;

    gWifiMgrSetting.wifiCallback = wifiSetting.wifiCallback;

    if (init_mode ==WIFIMGR_MODE_CLIENT){
        _WifiMgr_Sta_Entry_Info(wifiSetting.ssid, wifiSetting.password, wifiSetting.secumode);
    } else if (init_mode ==WIFIMGR_MODE_SOFTAP){
        printf("[Wifimgr] WIFI start AP: SSID(%s)/PW(%s) \n", wifiSetting.ssid, wifiSetting.password);
        _WifiMgr_HostAP_Entry_Info(wifiSetting.ssid, wifiSetting.password);
        _WifiMgr_HostAP_Init();
    }

    // default select dhcp
    gWifiMgrSetting.setting.dhcp = 1;
    if (wifiSetting.setting.ipaddr[0]>0){
        memcpy(&gWifiMgrSetting.setting,&wifiSetting.setting,sizeof(ITPEthernetSetting));
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
    printf("[WifiMgr] Create thread \n");

    _WifiMgr_Create_Worker_Thread(ClientModeTask, _WifiMgr_Sta_Thread,          NULL);
    _WifiMgr_Create_Worker_Thread(ProcessTask,    _WifiMgr_Main_Process_Thread, NULL);


    gWifiMgrVar.WIFI_Mode = init_mode;
    printf("[WIFIMGR] %s() L#%ld: WIFI Mode = %ld\r\n", __FUNCTION__, __LINE__, gWifiMgrVar.WIFI_Mode);

    WifiMgr_Sta_Not_Cancel_Connect();
    gWifiMgrVar.Cancel_WPS      = false;

    gWifiMgrVar.WIFI_Init_Ready = true;
    gWifiMgrVar.WIFI_Terminate  = false;
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



int WifiMgr_Terminate(void)
{
    int nRet = WIFIMGR_ECODE_OK;

    printf("WifiMgr_Terminate \n");
    if (!gWifiMgrVar.WIFI_Init_Ready) {
        return WIFIMGR_ECODE_NOT_INIT;
    }

    gWifiMgrVar.WPA_Terminate = true;
    if (gWifiMgrVar.WIFI_Mode == WIFIMGR_MODE_SOFTAP) {
        dhcps_deinit();
        usleep(1000*10);

        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFIAP_DISABLE, NULL);
        usleep(1000*1000);

    } else {
        //client mode: clean "gWifiSetting" at network.c
        if (gWifiMgrSetting.wifiCallback && (gWifiMgrVar.Client_On_Off == WIFIMGR_SWITCH_OFF))
            gWifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_CLIENT_MODE_SLEEP_CLEAN_INFO);

        gWifiMgrVar.Cancel_WPS = true;
    }

    gWifiMgrVar.WIFI_Terminate = true;
    //wifi_off();

    sem_post(&semWPSStart);
    sem_post(&semConnectStart);

    pthread_join(ClientModeTask, NULL);
    pthread_join(ProcessTask, NULL);

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
    gWifiMgrVar.SoftAP_Hidden = false;
    WifiMgr_Sta_Not_Cancel_Connect();

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
        wifi_off();
        sleep(1);
        wifi_on(RTW_MODE_STA);

        nTemp = WifiMgr_Init(WIFIMGR_MODE_CLIENT, 0, gWifiSetting);
    } else {
        // init softap mode
		printf("WifiMgr_Sta_HostAP_Switch init softap  \n");
		wifi_off();
		sleep(1);
		wifi_on(RTW_MODE_AP);
		printf("WifiMgr_Sta_HostAP_Switch deinit done  \n");

        nTemp = WifiMgr_Init(WIFIMGR_MODE_SOFTAP, 0, gWifiSetting);
    }

    if (gWifiMgrSetting.wifiCallback)
        gWifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_SWITCH_CLIENT_SOFTAP_FINISH);

    return 0;

}


int
WifiMgr_HostAP_First_Start(void)
{
    printf("WifiMgr_HostAP_First_Start \n");

    wifi_off();
    sleep(1);
    wifi_on(RTW_MODE_AP);
}


int
WifiMgr_Sta_HostAP_Switch(WIFI_MGR_SETTING wifiSetting)
{
    pthread_t task;
    pthread_attr_t attr;

    gettimeofday(&gSwitchStart, NULL);

    memcpy(&gWifiSetting,&wifiSetting,sizeof(WIFI_MGR_SETTING));

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr, WIFI_STACK_SIZE);
    pthread_create(&task, &attr, _WifiMgr_Sta_HostAP_Switch_Thread, NULL);
}


#elif defined(_WIN32)

int
WifiMgr_Get_Scan_AP_Info(WIFI_MGR_SCANAP_LIST* pList)
{
    return 0;
}


int
WifiMgr_Get_Connect_State(int *conn_state, int *e_code)
{
    int nRet = WIFIMGR_ECODE_OK;

    *conn_state = 0;
    *e_code = 0;


    return nRet;
}


int WifiMgr_Sta_Connect_AP(char* ssid, char* password, char* secumode)
{
    return WIFIMGR_ECODE_OK;
}


int WifiMgr_Sta_Disconnect()
{
    return WIFIMGR_ECODE_OK;
}


void WifiMgr_Sta_Cancel_Connect()
{
    return;
}
#endif

