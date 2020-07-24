if (DEFINED CFG_WATCHDOG_ENABLE)
    add_definitions(
        -DCFG_WATCHDOG_ENABLE
        -DCFG_WATCHDOG_TIMEOUT=${CFG_WATCHDOG_TIMEOUT}
        -DCFG_WATCHDOG_REFRESH_INTERVAL=${CFG_WATCHDOG_REFRESH_INTERVAL}
    )

    ITE_ADD_DEFINITIONS_IF_DEFINED(CFG_WATCHDOG_INTR)
    ITE_ADD_DEFINITIONS_IF_DEFINED(CFG_WATCHDOG_IDLETASK)
endif()

ITE_ADD_DEFINITIONS_IF_DEFINED(CFG_ITP_ERR)
ITE_ADD_DEFINITIONS_IF_DEFINED(CFG_ITP_WARN)
ITE_ADD_DEFINITIONS_IF_DEFINED(CFG_ITP_INFO)
ITE_ADD_DEFINITIONS_IF_DEFINED(CFG_ITP_DBG)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/sdk/driver/usb/inc/
)

add_library(itp_boot STATIC
    ${PROJECT_SOURCE_DIR}/sdk/include/ite/itp.h
    itp_backtrace.c
    itp_base.c
    itp_cfg.h
    itp_defer.c
    itp_error.c
    itp_vmem.c
)

if (DEFINED CFG_GCC_LTO)
    SET_TARGET_PROPERTIES(itp_boot PROPERTIES COMPILE_FLAGS "-fno-lto")
endif()

if (DEFINED CFG_DBG_RMALLOC)
    add_library(itp_boot STATIC
        itp_rmalloc.c
    )
endif()

if (DEFINED CFG_DBG_PRINTBUF)
    add_library(itp_boot STATIC
        itp_printbuf.c
    )
endif()

if (DEFINED CFG_LCD_ENABLE)
    add_library(itp_boot STATIC
        itp_lcdconsole.c
    )
endif()

if (DEFINED CFG_DBG_OSDCONSOLE)
    add_library(itp_boot STATIC
        itp_osdconsole.c
    )
endif()

if (DEFINED CFG_LCD_ENABLE OR DEFINED CFG_TVOUT_ENABLE)
    add_library(itp_boot STATIC
        itp_screen.c
    )
endif()

if (DEFINED CFG_BACKLIGHT_ENABLE)
    add_library(itp_boot STATIC
        itp_backlight.c
    )
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/driver/itp
)

add_library(itp STATIC
    itp_uart_api.c
)

if ($ENV{CFG_PLATFORM} STREQUAL openrtos)
    if (DEFINED CFG_UART0_ENABLE OR DEFINED CFG_UART1_ENABLE OR DEFINED CFG_UART2_ENABLE OR DEFINED CFG_UART3_ENABLE OR DEFINED CFG_UART4_ENABLE OR DEFINED CFG_UART5_ENABLE)
        add_library(itp_boot STATIC
            itp_uart.c
        )
    endif()

    if (DEFINED CFG_KEYPAD_ENABLE)
        add_library(itp STATIC
            itp_keypad.c
            keypad/${CFG_KEYPAD_MODULE}
        )
    endif()

    if (DEFINED CFG_IR0_ENABLE OR DEFINED CFG_IR1_ENABLE OR DEFINED CFG_IR2_ENABLE OR DEFINED CFG_IR3_ENABLE)
        add_library(itp STATIC
            itp_ir.c
        )
    endif()
endif()

if (DEFINED CFG_DBG_SWUART_CODEC OR DEFINED CFG_RS485_4_ENABLE OR DEFINED CFG_SWUART_CODEC_ENABLE)
    add_library(itp_boot STATIC
        itp_swuart_codec.c
        itp_swuart_dbg_codec.c
    )
endif()

if (DEFINED CFG_UART0_RS485 OR DEFINED CFG_UART1_RS485 OR DEFINED CFG_UART2_RS485 OR DEFINED CFG_UART3_RS485 OR DEFINED CFG_UART4_RS485 OR DEFINED CFG_UART5_RS485)
    add_library(itp STATIC
        itp_rs485.c
    )
endif()

add_library(itp STATIC
    ${PROJECT_SOURCE_DIR}/sdk/include/ite/itp.h
    itp_cfg.h
)

if (DEFINED CFG_WATCHDOG_ENABLE)
    add_library(itp STATIC
        itp_watchdog.c
    )
endif()

