include_directories(
    .
    ..
    ${PROJECT_SOURCE_DIR}/sdk/include/iic
)

add_library(iic STATIC
    iic.c
)
