add_subdirectory( lib )

include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES})

set(plasma_applet_LIB_SRCS 
   plasma.cpp 
   plasmahandlers.cpp )

kde4_add_library(plasma_applet SHARED ${plasma_applet_LIB_SRCS})

target_link_libraries(plasma_applet 
    ${PLASMA_LIBS} 
    ${RUBY_LIBRARY} 
    qtruby4shared 
    smokeplasma)
set_target_properties(plasma_applet PROPERTIES PREFIX "")
install(TARGETS plasma_applet DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})

install(FILES package_ruboid.rb DESTINATION ${DATA_INSTALL_DIR}/ruby_applet_script)
install(FILES applet.rb DESTINATION ${DATA_INSTALL_DIR}/ruby_applet_script)
install(FILES data_engine.rb DESTINATION ${DATA_INSTALL_DIR}/ruby_applet_script)

install(FILES plasma-scriptengine-ruby-package.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES plasma-scriptengine-ruby-applet.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES plasma-scriptengine-ruby-dataengine.desktop DESTINATION ${SERVICES_INSTALL_DIR})

