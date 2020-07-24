add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
    )
		
if (DEFINED CFG_TEST_GPIO_INPUT)
    add_executable(${CMAKE_PROJECT_NAME}
        gpio_input.c
        )
		
	add_definitions(
        -DCFG_GPIO_TEST_INPUT_PIN=${CFG_GPIO_TEST_INPUT_PIN}
		)
		
elseif (DEFINED CFG_TEST_GPIO_OUTPUT_TO_INPUT)
    add_executable(${CMAKE_PROJECT_NAME}
        gpio_output_to_input.c
        )
	
	add_definitions(
        -DCFG_GPIO_TEST_INPUT_PIN=${CFG_GPIO_TEST_INPUT_PIN}
        -DCFG_GPIO_TEST_OUTPUT_PIN=${CFG_GPIO_TEST_OUTPUT_PIN}
		)
		
elseif (DEFINED CFG_TEST_GPIO_INTERRUPT)
    add_executable(${CMAKE_PROJECT_NAME}
        gpio_interrupt.c
        )
		
	add_definitions(
        -DCFG_GPIO_TEST_INTR_PIN=${CFG_GPIO_TEST_INTR_PIN}
		)
		
elseif (DEFINED CFG_TEST_GPIO_EX_4)
    add_executable(${CMAKE_PROJECT_NAME}
        test_gpio_ex4.c
        )
endif()

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)
