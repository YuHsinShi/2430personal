add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c  
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/sdk/share/wifi_mgr/ 
    ${PROJECT_SOURCE_DIR}/sdk/share/ping/
    )
    
add_executable(${CMAKE_PROJECT_NAME}
    config.c
    network.c
    doorbell.h
    test_send_email.c
    )

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)	
