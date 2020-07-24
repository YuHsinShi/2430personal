﻿
/*-------------------------------------------------------------------------*/

/*
 * EHCI scheduled transaction support:  interrupt, iso, split iso
 * These are called "periodic" transactions in the EHCI spec.
 *
 * Note that for interrupt transfers, the QH/QTD manipulation is shared
 * with the "asynchronous" transaction support (control/bulk transfers).
 * The only real difference is in how interrupt transfers are scheduled.
 *
 * For ISO, we make an "iso_stream" head to serve the same role as a QH.
 * It keeps track of every ITD (or SITD) that's linked, and holds enough
 * pre-calculated schedule data to make appending to the queue be quick.
 */

#define udelay	ithDelay
static int ehci_get_frame (struct usb_hcd *hcd);

/*-------------------------------------------------------------------------*/

/*
 * periodic_next_shadow - return "next" pointer on shadow list
 * @periodic: host pointer to qh/itd/sitd
 * @tag: hardware tag for type of this record
 */
static union ehci_shadow *
periodic_next_shadow(struct ehci_hcd *ehci, union ehci_shadow *periodic,
		uint32_t tag)
{
	switch (hc32_to_cpu(ehci, tag)) {
    case Q_TYPE_QH:
		return &periodic->qh->qh_next;
    case Q_TYPE_FSTN:
        return &periodic->fstn->fstn_next;
    case Q_TYPE_ITD:
        return &periodic->itd->itd_next;
	// case Q_TYPE_SITD:
	default:
        return &periodic->sitd->sitd_next;
    }
}

/* caller must hold ehci->lock */
static void periodic_unlink (struct ehci_hcd *ehci, unsigned frame, void *ptr)
{
	union ehci_shadow	*prev_p = &ehci->pshadow[frame];
	uint32_t			*hw_p = &ehci->periodic[frame];
    union ehci_shadow	here = *prev_p;

    /* find predecessor of "ptr"; hw and shadow lists are in sync */
    while (here.ptr && here.ptr != ptr) {
		prev_p = periodic_next_shadow(ehci, prev_p,
				Q_NEXT_TYPE(ehci, *hw_p));
		hw_p = here.hw_next;
        here = *prev_p;
    }
    /* an interrupt entry (at list end) could have been shared */
	if (!here.ptr)
		return;

	/* update shadow and hardware lists ... the old "next" pointers
	 * from ptr may still be in use, the caller updates them.
     */
	*prev_p = *periodic_next_shadow(ehci, &here,
			Q_NEXT_TYPE(ehci, *hw_p));
	*hw_p = *here.hw_next;
}

/* how many of the uframe's 125 usecs are allocated? */
static unsigned short
periodic_usecs (struct ehci_hcd *ehci, unsigned frame, unsigned uframe)
{
	uint32_t			*hw_p = &ehci->periodic [frame];
	union ehci_shadow	*q = &ehci->pshadow [frame];
	unsigned		usecs = 0;

	while (q->ptr) {
		switch (hc32_to_cpu(ehci, Q_NEXT_TYPE(ehci, *hw_p))) {
        case Q_TYPE_QH:
            /* is it in the S-mask? */
			if (q->qh->hw_info2 & cpu_to_hc32(ehci, 1 << uframe))
                usecs += q->qh->usecs;
			/* ... or C-mask? */
			if (q->qh->hw_info2 & cpu_to_hc32(ehci,
					1 << (8 + uframe)))
				usecs += q->qh->c_usecs;
			hw_p = &q->qh->hw_next;
            q = &q->qh->qh_next;
            break;
#if 1 // enable by powei. 0 // TODO: iso
		// case Q_TYPE_FSTN:
		default:
            /* for "save place" FSTNs, count the relevant INTR
             * bandwidth from the previous frame
             */
			if (q->fstn->hw_prev != EHCI_LIST_END(ehci)) {
				ehci_dbg (ehci, "ignoring FSTN cost ...\n");
            }
			hw_p = &q->fstn->hw_next;
            q = &q->fstn->fstn_next;
            break;
        case Q_TYPE_ITD:
			if (q->itd->hw_transaction[uframe])
				usecs += q->itd->stream->usecs;
			hw_p = &q->itd->hw_next;
            q = &q->itd->itd_next;
            break;
        case Q_TYPE_SITD:
            /* is it in the S-mask?  (count SPLIT, DATA) */
			if (q->sitd->hw_uframe & cpu_to_hc32(ehci,
					1 << uframe)) {
				if (q->sitd->hw_fullspeed_ep &
						cpu_to_hc32(ehci, 1<<31))
					usecs += q->sitd->stream->usecs;
				else	/* worst case for OUT start-split */
					usecs += HS_USECS_ISO (188);
            }

            /* ... C-mask?  (count CSPLIT, DATA) */
            if (q->sitd->hw_uframe &
					cpu_to_hc32(ehci, 1 << (8 + uframe))) {
				/* worst case for IN complete-split */
				usecs += q->sitd->stream->c_usecs;
            }

			hw_p = &q->sitd->hw_next;
            q = &q->sitd->sitd_next;
            break;
#endif
        }
    }

	if (usecs > 100)
		ehci_err (ehci, "uframe %d sched overrun: %d usecs\n",
			frame * 8 + uframe, usecs);

    return usecs;
}

/*-------------------------------------------------------------------------*/

static int same_tt (struct usb_device *dev1, struct usb_device *dev2)
{
	if (!dev1->tt || !dev2->tt)
		return 0;
	if (dev1->tt != dev2->tt)
		return 0;
	if (dev1->tt->multi)
		return dev1->ttport == dev2->ttport;
	else
		return 1;
}

/* return true iff the device's transaction translator is available
 * for a periodic transfer starting at the specified frame, using
 * all the uframes in the mask.
 */
static int tt_no_collision (
	struct ehci_hcd		*ehci,
	unsigned		period,
	struct usb_device	*dev,
	unsigned		frame,
	u32			uf_mask
)
{
	if (period == 0)	/* error */
		return 0;

	/* note bandwidth wastage:  split never follows csplit
	 * (different dev or endpoint) until the next uframe.
	 * calling convention doesn't make that distinction.
	 */
	for (; frame < ehci->periodic_size; frame += period) {
		union ehci_shadow	here;
		uint32_t			type;

		here = ehci->pshadow [frame];
		type = Q_NEXT_TYPE(ehci, ehci->periodic [frame]);
		while (here.ptr) {
			switch (hc32_to_cpu(ehci, type)) {
			case Q_TYPE_ITD:
				type = Q_NEXT_TYPE(ehci, here.itd->hw_next);
				here = here.itd->itd_next;
				continue;
			case Q_TYPE_QH:
				if (same_tt (dev, here.qh->dev)) {
					u32		mask;

					mask = hc32_to_cpu(ehci,
							here.qh->hw_info2);
					/* "knows" no gap is needed */
					mask |= mask >> 8;
					if (mask & uf_mask)
						break;
				}
				type = Q_NEXT_TYPE(ehci, here.qh->hw_next);
				here = here.qh->qh_next;
				continue;
			case Q_TYPE_SITD:
				if (same_tt (dev, here.sitd->urb->dev)) {
					u16		mask;

					mask = hc32_to_cpu(ehci, here.sitd
								->hw_uframe);
					/* FIXME assumes no gap for IN! */
					mask |= mask >> 8;
					if (mask & uf_mask)
						break;
				}
				type = Q_NEXT_TYPE(ehci, here.sitd->hw_next);
				here = here.sitd->sitd_next;
				continue;
			// case Q_TYPE_FSTN:
			default:
				ehci_dbg (ehci,
					"periodic frame %d bogus type %d\n",
					frame, type);
			}

			/* collision or error */
			return 0;
		}
	}

	/* no collision */
	return 1;
}

/*-------------------------------------------------------------------------*/

static int enable_periodic (struct ehci_hcd *ehci)
{
	u32	cmd;
	int	status;

	if (ehci->periodic_sched++)
		return 0;

	/* did clearing PSE did take effect yet?
	 * takes effect only at frame boundaries...
	 */
	status = handshake_on_error_set_halt(ehci, ehci->regs.status,
					     STS_PSS, 0, 9 * 125);
	if (status)
		return status;

    ithWriteRegMaskA(ehci->regs.command, CMD_PSE, CMD_PSE);
	/* posted write ... PSS happens later */
	ehci_to_hcd(ehci)->state = HC_STATE_RUNNING;

	/* make sure ehci_work scans these */
	ehci->next_uframe = ithReadRegA(ehci->regs.frame_index)
		% (ehci->periodic_size << 3);
            return 0;
}

