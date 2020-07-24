if (DEFINED CFG_NET_WIFI_SDIO_SSV)
  
	INCLUDE_DIRECTORIES(
        ${PROJECT_SOURCE_DIR}/sdk/driver/wifi_ssv_6x5x
		
        ${PROJECT_SOURCE_DIR}/sdk/driver/wifi_ssv_6x5x/include
		${PROJECT_SOURCE_DIR}/sdk/driver/wifi_ssv_6x5x/include/priv
		${PROJECT_SOURCE_DIR}/sdk/driver/wifi_ssv_6x5x/include/priv/hw
		
		${PROJECT_SOURCE_DIR}/sdk/driver/wifi_ssv_6x5x/ap
		${PROJECT_SOURCE_DIR}/sdk/driver/wifi_ssv_6x5x/ap/common
		${PROJECT_SOURCE_DIR}/sdk/driver/wifi_ssv_6x5x/ap/crypto
		
		${PROJECT_SOURCE_DIR}/sdk/driver/wifi_ssv_6x5x/app/cli
		${PROJECT_SOURCE_DIR}/sdk/driver/wifi_ssv_6x5x/app/cli/cmds
		
		${PROJECT_SOURCE_DIR}/sdk/driver/wifi_ssv_6x5x/app/netapp
		
		${PROJECT_SOURCE_DIR}/sdk/driver/wifi_ssv_6x5x/app/netmgr
		
		${PROJECT_SOURCE_DIR}/sdk/driver/wifi_ssv_6x5x/core
		
		${PROJECT_SOURCE_DIR}/sdk/driver/wifi_ssv_6x5x/drv
		${PROJECT_SOURCE_DIR}/sdk/driver/wifi_ssv_6x5x/drv/sdio
		
		${PROJECT_SOURCE_DIR}/sdk/driver/wifi_ssv_6x5x/hal
		${PROJECT_SOURCE_DIR}/sdk/driver/wifi_ssv_6x5x/hal/SSV6006
		${PROJECT_SOURCE_DIR}/sdk/driver/wifi_ssv_6x5x/hal/SSV6030
		
		${PROJECT_SOURCE_DIR}/sdk/driver/wifi_ssv_6x5x/mac
		
		${PROJECT_SOURCE_DIR}/sdk/driver/wifi_ssv_6x5x/netstack_wrapper/icomm_lwIP

		${PROJECT_SOURCE_DIR}/sdk/driver/wifi_ssv_6x5x/os_wrapper/FreeRTOS
		
		${PROJECT_SOURCE_DIR}/sdk/driver/wifi_ssv_6x5x/platform/linux_customer
		
		${PROJECT_SOURCE_DIR}/sdk/driver/sd/mmc/include
		${PROJECT_SOURCE_DIR}/sdk/share/dhcps/
		${PROJECT_SOURCE_DIR}/sdk/include/lwip/        
		)   

		
	ADD_LIBRARY(wifi_ssv_6x5x STATIC
		#wifi_ssv_6x5x
		country_cfg.c
		custom_cfg.c
		init.c
		macaddr.c
		main.c
		ssv_frame.c
		version.c

		#wifi_ssv_6x5x/ap
		ap/ap.c
		ap/ap_config.c
		ap/ap_drv_cmd.c
		ap/ap_mlme.c
		ap/ap_rx.c
		ap/ap_sta_info.c
		ap/ap_tx.c
		ap/beacon.c
		ap/hw_config.c
		ap/ieee802_11_ht.c
		ap/ieee802_11_mgmt.c
		ap/neighbor_ap_list.c
		ap/random.c
		ap/testcase.c
		ap/wmm.c
		ap/wpa_auth.c
		ap/wpa_auth_ie.c
		ap/wpa_debug.c
		
		#wifi_ssv_6x5x/ap/common
		ap/common/bitmap.c
		ap/common/ieee802_11_common.c
		ap/common/wpa_common.c
		
		#wifi_ssv_6x5x/ap/crypto
		ap/crypto/aes-internal-enc.c
		ap/crypto/aes-internal.c
		ap/crypto/aes-unwrap.c
		ap/crypto/aes-internal-dec.c
		ap/crypto/sha1-internal.c
		ap/crypto/rc4.c
		ap/crypto/sha1.c
		ap/crypto/aes-wrap.c
		ap/crypto/crypto_internal.c
		
		#wifi_ssv_6x5x/app/cli
		app/cli/cli.c
		
		#wifi_ssv_6x5x/app/cli/cmds
		app/cli/cmds/cli_cmd.c
		app/cli/cmds/cli_cmd_at.c
		app/cli/cmds/cli_cmd_net.c
		app/cli/cmds/cli_cmd_soc.c
		app/cli/cmds/cli_cmd_sys.c
		app/cli/cmds/cli_cmd_wifi.c
		
		#wifi_ssv_6x5x/app/netapp
		app/netapp/getopt.c
		app/netapp/net_app.c
		
		#wifi_ssv_6x5x/app/netmgr
		app/netmgr/net_mgr.c

		#wifi_ssv_6x5x/core
		core/channel.c
		core/host_apis.c
		core/host_cmd_engine.c
		core/host_cmd_engine_rx.c
		core/host_cmd_engine_tx.c
		core/mlme.c
		core/recover.c
		core/Regulatory.c
		core/txrx_hdl.c
		core/txrx_task.c
		
		#wifi_ssv_6x5x/drv
		drv/ssv_drv.c
		
		#wifi_ssv_6x5x/drv/sdio
		drv/sdio/sdio_delay_tuning.c
		drv/sdio/sdio_if_impl.c
		
		#wifi_ssv_6x5x/hal
		hal/ssv_hal.c
		
		#wifi_ssv_6x5x/hal/SSV6006
		hal/SSV6006/ssv6006_beacon.c
		hal/SSV6006/ssv6006_pkt.c
		hal/SSV6006/ssv6006_data_flow.c
		hal/SSV6006/ssv6006_hal.c
		hal/SSV6006/ssv6006_decision_tbl.c
		hal/SSV6006/turismo_common.c
		hal/SSV6006/ssv6006_efuse.c
		
		#wifi_ssv_6x5x/hal/SSV6030
		hal/SSV6030/ssv6030_beacon.c
		hal/SSV6030/ssv6030_pkt.c
		hal/SSV6030/ssv6030_data_flow.c
		hal/SSV6030/ssv6030_hal.c
		hal/SSV6030/ssv6030_decision_tbl.c
		hal/SSV6030/ssv6030_efuse.c
		
		#wifi_ssv_6x5x/lib
		lib/efuse.c
		lib/lib-impl.c
		lib/ssv_lib.c
		lib/ssv_msg.c
		lib/ssv_timer.c

		#wifi_ssv_6x5x/lib/apps
		lib/apps/host_global.c
		lib/apps/host_log.c
		lib/apps/msgevt.c
		lib/apps/pbuf.c

		#wifi_ssv_6x5x/mac
		mac/ap_cfg.c
		mac/sta_cfg.c

		#wifi_ssv_6x5x/netstack_wrapper/icomm_lwIP
		netstack_wrapper/icomm_lwIP/netstack.c

		#os_wrapper/FreeRTOS
		os_wrapper/FreeRTOS/rtos.c
		
		#wifi_ssv_6x5x/platform/linux_customer
		platform/linux_customer/porting.c
		platform/linux_customer/sdio_port.c
		platform/linux_customer/ssvdevice.c
		)

endif()