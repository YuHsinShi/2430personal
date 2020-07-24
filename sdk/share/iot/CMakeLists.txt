include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/include/
    ${PROJECT_SOURCE_DIR}/sdk/include/iot
    )

add_library(iot STATIC
    iot_socket.c
    iot_system.c
    iot_wlan.c
    protocol_wrapper.c
    airoha_iot_bus.c
    )
