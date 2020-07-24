add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c  
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/sdk/share/wifi_mgr/ 
    ${PROJECT_SOURCE_DIR}/sdk/share/ping/
    ${PROJECT_SOURCE_DIR}/sdk/driver/non_gpl_wifi/include/
    ${PROJECT_SOURCE_DIR}/sdk/driver/non_gpl_wifi/api/wifi/
    ${PROJECT_SOURCE_DIR}/sdk/share/wifi_mgr/
    )
    
add_executable(${CMAKE_PROJECT_NAME}
    config.c
    network.c
    doorbell.h
    test_wifi_ethernet.c
	mqtt_example.c
	mdns_example.c
	sntp_example.c
	tftp_example.c
    )

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)	