static int disable_periodic (struct ehci_hcd *ehci)
{
	u32	cmd;
	int	status;

	if (--ehci->periodic_sched)
            return 0;

	/* did setting PSE not take effect yet?
	 * takes effect only at frame boundaries...
	 */
	status = handshake_on_error_set_halt(ehci, ehci->regs.status,
					     STS_PSS, STS_PSS, 9 * 125);
	if (status)
		return status;

    ithWriteRegMaskA(ehci->regs.command, 0, CMD_PSE);
	/* posted write ... */

	ehci->next_uframe = -1;
	return 0;
}

/*-------------------------------------------------------------------------*/

/* periodic schedule slots have iso tds (normal or split) first, then a
 * sparse tree for active interrupt transfers.
 *
 * this just links in a qh; caller guarantees uframe masks are set right.
 * no FSTN support (yet; ehci 0.96+)
 */
static int qh_link_periodic (struct ehci_hcd *ehci, struct ehci_qh *qh)
{
	unsigned	i;
	unsigned	period = qh->period;

	dev_dbg (&qh->dev->dev,
		"link qh%d-%04x/%p start %d [%d/%d us]\n",
		period, hc32_to_cpup(ehci, &qh->hw_info2) & (QH_CMASK | QH_SMASK),
		qh, qh->start, qh->usecs, qh->c_usecs);

	/* high bandwidth, or otherwise every microframe */
	if (period == 0)
		period = 1;

	for (i = qh->start; i < ehci->periodic_size; i += period) {
		union ehci_shadow	*prev = &ehci->pshadow[i];
		uint32_t			*hw_p = &ehci->periodic[i];
		union ehci_shadow	here = *prev;
		uint32_t			type = 0;

		/* skip the iso nodes at list head */
		while (here.ptr) {
			type = Q_NEXT_TYPE(ehci, *hw_p);
			if (type == cpu_to_hc32(ehci, Q_TYPE_QH))
				break;
			prev = periodic_next_shadow(ehci, prev, type);
			hw_p = &here.qh->hw_next;
			here = *prev;
		}

		/* sorting each branch by period (slow-->fast)
		 * enables sharing interior tree nodes
		 */
		while (here.ptr && qh != here.qh) {
			if (qh->period > here.qh->period)
				break;
			prev = &here.qh->qh_next;
			hw_p = &here.qh->hw_next;
			here = *prev;
    }
		/* link in this qh, unless some earlier pass did that */
		if (qh != here.qh) {
			qh->qh_next = here;
			if (here.qh)
				qh->hw_next = *hw_p;
			wmb ();
			prev->qh = qh;
			*hw_p = QH_NEXT (ehci, qh->qh_dma);
		}
	}
	qh->qh_state = QH_STATE_LINKED;
	qh->xacterrs = 0;
	qh_get (qh);

	/* update per-qh bandwidth for usbfs */
	ehci_to_hcd(ehci)->self.bandwidth_allocated += qh->period
		? ((qh->usecs + qh->c_usecs) / qh->period)
		: (qh->usecs * 8);
    ithPrintf(" %s: bandwidth_allocate %d \n", __FUNCTION__, ehci_to_hcd(ehci)->self.bandwidth_allocated);

	/* maybe enable periodic schedule processing */
	return enable_periodic(ehci);
}

static int qh_unlink_periodic(struct ehci_hcd *ehci, struct ehci_qh *qh)
{
	unsigned	i;
	unsigned	period;

	_hcd_func_enter;

	// FIXME:
	// IF this isn't high speed
	//   and this qh is active in the current uframe
	//   (and overlay token SplitXstate is false?)
	// THEN
	//   qh->hw_info1 |= __constant_cpu_to_hc32(1 << 7 /* "ignore" */);

	/* high bandwidth, or otherwise part of every microframe */
	if ((period = qh->period) == 0)
		period = 1;

	for (i = qh->start; i < ehci->periodic_size; i += period)
		periodic_unlink (ehci, i, qh);

	/* update per-qh bandwidth for usbfs */
	ehci_to_hcd(ehci)->self.bandwidth_allocated -= qh->period
		? ((qh->usecs + qh->c_usecs) / qh->period)
		: (qh->usecs * 8);

	dev_dbg (&qh->dev->dev,
		"unlink qh%d-%04x/%p start %d [%d/%d us]\n",
		qh->period,
		hc32_to_cpup(ehci, &qh->hw_info2) & (QH_CMASK | QH_SMASK),
		qh, qh->start, qh->usecs, qh->c_usecs);

	/* qh->qh_next still "live" to HC */
	qh->qh_state = QH_STATE_UNLINK;
	qh->qh_next.ptr = NULL;
	qh_put (qh);

	/* maybe turn off periodic schedule */
	return disable_periodic(ehci);
}

static void intr_deschedule (struct ehci_hcd *ehci, struct ehci_qh *qh)
{
	unsigned	wait;

    _hcd_func_enter;

	qh_unlink_periodic (ehci, qh);

	/* simple/paranoid:  always delay, expecting the HC needs to read
	 * qh->hw_next or finish a writeback after SPLIT/CSPLIT ... and
	 * expect khubd to clean up after any CSPLITs we won't issue.
	 * active high speed queues may need bigger delays...
	 */
	if (list_empty (&qh->qtd_list)
			|| (cpu_to_hc32(ehci, QH_CMASK)
					& qh->hw_info2) != 0)
		wait = 2;
    else 
		wait = 55;	/* worst case: 3 * 1024 */

	udelay (wait);
	qh->qh_state = QH_STATE_IDLE;
	qh->hw_next = EHCI_LIST_END(ehci);
	wmb ();
}

/*-------------------------------------------------------------------------*/

static int check_period (
	struct ehci_hcd *ehci,
	unsigned	frame,
	unsigned	uframe,
	unsigned	period,
	unsigned	usecs
) {
	int		claimed;
    struct usb_hcd *hcd = ehci_to_hcd(ehci);

	/* complete split running into next frame?
	 * given FSTN support, we could sometimes check...
	 */
	if (uframe >= 8)
		return 0;

	/*
	 * 80% periodic == 100 usec/uframe available
	 * convert "usecs we need" to "max already claimed"
	 */
    usecs = 100 - usecs;

	/* we "know" 2 and 4 uframe intervals were rejected; so
	 * for period 0, check _every_ microframe in the schedule.
	 */
	if (unlikely (period == 0)) {
		do {
			for (uframe = 0; uframe < 7; uframe++) {
				claimed = periodic_usecs (ehci, frame, uframe);
				if (claimed > usecs)
					return 0;
			}
		} while ((frame += 1) < ehci->periodic_size);

	/* just check the specified uframe, at that period */
	} else {
		do {
			claimed = periodic_usecs (ehci, frame, uframe);
			if (claimed > usecs)
				return 0;
		} while ((frame += period) < ehci->periodic_size);
    }

    LOG_DEBUG " %s: frame %d, uframe %d, period %d, usecs %d ==> success!\n", __func__, frame, uframe, period, usecs LOG_END

	// success!
	return 1;
}

static int check_intr_schedule (
	struct ehci_hcd		*ehci,
	unsigned		frame,
	unsigned		uframe,
	const struct ehci_qh	*qh,
	uint32_t			*c_maskp
)
{
	int		retval = -ENOSPC;
	u8		mask = 0;

    LOG_DEBUG " %s: frame %d, uframe %d, qh.c_usecs %d \n", __func__, frame, uframe, qh->c_usecs LOG_END

	if (qh->c_usecs && uframe >= 6)		/* FSTN territory? */
		goto done;

	if (!check_period (ehci, frame, uframe, qh->period, qh->usecs))
		goto done;
	if (!qh->c_usecs) {
		retval = 0;
		*c_maskp = 0;
		goto done;
	}

	/* Make sure this tt's buffer is also available for CSPLITs.
	 * We pessimize a bit; probably the typical full speed case
	 * doesn't need the second CSPLIT.
             *
	 * NOTE:  both SPLIT and CSPLIT could be checked in just
	 * one smart pass...
             */
	mask = 0x03 << (uframe + qh->gap_uf);
	*c_maskp = cpu_to_hc32(ehci, mask << 8);

	mask |= 1 << uframe;
	if (tt_no_collision (ehci, qh->period, qh->dev, frame, mask)) {
		if (!check_period (ehci, frame, uframe + qh->gap_uf + 1,
					qh->period, qh->c_usecs))
			goto done;
		if (!check_period (ehci, frame, uframe + qh->gap_uf,
					qh->period, qh->c_usecs))
			goto done;
		retval = 0;
    }

done:
	return retval;
}

/* "first fit" scheduling policy used the first time through,
 * or when the previous schedule slot can't be re-used.
 */
