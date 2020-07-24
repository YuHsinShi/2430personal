file(MAKE_DIRECTORY
    ${CMAKE_BINARY_DIR}/lib/${CFG_CPU_NAME}
)

set(DATACONV dataconv)
SET(ENABLE_CODECS_PLUGIN 1)
add_definitions(
    -DENABLE_CODECS_PLUGIN=1
)

if (DEFINED CFG_GCC_LTO)
    if (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7)
        set(CODEC_FLAGS "-flto")
    endif()

    # For GCC 4.9 or latter version.
    # Use -ffat-lto-objects to create files which contain additionally the
    # object code. This will slow down the linking time, but it solves
    # the undefined symbol when linking.
    if (GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
        set(CODEC_FLAGS "${CODEC_FLAGS} -ffat-lto-objects")
    endif()

    add_definitions(
        -DCFG_GCC_LTO
    )
    message("<<enable LTO>>")
endif()
set(CODEC_FLAGS "${CODEC_FLAGS} -fsigned-char")
set(CODEC_FLAGS "${CODEC_FLAGS} -Wno-unused-result")

set(armLite_LD "default.ld")
set(CMAKE_C_FLAGS_DEBUG             "${CODEC_FLAGS} -mcpu=fa626te -fno-short-enums -Wl,--no-enum-size-warning -O3 -g3")
set(CMAKE_CXX_FLAGS_DEBUG           "${CODEC_FLAGS} -mcpu=fa626te -fno-short-enums -Wl,--no-enum-size-warning -O3 -g3 -std=c++11")
set(CMAKE_C_FLAGS_RELWITHDEBINFO    "${CODEC_FLAGS} -mcpu=fa626te -fno-short-enums -Wl,--no-enum-size-warning -O3 -g3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${CODEC_FLAGS} -mcpu=fa626te -fno-short-enums -Wl,--no-enum-size-warning -O3 -g3 -std=c++11 -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE           "${CODEC_FLAGS} -mcpu=fa626te -fno-short-enums -Wl,--no-enum-size-warning -O3 -g3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE         "${CODEC_FLAGS} -mcpu=fa626te -fno-short-enums -Wl,--no-enum-size-warning -O3 -std=c++11 -DNDEBUG")
set(CMAKE_C_FLAGS_MINSIZEREL        "${CODEC_FLAGS} -mcpu=fa626te -fno-short-enums -Wl,--no-enum-size-warning -O3 -g3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_MINSIZEREL      "${CODEC_FLAGS} -mcpu=fa626te -fno-short-enums -Wl,--no-enum-size-warning -O3 -std=c++11 -DNDEBUG")

set(CMAKE_EXE_LINKER_FLAGS  "${CODEC_FLAGS} -gstabs -nostartfiles -T\"${LIBRARY_OUTPUT_PATH}/${armLite_LD}\" \"${LIBRARY_OUTPUT_PATH}/armLite_tlb.o\" \"${LIBRARY_OUTPUT_PATH}/arm_crt0.o\"  \"${LIBRARY_OUTPUT_PATH}/armLite_gpio.o\" \"${LIBRARY_OUTPUT_PATH}/armLite_ith_utility.o\" \"${LIBRARY_OUTPUT_PATH}/armLite_main.o\"")

get_directory_property(defs COMPILE_DEFINITIONS)

foreach (def ${defs})
    set(defargs ${defargs} -D${def})
endforeach()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(defargs ${defargs} -g)
endif()

add_custom_target(
    armLite_tlb.o ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/armLite_tlb.o
)

## Build gpio.c
add_custom_command(
    OUTPUT ${LIBRARY_OUTPUT_PATH}/armLite_gpio.o
    COMMAND ${CMAKE_C_COMPILER}
        ${CMAKE_C_COMPILER_ARG1}
        ${CMAKE_CUSTOM_C_FLAGS}
        ${defargs}
        -I${PROJECT_SOURCE_DIR}/sdk/include
        -c
        -o ${LIBRARY_OUTPUT_PATH}/armLite_gpio.o
        ${CMAKE_SOURCE_DIR}/project/arm_lite_codec/gpio.c
    DEPENDS ${CMAKE_SOURCE_DIR}/project/arm_lite_codec/gpio.c
)
add_custom_target(
    armLite_gpio ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/armLite_gpio.o
)

## Build ith_utility.c
add_custom_command(
    OUTPUT ${LIBRARY_OUTPUT_PATH}/armLite_ith_utility.o
    COMMAND ${CMAKE_C_COMPILER}
        ${CMAKE_C_COMPILER_ARG1}
        ${CMAKE_CUSTOM_C_FLAGS}
        ${defargs}
        -I${PROJECT_SOURCE_DIR}/sdk/include
        -c
        -o ${LIBRARY_OUTPUT_PATH}/armLite_ith_utility.o
        ${CMAKE_SOURCE_DIR}/project/arm_lite_codec/ith_utility.c
    DEPENDS ${CMAKE_SOURCE_DIR}/project/arm_lite_codec/ith_utility.c
)
add_custom_target(
    armLite_ith_utility ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/armLite_ith_utility.o
)

## Build main.c
add_custom_command(
    OUTPUT ${LIBRARY_OUTPUT_PATH}/armLite_main.o
    COMMAND ${CMAKE_C_COMPILER}
        ${CMAKE_C_COMPILER_ARG1}
        ${CMAKE_CUSTOM_C_FLAGS}
        ${defargs}        
        -I${PROJECT_SOURCE_DIR}/sdk/include
        -c
        -o ${LIBRARY_OUTPUT_PATH}/armLite_main.o
        ${CMAKE_SOURCE_DIR}/project/arm_lite_codec/main.c
    DEPENDS ${CMAKE_SOURCE_DIR}/project/arm_lite_codec/main.c
)

add_custom_target(
    armLite_main ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/armLite_main.o
)

## Build arm_crt0.S
ADD_CUSTOM_COMMAND(
    OUTPUT ${LIBRARY_OUTPUT_PATH}/arm_crt0.o
    COMMAND ${CMAKE_C_COMPILER}
        ${CMAKE_C_COMPILER_ARG1}
        ${CMAKE_CUSTOM_C_FLAGS}
        ${defargs}        
        -c
        -o ${LIBRARY_OUTPUT_PATH}/arm_crt0.o
        ${CMAKE_SOURCE_DIR}/project/arm_lite_codec/arm_crt0.S
    DEPENDS ${CMAKE_SOURCE_DIR}/project/arm_lite_codec/arm_crt0.S
)

add_custom_target(
    arm_crt0 ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/arm_crt0.o
)

ADD_CUSTOM_TARGET(
    armLite_tlb ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/armLite_tlb.o
)

    add_custom_command(
        OUTPUT ${LIBRARY_OUTPUT_PATH}/armLite_tlb.o
        COMMAND ${CMAKE_C_COMPILER}
            ${CMAKE_C_COMPILER_ARG1}
            ${defargs}
            -c
            -o ${LIBRARY_OUTPUT_PATH}/armLite_tlb.o
            ${CMAKE_SOURCE_DIR}/project/arm_lite_codec/tlb.c
        DEPENDS ${CMAKE_SOURCE_DIR}/project/arm_lite_codec/tlb.c
    )

## Build default.ld
add_custom_command(
    OUTPUT ${LIBRARY_OUTPUT_PATH}/default.ld
    COMMAND ${CPP}
        ${CMAKE_CUSTOM_C_FLAGS}
        -I${PROJECT_SOURCE_DIR}/sdk/include
        -E
        -P
        -o ${LIBRARY_OUTPUT_PATH}/default.ld
        ${CMAKE_SOURCE_DIR}/project/arm_lite_codec/default.lds
    DEPENDS ${CMAKE_SOURCE_DIR}/project/arm_lite_codec/default.lds
)

add_custom_target(
    default.ld ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/default.ld
)

add_subdirectory(${CMAKE_SOURCE_DIR}/sdk/codec ${CMAKE_BINARY_DIR}/sdk/codec)
