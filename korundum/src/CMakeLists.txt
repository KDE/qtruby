
add_subdirectory( lib )

IF(QT_QTDBUS_FOUND)
    ADD_DEFINITIONS (-DQT_QTDBUS)
ENDIF(QT_QTDBUS_FOUND)

include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )

set(korundum_LIB_SRCS 
   Korundum.cpp 
   kdehandlers.cpp 
   ${CMAKE_SOURCE_DIR}/ruby/qtruby/src/Qt.cpp
   ${CMAKE_SOURCE_DIR}/ruby/qtruby/src/handlers.cpp
   ${CMAKE_SOURCE_DIR}/ruby/qtruby/src/marshall_types.cpp
   )

set(factory_SRCS
    krubypluginfactory.cpp)

kde4_add_library(korundum4 SHARED ${korundum_LIB_SRCS})

target_link_libraries(korundum4 ${KDE4_KDECORE_LIBS} ${RUBY_LIBRARY} ${QT_QTNETWORK_LIBRARY} smokekde )

set_target_properties(korundum4 PROPERTIES PREFIX "")

kde4_add_plugin(krubypluginfactory ${factory_SRCS})
target_link_libraries(krubypluginfactory ${KDE4_KDECORE_LIBS} ${RUBY_LIBRARY})

install(TARGETS korundum4  DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR} )
install(TARGETS krubypluginfactory DESTINATION ${PLUGIN_INSTALL_DIR})
