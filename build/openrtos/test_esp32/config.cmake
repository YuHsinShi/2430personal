#
# Automatically generated file; DO NOT EDIT.
# Project Configuration
# Sun Feb 07 18:31:09 2021
#
#
set(CFG_DEF_CHIP_PKG_IT9860 y)
# CFG_ROM_COMPRESS is not set
set(CFG_DEF_DBG_UART0 y)
set(CFG_UART0_ENABLE y)
set(CFG_GPIO_UART0_TX "4")
set(CFG_GPIO_UART0_RX "-1")
set(CFG_SD1_ENABLE y)
set(CFG_SDIO_ENABLE y)
set(CFG_SDIO1_STATIC y)
set(CFG_GPIO_SD1_IO "32,33,34,35,36,37,-1,-1,-1,-1")
# CFG_UART1_ENABLE is not set
set(CFG_ESP32_SDIO_AT y)
# CFG_SDIO_4BIT_MODE is not set
set(CFG_DBG_INIT_SCRIPT "IT9860_264Mhz_DDR2_264Mhz.txt")

#
# (unvisible)
#
set(CFG_DEF_CFG_RELEASE y)

#
# (unvisible)
#

#
# Develop Environment
#
set(CFG_DEV_DEVELOP y)
# CFG_DEV_RELEASE is not set
# CFG_BOOTLOADER_ENABLE is not set

#
# OpenRTOS
#
set(CFG_GCC_LTO y)
set(CFG_OPENRTOS_HEAP_SIZE "0")
# CFG_OPENRTOS_USE_TRACE_FACILITY is not set
# CFG_OPENRTOS_GENERATE_RUN_TIME_STATS is not set
# CFG_OPENRTOS_CLI is not set

#
# System
#
set(CFG_CPU_FA626 y)
# CFG_CPU_SM32 is not set
# CFG_CHIP_REV_AW1 is not set
set(CFG_CHIP_REV_AW2 y)
set(CFG_CHIP_FAMILY_IT9860 y)
set(CFG_CHIP_PKG_IT9866 y)
# CFG_CHIP_PKG_IT9862 is not set
# CFG_CHIP_PKG_IT9868 is not set
# CFG_CHIP_PKG_888_66 is not set
set(CFG_RAM_SIZE "0x4000000")
set(CFG_RAM_INIT_SCRIPT "IT9860_360Mhz_DDR2_360Mhz.scr")
set(CFG_MMAP_SIZE "0")
# CFG_WATCHDOG_ENABLE is not set
# CFG_EXT_WATCHDOG_ENABLE is not set

#
# Internal Settings
#
set(CFG_MEMDBG_ENABLE y)
# CFG_DCPS_ENABLE is not set
# CFG_DPU_ENABLE is not set
# CFG_CPU_WB is not set
# CFG_CPU_WRITE_ADDR0_DETECT is not set

#
# Screen (unvisible)
#

#
# Graphics (unvisible)
#

#
# Audio (unvisible)
#

#
# Speech Filter (unvisible)
#

#
# Speech Coding Codec (unvisible)
#

#
# Music Codec (unvisible)
#

#
# Audio Mgr Buffer Size Setting (unvisible)
#

#
# ASR Filter (unvisible)
#

#
# Video (unvisible)
#

#
# Storage
#
# CFG_NAND_ENABLE is not set
# CFG_NOR_ENABLE is not set
# CFG_NOR_USE_AXISPI is not set
# CFG_SD0_ENABLE is not set
set(CFG_SD1_NO_PIN_SHARE y)
set(CFG_SD1_STATIC y)
set(CFG_SD1_CARD_1BIT y)
set(CFG_MMC_ENABLE y)
# CFG_RAMDISK_ENABLE is not set

#
# File System
#
# CFG_FS_FAT is not set
set(CFG_SD1_RESERVED_SIZE "0")
# CFG_SD1_PARTITION0 is not set
set(CFG_PRIVATE_DRIVE "A")
set(CFG_PUBLIC_DRIVE "B")
set(CFG_TEMP_DRIVE "C")
set(CFG_BACKUP_DRIVE "D")

