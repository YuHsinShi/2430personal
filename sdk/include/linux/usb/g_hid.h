/*
 * g_hid.h -- Header file for USB HID gadget driver
 *
 * Copyright (C) 2010 Fabien Chouteau <fabien.chouteau@barco.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __LINUX_USB_G_HID_H
#define __LINUX_USB_G_HID_H

#include <linux/list.h>

#define ITP_IOCTL_USBD_HIHG_REFRESH	     			  ITP_IOCTL_CUSTOM_CTL_ID1
#define ITP_IOCTL_USBD_HIHG_SET_CONFIG	  			  ITP_IOCTL_CUSTOM_CTL_ID2
#define ITP_IOCTL_USBD_HIHG_READ                      ITP_IOCTL_CUSTOM_CTL_ID3
#define ITP_IOCTL_USBD_HIHG_WRITE                     ITP_IOCTL_CUSTOM_CTL_ID4
#define ITP_IOCTL_USBD_HIHG_OVERWRITE_VIDPID          ITP_IOCTL_CUSTOM_CTL_ID5

enum UsbdHidgTypeID
{
    USBD_HIDG_NONE = 0,
    USBD_HIDG_DEV,
    USBD_HIDG_MOUSE,
	USBD_HIDG_KBD,
	USBD_HIDG_MAX
};

enum UsbdHidgIdx
{
    USBD_IDX_NONE = 0,
    USBD_IDX_0,
    USBD_IDX_1,
    USBD_IDX_MAX
};

typedef struct hidg_func_config {
    const char          *name;
    struct usb_function *hidg_func;
    struct list_head 	list;
    unsigned short		idx;
    unsigned short		typeID;
	unsigned short		idVendor;
	unsigned short		idProduct;
	unsigned char		subclass;
	unsigned char		protocol;
	unsigned short		report_length;
	unsigned short		report_desc_length;
	unsigned char		*report_desc;
	unsigned char		default_report_desc[];
} HIDG_FUNC_CONFIG;

typedef struct hidg_func_cmd_data
{
    unsigned short      idx;
    unsigned short		typeID;
	unsigned short		idVendor;
	unsigned short		idProduct;
	size_t		        count;
    unsigned char		*buffer;
} HIDG_FUNC_CMD_DATA;

#endif /* __LINUX_USB_G_HID_H */
