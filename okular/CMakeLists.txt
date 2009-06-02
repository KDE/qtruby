include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES})

set(rubyokular_LIBRARY_SRC okular.cpp okularhandlers.cpp)

add_library(rubyokular MODULE ${rubyokular_LIBRARY_SRC})
target_link_libraries(rubyokular 
    ${QT_QTCORE_LIBRARY} 
    ${RUBY_LIBRARY} 
    ${OKULAR_LIBRARIES}
    smokeqt 
    smokekde 
    smokeokular 
    qtruby4shared)
set_target_properties(rubyokular PROPERTIES PREFIX "" OUTPUT_NAME okular)
install(TARGETS rubyokular DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES okular.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/okular)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/okular.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/okular.bundle\" )" )
endif (Q_WS_MAC)

