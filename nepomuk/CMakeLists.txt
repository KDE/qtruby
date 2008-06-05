include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES})

set(nepomuk_LIBRARY_SRC nepomuk.cpp nepomukhandlers.cpp)

add_library(nepomuk MODULE ${nepomuk_LIBRARY_SRC})
target_link_libraries(nepomuk 
    ${RUBY_LIBRARY} 
    ${NEPOMUK_LIBRARIES}
    smokeqt 
    smokesoprano
    smokekde 
    smokenepomuk 
    qtruby4shared)
set_target_properties(nepomuk PROPERTIES PREFIX "")
install(TARGETS nepomuk DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES nepomuk.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/nepomuk)
