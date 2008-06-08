include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES} ${KDE4_INCLUDE_DIR})

set(solid_LIBRARY_SRC solid.cpp solidhandlers.cpp)

add_library(solid MODULE ${solid_LIBRARY_SRC})
target_link_libraries(solid 
    ${QT_QTCORE_LIBRARY} 
    ${RUBY_LIBRARY} 
    ${KDE4_SOLID_LIBS}
    smokeqt 
    smokesolid 
    qtruby4shared)
set_target_properties(solid PROPERTIES PREFIX "")
install(TARGETS solid DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES solid.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/solid)
