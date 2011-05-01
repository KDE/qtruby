include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${QT_INCLUDES}
    ${RUBY_INCLUDE_DIRS}
    ${SMOKE_INCLUDE_DIR}
)

set(rubyqtcore_LIBRARY_SRC
    qtcore.cpp
    rubyqabstractitemmodel.cpp
    rubyqbytearray.cpp
    rubyqobject.cpp
    rubyqvariant.cpp
    typeresolver.cpp
    qtcorehandlers.cpp
    )

add_library(rubyqtcore MODULE ${rubyqtcore_LIBRARY_SRC})
target_link_libraries(rubyqtcore
    ${QT_QTCORE_LIBRARY}
    ${SMOKE_QTCORE_LIBRARY}
    qtrubyruntime
)
set_target_properties(rubyqtcore PROPERTIES PREFIX "" OUTPUT_NAME qtcore)
install(TARGETS rubyqtcore DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES qtcore.rb internal.rb enum.rb base.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/qtcore)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtcore.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtcore.bundle\" )" )
endif (Q_WS_MAC)
