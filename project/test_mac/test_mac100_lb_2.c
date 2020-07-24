#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ite/itp.h"
#include "ite/ite_mac.h"



#define TEST_PACKET_NUM		255


#define MAX_PACKET_SIZE		1500//1514
static uint8_t pattern[MAX_PACKET_SIZE];

struct eth_packet {
    uint8_t     *addr;
    uint32_t    size;
};

static struct eth_packet tx_packet[TEST_PACKET_NUM];
static struct eth_packet rx_packet[TEST_PACKET_NUM];
static uint32_t     rx_cnt;
static sem_t    sem;

void RxCallback(void* ctx, void* data, int len)
{
    rx_packet[rx_cnt].addr = data;
    rx_packet[rx_cnt].size = len;
    rx_cnt++;
    if (rx_cnt == TEST_PACKET_NUM)
        sem_post(&sem);
}



static uint8_t macaddr[] = {0x02, 0x22, 0x33, 0x44, 0x55, 0x66};
static const uint8_t ioConfig[] = { CFG_GPIO_ETHERNET };
static ITE_MAC_CFG_T mac_cfg;

static int
Initialize(void)
{
    int res, i;

    mac_cfg.flags |= (ITH_COUNT_OF(ioConfig) == ITE_MAC_GRMII_PIN_CNT) ? ITE_MAC_RGMII : 0;
    mac_cfg.clk_inv = CFG_NET_MAC_CLOCK_INVERSE;
    mac_cfg.clk_delay = CFG_NET_MAC_CLOCK_DELAY;
    mac_cfg.rxd_delay = CFG_NET_MAC_RXD_DELAY;
    mac_cfg.phyAddr = CFG_NET_ETHERNET_PHY_ADDR;
    mac_cfg.ioConfig = ioConfig;
    mac_cfg.linkGpio = CFG_GPIO_ETHERNET_LINK;
    mac_cfg.phy_link_change = itpPhyLinkChange;
    mac_cfg.linkGpio_isr  = itpPhylinkIsr;
    mac_cfg.phy_link_status = itpPhyLinkStatus;
    mac_cfg.phy_read_mode = itpPhyReadMode;

#if defined(CFG_GPIO_ETHERNET_LINK)
    // enable gpio interrupt
    ithIntrEnableIrq(ITH_INTR_GPIO);
#endif
    res = iteMacInitialize(&mac_cfg);
    if (res)
    {
        printf(" iteMacInitialize() fail! \n");
        while(1);
    }
    res = iteMacSetMacAddr(macaddr);
    if(res)
    {
        printf(" iteMacSetMacAddr() fail! \n");
        while(1);
    }
    PhyInit(CFG_ETH_MODE);
	
    /** fill test pattern */
    for (i = 0; i < MAX_PACKET_SIZE; i++)
        pattern[i] = (uint8_t)((i + 1) % 0x100);

    /** fill tx packets */
    tx_packet[0].addr = pattern;
    tx_packet[0].size = MAX_PACKET_SIZE;
    for (i = 1; i < TEST_PACKET_NUM; i++) {
        tx_packet[i].addr = tx_packet[i - 1].addr + 1;
        tx_packet[i].size = tx_packet[i - 1].size - 1;
	}

    return res;
}


static int
MainLoop(
    void)
{
    int res = 0, i, j, k;
    uint8_t macaddr[6];
    int linkup = 0;
    uint32_t loopcnt, fail_cnt;

    res = iteMacOpen(macaddr, RxCallback, NULL, CFG_ETH_MODE);
    if(res)
    {
        printf(" iteMacOpen() fail 0x%08X \n", res);
        while(1);
    }

#if (CFG_ETH_MODE != ITE_ETH_MAC_LB) && (CFG_ETH_MODE != ITE_ETH_MAC_LB_1000)
wait_linkup:
    printf("\n wait link up...... \n\n");
    while(!iteEthGetLink()) usleep(10*1000);
	
    linkup = 1;
    printf("\n link up! \n\n");
#endif

    loopcnt = 0;

    for(;;)
    {
        rx_cnt = 0;
        fail_cnt = 0;

        /* send tx packet */
        for (i = 0; i < TEST_PACKET_NUM; i++) 
        {
            res = iteMacSend(tx_packet[i].addr, tx_packet[i].size);
            if (res) {
                printf("iteMacSend() res = 0x%X \n", res);
                while (1);
            }
        }

        /* wait rx ready */
        res = itpSemWaitTimeout(&sem, 6000);
        if (res)
            printf("wait rx %d packets timeout! current rx_cnt:%d\n", TEST_PACKET_NUM, rx_cnt);

        /* compare data */
        for (j = 0; j < TEST_PACKET_NUM; j++) 
        {
            if (memcmp((void*)tx_packet[j].addr, (void*)rx_packet[j].addr, tx_packet[j].size))
		{
                fail_cnt++;

                for (k = 0; k<tx_packet[j].size; k++)
                {
                    if (!(k % 0x10))
                        printf("\n");
                    printf("%02x ", rx_packet[j].addr[k]);
                    if (!(k % 0x80)) usleep(5 * 1000); // delay for print buffer
		}
                printf(" Rx \r\n");

                usleep(5 * 1000); // delay for print buffer

                for (k = 0; k<tx_packet[j].size; k++)
		{
                    if (!(k % 0x10))
                        printf("\n");
                    printf("%02x ", tx_packet[j].addr[k]);
                    if (!(k % 0x80)) usleep(5 * 1000); // delay for print buffer
		}
                printf(" Tx \r\n");
                usleep(5 * 1000); // delay for print buffer

                if (fail_cnt > 5)
                    break;
            }
        }
        if (fail_cnt == 0)
            printf("loopcnt: %d Success!! \n", loopcnt++);
    }

    return res;
}


static int
Terminate(void)
{
    return 0;
}


void* TestFunc(void* arg)
{
	int res;
	
#define TEST_STACK_SIZE 102400
	pthread_t task;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, TEST_STACK_SIZE);
	param.sched_priority = sched_get_priority_max(1);
	pthread_attr_setschedparam(&attr, &param);
	pthread_create(&task, &attr, iteMacThreadFunc, NULL);

    sem_init(&sem, 0, 0);

	res = Initialize();
	if (res)
		goto end;

	res = MainLoop();
	if (res)
		goto end;

	res = Terminate();
	if (res)
		goto end;
	
end:
    return NULL;
}
