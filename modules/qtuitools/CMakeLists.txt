include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${QT_INCLUDES}
    ${RUBY_INCLUDE_DIRS}
    ${SMOKE_INCLUDE_DIR}
)

set(rubyqtuitools_LIBRARY_SRC qtuitools.cpp qtuitoolshandlers.cpp)
add_library(rubyqtuitools MODULE ${rubyqtuitools_LIBRARY_SRC})
target_link_libraries(rubyqtuitools
                            ${SMOKE_QTUITOOLS_LIBRARY}
                            qtrubyruntime
)
set_target_properties(rubyqtuitools PROPERTIES PREFIX "" OUTPUT_NAME qtuitools)
install(TARGETS rubyqtuitools DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtuitools.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtuitools.bundle\" )" )
endif (Q_WS_MAC)
