#include <assert.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include "ite/ite_usbhid.h"
#include "hid_transparent.h"
#include "linux/unaligned.h"

typedef struct usb_composite_overwrite {
	u16	idVendor;
	u16	idProduct;
	u16	bcdDevice;
	char	*serial_number;
	char	*manufacturer;
	char	*product;
}USB_COMPOSITE_OVERWRITE;

typedef struct hid_transparent_info {
    struct list_head 	list;
    u32 usbIdx;
    u32 reportID;
    u32 mapReportID;
    struct hid_device *hid;
}HID_TRANSPARENT_INFO;

typedef struct hid_read_thread {
    struct list_head 	list;
	u16	idVendor;
	u16	idProduct;
    pthread_t *hidReadthreadID;
    u8 readQuit;
}HID_READ_THREAD;

static LIST_HEAD(hid_info_mapping_list);
static LIST_HEAD(hid_read_thread_list);

static HIDG_FUNC_CONFIG *pFc = NULL;

static USB_COMPOSITE_OVERWRITE _regen_usbd_id()
{
    USB_COMPOSITE_OVERWRITE covr = {0};
    HID_TRANSPARENT_INFO *phid_info;
    list_for_each_entry(phid_info, &hid_info_mapping_list, list) {
        if(covr.idProduct == 0 || covr.idProduct % (phid_info->hid->vendor + phid_info->hid->product) != 0) {
            covr.idProduct += (phid_info->usbIdx * (phid_info->hid->vendor + phid_info->hid->product));
        } else {
            covr.idProduct += (phid_info->hid->vendor + phid_info->hid->product * phid_info->usbIdx);
        }
    }
    return covr;
}

static u8 *_fetch_hid_report_item(u8 *start, u8 *end, u8 **data, struct hid_item *item)
{
	u8 b;

	if ((end - start) <= 0)
		return NULL;

	b = *start++;
	item->type = (b >> 2) & 3;
	item->tag  = (b >> 4) & 15;

	if (item->tag == HID_ITEM_TAG_LONG) {
		item->format = HID_ITEM_FORMAT_LONG;

		if ((end - start) < 2)
			return NULL;

		item->size = *start++;
		item->tag  = *start++;

		if ((end - start) < item->size)
			return NULL;

		item->data.longdata = start;
		start += item->size;
		return start;
	}

	item->format = HID_ITEM_FORMAT_SHORT;
	item->size = b & 3;
    *data = start;

	switch (item->size) {
	case 0:
		return start;
	case 1:
		if ((end - start) < 1)
			return NULL;
		item->data.u8 = *start++;
		return start;
	case 2:
		if ((end - start) < 2)
			return NULL;
		item->data.u16 = get_unaligned_le16(start);
		start = (__u8 *)((__le16 *)start + 1);
		return start;
	case 3:
		item->size++;
		if ((end - start) < 4)
			return NULL;
		item->data.u32 = get_unaligned_le32(start);
		start = (__u8 *)((__le32 *)start + 1);
		return start;
	}

	return NULL;
}

static HID_TRANSPARENT_INFO* _add_hid_info_to_map_list(HID_TRANSPARENT_CONFIG *ptConf, int reportID, int mapReportID)
{
    HID_TRANSPARENT_INFO *ptmp_info;
    list_for_each_entry(ptmp_info, &hid_info_mapping_list, list) {
        if(ptConf->pHid == ptmp_info->hid && ptmp_info->reportID == reportID && ptmp_info->mapReportID == mapReportID) {
            return NULL;
        }
    }
    
    HID_TRANSPARENT_INFO *phid_info = malloc(sizeof(HID_TRANSPARENT_INFO));
    assert(phid_info);
    phid_info->hid = ptConf->pHid;
    phid_info->usbIdx = ptConf->pFc->idx;
    phid_info->reportID = reportID;
    phid_info->mapReportID = mapReportID;

    list_add(&phid_info->list, &hid_info_mapping_list);
    return phid_info;
}

