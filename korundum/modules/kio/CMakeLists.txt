include_directories( ${CMAKE_SOURCE_DIR}/generator ${RUBY_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)

set(rubykio_LIBRARY_SRC kio.cpp kiohandlers.cpp)
add_library(rubykio MODULE ${rubykio_LIBRARY_SRC})
target_link_libraries(rubykio
    ${SMOKE_BASE_LIBRARY}
    ${SMOKE_KIO_LIBRARY}
    ${RUBY_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${QTRUBY4SHARED_LIBRARY}
)
set_target_properties(rubykio PROPERTIES PREFIX "" OUTPUT_NAME kio)
install(TARGETS rubykio DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES kio.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/kio)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/kio.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/kio.bundle\" )" )
endif (Q_WS_MAC)
