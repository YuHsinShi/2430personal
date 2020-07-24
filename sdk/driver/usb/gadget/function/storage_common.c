/*
* storage_common.c -- Common definitions for mass storage functionality
*
* Copyright (C) 2003-2008 Alan Stern
* Copyeight (C) 2009 Samsung Electronics
* Author: Michal Nazarewicz (mina86@mina86.com)
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*/

/*
* This file requires the following identifiers used in USB strings to
* be defined (each of type pointer to char):
*  - fsg_string_interface    -- name of the interface
*/

/*
* When USB_GADGET_DEBUG_FILES is defined the module param num_buffers
* sets the number of pipeline buffers (length of the fsg_buffhd array).
* The valid range of num_buffers is: num >= 2 && num <= 4.
*/

#include <linux/module.h>
//#include <linux/blkdev.h>
//#include <linux/file.h>
//#include <linux/fs.h>
#include <linux/usb/composite.h>

#include "storage_common.h"

/* There is only one interface. */

struct usb_interface_descriptor fsg_intf_desc = {
    .bLength = sizeof fsg_intf_desc,
    .bDescriptorType = USB_DT_INTERFACE,

    .bNumEndpoints = 2,		/* Adjusted during fsg_bind() */
    .bInterfaceClass = USB_CLASS_MASS_STORAGE,
    .bInterfaceSubClass = USB_SC_SCSI,	/* Adjusted during fsg_bind() */
    .bInterfaceProtocol = USB_PR_BULK,	/* Adjusted during fsg_bind() */
    .iInterface = FSG_STRING_INTERFACE,
};
EXPORT_SYMBOL_GPL(fsg_intf_desc);

/*
* Three full-speed endpoint descriptors: bulk-in, bulk-out, and
* interrupt-in.
*/

struct usb_endpoint_descriptor fsg_fs_bulk_in_desc = {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,

    .bEndpointAddress = USB_DIR_IN,
    .bmAttributes = USB_ENDPOINT_XFER_BULK,
    /* wMaxPacketSize set by autoconfiguration */
};
EXPORT_SYMBOL_GPL(fsg_fs_bulk_in_desc);

struct usb_endpoint_descriptor fsg_fs_bulk_out_desc = {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,

    .bEndpointAddress = USB_DIR_OUT,
    .bmAttributes = USB_ENDPOINT_XFER_BULK,
    /* wMaxPacketSize set by autoconfiguration */
};
EXPORT_SYMBOL_GPL(fsg_fs_bulk_out_desc);

struct usb_descriptor_header *fsg_fs_function[] = {
    (struct usb_descriptor_header *) &fsg_intf_desc,
    (struct usb_descriptor_header *) &fsg_fs_bulk_in_desc,
    (struct usb_descriptor_header *) &fsg_fs_bulk_out_desc,
    NULL,
};
EXPORT_SYMBOL_GPL(fsg_fs_function);


/*
* USB 2.0 devices need to expose both high speed and full speed
* descriptors, unless they only run at full speed.
*
* That means alternate endpoint descriptors (bigger packets).
*/
struct usb_endpoint_descriptor fsg_hs_bulk_in_desc = {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,

    /* bEndpointAddress copied from fs_bulk_in_desc during fsg_bind() */
    .bmAttributes = USB_ENDPOINT_XFER_BULK,
    .wMaxPacketSize = cpu_to_le16(512),
};
EXPORT_SYMBOL_GPL(fsg_hs_bulk_in_desc);

struct usb_endpoint_descriptor fsg_hs_bulk_out_desc = {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,

    /* bEndpointAddress copied from fs_bulk_out_desc during fsg_bind() */
    .bmAttributes = USB_ENDPOINT_XFER_BULK,
    .wMaxPacketSize = cpu_to_le16(512),
    .bInterval = 1,	/* NAK every 1 uframe */
};
EXPORT_SYMBOL_GPL(fsg_hs_bulk_out_desc);


struct usb_descriptor_header *fsg_hs_function[] = {
    (struct usb_descriptor_header *) &fsg_intf_desc,
    (struct usb_descriptor_header *) &fsg_hs_bulk_in_desc,
    (struct usb_descriptor_header *) &fsg_hs_bulk_out_desc,
    NULL,
};
EXPORT_SYMBOL_GPL(fsg_hs_function);

struct usb_endpoint_descriptor fsg_ss_bulk_in_desc = {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,

    /* bEndpointAddress copied from fs_bulk_in_desc during fsg_bind() */
    .bmAttributes = USB_ENDPOINT_XFER_BULK,
    .wMaxPacketSize = cpu_to_le16(1024),
};
EXPORT_SYMBOL_GPL(fsg_ss_bulk_in_desc);

struct usb_ss_ep_comp_descriptor fsg_ss_bulk_in_comp_desc = {
    .bLength = sizeof(fsg_ss_bulk_in_comp_desc),
    .bDescriptorType = USB_DT_SS_ENDPOINT_COMP,

    /*.bMaxBurst =		DYNAMIC, */
};
EXPORT_SYMBOL_GPL(fsg_ss_bulk_in_comp_desc);

struct usb_endpoint_descriptor fsg_ss_bulk_out_desc = {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,

    /* bEndpointAddress copied from fs_bulk_out_desc during fsg_bind() */
    .bmAttributes = USB_ENDPOINT_XFER_BULK,
    .wMaxPacketSize = cpu_to_le16(1024),
};
EXPORT_SYMBOL_GPL(fsg_ss_bulk_out_desc);

struct usb_ss_ep_comp_descriptor fsg_ss_bulk_out_comp_desc = {
    .bLength = sizeof(fsg_ss_bulk_in_comp_desc),
    .bDescriptorType = USB_DT_SS_ENDPOINT_COMP,

    /*.bMaxBurst =		DYNAMIC, */
};
EXPORT_SYMBOL_GPL(fsg_ss_bulk_out_comp_desc);

struct usb_descriptor_header *fsg_ss_function[] = {
    (struct usb_descriptor_header *) &fsg_intf_desc,
    (struct usb_descriptor_header *) &fsg_ss_bulk_in_desc,
    (struct usb_descriptor_header *) &fsg_ss_bulk_in_comp_desc,
    (struct usb_descriptor_header *) &fsg_ss_bulk_out_desc,
    (struct usb_descriptor_header *) &fsg_ss_bulk_out_comp_desc,
    NULL,
};
EXPORT_SYMBOL_GPL(fsg_ss_function);


/*-------------------------------------------------------------------------*/


/*
 * If the next two routines are called while the gadget is registered,
 * the caller must own fsg->filesem for writing.
 */

void fsg_lun_close(struct fsg_lun *curlun)
{
	if (curlun->id == 0)
		curlun->ops->close();
}
EXPORT_SYMBOL_GPL(fsg_lun_close);

int fsg_lun_open(struct fsg_lun *curlun, const char *filename)
{
	int				rc = -EINVAL;
	uint32_t				num_sectors;
	unsigned int			blkbits;
	unsigned int			blksize;

    if (curlun->id == 0)
		curlun->ops->open();

	LDBG(curlun, "open backing file: %s\n", curlun->name);
	return 0;
}
EXPORT_SYMBOL_GPL(fsg_lun_open);


/*-------------------------------------------------------------------------*/

/*
 * Sync the file data, don't bother with the metadata.
 * This code was copied from fs/buffer.c:sys_fdatasync().
 */
int fsg_lun_fsync_sub(struct fsg_lun *curlun)
{
	return 0;
}
EXPORT_SYMBOL_GPL(fsg_lun_fsync_sub);

