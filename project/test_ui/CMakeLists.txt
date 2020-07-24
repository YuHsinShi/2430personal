set(PROJ_NAME ${CMAKE_PROJECT_NAME})

if (DEFINED CFG_BOOTLOADER_ENABLE)
    set(CMAKE_PROJECT_NAME bootloader)
    add_subdirectory(../bootloader ../bootloader)
endif()

set(CMAKE_PROJECT_NAME ${PROJ_NAME})

if (DEFINED CFG_WATCHDOG_ENABLE)
    add_definitions(
        -DCFG_WATCHDOG_ENABLE
    )
endif()

add_definitions(
    -DCFG_WEBSERVER_PORT=${CFG_WEBSERVER_PORT}
    -DCFG_WEBSERVER_PORT_STR="${CFG_WEBSERVER_PORT}"
    -DCFG_HW_VERSION="${CFG_HW_VERSION}"
    -DCFG_UPGRADE_FTP_URL="${CFG_UPGRADE_FTP_URL}"
    )

if (DEFINED CFG_CHECK_FILES_CRC_ON_BOOTING)
    add_definitions(
        -DCFG_CHECK_FILES_CRC_ON_BOOTING
        )
endif()

if (DEFINED CFG_PLAY_VIDEO_ON_BOOTING)
    add_definitions(
        -DCFG_PLAY_VIDEO_ON_BOOTING
        -DCFG_BOOT_VIDEO_BGCOLOR=${CFG_BOOT_VIDEO_BGCOLOR}
        -DCFG_BOOT_VIDEO_VOLUME=${CFG_BOOT_VIDEO_VOLUME}
        )
endif()

if (DEFINED CFG_ENABLE_ROTATE)
    add_definitions(
        -DCFG_ENABLE_ROTATE
        )
endif()

if (DEFINED CFG_BOOT_VIDEO_ENABLE_WINDOW_MODE)
    add_definitions(
        -DCFG_BOOT_VIDEO_ENABLE_WINDOW_MODE
        -DCFG_VIDEO_WINDOW_X_POS=${CFG_VIDEO_WINDOW_X_POS}
        -DCFG_VIDEO_WINDOW_Y_POS=${CFG_VIDEO_WINDOW_Y_POS}
        -DCFG_VIDEO_WINDOW_WIDTH=${CFG_VIDEO_WINDOW_WIDTH}
        -DCFG_VIDEO_WINDOW_HEIGHT=${CFG_VIDEO_WINDOW_HEIGHT}
        )
endif()

if (DEFINED CFG_SCREENSHOT_ENABLE)
    add_definitions(
        -DCFG_SCREENSHOT_ENABLE
        )
endif()

if (DEFINED CFG_UPGRADE_GUI)
    add_definitions(
        -DCFG_UPGRADE_GUI
        )
    execute_process(COMMAND dataconv
        -x ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/${CFG_ITU_PATH}/upgrade.itu
        -o ${CMAKE_CURRENT_BINARY_DIR}/upgrade.inc
        )        
endif()

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/sdk/driver/mmp/include
    ${PROJECT_SOURCE_DIR}/sdk/share/audio_mgr
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg
    ${PROJECT_SOURCE_DIR}/sdk/share/microhttpd
    ${PROJECT_SOURCE_DIR}/sdk/share/microhttpd/include
    ${PROJECT_SOURCE_DIR}/sdk/share/wifi_mgr/ 
    ${PROJECT_SOURCE_DIR}/sdk/driver/itv

if (CFG_CHIP_FAMILY STREQUAL 9850)
    include_directories(
        ${PROJECT_SOURCE_DIR}/sdk/share/itu
        ${PROJECT_SOURCE_DIR}/sdk/share/itu/it9850
    )
else()
    include_directories(
        ${PROJECT_SOURCE_DIR}/sdk/share/itu
        ${PROJECT_SOURCE_DIR}/sdk/share/itu/it9070
    )
endif()
    )
	
file(REMOVE
	${CMAKE_BINARY_DIR}/data/private/UI_3D_Flip.itu
	${CMAKE_BINARY_DIR}/data/private/UI_3D_Flip.mainLayer
	${CMAKE_BINARY_DIR}/data/private/UI_Animation_Switching.itu
	${CMAKE_BINARY_DIR}/data/private/UI_Cartoon_ScaleCoverflow.itu
	${CMAKE_BINARY_DIR}/data/private/UI_Cartoon_ScaleCoverflow.mainLayer
	${CMAKE_BINARY_DIR}/data/private/UI_CoverFlow_overlap_MyFavoriteBread.itu
	${CMAKE_BINARY_DIR}/data/private/UI_Coverflow_Part_MyFavorite.itu
	${CMAKE_BINARY_DIR}/data/private/UI_Dynamic_Menu.itu
	${CMAKE_BINARY_DIR}/data/private/UI_Laundry.itu
	${CMAKE_BINARY_DIR}/data/private/UI_Menu_LinkSurface.itu
	${CMAKE_BINARY_DIR}/data/private/UI_Menu_LinkSurface.mainLayer
	${CMAKE_BINARY_DIR}/data/private/UI_MyFavorite.itu
	${CMAKE_BINARY_DIR}/data/private/UI_Roulette_Arc_Widgets.itu
	${CMAKE_BINARY_DIR}/data/private/UI_Water_level_Adjusting.itu
	)

