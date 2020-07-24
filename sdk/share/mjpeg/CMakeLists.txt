include_directories(
		.
    ${PROJECT_SOURCE_DIR}/sdk/driver/itv
    )
    
add_library(mjpeg STATIC
    mjpeg_player.h
    mjpeg_player.c
    )