static int qh_schedule(struct ehci_hcd *ehci, struct ehci_qh *qh)
{
	int		status;
	unsigned	uframe;
	uint32_t		c_mask;
	unsigned	frame;		/* 0..(qh->period - 1), or NO_FRAME */

    _hcd_func_enter;

	qh_refresh(ehci, qh);
	qh->hw_next = EHCI_LIST_END(ehci);
	frame = qh->start;

	/* reuse the previous schedule slots, if we can */
	if (frame < qh->period) {
		uframe = ffs(hc32_to_cpup(ehci, &qh->hw_info2) & QH_SMASK);
		status = check_intr_schedule (ehci, frame, --uframe,
				qh, &c_mask);
	} else {
		uframe = 0;
		c_mask = 0;
		status = -ENOSPC;
	}

	/* else scan the schedule to find a group of slots such that all
	 * uframes have enough periodic bandwidth available.
	 */
	if (status) {
		/* "normal" case, uframing flexible except with splits */
		if (qh->period) {
			frame = qh->period - 1;
			do {
				for (uframe = 0; uframe < 8; uframe++) {
					status = check_intr_schedule (ehci,
							frame, uframe, qh,
							&c_mask);
					if (status == 0)
                        break;
                }
			} while (status && frame--);

		/* qh->period == 0 means every uframe */
		} else {
			frame = 0;
			status = check_intr_schedule (ehci, 0, 0, qh, &c_mask);
		}
		if (status)
			goto done;
		qh->start = frame;

		/* reset S-frame and (maybe) C-frame masks */
		qh->hw_info2 &= cpu_to_hc32(ehci, ~(QH_CMASK | QH_SMASK));
		qh->hw_info2 |= qh->period
			? cpu_to_hc32(ehci, 1 << uframe)
			: cpu_to_hc32(ehci, QH_SMASK);
		qh->hw_info2 |= c_mask;
	} else
		ehci_dbg (ehci, "reused qh %p schedule\n", qh);

	/* stuff into the periodic schedule */
	status = qh_link_periodic (ehci, qh);
done:
    check_result(status);
    _hcd_func_leave;
	return status;
}

static int intr_submit (
	struct ehci_hcd		*ehci,
	struct urb		*urb,
	struct list_head	*qtd_list,
	gfp_t			mem_flags
) {
	unsigned		epnum;
	unsigned long		flags;
	struct ehci_qh	*qh;
	int			    status;
	struct list_head	empty;

	/* get endpoint and transfer/schedule data */
	epnum = urb->ep->desc.bEndpointAddress;

	spin_lock_irqsave (&ehci->lock, flags);

    LOG_INFO " bus %d: intr_submit urb %p len %d ep %d-%s [qh %p] \n",
        ehci_to_hcd(ehci)->index, urb, urb->transfer_buffer_length,
        epnum & 0x0f, (epnum & USB_DIR_IN) ? "in" : "out",
        urb->ep->hcpriv  LOG_END

	status = usb_hcd_link_urb_to_ep(ehci_to_hcd(ehci), urb);
	if (unlikely(status))
		goto done_not_linked;

	/* get qh and force any scheduling errors */
	INIT_LIST_HEAD (&empty);
	qh = qh_append_tds(ehci, urb, &empty, epnum, &urb->ep->hcpriv);
	if (qh == NULL) {
		status = -ENOMEM;
		goto done;
	}
	if (qh->qh_state == QH_STATE_IDLE) {
		if ((status = qh_schedule (ehci, qh)) != 0)
			goto done;
	}

	/* then queue the urb's tds to the qh */
	qh = qh_append_tds(ehci, urb, qtd_list, epnum, &urb->ep->hcpriv);
	BUG_ON (qh == NULL);

	/* ... update usbfs periodic stats */
	ehci_to_hcd(ehci)->self.bandwidth_int_reqs++;
    
done:
	if (unlikely(status))
		usb_hcd_unlink_urb_from_ep(ehci_to_hcd(ehci), urb);
done_not_linked:
	spin_unlock_irqrestore (&ehci->lock, flags);
	if (status)
		qtd_list_free (ehci, urb, qtd_list);

	return status;
}

/*-------------------------------------------------------------------------*/
#if 1 // enable by powei. 0

/* ehci_iso_stream ops work with both ITD and SITD */

static struct ehci_iso_stream *
iso_stream_alloc (gfp_t mem_flags)
{
	struct ehci_iso_stream *stream;

	stream = kzalloc(sizeof *stream, mem_flags);
	if (likely (stream != NULL)) {
		INIT_LIST_HEAD(&stream->td_list);
		INIT_LIST_HEAD(&stream->free_list);
		stream->next_uframe = -1;
		stream->refcount = 1;
	}
	return stream;
}

static void
iso_stream_init (
	struct ehci_hcd		*ehci,
	struct ehci_iso_stream	*stream,
	struct usb_device	*dev,
	int			pipe,
	unsigned		interval
)
{
	static const u8 smask_out [] = { 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f };

	u32			buf1;
	unsigned		epnum, maxp;
	int			is_input;
	long			bandwidth;
        struct usb_hcd *hcd = ehci_to_hcd(ehci);

	/*
	 * this might be a "high bandwidth" highspeed endpoint,
	 * as encoded in the ep descriptor's wMaxPacket field
	 */
	epnum = usb_pipeendpoint (pipe);
	is_input = usb_pipein (pipe) ? USB_DIR_IN : 0;
	maxp = usb_maxpacket(dev, pipe, !is_input);
	if (is_input) {
		buf1 = (1 << 11);
	} else {
		buf1 = 0;
	}

	/* knows about ITD vs SITD */
	if (dev->speed == USB_SPEED_HIGH) {
		unsigned multi = hb_mult(maxp);

		stream->highspeed = 1;

		maxp = max_packet(maxp);
		buf1 |= maxp;
		maxp *= multi;

		stream->buf0 = cpu_to_hc32(ehci, (epnum << 8) | dev->devnum);
		stream->buf1 = cpu_to_hc32(ehci, buf1);
		stream->buf2 = cpu_to_hc32(ehci, multi);

		/* usbfs wants to report the average usecs per frame tied up
		 * when transfers on this endpoint are scheduled ...
		 */
		stream->usecs = HS_USECS_ISO (maxp);
		bandwidth = stream->usecs * 8;
		bandwidth /= interval;

	} else {
		u32		addr;
		int		think_time;
		int		hs_transfers;

		addr = dev->ttport << 24;
#if 0
		if (!ehci_is_TDI(ehci)
				|| (dev->tt->hub !=
					ehci_to_hcd(ehci)->self.root_hub))
			addr |= dev->tt->hub->devnum << 16;
#endif
		addr |= epnum << 8;
		addr |= dev->devnum;
		stream->usecs = HS_USECS_ISO (maxp);
		think_time = 0; //dev->tt ? dev->tt->think_time : 0;
		stream->tt_usecs = NS_TO_US (think_time + usb_calc_bus_time (
				dev->speed, is_input, 1, maxp));
		hs_transfers = max (1u, (maxp + 187) / 188);
		if (is_input) {
			u32	tmp;

			addr |= 1 << 31;
			stream->c_usecs = stream->usecs;
			stream->usecs = HS_USECS_ISO (1);
			stream->raw_mask = 1;

			/* c-mask as specified in USB 2.0 11.18.4 3.c */
			tmp = (1 << (hs_transfers + 2)) - 1;
			stream->raw_mask |= tmp << (8 + 2);
		} else
			stream->raw_mask = smask_out [hs_transfers - 1];
		bandwidth = stream->usecs + stream->c_usecs;
		bandwidth /= interval << 3;

		/* stream->splits gets created from raw_mask later */
		stream->address = cpu_to_hc32(ehci, addr);
	}
	
	stream->bandwidth = bandwidth;

	stream->udev = dev;

	stream->bEndpointAddress = is_input | epnum;
	stream->interval = interval;
	stream->maxp = maxp;
}

static void
iso_stream_put(struct ehci_hcd *ehci, struct ehci_iso_stream *stream)
{
	stream->refcount--;

	/* free whenever just a dev->ep reference remains.
	 * not like a QH -- no persistent state (toggle, halt)
	 */
	if (stream->refcount == 1) {
		int		is_in;

		// BUG_ON (!list_empty(&stream->td_list));

		while (!list_empty (&stream->free_list)) {
			struct list_head	*entry;

			entry = stream->free_list.next;
			list_del (entry);

			/* knows about ITD vs SITD */
			if (stream->highspeed) {
				struct ehci_itd		*itd;

				itd = list_entry (entry, struct ehci_itd,
						itd_list);
				ehci_itd_free (ehci, itd);
			} else {
				struct ehci_sitd	*sitd;

				sitd = list_entry (entry, struct ehci_sitd,
						sitd_list);
				ehci_sitd_free (ehci, sitd);
			}
		}

		is_in = (stream->bEndpointAddress & USB_DIR_IN) ? 0x10 : 0;
		stream->bEndpointAddress &= 0x0f;
		stream->ep->hcpriv = NULL;

		if (stream->rescheduled) {
			ehci_dbg (ehci, "ep%d%s-iso rescheduled "
				"%lu times in %lu seconds\n",
				stream->bEndpointAddress, is_in ? "in" : "out",
				stream->rescheduled,
				((jiffies - stream->start)/HZ)
				);
		}

		kfree(stream);
	}
}

