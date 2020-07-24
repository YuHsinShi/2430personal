if (CMAKE_BUILD_TYPE STREQUAL Debug)
    ADD_DEFINITIONS(
        -DDEBUG
    )
endif()

if (CFG_CHIP_FAMILY STREQUAL 9860)
    add_subdirectory(it9860)
endif()
