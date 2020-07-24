include_directories(
    .
    ..
    ${PROJECT_SOURCE_DIR}/sdk/include
    ${PROJECT_SOURCE_DIR}/sdk/include/can_bus/2517FD
)

add_library(can_bus STATIC
drv_canfdspi_api.c
drv_spi.c
can_api.c
)
