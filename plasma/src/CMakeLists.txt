add_subdirectory( lib )

#
# The plasma_applet extension needs to be run inside Plasma, and so QtRuby must
# be compiled with this flag as it can't be assumed that Ruby was initialized
# in the main() function. The qtrubyshared library cannot be used, and instead
# all the QtRuby sources are compiled with the plasma_applet extension sources.
#
add_definitions (-DRUBY_EMBEDDED)

include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES})

set(plasma_applet_LIB_SRCS 
   plasma.cpp 
   plasmahandlers.cpp 
   ${CMAKE_SOURCE_DIR}/ruby/qtruby/src/qtruby.cpp
   ${CMAKE_SOURCE_DIR}/ruby/qtruby/src/Qt.cpp
   ${CMAKE_SOURCE_DIR}/ruby/qtruby/src/handlers.cpp
   ${CMAKE_SOURCE_DIR}/ruby/qtruby/src/marshall_types.cpp
   )

kde4_add_library(plasma_applet SHARED ${plasma_applet_LIB_SRCS})

target_link_libraries(plasma_applet ${QT_QTCORE_LIBRARY} ${RUBY_LIBRARY} smokeqt smokekde smokeplasma)
set_target_properties(plasma_applet PROPERTIES PREFIX "")
install(TARGETS plasma_applet DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})

install(FILES package_ruboid.rb DESTINATION ${DATA_INSTALL_DIR}/ruby_applet_script)
install(FILES applet.rb DESTINATION ${DATA_INSTALL_DIR}/ruby_applet_script)
install(FILES data_engine.rb DESTINATION ${DATA_INSTALL_DIR}/ruby_applet_script)

install(FILES plasma-scriptengine-ruby-package.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES plasma-scriptengine-ruby-applet.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES plasma-scriptengine-ruby-dataengine.desktop DESTINATION ${SERVICES_INSTALL_DIR})

