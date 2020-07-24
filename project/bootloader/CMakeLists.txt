if (DEFINED CFG_CPU_FA626 AND $ENV{CFG_PLATFORM} STREQUAL openrtos) 
set(CMAKE_EXE_LINKER_FLAGS          "${EXTRA_FLAGS} -mcpu=fa626te ${MAP_FLAGS} -nostartfiles -T\"${LIBRARY_OUTPUT_PATH}/default.ld\" -L\"${LIBRARY_OUTPUT_PATH}\" \"${LIBRARY_OUTPUT_PATH}/tlb_wt.o\" \"${LIBRARY_OUTPUT_PATH}/startup.o\"")
endif()

if (DEFINED CFG_WATCHDOG_ENABLE)
    add_definitions(
        -DCFG_WATCHDOG_ENABLE
    )
endif()

if (DEFINED CFG_BL_ERR)
    add_definitions(
        -DCFG_BL_ERR
        )
endif()

if (DEFINED CFG_BL_WARN)
    add_definitions(
        -DCFG_BL_WARN
        )
endif()

if (DEFINED CFG_BL_INFO)
    add_definitions(
        -DCFG_BL_INFO
        )
endif()

if (DEFINED CFG_BL_DBG)
    add_definitions(
        -DCFG_BL_DBG
        )
endif()

if (DEFINED CFG_BL_LCD_CONSOLE)
    add_definitions(
        -DCFG_BL_LCD_CONSOLE
        )
endif()

if (DEFINED CFG_UPGRADE_FROM_UART)
    add_definitions(
        -DCFG_UPGRADE_FROM_UART
        )
endif()

if (DEFINED CFG_UPGRADE_UART0)
    add_definitions(
        -DCFG_UPGRADE_UART0
        )
elseif (DEFINED CFG_UPGRADE_UART1)
    add_definitions(
        -DCFG_UPGRADE_UART1
        )
endif()

if (DEFINED CFG_BOOT_TESTBIN_ENABLE)
    add_definitions(
        -DCFG_BOOT_TESTBIN_ENABLE
        -DCFG_TESTBIN_FILENAME="${CFG_TESTBIN_FILENAME}"
        )
endif()

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg
    ${PROJECT_SOURCE_DIR}/sdk/driver/itv
    )

if (CFG_CHIP_FAMILY STREQUAL 9850)
    include_directories(
        ${PROJECT_SOURCE_DIR}/sdk/share/itu
        ${PROJECT_SOURCE_DIR}/sdk/share/itu/it9850
    )
else()
    include_directories(
        ${PROJECT_SOURCE_DIR}/sdk/share/itu
        ${PROJECT_SOURCE_DIR}/sdk/share/itu/it9070
    )
endif()

add_executable(${CMAKE_PROJECT_NAME}
    main_$ENV{CFG_PLATFORM}.c
    )

add_executable(${CMAKE_PROJECT_NAME}
    bootloader.h
    boot.c
    config.h
    main.c
    upgrade.c
    )
    
#if (DEFINED CFG_BUILD_ASR)
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
#endif()

if (DEFINED CFG_ENABLE_UART_CLI)
include_directories(
	${PROJECT_SOURCE_DIR}/sdk/share/cli/
    )
endif()    

if (DEFINED CFG_OPENRTOS_CLI)
include_directories(
	${PROJECT_SOURCE_DIR}/sdk/share/cli/
    )
endif()

if ((DEFINED CFG_ENABLE_UART_CLI) AND (DEFINED CFG_NET_TFTP))
add_executable(${CMAKE_PROJECT_NAME}
	recovery.c
	)
include_directories(
	${PROJECT_SOURCE_DIR}/sdk/share/wifi_mgr/
	${PROJECT_SOURCE_DIR}/sdk/driver/non_gpl_wifi/include/
    ${PROJECT_SOURCE_DIR}/sdk/driver/non_gpl_wifi/api/wifi/  
    )
endif()  

if (DEFINED CFG_UPGRADE_BACKUP_PACKAGE)
add_executable(${CMAKE_PROJECT_NAME}
    restore.c
    )
endif()

# logo
if (DEFINED CFG_BL_SHOW_LOGO)
    add_executable(${CMAKE_PROJECT_NAME}
        logo.c
        )

    execute_process(COMMAND convert -define bmp:format=bmp3 -define bmp3:alpha=true ${PROJECT_SOURCE_DIR}/data/bitmap/${CFG_LCD_BOOT_BITMAP} ${CMAKE_CURRENT_BINARY_DIR}/logo.bmp)

    if (CFG_LCD_BPP STREQUAL "4")        
        set(args --format=ARGB8888)
    else()        
        set(args --format=RGB565)
    endif()

    if (DEFINED CFG_LCD_BOOT_BITMAP_DITHER)
        set(args ${args} --dither)
    endif()
    
    message("dataconv ${args}")
    execute_process(COMMAND dataconv ${args} -m ${CMAKE_CURRENT_BINARY_DIR}/logo.bmp -o ${CMAKE_CURRENT_BINARY_DIR}/logo.inc)
endif()

# recovery
if (DEFINED CFG_UPGRADE_RECOVERY)
    add_executable(${CMAKE_PROJECT_NAME}
        recovery.c
        )
endif()

# USB device mode
if (DEFINED CFG_UPGRADE_USB_DEVICE)
    add_executable(${CMAKE_PROJECT_NAME}
        usbdevice.c
        )
endif()

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)
