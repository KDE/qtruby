include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${QT_INCLUDES}
    ${RUBY_INCLUDE_DIRS}
    ${SMOKE_INCLUDE_DIR}
)

set(rubyqtwebkit_LIBRARY_SRC qtwebkit.cpp qtwebkithandlers.cpp)
add_library(rubyqtwebkit MODULE ${rubyqtwebkit_LIBRARY_SRC})
target_link_libraries(rubyqtwebkit
                            ${QT_QTCORE_LIBRARY}
                            ${QT_QTWEBKIT_LIBRARY}
                            ${SMOKE_QWEBKIT_LIBRARY}
                            qtrubyruntime
)
set_target_properties(rubyqtwebkit PROPERTIES PREFIX "" OUTPUT_NAME qtwebkit)
install(TARGETS rubyqtwebkit DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtwebkit.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtwebkit.bundle\" )" )
endif (Q_WS_MAC)
