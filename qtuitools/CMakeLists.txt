include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES})

set(qtuitools_LIBRARY_SRC qtuitools.cpp qtuitoolshandlers.cpp)
add_library(qtuitools MODULE ${qtuitools_LIBRARY_SRC})
target_link_libraries(qtuitools ${RUBY_LIBRARY} smokeqt smokeqtuitools qtruby4shared)
set_target_properties(qtuitools PROPERTIES PREFIX "")
install(TARGETS qtuitools DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES qtuitools.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/qtuitools)
