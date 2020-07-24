if (DEFINED CFG_ALT_CPU_ENABLE)

    if (DEFINED CFG_RSL_MASTER)
        add_subdirectory(rslMaster)
    endif()

    if (DEFINED CFG_RSL_SLAVE)
        add_subdirectory(rslSlave)
    endif()

    if (DEFINED CFG_SW_PWM)
        add_subdirectory(swPwm)
    endif()

    if (DEFINED CFG_ALT_CPU_CUSTOM_DEVICE)
        add_subdirectory(customDevice)
    endif()

    if (DEFINED CFG_PATTERN_GEN)
        add_subdirectory(patternGen)
    endif()
    
    if (DEFINED CFG_SW_UART)
        add_subdirectory(swUart)
    endif()
    
    if (DEFINED CFG_SW_SERIAL_PORT)
        add_subdirectory(swSerialPort)
    endif()
    
    if (DEFINED CFG_OLED_CTRL)
        add_subdirectory(oledCtrl)
    endif()
endif()
