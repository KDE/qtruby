find_package(Phonon REQUIRED)

include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)

set(phonon_LIBRARY_SRC phonon.cpp phononhandlers.cpp)
add_library(phonon MODULE ${phonon_LIBRARY_SRC})
target_link_libraries(phonon ${PHONON_LIBS} ${RUBY_LIBRARY} smokeqt smokephonon qtruby4shared)
set_target_properties(phonon PROPERTIES PREFIX "")
install(TARGETS phonon DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES phonon.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/phonon)
