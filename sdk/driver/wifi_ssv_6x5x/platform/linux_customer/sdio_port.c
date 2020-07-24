/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <log.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <hctrl.h>
#include <linux/sizes.h>
#if defined(CFG_MMC_ENABLE)
#include "ite/ite_sdio.h"
#include <linux/os.h>
#else
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#endif

#include "sdio_port.h"
#include "sdio_def.h"


//#define SDIO_PORT_DEBUG

#define SDIO_INDEX 0
#define SDIO_DEFAULT_BLOCK_SIZE  64 //256
#define BUILD_SDIO_CMD_ITE          1
#define BUILD_SDIO_CMD_SSV          1


typedef void (*SDIO_ISR_FUNC)(void);
SDIO_ISR_FUNC g_sdio_isr_func = NULL;


/* =====WIFI driver declaration ==== */
static int ssv6xxx_sdio_probe(struct sdio_func *func, const struct sdio_device_id *id);
static void ssv6xxx_sdio_remove(struct sdio_func *func);
static int  ssv6xxx_wifi_fake_driver_probe(struct sdio_func *func);
static void ssv6xxx_wifi_card_enable_flow(struct sdio_func *func);
static void ssv6xxx_wifi_card_disable_flow(struct sdio_func *func);
/* ======================== */


/* ===== SDIO driver register ===== */
struct sdio_func *g_func;

static const struct sdio_device_id ssv6xxx_sdio_devices[] =
{
    { SDIO_DEVICE(SSV_VENDOR_ID, SSV_CABRIO_DEVID), .driver_data = 0 }
};

struct sdio_driver ssv6xxx_sdio_driver =
{
    .name		= "SSV6XXX_SDIO",
    .id_table	= ssv6xxx_sdio_devices,
    .probe		= ssv6xxx_sdio_probe,
    .remove     = ssv6xxx_sdio_remove
};
/* ======================== */


ssv_type_u32 dbg_sdio_clk;
int sdio_set_clk(ssv_type_u32 clk)
{
	if (g_func) {
		/* set min to 1M */
		/*if (g_func->card->host->ops->set_ios && clk >= 1000000) {
			sdio_claim_host(g_func);
			g_func->card->host->ios.clock = (clk < 50000000) ? clk : 50000000;
			g_func->card->host->ops->set_ios(g_func->card->host,
			     &g_func->card->host->ios);
			sdio_release_host(g_func);
			SDIO_TRACE("%s:change mmc clk=%d\n", __func__,
				    g_func->card->host->ios.clock);
		} else {
			SDIO_TRACE("%s:fail change mmc clk=%d\n",
				    __func__, clk);
		}*/
	}
	return 0;
	SDIO_TRACE("%s\n", __func__);
}


/* =============== SSV WIFI CMD52/53 =============== */
static ssv_type_u32 sdio_block_size;
#if BUILD_SDIO_CMD_SSV
ssv_type_u8	sdio_readb_cmd52(ssv_type_u32 addr, ssv_type_u8 *data)
{
    ssv_type_u8 out;
    int ret;

    sdio_claim_host(g_func);
    out = sdio_readb(g_func, addr, &ret);
    sdio_release_host(g_func);
    if (0 != ret)
    {
        SDIO_ERROR("sdio_cmd52(read), ret = %d\r\n", ret);
        *data=0;
        return false;
    }
    *data=out;
	SDIO_TRACE("%-20s : 0x%08x, 0x%02x\n", "sdio_readb_cmd52", addr, out);

    return true;
}

ssv_type_bool sdio_writeb_cmd52(ssv_type_u32 addr, ssv_type_u8 data)
{
    int ret=0;
    sdio_claim_host(g_func);
    sdio_writeb(g_func,data, addr, &ret);
    sdio_release_host(g_func);
    if (0 != ret)
    {
        SDIO_ERROR("sdio_cmd52(write), ret = %d\r\n", ret);
        return false;
    }

	SDIO_TRACE("%-20s : 0x%08x, 0x%02x\n", "sdio_writeb_cmd52", addr, data);

    return true;

}

