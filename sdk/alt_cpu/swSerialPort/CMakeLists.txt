SET(SW_SERIAL_PORT
    main.c
    )

ADD_EXECUTABLE(swSerialPort.plugin
    ${SW_SERIAL_PORT}
)

ADD_CUSTOM_COMMAND(
    TARGET swSerialPort.plugin
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
            -O binary ${CMAKE_CURRENT_BINARY_DIR}/swSerialPort.plugin ${LIBRARY_OUTPUT_PATH}/swSerialPort.raw
    POST_BUILD
    COMMAND ${CMAKE_OBJDUMP}
            -D ${CMAKE_CURRENT_BINARY_DIR}/swSerialPort.plugin > ${LIBRARY_OUTPUT_PATH}/swSerialPort.dbg
    COMMAND ${READELF}
            -a ${CMAKE_CURRENT_BINARY_DIR}/swSerialPort.plugin > ${CMAKE_CURRENT_BINARY_DIR}/swSerialPort.symbol
    COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/swSerialPort.raw
)