
add_subdirectory( lib )

include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )

set(korundum_LIB_SRCS 
   Korundum.cpp 
   kdehandlers.cpp 
   )

kde4_add_library(korundum4 SHARED ${korundum_LIB_SRCS})

target_link_libraries(korundum4 ${KDE4_KDECORE_LIBS} ${RUBY_LIBRARY} ${QT_QTNETWORK_LIBRARY} smokekde qtrubyinternal )

set_target_properties(korundum4 PROPERTIES VERSION 0.0.0 SOVERSION 0 PREFIX "")
install(TARGETS korundum4  DESTINATION ${RUBY_SITE_ARCH_DIR} )
