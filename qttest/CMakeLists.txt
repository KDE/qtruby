include_directories(${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)

set(rubyqttest_LIBRARY_SRC qttest.cpp)
add_library(rubyqttest MODULE ${rubyqttest_LIBRARY_SRC})
target_link_libraries(rubyqttest ${RUBY_LIBRARY} qtruby4shared QtTest)
set_target_properties(rubyqttest PROPERTIES PREFIX "" OUTPUT_NAME qttest)
install(TARGETS rubyqttest DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
