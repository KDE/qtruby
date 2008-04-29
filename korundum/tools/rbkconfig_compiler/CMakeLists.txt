
########### next target ###############

set(rbkconfig_compiler4_SRCS rbkconfig_compiler.cpp)


kde4_add_executable(rbkconfig_compiler4 NOGUI RUN_UNINSTALLED ${rbkconfig_compiler4_SRCS})

target_link_libraries(rbkconfig_compiler4  ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} )

install(TARGETS rbkconfig_compiler4 DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############



