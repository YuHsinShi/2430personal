/*
* ite.c -- ITE Composite Gadget Driver
*/
#include <stdio.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/err.h>

#include "f_mass_storage.h"

#define DRIVER_DESC "ITE Composite Gadget"

static struct usb_composite_overwrite coverwrite = {
	.idVendor      = cpu_to_le16(CFG_USB_VENDOR_ID),
	.idProduct     = cpu_to_le16(CFG_USB_PRODUCT_ID),
	.bcdDevice     = cpu_to_le16(0x0200),
#if defined(CFG_USBD_MASS_STORAGE)
	.serial_number = CFG_USBD_STR_SERIAL_NUMBER,
#endif
	.manufacturer  = CFG_USBD_STR_MANUFACTURER,
	.product       = CFG_USBD_STR_PRODUCT,
};

/***************************** Device Descriptor ****************************/
#define STRING_DESCRIPTION_IDX USB_GADGET_FIRST_AVAIL_IDX

#define ITE_VENDOR_ID          0x3014
#define ITE_PRODUCT_ID         0x0970

// usb device descriptor, see usb ch9.
static struct usb_device_descriptor device_desc = {
	.bLength            = USB_DT_DEVICE_SIZE,
	.bDescriptorType    = USB_DT_DEVICE,
	/* .bcdUSB = DYNAMIC */
	.bDeviceClass       = USB_CLASS_PER_INTERFACE,
	.idVendor           = cpu_to_le16(ITE_VENDOR_ID),
	.idProduct          = cpu_to_le16(ITE_PRODUCT_ID),
	/* .iManufacturer = DYNAMIC */
	/* .iProduct = DYNAMIC */
	.bNumConfigurations =                           1,
};

static struct usb_string            strings_dev[] = {
	[USB_GADGET_MANUFACTURER_IDX].s = "",
	[USB_GADGET_PRODUCT_IDX].s      = "",
#if 1//defined(CFG_USBD_MASS_STORAGE)
	[USB_GADGET_SERIAL_IDX].s       = "",
#endif
	[STRING_DESCRIPTION_IDX].s      = "ITE Configuration",
	{} /* end of list */
};

static struct usb_gadget_strings    stringtab_dev = {
	.language =      0x0409,  /* en-us */
	.strings  = strings_dev,
};

static struct usb_gadget_strings    *dev_strings[] = {
	&stringtab_dev,
	NULL,
};

/****************************** Configurations ******************************/

/*-------------------------------------------------------------------------*/
static struct usb_function *f_msg_cfg;

static struct usb_configuration     ite_config_driver = {
	.label               = "ITE Gadget",
	.bConfigurationValue =                  1,
	/* .iConfiguration = DYNAMIC */
	.bmAttributes        = USB_CONFIG_ATT_ONE,
	.MaxPower            =                500,
};

static struct usb_function_instance *fi_msg;
static struct usb_function_instance *fi_acm;
static struct usb_function_instance *fi_ncm;
static struct usb_function_instance *fi_uvc;
static struct usb_function_instance *fi_hid;
static struct usb_function_instance *fi_hid_mouse;
static struct usb_function_instance *fi_hid_kbd;

static struct usb_function *f_msg;
static struct usb_function *f_acm;
static struct usb_function *f_ncm;
static struct usb_function *f_uvc;
static struct usb_function          *f_hid;
static struct usb_function          *f_hid_mouse;
static struct usb_function          *f_hid_kbd;

