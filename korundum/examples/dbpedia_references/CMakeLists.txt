FIND_PACKAGE (RUBY REQUIRED)

include_directories( ${RUBY_INCLUDE_DIRS} )
install(PROGRAMS dbpedia_references.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES dbpedia_references.rb DESTINATION ${DATA_INSTALL_DIR}/dbpedia_references)
