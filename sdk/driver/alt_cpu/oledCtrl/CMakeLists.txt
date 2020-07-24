file(COPY
    ${PROJECT_SOURCE_DIR}/build/$ENV{CFG_BUILDPLATFORM}/alt_cpu/lib/sm32/oledCtrl.hex
    DESTINATION ${PROJECT_SOURCE_DIR}/sdk/driver/alt_cpu/oledCtrl/.
)

if (DEFINED CFG_OLED_CTRL AND NOT $ENV{CFG_PLATFORM} STREQUAL win32)
    add_library(oledCtrl STATIC
        oledCtrl.c
    )
endif()