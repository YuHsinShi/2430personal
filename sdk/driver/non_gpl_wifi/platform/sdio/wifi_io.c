//#include "include/common.h"
#if defined(CFG_MMC_ENABLE)
#include "ite/ite_sdio.h"
#include <linux/os.h>
#else
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio.h>
#endif
#include "wifi_io.h"
#include "wifi_constants.h"
#include "rtwlan_bsp.h"
#include "openrtos/portmacro.h"

#define BUILD_SDIO
#define DRV_NAME "NGPL Wifi"

struct sdio_func *wifi_sdio_func;

static const struct sdio_device_id sdio_ids[] = {
	{ SDIO_DEVICE(0x024c, 0xF179),.driver_data = (void *)RTL8189F}
};

/*Declaration*/
static int drv_init(struct sdio_func *func, const struct sdio_device_id *id);
static void dev_remove(struct sdio_func *func);
static int wifi_fake_driver_probe_8189es(struct sdio_func *func);

/* Extern*/
extern void cmd_wifi_scan(int argc, char **argv);
extern int wifi_on(rtw_mode_t mode);
extern int wifi_off(void);
extern void vTaskDelay(const TickType_t xTicksToDelay );

struct sdio_drv_priv {
	struct sdio_driver wifi_drv;
	int drv_registered;
};

static struct sdio_drv_priv sdio_drvpriv = {
	.wifi_drv.probe = drv_init,
	.wifi_drv.remove = dev_remove,
	.wifi_drv.name = (char*)DRV_NAME,
	.wifi_drv.id_table = sdio_ids,
};

static int drv_init(struct sdio_func *func, const struct sdio_device_id *id)
{
    printk("====>drv_init: hook sdio_func!! vendor=0x%04x device=0x%04x class=0x%02x\n",
    	func->vendor, func->device, func->class);
    wifi_sdio_func = NULL;
    wifi_sdio_func = func;
    return 0;
}

static void dev_remove(struct sdio_func *func)
{
    printk("====>dev_remove()\n");
    return;
}

int mmpRtlWifiDriverRegister(void)
{
	int ret;

	printk("====>Do mmpRtlWifiDriverRegister....\n");
	ret = sdio_register_driver(&sdio_drvpriv.wifi_drv);

	return ret;
}

int mmpRtlWifiDriverCmdTest(void)
{
    int nRet;

    int mode;
    char 				*ssid;
    rtw_security_t	security_type;
    char 				*password;
    int 				ssid_len;
    int 				password_len;
    int 				key_id;
    void				*semaphore;
    int nMode = 0;

    printk("====>Do mmpRtlWifiDriverCmdTest: we call entry function or CMD test there.... \n");


    wifi_fake_driver_probe_8189es(wifi_sdio_func);
    usleep(100*1000);

    if (nMode == 0){
        nRet = wifi_on(RTW_MODE_STA);
    } else if (nMode ==1){
	//   wifi_off();
		vTaskDelay(20);
		nRet = wifi_on(RTW_MODE_AP);

    } else if (nMode ==2){
		wifi_off();
		vTaskDelay(20);
		nRet = wifi_on(RTW_MODE_STA_AP);
    }



    printk("====>wifi on: %d\n", nRet);

    while(nRet){
        printk("====>wait sdio wifi driver ready...\n");
        usleep(100*1000);
    }
    //start_interactive_mode();

    //cmd_wifi_scan(0, NULL);

    //cmd_wifi_info(0,NULL);

    // wifi_off();

    return nRet;
}

int testBuildFail()
{
    Set_WLAN_Power_On();
    //netif_post_sleep_processing();
	//wifi_on(1);
}
#ifdef BUILD_SDIO

/* test wifi driver */
#define ADDR_MASK 0x10000
#define LOCAL_ADDR_MASK 0x00000
#ifndef BIT
#define BIT(_x)	(1 << (_x))
#endif

