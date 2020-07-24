SET(SW_UART_SRC
    main.c
    )

ADD_EXECUTABLE(swUart.plugin
    ${SW_UART_SRC}
)

ADD_CUSTOM_COMMAND(
    TARGET swUart.plugin
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
            -O binary ${CMAKE_CURRENT_BINARY_DIR}/swUart.plugin ${LIBRARY_OUTPUT_PATH}/swUart.raw
    POST_BUILD
    COMMAND ${CMAKE_OBJDUMP}
            -D ${CMAKE_CURRENT_BINARY_DIR}/swUart.plugin > ${LIBRARY_OUTPUT_PATH}/swUart.dbg
    COMMAND ${READELF}
            -a ${CMAKE_CURRENT_BINARY_DIR}/swUart.plugin > ${CMAKE_CURRENT_BINARY_DIR}/swUart.symbol
    COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/swUart.raw
)