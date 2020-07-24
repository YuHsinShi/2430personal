/* SPDX-License-Identifier: GPL-2.0 */
#ifndef USB_STORAGE_COMMON_H
#define USB_STORAGE_COMMON_H

#include <linux/device.h>
#include <linux/usb/storage.h>
//#include <scsi/scsi.h>
#include <linux/scsi/scsi.h>
//#include <asm/unaligned.h>
#include <linux/unaligned.h>

#include <linux/os.h>
#include "ite/ith.h"
#include "ite/ite_fsg.h"


#define VERBOSE_DEBUG
//#define DUMP_MSGS
//#define DUMP_CDB

#define PR_ERR		1
#define PR_WARN		0
#define PR_INFO		0
#define PR_DBG		0


#ifdef VERBOSE_DEBUG
#define VLDBG	LDBG
#else
#define VLDBG(lun, fmt, args...) do { } while (0)
#endif /* VERBOSE_DEBUG */

#define _LMSG(func, lun, fmt, args...)					\
	do {								\
		if ((lun)->name_pfx && *(lun)->name_pfx)		\
			func("%s/%s: " fmt, *(lun)->name_pfx,		\
				 (lun)->name, ## args);			\
		else							\
			func("%s: " fmt, (lun)->name, ## args);		\
	} while (0)

#define LDBG(lun, fmt, args...)		_LMSG(pr_debug, lun, fmt, ## args)
#define LERROR(lun, fmt, args...)	_LMSG(pr_err, lun, fmt, ## args)
#define LWARN(lun, fmt, args...)	_LMSG(pr_warn, lun, fmt, ## args)
#define LINFO(lun, fmt, args...)	_LMSG(pr_info, lun, fmt, ## args)

#if PR_ERR
#define pr_err(string, args...)   do { ithPrintf("[FSG][ERR] "); ithPrintf(string, ## args); } while (0)
#else
#define pr_err(string, args...)
#endif

#if PR_WARN
#define pr_warn(string, args...)   do { ithPrintf("[FSG][WARN] "); ithPrintf(string, ## args); } while (0)
#else
#define pr_warn(string, args...)
#endif

#if PR_INFO
#define pr_info(string, args...)   do { ithPrintf("[FSG][INFO] "); ithPrintf(string, ## args); } while (0)
#else
#define pr_info(string, args...)
#endif

#if PR_DBG
#define pr_debug(string, args...)   do { ithPrintf("[FSG][DBG] "); ithPrintf(string, ## args); } while (0)
#else
#define pr_debug(string, args...)
#endif



#ifdef DUMP_MSGS
#define dump_msg(fsg, /* const char * */ label,			\
		   /* const u8 * */ buf, /* unsigned */ length)		\
do {									\
	if (length < 512) {						\
		DBG(fsg, "%s, length %u:\n", label, length);		\
		ithPrintVram8((u32)buf, length);			\
	}								\
} while (0)
#else
	
#define dump_msg(fsg, /* const char * */ label, \
			   /* const u8 * */ buf, /* unsigned */ length) do { } while (0)
#endif

#ifdef DUMP_CDB
#define dump_cdb(fsg)						\
		ithPrintVram8((u32)(fsg)->cmnd, (fsg)->cmnd_size)
#else
#define dump_cdb(fsg) do { } while (0)
#endif



/* Length of a SCSI Command Data Block */
#define MAX_COMMAND_SIZE	16

/* SCSI Sense Key/Additional Sense Code/ASC Qualifier values */
#define SS_NO_SENSE				0
#define SS_COMMUNICATION_FAILURE		0x040800
#define SS_INVALID_COMMAND			0x052000
#define SS_INVALID_FIELD_IN_CDB			0x052400
#define SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE	0x052100
#define SS_LOGICAL_UNIT_NOT_SUPPORTED		0x052500
#define SS_MEDIUM_NOT_PRESENT			0x023a00
#define SS_MEDIUM_REMOVAL_PREVENTED		0x055302
#define SS_NOT_READY_TO_READY_TRANSITION	0x062800
#define SS_RESET_OCCURRED			0x062900
#define SS_SAVING_PARAMETERS_NOT_SUPPORTED	0x053900
#define SS_UNRECOVERED_READ_ERROR		0x031100
#define SS_WRITE_ERROR				0x030c02
#define SS_WRITE_PROTECTED			0x072700

#define SK(x)		((u8) ((x) >> 16))	/* Sense Key byte, etc. */
#define ASC(x)		((u8) ((x) >> 8))
#define ASCQ(x)		((u8) (x))

/*
 * Vendor (8 chars), product (16 chars), release (4 hexadecimal digits) and NUL
 * byte
 */
#define INQUIRY_STRING_LEN ((size_t) (8 + 16 + 4 + 1))

struct fsg_lun {
#if 1
    const struct fsg_operations *ops;
#else
	struct file	*filp;
#endif
	//loff_t		file_length;
	u32		num_sectors;

