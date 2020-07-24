add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
    )
    
###### WIFI USB Test ########   
if (DEFINED CFG_TEST_USB_WIFI_LWIP)
    add_definitions(
        -DCFG_TEST_USB_WIFI_LWIP
        )
    add_executable(${CMAKE_PROJECT_NAME}
        test_wifi_lwip.c
        iperf_main.c
        socket_main.c
        dns_main.c
        )
endif()

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/sdk/share/wifi_mgr/
	${PROJECT_SOURCE_DIR}/sdk/share/ping/
if (DEFINED CFG_NET_HTTP) 
	${PROJECT_SOURCE_DIR}/sdk/share/iperf/
endif()
    )

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)
