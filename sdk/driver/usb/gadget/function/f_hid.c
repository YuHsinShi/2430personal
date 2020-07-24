/*
 * f_hid.c -- USB HID function driver
 *
 * Copyright (C) 2010 Fabien Chouteau <fabien.chouteau@barco.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

//#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hid.h>
//#include <linux/idr.h>
//#include <linux/cdev.h>
//#include <linux/mutex.h>
//#include <linux/poll.h>
//#include <linux/uaccess.h>
//#include <linux/wait.h>
//#include <linux/sched.h>
#include <linux/usb/g_hid.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/usb/composite.h>
#include <linux/err.h>

#include "../u_f.h"
#include "u_hid.h"

//#define HIDG_MINORS	4
//#define OUTEP_ENABLE
//static int major, minors;
//static struct class *hidg_class;
//static DEFINE_IDA(hidg_ida);
//static DEFINE_MUTEX(hidg_ida_lock); /* protects access to hidg_ida */

/*-------------------------------------------------------------------------*/
/*                            HID gadget struct                            */
#ifdef OUTEP_ENABLE
struct f_hidg_req_list {
	struct usb_request	*req;
	unsigned int		pos;
	struct list_head 	list;
};
#endif
/**
 * @brief
 *
 * @extends usb_function
 * @extends cdev
 */
struct f_hidg {
	/* configuration */
	unsigned char			bInterfaceSubClass;
	unsigned char			bInterfaceProtocol;
	unsigned short			report_desc_length;
	char				*report_desc;
	unsigned short			report_length;

	/* recv report */
#ifdef OUTEP_ENABLE
    struct list_head		completed_out_req;
    unsigned int          qlen;
#else
	char				*set_report_buff;
	unsigned short		set_report_length;
#endif
	spinlock_t			read_spinlock;
	wait_queue_head_t		read_queue;

	/* send report */
	spinlock_t			write_spinlock;
	bool				write_pending;
	wait_queue_head_t		write_queue;
	struct usb_request		*req;   // 存放要發給host的usb request

	//int				minor;
	//struct cdev			cdev;

	struct usb_function		func;

	struct usb_ep			*in_ep;
#ifdef OUTEP_ENABLE
	struct usb_ep			*out_ep;
#endif
    struct hidg_func_config *f_conf;
};

static inline struct f_hidg *func_to_hidg(struct usb_function *f) {
	return container_of(f, struct f_hidg, func);
}

/*-------------------------------------------------------------------------*/
/*                           Static descriptors                            */
/**
 * @brief HID interface descriptor
 */
static struct usb_interface_descriptor hidg_interface_desc = {
	.bLength		= sizeof hidg_interface_desc,
	.bDescriptorType	= USB_DT_INTERFACE,
	/* .bInterfaceNumber	= DYNAMIC */
	.bAlternateSetting	= 0,
#ifdef OUTEP_ENABLE
	.bNumEndpoints		= 2,
#else
	.bNumEndpoints		= 1,
#endif
	.bInterfaceClass	= USB_CLASS_HID,
	/* .bInterfaceSubClass	= DYNAMIC */
	/* .bInterfaceProtocol	= DYNAMIC */
	/* .iInterface		= DYNAMIC */
};

static struct hid_descriptor hidg_desc = {
	.bLength			= sizeof hidg_desc,
	.bDescriptorType		= HID_DT_HID,
	.bcdHID				= 0x0101,
	.bCountryCode			= 0x00,
	.bNumDescriptors		= 0x1,
	/*.desc[0].bDescriptorType	= DYNAMIC */
	/*.desc[0].wDescriptorLenght	= DYNAMIC */
};

/* Super-Speed Support */
#if 0
static struct usb_endpoint_descriptor hidg_ss_in_ep_desc = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress	= USB_DIR_IN,
	.bmAttributes		= USB_ENDPOINT_XFER_INT,
	/*.wMaxPacketSize	= DYNAMIC */
	.bInterval		= 4, /* FIXME: Add this field in the
				      * HID gadget configuration?
				      * (struct hidg_func_descriptor)
				      */
};

static struct usb_ss_ep_comp_descriptor hidg_ss_in_comp_desc = {
	.bLength                = sizeof(hidg_ss_in_comp_desc),
	.bDescriptorType        = USB_DT_SS_ENDPOINT_COMP,

	/* .bMaxBurst           = 0, */
	/* .bmAttributes        = 0, */
	/* .wBytesPerInterval   = DYNAMIC */
};

static struct usb_endpoint_descriptor hidg_ss_out_ep_desc = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress	= USB_DIR_OUT,
	.bmAttributes		= USB_ENDPOINT_XFER_INT,
	/*.wMaxPacketSize	= DYNAMIC */
	.bInterval		= 4, /* FIXME: Add this field in the
				      * HID gadget configuration?
				      * (struct hidg_func_descriptor)
				      */
};

static struct usb_ss_ep_comp_descriptor hidg_ss_out_comp_desc = {
	.bLength                = sizeof(hidg_ss_out_comp_desc),
	.bDescriptorType        = USB_DT_SS_ENDPOINT_COMP,

	/* .bMaxBurst           = 0, */
	/* .bmAttributes        = 0, */
	/* .wBytesPerInterval   = DYNAMIC */
};

static struct usb_descriptor_header *hidg_ss_descriptors[] = {
	(struct usb_descriptor_header *)&hidg_interface_desc,
	(struct usb_descriptor_header *)&hidg_desc,
	(struct usb_descriptor_header *)&hidg_ss_in_ep_desc,
	(struct usb_descriptor_header *)&hidg_ss_in_comp_desc,
	(struct usb_descriptor_header *)&hidg_ss_out_ep_desc,
	(struct usb_descriptor_header *)&hidg_ss_out_comp_desc,
	NULL,
};
#endif

/* High-Speed Support */

static struct usb_endpoint_descriptor hidg_hs_in_ep_desc = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress	= USB_DIR_IN,
	.bmAttributes		= USB_ENDPOINT_XFER_INT,
	/*.wMaxPacketSize	= DYNAMIC */
	.bInterval		= 4, /* FIXME: Add this field in the
				      * HID gadget configuration?
				      * (struct hidg_func_descriptor)
				      */
};

#ifdef OUTEP_ENABLE
static struct usb_endpoint_descriptor hidg_hs_out_ep_desc = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress	= USB_DIR_OUT,
	.bmAttributes		= USB_ENDPOINT_XFER_INT,
	/*.wMaxPacketSize	= DYNAMIC */
	.bInterval		= 4, /* FIXME: Add this field in the
				      * HID gadget configuration?
				      * (struct hidg_func_descriptor)
				      */
};
#endif

static struct usb_descriptor_header *hidg_hs_descriptors[] = {
	(struct usb_descriptor_header *)&hidg_interface_desc,
	(struct usb_descriptor_header *)&hidg_desc,
	(struct usb_descriptor_header *)&hidg_hs_in_ep_desc,
#ifdef OUTEP_ENABLE
	(struct usb_descriptor_header *)&hidg_hs_out_ep_desc,
#endif
	NULL,
};

/* Full-Speed Support */

