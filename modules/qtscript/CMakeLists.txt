include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${QT_INCLUDES}
    ${RUBY_INCLUDE_DIRS}
    ${SMOKE_INCLUDE_DIR}
)

set(rubyqtscript_LIBRARY_SRC qtscript.cpp qtscripthandlers.cpp)
add_library(rubyqtscript MODULE ${rubyqtscript_LIBRARY_SRC})
target_link_libraries(rubyqtscript
                            ${QT_QTCORE_LIBRARY}
                            ${QT_QTSCRIPT_LIBRARY}
                            ${SMOKE_QTCORE_LIBRARY}
                            smokeqtscript
                            qtrubyruntime
)
set_target_properties(rubyqtscript PROPERTIES PREFIX "" OUTPUT_NAME qtscript)
install(TARGETS rubyqtscript DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtscript.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtscript.bundle\" )" )
endif (Q_WS_MAC)
