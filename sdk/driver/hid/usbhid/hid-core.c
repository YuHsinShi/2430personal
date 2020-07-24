/*
 *  USB HID support for Linux
 *
 *  Copyright (c) 1999 Andreas Gal
 *  Copyright (c) 2000-2005 Vojtech Pavlik <vojtech@suse.cz>
 *  Copyright (c) 2005 Michael Haboustak <mike-@cinci.rr.com> for Concept2, Inc
 *  Copyright (c) 2007-2008 Oliver Neukum
 *  Copyright (c) 2006-2010 Jiri Kosina
 */
// this is a usb device driver

#include <linux/err.h>
#include <linux/hid.h>
#include "ite/ite_usbex.h"
#include "usb/usb/usb.h"
#include "usb/usb/host.h"
#include "usbhid.h"
#include <linux/bitops.h>

#define dev_err(x, ...)

static void hid_io_error(struct hid_device *hid);
static int hid_submit_out(struct hid_device *hid);
static int hid_submit_ctrl(struct hid_device *hid);

/* Start up the input URB */
static int hid_start_in(struct hid_device *hid)
{
	unsigned long flags;
	int rc = 0;
	struct usbhid_device *usbhid = hid->driver_data;

	//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);
	spin_lock_irqsave(&usbhid->lock, flags);
	if (test_bit(HID_IN_POLLING, &usbhid->iofl) &&
	    !test_bit(HID_DISCONNECTED, &usbhid->iofl) &&
	    !test_bit(HID_SUSPENDED, &usbhid->iofl) &&
	    !test_and_set_bit(HID_IN_RUNNING, &usbhid->iofl)) {
		rc = usb_submit_urb(usbhid->urbin);
		if (rc != 0) {
			clear_bit(HID_IN_RUNNING, &usbhid->iofl);
			if (rc == -ENOSPC)
				set_bit(HID_NO_BANDWIDTH, &usbhid->iofl);
		} else {
			clear_bit(HID_NO_BANDWIDTH, &usbhid->iofl);
		}
	}
	spin_unlock_irqrestore(&usbhid->lock, flags);
	//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);
	return rc;
}

/* Main I/O error handler */
static void hid_io_error(struct hid_device *hid)
{
	unsigned long flags;
	struct usbhid_device *usbhid = hid->driver_data;

	/* Stop when disconnected */
	if (test_bit(HID_DISCONNECTED, &usbhid->iofl))
		goto done;

	/* If it has been a while since the last error, we'll assume
	 * this a brand new error and reset the retry timeout. */
	if (time_after(jiffies, usbhid->stop_retry + HZ/2))
		usbhid->retry_delay = 0;

	/* When an error occurs, retry at increasing intervals */
	if (usbhid->retry_delay == 0) {
		usbhid->retry_delay = 13;	/* Then 26, 52, 104, 104, ... */
		usbhid->stop_retry = jiffies + msecs_to_jiffies(1000);
	} else if (usbhid->retry_delay < 100)
		usbhid->retry_delay *= 2;

	if (time_after(jiffies, usbhid->stop_retry)) {

		/* Retries failed, so do a port reset unless we lack bandwidth*/
		if (!test_bit(HID_NO_BANDWIDTH, &usbhid->iofl)
		     && !test_and_set_bit(HID_RESET_PENDING, &usbhid->iofl)) {

			//schedule_work(&usbhid->reset_work);
			goto done;
		}
	}
done:
	return;
}

static void usbhid_mark_busy(struct usbhid_device *usbhid)
{
	struct usb_interface *intf = usbhid->intf;

	usb_mark_last_busy(interface_to_usbdev(intf));
}

static int usbhid_restart_out_queue(struct usbhid_device *usbhid)
{
	struct hid_device *hid = usb_get_intfdata(usbhid->intf);
	int kicked;
	int r;

	if (!hid || test_bit(HID_RESET_PENDING, &usbhid->iofl) ||
			test_bit(HID_SUSPENDED, &usbhid->iofl))
		return 0;

	if ((kicked = (usbhid->outhead != usbhid->outtail))) {
		hid_dbg(hid, "Kicking head %d tail %d", usbhid->outhead, usbhid->outtail);

		/* Try to wake up from autosuspend... */
		r = usb_autopm_get_interface_async(usbhid->intf);
		if (r < 0)
			return r;

		/*
		 * If still suspended, don't submit.  Submission will
		 * occur if/when resume drains the queue.
		 */
		if (test_bit(HID_SUSPENDED, &usbhid->iofl)) {
			usb_autopm_put_interface_no_suspend(usbhid->intf);
			return r;
		}

		/* Asynchronously flush queue. */
		set_bit(HID_OUT_RUNNING, &usbhid->iofl);
		if (hid_submit_out(hid)) {
			clear_bit(HID_OUT_RUNNING, &usbhid->iofl);
			usb_autopm_put_interface_async(usbhid->intf);
		}
		//wake_up(&usbhid->wait);
	}
	return kicked;
}

static int usbhid_restart_ctrl_queue(struct usbhid_device *usbhid)
{
	struct hid_device *hid = usb_get_intfdata(usbhid->intf);
	int kicked;
	int r;

	WARN_ON(hid == NULL);
	if (!hid || test_bit(HID_RESET_PENDING, &usbhid->iofl) ||
			test_bit(HID_SUSPENDED, &usbhid->iofl))
	{
		//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);
		return 0;
	}

	//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);
	if ((kicked = (usbhid->ctrlhead != usbhid->ctrltail))) {
		hid_dbg(hid, "Kicking head %d tail %d", usbhid->ctrlhead, usbhid->ctrltail);

		//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);
		/* Try to wake up from autosuspend... */
		r = usb_autopm_get_interface_async(usbhid->intf);
		if (r < 0)
			return r;
		//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);

		/*
		 * If still suspended, don't submit.  Submission will
		 * occur if/when resume drains the queue.
		 */
		if (test_bit(HID_SUSPENDED, &usbhid->iofl)) {
			usb_autopm_put_interface_no_suspend(usbhid->intf);
			return r;
		}
		//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);

		/* Asynchronously flush queue. */
		set_bit(HID_CTRL_RUNNING, &usbhid->iofl);
		//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);
		if (hid_submit_ctrl(hid)) {
			//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);
			clear_bit(HID_CTRL_RUNNING, &usbhid->iofl);
			usb_autopm_put_interface_async(usbhid->intf);
		}
		//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);
	}
	return kicked;
}

