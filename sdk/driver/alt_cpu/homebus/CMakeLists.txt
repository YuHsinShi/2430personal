file(COPY
    ${PROJECT_SOURCE_DIR}/build/$ENV{CFG_BUILDPLATFORM}/alt_cpu/lib/sm32/homebus.hex
    DESTINATION ${PROJECT_SOURCE_DIR}/sdk/driver/alt_cpu/homebus/.
)



if (DEFINED CFG_HOMEBUS AND NOT $ENV{CFG_PLATFORM} STREQUAL win32)
    add_library(homebus STATIC
        homebusDevice.c
		hlink/uart_tx.c
		hlink/uart_rx.c
    )
endif()