int wifi_read(struct sdio_func *func, u32 addr, u32 cnt, void *pdata)
{
	int err;

	sdio_claim_host(func);

	err = sdio_memcpy_fromio(func, pdata, addr, cnt);
	if (err) {
		printk("%s: FAIL(%d)! ADDR=%#x Size=%d\n", __func__, err, addr, cnt);
	}

	sdio_release_host(func);

	return err;
}

int wifi_write(struct sdio_func *func, u32 addr, u32 cnt, void *pdata)
{
	int err;
	u32 size;

	sdio_claim_host(func);

	size = cnt;
	err = sdio_memcpy_toio(func, addr, pdata, size);
	if (err) {
		printk("%s: FAIL(%d)! ADDR=%#x Size=%d(%d)\n", __func__, err, addr, cnt, size);
	}

	sdio_release_host(func);

	return err;
}

u8 wifi_readb(struct sdio_func *func, u32 addr)
{
	int err;
	u8 ret = 0;

	sdio_claim_host(func);
	ret = sdio_readb(func, ADDR_MASK | addr, &err);
	sdio_release_host(func);

	if (err)
		printk("%s: FAIL!(%d) addr=0x%05x\n", __func__, err, addr);

	return ret;
}

u16 wifi_readw(struct sdio_func *func, u32 addr)
{
	int err;
	u16 v;

	sdio_claim_host(func);
	v = sdio_readw(func, ADDR_MASK | addr, &err);
	sdio_release_host(func);
	if (err)
		printk("%s: FAIL!(%d) addr=0x%05x\n", __func__, err, addr);

	return  v;
}

u32 wifi_readl(struct sdio_func *func, u32 addr)
{
	int err;
	u32 v;

	sdio_claim_host(func);
	v = sdio_readl(func, ADDR_MASK | addr, &err);
	sdio_release_host(func);

	return  v;
}

void wifi_writeb(struct sdio_func *func, u32 addr, u8 val)
{
	int err;

	sdio_claim_host(func);
	sdio_writeb(func, val, ADDR_MASK | addr, &err);
	sdio_release_host(func);
	if (err)
		printk("%s: FAIL!(%d) addr=0x%05x val=0x%02x\n", __func__, err, addr, val);
}

void wifi_writew(struct sdio_func *func, u32 addr, u16 v)
{
	int err;

	sdio_claim_host(func);
	sdio_writew(func, v, ADDR_MASK | addr, &err);
	sdio_release_host(func);
	if (err)
		printk("%s: FAIL!(%d) addr=0x%05x val=0x%04x\n", __func__, err, addr, v);
}

void wifi_writel(struct sdio_func *func, u32 addr, u32 v)
{
	int err;

	sdio_claim_host(func);
	sdio_writel(func, v, ADDR_MASK | addr, &err);
	sdio_release_host(func);
}

u8 wifi_readb_local(struct sdio_func *func, u32 addr)
{
    int err;
    u8 ret = 0;
    sdio_claim_host(func);
    ret = sdio_readb(func, LOCAL_ADDR_MASK | addr, &err);
    sdio_release_host(func);

    return ret;
}

void wifi_writeb_local(struct sdio_func *func, u32 addr, u8 val)
{
    int err;
    sdio_claim_host(func);
    sdio_writeb(func, val, LOCAL_ADDR_MASK | addr, &err);
    sdio_release_host(func);

}