/*
 * Input interrupt completion handler.
 */

static void hid_irq_in(struct urb *urb)
{
	struct hid_device	*hid = urb->context;
	struct usbhid_device	*usbhid = hid->driver_data;
	int			status;

	//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);

	switch (urb->status) {
	case 0:			/* success */
		usbhid->retry_delay = 0;
		//if (!test_bit(HID_OPENED, &usbhid->iofl)) {
			//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);
			//break;
		//}
		//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);
		usbhid_mark_busy(usbhid);
		if (!test_bit(HID_RESUME_RUNNING, &usbhid->iofl)) {
			//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);
			hid_input_report(urb->context, HID_INPUT_REPORT,
					 urb->transfer_buffer,
					 urb->actual_length, 1);
			/*
			 * autosuspend refused while keys are pressed
			 * because most keyboards don't wake up when
			 * a key is released
			 */
			//if (hid_check_keys_pressed(hid))
			//	set_bit(HID_KEYS_PRESSED, &usbhid->iofl);
			//else
			//	clear_bit(HID_KEYS_PRESSED, &usbhid->iofl);
		}
		break;
	case -EPIPE:		/* stall */
		usbhid_mark_busy(usbhid);
		clear_bit(HID_IN_RUNNING, &usbhid->iofl);
		set_bit(HID_CLEAR_HALT, &usbhid->iofl);
		return;
	case -ECONNRESET:	/* unlink */
	case -ENOENT:
	case -ESHUTDOWN:	/* unplug */
		clear_bit(HID_IN_RUNNING, &usbhid->iofl);
		return;
	case -EILSEQ:		/* protocol error or unplug */
	case -EPROTO:		/* protocol error or unplug */
	case -ETIME:		/* protocol error or unplug */
	case -ETIMEDOUT:	/* Should never happen, but... */
		usbhid_mark_busy(usbhid);
		clear_bit(HID_IN_RUNNING, &usbhid->iofl);
		hid_io_error(hid);
		return;
	default:		/* error */
		hid_warn(urb->dev, "input irq status %d received\n",
			 urb->status);
	}

	status = usb_submit_urb(urb);
	if (status) {
		clear_bit(HID_IN_RUNNING, &usbhid->iofl);
		if (status != -EPERM) {
			hid_err(hid, "can't resubmit intr, %s-%s/input%d, status %d\n",
				hid_to_usb_dev(hid)->bus->bus_name,
				hid_to_usb_dev(hid)->devpath,
				usbhid->ifnum, status);
			hid_io_error(hid);
		}
	}
}

static int hid_submit_out(struct hid_device *hid)
{
	struct hid_report *report;
	char *raw_report;
	struct usbhid_device *usbhid = hid->driver_data;
	int r;

	report = usbhid->out[usbhid->outtail].report;
	raw_report = usbhid->out[usbhid->outtail].raw_report;

	usbhid->urbout->transfer_buffer_length = hid_report_len(report);
	usbhid->urbout->dev = hid_to_usb_dev(hid);
	if (raw_report) {
		memcpy(usbhid->outbuf, raw_report,
				usbhid->urbout->transfer_buffer_length);
		kfree(raw_report);
		usbhid->out[usbhid->outtail].raw_report = NULL;
	}

	dbg_hid("submitting out urb\n");

	r = usb_submit_urb(usbhid->urbout);
	if (r < 0) {
		hid_err(hid, "usb_submit_urb(out) failed: %d\n", r);
		return r;
	}
	usbhid->last_out = jiffies;
	return 0;
}

