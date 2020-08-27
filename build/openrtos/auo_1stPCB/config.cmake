#
# Automatically generated file; DO NOT EDIT.
# Project Configuration
# Thu Aug 27 11:53:17 2020
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
set(CFG_ENABLE_ROTATE y)
set(CFG_LCD_WIDTH "480")
set(CFG_LCD_HEIGHT "480")
set(CFG_LCD_PITCH "960")
set(CFG_LCD_INIT_SCRIPT "lcd_script-HS0 - AUO1st.txt")
set(CFG_CMDQ_ENABLE y)
set(CFG_M2D_ENABLE y)
set(CFG_JPEG_HW_ENABLE y)
# CFG_AUDIO_ENABLE is not set
set(CFG_AUDIO_DAC_MODULE "itp_dac_ite970.c")
set(CFG_AUDIO_CODEC_MP3DEC y)
set(CFG_AUDIO_CODEC_WMADEC y)
set(CFG_AUDIO_CODEC_WAV y)
set(CFG_COMPRESS_AUDIO_PLUGIN y)
set(CFG_VIDEO_ENABLE y)
set(CFG_NOR_ENABLE y)
set(CFG_NOR_USE_AXISPI y)
set(CFG_NOR_ENABLE_QUADMODE y)

#
# (unvisible)
#
set(CFG_NOR_CACHE_SIZE "0x200000")
set(CFG_NOR_RESERVED_SIZE "0x250000")
set(CFG_NOR_PARTITION0 y)
set(CFG_NOR_PARTITION0_SIZE "0xAF0000")
set(CFG_NOR_PARTITION1 y)
set(CFG_NOR_PARTITION1_SIZE "0x260000")
set(CFG_NOR_PARTITION2 y)
set(CFG_NOR_PARTITION2_SIZE "0x40000")
set(CFG_NOR_PARTITION3 y)
# CFG_SD0_ENABLE is not set
set(CFG_GPIO_SD0_CARD_DETECT "22")
set(CFG_GPIO_SD0_IO "14, 13, 15, 16, 17, 18, -1, -1, -1, -1")

#
# (unvisible)
#
set(CFG_GPIO_SD1_IO "42, 43, 44,45,46, 47, -1, -1, -1, -1")
set(CFG_SDIO_ENABLE y)
set(CFG_SDIO1_STATIC y)

#
# (unvisible)
#
# CFG_NET_WIFI_REDEFINE is not set

#
# (unvisible)
#
set(CFG_USB0_ENABLE y)
set(CFG_MSC_ENABLE y)
set(CFG_FS_FAT y)
set(CFG_RTC_ENABLE y)
# CFG_SPI_ENABLE is not set
set(CFG_I2C1_ENABLE y)
set(CFG_GPIO_IIC1_CLK "47")
set(CFG_GPIO_IIC1_DATA "46")
# CFG_TOUCH_ENABLE is not set
set(CFG_TOUCH_MODULE "gt911")
set(CFG_TOUCH_X_MAX_VALUE "0x4FF")
set(CFG_TOUCH_Y_MAX_VALUE "0x1DF")
set(CFG_TOUCH_ADVANCE_CONFIG y)
set(CFG_TOUCH_REVERSE_X y)
set(CFG_GPIO_TOUCH_INT "44")
set(CFG_GPIO_TOUCH_RESET "45")
set(CFG_GPIO_TOUCH_WAKE "-1")
set(CFG_DEF_TOUCH_I2C1 y)
set(CFG_TOUCH_INTR y)
# CFG_AMPLIFIER_ENABLE is not set
set(CFG_GPIO_AMPLIFIER_ENABLE "23")
set(CFG_GPIO_AMPLIFIER_MUTE "23")
set(CFG_I2S_ENABLE y)
# CFG_I2S_SPDIF_ENABLE is not set
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
set(CFG_GPIO_BACKLIGHT_PWM "60")
set(CFG_NET_ENABLE y)
set(CFG_TASK_DRIVE_PROBE y)
set(CFG_CANBUS_ENABLE y)
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
# CFG_NET_ETHERNET is not set
set(CFG_NET_ETHERNET_LINK_INTR y)
set(CFG_NET_ETHERNET_MAC_ADDR_RANDOM y)

#
# (unvisible)
#

