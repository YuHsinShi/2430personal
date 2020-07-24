#ifndef _SSV6030_DATA_FLOW_H_
#define _SSV6030_DATA_FLOW_H_

extern void cabrio_init_ap_mode_data_flow(void);
extern void cabrio_init_sta_mode_data_flow(void);
//extern void set_ap_mode_rx_mgmt_flow_for_acs(void);
//extern void reset_ap_mode_rx_mgmt_flow(void);
extern int ssv6030_hal_acs_rx_mgmt_flow(void);
extern int ssv6030_hal_ap_rx_mgmt_flow(void);
extern int ssv6030_hal_sconfig_rx_data_flow(void);
extern int ssv6030_hal_sta_rx_data_flow(void);

#endif //#ifndef _SSV6030_DATA_FLOW_H_