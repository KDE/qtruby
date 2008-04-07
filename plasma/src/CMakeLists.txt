add_subdirectory( lib )

add_definitions (-DRUBY_EMBEDDED)

include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )

set(plasma_applet_LIB_SRCS 
   plasma.cpp 
   kdehandlers.cpp 
   ${CMAKE_SOURCE_DIR}/ruby/qtruby/src/Qt.cpp
   ${CMAKE_SOURCE_DIR}/ruby/qtruby/src/handlers.cpp
   ${CMAKE_SOURCE_DIR}/ruby/qtruby/src/marshall_types.cpp
   )

kde4_add_library(plasma_applet SHARED ${plasma_applet_LIB_SRCS})

target_link_libraries(plasma_applet ${KDE4_KDECORE_LIBS} ${RUBY_LIBRARY} ${QT_QTNETWORK_LIBRARY} smokeplasma )

set_target_properties(plasma_applet PROPERTIES PREFIX "")
install(TARGETS plasma_applet  DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR} )
