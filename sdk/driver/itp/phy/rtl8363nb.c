#include "ite/ite_mac.h"
#include "rtk_types.h"
#include "port.h"
#include "interrupt.h"
#include "rtk_switch.h"
#include "qos.h"
#include "cpu.h"
#include "stat.h"
#include "smi.h"

static int rtl8363nb_port[3] = { UTP_PORT1, UTP_PORT3, EXT_PORT0 };
static volatile uint32_t rtl8363nb_link_status = 0;  /* D[0]:link, D[28]:port 0, D[29]:port 1 */

static rtk_port_mac_ability_t g_mac_cfg;

static void rtl8363nb_link_init(void) /** for system reset but rtl8363nb not reset issue. */
{
    int ret;
    uint32_t i;
    rtk_int_info_t info;
    rtk_port_linkStatus_t linkStatus;
    rtk_port_speed_t speed;
    rtk_port_duplex_t duplex;

    /** only for UTP port */
    for (i = 0; i < 2; i++) 
    {
        ret = rtk_port_phyStatus_get(rtl8363nb_port[i], &linkStatus, &speed, &duplex);
        if (ret) {
            ithPrintf("[ERR] rtk_port_phyStatus_get(%d) ret=0x%X \n", i, ret);
            goto end;
        }
		if (linkStatus)
		{
			rtl8363nb_link_status |= (0x1 << (28 + i));
			
			ithPrintf("port %d: Link %s, %sM %s duplex \n", rtl8363nb_port[i],
				(linkStatus ? "up" : "down"),
				(speed, speed ? (speed == 1 ? "100" : "1000") : "10"),
				(duplex ? "Full" : "Half"));
		}
    }

    if (rtl8363nb_link_status & 0x30000000)
        rtl8363nb_link_status |= 0x1;
    else
        rtl8363nb_link_status = 0;

    //ithPrintf("rtl8363nb_link_status: 0x%08X \n", rtl8363nb_link_status);

end:
    return;
}

static void get_cpu_port_status(void)
{
    int ret;
    rtk_mode_ext_t mode;

    memset(&g_mac_cfg, 0x00, sizeof(rtk_port_mac_ability_t));
    ret = rtk_port_macForceLinkExt_get(EXT_PORT0, &mode, &g_mac_cfg);
    if (ret) {
        ithPrintf("[ERR] rtk_port_macForceLinkExt_get(%d) ret=0x%X \n", EXT_PORT0, ret);
        goto end;
    }
    ithPrintf("port %d:", EXT_PORT0);
    ithPrintf(" linkStatus = %d, ", g_mac_cfg.link);
    ithPrintf(" %sM ", ((g_mac_cfg.speed == PORT_SPEED_10M) ? "10" : ((g_mac_cfg.speed == PORT_SPEED_100M) ? "100" : "1000")));
    ithPrintf(" %s duplex\n", g_mac_cfg.duplex ? "Full" : "Half");
end:
    return;
}

