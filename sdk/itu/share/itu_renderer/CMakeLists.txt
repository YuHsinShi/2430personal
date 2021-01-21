if (DEFINED CFG_ITU_ERR)
    add_definitions(
        -DCFG_ITU_ERR
   )
endif()

if (DEFINED CFG_ITU_WARN)
    add_definitions(
        -DCFG_ITU_WARN
   )
endif()

if (DEFINED CFG_ITU_INFO)
    add_definitions(
        -DCFG_ITU_INFO
    )
endif()

if (DEFINED CFG_ITU_DBG)
    add_definitions(
        -DCFG_ITU_DBG
    )
endif()

if (DEFINED CFG_ITU_LOAD)
    add_definitions(
        -DCFG_ITU_LOAD
    )
endif()

if (DEFINED CFG_ITU_UPDATE)
    add_definitions(
        -DCFG_ITU_UPDATE
    )
endif()

if (DEFINED CFG_ITU_DRAW)
    add_definitions(
        -DCFG_ITU_DRAW
    )
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/itu/share/itu_renderer
)

add_library(itu_renderer STATIC
    ${PROJECT_SOURCE_DIR}/sdk/itu/include/ite/itu.h
    itu_base.c
    itu_cfg.h
    itu_surface.c
    itu_utility.c
)

if (DEFINED CFG_CHIP_FAMILY_IT970)
    set(dirname it970)
elseif (DEFINED CFG_CHIP_FAMILY_IT9860)
    set(dirname it9860)
endif()

if (DEFINED CFG_VIDEO_ENABLE)
    include_directories(
        ${PROJECT_SOURCE_DIR}/sdk/driver/itv
    )
    if (CFG_VIDEO_FRAME_FUN_ENABLE)
        add_library(itu_renderer STATIC
            ${dirname}/itu_framefunc_ff.c
        )
    else()
        add_library(itu_renderer STATIC
            ${dirname}/itu_framefunc.c
        )
    endif()
endif()

if (DEFINED CFG_LCD_ENABLE)
    add_library(itu_renderer STATIC
        ${dirname}/itu_lcd.c
    )
endif()

if (DEFINED CFG_M2D_ENABLE)
    add_library(itu_renderer STATIC
        ${dirname}/itu_m2d.c
        ${dirname}/itu_private.h
    )
else()
    add_library(itu_renderer STATIC
        itu_sw.c
    )
endif()

if (DEFINED CFG_ITV_VP_HIGH_QUALITY)
    add_library(itu_renderer STATIC
        ${dirname}/itu_timer_ff.c
    )
endif()

if (DEFINED CFG_STNLCD_ENABLE)
    add_library(itu_renderer STATIC
        itu_stnlcd.c
   )
endif()