#
# (unvisible)
#
set(CFG_NET_DHCP_SERVER y)
set(CFG_DHCPS_ADDR_COUNT "254")
set(CFG_PING y)
set(CFG_BUILD_MICROHTTPD y)
set(CFG_BUILD_XML2 y)
set(CFG_NET_HTTP y)
# CFG_BOOTLOADER_ENABLE is not set

#
# (unvisible)
#
# CFG_BL_SHOW_LOGO is not set

#
# (unvisible)
#
# CFG_UPGRADE_BOOTLOADER is not set
# CFG_UPGRADE_IMAGE is not set
# CFG_UPGRADE_DATA is not set

#
# (unvisible)
#

#
# (unvisible)
#
# CFG_CHECK_FILES_CRC_ON_BOOTING is not set
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
# CFG_SD_DUAL_BOOT is not set
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

#
# Speech Filter (unvisible)
#

#
# Speech Coding Codec (unvisible)
#

#
# Music Codec
#
# CFG_AUDIO_MGR_RESAMPLE is not set
# CFG_AUDIO_MGR_PARSING_MP3 is not set

#
# Audio Mgr Buffer Size Setting
#
set(CFG_AUDIO_MGR_THREAD_STACK_SIZE "64")
set(CFG_AUDIO_MGR_READ_BUFFER_SIZE "64")
set(CFG_AUDIO_SPECIAL_CASE_BUFFER_SIZE "64")

#
# ASR Filter (unvisible)
#

#
# Video
#
# CFG_MPEGENCODER_ENABLE is not set
# CFG_MULTICAST_ENABLE is not set
# CFG_VIDEO_FRAME_FUN_ENABLE is not set
# CFG_RTSP_CLIENT_ENABLE is not set
# CFG_UVC_ENABLE is not set
# CFG_MJPEG_DEC_ENABLE is not set

#
# Storage
#
# CFG_NAND_ENABLE is not set
set(CFG_NOR_CACHE_FLUSH_INTERVAL "0")
# CFG_NOR_ENABLE_DTRMODE is not set
set(CFG_SD1_ENABLE y)
# CFG_SD1_NO_PIN_SHARE is not set
# CFG_SD1_STATIC is not set
# CFG_SD1_CARD_1BIT is not set
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
set(CFG_RTC_MODULE "itp_rtc_internal.c")
# CFG_RTC_USE_I2C is not set
# CFG_RTC_REDUCE_IO_ACCESS_ENABLE is not set
set(CFG_RTC_DEFAULT_TIMESTAMP "1325376000")
set(CFG_INTERNAL_RTC_TIMER "5")
# CFG_I2C0_ENABLE is not set
set(CFG_I2C1_CLOCKRATE "400000")
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
set(CFG_UART1_ENABLE y)
set(CFG_UART1_INTR y)
# CFG_UART1_DMA is not set
# CFG_UART1_FIFO is not set
set(CFG_UART1_BAUDRATE "9600")
set(CFG_UART1_RS485 y)
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
# CFG_LED_ENABLE is not set
set(CFG_CAPTURE_DEFINE "DEMO_9860_DEFAULT.txt")
# CFG_SENSOR_ENABLE is not set
# CFG_USB_HID_ENABLE is not set
# CFG_USB_SERIAL is not set
# CFG_SDIO0_STATIC is not set
set(CFG_SDIO_4BIT_MODE y)
# CFG_ESP32_SDIO_AT is not set
# CFG_ALT_CPU_ENABLE is not set
set(CFG_SARADC_ENABLE y)
set(CFG_SARADC_VALID_XAIN "0xe1")
set(CFG_SARADC_CALIBRATION_ENABLE y)
set(CFG_SARADC_CALIBRATION_EXTERNAL y)
# CFG_SARADC_CALIBRATION_INTERNAL is not set
set(CFG_SARADC_CALIBRATION_XAIN "0xe1")
set(CFG_SARADC_CALIBRATION_VOLTAGE "0.55,2.75")

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
set(CFG_GPIO_SD1_POWER_ENABLE "-1")
set(CFG_GPIO_SD1_WIFI_POWER_ENABLE y)
set(CFG_GPIO_SD1_WIFI_POWER_PIN "52")
set(CFG_SD1_WIFI_DETECT_ACTIVE_HIGH y)
set(CFG_GPIO_SD1_WRITE_PROTECT "-1")
set(CFG_GPIO_PWM_NUMBER "1")
set(CFG_GPIO_LCD_PWR_EN "59")
# CFG_GPIO_LCD_PWR_EN_ACTIVE_LOW is not set
set(CFG_GPIO_UART1_RX "33")
set(CFG_GPIO_UART1_TX "32")
set(CFG_GPIO_RS485_1_TX_ENABLE "31")
set(CFG_GPIO_ETHERNET_LINK "23")
set(CFG_GPIO_XAIN_0_RX "19")
set(CFG_GPIO_XAIN_1_RX "20")
set(CFG_GPIO_XAIN_2_RX "21")
set(CFG_GPIO_XAIN_3_RX "22")
set(CFG_GPIO_XAIN_4_RX "23")
set(CFG_GPIO_XAIN_5_RX "24")
set(CFG_GPIO_XAIN_6_RX "25")
set(CFG_GPIO_XAIN_7_RX "26")

