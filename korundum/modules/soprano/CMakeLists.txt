include_directories( ${CMAKE_SOURCE_DIR}/generator ${RUBY_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
include_directories (${QT_INCLUDES} ${SOPRANO_INCLUDE_DIR})

set(rubysoprano_LIBRARY_SRC soprano.cpp sopranohandlers.cpp)
add_library(rubysoprano MODULE ${rubysoprano_LIBRARY_SRC})
target_link_libraries(rubysoprano
    ${SOPRANO_LIBRARIES}
    ${SMOKE_BASE_LIBRARY}
    ${SMOKE_SOPRANO_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QTRUBY4SHARED_LIBRARY}
    ${RUBY_LIBRARY}
)
set_target_properties(rubysoprano PROPERTIES PREFIX "" OUTPUT_NAME soprano)

install(TARGETS rubysoprano DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES soprano.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/soprano)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/soprano.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/soprano.bundle\" )" )
endif (Q_WS_MAC)

