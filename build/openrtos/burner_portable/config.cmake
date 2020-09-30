#
# Automatically generated file; DO NOT EDIT.
# Project Configuration
# Wed Sep 30 17:15:43 2020
#
#
set(CFG_DEF_CHIP_PKG_IT9860 y)
set(CFG_RAM_SIZE "0x4000000")
set(CFG_RAM_INIT_SCRIPT "IT9860_360Mhz_DDR2_360Mhz.scr")
set(CFG_TILING_WIDTH_128 0)
set(CFG_WATCHDOG_ENABLE y)
set(CFG_DCPS_ENABLE y)
set(CFG_NOR_ENABLE y)
set(CFG_NOR_USE_AXISPI y)
set(CFG_NOR_ENABLE_QUADMODE y)

#
# (unvisible)
#
set(CFG_NOR_CACHE_SIZE "0x10000")
set(CFG_NOR_RESERVED_SIZE "0x800000")
set(CFG_NOR_PARTITION0 y)
set(CFG_NOR_PARTITION0_SIZE "0xAF0000")
# CFG_NOR_PARTITION1 is not set
set(CFG_NOR_PARTITION1_SIZE "0x260000")
set(CFG_NOR_PARTITION2 y)
set(CFG_NOR_PARTITION2_SIZE "0x40000")
# CFG_NOR_PARTITION3 is not set
# CFG_SD0_ENABLE is not set
set(CFG_GPIO_SD0_CARD_DETECT "22")
set(CFG_GPIO_SD0_IO "14, 13, 15, 16, 17, 18, -1, -1, -1, -1")
set(CFG_USB0_ENABLE y)
set(CFG_MSC_ENABLE y)
set(CFG_FS_FAT y)
set(CFG_RTC_ENABLE y)
set(CFG_RTC_MODULE "itp_rtc_sw.c")
set(CFG_GPIO_UART0_TX "4")
set(CFG_GPIO_UART0_RX "-1")
set(CFG_AXISPI_ENABLE y)
set(CFG_AXISPI_MOSI_GPIO "6")
set(CFG_AXISPI_MISO_GPIO "7")
set(CFG_AXISPI_CLOCK_GPIO "10")
set(CFG_AXISPI_CHIP_SEL_GPIO "5")
set(CFG_AXISPI_WP_GPIO "8")
set(CFG_AXISPI_HOLD_GPIO "9")
set(CFG_TASK_DRIVE_PROBE y)
set(CFG_DBG_INIT_SCRIPT "IT9860_360Mhz_DDR2_360Mhz.txt")
set(CFG_DBG_STATS y)
set(CFG_DBG_STATS_HEAP y)
set(CFG_DEF_BUILD_MINSIZEREL y)
set(CFG_SYSTEM_NAME "burner")
# CFG_GCC_LTO is not set
set(CFG_BUILD_INIPARSER y)
set(CFG_BUILD_SDL y)
set(CFG_BUILD_UPGRADE y)
set(CFG_SPI_ENABLE y)
# CFG_BOOTLOADER_ENABLE is not set

#
# (unvisible)
#

#
# (unvisible)
#
# CFG_UPGRADE_BOOTLOADER is not set
# CFG_UPGRADE_IMAGE is not set
set(CFG_UPGRADE_DATA y)

#
# (unvisible)
#
set(CFG_UPGRADE_PRIVATE y)
# CFG_UPGRADE_PUBLIC is not set
# CFG_UPGRADE_TEMP is not set
set(CFG_UPGRADE_OPEN_FILE y)
set(CFG_UPGRADE_PARTITION y)
set(CFG_UPGRADE_DELETE_PKGFILE_AFTER_FINISH y)

#
# (unvisible)
#
set(CFG_UPGRADE_NOR_IMAGE y)
set(CFG_UPGRADE_NOR_IMAGE_SIZE "0x1000000")
set(CFG_CHECK_FILES_CRC_ON_BOOTING y)

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

#
# OpenRTOS
#
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
set(CFG_MMAP_SIZE "0")
set(CFG_WATCHDOG_TIMEOUT 10)
set(CFG_WATCHDOG_REFRESH_INTERVAL 1)
# CFG_WATCHDOG_INTR is not set
# CFG_WATCHDOG_IDLETASK is not set
# CFG_EXT_WATCHDOG_ENABLE is not set

#
# Internal Settings
#
set(CFG_MEMDBG_ENABLE y)
set(CFG_ROM_COMPRESS y)
# CFG_DPU_ENABLE is not set
set(CFG_CPU_WB y)
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
# CFG_SPI_NAND is not set
# CFG_SPI_NAND_USE_AXISPI is not set
# CFG_SPI_NAND_USE_SPI1 is not set
set(CFG_NOR_CACHE_FLUSH_INTERVAL "1")
# CFG_NOR_ENABLE_DTRMODE is not set
# CFG_SD1_ENABLE is not set
set(CFG_MMC_ENABLE y)
# CFG_RAMDISK_ENABLE is not set
# CFG_UAS_ENABLE is not set

#
# File System
#
set(CFG_PRIVATE_DRIVE "A")
set(CFG_PUBLIC_DRIVE "B")
set(CFG_TEMP_DRIVE "C")
set(CFG_BACKUP_DRIVE "D")

