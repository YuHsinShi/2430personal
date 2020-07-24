if (CFG_CHIP_FAMILY STREQUAL 9860)
    if (DEFINED CFG_M2D_MEMPOOL_ENABLE)
        add_definitions(
            -DCFG_M2D_MEMPOOL_ENABLE
            -DCFG_M2D_MEMPOOL_SIZE=${CFG_M2D_MEMPOOL_SIZE}
        )
    endif()

    include_directories(
    	.
    	..
        ${PROJECT_SOURCE_DIR}/sdk/include/gfx
        ${PROJECT_SOURCE_DIR}/sdk/share/nedmalloc
    )

    add_library(m2d STATIC
        it9860/driver.c
        it9860/surface.c
        it9860/hw.c
        it9860/gfx_mem.c
        it9860/gfx_math.c
    )
endif()
