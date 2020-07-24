SET(PATTERN_GEN_SRC
    main.c
    )

ADD_EXECUTABLE(patternGen.plugin
    ${PATTERN_GEN_SRC}
)

ADD_CUSTOM_COMMAND(
    TARGET patternGen.plugin
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
            -O binary ${CMAKE_CURRENT_BINARY_DIR}/patternGen.plugin ${LIBRARY_OUTPUT_PATH}/patternGen.raw
    POST_BUILD
    COMMAND ${CMAKE_OBJDUMP}
            -D ${CMAKE_CURRENT_BINARY_DIR}/patternGen.plugin > ${LIBRARY_OUTPUT_PATH}/patternGen.dbg
    COMMAND ${READELF}
            -a ${CMAKE_CURRENT_BINARY_DIR}/patternGen.plugin > ${CMAKE_CURRENT_BINARY_DIR}/patternGen.symbol
    COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/patternGen.raw
)