include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg/libavcodec
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg/libavformat
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg/libavdevice
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg/libavutil
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg/libswresample
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg/libswscale
    ${PROJECT_SOURCE_DIR}/sdk/driver/itv
)

add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
    filelist.h
    filelist.c
)

if (DEFINED CFG_TEST_ELEMENTARY)
    add_executable(${CMAKE_PROJECT_NAME}
        test_elementary.c
    )
elseif (DEFINED CFG_TEST_CONTAINER)
		add_executable(${CMAKE_PROJECT_NAME}
				test_player_970.c
		)
elseif (DEFINED CFG_TEST_MJPEG)
    add_executable(${CMAKE_PROJECT_NAME}
        test_mjpeg.c
    )
endif()

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)
