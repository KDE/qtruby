include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)

set(rubykio_LIBRARY_SRC kio.cpp kiohandlers.cpp)
add_library(rubykio MODULE ${rubykio_LIBRARY_SRC})
target_link_libraries(rubykio ${KIO_LIBS} ${KIO_LIBRARY} smokeqtcore smokekio qtruby4shared)
set_target_properties(rubykio PROPERTIES PREFIX "" OUTPUT_NAME kio)
install(TARGETS rubykio DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES kio.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/kio)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/kio.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/kio.bundle\" )" )
endif (Q_WS_MAC)
