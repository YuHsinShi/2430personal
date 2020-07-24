#####
include_directories(      
    ${PROJECT_SOURCE_DIR}/sdk/share/qrencode  
    )
#####
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
    ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/itu/qrencode.itu
    DESTINATION ${CMAKE_BINARY_DIR}/data/private
    )

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)