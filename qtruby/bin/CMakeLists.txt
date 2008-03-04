install( PROGRAMS rbqtapi DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )

install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CMAKE_INSTALL_PREFIX}/bin/rbqtapi\"  \"${CMAKE_INSTALL_PREFIX}/bin/rbqt3api\" )" )
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CMAKE_INSTALL_PREFIX}/bin/rbqtapi\"  \"${CMAKE_INSTALL_PREFIX}/bin/rbqt4api\" )" )
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CMAKE_INSTALL_PREFIX}/bin/rbqtapi\"  \"${CMAKE_INSTALL_PREFIX}/bin/rbkdeapi\" )" )
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CMAKE_INSTALL_PREFIX}/bin/rbqtapi\"  \"${CMAKE_INSTALL_PREFIX}/bin/rbkde4api\" )" )

