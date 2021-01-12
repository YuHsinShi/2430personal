#
# Automatically generated file; DO NOT EDIT.
# Project Configuration
# Tue Jan 12 17:50:46 2021
#
#
set(CFG_BOOTLOADER y)
set(CFG_DEF_CHIP_PKG_IT9860 y)
set(CFG_RAM_INIT_SCRIPT "IT9860_360Mhz_DDR2_360Mhz.scr")
set(CFG_DCPS_ENABLE y)
set(CFG_BACKLIGHT_DUTY_CYCLES "95, 92, 88, 82, 75, 65, 55, 40, 25, 5")
set(CFG_BACKLIGHT_DEFAULT_DUTY_CYCLE "5")
set(CFG_LCD_HEIGHT "600")
set(CFG_LCD_INIT_SCRIPT "DEMOIT9860_MIPI_JY_1024x600_24bits.txt")
set(CFG_NOR_CACHE_FLUSH_INTERVAL "0")
set(CFG_NOR_ENABLE y)
set(CFG_NOR_USE_AXISPI y)
set(CFG_NOR_ENABLE_QUADMODE y)
set(CFG_NOR_CACHE_SIZE "0x80000")
set(CFG_SD0_ENABLE y)
set(CFG_USB0_ENABLE y)
set(CFG_MSC_ENABLE y)
set(CFG_FS_FAT y)
# CFG_I2C0_ENABLE is not set
# CFG_UART1_ENABLE is not set
# CFG_KEYPAD_ENABLE is not set
set(CFG_KEYPAD_MODULE "itp_keypad_castor3.c")
set(CFG_KEYPAD_REPEAT y)
set(CFG_GPIO_UART1_TX "5")
set(CFG_GPIO_UART1_RX "-1")
set(CFG_GPIO_KEYPAD "42, 43")
set(CFG_TOUCH_KEY_USE_I2C y)
set(CFG_GPIO_ETHERNET_LINK "44")
set(CFG_GPIO_ETHERNET "27, 28, 29, 30, 31, 32, 33, 34, 35, 36")
set(CFG_TASK_DRIVE_PROBE y)
set(CFG_UPGRADE_IMAGE y)
set(CFG_UPGRADE_OPEN_FILE y)
set(CFG_DEF_DBG_UART1 y)
set(CFG_DBG_INIT_SCRIPT "IT9860_360Mhz_DDR2_360Mhz.txt")
set(CFG_BUILD_UPGRADE y)
set(CFG_HAVE_LCD y)
set(CFG_HAVE_GRAPHICS y)
set(CFG_HAVE_AUDIO y)

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
# CFG_GCC_LTO is not set
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
set(CFG_MMAP_SIZE "0")
# CFG_WATCHDOG_ENABLE is not set
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
# Screen
#
# CFG_ENABLE_ROTATE is not set
# CFG_LCD_ENABLE is not set
# CFG_BACKLIGHT_ENABLE is not set

#
# Graphics
#
# CFG_CMDQ_ENABLE is not set
# CFG_JPEG_HW_ENABLE is not set
set(CFG_FONT_FILENAME "WenQuanYiMicroHeiMono.ttf")

#
# Audio
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
# CFG_NOR_ENABLE_DTRMODE is not set
set(CFG_SD0_NO_PIN_SHARE y)
set(CFG_SD0_STATIC y)
# CFG_SD0_CARD_1BIT is not set
# CFG_SD1_ENABLE is not set
set(CFG_MMC_ENABLE y)
# CFG_RAMDISK_ENABLE is not set
# CFG_UAS_ENABLE is not set

