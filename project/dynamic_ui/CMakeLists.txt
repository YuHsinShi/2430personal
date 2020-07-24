set(PROJ_NAME ${CMAKE_PROJECT_NAME})

if (DEFINED CFG_BOOTLOADER_ENABLE)
    set(CMAKE_PROJECT_NAME bootloader)
    add_subdirectory(../bootloader ../bootloader)
endif()

set(CMAKE_PROJECT_NAME ${PROJ_NAME})

include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/share/audio_mgr
    )

add_executable(${CMAKE_PROJECT_NAME}
    config.c
    dynamic_ui.c
    dynamic_ui.h
    function_table.c
    main.c
    scene.c
    )

# files
file(COPY
    ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/itu/dynamic_ui.itu
    ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/dynamic_ui.ini
    DESTINATION ${CMAKE_BINARY_DIR}/data/private
    )

# font
file(COPY
    ${PROJECT_SOURCE_DIR}/data/font/${CFG_FONT_FILENAME}
    DESTINATION ${CMAKE_BINARY_DIR}/data/private/font
    )

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)


# media
file(GLOB files ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/media/*.*)
foreach (src ${files})
   string(REPLACE "${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/media/" "${CMAKE_BINARY_DIR}/data/private/" dest ${src})
   configure_file(${src} ${dest} COPYONLY)
endforeach()

# codec
if (DEFINED CFG_COMPRESS_AUDIO_PLUGIN)
    configure_file(${CMAKE_BINARY_DIR}/lib/sm32/mp3.codecs ${CMAKE_BINARY_DIR}/data/private/codec/mp3.codecs COPYONLY)
    configure_file(${CMAKE_BINARY_DIR}/lib/sm32/aac.codecs ${CMAKE_BINARY_DIR}/data/private/codec/aac.codecs COPYONLY)
    configure_file(${CMAKE_BINARY_DIR}/lib/sm32/wma.codecs ${CMAKE_BINARY_DIR}/data/private/codec/wma.codecs COPYONLY)
    configure_file(${CMAKE_BINARY_DIR}/lib/sm32/wave.codecs ${CMAKE_BINARY_DIR}/data/private/codec/wave.codecs COPYONLY)
endif()