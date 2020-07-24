add_executable(${CMAKE_PROJECT_NAME}
	function_table.c
	test_input.c
    )

# layers
include(layer.cmake)

# font
file(COPY
    ${PROJECT_SOURCE_DIR}/data/font
    DESTINATION ${CMAKE_BINARY_DIR}/data/private
    )

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)

# itu
file(COPY
    ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/itu/800x480/ChineseKeyboard.itu
    DESTINATION ${CMAKE_BINARY_DIR}/data/private
    )
