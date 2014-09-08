find_package(Qt5Gui)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${Qt5Gui_INCLUDE_DIRS}
    ${RUBY_INCLUDE_DIRS}
    ${SMOKE_INCLUDE_DIR}
)

set(rubyqtgui_LIBRARY_SRC
    qtgui.cpp
    qtguihandlers.cpp
    rubyqimage.cpp
    rubyqitemselection.cpp
    rubyqpainter.cpp
    typeresolver.cpp
    )

add_library(rubyqtgui MODULE ${rubyqtgui_LIBRARY_SRC})
target_link_libraries(rubyqtgui
                            Qt5::Core
                            Qt5::Gui
                            ${SMOKE_QTCORE_LIBRARY}
                            smokeqt5gui
                            qtrubyruntime
)
set_target_properties(rubyqtgui PROPERTIES PREFIX "" OUTPUT_NAME qtgui)
install(TARGETS rubyqtgui DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES qtgui.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/qtgui)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtgui.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtgui.bundle\" )" )
endif (Q_WS_MAC)


