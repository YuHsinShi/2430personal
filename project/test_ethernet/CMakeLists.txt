add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
    )
    
 ###### Ethernet Test ########   
if (DEFINED CFG_TEST_ETHERNET_LWIP)
    add_definitions(
        -DCFG_TEST_ETHERNET_LWIP
        )
    add_executable(${CMAKE_PROJECT_NAME}
        test_ethernet_lwip.c
		iperf_main.c
        socket_main.c
		dns_main.c
        )
endif()

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/sdk/share/ping/
if (DEFINED CFG_NET_HTTP) 
	${PROJECT_SOURCE_DIR}/sdk/share/iperf/
endif()
    )

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)
