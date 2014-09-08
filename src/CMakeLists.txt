find_package(Qt5Widgets)

add_definitions(-DBUILD_QTRUBY)

if(RUBY_VERSION)
    add_definitions(-DRUBY_VERSION=0x${RUBY_VERSION_NUMBER})
endif(RUBY_VERSION)

include_directories(
    ${Qt5Widgets_INCLUDE_DIRS}
    ${RUBY_INCLUDE_DIRS}
    ${SMOKE_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(qtrubyruntime_LIB_SRCS
    debug.cpp
    emitsignal.cpp
    funcall.cpp
    global.cpp
    object.cpp
    invokeslot.cpp
    metaclass.cpp
    methodmissing.cpp
    prettyprint.cpp
    smokebinding.cpp
    utils.cpp
    methodcall.cpp
    rubymetatype.cpp
#     rubyqmetaobject.cpp
    virtualmethodcall.cpp
    handlers.cpp )

add_library(qtrubyruntime SHARED ${qtrubyruntime_LIB_SRCS})

target_link_libraries(qtrubyruntime
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    ${RUBY_LIBRARY}
    ${SMOKE_BASE_LIBRARY}
)

set_target_properties(qtrubyruntime PROPERTIES VERSION 1.0.0 SOVERSION 1)

install(    TARGETS qtrubyruntime
            DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX} )

# TODO: not all the qtrubyruntime headers should be installed - decide which
# ones should be private and rename them '*_p.h'
install(FILES
    debug.h
    global.h
    marshall.h
    methodcall.h
    object.h
    qtruby_export.h
    rubymetatype.h
    smokebinding.h
    utils.h
DESTINATION ${CMAKE_INSTALL_PREFIX}/include/qtruby )
