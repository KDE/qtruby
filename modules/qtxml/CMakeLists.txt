find_package(Qt5Xml)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${Qt5Xml_INCLUDE_DIRS}
    ${RUBY_INCLUDE_DIRS}
    ${SMOKE_INCLUDE_DIR}
)

set(rubyqtxml_LIBRARY_SRC qtxml.cpp qtxmlhandlers.cpp)
add_library(rubyqtxml MODULE ${rubyqtxml_LIBRARY_SRC})
target_link_libraries(rubyqtxml
                            Qt5::Core
                            Qt5::Xml
                            ${SMOKE_QXML_LIBRARY}
                            smokeqtxml
                            qtrubyruntime
)
set_target_properties(rubyqtxml PROPERTIES PREFIX "" OUTPUT_NAME qtxml)
install(TARGETS rubyqtxml DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtxml.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtxml.bundle\" )" )
endif (Q_WS_MAC)
