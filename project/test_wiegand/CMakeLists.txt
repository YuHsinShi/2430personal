add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
    )

if (DEFINED CFG_TEST_WIEGAND)
    add_executable(${CMAKE_PROJECT_NAME}
        test_wiegand.c
        )
elseif (DEFINED CFG_TEST_READ_CARD)
    add_executable(${CMAKE_PROJECT_NAME}
        test_read_card.c
        )
elseif (DEFINED CFG_TEST_WIEGAND_OUT_O2I)
    add_executable(${CMAKE_PROJECT_NAME}
        test_wiegandout_o2i.c
        )
endif()

if (DEFINED CFG_TEST_FROM_WIEGAND0)
    add_definitions(
        -DCFG_TEST_FROM_WIEGAND0
    )
else (DEFINED CFG_TEST_FROM_WIEGAND1)
    add_definitions(
        -DCFG_TEST_FROM_WIEGAND1
    )
endif()

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)
