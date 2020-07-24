INCLUDE_DIRECTORIES(
    .
    ..
    ${PROJECT_SOURCE_DIR}/sdk/include/
)

add_library(saradc STATIC
    ./saradc.c
    ./saradc_hw.c
)

