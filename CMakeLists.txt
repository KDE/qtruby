set(COMPILE_RUBY FALSE CACHE INTERNAL "")

macro_optional_find_package(Ruby)

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

add_subdirectory( qtruby )
add_subdirectory( qtwebkit )
add_subdirectory( qtuitools )
add_subdirectory( qtscript )
add_subdirectory( qttest )
set (ENABLED_RUBY_MODULES "Qt, QtWebKit, QtUiTools, QtScript, QtTest")

if(PHONON_FOUND)
    add_subdirectory( phonon )
    set(ENABLED_RUBY_MODULES "${ENABLED_RUBY_MODULES}, Phonon")
endif(PHONON_FOUND)

if(QSCINTILLA_FOUND)
    add_subdirectory( qscintilla )
    set(ENABLED_RUBY_MODULES "${ENABLED_RUBY_MODULES}, QScintilla")
endif(QSCINTILLA_FOUND)

if(Qwt5_Qt4_FOUND)
    add_subdirectory( qwt )
    set(ENABLED_RUBY_MODULES "${ENABLED_RUBY_MODULES}, Qwt5")
endif(Qwt5_Qt4_FOUND)

if(Soprano_FOUND)
    add_subdirectory( soprano )
    set(ENABLED_RUBY_MODULES "${ENABLED_RUBY_MODULES}, Soprano")
endif(Soprano_FOUND)

if(KDEPIMLIBS_FOUND AND Akonadi_FOUND)
    add_subdirectory( akonadi )
    set(ENABLED_RUBY_MODULES "${ENABLED_RUBY_MODULES}, Akonadi")
endif(KDEPIMLIBS_FOUND AND Akonadi_FOUND)

if(KDE4_FOUND)
    add_subdirectory( korundum )
    add_subdirectory( kio )
    add_subdirectory( khtml )
    add_subdirectory( ktexteditor )
    add_subdirectory( solid )
    add_subdirectory( plasma )
    set(ENABLED_RUBY_MODULES "${ENABLED_RUBY_MODULES}, KDE4, KIO, KHTML, KTextEditor, Solid, Plasma")
endif(KDE4_FOUND)
    
if(KDEVPLATFORM_FOUND)
    add_subdirectory( kdevplatform )
    set(ENABLED_RUBY_MODULES "${ENABLED_RUBY_MODULES}, KDevPlatform")
endif(KDEVPLATFORM_FOUND)

if(Nepomuk_FOUND)
    add_subdirectory( nepomuk )
    set(ENABLED_RUBY_MODULES "${ENABLED_RUBY_MODULES}, Nepomuk")
endif(Nepomuk_FOUND)

if(OKULAR_FOUND)
    add_subdirectory( okular )
    set(ENABLED_RUBY_MODULES "${ENABLED_RUBY_MODULES}, Okular")
endif(OKULAR_FOUND)

message(STATUS "Building Ruby bindings for ${ENABLED_RUBY_MODULES}")

#####################################################################
# Kross Ruby

OPTION(ENABLE_KROSSRUBY "build Kross Ruby scripting library" ON)
#OPTION(ENABLE_KROSSRUBY "build Kross Ruby scripting library" OFF)

set(KROSSRUBY_ENABLED "no")
if(ENABLE_KROSSRUBY)
    if (RUBY_EXECUTABLE AND RUBY_LIBRARY AND RUBY_INCLUDE_DIRS)
        EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r mkmf -e "exit(have_func('rb_hash_foreach') ? 0 : 1)" RESULT_VARIABLE RUBY_HASH_FOREACH OUTPUT_QUIET)
        if (RUBY_HASH_FOREACH EQUAL 0)

            set(CMAKE_REQUIRED_INCLUDES ${RUBY_INCLUDE_DIRS})
            set(CMAKE_REQUIRED_LIBRARIES ${RUBY_LIBRARY})
            CHECK_CXX_SOURCE_COMPILES("
                #include <ruby.h>
                int main() { ruby_init(); return 0; }" RubyCompiles)
            if(RubyCompiles)
                add_subdirectory(krossruby)
                set(KROSSRUBY_ENABLED "yes")
            else(RubyCompiles)
                message(STATUS "Failed to compile a C++ program that embeds ruby.")
            endif(RubyCompiles)

        else (RUBY_HASH_FOREACH EQUAL 0)

            # 2007-02-12 sebsauer; this is a workaround for the case we failed to load
            # the "mkmf" library what is the case e.g. on some debian distributions.
            #EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r mkmf -e "exit(0)" RESULT_VARIABLE RUBY_HASH_FOREACH OUTPUT_QUIET ERROR_QUIET)
            #if (${RUBY_HASH_FOREACH} EQUAL 0)
            message(STATUS "The detected Ruby is too old for kross. A Ruby which has rb_hash_foreach is required. Upgrade to 1.8.2 or later")
            #else (${RUBY_HASH_FOREACH} EQUAL 0)
            #    message(STATUS "Failed to load the Ruby mkmf library. Please install the Ruby developer packages.")
            #endif (${RUBY_HASH_FOREACH} EQUAL 0)

        endif (RUBY_HASH_FOREACH EQUAL 0)
    endif (RUBY_EXECUTABLE AND RUBY_LIBRARY AND RUBY_INCLUDE_DIRS)
    message(STATUS "Ruby Executable: " ${RUBY_EXECUTABLE})
    message(STATUS "Ruby Libraries: " ${RUBY_LIBRARY})
    message(STATUS "Ruby Include Directories: " ${RUBY_INCLUDE_DIRS})
endif(ENABLE_KROSSRUBY)
message(STATUS "Build Kross Ruby... " ${KROSSRUBY_ENABLED})
