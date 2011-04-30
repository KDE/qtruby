include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${QT_INCLUDES}
    ${RUBY_INCLUDE_DIRS}
    ${SMOKE_INCLUDE_DIR}
)

set(rubyphonon_LIBRARY_SRC phonon.cpp phononhandlers.cpp)
add_library(rubyphonon MODULE ${rubyphonon_LIBRARY_SRC})
target_link_libraries(rubyphonon
                            ${PHONON_LIBS}
                            ${SMOKE_PHONON_LIBRARY}
                            qtrubyruntime
)
set_target_properties(rubyphonon PROPERTIES PREFIX "" OUTPUT_NAME phonon)
install(TARGETS rubyphonon DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/phonon.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/phonon.bundle\" )" )
endif (Q_WS_MAC)
