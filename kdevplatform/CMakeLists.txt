include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)
INCLUDE_DIRECTORIES (${QT_INCLUDES})

set(kdevplatform_LIBRARY_SRC kdevplatform.cpp kdevplatformhandlers.cpp)

add_library(kdevplatform MODULE ${kdevplatform_LIBRARY_SRC})
target_link_libraries(kdevplatform 
    ${RUBY_LIBRARY} 
    smokeqt 
    smokekde 
    smokektexteditor
    smokekdevplatform 
    qtruby4shared)
set_target_properties(kdevplatform PROPERTIES PREFIX "")
install(TARGETS kdevplatform DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES kdevplatform.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/kdevplatform)