#
# Peripheral
#
# CFG_RTC_ENABLE is not set
set(CFG_RTC_DEFAULT_TIMESTAMP "1325376000")
set(CFG_INTERNAL_RTC_TIMER "5")
# CFG_I2C0_ENABLE is not set
# CFG_I2C1_ENABLE is not set
# CFG_I2C2_ENABLE is not set
# CFG_I2C3_ENABLE is not set
# CFG_I2S_ENABLE is not set
set(CFG_AXISPI_ENABLE y)
# CFG_SPI_ENABLE is not set
# CFG_RGBTOMIPI_ENABLE is not set
# CFG_WIEGAND_ENABLE is not set
set(CFG_UART_ENABLE y)
set(CFG_UART0_INTR y)
# CFG_UART0_DMA is not set
# CFG_UART0_FIFO is not set
set(CFG_UART0_BAUDRATE "115200")
# CFG_UART0_RS485 is not set
# CFG_UART2_ENABLE is not set
# CFG_UART3_ENABLE is not set
# CFG_UART4_ENABLE is not set
# CFG_UART5_ENABLE is not set
# CFG_UART_FORCE_FLUSH is not set
# CFG_SWUART_CODEC_ENABLE is not set
# CFG_USBHCC is not set
# CFG_USB0_ENABLE is not set
# CFG_USB1_ENABLE is not set
# CFG_IRDA_ENABLE is not set
# CFG_KEYPAD_ENABLE is not set
# CFG_AMPLIFIER_ENABLE is not set
# CFG_LED_ENABLE is not set
set(CFG_CAPTURE_DEFINE "DEMO_9860_DEFAULT.txt")
# CFG_SENSOR_ENABLE is not set
# CFG_SDIO0_STATIC is not set
# CFG_ALT_CPU_ENABLE is not set
# CFG_SARADC_ENABLE is not set
# CFG_CANBUS_ENABLE is not set

#
# Power
#
# CFG_POWER_SAVING_ENABLE is not set
set(CFG_POWER_DOZE y)
# CFG_POWER_TICKLESS_IDLE is not set

#
# GPIO
#
set(CFG_GPIO_SD1_POWER_ENABLE "-1")
set(CFG_GPIO_SD1_WRITE_PROTECT "-1")
set(CFG_AXISPI_MISO_GPIO "7")
set(CFG_AXISPI_MOSI_GPIO "6")
set(CFG_AXISPI_CLOCK_GPIO "10")
set(CFG_AXISPI_CHIP_SEL_GPIO "5")
set(CFG_AXISPI_WP_GPIO "8")
set(CFG_AXISPI_HOLD_GPIO "9")

#
# Network
#
# CFG_NET_ENABLE is not set

#
# (unvisible)
#

#
# (unvisible)
#

#
# (unvisible)
#

#
# Task
#
# CFG_TASK_DRIVE_PROBE is not set

#
# Debug
#
# CFG_DBG_NONE is not set
# CFG_DBG_PRINTBUF is not set
# CFG_DBG_SWUART_CODEC is not set
set(CFG_DBG_UART0 y)
set(CFG_DBG_ICE_SCRIPT "IT9070A1_Initial_DDR2_Mem_tiling_pitch2048_320MHZ.csf")
# CFG_DBG_RMALLOC is not set
# CFG_DBG_STATS is not set
# CFG_ENABLE_UART_CLI is not set
# CFG_DBG_OUTPUT_DEBUG_FILES is not set

#
# Upgrade
#
# CFG_UPGRADE_BOOTLOADER is not set
# CFG_UPGRADE_IMAGE is not set
# CFG_UPGRADE_DATA is not set
set(CFG_UPGRADE_ENC_KEY "0")
set(CFG_UPGRADE_FILENAME "ITEPKG03.PKG")
set(CFG_UPGRADE_FILENAME_LIST "ITEPKG03.PKG")
# CFG_UPGRADE_BACKUP_PACKAGE is not set
set(CFG_UPGRADE_MARK_POS "0")

#
# SDK
#
# CFG_BUILD_DEBUG is not set
# CFG_BUILD_DEBUGREL is not set
set(CFG_BUILD_RELEASE y)
# CFG_BUILD_MINSIZEREL is not set
set(CFG_VERSION_MAJOR "2")
set(CFG_VERSION_MINOR "4")
set(CFG_VERSION_PATCH "3")
set(CFG_VERSION_CUSTOM "0")
set(CFG_SYSTEM_NAME "ITE Castor3")
set(CFG_MANUFACTURER "www.ite.com.tw")
# CFG_GENERATE_DOC is not set
# CFG_GENERATE_VERSION_TWEAK is not set

#
# Drivers
#

#
# ith
#
set(CFG_ITH_ERR y)
set(CFG_ITH_WARN y)
set(CFG_ITH_INFO y)
# CFG_ITH_DBG is not set
# CFG_ITH_FPGA is not set

#
# itp
#
set(CFG_ITP_ERR y)
set(CFG_ITP_WARN y)
set(CFG_ITP_INFO y)
# CFG_ITP_DBG is not set

#
# Libraries
#

#
# (unvisible)
#

#
# cli (unvisible)
#

#
# (unvisible)
#

#
# (unvisible)
#

#
# dhcps (unvisible)
#

#
# ffmpeg (unvisible)
#

#
# (unvisible)
#

#
# itc (unvisible)
#

#
# (unvisible)
#

#
# sdl (unvisible)
#

#
# (unvisible)
#

#
# tslib (unvisible)
#

#
# (unvisible)
#

#
# upgrade (unvisible)
#

#
# (unvisible)
#

#
# itu (unvisible)
#
