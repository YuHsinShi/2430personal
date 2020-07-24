#include "ite/ith.h"
#include "ite/itp.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "protocol_wrapper.h"
#include "iot_system.h"

#define GET_SYSTEM_STATE                0x00000000
#define GET_SYSTEM_EVENT                0x00000001
#define HOST_SHUTDOWN                   0x00000002
#define GET_SYSTEM_IP                   0x00000003


typedef struct SYSTEM_STATE_RESPONSE_TAG
{
    int     result;
    int     systemState;
}SYSTEM_STATE_RESPONSE;

typedef struct SYSTEM_EVENT_RESPONSE_TAG
{
    int         result;
    uint32_t    systemEvent;
}SYSTEM_EVENT_RESPONSE;


typedef struct SYSTEM_IP_RESPONSE_TAG
{
    int     result;
    uint8_t ip[4];
    uint8_t netmask[4];
    uint8_t gateway[4];
}SYSTEM_IP_RESPONSE;

int IotSystemGetState(void)
{
    int result = 0;
    SYSTEM_STATE_RESPONSE tResponse = { 0 };

    result = IotProtocolRead(GET_SYSTEM_STATE, 0, 0, &tResponse, sizeof(tResponse));

    if (tResponse.result == 0x12345678)
    {
        return tResponse.systemState;
    }
    else
        return -1;
}

int IotSystemGetEvent(void)
{
    int result = 0;
    SYSTEM_EVENT_RESPONSE tResponse = { 0 };
    result = IotProtocolRead(GET_SYSTEM_EVENT, 0, 0, &tResponse, sizeof(tResponse));
    if (tResponse.result == 0x12345678)
    {
        return tResponse.systemEvent;
    }
}

void IotSystemGetIp(SYSTEM_IP* ptIp)
{
    int result = 0;
    SYSTEM_IP_RESPONSE tResponse = { 0 };
    result = IotProtocolRead(GET_SYSTEM_IP, 0, 0, &tResponse, sizeof(tResponse));
    if (tResponse.result == 0x12345678)
    {
        memcpy(ptIp->ip, tResponse.ip, 4);
        memcpy(ptIp->netmask, tResponse.netmask, 4);        
        memcpy(ptIp->gateway, tResponse.gateway, 4);
    }
}

void IotSystemShutdown(void)
{
    int result = 0;
    result = IotProtocolWrite(HOST_SHUTDOWN, NULL, NULL);
}