if (DEFINED CFG_I2C0_ENABLE OR CFG_I2C1_ENABLE OR CFG_I2C2_ENABLE OR CFG_I2C3_ENABLE)
    add_library(itp STATIC
        itp_i2c.c
    )
endif()

if (DEFINED CFG_SPI_ENABLE)
    add_library(itp STATIC
        itp_spi.c
    )
endif()

if (DEFINED CFG_DCPS_ENABLE)
    add_library(itp STATIC
        itp_decompress.c
    )
endif()

if (DEFINED CFG_DPU_ENABLE)
    add_library(itp STATIC
        itp_dpu.c
    )
endif()

if (DEFINED CFG_BATTERY_ENABLE)
    add_library(itp STATIC
        battery/${CFG_BATTERY_MODULE}
    )
endif()

if (DEFINED CFG_BATTERY_ENABLE OR DEFINED CFG_POWER_ON OR DEFINED CFG_POWER_STANDBY OR DEFINED CFG_POWER_SLEEP OR DEFINED CFG_POWER_HIBERNATION OR DEFINED CFG_POWER_SUSPEND OR DEFINED CFG_POWER_OFF)
    add_library(itp STATIC
        itp_power.c
    )
endif()

if (DEFINED CFG_GSENSOR_ENABLE)
    add_library(itp STATIC
        itp_gsensor.c
        gsensor/${CFG_GSENSOR_MODULE}
    )
endif()

if (DEFINED CFG_HEADSET_ENABLE)
    add_library(itp STATIC
        itp_headset.c
        headset/${CFG_HEADSET_MODULE}
    )
endif()

if (DEFINED CFG_AMPLIFIER_ENABLE)
    add_library(itp STATIC
        itp_amplifier.c
        amplifier/${CFG_AMPLIFIER_MODULE}
    )
endif()

if (DEFINED CFG_LED_ENABLE)
    add_library(itp STATIC
        itp_led.c
    )
endif()

if (DEFINED CFG_SWITCH_ENABLE)
    add_library(itp STATIC
        itp_switch.c
    )
endif()

if (DEFINED CFG_TUNER_ENABLE)
    add_library(itp STATIC
        itp_tuner.c
    )
endif()

if (DEFINED CFG_WIEGAND0_BIT_CUSTOMIZE)
    add_library(itp STATIC
        ${CFG_WIEGAND0_CUSTOMIZE_MODULE}
    )
elseif (DEFINED CFG_WIEGAND1_BIT_CUSTOMIZE)
    add_library(itp STATIC
        ${CFG_WIEGAND1_CUSTOMIZE_MODULE}
    )
elseif (DEFINED CFG_WIEGAND0_ENABLE OR DEFINED CFG_WIEGAND1_ENABLE)
    add_library(itp STATIC
        itp_wiegand.c
		itp_wiegandout.c
    )
endif()

add_library(itp STATIC
    itp_ctrlboard_codec.c
)

if (DEFINED CFG_STNLCD_ENABLE)
    add_library(itp STATIC
        itp_stnlcd.c
        stnlcd/${CFG_STNLCD_MODULE}
    )
endif()

if (DEFINED CFG_RTC_ENABLE)
    if (DEFINED CFG_RTC_REDUCE_IO_ACCESS_ENABLE)
        add_library(itp STATIC
            rtc/itp_rtc_assist.c
        )
    endif()

    add_library(itp STATIC
        itp_rtc.c
        rtc/${CFG_RTC_MODULE}
    )
endif()

if (DEFINED CFG_RGBTOMIPI_ENABLE)
    add_library(itp STATIC
        itp_rgbtomipi.c
        rgbtomipi/${CFG_RGBTOMIPI_MODULE}
    )
endif()

if (DEFINED CFG_NAND_ENABLE)
    add_library(itp STATIC
        itp_nand.c
    )
endif()

if (DEFINED CFG_NOR_ENABLE)
    add_library(itp STATIC
        itp_nor.c
    )
endif()

if (DEFINED CFG_XD_ENABLE)
    add_library(itp STATIC
        itp_xd.c
    )
endif()

if (DEFINED CFG_USBHCC)
    add_library(itp STATIC
        usbhcc/itp_usb.c
        usbhcc/itp_usbh.c
    )
endif()

if (DEFINED CFG_USBH_CD_HUB)
    add_library(itp STATIC
        usbhcc/itp_usbh_hub.c
    )
endif()

if (DEFINED CFG_USBH_CD_MST)
    add_library(itp STATIC
        usbhcc/itp_usbh_mst.c
    )
endif()