static struct usb_endpoint_descriptor hidg_fs_in_ep_desc = {
	.bLength		= USB_DT_ENDPOINT_SIZE, // endpoint descriptor size
	.bDescriptorType	= USB_DT_ENDPOINT,      // descriptor type
	.bEndpointAddress	= USB_DIR_IN,
	.bmAttributes		= USB_ENDPOINT_XFER_INT,
	/*.wMaxPacketSize	= DYNAMIC */
	.bInterval		= 10, /* FIXME: Add this field in the
				       * HID gadget configuration?
				       * (struct hidg_func_descriptor)
				       */
};

#ifdef OUTEP_ENABLE
static struct usb_endpoint_descriptor hidg_fs_out_ep_desc = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress	= USB_DIR_OUT,
	.bmAttributes		= USB_ENDPOINT_XFER_INT,
	/*.wMaxPacketSize	= DYNAMIC */
	.bInterval		= 10, /* FIXME: Add this field in the
				       * HID gadget configuration?
				       * (struct hidg_func_descriptor)
				       */
};
#endif

static struct usb_descriptor_header *hidg_fs_descriptors[] = {
	(struct usb_descriptor_header *)&hidg_interface_desc,
	(struct usb_descriptor_header *)&hidg_desc,
	(struct usb_descriptor_header *)&hidg_fs_in_ep_desc,
#ifdef OUTEP_ENABLE
	(struct usb_descriptor_header *)&hidg_fs_out_ep_desc,
#endif
	NULL,
};

/*-------------------------------------------------------------------------*/
/*                                 Strings                                 */

#define CT_FUNC_HID_IDX	0

static struct usb_string ct_func_string_defs[] = {
	[CT_FUNC_HID_IDX].s	= "HID Interface",
	{},			/* end of list */
};

static struct usb_gadget_strings ct_func_string_table = {
	.language	= 0x0409,	/* en-US */
	.strings	= ct_func_string_defs,
};

static struct usb_gadget_strings *ct_func_strings[] = {
	&ct_func_string_table,
	NULL,
};

/*-------------------------------------------------------------------------*/
/*                              Char Device                                */

#define __user

static ssize_t f_hidg_read(struct usb_function **f, const char __user *buffer,
			   size_t count, loff_t *ptr)
{
#ifdef OUTEP_ENABLE
	struct f_hidg *hidg;
	struct f_hidg_req_list *list;
	struct usb_request *req;
	unsigned long flags;
	int ret;

	if (*f == NULL)
		return 0;

	if (!count)
		return 0;

	hidg = func_to_hidg(*f);

//	if (!access_ok(VERIFY_WRITE, buffer, count))
//		return -EFAULT;

	spin_lock_irqsave(&hidg->read_spinlock, flags);

#define READ_COND (!list_empty(&hidg->completed_out_req))

	/* wait for at least one buffer to complete */
	while (!READ_COND) {
        if (*f == NULL)
            return 0;
		spin_unlock_irqrestore(&hidg->read_spinlock, flags);
//		if (file->f_flags & O_NONBLOCK)
//			return -EAGAIN;

		if (wait_event_interruptible_timeout(hidg->read_queue, !READ_COND, 500))
			return 0;//-ERESTARTSYS;

		spin_lock_irqsave(&hidg->read_spinlock, flags);
	}

	/* pick the first one */
	list = list_first_entry(&hidg->completed_out_req,
				struct f_hidg_req_list, list);

	/*
	 * Remove this from list to protect it from beign free()
	 * while host disables our function
	 */
	list_del(&list->list);

	req = list->req;
	count = min_t(unsigned int, count, req->actual - list->pos);
	spin_unlock_irqrestore(&hidg->read_spinlock, flags);

	/* copy to user outside spinlock */
	count -= copy_to_user((void *)buffer, req->buf + list->pos, count);
	list->pos += count;

	/*
	 * if this request is completely handled and transfered to
	 * userspace, remove its entry from the list and requeue it
	 * again. Otherwise, we will revisit it again upon the next
	 * call, taking into account its current read position.
	 */
	if (list->pos == req->actual) {
		kfree(list);

		req->length = hidg->report_length;
		ret = usb_ep_queue(hidg->out_ep, req, GFP_KERNEL);
		if (ret < 0) {
			free_ep_req(hidg->out_ep, req);
			return ret;
		}
	} else {
		spin_lock_irqsave(&hidg->read_spinlock, flags);
		list_add(&list->list, &hidg->completed_out_req);
		spin_unlock_irqrestore(&hidg->read_spinlock, flags);

		wake_up(&hidg->read_queue);
	}

	return count;
#else
    struct f_hidg   *hidg;
    char        *tmp_buff = NULL;
    unsigned long   flags;

	if (*f == NULL)
		return 0;

    if (!count)
        return 0;

    hidg = func_to_hidg(*f);

//    if (!access_ok(VERIFY_WRITE, buffer, count))
//        return -EFAULT;

    spin_lock_irqsave(&hidg->read_spinlock, flags);

#define READ_COND (hidg->set_report_buff != NULL)

    while (!READ_COND) {
        if (*f == NULL)
            return 0;

        spin_unlock_irqrestore(&hidg->read_spinlock, flags);
//        if (file->f_flags & O_NONBLOCK)
//            return -EAGAIN;

        if (wait_event_interruptible_timeout(hidg->read_queue, !READ_COND, 500))
            return 0;//-ERESTARTSYS;

        spin_lock_irqsave(&hidg->read_spinlock, flags);
    }

    count = min_t(unsigned, count, hidg->set_report_length);
    tmp_buff = hidg->set_report_buff;
    hidg->set_report_buff = NULL;

    spin_unlock_irqrestore(&hidg->read_spinlock, flags);

    if (tmp_buff != NULL) {
        /* copy to user outside spinlock */
        count -= copy_to_user((void *)buffer, tmp_buff, count);
        kfree(tmp_buff);
    } else
        count = -ENOMEM;

    return count;
#endif
}

static void f_hidg_req_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct f_hidg *hidg = (struct f_hidg *)ep->driver_data;
	unsigned long flags;

	if (req->status != 0) {
		ERROR(hidg->func.config->cdev,
		      "End Point Request ERROR: %d\n", req->status);
	}

	spin_lock_irqsave(&hidg->write_spinlock, flags);
	hidg->write_pending = 0;
	spin_unlock_irqrestore(&hidg->write_spinlock, flags);
	wake_up(&hidg->write_queue);
}

