file(COPY
    ${PROJECT_SOURCE_DIR}/build/$ENV{CFG_BUILDPLATFORM}/arm_lite_dev/lib/fa626/opuscodec.hex
    DESTINATION ${PROJECT_SOURCE_DIR}/sdk/driver/arm_lite_dev/opuscodec/.
)

if (DEFINED CFG_ARMLITE_OPUS_CODEC AND NOT $ENV{CFG_PLATFORM} STREQUAL win32)
    add_library(opuscodec STATIC
        opuscodec.c
    )
endif()