static inline struct ehci_iso_stream *
iso_stream_get (struct ehci_iso_stream *stream)
{
	if (likely (stream != NULL))
		stream->refcount++;
	return stream;
}

static struct ehci_iso_stream *
iso_stream_find (struct ehci_hcd *ehci, struct urb *urb)
{
	unsigned		epnum;
	struct ehci_iso_stream	*stream;
	struct usb_host_endpoint *ep;
	unsigned long		flags;

	epnum = usb_pipeendpoint (urb->pipe);
	if (usb_pipein(urb->pipe))
		ep = urb->dev->ep_in[epnum];
	else
		ep = urb->dev->ep_out[epnum];

	spin_lock_irqsave (&ehci->lock, flags);
	stream = ep->hcpriv;

	if (unlikely (stream == NULL)) {
		stream = iso_stream_alloc(GFP_ATOMIC);
		if (likely (stream != NULL)) {
			/* dev->ep owns the initial refcount */
			ep->hcpriv = stream;
			stream->ep = ep;		
			iso_stream_init(ehci, stream, urb->dev, urb->pipe,
					urb->interval);
		}

	/* if dev->ep [epnum] is a QH, info1.maxpacket is nonzero */
	} else if (unlikely (stream->hw_info1 != 0)) {
		ehci_dbg (ehci, "dev ep%d%s, not iso??\n",
			epnum,
			usb_pipein(urb->pipe) ? "in" : "out");
		stream = NULL;
	}

	/* caller guarantees an eventual matching iso_stream_put */
	stream = iso_stream_get (stream);

	spin_unlock_irqrestore (&ehci->lock, flags);
	return stream;
}

/*-------------------------------------------------------------------------*/

/* ehci_iso_sched ops can be ITD-only or SITD-only */

static struct ehci_iso_sched *
iso_sched_alloc (unsigned packets, gfp_t mem_flags)
{
	struct ehci_iso_sched	*iso_sched;
	int			size = sizeof *iso_sched;

	size += packets * sizeof (struct ehci_iso_packet);
	iso_sched = kzalloc(size, mem_flags);
	if (likely (iso_sched != NULL)) {
		INIT_LIST_HEAD (&iso_sched->td_list);
	}
	return iso_sched;
}

static inline void
itd_sched_init(
	struct ehci_hcd		*ehci,
	struct ehci_iso_sched	*iso_sched,
	struct ehci_iso_stream	*stream,
	struct urb		*urb
)
{
	unsigned	i;
	dma_addr_t	dma = urb->transfer_dma;

	/* how many uframes are needed for these transfers */
	iso_sched->span = urb->number_of_packets * stream->interval;

	/* figure out per-uframe itd fields that we'll need later
	 * when we fit new itds into the schedule.
	 */
	for (i = 0; i < urb->number_of_packets; i++) {
		struct ehci_iso_packet	*uframe = &iso_sched->packet [i];
		unsigned		length;
		dma_addr_t		buf;
		u32			trans;

		length = urb->iso_frame_desc [i].length;
		buf = dma + urb->iso_frame_desc [i].offset;

		trans = EHCI_ISOC_ACTIVE;
		trans |= buf & 0x0fff;
		if (unlikely (((i + 1) == urb->number_of_packets))
				&& !(urb->transfer_flags & URB_NO_INTERRUPT))
			trans |= EHCI_ITD_IOC;
		trans |= length << 16;
		uframe->transaction = cpu_to_hc32(ehci, trans);

		/* might need to cross a buffer page within a uframe */
		uframe->bufp = (buf & ~(uint64_t)0x0fff);
		buf += length;
		if (unlikely ((uframe->bufp != (buf & ~(uint64_t)0x0fff))))
			uframe->cross = 1;
	}
}

static void
iso_sched_free (
	struct ehci_iso_stream	*stream,
	struct ehci_iso_sched	*iso_sched
)
{
	if (!iso_sched)
		return;
	// caller must hold ehci->lock!
	list_splice (&iso_sched->td_list, &stream->free_list);
	kfree (iso_sched);
}

static int
itd_urb_transaction (
	struct ehci_iso_stream	*stream,
	struct ehci_hcd		*ehci,
	struct urb		*urb,
	gfp_t			mem_flags
)
{
	struct ehci_itd		*itd;
	dma_addr_t		itd_dma;
	int			i;
	unsigned		num_itds;
	struct ehci_iso_sched	*sched;
	unsigned long		flags;

	sched = iso_sched_alloc (urb->number_of_packets, mem_flags);
	if (unlikely (sched == NULL))
		return -ENOMEM;



	itd_sched_init(ehci, sched, stream, urb);

	if (urb->interval < 8)
		num_itds = 1 + (sched->span + 7) / 8;
	else
		num_itds = urb->number_of_packets;

	/* allocate/init ITDs */
	spin_lock_irqsave (&ehci->lock, flags);
	for (i = 0; i < num_itds; i++) {
		uint32_t buf_index;
		/* free_list.next might be cache-hot ... but maybe
		 * the HC caches it too. avoid that issue for now.
		 */

		/* prefer previously-allocated itds */
		if (likely (!list_empty(&stream->free_list))) {
			itd = list_entry (stream->free_list.prev,
					struct ehci_itd, itd_list);
			list_del (&itd->itd_list);
			itd_dma = itd->itd_dma;
			buf_index = itd->buf_index;
		} else {	
			spin_unlock_irqrestore (&ehci->lock, flags);
			itd = ehci_itd_alloc (ehci, mem_flags,
					&itd_dma);
			buf_index = itd->buf_index;
			spin_lock_irqsave (&ehci->lock, flags);			
			if (!itd) {			
				iso_sched_free(stream, sched);
				spin_unlock_irqrestore(&ehci->lock, flags);					
				return -ENOMEM;
			}
		}

		memset (itd, 0, sizeof *itd);
		itd->itd_dma = itd_dma;
		itd->buf_index = buf_index;
		list_add (&itd->itd_list, &sched->td_list);
	}
	spin_unlock_irqrestore (&ehci->lock, flags);

	/* temporarily store schedule info in hcpriv */
	urb->hcpriv = sched;
	urb->error_count = 0;
	return 0;
}

/*-------------------------------------------------------------------------*/

static inline int
itd_slot_ok (
	struct ehci_hcd		*ehci,
	u32			mod,
	u32			uframe,
	u8			usecs,
	u32			period
)
{
	uframe %= period;
	do {
		/* can't commit more than 80% periodic == 100 usec */
		if (periodic_usecs (ehci, uframe >> 3, uframe & 0x7)
				> (100 - usecs))
			return 0;

		/* we know urb->interval is 2^N uframes */
		uframe += period;
	} while (uframe < mod);
	return 1;
}

static inline int
sitd_slot_ok (
	struct ehci_hcd		*ehci,
	u32			mod,
	struct ehci_iso_stream	*stream,
	u32			uframe,
	struct ehci_iso_sched	*sched,
	u32			period_uframes
)
{
	u32			mask, tmp;
	u32			frame, uf;
                struct usb_hcd *hcd = ehci_to_hcd(ehci);

	mask = stream->raw_mask << (uframe & 7);

	/* for IN, don't wrap CSPLIT into the next frame */
	if (mask & ~0xffff)
		return 0;

	/* this multi-pass logic is simple, but performance may
	 * suffer when the schedule data isn't cached.
	 */

	/* check bandwidth */
	uframe %= period_uframes;
	do {
		u32		max_used;

		frame = uframe >> 3;
		uf = uframe & 7;

#ifdef CONFIG_USB_EHCI_TT_NEWSCHED
		/* The tt's fullspeed bus bandwidth must be available.
		 * tt_available scheduling guarantees 10+% for control/bulk.
		 */
		if (!tt_available (ehci, period_uframes << 3,
				stream->udev, frame, uf, stream->tt_usecs))
			return 0;
#else
		/* tt must be idle for start(s), any gap, and csplit.
		 * assume scheduling slop leaves 10+% for control/bulk.
		 */
		if (!tt_no_collision (ehci, period_uframes << 3,
				stream->udev, frame, mask))
			return 0;
#endif

		/* check starts (OUT uses more than one) */
		max_used = 100 - stream->usecs;
		for (tmp = stream->raw_mask & 0xff; tmp; tmp >>= 1, uf++) {
			if (periodic_usecs (ehci, frame, uf) > max_used)
				return 0;
		}

		/* for IN, check CSPLIT */
		if (stream->c_usecs) {
			uf = uframe & 7;
			max_used = 100 - stream->c_usecs;
			do {
				tmp = 1 << uf;
				tmp <<= 8;
				if ((stream->raw_mask & tmp) == 0)
					continue;
				if (periodic_usecs (ehci, frame, uf)
						> max_used)
					return 0;
			} while (++uf < 8);
		}

		/* we know urb->interval is 2^N uframes */
		uframe += period_uframes;
	} while (uframe < mod);

	stream->splits = cpu_to_hc32(ehci, stream->raw_mask << (uframe & 7));
	
	return 1;
}

