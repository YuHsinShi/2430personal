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


#ifndef __SSVDEVICE_H__
#define __SSVDEVICE_H__

#include "linux/device.h"

#define   KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
/* emulate a modern version */
#define LINUX_VERSION_CODE KERNEL_VERSION(2, 6, 17)
#define to_platform_driver(drv)	 container_of((drv), struct platform_driver, driver)
#define to_platform_device(x)    container_of((x), struct platform_device, dev)


//Porting form Linux 5.5.11
struct platform_device_id {
	char name[20];
	unsigned char driver_data;
};

struct platform_device {
	const char      *name;
	int             id;
	bool            id_auto;
	struct device   dev;
	ssv_type_u64    platform_dma_mask;
	ssv_type_u32    num_resources;
	struct resource *resource;

	const struct platform_device_id	*id_entry;
	char            *driver_override; /* Driver name to force a match */

	/* MFD cell pointer */
	struct mfd_cell *mfd_cell;

	/* arch specific additions */
	//struct pdev_archdata	archdata;
};

struct platform_driver {
	int (*probe)(struct platform_device *);
	int (*remove)(struct platform_device *);
	void (*shutdown)(struct platform_device *);
	//int (*suspend)(struct platform_device *, pm_message_t state);
	int (*resume)(struct platform_device *);
	struct device_driver driver;
	const struct platform_device_id *id_table;
	//bool prevent_deferred_probe;
};

struct ssv6xxx_platform_data {
    //use to avoid remove mmc cause dead lock.
    //atomic_t                   irq_handling;
    bool                        is_enabled;
    //u8                          chip_id[SSV6XXX_CHIP_ID_LENGTH];
    //u8                          short_chip_id[SSV6XXX_CHIP_ID_SHORT_LENGTH+1];
    unsigned short              vendor;		/* vendor id */
    unsigned short              device;		/* device id */
    void *hwif_hdl;
    void *devinfo;
};
#endif
