project(Korundum)

set(COMPILE_RUBY FALSE CACHE INTERNAL "")

find_package(Ruby REQUIRED)
find_package(KDE4 REQUIRED)
include(KDE4Defaults)

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


macro_display_bindings_log()

add_subdirectory( examples )
add_subdirectory( src )
add_subdirectory( tools )