// 可以被 ap 呼叫，用來發送一個report到host
static ssize_t f_hidg_write(struct usb_function **f, const char __user *buffer,
		     size_t count, loff_t *offp)
{
	if (*f == NULL)
		return 0;
	struct f_hidg *hidg  = func_to_hidg(*f);
	struct usb_request *req;
	unsigned long flags;
	ssize_t status = -ENOMEM;

	//if (!access_ok(VERIFY_READ, buffer, count))
	//	return -EFAULT;

	spin_lock_irqsave(&hidg->write_spinlock, flags);

#define WRITE_COND (!hidg->write_pending)
try_again:
	/* write queue */
	while (!WRITE_COND) {
		spin_unlock_irqrestore(&hidg->write_spinlock, flags);
		//if (file->f_flags & O_NONBLOCK)
		//	return -EAGAIN;

		if (wait_event_interruptible_timeout(
			    hidg->write_queue, WRITE_COND, 500))
			return -ERESTARTSYS;

		spin_lock_irqsave(&hidg->write_spinlock, flags);
	}

	hidg->write_pending = 1;
	req = hidg->req;
	count  = min_t(unsigned, count, hidg->report_length);

	spin_unlock_irqrestore(&hidg->write_spinlock, flags);
	status = copy_from_user(req->buf, buffer, count);

	if (status != 0) {
		ERROR(hidg->func.config->cdev,
		      "copy_from_user error\n");
		status = -EINVAL;
		goto release_write_pending;
	}

	spin_lock_irqsave(&hidg->write_spinlock, flags);

	/* when our function has been disabled by host */
	if (!hidg->req) {
		free_ep_req(hidg->in_ep, req);
		/*
		 * TODO
		 * Should we fail with error here?
		 */
		goto try_again;
	}

	req->status   = 0;
	req->zero     = 0;
	req->length   = count;
	req->complete = f_hidg_req_complete;    // send report complete callback
	req->context  = hidg;

	status = usb_ep_queue(hidg->in_ep, req, GFP_ATOMIC);
	if (status < 0) {
		ERROR(hidg->func.config->cdev,
		      "usb_ep_queue error on int endpoint %zd\n", status);
		goto release_write_pending_unlocked;
	} else {
		status = count;
	}
	spin_unlock_irqrestore(&hidg->write_spinlock, flags);

	return status;
release_write_pending:
	spin_lock_irqsave(&hidg->write_spinlock, flags);
release_write_pending_unlocked:
	hidg->write_pending = 0;
	spin_unlock_irqrestore(&hidg->write_spinlock, flags);

	wake_up(&hidg->write_queue);

	return status;
}

#if 0
static unsigned int f_hidg_poll(struct file *file, poll_table *wait)
{
	struct f_hidg	*hidg  = file->private_data;
	unsigned int	ret = 0;

	poll_wait(file, &hidg->read_queue, wait);
	poll_wait(file, &hidg->write_queue, wait);

	if (WRITE_COND)
		ret |= POLLOUT | POLLWRNORM;

	if (READ_COND)
		ret |= POLLIN | POLLRDNORM;

	return ret;
}

#undef WRITE_COND
#undef READ_COND

static int f_hidg_release(struct inode *inode, struct file *fd)
{
	fd->private_data = NULL;
	return 0;
}

static int f_hidg_open(struct inode *inode, struct file *fd)
{
	struct f_hidg *hidg =
		container_of(inode->i_cdev, struct f_hidg, cdev);

	fd->private_data = hidg;

	return 0;
}
#endif

/*-------------------------------------------------------------------------*/
/*                                usb_function                             */
static LIST_HEAD(hidg_func_list);
static LIST_HEAD(hidg_default_list);
static DEFINE_MUTEX(hidg_func_lock);

#include "ite_hidg_device.c"
#include "ite_hidg_mouse.c"
#include "ite_hidg_keyboard.c"


static struct hidg_func_config *hidg_try_add_default_config(struct hidg_func_config *defc)
{
    struct hidg_func_config *newfc;
    
    newfc = kzalloc(sizeof(struct hidg_func_config), GFP_KERNEL);
    if (!newfc)
        return NULL;

    newfc->name = kmemdup(defc->name,
					    strlen(defc->name)+1,
					    GFP_KERNEL);
    newfc->idx= defc->idx;
    newfc->typeID = defc->typeID;
	newfc->subclass = defc->subclass;
	newfc->protocol = defc->protocol;
	newfc->report_length = defc->report_length;
	newfc->report_desc_length = defc->report_desc_length;
	newfc->report_desc = kmemdup(defc->default_report_desc,
					    newfc->report_desc_length,
					    GFP_KERNEL);
	if (!newfc->report_desc || !newfc->name) {
        kfree(newfc->name);
        kfree(newfc->report_desc);
        kfree(newfc);
        return NULL;
	}

	list_add(&newfc->list, &hidg_func_list);
    return newfc;
}

static struct hidg_func_config *hidg_add_default_config(const char *name)
{
    struct hidg_func_config *fc;
    list_for_each_entry(fc, &hidg_default_list, list) {
		if (!strcmp(name, fc->name)) {
            return hidg_try_add_default_config(fc);
        }
    }
    return NULL;
}

static struct hidg_func_config *hidg_get_config(const char *name)
{
    struct hidg_func_config *fc;    
    list_for_each_entry(fc, &hidg_func_list, list) {
        if (!strcmp(fc->name, name))
            return fc;
    }
    return hidg_add_default_config(name);
}