static int _regen_hid_report(HID_TRANSPARENT_CONFIG *ptConf, HIDG_FUNC_CONFIG *pFc, int *report_id)
{
#define HID_REPORT_ID_PREFIX (((HID_GLOBAL_ITEM_TAG_REPORT_ID<<4)&0xf0) + ((HID_ITEM_TYPE_GLOBAL<<2)&0x0f) + 1) //0x85
    struct hid_item item;
	u8 *start = pFc->report_desc + (pFc->report_desc_length - ptConf->pFc->report_desc_length);
	u8 *end = start + pFc->report_desc_length;
    u8 *data;
    int has_output = 0;

    while ((start = _fetch_hid_report_item(start, end, &data, &item)) != NULL) {
        switch (item.type) {
        case HID_ITEM_TYPE_MAIN:
            switch (item.tag) {
            case HID_MAIN_ITEM_TAG_OUTPUT:
                printf("HID Report has output\n");
                has_output = 1;
            break;
            case HID_MAIN_ITEM_TAG_BEGIN_COLLECTION:
                if(*data == HID_COLLECTION_APPLICATION && *(start+4) == HID_REPORT_ID_PREFIX) {
                    continue;
                } else if (*data == HID_COLLECTION_APPLICATION && *start != HID_REPORT_ID_PREFIX) //report id
                {
                    // printf("no report ID add it !!! (%p)(%p)\n", start, end);
                    _add_hid_info_to_map_list(ptConf, 0, *report_id);
                    u8* tmp = kmemdup(pFc->report_desc ,pFc->report_desc_length+2, GFP_KERNEL);
                    assert(tmp);
                    int len = start - pFc->report_desc;
                    free(pFc->report_desc);
                    pFc->report_desc = tmp;
                    start = pFc->report_desc + len;
                    end = pFc->report_desc + pFc->report_desc_length;
                    pFc->report_desc_length += 2;
                    tmp = kmemdup(start, end - start, GFP_KERNEL);
                    assert(tmp);
                    memcpy(start+2, tmp, end - start);
                    end += 2;
                    *start = HID_REPORT_ID_PREFIX;
                    start++;
                    *start = (*report_id)++;
                    start++;
                    free(tmp);
                }
            break;
            }
        break;
        case HID_ITEM_TYPE_GLOBAL:
            switch (item.tag) {
            case HID_GLOBAL_ITEM_TAG_REPORT_ID:
                _add_hid_info_to_map_list(ptConf, *data, (*data)+1);
                (*data)++;
                if(*data > *report_id)
                    *report_id = (*data)+1;
            break;
            }
        break;
        
        }
        if (start == end) {
            break;
        }
    }
    // ithPrintf("after _regen_hid_report : \n");
    // ithPrintVram8((uint32_t)pFc->report_desc, pFc->report_desc_length);
    return has_output;
}

static u8 _read_quit(pthread_t threadID)
{
    HID_READ_THREAD *pThread;
    list_for_each_entry(pThread, &hid_read_thread_list, list) {
        if(*pThread->hidReadthreadID == threadID) {
            return pThread->readQuit;
        }
    }
    return 1;
}

void *_hid_read_thread_func(void *arg)
{
    HID_TRANSPARENT_CONFIG *ptConf = (HID_TRANSPARENT_CONFIG *)arg;
    uint8_t tmpBuffer[16] = { 0 };
    uint8_t *pBuf;
    HIDG_FUNC_CMD_DATA usbData = {0};
    usbData.idx = ptConf->pFc->idx;
    usbData.idVendor = ptConf->pHid->vendor;
    usbData.idProduct = ptConf->pHid->product;
    usbData.buffer = tmpBuffer;
    usbData.count = sizeof(tmpBuffer);
    while(!_read_quit(pthread_self()))
    {
        int len = ioctl(ITP_DEVICE_USBDHID, ITP_IOCTL_USBD_HIHG_READ, &usbData);
        if(len > 0)
        {
            int i;
            // printf("hid_read_thread ori : ");
            // for(i = 0; i<len; i++)
            // {
                // printf("0x%x ", tmpBuffer[i]);
            // }
            // printf("\r\n");
            
            pBuf = tmpBuffer;
            HID_TRANSPARENT_INFO *phid_info;
            list_for_each_entry(phid_info, &hid_info_mapping_list, list) {
                if(usbData.idx == phid_info->usbIdx && pBuf[0] == phid_info->mapReportID && usbData.idVendor == phid_info->hid->vendor && usbData.idProduct == phid_info->hid->product) {//chk idx
                    if(phid_info->reportID == 0) {
                        pBuf++;
                        len--;
                    } else {
                        pBuf[0] = phid_info->reportID;
                    }
                    iteUsbHidWrite(phid_info->hid, pBuf, len);

                    printf("hid_read_thread : (%p) ", phid_info->hid);
                    for(i = 0; i<len; i++)
                    {
                        printf("0x%x ", pBuf[i]);
                    }
                    printf("\r\n");
                    break;
                }
            }
        }
        else
        {
            usleep(60000);
        }
    }
}
///////////////////
int HidTransparent_AddConfig(HID_TRANSPARENT_CONFIG *ptConf)
{
    static int report_id = 1;
    static int has_output = 0;

    if(pFc == NULL)
    {
        if(ptConf->bInterfaceID == 0)
        {
            report_id = 1;
            has_output = 0;
            pFc = kmemdup(ptConf->pFc, sizeof(HIDG_FUNC_CONFIG), GFP_KERNEL);
            assert(pFc);
            pFc->report_desc = kmemdup(ptConf->pFc->report_desc, ptConf->pFc->report_desc_length, GFP_KERNEL);
            assert(pFc->report_desc);
            pFc->report_length++;
        }
        else return 0;
    }
    else
    {
        if(pFc->idVendor == ptConf->pFc->idVendor && pFc->idProduct == ptConf->pFc->idProduct && ptConf->bInterfaceID > 0 && ptConf->bInterfaceID < ptConf->bMaxInterfaces)
        {
            if (ptConf->bInterfaceID > 0 && ptConf->bInterfaceID < ptConf->bMaxInterfaces)
            {
                pFc->report_length = 64;
                u8 *new_desc = (u8 *)malloc(pFc->report_desc_length + ptConf->pFc->report_desc_length);
                assert(new_desc);
                memcpy(new_desc, pFc->report_desc, pFc->report_desc_length);
                memcpy(new_desc+pFc->report_desc_length, ptConf->pFc->report_desc, ptConf->pFc->report_desc_length);
                pFc->report_desc_length += ptConf->pFc->report_desc_length;
                free(pFc->report_desc);
                pFc->report_desc = new_desc;
            }
        }
        else if((pFc->idVendor != ptConf->pFc->idVendor || pFc->idProduct != ptConf->pFc->idProduct) && ptConf->bInterfaceID == 0)
        {
            free(pFc->report_desc);
            free(pFc);
            report_id = 1;
            has_output = 0;
            pFc = kmemdup(ptConf->pFc, sizeof(HIDG_FUNC_CONFIG), GFP_KERNEL);
            assert(pFc);
            pFc->report_desc = kmemdup(ptConf->pFc->report_desc, ptConf->pFc->report_desc_length, GFP_KERNEL);
            assert(pFc->report_desc);
            pFc->report_length++;
        }
        else
        {
            free(pFc->report_desc);
            free(pFc);
            report_id = 1;
            has_output = 0;
            pFc = NULL;
            return 0;
        }
    }

    has_output |= _regen_hid_report(ptConf, pFc, &report_id);

    //chk max interfaces
    if(ptConf->bInterfaceID+1 < ptConf->bMaxInterfaces) return 1; //rcv next interface config
    
    if(ioctl(ITP_DEVICE_USBDHID, ITP_IOCTL_USBD_HIHG_SET_CONFIG, pFc))
    {
        USB_COMPOSITE_OVERWRITE covr = _regen_usbd_id();
        ioctl(ITP_DEVICE_USBDHID, ITP_IOCTL_USBD_HIHG_OVERWRITE_VIDPID, &covr);
        ioctl(ITP_DEVICE_USBDHID, ITP_IOCTL_USBD_HIHG_REFRESH, NULL);
    }
    
    //chk has output to create read thread.
    if(has_output)
    {
        HID_READ_THREAD *pThread = (HID_READ_THREAD *)malloc(sizeof(HID_READ_THREAD));
        assert(pThread);
        pThread->idVendor = pFc->idVendor;
        pThread->idProduct = pFc->idProduct;
        pThread->readQuit = 0;
        pThread->hidReadthreadID = (pthread_t *)malloc(sizeof(pthread_t));
        assert(pThread->hidReadthreadID);
        list_add(&pThread->list, &hid_read_thread_list);
        pthread_create(pThread->hidReadthreadID, NULL, _hid_read_thread_func, (void *)ptConf);
    }
    
    free(pFc->report_desc);
    free(pFc);
    pFc = NULL;
    
    return 1;
}