if (DEFINED CFG_USB0_ENABLE OR DEFINED CFG_USB1_ENABLE)
    add_library(itp STATIC
        itp_usb.c
    )
endif()

if (DEFINED CFG_USB_OPTION)
    add_library(itp STATIC
        itp_usb_option.c
    )
endif()

if (DEFINED CFG_USB_DEVICE)
    add_library(itp STATIC
        itp_usb_device.c
    )
endif()

if (DEFINED CFG_USB_HID)
    add_library(itp STATIC
        itp_usb_hid.c
    )
else()
    if (DEFINED CFG_USB_MOUSE)
        add_library(itp STATIC
            itp_usb_mouse.c
        )
    endif()

    if (DEFINED CFG_USB_KBD)
        add_library(itp STATIC
            itp_usb_kbd.c
        )
    endif()
endif()

if (DEFINED CFG_USBD_FILE_STORAGE OR DEFINED CFG_USBD_MASS_STORAGE)
    add_library(itp STATIC
        itp_usbd_fsg.c
    )
    if (DEFINED CFG_USBD_IDB)
        add_library(itp STATIC
            itp_usbd_idb.c
        )
    endif()
endif()

if (DEFINED CFG_USBD_ACM)
    add_library(itp STATIC
        itp_usbd_acm.c
    )
endif()

if (DEFINED CFG_USBD_HID)
    add_library(itp STATIC
        itp_usbd_hid.c
    )
endif()

if (DEFINED CFG_SD0_ENABLE OR DEFINED CFG_SD1_ENABLE)
    add_library(itp STATIC
        itp_sd.c
    )
endif()

if (DEFINED CFG_SDIO_ENABLE)
    add_library(itp STATIC
        itp_sdio.c
    )
endif()

if (DEFINED CFG_FS_FAT)
    add_library(itp STATIC
        itp_fat.c
    )
endif()

if (DEFINED CFG_FS_FAT OR $ENV{CFG_PLATFORM} STREQUAL win32)
    add_library(itp STATIC
        itp_logdisk.c
    )
endif()

if (DEFINED CFG_AUDIO_ENABLE)
    add_library(itp STATIC
        itp_codec.c
        dac/${CFG_AUDIO_DAC_MODULE}
    )
else()
    add_library(itp STATIC
        itp_codec.c
        dac/itp_dac_null.c
    )
endif()

if (DEFINED CFG_VIDEO_ENABLE)
    if ($ENV{CFG_PLATFORM} STREQUAL openrtos)
        add_library(itp STATIC
            it9860/itp_stc.c
        )
    elseif ($ENV{CFG_PLATFORM} STREQUAL win32)
        add_library(itp STATIC
            itp_stc_win32.c
        )
    endif()
endif()

if (DEFINED CFG_NET_ENABLE)
    add_library(itp STATIC
        itp_socket.c
    )
endif()

if (DEFINED CFG_NET_ETHERNET OR DEFINED CFG_USB_ECM OR DEFINED CFG_USBD_NCM)
    add_library(itp STATIC
        itp_ethernet.c
    )
endif()

if (DEFINED CFG_USBD_NCM)
    add_library(itp STATIC
        itp_usbd_ncm.c
    )
endif()

if (DEFINED CFG_NET_RTL8201FL)
    add_library(itp STATIC
        phy/phy_rtl8201fl.c
    )
endif()

if (DEFINED CFG_NET_IP101GR)
    add_library(itp STATIC
        phy/phy_ip101gr.c
    )
endif()

if (DEFINED CFG_NET_IP175LLF)
    add_library(itp STATIC
        phy/ip175llf.c
    )
endif()

if (DEFINED CFG_NET_LAN8720)
    add_library(itp STATIC
        phy/phy_lan8720.c
    )
endif()

if (DEFINED CFG_NET_RTL8211F)
    add_library(itp STATIC
        phy/phy_rtl8211f.c
    )
endif()

if (DEFINED CFG_NET_RTL8304MB)
    include_directories(
        ${PROJECT_SOURCE_DIR}/sdk/include/rtl8304mb
    )
    add_library(itp STATIC
        phy/rtl8304mb.c
    )
endif()

if (DEFINED CFG_NET_RTL8363NB)
    include_directories(
        ${PROJECT_SOURCE_DIR}/sdk/include/rtl8363nb
    )
    add_library(itp STATIC
        phy/rtl8363nb.c
    )
endif()

if (DEFINED CFG_NET_WIFI)
    add_library(itp STATIC
        itp_wifi.c
    )
endif()

