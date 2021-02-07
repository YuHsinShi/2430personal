add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
    )
 
add_executable(${CMAKE_PROJECT_NAME}
    test_esp32.c
    )

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)
