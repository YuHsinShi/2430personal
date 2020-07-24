add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
)

if (CFG_CHIP_FAMILY STREQUAL 970 OR CFG_CHIP_FAMILY STREQUAL 9860)
	add_executable(${CMAKE_PROJECT_NAME}
        test_iic.c
    )
endif()

target_link_libraries(${CMAKE_PROJECT_NAME}
    iic
)

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)
