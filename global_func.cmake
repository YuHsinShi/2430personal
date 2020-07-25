#
# usage example:
#
#   if there are 3 subdirectories in current directory
#       \folder_a
#       \folder_b
#       \folder_c
#
#   ITE_ADD_ALL_SUBDIRECTORIES()
#
#       equals to
#
#   add_subdirectory(folder_a)
#   add_subdirectory(folder_b)
#   add_subdirectory(folder_c)
#
function(ITE_ADD_ALL_SUBDIRECTORIES)
    # generate a list of all files that match the globbing expressions
    # and store it into the variable ${sub_dir}.
    file(GLOB sub_dir RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)

    foreach (dir ${sub_dir})
        if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir})
            add_subdirectory(${dir})
        endif()
    endforeach()
endfunction()

#
# usage example:
#
#   if there are 3 subdirectories in current directory
#       \folder_a
#       \folder_b
#       \folder_c
#
#   ITE_ADD_ALL_SUBDIRECTORIES_EXCEPT(folder_a folder_c)
#
#       equals to
#
#   add_subdirectory(folder_b)
#
function(ITE_ADD_ALL_SUBDIRECTORIES_EXCEPT excluded_dirs)
    # generate a list of all files that match the globbing expressions
    # and store it into the variable ${sub_dir}.
    file(GLOB sub_dir RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)

    if (${ARGC} GREATER 0)
        foreach (dir ${sub_dir})
            if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir})
                set(exclude false)

                foreach (ex_dir ${excluded_dirs})
                    if (${dir} STREQUAL ${ex_dir})
                        set(exclude true)
                    endif()
                endforeach()

                if (NOT exclude)
                    add_subdirectory(${dir})
                endif()
            endif()
        endforeach()
    else()
        # same as ITE_ADD_ALL_SUBDIRECTORIES()
        foreach (dir ${sub_dir})
            if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir})
                add_subdirectory(${dir})
            endif()
        endforeach()
    endif()
endfunction()

#
# usage example:
#   ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(mad)
#
#   equals to
#
#   if (DEFINED(CFG_BUILD_MAD)
#       target_link_libraries(${CMAKE_PROJECT_NAME}
#           mad
#       )
#   endif()
#
function(ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB lib)
    string(TOUPPER CFG_BUILD_${lib} cfg)
    #message("CFG:${cfg}")

    if (DEFINED ${cfg})
        #message("LINK:${cfg}")
        target_link_libraries(${CMAKE_PROJECT_NAME}
            ${lib}
        )
    endif()
endfunction()

#
# usage example:
#   ITE_LINK_LIBRARY_IF_DEFINED(CFG_BUILD_AUDIO_PREPROCESS voipdsp eigens)
#
#   equals to
#
#   if (DEFINED(CFG_BUILD_AUDIO_PREPROCESS))
#       target_link_libraries(${CMAKE_PROJECT_NAME}
#           voipdsp
#           eigens
#       )
#   endif()
#
function(ITE_LINK_LIBRARY_IF_DEFINED cfg)
    if (DEFINED ${cfg})
        foreach(lib IN LISTS ARGN)
            #message("LINK:${lib}")
            target_link_libraries(${CMAKE_PROJECT_NAME}
                ${lib}
            )
        endforeach()
    endif()
endfunction()

#
# usage example:
#   ITE_ADD_DEFINITIONS_IF_DEFINED(CFG_DOORBELL_INDOOR)
#
#   equals to
#
#   if (DEFINED(CFG_DOORBELL_INDOOR))
#       add_definitions(
#           -DCFG_DOORBELL_INDOOR
#       )
#   endif()
#
function(ITE_ADD_DEFINITIONS_IF_DEFINED def)
    if (DEFINED ${def})
        add_definitions(
            -D${def}
        )
    endif()
endfunction()

#
# usage example:
#   ITE_ADD_DEFINITIONS_IF_DEFINED_BOOL(CFG_AGC_ENABLE_SPK)
#
#   equals to
#
#   if (DEFINED CFG_AGC_ENABLE_SPK)
#       add_definitions(
#           -DCFG_AGC_ENABLE_SPK=1
#       )
#   else()
#       add_definitions(
#           -DCFG_AGC_ENABLE_SPK=0
#       )
#    endif()