static void rtl8363nb_link(void)
{
    int ret;
    uint32_t i;
    rtk_int_info_t info;
    rtk_port_linkStatus_t linkStatus;
    rtk_port_speed_t speed;
    rtk_port_duplex_t duplex;

    rtk_int_advanceInfo_get(ADV_PORT_LINKUP_PORT_MASK, &info);
    if (info.portMask.bits[0]) 
    {
        //ithPrintf("link up port: 0x%X \n", info.portMask.bits[0]);
        /** for UTP port */
        for (i = 0; i < 2; i++) 
        {
            if (info.portMask.bits[0] & (0x1 << rtl8363nb_port[i])) 
            {
                rtl8363nb_link_status |= (0x1 << (28 + i));

                ret = rtk_port_phyStatus_get(rtl8363nb_port[i], &linkStatus, &speed, &duplex);
                if (ret) {
                    ithPrintf("[ERR] rtk_port_phyStatus_get(%d) ret=0x%X \n", i, ret);
                    goto end;
                }
                ithPrintf("port %d: Link %s, %sM %s duplex \n", rtl8363nb_port[i],
                    (linkStatus ? "up" : "down"),
                    (speed, speed ? (speed == 1 ? "100" : "1000") : "10"),
                    (duplex ? "Full" : "Half"));
            }
        }
        /** for CPU port */
        if (info.portMask.bits[0] & (0x1 << EXT_PORT0))
            get_cpu_port_status();
    }

    rtk_int_advanceInfo_get(ADV_PORT_LINKDOWN_PORT_MASK, &info);
    if (info.portMask.bits[0]) 
    {
        //ithPrintf("link down port: 0x%X \n", info.portMask.bits[0]);
        /** for UTP port */
        for (i = 0; i < 2; i++) 
        {
            if (info.portMask.bits[0] & (0x1 << rtl8363nb_port[i])) 
            {
                rtl8363nb_link_status &= ~(0x1 << (28 + i));

                ret = rtk_port_phyStatus_get(rtl8363nb_port[i], &linkStatus, &speed, &duplex);
                if (ret) {
                    ithPrintf("[ERR] rtk_port_phyStatus_get(%d) ret=0x%X \n", i, ret);
                    goto end;
                }
                ithPrintf("port %d: Link %s, %sM %s duplex \n", rtl8363nb_port[i],
                    (linkStatus ? "Up" : "Down"),
                    (speed, speed ? (speed == 1 ? "100" : "1000") : "10"),
                    (duplex ? "Full" : "Half"));
            }
        }

        /** for CPU port */
        if (info.portMask.bits[0] & (0x1 << EXT_PORT0))
            get_cpu_port_status();
    }

    if (rtl8363nb_link_status & 0x30000000)
        rtl8363nb_link_status |= 0x1;
    else
        rtl8363nb_link_status = 0;

    //ithPrintf("rtl8363nb_link_status: 0x%08X \n", rtl8363nb_link_status);

end:
    return;
}

static void rtl8363nb_link_speed_change(void)
{
    int ret, i;
    rtk_int_info_t info;
    rtk_port_linkStatus_t linkStatus;
    rtk_port_speed_t speed;
    rtk_port_duplex_t duplex;

    rtk_int_advanceInfo_get(ADV_SPEED_CHANGE_PORT_MASK, &info);
    //ithPrintf("speed change port: 0x%X \n", info.portMask.bits[0]);

    for (i = 0; i <= 2; i++) 
    {
        if (info.portMask.bits[0] & (0x1 << rtl8363nb_port[i]))
        {
            /** get external interface ability */
            if (rtl8363nb_port[i] == EXT_PORT0) 
            { 
                get_cpu_port_status();
            }
            else
            {
                ret = rtk_port_phyStatus_get(rtl8363nb_port[i], &linkStatus, &speed, &duplex);
                if (ret) {
                    ithPrintf("[ERR] rtk_port_phyStatus_get(%d) ret=0x%X \n", i, ret);
                    goto end;
                }
                ithPrintf("port %d:", rtl8363nb_port[i]);
                ithPrintf(" linkStatus = %d, ", linkStatus);
                ithPrintf(" %sM ", ((speed == PORT_SPEED_10M) ? "10" : ((speed == PORT_SPEED_100M) ? "100" : "1000")));
                ithPrintf(" %s duplex\n", duplex ? "Full" : "Half");
            }
        }
    }
end:
	return;
}

