INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/sdk/include
    )

SET(MP2_SRCS
    main.c
    mpegaudio.c
    mpegaudioenc.c
    resample2.c
)

IF(ENABLE_CODECS_PLUGIN)
    ADD_EXECUTABLE(mp2encode.plugin
        ${MP2_SRCS}
    )
  if (DEFINED CFG_ROM_COMPRESS)    
	  set(args ${args} -z -b 512K)
    if (CFG_CHIP_FAMILY EQUAL 9070)
        ADD_CUSTOM_COMMAND(
            TARGET mp2encode.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    #-O binary ${CMAKE_CURRENT_BINARY_DIR}/mp2encode.plugin ${LIBRARY_OUTPUT_PATH}/mp2enc.codecs
                    -O binary -j .risc1_header -j .text -j .data ${CMAKE_CURRENT_BINARY_DIR}/mp2encode.plugin ${LIBRARY_OUTPUT_PATH}/mp2enc.rawcodecs
            COMMAND ${READELF} -a ${CMAKE_CURRENT_BINARY_DIR}/mp2encode.plugin > ${LIBRARY_OUTPUT_PATH}/mp2enc.symbol
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/mp2enc.rawcodecs
            COMMAND mkrom
            ARGS ${args} ${PROJECT_SOURCE_DIR}/sdk/target/ram/${CFG_RAM_INIT_SCRIPT} ${LIBRARY_OUTPUT_PATH}/mp2enc.rawcodecs ${LIBRARY_OUTPUT_PATH}/mp2enc.codecs
        )
     else ()
        ADD_CUSTOM_COMMAND(
            TARGET mp2encode.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    #-O binary ${CMAKE_CURRENT_BINARY_DIR}/mp2encode.plugin ${LIBRARY_OUTPUT_PATH}/mp2enc.codecs
                    -O binary ${CMAKE_CURRENT_BINARY_DIR}/mp2encode.plugin ${LIBRARY_OUTPUT_PATH}/mp2enc.rawcodecs
            COMMAND ${READELF} -a ${CMAKE_CURRENT_BINARY_DIR}/mp2encode.plugin > ${LIBRARY_OUTPUT_PATH}/mp2enc.symbol
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/mp2enc.rawcodecs
            COMMAND mkrom
            ARGS ${args} ${PROJECT_SOURCE_DIR}/sdk/target/ram/${CFG_RAM_INIT_SCRIPT} ${LIBRARY_OUTPUT_PATH}/mp2enc.rawcodecs ${LIBRARY_OUTPUT_PATH}/mp2enc.codecs
        )
     endif ()
  else (DEFINED CFG_ROM_COMPRESS)
    if (DEFINED CFG_CHIP_PKG_IT9070)
        ADD_CUSTOM_COMMAND(
            TARGET mp2encode.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    -O binary -j .codec_header -j .text -j .data ${CMAKE_CURRENT_BINARY_DIR}/mp2encode.plugin ${LIBRARY_OUTPUT_PATH}/mp2enc.codecs
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/mp2enc.codecs
        )
    endif ()
  endif (DEFINED CFG_ROM_COMPRESS)
ELSE(ENABLE_CODECS_PLUGIN)
    ADD_LIBRARY(mp2encode STATIC
        ${MP2_SRCS}
    )
ENDIF(ENABLE_CODECS_PLUGIN)