if (DEFINED CFG_UI_3D_Flip)
    add_executable(${CMAKE_PROJECT_NAME}
        UI_3D_Flip.c
        )
	file(COPY
    itu/UI_3D_Flip.itu
	itu/UI_3D_Flip.mainLayer
    DESTINATION ${CMAKE_BINARY_DIR}/data/private
    )
elseif (DEFINED CFG_UI_MyFavorite)
    add_executable(${CMAKE_PROJECT_NAME}
        UI_MyFavorite.c
        )
	file(COPY
    itu/UI_MyFavorite.itu
    DESTINATION ${CMAKE_BINARY_DIR}/data/private
    )
elseif (DEFINED CFG_UI_Water_level_Adjusting)
    add_executable(${CMAKE_PROJECT_NAME}
        UI_Water_level_Adjusting.c
        )
	file(COPY
    itu/UI_Water_level_Adjusting.itu
    DESTINATION ${CMAKE_BINARY_DIR}/data/private
    )
elseif (DEFINED CFG_UI_Menu_LinkSurface)
    add_executable(${CMAKE_PROJECT_NAME}
        UI_Menu_LinkSurface.c
        )
	file(COPY
    itu/UI_Menu_LinkSurface.itu
	itu/UI_Menu_LinkSurface.mainLayer
    DESTINATION ${CMAKE_BINARY_DIR}/data/private
    )
elseif (DEFINED CFG_UI_Laundry)
    add_executable(${CMAKE_PROJECT_NAME}
        UI_Laundry.c
        )
	file(COPY
    itu/UI_Laundry.itu
    DESTINATION ${CMAKE_BINARY_DIR}/data/private
    )
elseif (DEFINED CFG_UI_Animation_Switching)
    add_executable(${CMAKE_PROJECT_NAME}
        UI_Animation_Switching.c
        )
	file(COPY
    itu/UI_Animation_Switching.itu
    DESTINATION ${CMAKE_BINARY_DIR}/data/private
    )
elseif (DEFINED CFG_UI_Cartoon_ScaleCoverflow)
    add_executable(${CMAKE_PROJECT_NAME}
        UI_Cartoon_ScaleCoverflow.c
        )
	file(COPY
    itu/UI_Cartoon_ScaleCoverflow.itu
	itu/UI_Cartoon_ScaleCoverflow.mainLayer
    DESTINATION ${CMAKE_BINARY_DIR}/data/private
    )
elseif (DEFINED CFG_UI_CoverFlow_overlap_MyFavoriteBread)
    add_executable(${CMAKE_PROJECT_NAME}
        UI_CoverFlow_overlap_MyFavoriteBread.c
        )
	file(COPY
    itu/UI_CoverFlow_overlap_MyFavoriteBread.itu
    DESTINATION ${CMAKE_BINARY_DIR}/data/private
    )
elseif (DEFINED CFG_UI_ARC)
    add_executable(${CMAKE_PROJECT_NAME}
        UI_Roulette_Arc_Widgets.c
        )
	file(COPY
    itu/UI_Roulette_Arc_Widgets.itu
    DESTINATION ${CMAKE_BINARY_DIR}/data/private
    )
elseif (DEFINED CFG_UI_COVERFLOW_PART_MYFAVORITE)
    add_executable(${CMAKE_PROJECT_NAME}
        UI_Coverflow_Part_MyFavorite.c
        )
	file(COPY
    itu/UI_Coverflow_Part_MyFavorite.itu
    DESTINATION ${CMAKE_BINARY_DIR}/data/private
    )
elseif (DEFINED CFG_UI_DYNAMIC_MENU)
    add_executable(${CMAKE_PROJECT_NAME}
        UI_Dynamic_Menu.c
        )
	file(COPY
    itu/UI_Dynamic_Menu.itu
    DESTINATION ${CMAKE_BINARY_DIR}/data/private
    )
endif()

if (DEFINED CFG_NET_ENABLE)
    add_executable(${CMAKE_PROJECT_NAME}
        network.c    
        webserver.c
        )
endif()

