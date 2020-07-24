/*
* ITE HID Gadget Device Configuration
*/
#if defined(CFG_USBD_HID_DEVICE)
static struct hidg_func_config ite_hidg_device = {
    .name = "hid-dev",
    .idx  = USBD_IDX_NONE,
    .typeID = USBD_HIDG_DEV,
    .subclass      = 0,
	.protocol      = 0,
	.report_length = 64,
	.report_desc_length	= 42,
	.default_report_desc = {
        0x05, 0x00,                     // usage page (undefined) 
        0x09, 0x02,                     // Usage (undefined)
        0xA1, 0x01,                     // collection (application) 
        //----------(Report ID = 0x99)----------
        0x85, 0x99,                     // Report ID (0x99)
        0x05, 0x00,                     // usage page (undefined)
        0x09, 0x02,                     // Usage (undefined)
        0xA1, 0x00,                     // Collection (Physical)
        //-----Input report-----
        0x09, 0x00,                     // USAGE (Undefined)
        0x15, 0x00,                     // LOGICAL_MINIMUM (0)
        0x26, 0xFF, 0x00,               // LOGICAL_MAXIMUM (255)
        0x75, 0x08,                     // REPORT_SIZE (8)
        0x95, 0x20,                     // REPORT_COUNT (8)
        0x81, 0x02,                     // INPUT (Data,Vari,Abs)
        //-----Output report-----
        0x09, 0x00,                     // USAGE (Undefined)
        0x15, 0x00,                     // logical minimum (0)
        0x26, 0xFF, 0x00,               // logical maximum (255)
        0x75, 0x08,                     // report size (8)
        0x95, 0x20,                     // report count (8)
        0x91, 0x02,                     // OUTPUT (Data,Vari,Abs)

        0xC0,                           // end collection
        0xC0                            // end collection
	}
};

#endif