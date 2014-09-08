find_package(Qt5DBus)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${Qt5DBus_INCLUDE_DIRS}
    ${RUBY_INCLUDE_DIRS}
    ${SMOKE_INCLUDE_DIR}
)

set(rubyqtdbus_LIBRARY_SRC qtdbus.cpp qtdbushandlers.cpp)
add_library(rubyqtdbus MODULE ${rubyqtdbus_LIBRARY_SRC})
target_link_libraries(rubyqtdbus
                            Qt5::Core
                            Qt5::DBus
                            ${SMOKE_QTCORE_LIBRARY}
                            smokeqt5dbus
                            qtrubyruntime
)
set_target_properties(rubyqtdbus PROPERTIES PREFIX "" OUTPUT_NAME qtdbus)
install(TARGETS rubyqtdbus DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES qtdbus.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/qtdbus)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtdbus.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtdbus.bundle\" )" )
endif (Q_WS_MAC)
