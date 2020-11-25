#
# Automatically generated file; DO NOT EDIT.
# Project Configuration
# Wed Nov 25 10:09:38 2020
#
#
set(CFG_CTRLBOARD y)
set(CFG_DEF_CHIP_PKG_IT9860 y)
set(CFG_RAM_SIZE "0x4000000")
set(CFG_RAM_INIT_SCRIPT "IT9860_360Mhz_DDR2_360Mhz.scr")
set(CFG_TILING_WIDTH_128 0)
set(CFG_WATCHDOG_ENABLE y)
set(CFG_DCPS_ENABLE y)
set(CFG_FONT_FILENAME "wqyMicroHeiProportional.ttf")
set(CFG_BACKLIGHT_ENABLE y)
set(CFG_BACKLIGHT_DUTY_CYCLES "95, 92, 88, 82, 75, 65, 55, 40, 25, 5")
set(CFG_BACKLIGHT_DEFAULT_DUTY_CYCLE "5")
set(CFG_LCD_ENABLE y)

#
# (unvisible)
#
# CFG_ENABLE_ROTATE is not set
set(CFG_LCD_WIDTH "720")
set(CFG_LCD_HEIGHT "720")
set(CFG_LCD_PITCH "1440")
set(CFG_LCD_INIT_SCRIPT "lcd_script-HS-0_hitachi_ok.txt")
set(CFG_CMDQ_ENABLE y)
set(CFG_M2D_ENABLE y)
set(CFG_JPEG_HW_ENABLE y)
set(CFG_AUDIO_ENABLE y)
set(CFG_AUDIO_DAC_MODULE "itp_dac_ite970.c")
set(CFG_AUDIO_CODEC_MP3DEC y)
set(CFG_AUDIO_CODEC_WMADEC y)
set(CFG_AUDIO_CODEC_WAV y)
set(CFG_COMPRESS_AUDIO_PLUGIN y)
# CFG_VIDEO_ENABLE is not set
set(CFG_NOR_ENABLE y)
set(CFG_NOR_USE_AXISPI y)
set(CFG_NOR_ENABLE_QUADMODE y)

#
# (unvisible)
#
set(CFG_NOR_CACHE_SIZE "0x200000")
set(CFG_NOR_RESERVED_SIZE "0x200000")
set(CFG_NOR_PARTITION0 y)
set(CFG_NOR_PARTITION0_SIZE "0x900000")
set(CFG_NOR_PARTITION1 y)
set(CFG_NOR_PARTITION1_SIZE "0x200000")
set(CFG_NOR_PARTITION2 y)
set(CFG_NOR_PARTITION2_SIZE "0x40000")
set(CFG_NOR_PARTITION3 y)
# CFG_SD0_ENABLE is not set
set(CFG_GPIO_SD0_CARD_DETECT "22")
set(CFG_GPIO_SD0_IO "14, 13, 15, 16, 17, 18, -1, -1, -1, -1")

#
# (unvisible)
#

#
# (unvisible)
#

#
# (unvisible)
#
set(CFG_USB0_ENABLE y)
set(CFG_MSC_ENABLE y)
set(CFG_FS_FAT y)
set(CFG_RTC_ENABLE y)
set(CFG_RTC_MODULE "itp_rtc_sw.c")
# CFG_SPI_ENABLE is not set
set(CFG_I2C0_ENABLE y)
set(CFG_GPIO_IIC0_CLK "59")
set(CFG_GPIO_IIC0_DATA "60")
set(CFG_TOUCH_ENABLE y)
set(CFG_TOUCH_MODULE "gt911")
set(CFG_TOUCH_X_MAX_VALUE "4095")
set(CFG_TOUCH_Y_MAX_VALUE "4095")
set(CFG_TOUCH_ADVANCE_CONFIG y)
set(CFG_TOUCH_I2C_SLAVE_ID "0x48")
set(CFG_TOUCH_SWAP_XY y)
# CFG_TOUCH_REVERSE_X is not set
# CFG_TOUCH_REVERSE_Y is not set
set(CFG_GPIO_TOUCH_INT "61")
set(CFG_GPIO_TOUCH_RESET "62")
set(CFG_GPIO_TOUCH_WAKE "-1")
set(CFG_TOUCH_INTR y)
# CFG_AMPLIFIER_ENABLE is not set
set(CFG_GPIO_AMPLIFIER_ENABLE "23")
set(CFG_GPIO_AMPLIFIER_MUTE "23")
set(CFG_I2S_ENABLE y)
# CFG_I2S_SPDIF_ENABLE is not set

#
# (unvisible)
#
# CFG_I2C1_ENABLE is not set