/*
 * This scheduler plans almost as far into the future as it has actual
 * periodic schedule slots.  (Affected by TUNE_FLS, which defaults to
 * "as small as possible" to be cache-friendlier.)  That limits the size
 * transfers you can stream reliably; avoid more than 64 msec per urb.
 * Also avoid queue depths of less than ehci's worst irq latency (affected
 * by the per-urb URB_NO_INTERRUPT hint, the log2_irq_thresh module parameter,
 * and other factors); or more than about 230 msec total (for portability,
 * given EHCI_TUNE_FLS and the slop).  Or, write a smarter scheduler!
 */

#define SCHEDULE_SLOP	10	/* frames */
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

static int
iso_stream_schedule (
	struct ehci_hcd		*ehci,
	struct urb		*urb,
	struct ehci_iso_stream	*stream
)
{
	u32			now, start, max, period;
	int			status;
	unsigned		mod = ehci->periodic_size << 3;
	struct ehci_iso_sched	*sched = urb->hcpriv;

	if (sched->span > (mod - 8 * SCHEDULE_SLOP)) {
		ehci_dbg (ehci, "iso request %p too long\n", urb);
		status = -EFBIG;
		goto fail;
	}

	if ((stream->depth + sched->span) > mod) {
		ehci_dbg (ehci, "request %p would overflow (%d+%d>%d)\n",
			urb, stream->depth, sched->span, mod);
		status = -EFBIG;
		goto fail;
	}

	now = ithReadRegA(ehci->regs.frame_index) % mod;

	/* when's the last uframe this urb could start? */
	max = now + mod;

	/* Typical case: reuse current schedule, stream is still active.
	 * Hopefully there are no gaps from the host falling behind
	 * (irq delays etc), but if there are we'll take the next
	 * slot in the schedule, implicitly assuming URB_ISO_ASAP.
	 */
	if (likely (!list_empty (&stream->td_list))) {
		start = stream->next_uframe;
		if (start < now)
			start += mod;

		/* Fell behind (by up to twice the slop amount)? */
		if (start >= max - 2 * 8 * SCHEDULE_SLOP)
			start += stream->interval * DIV_ROUND_UP(
					max - start, stream->interval) - mod;

		/* Tried to schedule too far into the future? */
		if (unlikely((start + sched->span) >= max)) {
			status = -EFBIG;
			goto fail;
		}
		goto ready;
	}

	/* need to schedule; when's the next (u)frame we could start?
	 * this is bigger than ehci->i_thresh allows; scheduling itself
	 * isn't free, the slop should handle reasonably slow cpus.  it
	 * can also help high bandwidth if the dma and irq loads don't
	 * jump until after the queue is primed.
	 */
	start = SCHEDULE_SLOP * 8 + (now & ~0x07);
	start %= mod;
	stream->next_uframe = start;

	/* NOTE:  assumes URB_ISO_ASAP, to limit complexity/bugs */

	period = urb->interval;
	if (!stream->highspeed)
		period <<= 3;

	/* find a uframe slot with enough bandwidth */
	for (; start < (stream->next_uframe + period); start++) {
		int		enough_space;

		/* check schedule: enough space? */
		if (stream->highspeed)
			enough_space = itd_slot_ok (ehci, mod, start,
					stream->usecs, period);
		else {
			if ((start % 8) >= 6)
				continue;
			enough_space = sitd_slot_ok (ehci, mod, stream,
					start, sched, period);
		}

		/* schedule it here if there's enough bandwidth */
		if (enough_space) {
			stream->next_uframe = start % mod;
			goto ready;
		}
	}

	/* no room in the schedule */
	ehci_dbg (ehci, "iso %ssched full %p (now %d max %d)\n",
		list_empty (&stream->td_list) ? "" : "re",
		urb, now, max);
	status = -ENOSPC;

fail:
	iso_sched_free (stream, sched);
	urb->hcpriv = NULL;
	return status;

ready:
	/* report high speed start in uframes; full speed, in frames */
	urb->start_frame = stream->next_uframe;
	if (!stream->highspeed)
		urb->start_frame >>= 3;
	return 0;
}

/*-------------------------------------------------------------------------*/

static inline void
itd_init(struct ehci_hcd *ehci, struct ehci_iso_stream *stream,
		struct ehci_itd *itd)
{
	int i;

	/* it's been recently zeroed */
	itd->hw_next = EHCI_LIST_END(ehci);
	itd->hw_bufp [0] = stream->buf0;
	itd->hw_bufp [1] = stream->buf1;
	itd->hw_bufp [2] = stream->buf2;

	for (i = 0; i < 8; i++)
		itd->index[i] = -1;

	/* All other fields are filled when scheduling */
}

static inline void
itd_patch(
	struct ehci_hcd		*ehci,
	struct ehci_itd		*itd,
	struct ehci_iso_sched	*iso_sched,
	unsigned		index,
	u16			uframe
)
{
	struct ehci_iso_packet	*uf = &iso_sched->packet [index];
	unsigned		pg = itd->pg;

	// BUG_ON (pg == 6 && uf->cross);

	uframe &= 0x07;
	itd->index [uframe] = index;

	itd->hw_transaction[uframe] = uf->transaction;
	itd->hw_transaction[uframe] |= cpu_to_hc32(ehci, pg << 12);
	itd->hw_bufp[pg] |= cpu_to_hc32(ehci, uf->bufp & ~(u32)0);
	itd->hw_bufp_hi[pg] |= cpu_to_hc32(ehci, (u32)(uf->bufp >> 32));

	/* iso_frame_desc[].offset must be strictly increasing */
	if (unlikely (uf->cross)) {
		uint64_t	bufp = uf->bufp + 4096;

		itd->pg = ++pg;
		itd->hw_bufp[pg] |= cpu_to_hc32(ehci, bufp & ~(u32)0);
		itd->hw_bufp_hi[pg] |= cpu_to_hc32(ehci, (u32)(bufp >> 32));
	}

}

static inline void
itd_link (struct ehci_hcd *ehci, unsigned frame, struct ehci_itd *itd)
{
	/* always prepend ITD/SITD ... only QH tree is order-sensitive */
	itd->itd_next = ehci->pshadow [frame];
	itd->hw_next = ehci->periodic [frame];
	ehci->pshadow [frame].itd = itd;
	itd->frame = frame;
	wmb ();
	ehci->periodic[frame] = cpu_to_hc32(ehci, itd->itd_dma | Q_TYPE_ITD);
}

/* fit urb's itds into the selected schedule slot; activate as needed */
static int
itd_link_urb (
	struct ehci_hcd		*ehci,
	struct urb		*urb,
	unsigned		mod,
	struct ehci_iso_stream	*stream
)
{
	int			packet;
	unsigned		next_uframe, uframe, frame;
	struct ehci_iso_sched	*iso_sched = urb->hcpriv;
	struct ehci_itd		*itd;

	next_uframe = stream->next_uframe % mod;
	if (unlikely (list_empty(&stream->td_list))) {
		ehci_to_hcd(ehci)->self.bandwidth_allocated
				+= stream->bandwidth;
		ehci_vdbg (ehci,
			"schedule ep%d%s-iso period %d start %d.%d\n",
			stream->bEndpointAddress & 0x0f,
			(stream->bEndpointAddress & USB_DIR_IN) ? "in" : "out",
			urb->interval,
			next_uframe >> 3, next_uframe & 0x7);
		stream->start = jiffies;
	}
	ehci_to_hcd(ehci)->self.bandwidth_isoc_reqs++;

	/* fill iTDs uframe by uframe */
	for (packet = 0, itd = NULL; packet < urb->number_of_packets; ) {
		if (itd == NULL) {
			/* ASSERT:  we have all necessary itds */
			// BUG_ON (list_empty (&iso_sched->td_list));

			/* ASSERT:  no itds for this endpoint in this uframe */

			itd = list_entry (iso_sched->td_list.next,
					struct ehci_itd, itd_list);
			list_move_tail (&itd->itd_list, &stream->td_list);
			itd->stream = iso_stream_get (stream);
			itd->urb = usb_get_urb (urb);
			itd_init (ehci, stream, itd);
		}

		uframe = next_uframe & 0x07;
		frame = next_uframe >> 3;

		itd_patch(ehci, itd, iso_sched, packet, uframe);

		next_uframe += stream->interval;
		stream->depth += stream->interval;
		next_uframe %= mod;
		packet++;

		/* link completed itds into the schedule */
		if (((next_uframe >> 3) != frame)
				|| packet == urb->number_of_packets) {
			itd_link (ehci, frame % ehci->periodic_size, itd);			
			itd = NULL;
		}
	}
	stream->next_uframe = next_uframe;

	/* don't need that schedule data any more */
	iso_sched_free (stream, iso_sched);
	urb->hcpriv = NULL;
#if 0
	timer_action (ehci, TIMER_IO_WATCHDOG);
#endif

	return enable_periodic(ehci);
}

