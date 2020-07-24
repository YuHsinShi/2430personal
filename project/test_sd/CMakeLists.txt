add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
    )
    
 ###### SD Card Test ########   
if (DEFINED CFG_TEST_SCAN_WINDOW)
      add_definitions(
          -DCFG_SD_INDEX=0
      )

    add_executable(${CMAKE_PROJECT_NAME}
        test_sd_window.c
        )
endif()    

if (DEFINED CFG_TEST_SINGLE_SD)
    if (DEFINED CFG_TEST_SD0)
        add_definitions(
            -DCFG_SD_INDEX=0
            )
    elseif (DEFINED CFG_TEST_SD2)
        add_definitions(
            -DCFG_SD_INDEX=1
            )
    endif()    

    add_executable(${CMAKE_PROJECT_NAME}
        test_sd_hcc.c
        )
endif()    

if (DEFINED CFG_TEST_TWO_SD)
    add_executable(${CMAKE_PROJECT_NAME}
        test_2sd_hcc.c
        )
endif()    

if (DEFINED CFG_TEST_ERASE)
    add_executable(${CMAKE_PROJECT_NAME}
        test_sd_erase.c
        )
endif()    


 ###### Ameba SDIO Test ########   
 if (DEFINED CFG_TEST_AMEBA_RAW)
    add_executable(${CMAKE_PROJECT_NAME}
        test_ameba.c
        )
elseif (DEFINED CFG_TEST_AMEBA_LWIP)
    add_definitions(
        -DCFG_TEST_AMEBA_LWIP
        )
    add_executable(${CMAKE_PROJECT_NAME}
        test_ameba_lwip.c
        )
elseif (DEFINED CFG_TEST_AMEBA_FTP)
    add_definitions(
        -DCFG_TEST_AMEBA_FTP
        )
    add_executable(${CMAKE_PROJECT_NAME}
        test_ameba_ftp.c
        )
endif()


 ###### Test SD Card and Ameba SDIO ########   
 if (DEFINED CFG_TEST_SD_AMEBA)
    add_definitions(
        -DCFG_TEST_SD_AMEBA
        )
 
    add_executable(${CMAKE_PROJECT_NAME}
        test_sd_ameba.c
        test_card.c
        #test_ameba_lwip.c
        test_ameba_ftp.c
        )
endif()



# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)
