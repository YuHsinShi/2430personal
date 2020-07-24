if (DEFINED CFG_NET_WIFI_SDIO_NGPL)

	INCLUDE_DIRECTORIES(
			${PROJECT_SOURCE_DIR}/sdk/share/wifi_mgr/
			${PROJECT_SOURCE_DIR}/sdk/driver/non_gpl_wifi/include/
			${PROJECT_SOURCE_DIR}/sdk/driver/non_gpl_wifi/api/wifi/
			${PROJECT_SOURCE_DIR}/sdk/share/dhcps/
			${PROJECT_SOURCE_DIR}/sdk/include/lwip/       
		)

add_library(wifi_mgr STATIC
    wifiMgr_ngpl.c
    wifiMgr.h
    )
else()
add_library(wifi_mgr STATIC
    wifiMgr.c
    wifiMgr.h
    )
endif()    