void PhyInit(int ethMode)
{
    int i;
    rtk_api_ret_t ret;
    rtk_port_mac_ability_t mac_cfg;

    rtk_uint32 data = 0;
    smi_read(0x1202, &data);
    printf("rtl8363nb reg 0x1202 = 0x%X (0x88A8)\n", data);

    if (ret = rtk_switch_init()) {
        printf("[ERR] rtk_switch_init() ret=0x%X \n", ret);
        goto end;
    }
#if 0
    if (ret = rtk_qos_init(1)) {
        printf("[ERR] rtk_qos_init() ret=0x%X \n", ret);
        goto end;
    }
    if (ret = rtk_cpu_enable_set(ENABLED)) {
        printf("[ERR] rtk_cpu_enable_set() ret=0x%X \n", ret);
        goto end;
    }
    if (ret = rtk_cpu_tagPort_set(EXT_PORT0, CPU_INSERT_TO_NONE)) {
        printf("[ERR] rtk_cpu_tagPort_set() ret=0x%X \n", ret);
        goto end;
    }
#endif

    /** set external interface ability */
    memset(&mac_cfg, 0x00, sizeof(rtk_port_mac_ability_t));
    mac_cfg.forcemode = 1;
    mac_cfg.speed = PORT_SPEED_100M;
    mac_cfg.duplex = PORT_FULL_DUPLEX;
    mac_cfg.link = 1;
    /** MODE_EXT_RMII_PHY: clock output mode, MODE_EXT_RMII_MAC: clock input mode */
    if (ret = rtk_port_macForceLinkExt_set(EXT_PORT0, MODE_EXT_RMII_PHY, &mac_cfg)) {
        printf("[ERR] rtk_port_macForceLinkExt_set() ret=0x%X \n", ret);
        goto end;
    }

    /** reset MIB counters */
    for (i = 0; i < 3; i++)
        rtk_stat_port_reset(rtl8363nb_port[i]);

#if 0
    {
        rtk_cpu_insert_t mode;
        rtk_port_t port;
        rtk_cpu_tagPort_get(&port, &mode);
        printf("CPU port:%d, mode:%d \n", port, mode);
    }
#endif

#if 0
    /** read phy abililty */
    {
        int i;
        rtk_port_phy_ability_t ability;

        for (i = 0; i <= 1; i++) {
            ret = rtk_port_phyAutoNegoAbility_get(rtl8363nb_port[i], &ability);
            if (ret) {
                printf("[ERR] rtk_port_phyAutoNegoAbility_get(%d) ret=0x%X \n", i, ret);
                goto end;
            }
            printf("rtk_port_phyAutoNegoAbility_get(): %d\n", i);
            printf(" ability.AutoNegotiation = %d \n", ability.AutoNegotiation);
            printf(" ability.Half_10 = %d \n", ability.Half_10);
            printf(" ability.Full_10 = %d \n", ability.Full_10);
            printf(" ability.Half_100 = %d \n", ability.Half_100);
            printf(" ability.Full_100 = %d \n", ability.Full_100);
            printf(" ability.Full_1000 = %d \n", ability.Full_1000);
            printf(" ability.FC = %d \n", ability.FC);
            printf(" ability.AsyFC = %d \n", ability.AsyFC);
        }
    }
#endif

    rtl8363nb_link_init();

    /* enable link/speed interrupt */
    rtk_int_polarity_set(INT_POLAR_LOW);
    rtk_int_control_set(INT_TYPE_LINK_STATUS, ENABLED);
    rtk_int_control_set(INT_TYPE_LINK_SPEED, ENABLED);

end:
    if (ret)
    {
        printf(" stop! \n");
        while (1);
    }
    return;
}

