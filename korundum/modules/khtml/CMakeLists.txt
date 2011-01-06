include_directories( ${CMAKE_SOURCE_DIR}/generator ${RUBY_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)

set(rubykhtml_LIBRARY_SRC khtml.cpp khtmlhandlers.cpp)
add_library(rubykhtml MODULE ${rubykhtml_LIBRARY_SRC})
target_link_libraries(rubykhtml
    ${SMOKE_BASE_LIBRARY}
    ${SMOKE_KHTML_LIBRARY}
    ${QTRUBY4SHARED_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${RUBY_LIBRARY}
)
set_target_properties(rubykhtml PROPERTIES PREFIX "" OUTPUT_NAME khtml)
install(TARGETS rubykhtml DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES khtml.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/khtml)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/khtml.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/khtml.bundle\" )" )
endif (Q_WS_MAC)
