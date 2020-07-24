/*
  USB Driver for GSM modems

  Copyright (C) 2005  Matthias Urlichs <smurf@smurf.noris.de>

  This driver is free software; you can redistribute it and/or modify
  it under the terms of Version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  Portions copied from the Keyspan driver by Hugh Blemings <hugh@blemings.org>

  History: see the git log.

  Work sponsored by: Sigos GmbH, Germany <info@sigos.de>

  This driver exists because the "normal" serial driver doesn't work too well
  with GSM modems. Issues:
  - data loss -- one single Receive URB is not nearly enough
  - nonstandard flow (Option devices) control
  - controlling the baud rate doesn't make sense

  This driver is named "option" because the most common device it's
  used for is a PC-Card (with an internal OHCI-USB interface, behind
  which the GSM interface sits), made by Option Inc.

  Some of the "one port" devices actually exhibit multiple USB instances
  on the USB bus. This is not a bug, these ports are used for different
  device features.
*/

#define DRIVER_AUTHOR "Matthias Urlichs <smurf@smurf.noris.de>"
#define DRIVER_DESC "USB Driver for GSM modems"

#include "serial-dbg.h"

#include <linux/os.h>
#include "ite/ite_usbex.h"
#include "usb/usb/usb.h"
#include "tty.h"

//#include <linux/kernel.h>
//#include <linux/jiffies.h>
#include <linux/errno.h>
//#include <linux/tty.h>
//#include <linux/tty_flip.h>
//#include <linux/slab.h>
#include <linux/module.h>
#include <linux/bitops.h>
//#include <linux/usb.h>
#include <linux/usb/serial.h>
#include "usb-wwan.h"

#define usb_alloc_urb(a,b)  usb_alloc_urb(a)
#define usb_submit_urb(a,b) usb_submit_urb(a)

/* Function prototypes */
static int  option_probe(struct usb_serial *serial,
			const struct usb_device_id *id);
static int option_attach(struct usb_serial *serial);
static void option_release(struct usb_serial *serial);
static void option_instat_callback(struct urb *urb);

/* Fibocom products */
#define FIBOCOM_VENDOR_ID 		0x2cb7
#define FIBOCOM_PRODUCT_L71X 	0x0001

#define FIBOCOM_USB_VENDOR_AND_INTERFACE_INFO(vend, cl, sc, pr) \
		.match_flags = USB_DEVICE_ID_MATCH_INT_INFO \
		| USB_DEVICE_ID_MATCH_VENDOR, \
		.idVendor = (vend), \
		.bInterfaceClass = (cl), \
		.bInterfaceSubClass = (sc), \
		.bInterfaceProtocol = (pr)


struct option_blacklist_info {
	/* bitmask of interface numbers blacklisted for send_setup */
	const unsigned long sendsetup;
	/* bitmask of interface numbers that are reserved */
	const unsigned long reserved;
};

static const struct option_blacklist_info neoway_3763_3C93_blacklist = {
	.reserved = BIT(0) | BIT(2) | BIT(3)  | BIT(4) | BIT(5),
};

static const struct option_blacklist_info neoway_05C6_8241_blacklist = {
	.reserved = BIT(1) | BIT(2) | BIT(3)  | BIT(4) | BIT(5) | BIT(6),
};

static const struct option_blacklist_info fibocom_2cb7_0001_blacklist = {
	.reserved = BIT(0) | BIT(1) | BIT(2) | BIT(3) | BIT(5) | BIT(6) | BIT(7),
};

static const struct option_blacklist_info air720sl_1286_4E3D_blacklist = {
	.reserved = BIT(0) | BIT(1) | BIT(3) | BIT(4),
};

static const struct usb_device_id option_ids[] = {
	{ USB_DEVICE(0x3763, 0x3C93),
		.driver_info = (unsigned long)&neoway_3763_3C93_blacklist },
	{ USB_DEVICE(0x05C6, 0x8241),
		.driver_info = (unsigned long)&neoway_05C6_8241_blacklist },
	{ FIBOCOM_USB_VENDOR_AND_INTERFACE_INFO(FIBOCOM_VENDOR_ID, 0xff, 0xff, 0xff),
		.driver_info = (unsigned long)&fibocom_2cb7_0001_blacklist },
    #if 0
    { FIBOCOM_USB_VENDOR_AND_INTERFACE_INFO(FIBOCOM_VENDOR_ID, 0x0a, 0x00, 0xff) },
    /* FIBOCOM: DL mode */
    { USB_DEVICE_AND_INTERFACE_INFO(0x19d2, 0x0256, 0xff, 0xff, 0xff) },
    { USB_DEVICE_AND_INTERFACE_INFO(0x19d2, 0x0579, 0xff, 0xff, 0xff) },
    #endif
	{ USB_DEVICE(0x1286, 0x4E3D),
		.driver_info = (unsigned long)&air720sl_1286_4E3D_blacklist },
	{ } /* Terminating entry */
};
//MODULE_DEVICE_TABLE(usb, option_ids);

/* The card has three separate interfaces, which the serial driver
 * recognizes separately, thus num_port=1.
 */

