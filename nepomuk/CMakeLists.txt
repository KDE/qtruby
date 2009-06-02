include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES})

set(rubynepomuk_LIBRARY_SRC nepomuk.cpp nepomukhandlers.cpp)

add_library(rubynepomuk MODULE ${rubynepomuk_LIBRARY_SRC})
target_link_libraries(rubynepomuk 
    ${RUBY_LIBRARY} 
    ${NEPOMUK_LIBRARIES}
    smokeqt 
    smokesoprano
    smokekde 
    smokenepomuk 
    qtruby4shared)
set_target_properties(rubynepomuk PROPERTIES PREFIX "" OUTPUT_NAME nepomuk)
install(TARGETS rubynepomuk DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES nepomuk.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/nepomuk)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/nepomuk.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/nepomuk.bundle\" )" )
endif (Q_WS_MAC)

