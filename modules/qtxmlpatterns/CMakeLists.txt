include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${QT_INCLUDES}
    ${RUBY_INCLUDE_DIRS}
    ${SMOKE_INCLUDE_DIR}
)

set(rubyqtxmlpatterns_LIBRARY_SRC qtxmlpatterns.cpp qtxmlpatternshandlers.cpp)
add_library(rubyqtxmlpatterns MODULE ${rubyqtxmlpatterns_LIBRARY_SRC})
target_link_libraries(rubyqtxmlpatterns
                            ${QT_QTCORE_LIBRARY}
                            ${QT_QTXMLPATTERNS_LIBRARY}
                            ${SMOKE_QTXMLPATTERNS_LIBRARY}
                            smokeqtxmlpatterns
                            qtrubyruntime
)
set_target_properties(rubyqtxmlpatterns PROPERTIES PREFIX "" OUTPUT_NAME qtxmlpatterns)
install(TARGETS rubyqtxmlpatterns DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtxmlpatterns.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtxmlpatterns.bundle\" )" )
endif (Q_WS_MAC)
