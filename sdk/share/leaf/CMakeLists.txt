add_definitions(
    -DHAVE_CONFIG_H
    -DVIDEO_ENABLED
    )

include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/include/
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg
    ${PROJECT_SOURCE_DIR}/sdk/include/mediastreamer2
    ${PROJECT_SOURCE_DIR}/sdk/share/mediastreamer2
    ${PROJECT_SOURCE_DIR}/sdk/include/leaf
    )

add_library(leaf STATIC
    leaf_mediastream.c
    )
    
if (DEFINED CFG_BUILD_ASR)
    add_library(leaf STATIC
        leaf_asr.c      
    )
endif()
    
