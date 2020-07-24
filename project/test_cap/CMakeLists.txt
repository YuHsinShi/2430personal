include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/include/
    ${PROJECT_SOURCE_DIR}/sdk/include/capture/capture_9860
)
add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
    )
if (DEFINED CFG_CAP_MODULE_MODE)
add_executable(${CMAKE_PROJECT_NAME}
    test_modules.c
    test_itv.c
    )
endif()

target_link_libraries(${CMAKE_PROJECT_NAME}
)
# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)
