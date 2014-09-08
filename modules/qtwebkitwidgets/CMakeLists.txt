find_package(Qt5WebKitWidgets)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${Qt5WebKitWidgets_INCLUDE_DIRS}
    ${RUBY_INCLUDE_DIRS}
    ${SMOKE_INCLUDE_DIR}
)

set(rubyqtwebkitwidgets_LIBRARY_SRC qtwebkitwidgets.cpp qtwebkitwidgetshandlers.cpp)
add_library(rubyqtwebkitwidgets MODULE ${rubyqtwebkitwidgets_LIBRARY_SRC})
target_link_libraries(rubyqtwebkitwidgets
                            Qt5::Core
                            Qt5::WebKitWidgets
                            ${SMOKE_QWEBKIT_LIBRARY}
                            qtrubyruntime
)
set_target_properties(rubyqtwebkitwidgets PROPERTIES PREFIX "" OUTPUT_NAME qtwebkitwidgets)
install(TARGETS rubyqtwebkitwidgets DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtwebkitwidgets.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtwebkitwidgets.bundle\" )" )
endif (Q_WS_MAC)
