#define SSV6006_DATA_FLOW_C
#include <host_config.h>
#if((CONFIG_CHIP_ID==SSV6006B)||(CONFIG_CHIP_ID==SSV6006C))
#include <rtos.h>
#include <log.h>
#include <cmd_def.h>
#include <ssv_dev.h>
#include "ssv6006_data_flow.h"
#include "ssv6006_hal.h"


#define CMD_FLOW_TO_HW(data,a,b,c,d,e,f,g,h)  data[0]=(a&0xF)|((b&0xF)<<4)|((c&0xF)<<8)|((d&0xF)<<12)|((e&0xF)<<16)|((f&0xF)<<20)|((g&0xF)<<24)|((h&0xF)<<28)
#define CMD_FLOW_TO_MCU(data,a,b,c,d,e,f,g,h) data[1]=(a&0xF)|((b&0xF)<<4)|((c&0xF)<<8)|((d&0xF)<<12)|((e&0xF)<<16)|((f&0xF)<<20)|((g&0xF)<<24)|((h&0xF)<<28)

#if(SW_8023TO80211==1)
ssv_type_u32 g_ap_tx_data_flow=0;
ssv_type_u32 g_ap_tx_mgmt_flow=0;
ssv_type_u32 g_sta_tx_data_flow=0;
#endif
extern ssv_type_s32 _ssv6xxx_wifi_ioctl_Ext(ssv_type_u32 cmd_id, void *data, ssv_type_u32 len, ssv_type_bool blocking,const ssv_type_bool mutexLock);

ssv_inline static void init_ap_tx_data_flow(void)
{
    unsigned int cmd_data[2];

    CMD_FLOW_TO_HW(cmd_data,M_ENG_HWHCI, M_ENG_MIC, M_ENG_ENCRYPT, M_ENG_CPU, M_ENG_TX_EDCA0, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU);
    CMD_FLOW_TO_MCU(cmd_data,M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_EDCATX, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG);

#if(SW_8023TO80211==1)
    g_ap_tx_data_flow=cmd_data[0];
#endif
    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_TXDATA, (void *)cmd_data, 8, TRUE, FALSE);
}

ssv_inline static void init_ap_tx_mgmt_flow(void)
{
    unsigned int cmd_data[2];

    CMD_FLOW_TO_HW(cmd_data,M_ENG_HWHCI, M_ENG_CPU, M_ENG_ENCRYPT, M_ENG_TX_MNG, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU);
    CMD_FLOW_TO_MCU(cmd_data,M_CPU_HWENG, M_CPU_EDCATX, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG);

#if(SW_8023TO80211==1)
    g_ap_tx_mgmt_flow=cmd_data[0];
#endif
    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_TXMGMT, (void *)cmd_data, 8, TRUE, FALSE);
}

ssv_inline static void init_ap_tx_ctrl_flow(void)
{
    unsigned int cmd_data[2];

    CMD_FLOW_TO_HW(cmd_data,M_ENG_HWHCI, M_ENG_CPU, M_ENG_TX_MNG, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU);
    CMD_FLOW_TO_MCU(cmd_data,M_CPU_HWENG, M_CPU_EDCATX, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG);

    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_TXCTRL, (void *)cmd_data, 8, TRUE, FALSE);
}

ssv_inline static void init_ap_rx_data_flow(void)
{
    //We can trap into CPU many times, but the hw moudle that is before HCI must be CPU
    unsigned int cmd_data[2];
    CMD_FLOW_TO_HW(cmd_data,M_ENG_MACRX, M_ENG_ENCRYPT_SEC, M_ENG_MIC_SEC, M_ENG_CPU, M_ENG_HWHCI, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU);
    CMD_FLOW_TO_MCU(cmd_data,M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_RXDATA, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG);

    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_RXDATA, (void *)cmd_data, 8, TRUE, FALSE);
}

ssv_inline static void init_ap_rx_mgmt_flow(void)
{
    //We can trap into CPU many times, but the hw moudle that is before HCI must be CPU
    unsigned int cmd_data[2];
    CMD_FLOW_TO_HW(cmd_data,M_ENG_MACRX, M_ENG_CPU, M_ENG_HWHCI, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU);
    CMD_FLOW_TO_MCU(cmd_data,M_CPU_HWENG, M_CPU_RXMGMT, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG);

    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_RXMGMT, (void *)cmd_data, 8, TRUE, FALSE);
}

ssv_inline static void init_ap_rx_ctrl_flow(void)
{
    //We can trap into CPU many times, but the hw moudle that is before HCI must be CPU
    unsigned int cmd_data[2];

    CMD_FLOW_TO_HW(cmd_data,M_ENG_MACRX, M_ENG_CPU, M_ENG_HWHCI, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU);
    CMD_FLOW_TO_MCU(cmd_data,M_CPU_HWENG, M_CPU_RXCTRL, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG);

    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_RXCTRL, (void *)cmd_data, 8, TRUE, FALSE);

}

