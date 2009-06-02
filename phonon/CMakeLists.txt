include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)

set(rubyphonon_LIBRARY_SRC phonon.cpp phononhandlers.cpp)
add_library(rubyphonon MODULE ${rubyphonon_LIBRARY_SRC})
target_link_libraries(rubyphonon ${PHONON_LIBS} ${RUBY_LIBRARY} smokeqt smokephonon qtruby4shared)
set_target_properties(rubyphonon PROPERTIES PREFIX "" OUTPUT_NAME phonon)
install(TARGETS rubyphonon DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES phonon.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/phonon)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/phonon.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/phonon.bundle\" )" )
endif (Q_WS_MAC)
