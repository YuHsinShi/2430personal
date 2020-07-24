# Global compiler options
add_definitions(
    -D__DYNAMIC_REENT__
    -D__OPENRTOS__=1
    -D_DEFAULT_SOURCE
    -D_GNU_SOURCE
    -D_POSIX_BARRIERS
    -D_POSIX_C_SOURCE=200809L
    -D_POSIX_MONOTONIC_CLOCK
    -D_POSIX_PRIORITY_SCHEDULING=1
    -D_POSIX_READER_WRITER_LOCKS
    -D_POSIX_THREADS
    -D_POSIX_TIMERS
    -D_UNIX98_THREAD_MUTEX_ATTRIBUTES
    -DOSIP_MT
)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(
        -DENABLE_TRACE
    )
endif()

# Compiler flags
set(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib/${CFG_CPU_NAME}")

if (DEFINED CFG_DBG_OUTPUT_DEBUG_FILES)
    set(MAP_FLAGS "-Wl,-Map,objects.map -Wl,--cref")
else()
    set(MAP_FLAGS "")
endif()


if (DEFINED CFG_GCC_LTO)
    if (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7)
        set(EXTRA_FLAGS "-flto=4")
    endif()

    # For GCC 4.9 or latter version.
    # Use -ffat-lto-objects to create files which contain additionally the
    # object code. This will slow down the linking time, but it solves
    # the undefined symbol when linking.
    if (GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
        set(EXTRA_FLAGS "${EXTRA_FLAGS} -ffat-lto-objects")
    endif()

    add_definitions(
        -DCFG_GCC_LTO
    )
endif ()

# enable unwind backtrace table
if (DEFINED CFG_CPU_UNWIND)
    # For stack backtrace _Unwind_Backtrace functions
    set(EXTRA_FLAGS "${EXTRA_FLAGS} -funwind-tables -fasynchronous-unwind-tables")

    # Disable sibling call to get better backtrace results
    set(EXTRA_FLAGS "${EXTRA_FLAGS} -fno-optimize-sibling-calls")
else ()
    set(EXTRA_FLAGS "${EXTRA_FLAGS} -fno-asynchronous-unwind-tables")
endif ()

#set(EXTRA_FLAGS "${EXTRA_FLAGS} -fstack-usage -fstack-check")
#set(MAP_FLAGS "${MAP_FLAGS} -fcallgraph-info=su -fstack-usage -fstack-check")
set(EXTRA_FLAGS "${EXTRA_FLAGS} -fno-omit-frame-pointer -fsigned-char -ffunction-sections -fdata-sections")
set(EXTRA_FLAGS "${EXTRA_FLAGS} -Wno-unused-result")
if (DEFINED CFG_CPU_FA626)

    set(CMAKE_C_FLAGS_DEBUG             "${EXTRA_FLAGS} -mcpu=fa626te -fno-short-enums -Wl,--no-enum-size-warning -O0 -g3")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO    "${EXTRA_FLAGS} -mcpu=fa626te -fno-short-enums -Wl,--no-enum-size-warning -Og -g3 -DNDEBUG")
    set(CMAKE_C_FLAGS_RELEASE           "${EXTRA_FLAGS} -mcpu=fa626te -fno-short-enums -Wl,--no-enum-size-warning -O3 -g3 -DNDEBUG ")
    set(CMAKE_C_FLAGS_MINSIZEREL        "${EXTRA_FLAGS} -mcpu=fa626te -fno-short-enums -Wl,--no-enum-size-warning -Os -g3 -DNDEBUG ")

    if (GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
        set(CMAKE_CXX_FLAGS_DEBUG           "${EXTRA_FLAGS} -mcpu=fa626te -fno-short-enums -Wl,--no-enum-size-warning -O0 -g3 -std=c++11")
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${EXTRA_FLAGS} -mcpu=fa626te -fno-short-enums -Wl,--no-enum-size-warning -Og -g3 -std=c++11 -DNDEBUG")
        set(CMAKE_CXX_FLAGS_RELEASE         "${EXTRA_FLAGS} -mcpu=fa626te -fno-short-enums -Wl,--no-enum-size-warning -O3 -std=c++11 -DNDEBUG")
        set(CMAKE_CXX_FLAGS_MINSIZEREL      "${EXTRA_FLAGS} -mcpu=fa626te -fno-short-enums -Wl,--no-enum-size-warning -Os -std=c++11 -DNDEBUG")
    else()
        set(CMAKE_CXX_FLAGS_DEBUG           "${EXTRA_FLAGS} -mcpu=fa626te -fno-short-enums -Wl,--no-enum-size-warning -O0 -g3")
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${EXTRA_FLAGS} -mcpu=fa626te -fno-short-enums -Wl,--no-enum-size-warning -Og -g3 -DNDEBUG")
        set(CMAKE_CXX_FLAGS_RELEASE         "${EXTRA_FLAGS} -mcpu=fa626te -fno-short-enums -Wl,--no-enum-size-warning -O3 -DNDEBUG")
        set(CMAKE_CXX_FLAGS_MINSIZEREL      "${EXTRA_FLAGS} -mcpu=fa626te -fno-short-enums -Wl,--no-enum-size-warning -Os -DNDEBUG")
    endif()

    set(CMAKE_EXE_LINKER_FLAGS          "${EXTRA_FLAGS} -mcpu=fa626te ${MAP_FLAGS} -Wl,--gc-sections -nostartfiles -T\"${LIBRARY_OUTPUT_PATH}/default.ld\" -L\"${LIBRARY_OUTPUT_PATH}\" \"${LIBRARY_OUTPUT_PATH}/tlb.o\" \"${LIBRARY_OUTPUT_PATH}/startup.o\"")

    if (DEFINED CFG_CPU_WB)
        add_definitions(
            -DMALLOC_ALIGNMENT=32
        )
    endif()

elseif (DEFINED CFG_CPU_SM32)

    if (DEFINED CFG_CPU_LITTLE_ENDIAN)
        set(EXTRA_FLAGS "${EXTRA_FLAGS} -mlittle-endian")
    else()
        add_definitions(
            -DORTP_BIGENDIAN
        )
    endif ()

    set(CMAKE_C_FLAGS_DEBUG             "${EXTRA_FLAGS} -gstabs -fno-short-enums -g3 -O0 -DDEBUG  -mhard-div")
    set(CMAKE_CXX_FLAGS_DEBUG           "${EXTRA_FLAGS} -gstabs -fno-short-enums -g3 -O0 -DDEBUG  -mhard-div -std=c++11")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO    "${EXTRA_FLAGS} -gstabs -fno-short-enums -g3 -Og -DNDEBUG -mhard-div")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${EXTRA_FLAGS} -gstabs -fno-short-enums -g3 -Og -DNDEBUG -mhard-div -std=c++11")
    set(CMAKE_C_FLAGS_RELEASE           "${EXTRA_FLAGS} -gstabs -fno-short-enums -g3 -O3 -DNDEBUG -mhard-div")
    set(CMAKE_CXX_FLAGS_RELEASE         "${EXTRA_FLAGS} -gstabs -fno-short-enums -g3 -O3 -DNDEBUG -mhard-div -std=c++11")
    set(CMAKE_C_FLAGS_MINSIZEREL        "${EXTRA_FLAGS} -gstabs -fno-short-enums -g3 -Os -DNDEBUG -mhard-div")
    set(CMAKE_CXX_FLAGS_MINSIZEREL      "${EXTRA_FLAGS} -gstabs -fno-short-enums -g3 -Os -DNDEBUG -mhard-div -std=c++11")
    set(CMAKE_EXE_LINKER_FLAGS          "${EXTRA_FLAGS} -gstabs -nostartfiles -T\"${LIBRARY_OUTPUT_PATH}/default.ld\" -L\"${LIBRARY_OUTPUT_PATH}\" \"${LIBRARY_OUTPUT_PATH}/startup.o\"")

    enable_language(ASM)

elseif (DEFINED CFG_CPU_RISCV)

    set(EXTRA_FLAGS "${EXTRA_FLAGS} -mcpu=d25f")

    set(CMAKE_C_FLAGS_DEBUG             "${EXTRA_FLAGS} -fno-short-enums -g3 -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG           "${EXTRA_FLAGS} -fno-short-enums -g3 -O0 -DDEBUG -std=c++11")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO    "${EXTRA_FLAGS} -fno-short-enums -g3 -Og -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${EXTRA_FLAGS} -fno-short-enums -g3 -Og -DNDEBUG -std=c++11")
    set(CMAKE_C_FLAGS_RELEASE           "${EXTRA_FLAGS} -fno-short-enums -g3 -O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE         "${EXTRA_FLAGS} -fno-short-enums -g3 -O3 -DNDEBUG -std=c++11")
    set(CMAKE_C_FLAGS_MINSIZEREL        "${EXTRA_FLAGS} -fno-short-enums -g3 -Os -DNDEBUG")
    set(CMAKE_CXX_FLAGS_MINSIZEREL      "${EXTRA_FLAGS} -fno-short-enums -g3 -Os -DNDEBUG -std=c++11")
    set(CMAKE_EXE_LINKER_FLAGS          "${EXTRA_FLAGS} -Wl,--gc-sections -nostartfiles -T\"${LIBRARY_OUTPUT_PATH}/default.ld\" -L\"${LIBRARY_OUTPUT_PATH}\" \"${LIBRARY_OUTPUT_PATH}/startup.o\"")

    enable_language(ASM)

    include_directories(
        ${PROJECT_SOURCE_DIR}/$ENV{CFG_PLATFORM}/include/riscv
    )

endif()

add_definitions(
    -DCFG_OPENRTOS_HEAP_SIZE=${CFG_OPENRTOS_HEAP_SIZE}
)

ITE_ADD_DEFINITIONS_IF_DEFINED(CFG_OPENRTOS_USE_TRACE_FACILITY)
ITE_ADD_DEFINITIONS_IF_DEFINED(CFG_OPENRTOS_GENERATE_RUN_TIME_STATS)
