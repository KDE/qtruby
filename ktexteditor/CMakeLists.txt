include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES})

set(ktexteditor_LIBRARY_SRC ktexteditor.cpp ktexteditorhandlers.cpp)

add_library(ktexteditor MODULE ${ktexteditor_LIBRARY_SRC})
target_link_libraries(ktexteditor ${QT_QTCORE_LIBRARY} ${RUBY_LIBRARY} smokeqt smokekde smokektexteditor qtruby4shared)
set_target_properties(ktexteditor PROPERTIES PREFIX "")
install(TARGETS ktexteditor DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES ktexteditor.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/ktexteditor)