/*-------------------------------------------------------------------------*/
static int ite_do_config(struct usb_configuration *c)
{
	int ret = 0;

if(	is_in_msc_mode())
{
	f_msg = usb_get_function(fi_msg);
	if (IS_ERR(f_msg)) {
		ret = PTR_ERR(f_msg);
		goto err;
	}

	ret = usb_add_function(c, f_msg);
	if (ret)
		goto err_func_msg;
}

#if defined(CFG_USBD_NCM)
    f_ncm = usb_get_function(fi_ncm);
    if (IS_ERR(f_ncm)) {
        ret = PTR_ERR(f_ncm);
        goto err_func_ncm;
	}

    ret = usb_add_function(c, f_ncm);
	if (ret)
        goto err_func_ncm1;
#endif

#if defined(CFG_USBD_UVC)
	f_uvc = usb_get_function(fi_uvc);
	if (IS_ERR(f_uvc)) {
		ret = PTR_ERR(f_uvc);
		goto err_func_uvc;
	}

	ret = usb_add_function(c, f_uvc);
	if (ret)
		goto err_func_uvc1;
#endif

if(	is_in_acm_mode())
{
	f_acm = usb_get_function(fi_acm);
	if (IS_ERR(f_acm)) {
		ret = PTR_ERR(f_acm);
		goto err_func_acm;
	}

	ret = usb_add_function(c, f_acm);
	if (ret)
		goto err_func_acm1;
}

#if defined(CFG_USBD_HID_DEVICE)
	f_hid = usb_get_function(fi_hid);
	if (IS_ERR(f_hid)) {
		ret = PTR_ERR(f_hid);
		goto err_func_hid;
	}

	ret = usb_add_function(c, f_hid);
	if (ret)
		goto err_func_hid1;
#endif

#if defined(CFG_USBD_HID_MOUSE)
        f_hid_mouse = usb_get_function(fi_hid_mouse);
        if (IS_ERR(f_hid_mouse)) {
            ret = PTR_ERR(f_hid_mouse);
            goto err_func_hid_mouse;
        }
    
        ret = usb_add_function(c, f_hid_mouse);
        if (ret)
            goto err_func_hid_mouse1;
#endif

#if defined(CFG_USBD_HID_KBD)
            f_hid_kbd = usb_get_function(fi_hid_kbd);
            if (IS_ERR(f_hid_kbd)) {
                ret = PTR_ERR(f_hid_kbd);
                goto err_func_hid_kbd;
            }
        
            ret = usb_add_function(c, f_hid_kbd);
            if (ret)
                goto err_func_hid_kbd1;
#endif
	return 0;

err_func_hid_kbd1:
#if defined(CFG_USBD_HID_KBD)
	usb_put_function(f_hid_kbd);
#endif
err_func_hid_kbd:
#if defined(CFG_USBD_HID_MOUSE)
	usb_remove_function(c, f_hid_mouse);
#endif
err_func_hid_mouse1:
#if defined(CFG_USBD_HID_MOUSE)
	usb_put_function(f_hid_mouse);
#endif
err_func_hid_mouse:
#if defined(CFG_USBD_HID_DEVICE)
	usb_remove_function(c, f_hid);
#endif
err_func_hid1:
#if defined(CFG_USBD_HID_DEVICE)
	usb_put_function(f_hid);
#endif
err_func_hid:
if( is_in_acm_mode())
{
	usb_remove_function(c, f_acm);
}
err_func_acm1:
if( is_in_acm_mode())
{
	usb_put_function(f_acm);
}
err_func_acm:
#if defined(CFG_USBD_UVC)
	usb_remove_function(c, f_uvc);
#endif
err_func_uvc1:
#if defined(CFG_USBD_UVC)
	usb_put_function(f_uvc);
#endif
err_func_uvc:
#if defined(CFG_USBD_NCM)
	usb_remove_function(c, f_ncm);
#endif
err_func_ncm1:
#if defined(CFG_USBD_NCM)
    usb_put_function(f_ncm);
#endif
err_func_ncm:
if(is_in_msc_mode())
{
	usb_remove_function(c, f_msg);
}
err_func_msg:
if(is_in_msc_mode())
{
	usb_put_function(f_msg);
}
err:
	return ret;
}

/**
 * Used to allocate resources that are shared across the whole device, such as
 * string IDs, and add its configurations using usb_add_config(). This may fail
 * by returning a negative errno value; it should return zero on successful
 * initialization.
 *
 * called by composite_bind();
 *
 * @param cdev the composite device to be bind
 * @return 0 on successful initialization, or a negative errno value.
 */
