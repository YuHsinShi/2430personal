file(COPY
    ${PROJECT_SOURCE_DIR}/build/$ENV{CFG_BUILDPLATFORM}/alt_cpu/lib/sm32/swUart.hex
    DESTINATION ${PROJECT_SOURCE_DIR}/sdk/driver/alt_cpu/swUart/.
)

if (DEFINED CFG_SW_UART AND NOT $ENV{CFG_PLATFORM} STREQUAL win32)
    add_library(swUart STATIC
        swUartDevice.c
    )
endif()