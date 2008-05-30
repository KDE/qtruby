include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES})

set(qsci_LIBRARY_SRC qscintilla.cpp qscintillahandlers.cpp)
add_library(qscintilla MODULE ${qsci_LIBRARY_SRC})
target_link_libraries(qscintilla ${QT_QTCORE_LIBRARY} ${RUBY_LIBRARY} smokeqt smokeqsci qtruby4shared)
set_target_properties(qscintilla PROPERTIES PREFIX "")
install(TARGETS qscintilla DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES qscintilla.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/qscintilla)
