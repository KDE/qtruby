find_package(Qt5OpenGL)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${Qt5OpenGL_INCLUDE_DIRS}
    ${RUBY_INCLUDE_DIRS}
    ${SMOKE_INCLUDE_DIR}
)

set(rubyqtopengl_LIBRARY_SRC qtopengl.cpp qtopenglhandlers.cpp)
add_library(rubyqtopengl MODULE ${rubyqtopengl_LIBRARY_SRC})
target_link_libraries(rubyqtopengl
                            Qt5::Core
                            Qt5::OpenGL
                            ${SMOKE_QTOPENGL_LIBRARY}
                            smokeqt5opengl
                            qtrubyruntime
)
set_target_properties(rubyqtopengl PROPERTIES PREFIX "" OUTPUT_NAME qtopengl)
install(TARGETS rubyqtopengl DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtopengl.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtopengl.bundle\" )" )
endif (Q_WS_MAC)
