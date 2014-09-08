find_package(Qt5Network)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${Qt5Network_INCLUDE_DIRS}
    ${RUBY_INCLUDE_DIRS}
    ${SMOKE_INCLUDE_DIR}
)

set(rubyqtnetwork_LIBRARY_SRC qtnetwork.cpp qtnetworkhandlers.cpp)
add_library(rubyqtnetwork MODULE ${rubyqtnetwork_LIBRARY_SRC})
target_link_libraries(rubyqtnetwork
                            Qt5::Core
                            ${QT_QTNETWORK_LIBRARY}
                            ${SMOKE_QTNETWORK_LIBRARY}
                            smokeqtnetwork
                            qtrubyruntime
)
set_target_properties(rubyqtnetwork PROPERTIES PREFIX "" OUTPUT_NAME qtnetwork)
install(TARGETS rubyqtnetwork DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtnetwork.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtnetwork.bundle\" )" )
endif (Q_WS_MAC)