static int ite_bind(struct usb_composite_dev *cdev)
{
	struct fsg_opts *fsg_opts;
	int status;

	printf("ite_bind() \n");

	status = usb_string_ids_tab(cdev, strings_dev);
	if (status < 0)
		goto err;
	device_desc.iManufacturer = strings_dev[USB_GADGET_MANUFACTURER_IDX].id;
	device_desc.iProduct = strings_dev[USB_GADGET_PRODUCT_IDX].id;
	ite_config_driver.iConfiguration = strings_dev[STRING_DESCRIPTION_IDX].id;

if(is_in_msc_mode())
{

	device_desc.iSerialNumber = strings_dev[USB_GADGET_SERIAL_IDX].id;
	fi_msg = usb_get_function_instance("mass_storage");
	if (IS_ERR(fi_msg)) {
		status = PTR_ERR(fi_msg);
		goto err;
	}
}

#if defined(CFG_USBD_NCM)
	fi_ncm = usb_get_function_instance("ncm");
	if (IS_ERR(fi_ncm)) {
		status = PTR_ERR(fi_ncm);
		goto err_fsg_inst;
	}
#endif

#if defined(CFG_USBD_UVC)
	fi_uvc = usb_get_function_instance("uvc");
	if (IS_ERR(fi_uvc)) {
		status = PTR_ERR(fi_uvc);
		goto err_ncm_inst;
	}
#endif

if(	is_in_acm_mode())
{
	fi_acm = usb_get_function_instance("acm");
	if (IS_ERR(fi_acm)) {
		status = PTR_ERR(fi_acm);
		goto err_uvc_inst;
	}
}
#if defined(CFG_USBD_HID_DEVICE)
	fi_hid = usb_get_function_instance("hid-dev");
	if (IS_ERR(fi_hid)) {
		status = PTR_ERR(fi_hid);
		goto err_hid_inst;
	}
#endif

#if defined(CFG_USBD_HID_MOUSE)
        fi_hid_mouse = usb_get_function_instance("hid-mouse");
        if (IS_ERR(fi_hid_mouse)) {
            status = PTR_ERR(fi_hid_mouse);
            goto err_hid_mouse_inst;
        }
#endif

#if defined(CFG_USBD_HID_KBD)
        fi_hid_kbd = usb_get_function_instance("hid-kbd");
        if (IS_ERR(fi_hid_kbd)) {
            status = PTR_ERR(fi_hid_kbd);
            goto err_hid_kbd_inst;
        }
#endif

	/* finally register the configuration */
	status = usb_add_config(cdev, &ite_config_driver,
	                        ite_do_config);
	if (status < 0)
		goto err_acm_inst;

	usb_composite_overwrite_options(cdev, &coverwrite);

	return 0;

err_hid_inst:
#if defined(CFG_USBD_HID_DEVICE)
	usb_put_function_instance(fi_hid);
#endif
err_hid_mouse_inst:
#if defined(CFG_USBD_HID_MOUSE)
	usb_put_function_instance(fi_hid_mouse);
#endif
err_hid_kbd_inst:
#if defined(CFG_USBD_HID_KBD)
	usb_put_function_instance(fi_hid_kbd);
#endif
err_acm_inst:
if( is_in_acm_mode())
{
	usb_put_function_instance(fi_acm);
}
err_uvc_inst:
#if defined(CFG_USBD_UVC)
	usb_put_function_instance(fi_uvc);
#endif
err_ncm_inst:
#if defined(CFG_USBD_NCM)
	usb_put_function_instance(fi_ncm);
#endif
err_fsg_inst:
if(is_in_msc_mode())
{
	usb_put_function_instance(fi_msg);
}
err:
	return status;
}

static int ite_unbind(struct usb_composite_dev *cdev)
{
#if defined(CFG_USBD_HID_DEVICE)
    usb_put_function(f_hid);
    usb_put_function_instance(fi_hid);
#endif

#if defined(CFG_USBD_HID_MOUSE)
    usb_put_function(f_hid_mouse);
    usb_put_function_instance(fi_hid_mouse);
#endif

#if defined(CFG_USBD_HID_KBD)
    usb_put_function(f_hid_kbd);
    usb_put_function_instance(fi_hid_kbd);
#endif

if(is_in_msc_mode())
{
	usb_put_function(f_msg);
	usb_put_function_instance(fi_msg);
}

#if defined(CFG_USBD_NCM)
	usb_put_function(f_ncm);
	usb_put_function_instance(fi_ncm);
#endif

#if defined(CFG_USBD_UVC)
	usb_put_function(f_uvc);
	usb_put_function_instance(fi_uvc);
#endif

if(	is_in_acm_mode())
{
	usb_put_function(f_acm);
	usb_put_function_instance(fi_acm);
}
	printf("ite_unbind() \n");

	return 0;
}

static struct usb_composite_driver ite_driver = {
	.name = "ite_gadget",           // For diagnostics, identifies the driver.
	.dev = &device_desc,            // Template descriptor for the device,
	// including default device identifiers.
	.strings = dev_strings,
	.max_speed = USB_SPEED_HIGH,
	.bind = ite_bind,
	.unbind = ite_unbind,
#if defined(CFG_USBD_MASS_STORAGE)
	.needs_serial = 1,
#endif
};

int iteUsbCompositeOverwrite(struct usb_composite_overwrite *covr)
{
	if (covr->idVendor)
		coverwrite.idVendor = cpu_to_le16(covr->idVendor);
	if (covr->idProduct)
		coverwrite.idProduct = cpu_to_le16(covr->idProduct);
    return 0;
}

/**
 * @brief Register the internal [usb gadget device](https://www.kernel.org/doc/html/v4.16/driver-api/usb/gadget.html) driver.
 *
 * Register as a composite driver. This function is not thread-safe
 * and had better to be run in the system initial stage.
 *
 * On successful return, the gadget is ready to respond to requests from
 * the host, unless one of its components invokes usb_gadget_disconnect()
 * while it was binding.

 * @return 0 if success, or a negative errno value.
 */
int iteUsbRegisterGadget(void)
{
	return usb_composite_probe(&ite_driver);
}

/**
 * @brief Unregister the internal usb gadget device driver.
 * @see iteUsbRegisterGadget()
 *
 * unregister a composite driver
 * @driver: the driver to unregister
 *
 * This function is used to unregister drivers using the composite
 * driver framework.
 *
 * @return 0 if success, or a negative errno value.
 */
int iteUsbUnRegisterGadget(void)
{
	usb_composite_unregister(&ite_driver);
	return 0;
}