ssv_type_bool sdio_read_cmd53(ssv_type_u32 dataPort, ssv_type_u8 *dat, size_t size)
{
	ssv_type_u32 rx_blocks = 0, blksize = 0;
    ssv_type_u32 ret = 0;

    if (((unsigned int)dat & 3) || (size & 3))
    {
        SDIO_ERROR("data and len must be align 4 byte, data = 0x%08x, size = %d\r\n", (unsigned int)dat, size);
        return false;
    }

    if (size > sdio_block_size && sdio_block_size != 0)
    {
        rx_blocks = (size + sdio_block_size - 1) / sdio_block_size;
        blksize = sdio_block_size;
    }
    else
    {
        blksize = size;
        rx_blocks = 1;
    }

    sdio_claim_host(g_func);

    ret = sdio_memcpy_fromio(g_func, (void*)dat, dataPort, blksize*rx_blocks);

    sdio_release_host(g_func);
	if (0 != ret)
	{
        SDIO_ERROR("sgm_sdc_api_dio_cmd53(read), ret = %d\r\n", ret);
	    return false;
	}

	SDIO_TRACE("%-20s : 0x%x, 0x%08x, 0x%02x\n", "sdio_read_cmd53", dataPort, dat, size);

	return true;
}

ssv_type_bool sdio_write_cmd53(ssv_type_u32 dataPort, ssv_type_u8 *dat, size_t size)
{
	ssv_type_u32 tx_blocks = 0, blksize = 0;
    int ret = 0, writesize;

    if (((unsigned int)dat & 3) || (size & 3))
    {
        SDIO_ERROR("data and len must be align 4 byte, data = 0x%08x, size = %d\r\n", (unsigned int)dat, size);
        return false;
    }


    if (size > sdio_block_size && sdio_block_size != 0)
    {
        tx_blocks = (size + sdio_block_size - 1) / sdio_block_size;
        blksize = sdio_block_size;
    }
    else
    {
        blksize = size;
        tx_blocks = 1;
    }

    sdio_claim_host(g_func);
    writesize = blksize*tx_blocks;

    do
    {
        ret = sdio_memcpy_toio(g_func, dataPort, (void*)dat, writesize);
        if ( ret == -EILSEQ || ret == -ETIMEDOUT )
        {
            ret = -1;
            break;
        }
        else
        {
            if(ret)
                SDIO_ERROR("Unexpected return value ret=[%d]\n",ret);
                //dev_err(glue->dev,"Unexpected return value ret=[%d]\n",ret);
        }
    }
    while( ret == -EILSEQ || ret == -ETIMEDOUT);
    sdio_release_host(g_func);

    if (0 != ret)
    {
        SDIO_ERROR("sdio_cmd53(write), ret = %d\r\n", ret);
        return false;
    }

	SDIO_TRACE("%-20s : 0x%x, 0x%p, 0x%02x\n", "sdio_write_cmd53", dataPort, dat, size);

	return true;
}
#endif
/* ============================================ */

static void sdio_host_isr (struct sdio_func *func)
{

    sdio_host_enable_isr(false); //Disable first
    if (g_sdio_isr_func)
    {
        g_sdio_isr_func();
    }

}

ssv_type_bool sdio_host_enable_isr(ssv_type_bool enable)
{
    int ret = 0;


    sdio_claim_host(g_func);

    if(enable)
        ret =  sdio_claim_irq(g_func, sdio_host_isr);
    else
        ret = sdio_release_irq(g_func);

    if (0 != ret)
    {
        SDIO_ERROR("gm_sdc_api_action, ret = %d enable = %d\r\n", ret, enable);
        return false;
    }

	SDIO_TRACE("%-20s : %d\n", "sdio_host_enable_isr", enable);

    sdio_release_host(g_func);

    return true;
}


ssv_type_u32 sdio_get_block_size(void)
{
    return sdio_block_size;
}

ssv_type_bool is_truly_isr(void)
{
    return TRUE;
}


ssv_type_bool sdio_host_detect_card(void)
{
    /**/
    return TRUE;

}


ssv_type_bool sdio_host_init(void (*sdio_isr)(void))
{
    ssv_type_u32 ret = 0;
    ssv_type_u8 in, out;

    sdio_claim_host(g_func);

    /* set block size */
    sdio_set_block_size(g_func,SDIO_DEFAULT_BLOCK_SIZE);

    /* enable sdio control interupt */
    /* install isr here */
    g_sdio_isr_func = sdio_isr;
    ret =  sdio_claim_irq(g_func, sdio_host_isr);
    if (ret)
    {
        SDIO_ERROR("sdio_host_init, ret = %d\r\n", ret);
        return false;
    }

    sdio_release_host(g_func);
    // output timing
    if (!sdio_writeb_cmd52(REG_OUTPUT_TIMING_REG, SDIO_DEF_OUTPUT_TIMING))
        SDIO_FAIL_RET(0, "sdio_write_byte(0x55, %d)\n", SDIO_DEF_OUTPUT_TIMING);
    LOG_PRINTF("output timing to %d (0x%08x)\n", SDIO_DEF_OUTPUT_TIMING, SDIO_DEF_OUTPUT_TIMING);

	SDIO_TRACE("%-20s : %s\n", "host_int", "ok");

    return true;
}


