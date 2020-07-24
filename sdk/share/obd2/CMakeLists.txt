include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/include/
    ${PROJECT_SOURCE_DIR}/sdk/include/can_bus
    )

add_library(obd2 STATIC
    obd2.c
    obd2.h
    obd2_hal.c
    obd2_hal.h
    )
