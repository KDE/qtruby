include_directories( ${CMAKE_SOURCE_DIR}/generator ${RUBY_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES} ${KDE4_INCLUDE_DIR})

set(rubysolid_LIBRARY_SRC solid.cpp solidhandlers.cpp)

add_library(rubysolid MODULE ${rubysolid_LIBRARY_SRC})
target_link_libraries(rubysolid
    ${KDE4_SOLID_LIBS}
    ${SMOKE_BASE_LIBRARY}
    ${SMOKE_SOLID_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QTRUBY4SHARED_LIBRARY}
    ${RUBY_LIBRARY}
)
set_target_properties(rubysolid PROPERTIES PREFIX "" OUTPUT_NAME solid)
install(TARGETS rubysolid DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES solid.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/solid)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/solid.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/solid.bundle\" )" )
endif (Q_WS_MAC)
