include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES})

set(rubyqsci_LIBRARY_SRC qscintilla.cpp qscintillahandlers.cpp)
add_library(rubyqscintilla MODULE ${rubyqsci_LIBRARY_SRC})
target_link_libraries(rubyqscintilla ${QT_QTCORE_LIBRARY} ${RUBY_LIBRARY} smokeqt smokeqsci qtruby4shared)
set_target_properties(rubyqscintilla PROPERTIES PREFIX "" OUTPUT_NAME qscintilla)
install(TARGETS rubyqscintilla DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES qscintilla.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/qscintilla)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qscintilla.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qscintilla.bundle\" )" )
endif (Q_WS_MAC)


