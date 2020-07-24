file(COPY
    ${PROJECT_SOURCE_DIR}/build/$ENV{CFG_BUILDPLATFORM}/alt_cpu/lib/sm32/swSerialPort.hex
    DESTINATION ${PROJECT_SOURCE_DIR}/sdk/driver/alt_cpu/swSerialPort/.
)

if (DEFINED CFG_SW_SERIAL_PORT AND NOT $ENV{CFG_PLATFORM} STREQUAL win32)
    add_library(swSerialPort STATIC
        swSerialPortDevice.c
    )
endif()