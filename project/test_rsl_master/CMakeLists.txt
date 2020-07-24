include_directories(
    .
    ${PROJECT_SOURCE_DIR}/sdk/include/
    )

add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
    )
    
add_executable(${CMAKE_PROJECT_NAME}
    test_rsl_master.c
    )

target_link_libraries(${CMAKE_PROJECT_NAME}
	risc
	)

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)