/* =============== ITE WIFI CMD52/53 =============== */
#if BUILD_SDIO_CMD_ITE

/* test wifi driver */
#define ADDR_MASK       0x10000
#define LOCAL_ADDR_MASK 0x00000
#ifndef BIT
#define BIT(_x)	        (1 << (_x))
#endif

int wifi_read(struct sdio_func *func, ssv_type_u32 addr, ssv_type_u32 cnt, void *pdata)
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

int wifi_write(struct sdio_func *func, ssv_type_u32 addr, ssv_type_u32 cnt, void *pdata)
{
	int err;
	ssv_type_u32 size;

	sdio_claim_host(func);

	size = cnt;
	err = sdio_memcpy_toio(func, addr, pdata, size);
	if (err) {
		printk("%s: FAIL(%d)! ADDR=%#x Size=%d(%d)\n", __func__, err, addr, cnt, size);
	}

	sdio_release_host(func);

	return err;
}

ssv_type_u8 wifi_readb(struct sdio_func *func, ssv_type_u32 addr)
{
	int err;
	ssv_type_u8 ret = 0;

	sdio_claim_host(func);
	ret = sdio_readb(func, ADDR_MASK | addr, &err);
	sdio_release_host(func);

	if (err)
		printk("%s: FAIL!(%d) addr=0x%05x\n", __func__, err, addr);

	return ret;
}

ssv_type_u16 wifi_readw(struct sdio_func *func, ssv_type_u32 addr)
{
	int err;
	ssv_type_u16 v;

	sdio_claim_host(func);
	v = sdio_readw(func, ADDR_MASK | addr, &err);
	sdio_release_host(func);
	if (err)
		printk("%s: FAIL!(%d) addr=0x%05x\n", __func__, err, addr);

	return  v;
}

ssv_type_u32 wifi_readl(struct sdio_func *func, ssv_type_u32 addr)
{
	int err;
	ssv_type_u32 v;

	sdio_claim_host(func);
	v = sdio_readl(func, ADDR_MASK | addr, &err);
	sdio_release_host(func);

	return  v;
}

void wifi_writeb(struct sdio_func *func, ssv_type_u32 addr, ssv_type_u8 val)
{
	int err;

	sdio_claim_host(func);
	sdio_writeb(func, val, ADDR_MASK | addr, &err);
	sdio_release_host(func);
	if (err)
		printk("%s: FAIL!(%d) addr=0x%05x val=0x%02x\n", __func__, err, addr, val);
}

void wifi_writew(struct sdio_func *func, ssv_type_u32 addr, ssv_type_u16 v)
{
	int err;

	sdio_claim_host(func);
	sdio_writew(func, v, ADDR_MASK | addr, &err);
	sdio_release_host(func);
	if (err)
		printk("%s: FAIL!(%d) addr=0x%05x val=0x%04x\n", __func__, err, addr, v);
}

void wifi_writel(struct sdio_func *func, ssv_type_u32 addr, ssv_type_u32 v)
{
	int err;

	sdio_claim_host(func);
	sdio_writel(func, v, ADDR_MASK | addr, &err);
	sdio_release_host(func);
}

ssv_type_u8 wifi_readb_local(struct sdio_func *func, ssv_type_u32 addr)
{
    int err;
    ssv_type_u8 ret = 0;
    sdio_claim_host(func);
    ret = sdio_readb(func, LOCAL_ADDR_MASK | addr, &err);
    sdio_release_host(func);

    return ret;
}

void wifi_writeb_local(struct sdio_func *func, ssv_type_u32 addr, ssv_type_u8 val)
{
    int err;
    sdio_claim_host(func);
    sdio_writeb(func, val, LOCAL_ADDR_MASK | addr, &err);
    sdio_release_host(func);

}

