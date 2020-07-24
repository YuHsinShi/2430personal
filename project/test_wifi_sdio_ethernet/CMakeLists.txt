add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
    )
	
INCLUDE_DIRECTORIES(
	${PROJECT_SOURCE_DIR}/project/test_wifi_sdio_ethernet
)
    
if (DEFINED CFG_TEST_SDIO_WIFI_LWIP)
    ###### WIFI SDIO Test ########   
    add_definitions(
        -DCFG_TEST_SDIO_WIFI_LWIP
        )

    add_executable(${CMAKE_PROJECT_NAME}
		test_wifi_ngpl_lwip.c
		iperf_main.c
		socket_main.c
        )

elseif(DEFINED CFG_TEST_USB_WIFI_LWIP)
    ###### WIFI USB Test ########
    add_definitions(
        -DCFG_TEST_USB_WIFI_LWIP
        )
		
    add_executable(${CMAKE_PROJECT_NAME}
		test_wifi_usb_lwip.c
		iperf_main.c
		socket_main.c
        )
		
endif()

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/sdk/driver/non_gpl_wifi/include/
    ${PROJECT_SOURCE_DIR}/sdk/driver/non_gpl_wifi/api/wifi/ 
	${PROJECT_SOURCE_DIR}/sdk/share/wifi_mgr/ 
	${PROJECT_SOURCE_DIR}/sdk/share/ping/
if (DEFINED CFG_NET_HTTP) 
	${PROJECT_SOURCE_DIR}/sdk/share/iperf/
endif()
    )

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)
