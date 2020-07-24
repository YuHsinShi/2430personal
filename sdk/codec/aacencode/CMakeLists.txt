SET(AAC_SRCS
    aac_rom.c
    aacenc.c
    aac-enc.c
    aacenc_core.c
    adj_thr.c
    band_nrg.c
    basicop2.c
    bit_cnt.c
    bitbuffer.c
    bitenc.c
    block_switch.c
    channel_map.c
    cmnMemory.c
    dyn_bits.c
    grp_data.c
    interface.c
    line_pe.c
    memalign.c
    ms_stereo.c
    oper_32b.c
    pre_echo_control.c
    psy_configuration.c
    psy_main.c
    qc_main.c
    quantize.c
    sf_estim.c
    spreading.c
    stat_bits.c
    tns.c
    transform.c
    #wavreader.c
    resample2.c
)

IF(ENABLE_CODECS_PLUGIN)
    ADD_EXECUTABLE(aacencode.plugin
        ${AAC_SRCS}
    )
  if (DEFINED CFG_ROM_COMPRESS)
		set(args ${args} -z -b 512K)
    if (CFG_CHIP_FAMILY EQUAL 9070)
        ADD_CUSTOM_COMMAND(
            TARGET aacencode.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    #-O binary ${CMAKE_CURRENT_BINARY_DIR}/aacencode.plugin ${LIBRARY_OUTPUT_PATH}/aacenc.codecs
                    -O binary -j .risc1_header -j .text -j .data ${CMAKE_CURRENT_BINARY_DIR}/aacencode.plugin ${LIBRARY_OUTPUT_PATH}/aacenc.rawcodecs
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/aac.rawcodecs
            COMMAND mkrom
            ARGS ${args} ${PROJECT_SOURCE_DIR}/sdk/target/ram/${CFG_RAM_INIT_SCRIPT} ${LIBRARY_OUTPUT_PATH}/aacenc.rawcodecs ${LIBRARY_OUTPUT_PATH}/aacenc.codecs
        )
     else ()
        ADD_CUSTOM_COMMAND(
            TARGET aacencode.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    #-O binary ${CMAKE_CURRENT_BINARY_DIR}/aacencode.plugin ${LIBRARY_OUTPUT_PATH}/aacenc.codecs
                    -O binary ${CMAKE_CURRENT_BINARY_DIR}/aacencode.plugin ${LIBRARY_OUTPUT_PATH}/aacenc.rawcodecs
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/aacenc.rawcodecs
            COMMAND mkrom
            ARGS ${args} ${PROJECT_SOURCE_DIR}/sdk/target/ram/${CFG_RAM_INIT_SCRIPT} ${LIBRARY_OUTPUT_PATH}/aacenc.rawcodecs ${LIBRARY_OUTPUT_PATH}/aacenc.codecs
        )
     endif ()
  else (DEFINED CFG_ROM_COMPRESS)
    if (CFG_CHIP_FAMILY EQUAL 9070)
        ADD_CUSTOM_COMMAND(
            TARGET aacencode.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    -O binary -j .codec_header -j .text -j .data ${CMAKE_CURRENT_BINARY_DIR}/aacencode.plugin ${LIBRARY_OUTPUT_PATH}/aacenc.codecs
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/aacenc.codecs
        )
    endif ()
  endif (DEFINED CFG_ROM_COMPRESS)   
ELSE(ENABLE_CODECS_PLUGIN)
    ADD_LIBRARY(aacencode STATIC
        ${AAC_SRCS}
    )
ENDIF(ENABLE_CODECS_PLUGIN)



