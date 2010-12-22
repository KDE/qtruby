project(Ruby)

set(COMPILE_RUBY FALSE CACHE INTERNAL "")

find_package(Ruby REQUIRED)
find_package(Qt4 REQUIRED)

if (RUBY_LIBRARY AND RUBY_INCLUDE_DIRS AND RUBY_EXECUTABLE)
    set(COMPILE_RUBY TRUE CACHE INTERNAL "")
endif (RUBY_LIBRARY AND RUBY_INCLUDE_DIRS AND RUBY_EXECUTABLE)

macro_log_feature(COMPILE_RUBY "Ruby" "Ruby interpreter and libraries" "http://www.ruby-lang.org" FALSE "" "Needed to compile the Ruby bindings")

macro_optional_add_bindings(COMPILE_RUBY "QtRuby" qtruby)
macro_optional_add_bindings(COMPILE_RUBY "Korundum" korundum)

macro_display_bindings_log()
