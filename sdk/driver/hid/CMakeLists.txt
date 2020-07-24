include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/driver/usb/inc/
)

if (DEFINED CFG_USB_HID)
    add_library(hid STATIC
        usbhid/hid-core.c
        hid-core.c
        hid-input.c
        input.c
    )
else()
    if (DEFINED CFG_USB_MOUSE)
        add_library(hid STATIC
            usbhid/usbmouse.c
        )
    endif()

    if (DEFINED CFG_USB_KBD)
        add_library(hid STATIC
            usbhid/usbkbd.c
        )
    endif()
endif()
