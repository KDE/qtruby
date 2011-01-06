find_PACKAGE (RUBY REQUIRED)

SET(UI_CLOCKCONFIG ${CMAKE_CURRENT_SOURCE_DIR}/digital_clock_config.ui)
SET(UI_CALENDAR ${CMAKE_CURRENT_SOURCE_DIR}/calendar.ui)
find_PROGRAM(RBUIC4 rbuic4 PATHS ${BIN_INSTALL_DIR})
if(RBUIC4)
	ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/digital_clock_config.rb COMMAND ${RBUIC4} ${UI_CLOCKCONFIG} -o ${CMAKE_CURRENT_BINARY_DIR}/digital_clock_config.rb COMMENT "Generating digital_clock_config.rb")
	ADD_CUSTOM_TARGET(digital_clock_config ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/digital_clock_config.rb)
	ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/calendar.rb COMMAND ${RBUIC4} ${UI_CALENDAR} -o ${CMAKE_CURRENT_BINARY_DIR}/calendar.rb COMMENT "Generating calendar.rb")
	ADD_CUSTOM_TARGET(calendar ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/calendar.rb)

	include_directories( ${RUBY_INCLUDE_DIRS} )

	install(FILES plasma-ruby-digital-clock-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})

	install(FILES clock.rb DESTINATION ${DATA_INSTALL_DIR}/plasma_ruby_digital_clock)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/digital_clock_config.rb DESTINATION ${DATA_INSTALL_DIR}/plasma_ruby_digital_clock)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/calendar.rb DESTINATION ${DATA_INSTALL_DIR}/plasma_ruby_digital_clock)
else(RBUIC4)
        MESSAGE(STATUS "rbuic4 not found, ruby-digital-clock will not created")
endif(RBUIC4)

