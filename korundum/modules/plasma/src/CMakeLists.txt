add_subdirectory( lib )

include_directories( ${CMAKE_SOURCE_DIR}/generator ${RUBY_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES})

set(plasma_applet_LIB_SRCS
   plasma.cpp
   plasmahandlers.cpp )

kde4_add_library(plasma_applet SHARED ${plasma_applet_LIB_SRCS})

target_link_libraries(plasma_applet
    ${SMOKE_BASE_LIBRARY}
    ${SMOKE_QTCORE_LIBRARY}
    ${SMOKE_PLASMA_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QTRUBY4SHARED_LIBRARY}
    ${RUBY_LIBRARY}
)
set_target_properties(plasma_applet PROPERTIES PREFIX "")
install(TARGETS plasma_applet DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/plasma_applet.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/plasma_applet.bundle\" )" )
endif (Q_WS_MAC)