int rtl8363nb_read_mode(int* speed, int* duplex)
{
#if 1  // from interrupt
    (*speed) = (*duplex) = 0;

    if (g_mac_cfg.speed == PORT_SPEED_10M)
        (*speed) = SPEED_10;
    else if (g_mac_cfg.speed == PORT_SPEED_100M)
        (*speed) = SPEED_100;
    else if (g_mac_cfg.speed == PORT_SPEED_1000M)
        (*speed) = SPEED_1000;
    else
        ithPrintf("[ERR]rtl8363nb cpu speed: %d \n", g_mac_cfg.speed);

    if (g_mac_cfg.duplex == PORT_FULL_DUPLEX)
        (*duplex) = DUPLEX_FULL;
    else if (g_mac_cfg.duplex == PORT_HALF_DUPLEX)
        (*duplex) = DUPLEX_HALF;
    else
        ithPrintf("[ERR]rtl8363nb cpu duplex: %d \n", g_mac_cfg.duplex);

    return (g_mac_cfg.link ? 0 : -1); // 0 means link up
#else
    rtk_api_ret_t ret;
    rtk_mode_ext_t mode = 0;
    rtk_port_mac_ability_t mac_cfg = { 0 };

    (*speed) = (*duplex) = 0;

    ret = rtk_port_macForceLinkExt_get(EXT_PORT0, &mode, &mac_cfg);
    if (ret)
        ithPrintf("[ERR] rtk_port_macForceLinkExt_get() ret=0x%X \n", ret);

    if (mac_cfg.speed == PORT_SPEED_10M)
        (*speed) = SPEED_10;
    else if (mac_cfg.speed == PORT_SPEED_100M)
        (*speed) = SPEED_100;
    else if (mac_cfg.speed == PORT_SPEED_1000M)
        (*speed) = SPEED_1000;
    else
        ithPrintf("[ERR]rtl8363nb cpu speed: %d \n", mac_cfg.speed);

    if (mac_cfg.duplex == PORT_FULL_DUPLEX)
        (*duplex) = DUPLEX_FULL;
    else if (mac_cfg.duplex == PORT_HALF_DUPLEX)
        (*duplex) = DUPLEX_HALF;
    else
        ithPrintf("[ERR]rtl8363nb cpu duplex: %d \n", mac_cfg.duplex);

    return (mac_cfg.link ? 0 : -1); // 0 means link up
#endif
}

void rtl8363nb_linkIntrHandler(unsigned int pin, void *arg)
{
    rtk_int_status_t statusMask;

    if (pin != CFG_GPIO_ETHERNET_LINK)
    {
        ithPrintf("rtl8363nb link gpio error! %d \n", pin);
        return;
    }

    rtk_int_status_get(&statusMask);
    if (statusMask.value[0])
    {
        //ithPrintf("rtl8363nb_intr_status: 0x%X \n", statusMask.value[0]);
        rtk_int_status_set(&statusMask);
        /** get advanced information */
        if (statusMask.value[0] & (0x1 << INT_TYPE_LINK_STATUS))
            rtl8363nb_link();

        if (statusMask.value[0] & (0x1 << INT_TYPE_LINK_SPEED))
            rtl8363nb_link_speed_change();
    }

    return;
}

uint32_t rtl8363nb_get_link_status(void)
{
    return rtl8363nb_link_status;
}

