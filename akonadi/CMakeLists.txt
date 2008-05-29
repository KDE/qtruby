include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES})

#
# The akonadi plugin extension needs to be run inside Akonadi, and so QtRuby must
# be compiled with this flag as it can't be assumed that Ruby was initialized
# in the main() function. The qtrubyshared library cannot be used, and instead
# all the QtRuby sources are compiled with the akonadi extension sources.
#
add_definitions (-DRUBY_EMBEDDED)

set(akonadi_LIBRARY_SRC akonadi.cpp akonadihandlers.cpp
   ${CMAKE_SOURCE_DIR}/ruby/qtruby/src/qtruby.cpp
   ${CMAKE_SOURCE_DIR}/ruby/qtruby/src/Qt.cpp
   ${CMAKE_SOURCE_DIR}/ruby/qtruby/src/handlers.cpp
   ${CMAKE_SOURCE_DIR}/ruby/qtruby/src/marshall_types.cpp
   )

add_library(akonadi MODULE ${akonadi_LIBRARY_SRC})
target_link_libraries(akonadi ${QT_QTCORE_LIBRARY} ${RUBY_LIBRARY} smokeqt smokekde smokeakonadi)
set_target_properties(akonadi PROPERTIES PREFIX "")
install(TARGETS akonadi DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES akonadi.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/akonadi)