#
# (unvisible)
#
# CFG_SARADC_ENABLE is not set
set(CFG_POWER_SAVING_ENABLE y)
set(CFG_DEF_POWER_STANDBY y)
set(CFG_POWER_STANDBY_CPU_FREQ_RATIO "15")
set(CFG_POWER_STANDBY_BUS_FREQ_RATIO "30")
set(CFG_GPIO_UART0_TX "4")
set(CFG_GPIO_UART0_RX "-1")
set(CFG_AXISPI_ENABLE y)
set(CFG_AXISPI_MOSI_GPIO "6")
set(CFG_AXISPI_MISO_GPIO "7")
set(CFG_AXISPI_CLOCK_GPIO "10")
set(CFG_AXISPI_CHIP_SEL_GPIO "5")
set(CFG_AXISPI_WP_GPIO "8")
set(CFG_AXISPI_HOLD_GPIO "9")
set(CFG_GPIO_BACKLIGHT_PWM "53")
set(CFG_GPIO_LCD_PWR_EN "55")
# CFG_GPIO_LCD_PWR_EN_ACTIVE_LOW is not set
# CFG_NET_ENABLE is not set
set(CFG_TASK_DRIVE_PROBE y)
set(CFG_DBG_INIT_SCRIPT "IT9860_360Mhz_DDR2_360Mhz.txt")
set(CFG_DBG_STATS y)
set(CFG_DBG_STATS_HEAP y)
set(CFG_DEF_BUILD_MINSIZEREL y)
set(CFG_SYSTEM_NAME "ITE Display Control Board")
set(CFG_ITU_FT_CACHE_ENABLE y)
set(CFG_ITU_FT_CACHE_SIZE "0xFA000")
set(CFG_GCC_LTO y)
set(CFG_BUILD_AUDIO_MGR y)
set(CFG_BUILD_INIPARSER y)
set(CFG_BUILD_ITU y)
set(CFG_BUILD_SDL y)
set(CFG_BUILD_UPGRADE y)
set(CFG_BUILD_PYINPUT y)

#
# (unvisible)
#

#
# (unvisible)
#

#
# (unvisible)
#
set(CFG_BOOTLOADER_ENABLE y)

#
# (unvisible)
#
set(CFG_LCD_BOOT_BITMAP "logo.bmp")
# CFG_BL_SHOW_LOGO is not set

#
# (unvisible)
#
set(CFG_UPGRADE_BOOTLOADER y)
set(CFG_UPGRADE_IMAGE y)
set(CFG_UPGRADE_DATA y)

#
# (unvisible)
#
set(CFG_UPGRADE_PRIVATE y)
set(CFG_UPGRADE_PUBLIC y)
set(CFG_UPGRADE_TEMP y)
set(CFG_UPGRADE_OPEN_FILE y)
set(CFG_UPGRADE_PARTITION y)
set(CFG_UPGRADE_DELETE_PKGFILE_AFTER_FINISH y)

#
# (unvisible)
#
set(CFG_DEF_UPGRADE_BOOTLOADER_NOR y)
set(CFG_DEF_UPGRADE_IMAGE_NOR y)
set(CFG_UPGRADE_IMAGE_POS "0x80000")
set(CFG_DEF_UPGRADE_PRIVATE_NOR y)
set(CFG_DEF_UPGRADE_PUBLIC_NOR y)
set(CFG_DEF_UPGRADE_TEMP_NOR y)
set(CFG_UPGRADE_USB_DETECT_TIMEOUT "250")
set(CFG_UPGRADE_NOR_IMAGE y)
set(CFG_UPGRADE_NOR_IMAGE_SIZE "0x1000000")
set(CFG_BL_LCD_CONSOLE y)
set(CFG_BL_ERR y)
set(CFG_BL_WARN y)
set(CFG_BL_INFO y)
set(CFG_BL_DBG y)
set(CFG_CHECK_FILES_CRC_ON_BOOTING y)
set(CFG_HAVE_LCD y)
set(CFG_HAVE_GRAPHICS y)
set(CFG_HAVE_AUDIO y)
set(CFG_HAVE_VIDEO y)

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
# CFG_DEV_DEVELOP is not set
set(CFG_DEV_RELEASE y)

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
set(CFG_CHIP_FAMILY_IT9860 y)
# CFG_CHIP_PKG_IT9860 is not set
# CFG_CHIP_PKG_888_66 is not set
# CFG_CHIP_PKG_HJ_2019_1 is not set
# CFG_CHIP_PKG_IT9862 is not set
# CFG_CHIP_PKG_IT9864 is not set
set(CFG_CHIP_PKG_IT9866 y)
# CFG_CHIP_PKG_IT9866_AT is not set
# CFG_CHIP_PKG_IT9868 is not set
# CFG_CHIP_PKG_IT9868_AT is not set
# CFG_CHIP_PKG_IT9863 is not set
# CFG_CHIP_PKG_IT9865 is not set
# CFG_CHIP_PKG_IT9867 is not set
# CFG_CHIP_PKG_IT9867_AT is not set
# CFG_CHIP_PKG_IT9869 is not set
# CFG_CHIP_PKG_IT9869_AT is not set
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
# Screen
#
set(CFG_LCD_BPP "2")
# CFG_LCD_MULTIPLE is not set
set(CFG_LCD_TRIPLE_BUFFER y)
# CFG_LCD_PQ_TUNING is not set
set(CFG_BACKLIGHT_FREQ "1000")
set(CFG_BACKLIGHT_BOOTLODER_DELAY "200")

