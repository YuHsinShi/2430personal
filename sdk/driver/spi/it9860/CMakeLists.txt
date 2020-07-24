if (DEFINED CFG_ARM_NOR_WRITER)
    add_definitions(
        -DARM_NOR_WRITER
    )
endif()

include_directories(
    .
    ..
    ${PROJECT_SOURCE_DIR}/sdk/include/
    ${PROJECT_SOURCE_DIR}/sdk/include/nor
)

add_library(axispi STATIC
    ./axispi/axispi_hw.c
    ./axispi/mmp_axispi.c
)
add_library(spi STATIC
    ./spi/spi_hw.c
    ./spi/mmp_spi.c
    ./spi/armNorWriterUtility.c
)
