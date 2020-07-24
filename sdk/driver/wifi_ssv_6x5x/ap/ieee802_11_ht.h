/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include "ap_info.h"
#include "ap_sta_info.h"


ssv_type_u8 * hostapd_eid_ht_capabilities(ApInfo_st *pApInfo, ssv_type_u8 *eid);
ssv_type_u8 * hostapd_eid_ht_operation(ApInfo_st *pApInfo, ssv_type_u8 *eid);
ssv_type_u16 copy_sta_ht_capab(ApInfo_st *pApInfo, APStaInfo_st *sta, const ssv_type_u8 *ht_capab);
int hostapd_ht_operation_update(ApInfo_st *pApInfo);
void update_ht_state(ApInfo_st *pApInfo, APStaInfo_st *sta);




