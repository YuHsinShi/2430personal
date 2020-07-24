add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
    )

include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/share/ping/
    )
 
if (DEFINED CFG_TEST_PING)
    add_definitions(
        -DCFG_TEST_PING
        -DCFG_PING_IP="${CFG_PING_IP}"
    )
elseif (DEFINED CFG_TEST_HTTP)
    add_definitions(
        -DCFG_TEST_HTTP
        -DCFG_HTTP_URL="${CFG_HTTP_URL}"
    )
endif()    

add_executable(${CMAKE_PROJECT_NAME}
    test_ecm.c
    test_dial_io_mode.c
    )

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)
