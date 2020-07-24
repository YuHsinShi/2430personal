SET(OLED_CTRL_SRC
    main.c
    )

ADD_EXECUTABLE(oledCtrl.plugin
    ${OLED_CTRL_SRC}
)

ADD_CUSTOM_COMMAND(
    TARGET oledCtrl.plugin
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
            -O binary ${CMAKE_CURRENT_BINARY_DIR}/oledCtrl.plugin ${LIBRARY_OUTPUT_PATH}/oledCtrl.raw
    POST_BUILD
    COMMAND ${CMAKE_OBJDUMP}
            -D ${CMAKE_CURRENT_BINARY_DIR}/oledCtrl.plugin > ${LIBRARY_OUTPUT_PATH}/oledCtrl.dbg
    COMMAND ${READELF}
            -a ${CMAKE_CURRENT_BINARY_DIR}/oledCtrl.plugin > ${CMAKE_CURRENT_BINARY_DIR}/oledCtrl.symbol
    COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/oledCtrl.raw
)