#
# Graphics
#
set(CFG_CMDQ_SIZE "0x40000")
# CFG_M2D_MEMPOOL_ENABLE is not set

#
# Audio
#
# CFG_DIALOGUE_PARAMETER_SET is not set

#
# Speech Filter (unvisible)
#

#
# Speech Coding Codec (unvisible)
#

#
# Music Codec
#
# CFG_AUDIO_CODEC_AACDEC is not set
# CFG_AUDIO_CODEC_MP2ENC is not set
# CFG_AUDIO_MGR_RESAMPLE is not set
# CFG_AUDIO_MGR_PARSING_MP3 is not set

#
# Audio Mgr Buffer Size Setting
#
set(CFG_AUDIO_MGR_THREAD_STACK_SIZE "64")
set(CFG_AUDIO_MGR_READ_BUFFER_SIZE "64")
set(CFG_AUDIO_SPECIAL_CASE_BUFFER_SIZE "64")
# CFG_BUILD_ASR is not set

#
# ASR Filter (unvisible)
#

#
# Video
#
# CFG_MPEGENCODER_ENABLE is not set
# CFG_UVC_ENABLE is not set

#
# Storage
#
# CFG_NAND_ENABLE is not set
set(CFG_NOR_CACHE_FLUSH_INTERVAL "0")
# CFG_NOR_ENABLE_DTRMODE is not set
# CFG_SD1_ENABLE is not set
set(CFG_MMC_ENABLE y)
# CFG_RAMDISK_ENABLE is not set
# CFG_UAS_ENABLE is not set

#
# File System
#
set(CFG_NOR_PARTITION3_SIZE "0")
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
set(CFG_I2C0_CLOCKRATE "400000")
# CFG_I2C2_ENABLE is not set
# CFG_I2C3_ENABLE is not set
set(CFG_I2S_INTERNAL_CODEC y)
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
set(CFG_UART2_ENABLE y)
# CFG_UART2_INTR is not set
# CFG_UART2_DMA is not set
set(CFG_UART2_FIFO y)
set(CFG_UART2_BAUDRATE "9600")
# CFG_UART2_RS485 is not set
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
set(CFG_TOUCH_I2C0 y)
set(CFG_TOUCH_CAPACITIVE y)
# CFG_TOUCH_RESISTIVE is not set
# CFG_TOUCH_MULTI_FINGER is not set
# CFG_TOUCH_BUTTON is not set
# CFG_LED_ENABLE is not set
set(CFG_CAPTURE_DEFINE "DEMO_9860_DEFAULT.txt")
# CFG_SENSOR_ENABLE is not set
# CFG_USB_HID_ENABLE is not set
# CFG_USB_SERIAL is not set
set(CFG_ALT_CPU_ENABLE y)
# CFG_RSL_MASTER is not set
# CFG_RSL_SLAVE is not set
# CFG_SW_PWM is not set
# CFG_PATTERN_GEN is not set
# CFG_OLED_CTRL is not set
# CFG_SW_UART is not set
# CFG_SW_SERIAL_PORT is not set
set(CFG_HOMEBUS y)
# CFG_CANBUS_ENABLE is not set

#
# Power
#
set(CFG_POWER_STANDBY y)
# CFG_POWER_SLEEP is not set
set(CFG_POWER_DOZE y)
# CFG_POWER_TICKLESS_IDLE is not set

#
# GPIO
#
set(CFG_GPIO_PWM_NUMBER "1")
set(CFG_GPIO_UART2_RX "37")
set(CFG_GPIO_UART2_TX "-1")
# CFG_GPIO_TOUCH_INT_ACTIVE_HIGH is not set
set(CFG_GPIO_HOMEBUS_TXD "35")