static int hid_submit_ctrl(struct hid_device *hid)
{
	struct hid_report *report;
	unsigned char dir;
	char *raw_report;
	int len, r;
	struct usbhid_device *usbhid = hid->driver_data;

	report = usbhid->ctrl[usbhid->ctrltail].report;
	raw_report = usbhid->ctrl[usbhid->ctrltail].raw_report;
	dir = usbhid->ctrl[usbhid->ctrltail].dir;

	len = ((report->size - 1) >> 3) + 1 + (report->id > 0);
	if (dir == USB_DIR_OUT) {
                //ithPrintf("%s(%d) usb_intf(%p)\n", __FUNCTION__,__LINE__, to_usb_interface(hid));
		usbhid->urbctrl->pipe = usb_sndctrlpipe(hid_to_usb_dev(hid), 0);
		usbhid->urbctrl->transfer_buffer_length = len;
		if (raw_report) {
			ithPrintf("%s(%d) len(%d)\n", __FUNCTION__, __LINE__, len);
			ithPrintVram8((uint32_t)raw_report, len);
			memcpy(usbhid->ctrlbuf, raw_report, len);
			kfree(raw_report);
			usbhid->ctrl[usbhid->ctrltail].raw_report = NULL;
		}
	} else {
		int maxpacket, padlen;

		usbhid->urbctrl->pipe = usb_rcvctrlpipe(hid_to_usb_dev(hid), 0);
		maxpacket = usb_maxpacket(hid_to_usb_dev(hid),
					  usbhid->urbctrl->pipe, 0);
		if (maxpacket > 0) {
			padlen = DIV_ROUND_UP(len, maxpacket);
			padlen *= maxpacket;
			if (padlen > usbhid->bufsize)
				padlen = usbhid->bufsize;
		} else
			padlen = 0;
		usbhid->urbctrl->transfer_buffer_length = padlen;
	}
	usbhid->urbctrl->dev = hid_to_usb_dev(hid);

	usbhid->cr->bRequestType = USB_TYPE_CLASS | USB_RECIP_INTERFACE | dir;
	usbhid->cr->bRequest = (dir == USB_DIR_OUT) ? HID_REQ_SET_REPORT :
						      HID_REQ_GET_REPORT;
	usbhid->cr->wValue = cpu_to_le16(((report->type + 1) << 8) |
					 report->id);
	usbhid->cr->wIndex = cpu_to_le16(usbhid->ifnum);
	usbhid->cr->wLength = cpu_to_le16(len);

	//ithPrintf("submitting ctrl urb: %s wValue=0x%04x wIndex=0x%04x wLength=%u\n",
	//	usbhid->cr->bRequest == HID_REQ_SET_REPORT ? "Set_Report" :
	//						     "Get_Report",
	//	usbhid->cr->wValue, usbhid->cr->wIndex, usbhid->cr->wLength);
	dbg_hid("submitting ctrl urb: %s wValue=0x%04x wIndex=0x%04x wLength=%u\n",
		usbhid->cr->bRequest == HID_REQ_SET_REPORT ? "Set_Report" :
							     "Get_Report",
		usbhid->cr->wValue, usbhid->cr->wIndex, usbhid->cr->wLength);

	r = usb_submit_urb(usbhid->urbctrl);
	
	//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);
	if (r < 0) {
		
		//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);
		hid_err(hid, "usb_submit_urb(ctrl) failed: %d\n", r);
		return r;
	}
	
	//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);
	usbhid->last_ctrl = jiffies;
	return 0;
}

/*
 * Output interrupt completion handler.
 */

static void hid_irq_out(struct urb *urb)
{
	struct hid_device *hid = urb->context;
	struct usbhid_device *usbhid = hid->driver_data;
	unsigned long flags;
	int unplug = 0;

	switch (urb->status) {
	case 0:			/* success */
		break;
	case -ESHUTDOWN:	/* unplug */
		unplug = 1;
	case -EILSEQ:		/* protocol error or unplug */
	case -EPROTO:		/* protocol error or unplug */
	case -ECONNRESET:	/* unlink */
	case -ENOENT:
		break;
	default:		/* error */
		hid_warn(urb->dev, "output irq status %d received\n",
			 urb->status);
	}

	spin_lock_irqsave(&usbhid->lock, flags);

	if (unplug) {
		usbhid->outtail = usbhid->outhead;
	} else {
		usbhid->outtail = (usbhid->outtail + 1) & (HID_OUTPUT_FIFO_SIZE - 1);

		if (usbhid->outhead != usbhid->outtail &&
				hid_submit_out(hid) == 0) {
			/* Successfully submitted next urb in queue */
			spin_unlock_irqrestore(&usbhid->lock, flags);
			return;
		}
	}

	clear_bit(HID_OUT_RUNNING, &usbhid->iofl);
	spin_unlock_irqrestore(&usbhid->lock, flags);
	usb_autopm_put_interface_async(usbhid->intf);
}

/*
 * Control pipe completion handler.
 */

static void hid_ctrl(struct urb *urb)
{
	struct hid_device *hid = urb->context;
	struct usbhid_device *usbhid = hid->driver_data;
	int unplug = 0, status = urb->status;

	switch (status) {
	case 0:			/* success */
		if (usbhid->ctrl[usbhid->ctrltail].dir == USB_DIR_IN)   // control fifo 的 direction如果是IN
			hid_input_report(urb->context,                      // 透過 hid_input_report 將收到的 raw data packet 傳送到 hid core
				usbhid->ctrl[usbhid->ctrltail].report->type,
				urb->transfer_buffer, urb->actual_length, 0);
		break;
	case -ESHUTDOWN:	/* unplug */
		unplug = 1;
	case -EILSEQ:		/* protocol error or unplug */
	case -EPROTO:		/* protocol error or unplug */
	case -ECONNRESET:	/* unlink */
	case -ENOENT:
	case -EPIPE:		/* report not available */
		break;
	default:		/* error */
		hid_warn(urb->dev, "ctrl urb status %d received\n", status);
	}

	spin_lock(&usbhid->lock);

	if (unplug) {
		usbhid->ctrltail = usbhid->ctrlhead;
	} else {
		usbhid->ctrltail = (usbhid->ctrltail + 1) & (HID_CONTROL_FIFO_SIZE - 1);

		if (usbhid->ctrlhead != usbhid->ctrltail &&
				hid_submit_ctrl(hid) == 0) {
			/* Successfully submitted next urb in queue */
			spin_unlock(&usbhid->lock);
			return;
		}
	}

	clear_bit(HID_CTRL_RUNNING, &usbhid->iofl);
	spin_unlock(&usbhid->lock);
	usb_autopm_put_interface_async(usbhid->intf);
}