#
# File System
#
set(CFG_NOR_RESERVED_SIZE "0x1000000")
set(CFG_NOR_PARTITION0 y)
set(CFG_NOR_PARTITION0_SIZE "0xAF0000")
set(CFG_NOR_PARTITION1 y)
set(CFG_NOR_PARTITION1_SIZE "0x40000")
set(CFG_NOR_PARTITION2 y)
set(CFG_NOR_PARTITION2_SIZE "0")
set(CFG_NOR_PARTITION3 y)
set(CFG_NOR_PARTITION3_SIZE "0")
set(CFG_SD0_RESERVED_SIZE "0")
# CFG_SD0_PARTITION0 is not set
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
# CFG_I2C1_ENABLE is not set
# CFG_I2C2_ENABLE is not set
# CFG_I2C3_ENABLE is not set
# CFG_I2S_ENABLE is not set
set(CFG_AXISPI_ENABLE y)
set(CFG_SPI_ENABLE y)
set(CFG_SPI0_ENABLE y)
# CFG_SPI0_40MHZ_ENABLE is not set
# CFG_SPI1_ENABLE is not set
# CFG_RGBTOMIPI_ENABLE is not set
# CFG_WIEGAND_ENABLE is not set
set(CFG_UART_ENABLE y)
set(CFG_UART0_ENABLE y)
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
# CFG_USB1_ENABLE is not set
# CFG_USB_DEVICE is not set
# CFG_IRDA_ENABLE is not set
# CFG_TOUCH_ENABLE is not set
# CFG_AMPLIFIER_ENABLE is not set
# CFG_LED_ENABLE is not set
set(CFG_CAPTURE_DEFINE "DEMO_9860_DEFAULT.txt")
# CFG_SENSOR_ENABLE is not set
# CFG_USB_HID_ENABLE is not set
# CFG_USB_SERIAL is not set
# CFG_SDIO_ENABLE is not set
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
set(CFG_GPIO_SD0_POWER_ENABLE "-1")
set(CFG_GPIO_SD0_WRITE_PROTECT "-1")
set(CFG_GPIO_SD0_IO "51, 52, 53, 54, 55, 56, -1, -1, -1, -1")
set(CFG_GPIO_UART0_RX "-1")
set(CFG_GPIO_UART0_TX "4")
set(CFG_AXISPI_MISO_GPIO "7")
set(CFG_AXISPI_MOSI_GPIO "6")
set(CFG_AXISPI_CLOCK_GPIO "10")
set(CFG_AXISPI_CHIP_SEL_GPIO "5")
set(CFG_AXISPI_WP_GPIO "8")
set(CFG_AXISPI_HOLD_GPIO "9")
set(CFG_SPI0_MISO_GPIO "22")
set(CFG_SPI0_MOSI_GPIO "21")
set(CFG_SPI0_CLOCK_GPIO "19")
set(CFG_SPI0_CHIP_SEL_GPIO "20")

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
# CFG_DBG_STATS is not set
# CFG_ENABLE_UART_CLI is not set
# CFG_DBG_OUTPUT_DEBUG_FILES is not set

#
# Upgrade
#
# CFG_UPGRADE_BOOTLOADER is not set
# CFG_UPGRADE_IMAGE_NAND is not set
set(CFG_UPGRADE_IMAGE_NOR y)
# CFG_UPGRADE_IMAGE_SD0 is not set
# CFG_UPGRADE_IMAGE_SD1 is not set
set(CFG_UPGRADE_IMAGE_POS "0x80000")
# CFG_UPGRADE_DATA is not set
set(CFG_UPGRADE_ENC_KEY "0")
# CFG_UPGRADE_PRESSKEY is not set
set(CFG_UPGRADE_FILENAME "ITEPKG03.PKG")
set(CFG_UPGRADE_FILENAME_LIST "ITEPKG03.PKG")
set(CFG_UPGRADE_DELETE_PKGFILE_AFTER_FINISH y)
set(CFG_UPGRADE_DELAY_AFTER_FINISH "0")
set(CFG_UPGRADE_USB_DETECT_TIMEOUT "200")
set(CFG_UPGRADE_USB_TIMEOUT "8000")
# CFG_UPGRADE_NOR_IMAGE is not set
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
# BootLoader
#
set(CFG_BL_ERR y)
set(CFG_BL_WARN y)
set(CFG_BL_INFO y)
set(CFG_BL_DBG y)
