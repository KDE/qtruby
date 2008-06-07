include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES})

set(okular_LIBRARY_SRC okular.cpp okularhandlers.cpp)

add_library(okular MODULE ${okular_LIBRARY_SRC})
target_link_libraries(okular ${QT_QTCORE_LIBRARY} ${RUBY_LIBRARY} smokeqt smokekde smokeokular qtruby4shared)
set_target_properties(okular PROPERTIES PREFIX "")
install(TARGETS okular DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES okular.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/okular)
