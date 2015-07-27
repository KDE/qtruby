cmake_minimum_required(VERSION 2.8.9)
project(QtRuby)
set(COMPILE_RUBY FALSE CACHE INTERNAL "")

find_package(Ruby REQUIRED)
find_package(Qt4 REQUIRED)
find_package(Smoke COMPONENTS QtCore QtGui QtXml QtOpenGl QtSql QtNetwork QtDbus QtSvg Phonon QSci QtDeclarative QtScript QtWebkit QtUiTools QtTest Qwt)
include_directories(${SMOKE_INCLUDE_DIR} ${SMOKE_QTCORE_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src ${QT_INCLUDES})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${SMOKE_CMAKE_MODULE_DIR})
include(MacroOptionalFindPackage)
include(MacroOptionalAddBindings)
include(MacroLogFeature)

macro_optional_find_package(QScintilla)
macro_log_feature(QSCINTILLA_FOUND "QScintilla2" "QScintilla2 libraries" "http://www.riverbankcomputing.co.uk/software/qscintilla/intro" FALSE "" "Needed to compile QScintilla2 bindings")

macro_optional_find_package(Qwt5)
macro_log_feature(Qwt5_Qt4_FOUND "Qwt5 for Qt4" "Qwt5 libraries for Qt4" "http://qwt.sourceforge.net" FALSE "" "Needed to compile Qwt5 bindings")

macro_optional_find_package(Phonon)
macro_log_feature(PHONON_FOUND "Phonon" "Phonon multimedia framework" "http://www.kde.org" FALSE "" "Needed to compile Phonon bindings")

if (RUBY_LIBRARY AND RUBY_INCLUDE_DIRS AND RUBY_EXECUTABLE)
    set(COMPILE_RUBY TRUE CACHE INTERNAL "")
endif (RUBY_LIBRARY AND RUBY_INCLUDE_DIRS AND RUBY_EXECUTABLE)

macro_log_feature(COMPILE_RUBY "Ruby" "Ruby interpreter and libraries" "http://www.ruby-lang.org" FALSE "" "Needed to compile the Ruby bindings")

if (NOT COMPILE_RUBY)
    return()
endif (NOT COMPILE_RUBY)

SET(CUSTOM_RUBY_SITE_ARCH_DIR ${RUBY_SITEARCH_DIR} CACHE DIR "custom installation directory for ruby binary extension" )
SET(CUSTOM_RUBY_SITE_LIB_DIR ${RUBY_SITELIB_DIR} CACHE DIR "custom installation directory for ruby extension" )

# compute an overall version number which can be compared at once
MATH(EXPR RUBY_VERSION_NUMBER "${RUBY_VERSION_MAJOR}*10000 + ${RUBY_VERSION_MINOR}*100 + ${RUBY_VERSION_PATCH}")


message(STATUS "Ruby Executable: " ${RUBY_EXECUTABLE})
message(STATUS "Ruby Libraries: " ${RUBY_LIBRARY})
message(STATUS "Ruby Include Directories: " ${RUBY_INCLUDE_DIRS})

add_subdirectory( bin )
add_subdirectory( modules )
add_subdirectory( rails_support )
add_subdirectory( src )
add_subdirectory( tools )

add_subdirectory( cmake ) #needs to be after 'modules'

macro_display_feature_log()
