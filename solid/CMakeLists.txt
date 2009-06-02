include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES} ${KDE4_INCLUDE_DIR})

set(rubysolid_LIBRARY_SRC solid.cpp solidhandlers.cpp)

add_library(rubysolid MODULE ${rubysolid_LIBRARY_SRC})
target_link_libraries(rubysolid 
    ${QT_QTCORE_LIBRARY} 
    ${RUBY_LIBRARY} 
    ${KDE4_SOLID_LIBS}
    smokeqt 
    smokesolid 
    qtruby4shared)
set_target_properties(rubysolid PROPERTIES PREFIX "" OUTPUT_NAME solid)
install(TARGETS rubysolid DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES solid.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/solid)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/solid.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/solid.bundle\" )" )
endif (Q_WS_MAC)
