/** @file
 * ITE IoT Wlan declaration.
 *
 * @author Steven Hsiao
 * @version 1.0
 * @date 2016
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
#ifndef IOT_WLAN_H
#define IOT_WLAN_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum WLAN_AUTH_MODE_TAG{
    WIFI_AUTH_MODE_OPEN = 0,                        /**< Open mode.     */
    WIFI_AUTH_MODE_SHARED,                          /**< Not supported. */
    WIFI_AUTH_MODE_AUTO_WEP,                        /**< Not supported. */
    WIFI_AUTH_MODE_WPA,                             /**< Not supported. */
    WIFI_AUTH_MODE_WPA_PSK,                         /**< WPA_PSK.       */
    WIFI_AUTH_MODE_WPA_None,                        /**< Not supported. */
    WIFI_AUTH_MODE_WPA2,                            /**< Not supported. */
    WIFI_AUTH_MODE_WPA2_PSK,                        /**< WPA2_PSK.      */
    WIFI_AUTH_MODE_WPA_WPA2,                        /**< Not supported. */
    WIFI_AUTH_MODE_WPA_PSK_WPA2_PSK,                /**< Mixture mode.  */
} WLAN_AUTH_MODE;

typedef enum WLAN_ENCRYPT_TYPE_TAG{
    WIFI_ENCRYPT_TYPE_WEP_ENABLED = 0,                                              /**< WEP encryption type.  */
    WIFI_ENCRYPT_TYPE_ENCRYPT1_ENABLED = WIFI_ENCRYPT_TYPE_WEP_ENABLED,             /**< WEP encryption type.  */
    WIFI_ENCRYPT_TYPE_WEP_DISABLED = 1,                                             /**< No encryption.     */
    WIFI_ENCRYPT_TYPE_ENCRYPT_DISABLED = WIFI_ENCRYPT_TYPE_WEP_DISABLED,            /**< No encryption.     */
    WIFI_ENCRYPT_TYPE_WEP_KEY_ABSENT = 2,                                           /**< Not supported.     */
    WIFI_ENCRYPT_TYPE_ENCRYPT_KEY_ABSENT = WIFI_ENCRYPT_TYPE_WEP_KEY_ABSENT,        /**< Not supported.     */
    WIFI_ENCRYPT_TYPE_WEP_NOT_SUPPORTED = 3,                                        /**< Not supported.     */
    WIFI_ENCRYPT_TYPE_ENCRYPT_NOT_SUPPORTED = WIFI_ENCRYPT_TYPE_WEP_NOT_SUPPORTED,  /**< Not supported.     */
    WIFI_ENCRYPT_TYPE_TKIP_ENABLED = 4,                                             /**< TKIP encryption.   */
    WIFI_ENCRYPT_TYPE_ENCRYPT2_ENABLED = WIFI_ENCRYPT_TYPE_TKIP_ENABLED,            /**< TKIP encryption.   */
    WIFI_ENCRYPT_TYPE_AES_ENABLED = 6,                                              /**< AES encryption.    */
    WIFI_ENCRYPT_TYPE_ENCRYPT3_ENABLED = WIFI_ENCRYPT_TYPE_AES_ENABLED,             /**< AES encryption.     */
    WIFI_ENCRYPT_TYPE_AES_KEY_ABSENT = 7,                                           /**< Not supported.     */
    WIFI_ENCRYPT_TYPE_TKIP_AES_MIX = 8,                                             /**< TKIP or AES mix.   */
    WIFI_ENCRYPT_TYPE_ENCRYPT4_ENABLED = WIFI_ENCRYPT_TYPE_TKIP_AES_MIX,            /**< TKIP or AES mix.   */
    WIFI_ENCRYPT_TYPE_TKIP_AES_KEY_ABSENT = 9,                                      /**< Not supported.     */
    WIFI_ENCRYPT_TYPE_GROUP_WEP40_ENABLED = 10,                                     /**< Not supported.     */
    WIFI_ENCRYPT_TYPE_GROUP_WEP104_ENABLED = 11,                                    /**< Not supported.     */
} WLAN_ENCRYPT_TYPE;

typedef struct WLAN_SCAN_ENTRY_TAG
{
    uint8_t     pSsid[32];
    int8_t      ssidSize;
    uint8_t     pBssid[6];
    int8_t      rssi;
    uint16_t    securityInfo;
    int8_t      channel;
    uint8_t     reserved;
} WLAN_SCAN_ENTRY;

typedef struct WLAN_GET_SCAN_RESPONSE_TAG
{
    int                 result;
    int                 validCount;
    WLAN_SCAN_ENTRY     tEntries[32];
} WLAN_GET_SCAN_RESPONSE;

void IotWlanSetConnection(uint8_t* pSsid, uint32_t ssidSize, uint8_t* pKey, uint32_t keySize, int authMode, int encryptType);

void IotWlanStartScan(void);

void IotWlanGetScanResult(WLAN_GET_SCAN_RESPONSE* ptScanResponse);

void IotWlanStopScan();

#ifdef __cplusplus
}
#endif

#endif /* IOT_WLAN_H */
/** @} */ // end of IOT_WLAN_H
