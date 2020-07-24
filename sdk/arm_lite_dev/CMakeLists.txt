file(GLOB sub_dir RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)

foreach (dir ${sub_dir})
    if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir})
        add_subdirectory(${dir})
    endif()

endforeach()