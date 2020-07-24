add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
    test_canbus_9860.c
    )

include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/include/
)

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)
