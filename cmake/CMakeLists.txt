set(QTRUBY4_EXPORT ${CMAKE_INSTALL_PREFIX}/share/qtruby4/cmake/QtRuby4Export.cmake)
set(QTRUBY4SHARED_LIBRARY QTRUBY4_qtruby4shared)

configure_file(QtRuby4Config.cmake.in
                    ${CMAKE_CURRENT_BINARY_DIR}/QtRuby4Config.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/QtRuby4Config.cmake DESTINATION ${CMAKE_INSTALL_PREFIX}/share/qtruby4/cmake)
