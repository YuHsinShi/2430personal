ITE_ADD_DEFINITIONS_IF_DEFINED(CFG_CLI_ERR)
ITE_ADD_DEFINITIONS_IF_DEFINED(CFG_CLI_WARN)
ITE_ADD_DEFINITIONS_IF_DEFINED(CFG_CLI_INFO)
ITE_ADD_DEFINITIONS_IF_DEFINED(CFG_CLI_DBG)

if (DEFINED CFG_CLI_BUF_SIZE)
    add_definitions(
        -DCFG_CLI_BUF_SIZE=${CFG_CLI_BUF_SIZE}
        )
else()
    add_definitions(
        -DCFG_CLI_BUF_SIZE=0x20000
        )
endif()

add_library(cli STATIC
    ${PROJECT_SOURCE_DIR}/sdk/include/ite/cli.h
    cli.c
    cli_cfg.h
    cli_system.c
)

if (DEFINED CFG_FS_FAT)
    add_library(cli STATIC
        cli_disk.c
        cli_file.c
    )
endif()

if (DEFINED CFG_OPENRTOS_USE_TRACE_FACILITY AND DEFINED CFG_OPENRTOS_GENERATE_RUN_TIME_STATS)
    add_library(cli STATIC
        cli_stats.c
    )
endif()

if (DEFINED CFG_CLI_USB_ACM)
    add_definitions(
        -DCFG_CLI_USB_ACM
    )
    if (DEFINED CFG_SD0_STATIC OR DEFINED CFG_SD1_STATIC OR DEFINED CFG_NAND_ENABLE OR DEFINED CFG_NOR_ENABLE)
        add_library(cli STATIC
            cli_rawdata.c
        )
    endif()
    
    if (DEFINED CFG_UPGRADE_BOOTLOADER OR DEFINED CFG_UPGRADE_IMAGE OR DEFINED CFG_UPGRADE_DATA)
        add_definitions(
            -DCFG_CLI_UPGRADE
        )
        add_library(cli STATIC
            cli_upgrade.c
        )

    endif()
    
endif()
if (DEFINED CFG_ENABLE_UART_CLI)
	add_library(cli STATIC
		cli_upgrade_uart.c
    )
endif()

