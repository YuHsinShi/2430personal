/*
 * Provides code common for host and device side USB.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2.
 *
 * If either host side (ie. CONFIG_USB=y) or device side USB stack
 * (ie. CONFIG_USB_GADGET=y) is compiled in the kernel, this module is
 * compiled-in as well.  Otherwise, if either of the two stacks is
 * compiled as module, this file is compiled as module as well.
 */

#include <linux/os.h>
//#include <linux/kernel.h>
#include <linux/module.h>
//#include <linux/of.h>
#include <linux/usb/ch9.h>
//#include <linux/usb/of.h>
//#include <linux/usb/otg.h>
//#include <linux/of_platform.h>

#if 0
const char *usb_otg_state_string(enum usb_otg_state state)
{
	static const char *const names[] = {
		[OTG_STATE_A_IDLE] = "a_idle",
		[OTG_STATE_A_WAIT_VRISE] = "a_wait_vrise",
		[OTG_STATE_A_WAIT_BCON] = "a_wait_bcon",
		[OTG_STATE_A_HOST] = "a_host",
		[OTG_STATE_A_SUSPEND] = "a_suspend",
		[OTG_STATE_A_PERIPHERAL] = "a_peripheral",
		[OTG_STATE_A_WAIT_VFALL] = "a_wait_vfall",
		[OTG_STATE_A_VBUS_ERR] = "a_vbus_err",
		[OTG_STATE_B_IDLE] = "b_idle",
		[OTG_STATE_B_SRP_INIT] = "b_srp_init",
		[OTG_STATE_B_PERIPHERAL] = "b_peripheral",
		[OTG_STATE_B_WAIT_ACON] = "b_wait_acon",
		[OTG_STATE_B_HOST] = "b_host",
	};

	if (state < 0 || state >= ARRAY_SIZE(names))
		return "UNDEFINED";

	return names[state];
}
EXPORT_SYMBOL_GPL(usb_otg_state_string);
#endif
static const char *const speed_names[] = {
	[USB_SPEED_UNKNOWN] = "UNKNOWN",
	[USB_SPEED_LOW] = "low-speed",
	[USB_SPEED_FULL] = "full-speed",
	[USB_SPEED_HIGH] = "high-speed",
	[USB_SPEED_WIRELESS] = "wireless",
	[USB_SPEED_SUPER] = "super-speed",
	[USB_SPEED_SUPER_PLUS] = "super-speed-plus",
};

const char *usb_speed_string(enum usb_device_speed speed)
{
	if (speed < 0 || speed >= ARRAY_SIZE(speed_names))
		speed = USB_SPEED_UNKNOWN;
	return speed_names[speed];
}
EXPORT_SYMBOL_GPL(usb_speed_string);
#if 0
enum usb_device_speed usb_get_maximum_speed(struct device *dev)
{
	const char *maximum_speed;
	int ret;

	ret = device_property_read_string(dev, "maximum-speed", &maximum_speed);
	if (ret < 0)
		return USB_SPEED_UNKNOWN;

	ret = match_string(speed_names, ARRAY_SIZE(speed_names), maximum_speed);

	return (ret < 0) ? USB_SPEED_UNKNOWN : ret;
}
EXPORT_SYMBOL_GPL(usb_get_maximum_speed);
#endif
const char *usb_state_string(enum usb_device_state state)
{
	static const char *const names[] = {
		[USB_STATE_NOTATTACHED] = "not attached",
		[USB_STATE_ATTACHED] = "attached",
		[USB_STATE_POWERED] = "powered",
		[USB_STATE_RECONNECTING] = "reconnecting",
		[USB_STATE_UNAUTHENTICATED] = "unauthenticated",
		[USB_STATE_DEFAULT] = "default",
		[USB_STATE_ADDRESS] = "addressed",
		[USB_STATE_CONFIGURED] = "configured",
		[USB_STATE_SUSPENDED] = "suspended",
	};

	if (state < 0 || state >= ARRAY_SIZE(names))
		return "UNKNOWN";

	return names[state];
}
EXPORT_SYMBOL_GPL(usb_state_string);
#if 0
static const char *const usb_dr_modes[] = {
	[USB_DR_MODE_UNKNOWN]		= "",
	[USB_DR_MODE_HOST]		= "host",
	[USB_DR_MODE_PERIPHERAL]	= "peripheral",
	[USB_DR_MODE_OTG]		= "otg",
};

static enum usb_dr_mode usb_get_dr_mode_from_string(const char *str)
{
	int ret;

	ret = match_string(usb_dr_modes, ARRAY_SIZE(usb_dr_modes), str);
	return (ret < 0) ? USB_DR_MODE_UNKNOWN : ret;
}
#endif

MODULE_LICENSE("GPL");
