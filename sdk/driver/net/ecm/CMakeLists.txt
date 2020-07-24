INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/sdk/driver/usb/inc/
    )

ADD_LIBRARY(ecm STATIC
    usbnet.c
    cdc_ether.c
    ite_ecm.c
    )