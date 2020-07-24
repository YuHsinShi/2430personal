ITE_ADD_DEFINITIONS_IF_DEFINED(CFG_ITH_ERR)
ITE_ADD_DEFINITIONS_IF_DEFINED(CFG_ITH_WARN)
ITE_ADD_DEFINITIONS_IF_DEFINED(CFG_ITH_INFO)
ITE_ADD_DEFINITIONS_IF_DEFINED(CFG_ITH_DBG)

if (DEFINED CFG_ITH_FPGA)
    add_definitions(
        -DCFG_ITH_FPGA
        -DCFG_ITH_FPGA_CLK_CPU=${CFG_ITH_FPGA_CLK_CPU}
        -DCFG_ITH_FPGA_CLK_MEM=${CFG_ITH_FPGA_CLK_MEM}
        -DCFG_ITH_FPGA_CLK_BUS=${CFG_ITH_FPGA_CLK_BUS}
    )
endif()

ITE_ADD_DEFINITIONS_IF_DEFINED(CFG_BUILD_MEDIASTREAMER2)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/$ENV{CFG_PLATFORM}/include
)

add_library(ith STATIC
    ${PROJECT_SOURCE_DIR}/sdk/include/ith/ith_generic.h
    ith_cfg.h
    ith_chipid.c
    ith_clock.c
    ith_isp.c
    ith_dma.c
    ith_host.c
    ith_interrupt.c
    ith_interrupt_handler.c
    ith_printf.c
    ith_pwm.c
    ith_timer.c
    ith_utility.c
    #ith_codec.c
)

include_directories(
    ${CMAKE_BINARY_DIR}/lib/sm32
)

if (DEFINED CFG_CMDQ_ENABLE)
    add_library(ith STATIC
        it${CFG_CHIP_FAMILY}/ith_cmdq.c
        it${CFG_CHIP_FAMILY}/ith_cmdq.h
    )
endif()

if (DEFINED CFG_KEYPAD_ENABLE)
    add_library(ith STATIC
        ith_keypad.c
    )
endif()

if (DEFINED CFG_LCD_ENABLE)
    add_library(ith STATIC
        it${CFG_CHIP_FAMILY}/ith_lcd.c
        it${CFG_CHIP_FAMILY}/ith_lcd.h
    )
endif()

if (DEFINED CFG_RTC_ENABLE)
    add_library(ith STATIC
        ith_rtc.c
    )
endif()

if (CFG_CHIP_FAMILY STREQUAL 970 OR CFG_CHIP_FAMILY STREQUAL 9860)
    add_library(ith STATIC
        it970/ith_gpio.c
        it970/ith_gpio_handler.c
    )
endif()

if (DEFINED CFG_UART0_ENABLE OR DEFINED CFG_UART1_ENABLE OR DEFINED CFG_UART2_ENABLE OR DEFINED CFG_UART3_ENABLE OR DEFINED CFG_UART4_ENABLE OR DEFINED CFG_UART5_ENABLE OR DEFINED CFG_RS485_0_ENABLE OR DEFINED CFG_RS485_1_ENABLE OR DEFINED CFG_RS485_2_ENABLE OR DEFINED CFG_RS485_3_ENABLE OR DEFINED CFG_RS485_4_ENABLE OR DEFINED CFG_RS485_5_ENABLE)
    add_library(ith STATIC
        ith_uart.c
    )
endif()

if (DEFINED CFG_TVOUT_ENABLE)
    add_library(ith STATIC
        ith_tve.c
    )
endif()

if (DEFINED CFG_VIDEO_ENABLE)
    add_library(ith STATIC
        #${PROJECT_SOURCE_DIR}/sdk/include/ith/ith_vregs.h
        ${PROJECT_SOURCE_DIR}/sdk/include/ith/ith_video.h
        ith_video.c
    )
endif()

if (DEFINED CFG_JPEG_HW_ENABLE)
    add_library(ith STATIC
        ith_jpeg.c
    )
endif()

if (DEFINED CFG_UI_ENC_ENABLE)
    add_library(ith STATIC
        ith_uiEnc.c
    )
endif()

if (DEFINED CFG_SD0_ENABLE OR DEFINED CFG_SD1_ENABLE OR DEFINED CFG_CF_ENABLE OR DEFINED CFG_MS_ENABLE OR DEFINED CFG_XD_ENABLE OR DEFINED CFG_NOR_ENABLE OR DEFINED CFG_NAND_ENABLE)
    add_library(ith STATIC
        ith_card.c
    )
endif()

if (DEFINED CFG_USB0_ENABLE OR DEFINED CFG_USB1_ENABLE OR DEFINED CFG_USBHCC)
    add_library(ith STATIC
        ith_usb.c
    )
endif()

if (DEFINED CFG_WATCHDOG_ENABLE)
    add_library(ith STATIC
        ith_watchdog.c
    )
endif()

if (DEFINED CFG_DCPS_ENABLE)
    add_library(ith STATIC
        ith_dcps.c
    )
endif()

if (DEFINED CFG_DPU_ENABLE)
    if (CFG_CHIP_FAMILY STREQUAL 970)
        add_library(ith STATIC
            it970/ith_dpu.c
        )
    endif()
    if (CFG_CHIP_FAMILY STREQUAL 9860)
        add_library(ith STATIC
            it9860/ith_dpu.c
        )
    endif()
endif()

if (DEFINED CFG_NAND_ENABLE)
    add_library(ith STATIC
        ith_nand.c
    )
endif()

if (DEFINED CFG_XD_ENABLE)
    add_library(ith STATIC
        ith_xd.c
    )
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/driver/ith
    ${PROJECT_SOURCE_DIR}/sdk/driver/ith/it${CFG_CHIP_FAMILY}
)

if (DEFINED CFG_IR0_ENABLE OR DEFINED CFG_IR1_ENABLE OR DEFINED CFG_IR2_ENABLE OR DEFINED CFG_IR3_ENABLE)
    add_library(ith STATIC
        ith_ir.c
    )
endif()

if (DEFINED CFG_SARADC_ENABLE)
    if (CFG_CHIP_FAMILY STREQUAL 970 OR CFG_CHIP_FAMILY STREQUAL 9860)
        add_library(ith STATIC
            it${CFG_CHIP_FAMILY}/ith_saradc.c
        )
    endif()
endif()

if (DEFINED CFG_WATCHDOG_ENABLE)
    add_definitions(
        -DCFG_WATCHDOG_ENABLE
    )
endif()

if (DEFINED CFG_CPU_WB OR CFG_WT_SIZE STRGREATER 0 OR $ENV{CFG_PLATFORM} STREQUAL win32)
    add_library(ith STATIC
        ith_vmem.c
    )
endif()

if ($ENV{CFG_PLATFORM} STREQUAL win32)

    execute_process(COMMAND mkscript ${PROJECT_SOURCE_DIR}/sdk/target/debug/${CFG_DBG_INIT_SCRIPT} ${CMAKE_CURRENT_BINARY_DIR}/ram.inc)

    add_library(ith_platform STATIC
        ith_win32.c
    )

    if (DEFINED CFG_WIN32_SIMULATOR)
        add_library(ith_platform STATIC
            ith_win32_sim.c
        )
    else()
        add_library(ith_platform STATIC
            ith_win32_spi.c
        )
    endif()

else()

    add_library(ith_platform STATIC
        ith_openrtos.c
    )

endif()