ssv_inline static void init_sta_tx_data_flow(void)
{
    unsigned int cmd_data[2];

    CMD_FLOW_TO_HW(cmd_data,M_ENG_HWHCI, M_ENG_MIC, M_ENG_ENCRYPT, M_ENG_CPU, M_ENG_TX_EDCA0, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU);
    CMD_FLOW_TO_MCU(cmd_data,M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_EDCATX, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG);

#if(SW_8023TO80211==1)
    g_sta_tx_data_flow=cmd_data[0];
#endif
    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_TXDATA, (void *)cmd_data, 8, TRUE, FALSE);
}

ssv_inline static void init_sta_tx_mgmt_flow(void)
{
    unsigned int cmd_data[2];

    CMD_FLOW_TO_HW(cmd_data,M_ENG_CPU, M_ENG_ENCRYPT, M_ENG_CPU, M_ENG_TX_MNG, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU);
    CMD_FLOW_TO_MCU(cmd_data,M_CPU_HWENG, M_CPU_HWENG, M_CPU_EDCATX, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG);

    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_TXMGMT, (void *)cmd_data, 8, TRUE, FALSE);
}

ssv_inline static void init_sta_tx_ctrl_flow(void)
{
    unsigned int cmd_data[2];

    CMD_FLOW_TO_HW(cmd_data,M_ENG_CPU, M_ENG_TX_MNG, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU);
    CMD_FLOW_TO_MCU(cmd_data,M_CPU_EDCATX, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_ENG_CPU, M_ENG_CPU);
    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_TXCTRL, (void *)cmd_data, 8, TRUE, FALSE);
}

ssv_inline static void init_sta_rx_data_flow(void)
{
    //We can trap into CPU many times, but the hw moudle that is before HCI must be CPU
    unsigned int cmd_data[2];

    CMD_FLOW_TO_HW(cmd_data,M_ENG_MACRX, M_ENG_ENCRYPT_SEC, M_ENG_MIC_SEC, M_ENG_CPU, M_ENG_HWHCI, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU);
    CMD_FLOW_TO_MCU(cmd_data,M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_RXDATA, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG);

    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_RXDATA, (void *)cmd_data, 8, TRUE, FALSE);
}

ssv_inline static void init_sta_rx_mgmt_flow(void)
{
    //We can trap into CPU many times, but the hw moudle that is before HCI must be CPU
    unsigned int cmd_data[2];

    CMD_FLOW_TO_HW(cmd_data, M_ENG_MACRX, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU);
    CMD_FLOW_TO_MCU(cmd_data, M_CPU_HWENG, M_CPU_RXMGMT, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG);

    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_RXMGMT, (void *)cmd_data, 8, TRUE, FALSE);

}

ssv_inline static void init_sta_rx_ctrl_flow(void)
{
    //We can trap into CPU many times, but the hw moudle that is before HCI must be CPU
    unsigned int cmd_data[2];

    CMD_FLOW_TO_HW(cmd_data, M_ENG_MACRX, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU);
    CMD_FLOW_TO_MCU(cmd_data, M_CPU_HWENG, M_CPU_RXCTRL, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG);
    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_RXCTRL, (void *)cmd_data, 8, TRUE, FALSE);
}

void cabrio_init_ap_mode_data_flow(void)
{
    init_ap_tx_data_flow();
    init_ap_tx_mgmt_flow();
    init_ap_tx_ctrl_flow();

    init_ap_rx_data_flow();
    init_ap_rx_mgmt_flow();
    init_ap_rx_ctrl_flow();
}

void cabrio_init_sta_mode_data_flow(void)
{
    init_sta_tx_data_flow();
    init_sta_tx_mgmt_flow();
    init_sta_tx_ctrl_flow();

    init_sta_rx_data_flow();
    init_sta_rx_mgmt_flow();
    init_sta_rx_ctrl_flow();
}

//void set_ap_mode_rx_mgmt_flow_for_acs(void)
int ssv6006_hal_acs_rx_mgmt_flow(void)
{
    unsigned int cmd_data[2];
    CMD_FLOW_TO_HW(cmd_data, M_ENG_MACRX, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU);
    CMD_FLOW_TO_MCU(cmd_data, M_CPU_HWENG, M_CPU_RXMGMT_ACS, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG);
    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_RXMGMT, (void *)cmd_data, 8, TRUE, FALSE);
    return 0;
}

//void reset_ap_mode_rx_mgmt_flow(void)
int ssv6006_hal_ap_rx_mgmt_flow(void)
{
    init_ap_rx_mgmt_flow();
    return 0;
}

//void set_sconfig_rx_data_flow(void)
int ssv6006_hal_sconfig_rx_data_flow(void)
{
    unsigned int cmd_data[2];

    CMD_FLOW_TO_HW(cmd_data,M_ENG_MACRX, M_ENG_CPU, M_ENG_HWHCI, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU, M_ENG_CPU);
    CMD_FLOW_TO_MCU(cmd_data,M_CPU_HWENG, M_CPU_RXDATA, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG, M_CPU_HWENG);

    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_RXDATA, (void *)cmd_data, 8, TRUE, FALSE);
    return 0;
}

//void reset_sconfig_rx_data_flow(void)
int ssv6006_hal_sta_rx_data_flow(void)
{
    init_sta_rx_data_flow();
    return 0;
}

#endif
