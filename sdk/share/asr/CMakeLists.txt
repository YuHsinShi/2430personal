add_definitions(
        -DASR_FN_ASR_H_
        )
        
if (DEFINED CFG_BUILD_ASR)
    #add_library(asr STATIC  
    #    asr.c
    #)
	file(COPY ${CFG_ASR_LIB_MODULE} 
         DESTINATION ${CMAKE_BINARY_DIR}/lib/${CFG_CPU_NAME}
		)
    file(RENAME ${CMAKE_BINARY_DIR}/lib/${CFG_CPU_NAME}/${CFG_ASR_LIB_MODULE}
               ${CMAKE_BINARY_DIR}/lib/${CFG_CPU_NAME}/libasr.a
        )
else()
    file(COPY ${CFG_ASR_LIB_MODULE} 
         DESTINATION ${CMAKE_BINARY_DIR}/lib/${CFG_CPU_NAME}
		)
endif()