#define	ISO_ERRS (EHCI_ISOC_BUF_ERR | EHCI_ISOC_BABBLE | EHCI_ISOC_XACTERR)

/* Process and recycle a completed ITD.  Return true iff its urb completed,
 * and hence its completion callback probably added things to the hardware
 * schedule.
 *
 * Note that we carefully avoid recycling this descriptor until after any
 * completion callback runs, so that it won't be reused quickly.  That is,
 * assuming (a) no more than two urbs per frame on this endpoint, and also
 * (b) only this endpoint's completions submit URBs.  It seems some silicon
 * corrupts things if you reuse completed descriptors very quickly...
 */
static unsigned
itd_complete (
	struct ehci_hcd	*ehci,
	struct ehci_itd	*itd
) {
	struct urb				*urb = itd->urb;
	struct usb_iso_packet_descriptor	*desc;
	u32					t;
	unsigned				uframe;
	int					urb_index = -1;
	struct ehci_iso_stream			*stream = itd->stream;
	struct usb_device			*dev;
	unsigned				retval = false;
	u32 itdDir = 0;
    struct usb_hcd *hcd = ehci_to_hcd(ehci);

	ehci_invalid_itd(itd);

	/* for each uframe with a packet */
	for (uframe = 0; uframe < 8; uframe++) {
		itdDir = hc32_to_cpup(ehci, &itd->hw_bufp[1])&(1<<11);

		if (likely (itd->index[uframe] == -1))
			continue;
		urb_index = itd->index[uframe];

		desc = &urb->iso_frame_desc [urb_index];

		t = hc32_to_cpup(ehci, &itd->hw_transaction [uframe]);
		itd->hw_transaction [uframe] = 0;
		stream->depth -= stream->interval;

		/* report transfer status */
		if (unlikely (t & ISO_ERRS)) {				
			urb->error_count++;
			if (t & EHCI_ISOC_BUF_ERR)
				desc->status = usb_pipein (urb->pipe)
					? -ENOSR  /* hc couldn't read */
					: -ECOMM; /* hc couldn't write */
			else if (t & EHCI_ISOC_BABBLE)
				desc->status = -EOVERFLOW;
			else /* (t & EHCI_ISOC_XACTERR) */
				desc->status = -EPROTO;

			/* HC need not update length with this error */
			if (!(t & EHCI_ISOC_BABBLE)) {
		 	    //Faraday EHCI HC decrement the counter of the transaction length for
			    //itd ISO IN (It is differ with EHCI Spec)
      			//So the length means how many remaining bytes have not been sent
		        //by host for ISO IN.				
				if ( itdDir )
					desc->actual_length = desc->length - EHCI_ITD_LENGTH (t);
				else
					desc->actual_length = EHCI_ITD_LENGTH (t);
			}
		} else if (likely ((t & EHCI_ISOC_ACTIVE) == 0)) {	
		    desc->status = 0;		
			//Faraday EHCI HC decrement the counter of the transaction length for
			//itd ISO IN (It is differ with EHCI Spec)
			//So the length means how many remaining bytes have not been sent
			//by host for ISO IN.				
			if ( itdDir )
				desc->actual_length = desc->length - EHCI_ITD_LENGTH (t);
			else
				desc->actual_length = EHCI_ITD_LENGTH (t);	
		} else {			
			/* URB was too late */
			desc->status = -EXDEV;
		}
	}

	/* handle completion now? */
	if (likely ((urb_index + 1) != urb->number_of_packets))
		goto done;

	/* ASSERT: it's really the last itd for this urb
	list_for_each_entry (itd, &stream->td_list, itd_list)
		BUG_ON (itd->urb == urb);
	 */

	/* give urb back to the driver; completion often (re)submits */
	dev = urb->dev;
	ehci_urb_done(ehci, urb, 0);
	retval = true;
	urb = NULL;
	(void) disable_periodic(ehci);
	ehci_to_hcd(ehci)->self.bandwidth_isoc_reqs--;

	if (unlikely (list_empty (&stream->td_list))) {
		ehci_to_hcd(ehci)->self.bandwidth_allocated
				-= stream->bandwidth;
		ehci_vdbg (ehci,
			"deschedule ep%d%s-iso\n",
			stream->bEndpointAddress & 0x0f,
			(stream->bEndpointAddress & USB_DIR_IN) ? "in" : "out");
	}
	iso_stream_put (ehci, stream);
	/* OK to recycle this ITD now that its completion callback ran. */
done:
	usb_put_urb(urb);
	itd->urb = NULL;
	itd->stream = NULL;
	list_move(&itd->itd_list, &stream->free_list);
	iso_stream_put(ehci, stream);

	return retval;
}

/*-------------------------------------------------------------------------*/

static int itd_submit (struct ehci_hcd *ehci, struct urb *urb,
	gfp_t mem_flags)
{
	int			status = -EINVAL;
	unsigned long		flags;
	struct ehci_iso_stream	*stream;
	struct usb_hcd *hcd = ehci_to_hcd(ehci);

	/* Get iso_stream head */
	stream = iso_stream_find (ehci, urb);
	if (unlikely (stream == NULL)) {
		ehci_dbg (ehci, "can't get iso stream\n");
		return -ENOMEM;
	}

	if (unlikely (urb->interval != stream->interval)) {
		ehci_dbg (ehci, "can't change iso interval %d --> %d\n",
		stream->interval, urb->interval);
		goto done;
	}


#ifdef EHCI_URB_TRACE
	ehci_dbg (ehci,
		"%s urb %p ep%d%s len %d, %d pkts %d uframes [%p]\n",
		__func__, urb,
		usb_pipeendpoint (urb->pipe),
		usb_pipein (urb->pipe) ? "in" : "out",
		urb->transfer_buffer_length,
		urb->number_of_packets, urb->interval,
		stream);
#endif

	/* allocate ITDs w/o locking anything */
	status = itd_urb_transaction (stream, ehci, urb, mem_flags);

	if (unlikely (status < 0)) {
		ehci_dbg (ehci, "can't init itds\n");
		goto done;
	}

	/* schedule ... need to lock */
	spin_lock_irqsave (&ehci->lock, flags);
	if (!_ehci_dev_exist(ehci)) {
		status = -ESHUTDOWN;
		goto done_not_linked;
	}
	status = usb_hcd_link_urb_to_ep(ehci_to_hcd(ehci), urb);	
	if (unlikely(status))
		goto done_not_linked;
	status = iso_stream_schedule(ehci, urb, stream);
	if (likely (status == 0))
		itd_link_urb (ehci, urb, ehci->periodic_size << 3, stream);
	else
		usb_hcd_unlink_urb_from_ep(ehci_to_hcd(ehci), urb);
done_not_linked:
	spin_unlock_irqrestore (&ehci->lock, flags);

done:
	if (unlikely (status < 0))
		iso_stream_put (ehci, stream);
	
	return status;
}

/*-------------------------------------------------------------------------*/

/*
 * "Split ISO TDs" ... used for USB 1.1 devices going through the
 * TTs in USB 2.0 hubs.  These need microframe scheduling.
 */

static inline void
sitd_sched_init(
	struct ehci_hcd		*ehci,
	struct ehci_iso_sched	*iso_sched,
	struct ehci_iso_stream	*stream,
	struct urb		*urb
)
{
	unsigned	i;
	dma_addr_t	dma = urb->transfer_dma;

	/* how many frames are needed for these transfers */
	iso_sched->span = urb->number_of_packets * stream->interval;

	/* figure out per-frame sitd fields that we'll need later
	 * when we fit new sitds into the schedule.
	 */
	for (i = 0; i < urb->number_of_packets; i++) {
		struct ehci_iso_packet	*packet = &iso_sched->packet [i];
		unsigned		length;
		dma_addr_t		buf;
		u32			trans;

		length = urb->iso_frame_desc [i].length & 0x03ff;
		buf = dma + urb->iso_frame_desc [i].offset;

		trans = SITD_STS_ACTIVE;
		if (((i + 1) == urb->number_of_packets)
				&& !(urb->transfer_flags & URB_NO_INTERRUPT))
			trans |= SITD_IOC;
		trans |= length << 16;
		packet->transaction = cpu_to_hc32(ehci, trans);

		/* might need to cross a buffer page within a td */
		packet->bufp = buf;
		packet->buf1 = (buf + length) & ~0x0fff;
		if (packet->buf1 != (buf & ~(uint64_t)0x0fff))
			packet->cross = 1;

		/* OUT uses multiple start-splits */
		if (stream->bEndpointAddress & USB_DIR_IN)
			continue;
		length = (length + 187) / 188;
		if (length > 1) /* BEGIN vs ALL */
			length |= 1 << 3;
		packet->buf1 |= length;
	}
}