static int hidg_set_config_by_typeID(struct hidg_func_config *pUsbData)
{
    const char *name[] = {0, "hid-dev", "hid-mouse", "hid-kbd"}; //order by UsbdHidgTypeID
    const unsigned char idx[] = {USBD_IDX_NONE, USBD_IDX_NONE, USBD_IDX_0, USBD_IDX_1}; //for usb 1-1 transparent
    if(pUsbData->typeID == USBD_HIDG_NONE || pUsbData->typeID >= USBD_HIDG_MAX)
        return 0;
    //update case
    struct hidg_func_config *fc;    
    list_for_each_entry(fc, &hidg_func_list, list) {
        if (!strcmp(fc->name, name[pUsbData->typeID]))
        {
            if(fc->idVendor == pUsbData->idVendor && fc->idProduct == pUsbData->idProduct)
                return 0;
            unsigned char *ptReport_desc = kmemdup(pUsbData->report_desc,
        					    pUsbData->report_desc_length,
        					    GFP_KERNEL);
            if (!ptReport_desc)
                return 0;
            
            fc->idVendor = pUsbData->idVendor;
            fc->idProduct = pUsbData->idProduct;
          	fc->subclass = pUsbData->subclass;
        	fc->protocol = pUsbData->protocol;
            fc->report_length = pUsbData->report_length;
        	fc->report_desc_length = pUsbData->report_desc_length;
            kfree(fc->report_desc);
            fc->report_desc = ptReport_desc;
            return 1;
        }
    }
    //add new case
    struct hidg_func_config *newfc = kzalloc(sizeof(struct hidg_func_config), GFP_KERNEL);
    if (!newfc)
        return 0;    
    newfc->name = kmemdup(name[pUsbData->typeID],
				    strlen(name[pUsbData->typeID])+1,
				    GFP_KERNEL);
    newfc->idx = idx[pUsbData->typeID];
    newfc->typeID = pUsbData->typeID;
    newfc->idVendor = pUsbData->idVendor;
    newfc->idProduct = pUsbData->idProduct;
	newfc->subclass = pUsbData->subclass;
	newfc->protocol = pUsbData->protocol;
	newfc->report_length = pUsbData->report_length;
	newfc->report_desc_length = pUsbData->report_desc_length;
	newfc->report_desc = kmemdup(pUsbData->report_desc,
					    pUsbData->report_desc_length,
					    GFP_KERNEL);
	if (!newfc->report_desc || !newfc->name) {
        kfree(newfc->name);
        kfree(newfc->report_desc);
        kfree(newfc);
        return 0;
	}
	list_add(&newfc->list, &hidg_func_list);

    return 1;
}
//for usb 1-1 transparent only support "hid-mouse", "hid-kbd"
static int hidg_set_config_by_idx(struct hidg_func_config *pUsbData)
{
    const char *name[] = {0, "hid-mouse", "hid-kbd"}; //order by UsbdHidgIdx
    const unsigned char typeID[] = {USBD_HIDG_NONE, USBD_HIDG_MOUSE, USBD_HIDG_KBD}; //for usb 1-1 transparent
    if(pUsbData->idx == USBD_IDX_NONE || pUsbData->idx >= USBD_IDX_MAX)
        return 0;
    //update case
    struct hidg_func_config *fc;    
    list_for_each_entry(fc, &hidg_func_list, list) {
        if (fc->idx == pUsbData->idx)
        {
            if(fc->idVendor == pUsbData->idVendor && fc->idProduct == pUsbData->idProduct)
                return 0;
            unsigned char *ptReport_desc = kmemdup(pUsbData->report_desc,
        					    pUsbData->report_desc_length,
        					    GFP_KERNEL);
            if (!ptReport_desc)
                return 0;
            
            fc->idVendor = pUsbData->idVendor;
            fc->idProduct = pUsbData->idProduct;
          	fc->subclass = pUsbData->subclass;
        	fc->protocol = pUsbData->protocol;
            fc->report_length = pUsbData->report_length;
        	fc->report_desc_length = pUsbData->report_desc_length;
            kfree(fc->report_desc);
            fc->report_desc = ptReport_desc;
            return 1;
        }
    }
    //add new case
    struct hidg_func_config *newfc = kzalloc(sizeof(struct hidg_func_config), GFP_KERNEL);
    if (!newfc)
        return 0;    
    newfc->name = kmemdup(name[pUsbData->idx],
				    strlen(name[pUsbData->idx])+1,
				    GFP_KERNEL);
    newfc->idx = pUsbData->idx;
    newfc->typeID = typeID[pUsbData->idx];
    newfc->idVendor = pUsbData->idVendor;
    newfc->idProduct = pUsbData->idProduct;
	newfc->subclass = pUsbData->subclass;
	newfc->protocol = pUsbData->protocol;
	newfc->report_length = pUsbData->report_length;
	newfc->report_desc_length = pUsbData->report_desc_length;
	newfc->report_desc = kmemdup(pUsbData->report_desc,
					    pUsbData->report_desc_length,
					    GFP_KERNEL);
	if (!newfc->report_desc || !newfc->name) {
        kfree(newfc->name);
        kfree(newfc->report_desc);
        kfree(newfc);
        return 0;
	}
	list_add(&newfc->list, &hidg_func_list);

    return 1;
}

int hidg_set_config(struct hidg_func_config *pUsbData)
{
    if(hidg_set_config_by_idx(pUsbData))
        return 1;
    else if (hidg_set_config_by_typeID(pUsbData))
        return 1;

    return 0;
}

// this is for only one hidg device
ssize_t hidg_read(const char *buffer,
		     size_t count)
{
    /* pick the first one */
    struct hidg_func_config *fc = list_first_entry_or_null(&hidg_func_list,
				struct hidg_func_config, list);
    if(fc) return f_hidg_read(&fc->hidg_func, buffer, count, 0);
    return 0;
}
// this is for only one hidg device
ssize_t hidg_write(const char *buffer,
		     size_t count)
{
    /* pick the first one */
    struct hidg_func_config *fc = list_first_entry_or_null(&hidg_func_list,
				struct hidg_func_config, list);
    if(fc) return f_hidg_write(&fc->hidg_func, buffer, count, 0);
    return 0;
}
// this is for multi hidg device
ssize_t hidg_read_by_ID(HIDG_FUNC_CMD_DATA *pUsbData)
{
    struct hidg_func_config *fc;
    list_for_each_entry(fc, &hidg_func_list, list) {
        if ((fc->idx== pUsbData->idx) && (fc->idVendor == pUsbData->idVendor && fc->idProduct == pUsbData->idProduct))
            return f_hidg_read(&fc->hidg_func, pUsbData->buffer, pUsbData->count, 0);
        else if ((fc->typeID == pUsbData->typeID) || (fc->idVendor == pUsbData->idVendor && fc->idProduct == pUsbData->idProduct))
            return f_hidg_read(&fc->hidg_func, pUsbData->buffer, pUsbData->count, 0);
    }
    return 0;
}
// this is for multi hidg device
ssize_t hidg_write_by_ID(HIDG_FUNC_CMD_DATA *pUsbData)
{
    struct hidg_func_config *fc;
    list_for_each_entry(fc, &hidg_func_list, list) {
        if ((fc->idx== pUsbData->idx) && (fc->idVendor == pUsbData->idVendor && fc->idProduct == pUsbData->idProduct))
            return f_hidg_write(&fc->hidg_func, pUsbData->buffer, pUsbData->count, 0);
        else if ((fc->typeID == pUsbData->typeID) || (fc->idVendor == pUsbData->idVendor && fc->idProduct == pUsbData->idProduct))
            return f_hidg_write(&fc->hidg_func, pUsbData->buffer, pUsbData->count, 0);
    }
    return 0;
}

//////
static inline struct usb_request *hidg_alloc_ep_req(struct usb_ep *ep,
		unsigned length) 
{
	return alloc_ep_req(ep, length);
}

static void hidg_set_report_complete(struct usb_ep *ep, struct usb_request *req)
{
#ifdef OUTEP_ENABLE
	struct f_hidg *hidg = (struct f_hidg *) req->context;
	struct usb_composite_dev *cdev = hidg->func.config->cdev;
	struct f_hidg_req_list *req_list;
	unsigned long flags;

	switch (req->status) {
	case 0:
		req_list = kzalloc(sizeof(*req_list), GFP_ATOMIC);
		if (!req_list) {
			ERROR(cdev, "Unable to allocate mem for req_list\n");
			goto free_req;
		}

		req_list->req = req;

		spin_lock_irqsave(&hidg->read_spinlock, flags);
		list_add_tail(&req_list->list, &hidg->completed_out_req);
		spin_unlock_irqrestore(&hidg->read_spinlock, flags);

		wake_up(&hidg->read_queue);
		break;
	default:
		ERROR(cdev, "Set report failed %d\n", req->status);
		/* FALLTHROUGH */
	case -ECONNABORTED:		/* hardware forced ep reset */
	case -ECONNRESET:		/* request dequeued */
	case -ESHUTDOWN:		/* disconnect from host */
free_req:
		free_ep_req(ep, req);
		return;
	}
#else
    struct f_hidg *hidg = (struct f_hidg *)req->context;
    unsigned long flags;

    if (req->status != 0 || req->buf == NULL || req->actual == 0) {
        ERROR(hidg->func.config->cdev, "%s FAILED\n", __func__);
        return;
    }

    spin_lock_irqsave(&hidg->read_spinlock, flags);

    hidg->set_report_buff = realloc(hidg->set_report_buff,
                     ITH_ALIGN_UP(req->actual, 32));

    if (hidg->set_report_buff == NULL) {
        spin_unlock_irqrestore(&hidg->read_spinlock, flags);
        return;
    }
    hidg->set_report_length = req->actual;
    memcpy(hidg->set_report_buff, req->buf, req->actual);

    spin_unlock_irqrestore(&hidg->read_spinlock, flags);

    wake_up(&hidg->read_queue);
#endif
}