#
# Network
#

#
# (unvisible)
#
# CFG_NET_ETHERNET_4G is not set
set(CFG_NET_WIFI y)
set(CFG_NET_WIFI_MGR y)
set(CFG_NET_WIFI_POLL_INTERVAL "50")
# CFG_NET_WIFI_8188EUS is not set
set(CFG_NET_WIFI_SDIO_NGPL y)
# CFG_NET_WIFI_SDIO_SSV is not set
# CFG_NET_WIFI_MAY_NOT_EXIST is not set
# CFG_NET_WIFI_HOSTAPD is not set
# CFG_NET_WIFI_TRANS_LED is not set
set(CFG_NET_WIFI_MP_SSID "audiolink")
set(CFG_NET_WIFI_MP_PASSWORD "12345678")
set(CFG_NET_WIFI_MP_SECURITY "0")

#
# (unvisible)
#
# CFG_NET_WIFI_VIDEO_DOORBELL is not set

#
# (unvisible)
#
# CFG_NET_FTP is not set
# CFG_NET_TFTP is not set
# CFG_NET_SMTP is not set
# CFG_NET_UPNP is not set
# CFG_NET_DNS_SD is not set
# CFG_BLOCK_UDP is not set
# CFG_SIP_PROXD_TEST is not set
# CFG_SIP_SERVER_TEST is not set
# CFG_P2P is not set

#
# Task
#

#
# Debug
#
# CFG_DBG_NONE is not set
# CFG_DBG_PRINTBUF is not set
# CFG_DBG_SWUART_CODEC is not set
# CFG_DBG_UART0 is not set
set(CFG_DBG_UART1 y)
# CFG_DBG_LCDCONSOLE is not set
# CFG_DBG_OSDCONSOLE is not set
set(CFG_DBG_ICE_SCRIPT "IT9070A1_Initial_DDR2_Mem_tiling_pitch2048_320MHZ.csf")
# CFG_DBG_BLUESCREEN is not set
set(CFG_DBG_STATS_PERIOD "10")
# CFG_DBG_STATS_MEM_BANDWIDTH is not set
# CFG_DBG_STATS_GPIO is not set
# CFG_DBG_STATS_TCPIP is not set
# CFG_DBG_STATS_FAT is not set
# CFG_ENABLE_UART_CLI is not set
# CFG_DBG_OUTPUT_DEBUG_FILES is not set

#
# Upgrade
#
set(CFG_UPGRADE_ENC_KEY "0")
set(CFG_UPGRADE_FILENAME "ITEPKG03.PKG")
set(CFG_UPGRADE_FILENAME_LIST "ITEPKG03.PKG")
set(CFG_UPGRADE_USB_DETECT_TIMEOUT "200")
set(CFG_UPGRADE_USB_TIMEOUT "8000")
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
# ffmpeg
#
# CFG_FFMPEG_ENC is not set
# CFG_FFMPEG_H264_SW is not set

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
set(CFG_BUILD_FFMPEG y)

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
set(CFG_ITU_PATH "itu/480x480")
set(CFG_UPGRADE_FTP_URL "ftp://192.168.1.1/ctrlboard/ITEPKG03.PKG")
# CFG_UPGRADE_GUI is not set
# CFG_DYNAMIC_LOAD_TP_MODULE is not set
