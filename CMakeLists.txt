macro_optional_find_package(RUBY)

#####################################################################
# QtRuby/Korundum

OPTION(ENABLE_QTRUBY "build QtRuby" ON)
OPTION(ENABLE_KORUNDUM "build Korundum" OFF)

set(QTRUBY_ENABLED "no")
set(KORUNDUM_ENABLED "no")
if(RUBY_EXECUTABLE)
    if(ENABLE_QTRUBY)
        add_subdirectory( qtruby )
        set(QTRUBY_ENABLED "yes")
    endif(ENABLE_QTRUBY)
    if(ENABLE_KORUNDUM)
        add_subdirectory( korundum )
        set(KORUNDUM_ENABLED "yes")
    endif(ENABLE_KORUNDUM)
endif(RUBY_EXECUTABLE)
message(STATUS "Build QtRuby... " ${QTRUBY_ENABLED})
message(STATUS "Build Korundum... " ${KORUNDUM_ENABLED})

#####################################################################
# Kross Ruby

OPTION(ENABLE_KROSSRUBY "build Kross Ruby scripting library" ON)
#OPTION(ENABLE_KROSSRUBY "build Kross Ruby scripting library" OFF)

set(KROSSRUBY_ENABLED "no")
if(ENABLE_KROSSRUBY)
    if (RUBY_EXECUTABLE AND RUBY_LIBRARY AND RUBY_INCLUDE_PATH)
        EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r mkmf -e "exit(have_func('rb_hash_foreach') ? 0 : 1)" RESULT_VARIABLE RUBY_HASH_FOREACH OUTPUT_QUIET)
        if (RUBY_HASH_FOREACH EQUAL 0)
            add_subdirectory(krossruby)
            set(KROSSRUBY_ENABLED "yes")
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