static void __usbhid_submit_report(struct hid_device *hid, struct hid_report *report,
				   unsigned char dir)
{
	int head;
	struct usbhid_device *usbhid = hid->driver_data;

	//ithPrintf("======%s(%d)\n", __FUNCTION__, __LINE__);

	if (((hid->quirks & HID_QUIRK_NOGET) && dir == USB_DIR_IN) ||
		test_bit(HID_DISCONNECTED, &usbhid->iofl))
		return;

	//ithPrintf("======%s(%d)usbhid->urbout(%p)dir(%d)report->type(%d)\n", __FUNCTION__, __LINE__,
	//	usbhid->urbout, dir, report->type);
	if (usbhid->urbout && dir == USB_DIR_OUT && report->type == HID_OUTPUT_REPORT) {
		
		//ithPrintf("======%s(%d)\n", __FUNCTION__, __LINE__);
		if ((head = (usbhid->outhead + 1) & (HID_OUTPUT_FIFO_SIZE - 1)) == usbhid->outtail) {
			hid_warn(hid, "output queue full\n");
			return;
		}

		//ithPrintf("======%s(%d)\n", __FUNCTION__, __LINE__);
		usbhid->out[usbhid->outhead].raw_report = hid_alloc_report_buf(report, GFP_ATOMIC);
		if (!usbhid->out[usbhid->outhead].raw_report) {
			hid_warn(hid, "output queueing failed\n");
			return;
		}
		hid_output_report(report, usbhid->out[usbhid->outhead].raw_report);
		usbhid->out[usbhid->outhead].report = report;
		usbhid->outhead = head;

		/* If the queue isn't running, restart it */
		if (!test_bit(HID_OUT_RUNNING, &usbhid->iofl)) {
			usbhid_restart_out_queue(usbhid);

		/* Otherwise see if an earlier request has timed out */
		} else if (time_after(jiffies, usbhid->last_out + HZ * 5)) {

			/* Prevent autosuspend following the unlink */
			usb_autopm_get_interface_no_resume(usbhid->intf);

			/*
			 * Prevent resubmission in case the URB completes
			 * before we can unlink it.  We don't want to cancel
			 * the wrong transfer!
			 */
			usb_block_urb(usbhid->urbout);

			/* Drop lock to avoid deadlock if the callback runs */
			spin_unlock(&usbhid->lock);

			usb_unlink_urb(usbhid->urbout);
			spin_lock(&usbhid->lock);
			usb_unblock_urb(usbhid->urbout);

			/* Unlink might have stopped the queue */
			if (!test_bit(HID_OUT_RUNNING, &usbhid->iofl))
				usbhid_restart_out_queue(usbhid);

			/* Now we can allow autosuspend again */
			usb_autopm_put_interface_async(usbhid->intf);
		}
		return;
	}

	if ((head = (usbhid->ctrlhead + 1) & (HID_CONTROL_FIFO_SIZE - 1)) == usbhid->ctrltail) {
		hid_warn(hid, "control queue full\n");
		//ithPrintf("======%s(%d)\n", __FUNCTION__, __LINE__);
		return;
	}

	if (dir == USB_DIR_OUT) {
		//ithPrintf("======%s(%d)\n", __FUNCTION__, __LINE__);
		usbhid->ctrl[usbhid->ctrlhead].raw_report = hid_alloc_report_buf(report, GFP_ATOMIC);
		if (!usbhid->ctrl[usbhid->ctrlhead].raw_report) {
			hid_warn(hid, "control queueing failed\n");
			return;
		}
		hid_output_report(report, usbhid->ctrl[usbhid->ctrlhead].raw_report);
	}
	usbhid->ctrl[usbhid->ctrlhead].report = report;
	usbhid->ctrl[usbhid->ctrlhead].dir = dir;
	usbhid->ctrlhead = head;

	/* If the queue isn't running, restart it */
	if (!test_bit(HID_CTRL_RUNNING, &usbhid->iofl)) {
		usbhid_restart_ctrl_queue(usbhid);

		//ithPrintf("======%s(%d)\n", __FUNCTION__, __LINE__);
	/* Otherwise see if an earlier request has timed out */
	} else if (time_after(jiffies, usbhid->last_ctrl + HZ * 5)) {	
		//ithPrintf("======%s(%d)\n", __FUNCTION__, __LINE__);
		/* Prevent autosuspend following the unlink */
		usb_autopm_get_interface_no_resume(usbhid->intf);

		/*
		 * Prevent resubmission in case the URB completes
		 * before we can unlink it.  We don't want to cancel
		 * the wrong transfer!
		 */
		usb_block_urb(usbhid->urbctrl);
		usb_unlink_urb(usbhid->urbctrl);
		usb_unblock_urb(usbhid->urbctrl);

		/* Unlink might have stopped the queue */
		if (!test_bit(HID_CTRL_RUNNING, &usbhid->iofl))
			usbhid_restart_ctrl_queue(usbhid);

		/* Now we can allow autosuspend again */
		usb_autopm_put_interface_async(usbhid->intf);
	}
	//ithPrintf("======%s(%d)\n", __FUNCTION__, __LINE__);

}

static void usbhid_submit_report(struct hid_device *hid, struct hid_report *report, unsigned char dir)
{
	struct usbhid_device *usbhid = hid->driver_data;
	unsigned long flags;

	spin_lock_irqsave(&usbhid->lock, flags);
	__usbhid_submit_report(hid, report, dir);
	spin_unlock_irqrestore(&usbhid->lock, flags);
}

static int hid_set_idle(struct usb_device *dev, int ifnum, int report, int idle)
{
	return usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		HID_REQ_SET_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE, (idle << 8) | report,
		ifnum, NULL, 0, USB_CTRL_SET_TIMEOUT);
}

// get usb class descriptor
static int hid_get_class_descriptor(struct usb_device *dev, int ifnum,
		unsigned char type, void *buf, int size)
{
	int result, retries = 4;

	memset(buf, 0, size);

	do {
		result = usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
				USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_IN,
				(type << 8), ifnum, buf, size, USB_CTRL_GET_TIMEOUT);
		retries--;
	} while (result < size && retries);
	return result;
}

/*
 * Reset LEDs which BIOS might have left on. For now, just NumLock (0x01).
 */
