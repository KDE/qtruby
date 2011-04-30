include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${QT_INCLUDES}
    ${RUBY_INCLUDE_DIRS}
    ${SMOKE_INCLUDE_DIR}
)

set(rubyqtsvg_LIBRARY_SRC qtsvg.cpp qtsvghandlers.cpp)
add_library(rubyqtsvg MODULE ${rubyqtsvg_LIBRARY_SRC})
target_link_libraries(rubyqtsvg
                            ${QT_QTCORE_LIBRARY}
                            ${QT_QTSVG_LIBRARY}
                            ${SMOKE_QTSVG_LIBRARY}
                            smokeqtsvg
                            qtrubyruntime
)
set_target_properties(rubyqtsvg PROPERTIES PREFIX "" OUTPUT_NAME qtsvg)
install(TARGETS rubyqtsvg DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtsvg.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtsvg.bundle\" )" )
endif (Q_WS_MAC)