static int hidg_setup(struct usb_function *f,
		      const struct usb_ctrlrequest *ctrl)
{
	//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);
	struct f_hidg			*hidg = func_to_hidg(f);
	struct usb_composite_dev	*cdev = f->config->cdev;
	struct usb_request		*req  = cdev->req;
	int status = 0;
	__u16 value, length;

	value	= __le16_to_cpu(ctrl->wValue);
	length	= __le16_to_cpu(ctrl->wLength);

	//ithPrintf("%s(%d) crtl_request : bRequestType:0x%x bRequest:0x%x Value:0x%x\n",
	//	  __func__, __LINE__, ctrl->bRequestType, ctrl->bRequest, value);

	VDBG(cdev,
	     "%s crtl_request : bRequestType:0x%x bRequest:0x%x Value:0x%x\n",
	     __func__, ctrl->bRequestType, ctrl->bRequest, value);

	switch ((ctrl->bRequestType << 8) | ctrl->bRequest) {
	case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8
			      | HID_REQ_GET_REPORT):
		//ithPrintf("%s(%d) - get_report\n", __FUNCTION__, __LINE__);
		VDBG(cdev, "get_report\n");

		/* send an empty report */
		length = min_t(unsigned, length, hidg->report_length);
		memset(req->buf, 0x0, length);

		goto respond;
		break;

	case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8
			      | HID_REQ_GET_PROTOCOL):
		//ithPrintf("%s(%d) - get_protocol\n", __FUNCTION__, __LINE__);
		VDBG(cdev, "get_protocol\n");
		goto stall;
		break;

	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8
			      | HID_REQ_SET_REPORT):
		//ithPrintf("%s(%d) - set_report | wLength=%d\n", __FUNCTION__, __LINE__, ctrl->wLength);
		VDBG(cdev, "set_report | wLength=%d\n", ctrl->wLength);
#ifndef OUTEP_ENABLE
		req->context  = hidg;
		req->complete = hidg_set_report_complete;
#endif
		goto respond;
		break;

	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8
			      | HID_REQ_SET_PROTOCOL):
		//ithPrintf("%s(%d) - set_protocol\n", __FUNCTION__, __LINE__);
		VDBG(cdev, "set_protocol\n");
		goto stall;
		break;

	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8
			      | HID_REQ_SET_IDLE):
		//ithPrintf("%s(%d) - set_idle\n", __FUNCTION__, __LINE__);
		VDBG(cdev, "set_protocol\n");
		goto respond;
		break;

	case ((USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE) << 8
			      | USB_REQ_GET_DESCRIPTOR):
		switch (value >> 8) {
		case HID_DT_HID: {
			struct hid_descriptor hidg_desc_copy = hidg_desc;

			//ithPrintf("%s(%d) - USB_REQ_GET_DESCRIPTOR: HID\n", __FUNCTION__, __LINE__);
			VDBG(cdev, "USB_REQ_GET_DESCRIPTOR: HID\n");
			hidg_desc_copy.desc[0].bDescriptorType = HID_DT_REPORT;
			hidg_desc_copy.desc[0].wDescriptorLength =
				cpu_to_le16(hidg->report_desc_length);

			length = min_t(unsigned short, length,
				       hidg_desc_copy.bLength);
			memcpy(req->buf, &hidg_desc_copy, length);
			goto respond;
			break;
		}
		case HID_DT_REPORT:
			//ithPrintf("%s(%d) - USB_REQ_GET_DESCRIPTOR: REPORT\n", __FUNCTION__, __LINE__);
			VDBG(cdev, "USB_REQ_GET_DESCRIPTOR: REPORT\n");
			length = min_t(unsigned short, length,
				       hidg->report_desc_length);
			memcpy(req->buf, hidg->report_desc, length);
			goto respond;
			break;

		default:
			//ithPrintf("%s(%d) - Unknown descriptor request\n", __FUNCTION__, __LINE__);
			VDBG(cdev, "Unknown descriptor request 0x%x\n",
			     value >> 8);
			goto stall;
			break;
		}
		break;

	default:
		//ithPrintf("%s(%d) - Unknown request 0x%x\n", __FUNCTION__, __LINE__, ctrl->bRequest);
		VDBG(cdev, "Unknown request 0x%x\n",
		     ctrl->bRequest);
		goto stall;
		break;
	}

stall:
	return -EOPNOTSUPP;

respond:
	req->zero = 0;
	req->length = length;
	status = usb_ep_queue(cdev->gadget->ep0, req, GFP_ATOMIC);
	if (status < 0)
		ERROR(cdev, "usb_ep_queue error on ep0 %d\n", value);
	return status;
}

static void hidg_disable(struct usb_function *f)
{
	struct f_hidg *hidg = func_to_hidg(f);
	struct f_hidg_req_list *list, *next;
	unsigned long flags;

	usb_ep_disable(hidg->in_ep);
#ifdef OUTEP_ENABLE
	usb_ep_disable(hidg->out_ep);

	spin_lock_irqsave(&hidg->read_spinlock, flags);
	list_for_each_entry_safe(list, next, &hidg->completed_out_req, list) {
		free_ep_req(hidg->out_ep, list->req);
		list_del(&list->list);
		kfree(list);
	}
	spin_unlock_irqrestore(&hidg->read_spinlock, flags);
#endif
	spin_lock_irqsave(&hidg->write_spinlock, flags);
	if (!hidg->write_pending) {
		free_ep_req(hidg->in_ep, hidg->req);
		hidg->write_pending = 1;
	}

	hidg->req = NULL;
	spin_unlock_irqrestore(&hidg->write_spinlock, flags);
}

