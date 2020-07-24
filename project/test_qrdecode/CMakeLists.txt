set(PROJ_NAME ${CMAKE_PROJECT_NAME})

#if (DEFINED CFG_BOOTLOADER_ENABLE)
#    set(CMAKE_PROJECT_NAME bootloader)
#    add_subdirectory(../bootloader ../bootloader)
#endif()

set(CMAKE_PROJECT_NAME ${PROJ_NAME})

include_directories(      
    ${PROJECT_SOURCE_DIR}/sdk/share/qrdecode  
    )

add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
    )

add_executable(${CMAKE_PROJECT_NAME}
    test_main.c
    )
# font
file(COPY
    ${PROJECT_SOURCE_DIR}/data/font/${CFG_FONT_FILENAME}
    DESTINATION ${CMAKE_BINARY_DIR}/data/private/font
    )
#itu
file(COPY
    ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/itu/qrdecode.itu
    DESTINATION ${CMAKE_BINARY_DIR}/data/private
    )
# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)