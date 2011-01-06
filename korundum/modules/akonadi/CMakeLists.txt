include_directories( ${CMAKE_SOURCE_DIR}/generator ${RUBY_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
include_directories (${QT_INCLUDES} ${KDE4_INCLUDE_DIR} )
include_directories( ${KDEPIMLIBS_INCLUDE_DIR} )

set(rubyakonadi_LIBRARY_SRC akonadi.cpp akonadihandlers.cpp)

add_library(rubyakonadi MODULE ${rubyakonadi_LIBRARY_SRC})
target_link_libraries(rubyakonadi
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
    smokeakonadi
    ${SMOKE_BASE_LIBRARY}
    ${QTRUBY4SHARED_LIBRARY}
    ${RUBY_LIBRARY}
)
set_target_properties(rubyakonadi PROPERTIES PREFIX "" OUTPUT_NAME akonadi)

install(TARGETS rubyakonadi DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES akonadi.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/akonadi)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/akonadi.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/akonadi.bundle\" )" )
endif (Q_WS_MAC)

