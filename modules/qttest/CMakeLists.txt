find_package(Qt5Test)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${Qt5Test_INCLUDE_DIRS}
    ${RUBY_INCLUDE_DIRS}
    ${SMOKE_INCLUDE_DIR}
)

set(rubyqttest_LIBRARY_SRC qttest.cpp qttesthandlers.cpp)
add_library(rubyqttest MODULE ${rubyqttest_LIBRARY_SRC})
target_link_libraries(rubyqttest
                            Qt5::Core
                            Qt5::Test
                            ${SMOKE_QTTEST_LIBRARY}
                            smokeqt5test
                            qtrubyruntime
)
set_target_properties(rubyqttest PROPERTIES PREFIX "" OUTPUT_NAME qttest)
install(TARGETS rubyqttest DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES qttest.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/qttest)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qttest.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qttest.bundle\" )" )
endif (Q_WS_MAC)
