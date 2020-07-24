add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c  
)

add_executable(${CMAKE_PROJECT_NAME}
    test_rtc.c
)

if (DEFINED CFG_TEST_RTC_COUNT)
    add_definitions(
       -DCFG_TEST_RTC_COUNT
    )
elseif (DEFINED CFG_TEST_RTC_MS)
    add_definitions(
       -DCFG_TEST_RTC_MS
    )
elseif (DEFINED CFG_TEST_RTC_ALARM)
    add_definitions(
       -DCFG_TEST_RTC_ALARM
    )
endif()

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)	
