add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c  
)

if (DEFINED CFG_TEST_IR_RX)
    add_executable(${CMAKE_PROJECT_NAME}
        test_ir_rx.c
        )
elseif (DEFINED CFG_TEST_IR_TXRX)
    add_executable(${CMAKE_PROJECT_NAME}
        test_ir_txrx.c
        )
elseif (DEFINED CFG_TEST_IR_SAMPLE)
    add_executable(${CMAKE_PROJECT_NAME}
        test_clk_sample.c
        )
endif()

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)	