#
# Network
#

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
# CFG_DBG_UART2 is not set
# CFG_DBG_LCDCONSOLE is not set
# CFG_DBG_OSDCONSOLE is not set
set(CFG_DBG_ICE_SCRIPT "IT9070A1_Initial_DDR2_Mem_tiling_pitch2048_320MHZ.csf")
# CFG_DBG_BLUESCREEN is not set
set(CFG_DBG_STATS_PERIOD "10")
# CFG_DBG_STATS_MEM_BANDWIDTH is not set
# CFG_DBG_STATS_GPIO is not set
# CFG_DBG_STATS_FAT is not set
# CFG_ENABLE_UART_CLI is not set
# CFG_BOOT_TESTBIN_ENABLE is not set
# CFG_DBG_OUTPUT_DEBUG_FILES is not set

#
# Upgrade
#
# CFG_UPGRADE_BOOTLOADER_NAND is not set
set(CFG_UPGRADE_BOOTLOADER_NOR y)
# CFG_UPGRADE_BOOTLOADER_SD0 is not set
# CFG_UPGRADE_BOOTLOADER_SD1 is not set
# CFG_UPGRADE_BOOTLOADER_EXTERNAL_PROJECT is not set
# CFG_UPGRADE_IMAGE_NAND is not set
set(CFG_UPGRADE_IMAGE_NOR y)
# CFG_UPGRADE_IMAGE_SD0 is not set
# CFG_UPGRADE_IMAGE_SD1 is not set
# CFG_UPGRADE_PRIVATE_NAND is not set
set(CFG_UPGRADE_PRIVATE_NOR y)
# CFG_UPGRADE_PRIVATE_SD0 is not set
# CFG_UPGRADE_PRIVATE_SD1 is not set
# CFG_UPGRADE_PUBLIC_NAND is not set
set(CFG_UPGRADE_PUBLIC_NOR y)
# CFG_UPGRADE_PUBLIC_SD0 is not set
# CFG_UPGRADE_PUBLIC_SD1 is not set
# CFG_UPGRADE_TEMP_NAND is not set
set(CFG_UPGRADE_TEMP_NOR y)
# CFG_UPGRADE_TEMP_SD0 is not set
# CFG_UPGRADE_TEMP_SD1 is not set
set(CFG_UPGRADE_ENC_KEY "0")
# CFG_UPGRADE_PRESSKEY is not set
set(CFG_UPGRADE_FILENAME "ITEPKG03.PKG")
set(CFG_UPGRADE_FILENAME_LIST "ITEPKG03.PKG")
set(CFG_UPGRADE_DELAY_AFTER_FINISH "10")
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
# CFG_BUILD_RELEASE is not set
set(CFG_BUILD_MINSIZEREL y)
set(CFG_VERSION_MAJOR "2")
set(CFG_VERSION_MINOR "4")
set(CFG_VERSION_PATCH "3")
set(CFG_VERSION_CUSTOM "0")
set(CFG_MANUFACTURER "www.ite.com.tw")
# CFG_GENERATE_DOC is not set
# CFG_GENERATE_PACK_ENV is not set
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
set(CFG_BUILD_ZLIB y)

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
# CFG_SDL_M2D_ENABLE is not set
set(CFG_BUILD_PNG y)
set(CFG_BUILD_FREETYPE y)

#
# (unvisible)
#

#
# tslib
#
# CFG_TSLIB_STATIC_CONF is not set

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
# itu
#
set(CFG_ITU_ERR y)
set(CFG_ITU_WARN y)
set(CFG_ITU_INFO y)
# CFG_ITU_DBG is not set
# CFG_ITU_LOAD is not set
# CFG_ITU_UPDATE is not set
# CFG_ITU_DRAW is not set
# CFG_ITU_UCL_ENABLE is not set
# CFG_ITU_ASSERT_THREAD is not set
set(CFG_BUILD_ITU_RENDERER y)
set(CFG_BUILD_REDBLACK y)

#
# Display Control Board
#
set(CFG_HW_VERSION "V01")
# CFG_SCREENSHOT_ENABLE is not set
set(CFG_WEBSERVER_PORT "80")
set(CFG_ITU_PATH "itu/720x720")
set(CFG_UPGRADE_FTP_URL "ftp://192.168.1.1/ctrlboard/ITEPKG03.PKG")
# CFG_UPGRADE_GUI is not set
# CFG_DYNAMIC_LOAD_TP_MODULE is not set
# CFG_SHT20_ENABLE is not set
# CFG_NCP18_ENABLE is not set