static void wifi_card_enable_flow_8189es(struct sdio_func *func)
{
	//RTL8188E_TRANS_CARDDIS_TO_CARDEMU

#ifdef CONFIG_8192E_SDIO
	volatile u8 temp1;
	u8 idx;
	printk("%s %s \n",__FILE__,__FUNCTION__);
	//card disable to enumeration flow
	//sido local 0x86[0]<=1'b0
	temp1 = wifi_readb_local(func, 0x86);
	wifi_writeb_local(func, 0x86, temp1&0xFE);
	//polling 0x86[1]=1
	u8 count=0;
	while(count<50){
		usleep(20);
		temp1 = wifi_readb_local(func, 0x86);
		count++;
		printk("0x86=0x%x count=%d \n",temp1,count);
		if(temp1&0x02)
			break;
	}
	if(count==50)
		printk("fail to polling sdio local register 0x86[1] == 1");
	//11[0]<=1'b1
	temp1 = wifi_readb(func,0x11);
	wifi_writeb(func, 0x11,temp1|0x1);
	// cc[2]<=1'b0
	temp1 = wifi_readb(func,0xcc);
	wifi_writeb(func, 0xcc,temp1&0xFB);
	// 5[7]<=0
	temp1 = wifi_readb(func,0x5);
	wifi_writeb(func, 0x5,temp1&0x7F);
	// 5[2]<=0
	temp1 = wifi_readb(func,0x5);
	wifi_writeb(func, 0x5,temp1&0xFB);
	// 5[4,3]<=0
	temp1 = wifi_readb(func,0x5);
	wifi_writeb(func, 0x5,temp1&0xE7);
	// polling 0x6[1]==1
	count=0;
	while(count<50){
		usleep(20);
		temp1 = wifi_readb(func, 0x6);
		count++;
		printk("0x6=0x%x count=%d \n",temp1,count);
		if(temp1&0x02)
			break;
	}
	if(count==50)
		printk("fail to polling sdio local register 0x6[1] == 1");
	// 6[0]<=1'b1
	temp1 = wifi_readb(func,0x6);
	wifi_writeb(func, 0x6,temp1 |0x01);

	// 5[0]<=1'b1
	temp1 = wifi_readb(func,0x5);
	wifi_writeb(func, 0x5,temp1 |0x01);
	// polling 5[0]==0
	count=50;
	while(count<50){
		usleep(20);
		temp1 = wifi_readb(func, 0x5);
		count++;
		printk("0x5=0x%x count=%d \n",temp1,count);
		if(!(temp1&0x01))
			break;
	}
	if(count==50)
		printk("fail to polling sdio local register 0x6[1] == 1");


#elif defined CONFIG_8723D_SDIO
	volatile u8 temp1;
	u8 count;
	printk("\r%s :CONFIG_8723D_SDIO\n",__func__);
	//0x5[7,3]=0
	temp1=wifi_readb(func,0x5);
	wifi_writeb(func,0x5,temp1&0x77);
	//sdio local 0x86[0]=0
	temp1 = wifi_readb_local(func, 0x86);
	wifi_writeb_local(func, 0x86, temp1&0xFE);
	//polling 0x86[1]=1
	count=0;
	while(count<50){
		usleep(20);
		temp1 = wifi_readb_local(func, 0x86);
		count++;
		printk("0x86=0x%x count=%d \n",temp1,count);
		if(temp1&0x02)
			break;
	}
	if(count==50)
		printk("fail to polling sdio local register 0x86[1] == 1");
	//0x5[4,3]=0
	temp1=wifi_readb(func,0x5);
	wifi_writeb(func,0x5,temp1&0xE7);
	//0x23[4]=0
	temp1=wifi_readb(func,0x23);
	wifi_writeb(func,0x23,temp1&0xEF);
	//0x5[4,3,2]=0
	temp1=wifi_readb(func,0x5);
	wifi_writeb(func,0x5,temp1&0xE3);
	//polling 0x6[1]=1
	count=0;
	while(count<50){
		usleep(20);
		temp1 = wifi_readb(func, 0x6);
		count++;
		printk("0x6=0x%x count=%d \n",temp1,count);
		if(temp1&0x02)
			break;
	}
	if(count==50)
		printk("fail to polling register 0x6[1] == 1");
	//0x6[0]=1
	temp1=wifi_readb(func,0x6);
	wifi_writeb(func,0x6,temp1|0x1);
	//polling 0x5[1,0]=0
	count=0;
	while(count<50){
		usleep(20);
		temp1 = wifi_readb(func, 0x5);
		count++;
		printk("0x5=0x%x count=%d \n",temp1,count);
		if((~temp1)&0x03)
			break;
	}
	if(count==50)
		printk("fail to polling register 0x5[1,0] == 0");
	//0x5[7]=0
	temp1=wifi_readb(func,0x5);
	wifi_writeb(func,0x5,temp1&0x7F);
	//0x5[4,3]=0
	temp1=wifi_readb(func,0x5);
	wifi_writeb(func,0x5,temp1&0xE7);
	//0x5[0]=1
	temp1=wifi_readb(func,0x5);
	wifi_writeb(func,0x5,temp1|0x1);
	//polling 0x5[0]=0
	count=0;
	while(count<50){
		usleep(20);
		temp1 = wifi_readb(func, 0x5);
		count++;
		printk("0x5=0x%x count=%d \n",temp1,count);
		if((~temp1)&0x01)
			break;
	}
	if(count==50)
		printk("fail to polling register 0x5[1,0] == 0");
	//0x10[6]=1
	temp1=wifi_readb(func,0x10);
	wifi_writeb(func,0x10,temp1|0x40);
	//0x49[1]=1
	temp1=wifi_readb(func,0x49);
	wifi_writeb(func,0x49,temp1|0x2);
	//0x63[1]=1
	temp1=wifi_readb(func,0x63);
	wifi_writeb(func,0x63,temp1|0x2);
	//0x62[1]=0
	temp1=wifi_readb(func,0x62);
	wifi_writeb(func,0x62,temp1&0xFD);
	//0x58[0]=1
	temp1=wifi_readb(func,0x58);
	wifi_writeb(func,0x58,temp1|0x1);
	//0x5A[1]=1
	temp1=wifi_readb(func,0x5A);
	wifi_writeb(func,0x5A,temp1|0x2);
	//0x69[6]=1
	temp1=wifi_readb(func,0x69);
	wifi_writeb(func,0x69,temp1|0x40);
	//0x1F=0x0
	wifi_writeb(func,0x1F,0x0);
	//0x77=0x0
	wifi_writeb(func,0x77,0x0);
	//0x1F=0x7
	wifi_writeb(func,0x1F,0x7);
	//0x77=0x7
	wifi_writeb(func,0x77,0x7);
#else
	wifi_writeb_local(func, 0x0086, wifi_readb_local(func, 0x0086) & (~(BIT(0))));
	usleep(20);
	wifi_writeb(func, 0x0005, wifi_readb(func, 0x0005) & ~(BIT(3)|BIT(4)));

	//RTL8188E_TRANS_CARDEMU_TO_ACT
	usleep(20);
	wifi_writeb(func, 0x0002, wifi_readb(func, 0x0002) & ~(BIT(1)|BIT(0)));
	wifi_writeb(func, 0x0026, wifi_readb(func, 0x0027) | (BIT(7)));
	wifi_writeb(func, 0x0005, wifi_readb(func, 0x0005) & ~(BIT(7)));
	wifi_writeb(func, 0x0005, wifi_readb(func, 0x0005) & ~(BIT(4)|BIT(3)));
	wifi_writeb(func, 0x0005, wifi_readb(func, 0x0005) | (BIT(0)));
	usleep(20);
	wifi_writeb(func, 0x0023, wifi_readb(func, 0x0023) & ~(BIT(4)));
	wifi_writeb(func, 0x0074, wifi_readb(func, 0x0074) | (BIT(4)));
#endif
}

