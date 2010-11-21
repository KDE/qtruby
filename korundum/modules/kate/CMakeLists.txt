add_subdirectory( lib )

include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src ${KATE_INCLUDE_DIR})
INCLUDE_DIRECTORIES (${QT_INCLUDES})

set(kate_LIB_SRCS
   kate.cpp
   katehandlers.cpp )

kde4_add_library(kate SHARED ${kate_LIB_SRCS})

target_link_libraries(kate
    qtruby4shared
    smokekate
    ${KATEINTERFACES_LIBRARY}
)
set_target_properties(kate PROPERTIES PREFIX "")
install(TARGETS kate DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/kate.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/kate.bundle\" )" )
endif (Q_WS_MAC)


