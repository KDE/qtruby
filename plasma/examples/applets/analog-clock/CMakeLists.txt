FIND_PACKAGE (RUBY REQUIRED)

SET(UI_CLOCKCONFIG ${CMAKE_CURRENT_SOURCE_DIR}/clockConfig.ui)
FIND_PROGRAM(RBUIC4 rbuic4 PATHS ${BIN_INSTALL_DIR})
if(RBUIC4)
	ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/clockConfig.rb COMMAND ${RBUIC4} ${UI_CLOCKCONFIG} -o ${CMAKE_CURRENT_BINARY_DIR}/clockConfig.rb COMMENT "Generating clockConfig.rb")
	ADD_CUSTOM_TARGET(clockConfig ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/clockConfig.rb)

	include_directories( ${RUBY_INCLUDE_PATH} )

	install(FILES plasma-ruby-clock-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})

	install(FILES clock.rb DESTINATION ${DATA_INSTALL_DIR}/plasma-ruby-clock)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/clockConfig.rb DESTINATION ${DATA_INSTALL_DIR}/plasma-ruby-clock)
else(RBUIC4)
        MESSAGE(STATUS "rbuic4 not found, ruby-clock will not created")
endif(RBUIC4)