void rtl8363nb_mib_dump(int port)
{
    rtk_api_ret_t ret;
    rtk_stat_port_cntr_t stats = { 0 };

    ret = rtk_stat_port_getAll(port, &stats);
    if (ret) {
        printf("[ERR] rtk_stat_port_getAll(%d) ret=0x%X \n", port, ret);
        goto end;
    }
    printf("\n\nMIB counter: port %d \n", port);
    printf(" ifInOctets: %d \n", stats.ifInOctets);
    printf(" dot3StatsFCSErrors: %d \n", stats.dot3StatsFCSErrors);
    printf(" dot3StatsSymbolErrors: %d \n", stats.dot3StatsSymbolErrors);
    printf(" dot3InPauseFrames: %d \n", stats.dot3InPauseFrames);
    printf(" dot3ControlInUnknownOpcodes: %d \n", stats.dot3ControlInUnknownOpcodes);
    printf(" etherStatsFragments: %d \n", stats.etherStatsFragments);
    printf(" etherStatsJabbers: %d \n", stats.etherStatsJabbers);
    printf(" ifInUcastPkts: %d \n", stats.ifInUcastPkts);
    printf(" etherStatsDropEvents: %d \n", stats.etherStatsDropEvents);
    printf(" etherStatsOctets: %d \n", stats.etherStatsOctets);
    printf(" etherStatsUndersizePkts: %d \n", stats.etherStatsUndersizePkts);
    printf(" etherStatsOversizePkts: %d \n", stats.etherStatsOversizePkts);
    printf(" etherStatsPkts64Octets: %d \n", stats.etherStatsPkts64Octets);
    printf(" etherStatsPkts65to127Octets: %d \n", stats.etherStatsPkts65to127Octets);
    printf(" etherStatsPkts128to255Octets: %d \n", stats.etherStatsPkts128to255Octets);
    printf(" etherStatsPkts256to511Octets: %d \n", stats.etherStatsPkts256to511Octets);
    printf(" etherStatsPkts512to1023Octets: %d \n", stats.etherStatsPkts512to1023Octets);
    printf(" etherStatsPkts1024toMaxOctets: %d \n", stats.etherStatsPkts1024toMaxOctets);
    printf(" etherStatsMcastPkts: %d \n", stats.etherStatsMcastPkts);
    printf(" etherStatsBcastPkts: %d \n", stats.etherStatsBcastPkts);
    printf(" ifOutOctets: %d \n", stats.ifOutOctets);
    printf(" dot3StatsSingleCollisionFrames: %d \n", stats.dot3StatsSingleCollisionFrames);
    printf(" dot3StatsMultipleCollisionFrames: %d \n", stats.dot3StatsMultipleCollisionFrames);
    printf(" dot3StatsDeferredTransmissions: %d \n", stats.dot3StatsDeferredTransmissions);
    printf(" dot3StatsLateCollisions: %d \n", stats.dot3StatsLateCollisions);
    printf(" etherStatsCollisions: %d \n", stats.etherStatsCollisions);
    printf(" dot3StatsExcessiveCollisions: %d \n", stats.dot3StatsExcessiveCollisions);
    printf(" dot3OutPauseFrames: %d \n", stats.dot3OutPauseFrames);
    printf(" dot1dBasePortDelayExceededDiscards: %d \n", stats.dot1dBasePortDelayExceededDiscards);
    printf(" dot1dTpPortInDiscards: %d \n", stats.dot1dTpPortInDiscards);
    printf(" ifOutUcastPkts: %d \n", stats.ifOutUcastPkts);
    printf(" ifOutMulticastPkts: %d \n", stats.ifOutMulticastPkts);
    printf(" ifOutBrocastPkts: %d \n", stats.ifOutBrocastPkts);
    printf(" outOampduPkts: %d \n", stats.outOampduPkts);
    printf(" inOampduPkts: %d \n", stats.inOampduPkts);
    printf(" pktgenPkts: %d \n", stats.pktgenPkts);
    printf(" inMldChecksumError: %d \n", stats.inMldChecksumError);
    printf(" inIgmpChecksumError: %d \n", stats.inIgmpChecksumError);
    printf(" inMldSpecificQuery: %d \n", stats.inMldSpecificQuery);
    printf(" inMldGeneralQuery: %d \n", stats.inMldGeneralQuery);
    printf(" inIgmpSpecificQuery: %d \n", stats.inIgmpSpecificQuery);
    printf(" inIgmpGeneralQuery: %d \n", stats.inIgmpGeneralQuery);
    printf(" inMldLeaves: %d \n", stats.inMldLeaves);
    printf(" inIgmpLeaves: %d \n", stats.inIgmpLeaves);
    printf(" inIgmpJoinsSuccess: %d \n", stats.inIgmpJoinsSuccess);
    printf(" inIgmpJoinsFail: %d \n", stats.inIgmpJoinsFail);
    printf(" inMldJoinsSuccess: %d \n", stats.inMldJoinsSuccess);
    printf(" inMldJoinsFail: %d \n", stats.inMldJoinsFail);
    printf(" inReportSuppressionDrop: %d \n", stats.inReportSuppressionDrop);
    printf(" inLeaveSuppressionDrop: %d \n", stats.inLeaveSuppressionDrop);
    printf(" outIgmpReports: %d \n", stats.outIgmpReports);
    printf(" outIgmpLeaves: %d \n", stats.outIgmpLeaves);
    printf(" outIgmpGeneralQuery: %d \n", stats.outIgmpGeneralQuery);
    printf(" outIgmpSpecificQuery: %d \n", stats.outIgmpSpecificQuery);
    printf(" outMldReports: %d \n", stats.outMldReports);
    printf(" outMldLeaves: %d \n", stats.outMldLeaves);
    printf(" outMldGeneralQuery: %d \n", stats.outMldGeneralQuery);
    printf(" outMldSpecificQuery: %d \n", stats.outMldSpecificQuery);
    printf(" inKnownMulticastPkts: %d \n", stats.inKnownMulticastPkts);
    printf(" ifInMulticastPkts: %d \n", stats.ifInMulticastPkts);
    printf(" ifInBroadcastPkts: %d \n", stats.ifInBroadcastPkts);
    printf(" ifOutDiscards: %d \n", stats.ifOutDiscards);
end:
    return;
}

