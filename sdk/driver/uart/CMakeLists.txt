if ($ENV{CFG_PLATFORM} STREQUAL openrtos)
	add_subdirectory(it${CFG_CHIP_FAMILY})
endif()