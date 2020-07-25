project($ENV{CFG_PROJECT})
cmake_minimum_required(VERSION 3.1)
cmake_policy(SET CMP0002 OLD)
cmake_policy(SET CMP0014 OLD)

include(global_func.cmake)

if (EXISTS ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/config.cmake)
    include(project/${CMAKE_PROJECT_NAME}/config.cmake)
else()
    include(build/$ENV{CFG_BUILDPLATFORM}/${CMAKE_PROJECT_NAME}/config.cmake)
endif()

if (EXISTS ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/project.cmake)
    include(project/${CMAKE_PROJECT_NAME}/project.cmake)
endif()

include(sdk/def.cmake)
include($ENV{CFG_PLATFORM}/def.cmake)

include_directories(${PROJECT_SOURCE_DIR}/sdk/chip/it${CFG_CHIP_FAMILY}/include)
include_directories(${PROJECT_SOURCE_DIR}/sdk/include)

if ($ENV{CFG_PROJECT} STREQUAL alt_cpu
 OR $ENV{CFG_PROJECT} STREQUAL arm_lite_codec
 OR $ENV{CFG_PROJECT} STREQUAL arm_lite_dev)
    add_subdirectory(project/$ENV{CFG_PROJECT})
else()
    add_subdirectory(sdk)
    add_subdirectory($ENV{CFG_PLATFORM})
    add_subdirectory(project)
endif()
