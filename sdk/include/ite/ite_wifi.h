/*
 * Copyright (c) 2012 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * Ethernet Driver API header file.
 *
 * @author James Lin
 * @version 1.0
 */
#ifndef ITE_WIFI_H
#define ITE_WIFI_H


#ifdef __cplusplus
extern "C" {
#endif

#include "wifi/ite_port.h"
#include "openrtos/portmacro.h"


int mmpRtlWifiDriverRegister(void);
int mmpSsvWifiDriverRegister(void);
int mmpRtlWifiDriverCmdTest(void);
int mmpSsvWifiDriverCmdTest(void);
int mmpRtlWifiDriverNetlinkrecvfrom(char* buf, int size);


void WifiConnect(int type);

void WifiDisconnect(int type);

int iteStartWPACtrl(void);
void iteStopWPACtrl(void);
int iteWPACtrlIsReady(void);
void iteWPACtrlWpsCancel(void);
int iteWPACtrlWpsIsComplete(void);
void iteWPACtrlDisconnectNetwork(void);
void iteWPACtrlTerminate(void);
int iteStopWPADone(void);
int iteWPADeinitDone(void);
int iteWPAConnectState(void);

int iteStartHostapdCtrl(void);
void iteStopHostapdCtrl(void);
void iteStartHostapdWPS(void);
int hostapd_config_write(char* ssid, char* psk);
int iteHOSTAPDCtrlIsReady(void);
int iteStopHostapdDone(void);
void iteGetHostapdSetting(char* ssid, char* psk);


void* iteGetwpa_s(void);

void iteWPACtrlConnectNetwork(struct net_device_config * netDeviceConfig);
void iteWPACtrlGetNetwork(struct net_device_config * netDeviceConfig);


void iteHostapdSetSSIDWithMac(char* macstr);
void tasklet_deinit(void);


//@}

#ifdef __cplusplus
}
#endif

#endif /* ITE_WIFI_H */