static int hidg_set_alt(struct usb_function *f, unsigned intf, unsigned alt)
{
	struct usb_composite_dev		*cdev = f->config->cdev;
	struct f_hidg				*hidg = func_to_hidg(f);
	struct usb_request			*req_in = NULL;
	unsigned long				flags;
	int i, status = 0;

	//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);

	VDBG(cdev, "hidg_set_alt intf:%d alt:%d\n", intf, alt);

	if (hidg->in_ep != NULL) {
		/* restart endpoint */
		usb_ep_disable(hidg->in_ep);

		status = config_ep_by_speed(f->config->cdev->gadget, f,
					    hidg->in_ep);
		if (status) {
			ERROR(cdev, "config_ep_by_speed FAILED!\n");
			goto fail;
		}
		status = usb_ep_enable(hidg->in_ep);
		if (status < 0) {
			ERROR(cdev, "Enable IN endpoint FAILED!\n");
			goto fail;
		}
		hidg->in_ep->driver_data = hidg;

		//ithPrintf("hidg->in_ep(%p)hidg->report_length(%d)\n",  
		//	hidg->in_ep, hidg->report_length);
#if 1
		req_in = hidg_alloc_ep_req(hidg->in_ep, hidg->report_length);
		//ithPrintf("hidg->in_ep(%p)hidg->report_length(%d) req_in(%p)\n", 
		//	hidg->in_ep, hidg->report_length, req_in);
		if (!req_in) {
			status = -ENOMEM;
			goto disable_ep_in;
		}
#endif
	}


#ifdef OUTEP_ENABLE
	if (hidg->out_ep != NULL) {
		/* restart endpoint */
		usb_ep_disable(hidg->out_ep);

		status = config_ep_by_speed(f->config->cdev->gadget, f,
					    hidg->out_ep);
		if (status) {
			ERROR(cdev, "config_ep_by_speed FAILED!\n");
			goto free_req_in;
		}
		status = usb_ep_enable(hidg->out_ep);
		if (status < 0) {
			ERROR(cdev, "Enable OUT endpoint FAILED!\n");
			goto free_req_in;
		}
		hidg->out_ep->driver_data = hidg;

		/*
		 * allocate a bunch of read buffers and queue them all at once.
		 */
		for (i = 0; i < hidg->qlen && status == 0; i++) {
			// alloc usb_request
			struct usb_request *req =
				hidg_alloc_ep_req(hidg->out_ep,
						  hidg->report_length);
			if (req) {
				req->complete = hidg_set_report_complete;
				req->context  = hidg;
				status = usb_ep_queue(hidg->out_ep, req,
						      GFP_ATOMIC);
				if (status) {
					ERROR(cdev, "%s queue req --> %d\n",
					      hidg->out_ep->name, status);
					free_ep_req(hidg->out_ep, req);
				}
			} else {
				status = -ENOMEM;
				goto disable_out_ep;
			}
		}
	}
#endif

#if 1
	if (hidg->in_ep != NULL) {
		spin_lock_irqsave(&hidg->write_spinlock, flags);
		hidg->req = req_in;
		hidg->write_pending = 0;
		spin_unlock_irqrestore(&hidg->write_spinlock, flags);

		wake_up(&hidg->write_queue);

	}
#endif
	return 0;
#ifdef OUTEP_ENABLE
disable_out_ep:
	usb_ep_disable(hidg->out_ep);
#endif
free_req_in:
	if (req_in)
		free_ep_req(hidg->in_ep, req_in);

disable_ep_in:
	if (hidg->in_ep)
		usb_ep_disable(hidg->in_ep);

fail:
	return status;
}

#if 0
static const struct file_operations f_hidg_fops = {
	.owner		= THIS_MODULE,
	.open		= f_hidg_open,
	.release	= f_hidg_release,
	.write		= f_hidg_write,
	.read		= f_hidg_read,
	.poll		= f_hidg_poll,
	.llseek		= noop_llseek,
};
#endif

// c -> ite_config_driver
// usb function driver 的 bind 功能是對現有config加上描述
static int hidg_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_ep		*ep;
	struct f_hidg		*hidg = func_to_hidg(f);
	struct usb_string	*us;
	struct device		*device;
	int			status;
	dev_t			dev;

	//ithPrintf("%s(%d) c(%p)c->cdev(%p)\n", __FUNCTION__, __LINE__, c, c->cdev);

	/* maybe allocate device-global string IDs, and patch descriptors */
	us = usb_gstrings_attach(c->cdev, ct_func_strings,
				 ARRAY_SIZE(ct_func_string_defs));
	if (IS_ERR(us))
		return PTR_ERR(us);
	hidg_interface_desc.iInterface = us[CT_FUNC_HID_IDX].id;

	/* allocate instance-specific interface IDs, and patch descriptors */
	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;
	hidg_interface_desc.bInterfaceNumber = status;

	/* allocate instance-specific endpoints */
	status = -ENODEV;
	ep = usb_ep_autoconfig(c->cdev->gadget, &hidg_fs_in_ep_desc);
	if (!ep)
		goto fail;
	hidg->in_ep = ep;

#ifdef OUTEP_ENABLE
	ep = usb_ep_autoconfig(c->cdev->gadget, &hidg_fs_out_ep_desc);
	if (!ep)
		goto fail;
	hidg->out_ep = ep;
#endif

	/* set descriptor dynamic values */
	hidg_interface_desc.bInterfaceSubClass = hidg->bInterfaceSubClass;
	hidg_interface_desc.bInterfaceProtocol = hidg->bInterfaceProtocol;
	//hidg_ss_in_ep_desc.wMaxPacketSize = cpu_to_le16(hidg->report_length);
	//hidg_ss_in_comp_desc.wBytesPerInterval =
	//			cpu_to_le16(hidg->report_length);
	hidg_hs_in_ep_desc.wMaxPacketSize = cpu_to_le16(hidg->report_length);
	hidg_fs_in_ep_desc.wMaxPacketSize = cpu_to_le16(hidg->report_length);
	//hidg_ss_out_ep_desc.wMaxPacketSize = cpu_to_le16(hidg->report_length);
	//hidg_ss_out_comp_desc.wBytesPerInterval =
	//			cpu_to_le16(hidg->report_length);
#ifdef OUTEP_ENABLE
	hidg_hs_out_ep_desc.wMaxPacketSize = cpu_to_le16(hidg->report_length);
	hidg_fs_out_ep_desc.wMaxPacketSize = cpu_to_le16(hidg->report_length);
#endif
	/*
	 * We can use hidg_desc struct here but we should not relay
	 * that its content won't change after returning from this function.
	 */
	hidg_desc.desc[0].bDescriptorType = HID_DT_REPORT;
	hidg_desc.desc[0].wDescriptorLength =
		cpu_to_le16(hidg->report_desc_length);

	hidg_hs_in_ep_desc.bEndpointAddress =
		hidg_fs_in_ep_desc.bEndpointAddress;
#ifdef OUTEP_ENABLE
	hidg_hs_out_ep_desc.bEndpointAddress =
		hidg_fs_out_ep_desc.bEndpointAddress;
#endif
	//hidg_ss_in_ep_desc.bEndpointAddress =
	//	hidg_fs_in_ep_desc.bEndpointAddress;
	//hidg_ss_out_ep_desc.bEndpointAddress =
	//	hidg_fs_out_ep_desc.bEndpointAddress;

	status = usb_assign_descriptors(f, hidg_fs_descriptors,
					hidg_hs_descriptors, NULL/*hidg_ss_descriptors*/, NULL);
	if (status)
		goto fail;

	spin_lock_init(&hidg->write_spinlock);
	hidg->write_pending = 1;
	hidg->req = NULL;
	spin_lock_init(&hidg->read_spinlock);
	init_waitqueue_head(&hidg->write_queue);
	init_waitqueue_head(&hidg->read_queue);
#ifdef OUTEP_ENABLE
	INIT_LIST_HEAD(&hidg->completed_out_req);
#else
    hidg->set_report_buff = NULL;
#endif
#if 0
	/* create char device */
	cdev_init(&hidg->cdev, &f_hidg_fops);
	dev = MKDEV(major, hidg->minor);
	status = cdev_add(&hidg->cdev, dev, 1);
	if (status)
		goto fail_free_descs;

	device = device_create(hidg_class, NULL, dev, NULL,
			       "%s%d", "hidg", hidg->minor);
	if (IS_ERR(device)) {
		status = PTR_ERR(device);
		goto del;
	}
