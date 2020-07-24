add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
)

if (DEFINED CFG_TEST_MOUSE)
    add_executable(${CMAKE_PROJECT_NAME}
        test_usb_mouse.c
    )
elseif (DEFINED CFG_TEST_KBD)
    add_executable(${CMAKE_PROJECT_NAME}
        test_usb_kbd.c
    )
elseif (DEFINED CFG_TEST_HID_TRANSPORT)
    add_executable(${CMAKE_PROJECT_NAME}
        test_usb_hid_transport.c
    )
elseif (DEFINED CFG_TEST_HID_TRANSPARENT)
    add_executable(${CMAKE_PROJECT_NAME}
        test_usb_hid_transparent.c
        hid_transparent.c
    )
elseif (DEFINED CFG_TEST_MSC)
    add_executable(${CMAKE_PROJECT_NAME}
        test_usb_msc.c
    )
elseif (DEFINED CFG_TEST_SD)
    add_executable(${CMAKE_PROJECT_NAME}
        test_card.c
    )
elseif (DEFINED CFG_TEST_MASS_STORAGE)
    add_executable(${CMAKE_PROJECT_NAME}
        test_usbd_fsg.c
    )
elseif (DEFINED CFG_USBD_ACM)
    ITE_ADD_DEFINITIONS_IF_DEFINED(CFG_TEST_ACM_LOOPBACK)
    ITE_ADD_DEFINITIONS_IF_DEFINED(CFG_TEST_ACM_SEND_FILE)
    ITE_ADD_DEFINITIONS_IF_DEFINED(CFG_TEST_ACM_DBG)

    add_executable(${CMAKE_PROJECT_NAME}
        test_usbd_acm.c
    )
endif()

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)