static int hid_find_field_early(struct hid_device *hid, unsigned int page,
    unsigned int hid_code, struct hid_field **pfield)
{
	struct hid_report *report;
	struct hid_field *field;
	struct hid_usage *usage;
	int i, j;

	list_for_each_entry(report, &hid->report_enum[HID_OUTPUT_REPORT].report_list, list) {
		for (i = 0; i < report->maxfield; i++) {
			field = report->field[i];
			for (j = 0; j < field->maxusage; j++) {
				usage = &field->usage[j];
				if ((usage->hid & HID_USAGE_PAGE) == page &&
				    (usage->hid & 0xFFFF) == hid_code) {
					*pfield = field;
					return j;
				}
			}
		}
	}
	return -1;
}

static void usbhid_set_leds(struct hid_device *hid)
{
	struct hid_field *field;
	int offset;

	if ((offset = hid_find_field_early(hid, HID_UP_LED, 0x01, &field)) != -1) {
		hid_set_field(field, offset, 0);
		usbhid_submit_report(hid, field->report, USB_DIR_OUT);
	}
}

/*
 * Traverse the supplied list of reports and find the longest
 */
static void hid_find_max_report(struct hid_device *hid, unsigned int type,
		unsigned int *max)
{
	struct hid_report *report;
	unsigned int size;

	list_for_each_entry(report, &hid->report_enum[type].report_list, list) {
		//ithPrintf("report->size(%u)\n", report->size);
		size = ((report->size - 1) >> 3) + 1 + hid->report_enum[type].numbered;
		if (*max < size)
			*max = size;
	}
}

static int hid_alloc_buffers(struct usb_device *dev, struct hid_device *hid)
{
	struct usbhid_device *usbhid = hid->driver_data;

	usbhid->inbuf = usb_alloc_coherent(dev, usbhid->bufsize, GFP_KERNEL,
			&usbhid->inbuf_dma);
	usbhid->outbuf = usb_alloc_coherent(dev, usbhid->bufsize, GFP_KERNEL,
			&usbhid->outbuf_dma);
	usbhid->cr = kmalloc(sizeof(*usbhid->cr), GFP_KERNEL);
	usbhid->ctrlbuf = usb_alloc_coherent(dev, usbhid->bufsize, GFP_KERNEL,
			&usbhid->ctrlbuf_dma);
	if (!usbhid->inbuf || !usbhid->outbuf || !usbhid->cr ||
			!usbhid->ctrlbuf)
		return -1;

	return 0;
}

static int usbhid_set_raw_report(struct hid_device *hid, unsigned int reportnum,
				 __u8 *buf, size_t count, unsigned char rtype)
{
	struct usbhid_device *usbhid = hid->driver_data;
	struct usb_device *dev = hid_to_usb_dev(hid);
	struct usb_interface *intf = usbhid->intf;
	struct usb_host_interface *interface = intf->cur_altsetting;
	int ret, skipped_report_id = 0;

#if 0
	/* Byte 0 is the report number. Report data starts at byte 1.*/
	if ((rtype == HID_OUTPUT_REPORT) &&
	    (hid->quirks & HID_QUIRK_SKIP_OUTPUT_REPORT_ID))
		buf[0] = 0;
	else
		buf[0] = reportnum;
	if (buf[0] == 0x0) {
		/* Don't send the Report ID */
		buf++;
		count--;
		skipped_report_id = 1;
	}
#endif
	//ithPrintf("%s(%d) count(%d)\n", __FUNCTION__, __LINE__, count);

	ret = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
			HID_REQ_SET_REPORT,
			USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			((rtype + 1) << 8) | reportnum,
			interface->desc.bInterfaceNumber, buf, count,
			USB_CTRL_SET_TIMEOUT);
	/* count also the report id, if this was a numbered report. */
	if (ret > 0 && skipped_report_id)
		ret++;

	return ret;
}

static int usbhid_output_report(struct hid_device *hid, __u8 *buf, size_t count)
{
	struct usbhid_device *usbhid = hid->driver_data;
	struct usb_device *dev = hid_to_usb_dev(hid);
	int actual_length, skipped_report_id = 0, ret;

	if (!usbhid->urbout)
		return -ENOSYS;

	if (buf[0] == 0x0) {
		/* Don't send the Report ID */
		buf++;
		count--;
		skipped_report_id = 1;
	}

	ret = usb_interrupt_msg(dev, usbhid->urbout->pipe,
				buf, count, &actual_length,
				USB_CTRL_SET_TIMEOUT);
	/* return the number of bytes transferred */
	if (ret == 0) {
		ret = actual_length;
		/* count also the report id */
		if (skipped_report_id)
			ret++;
	}

	return ret;
}

static void hid_free_buffers(struct usb_device *dev, struct hid_device *hid)
{
	struct usbhid_device *usbhid = hid->driver_data;

	usb_free_coherent(dev, usbhid->bufsize, usbhid->inbuf, usbhid->inbuf_dma);
	usb_free_coherent(dev, usbhid->bufsize, usbhid->outbuf, usbhid->outbuf_dma);
	kfree(usbhid->cr);
	usb_free_coherent(dev, usbhid->bufsize, usbhid->ctrlbuf, usbhid->ctrlbuf_dma);
}

