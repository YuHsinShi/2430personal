add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
    test_nor.c
    )

target_link_libraries(${CMAKE_PROJECT_NAME}
	nor
	spi
	)

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)
