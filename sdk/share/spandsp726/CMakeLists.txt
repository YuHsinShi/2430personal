include_directories(
    .
    ${PROJECT_SOURCE_DIR}/sdk/include/
    )
    
add_library(spandsp726 STATIC
    alloc.c
    bit_operations.c
    bitstream.c
    g726.c
    )
