find_package(Qt5Widgets)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${Qt5Widgets_INCLUDE_DIRS}
    ${RUBY_INCLUDE_DIRS}
    ${SMOKE_INCLUDE_DIR}
)

set(rubyqtwidgets_LIBRARY_SRC
    qtwidgets.cpp
    qtwidgetshandlers.cpp
    rubyqpainter.cpp
    typeresolver.cpp
    )

add_library(rubyqtwidgets MODULE ${rubyqtwidgets_LIBRARY_SRC})
target_link_libraries(rubyqtwidgets
                            Qt5::Core
                            Qt5::Gui
                            Qt5::Widgets
                            ${SMOKE_QTCORE_LIBRARY}
                            smokeqt5widgets
                            qtrubyruntime
)
set_target_properties(rubyqtwidgets PROPERTIES PREFIX "" OUTPUT_NAME qtwidgets)
install(TARGETS rubyqtwidgets DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES qtwidgets.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/qtwidgets)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtwidgets.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtwidgets.bundle\" )" )
endif (Q_WS_MAC)


