include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES})

set(rubysoprano_LIBRARY_SRC soprano.cpp sopranohandlers.cpp)
add_library(rubysoprano MODULE ${rubysoprano_LIBRARY_SRC})
target_link_libraries(rubysoprano ${QT_QTCORE_LIBRARY} ${RUBY_LIBRARY} smokeqt smokesoprano qtruby4shared)
set_target_properties(rubysoprano PROPERTIES PREFIX "" OUTPUT_NAME soprano)
install(TARGETS rubysoprano DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES soprano.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/soprano)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/soprano.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/soprano.bundle\" )" )
endif (Q_WS_MAC)

