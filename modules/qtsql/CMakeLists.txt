include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${QT_INCLUDES}
    ${RUBY_INCLUDE_DIRS}
    ${SMOKE_INCLUDE_DIR}
)

set(rubyqtsql_LIBRARY_SRC qtsql.cpp qtsqlhandlers.cpp)
add_library(rubyqtsql MODULE ${rubyqtsql_LIBRARY_SRC})
target_link_libraries(rubyqtsql
                            ${QT_QTCORE_LIBRARY}
                            ${QT_QTSQL_LIBRARY}
                            ${SMOKE_QTSQL_LIBRARY}
                            smokeqtsql
                            qtrubyruntime
)
set_target_properties(rubyqtsql PROPERTIES PREFIX "" OUTPUT_NAME qtsql)
install(TARGETS rubyqtsql DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtsql.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtsql.bundle\" )" )
endif (Q_WS_MAC)
