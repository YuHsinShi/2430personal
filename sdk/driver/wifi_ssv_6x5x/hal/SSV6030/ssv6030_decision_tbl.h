#ifndef _SSV6030_DECISION_TBL_H_
#define _SSV6030_DECISION_TBL_H_
#include <ssv_types.h>

extern const ssv_type_u16 ap_deci_tbl[];
extern const ssv_type_u16 sta_deci_tbl[];
extern int ssv6030_hal_sta_rcv_all_bcn(void);
extern int ssv6030_hal_sta_rcv_specific_bcn(void);
extern int ssv6030_hal_sta_reject_bcn(void);
extern int ssv6030_hal_accept_none_wsid_frame(void);
extern int ssv6030_hal_drop_none_wsid_frame(void);
extern int ssv6030_hal_drop_probe_request(ssv_type_bool IsDrop);
extern int ssv6030_hal_ap_listen_neighborhood(ssv_type_bool en);
#endif //#ifndef _SSV6030_DECISION_TBL_H_