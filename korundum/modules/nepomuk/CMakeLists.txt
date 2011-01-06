include_directories( ${CMAKE_SOURCE_DIR}/generator ${RUBY_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
include_directories (${SOPRANO_INCLUDE_DIR} ${QT_INCLUDES})

set(rubynepomuk_LIBRARY_SRC nepomuk.cpp nepomukhandlers.cpp)

add_library(rubynepomuk MODULE ${rubynepomuk_LIBRARY_SRC})
target_link_libraries(rubynepomuk
    ${NEPOMUK_LIBRARIES}
    ${SOPRANO_LIBRARIES}
    smokenepomuk
    ${SMOKE_BASE_LIBRARY}
    ${SMOKE_QTCORE_LIBRARY}
    ${QTRUBY4SHARED_LIBRARY}
    ${RUBY_LIBRARY}
    ${QT_QTCORE_LIBRARY}
)
set_target_properties(rubynepomuk PROPERTIES PREFIX "" OUTPUT_NAME nepomuk)
install(TARGETS rubynepomuk DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES nepomuk.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/nepomuk)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/nepomuk.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/nepomuk.bundle\" )" )
endif (Q_WS_MAC)

