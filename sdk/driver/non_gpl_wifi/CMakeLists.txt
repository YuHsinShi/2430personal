if (DEFINED CFG_NET_WIFI_SDIO_NGPL)
  
	INCLUDE_DIRECTORIES(
	${PROJECT_SOURCE_DIR}/sdk/driver/non_gpl_wifi
        ${PROJECT_SOURCE_DIR}/sdk/driver/non_gpl_wifi/include
		#${PROJECT_SOURCE_DIR}/sdk/driver/non_gpl_wifi/include/ports
		${PROJECT_SOURCE_DIR}/sdk/driver/non_gpl_wifi/api/wifi/		
		${PROJECT_SOURCE_DIR}/sdk/driver/usb/inc
		${PROJECT_SOURCE_DIR}/sdk/driver/non_gpl_wifi/os/customer_rtos/
		${PROJECT_SOURCE_DIR}/sdk/driver/non_gpl_wifi/os/os_dep/include/
        ${PROJECT_SOURCE_DIR}/sdk/driver/non_gpl_wifi/platform/include
		${PROJECT_SOURCE_DIR}/sdk/driver/non_gpl_wifi/platform/rtwlan_bsp/
		${PROJECT_SOURCE_DIR}/sdk/driver/non_gpl_wifi/platform/sdio/core/
		${PROJECT_SOURCE_DIR}/sdk/driver/non_gpl_wifi/platform/sdio/include/
		${PROJECT_SOURCE_DIR}/sdk/driver/sd/mmc/include
		${PROJECT_SOURCE_DIR}/sdk/share/dhcps/
		${PROJECT_SOURCE_DIR}/sdk/include/lwip/        
        
		)

  LINK_LIBRARIES(${PROJECT_SOURCE_DIR}/sdk/driver/non_gpl_wifi/wlan_lib/wlan_lib.a)    

		
	ADD_LIBRARY(non_gpl_wifi STATIC
		api/wifi/wifi_conf.c
		api/wifi/wifi_ind.c
		api/wifi/wifi_promisc.c
		api/wifi/wifi_util.c
		api/wifi_interactive_mode.c
		
		os/customer_rtos/customer_rtos_service.c
		os/os_dep/device_lock.c

		os/os_dep/net_stack_intf.c
		os/os_dep/osdep_service.c    
		
		platform/rtwlan_bsp/rtwlan_bsp.c
		platform/sdio/wifi_io.c
	
		#os/os_dep/ports/
		#os/os_dep/ports/ite_ndis.c    
		#os/os_dep/ports/ite_os.c
		#os/os_dep/ports/ite_skbuf.c
		#os/os_dep/ports/ite_timer.c   
		#os/os_dep/ports/ite_util.c
		#os/os_dep/ports/workqueue.c


		)
		
	target_link_libraries(non_gpl_wifi ${PROJECT_SOURCE_DIR}/sdk/driver/non_gpl_wifi/wlan_lib/wlan_lib.a)
endif()