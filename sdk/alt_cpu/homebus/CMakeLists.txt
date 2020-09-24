SET(HOMEBUS
    main.c
    )

ADD_EXECUTABLE(homebus.plugin
    ${HOMEBUS}
)

ADD_CUSTOM_COMMAND(
    TARGET homebus.plugin
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
            -O binary ${CMAKE_CURRENT_BINARY_DIR}/homebus.plugin ${LIBRARY_OUTPUT_PATH}/homebus.raw
    POST_BUILD
    COMMAND ${CMAKE_OBJDUMP}
            -D ${CMAKE_CURRENT_BINARY_DIR}/homebus.plugin > ${LIBRARY_OUTPUT_PATH}/homebus.dbg
    COMMAND ${READELF}
            -a ${CMAKE_CURRENT_BINARY_DIR}/homebus.plugin > ${CMAKE_CURRENT_BINARY_DIR}/homebus.symbol
    COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/homebus.raw
)