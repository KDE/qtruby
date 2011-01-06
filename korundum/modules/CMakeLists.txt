project(KorundumModules)

macro_optional_find_package(Soprano)
macro_log_feature(SOPRANO_FOUND "Soprano" "Soprano libraries" "http://soprano.sourceforge.net" FALSE "" "Needed to compile Soprano bindings")

macro_optional_find_package(Nepomuk)
macro_log_feature(NEPOMUK_FOUND "Nepomuk" "Nepomuk libraries" "http://www.kde.org" FALSE "" "Needed to compile Nepomuk bindings")

macro_optional_find_package(KdepimLibs)
macro_log_feature(KDEPIMLIBS_FOUND "kdepimlibs" "KDE PIM libraries" "http://www.kde.org" FALSE "" "Needed to compile Akonadi bindings")

macro_optional_find_package(Akonadi)
macro_log_feature(AKONADI_FOUND "Akonadi" "Akonadi libraries" "http://www.kde.org" FALSE "" "Needed to compile Akonadi bindings")

macro_optional_find_package(Okular)
macro_log_feature(OKULAR_FOUND "Okular" "Okular libraries" "http://www.kde.org" FALSE "" "Needed to compile Okular bindings")

macro_optional_add_bindings(Soprano_FOUND "Soprano" soprano)

if(KDEPIMLIBS_FOUND AND Akonadi_FOUND)
    set(BUILD_Akonadi TRUE)
else(KDEPIMLIBS_FOUND AND Akonadi_FOUND)
    set(BUILD_Akonadi FALSE)
endif(KDEPIMLIBS_FOUND AND Akonadi_FOUND)
macro_optional_add_bindings(BUILD_Akonadi "Akonadi" akonadi)

add_subdirectory(kio)
add_subdirectory(khtml)
add_subdirectory(ktexteditor)
add_subdirectory(solid)
add_subdirectory(plasma)

# For nepomuk, we also must have found soprano, since soprano headers are used:
if(Soprano_FOUND AND Nepomuk_FOUND)
    set(BUILD_Nepomuk TRUE)
else(Soprano_FOUND AND Nepomuk_FOUND)
    set(BUILD_Nepomuk FALSE)
endif(Soprano_FOUND AND Nepomuk_FOUND)
macro_optional_add_bindings(BUILD_Nepomuk "Nepomuk" nepomuk)

macro_optional_add_bindings(OKULAR_FOUND "Okular" okular)

find_library(KATEINTERFACES_LIBRARY kateinterfaces HINTS ${KDE4_LIB_DIR})
find_path(KATE_INCLUDE_DIR kate/plugin.h HINTS ${KDE4_INCLUDE_DIR})

macro_optional_add_bindings(KATE_INCLUDE_DIR "Kate" kate)

#####################################################################
# Kross Ruby

OPTION(ENABLE_KROSSRUBY "build Kross Ruby scripting library" ON)
#OPTION(ENABLE_KROSSRUBY "build Kross Ruby scripting library" OFF)

if (RUBY_EXECUTABLE AND RUBY_LIBRARY AND RUBY_INCLUDE_DIRS)
    EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r mkmf -e "exit(have_func('rb_hash_foreach') ? 0 : 1)" RESULT_VARIABLE RUBY_HASH_FOREACH OUTPUT_QUIET)
    if (RUBY_HASH_FOREACH EQUAL 0)

        set(CMAKE_REQUIRED_INCLUDES ${RUBY_INCLUDE_DIRS})
        set(CMAKE_REQUIRED_LIBRARIES ${RUBY_LIBRARY})
        CHECK_CXX_SOURCE_COMPILES("
            #include <ruby.h>
            int main() { ruby_init(); return 0; }" RubyCompiles)
        if(RubyCompiles)
            if(ENABLE_KROSSRUBY AND RUBY_VERSION_MAJOR EQUAL 1 AND RUBY_VERSION_MINOR LESS 9)
                set(BUILD_KrossRuby TRUE)
            endif(ENABLE_KROSSRUBY AND RUBY_VERSION_MAJOR EQUAL 1 AND RUBY_VERSION_MINOR LESS 9)
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

macro_optional_add_bindings(BUILD_KrossRuby "KrossRuby" krossruby)

macro_display_bindings_log()
