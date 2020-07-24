INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/sdk/driver/usb/inc
)

ADD_LIBRARY(usb STATIC
    mmp_usbex.c
    usb_port.c
    usb/hcd.c
    usb/usb.c
    usb/urb.c
    usb/message.c
    usb/config.c
    usb/quirks.c
    usb/hub.c
    usb/driver.c
    usb/hcd/ehci-hcd.c
)

if (DEFINED CFG_USB_SERIAL)
		INCLUDE_DIRECTORIES(
		    ${PROJECT_SOURCE_DIR}/sdk/driver/usb/tty/
		)
    add_library(usb STATIC
        usb/serial/bus.c
        usb/serial/usb-serial.c
    )
endif()

if (DEFINED CFG_USB_OPTION)
		INCLUDE_DIRECTORIES(
		    ${PROJECT_SOURCE_DIR}/sdk/driver/usb/tty/
		)
    add_library(usb STATIC
        usb/serial/option.c
        usb/serial/usb_wwan.c
    )
endif()

if (DEFINED CFG_USB_DEVICE)
    INCLUDE_DIRECTORIES(
        ${PROJECT_SOURCE_DIR}/sdk/driver/usb/gadget/function/
        ${PROJECT_SOURCE_DIR}/sdk/driver/usb/gadget/include/
    )

    add_library(usb STATIC
        gadget/composite.c
        gadget/config.c
        gadget/epautoconf.c
        gadget/functions.c
        gadget/u_f.c
        gadget/usbstring.c
        gadget/common.c
        gadget/ite.c
        gadget/udc/core.c
    )

    if (DEFINED CFG_USBD_MASS_STORAGE)
        add_library(usb STATIC
            gadget/function/storage_common.c
            gadget/function/f_mass_storage.c
        )
    endif()

    if (DEFINED CFG_USBD_ACM)
        add_library(usb STATIC
            gadget/function/f_acm.c
            gadget/function/u_serial.c
            #gadget/function/tty.c
        )
    endif()

    if (DEFINED CFG_USBD_NCM)
        add_library(usb STATIC
            gadget/function/f_ncm.c
            gadget/function/u_ether.c
            gadget/function/ite_usbd_ncm.c
        )
    endif()

    if (DEFINED CFG_USBD_HID)
        add_library(usb STATIC
            gadget/function/f_hid.c
        )
    endif()
endif()


if (DEFINED CFG_USBD_ACM OR DEFINED CFG_USB_SERIAL)
    INCLUDE_DIRECTORIES(
        ${PROJECT_SOURCE_DIR}/sdk/driver/usb/tty/
    )
    add_library(usb STATIC
        tty/tty.c
    )
endif()
