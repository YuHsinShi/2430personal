if (DEFINED CFG_MMC_ENABLE)
else()
	INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/sdk/driver/sd/mmc/include
	)
endif()


ADD_LIBRARY(esp32_sdio_at STATIC
		esp32_io.c
    )