void rtl8363nb_mib_dump2(void)
{
    int i;
    rtk_api_ret_t ret;
    rtk_stat_port_cntr_t stats = { 0 };

    for (i=0; i<3; i++)
	{
		ret = rtk_stat_port_getAll(rtl8363nb_port[i], &stats);
		if (ret) {
			printf("[ERR] rtk_stat_port_getAll(%d) ret=0x%X \n", rtl8363nb_port[i], ret);
			goto end;
		}
		printf("\n\nMIB counter: port %d \n", i);
		printf(" etherStatsPkts64Octets: %d \n", stats.etherStatsPkts64Octets);
		printf(" etherStatsPkts65to127Octets: %d \n", stats.etherStatsPkts65to127Octets);
		printf(" etherStatsPkts128to255Octets: %d \n", stats.etherStatsPkts128to255Octets);
		printf(" etherStatsPkts256to511Octets: %d \n", stats.etherStatsPkts256to511Octets);
		printf(" etherStatsPkts512to1023Octets: %d \n", stats.etherStatsPkts512to1023Octets);
		printf(" etherStatsPkts1024toMaxOctets: %d \n", stats.etherStatsPkts1024toMaxOctets);
		printf(" etherStatsMcastPkts: %d \n", stats.etherStatsMcastPkts);
		printf(" etherStatsBcastPkts: %d \n", stats.etherStatsBcastPkts);
		printf(" ifOutUcastPkts: %d \n", stats.ifOutUcastPkts);
		printf(" ifOutMulticastPkts: %d \n", stats.ifOutMulticastPkts);
		printf(" ifOutBrocastPkts: %d \n", stats.ifOutBrocastPkts);
		printf(" ifInMulticastPkts: %d \n", stats.ifInMulticastPkts);
		printf(" ifInBroadcastPkts: %d \n", stats.ifInBroadcastPkts);
	}
end:
	return;
}

/**
* Check interrupt status for link change.
* Call from mac driver's internal ISR for phy's interrupt.
*/
int(*itpPhyLinkChange)(void) = NULL;
/**
* Replace mac driver's ISR for phy's interrupt.
*/
ITHGpioIntrHandler itpPhylinkIsr = rtl8363nb_linkIntrHandler;
/**
* Returns 0 if the device reports link status up/ok
*/
int(*itpPhyReadMode)(int* speed, int* duplex) = rtl8363nb_read_mode;
/**
* Get link status.
*/
uint32_t(*itpPhyLinkStatus)(void) = rtl8363nb_get_link_status;
