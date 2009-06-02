add_subdirectory( lib )

include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES})

set(plasma_applet_LIB_SRCS 
   plasma.cpp 
   plasmahandlers.cpp )

kde4_add_library(plasma_applet SHARED ${plasma_applet_LIB_SRCS})

target_link_libraries(plasma_applet 
    ${KDE4_PLASMA_LIBS} 
    ${RUBY_LIBRARY} 
    qtruby4shared 
    smokeplasma)
set_target_properties(plasma_applet PROPERTIES PREFIX "")
install(TARGETS plasma_applet DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/plasma_applet.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/plasma_applet.bundle\" )" )
endif (Q_WS_MAC)


