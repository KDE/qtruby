include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES})

set(qsci_LIBRARY_SRC qwt.cpp qwthandlers.cpp)
add_library(qwt MODULE ${qsci_LIBRARY_SRC})
target_link_libraries(qwt ${QT_QTCORE_LIBRARY} ${RUBY_LIBRARY} smokeqt smokeqwt qtruby4shared)
set_target_properties(qwt PROPERTIES PREFIX "")
install(TARGETS qwt DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES qwt.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/qwt)
