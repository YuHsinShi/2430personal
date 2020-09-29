set(PROJ_NAME ${CMAKE_PROJECT_NAME})

if (DEFINED CFG_BOOTLOADER_ENABLE)
    set(CMAKE_PROJECT_NAME bootloader)
    add_subdirectory(../bootloader ../bootloader)
endif()

set(CMAKE_PROJECT_NAME ${PROJ_NAME})

if (DEFINED CFG_WATCHDOG_ENABLE)
    add_definitions(
        -DCFG_WATCHDOG_ENABLE
    )
endif()

if (DEFINED CFG_CHECK_FILES_CRC_ON_BOOTING)
    add_definitions(
        -DCFG_CHECK_FILES_CRC_ON_BOOTING
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



add_executable(${CMAKE_PROJECT_NAME}
   # audio.c
    config.c
    project.h
    external.c
    external_process.c
    function_table.c
    main.c
    reset.c
    scene.c
    scene.h
    storage.c
    upgrade.c
    backup.c
	burner.c
    )



# upgrade fw from uart
if (DEFINED CFG_UPGRADE_FROM_UART)
    add_executable(${CMAKE_PROJECT_NAME}
        upgrade_uart.c
        )
endif()


# files
configure_file(${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/config.ini ${CMAKE_BINARY_DIR}/data/public/config.ini COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/setting.ini ${CMAKE_BINARY_DIR}/data/public/setting.ini COPYONLY)


# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)
