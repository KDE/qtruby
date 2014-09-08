find_package(Qt5Sql)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${Qt5Sql_INCLUDE_DIRS}
    ${RUBY_INCLUDE_DIRS}
    ${SMOKE_INCLUDE_DIR}
)

set(rubyqtsql_LIBRARY_SRC qtsql.cpp qtsqlhandlers.cpp)
add_library(rubyqtsql MODULE ${rubyqtsql_LIBRARY_SRC})
target_link_libraries(rubyqtsql
                            Qt5::Core
                            Qt5::Sql
                            ${SMOKE_QTSQL_LIBRARY}
                            smokeqt5sql
                            qtrubyruntime
)
set_target_properties(rubyqtsql PROPERTIES PREFIX "" OUTPUT_NAME qtsql)
install(TARGETS rubyqtsql DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtsql.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtsql.bundle\" )" )
endif (Q_WS_MAC)