static int usbhid_parse(struct hid_device *hid)
{
	struct usb_interface *intf = to_usb_interface(hid->dev.parent);
	struct usb_host_interface *interface = intf->cur_altsetting;
	struct usb_device *dev = interface_to_usbdev (intf);
	struct hid_descriptor *hdesc;
	u32 quirks = 0;
	unsigned int rsize = 0;
	char *rdesc;
	int ret, n;

	//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);

	/* Many keyboards and mice don't like to be polled for reports,
	 * so we will always set the HID_QUIRK_NOGET flag for them. */
	if (interface->desc.bInterfaceSubClass == USB_INTERFACE_SUBCLASS_BOOT) {
		if (interface->desc.bInterfaceProtocol == USB_INTERFACE_PROTOCOL_KEYBOARD ||
			interface->desc.bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE)
				quirks |= HID_QUIRK_NOGET;
	}

	if (usb_get_extra_descriptor(interface, HID_DT_HID, &hdesc) &&
	    (!interface->desc.bNumEndpoints ||
	     usb_get_extra_descriptor(&interface->endpoint[0], HID_DT_HID, &hdesc))) {
		dbg_hid("class descriptor not present\n");
		//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);
		return -ENODEV;
	}
	
	for (n = 0; n < hdesc->bNumDescriptors; n++)
		if (hdesc->desc[n].bDescriptorType == HID_DT_REPORT)
			rsize = le16_to_cpu(hdesc->desc[n].wDescriptorLength);

	if (!rsize || rsize > HID_MAX_DESCRIPTOR_SIZE) {
		dbg_hid("weird size of report descriptor (%u)\n", rsize);
		return -EINVAL;
	}

	rdesc = kmalloc(rsize, GFP_KERNEL);
	if (!rdesc)
		return -ENOMEM;

	hid_set_idle(dev, interface->desc.bInterfaceNumber, 0, 0);

	ret = hid_get_class_descriptor(dev, interface->desc.bInterfaceNumber,
			HID_DT_REPORT, rdesc, rsize);
	if (ret < 0) {
		dbg_hid("reading report descriptor failed\n");
		kfree(rdesc);
		goto err;
	}

	ret = hid_parse_report(hid, rdesc, rsize);
	kfree(rdesc);
	if (ret) {
		dbg_hid("parsing report descriptor failed\n");
		goto err;
	}

	hid->quirks |= quirks;

	return 0;
err:
	return ret;
}

static int usbhid_start(struct hid_device *hid)
{
	struct usb_interface *intf = to_usb_interface(hid->dev.parent);
	struct usb_host_interface *interface = intf->cur_altsetting;
	struct usb_device *dev = interface_to_usbdev(intf);
	struct usbhid_device *usbhid = hid->driver_data;
	unsigned int n, insize = 0;
	int ret;

	//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);

	clear_bit(HID_DISCONNECTED, &usbhid->iofl);

	usbhid->bufsize = HID_MIN_BUFFER_SIZE;
	hid_find_max_report(hid, HID_INPUT_REPORT, &usbhid->bufsize);
	hid_find_max_report(hid, HID_OUTPUT_REPORT, &usbhid->bufsize);
	hid_find_max_report(hid, HID_FEATURE_REPORT, &usbhid->bufsize);
 
	if (usbhid->bufsize > HID_MAX_BUFFER_SIZE)
		usbhid->bufsize = HID_MAX_BUFFER_SIZE;

	hid_find_max_report(hid, HID_INPUT_REPORT, &insize);

	if (insize > HID_MAX_BUFFER_SIZE)
		insize = HID_MAX_BUFFER_SIZE;

	if (hid_alloc_buffers(dev, hid)) {
		ret = -ENOMEM;
		goto fail;
	}

	//ithPrintf("%s(%d)interface->desc.bNumEndpoints(%d)\n", __FUNCTION__, __LINE__, interface->desc.bNumEndpoints);
	for (n = 0; n < interface->desc.bNumEndpoints; n++) {
		struct usb_endpoint_descriptor *endpoint;
		int pipe;
		int interval;

		endpoint = &interface->endpoint[n].desc;
		if (!usb_endpoint_xfer_int(endpoint))
			continue;

		interval = endpoint->bInterval;

		/* Some vendors give fullspeed interval on highspeed devides */
		if (hid->quirks & HID_QUIRK_FULLSPEED_INTERVAL &&
		    dev->speed == USB_SPEED_HIGH) {
			interval = fls(endpoint->bInterval*8);
			//pr_info("%s: Fixing fullspeed to highspeed interval: %d -> %d\n",
			//	hid->name, endpoint->bInterval, interval);
		}

#if 0
		/* Change the polling interval of mice and joysticks. */
		switch (hid->collection->usage) {
		case HID_GD_MOUSE:
			if (hid_mousepoll_interval > 0)
				interval = hid_mousepoll_interval;
			break;
		case HID_GD_JOYSTICK:
			if (hid_jspoll_interval > 0)
				interval = hid_jspoll_interval;
			break;
		}
#endif

		ret = -ENOMEM;
		if (usb_endpoint_dir_in(endpoint)) {
			//ithPrintf("%s(%d) IN\n", __FUNCTION__, __LINE__);
			if (usbhid->urbin)
				continue;
			if (!(usbhid->urbin = usb_alloc_urb(0)))
				goto fail;
			pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);
			usb_fill_int_urb(usbhid->urbin, dev, pipe, usbhid->inbuf, insize,
					 hid_irq_in, hid, interval);
			usbhid->urbin->transfer_dma = usbhid->inbuf_dma;
			usbhid->urbin->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
		} else {
			//ithPrintf("%s(%d) OUT\n", __FUNCTION__, __LINE__);
			if (usbhid->urbout)
				continue;
			if (!(usbhid->urbout = usb_alloc_urb(0)))
				goto fail;
			pipe = usb_sndintpipe(dev, endpoint->bEndpointAddress);
			usb_fill_int_urb(usbhid->urbout, dev, pipe, usbhid->outbuf, 0,
					 hid_irq_out, hid, interval);
			usbhid->urbout->transfer_dma = usbhid->outbuf_dma;
			usbhid->urbout->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
		}
	}

	usbhid->urbctrl = usb_alloc_urb(0);
	if (!usbhid->urbctrl) {
		ret = -ENOMEM;
		goto fail;
	}

	usb_fill_control_urb(usbhid->urbctrl, dev, 0, (void *) usbhid->cr,
			     usbhid->ctrlbuf, 1, hid_ctrl, hid);
	usbhid->urbctrl->transfer_dma = usbhid->ctrlbuf_dma;
	usbhid->urbctrl->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	set_bit(HID_STARTED, &usbhid->iofl);

	if (hid->quirks & HID_QUIRK_ALWAYS_POLL) {
		ret = usb_autopm_get_interface(usbhid->intf);
		if (ret)
			goto fail;
		set_bit(HID_IN_POLLING, &usbhid->iofl);
		//usbhid->intf->needs_remote_wakeup = 1;
		ret = hid_start_in(hid);
		if (ret) {
			dev_err(&hid->dev,
				"failed to start in urb: %d\n", ret);
		}
		usb_autopm_put_interface(usbhid->intf);
	}

	/* Some keyboards don't work until their LEDs have been set.
	 * Since BIOSes do set the LEDs, it must be safe for any device
	 * that supports the keyboard boot protocol.
	 * In addition, enable remote wakeup by default for all keyboard
	 * devices supporting the boot protocol.
	 */
	if (interface->desc.bInterfaceSubClass == USB_INTERFACE_SUBCLASS_BOOT &&
			interface->desc.bInterfaceProtocol ==
				USB_INTERFACE_PROTOCOL_KEYBOARD) {
		usbhid_set_leds(hid);
	}
	//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);

	return 0;

