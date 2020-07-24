/** @file
 * ITE IoT System declaration.
 *
 * @author Steven Hsiao
 * @version 1.0
 * @date 2016
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
#ifndef IOT_SYSTEM_H
#define IOT_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#define SYSTEM_WAIT_INIT_STATE              0
#define SYSTEM_AP_DISCONNECT_STATE          1
#define SYSTEM_AP_CONNECTED_STATE           2
#define SYSTEM_WAIT_TERMINATE               3

#define CALL_BUTTON_PRESS_EVENT             (0x1 << 1)
#define SENSOR_DETECT_EVENT                 (0x1 << 2)
#define SHUTDOWN_EVENT                      (0x1 << 3)
#define REMOTE_MONITOR_EVENT                (0x1 << 4)
#define SCAN_DONE_EVENT                     (0x1 << 5)

typedef struct SYSTEM_IP_TAG
{
    uint8_t ip[4];
    uint8_t netmask[4];
    uint8_t gateway[4];
}SYSTEM_IP;

int IotSystemGetState(void);

void IotSystemGetIp(SYSTEM_IP* ptIp);

void IotSystemShutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* IOT_SYSTEM_H */
/** @} */ // end of IOT_SYSTEM_H
