OPTION(ENABLE_QTRUBY "build QtRuby" ON)
OPTION(ENABLE_QTCORE_QTRUBY "build QtCoreQtRuby" ON)
OPTION(ENABLE_QTGUI_QTRUBY "build QtGuiQtRuby" ON)

set(QTRUBY_ENABLED "yes")
if(ENABLE_QTRUBY)
    add_definitions(-DQTRUBY_BUILDING)
    add_subdirectory(qtcore)
    set(QTRUBY_ENABLED "yes - QtCore")

    if(ENABLE_QTGUI_QTRUBY)
        add_subdirectory(qtgui)
        set(QTRUBY_ENABLED "${QTRUBY_ENABLED}, QtGui")
    endif(ENABLE_QTGUI_QTRUBY)
endif(ENABLE_QTRUBY)

message(STATUS "Build QtRuby... " ${QTRUBY_ENABLED})