fail:
	usb_free_urb(usbhid->urbin);
	usb_free_urb(usbhid->urbout);
	usb_free_urb(usbhid->urbctrl);
	usbhid->urbin = NULL;
	usbhid->urbout = NULL;
	usbhid->urbctrl = NULL;
	hid_free_buffers(dev, hid);
	return ret;
}

static void usbhid_stop(struct hid_device *hid)
{
	struct usbhid_device *usbhid = hid->driver_data;

	if (WARN_ON(!usbhid))
		return;

	if (hid->quirks & HID_QUIRK_ALWAYS_POLL) {
		clear_bit(HID_IN_POLLING, &usbhid->iofl);
		//usbhid->intf->needs_remote_wakeup = 0;
	}

	clear_bit(HID_STARTED, &usbhid->iofl);
	spin_lock_irq(&usbhid->lock);	/* Sync with error and led handlers */
	set_bit(HID_DISCONNECTED, &usbhid->iofl);
	spin_unlock_irq(&usbhid->lock);
	usb_kill_urb(usbhid->urbin);
	usb_kill_urb(usbhid->urbout);
	usb_kill_urb(usbhid->urbctrl);

	//hid_cancel_delayed_stuff(usbhid);

	hid->claimed = 0;

	usb_free_urb(usbhid->urbin);
	usb_free_urb(usbhid->urbctrl);
	usb_free_urb(usbhid->urbout);
	usbhid->urbin = NULL; /* don't mess up next start */
	usbhid->urbctrl = NULL;
	usbhid->urbout = NULL;

	hid_free_buffers(hid_to_usb_dev(hid), hid);
}

static int usbhid_raw_request(struct hid_device *hid, unsigned char reportnum,
			      __u8 *buf, size_t len, unsigned char rtype,
			      int reqtype)
{
	//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);
	switch (reqtype) {
	//case HID_REQ_GET_REPORT:
	//	break;
	case HID_REQ_SET_REPORT:
		return usbhid_set_raw_report(hid, reportnum, buf, len, rtype);
		break;
	default:
		return -EIO;
	}
}

static int usbhid_idle(struct hid_device *hid, int report, int idle,
		int reqtype)
{
	struct usb_device *dev = hid_to_usb_dev(hid);
	struct usb_interface *intf = to_usb_interface(hid->dev.parent);
	struct usb_host_interface *interface = intf->cur_altsetting;
	int ifnum = interface->desc.bInterfaceNumber;

	if (reqtype != HID_REQ_SET_IDLE)
		return -EINVAL;

	return hid_set_idle(dev, ifnum, report, idle);
}

// low level transport driver
static struct hid_ll_driver usb_hid_driver = {
	.parse = usbhid_parse,
	.start = usbhid_start,
	.stop = usbhid_stop,
	.raw_request = usbhid_raw_request,
	.output_report = usbhid_output_report,
	.idle = usbhid_idle,
};

__attribute__((weak))
void iteUsbHidConnectedCallback(
    struct hid_device *hid,
    uint32_t usbIdx)
{}

__attribute__((weak))
void iteUsbHidDescriptorReceivedCallback(
    struct hid_device *hid, 
    uint32_t type, 
    uint32_t total,
    uint32_t index,
    void* start, 
    int size)
{}

