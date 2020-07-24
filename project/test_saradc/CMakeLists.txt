if (DEFINED CFG_TEST_ALL_XAIN)
    add_executable(${CMAKE_PROJECT_NAME}
        test_$ENV{CFG_PLATFORM}.c
        test_saradc.c
        )
elseif (DEFINED CFG_TEST_KEYPAD_16KEY)
    add_executable(${CMAKE_PROJECT_NAME}
        test_$ENV{CFG_PLATFORM}.c
        test_keypad_16.c
        )
endif()

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)