static void wifi_card_disable_flow_8189es(struct sdio_func *func)
{
	//RTL8188E_TRANS_ACT_TO_CARDEMU
	wifi_writeb(func, 0x001F, 0x00);
	wifi_writeb(func, 0x0023, wifi_readb(func, 0x0023) | (BIT(4)));
	wifi_writeb(func, 0x0005, wifi_readb(func, 0x0005) | (BIT(1)));
	usleep(20);

	//RTL8188E_TRANS_CARDEMU_TO_CARDDIS
	wifi_writeb(func, 0x0026, wifi_readb(func, 0x0026) | (BIT(7)));
	wifi_writeb(func, 0x0005, wifi_readb(func, 0x0005) | (BIT(3)));
	wifi_writeb(func, 0x0005, wifi_readb(func, 0x0005) & ~(BIT(4)));
	wifi_writeb(func, 0x0007, 0x00);
	wifi_writeb(func, 0x0041, wifi_readb(func, 0x0041) & ~(BIT(4)));
	wifi_writeb_local(func, 0x0086, wifi_readb_local(func, 0x0086) | (BIT(0)));
	usleep(20);

}

int wifi_fake_driver_probe_8189es(struct sdio_func *func)
{
	volatile u32 temp = 0;

	printk("wifi: wifi_fake_driver_probe_8189es enter\n");

	if (!func)
		return -1;
#ifdef CONFIG_HARDWARE_8821C
	wifi_card_enable_flow_8821cs(func);
#else
	wifi_card_enable_flow_8189es(func);
#endif



	printk("wifi: CMD52 read test:0x93 is 0x%x, 0x23 is 0x%x, 0x07 is 0x%x \n",
	 	wifi_readb(func, 0x93), wifi_readb(func, 0x23),
	 	wifi_readb(func, 0x07));

	printk("wifi: CMD52 read(08) test:0x3c is 0x%x \n",wifi_readb(func, 0x3c));
	printk("wifi: CMD52 read(08) test:0x3d is 0x%x \n",wifi_readb(func, 0x3d));
	printk("wifi: CMD52 read(08) test:0x3e is 0x%x \n",wifi_readb(func, 0x3e));
	printk("wifi: CMD52 read(08) test:0x3f is 0x%x \n",wifi_readb(func, 0x3f));
	printk("wifi: CMD52 read(08) test:0x4c is 0x%x \n",wifi_readb(func, 0x4c));
	printk("wifi: CMD52 read(08) test:0x4d is 0x%x \n",wifi_readb(func, 0x4d));
	printk("wifi: CMD52 read(08) test:0x4e is 0x%x \n",wifi_readb(func, 0x4e));
	printk("wifi: CMD52 read(08) test:0x4f is 0x%x \n",wifi_readb(func, 0x4f));
	usleep(500);
	printk("wifi: CMD53 read(16) test: 0x3c:%x\n", wifi_readw(func, 0x3c));
	printk("wifi: CMD53 read(32) test: 0x3c:%x\n", wifi_readl(func, 0x3c));
	printk("wifi: CMD53 read(16) test: 0x3e:%x\n", wifi_readw(func, 0x3e));
	printk("wifi: CMD53 read(16) test: 0x4c:%x\n", wifi_readw(func, 0x4c));
	printk("wifi: CMD53 read(32) test: 0x4c:%x\n", wifi_readl(func, 0x4c));
	printk("wifi: CMD53 read(16) test: 0x3c:%x\n", wifi_readw(func, 0x3c));


#ifdef CONFIG_HARDWARE_8821C
	wifi_card_disable_flow_8821cs(func);
#else
	wifi_card_disable_flow_8189es(func);
#endif

	printk("RTL871X(adapter): %s exit\n", __func__);

	return 0;
}


//extern int sdio_bus_probe(void);
//extern int sdio_bus_remove(void);
extern int sdio_register_driver(struct sdio_driver *);
SDIO_BUS_OPS rtw_sdio_bus_ops = {
	//sdio_bus_probe,
	//sdio_bus_remove,
	NULL,
	NULL,
	sdio_enable_func,
	sdio_disable_func,
	sdio_register_driver,//reg_driver
	NULL, //sdio_unregister_driver,//unreg_driver
	sdio_claim_irq,
	sdio_release_irq,
	sdio_claim_host,
	sdio_release_host,
	sdio_readb,
	sdio_readw,
	sdio_readl,
	sdio_writeb,
	sdio_writew,
	sdio_writel,
	sdio_memcpy_fromio,
	sdio_memcpy_toio
};
#endif