int HidTransparent_DelConfig(struct hid_device *hid, uint32_t usbIdx)
{
    //delete mapping info in list
    HID_TRANSPARENT_INFO *phid_info, *tmp_info;
	list_for_each_entry_safe(phid_info, tmp_info, &hid_info_mapping_list, list) {
		if(phid_info->hid == hid) {
            list_del(&phid_info->list);
            free(phid_info);
        }
	}

    //drop read thread ,if has read
    HID_READ_THREAD *pThread, *tmp_thread;
    list_for_each_entry_safe(pThread, tmp_thread, &hid_read_thread_list, list) {
        if(pThread->idVendor == hid->vendor && pThread->idProduct == hid->product) {
            list_del(&pThread->list);
            pThread->readQuit = 1;
            pthread_join(*pThread->hidReadthreadID, NULL);
            free(pThread->hidReadthreadID);
            free(pThread);
        }
    }
    return 1;
}

int HidTransparent_Write(struct hid_device *hid, HIDG_FUNC_CMD_DATA *usbData)
{
    int ret = 0;
    HID_TRANSPARENT_INFO *phid_info;
    list_for_each_entry(phid_info, &hid_info_mapping_list, list) {
        if(hid == phid_info->hid) {
            HIDG_FUNC_CMD_DATA *ptData = kmemdup(usbData, sizeof(HIDG_FUNC_CMD_DATA), GFP_KERNEL);
            assert(ptData);
            ptData->idx = phid_info->usbIdx;
            if(phid_info->reportID == 0) {
                ptData->buffer = malloc(usbData->count+1);
                assert(ptData->buffer);
                memcpy(ptData->buffer+1, usbData->buffer, usbData->count);
                ptData->buffer[0] = phid_info->mapReportID;
                ptData->count++;
            } else if(ptData->buffer[0] != phid_info->reportID){
                continue;
            } else {
                ptData->buffer = kmemdup(usbData->buffer, usbData->count, GFP_KERNEL);
                assert(ptData->buffer);
                ptData->buffer[0] = phid_info->mapReportID;
            }

            ret = ioctl(ITP_DEVICE_USBDHID, ITP_IOCTL_USBD_HIHG_WRITE, ptData);
            free(ptData->buffer);
            free(ptData);
            break;
        }
    }
    return ret;
}