	unsigned int	initially_ro:1;
	unsigned int	ro:1;
	unsigned int	removable:1;
	unsigned int	cdrom:1;
	unsigned int	prevent_medium_removal:1;
	unsigned int	registered:1;
	unsigned int	info_valid:1;
	unsigned int	nofua:1;
	unsigned int	id:4;

	u32		sense_data;
	u32		sense_data_info;
	u32		unit_attention_data;

	unsigned int	blkbits; /* Bits of logical block size
						       of bound block device */
	unsigned int	blksize; /* logical block size of bound block device */
	struct device	dev;
	const char	*name;		/* "lun.name" */
	const char	**name_pfx;	/* "function.name" */
	char		inquiry_string[INQUIRY_STRING_LEN];
};

static inline bool fsg_lun_is_open(struct fsg_lun *curlun)
{
#if 1
    return (curlun->ops->response(curlun->id) ? true : false);
#else
	return curlun->filp != NULL;
#endif
}

/* Default size of buffer length. */
#define FSG_BUFLEN	((u32)(32*1024))

/* Maximal number of LUNs supported in mass storage function */
#define FSG_MAX_LUNS	4

enum fsg_buffer_state {
	BUF_STATE_SENDING = -2,
	BUF_STATE_RECEIVING,
	BUF_STATE_EMPTY = 0,
	BUF_STATE_FULL
};

struct fsg_buffhd {
	void				*buf;
	volatile enum fsg_buffer_state		state;
	struct fsg_buffhd		*next;

	/*
	 * The NetChip 2280 is faster, and handles some protocol faults
	 * better, if we don't submit any short bulk-out read requests.
	 * So we will record the intended request length here.
	 */
	unsigned int			bulk_out_intended_length;

	struct usb_request		*inreq;
	struct usb_request		*outreq;
};

enum fsg_state {
	FSG_STATE_NORMAL,
	FSG_STATE_ABORT_BULK_OUT,
	FSG_STATE_PROTOCOL_RESET,
	FSG_STATE_CONFIG_CHANGE,
	FSG_STATE_EXIT,
	FSG_STATE_TERMINATED
};

enum data_direction {
	DATA_DIR_UNKNOWN = 0,
	DATA_DIR_FROM_HOST,
	DATA_DIR_TO_HOST,
	DATA_DIR_NONE
};

static inline u32 get_unaligned_be24(u8 *buf)
{
	return 0xffffff & (u32) get_unaligned_be32(buf - 1);
}

static inline struct fsg_lun *fsg_lun_from_dev(struct device *dev)
{
	return container_of(dev, struct fsg_lun, dev);
}

enum {
	FSG_STRING_INTERFACE
};

extern struct usb_interface_descriptor fsg_intf_desc;

extern struct usb_endpoint_descriptor fsg_fs_bulk_in_desc;
extern struct usb_endpoint_descriptor fsg_fs_bulk_out_desc;
extern struct usb_descriptor_header *fsg_fs_function[];

extern struct usb_endpoint_descriptor fsg_hs_bulk_in_desc;
extern struct usb_endpoint_descriptor fsg_hs_bulk_out_desc;
extern struct usb_descriptor_header *fsg_hs_function[];

extern struct usb_endpoint_descriptor fsg_ss_bulk_in_desc;
extern struct usb_ss_ep_comp_descriptor fsg_ss_bulk_in_comp_desc;
extern struct usb_endpoint_descriptor fsg_ss_bulk_out_desc;
extern struct usb_ss_ep_comp_descriptor fsg_ss_bulk_out_comp_desc;
extern struct usb_descriptor_header *fsg_ss_function[];

void fsg_lun_close(struct fsg_lun *curlun);
int fsg_lun_open(struct fsg_lun *curlun, const char *filename);
int fsg_lun_fsync_sub(struct fsg_lun *curlun);
void store_cdrom_address(u8 *dest, int msf, u32 addr);
#if 0
ssize_t fsg_show_ro(struct fsg_lun *curlun, char *buf);
ssize_t fsg_show_nofua(struct fsg_lun *curlun, char *buf);
ssize_t fsg_show_file(struct fsg_lun *curlun, struct rw_semaphore *filesem,
		      char *buf);
ssize_t fsg_show_inquiry_string(struct fsg_lun *curlun, char *buf);
ssize_t fsg_show_cdrom(struct fsg_lun *curlun, char *buf);
ssize_t fsg_show_removable(struct fsg_lun *curlun, char *buf);
ssize_t fsg_store_ro(struct fsg_lun *curlun, struct rw_semaphore *filesem,
		     const char *buf, size_t count);
ssize_t fsg_store_nofua(struct fsg_lun *curlun, const char *buf, size_t count);
ssize_t fsg_store_file(struct fsg_lun *curlun, struct rw_semaphore *filesem,
		       const char *buf, size_t count);
ssize_t fsg_store_cdrom(struct fsg_lun *curlun, struct rw_semaphore *filesem,
			const char *buf, size_t count);
ssize_t fsg_store_removable(struct fsg_lun *curlun, const char *buf,
			    size_t count);
ssize_t fsg_store_inquiry_string(struct fsg_lun *curlun, const char *buf,
				 size_t count);
#endif
#endif /* USB_STORAGE_COMMON_H */