#
# Peripheral
#
# CFG_RTC_USE_I2C is not set
# CFG_RTC_REDUCE_IO_ACCESS_ENABLE is not set
set(CFG_RTC_DEFAULT_TIMESTAMP "1325376000")
set(CFG_INTERNAL_RTC_TIMER "5")
# CFG_I2C0_ENABLE is not set
# CFG_I2C1_ENABLE is not set
# CFG_I2C2_ENABLE is not set
# CFG_I2C3_ENABLE is not set
# CFG_I2S_ENABLE is not set
# CFG_SPI0_ENABLE is not set
set(CFG_SPI1_ENABLE y)
# CFG_RGBTOMIPI_ENABLE is not set
# CFG_WIEGAND_ENABLE is not set
set(CFG_UART_ENABLE y)
set(CFG_UART0_ENABLE y)
set(CFG_UART0_INTR y)
# CFG_UART0_DMA is not set
# CFG_UART0_FIFO is not set
set(CFG_UART0_BAUDRATE "115200")
# CFG_UART0_RS485 is not set
# CFG_UART1_ENABLE is not set
# CFG_UART2_ENABLE is not set
# CFG_UART3_ENABLE is not set
# CFG_UART4_ENABLE is not set
# CFG_UART5_ENABLE is not set
# CFG_UART_FORCE_FLUSH is not set
# CFG_SWUART_CODEC_ENABLE is not set
# CFG_USBHCC is not set
# CFG_USB1_ENABLE is not set
# CFG_USB_DEVICE is not set
# CFG_IRDA_ENABLE is not set
# CFG_KEYPAD_ENABLE is not set
# CFG_TOUCH_ENABLE is not set
# CFG_AMPLIFIER_ENABLE is not set
# CFG_LED_ENABLE is not set
set(CFG_CAPTURE_DEFINE "DEMO_9860_DEFAULT.txt")
# CFG_SENSOR_ENABLE is not set
# CFG_USB_HID_ENABLE is not set
# CFG_USB_SERIAL is not set
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
set(CFG_SPI1_MISO_GPIO "41")
set(CFG_SPI1_MOSI_GPIO "40")
set(CFG_SPI1_CLOCK_GPIO "38")
set(CFG_SPI1_CHIP_SEL_GPIO "42")

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

#
# Debug
#
# CFG_DBG_NONE is not set
# CFG_DBG_PRINTBUF is not set
# CFG_DBG_SWUART_CODEC is not set
set(CFG_DBG_UART0 y)
set(CFG_DBG_ICE_SCRIPT "IT9070A1_Initial_DDR2_Mem_tiling_pitch2048_320MHZ.csf")
set(CFG_DBG_STATS_PERIOD "10")
# CFG_DBG_STATS_MEM_BANDWIDTH is not set
# CFG_DBG_STATS_GPIO is not set
# CFG_DBG_STATS_FAT is not set
# CFG_ENABLE_UART_CLI is not set
# CFG_DBG_OUTPUT_DEBUG_FILES is not set

#
# Upgrade
#
# CFG_UPGRADE_IMAGE_NAND is not set
# CFG_UPGRADE_IMAGE_NOR is not set
# CFG_UPGRADE_IMAGE_SD0 is not set
# CFG_UPGRADE_IMAGE_SD1 is not set
# CFG_UPGRADE_PRIVATE_NAND is not set
set(CFG_UPGRADE_PRIVATE_NOR y)
# CFG_UPGRADE_PRIVATE_SD0 is not set
# CFG_UPGRADE_PRIVATE_SD1 is not set
set(CFG_UPGRADE_ENC_KEY "0")
# CFG_UPGRADE_PRESSKEY is not set
set(CFG_UPGRADE_FILENAME "ITEPKG03.PKG")
set(CFG_UPGRADE_FILENAME_LIST "ITEPKG03.PKG")
set(CFG_UPGRADE_DELAY_AFTER_FINISH "0")
set(CFG_UPGRADE_USB_DETECT_TIMEOUT "200")
set(CFG_UPGRADE_USB_TIMEOUT "8000")
set(CFG_UPGRADE_NOR_IMAGE_FILENAME "ITE_NOR.ROM")
set(CFG_UPGRADE_PACKAGE_VERSION "1.0")
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
set(CFG_BUILD_ITC y)

#
# ffmpeg (unvisible)
#

#
# (unvisible)
#

#
# itc
#
set(CFG_ITC_ERR y)
set(CFG_ITC_WARN y)
set(CFG_ITC_INFO y)
# CFG_ITC_DBG is not set

#
# (unvisible)
#

#
# sdl
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
# upgrade
#
set(CFG_UG_ERR y)
set(CFG_UG_WARN y)
set(CFG_UG_INFO y)
# CFG_UG_DBG is not set
set(CFG_UG_BUF_SIZE "0x20000")

#
# (unvisible)
#

#
# itu (unvisible)
#

#
# Display Control Board
#
set(CFG_HW_VERSION "V01")
# CFG_SCREENSHOT_ENABLE is not set
set(CFG_WEBSERVER_PORT "80")
set(CFG_ITU_PATH "itu/1280x480")
set(CFG_UPGRADE_FTP_URL "ftp://192.168.1.1/ctrlboard/ITEPKG03.PKG")
# CFG_UPGRADE_GUI is not set
# CFG_DYNAMIC_LOAD_TP_MODULE is not set