#endif

	return 0;
del:
	//cdev_del(&hidg->cdev);
fail_free_descs:
	usb_free_all_descriptors(f);
fail:
	ERROR(f->config->cdev, "hidg_bind FAILED\n");
	if (hidg->req != NULL)
		free_ep_req(hidg->in_ep, hidg->req);

	return status;
}

#if 0
static inline int hidg_get_minor(void)
{
	int ret;

	ret = ida_simple_get(&hidg_ida, 0, 0, GFP_KERNEL);
	if (ret >= HIDG_MINORS) {
		ida_simple_remove(&hidg_ida, ret);
		ret = -ENODEV;
	}

	return ret;
}

static inline struct f_hid_opts *to_f_hid_opts(struct config_item *item) {
	return container_of(to_config_group(item), struct f_hid_opts,
			    func_inst.group);
}

static void hid_attr_release(struct config_item *item)
{
	struct f_hid_opts *opts = to_f_hid_opts(item);

	usb_put_function_instance(&opts->func_inst);
}

static struct configfs_item_operations hidg_item_ops = {
	.release	= hid_attr_release,
};

#define F_HID_OPT(name, prec, limit)					\
static ssize_t f_hid_opts_##name##_show(struct config_item *item, char *page)\
{									\
	struct f_hid_opts *opts = to_f_hid_opts(item);			\
	int result;							\
									\
	mutex_lock(&opts->lock);					\
	result = sprintf(page, "%d\n", opts->name);			\
	mutex_unlock(&opts->lock);					\
									\
	return result;							\
}									\
									\
static ssize_t f_hid_opts_##name##_store(struct config_item *item,	\
					 const char *page, size_t len)	\
{									\
	struct f_hid_opts *opts = to_f_hid_opts(item);			\
	int ret;							\
	u##prec num;							\
									\
	mutex_lock(&opts->lock);					\
	if (opts->refcnt) {						\
		ret = -EBUSY;						\
		goto end;						\
	}								\
									\
	ret = kstrtou##prec(page, 0, &num);				\
	if (ret)							\
		goto end;						\
									\
	if (num > limit) {						\
		ret = -EINVAL;						\
		goto end;						\
	}								\
	opts->name = num;						\
	ret = len;							\
									\
end:									\
	mutex_unlock(&opts->lock);					\
	return ret;							\
}									\
									\
CONFIGFS_ATTR(f_hid_opts_, name)

F_HID_OPT(subclass, 8, 255);
F_HID_OPT(protocol, 8, 255);
F_HID_OPT(report_length, 16, 65535);

static ssize_t f_hid_opts_report_desc_show(struct config_item *item, char *page)
{
	struct f_hid_opts *opts = to_f_hid_opts(item);
	int result;

	mutex_lock(&opts->lock);
	result = opts->report_desc_length;
	memcpy(page, opts->report_desc, opts->report_desc_length);
	mutex_unlock(&opts->lock);

	return result;
}

static ssize_t f_hid_opts_report_desc_store(struct config_item *item,
		const char *page, size_t len)
{
	struct f_hid_opts *opts = to_f_hid_opts(item);
	int ret = -EBUSY;
	char *d;

	mutex_lock(&opts->lock);

	if (opts->refcnt)
		goto end;
	if (len > PAGE_SIZE) {
		ret = -ENOSPC;
		goto end;
	}
	d = kmemdup(page, len, GFP_KERNEL);
	if (!d) {
		ret = -ENOMEM;
		goto end;
	}
	kfree(opts->report_desc);
	opts->report_desc = d;
	opts->report_desc_length = len;
	opts->report_desc_alloc = true;
	ret = len;
end:
	mutex_unlock(&opts->lock);
	return ret;
}

CONFIGFS_ATTR(f_hid_opts_, report_desc);

static ssize_t f_hid_opts_dev_show(struct config_item *item, char *page)
{
	struct f_hid_opts *opts = to_f_hid_opts(item);

	return sprintf(page, "%d:%d\n", major, opts->minor);
}

CONFIGFS_ATTR_RO(f_hid_opts_, dev);

static struct configfs_attribute *hid_attrs[] = {
	&f_hid_opts_attr_subclass,
	&f_hid_opts_attr_protocol,
	&f_hid_opts_attr_report_length,
	&f_hid_opts_attr_report_desc,
	&f_hid_opts_attr_dev,
	NULL,
};

static struct config_item_type hid_func_type = {
	.ct_item_ops	= &hidg_item_ops,
	.ct_attrs	= hid_attrs,
	.ct_owner	= THIS_MODULE,
};

static inline void hidg_put_minor(int minor)
{
	ida_simple_remove(&hidg_ida, minor);
}
#endif

/**
 * @brief Used to free the HID usb function driver instance
 *
 * @param f	the usb function driver instance to be freed.
 */
static void hidg_free_inst(struct usb_function_instance *f)
{
	struct f_hid_opts *opts;

	opts = container_of(f, struct f_hid_opts, func_inst);

	//mutex_lock(&hidg_ida_lock);

//	hidg_put_minor(opts->minor);
//	if (ida_is_empty(&hidg_ida))
//		ghid_cleanup();

	//mutex_unlock(&hidg_ida_lock);

	if (opts->report_desc_alloc)
		kfree(opts->report_desc);

	kfree(opts);
}


//struct f_hid_cfg {
//	unsigned char			subclass;
//	unsigned char			protocol;
//	unsigned short			report_length;
//	unsigned short			report_desc_length;
//	unsigned char			*report_desc;
//	bool				report_desc_alloc;
//};

/**
 * @brief alloc usb function driver instance
 *
 * @return struct usb_function_instance*
 */
static struct usb_function_instance *hidg_alloc_inst(void) {
	struct f_hid_opts *opts;
	struct usb_function_instance *ret;
	int status = 0;

	//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);

	opts = kzalloc(sizeof(*opts), GFP_KERNEL);
	if (!opts)
		return ERR_PTR(-ENOMEM);
	mutex_init(&opts->lock);
	opts->func_inst.free_func_inst = hidg_free_inst;
	ret = &opts->func_inst;

	
	//ithPrintf("%s(%d) opts(%p)\n", __FUNCTION__, __LINE__, opts);
	//mutex_lock(&hidg_ida_lock);

#if 0
	if (ida_is_empty(&hidg_ida)) {
		status = ghid_setup(NULL, HIDG_MINORS);
		if (status)  {
			ret = ERR_PTR(status);
			kfree(opts);
			goto unlock;
		}
	}

	opts->minor = hidg_get_minor();
	if (opts->minor < 0) {
		ret = ERR_PTR(opts->minor);
		kfree(opts);
		if (ida_is_empty(&hidg_ida))
			ghid_cleanup();
		goto unlock;
	}
#endif
	//config_group_init_type_name(&opts->func_inst.group, "", &hid_func_type);
unlock:
	//mutex_unlock(&hidg_ida_lock);
	return ret;
}

