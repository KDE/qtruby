macro_optional_find_package(RUBY)

#####################################################################
# QtRuby/Korundum

OPTION(ENABLE_QTRUBY "build QtRuby" ON)
OPTION(ENABLE_KORUNDUM "build Korundum" ON)
OPTION(ENABLE_QSCIRUBY "build QsciRuby" ON)

SET(CUSTOM_RUBY_SITE_ARCH_DIR ${RUBY_SITE_ARCH_DIR} CACHE DIR "custom installation directory for ruby binary extension" )
SET(CUSTOM_RUBY_SITE_LIB_DIR ${RUBY_SITE_LIB_DIR} CACHE DIR "custom installation directory for ruby extension" )

set(QTRUBY_ENABLED "no")
set(KORUNDUM_ENABLED "no")
set(QSCIRUBY_ENABLED "no")

if(RUBY_EXECUTABLE AND RUBY_LIBRARY AND RUBY_INCLUDE_PATH)
    if(ENABLE_QTRUBY)
        add_subdirectory( qtruby )
        set(QTRUBY_ENABLED "yes")
    endif(ENABLE_QTRUBY)
    if(ENABLE_KORUNDUM)
        add_subdirectory( korundum )
        set(KORUNDUM_ENABLED "yes")
    endif(ENABLE_KORUNDUM)
    if(ENABLE_QSCIRUBY)
        add_subdirectory( qsci )
        set(QSCI_ENABLED "yes")
    endif(ENABLE_QSCIRUBY)
endif(RUBY_EXECUTABLE AND RUBY_LIBRARY AND RUBY_INCLUDE_PATH)
message(STATUS "Build QtRuby... " ${QTRUBY_ENABLED})
message(STATUS "Build Korundum... " ${KORUNDUM_ENABLED})
message(STATUS "Build QsciRuby... " ${QSCIRUBY_ENABLED})

#####################################################################
# Kross Ruby

OPTION(ENABLE_KROSSRUBY "build Kross Ruby scripting library" ON)
#OPTION(ENABLE_KROSSRUBY "build Kross Ruby scripting library" OFF)

set(KROSSRUBY_ENABLED "no")
if(ENABLE_KROSSRUBY)
    if (RUBY_EXECUTABLE AND RUBY_LIBRARY AND RUBY_INCLUDE_PATH)
        EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r mkmf -e "exit(have_func('rb_hash_foreach') ? 0 : 1)" RESULT_VARIABLE RUBY_HASH_FOREACH OUTPUT_QUIET)
        if (RUBY_HASH_FOREACH EQUAL 0)

            set(CMAKE_REQUIRED_INCLUDES ${RUBY_INCLUDE_PATH})
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
    endif (RUBY_EXECUTABLE AND RUBY_LIBRARY AND RUBY_INCLUDE_PATH)
    message(STATUS "Ruby Executable: " ${RUBY_EXECUTABLE})
    message(STATUS "Ruby Libraries: " ${RUBY_LIBRARY})
    message(STATUS "Ruby Include Path: " ${RUBY_INCLUDE_PATH})
endif(ENABLE_KROSSRUBY)
message(STATUS "Build Kross Ruby... " ${KROSSRUBY_ENABLED})
