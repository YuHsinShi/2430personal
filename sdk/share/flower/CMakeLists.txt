if (DEFINED CFG_GCC_LTO)
    add_definitions("-fno-lto")
endif ()

include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/share/flower/
    ${PROJECT_SOURCE_DIR}/sdk/include/
)

add_library(flower STATIC
    filter_chain.c
    flower.h
    example/filterA.c
    example/filterB.c
    example/filterC.c
    example/filterD.c
    example/filterE.c
    example/filterF.c
    filter_des.c
    ite_queue.c
    ite_queue.h
    filter_capture.c
)
