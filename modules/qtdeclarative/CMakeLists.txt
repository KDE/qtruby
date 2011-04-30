include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${QT_INCLUDES}
    ${RUBY_INCLUDE_DIRS}
    ${SMOKE_INCLUDE_DIR}
)

set(rubyqtdeclarative_LIBRARY_SRC qtdeclarative.cpp qtdeclarativehandlers.cpp)
add_library(rubyqtdeclarative MODULE ${rubyqtdeclarative_LIBRARY_SRC})
target_link_libraries(rubyqtdeclarative
                            ${QT_QTCORE_LIBRARY}
                            ${QT_QTDECLARATIVE_LIBRARY}
                            ${SMOKE_QTCORE_LIBRARY}
                            smokeqtdeclarative
                            qtrubyruntime
)
set_target_properties(rubyqtdeclarative PROPERTIES PREFIX "" OUTPUT_NAME qtdeclarative)
install(TARGETS rubyqtdeclarative DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtdeclarative.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtdeclarative.bundle\" )" )
endif (Q_WS_MAC)