static int
sitd_urb_transaction (
	struct ehci_iso_stream	*stream,
	struct ehci_hcd		*ehci,
	struct urb		*urb,
	gfp_t			mem_flags
)
{
	struct ehci_sitd	*sitd;
	dma_addr_t		sitd_dma;
	int			i;
	struct ehci_iso_sched	*iso_sched;
	unsigned long		flags;

	iso_sched = iso_sched_alloc (urb->number_of_packets, mem_flags);
	if (iso_sched == NULL)
		return -ENOMEM;

	sitd_sched_init(ehci, iso_sched, stream, urb);

	/* allocate/init sITDs */
	spin_lock_irqsave (&ehci->lock, flags);
	for (i = 0; i < urb->number_of_packets; i++) {
		uint32_t buf_index;
		/* NOTE:  for now, we don't try to handle wraparound cases
		 * for IN (using sitd->hw_backpointer, like a FSTN), which
		 * means we never need two sitds for full speed packets.
		 */

		/* free_list.next might be cache-hot ... but maybe
		 * the HC caches it too. avoid that issue for now.
		 */

		/* prefer previously-allocated sitds */
		if (!list_empty(&stream->free_list)) {
			sitd = list_entry (stream->free_list.prev,
					 struct ehci_sitd, sitd_list);
			list_del (&sitd->sitd_list);
			sitd_dma = sitd->sitd_dma;
			buf_index = sitd->buf_index;
		} else {
			spin_unlock_irqrestore (&ehci->lock, flags);
			sitd = ehci_sitd_alloc (ehci, mem_flags,
					&sitd_dma);
			buf_index = sitd->buf_index;
			spin_lock_irqsave (&ehci->lock, flags);
			if (!sitd) {
				iso_sched_free(stream, iso_sched);
				spin_unlock_irqrestore(&ehci->lock, flags);
				return -ENOMEM;
			}
		}

		memset (sitd, 0, sizeof *sitd);
		sitd->sitd_dma = sitd_dma;
		sitd->buf_index = buf_index;
		list_add (&sitd->sitd_list, &iso_sched->td_list);
	}

	/* temporarily store schedule info in hcpriv */
	urb->hcpriv = iso_sched;
	urb->error_count = 0;

	spin_unlock_irqrestore (&ehci->lock, flags);
	return 0;
}

/*-------------------------------------------------------------------------*/

static inline void
sitd_patch(
	struct ehci_hcd		*ehci,
	struct ehci_iso_stream	*stream,
	struct ehci_sitd	*sitd,
	struct ehci_iso_sched	*iso_sched,
	unsigned		index
)
{
	struct ehci_iso_packet	*uf = &iso_sched->packet [index];
	uint64_t		bufp = uf->bufp;

	sitd->hw_next = EHCI_LIST_END(ehci);
	sitd->hw_fullspeed_ep = stream->address;
	sitd->hw_uframe = stream->splits;
	sitd->hw_results = uf->transaction;
	sitd->hw_backpointer = EHCI_LIST_END(ehci);

	bufp = uf->bufp;
	sitd->hw_buf[0] = cpu_to_hc32(ehci, bufp);
	sitd->hw_buf_hi[0] = cpu_to_hc32(ehci, bufp >> 32);

	sitd->hw_buf[1] = cpu_to_hc32(ehci, uf->buf1);
	if (uf->cross)
		bufp += 4096;
	sitd->hw_buf_hi[1] = cpu_to_hc32(ehci, bufp >> 32);
	sitd->index = index;
}

static inline void
sitd_link (struct ehci_hcd *ehci, unsigned frame, struct ehci_sitd *sitd)
{
	/* note: sitd ordering could matter (CSPLIT then SSPLIT) */
	sitd->sitd_next = ehci->pshadow [frame];
	sitd->hw_next = ehci->periodic [frame];
	ehci->pshadow [frame].sitd = sitd;
	sitd->frame = frame;
	wmb ();
	ehci->periodic[frame] = cpu_to_hc32(ehci, sitd->sitd_dma | Q_TYPE_SITD);	
}

/* fit urb's sitds into the selected schedule slot; activate as needed */
static int
sitd_link_urb (
	struct ehci_hcd		*ehci,
	struct urb		*urb,
	unsigned		mod,
	struct ehci_iso_stream	*stream
)
{
	int			packet;
	unsigned		next_uframe;
	struct ehci_iso_sched	*sched = urb->hcpriv;
	struct ehci_sitd	*sitd;

	next_uframe = stream->next_uframe;

	if (list_empty(&stream->td_list)) {
		/* usbfs ignores TT bandwidth */
		ehci_to_hcd(ehci)->self.bandwidth_allocated
				+= stream->bandwidth;
		ehci_vdbg (ehci,
			"sched ep%d%s-iso [%d] %dms/%04x\n",
			stream->bEndpointAddress & 0x0f,
			(stream->bEndpointAddress & USB_DIR_IN) ? "in" : "out",
			(next_uframe >> 3) % ehci->periodic_size,
			stream->interval, hc32_to_cpu(ehci, stream->splits));
		stream->start = jiffies;
	}
	ehci_to_hcd(ehci)->self.bandwidth_isoc_reqs++;

	/* fill sITDs frame by frame */
	for (packet = 0, sitd = NULL;
			packet < urb->number_of_packets;
			packet++) {

		/* ASSERT:  we have all necessary sitds */
		BUG_ON (list_empty (&sched->td_list));

		/* ASSERT:  no itds for this endpoint in this frame */

		sitd = list_entry (sched->td_list.next,
				struct ehci_sitd, sitd_list);
		list_move_tail (&sitd->sitd_list, &stream->td_list);
		sitd->stream = iso_stream_get (stream);
		sitd->urb = usb_get_urb (urb);

		sitd_patch(ehci, stream, sitd, sched, packet);
		sitd_link (ehci, (next_uframe >> 3) % ehci->periodic_size,
				sitd);

		next_uframe += stream->interval << 3;
		stream->depth += stream->interval << 3;
	}
	stream->next_uframe = next_uframe % mod;

	/* don't need that schedule data any more */
	iso_sched_free (stream, sched);
	urb->hcpriv = NULL;
#if 0
	timer_action (ehci, TIMER_IO_WATCHDOG);
#endif
	return enable_periodic(ehci);
}

/*-------------------------------------------------------------------------*/

#define	SITD_ERRS (SITD_STS_ERR | SITD_STS_DBE | SITD_STS_BABBLE \
				| SITD_STS_XACT | SITD_STS_MMF)

/* Process and recycle a completed SITD.  Return true iff its urb completed,
 * and hence its completion callback probably added things to the hardware
 * schedule.
 *
 * Note that we carefully avoid recycling this descriptor until after any
 * completion callback runs, so that it won't be reused quickly.  That is,
 * assuming (a) no more than two urbs per frame on this endpoint, and also
 * (b) only this endpoint's completions submit URBs.  It seems some silicon
 * corrupts things if you reuse completed descriptors very quickly...
 */
static unsigned
sitd_complete (
	struct ehci_hcd		*ehci,
	struct ehci_sitd	*sitd
) {
	struct urb				*urb = sitd->urb;
	struct usb_iso_packet_descriptor	*desc;
	u32					t;
	int					urb_index = -1;
	struct ehci_iso_stream			*stream = sitd->stream;
	struct usb_device			*dev;
	unsigned				retval = false;

//ithPrintf("sitd_complete urb = %x\n", urb);

	ehci_invalid_sitd(sitd);

	urb_index = sitd->index;
	desc = &urb->iso_frame_desc [urb_index];
	t = hc32_to_cpup(ehci, &sitd->hw_results);

	/* report transfer status */
	if (t & SITD_ERRS) {
		urb->error_count++;
		if (t & SITD_STS_DBE)
			desc->status = usb_pipein (urb->pipe)
				? -ENOSR  /* hc couldn't read */
				: -ECOMM; /* hc couldn't write */
		else if (t & SITD_STS_BABBLE)
			desc->status = -EOVERFLOW;
		else /* XACT, MMF, etc */
			desc->status = -EPROTO;
	} else {
		desc->status = 0;
		desc->actual_length = desc->length - SITD_LENGTH (t);
	}
	stream->depth -= stream->interval << 3;

	/* handle completion now? */
	if ((urb_index + 1) != urb->number_of_packets)
		goto done;

	/* ASSERT: it's really the last sitd for this urb
	list_for_each_entry (sitd, &stream->td_list, sitd_list)
		BUG_ON (sitd->urb == urb);
	 */

	/* give urb back to the driver; completion often (re)submits */
	dev = urb->dev;
	ehci_urb_done(ehci, urb, 0);
	retval = true;
	urb = NULL;
	(void) disable_periodic(ehci);
	ehci_to_hcd(ehci)->self.bandwidth_isoc_reqs--;

	if (list_empty (&stream->td_list)) {
		ehci_to_hcd(ehci)->self.bandwidth_allocated
				-= stream->bandwidth;
		ehci_vdbg (ehci,
			"deschedule ep%d%s-iso\n",
			stream->bEndpointAddress & 0x0f,
			(stream->bEndpointAddress & USB_DIR_IN) ? "in" : "out");
	}
	iso_stream_put (ehci, stream);
	/* OK to recycle this SITD now that its completion callback ran. */
done:
	usb_put_urb(urb);
	sitd->urb = NULL;
	sitd->stream = NULL;
	list_move(&sitd->sitd_list, &stream->free_list);
	iso_stream_put(ehci, stream);

	return retval;
}


