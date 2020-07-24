/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef EAPOL_COMMON_H
#define EAPOL_COMMON_H

/* IEEE Std 802.1X-2004 */
#include "porting.h"
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif /* _MSC_VER */

SSV_PACKED_STRUCT_BEGIN  
struct SSV_PACKED_STRUCT ieee802_1x_hdr {
	ssv_type_u8 version;
	ssv_type_u8 type;
	be16_ssv_type length;
	/* followed by length octets of data */
}SSV_PACKED_STRUCT_STRUCT;
SSV_PACKED_STRUCT_END

#ifdef _MSC_VER
#pragma pack(pop)
#endif /* _MSC_VER */

#define EAPOL_VERSION 2

enum { IEEE802_1X_TYPE_EAP_PACKET = 0,
       IEEE802_1X_TYPE_EAPOL_START = 1,
       IEEE802_1X_TYPE_EAPOL_LOGOFF = 2,
       IEEE802_1X_TYPE_EAPOL_KEY = 3,
       IEEE802_1X_TYPE_EAPOL_ENCAPSULATED_ASF_ALERT = 4
};

enum { EAPOL_KEY_TYPE_RC4 = 1, EAPOL_KEY_TYPE_RSN = 2,
       EAPOL_KEY_TYPE_WPA = 254 };

#endif /* EAPOL_COMMON_H */