if (DEFINED CFG_DBG_NETCONSOLE)
    add_library(itp STATIC
        itp_netconsole.c
    )
endif()

if (DEFINED CFG_DBG_STATS)
    add_library(itp STATIC
        itp_stats.c
    )
endif()

if (DEFINED CFG_DEMOD_ENABLE)
    add_library(itp STATIC
        itp_demod.c
        itp_demod_thread.c
    )
elseif(DEFINED CFG_DEMOD_ISDB_ENABLE)
    add_library(itp STATIC
        itp_demod_isdb.c
        itp_demod_thread.c
    )
endif()

if (DEFINED CFG_FS_FAT)
    add_library(itp STATIC
        itp_dlfcn_$ENV{CFG_PLATFORM}.c
    )
endif()

if (DEFINED CFG_ARMLITE_ENABLE)
    add_library(itp_boot STATIC
        itp_armlite_dev.c
    )
endif()

if (DEFINED CFG_ALT_CPU_ENABLE)
    add_library(itp_boot STATIC
        itp_alt_cpu.c
    )
endif()

if ($ENV{CFG_PLATFORM} STREQUAL openrtos)
    add_library(itp_boot STATIC
        itp_posix_openrtos.c
    )

    add_library(itp STATIC
        itp_init_openrtos.c
    )

    if (DEFINED CFG_SD0_ENABLE OR DEFINED CFG_SD1_ENABLE OR DEFINED CFG_RAMDISK_ENABLE OR DEFINED CFG_MSC_ENABLE OR DEFINED CFG_USBH_CD_MST)
        add_library(itp STATIC
            itp_card.c
        )
    endif()

    if (DEFINED CFG_FS_FAT)
        add_library(itp STATIC
            itp_drive_openrtos.c
        )
    endif()

    if (DEFINED CFG_DBG_TRACE_ANALYZER AND DEFINED CFG_DBG_VCD)
        add_library(itp_boot STATIC
            itp_stats_taskvcd.c
        )
    endif()

elseif ($ENV{CFG_PLATFORM} STREQUAL win32)
    add_library(itp_boot STATIC
        itp_file.c
        itp_posix_win32.c
    )

    add_library(itp STATIC
        itp_drive_win32.c
        itp_init_win32.c
    )
endif()

if (DEFINED CFG_LCD_ENABLE)

    set(CFG_LCDA_ADDR "0x0")

    if (DEFINED CFG_LCD_MULTIPLE)

        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/lcd.inc "")

        foreach (script ${CFG_LCD_INIT_SCRIPTS})
            execute_process(COMMAND mkscript ${PROJECT_SOURCE_DIR}/sdk/target/lcd/${script}.txt ${CMAKE_CURRENT_BINARY_DIR}/${script}.inc)

            file(READ ${CMAKE_CURRENT_BINARY_DIR}/${script}.inc array)

            string(REPLACE "." "" var ${script})
            file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/lcd.inc "static const LCD_SCRIPT_TYPE ${var}[] = {\n${array}};\n")

        endforeach()

        file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/lcd.inc "static const LCD_SCRIPT_TYPE* lcdScript[] =\n{\n")

        foreach (script ${CFG_LCD_INIT_SCRIPTS})
            string(REPLACE "." "" var ${script})
            file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/lcd.inc "${var},\n")

        endforeach()

        file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/lcd.inc "};\n")

        file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/lcd.inc "static const int lcdScriptSize[] =\n{\n")

        foreach (script ${CFG_LCD_INIT_SCRIPTS})
            string(REPLACE "." "" var ${script})
            file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/lcd.inc "ITH_COUNT_OF(${var}),\n")

        endforeach()

        file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/lcd.inc "};\n")

    else()
        configure_file(${PROJECT_SOURCE_DIR}/sdk/target/lcd/${CFG_LCD_INIT_SCRIPT} ${CMAKE_CURRENT_BINARY_DIR}/lcd.txt)

        execute_process(COMMAND mkscript ${CMAKE_CURRENT_BINARY_DIR}/lcd.txt ${CMAKE_CURRENT_BINARY_DIR}/lcd.inc)
    endif()

endif()

if (DEFINED CFG_TVOUT_ENABLE)
    configure_file(${PROJECT_SOURCE_DIR}/sdk/target/tvout/tv_ntsc.inc ${CMAKE_CURRENT_BINARY_DIR}/tv_ntsc.inc)
    configure_file(${PROJECT_SOURCE_DIR}/sdk/target/tvout/tv_pal.inc ${CMAKE_CURRENT_BINARY_DIR}/tv_pal.inc)
endif()
