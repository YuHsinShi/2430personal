#include "ite/ith.h"
#include "ite/itp.h"
#include "stdio.h"
#include "string.h"
#include "protocol_wrapper.h"
#include "iot_wlan.h"

#define SET_WLAN_CONNECT                0x10000000
#define SET_WLAN_DISCONNECT             0x10000001
#define GET_WLAN_START_SCAN             0x10000002
#define GET_WLAN_SCAN_RESULT            0x10000003
#define GET_WLAN_STOP_SCAN              0x10000004

typedef struct WLAN_CONNECTION_PARAM_TAG
{
    uint8_t     pSsid[32];
    int8_t      ssidSize;
    uint8_t     pKey[64];
    int8_t      keySize;
    int         authMode;
    int         encryptType;
}WLAN_CONNECTION_PARAM;

void IotWlanSetConnection(uint8_t* pSsid, uint32_t ssidSize, uint8_t* pKey, uint32_t keySize, int authMode, int encryptType)
{
    int result = 0;
    WLAN_CONNECTION_PARAM tConnectParam = { 0 };

    memcpy(tConnectParam.pSsid, pSsid, ssidSize);
    tConnectParam.ssidSize = ssidSize;
    tConnectParam.authMode = authMode;
    tConnectParam.encryptType = encryptType;
    memcpy(tConnectParam.pKey, pKey, keySize);
    tConnectParam.keySize = keySize;
    IotProtocolWrite(SET_WLAN_CONNECT, (uint8_t*) &tConnectParam, sizeof(WLAN_CONNECTION_PARAM));
}


void IotWlanStartScan(void)
{
    int result = 0;
    IotProtocolWrite(GET_WLAN_START_SCAN, NULL, NULL);
}

void IotWlanGetScanResult(WLAN_GET_SCAN_RESPONSE* ptScanResponse)
{
    int result = 0;
    IotProtocolRead(GET_WLAN_SCAN_RESULT, 0, 0, (uint8_t*) ptScanResponse, sizeof(WLAN_GET_SCAN_RESPONSE));
}

void IotWlanStopScan()
{
    int result = 0;
    IotProtocolWrite(GET_WLAN_STOP_SCAN, NULL, NULL);
}
