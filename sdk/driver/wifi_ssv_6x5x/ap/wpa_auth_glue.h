/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef WPA_AUTH_GLUE_H
#define WPA_AUTH_GLUE_H

int hostapd_setup_wpa(struct hostapd_data *hapd);
void hostapd_reconfig_wpa(struct hostapd_data *hapd);
void hostapd_deinit_wpa(struct hostapd_data *hapd);

#endif /* WPA_AUTH_GLUE_H */
