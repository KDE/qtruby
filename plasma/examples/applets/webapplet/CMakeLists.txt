FIND_PACKAGE (RUBY REQUIRED)

include_directories( ${RUBY_INCLUDE_PATH} )
install(FILES plasma-ruby-applet-web.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES web_applet.rb DESTINATION ${DATA_INSTALL_DIR}/plasma-ruby-webapplet)
