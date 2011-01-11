get_target_property(QTRUBY4SHARED_LIBRARY qtruby4shared LOCATION) 
configure_file(QtRuby4Config.cmake.in
                    ${CMAKE_CURRENT_BINARY_DIR}/QtRuby4Config.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/QtRuby4Config.cmake DESTINATION ${CMAKE_INSTALL_PREFIX}/share/qtruby4/cmake)
