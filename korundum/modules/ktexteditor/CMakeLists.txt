include_directories( ${CMAKE_SOURCE_DIR}/generator ${RUBY_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES})

set(rubyktexteditor_LIBRARY_SRC ktexteditor.cpp ktexteditorhandlers.cpp)

add_library(rubyktexteditor MODULE ${rubyktexteditor_LIBRARY_SRC})
target_link_libraries(rubyktexteditor
    ${SMOKE_BASE_LIBRARY}
    ${SMOKE_KTEXTEDITOR_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QTRUBY4SHARED_LIBRARY}
    ${RUBY_LIBRARY})
set_target_properties(rubyktexteditor PROPERTIES PREFIX "" OUTPUT_NAME ktexteditor)
install(TARGETS rubyktexteditor DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES ktexteditor.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/ktexteditor)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/ktexteditor.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/ktexteditor.bundle\" )" )
endif (Q_WS_MAC)
