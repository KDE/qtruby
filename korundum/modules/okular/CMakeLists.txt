include_directories( ${CMAKE_SOURCE_DIR}/generator ${RUBY_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES})

set(rubyokular_LIBRARY_SRC okular.cpp okularhandlers.cpp)

add_library(rubyokular MODULE ${rubyokular_LIBRARY_SRC})
target_link_libraries(rubyokular
    ${OKULAR_LIBRARIES}
    smokeokular
    ${QTRUBY4SHARED_LIBRARY}
    ${RUBY_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${SMOKE_BASE_LIBRARY}
)
set_target_properties(rubyokular PROPERTIES PREFIX "" OUTPUT_NAME okular)
install(TARGETS rubyokular DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES okular.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/okular)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/okular.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/okular.bundle\" )" )
endif (Q_WS_MAC)

