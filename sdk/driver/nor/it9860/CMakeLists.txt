if (DEFINED CFG_ARM_NOR_WRITER)
    add_definitions(
        -DARM_NOR_WRITER
    )
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/include/nor
)

add_library(nor STATIC
    nor.c
    nor_msg.h
)
