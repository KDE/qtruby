
add_subdirectory( lib )

IF(QT_QTDBUS_FOUND)
    ADD_DEFINITIONS (-DQT_QTDBUS)
ENDIF(QT_QTDBUS_FOUND)

if(RUBY_VERSION)
    ADD_DEFINITIONS (-DRUBY_VERSION=0x${RUBY_VERSION_NUMBER})
ENDIF(RUBY_VERSION)

include_directories( ${CMAKE_SOURCE_DIR}/generator ${RUBY_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )

set(korundum_LIB_SRCS 
   Korundum.cpp 
   kdehandlers.cpp 
   )

set(factory_SRCS
    krubypluginfactory.cpp)

set(application_SRCS
    krubyapplication.cpp)

kde4_add_library(korundum4 SHARED ${korundum_LIB_SRCS})

target_link_libraries(korundum4 
    ${QT_QTNETWORK_LIBRARY} 
    ${QT_QTSQL_LIBRARY} 
    ${QT_QTOPENGL_LIBRARY} 
    ${QT_QTXML_LIBRARY} 
    ${QT_QTSVG_LIBRARY} 
    ${KDE4_KDECORE_LIBS} 
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS} 
    ${KDE4_KNEWSTUFF2_LIBS}
    ${KDE4_KPTY_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${RUBY_LIBRARY}
    ${SMOKE_BASE_LIBRARY}
    smokekdecore 
    smokekdeui 
    ${SMOKE_KIO_LIBRARY} 
    smokekparts 
    smokekfile 
    smokeknewstuff2 
    smokeknewstuff3 
    smokekutils      
    ${QTRUBY4SHARED_LIBRARY})

set_target_properties(korundum4 PROPERTIES PREFIX "")

kde4_add_plugin(krubypluginfactory ${factory_SRCS})
target_link_libraries(krubypluginfactory ${KDE4_KDECORE_LIBS}
        ${KDE4_KIO_LIBS}
        ${RUBY_LIBRARY}
        ${SMOKE_BASE_LIBRARY}
        ${QTRUBY4SHARED_LIBRARY})

kde4_add_executable(krubyapplication ${application_SRCS})
target_link_libraries(krubyapplication ${KDE4_KDECORE_LIBS} ${RUBY_LIBRARY})

install(TARGETS korundum4  DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR} )
install(TARGETS krubypluginfactory DESTINATION ${PLUGIN_INSTALL_DIR})
install(TARGETS krubyapplication DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/korundum4.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/korundum4.bundle\" )" )
endif (Q_WS_MAC)



