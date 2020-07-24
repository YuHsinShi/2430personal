/*
 * Copyright (c) 2014 South Silicon Valley Microelectronics Inc.
 * Copyright (c) 2015 iComm Semiconductor Ltd.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
//#include <linux/kernel.h>
//#include <linux/module.h>
//#include <linux/timer.h>
#include <linux/errno.h>

#include <host_config.h>
#include <ssv_common.h>
#include <ssv_devinfo.h>
#include "ssvdevice.h"

extern int ssv6xxx_sdio_init(void);
static int __init ssvdevice_init(void)
{
    int ret = 0;

    if(!ret){
        ret = ssv6xxx_sdio_init();
    }
    printk(KERN_INFO "333ssvdevice_init ret=%d\r\n",ret);

    return ret;
}
 extern void ssv6xxx_sdio_exit(void);
 static void __exit ssvdevice_exit(void)
 {
    printk(KERN_INFO "ssvdevice_exit\r\n");
    ssv6xxx_sdio_exit();
    msleep(150);
    printk(KERN_INFO "unregister ssv6xxx_driver\r\n");
}

int mmpSsvWifiDriverRegister(void)
{
	int ret;

	printk("====>Do mmpSsvWifiDriverRegister....\n");
	ret = ssvdevice_init();

	return ret;
}

module_init(ssvdevice_init);
module_exit(ssvdevice_exit);
//module_param_named(devicetype,ssv_devicetype, uint , S_IRUSR | S_IWUSR);
MODULE_DESCRIPTION("IEEE 802.11 subsystem");
MODULE_LICENSE("GPL");


