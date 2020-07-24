include_directories(
    .
    ..
    ${PROJECT_SOURCE_DIR}/sdk/include/uart
	${CMAKE_CURRENT_BINARY_DIR}
)

add_library(uart STATIC
    uart.c
	uart_intr.c
	uart_dma.c
	uart_fifo.c
)

if (CFG_UART0_RS485 OR CFG_UART1_RS485
 OR CFG_UART2_RS485 OR CFG_UART3_RS485
 OR CFG_UART4_RS485 OR CFG_UART5_RS485)
add_library(rs485 STATIC interface/rs485.c)
endif()
