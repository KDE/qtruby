include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES})

set(qtwebkit_LIBRARY_SRC qtwebkit.cpp qtwebkithandlers.cpp)
add_library(qtwebkit MODULE ${qtwebkit_LIBRARY_SRC})
target_link_libraries(qtwebkit ${QT_QTWEBKIT_LIBRARY} ${RUBY_LIBRARY} smokeqt smokeqtwebkit qtruby4shared)
set_target_properties(qtwebkit PROPERTIES PREFIX "")
install(TARGETS qtwebkit DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES qtwebkit.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/qtwebkit)