static void ssv6xxx_wifi_card_enable_flow(struct sdio_func *func)
{
	wifi_writeb_local(func, 0x0086, wifi_readb_local(func, 0x0086) & (~(BIT(0))));
	usleep(20);
	wifi_writeb(func, 0x0005, wifi_readb(func, 0x0005) & ~(BIT(3)|BIT(4)));

	usleep(20);
	wifi_writeb(func, 0x0002, wifi_readb(func, 0x0002) & ~(BIT(1)|BIT(0)));
	wifi_writeb(func, 0x0026, wifi_readb(func, 0x0027) | (BIT(7)));
	wifi_writeb(func, 0x0005, wifi_readb(func, 0x0005) & ~(BIT(7)));
	wifi_writeb(func, 0x0005, wifi_readb(func, 0x0005) & ~(BIT(4)|BIT(3)));
	wifi_writeb(func, 0x0005, wifi_readb(func, 0x0005) | (BIT(0)));
	usleep(20);
	wifi_writeb(func, 0x0023, wifi_readb(func, 0x0023) & ~(BIT(4)));
	wifi_writeb(func, 0x0074, wifi_readb(func, 0x0074) | (BIT(4)));
}


static void ssv6xxx_wifi_card_disable_flow(struct sdio_func *func)
{
	wifi_writeb(func, 0x001F, 0x00);
	wifi_writeb(func, 0x0023, wifi_readb(func, 0x0023) | (BIT(4)));
	wifi_writeb(func, 0x0005, wifi_readb(func, 0x0005) | (BIT(1)));
	usleep(20);

	wifi_writeb(func, 0x0026, wifi_readb(func, 0x0026) | (BIT(7)));
	wifi_writeb(func, 0x0005, wifi_readb(func, 0x0005) | (BIT(3)));
	wifi_writeb(func, 0x0005, wifi_readb(func, 0x0005) & ~(BIT(4)));
	wifi_writeb(func, 0x0007, 0x00);
	wifi_writeb(func, 0x0041, wifi_readb(func, 0x0041) & ~(BIT(4)));
	wifi_writeb_local(func, 0x0086, wifi_readb_local(func, 0x0086) | (BIT(0)));
	usleep(20);

}


static int ssv6xxx_wifi_fake_driver_probe(struct sdio_func *func)
{
    printk("wifi: ssv6xxx_wifi_fake_driver_probe enter\n");
    ssv6xxx_wifi_card_enable_flow(func);

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

    ssv6xxx_wifi_card_disable_flow(func);

    printk("SSV6XXX(adapter): %s exit\n", __func__);

	return 0;
}
#endif
/* ============================================ */


static int ssv6xxx_sdio_probe(struct sdio_func *func, const struct sdio_device_id *id)
{
    printk("====>ssv6xxx_sdio_probe: hook sdio_func!! vendor=0x%04x device=0x%04x class=0x%02x\n",
    	func->vendor, func->device, func->class);
    g_func = NULL;
    g_func = func;
    return 0;
}


static void ssv6xxx_sdio_remove(struct sdio_func *func)
{
    printk("====>ssv6xxx_sdio_remove: dev_remove()\n");
    return;
}


int ssv6xxx_sdio_init(void)
{
    printk(KERN_INFO "ssv6xxx_sdio_init\n");
    return sdio_register_driver(&ssv6xxx_sdio_driver);
}


void ssv6xxx_sdio_exit(void)
{
    printk(KERN_INFO "ssv6xxx_sdio_exit\n");
#if !defined(CFG_MMC_ENABLE)
    sdio_unregister_driver(&ssv6xxx_sdio_driver);
#endif
}

extern ssv_type_s32 ssv_main(ssv_type_s32 argc, char *argv[]);
int mmpSsvWifiDriverCmdTest(void)
{
    ssv_type_s32 init_ret;

    /* Start to initial STA/AP mode in main task */
    init_ret = ssv_main(0, NULL);
    printk("====>SSV SDIO WIFI Tasklet: %d\n", (init_ret == OS_SUCCESS) ? "ON":"OFF");

    usleep(500*1000);

    printk("====>Do mmpSsvWifiDriverCmdTest(%d): we call entry function or CMD test there.... \n", init_ret);

#if BUILD_SDIO_CMD_ITE
    ssv6xxx_wifi_fake_driver_probe(g_func);
#endif

    while(init_ret){
        printk("====>Wait SSV SDIO WIFI Tasklet ready...\n");
        usleep(100*1000);
    }

    return init_ret;
}