static void hidg_free(struct usb_function *f)
{
	struct f_hidg *hidg;
	struct f_hid_opts *opts;

	hidg = func_to_hidg(f);
	opts = container_of(f->fi, struct f_hid_opts, func_inst);
    hidg->f_conf->hidg_func = NULL;
#ifndef OUTEP_ENABLE
	kfree(hidg->set_report_buff);
#endif
	kfree(hidg->report_desc);
	kfree(hidg);
	mutex_lock(&opts->lock);
	--opts->refcnt;
	mutex_unlock(&opts->lock);
}

static void hidg_unbind(struct usb_configuration *c, struct usb_function *f)
{
	struct f_hidg *hidg = func_to_hidg(f);

	//device_destroy(hidg_class, MKDEV(major, hidg->minor));
	//cdev_del(&hidg->cdev);

	usb_free_all_descriptors(f);
}

static struct usb_function *hidg_alloc(struct usb_function_instance *fi) 
{
	struct f_hidg *hidg;
	struct f_hid_opts *opts;

	//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);

	/* allocate and initialize one new instance */
	hidg = kzalloc(sizeof(*hidg), GFP_KERNEL);
	if (!hidg)
		return ERR_PTR(-ENOMEM);

	opts = container_of(fi, struct f_hid_opts, func_inst);

	
	//ithPrintf("%s(%d) opts(%p)\n", __FUNCTION__, __LINE__, opts);

	mutex_lock(&opts->lock);
	++opts->refcnt;

	//hidg->minor = opts->minor;
#if 0
	hidg->bInterfaceSubClass = opts->subclass;
	hidg->bInterfaceProtocol = opts->protocol;
	hidg->report_length = opts->report_length;
	hidg->report_desc_length = opts->report_desc_length;
	if (opts->report_desc) {
		hidg->report_desc = kmemdup(opts->report_desc,
					    opts->report_desc_length,
					    GFP_KERNEL);
		if (!hidg->report_desc) {
			kfree(hidg);
			mutex_unlock(&opts->lock);
			return ERR_PTR(-ENOMEM);
		}
	}
#else
    struct hidg_func_config *fc = hidg_get_config(fi->fd->name);
    if (!fc) return ERR_PTR(-ENOMEM);

    hidg->bInterfaceSubClass = fc->subclass;
    hidg->bInterfaceProtocol = fc->protocol;
    hidg->report_length = fc->report_length;
    hidg->report_desc_length = fc->report_desc_length;
    hidg->report_desc = kmemdup(fc->report_desc,
		    fc->report_desc_length,
		    GFP_KERNEL);
    if (!hidg->report_desc) {
    	kfree(hidg);
    	mutex_unlock(&opts->lock);
    	return ERR_PTR(-ENOMEM);
    }

    fc->hidg_func = &hidg->func;
    hidg->f_conf = fc;
#endif

#if 0
	ithPrintf("hidg->bInterfaceSubClass: %u\n"
		  "hidg->bInterfaceProtocol: %u\n",
		  hidg->bInterfaceSubClass,
		  hidg->bInterfaceProtocol);
#endif
	// opts must be initialized at hidg_alloc_inst()

	mutex_unlock(&opts->lock);

	// initialize the usb_function struct
	hidg->func.name    = "hid";
	hidg->func.bind    = hidg_bind;
	hidg->func.unbind  = hidg_unbind;
	hidg->func.set_alt = hidg_set_alt;    // set alternative setting?
	hidg->func.disable = hidg_disable;
	hidg->func.setup   = hidg_setup;
	hidg->func.free_func = hidg_free;

	/* this could me made configurable at some point */
#ifdef OUTEP_ENABLE
	hidg->qlen	   = 4;
#endif
	return &hidg->func;
}

#if 0
DECLARE_USB_FUNCTION_INIT(hid, hidg_alloc_inst, hidg_alloc);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Fabien Chouteau");

int ghid_setup(struct usb_gadget *g, int count)
{
	int status;
	dev_t dev;

	hidg_class = class_create(THIS_MODULE, "hidg");
	if (IS_ERR(hidg_class)) {
		status = PTR_ERR(hidg_class);
		hidg_class = NULL;
		return status;
	}

	status = alloc_chrdev_region(&dev, 0, count, "hidg");
	if (status) {
		class_destroy(hidg_class);
		hidg_class = NULL;
		return status;
	}

	major = MAJOR(dev);
	minors = count;

	return 0;
}

void ghid_cleanup(void)
{
	if (major) {
		unregister_chrdev_region(MKDEV(major, 0), minors);
		major = minors = 0;
	}

	class_destroy(hidg_class);
	hidg_class = NULL;
}
#endif

// Expand DECLARE_USB_FUNCTION_INIT() macro
#if defined(CFG_USBD_HID_DEVICE)
static struct usb_function_driver hidg_dev_usb_func = {
	.name = "hid-dev",
	.mod = THIS_MODULE,
	.alloc_inst = hidg_alloc_inst,
	.alloc_func = hidg_alloc,
};
#endif
#if defined(CFG_USBD_HID_MOUSE)
static struct usb_function_driver hidg_mouse_usb_func = {
	.name = "hid-mouse",
	.mod = THIS_MODULE,
	.alloc_inst = hidg_alloc_inst,
	.alloc_func = hidg_alloc,
};
#endif
#if defined(CFG_USBD_HID_KBD)
static struct usb_function_driver hidg_kbd_usb_func = {
	.name = "hid-kbd",
	.mod = THIS_MODULE,
	.alloc_inst = hidg_alloc_inst,
	.alloc_func = hidg_alloc,
};
#endif

int iteHiddInitialize(void)
{
#if defined(CFG_USBD_HID_DEVICE)
	usb_function_register(&hidg_dev_usb_func);
    list_add(&ite_hidg_device.list, &hidg_default_list);
#endif
#if defined(CFG_USBD_HID_MOUSE)
    usb_function_register(&hidg_mouse_usb_func);
    list_add(&ite_hidg_mouse.list, &hidg_default_list);
#endif
#if defined(CFG_USBD_HID_KBD)
    usb_function_register(&hidg_kbd_usb_func);
    list_add(&ite_hidg_kbd.list, &hidg_default_list);
#endif
	//userial_init();

	dev_dbg(NULL, "iteHiddInitialize\n");
}

int iteHiddTerminate(void)
{
	//userial_cleanup();
#if defined(CFG_USBD_HID_DEVICE)
	usb_function_unregister(&hidg_dev_usb_func);
#endif
#if defined(CFG_USBD_HID_MOUSE)
	usb_function_unregister(&hidg_mouse_usb_func);
#endif
#if defined(CFG_USBD_HID_KBD)
    usb_function_unregister(&hidg_kbd_usb_func);
#endif

    struct hidg_func_config *fc, *tmp;
	mutex_lock(&hidg_func_lock);
	list_for_each_entry_safe(fc, tmp, &hidg_func_list, list) {
		list_del(&fc->list);
        kfree(fc->name);
        kfree(fc->report_desc);
		kfree(fc);
	}
    mutex_unlock(&hidg_func_lock);

	dev_dbg(NULL, "iteHiddTerminate\n");
}
