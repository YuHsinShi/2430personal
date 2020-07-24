include_directories(
    .
    ..
    ${PROJECT_SOURCE_DIR}/sdk/include
    ${PROJECT_SOURCE_DIR}/sdk/include/capture/capture_9860
)

add_library(capture STATIC
    mmp_capture.c
    capture_hw.c
    capture_hw.h
    capture_config.h
    capture_reg.h
    capture_util.h
    capture.c
    capture.h
    cap_util.c
    device_modules.c
)

if(NOT DEFINED CFG_CAPTURE_DEFINE)
   set(CFG_CAPTURE_DEFINE "DEMO_9860_DEFAULT.txt")
endif()

configure_file(${PROJECT_SOURCE_DIR}/sdk/target/defcap/${CFG_CAPTURE_DEFINE} ${CMAKE_CURRENT_BINARY_DIR}/defcap_config.h)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)