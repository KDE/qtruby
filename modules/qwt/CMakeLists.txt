find_package(Qwt5 REQUIRED)

include_directories( ${CMAKE_SOURCE_DIR}/generator ${RUBY_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES} ${Qwt5_INCLUDE_DIR} )

set(rubyqwt_LIBRARY_SRC qwt.cpp qwthandlers.cpp)
add_library(rubyqwt MODULE ${rubyqwt_LIBRARY_SRC})
target_link_libraries(rubyqwt
    ${SMOKE_QWT_LIBRARY}
    qtruby4shared
)
set_target_properties(rubyqwt PROPERTIES PREFIX "" OUTPUT_NAME qwt)
install(TARGETS rubyqwt DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES qwt.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/qwt)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qwt.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qwt.bundle\" )" )
endif (Q_WS_MAC)