static struct usb_serial_driver option_1port_device = {
	.driver = {
		.owner =	THIS_MODULE,
		.name =		"option1",
	},
	.description       = "GSM modem (1-port)",
	.id_table          = option_ids,
	.num_ports         = 1,
	.probe             = option_probe,
	.open              = usb_wwan_open,
	.close             = usb_wwan_close,
	.dtr_rts	   = usb_wwan_dtr_rts,
	.write             = usb_wwan_write,
	.write_room        = usb_wwan_write_room,
	.chars_in_buffer   = usb_wwan_chars_in_buffer,
	.tiocmget          = usb_wwan_tiocmget,
	.tiocmset          = usb_wwan_tiocmset,
	.ioctl             = usb_wwan_ioctl,
	.attach            = option_attach,
	.release           = option_release,
	.port_probe        = usb_wwan_port_probe,
	.port_remove	   = usb_wwan_port_remove,
	.read_int_callback = option_instat_callback,
#ifdef CONFIG_PM
	.suspend           = usb_wwan_suspend,
	.resume            = usb_wwan_resume,
#endif
};

static struct usb_serial_driver * const serial_drivers[] = {
	&option_1port_device, NULL
};

#if 1
void iteUsbOptionRegister(void)
{
    usb_serial_register_drivers(serial_drivers, "option", option_ids);
}
#else
module_usb_serial_driver(serial_drivers, option_ids);
#endif

static int option_probe(struct usb_serial *serial,
			const struct usb_device_id *id)
{
	struct usb_interface_descriptor *iface_desc =
				&serial->interface->cur_altsetting->desc;
	struct usb_device_descriptor *dev_desc = &serial->dev->descriptor;
	const struct option_blacklist_info *blacklist;

	/* Never bind to the CD-Rom emulation interface	*/
	if (iface_desc->bInterfaceClass == 0x08)
		return -ENODEV;

	/*
	 * Don't bind reserved interfaces (like network ones) which often have
	 * the same class/subclass/protocol as the serial interfaces.  Look at
	 * the Windows driver .INF files for reserved interface numbers.
	 */
	blacklist = (void *)id->driver_info;
	if (blacklist && test_bit(iface_desc->bInterfaceNumber,
						&blacklist->reserved))
		return -ENODEV;

	/* Store the blacklist info so we can use it during attach. */
	usb_set_serial_data(serial, (void *)blacklist);

	#if 1 // Irene Lin
	serial->dev->type = USB_DEVICE_TYPE_OPTION;
	#endif

	return 0;
}

static int option_attach(struct usb_serial *serial)
{
	struct usb_interface_descriptor *iface_desc;
	const struct option_blacklist_info *blacklist;
	struct usb_wwan_intf_private *data;

	data = kzalloc(sizeof(struct usb_wwan_intf_private), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	/* Retrieve blacklist info stored at probe. */
	blacklist = usb_get_serial_data(serial);

	iface_desc = &serial->interface->cur_altsetting->desc;

	if (!blacklist || !test_bit(iface_desc->bInterfaceNumber,
						&blacklist->sendsetup)) {
		data->use_send_setup = 1;
	}
	spin_lock_init(&data->susp_lock);

	usb_set_serial_data(serial, data);

	return 0;
}

static void option_release(struct usb_serial *serial)
{
	struct usb_wwan_intf_private *intfdata = usb_get_serial_data(serial);

	kfree(intfdata);
}

static void option_instat_callback(struct urb *urb)
{
	int err;
	int status = urb->status;
	struct usb_serial_port *port = urb->context;
	struct device *dev = &port->dev;
	struct usb_wwan_port_private *portdata =
					usb_get_serial_port_data(port);

	dev_dbg(dev, "%s: urb %p port %p has data %p\n", __func__, urb, port, portdata);

	if (status == 0) {
		struct usb_ctrlrequest *req_pkt =
				(struct usb_ctrlrequest *)urb->transfer_buffer;

		if (!req_pkt) {
			dev_dbg(dev, "%s: NULL req_pkt\n", __func__);
			return;
		}
		if ((req_pkt->bRequestType == 0xA1) &&
				(req_pkt->bRequest == 0x20)) {
			int old_dcd_state;
			unsigned char signals = *((unsigned char *)
					urb->transfer_buffer +
					sizeof(struct usb_ctrlrequest));

			dev_dbg(dev, "%s: signal x%x\n", __func__, signals);

			old_dcd_state = portdata->dcd_state;
			portdata->cts_state = 1;
			portdata->dcd_state = ((signals & 0x01) ? 1 : 0);
			portdata->dsr_state = ((signals & 0x02) ? 1 : 0);
			portdata->ri_state = ((signals & 0x08) ? 1 : 0);

			if (old_dcd_state && !portdata->dcd_state)
				tty_port_tty_hangup(&port->port, true);
		} else {
			dev_dbg(dev, "%s: type %x req %x\n", __func__,
				req_pkt->bRequestType, req_pkt->bRequest);
		}
	} else if (status == -ENOENT || status == -ESHUTDOWN) {
		dev_dbg(dev, "%s: urb stopped: %d\n", __func__, status);
	} else
		dev_dbg(dev, "%s: error %d\n", __func__, status);

	/* Resubmit urb so we continue receiving IRQ data */
	if (status != -ESHUTDOWN && status != -ENOENT) {
		usb_mark_last_busy(port->serial->dev);
		err = usb_submit_urb(urb, GFP_ATOMIC);
		if (err)
			dev_dbg(dev, "%s: resubmit intr urb failed. (%d)\n",
				__func__, err);
	}
}

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
