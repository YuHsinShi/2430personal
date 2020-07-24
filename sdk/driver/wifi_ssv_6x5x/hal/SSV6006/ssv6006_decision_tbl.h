#ifndef _SSV6006_DECISION_TBL_H_
#define _SSV6006_DECISION_TBL_H_
#include <ssv_types.h>



extern ssv_type_u16 ap_deci_tbl[];
extern ssv_type_u16 sta_deci_tbl[];
extern ssv_type_u16 null_deci_tbl[];
extern ssv_type_u16 generic_deci_tbl[];
extern int ssv6006_hal_sta_rcv_all_bcn(void);
extern int ssv6006_hal_sta_rcv_specific_bcn(void);
extern int ssv6006_hal_sta_reject_bcn(void);
extern int ssv6006_hal_accept_none_wsid_frame(void);
extern int ssv6006_hal_drop_none_wsid_frame(void);
extern int ssv6006_hal_ap_listen_neighborhood(ssv_type_bool en);
#endif //#ifndef _SSV6006_DECISION_TBL_H_