function(ITE_ADD_DEFINITIONS_IF_DEFINED_BOOL def)
    if (DEFINED ${def})
        add_definitions(
            -D${def}=1
        )
    else()
        add_definitions(
            -D${def}=0
        )    
    endif()
endfunction()

#
# usage example:
#   1. ITE_ADD_DEFINITIONS_IF_DEFINED_VALUE(CFG_WIN32_LWIP_ADAPTER)
#
#   equals to
#
#   if (DEFINED(CFG_WIN32_LWIP_ADAPTER))
#       add_definitions(
#           -DCFG_WIN32_LWIP_ADAPTER=${CFG_WIN32_LWIP_ADAPTER}
#       )
#   endif()
#
#   2. ITE_ADD_DEFINITIONS_IF_DEFINED_VALUE(CFG_WIN32_LWIP_ADAPTER 0)
#   equals to
#
#   if (DEFINED(CFG_WIN32_LWIP_ADAPTER))
#       add_definitions(
#           -DCFG_WIN32_LWIP_ADAPTER=${CFG_WIN32_LWIP_ADAPTER}
#       )
#   else()
#       add_definitions(
#           -DCFG_WIN32_LWIP_ADAPTER=0
#       )
#   endif()
#
function(ITE_ADD_DEFINITIONS_IF_DEFINED_VALUE def)
    if (${ARGC} GREATER 1)
        if (DEFINED ${def})
            add_definitions(
                -D${def}=${${def}}
            )
        else()
            add_definitions(
                -D${def}=${ARGV1}
            )
        endif()
        #message("==2=>${def} ${${def}} ${ARGV1}")
    else()
        #message("====>${def} ${${def}}")
        if (DEFINED ${def})
            add_definitions(
                -D${def}=${${def}}
            )
        endif()
    endif()
endfunction()


#
# usage example:
#   1. ITE_ADD_DEFINITIONS_IF_DEFINED_VALUE(CFG_WIN32_LWIP_ADAPTER)
#
#   equals to
#
#   if (DEFINED(CFG_NET_WIFI_MP_SSID))
#       add_definitions(
#           -DCFG_NET_WIFI_MP_SSID="${CFG_NET_WIFI_MP_SSID}"
#       )
#   endif()
#
#   2. ITE_ADD_DEFINITIONS_IF_DEFINED_VALUE(CFG_NET_WIFI_MP_SSID 0)
#   equals to
#
#   if (DEFINED(CFG_NET_WIFI_MP_SSID))
#       add_definitions(
#           -DCFG_NET_WIFI_MP_SSID="${CFG_NET_WIFI_MP_SSID}"
#       )
#   else()
#       add_definitions(
#           -DCFG_NET_WIFI_MP_SSID="0"
#       )
#   endif()
#
function(ITE_ADD_DEFINITIONS_IF_DEFINED_STRING_VALUE def)
    if (${ARGC} GREATER 1)
        if (DEFINED ${def})
            add_definitions(
                -D${def}="${${def}}"
            )
        else()
            add_definitions(
                -D${def}="${ARGV1}"
            )
        endif()
        #message("==2=>${def} ${${def}} ${ARGV1}")
    else()
        #message("====>${def} ${${def}}")
        if (DEFINED ${def})
            add_definitions(
                -D${def}="${${def}}"
            )
        endif()
    endif()
endfunction()

#
# This function is used to copy files to the output executable
# directory in the MSVC environment.
#
# usage example:
#   ITE_MSVC_COPY_TO_OUTPUT_BINARY_DIR(a.dll b.dll)
#
function(ITE_MSVC_COPY_TO_OUTPUT_BINARY_DIR in_files)
    foreach(in_file ${in_files})
        add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD  # Adds a post-build event to ${CMAKE_PROJECT_NAME}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different           # which executes "cmake - E copy_if_different..."
            "${in_file}"                                            # <--this is input file
            $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>)               # <--this is output file path
    endforeach()
endfunction()