static int sitd_submit (struct ehci_hcd *ehci, struct urb *urb,
	gfp_t mem_flags)
{
	int			status = -EINVAL;
	unsigned long		flags;
	struct ehci_iso_stream	*stream;

	/* Get iso_stream head */
	stream = iso_stream_find (ehci, urb);
	if (stream == NULL) {
		ehci_dbg (ehci, "can't get iso stream\n");
		return -ENOMEM;
	}
	if (urb->interval != stream->interval) {
		ehci_dbg (ehci, "can't change iso interval %d --> %d\n",
			stream->interval, urb->interval);
		goto done;
	}

#ifdef EHCI_URB_TRACE
	ehci_dbg (ehci,
		"submit %p ep%d%s-iso len %d\n",
		urb,
		usb_pipeendpoint (urb->pipe),
		usb_pipein (urb->pipe) ? "in" : "out",
		urb->transfer_buffer_length);
#endif

	/* allocate SITDs */
	status = sitd_urb_transaction (stream, ehci, urb, mem_flags);
	if (status < 0) {
		ehci_dbg (ehci, "can't init sitds\n");
		goto done;
	}

	/* schedule ... need to lock */
	spin_lock_irqsave (&ehci->lock, flags);
	if (!_ehci_dev_exist(ehci)) {
		status = -ESHUTDOWN;
		goto done_not_linked;
	}
	status = usb_hcd_link_urb_to_ep(ehci_to_hcd(ehci), urb);
	if (unlikely(status))
		goto done_not_linked;
	status = iso_stream_schedule(ehci, urb, stream);
	if (status == 0)
		sitd_link_urb (ehci, urb, ehci->periodic_size << 3, stream);
	else
		usb_hcd_unlink_urb_from_ep(ehci_to_hcd(ehci), urb);
done_not_linked:
	spin_unlock_irqrestore (&ehci->lock, flags);

done:
	if (status < 0)
		iso_stream_put (ehci, stream);
	return status;
}
#endif // #if 0

/*-------------------------------------------------------------------------*/

static void
scan_periodic (struct ehci_hcd *ehci)
{
	unsigned	now_uframe, frame, clock, clock_frame, mod;
	unsigned	modified;

    _hcd_func_enter;
	
    mod = ehci->periodic_size << 3;

	/*
     * When running, scan from last scan point up to "now"
     * else clean up by scanning everything that's left.
     * Touches as few pages as possible:  cache-friendly.
     */
	now_uframe = ehci->next_uframe;
	if (HC_IS_RUNNING (ehci_to_hcd(ehci)->state))
		clock = ithReadRegA(ehci->regs.frame_index);
    else
		clock = now_uframe + mod - 1;

    clock %= mod;
	clock_frame = clock >> 3;

	for (;;) {
		union ehci_shadow	q, *q_p;
		uint32_t			type, *hw_p;
		unsigned		incomplete = false;

		frame = now_uframe >> 3;
restart:
		/* scan each element in frame's queue for completions */
		q_p = &ehci->pshadow [frame];
		hw_p = &ehci->periodic [frame];
		q.ptr = q_p->ptr;
		type = Q_NEXT_TYPE(ehci, *hw_p);
		modified = 0;

		while (q.ptr != NULL) {
			unsigned		uf;
			union ehci_shadow	temp;
			int			live;

			live = HC_IS_RUNNING (ehci_to_hcd(ehci)->state);
			switch (hc32_to_cpu(ehci, type)) {
            case Q_TYPE_QH:
				/* handle any completions */
				temp.qh = qh_get (q.qh);
				type = Q_NEXT_TYPE(ehci, q.qh->hw_next);
				q = q.qh->qh_next;
				modified = qh_completions (ehci, temp.qh);
				if (unlikely (list_empty (&temp.qh->qtd_list)))
					intr_deschedule (ehci, temp.qh);
				qh_put (temp.qh);
                break;
            case Q_TYPE_FSTN:
                /* for "save place" FSTNs, look at QH entries
                 * in the previous frame for completions.
                 */
				if (q.fstn->hw_prev != EHCI_LIST_END(ehci)) {
                    dbg ("ignoring completions from FSTNs");
                }
				type = Q_NEXT_TYPE(ehci, q.fstn->hw_next);
                q = q.fstn->fstn_next;
                break;
            case Q_TYPE_ITD:
				/* If this ITD is still active, leave it for
				 * later processing ... check the next entry.
				 * No need to check for activity unless the
				 * frame is current.
				 */
				if (frame == clock_frame && live) {
					rmb();
					for (uf = 0; uf < 8; uf++) {
						if (q.itd->hw_transaction[uf] &
							    ITD_ACTIVE(ehci))
                        break;
                    }
					if (uf < 8) {
						incomplete = true;
                    q_p = &q.itd->itd_next;
                    hw_p = &q.itd->hw_next;
						type = Q_NEXT_TYPE(ehci,
							q.itd->hw_next);
						q = *q_p;
						break;
					}
                }

				/* Take finished ITDs out of the schedule
				 * and process them:  recycle, maybe report
				 * URB completion.  HC won't cache the
				 * pointer for much longer, if at all.
				 */
				*q_p = q.itd->itd_next;
				*hw_p = q.itd->hw_next;
				type = Q_NEXT_TYPE(ehci, q.itd->hw_next);
				wmb();
				modified = itd_complete (ehci, q.itd);
                q = *q_p;
                break;
            case Q_TYPE_SITD:
				/* If this SITD is still active, leave it for
				 * later processing ... check the next entry.
				 * No need to check for activity unless the
				 * frame is current.
				 */
				if (frame == clock_frame && live &&
						(q.sitd->hw_results &
							SITD_ACTIVE(ehci))) {
					incomplete = true;
					q_p = &q.sitd->sitd_next;
					hw_p = &q.sitd->hw_next;
					type = Q_NEXT_TYPE(ehci,
							q.sitd->hw_next);
					q = *q_p;
					break;
				}

				/* Take finished SITDs out of the schedule
				 * and process them:  recycle, maybe report
				 * URB completion.
				 */
				*q_p = q.sitd->sitd_next;
				*hw_p = q.sitd->hw_next;
				type = Q_NEXT_TYPE(ehci, q.sitd->hw_next);
				wmb();
				modified = sitd_complete (ehci, q.sitd);
				q = *q_p;
                break;
            default:
				dbg ("corrupt type %d frame %d shadow %p",
					type, frame, q.ptr);
				while(1);
				q.ptr = NULL;
            }

			/* assume completion callbacks modify the queue */
			if (unlikely (modified)) {
				if (likely(ehci->periodic_sched > 0))
                    goto restart;

				/* short-circuit this scan */
				now_uframe = clock;
				break;
			}
        }

		/* If we can tell we caught up to the hardware, stop now.
		 * We can't advance our scan without collecting the ISO
		 * transfers that are still pending in this frame.
		 */
		if (incomplete && HC_IS_RUNNING(ehci_to_hcd(ehci)->state)) {
			ehci->next_uframe = now_uframe;
			break;
		}

        // FIXME:  this assumes we won't get lapped when
        // latencies climb; that should be rare, but...
        // detect it, and just go all the way around.
        // FLR might help detect this case, so long as latencies
        // don't exceed periodic_size msec (default 1.024 sec).

        // FIXME:  likewise assumes HC doesn't halt mid-scan

		if (now_uframe == clock) {
			unsigned	now;

			if (!HC_IS_RUNNING (ehci_to_hcd(ehci)->state)
					|| ehci->periodic_sched == 0)
                break;
            ehci->next_uframe = now_uframe;
			now = ithReadRegA(ehci->regs.frame_index) % mod;
			if (now_uframe == now)
                break;

			/* rescan the rest of this frame, then ... */
			clock = now;
			clock_frame = clock >> 3;
		} else {
			now_uframe++;
			now_uframe %= mod;
        } 
    } 
	_hcd_func_leave;
}