# # boot video
# if (DEFINED CFG_PLAY_VIDEO_ON_BOOTING)
	# add_executable(${CMAKE_PROJECT_NAME}
		# video.c
		# )
		
	# file(COPY
		# ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/media/boot/boot.mp4
		# DESTINATION ${CMAKE_BINARY_DIR}/data/private/media
		# )
# endif()

# if (DEFINED CFG_PLAY_MJPEG_ON_BOOTING)
	# add_executable(${CMAKE_PROJECT_NAME}
		# video_mjpeg.c
		# )
		
	# file(COPY
        # ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/media/boot/${CFG_BOOT_MJPEG_FILE}
		# DESTINATION ${CMAKE_BINARY_DIR}/data/private/media
		# )
# endif()

# files
configure_file(${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/ctrlboard.ini ${CMAKE_BINARY_DIR}/data/public/ctrlboard.ini COPYONLY)


# if (DEFINED CFG_NET_WIFI_WPA_ENABLE)
# configure_file(
    # ${PROJECT_SOURCE_DIR}/sdk/share/wifi_hostapd/hostapd_wpa.conf.in
    # ${CMAKE_BINARY_DIR}/data/public/hostapd.conf
    # NEWLINE_STYLE UNIX
    # )
# else()
# configure_file(
    # ${PROJECT_SOURCE_DIR}/sdk/share/wifi_hostapd/hostapd.conf.in
    # ${CMAKE_BINARY_DIR}/data/public/hostapd.conf
    # NEWLINE_STYLE UNIX
    # )
# endif()

# backups
configure_file(${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/ctrlboard.ini ${CMAKE_BINARY_DIR}/data/private/backup/${CFG_PUBLIC_DRIVE}/ctrlboard.ini COPYONLY)
# crc
if (DEFINED CFG_CHECK_FILES_CRC_ON_BOOTING)
    file(REMOVE ${CMAKE_BINARY_DIR}/data/private/backup/${CFG_PUBLIC_DRIVE}/ite_crc.dat)
    execute_process(COMMAND makecrc -c ${CMAKE_BINARY_DIR}/data/public/ite_crc.dat -i ${CMAKE_BINARY_DIR}/data/private/backup/${CFG_PUBLIC_DRIVE})
    execute_process(COMMAND makecrc -v ${CMAKE_BINARY_DIR}/data/public/ite_crc.dat -i ${CMAKE_BINARY_DIR}/data/private/backup/${CFG_PUBLIC_DRIVE})
    file(COPY ${CMAKE_BINARY_DIR}/data/public/ite_crc.dat DESTINATION ${CMAKE_BINARY_DIR}/data/private/backup/${CFG_PUBLIC_DRIVE})
endif()

# font
file(COPY
    ${PROJECT_SOURCE_DIR}/data/font/wqyMicroHei.ttf
    DESTINATION ${CMAKE_BINARY_DIR}/data/private/font
    )

# web
file(GLOB files ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/web/*.*)
foreach (src ${files})
    string(REPLACE "${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/web/" "${CMAKE_BINARY_DIR}/data/private/web/" dest ${src})
    configure_file(${src} ${dest} COPYONLY)
endforeach()

# screen saver
file(COPY
    ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/screensaver.jpg
    DESTINATION ${CMAKE_BINARY_DIR}/data/public
    )

# codec
if (DEFINED CFG_COMPRESS_AUDIO_PLUGIN)
    configure_file(${CMAKE_BINARY_DIR}/lib/sm32/mp3.codecs ${CMAKE_BINARY_DIR}/data/private/codec/mp3.codecs COPYONLY)
    configure_file(${CMAKE_BINARY_DIR}/lib/sm32/aac.codecs ${CMAKE_BINARY_DIR}/data/private/codec/aac.codecs COPYONLY)    
    configure_file(${CMAKE_BINARY_DIR}/lib/sm32/wma.codecs ${CMAKE_BINARY_DIR}/data/private/codec/wma.codecs COPYONLY)
    configure_file(${CMAKE_BINARY_DIR}/lib/sm32/wave.codecs ${CMAKE_BINARY_DIR}/data/private/codec/wave.codecs COPYONLY)
endif()

if (DEFINED CFG_NET_WIFI_WPA_ENABLE)
configure_file(
    ${PROJECT_SOURCE_DIR}/sdk/share/wifi_hostapd/hostapd_wpa.conf.in
    ${CMAKE_BINARY_DIR}/data/private/backup/${CFG_PUBLIC_DRIVE}/hostapd.conf
    NEWLINE_STYLE UNIX
    )
else()    
configure_file(
    ${PROJECT_SOURCE_DIR}/sdk/share/wifi_hostapd/hostapd.conf.in
    ${CMAKE_BINARY_DIR}/data/private/backup/${CFG_PUBLIC_DRIVE}/hostapd.conf
    NEWLINE_STYLE UNIX
    )
endif()

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)
