if (NOT $ENV{CFG_PLATFORM} STREQUAL win32)
INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/sdk/include/
    ${CMAKE_BINARY_DIR}/lib/sm32
)

    if (CFG_CHIP_FAMILY EQUAL 9860)
        add_library(risc STATIC
            it9860/ite_risc.c
        )
    else()
        add_library(risc STATIC
            it970/ite_risc.c
        )
    endif()
endif()