// TODO 完成此函式
static void* usbhid_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_host_interface *interface = intf->cur_altsetting;
	struct usb_device *dev = interface_to_usbdev(intf);
	struct usbhid_device *usbhid;
	struct hid_device *hid;
	unsigned int n, has_in = 0;
	size_t len;
	int ret;

	//"HID probe called for ifnum %d\n",
	//		intf->altsetting->desc.bInterfaceNumber);

	// 先判斷 interface descriptor 下面列的 endpoints
	for (n = 0; n < interface->desc.bNumEndpoints; n++)
		if (usb_endpoint_is_int_in(&interface->endpoint[n].desc))	// 如果是 interrupt input endpoint
			has_in++;
	if (!has_in) {	// 沒有 interrupt input endpoint
		hid_err(intf, "couldn't find an input interrupt endpoint\n");
		return NULL;
	}

	hid = hid_allocate_device();
	//ithPrintf("%s(%d) hid(%p)\n", __FUNCTION__, __LINE__, hid);
	if (IS_ERR(hid)) {
		//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);
		//return PTR_ERR(hid);
		return NULL;
	}

	usb_set_intfdata(intf, hid);
	hid->ll_driver = &usb_hid_driver;	// hid: hid device, 其 low level driver 是 usb_hid_driver

	hid->dev.parent = &intf->dev;
	hid->vendor = le16_to_cpu(dev->descriptor.idVendor);
	hid->product = le16_to_cpu(dev->descriptor.idProduct);
	hid->quirks |= HID_QUIRK_ALWAYS_POLL;
	if (intf->cur_altsetting->desc.bInterfaceProtocol ==
			USB_INTERFACE_PROTOCOL_MOUSE)	// 如果 interface desc 的 interface protocol 是 mouse
		hid->type = HID_TYPE_USBMOUSE;
	else if (intf->cur_altsetting->desc.bInterfaceProtocol == 0)
		hid->type = HID_TYPE_USBNONE;

	// 配置 usbhid_device
	usbhid = kzalloc(sizeof(*usbhid), GFP_KERNEL);
	if (usbhid == NULL) {
		ret = -ENOMEM;
		goto err;
	}

	hid->driver_data = usbhid;
	//ithPrintf("%s(%d) hid->driver_data(%p)\n", __FUNCTION__, __LINE__, hid->driver_data);
	usbhid->hid = hid;
	usbhid->intf = intf;
	usbhid->ifnum = interface->desc.bInterfaceNumber;

	ret = hid_add_device(hid);
	if (ret) {
		if (ret != -ENODEV)
			hid_err(intf, "can't add hid device: %d\n", ret);
		goto err_free;
	}

	
	// try to print intf
	{
        struct usb_hcd *hcd = bus_to_hcd(dev->bus);
        //ithPrintf("%s(%d) hcd idx : %x\n", __FUNCTION__, __LINE__, (hcd->index & ~USB_HAS_DEVICE_MODE));
		int i;
		iteUsbHidConnectedCallback(hid, hcd->index & ~USB_HAS_DEVICE_MODE);
		iteUsbHidDescriptorReceivedCallback(hid, USB_DT_DEVICE, 1, 0, (void*)(&dev->descriptor), dev->descriptor.bLength);
        iteUsbHidDescriptorReceivedCallback(hid, USB_DT_CONFIG, 1, 0, (void*)(&dev->config->desc), dev->config->desc.bLength);
		iteUsbHidDescriptorReceivedCallback(hid, USB_DT_INTERFACE, 1, 0, (void*)(&intf->cur_altsetting->desc), sizeof(intf->cur_altsetting->desc));
		iteUsbHidDescriptorReceivedCallback(hid, HID_DT_HID, 1, 0, (void*)(intf->cur_altsetting->extra), intf->cur_altsetting->extralen);

		for (i = 0; i < intf->cur_altsetting->desc.bNumEndpoints; i++)
		{
			struct usb_endpoint_descriptor *desc = &intf->cur_altsetting->endpoint[i].desc;
			iteUsbHidDescriptorReceivedCallback(hid, USB_DT_ENDPOINT, 
				intf->cur_altsetting->desc.bNumEndpoints, i, 
				(void*)(desc), desc->bLength);
		}

		iteUsbHidDescriptorReceivedCallback(hid, HID_DT_REPORT, 1, 0, (void*)(hid->dev_rdesc), hid->dev_rsize);
	}

	//ithPrintf("%s(%d) ret 0\n", __FUNCTION__, __LINE__);
	return hid;
err_free:
	kfree(usbhid);
err:
	hid_destroy_device(hid);
	return NULL;
}

__attribute__((weak))
void iteUsbHidDisconnectingCallback(
    struct hid_device *hid,
    uint32_t usbIdx)
{}

static void usbhid_disconnect(struct usb_interface *intf)
{
	struct hid_device *hid = usb_get_intfdata(intf);
	struct usbhid_device *usbhid;
    struct usb_device *dev = interface_to_usbdev(intf);

	if (WARN_ON(!hid))
		return;

    struct usb_hcd *hcd = bus_to_hcd(dev->bus);
    //ithPrintf("%s(%d) hcd idx : %x\n", __FUNCTION__, __LINE__, (hcd->index & ~USB_HAS_DEVICE_MODE));
	//ithPrintf("%s(%d)\n", __FUNCTION__, __LINE__);
	iteUsbHidDisconnectingCallback(hid, hcd->index & ~USB_HAS_DEVICE_MODE);
	usbhid = hid->driver_data;
	set_bit(HID_DISCONNECTED, &usbhid->iofl);
	hid_destroy_device(hid);
	kfree(usbhid);

	// send disconnect
}

// 判斷條件，只要interface class符合即可
// 比 usbkbd.c 和 usbmouse.c 更廣
static const struct usb_device_id hid_usb_ids[] = {
    { .match_flags = (USB_DEVICE_ID_MATCH_INT_CLASS),
            .bInterfaceClass = USB_INTERFACE_CLASS_HID
    },
	{ }						/* Terminating entry */
};

//MODULE_DEVICE_TABLE (usb, hid_usb_ids);

static struct usb_driver hid_driver = {
	.name =		"usbhid",
	.probe =	usbhid_probe,
	.disconnect =	usbhid_disconnect,
	.id_table =	hid_usb_ids,
};

static int __init hid_init(void)
{
	int retval = -ENOMEM;
	retval = usb_register(&hid_driver);
	if (retval)
		goto usb_register_fail;

	return 0;
usb_register_fail:
	return retval;
}

static void __exit hid_exit(void)
{
}

#if 0
module_init(hid_init);
module_exit(hid_exit);

MODULE_AUTHOR("Andreas Gal");
MODULE_AUTHOR("Vojtech Pavlik");
MODULE_AUTHOR("Jiri Kosina");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
#endif

int iteUsbHidRegister(void) {
	return hid_init();
}
