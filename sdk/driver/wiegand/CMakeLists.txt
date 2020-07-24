include_directories(
	.
	..
    ${PROJECT_SOURCE_DIR}/sdk/include/wiegand
    )

add_library(wiegand STATIC
    wiegand.c
	wiegandout.c
    )
