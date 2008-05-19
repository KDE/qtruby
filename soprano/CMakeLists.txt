include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES})

set(soprano_LIBRARY_SRC soprano.cpp sopranohandlers.cpp)
add_library(soprano MODULE ${soprano_LIBRARY_SRC})
target_link_libraries(soprano ${QT_QTCORE_LIBRARY} ${RUBY_LIBRARY} smokeqt smokesoprano qtruby4shared)
set_target_properties(soprano PROPERTIES PREFIX "")
install(TARGETS soprano DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES soprano.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/soprano)
